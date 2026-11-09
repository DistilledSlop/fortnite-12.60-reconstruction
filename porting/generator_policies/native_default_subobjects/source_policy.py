"""Import-evidenced native CDO templates. No inferred attachment or root topology."""
import json
import re
from pathlib import Path


class Policy:
    def __init__(self, path, parents, listed, header_index, blocks=None, names=None, type_map=None):
        data = json.loads(Path(path).read_text())
        if data.get('conflicts') or not data.get('coverage', {}).get('complete'):
            raise ValueError('native default subobject evidence has parse failures or class conflicts')
        raw = data['subobjects']
        by_path = {(r['native_module'], r['native_class'], tuple(r['outer_path']) + (r['subobject_name'],)): r
                   for r in raw}
        effective = {}
        for record in raw:
            r = dict(record)
            if r['outer_path']:
                owner = by_path.get((r['native_module'], r['native_class'], tuple(r['outer_path'])))
                if owner is None:
                    raise ValueError('native nested subobject owner import missing: %s' % r)
                # A nested CDO import belongs to the component/object's constructor,
                # not to the containing actor's constructor (which has already run it).
                r.update(native_module=owner['subobject_module'], native_class=owner['subobject_class'], outer_path=[])
            key = (r['native_module'], r['native_class'], r['subobject_name'])
            if key in effective and any(effective[key][k] != r[k] for k in ('subobject_module', 'subobject_class')):
                raise ValueError('conflicting nested native constructor evidence: %s' % (key,))
            effective[key] = r
        self.records = list(effective.values())
        self.parents = {v['name']: v for v in parents.values() if v.get('name')}
        self.listed = set(listed)
        self.properties = {}
        for block, cpp_name in (names or {}).items():
            props = {}
            for prop in (blocks or {}).get(block, {}).get('properties', []):
                ref = prop.get('type_refs', {}).get('class', '').lower()
                typ = (type_map or {}).get(ref)
                accessible = cpp_name in self.listed or (cpp_name == 'AActor' and prop['name'] == 'RootComponent')
                if accessible and prop.get('kind') == 'Object' and prop.get('array_dim', 1) == 1 and typ:
                    props[prop['name']] = {'decl': '%s* %s;' % (typ, prop['name'])}
            self.properties[cpp_name] = props
        self.headers = header_index
        self.names = {((v.get('package') or '').rsplit('/', 1)[-1], n[1:]): n
                      for n, v in self.parents.items()}

    def resolve(self, module, name):
        cpp = self.names.get((module, name))
        if cpp in self.listed:
            return cpp, cpp + '.h'
        for candidate in ([cpp] if cpp else []) + ['U' + name, 'A' + name]:
            inc = self.headers.find(module, candidate)
            if inc:
                return candidate, inc
        raise ValueError('native default subobject class has no includable header: %s.%s' % (module, name))

    def configure(self, model):
        name = model['name']
        module = self.parents[name]['package'].rsplit('/', 1)[-1]
        records = [r for r in self.records if r['native_module'] == module and r['native_class'] == name[1:]]
        if not records:
            return
        inherited = {}
        for ancestor in self.parents[name].get('chain', [])[1:]:
            for record in self.records:
                if self.names.get((record['native_module'], record['native_class'])) == ancestor:
                    inherited.setdefault(record['subobject_name'], record)
        lines = []
        overrides = []
        includes = set(model.get('cpp_includes', []))
        subobject_includes = set()
        deps = set(model.get('runtime_module_dependencies', []))
        members = {}
        for ancestor in reversed(self.parents[name].get('chain', [])[1:]):
            members.update(self.properties.get(ancestor, {}))
        members.update({m['name']: m for m in model.get('members_decl_order', [])})
        for i, r in enumerate(records):
            if r['outer_path']:
                raise ValueError('%s: nested native subobject requires owning constructor evidence: %s' % (name, r))
            inherited_record = inherited.get(r['subobject_name'])
            if inherited_record and all(inherited_record[k] == r[k] for k in ('subobject_module', 'subobject_class')):
                continue
            cpp, inc = self.resolve(r['subobject_module'], r['subobject_class'])
            includes.add(inc)
            subobject_includes.add(inc)
            if r['subobject_module'] != module:
                deps.add(r['subobject_module'])
            sub = r['subobject_name']
            # UE4.25 Character.cpp creates these before a native subclass body runs.
            # The imported concrete type can override the engine default (notably
            # Fortnite's movement component). Install that type in the initializer.
            character_defaults = {'CharMoveComp': 'UCharacterMovementComponent',
                                  'CollisionCylinder': 'UCapsuleComponent', 'CharacterMesh0': 'USkeletalMeshComponent'}
            engine_default = character_defaults.get(sub) if 'ACharacter' in self.parents[name].get('chain', []) else None
            if sub == 'BrushComponent0' and 'ABrush' in self.parents[name].get('chain', []):
                engine_default = 'UBrushComponent'
            engine_defaults = {
                'ANavLinkProxy': {'PositionComponent': 'USceneComponent', 'SmartLinkComp': 'UNavLinkCustomComponent'},
                'ANavSystemConfigOverride': {'SceneComp': 'USceneComponent'},
                'ANavigationData': {'SceneComp': 'USceneComponent'},
                'AAIController': {'PathFollowingComponent': 'UPathFollowingComponent', 'ActionsComp': 'UPawnActionsComponent'},
                'AController': {'TransformComponent0': 'USceneComponent'},
                'APlayerCameraManager': {'TransformComponent0': 'USceneComponent'},
                'ADefaultPawn': {'CollisionComponent0': 'USphereComponent', 'MovementComponent0': 'UFloatingPawnMovement'},
                'AWaterBody': {'WaterSpline': 'UWaterSplineComponent', 'WaterSplineMetadata': 'UWaterSplineMetadata'},
                'ASpectatorPawn': {'MovementComponent0': 'USpectatorPawnMovement'},
            }
            for ancestor, defaults in engine_defaults.items():
                if ancestor in self.parents[name].get('chain', []) and sub in defaults:
                    engine_default = defaults[sub]
            if inherited_record or engine_default:
                base = self.resolve(inherited_record['subobject_module'], inherited_record['subobject_class'])[0] if inherited_record else engine_default
                if cpp != base:
                    if base not in self.parents.get(cpp, {}).get('chain', []):
                        raise ValueError('%s.%s: incompatible inherited subobject class %s (expected %s)' % (name, sub, cpp, base))
                    overrides.append((cpp, sub))
                # The superclass creates this object with the initializer's class override.
                # Creating it again in this constructor would duplicate its name.
                continue
            sub_cpp = json.dumps(sub, ensure_ascii=True)[1:-1]
            var = 'NativeDefaultSubobject%d' % i
            lines.append('    %s* %s = CreateDefaultSubobject<%s>(TEXT("%s"));' % (cpp, var, cpp, sub_cpp))
            # Original 0x1422fe830 stores these results at +0x27b8/+0x27c0;
            # reflection identifies the corresponding differently named properties.
            aliases = {'AFortPlayerPawn': {'PlayerInWaterFX': 'PSC_PlayerInWater',
                       'PlayerInWaterSurfaceSwimmingFX': 'PSC_PlayerInWaterSurfaceSwimming'}}
            prop = aliases.get(name, {}).get(sub, sub)
            if prop in members and '*' in members[prop]['decl']:
                declaration = members[prop]['decl']
                match = re.fullmatch(r'([AU]\w+)\s*\*\s*' + re.escape(prop) + r'\s*;', declaration)
                if not match or (match[1] != cpp and match[1] not in self.parents.get(cpp, {}).get('chain', [])):
                    raise ValueError('%s.%s: unverified default subobject pointer assignment (%s -> %s)' %
                                     (name, prop, cpp, declaration))
                lines.append('    %s = %s;' % (prop, var))
            else:
                lines.append('    (void)%s;' % var)
        if lines or overrides:
            model['native_default_subobject_lines'] = lines
            model['native_default_subobject_overrides'] = overrides
            model['native_default_subobject_includes'] = sorted(subobject_includes)
            model['cpp_includes'] = sorted(includes)
            model['runtime_module_dependencies'] = sorted(deps)


