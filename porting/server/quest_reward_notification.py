"""Consumer-supported explicit claim response, built after reward conversions.

This is not evidence for unsolicited notifications or native popup presentation.
Local representation policy: stat XP has no inventory instance, so its GUID
and attributes are empty; consumed style tokens use their newly unlocked owned
cosmetic target with final variant attributes and quantity one.
"""
import copy

from banner_reward_service import MAPPINGS as BANNERS
from season12_progression_data import XP_TO_NEXT


def begin_claim_notification(ctx, quest_id):
    # Future destinations are captured by CommandContext.tx before mutation;
    # do not create an otherwise-unused profile merely to build a receipt.
    ctx._quest_claim_notification = (
        quest_id,
        {pid: copy.deepcopy(tx.profile) for pid, tx in ctx.txs.items()},
        {pid: len(tx.changes) for pid, tx in ctx.txs.items()},
        len(ctx.notifications),
    )


def _total_xp(profile):
    stats = profile['stats']['attributes']
    return sum(XP_TO_NEXT[:stats['level'] - 1]) + stats.get('xp', 0)


def finish_claim_notification(ctx):
    pending = getattr(ctx, '_quest_claim_notification', None)
    if pending is None:
        return
    del ctx._quest_claim_notification
    quest_id, before, offsets, notification_offset = pending
    # Retain provenance of newly awarded conversion tokens even when consumed.
    fresh = set()
    for pid, tx in ctx.txs.items():
        old = before.get(pid, {}).get('items', {})
        working = copy.deepcopy(old)
        for change in tx.changes[offsets.get(pid, 0):]:
            kind, guid = change['changeType'], change.get('itemId')
            if kind == 'itemAdded':
                working[guid] = copy.deepcopy(change['item'])
                fresh.add(change['item']['templateId'].casefold())
            elif kind == 'itemQuantityChanged' and guid in working:
                if change['quantity'] > working[guid]['quantity']:
                    fresh.add(working[guid]['templateId'].casefold())
                working[guid]['quantity'] = change['quantity']
            elif kind == 'itemRemoved':
                working.pop(guid, None)
    from quest_service import TOKENS, FACTION_VARIANTS, DONUT_VARIANTS
    expected_variants = {(spec[0].casefold(), spec[1], spec[2]) for token, spec in
                         {**FACTION_VARIANTS, **DONUT_VARIANTS}.items() if token in fresh}
    expected_variants.update(
        ('athenacharacter:cid_701_athena_commando_m_bananaagent', 'Progressive', stage)
        for token, stage in TOKENS.items() if token in fresh)
    stale_banners = {icon.casefold() for profile in before.values()
                     for item in profile['items'].values()
                     if (icon := BANNERS.get(item['templateId'].casefold()))
                     and item['templateId'].casefold() not in fresh}
    loot = []
    for pid, tx in ctx.txs.items():
        old = before.get(pid, {}).get('items', {})
        for guid, item in tx.profile['items'].items():
            previous = old.get(guid)
            template = item['templateId'].casefold()
            quantity = item['quantity'] - (previous['quantity'] if previous else 0)
            # Existing cosmetics represent a newly unlocked variant only when
            # this claim awarded the specific newly owned channel option.
            if quantity <= 0 and previous:
                attrs = item['attributes']
                owned_before = {v['channel']: set(v.get('owned', []))
                                for v in previous['attributes'].get('variants', [])}
                if any((template, v['channel'], option) in expected_variants
                       for v in attrs.get('variants', [])
                       for option in set(v.get('owned', [])) - owned_before.get(v['channel'], set())):
                    quantity = 1
            if quantity <= 0:
                continue
            # Global reconciliation can consume older pending banner tokens;
            # those are not attributable to this claim.
            if template in stale_banners:
                continue
            loot.append({'itemType': item['templateId'], 'itemGuid': guid,
                         'quantity': quantity, 'attributes': copy.deepcopy(item['attributes']),
                         'itemProfile': pid})
    xp = _total_xp(ctx.txs['athena'].profile) - _total_xp(before['athena'])
    if xp > 0:
        loot.append({'itemType': 'AccountResource:athenaseasonalxp', 'itemGuid': '',
                     'quantity': xp, 'attributes': {}, 'itemProfile': 'athena'})
    # Nested battle-pass grants are already represented by final inventory
    # effects here. Remove their newly generated catalog notifications so they
    # cannot dispatch a second generic purchase presentation.
    ctx.notifications[notification_offset:] = [
        notification for notification in ctx.notifications[notification_offset:]
        if notification.get('type') != 'CatalogPurchase']
    ctx.notifications.append({'primary': True, 'questId': quest_id,
                              'loot': {'tierGroupName': '', 'items': loot}})
