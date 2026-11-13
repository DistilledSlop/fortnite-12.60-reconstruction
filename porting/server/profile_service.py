"""MCP profile store and command set (local stand-in for the Fortnite MCP profile service).

Route: POST /fortnite/api/game/v2/profile/{accountId}/{client|dedicated_server|public}/{command}
Query: profileId (athena, common_core, common_public, campaign, creative), rvn.

Evidence:
  observed  the route templates (McpClientCommandUrl, McpDedicatedServerCommandUrl,
            McpPublicCommandUrl in DefaultEngine.ini); QueryProfile returns a
            fullProfileUpdate (earlier login capture); the envelope and change-type
            names (profileChanges, profileChangesBaseRevision, profileRevision,
            profileCommandRevision, multiUpdate, notifications, serverTime,
            responseVersion, fullProfileUpdate, itemAdded, itemRemoved,
            itemAttrChanged, itemQuantityChanged, statModified); the MCP log lines
            such as "Received profile deltas but was not on the previous revision".
  inferred  request body field names for each command, and the rules for when a
            command bumps profileRevision.
  guess     EquipBattleRoyaleCustomization and SetBattleRoyaleBanner. Neither name is
            in the 12.60 binary strings, so the client may never call them.

Revision model:
  * profileRevision (rvn) rises by 1 for each committed command that changed the
    profile. A no-op command leaves it alone.
  * profileCommandRevision rises by 1 for every executed command, changed or not.
  * For deltas, profileChangesBaseRevision is the revision before the command.
    A mismatched client rvn gets a full snapshot instead, based at its revision;
    this recovery is a local policy, not captured server behavior.
  * A command is atomic. Any ApiError means the stored profile is unchanged.
  * multiUpdate lists the other profiles that a command changed.
"""

import copy
import json
import re
import secrets
import sqlite3
from contextlib import closing
from dataclasses import dataclass
from pathlib import Path
from typing import Callable

from api_common import ApiError, Err
from banner_inventory import repair_starter_items
from banner_reward_service import convert_banner_tokens
from catalog import LOCAL_CREDIT_OFFER, SEASON12_LEVEL_OFFER, SEASON12_PASS_OFFER, cosmetic_attributes, get_offer
from season12_rewards import FREE as SEASON12_FREE, PAID as SEASON12_PAID
from season12_grants import SUPPORTED_GRANTS_TO_EVERYONE, initial_grant_attributes
from profile_defaults import (CATEGORY_BY_PREFIX, MAX_SLOT_INDEX, WRAP_SLOT_COUNT, MTX_PLATFORMS, PROFILE_IDS,
                              SLOT_CATEGORIES, build, empty_slots)

# lockExpiration is an FDateTime (reflected member of FProfileUpdateSingle, sent as ISO 8601). A profile
# that is not locked carries FDateTime::MinValue. The value is a convention; the member is reflected.
LOCK_EXPIRATION_UNLOCKED = "0001-01-01T00:00:00.000Z"

# observed: these names are in the 12.60 binary but have no implementation here yet.
UNIMPLEMENTED = ("RefreshExpeditions", "SetActiveHeroLoadout", "ClearHeroLoadout")

COMMANDS = {}


@dataclass(frozen=True)
class Command:
    name: str
    profiles: frozenset
    name_status: str  # observed | guess
    body: str  # which body fields the command reads, and their evidence label
    fn: Callable


def command(name, profiles, name_status, body):
    def register(fn):
        COMMANDS[name] = Command(name, frozenset(profiles), name_status, body, fn)
        return fn
    return register


class ProfileTx:
    """Edits one profile on a private copy. The service stores the copy only on success."""

    def __init__(self, ctx, profile):
        self.ctx = ctx
        self.profile = copy.deepcopy(profile)
        self.base_rvn = profile["rvn"]
        self.base_command_revision = profile["commandRevision"]
        self.changes = []

    def item(self, item_id):
        item = self.profile["items"].get(item_id)
        if item is None:
            raise ApiError(400, Err.INVALID_PAYLOAD, f"The item {item_id} is not owned by this profile.")
        return item

    def set_item_attr(self, item_id, name, value):
        item = self.item(item_id)
        if name in item["attributes"] and item["attributes"][name] == value:
            return False
        item["attributes"][name] = value
        self.changes.append({"changeType": "itemAttrChanged", "itemId": item_id,
                             "attributeName": name, "attributeValue": value})
        return True

    def set_item_quantity(self, item_id, value):
        item = self.item(item_id)
        if item["quantity"] == value:
            return False
        item["quantity"] = value
        self.changes.append({"changeType": "itemQuantityChanged", "itemId": item_id,
                             "quantity": value})
        return True

    def set_stat(self, name, value):
        attributes = self.profile["stats"]["attributes"]
        if name in attributes and attributes[name] == value:
            return False
        attributes[name] = value
        self.changes.append({"changeType": "statModified", "name": name, "value": value})
        return True

    def add_item(self, item_id, template_id, attributes, quantity=1):
        # The profile owns its item data; callers may reuse or mutate nested
        # attribute values after a grant without changing stored state.
        item = {"templateId": template_id, "attributes": copy.deepcopy(attributes), "quantity": quantity}
        self.profile["items"][item_id] = item
        self.changes.append({"changeType": "itemAdded", "itemId": item_id, "item": copy.deepcopy(item)})

    def remove_item(self, item_id):
        self.item(item_id)
        del self.profile["items"][item_id]
        self.changes.append({"changeType": "itemRemoved", "itemId": item_id})

    @property
    def changed(self):
        return bool(self.changes)


