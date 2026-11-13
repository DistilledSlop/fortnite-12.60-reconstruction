"""TNTina faction entitlement from cooked grant, branch, and bundle assets.

Grant/branch property reports are from Quest/BattlePass/Season12; bundle data
is in season12-agent-quest-reports/normalized.json. Acquisition is internal.
"""
GRANT_QUEST = 'quest_AB_tntina_grant'
TOKEN = 'AthenaBattlePass_TnTinaBundle_Token'
OBJECTIVE = 'season12_cumulative_tntina_token'
BUNDLE = 'MissionBundle_S12_AB_TNTina'
SCHEDULE = 'Season12_ABChallenges_Schedule'
GRANT_SHA256 = {'uasset': 'b08a4197b61745cb3d50f45daed080660b11c5f955987da55be1ffb9a3106853',
                'uexp': 'd6d34bde03f27197bb35dd4ef8e57dc95f30511cfbbd931460139b1013441c6a'}
BRANCH_SHA256 = {
    'AB_tntina_alter': ('2184de02b7d412b1582826720914bd69cca482a93e15ef2b178be016cb003d1a', '5a237b495643f7a322de96d924011d33dfbf84f22edb35ef85392647b9add628'),
    'AB_tntina_ego': ('90dc943b27faff1e3770d6334eda19acd4fe0f7870b495fbe0d4fa0aff224204', '465b4ef49dec7044974025dd121c2a07faec96bd2af659002ee23b3d1c9207d5'),
}
QUESTS = {
    GRANT_QUEST: {'objectives': [(OBJECTIVE, 'item_acquire_tntina_token', 18)],
                  'rewards': [('ChallengeBundle', BUNDLE, 1)]},
    'AB_tntina_alter': {
        'objectives': [('AB_tntina_alter_obj', 'AB_tntina_alter', 2)],
        'rewards': [('CosmeticVariantToken', x, 1) for x in
                    ('VTID_499_TNTina_StyleC', 'VTID_501_TNTina_Backbling_StyleC', 'VTID_545_TNTinaFemalePickaxe_StyleC', 'VTID_551_TNTinaFemaleGlider_StyleC')]},
    'AB_tntina_ego': {
        'objectives': [('AB_tntina_ego_obj', 'AB_tntina_ego', 2)],
        'rewards': [('CosmeticVariantToken', x, 1) for x in
                    ('VTID_498_TNTina_StyleB', 'VTID_500_TNTina_Backbling_StyleB', 'VTID_544_TNTinaFemalePickaxe_StyleB', 'VTID_550_TNTinaFemaleGlider_StyleB')]},
}
BUNDLES = {BUNDLE: {'quest_infos': [('AB_tntina_alter', 'GrantWithBundle', 0, False),
                                   ('AB_tntina_ego', 'GrantWithBundle', 0, False)]}}
