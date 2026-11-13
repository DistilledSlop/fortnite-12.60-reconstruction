"""Keep locker snapshots consistent with auto-equipped cosmetic rewards."""
import copy

from api_common import ApiError, Err
from profile_defaults import CATEGORY_BY_PREFIX


def sync_lockers(tx, item_id, template, variants, *, channel=None):
    category = next((slot for prefix, slot in CATEGORY_BY_PREFIX.items()
                     if prefix.casefold() == template.split(':', 1)[0].casefold()), None)
    if category is None:
        return
    for locker_id, locker in list(tx.profile['items'].items()):
        if not locker.get('templateId', '').casefold().startswith('cosmeticlocker:'):
            continue
        data = locker['attributes'].get('locker_slots_data', {})
        slot = data.get('slots', {}).get(category, {})
        items = slot.get('items', [])
        indices = [i for i, value in enumerate(items)
                   if isinstance(value, str) and value.casefold() in
                   (item_id.casefold(), template.casefold())]
        if not indices:
            continue
        data = copy.deepcopy(data)
        slot = data['slots'][category]
        active = slot.get('activeVariants', [])
        if not isinstance(active, list):
            raise ApiError(400, Err.INVALID_PAYLOAD, 'Invalid equipped cosmetic variants.')
        if channel is None:
            active = active[:len(items)]
        while len(active) < len(items):
            active.append({'variants': []})
        for index in indices:
            if channel is None:
                active[index] = {'variants': copy.deepcopy(variants)}
                continue
            snapshot = active[index]
            if not isinstance(snapshot, dict) or not isinstance(snapshot.get('variants'), list):
                raise ApiError(400, Err.INVALID_PAYLOAD, 'Invalid equipped cosmetic variants.')
            entries = snapshot['variants']
            replacement = next(v for v in variants if v.get('channel') == channel)
            for i, entry in enumerate(entries):
                if isinstance(entry, dict) and entry.get('channel') == channel:
                    entries[i] = copy.deepcopy(replacement)
                    break
            else:
                entries.append(copy.deepcopy(replacement))
        slot['activeVariants'] = active
        tx.set_item_attr(locker_id, 'locker_slots_data', data)
