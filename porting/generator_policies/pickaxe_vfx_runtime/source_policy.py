"""12.60 native pickaxe VFX selector: pinned, fail-closed generator hooks."""
STRUCT = 'FMarshalledVFXRuntimeData'
CLASS = 'AFortWeaponPickaxeAthena'
FUNCTIONS = ('GetIdleVFXComponent', 'GetSwingVFXComponent')


def struct_includes(model):
    if model.get('cpp_name') != STRUCT:
        return []
    _check_struct(model)
    return ['GameplayTagContainer.h']


def _check_struct(model):
    members = [(m.get('name'), m.get('offset'), m.get('decl')) for m in model.get('members', [])]
    expected = [('DynamicSystems', 80, 'TArray<UFXSystemComponent*> DynamicSystems;'),
                ('BasedOn', 96, 'USceneComponent* BasedOn;')]
    if (members != expected or model.get('parent') or model.get('fn') != '0x142b36fd0'
            or model.get('size') != 104 or model.get('align') != 8
            or 'UFXSystemComponent' not in model.get('forward_decls', [])):
        raise ValueError('FMarshalledVFXRuntimeData original member contract changed: %r' % (members,))


def struct_fields(model):
    if model.get('cpp_name') != STRUCT:
        return []
    _check_struct(model)
    return ['\t// 0x141dcc1d0: non-reflected weak component -> owned gameplay tags map.',
            '\tTMap<TWeakObjectPtr<UFXSystemComponent>, FGameplayTagContainer> ComponentTags;', '']


def interface_preamble(name):
    return ['struct FMarshalledVFXRuntimeData;'] if name == 'UFortVFXDataOwner' else []


def interface_declarations(name):
    if name != 'UFortVFXDataOwner':
        return []
    return ['public:', '\tvirtual FMarshalledVFXRuntimeData* GetMarshalledVFXRuntimeData() = 0;']


def apply(model):
    if model.get('name') != CLASS:
        return model
    found = {f['name']: f for f in model.get('functions', []) if f.get('name') in FUNCTIONS}
    if set(found) != set(FUNCTIONS):
        raise ValueError('both pickaxe VFX reflection splices must be accepted before policy application')
    for name, f in found.items():
        if (f.get('ret') != 'UFXSystemComponent*' or f.get('args') != [] or not f.get('const')
                or f.get('original_flags') != 0x54020400 or not f.get('accepted')):
            raise ValueError('unexpected pickaxe VFX getter contract for ' + name)
        f['virtual'] = True
    includes = model.setdefault('cpp_includes', [])
    if 'FortPickaxeVFXRuntime.h' not in includes:
        includes.append('FortPickaxeVFXRuntime.h')
    if 'Particles/ParticleSystemComponent.h' not in includes:
        includes.append('Particles/ParticleSystemComponent.h')
    return model
