"""Faction selection, limited to the recovered Season 12 native request shape.

Original 0x144e64070/0x144e643b0 send one or two quest InstanceIds to
ToggleQuestActiveState, including the active opposite quest when switching.
Backend rejection rules were not shipped with the client. The validation below
is local policy: require the earned bundle, validate the entire batch, preserve
one selected branch, and never reopen a completed selection.
"""

from api_common import ApiError, Err
from season12_rewards import PAID
import season12_brutus_faction_data as brutus
import season12_tntina_faction_data as tntina
import season12_remaining_faction_data as remaining


# SpatialHenchmenData associates each mission pair with this character. Native
# 0x142006980 derives its level and premium requirement from the season reward
# schedule, and 0x144ec47d0 checks them before offering activation. All five
# characters occur in the paid schedule; its keys are the cooked array indices.
_CHARACTERS = {
    'Brutus': 'AthenaCharacter:CID_692_Athena_Commando_M_HenchmanTough',
    'TNTina': 'AthenaCharacter:CID_691_Athena_Commando_F_TNTina',
    'meowscles': 'AthenaCharacter:CID_693_Athena_Commando_M_BuffCat',
    'skye': 'AthenaCharacter:CID_690_Athena_Commando_F_Photographer',
    'midas': 'AthenaCharacter:CID_694_Athena_Commando_M_CatBurglar',
}
_REQUIRED_LEVEL = {
    name: next(level for level, rewards in PAID.items()
               if (character, 1) in rewards)
    for name, character in _CHARACTERS.items()
}


# The branch names come from each cooked bundle, not a naming convention.
SELECTIONS = tuple(
    (name, data.BUNDLE, data.GRANT_QUEST,
     frozenset(('quest:' + quest).casefold()
               for quest, _, _, _ in data.BUNDLES[data.BUNDLE]['quest_infos']))
    for name, data in (('Brutus', brutus), ('TNTina', tntina))
) + tuple(
    (name, faction['bundle'], faction['grant_quest'],
     frozenset(('quest:' + quest).casefold()
               for quest, _, _, _ in remaining.BUNDLES[faction['bundle']]['quest_infos']))
    for name, faction in remaining.FACTIONS.items()
)


def _invalid(message):
    return ApiError(400, Err.INVALID_PAYLOAD, message)


def toggle_faction_selection(ctx, quest_ids):
    if (not 1 <= len(quest_ids) <= 2 or
            any(not isinstance(value, str) or not value for value in quest_ids) or
            len(set(quest_ids)) != len(quest_ids)):
        raise _invalid('questIds must contain one or two distinct quest instance IDs.')
    tx = ctx.tx('athena')
    items = tx.profile['items']
    # Resolve the selection from owned instance IDs and reject mixed batches.
    requested_templates = {items[item_id].get('templateId', '').casefold()
                           for item_id in quest_ids if item_id in items}
    matching = [selection for selection in SELECTIONS
                if requested_templates and requested_templates <= selection[3]]
    if len(matching) != 1 or any(item_id not in items for item_id in quest_ids):
        raise _invalid('questIds must refer to owned branch quests from one faction.')
    name, bundle, grant, branch_templates = matching[0]
    stats = tx.profile['stats']['attributes']
    level = stats.get('level')
    # 0x141997780 reads "level", guarded by season_num, rather than book_level.
    # Reject malformed stored values as well as callers bypassing the UI gates.
    if (stats.get('season_num') != 12 or stats.get('book_purchased') is not True or
            type(level) is not int or level < _REQUIRED_LEVEL[name]):
        raise _invalid(f'The {name} faction challenges require the Battle Pass '
                       f'and season level {_REQUIRED_LEVEL[name]}.')
    branches = {}
    bundles = []
    earned = False
    for item_id, item in items.items():
        template = item.get('templateId', '').casefold()
        if template in branch_templates:
            if template in branches:
                raise _invalid(f'The {name} selection has duplicate branch quests.')
            branches[template] = (item_id, item)
        elif template == ('challengebundle:' + bundle).casefold():
            bundles.append(item_id)
        elif template == ('quest:' + grant).casefold():
            earned = item.get('attributes', {}).get('quest_state') == 'Claimed'
    if not earned or len(bundles) != 1 or set(branches) != branch_templates:
        raise _invalid(f'The {name} faction challenges have not been earned.')

    states = {}
    for item_id, item in branches.values():
        attrs = item.get('attributes', {})
        if attrs.get('challenge_bundle_id') != bundles[0]:
            raise _invalid(f'The {name} branch quest belongs to another bundle.')
        state = attrs.get('quest_state')
        if state not in ('Inactive', 'Active'):
            raise _invalid(f'The {name} faction selection is no longer available.')
        states[item_id] = state
    for item_id in quest_ids:
        if item_id not in states:
            raise _invalid(f'questIds must refer to owned {name} branch quests.')
    next_states = dict(states)
    for item_id in quest_ids:
        next_states[item_id] = 'Inactive' if states[item_id] == 'Active' else 'Active'
    if list(next_states.values()).count('Active') != 1:
        raise _invalid(f'Select exactly one {name} faction challenge.')

    timestamp = ctx.ctx.iso()
    for item_id in quest_ids:
        tx.set_item_attr(item_id, 'quest_state', next_states[item_id])
        tx.set_item_attr(item_id, 'last_state_change_time', timestamp)
