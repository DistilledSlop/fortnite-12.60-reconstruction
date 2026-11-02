# Internal button minimum size and non-creating style lookup

Native size adapters extract the two operations from original size refresh
`0x1407ec1c0`: write each signed integer field, then, if the actual SBox exists,
set its minimum desired dimension to an explicitly set float. These are project-local
C++ names and add no reflected functions. Original callers must still resolve the
weak internal button independently for each dimension and honor its valid-object
precondition. Complete CommonButton style/lifecycle integration remains unfinished.

Original rebuild copies width/height arguments +210/+228 to SBox +338/+350;
update setters `0x1430ffa30`/`0x1430ff980` target those same members. Local SBox
construction names them MinDesiredWidth/Height, distinct from overrides. Source and
instruction replay: `/mnt/vibrato/fortnite-work/internal-button-size-update-candidate-1/verify.py`.
Full caller fragments: `common-button-set-style-contract-1/`.

Original button-style owner resolver `0x1407c5240` reads ClassDefaultObject at
`0x1407c52b8`. Text selector `0x1407c2170` reads candidate CDOs directly at
`0x1407c221e`, `0x1407c22fc`, `0x1407c239e`, `0x1407c2454`, `0x1407c24f6`, with
null/type checks. Text-block apply helper `0x140802830` likewise reads at
`0x1408028b9`, then checks null at `0x1408028c0` before validating the CDO type.
These paths do not create missing CDOs. Existing creating lookups are corrected to
GetDefaultObject(false). Original image SHA256 is
338c83a2314ccac1040a2c0bb520b893013cc7b58eda58e550da67f5d84de680.
Complete selector proof is in `button-style-rebuild-candidate-1/verify.py`;
text-block proof is in `common-text-block-set-style-contract-1/` and the bounded
`text-block-cdo-source-audit-1/` receipt. These are static original-code proofs.

The engine at revision87acab901 provides a pure read in UClass::GetDefaultObject(false).
The fixture sequence NewObject<UClass> -> SetSuperStruct -> Bind -> StaticLink
also appears in Engine/Source/Editor/DetailCustomizations/Private/ConfigEditorPropertyDetails.cpp,
before its separate explicit GetDefaultObject(true). Local Class.cpp Bind/Link and
SetSuperStruct do not create a CDO. Strong transient class fixtures retain absent
CDOs through direct/reflected getter calls; existing global CDO pointers are never
removed. Text style fields are restored before garbage collection.

Runtime probes exercise real SBox desired size with an actual USizeBox child,
independent dimensions, negative minima, int-to-float rounding and release/rebuild;
these checks do not establish arranged viewport geometry or original pixels.
