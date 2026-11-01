"""Bind one fresh UBT .o to the exact generated source using its response/unity inputs."""
from pathlib import Path
import re,shlex

def need(v,s):
 if not v:raise ValueError(s)
def resolve(development, generated, since_ns):
 generated=Path(generated).resolve();dev=Path(development);hits=[]
 for rsp in sorted(dev.glob('*.cpp.o.rsp')):
  args=shlex.split(rsp.read_text());cpp=[Path(x).resolve() for x in args if x.endswith('.cpp')];out=[Path(args[i+1]).resolve() for i,x in enumerate(args[:-1]) if x=='-o']
  if len(cpp)!=1 or len(out)!=1:continue
  unit=cpp[0];obj=Path(str(rsp)[:-4]).resolve()
  if not unit.is_file() or not obj.is_file() or out[0]!=obj or obj.stat().st_mtime_ns<since_ns:continue
  if unit==generated: matches=1
  else:
   matches=0
   for inc in re.findall(r'^\s*#\s*include\s+"([^"\n]+)"',unit.read_text(),flags=re.M):
    incpath=Path(inc);incpath=incpath.resolve() if incpath.is_absolute() else (unit.parent/incpath).resolve()
    if incpath==generated:matches+=1
  if matches==1:hits.append({'object':obj,'rsp':rsp.resolve(),'unity_source':unit})
 need(len(hits)==1,'expected unique fresh UBT response/object/unity including exact generated source; got '+str(len(hits)))
 return hits[0]
