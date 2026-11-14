#!/usr/bin/env python3
"""Read-only, pinned original sidewalk Chaos schema decoder. No version overrides.
Original serializers: 141538340,1437feff0,14151b420,141537e60,141538690.
Original registered versions: external37(140476b30),destruction10(140476a70).
Only supports this fixture's convex objects and empty derived-data tail.
"""
from pathlib import Path
import hashlib,itertools,json,math,struct
ROOT=Path(__file__).resolve().parents[1]/'generated/linux-stage/LinuxNoEditor/FortniteLinux/Content/Environments/World/Sidewalks/Meshes'
class Reader:
 def __init__(self,p): self.p=p;self.o=0
 def take(self,fmt):
  n=struct.calcsize('<'+fmt);assert self.o+n<=len(self.p),(self.o,n)
  v=struct.unpack_from('<'+fmt,self.p,self.o);self.o+=n
  assert all(not isinstance(x,float) or math.isfinite(x) for x in v)
  return v[0] if len(v)==1 else v
 def count(self,stride):
  n=self.take('i');assert 0<=n<=min(4096,(len(self.p)-self.o)//stride),(self.o,n,stride)
  return n
 def vec(self):return self.take('3f')
def dot(a,b):return sum(x*y for x,y in zip(a,b))
def main():
 if not __debug__:raise SystemExit('Validation requires Python without -O')
 a=(ROOT/'S_Sidewalk_1x1_A.uasset').read_bytes();e=(ROOT/'S_Sidewalk_1x1_A.uexp').read_bytes()
 assert hashlib.sha256(a).hexdigest()=='50143cfe13ba47387fa6526c92ca375a21eee95bb3fc7c08b4ff3e8f36bf502b'
 assert hashlib.sha256(e).hexdigest()=='44c05921aacb7c25dad3a971a77b90c770f449015db5574e5c251a5b2d12099d'
 assert struct.unpack_from('<IIIq',e,0x80e)==(0x48,934,934,0x1315)
 r=Reader(e[0x822:0x822+934]);assert len(r.p)==934;assert r.take('i')==4
 assert r.count(17)==2
 objects=[]
 for identity in range(2):
  start=r.o
  assert r.take('I')==1;assert r.take('i')==identity;assert r.take('B')==8
  assert r.take('II')==(1,1)
  planes=[(r.vec(),r.vec()) for _ in range(r.count(24))]
  assert r.take('I')==1
  vertices=[r.vec() for _ in range(r.count(12))]
  lower,upper=r.vec(),r.vec();volume=r.take('f');center=r.vec()
  assert len(planes)>=4 and len(vertices)>=4
  assert volume>0 and volume<1e12
  for k in range(3):
   assert lower[k]<upper[k] and lower[k]-0.01<=center[k]<=upper[k]+0.01
   assert abs(min(v[k] for v in vertices)-lower[k])<0.01
   assert abs(max(v[k] for v in vertices)-upper[k])<0.01
  residuals=[]
  for origin,normal in planes:
   assert abs(dot(normal,normal)-1)<1e-4
   distances=[dot(tuple(v[k]-origin[k] for k in range(3)),normal) for v in vertices]
   assert max(distances)<0.01,max(distances)
   assert abs(max(distances))<0.01
   assert sum(abs(d)<0.01 for d in distances)>=3
   residuals.append(max(distances))
  assert all(any(abs(dot(tuple(v[k]-p[k] for k in range(3)),n))<0.01 for p,n in planes) for v in vertices)
  boxvol=math.prod(upper[k]-lower[k] for k in range(3));assert volume<=boxvol*1.001
  # This pinned fixture is a cuboid: verify all eight unique corners, volume and COM.
  assert len(vertices)==8 and len(set(vertices))==8
  corners=list(itertools.product(*[(lower[k],upper[k]) for k in range(3)]))
  assert all(any(max(abs(v[k]-c[k]) for k in range(3))<0.001 for v in vertices) for c in corners)
  assert abs(volume-boxvol)<boxvol*1e-6
  assert max(abs(center[k]-(lower[k]+upper[k])/2) for k in range(3))<0.001
  objects.append(dict(identity=identity,start=start,end=r.o,planes=planes,vertices=vertices,bounds=[lower,upper],volume=volume,center=center,max_plane_violation=max(residuals),box_volume=boxvol))
 tail_start=r.o
 # These empty arrays are the only supported tail; nonempty tail fails closed.
 for label in ['trimesh_objects','uv_indices','uv_positions','uv_channels','face_remap']:
  assert r.take('i')==0,label
 assert r.o==len(r.p),(r.o,len(r.p))
 print(json.dumps(dict(payload_sha256=hashlib.sha256(r.p).hexdigest(),bytes_consumed=r.o,tail_start=tail_start,registered_versions={'external_physics':37,'destruction':10},objects=objects),indent=2))
if __name__=='__main__':main()
