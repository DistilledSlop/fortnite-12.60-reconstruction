import copy
import importlib.util
import json
import subprocess
import tempfile
import unittest
from types import SimpleNamespace
from unittest.mock import patch
from pathlib import Path

HERE = Path(__file__).resolve().parent
spec = importlib.util.spec_from_file_location('current_type_policy', HERE/'policy.py')
p = importlib.util.module_from_spec(spec); spec.loader.exec_module(p)

def fixture():
    return ({'name':'UCommonInputSubsystem','package':'/Script/CommonInput','parent':'ULocalPlayerSubsystem',
             'functions':[], 'members_decl_order':[{'name':'CurrentInputType','decl':'ECommonInputType CurrentInputType;'}]},
            {'block_va':'0x145a0e0d0','functions':[copy.deepcopy(p.CONTRACT['function'])], 'properties':copy.deepcopy(p.CONTRACT['properties'])},
            {'0x14081e010'}, {'GetCurrentInputType':copy.deepcopy(p.CONTRACT['exec'])})

def test_policy():
    args=fixture(); result=p.apply(*args)
    f=result['functions'][0]
    assert f['ret']=='ECommonInputType' and f['const'] and f['specifiers']==['BlueprintCallable','BlueprintPure']
    assert p.definitions(result)==(HERE/'definitions.inc').read_text()
    for mutate in (lambda a:a[1]['functions'][0]['params'][0]['type_refs'].update(enum='0x0'),
                   lambda a:a[1]['properties'][0].update(offset=0),
                   lambda a:a[3]['GetCurrentInputType'].update(exec_addr='0x0'),
                   lambda a:a[0]['members_decl_order'][0].update(decl='uint8 CurrentInputType;'),
                   lambda a:a[2].clear()):
        args=fixture(); mutate(args)
        try: p.apply(*args)
        except ValueError: pass
        else: raise AssertionError('source guard failed')

def test_compiled_mutable_getter():
    source = '''#include <cstdint>\n#include <cassert>\nenum class ECommonInputType : uint8_t { MouseAndKeyboard=0, Gamepad=1, Touch=2, Count=3 };\nstruct UCommonInputSubsystem { ECommonInputType CurrentInputType; ECommonInputType GetCurrentInputType() const; };\n'''+(HERE/'definitions.inc').read_text()+'''\nint main() { UCommonInputSubsystem s; for (int i=0;i<=255;++i) { s.CurrentInputType=static_cast<ECommonInputType>(i); const auto& c=s; assert(static_cast<int>(c.GetCurrentInputType())==i); } }\n'''
    with tempfile.TemporaryDirectory() as td:
        cpp=Path(td)/'test.cpp'; binary=Path(td)/'test'; cpp.write_text(source)
        subprocess.run(['c++','-std=c++17','-Wall','-Wextra','-Werror',str(cpp),'-o',str(binary)],check=True)
        subprocess.run([str(binary)],check=True)

class PolicyTests(unittest.TestCase):
    def test_guards(self):
        test_policy()

    def test_mutable_cpp(self):
        test_compiled_mutable_getter()

    def test_enum_type_guard(self):
        info = SimpleNamespace(fn='0x14081ee20', kind='enum', cpp_name='ECommonInputType',
                               package='/Script/CommonInput', rec=copy.deepcopy(p.CONTRACT['enum']))
        gate = SimpleNamespace(info=lambda fn: info, reason=lambda value: None)
        p.validate_types(gate)
        for field, value in [('fn', '0x0'), ('kind', 'struct'), ('cpp_name', 'Wrong'),
                             ('package', '/Script/Wrong'), ('rec', {})]:
            bad = copy.deepcopy(info)
            setattr(bad, field, value)
            gate.info = lambda fn, bad=bad: bad
            with self.assertRaises(ValueError):
                p.validate_types(gate)
        gate.info = lambda fn: info
        gate.reason = lambda value: 'unavailable'
        with self.assertRaises(ValueError):
            p.validate_types(gate)
        gate.info = lambda fn: None
        with self.assertRaises(ValueError):
            p.validate_types(gate)

    def test_generator_integration(self):
        path = HERE.parents[1] / 'gen_native_classes.py'
        spec = importlib.util.spec_from_file_location('native_generator_current_type_test', path)
        gen = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(gen)
        _, block, owner, native = fixture()
        owners = json.loads((HERE.parent/'common_panel_runtime'/'panel-input-owner-original-policy.json').read_text())
        block['properties'] = owners[block['block_va']]['properties']
        member = {'name': 'CurrentInputType', 'decl': 'ECommonInputType CurrentInputType;',
                  'rep_notify': None, 'replicated': False, 'used': {}, 'table_index': 0,
                  'property_flags': '0', 'specifiers': [], 'offset': 0}
        resolver = SimpleNamespace()
        parent = {'parent_name': 'ULocalPlayerSubsystem', 'package': '/Script/CommonInput'}
        with patch.object(gen, 'build_members', return_value=([member], [])), \
             patch.object(gen, 'build_functions_promoted', return_value=[]), \
             patch.object(gen, 'function_view', return_value=(None, None)), \
             patch.object(gen, 'uclass_line', return_value='UCLASS()'):
            model = gen.build_class_promoted(block, 'UCommonInputSubsystem', resolver, parent, owner,
                                             native_execs=native)
            header = gen.render_promoted_header(model, 'API', 'Parent.h', 'test')
            cpp = gen.render_promoted_cpp(model, 'test')
            self.assertIn('UFUNCTION(BlueprintCallable, BlueprintPure)', header)
            self.assertIn('ECommonInputType GetCurrentInputType() const;', header)
            self.assertEqual(cpp.count('UCommonInputSubsystem::GetCurrentInputType() const'), 1)
            self.assertIn((HERE/'definitions.inc').read_text(), cpp)
            with self.assertRaises(ValueError):
                gen.build_class_promoted(block, 'UCommonInputSubsystem', resolver, parent, owner,
                                         native_execs={})

if __name__=='__main__':
    unittest.main()
