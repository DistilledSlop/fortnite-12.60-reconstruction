"""Restore evidenced native inventory struct members without UPROPERTY metadata."""
import hashlib
import json
from pathlib import Path
HERE = Path(__file__).resolve().parent

def fields_after(model, member):
    name = model['cpp_name']
    expected = {'FFortGiftingInfo': ('UFortHeroType* HeroType;', 16, 40),
                'FFortItemList': ('TArray<FFortItemEntry> ReplicatedEntries;', 264, 456)}
    if name not in expected:
        return []
    decl, offset, size = expected[name]
    if member['decl'] != decl:
        return []
    if member['offset'] != offset or model['size'] != size:
        raise ValueError('native inventory struct identity changed: ' + name)
    evidence = json.loads((HERE / 'native-evidence.json').read_text())
    with Path(evidence['image']).open('rb') as stream:
        for window in evidence['windows']:
            start, end = int(window['begin'], 16), int(window['end_exclusive'], 16)
            stream.seek(start - 0x140000000)
            if hashlib.sha256(stream.read(end - start)).hexdigest() != window['sha256']:
                raise ValueError('native inventory original bytes changed: ' + window['begin'])
    if name == 'FFortGiftingInfo':
        return ['\t// Original native shared state at +0x18; dynamic controller retains payload destruction.',
                '\tTSharedPtr<void, ESPMode::NotThreadSafe> NativeGiftPayload;', '']
    return ['\t// Original native GUID index at +0x118; values alias existing entries.',
            '\tTMap<FGuid, FFortItemEntry*, FDefaultSetAllocator, FFortEntryGuidKeyFuncs> NativeEntriesByGuid;', '']

def preamble(model):
    if model['cpp_name'] != 'FFortItemList':
        return []
    return ['// Original GUID index uses deprecated memory CRC, unlike current engine CityHash GUID keys.',
            'struct FFortEntryGuidKeyFuncs : TDefaultMapHashableKeyFuncs<FGuid, FFortItemEntry*, false>',
            '{', '    static uint32 GetKeyHash(KeyInitType Key)', '    {',
            '        return FCrc::MemCrc_DEPRECATED(&Key, sizeof(FGuid), 0);', '    }', '};', '']
