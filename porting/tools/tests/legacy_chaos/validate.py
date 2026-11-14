#!/usr/bin/env python3
"""Standalone parser differential/synthetic/malformed tests; assets stay ignored."""
import argparse, json, pathlib, struct, subprocess, math, sys

if not __debug__:
 raise SystemExit('This validation uses assertions; run without Python -O.')

ROOT=pathlib.Path(__file__).resolve().parents[4]
HERE=pathlib.Path(__file__).resolve().parent
OUTPUT=ROOT/'porting/generated/agents/legacy-parser-retained-tests44'
parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('--engine-root', type=pathlib.Path, default=ROOT.parent/'UnrealEngine',
                    help='UnrealEngine checkout root (default: sibling of repository)')
args=parser.parse_args()
header_dir=args.engine_root.resolve()/'Engine/Source/Runtime/Engine/Private/PhysicsEngine'
header=header_dir/'FortniteLegacyChaosCookedData.h'
if not header.is_file():
 raise SystemExit(f'Engine parser header not found: {header}')
STAGE=ROOT/'porting/generated/linux-stage/LinuxNoEditor/FortniteLinux/Content'
fixtures=[STAGE/'Environments/World/Sidewalks/Meshes/S_Sidewalk_1x1_A.uexp',
          STAGE/'Environments/Apollo/Sets/OilRig/Meshes/SM_OilRig_Trim2_Cap_TriangleFloor.uexp']
missing=[str(path) for path in fixtures if not path.is_file()]
if missing:
 raise SystemExit('Recovered stage fixtures are required; missing:\n'+'\n'.join(missing))
OUTPUT.mkdir(parents=True, exist_ok=True)
subprocess.run(['g++','-std=c++14','-Wall','-Wextra','-Werror',
                '-fsanitize=undefined','-fno-sanitize-recover=all',
                '-I',str(HERE),'-I',str(header_dir),str(HERE/'harness.cpp'),
                '-o',str(OUTPUT/'harness')],check=True)
side=(STAGE/'Environments/World/Sidewalks/Meshes/S_Sidewalk_1x1_A.uexp').read_bytes()[0x822:0x822+934]
oil=(STAGE/'Environments/Apollo/Sets/OilRig/Meshes/SM_OilRig_Trim2_Cap_TriangleFloor.uexp').read_bytes()[1569:1569+609]
expected_side=json.loads(subprocess.check_output([sys.executable,str(ROOT/'porting/tools/inspect_original_sidewalk_collision.py')]))
expected_oil=json.loads(subprocess.check_output([sys.executable,str(ROOT/'porting/tools/inspect_original_oilrig_collision.py')]))
fields={v['name']:v for v in expected_oil['field_map']}
count=0
results=[]
def execute(label,data,ok=True):
 global count
 p=OUTPUT/'case.bin';p.write_bytes(data)
 r=subprocess.run([str(OUTPUT/'harness'),str(p)],capture_output=True,text=True)
 assert r.returncode==(0 if ok else 1),(label,r.returncode,r.stdout,r.stderr)
 count+=1
 if ok:return json.loads(r.stdout)
 assert 'Original Chaos at byte ' in r.stdout,(label,r.stdout)
 return r.stdout.strip()
def pack(fmt,*x):return struct.pack('<'+fmt,*x)
def change(data,offset,fmt,*values):return data[:offset]+pack(fmt,*values)+data[offset+struct.calcsize('<'+fmt):]
def reject(label,data):results.append({'label':label,'error':execute(label,data,False)})
def close(a,b):
 if isinstance(a,list):return len(a)==len(b) and all(close(x,y) for x,y in zip(a,b))
 return math.isclose(a,b,rel_tol=1e-8,abs_tol=1e-6)
a=execute('sidewalk fixture',side)
assert len(a['convexes'])==2 and not a['meshes']
for actual,expected in zip(a['convexes'],expected_side['objects']):
 assert actual['collide']==1 and close(actual['vertices'],expected['vertices'])
