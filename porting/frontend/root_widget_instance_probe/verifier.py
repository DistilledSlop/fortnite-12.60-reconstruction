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
STARTUP_CLASSES = {
    'game_instance_class': '/Script/FortniteGame.FortGameInstance',
    'player_class': '/Script/FortniteGame.FortLocalPlayer',
    'viewport_class': '/Script/FortniteGame.FortGameViewportClient',
}
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
        fields = {}
        if not row.startswith('ROOTINSTANCE ') or line.count('ROOTINSTANCE') != 1:
            problem = problem or 'malformed diagnostic row'
            continue
        payload = row[len('ROOTINSTANCE '):]
        matches = list(re.finditer(r'(?:^| )([a-z_]+)=', payload))
        if not matches or matches[0].start() != 0:
            problem = problem or 'malformed diagnostic field'
        for index, match in enumerate(matches):
            key = match.group(1)
            end = matches[index + 1].start() if index + 1 < len(matches) else len(payload)
            value = payload[match.end():end].strip()
            if not key or not value or key in fields:
                problem = problem or 'malformed or duplicate diagnostic field'
            fields[key] = value
        last = fields.get('phase', last)
        rows.append(fields)
    # Preserve observations even if another row, an error log, or ownership fails.
    prereq_rows = [r for r in rows if r.get('phase') == 'prerequisites']
    created_rows = [r for r in rows if r.get('phase') == 'after_create_widget']
    prereq_observed = prereq_rows[0] if len(prereq_rows) == 1 else {}
    created_observed = created_rows[0] if len(created_rows) == 1 else {}
    class_keys = (*STARTUP_CLASSES, 'controller_class')
    class_observations = {key: prereq_observed.get(key) for key in class_keys}
    startup_classes_match = None
    if all(class_observations.values()):
        startup_classes_match = (all(class_observations[k] == v for k, v in STARTUP_CLASSES.items())
                                 and class_observations['controller_class'] != 'None')
    count_text = created_observed.get('widget_count')
    try:
        widget_count = int(count_text) if count_text and re.fullmatch(r'[0-9]+', count_text) else None
    except ValueError:  # Includes integers exceeding Python's conversion limit.
        widget_count = None
    root_widget = created_observed.get('root_widget')
    tree_populated = (widget_count > 0 and bool(root_widget) and root_widget != 'None'
                      if widget_count is not None and root_widget is not None else None)
    result = {'creation_completed': False, 'last_phase': last, 'rows': len(rows),
              'blueprint_errors': errors, 'blueprint_error_count': error_count,
              'dependency_errors': dependencies, 'dependency_error_count': dependency_count,
              'startup_class_observations': class_observations,
              'startup_classes_match': startup_classes_match, 'root_widget': root_widget,
              'widget_count': widget_count, 'tree_populated': tree_populated, 'scope': SCOPE}
    try:
        if problem:
            raise ValueError(problem)
        if len(rows) != len(PHASES):
            raise ValueError('expected eight diagnostic rows, got %d' % len(rows))
        version = rows[0].get('v')
        if version not in ('1', '2'):
            raise ValueError('unsupported diagnostic version')
        for phase, row in zip(PHASES, rows):
            if row.get('v') != version or row.get('phase') != phase:
                raise ValueError('wrong version, order, or phase at ' + phase)
        def exact(i, expected):
            if rows[i] != {'v': version, 'phase': PHASES[i], **expected}:
                raise ValueError('invalid fields at ' + PHASES[i])
        exact(0, {'timeout_seconds': '180'})
        prereq = rows[1]
        prereq_keys = ('world', 'game_instance', 'player', 'controller')
        if version == '2':
            prereq_keys += class_keys
        exact(1, {key: prereq.get(key) for key in prereq_keys})
        if any(not prereq.get(k) or prereq[k] == 'None' for k in ('world', 'game_instance', 'player', 'controller')):
            raise ValueError('null prerequisite')
        exact(2, {'path': ROOT})
        exact(3, {'class': ROOT, 'ok': '1'})
        exact(4, {})
        created = rows[5]
        created_keys = ('widget', 'class', 'outer', 'world', 'player', 'controller', 'tree')
        if version == '2':
            created_keys += ('root_widget', 'widget_count')
        exact(5, {key: created.get(key) for key in created_keys})
        if any(not created.get(k) or created[k] == 'None' for k in ('widget', 'class', 'outer', 'world', 'player', 'controller', 'tree')):
            raise ValueError('null creation observation')
        if created['class'] != ROOT or any(created[a] != prereq[b] for a, b in (('outer', 'game_instance'), ('world', 'world'), ('player', 'player'), ('controller', 'controller'))):
            raise ValueError('creation ownership mismatch')
        if version == '2' and (widget_count is None or not root_widget):
            raise ValueError('invalid tree observations')
        exact(6, {'ok': '1', 'scope': 'create_return_and_ownership'})
        exact(7, {'ok': '1'})
        creation_completed = True
        result['creation_completed'] = True
        if version == '2' and not startup_classes_match:
            raise ValueError('startup class mismatch or null class')
        if version == '2' and not tree_populated:
            raise ValueError('widget tree is empty')
        if errors:
            raise ValueError('Blueprint initialization errors detected')
        if dependencies:
            raise ValueError('Unresolved native imports or data-table row structures')
    except ValueError as exc:
        result.update(passed=False, error=str(exc))
        return result
    result['passed'] = True
    return result


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
