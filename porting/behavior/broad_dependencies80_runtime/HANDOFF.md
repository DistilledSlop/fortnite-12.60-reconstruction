# Broad dependency source recovery80

Five uncounted actual helper drafts:141d507d0 first matching named variant;
141e5f200 conditional classification forwarding;142040900 fallback item creation;
141ed2350 engine selection/failure recovery;142706020 configured string/character
filtering followed by text construction, consuming the input allocation.

Root reviewed all five complete C sources. A bounded assembly read at
142040900 CALL142ed7360..RET142040958 confirms the final allocator RAX survives
the epilogue despite Ghidra declaring the function void. The invalid-engine
constructor argument preserves the native literal pointer's high32 bits.
Filtering retains the source's decrement-and-retry character-removal back edge,
including its unusual negative-index possibility; do not normalize it without
resolving the instructions. Static empty text uses its original guarded epoch,
shared strong increment and20-byte copying. Calls, literals, layouts, ownership,
allocators, text operations and hidden ABI remain adapters, not engine bindings.
No compiler, tests, runtime, canonical generation or promotion. Source hashes in
sources.json; root native-broad80 probe succeeded/rolled back.
