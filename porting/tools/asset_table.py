import json,sys,re
t=open(sys.argv[1]).read(); t=t[t.index('{'):]
d=json.loads(t); pat=re.compile(sys.argv[2]) if len(sys.argv)>2 else None
for e in d['exports']:
    if not e.get('table'): continue
    tb=json.loads(e['table']); rows=tb.get('Rows',{})
    print('##',e['name'],tb.get('CurveTableMode',''),len(rows),'rows')
    for k,r in rows.items():
        if pat and not pat.search(k): continue
        if isinstance(r,dict) and 'Keys' in r: print(k,' '.join('%g:%g'%(x['Time'],x['Value']) for x in r['Keys']))
        else: print(k,json.dumps(r)[:int(sys.argv[3]) if len(sys.argv)>3 else 300])