class CommandContext:
    """What a command handler can read and change. Validators raise ApiError(400)."""

    def __init__(self, service, account_id, profile_id, body):
        self.service = service
        self.ctx = service.ctx
        self.account_id = account_id
        self.profile_id = profile_id
        self.body = body
        self.txs = {}
        self.notifications = []

    def tx(self, profile_id):
        if profile_id not in self.txs:
            self.txs[profile_id] = ProfileTx(self.ctx, self.service.profile(self.account_id, profile_id))
        return self.txs[profile_id]

    def str_field(self, key, *, required=True, choices=None, pattern=None, max_len=None, allow_empty=False):
        if key not in self.body:
            if required:
                raise _invalid(f"{key} is required.")
            return None
        value = self.body[key]
        if not isinstance(value, str):
            raise _invalid(f"{key} must be a string.")
        if not value and not allow_empty:
            raise _invalid(f"{key} must not be empty.")
        if max_len is not None and len(value) > max_len:
            raise _invalid(f"{key} is longer than {max_len} characters.")
        if choices is not None and value not in choices:
            raise _invalid(f"{key} must be one of {', '.join(choices)}.")
        if pattern is not None and value and not re.fullmatch(pattern, value):
            raise _invalid(f"{key} has an invalid format.")
        return value

    def bool_field(self, key):
        value = self.body.get(key)
        if not isinstance(value, bool):
            raise _invalid(f"{key} must be true or false.")
        return value

    def int_field(self, key, *, default=None, minimum=0, maximum=None):
        if key not in self.body:
            if default is None:
                raise _invalid(f"{key} is required.")
            return default
        value = self.body[key]
        if isinstance(value, bool) or not isinstance(value, int):
            raise _invalid(f"{key} must be an integer.")
        if value < minimum or (maximum is not None and value > maximum):
            raise _invalid(f"{key} is out of range.")
        return value

    def list_field(self, key):
        value = self.body.get(key)
        if not isinstance(value, list):
            raise _invalid(f"{key} must be a list.")
        return value


def _invalid(message):
    return ApiError(400, Err.INVALID_PAYLOAD, message)


def _check_variant_updates(body):
    updates = body.get("variantUpdates", [])
    if not isinstance(updates, list):
        raise _invalid("variantUpdates must be a list.")
    return updates


def _locker(ctx, locker_id):
    tx = ctx.tx("athena")
    item = tx.item(locker_id)
    if not item["templateId"].startswith("CosmeticLocker:"):
        raise _invalid("lockerItem must refer to a cosmetic locker.")
    return tx, item


def _equip(ctx, locker_id, category, item_id, slot_index, variant_updates=None, *, allow_wrap_all=False):
    """Place an owned cosmetic into one slot of a locker. An empty item_id clears the slot."""
    if category not in SLOT_CATEGORIES:
        raise _invalid(f"category {category} is not a slot category.")
    apply_all = category == "ItemWrap" and slot_index == -1 and allow_wrap_all
    max_index = WRAP_SLOT_COUNT - 1 if category == "ItemWrap" else MAX_SLOT_INDEX
    if not apply_all and not 0 <= slot_index <= max_index:
        raise _invalid("slotIndex is out of range.")
    tx, locker = _locker(ctx, locker_id)
    slots = copy.deepcopy(locker["attributes"].get("locker_slots_data", {"slots": empty_slots()}))
    entry = slots["slots"].setdefault(category, {"items": [], "activeVariants": []})
    updates = variant_updates or []
    if item_id == "":
        if updates:
            raise _invalid("Cannot apply variants to an empty slot.")
        template = ""
        variants = []
    else:
        # Clients can send either an owned item instance ID or its template ID.
        # Prefer an exact instance ID if the two namespaces happen to overlap.
        resolved_id = item_id
        item = tx.profile["items"].get(resolved_id)
        if item is None:
            resolved_id = next((key for key, owned in tx.profile["items"].items()
                                if owned["templateId"].casefold() == item_id.casefold()), None)
            item = tx.profile["items"].get(resolved_id)
        if item is None:
            raise _invalid(f"The item {item_id} is not owned by this profile.")
        template = item["templateId"]
        prefix = template.split(":", 1)[0]
        if not any(key.casefold() == prefix.casefold() and value == category
                   for key, value in CATEGORY_BY_PREFIX.items()):
            raise _invalid(f"{template} cannot be placed in the {category} slot.")
        variants = copy.deepcopy(item["attributes"].get("variants", []))
        seen = set()
        for update in updates:
            if not isinstance(update, dict):
                raise _invalid("variantUpdates must contain objects.")
            channel, active, owned = update.get("channel"), update.get("active"), update.get("owned", [])
            if (not isinstance(channel, str) or not channel or not isinstance(active, str) or not active
                    or not isinstance(owned, list) or not all(isinstance(value, str) for value in owned)):
                raise _invalid("Invalid variant update.")
            if channel in seen:
                raise _invalid("Duplicate variant channel.")
            seen.add(channel)
            variant = next((v for v in variants if isinstance(v, dict) and v.get("channel") == channel), None)
            maya_item = template.casefold() == 'athenacharacter:cid_695_athena_commando_f_desertopscamo'
            maya_color = maya_item and channel == 'RichColor'
            if maya_color:
                # RichColor is the owned tag; active additionally carries raw
                # color words. Maya's permanently chosen color may be equipped
                # again, but locker updates cannot replace it with a new color.
                owned_active = (variant is not None and
                                isinstance(variant.get('owned'), list) and
                                'RichColor' in variant['owned'] and
                                active == variant.get('active') and
                                re.fullmatch(r'RichColor\.[0-9A-Fa-f]{32}', active))
            else:
                owned_active = (variant is not None and
                                isinstance(variant.get('owned'), list) and active in variant['owned'])
                if maya_item and variant is not None and active != variant.get('active'):
                    # Maya choices are permanent; only UnlockRewardNode changes
                    # this outfit's selected styles. Ordinary locker equip can
                    # reapply the existing choice, including its default tag.
                    owned_active = False
            if not owned_active:
                raise _invalid("Variant is not owned by this cosmetic.")
            variant["active"] = active
        if updates:
            tx.set_item_attr(resolved_id, "variants", variants)
    items = list(entry.get("items", []))
    indices = range(WRAP_SLOT_COUNT) if apply_all else (slot_index,)
    while len(items) <= max(indices):
        items.append("")
    for index in indices:
        items[index] = template
    entry["items"] = items
    active_variants = list(entry.get("activeVariants", []))[:len(items)]
    while len(active_variants) < len(items):
        active_variants.append({"variants": []})
    for index in indices:
        active_variants[index] = {"variants": copy.deepcopy(variants)}
    entry["activeVariants"] = active_variants
    tx.set_item_attr(locker_id, "locker_slots_data", slots)


