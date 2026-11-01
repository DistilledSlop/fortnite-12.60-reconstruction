import copy,json,sys,unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]))
import verify_reflection as v,verify_v1

def valid():
    rows=[]
    for name,raw,offset,kind,prop,meta in v.EXPECTED:
        d=dict(name=name,found='1',kind=kind,expected_kind=kind,raw_flags=f'{raw:016x}',computed_flags=f'{v.COMPUTED:016x}',actual_flags=f'{raw|v.COMPUTED:016x}',expected_flags=f'{raw|v.COMPUTED:016x}',actual_offset=str(offset),expected_offset=str(offset),actual_dim='1',expected_dim='1',property_class=prop,expected_property_class=prop,meta_class=meta,expected_meta_class=meta)
        rows.append(d)
    return rows

def render(rows):return '\n'.join('[actual timestamp]LogTemp: Display: CONTEXT_REFLECTION '+' '.join(k+'='+d[k] for k in v.KEYS) for d in rows)
class Reflection(unittest.TestCase):
    def test_all_measured_fields_exact(self):self.assertEqual(len(v.verify(render(valid()))['properties']),3)
    def test_missing_duplicate_and_reordered_rows(self):
        d=valid()
        for rows in [d[:-1],d+[d[0]],d[::-1]]:
            with self.subTest(rows=rows),self.assertRaises(ValueError):v.verify(render(rows))
    def test_unknown_flag_bits_never_masked(self):
        for key in ['raw_flags','computed_flags','actual_flags','expected_flags']:
            rows=valid();rows[0][key]=f'{int(rows[0][key],16)|0x40:016x}'
            with self.subTest(key=key),self.assertRaises(ValueError):v.verify(render(rows))
    def test_offsets_dimensions_types_and_notfound(self):
        for index in range(3):
            for key,value in [('found','0'),('kind','IntProperty'),('expected_kind','IntProperty'),('actual_offset','24'),('expected_offset','24'),('actual_dim','2'),('expected_dim','2'),('property_class','/Script/CoreUObject.Object' if index<2 else '/Script/Engine.Subsystem'),('expected_property_class','-'),('meta_class','/Script/CoreUObject.Object'),('expected_meta_class','/Script/CoreUObject.Object')]:
                rows=valid();rows[index][key]=value
                with self.subTest(index=index,key=key),self.assertRaises(ValueError):v.verify(render(rows))
    def test_malformed_and_duplicate_fields(self):
        text=render(valid())
        for bad in [text.replace('found=1','found=1 found=1',1),text.replace('computed_flags=0008001040000200','computed_flags=-1',1),text+'\nCONTEXT_REFLECTION malformed',text.replace('actual_dim=1 ','',1)]:
            with self.assertRaises(ValueError):v.verify(bad)
    def test_computed_gate_is_required_before_function(self):
        required=list(verify_v1.REQUIRED);i=required.index('reflection_computed_flags');self.assertEqual(required[i+1],'reflection_function')
        good='\n'.join('CONTEXT_PROBE_V1 '+k+' PASS' for k in required)+'\nCONTEXT_PROBE_V1 END';self.assertTrue(verify_v1.verify(good))
        with_removed=good.replace('CONTEXT_PROBE_V1 reflection_computed_flags PASS\n','');self.assertFalse(verify_v1.verify(with_removed));self.assertFalse(verify_v1.verify(good.replace('reflection_computed_flags PASS','reflection_computed_flags FAIL')))
    def test_probe_uses_typed_computation_and_exact_or(self):
        root=Path(__file__).resolve().parents[4];p=root/'porting/native/Source/FortniteGame/ContextRuntimeProbe.cpp';s=p.read_text()
        self.assertIn('TPropertyTypeFundamentals<UObject*>',s);self.assertIn('return uint64(GetComputedFlagsPropertyFlags())',s);self.assertIn('ComputedFlags==0x0008001040000200ULL',s)
        for raw in ['0x10000000000580ULL','0x14000000000080ULL','0x10000000000080ULL']:self.assertIn('PropertyFlags) == ('+raw+' | ComputedFlags)',s)
        self.assertNotIn('PropertyFlags &',s)
if __name__=='__main__':unittest.main()