b=execute('oilrig fixture',oil);m=b['meshes'][0]
assert not b['convexes'] and len(b['meshes'])==1 and m['collide']==1
for key,label in [('vertices','particles'),('triangles','triangles'),('materials','material_indices'),('facemap','face_index_map')]:assert close(m[key],fields[label]['value'])
# Original UV nested float2 serializer fully consumed, including several channels.
tail=pack('i3i',3,0,1,2)+pack('i9f',3,1,2,3,4,5,6,7,8,9)+pack('i',2)
tail+=pack('i6f',3,0,.25,.5,.75,1,-1)+pack('i6f',3,-2,2,-3,3,-4,4)+pack('i3i',3,7,4,-1)
c=execute('nonempty original UV/remap',oil[:-16]+tail)
assert c['uv_indices']==[0,1,2] and c['uv_positions']==[[1,2,3],[4,5,6],[7,8,9]]
assert c['uv_channels']==[[[0,.25],[.5,.75],[1,-1]],[[-2,2],[-3,3],[-4,4]]] and c['face_remap']==[7,4,-1]
# Mixed object families share one registry. References omit the type/body.
convex=side[8:461];mesh=oil[12:-16]
# Alter mesh ID to 1 so registry has both families, then alias each twice.
mixed=pack('ii',4,3)+convex+pack('Ii',1,0)+pack('I',0)+pack('i',3)+change(mesh,4,'i',1)+pack('IiI',1,1,0)+pack('4i',0,0,0,0)
c=execute('mixed families null and aliases',mixed)
assert c['convex_alias']==1 and c['mesh_alias']==1
assert len(c['convexes'])==3 and c['convexes'][2] is None and c['convexes'][0]==c['convexes'][1]
assert len(c['meshes'])==3 and c['meshes'][2] is None and c['meshes'][0]==c['meshes'][1]
execute('sparse nonsequential ID',change(side,12,'i',999999))
# Stored false collision is overridden for convex too.
c=execute('destruction10 override convex false',change(side,21,'I',0));assert c['convexes'][0]['collide']==1
# Large triangle indices follow the selected wire representation, not inferred flags.
t=fields['triangles'];large=change(oil,fields['requires_large_indices']['start'],'I',1)
large=large[:t['start']]+pack('i',8)+b''.join(pack('3i',*tri) for tri in t['value'])+large[t['end']:]
assert execute('large int32 triangles',large)['meshes'][0]==m
# BVH optional map is keys first followed by triples. Exercise dirty/global arrays too.
d=fields['bvh_dirty_elements'];g=fields['bvh_global_payloads'];flag=fields['bvh_serialize_payload_info'];settings=fields['bvh_settings']
payload=pack('i',1)+pack('i6f',0,0,0,0,1,1,1)
withmap=oil[:d['start']]+payload+payload+pack('Ii2i6i',1,2,0,1,0,-1,-1,-1,0,-1)+oil[settings['start']:]
execute('nonempty BVH dirty global keys-first map',withmap)
reject('cross-family registry reference',pack('ii',4,1)+convex+pack('iIi',1,1,0)+pack('4i',0,0,0,0))
for ty in [0,1,7,9,10,12,0x48,0x4b,0x88,0x8b,0xff]:reject('unsupported type '+hex(ty),change(oil,20,'B',ty))
for label,data in [
 ('negative object count',change(oil,4,'i',-1)),('huge object count',change(oil,4,'i',2147483647)),
 ('negative identity',change(oil,16,'i',-1)),('identity cap',change(oil,16,'i',1048576)),
 ('precision8',change(oil,0,'i',8)),('invalid present bool',change(oil,12,'I',2)),
 ('invalid implicit bool',change(oil,21,'I',2)),('invalid view bool',change(oil,29,'I',2)),
 ('NaN vertex',change(oil,37,'I',0x7fc00000)),('negative vertex count',change(oil,33,'i',-4)),
 ('triangle index overflow',change(oil,t['start']+4,'H',65535)),
 ('trailing byte',oil+b'X'),('empty',b''),('UV index out of bounds',oil[:-16]+pack('i3i',3,0,0,0)+pack('3i',0,0,0)),
 ('negative face remap',oil[:-4]+pack('ii',1,-2)),
 ('invalid BVH map key',withmap.replace(pack('Ii2i6i',1,2,0,1,0,-1,-1,-1,0,-1),pack('Ii2i6i',1,2,0,0,0,-1,-1,-1,0,-1))),
]:reject(label,data)
# Every proper prefix of each real fixture must fail, without changing Out.
for label,data in [('sidewalk',side),('oilrig',oil)]:
 for n in range(len(data)):execute(label+' truncated at '+str(n),data[:n],False)
(OUTPUT/'validation-results.json').write_text(json.dumps({'passed_cases':count,'fixture_geometry':'all decoded vertices, triangles, materials, face map match independent pinned inspectors','negative_cases':results,'truncations':len(side)+len(oil)},indent=2)+'\n')
print(json.dumps({'passed_cases':count,'truncations':len(side)+len(oil),'explicit_negative_cases':len(results)}))
