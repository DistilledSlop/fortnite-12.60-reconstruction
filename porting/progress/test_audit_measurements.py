#!/usr/bin/env python3
"""Regression tests for independent audit measurement scopes."""
import importlib.util
import json
from pathlib import Path
import tempfile
import unittest

HERE = Path(__file__).resolve().parent
REPO = HERE.parent.parent
spec = importlib.util.spec_from_file_location('progress', HERE / 'progress.py')
progress = importlib.util.module_from_spec(spec)
spec.loader.exec_module(progress)


def leaves():
    found = {}
    def visit(value):
        if isinstance(value, dict):
            if 'id' in value and 'measure' in value:
                found[value['id']] = value
            for item in value.values():
                visit(item)
        elif isinstance(value, list):
            for item in value:
                visit(item)
    visit(json.loads((HERE / 'components.json').read_text()))
    return found


class LocalContext:
    def __init__(self, root, totals):
        self.root, self.totals, self.used_local = root, totals, False
    def local_path(self, path):
        return str(self.root / path)
    def total_value(self, spec):
        return self.totals[spec['total_key']], ''


class AuditMeasurementTests(unittest.TestCase):
    def test_wine_checks_do_not_credit_original_join(self):
        items = leaves()
        checks = {}
        for leaf in items.values():
            measure = leaf['measure']
            if measure['kind'] == 'ledger':
                for check in measure['checks']:
                    self.assertNotIn(check, checks, (check, checks.get(check), leaf['id']))
                    checks[check] = leaf['id']
        original = items['net.original_join']['measure']['checks']
        self.assertEqual(original, ['original_client_hello', 'original_client_login',
                                    'original_client_welcome', 'original_client_join'])
        self.assertTrue(all(checks[c] == 'platform.wine_launch' for c in
                            ['wine_entitlement_passed', 'wine_lobby_reached']))

    def test_non_mesh_numerator_matches_denominator_units_and_excludes_meshes(self):
        items = leaves()
        package = items['assets.package_coverage']['measure']
        totals = json.loads((HERE / 'totals.json').read_text())
        counts = totals['cooked_name_prefix_counts']['value']
        expected = totals['cooked_uasset_distinct_names']['value'] - counts['static_mesh'] - counts['skeletal_mesh']
        self.assertEqual(totals[package['total_key']]['value'], expected)
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            for relative in ['porting/generated/extracted/a/SM_A.uasset',
                             'porting/generated/extracted/a/SK_B.uasset',
                             'porting/generated/extracted/a/T_C.uasset',
                             'porting/native/Content/b/SM_A.uasset',
                             'porting/native/Content/b/SK_B.uasset',
                             'porting/native/Content/b/T_C.uasset',
                             'porting/native/Content/b/M_D.uasset']:
                path = root / relative
                path.parent.mkdir(parents=True, exist_ok=True)
                path.touch()
            ctx = LocalContext(root, {package['total_key']: 2,
                                      'cooked_name_prefix_counts': 2,
                                      'cooked_mesh_exportable': 2})
            result = progress.m_local_glob(package, ctx)
            self.assertEqual((result['done'], result['total']), (2, 2))
            self.assertEqual(progress.m_local_glob(items['assets.static_meshes']['measure'], ctx)['done'], 1)
            self.assertEqual(progress.m_local_glob(items['assets.skeletal_anim']['measure'], ctx)['done'], 1)

    def test_voice_routes_have_one_owner(self):
        items = leaves()
        routes_path = REPO / 'porting/generated/routes.json'
        inventory = (json.loads(routes_path.read_text())['routes'] if routes_path.exists() else
                     ['/api/game/v2/voice/{accountId}/createJoinToken',
                      '/api/game/v2/voice/{accountId}/createLoginToken'])
        def selected(measure, route):
            return (any(part in route for part in measure.get('include', [])) and
                    not any(part in route for part in measure.get('exclude', [])))
        game = items['backend.game_profile']['measure']
        voice = items['audio.voice']['measure']
        voice_routes = [route for route in inventory if selected(voice, route)]
        self.assertTrue(voice_routes)
        self.assertTrue(all(not selected(game, route) for route in voice_routes))


if __name__ == '__main__':
    unittest.main()
