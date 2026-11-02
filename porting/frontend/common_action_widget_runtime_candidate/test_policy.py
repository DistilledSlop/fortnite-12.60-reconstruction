import copy
import json
import unittest
from pathlib import Path
import policy

ROOT = Path(__file__).resolve().parents[3]
REF = ROOT / 'porting/generated/reflection'
IMAGE = Path('/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin')

class CandidateGuards(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        classes = json.loads((REF / 'fortnite_game_reflection.json').read_text())['classes_in_groups_with_fort_names']
        cls.subsystem = next(c for c in classes if c['block_va'] == '0x145a0e0d0')
        cls.widget = next(c for c in classes if c['block_va'] == '0x145a01470')
        cls.execs = json.loads((REF / 'native_funcs.json').read_text())['UCommonActionWidget']

    def test_bounded_source(self):
        policy.validate_source(IMAGE)

    def test_gamepad_field_and_collision_guards(self):
        model = {'name':'UCommonInputSubsystem', 'package':'/Script/CommonInput', 'parent':'ULocalPlayerSubsystem',
                 'members_decl_order':[{'name':'GamepadInputType', 'decl':'ECommonGamepadType GamepadInputType;'}]}
        result = policy.apply_gamepad(copy.deepcopy(model), self.subsystem, ['0x14081e010'])
        self.assertIn('return GamepadInputType;', policy.gamepad_definitions(result))
        bad = copy.deepcopy(self.subsystem)
        next(p for p in bad['properties'] if p['name'] == 'GamepadInputType')['offset'] += 1
        with self.assertRaises(ValueError): policy.apply_gamepad(copy.deepcopy(model), bad, ['0x14081e010'])
        with self.assertRaises(ValueError): policy.apply_gamepad(result, self.subsystem, ['0x14081e010'])

    def test_widget_requires_lifecycle_and_original_exec(self):
        model = {'name':'UCommonActionWidget', 'package':'/Script/CommonUI', 'parent':'UWidget',
                 'members_decl_order':[{'name':'InputActions', 'decl':'TArray<FDataTableRowHandle> InputActions;'}], 'functions':[]}
        with self.assertRaisesRegex(ValueError, 'lifecycle'):
            policy.apply_icon(copy.deepcopy(model), self.widget, ['0x140807790'], self.execs)
        model['common_action_binding_key_lifecycle_verified'] = True
        result = policy.apply_icon(copy.deepcopy(model), self.widget, ['0x140807790'], self.execs)
        self.assertEqual(result['functions'][0]['name'], 'GetIcon')
        bad = copy.deepcopy(self.execs)
        bad['GetIcon']['exec_addr'] = '0x0'
        with self.assertRaises(ValueError): policy.apply_icon(copy.deepcopy(model), self.widget, ['0x140807790'], bad)

if __name__ == '__main__': unittest.main()
