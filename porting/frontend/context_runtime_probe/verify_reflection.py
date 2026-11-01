#!/usr/bin/env python3
"""Verify all measured loaded fields exactly; raw original metadata stays separate."""
import re

COMPUTED=0x0008001040000200
EXPECTED=(('ReturnValue',0x0010000000000580,16,'ObjectProperty','/Script/Engine.Subsystem','-'),
          ('Class',0x0014000000000080,8,'ClassProperty','/Script/CoreUObject.Class','/Script/Engine.Subsystem'),
          ('ContextObject',0x0010000000000080,0,'ObjectProperty','/Script/CoreUObject.Object','-'))
KEYS=('name','found','kind','expected_kind','raw_flags','computed_flags','actual_flags','expected_flags',
      'actual_offset','expected_offset','actual_dim','expected_dim','property_class','expected_property_class','meta_class','expected_meta_class')

def need(value,message):
    if not value:raise ValueError('loaded reflection: '+message)

def verify(text):
    rows=[]
    for line in text.splitlines():
        if 'CONTEXT_REFLECTION' not in line:continue
        marker='CONTEXT_REFLECTION '
        need(line.count(marker)==1,'malformed row marker')
        words=line.split(marker,1)[1].split()
        need(len(words)==len(KEYS) and all(word.count('=')==1 for word in words),'exact row fields')
        pairs=[word.split('=',1) for word in words]
        need(tuple(key for key,_ in pairs)==KEYS,'ordered fields/missing/duplicate key')
        rows.append(dict(pairs))
    need(len(rows)==3,'three rows exactly once')
    result=[]
    for row,(name,raw,offset,kind,prop,meta) in zip(rows,EXPECTED):
        need(row['name']==name,'original ordered property identity')
        required=dict(found='1',kind=kind,expected_kind=kind,actual_offset=str(offset),expected_offset=str(offset),actual_dim='1',expected_dim='1',property_class=prop,expected_property_class=prop,meta_class=meta,expected_meta_class=meta)
        need(all(row[key]==value for key,value in required.items()),'actual/expected offset, dimension, type or presence '+name)
        flags={}
        for key in ['raw_flags','computed_flags','actual_flags','expected_flags']:
            need(re.fullmatch('[0-9a-f]{16}',row[key]) is not None,'exact unsigned flag word '+name)
            flags[key]=int(row[key],16)
        need(flags==dict(raw_flags=raw,computed_flags=COMPUTED,actual_flags=raw|COMPUTED,expected_flags=raw|COMPUTED),'exact raw | original computed bits; no unknown bits '+name)
        result.append(dict(name=name,**flags,offset=offset,array_dim=1,kind=kind,property_class=prop,meta_class=meta))
    return dict(passed=True,original_computed_flags=COMPUTED,properties=result,scope='Measured actual loaded properties equal exact raw original flags OR source-proven pointer computed bits; no unknown-bit mask. Raw UHT/object/ELF audit remains independent.')
