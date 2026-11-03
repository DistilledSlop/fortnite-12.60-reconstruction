"""Attach recovered native volume accessor bases and declare the interface contract."""

BLOCKS = {
    'AFortMinigame': ('0x146446ef0', 3, 'Volume', 0x910, '0x20080100022835', 'OnRep_Volume'),
    'AFortMinigameSettingsBuilding': ('0x1464535b0', 2, 'SettingsVolume', 0x308, '0x20080100002034', 'OnRep_SettingsVolume'),
    'AFortVolume': ('0x1465647c0', 2, None, None, None, None),
}


def apply(model, block, owner_fns):
    binding = BLOCKS.get(model['name'])
    if binding is None:
        return model
    validate_evidence()
    expected, count, field, offset, flags, notify = binding
    if (model.get('block_va') != expected or block.get('block_va') != expected
            or block.get('interface_count') != count):
        raise ValueError('volume accessor original class identity changed')
    if field is not None:
        matches = [p for p in block.get('properties', []) if p.get('name') == field]
        if len(matches) != 1:
            raise ValueError('volume accessor field missing or ambiguous: ' + field)
        p = matches[0]
        if (p.get('kind') != 'Object' or p.get('offset') != offset
                or p.get('type_refs') != {'class': '0x142b32780'}
                or p.get('property_flags') != flags or p.get('rep_notify') != notify
                or p.get('array_dim') != 1):
            raise ValueError('volume accessor field metadata changed: ' + field)
    bases = model.setdefault('native_interface_bases', [])
    if 'IFortVolumeAccessor' not in bases:
        bases.append('IFortVolumeAccessor')
    model['forward_decls'] = sorted(set(model.get('forward_decls', [])) | {'IFortVolumeAccessor'})
    return model


def interface_preamble(name):
    return ['class AFortVolume;'] if name == 'UFortVolumeAccessor' else []


def interface_declarations(name):
    if name == 'UFortVolumeAccessor':
        return ['public:', '    virtual AFortVolume* NativeGetVolume() = 0;']
    return []


_VALIDATED = False

def validate_evidence():
    global _VALIDATED
    if _VALIDATED:
        return
    import hashlib
    import json
    from pathlib import Path
    evidence = json.loads((Path(__file__).parent / 'native-evidence.json').read_text())
    with Path(evidence['image']).open('rb') as stream:
        for window in evidence['windows']:
            stream.seek(int(window['begin'], 16) - int(evidence['image_base'], 16))
            data = stream.read(window['length'])
            if len(data) != window['length'] or hashlib.sha256(data).hexdigest() != window['sha256']:
                raise ValueError('volume accessor original capture changed: ' + window['begin'])
    _VALIDATED = True
