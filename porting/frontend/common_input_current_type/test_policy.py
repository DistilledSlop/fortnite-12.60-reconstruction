import copy
import importlib.util
import subprocess
import tempfile
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

if __name__=='__main__':
    test_policy(); test_compiled_mutable_getter(); print('PASS: guarded signature and compiled mutable getter')
