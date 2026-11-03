# Two-input direct leaf recovery

`python3 -I porting/behavior/bulk_two_scalar_runtime/prepare.py` checks the SHA-pinned original runtime image, reflection, generator signature and generated declarations and writes source-only proposals and whole-instruction evidence to `/tmp/gameplay-bulk-two-scalar/{manifest-entries,report}.json`. The implemented family requires the complete exact two-object FFrame thunk, both branch destinations and join points, and a leaf containing exactly two pointer stores and return. Parameter order follows reflected parameter offsets. Both stored fields must be uniquely resolved reflected object fields with exactly the corresponding parameter type and accessible generated declarations. The scan found two proposals; neither has been built or runtime tested.

`probe.py` is a wider non-proposing inventory: it found eight small complete leaves among 553 eligible records, including two bool-byte stores and two byte ORs whose offsets do not identify reflected fields; the float leaf also writes an unreflected byte. The unselected Hoagie class and unreflected or otherwise unmatched fields are not credited. The probe's call-pattern sightings do not prove complete wrappers and cannot alone justify proposals.

Integrated in native source batch 18: 2 bodies. Compilation and runtime checks remain pending at approximately 2,040 bodies.
