"""Season-level snapshots for owned Season 12 superlevel quests.

The snapshot and Stage1 fallback are local policies. The source does not
establish the continuous gold appearance or its interpolation formula.
"""
from profile_service import _invalid
from season12_superlevel_data import SUPERLEVEL_QUESTS, SUPERLEVEL_VARIANT_TOKENS

SUPERLEVEL_VARIANTS = {
    ('CosmeticVariantToken:' + name).casefold(): (
        ('AthenaCharacter:' if data['cosmetic_item'].startswith('CID_') else 'AthenaBackpack:')
        + data['cosmetic_item'],
        data['channel'].rsplit('.', 1)[-1], data['variant'].rsplit('.', 1)[-1], 'Stage1')
    for name, data in SUPERLEVEL_VARIANT_TOKENS.items()
}


def reconcile_level(quests, ctx):
    """Synchronize eligible owned quests inside the caller's transaction."""
    tx = ctx.tx('athena')
    stats = tx.profile['stats']['attributes']
    if stats.get('season_num') != 12 or stats.get('book_purchased') is not True:
        return
    level = stats.get('level')
    if isinstance(level, bool) or not isinstance(level, int) or not 1 <= level <= 1000:
        raise _invalid('Invalid season level.')

    pending = []
    seen = set()
    items = tx.profile['items']
    for item_id, item in list(items.items()):
        template = item.get('templateId')
        if not isinstance(template, str) or not template.casefold().startswith('quest:'):
            continue
        name = template.split(':', 1)[1].casefold()
        if name not in SUPERLEVEL_QUESTS:
            continue
        attrs = item.get('attributes')
        if not isinstance(attrs, dict):
            raise _invalid('Invalid quest attributes.')
        bundle_id = attrs.get('challenge_bundle_id')
        if bundle_id is not None and not isinstance(bundle_id, str):
            raise _invalid('Invalid quest bundle reference.')
        bundle = items.get(bundle_id)
        if not isinstance(bundle, dict) or str(bundle.get('templateId', '')).casefold() != 'challengebundle:missionbundle_s12_superlevel':
            continue
        if name in seen:
            raise _invalid('Duplicate superlevel quest.')
        seen.add(name)
        bundle_attrs = bundle.get('attributes')
        if not isinstance(bundle_attrs, dict):
            raise _invalid('Invalid bundle attributes.')
        for counter in ('num_quests_completed', 'num_progress_quests_completed'):
            count = bundle_attrs.get(counter, 0)
            if isinstance(count, bool) or not isinstance(count, int) or count < 0:
                raise _invalid('Invalid bundle progress.')
        objective, _, target = SUPERLEVEL_QUESTS[name]['objectives'][0]
        key = 'completion_' + objective.lower()
        value = attrs.get(key)
        state = attrs.get('quest_state')
        if (state not in ('Active', 'Inactive', 'Completed', 'Claimed') or isinstance(value, bool)
                or not isinstance(value, int) or not 0 <= value <= target):
            raise _invalid('Invalid quest progress.')
        if state in ('Completed', 'Claimed') and value != target:
            raise _invalid('Invalid completed quest progress.')
        if state == 'Completed' and level >= target:
            pending.append((item_id, item, name, key, target, target))
        elif state == 'Active':
            pending.append((item_id, item, name, key, min(level, target), target))

    for item_id, item, name, key, value, target in pending:
        if item['attributes'][key] != value:
            tx.set_item_attr(item_id, key, value)
        if value == target:
            quests._award_completed(ctx, tx, item_id, item, name)
