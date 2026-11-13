"""Maya preview and permanent choices, using the recovered 12.60 commands.

Client callers pass the graph instance ID and ERG node tags. Native key
eligibility subtracts reward_keys.unlock_keys_used from owned Token quantity;
earned tokens therefore remain owned. Backend validation, retry handling and
purchase-token reconciliation below are local policies, not recovered Epic
server code. All mutations share the ordinary profile transaction.
"""

import copy
import math
import re
import struct

from api_common import ApiError, Err
from locker_variant_sync import sync_lockers
from season12_maya_graph_data import (CHANNELS, GRAPH_TEMPLATE, NODES,
                                      PURCHASE_TOKEN, TARGET_ITEM, TIER_KEYS)


def _invalid(message):
    return ApiError(400, Err.INVALID_PAYLOAD, message)


def _owned(tx, template):
    return [(item_id, item) for item_id, item in tx.profile['items'].items()
            if item.get('templateId', '').casefold() == template.casefold()]


def reconcile_purchase(ctx):
    """Apply the cooked purchase entitlement once; leave other graphs alone."""
    tx = ctx.tx('athena')
    tokens = _owned(tx, PURCHASE_TOKEN)
    if not tokens:
        return
    graphs = _owned(tx, GRAPH_TEMPLATE)
    if not graphs or not _owned(tx, 'AthenaCharacter:' + TARGET_ITEM):
        return
    if len(graphs) != 1 or any(type(v.get('quantity')) is not int or
                               v['quantity'] != 1 for _, v in tokens):
        raise _invalid('Invalid Maya purchase entitlement.')
    graph_id, graph = graphs[0]
    if graph['attributes'].get('reward_graph_purchased') is not True:
        tx.set_item_attr(graph_id, 'reward_graph_purchased', True)
        tx.set_item_attr(graph_id, 'reward_graph_purchased_timestamp', int(ctx.ctx.now()))
    for token_id, _ in tokens:
        tx.remove_item(token_id)


def _graph(ctx):
    graph_id = ctx.str_field('rewardGraphId', max_len=200)
    tx = ctx.tx('athena')
    graph = tx.item(graph_id)
    if (graph.get('templateId', '').casefold() != GRAPH_TEMPLATE.casefold() or
            len(_owned(tx, GRAPH_TEMPLATE)) != 1):
        raise _invalid('rewardGraphId must identify the owned Maya graph instance.')
    return tx, graph_id, graph


def _claims(graph):
    claimed = graph['attributes'].get('reward_nodes_claimed', [])
    if not isinstance(claimed, list):
        raise _invalid('Invalid Maya claimed nodes.')
    tiers = {}
    for tag in claimed:
        if not isinstance(tag, str) or tag not in NODES:
            raise _invalid('Invalid Maya claimed node.')
        node = NODES[tag]
        if node.tier_key in tiers:
            raise _invalid('Multiple permanent Maya choices in one tier.')
        tiers[node.tier_key] = tag
    return tiers


def _color(value):
    # The original caller prints four raw float words with %08X. Restrict local
    # writes to finite RGBA values: the client reader itself is more permissive.
    if not isinstance(value, str) or not re.fullmatch('[0-9A-Fa-f]{32}', value):
        raise _invalid('Maya color must contain four eight-digit hexadecimal words.')
    floats = struct.unpack('>ffff', bytes.fromhex(value))
    if not all(math.isfinite(part) for part in floats):
        raise _invalid('Maya color components must be finite.')
    return value.upper()


def _parse_state(state):
    if not isinstance(state, list) or len(state) > len(TIER_KEYS) + 1:
        raise _invalid('Invalid Maya preview state.')
    chosen = {}
    color = None
    normalized = []
    for entry in state:
        if not isinstance(entry, str):
            raise _invalid('Maya preview entries must be strings.')
        if entry in NODES:
            node = NODES[entry]
            if node.tier_key in chosen:
                raise _invalid('Maya preview contains multiple choices for one tier.')
            chosen[node.tier_key] = entry
            normalized.append(entry)
        elif entry.startswith('Cosmetics.Variant.Channel.RichColor:') and color is None:
            color = _color(entry.split(':', 1)[1])
            normalized.append('Cosmetics.Variant.Channel.RichColor:' + color)
        else:
            raise _invalid('Unknown or duplicate Maya preview entry.')
    return chosen, color, normalized


