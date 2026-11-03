# UFortItem evolution restriction text candidate

Uncounted draft; do not promote until default-text equivalence is established. The
manifest is intentionally empty. No build or runtime test was performed; the
next project test checkpoint is approximately 3,040 native bodies.

`native94-broad/0x142958df0.c` reads one byte argument and calls
`native94-deps/0x14202d350.c` without a receiver. Reflection in
`generated/reflection/fortnite_game_reflection.json` gives flags `0x14022401`
(including static), FText return and enum parameter `Reason`; enum constructor
`0x1429582b0` identifies `EItemEvolutionRestrictionReason`, whose byte-backed
values 0–6 are listed in `generated/reflection/enum_blocks.json`. Candidate
signature: `static FText GetEvolveRestrictionReasonText(EItemEvolutionRestrictionReason Reason)`.
Both reflected parameter candidates agree on these fields despite the export's
ambiguity note.

The body switches on the raw byte. Each known case calls `0x142bedc40` with
output, source, namespace `FortItem`, and key; source/key strings were decoded
from `/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin`
(base `0x140000000`). Their exact mappings are recorded in the draft. Unknown
values copy the shared result of `0x1407c2c80`, including text flags and shared
reference increment. The old rough draft had source and key address labels
reversed; the actual native argument order above is authoritative.

`generated/callee_probe/extra/0x142bedc40.c` calls the singleton
`0x142c274f0` and `0x142c244c0`. The latter's namespace/key map lookup,
critical sections, source comparison, immutable flag 8 and cache insertion
match the local UE `Core/Private/Internationalization/TextCache.cpp`
`FTextCache::Get` and `FindOrCache`. `Internationalization.cpp` routes the
literal constructor through that cache; `Internationalization.h` defines
`NSLOCTEXT` using the same source, namespace, key order. The known cases are
therefore supported by the candidate.

`native94-text/0x1407c2c80.c` returns a static FText initialized by
`0x142c0c150(&static, 0)`, matching the singleton shape of UE
`Core/Public/Internationalization/Text.h` `FText::GetEmpty`. However, the
initializer body is not in the available evidence: singleton shape alone does
not prove that it initializes the same empty text data and flags. Exact
remaining probe: decompile `0x142c0c150` and compare it with UE
`FText::FText(EInitToEmptyString)` in `Core/Private/Internationalization/Text.cpp`.
