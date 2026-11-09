"""Default MCP profile contents for a new local account. No real account data.

Evidence:
  * Asset names. The cooked pak index (porting/generated/assets.json, built by
    list_pak_assets.py) contains CID_001_Athena_Commando_F_Default, BID_001_BlueSquire,
    DefaultPickaxe, DefaultGlider, EID_Celebration and CosmeticLocker_Athena. The
    default-locker template ids below use those names, so they refer to assets that
    exist in the 12.60 build.
  * Attribute names. The 12.60 binary contains locker_slots_data, locker_name,
    banner_icon_template, banner_color_template, loadouts, last_applied_loadout,
    active_loadout_index, quest_manager, daily_rewards, item_seen, favorite,
    current_mtx_platform, mtx_affiliate, allowed_to_receive_gifts, banner_icon,
    banner_color, book_purchased and use_count.
  * Inferred: the templateId casing (AthenaCharacter:cid_...), the slot-to-template
    mapping, and the stored value types (list of template ids per slot).
"""

PROFILE_IDS = ("athena", "common_core", "common_public", "campaign", "creative")

LOCKER_TEMPLATE = "CosmeticLocker:cosmeticlocker_athena"
ATHENA_DEFAULT_COSMETICS = (
    ("Character", "AthenaCharacter:cid_001_athena_commando_f_default"),
    ("Backpack", "AthenaBackpack:bid_001_bluesquire"),
    ("Pickaxe", "AthenaPickaxe:defaultpickaxe"),
    ("Glider", "AthenaGlider:defaultglider"),
    ("Dance", "AthenaDance:eid_celebration"),
)
# observed: category names appear in the binary. The list is inferred from them.
SLOT_CATEGORIES = ("Character", "Backpack", "Pickaxe", "Glider", "SkyDiveContrail",
                   "MusicPack", "LoadingScreen", "Dance", "ItemWrap")
CATEGORY_BY_PREFIX = {
    "AthenaCharacter": "Character",
    "AthenaBackpack": "Backpack",
    "AthenaPickaxe": "Pickaxe",
    "AthenaGlider": "Glider",
    "AthenaSkyDiveContrail": "SkyDiveContrail",
    "AthenaMusicPack": "MusicPack",
    "AthenaLoadingScreen": "LoadingScreen",
    "AthenaDance": "Dance",
    "AthenaItemWrap": "ItemWrap",
}
MAX_SLOT_INDEX = 5  # guess: six slots for categories other than ItemWrap
WRAP_SLOT_COUNT = 7  # observed: seven wrap tiles, indexed 0 through 6
# observed: these platform strings all appear in the 12.60 binary.
MTX_PLATFORMS = ("EpicPC", "PSN", "XBL", "Nintendo", "Samsung", "IOS", "Android")


def empty_slots():
    return {category: {"items": [], "activeVariants": []} for category in SLOT_CATEGORIES}


def _cosmetic_item(extra=None):
    attributes = {"item_seen": True, "favorite": False, "variants": []}
    attributes.update(extra or {})
    return {"templateId": "", "attributes": attributes, "quantity": 1}


def build_athena(ctx, account_id):
    items = {}
    slots = empty_slots()
    for category, template in ATHENA_DEFAULT_COSMETICS:
        item_id = ctx.stable_id(f"{account_id}|athena|{template}")
        item = _cosmetic_item()
        item["templateId"] = template
        items[item_id] = item
        slots[category]["items"] = [template]
    locker_id = ctx.stable_id(f"{account_id}|athena|locker|0")
    locker = _cosmetic_item({
        "locker_slots_data": {"slots": slots},
        "locker_name": "",
        "banner_icon_template": "StandardBanner1",
        "banner_color_template": "DefaultColor1",
        "use_count": 0,
    })
    locker["templateId"] = LOCKER_TEMPLATE
    items[locker_id] = locker
    attributes = {
        "season_num": 12,
        "level": 1,
        "book_level": 1,
        "xp": 0,
        "accountLevel": 1,
        "book_purchased": False,
        "banner_icon": "StandardBanner1",
        "banner_color": "DefaultColor1",
        "loadouts": [locker_id],
        "last_applied_loadout": locker_id,
        "active_loadout_index": 0,
        "use_random_loadout": False,
        "quest_manager": {"dailyLoginInterval": "", "dailyQuestRerolls": 0},
        "daily_rewards": {"nextDefaultReward": 0, "totalDaysLoggedIn": 0},
    }
    return items, attributes


def build_common_core(ctx, account_id):
    currency_id = ctx.stable_id(f"{account_id}|common_core|currency")
    items = {currency_id: {"templateId": "Currency:MtxGiveaway", "attributes": {}, "quantity": 0}}
    attributes = {
        "mfa_reward_claimed": True,
        "current_mtx_platform": "EpicPC",
        "mtx_affiliate": "",
        "survey_data": {},
        "personal_offers": {},
        "intro_game_played": True,
        "import_friends_claimed": {},
        "allowed_to_send_gifts": True,
        "allowed_to_receive_gifts": True,
        "gift_history": {},
    }
    return items, attributes


def build_common_public(ctx, account_id):
    attributes = {
        "banner_color": "DefaultColor1",
        "banner_icon": "StandardBanner1",
        "homebase_name": "LocalPlayer",
    }
    return {}, attributes


def build_campaign(ctx, account_id):
    # guess: the campaign profile is empty apart from its log attributes.
    return {}, {"past_actions_log": [], "mission_alert_redemption_record": {}}


def build_creative(ctx, account_id):
    # guess: the creative profile starts empty.
    return {}, {"past_actions_log": []}


BUILDERS = {
    "athena": build_athena,
    "common_core": build_common_core,
    "common_public": build_common_public,
    "campaign": build_campaign,
    "creative": build_creative,
}


def build(ctx, account_id, profile_id):
    """Return (items, stat attributes) for a profile id. Ids are stable per account."""
    return BUILDERS[profile_id](ctx, account_id)
