"""Emit the evidenced FQuickBar refresh member and its single translation unit."""
import hashlib
from pathlib import Path

HERE = Path(__file__).resolve().parent
IMAGE = Path('/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin')
WINDOWS = {
    0x142063590: 'eee2eb53121404c1c4bf83320f730802af79b50030bf12cf60f3048a9c3e19db',
    0x142050360: 'fdff247de809a46650a61e0259e80c23fc463dff755f04624e7994b835f69617',
    0x14202b8d0: '34ee53abc8375e2ac37beb97729c96b6814e2cffd64021e07b73994a76b6b214',
    0x141ff80b0: '2441f2e5d9e8978d322c9f9d82e23e1f5799a6eb78400fc57a27fa650a7821f7',
    0x141c96700: '78e8e8c2a08f67f513f61432a9928813735d7fa1f6fa561de2173f2e4f85e0d8',
    0x1402b87c0: '14026135181e45c170c9743ae3c054d01c996b72f3041a2f51013c700c9ca391',
    0x1402e35b0: 'c649671747401e7239888cffb552f7cd60c64a9b730523f4b85da31eeed909b5',
    0x1402e3540: '5a755ff014ac859e3a44e9cf17931039979c7df04d046d678c3ae74d95f26a83',
}

def validate():
    with IMAGE.open('rb') as stream:
        for address, expected in WINDOWS.items():
            stream.seek(address - 0x140000000)
            if hashlib.sha256(stream.read(128)).hexdigest() != expected:
                raise ValueError('quickbar native identity changed at ' + hex(address))

def declarations(model):
    if model['cpp_name'] != 'FQuickBar':
        return []
    expected = [(0, 'int32 CurrentFocusedSlot;'), (4, 'int32 PreviousFocusedSlot;'),
                (8, 'int32 SecondaryFocusedSlot;'), (16, 'TArray<FQuickBarSlot> Slots;'),
                (32, 'FQuickBarData DataDefinition;'),
                (48, 'TSet<UFortItemDefinition*> EquippedItemDefinitions;'),
                (128, 'TArray<int32> SharedVisibleSlotIndicesWhenUsingGamepad;')]
    if (model['package'] != '/Script/FortniteGame' or model['size'] != 144 or
            model['align'] != 8 or [(m['offset'], m['decl']) for m in model['members']] != expected):
        raise ValueError('FQuickBar reflected identity changed')
    validate()
    return ['\tvoid RefreshEquippedItemDefinitions(class AFortPlayerController* Owner);', '']

def sources():
    validate()
    return {'FortQuickBarRefresh.cpp': (HERE / 'FortQuickBarRefresh.cpp').read_text()}
