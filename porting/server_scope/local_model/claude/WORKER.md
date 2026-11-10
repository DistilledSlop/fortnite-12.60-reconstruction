# Worker instructions (haiku port workers)

You port Fortnite 12.60 native functions to C++ source bodies. Work dir: /opt/fortnite/local-model-package. Your chunk is a JSON list in work/claude/chunks/NN.json: [{"function":"Class::Fn","exec":"0xADDR"}]. Do ONLY your chunk. Write ONLY under work/claude/out/ . Do not touch git, the checkout, other files, or the inventory state.

For every function you MUST produce a complete body. There is NO "blocked", "unresolved" or "skipped" result. If something is unknown, decompile it and keep going.

## Method per function
1. Find the decompile: `ls captures/*/<exec>.c` (already captured; do not run any capture for the exec). Read it.
2. Exec thunks are wrappers. `*(param_2+0x20) = ... ` is just P_FINISH, ignore. Parameters come from the FFrame; the result is written through param_3. Real behaviour is either inline, a direct `FUN_14xxxxxxx(...)` call, or a virtual call `(**(code **)(*param_1 + OFF))(...)`.
3. For a direct callee `FUN_x`: capture it. Write its address into a JSON list file inside the package, e.g. work/claude/out/<chunk>-req.json = ["0x14abc..."], then run `python3 -I scripts/evidence.py capture --input work/claude/out/<chunk>-req.json` (path must be inside the package; ~1s each). Read the new `captures/*/<addr>.c`. Follow callees until you understand it; named engine helpers (FMemory_Free, FWeakObjectPtr_Get, *_StaticClass, FScalableFloat_GetValueAtLevel ...) need no further reading.
4. Names/types: `python3 -I scripts/inventory_query.py show Class::Fn` (parameter names, kinds, flags), `python3 -I scripts/evidence.py field-offset Class 0xOFFSET` (member names at an offset through parents), `python3 -I scripts/evidence.py query function|class|member TERM`. For virtual slots use /opt/fortnite/FortniteClient-12.60-CL-13477524/porting/virtual_slots.py or porting/behavior/vtable_lookup.py <class> <hexoffset> (run from /opt/fortnite/FortniteClient-12.60-CL-13477524). Existing ported examples to imitate: /opt/fortnite/FortniteClient-12.60-CL-13477524/porting/behavior/<Class>/<Fn>.inc and MANIFEST.json.
5. Write `work/claude/out/<Class>__<Function>.inc`: C++ STATEMENTS ONLY (a function body without braces), real source-level code using member names and UE API, no raw pointer arithmetic / FFrame offsets / vtable-slot casts. First lines: `// CONFIDENCE: HIGH|MEDIUM|LOW ...` and the original addresses. A function whose true body is empty or returns a constant is fine; say why in a comment.
6. Write `work/claude/out/<Class>__<Function>.evidence.json`: {"function","status":"proposed","original":"0x..","signature":"...","captures":[paths],"callees":{addr:name_or_description},"notes":""}. Unnamed callees: describe what they do and, if reusable, list a proposed name.
7. If a helper/member/virtual declaration is needed, write it to work/claude/out/deps/<name>.json ({"name","address","signature","inc_or_decl","evidence"}).

When all functions in your chunk have a .inc and .evidence.json, reply with one line per function: name and one-sentence summary. Be economical: read bounded line ranges (head/sed), not huge outputs.
