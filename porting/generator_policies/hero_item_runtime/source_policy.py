"""Admit complete original persistable-definition and hero-level queries."""
import json
import re
from pathlib import Path
HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[2]
ENTRIES = json.loads((HERE / 'manifest-entries.json').read_text())
PARENTS = {'UFortHero': 'UFortWorker', 'UFortPersistableItem': 'UFortItem',
           'UFortHeroType': 'UFortWorkerType'}


def apply(model):
    name = model.get('name')
    if name not in PARENTS:
        return model
    if model.get('package') != '/Script/FortniteGame' or model.get('parent') != PARENTS[name]:
        raise ValueError(name + ': hero item ancestry changed')
    for entry in (e for e in ENTRIES if e['class'] == name):
        fn = entry['function']
        helpers = [h for h in model.get('helpers', []) if h.get('name') == fn]
        competing = [h for group in ('functions','static_helpers','fields','members_decl_order')
                     for h in model.get(group, []) if h.get('name') == fn]
        if len(helpers) != 1 or competing:
            raise ValueError(name + ': missing/competing ' + fn + '; rejected=' + repr(model.get('splice', {}).get('rejected', [])))
        h = helpers[0]
        factory = fn == 'NativeCreateTemporaryHero142017d90'
        if (h.get('args') != (['int32 InQuantity'] if factory else []) or h.get('const') is not (not factory) or
                h.get('virtual') != entry['virtual'] or h.get('override') != entry['override'] or
                h.get('body','').strip() != (ROOT / entry['inc_path']).read_text().strip() or
                model.get('splice', {}).get('accepted', []).count(name + '::' + fn) != 1):
            raise ValueError(name + ': hero item contract changed for ' + fn)
    includes = {'UFortHero': ['UFortHeroType.h'],
                'UFortPersistableItem': ['UFortPersistableItemDefinition.h'],
                'UFortHeroType': ['UFortHero.h', 'Misc/CommandLine.h', 'Misc/Parse.h',
                                  'Misc/Guid.h', 'UObject/Package.h', 'UObject/UObjectGlobals.h']}[name]
    for include in includes:
        if include not in model.setdefault('cpp_includes', []):
            model['cpp_includes'].append(include)
    if name == 'UFortHero':
        fields = [f for f in model.get('fields', []) if f.get('name') == 'NativeRecoveredInt32_0x228']
        if len(fields) != 1 or fields[0].get('type') != 'int32':
            raise ValueError(name + ': original level clamp field changed: ' + repr(fields))
    return model


def finalize(model, header, cpp):
    if model.get('name') != 'UFortHero':
        return header, cpp
    apply(model)
    name = model['name']
    declarations = list(re.finditer(r'^\s*(?:explicit\s+)?UFortHero\s*\(', header, re.M))
    definitions = list(re.finditer(r'\bUFortHero::UFortHero\s*\(', cpp))
    if len(declarations) > 1 or len(definitions) > 1 or bool(declarations) != bool(definitions):
        raise ValueError(name + ': mismatched constructors')
    if not definitions:
        if header.count('public:') != 1:
            raise ValueError(name + ': expected one public section')
        header = header.replace('public:', 'public:\n\tUFortHero(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());\n', 1)
        cpp += '\nUFortHero::UFortHero(const FObjectInitializer& ObjectInitializer)\n    : Super(ObjectInitializer)\n{\n}\n'
    matches = list(re.finditer(r'\bUFortHero::UFortHero\s*\(\s*const\s+FObjectInitializer\s*&\s*ObjectInitializer\s*\)\s*:[^{};]*\{', cpp))
    if len(matches) != 1:
        raise ValueError(name + ': unsupported constructor')
    start = end = matches[0].end()
    depth = 1
    while depth and end < len(cpp):
        depth += (cpp[end] == '{') - (cpp[end] == '}')
        end += 1
    if depth or 'NativeRecoveredInt32_0x228' in cpp[start:end-1]:
        raise ValueError(name + ': conflicting level clamp constructor')
    statement = (ROOT / 'porting/behavior/athena_hero_setup85_runtime/HeroLevelClampCtorDefault.inc').read_text()
    return header, cpp[:end-1] + statement + cpp[end-1:]
