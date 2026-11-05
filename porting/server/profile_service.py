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
from profile_defaults import (CATEGORY_BY_PREFIX, MAX_SLOT_INDEX, MTX_PLATFORMS, PROFILE_IDS,
                              SLOT_CATEGORIES, build, empty_slots)

# lockExpiration is an FDateTime (reflected member of FProfileUpdateSingle, sent as ISO 8601). A profile
# that is not locked carries FDateTime::MinValue. The value is a convention; the member is reflected.
LOCK_EXPIRATION_UNLOCKED = "0001-01-01T00:00:00.000Z"

# observed: these names are in the 12.60 binary but have no implementation here yet.
UNIMPLEMENTED = ("PurchaseCatalogEntry", "GiftCatalogEntry", "RemoveGiftBox",
                 "RefreshExpeditions", "SetActiveHeroLoadout", "ClearHeroLoadout")

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
    # The reflected request and response types identify variant fields, but do
    # not establish the server's ownership and update rules. Never acknowledge
    # a style change while silently discarding it.
    updates = body.get("variantUpdates", [])
    if not isinstance(updates, list):
        raise _invalid("variantUpdates must be a list.")
    if updates:
        raise ApiError(400, Err.OPERATION_NOT_FOUND,
                       "Cosmetic variant updates are not implemented by the local stand-in.")


def _locker(ctx, locker_id):
    tx = ctx.tx("athena")
    item = tx.item(locker_id)
    if not item["templateId"].startswith("CosmeticLocker:"):
        raise _invalid("lockerItem must refer to a cosmetic locker.")
    return tx, item


def _equip(ctx, locker_id, category, item_id, slot_index):
    """Place an owned cosmetic into one slot of a locker. An empty item_id clears the slot."""
    if category not in SLOT_CATEGORIES:
        raise _invalid(f"category {category} is not a slot category.")
    if not 0 <= slot_index <= MAX_SLOT_INDEX:
        raise _invalid("slotIndex is out of range.")
    tx, locker = _locker(ctx, locker_id)
    slots = copy.deepcopy(locker["attributes"].get("locker_slots_data", {"slots": empty_slots()}))
    entry = slots["slots"].setdefault(category, {"items": [], "activeVariants": []})
    if item_id == "":
        template = ""
    else:
        item = tx.item(item_id)
        template = item["templateId"]
        prefix = template.split(":", 1)[0]
        if CATEGORY_BY_PREFIX.get(prefix) != category:
            raise _invalid(f"{template} cannot be placed in the {category} slot.")
    items = list(entry.get("items", []))
    while len(items) <= slot_index:
        items.append("")
    items[slot_index] = template
    entry["items"] = items
    tx.set_item_attr(locker_id, "locker_slots_data", slots)


# Commands ---------------------------------------------------------------

@command("ClientQuestLogin", ("athena",), "observed", "no body. Once per UTC day: advances the daily login counter.")
def _client_quest_login(ctx):
    tx = ctx.tx("athena")
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
         "lockerItem, category, itemToSlot, slotIndex (inferred). Nonempty variantUpdates unsupported.")
def _set_cosmetic_locker_slot(ctx):
    _check_variant_updates(ctx.body)
    locker_id = ctx.str_field("lockerItem")
    category = ctx.str_field("category")
    item_id = ctx.str_field("itemToSlot", allow_empty=True, required=False) or ""
    index = ctx.int_field("slotIndex", default=0, maximum=MAX_SLOT_INDEX)
    _equip(ctx, locker_id, category, item_id, index)


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
        _check_variant_updates(entry)
        fields = CommandContext(ctx.service, ctx.account_id, ctx.profile_id, entry)
        category = fields.str_field("slotName")
        item_id = fields.str_field("itemToSlot", allow_empty=True)
        index = fields.int_field("indexWithinSlot", maximum=MAX_SLOT_INDEX)
        _equip(ctx, locker_id, category, item_id, index)


@command("EquipBattleRoyaleCustomization", ("athena",), "guess",
         "slotName, itemToSlot, indexWithinSlot (inferred). Targets the last applied loadout.")
def _equip_battle_royale_customization(ctx):
    locker_id = ctx.tx("athena").profile["stats"]["attributes"]["last_applied_loadout"]
    category = ctx.str_field("slotName")
    item_id = ctx.str_field("itemToSlot", allow_empty=True, required=False) or ""
    index = ctx.int_field("indexWithinSlot", default=0, maximum=MAX_SLOT_INDEX)
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
    tx.set_stat("loadouts", loadouts)


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

    def profile(self, account_id, profile_id):
        """The stored profile, created with defaults on first use. Callers must not edit it."""
        key = (account_id, profile_id)
        if key not in self._profiles:
            if self._database is not None:
                with closing(sqlite3.connect(self._database)) as db, db:
                    row = db.execute("SELECT data FROM profiles WHERE account_id = ? AND profile_id = ?",
                                     key).fetchone()
                if row is not None:
                    self._profiles[key] = json.loads(row[0])
                    return self._profiles[key]
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
        return self._profiles[key]

    def _store(self, account_id, txs, command_profile=None):
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
            with closing(sqlite3.connect(self._database)) as db, db:
                db.executemany("INSERT OR REPLACE INTO profiles (account_id, profile_id, data) "
                               "VALUES (?, ?, ?)", rows)
        for profile_id, tx in txs.items():
            self._profiles[(account_id, profile_id)] = tx.profile

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
        snapshot = copy.deepcopy(profile)
        entry = self._entry(profile_id, profile, profile["rvn"],
                            [{"changeType": "fullProfileUpdate", "profile": snapshot}])
        return self._envelope(entry, [], [])

    def run(self, account_id, profile_id, command_name, body, client_rvn=None):
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
