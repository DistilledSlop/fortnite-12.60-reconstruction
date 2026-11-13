"""Normalized Season 12 Peely Spy schedule, quests and Client stat rows.

The reward export is AthenaEmojiItemDefinition, derived from
AthenaDanceItemDefinition. DefaultGame.ini PrimaryAssetTypesToScan maps that
base class and the recursive Dances directory to AthenaDance.
"""

SOURCE_HASHES = {'AthenaSeason12': {'.uasset': '29edbcf3d814c299651a2f79c65353d7656e32fb52603b6e7d179d21db004e47',
                    '.uexp': '6adc59578e9a7d4958633c0c9cd97a1a5c97c1166ef095a307035440e94c5043'},
 'AthenaCompositeObjectiveStatTable_A': {'.uasset': 'f9d9e3a556548c313f0a2e45c688085980f07625ecd96c86b2a19d445bdb3c83',
                                         '.uexp': '548c7d765932ef250f2eb0c17ec4034e7cfec3c4bcd78461906ac9930dd27b5c'},
 'Season12_Schedule_Event_PeelySpy': {'.uasset': '1b47b6598650acdab3db02bb5f062b07bd7b1f4b15dccc1ecb848a1028ecca3d',
                                      '.uexp': 'd172713595948dc868550c4b07c3a31f41f487407c7afbbf2c3eb9877d1fe325'},
 'QuestBundle_Event_S12_PeelySpy': {'.uasset': 'c37f4faf3d79408d688fac3e1d8a00a8e93139bb7d42d429b563a50d59e7d484',
                                    '.uexp': '24c8fd963885aaa9467f0915d03341cdaf8418a135af4d0a120cc9c55e0773d1'},
 'quest_peely_frontend_spy_01': {'.uasset': 'da0e57237097400b284799ee80af17b2dcec546338e8dbc8756aa78dd96aa258',
                                 '.uexp': 'b6c85fce64b16402e6ed9ccf4010535f0874cacada295075c6385dad7f6c5aa9'},
 'quest_peely_frontend_spy_02': {'.uasset': '0a15a7af43586ce603c3afc550a803fce47e6e54af7b546b42ba9ec10b395db4',
                                 '.uexp': 'e2787359b7c8ad136d188886fe41696265e459c0e156cd929a100c2d25eecacc'},
 'quest_peely_frontend_spy_03': {'.uasset': '9787585f69cc6912f42802086e705416602c6f2e3d6b8b09fa79b158b503855e',
                                 '.uexp': '78e92160c0db2863870a5d2f90a11a494373695ee6ca3b961c3fccaed857a7e2'},
 'Emoji_S12_BananaAgent': {'.uasset': 'e9a956198d4f2bcdabb1adcb6cdf7a8c29dd05a64b6d54b2b8c369604015054f',
                           '.uexp': 'dd62f8ec127803f65ced7f83e12047627b75edc5c8a717d40c2f92b19ce7b0e7'}}

SCHEDULE = {'name': 'Season12_Schedule_Event_PeelySpy',
            'calendar_event_tag': 'EventFlag.LobbySeason12',
            'entries': [('QuestBundle_Event_S12_PeelySpy', 'OnScheduleGranted', 0)]}
STAT_DEFINITIONS = {f'peely_spy_frontend_0{i}': {
    'Type': 'EFortQuestObjectiveStatEvent::Client', 'TargetTagContainer': [],
    'SourceTagContainer': [], 'ContextTagContainer': [], 'Condition': '',
    'TemplateId': '', 'AlternateTemplateIds': []} for i in range(1, 4)}
QUESTS = {f'quest_peely_frontend_spy_0{i}': {
    'objectives': [(f'quest_peely_frontend_spy_0{i}_obj', f'peely_spy_frontend_0{i}', 1)],
    'rewards': []} for i in range(1, 4)}
BUNDLES = {'QuestBundle_Event_S12_PeelySpy': {
    'calendar_event_tag': 'EventFlag.LobbySeason12', 'bHideFromMapChallenges': True,
    'quest_infos': [(name, 'DaysFromEventStart', 43, True) for name in QUESTS],
    'completion_rewards': {3: [('AthenaDance', 'Emoji_S12_BananaAgent', 1)]}}}
CLIENT_OBJECTIVES = {objective.lower(): name for name, definition in QUESTS.items()
                     for objective, row, _ in definition['objectives']
                     if STAT_DEFINITIONS[row]['Type'] == 'EFortQuestObjectiveStatEvent::Client'}