# Commands ---------------------------------------------------------------

def _debit_local_currency(ctx, price):
    core = ctx.tx("common_core")
    wallets = [(item_id, item) for item_id, item in core.profile["items"].items()
               if item["templateId"].casefold() == "currency:mtxgiveaway"]
    if len(wallets) != 1:
        raise _invalid("Local currency balance is unavailable.")
    wallet_id, wallet = wallets[0]
    balance = wallet.get("quantity")
    if isinstance(balance, bool) or not isinstance(balance, int) or balance < price:
        raise _invalid("Insufficient local currency balance.")
    core.set_item_quantity(wallet_id, balance - price)


def _grant_season12_rewards(ctx, athena, rewards, price):
    """Validate the full grant before committing any debit or profile changes."""
    owned = {item["templateId"].casefold() for item in athena.profile["items"].values()}
    from season12_maya_graph_data import GRAPH_TEMPLATE, PURCHASE_TOKEN
    if any(item['templateId'].casefold() == GRAPH_TEMPLATE.casefold() and
           item['attributes'].get('reward_graph_purchased') is True
           for item in athena.profile['items'].values()):
        # The purchase token is consumed into the graph. Reward repair must
        # recognize the entitlement after consumption rather than regrant it.
        owned.add(PURCHASE_TOKEN.casefold())
    pending = []
    seen = set()
    credit = 0
    allowed = {prefix.casefold() for prefix in CATEGORY_BY_PREFIX} | {
        "challengebundleschedule", "rewardeventgraphpurchasetoken", "bannertoken",
        "cosmeticvarianttoken", "token"}
    for template, amount in rewards:
        if not isinstance(template, str) or ":" not in template or isinstance(amount, bool) or not isinstance(amount, int) or amount < 1:
            raise _invalid("Invalid Season 12 reward.")
        prefix, name = template.split(":", 1)
        if not name:
            raise _invalid("Invalid Season 12 reward.")
        if template.casefold() == "currency:mtxgiveaway":
            credit += amount
            continue
        if prefix.casefold() not in allowed or amount != 1:
            raise _invalid("Unsupported Season 12 reward.")
        key = template.casefold()
        if key in seen:
            raise _invalid("Duplicate Season 12 reward in schedule.")
        seen.add(key)
        if key not in owned:
            pending.append(template)
    _debit_local_currency(ctx, price)
    if credit:
        core = ctx.tx("common_core")
        wallet = next(item_id for item_id, item in core.profile["items"].items()
                      if item["templateId"].casefold() == "currency:mtxgiveaway")
        core.set_item_quantity(wallet, core.item(wallet)["quantity"] + credit)
    loot = []
    for template in pending:
        item_id = ctx.service._new_item_id("item", athena.profile)
        prefix = template.split(":", 1)[0].casefold()
        attrs = (cosmetic_attributes(template)
                 if prefix in {p.casefold() for p in CATEGORY_BY_PREFIX} else {"item_seen": False})
        athena.add_item(item_id, template, attrs)
        loot.append({"itemType": template, "itemGuid": item_id, "quantity": 1,
                     "attributes": attrs, "itemProfile": "athena"})
    quests = getattr(ctx.service, "quests", None)
    if quests is not None:
        quests.consume_variants(athena)
        quests.login(ctx, include_daily=False)
    from maya_reward_service import reconcile_purchase
    reconcile_purchase(ctx)
    convert_banner_tokens(ctx)
    if loot:
        ctx.notifications.append({"type": "CatalogPurchase", "primary": True, "lootResult": {"tierGroupName": "", "items": loot}})


@command("PurchaseCatalogEntry", ("common_core",), "observed",
         "offerId, purchaseQuantity, currency, currencySubType and expectedTotalPrice (inferred).")