def set_reward_graph_config(ctx):
    tx, graph_id, graph = _graph(ctx)
    claimed = _claims(graph)
    chosen, color, normalized = _parse_state(ctx.list_field('state'))
    # Preview choices do not grant cosmetic ownership or spend earned keys.
    # Permanent choices cannot be replaced by a later preview request.
    for key, tag in claimed.items():
        if chosen.get(key, tag) != tag:
            raise _invalid('A permanent Maya choice cannot be changed.')
        if key not in chosen:
            normalized.append(tag)
    if 'ERG.Node.Tier10.1' in claimed.values():
        _, old_color, _ = _parse_state(graph['attributes'].get('player_state', []))
        if color is not None and color != old_color:
            raise _invalid('A permanent Maya color cannot be changed.')
        if color is None and old_color is not None:
            normalized.append('Cosmetics.Variant.Channel.RichColor:' + old_color)
    _, _, old = _parse_state(graph['attributes'].get('player_state', []))
    if len(old) == len(normalized) and set(old) == set(normalized):
        return
    tx.set_item_attr(graph_id, 'player_state', normalized)


def _spent(graph, claimed):
    rows = graph['attributes'].get('reward_keys', [])
    if not isinstance(rows, list):
        raise _invalid('Invalid Maya key accounting.')
    used = {}
    known = {'Token:' + key.token: key for key in TIER_KEYS.values()}
    for row in rows:
        if not isinstance(row, dict):
            raise _invalid('Invalid Maya key accounting.')
        template, count = row.get('static_key_template_id'), row.get('unlock_keys_used')
        if (not isinstance(template, str) or template not in known or template in used or
                type(count) is not int or count < 0 or count > known[template].maximum):
            raise _invalid('Invalid Maya spent key.')
        used[template] = count
    for key in TIER_KEYS.values():
        expected = NODES[claimed[key.token]].key_count if key.token in claimed else 0
        if used.get('Token:' + key.token, 0) != expected:
            raise _invalid('Maya spent keys disagree with permanent choices.')
    return used


def _grant_variant(tx, node, config):
    """Apply the token's recovered channel/option without persisting the token."""
    targets = _owned(tx, 'AthenaCharacter:' + TARGET_ITEM)
    if not targets:
        raise _invalid('The Maya outfit must be owned before locking a choice.')
    channel = node.channel.removeprefix('Cosmetics.Variant.Channel.')
    option = node.unlocked_tag.removeprefix('Cosmetics.Variant.Property.')
    defaults = [tag.removeprefix('Cosmetics.Variant.Property.')
                for tag, default in CHANNELS[node.channel].options if default]
    for item_id, item in targets:
        variants = copy.deepcopy(item['attributes'].get('variants', []))
        if not isinstance(variants, list) or any(not isinstance(v, dict) for v in variants):
            raise _invalid('Invalid Maya cosmetic variants.')
        matches = [v for v in variants if v.get('channel') == channel]
        if len(matches) > 1:
            raise _invalid('Duplicate Maya cosmetic variant channel.')
        if matches:
            entry = matches[0]
            if (not isinstance(entry.get('owned'), list) or
                    any(not isinstance(v, str) for v in entry['owned']) or
                    not isinstance(entry.get('active'), str)):
                raise _invalid('Invalid Maya owned styles.')
        else:
            entry = {'channel': channel, 'active': defaults[0], 'owned': defaults[:]}
            variants.append(entry)
        if option not in entry['owned']:
            entry['owned'].append(option)
        if node.auto_equip is not False:
            entry['active'] = option
        if node.operation == 'RichColor':
            # Native FMcpVariantReader expands channel RichColor with the
            # channel prefix, and owned RichColor with the property prefix.
            # Its active parser splits RichColor.<raw RGBA words> into the
            # RichColor tag plus custom material data (0x141d601f0).
            rich = [v for v in variants if v.get('channel') == 'RichColor']
            if len(rich) > 1:
                raise _invalid('Duplicate Maya color channel.')
            if rich:
                entry = rich[0]
                if (not isinstance(entry.get('owned'), list) or
                        any(not isinstance(v, str) for v in entry['owned'])):
                    raise _invalid('Invalid Maya owned colors.')
                if 'RichColor' not in entry['owned']:
                    entry['owned'].append('RichColor')
                entry['active'] = 'RichColor.' + config
            else:
                variants.append({'channel': 'RichColor', 'active': 'RichColor.' + config,
                                 'owned': ['RichColor']})
        tx.set_item_attr(item_id, 'variants', variants)
        sync_lockers(tx, item_id, item['templateId'], variants)
        if node.mark_unseen is not False:
            tx.set_item_attr(item_id, 'item_seen', False)


