import json
from pathlib import Path
import subprocess
import sys

SCRIPT = Path(__file__).resolve().parents[2] / 'server_scope/runtime_report.py'


def record(**changes):
    value = dict(schema_version=1, event='rpc_send', outcome='queued', utc_unix_ms=12,
                 world_time=1.5, connection=7, object_id=8, object_class='/Script/FortniteGame.FortPawn',
                 declaring_class='/Script/FortniteGame.FortPawn', function='Test', actor_id=None,
                 channel=1, payload_bits=10, reliable=True)
    value.update(changes)
    return value


def declaration(owner='AFortPawn', kind='rpc'):
    return dict(kind=kind, owner_key=owner, name='Test', declaration_count=1,
                variants=[dict(declared_owner=owner, owner_candidates=[owner],
                               parent_metadata=dict(name=owner, package='/Script/FortniteGame'))])


def run(tmp_path, records, declarations):
    inventory, trace, out = tmp_path / 'inventory', tmp_path / 'trace', tmp_path / 'out'
    inventory.write_text(''.join(json.dumps(x) + '\n' for x in declarations))
    trace.write_text(''.join(x if isinstance(x, str) else json.dumps(x) + '\n' for x in records))
    result = subprocess.run([sys.executable, str(SCRIPT), '--inventory', str(inventory), '--output', str(out), str(trace)], capture_output=True, text=True)
    return result, json.loads((out / 'report.json').read_text())


def test_stages_and_unobserved_declarations(tmp_path):
    result, report = run(tmp_path, [record(), record(event='rpc_receive', outcome='invoked', utc_unix_ms=20), record(event='rpc_receive', outcome='validation_failed', utc_unix_ms=21)], [declaration(), declaration('UOther'), declaration('UOther', 'replicated_field')])
    assert result.returncode == 0
    assert {x['outcome'] for x in report['observations']} == {'queued', 'invoked', 'validation_failed'}
    assert all(x['receiver_observed'] == x['behavior_parity'] == 'unknown' for x in report['observations'])
    assert [x['status'] for x in report['declarations']] == ['observed_in_provided_traces', 'not_observed_in_provided_traces', 'not_covered_by_rpc_trace']
    assert report['observations'][0]['evidence'][0]['sha256'] == report['trace_sources'][0]['sha256']


def test_ambiguous_and_unresolved_function(tmp_path):
    result, report = run(tmp_path, [record(), record(event='rpc_receive', outcome='invalid_function', declaring_class=None, reliable=None)], [declaration(), declaration('UFortPawn')])
    assert result.returncode == 0
    assert {x['join_status'] for x in report['observations']} == {'ambiguous', 'unmatched'}
    assert all(x['status'] == 'not_observed_in_provided_traces' for x in report['declarations'])


def test_bad_lines_are_reported_and_fail(tmp_path):
    result, report = run(tmp_path, ['{bad}\n', record(schema_version=2), record()], [declaration()])
    assert result.returncode != 0
    assert report['valid_observation_count'] == 1
    assert [e['line'] for e in report['errors']] == [1, 2]
    assert report['trace_status'] == 'invalid_records'


def test_empty_trace_is_explicit(tmp_path):
    result, report = run(tmp_path, [], [declaration()])
    assert result.returncode == 0
    assert report['trace_status'] == 'no_observations'
    assert report['declarations'][0]['status'] == 'not_observed_in_provided_traces'


def test_blueprint_class_paths(tmp_path):
    blueprint = '/Game/Athena/PlayerPawn_Athena.PlayerPawn_Athena_C'
    result, report = run(tmp_path, [record(object_class=blueprint),
                                    record(object_class=blueprint, declaring_class=blueprint)],
                         [declaration()])
    assert result.returncode == 0
    assert report['valid_observation_count'] == 2
    assert {x['declaring_class']: x['join_status'] for x in report['observations']} == {
        '/Script/FortniteGame.FortPawn': 'matched', blueprint: 'unmatched'}


# Run without third-party test dependencies.
import tempfile
import unittest


class RuntimeReportTests(unittest.TestCase):
    def test_contracts(self):
        for check in (test_stages_and_unobserved_declarations, test_ambiguous_and_unresolved_function,
                      test_bad_lines_are_reported_and_fail, test_empty_trace_is_explicit,
                      test_blueprint_class_paths):
            with self.subTest(check=check.__name__), tempfile.TemporaryDirectory() as directory:
                check(Path(directory))


if __name__ == '__main__':
    unittest.main()
