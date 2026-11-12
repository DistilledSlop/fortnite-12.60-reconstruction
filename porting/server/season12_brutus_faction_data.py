"""Cooked Season 12 Brutus faction grant and manual bundle definitions.

Sources: season12-agent-quest-reports/normalized.json and the loose
quest_AB_skulldude_grant property report. Item acquisition is handled internally,
not exposed as an arbitrary trusted gameplay event.
"""
GRANT_QUEST = 'quest_AB_skulldude_grant'
TOKEN = 'AthenaBattlePass_SkullDudeBundle_Token'
OBJECTIVE = 'season12_cumulative_skulldude_token'
BUNDLE = 'MissionBundle_S12_AB_Skulldude'
SCHEDULE = 'Season12_ABChallenges_Schedule'
# Both branch stats are ComplexCustom with context Athena.Playlist.
BRANCH_SOURCE_TAGS = {'AB_skulldude_alter': 'Athena.Quests.Skulldude.ALTERDeposit',
                      'AB_skulldude_ego': 'Athena.Quests.Skulldude.EGODeposit'}
GRANT_SHA256 = {'uasset': 'cd31232a597c1708b0dbc330d0103b0c1a580e02da2136a5059dbf95b4d4fc71',
                'uexp': '6c00a945b909febddb1076795860ff12063556c7e4cfe6ff9c6a584f97cbd7c9'}
QUESTS = {
    GRANT_QUEST: {'objectives': [(OBJECTIVE, 'item_acquire_brutus_token', 18)],
                  'rewards': [('ChallengeBundle', BUNDLE, 1)]},
    'quest_AB_skulldude_alter': {
        'objectives': [('quest_AB_skulldude_alter_obj', 'AB_skulldude_alter', 1)],
        'rewards': [('CosmeticVariantToken', x, 1) for x in
                    ('VTID_507_HenchmanTough_StyleC', 'VTID_509_HenchmanTough_Backbling_StyleC', 'VTID_547_HenchmanMaleGlider_StyleC')]},
    'quest_AB_skulldude_ego': {
        'objectives': [('quest_AB_skulldude_ego_obj', 'AB_skulldude_ego', 1)],
        'rewards': [('CosmeticVariantToken', x, 1) for x in
                    ('VTID_506_HenchmanTough_StyleB', 'VTID_508_HenchmanTough_Backbling_StyleB', 'VTID_546_HenchmanMaleGlider_StyleB')]},
}
BUNDLES = {BUNDLE: {'quest_infos': [('quest_AB_skulldude_alter', 'GrantWithBundle', 0, False),
                                   ('quest_AB_skulldude_ego', 'GrantWithBundle', 0, False)]}}
