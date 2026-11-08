import json, sys
S = '/tmp/claude-1000/-opt-fortnite/46cd2cbf-c73d-4053-ba19-142367ff8af0/scratchpad/'
d = json.load(open(S + 'override_diff.json'))
want = {}
for c, v in d.items():
    for x in v['rows']:
        if x['eng'] and not x['stub']:
            want.setdefault(x['eng'], set()).add(c)
addr = {}
for l in open(S + 'syms_text.txt'):
    a, t, n = l.rstrip('\n').split(' ', 2)
    if n in want and n not in addr: addr[n] = a
miss = [n for n in want if n not in addr]
names = sorted(addr)
with open(S + 'hits.gdb', 'w') as f:
    f.write('set pagination off\nset confirm off\nhandle all nostop noprint pass\nhandle SIGINT stop print\n')
    for i, n in enumerate(names):
        f.write(f'set $c{i}=0\ntbreak *0x{addr[n]}\ncommands\nsilent\nset $c{i}=1\ncontinue\nend\n')
    f.write('run\n')
    for i, n in enumerate(names):
        f.write(f'printf "COUNT %d %s\\n", $c{i}, "{n.replace(chr(34), "")}"\n')
    f.write('kill\nquit\n')
json.dump({n: sorted(want[n]) for n in names}, open(S + 'hits_names.json', 'w'))
print(len(names), 'missing addr', len(miss), miss[:3])
