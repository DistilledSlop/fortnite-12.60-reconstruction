"""Require the complete recovered remote view producer lifecycle, then emit SIMD support."""
import re
from pathlib import Path

HERE = Path(__file__).resolve().parent
EXPECTED = {
    'AFortPlayerPawn': {
        'NativeUpdateAccelerationPack1423442e0': ('void', [], False, 'protected', False),
        'NativeUpdateRemoteViewData142344420': ('void', [], False, 'protected', False),
        'NativeHasSlipperyTags14232caf0': ('bool', [], True, 'protected', False),
        'PreReplication': ('void', ['IRepChangedPropertyTracker& ChangedPropertyTracker'], False, 'public', True),
        'PreReplicationForReplay': ('void', ['IRepChangedPropertyTracker& ChangedPropertyTracker'], False, 'public', True),
    },
    'UFortAbilitySystemComponent': {
        'NativeRepAnimMontageInfo1406ccd10': ('const FGameplayAbilityRepAnimMontage&', [], True, 'public', False),
    },
}
PARENTS = {'AFortPlayerPawn': 'AFortPawn', 'UFortAbilitySystemComponent': 'UAbilitySystemComponent'}
SIMD_SUPPORT = '''// remote_view_replication_runtime: original x64 SSE rounding/conversion contract.
#if !defined(__x86_64__) && !defined(_M_X64)
#error "Recovered remote view replication requires x86-64 SSE semantics"
#endif
#if defined(__FAST_MATH__)
#error "Recovered remote view replication requires precise floating-point semantics"
#endif
#include <xmmintrin.h>
#include <emmintrin.h>
#if defined(__clang__)
#pragma clang fp contract(off)
#elif defined(__GNUC__)
#pragma GCC optimize ("fp-contract=off")
#endif
'''


def validate_required_classes(class_names):
    missing = set(EXPECTED) - set(class_names)
    if missing:
        raise ValueError("remote view producer requires generated classes: " + ", ".join(sorted(missing)))


def validate_model(model):
    """Call for every generated model; either target rejects any incomplete local bundle."""
    owner = model['name']
    expected = EXPECTED.get(owner)
    if expected is None:
        return
    if model.get('parent') != PARENTS[owner]:
        raise ValueError(owner + ': remote view producer ancestry changed')
    splice = model.get('splice', {})
    for name, contract in expected.items():
        tag = owner + '::' + name
        accepted = splice.get('accepted', [])
        helpers = [h for h in model.get('helpers', []) if h.get('name') == name]
        rejected = [r for r in splice.get('rejected', []) if r.get('entry') == tag]
        competitors = [f for key in ('functions', 'static_helpers') for f in model.get(key, [])
                       if f.get('name') == name]
        if len(helpers) != 1 or accepted.count(tag) != 1 or rejected or competitors:
            raise ValueError(tag + ': incomplete or competing remote view producer bundle: ' + repr(rejected))
        helper = helpers[0]
        ret, args, const, access, override = contract
        if ((helper.get('ret'), helper.get('args'), helper.get('const'), helper.get('access'))
                != (ret, args, const, access)
                or helper.get('kind') != 'helper' or helper.get('static')
                or helper.get('virtual') is not override or helper.get('override') is not override
                or helper.get('body', '').strip() != (HERE / (name + '.inc')).read_text().strip()):
            raise ValueError(tag + ': remote view producer signature/body contract changed')


def _compact(text):
    return re.sub(r'\s+', '', text)


def finalize(model, header, cpp):
    validate_model(model)
    owner = model['name']
    if owner not in EXPECTED:
        return header, cpp
    for name in EXPECTED[owner]:
        if len(re.findall(r'\b' + re.escape(name) + r'\s*\(', header)) != 1:
            raise ValueError(owner + ': missing or duplicate remote view declaration: ' + name)
        definitions = list(re.finditer(r'\b' + re.escape(owner + '::' + name) + r'\s*\([^;{}]*\)\s*(?:const\s*)?\{', cpp))
        if len(definitions) != 1:
            raise ValueError(owner + ': missing or duplicate remote view definition: ' + name)
        match = definitions[0]
        depth, end = 1, match.end()
        while depth and end < len(cpp):
            if cpp[end] == '{':
                depth += 1
            elif cpp[end] == '}':
                depth -= 1
            end += 1
        if depth or _compact(cpp[match.end():end - 1]) != _compact((HERE / (name + '.inc')).read_text()):
            raise ValueError(owner + ': emitted remote view body differs: ' + name)
    if owner == 'AFortPlayerPawn':
        if re.search(r'^\s*#\s*include\s*[<"](?:xmmintrin|emmintrin)\.h[>"]', header + cpp, re.MULTILINE):
            raise ValueError(owner + ': competing SIMD support include')
        # Put the architecture guard before the system headers; no engine include-gate bypass.
        anchor = '#include \"AFortPlayerPawn.h\"\n'
        if cpp.count(anchor) != 1:
            raise ValueError(owner + ': cannot locate unique own-header include')
        cpp = cpp.replace(anchor, anchor + '\n' + SIMD_SUPPORT + '\n', 1)
    return header, cpp