def _purchase_catalog_entry(ctx):
    offer_id = ctx.str_field("offerId")
    quantity = ctx.int_field("purchaseQuantity", minimum=1, maximum=99)
    currency = ctx.str_field("currency")
    subtype = ctx.str_field("currencySubType", allow_empty=True)
    expected = ctx.int_field("expectedTotalPrice")
    offer = get_offer(offer_id)
    if offer is None:
        raise _invalid("Unknown catalog offer.")
    prices = offer.get("prices", [])
    matching = [p for p in prices if p.get("currencyType") == currency
                and p.get("currencySubType", "") == subtype]
    if currency != "MtxCurrency" or subtype != "" or len(matching) != 1:
        raise _invalid("Unsupported offer currency.")
    price = matching[0].get("finalPrice")
    if isinstance(price, bool) or not isinstance(price, int) or price < 0 or price * quantity != expected:
        raise _invalid("Offer price does not match expectedTotalPrice.")

    if offer_id == LOCAL_CREDIT_OFFER:
        grants = offer.get("itemGrants")
        if (quantity != 1 or price != 0 or grants != [
                {"templateId": "Currency:MtxGiveaway", "quantity": 1000}]):
            raise _invalid("Invalid local credit offer.")
        core = ctx.tx("common_core")
        wallets = [(item_id, item) for item_id, item in core.profile["items"].items()
                   if item["templateId"].casefold() == "currency:mtxgiveaway"]
        if len(wallets) != 1:
            raise _invalid("Local currency balance is unavailable.")
        wallet_id, wallet = wallets[0]
        balance = wallet.get("quantity")
        if isinstance(balance, bool) or not isinstance(balance, int) or balance < 0:
            raise _invalid("Local currency balance is invalid.")
        core.set_item_quantity(wallet_id, balance + 1000)
        ctx.notifications.append({"type": "CatalogPurchase", "primary": True, "lootResult": {
            "tierGroupName": "", "items": [{"itemType": "Currency:MtxGiveaway",
                "itemGuid": wallet_id, "quantity": 1000,
                "attributes": wallet.get("attributes", {}), "itemProfile": "common_core"}]}})
        return

    if offer_id in (SEASON12_PASS_OFFER, SEASON12_LEVEL_OFFER):
        athena = ctx.tx("athena")
        stats = athena.profile["stats"]["attributes"]
        level = stats.get("book_level")
        purchased = stats.get("book_purchased")
        if (stats.get("season_num") != 12 or isinstance(level, bool)
                or not isinstance(level, int) or not 1 <= level <= 100
                or not isinstance(purchased, bool)):
            raise _invalid("Season 12 progression is unavailable.")
        if offer_id == SEASON12_PASS_OFFER:
            if purchased or quantity != 1:
                raise _invalid("Season 12 Battle Pass is unavailable or already purchased.")
            rewards = [reward for n in range(1, level + 1) for reward in SEASON12_PAID.get(n, ())]
            new_level = level
        else:
            new_level = level + quantity
            if new_level > 100:
                raise _invalid("Season 12 level purchase exceeds level 100.")
            rewards = [reward for n in range(level + 1, new_level + 1)
                       for schedule in ((SEASON12_FREE, SEASON12_PAID) if purchased else (SEASON12_FREE,))
                       for reward in schedule.get(n, ())]
        _grant_season12_rewards(ctx, athena, rewards, price * quantity)
        if offer_id == SEASON12_PASS_OFFER:
            athena.set_stat("book_purchased", True)
        else:
            athena.set_stat("book_level", new_level)
            # Season 12 uses a shared XP curve for season and book levels.
            current_level = stats.get("level")
            if isinstance(current_level, int) and not isinstance(current_level, bool) and current_level < new_level:
                athena.set_stat("level", new_level)
        return

    if quantity != 1:
        raise _invalid("Cosmetic offers require a quantity of one.")

    grants = offer.get("itemGrants", [])
    if not isinstance(grants, list) or not grants:
        raise _invalid("Offer has no cosmetic grants.")
    templates = []
    prefixes = {prefix.lower() for prefix in CATEGORY_BY_PREFIX}
    for grant in grants:
        if not isinstance(grant, dict):
            raise _invalid("Invalid cosmetic grant.")
        template = grant.get("templateId")
        quantity = grant.get("quantity")
        if (not isinstance(template, str) or ":" not in template
                or template.split(":", 1)[0].lower() not in prefixes
                or not template.split(":", 1)[1]
                or isinstance(quantity, bool) or not isinstance(quantity, int) or quantity != 1):
            raise _invalid("Invalid cosmetic grant.")
        templates.append(template)
    folded = [template.casefold() for template in templates]
    if len(set(folded)) != len(folded):
        raise _invalid("Duplicate cosmetic grant.")
    athena = ctx.tx("athena")
    owned = {item["templateId"].casefold() for item in athena.profile["items"].values()}
    if owned.intersection(folded):
        raise _invalid("A cosmetic in this offer is already owned.")
    _debit_local_currency(ctx, price)
    loot = []
    for template in templates:
        item_id = ctx.service._new_item_id("item", athena.profile)
        attributes = cosmetic_attributes(template)
        athena.add_item(item_id, template, attributes)
        loot.append({"itemType": template, "itemGuid": item_id, "quantity": 1,
                     "attributes": attributes, "itemProfile": "athena"})
    # CatalogPurchaseNotification and McpLootResult fields are reflected;
    # the type discriminator is inferred from a client string.
    ctx.notifications.append({"type": "CatalogPurchase", "primary": True, "lootResult": {
        "tierGroupName": "", "items": loot}})

@command("ClientQuestLogin", ("athena",), "observed", "no body. Once per UTC day: advances the daily login counter.")
def _client_quest_login(ctx):
    tx = ctx.tx("athena")
    season = tx.profile["stats"]["attributes"].get("season_num")
    if type(season) is int and season == 12:
        owned = {item["templateId"].casefold() for item in tx.profile["items"].values()}
        for template, quantity in SUPPORTED_GRANTS_TO_EVERYONE:
            if template.casefold() not in owned:
                item_id = ctx.service._new_item_id("item", tx.profile)
                tx.add_item(item_id, template, initial_grant_attributes(template), quantity)
    quests = getattr(ctx.service, "quests", None)
    if quests is not None:
        quests.login(ctx)
    attributes = tx.profile["stats"]["attributes"]
    today = ctx.ctx.day()
    quest = dict(attributes.get("quest_manager") or {})
    if quest.get("dailyLoginInterval") == today:
        return
    rewards = dict(attributes.get("daily_rewards") or {})
    rewards["totalDaysLoggedIn"] = int(rewards.get("totalDaysLoggedIn", 0)) + 1
    quest["dailyLoginInterval"] = today
    tx.set_stat("quest_manager", quest)
    tx.set_stat("daily_rewards", rewards)


