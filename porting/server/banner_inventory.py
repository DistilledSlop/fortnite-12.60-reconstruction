"""Local starter banner policy, not a claim about original entitlements.

The 12.60 BannerIcons and BannerColors tables identify the Standard and
DefaultColorSet1 rows. The original client's picker displays these rows when
owned; which of them Epic granted to a new account remains unproven. This local
policy grants only those rows, never premium or other categories.

Extracted source SHA-256 (FortniteGame/Content/Banners):
 BannerIcons.uasset 595fef2ea22ebb4f6dece4761ef7725a9317fff1e8983c0c35d73b99a0e80619
 BannerIcons.uexp   74e6431b0092768d29903a529385255652e0409940a2ccbede36442b064318c2
 BannerColors.uasset 080cbb49ca37a3893c5e1af9c62e4c75e3349c1bdfffc67f2f41167f7a57e67f
 BannerColors.uexp   62e750058184632e9730a5d68f22ee89241fec59e089fe288f406be8b08ec257
"""

STARTER_BANNERS = (
    *(f"HomebaseBannerIcon:StandardBanner{i}" for i in range(1, 32)),
    *(f"HomebaseBannerColor:DefaultColor{i}" for i in range(1, 45)),
)


def starter_items(ctx, account_id):
    return {
        ctx.stable_id(f"{account_id}|common_core|{template}"): {
            "templateId": template, "attributes": {"item_seen": True}, "quantity": 1,
        }
        for template in STARTER_BANNERS
    }


def repair_starter_items(service, account_id, tx):
    """Add only missing template IDs, preserving existing instances and attributes."""
    owned = {item.get("templateId", "").casefold() for item in tx.profile["items"].values()}
    for template in STARTER_BANNERS:
        if template.casefold() not in owned:
            item_id = service._new_item_id("banner", tx.profile)
            tx.add_item(item_id, template, {"item_seen": True})
