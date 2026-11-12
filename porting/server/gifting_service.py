"""Local gifting across profiles.

GiftCatalogEntry and RemoveGiftBox are observed command names. The client
reflects ReceiverAccountIds, GiftWrapTemplateId, UserMessageToRecipients and
GiftBoxItemIds; request JSON keys and the gift_box attribute are inferred.
FGiftBoxInfo and FMcpLootEntry members are reflected. No receiver notification
transport or real-service gift limits have been observed.
"""
import copy
import json
import logging
import re
import sqlite3
from contextlib import closing

from api_common import ApiError, Err
from catalog import cosmetic_attributes, get_offer
from profile_defaults import CATEGORY_BY_PREFIX, build
from profile_service import CommandContext, ProfileTx, _debit_local_currency, _invalid


class GiftingService:
    def __init__(self, ctx, accounts, profiles, friends, notify=None):
        self.ctx, self.accounts, self.profiles, self.friends = ctx, accounts, profiles, friends
        self.notify = notify

    def _snapshot(self, account, profile_id, db):
        # Load inside the write transaction; no lazy profile creation or cache
        # publication can escape a failed gift.
        if db is not None:
            row = db.execute('SELECT data FROM profiles WHERE account_id=? AND profile_id=?',
                             (account, profile_id)).fetchone()
            if row:
                return json.loads(row[0])
        else:
            cached = self.profiles._profiles.get((account, profile_id))
            if cached is not None:
                return copy.deepcopy(cached)
        items, stats = build(self.ctx, account, profile_id)
        now = self.ctx.iso()
        return {'_id': account, 'created': now, 'updated': now, 'rvn': 1,
                'wipeNumber': 1, 'accountId': account, 'profileId': profile_id,
                'version': 'no_version', 'items': items, 'stats': {'attributes': stats},
                'commandRevision': 1}

    def run(self, account_id, profile_id, command_name, body, client_rvn=None):
        if profile_id != 'common_core':
            raise ApiError(400, Err.INVALID_PROFILE_COMMAND, f'{command_name} cannot be used on the {profile_id} profile.')
        if not isinstance(body, dict):
            raise _invalid('Body must be an object.')
        db = sqlite3.connect(self.profiles._database) if self.profiles._database is not None else None
        try:
            if db is not None:
                db.execute('BEGIN IMMEDIATE')
            txs = {}
            def tx(account, pid):
                key = (account, pid)
                if key not in txs:
                    txs[key] = ProfileTx(self.ctx, self._snapshot(account, pid, db))
                return txs[key]
            primary = tx(account_id, profile_id)
            context = CommandContext(self.profiles, account_id, profile_id, body)
            context.txs['common_core'] = primary
            events = []
            if command_name == 'GiftCatalogEntry':
                events = self._gift(context, tx)
            elif command_name == 'RemoveGiftBox':
                self._remove(context, primary)
            else:
                raise ApiError(400, Err.OPERATION_NOT_FOUND, f'Operation {command_name} not found.')
            by_account = {}
            for (account, pid), value in txs.items():
                by_account.setdefault(account, {})[pid] = value
            for account, items in by_account.items():
                self.profiles._store(account, items, command_profile=profile_id if account == account_id else None,
                                     database=db, publish=False)
            if db is not None:
                db.commit()
            for account, items in by_account.items():
                self.profiles._publish(account, items)
            # This local event and its shape are inferred, not an Epic protocol claim.
            if self.notify is not None:
                for recipient, event in events:
                    try:
                        self.notify(recipient, event)
                    except Exception:
                        logging.exception('Gift notification failed after commit')
            if client_rvn is not None and client_rvn != str(primary.base_rvn):
                changes = [{'changeType': 'fullProfileUpdate', 'profile': copy.deepcopy(primary.profile)}]
                base = primary.profile['rvn']
            else:
                changes, base = primary.changes, primary.base_rvn
            entry = self.profiles._entry(profile_id, primary.profile, base, changes)
            return self.profiles._envelope(entry, context.notifications, [])
        finally:
            if db is not None:
                db.close()  # Rolls back an uncommitted transaction on errors.

    def _recipient_failure(self, sender, recipient, folded, tx):
        """Shared local policy; callers choose the protocol error mapping."""
        if self.accounts.get(recipient) is None:
            return 'missing'
        if (sender == recipient or self.friends.is_blocked(sender, recipient)
                or not self.friends.are_friends(sender, recipient)):
            return 'unavailable'
        core, athena = tx(recipient, 'common_core'), tx(recipient, 'athena')
        if core.profile['stats']['attributes'].get('allowed_to_receive_gifts') is not True:
            return 'declined'
        owned = {item['templateId'].casefold() for item in athena.profile['items'].values()}
        return 'owned' if owned.intersection(folded) else None

    def eligibility(self, req, match):
        # Authentication and error mappings are local policy. The subcatalog
        # error strings themselves are observed in the client.
        sender = self.profiles.oauth.account_session(req).account_id
        recipient, offer_id = match.group(1, 2)
        offer = get_offer(offer_id)
        if not offer or not offer.get('giftInfo', {}).get('bIsEnabled'):
            raise ApiError(400, Err.INVALID_PAYLOAD, 'Offer cannot be gifted.')
        templates = self._offer_templates(offer)
        db = sqlite3.connect(self.profiles._database) if self.profiles._database is not None else None
        try:
            if db is not None:
                db.execute('BEGIN')
            txs = {}
            def tx(account, pid):
                key = (account, pid)
                if key not in txs:
                    txs[key] = ProfileTx(self.ctx, self._snapshot(account, pid, db))
                return txs[key]
            failure = self._recipient_failure(sender, recipient, {t.casefold() for t in templates}, tx)
        finally:
            if db is not None:
                db.close()
        if failure == 'missing':
            raise ApiError(404, Err.ACCOUNT_NOT_FOUND, 'Recipient account was not found.')
        if failure in ('declined', 'owned'):
            code = 'receiver_will_not_accept_gifts' if failure == 'declined' else 'purchase_not_allowed'
            raise ApiError(400, 'errors.com.epicgames.modules.gamesubcatalog.' + code,
                           'Recipient cannot receive this gift.')
        if failure:
            raise ApiError(400, Err.INVALID_PAYLOAD, 'Recipient is unavailable for gifting.')
        return 204, None

    def register(self, router):
        router.add('GET', r'(?:/fortnite)?/api/storefront/v2/gift/check_eligibility/recipient/([^/]+)/offer/([^/]+)',
                   self.eligibility, name='gift.recipient_eligibility', status='inferred')

    @staticmethod
    def _offer_templates(offer):
        grants = offer.get('itemGrants')
        prefixes = {p.casefold() for p in CATEGORY_BY_PREFIX}
        if not isinstance(grants, list) or not grants:
            raise _invalid('Offer has no cosmetic grants.')
        templates = []
        for grant in grants:
            if not isinstance(grant, dict):
                raise _invalid('Invalid cosmetic grant.')
            template, quantity = grant.get('templateId'), grant.get('quantity')
            if (not isinstance(template, str) or ':' not in template or not template.split(':', 1)[1]
                    or template.split(':', 1)[0].casefold() not in prefixes or type(quantity) is not int or quantity != 1):
                raise _invalid('Invalid cosmetic grant.')
            templates.append(template)
        if len({t.casefold() for t in templates}) != len(templates):
            raise _invalid('Duplicate cosmetic grant.')
        return templates

    def _gift(self, ctx, tx):
        offer = get_offer(ctx.str_field('offerId'))
        recipients = ctx.list_field('receiverAccountIds')
        if not recipients or len(recipients) > 100 or any(not isinstance(a, str) or not re.fullmatch(r'[0-9a-f]{32}', a) for a in recipients):
            raise _invalid('receiverAccountIds must contain account IDs.')
        if len(set(recipients)) != len(recipients) or ctx.account_id in recipients:
            raise _invalid('Recipients must be distinct and cannot include the sender.')
        wrap = ctx.str_field('giftWrapTemplateId')
        if not re.fullmatch(r'GiftBox:[A-Za-z0-9_]+', wrap, re.IGNORECASE):
            raise _invalid('Invalid gift wrap template.')
        # Local resource bounds, not claimed to match the live service.
        message = ctx.str_field('userMessageToRecipients', required=False, allow_empty=True, max_len=1000)
        legacy_message = ctx.str_field('personalMessage', required=False, allow_empty=True, max_len=1000)
        if message is not None and legacy_message is not None and message != legacy_message:
            raise _invalid('Conflicting gift messages.')
        if message is None:
            message = legacy_message if legacy_message is not None else ''
        currency = ctx.str_field('currency')
        subtype = ctx.str_field('currencySubType', allow_empty=True)
        expected = ctx.int_field('expectedTotalPrice')
        if not offer or not offer.get('giftInfo', {}).get('bIsEnabled'):
            raise _invalid('Offer cannot be gifted.')
        prices = [p for p in offer.get('prices', []) if p.get('currencyType') == currency and p.get('currencySubType', '') == subtype]
        if currency != 'MtxCurrency' or subtype != '' or len(prices) != 1:
            raise _invalid('Unsupported offer currency.')
        price = prices[0].get('finalPrice')
        if isinstance(price, bool) or not isinstance(price, int) or price < 0 or expected != price * len(recipients):
            raise _invalid('Offer price does not match expectedTotalPrice.')
        templates = self._offer_templates(offer)
        folded = {t.casefold() for t in templates}
        for recipient in recipients:
            failure = self._recipient_failure(ctx.account_id, recipient, folded, tx)
            if failure:
                raise _invalid({'missing': 'Recipient is unavailable for gifting.',
                                'unavailable': 'Recipient is unavailable for gifting.',
                                'declined': 'Recipient cannot receive gifts.',
                                'owned': 'A recipient already owns a cosmetic in this offer.'}[failure])
        _debit_local_currency(ctx, expected)
        events = []
        for recipient in recipients:
            core, athena = tx(recipient, 'common_core'), tx(recipient, 'athena')
            loot = []
            for template in templates:
                item_id = self.profiles._new_item_id('item', athena.profile)
                attrs = cosmetic_attributes(template)
                athena.add_item(item_id, template, attrs)
                loot.append({'itemType': template, 'itemGuid': item_id, 'quantity': 1,
                             'attributes': attrs, 'itemProfile': 'athena'})
            box_id = self.profiles._new_item_id('gift', core.profile)
            core.add_item(box_id, wrap, {'gift_box': {'giftedOn': self.ctx.iso(),
                'fromAccountId': ctx.account_id, 'lootList': loot,
                'params': {'personalMessage': message}}})
            events.append((recipient, {'type': 'gift_received', 'senderAccountId': ctx.account_id,
                                       'giftBoxItemId': box_id}))
        return events

    def _remove(self, ctx, core):
        ids = ctx.list_field('giftBoxItemIds')
        if any(not isinstance(item_id, str) or not item_id for item_id in ids):
            raise _invalid('giftBoxItemIds must contain item IDs.')
        for item_id in set(ids):
            item = core.profile['items'].get(item_id)
            if item is not None:
                if not item['templateId'].casefold().startswith('giftbox:'):
                    raise _invalid('Item is not a gift box.')
                core.remove_item(item_id)
