#!/usr/bin/env python3
"""Verify the bounded Root widget creation transcript and report Blueprint errors."""
import argparse
import json
import re
from pathlib import Path

ROOT = '/Game/UI/Foundation/RootLayout_Athena.RootLayout_Athena_C'
PHASES = ('registered', 'prerequisites', 'before_load', 'after_load',
          'before_create_widget', 'after_create_widget', 'observations', 'complete')
ERROR = re.compile(r'Accessed None|Blueprint Runtime Error|(?:missing|unable to (?:find|resolve)|failed to (?:find|resolve)|could not find|unresolved).{0,100}(?:bytecode|referenced field)|(?:bytecode|referenced field).{0,100}(?:missing|unable to (?:find|resolve)|failed to (?:find|resolve)|could not find|unresolved)', re.I)
DEPENDENCY_ERROR = re.compile(r'Skipping import .+depends on missing native class|Missing RowStruct', re.I)
SCOPE = 'CreateWidget return and ownership observations; not full Blueprint VM, interactive UI, or parity'


def verify(text):
    rows = []
    errors = []
    error_count = 0
    dependencies = []
    dependency_count = 0
    creation_completed = False
    last = None
    problem = None
    for line in text.splitlines():
        if ERROR.search(line):
            error_count += 1
            if len(errors) < 20:
                errors.append(line.strip()[:500])
        if DEPENDENCY_ERROR.search(line):
            dependency_count += 1
            if len(dependencies) < 20:
                dependencies.append(line.strip()[:500])
        if 'ROOTINSTANCE' not in line:
            continue
        row = line[line.index('ROOTINSTANCE'):].strip()
        tokens = row.split()
        fields = {}
        if tokens[0] != 'ROOTINSTANCE' or line.count('ROOTINSTANCE') != 1:
            problem = problem or 'malformed diagnostic row'
            continue
        for token in tokens[1:]:
            if '=' not in token:
                problem = problem or 'malformed diagnostic field'
                continue
            key, value = token.split('=', 1)
            if not key or not value or key in fields:
                problem = problem or 'malformed or duplicate diagnostic field'
            fields[key] = value
        last = fields.get('phase', last)
        rows.append(fields)
    try:
        if problem:
            raise ValueError(problem)
        if len(rows) != len(PHASES):
            raise ValueError('expected eight diagnostic rows, got %d' % len(rows))
        for phase, row in zip(PHASES, rows):
            if row.get('v') != '1' or row.get('phase') != phase:
                raise ValueError('wrong version, order, or phase at ' + phase)
        def exact(i, expected):
            if rows[i] != {'v': '1', 'phase': PHASES[i], **expected}:
                raise ValueError('invalid fields at ' + PHASES[i])
        exact(0, {'timeout_seconds': '180'})
        prereq = rows[1]
        exact(1, {key: prereq.get(key) for key in ('world', 'game_instance', 'player', 'controller')})
        if any(not prereq.get(k) or prereq[k] == 'None' for k in ('world', 'game_instance', 'player', 'controller')):
            raise ValueError('null prerequisite')
        exact(2, {'path': ROOT})
        exact(3, {'class': ROOT, 'ok': '1'})
        exact(4, {})
        created = rows[5]
        exact(5, {key: created.get(key) for key in ('widget', 'class', 'outer', 'world', 'player', 'controller', 'tree')})
        if any(not created.get(k) or created[k] == 'None' for k in ('widget', 'class', 'outer', 'world', 'player', 'controller', 'tree')):
            raise ValueError('null creation observation')
        if created['class'] != ROOT or any(created[a] != prereq[b] for a, b in (('outer', 'game_instance'), ('world', 'world'), ('player', 'player'), ('controller', 'controller'))):
            raise ValueError('creation ownership mismatch')
        exact(6, {'ok': '1', 'scope': 'create_return_and_ownership'})
        exact(7, {'ok': '1'})
        creation_completed = True
        if errors:
            raise ValueError('Blueprint initialization errors detected')
        if dependencies:
            raise ValueError('Unresolved native imports or data-table row structures')
    except ValueError as exc:
        return {'passed': False, 'error': str(exc), 'creation_completed': creation_completed, 'last_phase': last, 'rows': len(rows), 'blueprint_errors': errors, 'blueprint_error_count': error_count, 'dependency_errors': dependencies, 'dependency_error_count': dependency_count, 'scope': SCOPE}
    return {'passed': True, 'creation_completed': creation_completed, 'last_phase': last, 'rows': len(rows), 'blueprint_errors': errors, 'blueprint_error_count': error_count, 'dependency_errors': dependencies, 'dependency_error_count': dependency_count, 'scope': SCOPE}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('log', type=Path)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    result = verify(args.log.read_text(errors='replace'))
    args.output.write_text(json.dumps(result, indent=2) + '\n')
    raise SystemExit(0 if result['passed'] else 1)


if __name__ == '__main__':
    main()
