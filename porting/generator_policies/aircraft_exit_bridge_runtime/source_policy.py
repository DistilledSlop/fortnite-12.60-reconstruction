"""Emit the reflected aircraft-exit compatibility callback, not a recovered body."""
from pathlib import Path
import json
import re

HERE = Path(__file__).resolve().parent
FIXTURE = HERE.parent / 'athena_mode_reconstruction_runtime' / 'original-class.json'
NAME = 'OnAircraftExitedDropZone'


def validate_original_table():
    fixture = json.loads(FIXTURE.read_text())
    # Fixture is a class object; inspect only the named function and require all
    # candidate tables to agree on the object parameter used by this bridge.
    if fixture.get('block_va') != '0x1463cf250':
        raise ValueError('Aircraft exit callback original owner changed')
    functions = fixture.get('functions', [])
    matches = [entry for entry in functions if entry.get('name') == NAME]
    if len(matches) != 1:
        raise ValueError('Aircraft exit callback original table missing or duplicated')
    entry = matches[0]
    if entry.get('flags') != ['0x80401'] or entry.get('create_func') != '0x1428ee500':
        raise ValueError('Aircraft exit callback original function metadata changed')
    candidates = entry.get('candidates') or []
    if len(candidates) != 2:
        raise ValueError('Aircraft exit callback candidate count changed')
    for candidate in candidates:
        params = candidate.get('params') or []
        if candidate.get('flags') != '0x80401' or candidate.get('nprop') != 1 or len(params) != 1:
            raise ValueError('Aircraft exit callback parameter count changed')
        p = params[0]
        if (p.get('name'), p.get('kind'), p.get('offset'), p.get('array_dim'),
                p.get('type_refs', {}).get('class')) != (
                'FortAthenaAircraft', 'Object', 0, 1, '0x1427ccf90'):
            raise ValueError('Aircraft exit callback parameter mapping changed')


def finalize(model, header, cpp):
    if model['name'] != 'AFortGameModeAthena':
        return header, cpp
    validate_original_table()
    if any(f.get('name') == NAME for f in model.get('functions', [])):
        raise ValueError('Aircraft exit callback already exists in model; reconcile bridge')
    if re.search(r'\b' + NAME + r'\s*\(', header) or re.search(
            r'\bAFortGameModeAthena::' + NAME + r'\s*\(', cpp):
        raise ValueError('Aircraft exit callback already declared or defined; reconcile bridge')
    marker = '\tGENERATED_BODY()'
    if header.count(marker) != 1:
        raise ValueError('Aircraft exit callback class marker changed')
    if not re.search(r'\bclass\s+AFortAthenaAircraft\s*;', header):
        raise ValueError('Aircraft exit callback requires aircraft forward declaration')
    declaration = ("\n\npublic:\n"
                   "\t// Compatibility bridge; the original callback body is not recovered.\n"
                   "\tUFUNCTION()\n"
                   "\tvoid OnAircraftExitedDropZone(AFortAthenaAircraft* FortAthenaAircraft);")
    header = header.replace(marker, marker + declaration, 1)
    include = '#include "AFortGameModeAthena.h"'
    if cpp.count(include) != 1 or '#include "FortAthenaMatchFlow.h"' in cpp:
        raise ValueError('Aircraft exit callback source include layout changed')
    cpp = cpp.replace(include, include + '\n#include "FortAthenaMatchFlow.h"', 1)
    cpp += ('\n// Compatibility bridge to the existing match-flow implementation.\n'
            'void AFortGameModeAthena::OnAircraftExitedDropZone(AFortAthenaAircraft* FortAthenaAircraft)\n'
            '{\n'
            '    FortAthenaMatchFlow::OnAircraftExitedDropZone(this, FortAthenaAircraft);\n'
            '}\n')
    return header, cpp
