"""Small local storefront, shared by catalog display and purchase validation.

Offer IDs and prices are local fixtures, not the historical Epic catalog.
"""
from copy import deepcopy


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
]


def storefronts():
    return deepcopy(_STOREFRONTS)


def get_offer(offer_id):
    for storefront in _STOREFRONTS:
        for offer in storefront["catalogEntries"]:
            if offer["offerId"] == offer_id:
                return deepcopy(offer)
    return None