@command("ClaimQuestReward", ("athena",), "observed",
         "questId and explicit selectedRewardIndex -1 for supported fixed rewards; casing inferred.")
def _claim_quest_reward(ctx):
    quests = getattr(ctx.service, "quests", None)
    if quests is None:
        raise _invalid("Quest service is unavailable.")
    quests.claim(ctx, ctx.str_field("questId"),
                 ctx.int_field("selectedRewardIndex", minimum=-1, maximum=-1))


@command("FortRerollDailyQuest", ("athena",), "observed",
         "questId: owned active daily instance ID (reflected field; casing inferred). Local daily policy applies.")
def _reroll_daily_quest(ctx):
    quests = getattr(ctx.service, "quests", None)
    if quests is None:
        raise _invalid("Daily quest service is unavailable.")
    quests.daily.reroll(ctx, ctx.str_field("questId"))


@command("MarkNewQuestNotificationSent", ("athena",), "observed",
         "itemIds: quest instance IDs (reflected ItemIds; casing inferred).")
def _mark_new_quest_notification_sent(ctx):
    tx = ctx.tx("athena")
    item_ids = ctx.list_field("itemIds")
    # Validate the complete batch before applying any change.
    for item_id in item_ids:
        if not isinstance(item_id, str) or not item_id:
            raise _invalid("itemIds must contain nonempty strings.")
        item = tx.item(item_id)
        if not item.get("templateId", "").casefold().startswith("quest:"):
            raise _invalid("itemIds must refer to owned quest items.")
    for item_id in item_ids:
        # Reflected UFortQuestItem field, corresponding to this notification.
        tx.set_item_attr(item_id, "sent_new_notification", True)


@command("ToggleQuestActiveState", ("athena",), "observed",
         "questIds: owned faction quest instance IDs; native callers toggle one/two branches. Local validation policy applies.")
def _toggle_quest_active_state(ctx):
    from faction_quest_service import toggle_faction_selection
    toggle_faction_selection(ctx, ctx.list_field("questIds"))


@command("UnlockRewardNode", ("athena",), "observed",
         "nodeId, rewardGraphId instance ID, rewardCfg: recovered Maya native caller; backend validation is local policy.")
def _unlock_reward_node(ctx):
    from maya_reward_service import unlock_reward_node
    unlock_reward_node(ctx)


@command("SetRewardGraphConfig", ("athena",), "observed",
         "state string array and rewardGraphId instance ID: recovered Maya native caller.")
def _set_reward_graph_config(ctx):
    from maya_reward_service import set_reward_graph_config
    set_reward_graph_config(ctx)


@command("MarkItemSeen", ("athena", "common_core"), "observed", "itemIds: list of item ids (inferred).")
def _mark_item_seen(ctx):
    tx = ctx.tx(ctx.profile_id)
    for item_id in ctx.list_field("itemIds"):
        if not isinstance(item_id, str):
            raise _invalid("itemIds must contain strings.")
        tx.set_item_attr(item_id, "item_seen", True)


@command("SetMtxPlatform", ("common_core",), "observed", "newPlatform: one of the platform names (inferred).")
def _set_mtx_platform(ctx):
    platform = ctx.str_field("newPlatform", choices=MTX_PLATFORMS)
    ctx.tx("common_core").set_stat("current_mtx_platform", platform)


@command("SetAffiliateName", ("common_core",), "observed", "affiliateName (inferred). No affiliate check.")
def _set_affiliate_name(ctx):
    name = ctx.str_field("affiliateName", pattern=r"[A-Za-z0-9_-]{1,32}")
    ctx.tx("common_core").set_stat("mtx_affiliate", name)


@command("SetReceiveGiftsEnabled", ("common_core",), "observed", "bReceiveGifts: boolean (inferred).")
def _set_receive_gifts(ctx):
    ctx.tx("common_core").set_stat("allowed_to_receive_gifts", ctx.bool_field("bReceiveGifts"))


@command("SetItemFavoriteStatus", ("athena",), "observed", "targetItemId and bFavorite (inferred).")
def _set_item_favorite(ctx):
    item_id = ctx.str_field("targetItemId")
    ctx.tx("athena").set_item_attr(item_id, "favorite", ctx.bool_field("bFavorite"))


@command("SetItemFavoriteStatusBatch", ("athena",), "observed",
         "itemIds and itemFavStatus, equal-length lists (inferred).")
def _set_item_favorite_batch(ctx):
    ids = ctx.list_field("itemIds")
    flags = ctx.list_field("itemFavStatus")
    if len(ids) != len(flags) or not all(isinstance(flag, bool) for flag in flags):
        raise _invalid("itemIds and itemFavStatus must be equal-length lists, with booleans in the second.")
    tx = ctx.tx("athena")
    for item_id, flag in zip(ids, flags):
        if not isinstance(item_id, str):
            raise _invalid("itemIds must contain strings.")
        tx.set_item_attr(item_id, "favorite", flag)


@command("SetCosmeticLockerSlot", ("athena",), "observed",
         "lockerItem, category, itemToSlot, slotIndex and variantUpdates (inferred).")
def _set_cosmetic_locker_slot(ctx):
    updates = _check_variant_updates(ctx.body)
    locker_id = ctx.str_field("lockerItem")
    category = ctx.str_field("category")
    item_id = ctx.str_field("itemToSlot", allow_empty=True, required=False) or ""
    index = ctx.int_field("slotIndex", default=0, minimum=-1, maximum=WRAP_SLOT_COUNT - 1)
    _equip(ctx, locker_id, category, item_id, index, updates, allow_wrap_all=True)


@command("SetCosmeticLockerSlots", ("athena",), "observed",
         "lockerItem and loadoutData (reflected names; JSON casing inferred).")
