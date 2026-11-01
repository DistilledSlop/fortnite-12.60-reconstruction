#!/usr/bin/env python3
"""Verify every v3 artifact and reject missing, extra, changed, or symlink paths."""
import hashlib,json
from pathlib import Path

def digest(p):
 h=hashlib.sha256()
 with p.open('rb') as f:
  for b in iter(lambda:f.read(1048576),b''):h.update(b)
 return h.hexdigest()
def audit(root):
 root=Path(root).resolve();m=json.loads((root/'manifest.json').read_text());expected=m['files'];actual={}
 for p in root.rglob('*'):
  if p.is_symlink():raise ValueError('symlink '+str(p))
  if p.is_file() and p != root/'manifest.json':actual[str(p.relative_to(root))]={'sha256':digest(p),'size':p.stat().st_size}
 if actual!=expected:raise ValueError('artifact tree differs; missing=%s extra=%s changed=%s' % (sorted(set(expected)-set(actual))[:5],sorted(set(actual)-set(expected))[:5],sorted(k for k in set(actual)&set(expected) if actual[k]!=expected[k])[:5]))
 return {'passed':True,'file_count':len(actual),'manifest_sha256':digest(root/'manifest.json')}
if __name__=='__main__':print(json.dumps(audit(Path(__file__).parent),sort_keys=True))
