import hashlib
import json
import subprocess
import tempfile
import unittest
from pathlib import Path

HERE = Path(__file__).resolve().parent
IMAGE = Path('/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin')
CPP = r'''
#include "CommonActionKeyLookupCore.h"
#include <cassert>
#include <vector>
using namespace CommonActionKeyLookupCore;
struct Key { unsigned index, number, other; };
struct Entry { Key key; int brush; };
struct Set { std::vector<Key> keys; int brush; };
struct Controller { unsigned char type, pad; std::vector<Entry> map; std::vector<Set> sets; };
struct Adapter {
 std::vector<Controller*> slots; std::vector<int> resolved;
 std::size_t SlotCount() { return slots.size(); }
 Controller* ResolveSlot(std::size_t i) { resolved.push_back(int(i)); return slots[i]; }
 unsigned char InputType(Controller* c) { return c->type; }
 unsigned char GamepadType(Controller* c) { return c->pad; }
 std::size_t MapCount(Controller* c) { return c->map.size(); }
 Key MapKey(Controller* c, std::size_t i) { return c->map[i].key; }
 int* MapBrush(Controller* c, std::size_t i) { return &c->map[i].brush; }
 std::size_t SetCount(Controller* c) { return c->sets.size(); }
 std::size_t SetKeyCount(Controller* c, std::size_t i) { return c->sets[i].keys.size(); }
 Key SetKey(Controller* c, std::size_t i, std::size_t j) { return c->sets[i].keys[j]; }
 int* SetBrush(Controller* c, std::size_t i) { return &c->sets[i].brush; }
 unsigned NameIndex(Key k) { return k.index; }
 unsigned NameNumber(Key k) { return k.number; }
};
int main() {
 Key a{1,0,9}, a_other{1,0,99}, a_number{1,1,9}, b{2,0,9}, c{3,0,9};
 Controller wrong{1,2,{},{}}, first{1,3,{{a,10},{a_other,11}}, {{{a,b},20},{{b,a},21},{{a},22}}};
 Controller second{1,3,{{c,30}}, {{{a,c},31}}};
 Adapter x{{nullptr,&wrong,&first,&second},{}};
 auto run=[&](std::vector<Key> keys) { x.resolved.clear(); return Find(x,keys,1,3); };
 assert(*run({a_other})==10); assert((x.resolved==std::vector<int>{0,1,2}));
 assert(!run({a_number})); assert(!run({c})); assert((x.resolved==std::vector<int>{0,1,2}));
 assert(*run({b,a})==20); assert(*run({a,a})==20); assert(!run({a,c}));
 assert(!run({a,b,c})); assert(!run({})); assert((x.resolved==std::vector<int>{0,1,2}));
 Controller keyboard{0,9,{{a,40}}, {}}; Adapter y{{&keyboard},{}};
 assert(*Find(y,std::vector<Key>{a},0,1)==40); // gamepad ignored for other types
 Adapter z{{nullptr,&wrong},{}}; assert(!Find(z,std::vector<Key>{a},1,3));
}
'''

class LookupTests(unittest.TestCase):
 def test_compiled_core(self):
  with tempfile.TemporaryDirectory() as tmp:
   source = Path(tmp) / 'test.cpp'
   source.write_text(CPP)
   binary = Path(tmp) / 'test'
   subprocess.run(['c++', '-std=c++17', '-Wall', '-Wextra', '-Werror', '-I', str(HERE), str(source), '-o', str(binary)], check=True)
   subprocess.run([str(binary)], check=True)

 def test_original_evidence(self):
  evidence = json.loads((HERE / 'common-action-key-lookup-evidence.json').read_text())
  with IMAGE.open('rb') as f:
   self.assertEqual(hashlib.file_digest(f, 'sha256').hexdigest(), evidence['image_sha256'])
   for name, window in evidence['windows'].items():
    f.seek(int(window['va'], 16) - int(evidence['image_base'], 16))
    self.assertEqual(hashlib.sha256(f.read(window['size'])).hexdigest(), window['sha256'], name)

if __name__ == '__main__': unittest.main()
