"""Client objective allowlist from the Season 12 composite stat table.

Keys are lowercased quest objective BackendNames, as sent by the client.
Values are (quest definition, objective stat table row, target). Every listed
row has Type Client and empty tag containers, condition, and template IDs.
"""

SOURCE_HASHES = {
    '.uasset': 'f9d9e3a556548c313f0a2e45c688085980f07625ecd96c86b2a19d445bdb3c83',
    '.uexp': '548c7d765932ef250f2eb0c17ec4034e7cfec3c4bcd78461906ac9930dd27b5c',
}
ROW_TYPE = 'EFortQuestObjectiveStatEvent::Client'
CLIENT_OBJECTIVES = {'quest_s12_donut_w1_a': ('Quest_S12_Donut_W1_A', 'donut_w1_startquest', 1),
 'quest_s12_donut_w2_a': ('Quest_S12_Donut_W2_A', 'donut_w2_startquest', 1),
 'quest_s12_donut_w2_b_0': ('Quest_S12_Donut_W2_B', 'donut_chimi_1', 1),
 'quest_s12_donut_w2_b_1': ('Quest_S12_Donut_W2_B', 'donut_chimi_2', 1),
 'quest_s12_donut_w2_b_2': ('Quest_S12_Donut_W2_B', 'donut_chimi_3', 1),
 'quest_s12_donut_w3_a': ('Quest_S12_Donut_W3_A', 'donut_w3_startquest', 1),
 'quest_s12_donut_w4_a_0': ('Quest_S12_Donut_W4_A', 'donut_katana_1', 1),
 'quest_s12_donut_w4_a_1': ('Quest_S12_Donut_W4_A', 'donut_katana_2', 1),
 'quest_s12_donut_w5_a': ('Quest_S12_Donut_W5_A', 'donut_w5_startquest', 1),
 'quest_s12_donut_w6_a': ('Quest_S12_Donut_W6_A', 'donut_w6_startquest', 1),
 'quest_s12_donut_w7_a_0': ('Quest_S12_Donut_W7_A', 'donut_pistol_1', 1),
 'quest_s12_donut_w7_a_1': ('Quest_S12_Donut_W7_A', 'donut_pistol_2', 1),
 'quest_s12_donut_w8_a': ('Quest_S12_Donut_W8_A', 'donut_w8_startquest', 1),
 'quest_s12_donut_w9_a': ('Quest_S12_Donut_W9_A', 'donut_w9_startquest', 1),
 'quest_s12_sta_findfrontend_0': ('quest_s12_sta_findfrontend', 'frontend_midas_sta_1', 1),
 'quest_s12_sta_findfrontend_1': ('quest_s12_sta_findfrontend', 'frontend_midas_sta_2', 1),
 'quest_s12_sta_findfrontend_2': ('quest_s12_sta_findfrontend', 'frontend_midas_sta_3', 1),
 'quest_peely_frontend_spy_01_obj': ('quest_peely_frontend_spy_01', 'peely_spy_frontend_01', 1),
 'quest_peely_frontend_spy_02_obj': ('quest_peely_frontend_spy_02', 'peely_spy_frontend_02', 1),
 'quest_peely_frontend_spy_03_obj': ('quest_peely_frontend_spy_03', 'peely_spy_frontend_03', 1)}
