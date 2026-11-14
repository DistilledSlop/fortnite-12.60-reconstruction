"""Read-only original ext37 trimesh fixture inspector; research only, no installation."""
from pathlib import Path
import json,struct,math,hashlib
if not __debug__:raise SystemExit('Validation requires Python without -O')
x=dict(path='porting/generated/linux-stage/LinuxNoEditor/FortniteLinux/Content/Environments/Apollo/Sets/OilRig/Meshes/SM_OilRig_Trim2_Cap_TriangleFloor.uexp',bulk_header=1549,payload_start=1569,payload_size=609)
f=Path(__file__).resolve().parents[2]/x['path'];b=f.read_bytes();a=f.with_suffix('.uasset').read_bytes();p=b[x['payload_start']:][:x['payload_size']]
assert len(a)==3084 and len(b)==4880
assert hashlib.sha256(a).hexdigest()=='ce226fcea2a568769bc4f29df460670d0c878525da742410491a63e862cf2bfd'
assert hashlib.sha256(b).hexdigest()=='997b96a4a18fac04446f806664c13181d5f7182f20a688ea879437c996dc67b5'
assert struct.unpack_from('<IIIq',b,x['bulk_header'])==(0x48,609,609,4653)
assert len(p)==609 and x['payload_start']+len(a)==4653
assert hashlib.sha256(p).hexdigest()=='0cb3f6b7e6c19ff5578d6e60420622a72a2915f9cb57d0f4e4c83e8c9257c139'
class Reader:
 def __init__(self,p):self.p=p;self.o=0;self.fields=[]
 def get(self,fmt):
  n=struct.calcsize('<'+fmt);assert self.o+n<=len(self.p),(self.o,fmt)
  v=struct.unpack_from('<'+fmt,self.p,self.o);self.o+=n
  assert all(not isinstance(k,float) or math.isfinite(k) for k in v)
  return v[0] if len(v)==1 else list(v)
 def count(self,stride):
  n=self.get('i');assert 0<=n<=min(65536,(len(self.p)-self.o)//stride);return n
 def field(self,name,fn):
  start=self.o;v=fn();self.fields.append(dict(name=name,start=start,end=self.o,value=v));return v
 def bounds(self):
  v=self.get('6f');assert all(v[k]<=v[k+3] for k in range(3));return v
 def arr(self,fmt):return [self.get(fmt) for _ in range(self.count(struct.calcsize('<'+fmt)))]
 def payloads(self):return [dict(payload=self.get('i'),bounds=self.bounds()) for _ in range(self.count(28))]
r=Reader(p)
assert r.field('precision',lambda:r.get('i'))==4
assert r.field('convex_count',lambda:r.get('i'))==0
assert r.field('triangle_count',lambda:r.get('i'))==1
assert r.field('pointer_present',lambda:r.get('I'))==1
assert r.field('object_identity',lambda:r.get('i'))==0
assert r.field('type',lambda:r.get('B'))==11
flags=r.field('implicit_flags',lambda:r.get('II'));assert flags==[0,0]
# Destruction10 forces bDoCollide=true after reading the stored second flag.
effective_flags=dict(is_convex=bool(flags[0]),do_collide=True)
assert r.field('particles_view',lambda:r.get('I'))==1
verts=r.field('particles',lambda:r.arr('3f'))
large=r.field('requires_large_indices',lambda:r.get('I'));assert large in (0,1)
tris=r.field('triangles',lambda:r.arr('3i' if large else '3H'))
bounds=r.field('local_bounds',r.bounds)
full=r.field('bvh_full_bounds',r.bounds)
def node():return dict(bounds=[r.bounds(),r.bounds()],children=r.get('2i'),leaf=r.get('I'))
nodes=r.field('bvh_nodes',lambda:[node() for _ in range(r.count(60))])
leaves=r.field('bvh_leaves',lambda:[r.payloads() for _ in range(r.count(4))])
dirty=r.field('bvh_dirty_elements',r.payloads);glob=r.field('bvh_global_payloads',r.payloads)
hasmap=r.field('bvh_serialize_payload_info',lambda:r.get('I'));assert hasmap in (0,1)
def mapinfo():
 keys=r.arr('i');assert len(keys)==len(set(keys));assert all(0<=k<len(tris) for k in keys)
 return [dict(key=k,info=r.get('3i')) for k in keys]
info=r.field('bvh_payload_info',mapinfo) if hasmap else None
settings=r.field('bvh_settings',lambda:dict(max_children=r.get('i'),max_depth=r.get('i'),max_payload_bounds=r.get('f')))
materials=r.field('material_indices',lambda:r.arr('H'));facemap=r.field('face_index_map',lambda:r.arr('i'))
for label in ['uv_indices','uv_positions','uv_channels','face_remap']:
 assert r.field(label,lambda:r.get('i'))==0
assert r.o==len(p)
assert len(materials) in (0,len(tris)) and len(facemap) in (0,len(tris))
for t in tris:
 assert len(set(t))==3 and all(0<=i<len(verts) for i in t)
 v0,v1,v2=[verts[i] for i in t];e=[v1[k]-v0[k] for k in range(3)];g=[v2[k]-v0[k] for k in range(3)];cross=[e[1]*g[2]-e[2]*g[1],e[2]*g[0]-e[0]*g[2],e[0]*g[1]-e[1]*g[0]]
 assert sum(z*z for z in cross)>1e-10
for k in range(3):
 assert abs(min(v[k] for v in verts)-bounds[k])<1e-4
 assert abs(max(v[k] for v in verts)-bounds[k+3])<1e-4
allpayloads=[v for leaf in leaves for v in leaf]+dirty+glob
assert sorted(v['payload'] for v in allpayloads)==list(range(len(tris)))
for v in allpayloads:
 for k in range(3):
  tv=[verts[i][k] for i in tris[v['payload']]]
  assert abs(min(tv)-v['bounds'][k])<1e-4 and abs(max(tv)-v['bounds'][k+3])<1e-4
for n in nodes:
 assert n['leaf'] in (0,1)
 if n['leaf']:assert all(0<=c<len(leaves) for c in n['children'])
 else:assert all(0<=c<len(nodes) for c in n['children'])
print(json.dumps(dict(fixture=x,uasset_sha256=hashlib.sha256(a).hexdigest(),uexp_sha256=hashlib.sha256(b).hexdigest(),payload_sha256=hashlib.sha256(p).hexdigest(),bytes_consumed=r.o,versions=dict(external=37,destruction=10),effective_implicit_flags=effective_flags,particles=len(verts),triangles=len(tris),field_map=r.fields),indent=2))
