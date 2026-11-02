import copy
import importlib.util
import json
import unittest
from pathlib import Path

H = Path(__file__).resolve().parent
R = H.parents[2]
spec = importlib.util.spec_from_file_location('click_policy', H/'policy.py')
p = importlib.util.module_from_spec(spec); spec.loader.exec_module(p)
C = p.CONTRACT

def model():
    return {'name':'UCommonButton', 'block_va':C['identity']['block_va'], 'package':'/Script/CommonUI',
            'members_decl_order':[{'name':'OnButtonClicked','decl':C['member']}], 'functions':[]}

class PolicyTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        r = R/'porting/generated/reflection'
        cls.block = next(x for x in json.loads((r/'fortnite_game_reflection.json').read_text())['classes_in_groups_with_fort_names'] if x['block_va']==C['identity']['block_va'])
        cls.names = {v:k for k,v in json.loads((r/'class_name_map.json').read_text()).items()}
        cls.delegates = json.loads((r/'delegate_blocks.json').read_text())['delegates']
        g = R/'porting/native/Source/CommonUI/Generated'
        cls.header = (g/'UCommonButton.h').read_text()
        cls.dh = (g/'FCommonButtonClicked.h').read_text()
        cls.sh = (g/'UCommonUISubsystem.h').read_text()

    def classes(self, b=None, names=None, delegates=None):
        p.validate_classes({C['identity']['block_va']: b or self.block}, names or self.names, self.delegates if delegates is None else delegates)

    def test_authentic_and_metadata_mutations(self):
        self.classes()
        for key in C['identity']:
            b=copy.deepcopy(self.block); b[key]='wrong'
            with self.subTest(identity=key), self.assertRaises(ValueError): self.classes(b=b)
        for group, key in [('properties','property'),('functions','event')]:
            for action in ('remove','duplicate','mutate'):
                b=copy.deepcopy(self.block); items=b[group]; item=next(x for x in items if x['name']==C[key]['name'])
                if action=='remove': items.remove(item)
                elif action=='duplicate': items.append(copy.deepcopy(item))
                else: item['name'] += '_changed'
                with self.subTest(group=group,action=action), self.assertRaises(ValueError): self.classes(b=b)
        for group, key in [('properties','property'),('functions','event')]:
            for field in C[key]:
                b=copy.deepcopy(self.block)
                next(x for x in b[group] if x['name']==C[key]['name'])[field]='changed'
                with self.subTest(record=key,field=field), self.assertRaises(ValueError): self.classes(b=b)
        for action in ('remove','duplicate','signature','alias_va','alias_name','alias_constructor'):
            ds=copy.deepcopy(self.delegates); d=next(x for x in ds if x['block_va']==C['delegate']['block_va'])
            if action=='remove': ds.remove(d)
            elif action=='signature': d['params'][0]['type_refs']['class']='0x0'
            else:
                extra=copy.deepcopy(d)
                if action=='alias_va': extra['name']='Other'; extra['construct_fns']=[]
                if action=='alias_name': extra['block_va']='0x0'; extra['construct_fns']=[]
                if action=='alias_constructor': extra['block_va']='0x0'; extra['name']='Other'
                ds.append(extra)
            with self.subTest(delegate=action), self.assertRaises(ValueError): self.classes(delegates=ds)
        names=dict(self.names); names['UCommonButton']='0x0'
        with self.assertRaises(ValueError): self.classes(names=names)

    def test_model_and_collisions(self):
        b={'block_va':C['identity']['block_va']}; owners={'0x140805cb0'}
        for key in ('name','package','block_va'):
            m=model(); m[key]='wrong'
            with self.subTest(key=key), self.assertRaises(ValueError): p.validate_model(m)
        for action in ('remove','duplicate','change'):
            m=model()
            if action=='remove': m['members_decl_order'].clear()
            elif action=='duplicate': m['members_decl_order'].append(copy.deepcopy(m['members_decl_order'][0]))
            else: m['members_decl_order'][0]['decl']='int OnButtonClicked;'
            with self.subTest(action=action), self.assertRaises(ValueError): p.apply(m,b,owners)
        for group in p.GROUPS:
            for helper in p.HELPERS:
                m=model(); m.setdefault(group,[]).append({'name':helper})
                with self.subTest(group=group,helper=helper), self.assertRaises(ValueError): p.apply(m,b,owners)
        with self.assertRaises(ValueError): p.apply(model(),b,set())
        with self.assertRaises(ValueError): p.apply(model(),{'block_va':'0x0'},owners)
        m=p.apply(model(),b,owners)
        with self.assertRaises(ValueError): p.apply(m,b,owners)
        self.assertEqual(m['functions'],[])
        self.assertNotIn('UFUNCTION',p.declarations(m))
        self.assertEqual(p.apply({'name':'UOther'}, {}, set()).get('name'),'UOther')

    def test_rendered_sources_and_collision(self):
        m=p.apply(model(),{'block_va':C['identity']['block_va']},{'0x140805cb0'})
        header=self.header if 'NativeOnClicked' in self.header else self.header+'\n'+p.declarations(m)
        p.validate_output(m,header,self.dh,self.sh)
        for helper in p.HELPERS:
            with self.subTest(helper=helper), self.assertRaises(ValueError): p.validate_output(m,header+'\n'+helper,self.dh,self.sh)
        for index in range(3):
            args=[header,self.dh,self.sh]; args[index]=''
            with self.subTest(source=index), self.assertRaises(ValueError): p.validate_output(m,*args)
        with self.assertRaises(ValueError): p.validate_output(m,header+'\nvoid BP_OnClicked();',self.dh,self.sh)
        with self.assertRaises(ValueError): p.validate_output(m,header,self.dh+'\n'+C['delegate_header'],self.sh)
        with self.assertRaises(ValueError): p.validate_output(model(),header,self.dh,self.sh)

if __name__=='__main__': unittest.main()
