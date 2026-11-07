"""Helper: does a generated .cpp already define Class::Fn with a real (non-stub) body? Run from repo root."""
import glob, re

_DEF = {}


def defined_in_generated(cls, fn):
    """The generator emits a reflection-derived stub ('Body not recovered') for every reflected function, so stubs do not count.
    A real body means a generator policy (or a ported entry) already emits the function."""
    if cls not in _DEF:
        _DEF[cls] = ''.join(open(p).read() for p in glob.glob(f'porting/native/Source/*/Generated/{cls}.cpp'))
    for m in re.finditer(r'\b%s::(?:%s|%s_Implementation)\s*\([^)]*\)[^{;]*\n\{(.*?)\n\}' % (cls, fn, fn), _DEF[cls], re.S):
        if 'Body not recovered' not in m.group(1):
            return True
    return False
