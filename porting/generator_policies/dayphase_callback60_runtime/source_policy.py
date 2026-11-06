"""Guard the original BuildingActor native event before snippet attachment.

Call apply_signatures(name, block, functions, owner_fns, function_view, snippets)
just after build_functions_promoted, before attach_snippets. The standard native
 event promotion resolves this owner's candidate; this policy does not invent it.
"""


def apply_signatures(name, block, functions, owner_fns, function_view, snippets):
    if name != 'ABuildingActor' or not any(
            e.get('kind') == 'native_event_impl' and
            e.get('function') == 'OnDayPhaseChanged_Implementation' for e in snippets):
        return
    if block.get('block_va', '').lower() != '0x146252530' or '0x142733520' not in owner_fns:
        raise ValueError('original BuildingActor class identity changed')
    found = [f for f in block.get('functions', []) if f.get('name') == 'OnDayPhaseChanged']
    if len(found) != 1 or found[0].get('create_func', '').lower() != '0x142732c70':
        raise ValueError('original OnDayPhaseChanged identity changed')
    flags, params, problem = function_view(found[0], owner_fns)
    if problem or flags != 0x08020c00 or params is None or len(params) != 3:
        raise ValueError('original OnDayPhaseChanged metadata changed')
    expected = [
        (0, 'bAtCreation', 'Bool', '0x2c', None, {}),
        (1, 'PreviousDayPhase', 'Byte', '0x0', 1, {'enum': '0x142b24230'}),
        (2, 'CurrentDayPhase', 'Byte', '0x0', 0, {'enum': '0x142b24230'}),
    ]
    for p, (index, pname, kind, raw, offset, refs) in zip(params, expected):
        if (p.get('index') != index or p.get('name') != pname or p.get('kind') != kind
                or p.get('kind_raw') != raw or p.get('offset') != offset
                or p.get('type_refs', {}) != refs or p.get('array_dim') != 1
                or p.get('property_flags') != '0x10000000000080'
                or p.get('object_flags') != '0x45' or p.get('replicated') is not False):
            raise ValueError('original OnDayPhaseChanged parameter changed: ' + pname)
    if (params[0].get('element_size') != 1 or params[0].get('size_of_outer') != 3
            or params[0].get('bitfield') is not True
            or params[0].get('set_bit_fn', '').lower() != '0x14076c560'):
        raise ValueError('original OnDayPhaseChanged bool metadata changed')
    promoted = [f for f in functions if f.get('name') == 'OnDayPhaseChanged']
    expected_args = ['EFortDayPhase::Type CurrentDayPhase', 'EFortDayPhase::Type PreviousDayPhase',
                     'bool bAtCreation']
    if (len(promoted) != 1 or promoted[0].get('args') != expected_args
            or promoted[0].get('ret') != 'void' or promoted[0].get('original_flags') != flags
            or promoted[0].get('specifiers') != ['BlueprintNativeEvent']
            or not promoted[0].get('native_event') or not promoted[0].get('splice')
            or promoted[0].get('static') or promoted[0].get('const')):
        raise ValueError('OnDayPhaseChanged native event promotion failed or changed')