def _set_cosmetic_locker_slots(ctx):
    locker_id = ctx.str_field("lockerItem")
    entries = ctx.list_field("loadoutData")
    # Validate the locker even for an empty batch. Changes stay on the private
    # transaction copy until all entries have been successfully applied.
    _locker(ctx, locker_id)
    for entry in entries:
        if not isinstance(entry, dict):
            raise _invalid("loadoutData must contain objects.")
        updates = _check_variant_updates(entry)
        fields = CommandContext(ctx.service, ctx.account_id, ctx.profile_id, entry)
        category = fields.str_field("slotName")
        item_id = fields.str_field("itemToSlot", allow_empty=True)
        index = fields.int_field("indexWithinSlot", maximum=WRAP_SLOT_COUNT - 1)
        _equip(ctx, locker_id, category, item_id, index, updates)


@command("EquipBattleRoyaleCustomization", ("athena",), "guess",
         "slotName, itemToSlot, indexWithinSlot (inferred). Targets the last applied loadout.")
def _equip_battle_royale_customization(ctx):
    locker_id = ctx.tx("athena").profile["stats"]["attributes"]["last_applied_loadout"]
    category = ctx.str_field("slotName")
    item_id = ctx.str_field("itemToSlot", allow_empty=True, required=False) or ""
    index = ctx.int_field("indexWithinSlot", default=0, maximum=WRAP_SLOT_COUNT - 1)
    _equip(ctx, locker_id, category, item_id, index)


@command("SetCosmeticLockerName", ("athena",), "observed", "lockerItem and name (inferred).")
def _set_cosmetic_locker_name(ctx):
    locker_id = ctx.str_field("lockerItem")
    name = ctx.str_field("name", allow_empty=True, max_len=32)
    tx, _ = _locker(ctx, locker_id)
    tx.set_item_attr(locker_id, "locker_name", name)


@command("SetCosmeticLockerBanner", ("athena",), "observed",
         "lockerItem, bannerIconTemplateName, bannerColorTemplateName (reflected; JSON casing inferred).")
def _set_cosmetic_locker_banner(ctx):
    locker_id = ctx.str_field("lockerItem")
    def banner_field(name, legacy):
        if name in ctx.body and legacy in ctx.body and ctx.body[name] != ctx.body[legacy]:
            raise _invalid(f"{name} and {legacy} must agree when both are supplied.")
        key = name if name in ctx.body else legacy
        return ctx.str_field(key, required=False, pattern=r"[A-Za-z0-9_]{1,64}")

    icon = banner_field("bannerIconTemplateName", "bannerIconTemplate")
    color = banner_field("bannerColorTemplateName", "bannerColorTemplate")
    if icon is None and color is None:
        raise _invalid("bannerIconTemplateName or bannerColorTemplateName is required.")
    tx, _ = _locker(ctx, locker_id)
    if icon is not None:
        tx.set_item_attr(locker_id, "banner_icon_template", icon)
    if color is not None:
        tx.set_item_attr(locker_id, "banner_color_template", color)


@command("SetBattleRoyaleBanner", ("athena",), "guess",
         "bannerIconTemplate and bannerColorTemplate (inferred). Mirrors to common_public (multiUpdate).")
def _set_battle_royale_banner(ctx):
    icon = ctx.str_field("bannerIconTemplate", pattern=r"[A-Za-z0-9_]{1,64}")
    color = ctx.str_field("bannerColorTemplate", pattern=r"[A-Za-z0-9_]{1,64}")
    for profile_id in ("athena", "common_public"):
        tx = ctx.tx(profile_id)
        tx.set_stat("banner_icon", icon)
        tx.set_stat("banner_color", color)


@command("CopyCosmeticLoadout", ("athena",), "observed",
         "sourceIndex, targetIndex, optNewNameForTarget (inferred).")
def _copy_cosmetic_loadout(ctx):
    tx = ctx.tx("athena")
    loadouts = list(tx.profile["stats"]["attributes"].get("loadouts", []))
    source = ctx.int_field("sourceIndex")
    target = ctx.int_field("targetIndex", maximum=len(loadouts))
    new_name = ctx.str_field("optNewNameForTarget", required=False, allow_empty=True, max_len=32)
    if source >= len(loadouts):
        raise _invalid("sourceIndex is out of range.")
    source_item = tx.item(loadouts[source])
    attributes = copy.deepcopy(source_item["attributes"])
    if new_name:
        attributes["locker_name"] = new_name
    new_id = ctx.service._new_item_id("loadout", tx.profile)
    tx.add_item(new_id, source_item["templateId"], attributes, 1)
    if target == len(loadouts):
        loadouts.append(new_id)
    else:
        replaced_id = loadouts[target]
        loadouts[target] = new_id
        # Local selection policy: preserve the selected slot and its ID when
        # replacing it. The original server's behavior is unverified.
        stats = tx.profile["stats"]["attributes"]
        if (stats.get("active_loadout_index") == target and
                stats.get("last_applied_loadout") == replaced_id):
            tx.set_stat("last_applied_loadout", new_id)
        if replaced_id not in loadouts and stats.get("last_applied_loadout") != replaced_id:
            tx.remove_item(replaced_id)
    tx.set_stat("loadouts", loadouts)


@command("SetActiveLoadoutIndex", ("athena",), "observed",
         "index (reflected name; JSON casing inferred). Selection policy is local.")
def _set_active_loadout_index(ctx):
    tx = ctx.tx("athena")
    loadouts = tx.profile["stats"]["attributes"].get("loadouts", [])
    index = ctx.int_field("index")
    if index >= len(loadouts):
        raise _invalid("index is out of range.")
    _locker(ctx, loadouts[index])
    tx.set_stat("active_loadout_index", index)
    tx.set_stat("last_applied_loadout", loadouts[index])


