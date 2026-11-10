"""Small local storefront, shared by catalog display and purchase validation.

Shop offers are local fixtures. Season 12 IDs were recovered from the client;
their mapping to the pass and bundle, and prices, are local assumptions.
"""
from copy import deepcopy

SEASON12_PASS_OFFER = "89DED540434B169187E1D6A5C2086AA2"
SEASON12_LEVEL_OFFER = "350050574100C333CB9ACA8D1CEF9BF6"


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
        "giftInfo": {"bIsEnabled": False, "forcedGiftBoxTemplateId": "", "purchaseRequirements": [],
                     "giftRecordIds": []},
        "refundable": True,
        "itemGrants": [{"templateId": template, "quantity": 1}],
    }


_STOREFRONTS = [
    {"name": "BRDailyStorefront", "catalogEntries": [
        _offer("local-floss", "AthenaDance:eid_floss", 500, "Daily", 0),
        _offer("local-arctic-camo", "AthenaItemWrap:wrap_001_arcticcamo", 300, "Daily", 1),
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
