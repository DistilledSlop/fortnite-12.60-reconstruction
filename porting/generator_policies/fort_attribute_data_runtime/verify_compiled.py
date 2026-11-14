"""Compile actual emitted declarations and compare their stored bytes with original setters."""
import ctypes
import itertools
import json
from pathlib import Path
import shlex
import subprocess
import struct

COMPILER = Path('/opt/fortnite/UnrealEngine/Engine/Extras/ThirdPartyNotUE/SDKs/HostLinux/Linux_x64/v16_clang-9.0.1-centos7/x86_64-unknown-linux-gnu/bin/clang++')


def run(policy, out, original):
    repo = policy.HERE.parents[2]
    candidates = sorted((repo / 'porting/native/Intermediate/Build/Linux').glob(
        '*/FortniteLinuxServer/Development/FortniteGame/*.rsp'))
    if not candidates:
        raise RuntimeError('missing actual Linux server response file')
    rsp = candidates[0]
    args = shlex.split(rsp.read_text())
    relevant = [a for a in args if a.startswith(('-O', '-fno-math-errno', '-ffast-math',
        '-fno-fast-math', '-ffp-contract', '-fno-signed-zeros', '-fdenormal-fp-math',
        '-ffinite-math-only', '-funsafe-math-optimizations', '-frounding-math'))]
    source = '''
#include <xmmintrin.h>
using uint32 = unsigned int;
using uint8 = unsigned char;
#define MAX_flt 3.402823466e+38F
struct FMemory { static void Memcpy(void* D, const void* S, __SIZE_TYPE__ N) { __builtin_memcpy(D,S,N); } };
void operator delete(void*) noexcept {}
void operator delete(void*, __SIZE_TYPE__) noexcept {}
struct FGameplayAttributeData {
  FGameplayAttributeData(): BaseValue(0.f), CurrentValue(0.f) {}
  virtual ~FGameplayAttributeData() {}
  virtual void SetCurrentValue(float V) { CurrentValue=V; }
  virtual void SetBaseValue(float V) { BaseValue=V; }
protected:
  float BaseValue, CurrentValue;
};
struct FFortGameplayAttributeData : FGameplayAttributeData {
'''
    source += '\n'.join('  %s %s;' % (t, n) for n, t, _ in policy.FIELDS) + '\n'
    source += '\n'.join(policy.declarations({'cpp_name': 'FFortGameplayAttributeData'})) + '\n};\n'
    source += '\n'.join('static_assert(__builtin_offsetof(FFortGameplayAttributeData, %s)==%d,"offset");' % (n, o)
                        for n, _, o in policy.FIELDS)
    source += '''
static_assert(sizeof(FFortGameplayAttributeData)==40,"size");
static_assert(alignof(FFortGameplayAttributeData)==8,"align");
// Preserve caller's original vptr and all tail padding. Copy every data member plus interior padding.
extern "C" void Mutate(void* Data, uint32 Bits, int Base) {
  FFortGameplayAttributeData Object;
  __builtin_memcpy(reinterpret_cast<char*>(&Object)+8,reinterpret_cast<char*>(Data)+8,28);
  float Value;
  __builtin_memcpy(&Value,&Bits,4);
  FGameplayAttributeData& Parent=Object;
  if(Base) Parent.SetBaseValue(Value); else Parent.SetCurrentValue(Value);
  __builtin_memcpy(reinterpret_cast<char*>(Data)+8,reinterpret_cast<char*>(&Object)+8,28);
}
extern "C" uint32 ReadMxcsr() { uint32 V; __asm__ volatile("stmxcsr %0":"=m"(V)); return V; }
extern "C" void WriteMxcsr(uint32 V) { __asm__ volatile("ldmxcsr %0"::"m"(V)); }
'''
    src = out / 'compiled-declarations-harness.cpp'
    src.write_text(source)
    values = [0, 0x80000000, 0x3f800000, 0xbf800000, 0x42c80000, 0x7f7fffff, 0xff7fffff,
              0x7f800000, 0xff800000, 0x7fc12345, 0x7fa12345, 1, 0x80000001,
              0x007fffff, 0x807fffff, 0x00800000, 0x80800000]
    bounds = [(0xff7fffff,0x7f7fffff), (0,0x42c80000), (0x42c80000,0),
              (0x80000000,0), (0,0x80000000), (0x7fc12345,0x42c80000), (0,0x7fc12345),
              (0x7fa12345,0x42c80000), (0,0x7fa12345), (1,0x007fffff),
              (0x807fffff,0x80000001), (0x00800000,0x3f800000)]
    results = []
    for mode, flags in [('server', relevant), ('fast-math-control', ['-O2','-ffast-math'])]:
        library_path = out / ('compiled-declarations-' + mode + '.so')
        command = [str(COMPILER), '-shared', '-fPIC', '-nostdlib', '-fno-rtti', '-fno-exceptions',
                   '-std=c++14', '-Wno-invalid-offsetof', '-Wl,--no-undefined'] + flags + [str(src), '-o', str(library_path)]
        built = subprocess.run(command, capture_output=True, text=True)
        (out / ('compile-' + mode + '.log')).write_text(built.stdout + built.stderr)
        if built.returncode:
            raise RuntimeError('standalone declaration compilation failed: ' + mode)
        library = ctypes.CDLL(str(library_path))
        mutate = library.Mutate
        mutate.argtypes = [ctypes.c_void_p, ctypes.c_uint32, ctypes.c_int]
        library.ReadMxcsr.restype = ctypes.c_uint32
        library.WriteMxcsr.argtypes = [ctypes.c_uint32]
        saved = library.ReadMxcsr()
        count = 0
        try:
            # Four rounding modes, gradual underflow and DAZ/FTZ control. Mask FP traps.
            for mxcsr in [0x1f80,0x3f80,0x5f80,0x7f80,0x9fc0]:
                library.WriteMxcsr(mxcsr)
                for base, clamp, incoming, old, (minimum, maximum) in itertools.product(
                        [False,True], [False,True], values, values, bounds):
                    data = bytearray([0xa5]*40)
                    struct.pack_into('<IIII',data,8,0x447a0000,0xc47a0000,minimum,maximum)
                    data[24:27] = bytes([1,1,clamp])
                    struct.pack_into('<II',data,28,old,old)
                    wanted = ctypes.create_string_buffer(bytes(data),40)
                    actual = ctypes.create_string_buffer(bytes(data),40)
                    original[base](wanted,incoming)
                    mutate(actual,incoming,base)
                    if actual.raw != wanted.raw:
                        failure = dict(mode=mode,mxcsr=hex(mxcsr),base=base,clamp=clamp,
                                       incoming=hex(incoming),old=hex(old),minimum=hex(minimum),
                                       maximum=hex(maximum),original=wanted.raw.hex(),compiled=actual.raw.hex())
                        (out / ('compiled-failure-' + mode + '.json')).write_text(json.dumps(failure,indent=2)+'\n')
                        raise AssertionError(failure)
                    count += 1
        finally:
            library.WriteMxcsr(saved)
        results.append({'mode':mode,'flags':flags,'cases':count,'matched':True})
    result = {'compiler': str(COMPILER), 'response_file':str(rsp.relative_to(repo)),
              'actual_server_float_flags':relevant,'variants':results,
              'comparison':'all 40 bytes including sibling values, flags, original vptr and padding',
              'mxcsr_controls':['nearest','down','up','toward-zero','DAZ+FTZ'],
              'game_build_or_runtime_mutation':False}
    (out/'compiled-verification.json').write_text(json.dumps(result,indent=2)+'\n')
    return result
