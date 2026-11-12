"""Maya/PaperDoll quest definitions used by the local Season 12 backend.

Source hashes are below. Complete Cue4Inspect --property-report --loose output
and uninterpreted properties stay in ignored generated/maya-property-reports.
Source roots: /mnt/vibrato/fortnite-extracted-full and generated/extracted.
Objectives: (backend name, stat row, count); rewards: (asset type, name, quantity).
Quest infos: (name, unlock type, day offset, start active).

The bundle reward uses CompletionCount=-1 (all nine QuestInfos) and GB_PaperDoll.
Calendar dates require explicit configuration. Numbered quest names do not indicate
release order.
"""

MAYA_SCHEDULE_CALENDAR_EVENT_TAG = 'EventFlag.LobbySeason12'

MAYA_SCHEDULE = [('QuestBundle_Styles_PaperDoll', 'OnScheduleGranted', 0)]

MAYA_BUNDLE = {'quest_infos': [('quest_s12_styles1_q01_accolade_ar', 'GrantWithBundle', 0, True),
                 ('quest_s12_styles1_q04_athenarank', 'DaysFromEventStart', 5, True),
                 ('quest_s12_styles1_q05_accolade_pistol', 'DaysFromEventStart', 12, True),
                 ('quest_s12_styles1_q02_accolade_combat', 'DaysFromEventStart', 19, True),
                 ('quest_s12_styles1_q03_accolade_shotgun', 'DaysFromEventStart', 26, True),
                 ('quest_s12_styles1_q06_accolade_survival', 'DaysFromEventStart', 33, True),
                 ('quest_s12_styles1_q09_accolade_smg', 'DaysFromEventStart', 40, True),
                 ('quest_s12_styles1_q08_accolade_scavenger', 'DaysFromEventStart', 47, True),
                 ('quest_s12_styles1_q10_athenaoutlive', 'DaysFromEventStart', 54, True)],
 'calendar_event_tag': 'EventFlag.LobbySeason12',
 'completion_rewards': {-1: [('Token', 'AthenaToken_BRS12_HeroSelection_10', 1)]}}

MAYA_QUESTS = {'quest_s12_styles1_q01_accolade_ar': {'objectives': [('quest_s12_styles1_q01_accolade_ar_obj',
                                                       'accolade_assault_specialist',
                                                       5)],
                                       'rewards': [('Token', 'AthenaToken_BRS12_HeroSelection_01', 1)]},
 'quest_s12_styles1_q04_athenarank': {'objectives': [('quest_s12_styles1_q04_athenarank_obj',
                                                      'complete_athena_top_20',
                                                      5)],
                                      'rewards': [('Token', 'AthenaToken_BRS12_HeroSelection_02', 1)]},
 'quest_s12_styles1_q05_accolade_pistol': {'objectives': [('quest_s12_styles1_q05_accolade_pistol_obj',
                                                           'accolade_pistol_specialist',
                                                           5)],
                                           'rewards': [('Token', 'AthenaToken_BRS12_HeroSelection_03', 1)]},
 'quest_s12_styles1_q02_accolade_combat': {'objectives': [('quest_s12_styles1_q02_accolade_combat_obj',
                                                           'accolade_battlemedal_bronze',
                                                           5)],
                                           'rewards': [('Token', 'AthenaToken_BRS12_HeroSelection_04', 1)]},
 'quest_s12_styles1_q03_accolade_shotgun': {'objectives': [('quest_s12_styles1_q03_accolade_shotgun_obj',
                                                            'accolade_shotgun_specialist',
                                                            5)],
                                            'rewards': [('Token', 'AthenaToken_BRS12_HeroSelection_05', 1)]},
 'quest_s12_styles1_q06_accolade_survival': {'objectives': [('quest_s12_styles1_q06_accolade_survival_obj',
                                                             'accolade_silver_survival',
                                                             5)],
                                             'rewards': [('Token', 'AthenaToken_BRS12_HeroSelection_06', 1)]},
 'quest_s12_styles1_q09_accolade_smg': {'objectives': [('quest_s12_styles1_q09_accolade_smg_obj',
                                                        'accolade_smg_specialist',
                                                        5)],
                                        'rewards': [('Token', 'AthenaToken_BRS12_HeroSelection_07', 1)]},
 'quest_s12_styles1_q08_accolade_scavenger': {'objectives': [('quest_s12_styles1_q08_accolade_scavenger_obj',
                                                              'accolade_silver_scavenger',
                                                              5)],
                                              'rewards': [('Token',
                                                           'AthenaToken_BRS12_HeroSelection_08',
                                                           1)]},
 'quest_s12_styles1_q10_athenaoutlive': {'objectives': [('quest_s12_styles1_q10_athenaoutlive_obj',
                                                         'outlive_athena',
                                                         500)],
                                         'rewards': [('Token', 'AthenaToken_BRS12_HeroSelection_09', 1)]}}

