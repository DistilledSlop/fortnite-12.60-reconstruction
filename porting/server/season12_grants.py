"""Supported subset of AthenaSeason12's SeasonGrantsToEveryone.

Other entries in that source are not implemented by this table.
"""

SOURCE_PACKAGE = 'FortniteGame/Content/Athena/Items/Seasons/AthenaSeason12'
SOURCE_SHA256 = {
    'uasset': '29edbcf3d814c299651a2f79c65353d7656e32fb52603b6e7d179d21db004e47',
    'uexp': '6adc59578e9a7d4958633c0c9cd97a1a5c97c1166ef095a307035440e94c5043',
}

MAYA_GRAPH = 'AthenaRewardGraph:S12_HeroSelections'

SUPPORTED_GRANTS_TO_EVERYONE = (
    ('ChallengeBundleSchedule:Season12_Mission_Schedule', 1),
    (MAYA_GRAPH, 1),
    ('ChallengeBundleSchedule:Season12_ABChallenges_Schedule', 1),
    ('Quest:quest_AB_skulldude_grant', 1),
)


def initial_grant_attributes(template):
    """Local initial state using recovered client defaults, not server parity.

    Graph constructor 0x142727d20 initializes these counters/arrays and purchase
    state. The original backend's grant serialization is unavailable. In
    particular, unlock_epoch has no recovered initial value and is omitted.
    Reconciliation applies this only to a missing item, preserving all existing
    graph state and the separately granted Battle Pass purchase token.
    """
    attributes = {'item_seen': False}
    if template.casefold() == 'quest:quest_ab_skulldude_grant':
        attributes.update({'quest_state': 'Active', 'level': -1,
                           'completion_season12_cumulative_skulldude_token': 0})
    if template.casefold() == MAYA_GRAPH.casefold():
        attributes.update({
            'player_random_seed': 0,
            'reward_graph_purchased_timestamp': 0,
            'unlock_keys_used': 0,
            'reward_graph_purchased': False,
            'reward_nodes_claimed': [],
            'player_state': [],
            'reward_keys': [],
        })
    return attributes