def unlock_reward_node(ctx):
    tx, graph_id, graph = _graph(ctx)
    tag = ctx.str_field('nodeId', max_len=100)
    node = NODES.get(tag)
    if node is None:
        raise _invalid('Unknown Maya reward node.')
    config = ctx.str_field('rewardCfg', allow_empty=True, max_len=32)
    if node.operation == 'RichColor':
        config = _color(config)
    elif config:
        raise _invalid('This Maya choice does not accept a color configuration.')
    if graph['attributes'].get('reward_graph_purchased') is not True:
        raise _invalid('The Maya graph requires its purchase entitlement.')
    claimed = _claims(graph)
    used = _spent(graph, claimed)
    if node.tier_key in claimed:
        if claimed[node.tier_key] != tag:
            raise _invalid('This Maya tier already has a permanent choice.')
        _, old_color, _ = _parse_state(graph['attributes'].get('player_state', []))
        if node.operation == 'RichColor' and old_color != config:
            raise _invalid('This Maya tier already has a permanent color.')
        return
    template = 'Token:' + node.tier_key
    earned = _owned(tx, template)
    if any(type(item.get('quantity')) is not int or item['quantity'] < 0 for _, item in earned):
        raise _invalid('Invalid earned Maya key quantity.')
    quantity = sum(item['quantity'] for _, item in earned)
    if quantity < node.minimum_key_count or quantity - used.get(template, 0) < node.key_count:
        raise _invalid('The challenge key for this Maya choice has not been earned.')
    _grant_variant(tx, node, config)
    state = graph['attributes'].get('player_state', [])
    _, _, state = _parse_state(state)
    state = [entry for entry in state
             if ((entry not in NODES or NODES[entry].tier_key != node.tier_key) and
                 not (node.operation == 'RichColor' and
                      entry.startswith('Cosmetics.Variant.Channel.RichColor:')))]
    state.append(tag)
    if node.operation == 'RichColor':
        state.append('Cosmetics.Variant.Channel.RichColor:' + config)
    rows = copy.deepcopy(graph['attributes'].get('reward_keys', []))
    row = next((row for row in rows if row['static_key_template_id'] == template), None)
    if row is None:
        rows.append({'static_key_template_id': template, 'unlock_keys_used': node.key_count})
    else:
        row['unlock_keys_used'] += node.key_count
    tx.set_item_attr(graph_id, 'reward_keys', rows)
    tx.set_item_attr(graph_id, 'unlock_keys_used', sum(row['unlock_keys_used'] for row in rows))
    tx.set_item_attr(graph_id, 'reward_nodes_claimed', list(graph['attributes'].get('reward_nodes_claimed', [])) + [tag])
    tx.set_item_attr(graph_id, 'player_state', state)
