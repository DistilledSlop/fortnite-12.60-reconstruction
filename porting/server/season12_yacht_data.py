"""Normalized free Season 12 Yacht schedule and gameplay stat metadata.

The trusted gameplay caller qualifies stat rows; this metadata is not a filter
implementation. Quest and reward definitions are shared with season12_quest_data.
"""

SOURCE_HASHES = {
    'AthenaSeason12': {
        '.uasset': '29edbcf3d814c299651a2f79c65353d7656e32fb52603b6e7d179d21db004e47',
        '.uexp': '6adc59578e9a7d4958633c0c9cd97a1a5c97c1166ef095a307035440e94c5043'},
    'AthenaCompositeObjectiveStatTable_A': {
        '.uasset': 'f9d9e3a556548c313f0a2e45c688085980f07625ecd96c86b2a19d445bdb3c83',
        '.uexp': '548c7d765932ef250f2eb0c17ec4034e7cfec3c4bcd78461906ac9930dd27b5c'},
    'Season12_DonutYachtChallenge_Schedule': {
        '.uasset': '4d7237171b139d6c704ba3e7f2e68a01d913eec85cba16b0ec7a5d0fc920792a',
        '.uexp': 'b77075bf082a65f0c50539b3db89de89844e28ac520142e61b29fd1bd5fc5a16'},
    'QuestBundle_S12_Donut_YachtChallenge': {
        '.uasset': 'd132f08236ce55589a10d0300241aaa4bb535427512c1f29652700223d709a6a'},
    'Quest_S12_Donut_YachtChallenge': {
        '.uasset': '66573c3b5a4164dd255f6f089ab418859d68f2448d54f771872413663224355e'},
}
SCHEDULE = {'name': 'Season12_DonutYachtChallenge_Schedule',
            'calendar_event_tag': 'EventFlag.LobbySeason12',
            'category_tag': 'ChallengeCategory.NotSuggested',
            'entries': [('QuestBundle_S12_Donut_YachtChallenge', 'OnScheduleGranted', 0)]}
BUNDLE = 'QuestBundle_S12_Donut_YachtChallenge'
STAT_DEFINITIONS = {'visit_namedpoi_spybase_yacht': {
    'Type': 'EFortQuestObjectiveStatEvent::Visit', 'TargetTagContainer': [],
    'SourceTagContainer': ['Homebase.Class', 'Athena.Location.POI.SpyBase.Yacht'],
    'ContextTagContainer': ['Athena.Playlist'], 'Condition': '',
    'TemplateId': '', 'AlternateTemplateIds': []}}
BUNDLE_METADATA = {'bHideFromMapChallenges': True}