def finalize(model, header, cpp):
    lines = model.get('native_default_subobject_lines')
    if not lines and not model.get('native_default_subobject_overrides'):
        return header, cpp
    name = model['name']
    # Merge into a pre-existing policy/manifest constructor, including inline policies.
    pattern = re.compile(r'\b(?:' + re.escape(name) + r'::)?' + re.escape(name) + r'\s*\([^;{}]*\)\s*(?::[^{};]*)?\{')
    found = [(which, m) for which, text in [('header', header), ('cpp', cpp)] for m in pattern.finditer(text)]
    body = '\n' + '\n'.join(lines) + '\n'
    initializer = 'ObjectInitializer' + ''.join('.SetDefaultSubobjectClass<%s>(TEXT("%s"))' % item
                                               for item in model.get('native_default_subobject_overrides', []))
    if len(found) > 1:
        raise ValueError(name + ': multiple constructor bodies require explicit merge policy')
    if found:
        which, match = found[0]
        # Append before the matching closing brace, preserving existing constructor defaults first.
        text = header if which == 'header' else cpp
        depth, end = 1, match.end()
        while depth:
            if text[end] == '{': depth += 1
            if text[end] == '}': depth -= 1
            end += 1
        for line in lines:
            name_match = re.search(r'CreateDefaultSubobject<[^>]+>\(TEXT\("([^"]+)"\)\)', line)
            if name_match and ('TEXT("%s")' % name_match[1]) in text[match.start():end]:
                raise ValueError(name + ': existing constructor already refers to subobject ' + name_match[1])
        text = text[:end - 1] + body + text[end - 1:]
        if initializer != 'ObjectInitializer':
            ctor = text[match.start():match.end()]
            if ctor.count('Super(ObjectInitializer)') != 1:
                raise ValueError(name + ': existing constructor requires explicit initializer override merge')
            text = text[:match.start()] + ctor.replace('Super(ObjectInitializer)', 'Super(%s)' % initializer) + text[match.end():]
        if which == 'header':
            marker = '#include "%s.generated.h"' % name
            incs = [inc for inc in model['native_default_subobject_includes']
                    if '#include "%s"' % inc not in text]
            text = text.replace(marker, ''.join('#include "%s"\n' % inc for inc in incs) + marker, 1)
            return text, cpp
        return header, text
    declaration = '\n    %s(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());\n' % name
    if re.search(r'\b' + re.escape(name) + r'\s*\(', header):
        raise ValueError(name + ': constructor declaration without known body')
    header = header.replace('public:', 'public:' + declaration, 1)
    cpp += '\n%s::%s(const FObjectInitializer& ObjectInitializer)\n    : Super(%s)\n{%s}\n' % (name, name, initializer, body)
    return header, cpp