@command("DeleteCosmeticLoadout", ("athena",), "observed",
         "index and fallbackLoadoutIndex (reflected names; JSON casing inferred). Deletion policy is local.")
def _delete_cosmetic_loadout(ctx):
    tx = ctx.tx("athena")
    stats = tx.profile["stats"]["attributes"]
    loadouts = list(stats.get("loadouts", []))
    index = ctx.int_field("index")
    fallback = ctx.int_field("fallbackLoadoutIndex")
    if index >= len(loadouts) or fallback >= len(loadouts) or index == fallback:
        raise _invalid("index and fallbackLoadoutIndex must refer to different existing loadouts.")
    active = stats.get("active_loadout_index")
    if not isinstance(active, int) or isinstance(active, bool) or not 0 <= active < len(loadouts):
        raise _invalid("active loadout index is invalid.")
    _locker(ctx, loadouts[index])
    selected = fallback if active == index else active
    _locker(ctx, loadouts[selected])
    removed = loadouts.pop(index)
    if selected > index:
        selected -= 1
    tx.set_stat("loadouts", loadouts)
    tx.set_stat("active_loadout_index", selected)
    tx.set_stat("last_applied_loadout", loadouts[selected])
    if removed not in loadouts:
        tx.remove_item(removed)


class ProfileService:
    def __init__(self, ctx, accounts, oauth, state_dir=None):
        self.ctx = ctx
        self.accounts = accounts
        self.oauth = oauth
        self._profiles = {}
        self._database = None
        if state_dir is not None:
            directory = Path(state_dir)
            directory.mkdir(parents=True, exist_ok=True)
            self._database = directory / "profiles.sqlite3"
            with closing(sqlite3.connect(self._database)) as db, db:
                db.execute("CREATE TABLE IF NOT EXISTS profiles ("
                           "account_id TEXT NOT NULL, profile_id TEXT NOT NULL, "
                           "data TEXT NOT NULL, PRIMARY KEY (account_id, profile_id))")

    # Store -------------------------------------------------------------

    def _new_item_id(self, kind, profile):
        # The deterministic Context counter restarts with the process. Persistent
        # profiles need IDs that remain fresh across restarts.
        while True:
            item_id = (secrets.token_hex(16) if self._database is not None
                       else self.ctx.new_id(kind))
            if item_id not in profile["items"]:
                return item_id

    def existing_profile(self, account_id, profile_id):
        """Return an existing profile snapshot without creating or caching one."""
        key = (account_id, profile_id)
        if key in self._profiles:
            return copy.deepcopy(self._profiles[key])
        if self._database is not None:
            with closing(sqlite3.connect(self._database)) as db:
                row = db.execute("SELECT data FROM profiles WHERE account_id = ? AND profile_id = ?", key).fetchone()
            if row is not None:
                return json.loads(row[0])
        return None

    def profile(self, account_id, profile_id):
        """Return a snapshot of the profile, created with defaults on first use."""
        key = (account_id, profile_id)
        if key not in self._profiles:
            if self._database is not None:
                with closing(sqlite3.connect(self._database)) as db, db:
                    row = db.execute("SELECT data FROM profiles WHERE account_id = ? AND profile_id = ?",
                                     key).fetchone()
                if row is not None:
                    self._profiles[key] = json.loads(row[0])
                    return copy.deepcopy(self._profiles[key])
            items, attributes = build(self.ctx, account_id, profile_id)
            now = self.ctx.iso()
            profile = {
                "_id": account_id,
                "created": now,
                "updated": now,
                "rvn": 1,
                "wipeNumber": 1,
                "accountId": account_id,
                "profileId": profile_id,
                "version": "no_version",
                "items": items,
                "stats": {"attributes": attributes},
                "commandRevision": 1,
            }
            if self._database is not None:
                with closing(sqlite3.connect(self._database)) as db, db:
                    db.execute("INSERT INTO profiles (account_id, profile_id, data) VALUES (?, ?, ?)",
                               (account_id, profile_id, json.dumps(profile, separators=(",", ":"))))
            self._profiles[key] = profile
        return copy.deepcopy(self._profiles[key])

    def _store(self, account_id, txs, command_profile=None, *, database=None, publish=True):
        """Persist copies, optionally inside a caller-owned SQLite transaction.

        A caller supplying database must publish only after its transaction commits.
        This lets trusted match results share the profile transaction with a receipt.
        """
        now = self.ctx.iso()
        for profile_id, tx in txs.items():
            if tx.changed:
                tx.profile["rvn"] = tx.base_rvn + 1
                tx.profile["updated"] = now
        if command_profile is not None:
            primary = txs[command_profile]
            primary.profile["commandRevision"] = primary.base_command_revision + 1
        if self._database is not None:
            # One database transaction covers every profile changed by a command.
            # Publish to the in-memory cache only after the durable commit succeeds.
            rows = [(account_id, profile_id, json.dumps(tx.profile, separators=(",", ":")))
                    for profile_id, tx in txs.items()]
            sql = "INSERT OR REPLACE INTO profiles (account_id, profile_id, data) VALUES (?, ?, ?)"
            if database is not None:
                database.executemany(sql, rows)
            else:
                with closing(sqlite3.connect(self._database)) as db, db:
                    db.executemany(sql, rows)
        if publish:
            self._publish(account_id, txs)

    def _publish(self, account_id, txs):
        for profile_id, tx in txs.items():
            self._profiles[(account_id, profile_id)] = copy.deepcopy(tx.profile)

    def repair_season12_level_one(self, account_id):
        """Restore missing level-one rewards; call while the server is stopped."""
        ctx = CommandContext(self, account_id, "athena", {})
        athena = ctx.tx("athena")
        stats = athena.profile["stats"]["attributes"]
        if stats.get("season_num") != 12 or stats.get("book_purchased") is not True:
            raise _invalid("Account does not own the Season 12 Battle Pass.")
        rewards = SEASON12_PAID[1]
        if any(t.casefold().startswith("currency:") for t, _ in rewards):
            raise _invalid("Unexpected currency in level-one repair.")
        _grant_season12_rewards(ctx, athena, rewards, 0)
        convert_banner_tokens(ctx)
        if any(tx.changed for tx in ctx.txs.values()):
            self._store(account_id, ctx.txs)
        return len(athena.changes)

    def grant_item(self, account_id, profile_id, template_id, attributes=None):
        """Add an owned item without a client command. Used by tests and by future grant paths."""
        if profile_id not in PROFILE_IDS:
            raise ValueError(f"unknown profile {profile_id}")
        tx = ProfileTx(self.ctx, self.profile(account_id, profile_id))
        item_id = self._new_item_id("item", tx.profile)
        if attributes is None:
            attributes = {"item_seen": False, "favorite": False, "variants": []}
        tx.add_item(item_id, template_id, attributes)
        self._store(account_id, {profile_id: tx})
        return item_id

    # Envelope ----------------------------------------------------------

    @staticmethod
    def _entry(profile_id, profile, base_rvn, changes):
        # One FProfileUpdateSingle (McpProfileSys reflection): the root carries these members and
        # so does every multiUpdate entry. Names follow FJsonObjectConverter::StandardizeCase.
        return {
            "profileRevision": profile["rvn"],
            "profileId": profile_id,
            "profileChangesBaseRevision": base_rvn,
            # Responses can outlive the dispatch lock while they are encoded or
            # inspected by callers. Do not expose mutable cached profile values.
            "profileChanges": copy.deepcopy(changes),
            "lockExpiration": LOCK_EXPIRATION_UNLOCKED,
            "notifications": [],
            "profileCommandRevision": profile["commandRevision"],
        }

    def _envelope(self, entry, notifications, multi):
        body = dict(entry)
        body.update({
            "serverTime": self.ctx.iso(),
            "responseVersion": 1,
            "notifications": notifications,
            "multiUpdate": multi,
        })
        return body

    # Operations --------------------------------------------------------

    def query(self, account_id, profile_id):
        profile = self.profile(account_id, profile_id)
        if profile_id == "common_core":
            tx = ProfileTx(self.ctx, profile)
            repair_starter_items(self, account_id, tx)
            if tx.changed:
                self._store(account_id, {profile_id: tx})
                profile = tx.profile
        snapshot = copy.deepcopy(profile)
        entry = self._entry(profile_id, profile, profile["rvn"],
                            [{"changeType": "fullProfileUpdate", "profile": snapshot}])
        return self._envelope(entry, [], [])

    def run(self, account_id, profile_id, command_name, body, client_rvn=None):
        if command_name in ("GiftCatalogEntry", "RemoveGiftBox") and getattr(self, "gifting", None) is not None:
            return self.gifting.run(account_id, profile_id, command_name, body, client_rvn)
        if command_name in UNIMPLEMENTED:
            raise ApiError(400, Err.OPERATION_NOT_FOUND,
                           f"Operation {command_name} is not implemented by the local stand-in.")
        cmd = COMMANDS.get(command_name)
        if cmd is None:
            raise ApiError(400, Err.OPERATION_NOT_FOUND, f"Operation {command_name} not found.")
        if profile_id not in cmd.profiles:
            raise ApiError(400, Err.INVALID_PROFILE_COMMAND,
                           f"{command_name} cannot be used on the {profile_id} profile.")
        ctx = CommandContext(self, account_id, profile_id, body)
        ctx.tx(profile_id)
        cmd.fn(ctx)
        if "athena" in ctx.txs:
            from maya_reward_service import reconcile_purchase
            reconcile_purchase(ctx)
            convert_banner_tokens(ctx)
        self._store(account_id, ctx.txs, command_profile=profile_id)
        primary = ctx.txs[profile_id]
        # Deltas only apply to the revision they were generated from. When the
        # client supplies a different revision, send the resulting full profile
        # instead (local recovery policy; exact server behavior is unobserved).
        if client_rvn is not None and client_rvn != str(primary.base_rvn):
            changes = [{"changeType": "fullProfileUpdate", "profile": copy.deepcopy(primary.profile)}]
            entry = self._entry(profile_id, primary.profile, primary.profile["rvn"], changes)
        else:
            entry = self._entry(profile_id, primary.profile, primary.base_rvn, primary.changes)
        multi = [self._entry(other_id, tx.profile, tx.base_rvn, tx.changes)
                 for other_id, tx in ctx.txs.items() if other_id != profile_id and tx.changed]
        return self._envelope(entry, ctx.notifications, multi)

    # Handlers ----------------------------------------------------------

    def mcp_command(self, req, match):
        account_id, command_name = match.group(1), match.group(3)
        self.oauth.authenticate(req, account_id=account_id)
        if self.accounts.get(account_id) is None:
            raise ApiError(404, Err.PROFILE_ACCOUNT_NOT_FOUND, "The account was not found.")
        profile_id = req.query.get("profileId", "common_core")
        if profile_id not in PROFILE_IDS:
            raise ApiError(400, Err.PROFILE_ID_INVALID, f"{profile_id} is not a known profile id.")
        if command_name == "QueryProfile":
            return 200, self.query(account_id, profile_id)
        return 200, self.run(account_id, profile_id, command_name, req.json_object(), req.query.get("rvn"))

    def register(self, router):
        router.add("POST", r"/fortnite/api/game/v2/profile/([^/]+)/(client|dedicated_server|public)/([A-Za-z]+)",
                   self.mcp_command, name="profile.command", status="observed")
