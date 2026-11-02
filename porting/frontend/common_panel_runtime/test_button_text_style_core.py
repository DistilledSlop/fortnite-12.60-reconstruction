import hashlib
import json
import subprocess
import tempfile
import unittest
from pathlib import Path

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[1]
IMAGE = Path('/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin')

CPP = r'''
#include "CommonButtonTextStyleCore.h"
#include <cassert>
#include <map>
#include <string>
#include <vector>
using namespace CommonButtonTextStyleCore;
struct Object { int actual; };
struct Fixture {
 std::vector<std::string> trace;
 bool owner=true, enabled=true, selected=false;
 std::vector<bool> hovers; unsigned hi=0;
 std::map<Candidate,Object*> candidates;
 int run() {
  return Get([&]()->Fixture* {trace.push_back("owner"); return owner?this:nullptr;},
   [&]{trace.push_back("enabled"); return enabled;},
   [&]{trace.push_back("selected"); return selected;},
   [&]{trace.push_back("hover"); return hovers.at(hi++);},
   [&](Fixture* s,Candidate c)->Object* { assert(s==this); trace.push_back("try:"+std::to_string(int(c))); return candidates[c]; },
   [&](Object* o){trace.push_back("actual"); return o->actual;});
 }
};
int main() {
 Object normal{101}, hovered{102}, selected{103}, selectedHover{104}, disabled{105};
 Fixture a; a.owner=false; assert(a.run()==0); assert((a.trace==std::vector<std::string>{"owner"}));
 Fixture b; b.enabled=false; b.candidates[Candidate::Disabled]=&disabled; assert(b.run()==105); assert((b.trace==std::vector<std::string>{"owner","enabled","try:4","actual"}));
 Fixture c; c.enabled=false; c.hovers={true}; c.candidates[Candidate::NormalHovered]=&hovered; assert(c.run()==102); assert((c.trace==std::vector<std::string>{"owner","enabled","try:4","hover","try:1","actual"}));
 Fixture d; d.selected=true; d.hovers={true}; d.candidates[Candidate::SelectedHovered]=&selectedHover; assert(d.run()==104); assert((d.trace==std::vector<std::string>{"owner","enabled","selected","hover","try:3","actual"}));
 Fixture e; e.selected=true; e.hovers={false}; e.candidates[Candidate::Selected]=&selected; assert(e.run()==103); assert((e.trace==std::vector<std::string>{"owner","enabled","selected","hover","try:2","actual"}));
 Fixture f; f.selected=true; f.hovers={true,false}; f.candidates[Candidate::Normal]=&normal; assert(f.run()==101); assert((f.trace==std::vector<std::string>{"owner","enabled","selected","hover","try:3","try:2","hover","try:0","actual"}));
 Fixture g; g.hovers={true}; assert(g.run()==0); assert((g.trace==std::vector<std::string>{"owner","enabled","selected","hover","try:1","try:0"}));
 // Candidate resolution reenters and changes hover before the second query.
 Fixture inner; inner.hovers={false}; inner.candidates[Candidate::Normal]=&normal;
 std::vector<std::string> trace; bool hover=false;
 int result=Get([&]()->int* {trace.push_back("owner"); static int x; return &x;},
  [&]{trace.push_back("enabled");return true;}, [&]{trace.push_back("selected");return true;},
  [&]{trace.push_back("hover");return hover;},
  [&](int*,Candidate c)->Object* { trace.push_back("try:"+std::to_string(int(c))); if(c==Candidate::Selected) {assert(inner.run()==101); hover=true;} return c==Candidate::NormalHovered?&hovered:nullptr;},
  [&](Object* o){trace.push_back("actual");return o->actual;});
 assert(result==102); assert((trace==std::vector<std::string>{"owner","enabled","selected","hover","try:2","hover","try:1","actual"}));
 assert((inner.trace==std::vector<std::string>{"owner","enabled","selected","hover","try:0","actual"}));
 // Selection is sampled once, even if selected-hovered lookup changes it.
 trace.clear(); bool selectedState=true; int selectionReads=0;
 result=Get([&]()->int* {static int x; return &x;}, []{return true;},
  [&]{++selectionReads;return selectedState;}, []{return true;},
  [&](int*,Candidate c)->Object* {
   trace.push_back("try:"+std::to_string(int(c)));
   if(c==Candidate::SelectedHovered) selectedState=false;
   return c==Candidate::Selected?&selected:nullptr;
  }, [](Object* o){return o->actual;});
 assert(result==103 && !selectedState && selectionReads==1);
 assert((trace==std::vector<std::string>{"try:3","try:2"}));
}
'''

class TextStyleCoreTests(unittest.TestCase):
 def test_compiled_core(self):
  with tempfile.TemporaryDirectory() as tmp:
   source = Path(tmp) / 'test.cpp'
   source.write_text(CPP)
   binary = Path(tmp) / 'test'
   subprocess.run(['c++', '-std=c++17', '-Wall', '-Wextra', '-Werror', '-I', str(HERE), str(source), '-o', str(binary)], check=True)
   subprocess.run([str(binary)], check=True)

 def test_original_evidence(self):
  evidence = json.loads((HERE / 'common-button-text-style-evidence.json').read_text())
  with IMAGE.open('rb') as f:
   h = hashlib.sha256()
   for chunk in iter(lambda: f.read(4 * 1024 * 1024), b''): h.update(chunk)
   self.assertEqual(h.hexdigest(), evidence['image_sha256'])
   for name, window in evidence['windows'].items():
    f.seek(int(window['va'], 16) - int(evidence['image_base'], 16))
    self.assertEqual(hashlib.sha256(f.read(window['size'])).hexdigest(), window['sha256'], name)
  reflection = ROOT / 'generated/reflection'
  getter = json.loads((reflection / 'native_funcs.json').read_text())['UCommonButton']['GetCurrentTextStyleClass']
  for key, value in evidence['getter'].items(): self.assertEqual(getter[key], value)
  self.assertEqual(len(getter['params']), 1)
  p = getter['params'][0]
  self.assertEqual((p['name'], p['kind'], p['offset'], p['type_refs']['meta_class']), ('ReturnValue', 'Class', 0, evidence['meta_class']))
  classes = json.loads((reflection / 'fortnite_game_reflection.json').read_text())['classes_in_groups_with_fort_names']
  block = json.loads((reflection / 'native_class_index.json').read_text())['UCommonButtonStyle']['block_va']
  self.assertEqual(block, '0x145a007f0')
  style = next(c for c in classes if c['block_va'] == block)
  properties = {p['name']: p for p in style['properties']}
  for name, offset in evidence['property_offsets'].items():
   p = properties[name]
   self.assertEqual((p['offset'], p['kind'], p['type_refs']['meta_class']), (offset, 'Class', evidence['meta_class']))

if __name__ == '__main__': unittest.main()
