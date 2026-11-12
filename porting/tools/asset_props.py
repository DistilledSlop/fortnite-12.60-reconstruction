import json,sys
t=open(sys.argv[1]).read(); t=t[t.index('{'):]
d=json.loads(t)
def v(x,dep=0):
    if isinstance(x,dict):
        if 'path' in x: return x['path']
        if 'name' in x and 'type' in x: return x['name']+'='+v(x.get('value'),dep+1)
        if 'name' in x: return '@'+str(x['name'])
        return '{'+', '.join(k+':'+v(y,dep+1) for k,y in x.items())+'}'
    if isinstance(x,list): return '['+', '.join(v(y,dep+1) for y in x)+']'
    return str(x)
for e in d['exports']:
    print('##',e['name'],e.get('exportType'))
    for p in e.get('properties',[]): print('  ',v(p)[:int(sys.argv[2]) if len(sys.argv)>2 else 400])
    for k in e:
        if k not in('name','exportType','parserType','properties'): print('  +',k,str(e[k])[:300])
