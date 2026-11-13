"""Convert recovered banner tokens into selectable icons.

Local policy consumes known tokens when awarded or on quest login, and grants new
icons unseen. Original backend consumption timing and seen state are unproven.
BannerIcons.uasset SHA-256: 595fef2ea22ebb4f6dece4761ef7725a9317fff1e8983c0c35d73b99a0e80619
BannerIcons.uexp SHA-256: 74e6431b0092768d29903a529385255652e0409940a2ccbede36442b064318c2
Cooked token SHA-256 (uasset, uexp), under Athena/Items/BannerToken:
 BannerToken_029_S12_Cowboy 7f3b789c6c863c86ef3016e127ba3e77159dfda36f7e552b5396855f004ce252 af8f33031bd59635a384f10a0d2207d8a3f380287ee6e0e836aed703985c4ecd
 BannerToken_030_S12_Grenade cd275671845c73ba30f52bee5cbb7328443d688fdf9b4b555518c888e846521a 9ed0c53d47312ab508741fa21222d095a3c4bf803998a345ac4cfbc0a3364111
 BannerToken_031_S12_Kitty de45fe620ec5577917297cd553df51a39050321f5c91e021820abdbdda143764 cd2fad9afd8adf1da52937b19f9a299c4702165d2f3a19daaddc6e2b0e8b260c
 BannerToken_032_S12_Roses 4591511c33852b522de25bd32916f4777f4ac5939ce2aec912d6841041d1bb14 bb4537cbaca28e76ab90bf03b866fa3676b53732ad68d85dbace3ff5225c5c45
 BannerToken_033_S12_Skull 87e04955b671c2744ad2e7339efe81e51e6848e4161074da12e5d8a4532ac97f a3cea514aa059f6f16033b41fc9a9ed7c13f3734d80587fd0d2cc723e26c922e
 BannerToken_034_S12_WingedCritter 26fbbb67a850cc9b702111f0373d3f938967e68abb73c59578329859791eaab9 37e57d719b7373c58074a8eba89e24a4996712bd664cc9689b72a562668d5af9
 BannerToken_040_S12_Midas 7a438be46307c66bcaeac167b4d22215dd2fc8765c2e05d3f82e36f4647e9109 de7d150b093565e5b41d7ff746de5612e4aef25a41d85170b4f9b28ef18bf248
 BannerToken_043_S12_Donut 1116a9a14050ff1e494d3e924b916f505789b5da4dfc340ceb224ee19c0500dd d54aa8904ae9b40562a1d9aca9f5088d227fe2e6e4f9d2fed8b1b21370cd5a5a
 BannerToken_044_S12_BananaAgent 294454c39c36bdf4530295aa0a8ec34510c787a85829ef88af7ef3efc3f67b02 54b85de3dedcb4bf9cfb786e1777d54b49f928db574a54ab815d293d7c1cbcc7
Catalog loot notifications retain the token; client presentation is unverified.
"""
from api_common import ApiError, Err

MAPPINGS = {
    'bannertoken:bannertoken_029_s12_cowboy': 'HomebaseBannerIcon:brs12_cowboy',
    'bannertoken:bannertoken_030_s12_grenade': 'HomebaseBannerIcon:brs12_grenade',
    'bannertoken:bannertoken_031_s12_kitty': 'HomebaseBannerIcon:brs12_kitty',
    'bannertoken:bannertoken_032_s12_roses': 'HomebaseBannerIcon:brs12_roses',
    'bannertoken:bannertoken_033_s12_skull': 'HomebaseBannerIcon:brs12_skull',
    'bannertoken:bannertoken_034_s12_wingedcritter': 'HomebaseBannerIcon:brs12_wingedcritter',
    'bannertoken:bannertoken_040_s12_midas': 'HomebaseBannerIcon:brs12_midas',
    'bannertoken:bannertoken_043_s12_donut': 'HomebaseBannerIcon:brs12_donut',
    'bannertoken:bannertoken_044_s12_bananaagent': 'HomebaseBannerIcon:brs12_bananaagent',
}


def convert_banner_tokens(ctx):
    """Validate every known token first; mutate both private profile copies together."""
    athena = ctx.tx("athena")
    pending = []
    for item_id, item in athena.profile["items"].items():
        template = item.get("templateId")
        icon = MAPPINGS.get(template.casefold()) if isinstance(template, str) else None
        if icon is None:
            continue
        quantity = item.get("quantity")
        if type(quantity) is not int or quantity < 1:
            raise ApiError(400, Err.INVALID_PAYLOAD, "Invalid banner token quantity.")
        pending.append((item_id, icon))
    if not pending:
        return
    core = ctx.tx("common_core")
    owned = {item.get("templateId", "").casefold() for item in core.profile["items"].values()}
    for item_id, icon in pending:
        if icon.casefold() not in owned:
            core.add_item(ctx.service._new_item_id("banner", core.profile), icon, {"item_seen": False})
            owned.add(icon.casefold())
        athena.remove_item(item_id)
