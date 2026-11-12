"""Local storefront shared by catalog display and purchase validation."""
from copy import deepcopy

from release_day_shop import DAILY, FEATURED, INITIAL_MATERIAL_STYLES

LOCAL_CREDIT_OFFER = "local-free-credits"
SEASON12_PASS_OFFER = "89DED540434B169187E1D6A5C2086AA2"
SEASON12_LEVEL_OFFER = "350050574100C333CB9ACA8D1CEF9BF6"


def cosmetic_attributes(template):
    variants = ([{"channel": "Material", "active": "Mat1", "owned": ["Mat1", "Mat2"]}]
                if template.casefold() in INITIAL_MATERIAL_STYLES else [])
    return {"item_seen": False, "favorite": False, "variants": variants}


def _offer(offer_id, template, price, section, position):
    return {
        "offerId": offer_id,
        "devName": f"Local {template}",
        "offerType": "StaticPrice",
        "prices": [{"currencyType": "MtxCurrency", "currencySubType": "",
                    "regularPrice": price, "finalPrice": price, "basePrice": price}],
        "categories": ["BRItemShop"],
        "dailyLimit": -1, "weeklyLimit": -1, "monthlyLimit": -1,
        "meta": {"SectionId": section, "TileSize": "Normal", "SortPriority": str(position)},
        "matchFilter": "", "filterWeight": 0,
        "catalogGroup": offer_id, "catalogGroupPriority": 0, "sortPriority": position,
        "appStoreId": [], "requirements": [],
        "giftInfo": {"bIsEnabled": section in ("Daily", "Featured"), "forcedGiftBoxTemplateId": "", "purchaseRequirements": [],
                     "giftRecordIds": []},
        "refundable": True,
        "itemGrants": [{"templateId": template, "quantity": 1}],
    }


def _cosmetic_offers(rows, section):
    offers = []
    for position, (name, template, price, back_bling, display) in enumerate(rows):
        offer_id = "local-20200520-" + name.lower().replace(" ", "-")
        offer = _offer(offer_id, template, price, section, position)
        # The 12.60 shop groups offers with the same first category into one
        # cycling tile. Give each offer its own panel; historical panel grouping
        # has not been recovered.
        offer["categories"] = [f"{section} {position + 1:02d}"]
        offer["devName"] = name
        if back_bling:
            offer["itemGrants"].append({"templateId": back_bling, "quantity": 1})
        if display:
            offer["displayAssetPath"] = f"/Game/Catalog/DisplayAssets/{display}.{display}"
        offers.append(offer)
    return offers


_STOREFRONTS = [
    {"name": "BRWeeklyStorefront", "catalogEntries": _cosmetic_offers(FEATURED, "Featured")},
    {"name": "BRDailyStorefront", "catalogEntries": _cosmetic_offers(DAILY, "Daily")},
    {"name": "CurrencyStorefront", "catalogEntries": [
        {**_offer(LOCAL_CREDIT_OFFER, "Currency:MtxGiveaway", 0, "Currency", 0),
         "devName": "Local free test credits", "title": "Free local test V-Bucks",
         "description": "Local test credits; no real-money purchase.",
         "displayAssetPath": "/Game/Catalog/DisplayAssets/DA_MtxPack1000.DA_MtxPack1000",
         "metaInfo": [{"key": "MtxQuantity", "value": "1000"}, {"key": "MtxBonus", "value": "0"}, {"key": "IconSize", "value": "Small"}],
         "itemGrants": [{"templateId": "Currency:MtxGiveaway", "quantity": 1000}], "refundable": False},
    ]},
    {"name": "BRSeason12", "catalogEntries": [
        {**_offer(SEASON12_PASS_OFFER, "Season 12 Battle Pass", 950, "BattlePass", 0),
         "categories": [], "itemGrants": []},
        {**_offer(SEASON12_LEVEL_OFFER, "Season 12 level", 150, "BattlePass", 1),
         "categories": [], "itemGrants": []},
    ]},
]


def storefronts():
    return deepcopy(_STOREFRONTS)


def get_offer(offer_id):
    for storefront in _STOREFRONTS:
        for offer in storefront["catalogEntries"]:
            if offer["offerId"] == offer_id:
                return deepcopy(offer)
    return None
