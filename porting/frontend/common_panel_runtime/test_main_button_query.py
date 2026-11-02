import copy
import importlib.util
from pathlib import Path
import subprocess
import tempfile
import unittest

HERE = Path(__file__).resolve().parent
spec = importlib.util.spec_from_file_location('policy', HERE / 'policy.py')
p = importlib.util.module_from_spec(spec)
spec.loader.exec_module(p)

class MainButtonQuery(unittest.TestCase):
    def model(self):
        return {'name':'UCommonButton', 'package':'/Script/CommonUI', 'functions':[]}
    def block(self):
        return {'block_va':'0x145a01210', 'functions':[copy.deepcopy(p.MAIN_BUTTON_QUERY_EXPECTED)],
                'properties':copy.deepcopy(list(p.MAIN_BUTTON_DEFAULTS_EXPECTED.values()))}
    def run_it(self, m=None, b=None, owner=None):
        return p.apply(self.model() if m is None else m, self.block() if b is None else b,
                       {'0x140805cb0'} if owner is None else owner)
    def test_signature(self):
        m = self.run_it()
        f = m['functions'][0]
        self.assertEqual((f['name'],f['ret'],f['args'],f['const'],f['access'],f['specifiers'],f['runtime_implementation']),
                         ('IsInteractionEnabled','bool',[],True,'public',['BlueprintCallable','BlueprintPure'],True))
        self.assertTrue(m['common_button_query_runtime'])
    def test_guards(self):
        with self.assertRaises(ValueError): self.run_it(owner=set())
        b=self.block(); b['block_va']='0x0'
        with self.assertRaises(ValueError): self.run_it(b=b)
        m=self.model(); m['package']='/Script/Other'
        with self.assertRaises(ValueError): self.run_it(m=m)
        for change in ('flags','params','create_func','outer_fn','missing','duplicate'):
            b=self.block(); f=b['functions'][0]
            if change=='missing': b['functions']=[]
            elif change=='duplicate': b['functions'].append(copy.deepcopy(f))
            elif change=='params': f['params'][0]['property_flags']='0x0'
            else: f[change]=None
            with self.subTest(change=change), self.assertRaises(ValueError): self.run_it(b=b)
    def test_collisions(self):
        for group in ('functions','members_table_order','members_decl_order','fields','helpers','static_helpers'):
            for name in ('IsInteractionEnabled','NativeIsInteractionEnabled','bNativeButtonEnabled','bNativeInteractionEnabled'):
                m=self.model(); m[group]=[{'name':name}]
                with self.subTest(group=group,name=name), self.assertRaises(ValueError): self.run_it(m=m)
        m=self.model();m['common_button_query_runtime']=True
        with self.assertRaises(ValueError): self.run_it(m=m)
    def test_constructor_property_guards(self):
        for name in p.MAIN_BUTTON_DEFAULTS_EXPECTED:
            for change in ('missing', 'duplicate', 'setter', 'flags'):
                b=self.block()
                row=next(x for x in b['properties'] if x['name']==name)
                if change=='missing': b['properties'].remove(row)
                elif change=='duplicate': b['properties'].append(copy.deepcopy(row))
                elif change=='setter': row['set_bit_fn']='0x0'
                else: row['property_flags']='0x0'
                with self.subTest(name=name,change=change), self.assertRaises(ValueError): self.run_it(b=b)
    def test_exact_cpp_bodies(self):
        decl=(HERE/'main-button-query-declarations.inc').read_text()
        defs=(HERE/'main-button-query-definitions.inc').read_text()
        code=r'''
#include <cassert>
#include <vector>
enum class ESlateVisibility { Visible, Collapsed, Hidden, HitTestInvisible, SelfHitTestInvisible };
struct FObjectInitializer { static const FObjectInitializer& Get() { static FObjectInitializer Value; return Value; } };
struct Parent { explicit Parent(const FObjectInitializer&) {} bool bIsFocusable=false; };
struct UCommonButton : Parent {
 using Super=Parent;
 bool bApplyAlphaOnDisable=false, bSelectable=true, bShouldSelectUponReceivingFocus=true;
 bool bToggleable=true, bDisplayInputActionWhenNotInteractable=false, bShouldUseFallbackDefaultInputAction=false;
 bool bInteractableWhenSelected=true, bHideInputActionWithKeyboard=true;
 mutable std::vector<int> calls;
 ESlateVisibility visibility=ESlateVisibility::Visible;
 bool widgetEnabled=true, bIsEnabled=false, bInteractionEnabled=false;
 ESlateVisibility GetVisibility() const { calls.push_back(1); return visibility; }
 bool GetIsEnabled() const { calls.push_back(2); return widgetEnabled; }
 bool IsInteractionEnabled() const;
''' + decl + r'''
 public:
 void SetNativeForTest(bool a,bool b) { bNativeButtonEnabled=a; bNativeInteractionEnabled=b; }
};
''' + defs + r'''
int main() {
 UCommonButton defaults; assert(defaults.IsInteractionEnabled());
 assert(defaults.bApplyAlphaOnDisable && !defaults.bSelectable && !defaults.bShouldSelectUponReceivingFocus);
 assert(!defaults.bToggleable && defaults.bDisplayInputActionWhenNotInteractable);
 assert(defaults.bShouldUseFallbackDefaultInputAction && defaults.bIsFocusable);
 assert(defaults.bInteractableWhenSelected && defaults.bHideInputActionWithKeyboard);
 for (int v=0;v<5;++v) for (int w=0;w<2;++w) for (int a=0;a<2;++a) for (int b=0;b<2;++b)
 for (int reflected=0;reflected<4;++reflected) {
   UCommonButton x; x.visibility=static_cast<ESlateVisibility>(v); x.widgetEnabled=w;
   x.SetNativeForTest(a,b); x.bIsEnabled=reflected&1; x.bInteractionEnabled=reflected&2;
   bool expected=w && a && b && v!=1 && v!=2;
   assert(x.IsInteractionEnabled()==expected); assert((x.calls==std::vector<int>{1,2}));
   x.calls.clear(); assert(x.NativeIsInteractionEnabled()==expected); assert((x.calls==std::vector<int>{1,2}));
 }
}
'''
        with tempfile.TemporaryDirectory() as d:
            src=Path(d)/'control.cpp'; exe=Path(d)/'control'; src.write_text(code)
            subprocess.run(['c++','-std=c++14','-Wall','-Wextra','-Werror',str(src),'-o',str(exe)],check=True)
            subprocess.run([str(exe)],check=True)

if __name__=='__main__': unittest.main()
