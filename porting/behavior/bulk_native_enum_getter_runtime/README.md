# Native byte-backed enum getters

Run `python3 -I porting/behavior/bulk_native_enum_getter_runtime/prepare.py` to
write source proposals and a scratch manifest and evidence report to
`/tmp/gameplay-bulk-native-enum-getter/`. The report records decoded wrappers
and leaves, enum identity and class-size GPSC evidence. The scanner pins and
hashes the runtime image, requires an underlying-byte enum return and its exact
generated `enum class ... : uint8` definition in the matching package, and
matches only complete zero-argument byte reads and returns.

Both class and parent size require unique authoritative GPSC extraction. The
byte must lie within the declaring class and outside conservative reflected and
canonical member occupancy, except for one exact compatible canonical `uint8`
interval. New fields use `uint8`, with `static_cast` to the exact enum so every
byte value survives. Initialization, mutation and ownership are not inferred.
These are source proposals, not runtime or build validation. The latest scan
found five getters and five new fields; promotion is owned separately.

Integrated in native source batch 16: five bodies and five uint8 backing fields. Builds and runtime checks remain pending at approximately 2,040 bodies.
