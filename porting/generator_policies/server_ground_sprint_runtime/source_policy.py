"""Original sprint-owned constructor defaults.

Compressed movement decoding is now recovered through behavior/MANIFEST.json.
The former diagnostic bridge is retired; complete input/ability lifecycle remains open.
"""
import re
import hashlib
import json
from pathlib import Path

HERE = Path(__file__).resolve().parent


def validate_constructor_source():
    evidence = json.loads((HERE / 'native-evidence.json').read_text())
    with Path(evidence['image']).open('rb') as stream:
        for window in evidence['source_windows']:
            stream.seek(int(window['address'], 0) - 0x140000000)
            if hashlib.sha256(stream.read(window['length'])).hexdigest() != window['sha256']:
                raise ValueError('sprint constructor evidence changed: ' + window['address'])

SPRINT_CTOR = '''
UFortGameplayAbility_Sprint::UFortGameplayAbility_Sprint(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    // Complete own assignments of 0x141615d90; superclass recovery is separate.
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
    ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateNo;
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    ActivationType = EFortGameplayAbilityActivation::Triggered;
    AbilityTags.AddTag(FFortNativeGameplayTagsRuntime::Get().GetTagAtOriginalOffset(0x690));
    ActivationOwnedTags.AddTag(FFortNativeGameplayTagsRuntime::Get().GetTagAtOriginalOffset(0x690));
}
'''


def finalize(model, header, cpp):
    name = model.get('name')
    if name != 'UFortGameplayAbility_Sprint':
        return header, cpp
    if model.get('package') != '/Script/FortniteGame' or header.count('public:') != 1:
        raise ValueError(name + ': unexpected sprint integration owner')
    if name == 'UFortGameplayAbility_Sprint':
        validate_constructor_source()
        if model.get('parent') != 'UFortGameplayAbility_Action' or re.search(r'\b' + name + r'\s*\(', header):
            raise ValueError(name + ': changed constructor contract')
        header = header.replace('public:', 'public:\n\t' + name + '(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());', 1)
        return header, cpp + '\n#include "FortNativeGameplayTagsRuntime.h"\n' + SPRINT_CTOR