MAYA_SOURCE_SHA256 = {'FortniteGame/Content/Athena/Items/ChallengeBundleSchedules/Styles/BundleSchedule_Styles_PaperDoll': {'.uasset': 'd188ff9f6ef3fd0ba2a60723e3867d5152a3165e9e7b2b66c9befbf5d3b79df3',
                                                                                                       '.uexp': '0b5a49d5e64d1606b23a6eeaf717201eb9af994d72b1f04b45185440e9a0c67f'},
 'FortniteGame/Content/Athena/Items/ChallengeBundles/Styles/QuestBundle_Styles_PaperDoll': {'.uasset': 'dc9bf8d450b0b7bf40345f4defbe8d495c80eba651d990186209a21299f1be50',
                                                                                            '.uexp': 'e791343a933ae49cf246e594b022400273ba0e8c74190df849a0cc69d60985b6'},
 'FortniteGame/Content/Athena/Items/Quests/Styles/Season12/quest_s12_styles1_q01_accolade_ar': {'.uasset': '9b6341c5be3cc6ac8a48b42feb8cbcdd121f60829f859d25f5df211a078eb6e2',
                                                                                                '.uexp': '086cec9d1ebdb72f9ba033c87c1b17b232c3a389282c7eb2220d58d3600284f0'},
 'FortniteGame/Content/Athena/Items/Quests/Styles/Season12/quest_s12_styles1_q04_athenarank': {'.uasset': '1cc60079d3891e6a180adf9be24f6e1f73fbf09c70e6c977eb047b8f43f42dba',
                                                                                               '.uexp': '66b7d5199900dc342dcce036c7eeb65f382d51731b524705e856d4cae5e9c9f8'},
 'FortniteGame/Content/Athena/Items/Quests/Styles/Season12/quest_s12_styles1_q05_accolade_pistol': {'.uasset': 'aff71299f75fc9cdff5f04957a3644d5299c37e0446c1d970dfe386396f76f9c',
                                                                                                    '.uexp': 'b242eb26a8989de433fcfa35099d51c7991d2b00d527492759a07f21a04445e8'},
 'FortniteGame/Content/Athena/Items/Quests/Styles/Season12/quest_s12_styles1_q02_accolade_combat': {'.uasset': '4035d44e7aaebf467782997a5a7365b9e052d30823a9bcbf8e405d757d740633',
                                                                                                    '.uexp': '561be5a650eae1cea5c693b0b2088ff6b34ef859dac397aa6db589ca10af2fb0'},
 'FortniteGame/Content/Athena/Items/Quests/Styles/Season12/quest_s12_styles1_q03_accolade_shotgun': {'.uasset': '75e53b3b024c9eeaa56537dbde4526e92782faa33ddc1da4e43ecbf8bfdb8142',
                                                                                                     '.uexp': '963e8de7c4d3836edb7c7f76f717eddee3932971dee6cd1ef6214e8d416ea5a2'},
 'FortniteGame/Content/Athena/Items/Quests/Styles/Season12/quest_s12_styles1_q06_accolade_survival': {'.uasset': 'db512fade18803aa02b8326243fba12e4b383aa101a236d3fe2329c307ff4a07',
                                                                                                      '.uexp': '68ff541f9409aa96856dc95ebb02686396fc4d659d5498fbcc0e14d4bf5df889'},
 'FortniteGame/Content/Athena/Items/Quests/Styles/Season12/quest_s12_styles1_q09_accolade_smg': {'.uasset': '9ed8fc5351fae2b12313be68dab6989f7b1352b6fd02ad073508f26708166bf3',
                                                                                                 '.uexp': '8cdb8115f42208f94f821ddfa302cc08d86b4aa73a1c8eb39b34dcf38bed9bc6'},
 'FortniteGame/Content/Athena/Items/Quests/Styles/Season12/quest_s12_styles1_q08_accolade_scavenger': {'.uasset': '2e6544f21e63ec925c6d49a56bc7377b37036f48093d95997f965016828814b7',
                                                                                                       '.uexp': '42e7e95e9a38b56975ff8ff8d9049a9db08a20050d9c30062cca7253d75f3644'},
 'FortniteGame/Content/Athena/Items/Quests/Styles/Season12/quest_s12_styles1_q10_athenaoutlive': {'.uasset': '745d2ea5350d1409edde3514ff87daf9683352e70b9841f5a604a408b9afeecf',
                                                                                                  '.uexp': 'b6ce28309bc0c95657faf95373c43335ea158391849eb1d71be0a68c7511015b'}}
