"""Validated source proposals; deliberately installs no Tick/lifecycle hook."""
from pathlib import Path
import hashlib
import json
import re

HERE = Path(__file__).resolve().parent


def validate_original_image(image=None):
    contract = json.loads((HERE / 'evidence-contract.json').read_text())
    with open(image or contract['image'], 'rb') as stream:
        for item in contract['ranges']:
            stream.seek(int(item['address'], 16) - int(contract['image_base'], 16))
            if hashlib.sha256(stream.read(item['size'])).hexdigest() != item['sha256']:
                raise ValueError('Aircraft original body changed: ' + item['label'])


def validate_reflection(reflection, bool_bits):
    classes = reflection['classes_in_groups_with_fort_names']
    owner = next(c for c in classes if c['block_va'] == '0x1462e4f90')
    fields = {p['name']: p for p in owner['properties']}
    expected = {'ReplicatedFlightTimestamp': 0x2e4,
                'FlightElapsedTime': 0x2e8, 'ClientFlightTimerDrift': 0x2ec,
                'FlightStartTime': 0x2d0, 'FlightEndTime': 0x2d4,
                'DropStartTime': 0x2d8, 'DropEndTime': 0x2dc}
    bits = next(p for p in bool_bits['by_class']['AFortAthenaAircraft']
                if p['name'] == 'bIsOutOfPhaseAircraft')
    if (bits['offset'], bits['mask'], bits['form']) != (0x2e0, 1, 'mov8'):
        raise ValueError('Aircraft original out-of-phase byte mapping changed')
    for name, offset in expected.items():
        if fields[name]['offset'] != offset:
            raise ValueError('Aircraft original field mapping changed: ' + name)


def proposals():
    """Owner must explicitly integrate with the complete server driver contract.

    In particular NativeAircraftExitedDropZone must call the existing complete
    compatibility callback, and the old movement/event driver must be removed.
    This function does not claim recovery of the full native Tick or constructors.
    """
    validate_original_image()
    return {name: (HERE / name).read_text() for name in
            ('NativeTickAuthorityFlight.inc', 'ConstructorTiming.inc', 'BaseConstructorFlags.inc')}


def finalize(model, header, cpp):
    """Require the manifest helper and restore defaults in the existing ctor.

    Run after native_default_subobjects_policy.finalize. No Tick override or
    tick-flag change is emitted; the server driver owns the single call site.
    """
    if model['name'] != 'AFortAthenaAircraft':
        return header, cpp
    if model.get('parent') != 'AFortAircraft':
        raise ValueError('Aircraft authority helper parent changed')
    validate_original_image()
    name = 'NativeTickAuthorityFlight'
    tag = model['name'] + '::' + name
    helpers = [h for h in model.get('helpers', []) if h.get('name') == name]
    if len(helpers) != 1 or model.get('splice', {}).get('accepted', []).count(tag) != 1:
        raise ValueError(tag + ': require exactly one accepted authority helper')
    h = helpers[0]
    if ((h.get('ret'), h.get('args'), h.get('access'), h.get('kind')) !=
            ('void', ['AFortGameStateAthena* GameState'], 'public', 'helper')
            or h.get('const') or h.get('static') or h.get('virtual') or h.get('override')
            or h.get('body', '').strip() != (HERE / (name + '.inc')).read_text().strip()):
        raise ValueError(tag + ': authority subpath contract changed')
    if len(re.findall(r'\b' + name + r'\s*\(', header)) != 1:
        raise ValueError(tag + ': missing or competing helper declaration')
    if len(re.findall(r'\bAFortAthenaAircraft::' + name + r'\s*\(', cpp)) != 1:
        raise ValueError(tag + ': missing or competing helper definition')
    if re.search(r'\bAFortAthenaAircraft::Tick\s*\(', cpp):
        raise ValueError('Aircraft full Tick now exists: reconcile authority ownership explicitly')
    # Require the prior generator-owned ctor and preserve its complete body.
    matches = list(re.finditer(r'\bAFortAthenaAircraft::AFortAthenaAircraft\s*\([^;{}]*\)\s*:\s*Super\(ObjectInitializer\)\s*\{', cpp))
    if len(matches) != 1:
        raise ValueError('Aircraft constructor requires prior native default-subobject policy')
    begin = matches[0].end()
    depth, end = 1, begin
    while depth and end < len(cpp):
        depth += (cpp[end] == '{') - (cpp[end] == '}')
        end += 1
    if depth:
        raise ValueError('Aircraft constructor is unbalanced')
    existing = cpp[begin:end - 1]
    fragment = (HERE / 'ConstructorTiming.inc').read_text()
    names = re.findall(r'^([A-Za-z_][\w.]*)\s*=', fragment, re.MULTILINE)
    for field in names:
        if re.search(r'\b' + re.escape(field) + r'\s*=', existing):
            raise ValueError('Aircraft constructor has competing initialization: ' + field)
    if 'CreateDefaultSubobject<USceneComponent>' not in existing:
        raise ValueError('Aircraft root subobject creation missing; preserve the prior policy')
    cpp = cpp[:end - 1] + '\n' + fragment + cpp[end - 1:]
    return header, cpp
