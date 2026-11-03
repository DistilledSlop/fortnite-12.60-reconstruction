"""Restore the captured derived ASC interface and proven concrete bindings."""
import hashlib
import json
from pathlib import Path
HERE = Path(__file__).resolve().parent
EVIDENCE = json.loads((HERE / 'native-evidence.json').read_text())
BINDINGS = {e['class']: e for e in EVIDENCE['classes'] if e['class'] != 'ABuildingActor'}
_VALIDATED = False

def validate():
    global _VALIDATED
    if _VALIDATED:
        return
    with Path(EVIDENCE['source_image']).open('rb') as stream:
        for window in EVIDENCE['windows']:
            stream.seek(int(window['begin'], 16) - 0x140000000)
            if hashlib.sha256(stream.read(window['length'])).hexdigest() != window['sha256']:
                raise ValueError('Fort ability interface capture changed: ' + window['begin'])
    _VALIDATED = True

def derived_interface(name, parent):
    if name != 'UFortAbilitySystemInterface':
        return False
    validate()
    if parent.get('parent_name') != 'UAbilitySystemInterface' or parent.get('parent_va') != '0x1459d3530' or parent.get('package') != '/Script/FortniteGame':
        raise ValueError('Fort ability interface ancestry changed')
    return True

def apply(model, block, owner_fns):
    entry = BINDINGS.get(model['name'])
    if entry is None:
        return model
    validate()
    if model.get('block_va') != entry['block'] or block.get('block_va') != entry['block']:
        raise ValueError('Fort ability interface implementer changed')
    getters = [f for f in model.get('helpers', []) if f['name'] == 'GetAbilitySystemComponent']
    if len(getters) != 1 or getters[0]['ret'] != 'UAbilitySystemComponent*' or getters[0]['args'] or not getters[0]['const'] or not getters[0]['virtual']:
        raise ValueError('Fort ability interface requires its captured concrete getter: ' + model['name'] + ': ' + repr(getters) + ' ' + repr(model.get('splice', {}).get('rejected', [])))
    bases = model.setdefault('native_interface_bases', [])
    bases[:] = [base for base in bases if base != 'IAbilitySystemInterface']
    if 'IFortAbilitySystemInterface' not in bases:
        bases.append('IFortAbilitySystemInterface')
    model['splice'].setdefault('header_includes', set()).add('UFortAbilitySystemInterface.h')
    return model

def render_header(name, source_label, api):
    if name != 'UFortAbilitySystemInterface':
        return None
    validate()
    return f'''// Generated from {source_label}; native ability_interface302_runtime policy.
#pragma once
#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
class UFortAbilitySystemComponent;
#include "UFortAbilitySystemInterface.generated.h"

UINTERFACE(meta=(CannotImplementInterfaceInBlueprint))
class {api} UFortAbilitySystemInterface : public UAbilitySystemInterface
{{
    GENERATED_BODY()
}};

class {api} IFortAbilitySystemInterface : public IAbilitySystemInterface
{{
    GENERATED_IINTERFACE_BODY()
public:
    // Captured slot +0x20 (0x14084be30): invoke the inherited getter and cast.
    virtual UFortAbilitySystemComponent* NativeGetFortAbilitySystemComponent() const;
}};
'''

def render_cpp(name, source_label):
    if name != 'UFortAbilitySystemInterface':
        return None
    validate()
    return '''// Generated native ability interface default implementation.
#include "UFortAbilitySystemInterface.h"
#include "UFortAbilitySystemComponent.h"

UFortAbilitySystemComponent* IFortAbilitySystemInterface::NativeGetFortAbilitySystemComponent() const
{
    return Cast<UFortAbilitySystemComponent>(GetAbilitySystemComponent());
}
'''
