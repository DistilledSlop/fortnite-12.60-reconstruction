"""Cooked Donut bundle reward identity and variant metadata.

VTID_587 uasset/uexp: 4004e4795107985f422167ee2dc78be9d08e1037461a83a407831f58bd5fbb7f / cf285bb7982d279b3bcd2516a38644a134bfbc9a842604c9c7e47bca0314f34c
VTID_588 uasset/uexp: 6112b61875894c38d0237e8ac9df18db0cc3d45274cf35bdf0168794fc6bcbfa / 328a875bec2346bd1d9dc9f24e4ebd00dc4f8007cd4b288cb6f5a6c94d4987e8
CID_705 uasset/uexp: 7fc3d1d78fa056937772ce4db6160ab5c3008d345d52f4970aea01adf1b2d1b6 / 518deb5b795aff72d8f1a70da7d0bbb0736d9a06209bec8ebb0d63c23b92ea37
Token reports explicitly disable auto-equip and unseen. Character variant
options identify Stage1 and Mat1 as their defaults.
"""
_REWARD_ROWS = [
 ('/Game/Items/Tokens/AthenaSeason12/AthenaBattlePass_Donut_Challenge_Token', 'Token'),
 ('/Game/Athena/Items/BannerToken/BannerToken_043_S12_Donut', 'BannerToken'),
 ('/Game/Athena/Items/Cosmetics/Sprays/SPID_201_DonutA', 'AthenaDance'),
 ('/Game/Athena/Items/Cosmetics/Sprays/SPID_202_DonutB', 'AthenaDance'),
 ('/Game/Athena/Items/Cosmetics/Backpacks/BID_480_Donut', 'AthenaBackpack'),
 ('/Game/Athena/Items/Cosmetics/LoadingScreens/LSID_230_Donut', 'AthenaLoadingScreen'),
 ('/Game/Athena/Items/Cosmetics/ItemWraps/Wrap_226_Donut', 'AthenaItemWrap'),
 ('/Game/Athena/Items/Cosmetics/Characters/CID_705_Athena_Commando_M_Donut', 'AthenaCharacter'),
 ('/Game/Athena/Items/CosmeticVariantTokens/VTID_587_Donut_StyleB', 'CosmeticVariantToken'),
 ('/Game/Athena/Items/CosmeticVariantTokens/VTID_588_Donut_ColorB', 'CosmeticVariantToken'),
 ('/Game/Athena/Items/Cosmetics/MusicPacks/MusicPack_047_FreestylinClubRemix', 'AthenaMusicPack'),
]
DONUT_REWARD_TYPES = {path + '.' + path.rsplit('/', 1)[1]: (kind, path.rsplit('/', 1)[1])
                      for path, kind in _REWARD_ROWS}
DONUT_VARIANTS = {
 'cosmeticvarianttoken:vtid_587_donut_styleb': ('AthenaCharacter:CID_705_Athena_Commando_M_Donut', 'Parts', 'Stage2', 'Stage1'),
 'cosmeticvarianttoken:vtid_588_donut_colorb': ('AthenaCharacter:CID_705_Athena_Commando_M_Donut', 'Material', 'Mat2', 'Mat1'),
}
