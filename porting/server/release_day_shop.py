"""Reconstructed 12.60 release-day shop rotation, 2020-05-20 UTC.

Names, prices, and Featured/Daily grouping: https://fnbr.co/shop/may-20-2020
Offer IDs are deterministic local identifiers, not recovered Epic offer IDs.
Template and display asset names are checked against the local 12.60 asset index.
"""

# name, item template, price, bundled back bling (if any), display asset name
FEATURED = (
    ("Lace", "AthenaCharacter:CID_310_Athena_Commando_F_StreetGoth", 1500, "AthenaBackpack:BID_190_StreetGothFemale", "DA_Featured_CID_310_Athena_Commando_F_StreetGoth"),
    ("Paradox", "AthenaCharacter:CID_309_Athena_Commando_M_StreetGoth", 1500, "AthenaBackpack:BID_189_StreetGothMale", "DA_Featured_CID_309_Athena_Commando_M_StreetGoth"),
    ("Echo", "AthenaCharacter:CID_715_Athena_Commando_F_TwinDark", 1200, "AthenaBackpack:BID_489_TwinDark", "DA_Featured_CID_715_Athena_Commando_F_TwinDark"),
    ("Echo Jet", "AthenaGlider:Glider_ID_203_TwinDarkFemale", 800, None, "DA_Featured_Glider_ID_203_TwinDarkFemale"),
    ("Monks", "AthenaCharacter:CID_658_Athena_Commando_F_ToyMonkey", 1200, "AthenaBackpack:BID_447_ToyMonkey", "DA_Featured_CID_658_Athena_Commando_F_ToyMonkey"),
    ("Vision", "AthenaPickaxe:Pickaxe_ID_140_StreetGoth", 800, None, "DA_Featured_Pickaxe_ID_140_StreetGoth"),
    ("Echoes", "AthenaItemWrap:Wrap_221_TwinDark", 300, None, "DA_Featured_Wrap_221_TwinDark"),
    ("Equilibrium", "AthenaGlider:Glider_ID_109_StreetGoth", 500, None, "DA_Featured_Glider_ID_109_StreetGoth"),
    ("Inversion Blades", "AthenaPickaxe:Pickaxe_ID_374_TwinDarkFemale1H", 500, None, "DA_Featured_Pickaxe_ID_374_TwinDarkFemale1H"),
)

DAILY = (
    ("Electro Shuffle", "AthenaDance:EID_ElectroShuffle", 800, None, "DA_EID_ElectroShuffle"),
    ("Chromium", "AthenaCharacter:CID_123_Athena_Commando_F_Metal", 1200, None, "DA_Featured_CID_123_Athena_Commando_F_Metal"),
    ("Fractal Zero", "AthenaItemWrap:Wrap_112_ZeroPointEnergy", 500, None, "DA_Featured_Wrap_112_ZeroPointEnergy"),
    ("Hitchhiker", "AthenaDance:EID_Hitchhiker", 500, None, None),
    ("Jubilation", "AthenaDance:EID_Celebration", 200, None, None),
    ("Recon Ranger", "AthenaCharacter:CID_503_Athena_Commando_F_TacticalWoodlandFuture", 800, None, "DA_Featured_CID_503_Athena_Commando_F_TacticalWoodlandFuture"),
)

# Cooked ItemVariants: Material.Mat1 is default and both Mat1/Mat2 have
# bStartUnlocked=true. Short MCP channel/value serialization is inferred.
# SHA256 .uasset / .uexp, inspected through Cue4Inspect property reports:
# Lace: a9000e7510f49823796290089c033ac39c191b4a000bcdc874de3e21e3ac6e83 /
#       057033955f415808c03198271a0162ed58020bc1d5d46933315ebb935cb82861
# Echo: 2c9c5c155716209d7c50c104799eb0723fec78e218d39832279936493c527480 /
#       842e48758035e1dfdeb4071911fcf04af3b199f5ef15da5344216282c8ee8ce1
# True Reflection: ba4b5358cbff65522fe480c40b199d0b2f25e2808d96dcd86fbc6c95962d0af8 /
#                  39254aa963d00dfe0356e45cc334b617c1065d75787cf4259445895521c61232
INITIAL_MATERIAL_STYLES = frozenset(template.casefold() for template in (
    "AthenaCharacter:CID_310_Athena_Commando_F_StreetGoth",
    "AthenaCharacter:CID_715_Athena_Commando_F_TwinDark",
    "AthenaBackpack:BID_489_TwinDark",
))
