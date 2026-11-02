import copy
import importlib.util
import json
import unittest
from pathlib import Path

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[2]
spec = importlib.util.spec_from_file_location('double_click_policy', HERE / 'policy.py')
p = importlib.util.module_from_spec(spec)
spec.loader.exec_module(p)
C = p.CONTRACT


def model():
    return {'name': 'UCommonButton', 'package': '/Script/CommonUI',
            'block_va': C['identity']['block_va'], 'functions': [],
            'members_decl_order': [{'name': 'OnButtonDoubleClicked', 'decl': C['member']},
                                   {'name': 'bStopDoubleClickPropagation', 'decl': C['bit_member']}],
            'splice': {'accepted': ['UCommonButton::StopDoubleClickPropagation']}}


class PolicyTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        directory = ROOT / 'porting/generated/reflection'
        cls.block = next(b for b in json.loads((directory / 'fortnite_game_reflection.json').read_text())['classes_in_groups_with_fort_names'] if b['block_va'] == C['identity']['block_va'])
        cls.names = {v: k for k, v in json.loads((directory / 'class_name_map.json').read_text()).items()}
        cls.delegates = json.loads((directory / 'delegate_blocks.json').read_text())['delegates']
        cls.execs = json.loads((directory / 'native_funcs.json').read_text())['UCommonButton']

    def test_original_records_and_rejection(self):
        va = C['identity']['block_va']
        p.validate_classes({va: self.block}, self.names, self.delegates)
        for group, key in [('properties', 'property'), ('properties', 'bit'), ('functions', 'event'), ('functions', 'stop')]:
            for action in ('missing', 'duplicate', 'change'):
                block = copy.deepcopy(self.block)
                records = block[group]
                record = next(x for x in records if x['name'] == C[key]['name'])
                if action == 'missing': records.remove(record)
                elif action == 'duplicate': records.append(copy.deepcopy(record))
                else: record[next(k for k in record if k != 'name')] = 'wrong'
                with self.subTest(key=key, action=action), self.assertRaises(ValueError):
                    p.validate_classes({va: block}, self.names, self.delegates)
        delegates = copy.deepcopy(self.delegates)
        delegates.append(copy.deepcopy(next(d for d in delegates if d['block_va'] == C['delegate']['block_va'])))
        with self.assertRaises(ValueError): p.validate_classes({va: self.block}, self.names, delegates)

    def test_exec_splice_and_rendered_definition(self):
        owner = {'0x140805cb0'}
        m = p.apply(model(), self.block, owner, self.execs)
        for bad in (None, {}, {'StopDoubleClickPropagation': dict(C['exec'], exec_addr='0x0')},
                    {'StopDoubleClickPropagation': dict(C['exec'], create_func='0x0')}):
            with self.assertRaises(ValueError): p.apply(model(), self.block, owner, bad)
        for accepted in ([], ['UCommonButton::StopDoubleClickPropagation'] * 2):
            bad = model(); bad['splice']['accepted'] = accepted
            with self.assertRaises(ValueError): p.apply(bad, self.block, owner, self.execs)
        header = p.declarations(m) + '\nUFUNCTION(BlueprintImplementableEvent)\n\tvoid BP_OnDoubleClicked();\nUFUNCTION(BlueprintCallable)\n\tvoid StopDoubleClickPropagation();'
        body = (HERE / 'StopDoubleClickPropagation.inc').read_text().strip('\n')
        cpp = 'void UCommonButton::StopDoubleClickPropagation()\n{\n' + '\n'.join('\t' + s for s in body.split('\n')) + '\n}'
        p.validate_output(m, header, C['delegate_header'], cpp)
        with self.assertRaises(ValueError): p.validate_output(m, header, C['delegate_header'], cpp.replace(' = true', ' = false'))
        with self.assertRaises(ValueError): p.validate_output(m, header, C['delegate_header'], cpp + '\n' + cpp)
        bad = model(); bad['members_decl_order'][1]['decl'] = 'bool bStopDoubleClickPropagation;'
        with self.assertRaises(ValueError): p.apply(bad, self.block, owner, self.execs)


if __name__ == '__main__': unittest.main()
