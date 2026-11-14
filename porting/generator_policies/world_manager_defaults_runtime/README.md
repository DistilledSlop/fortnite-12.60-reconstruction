# World manager reflected constructor defaults

This policy merges assignments into the existing native default-subobject
constructor. It consumes the 19 scalar defaults already in
`native/Config/OriginalDefaults.json` and the missed stores in `recovered.json`.
It runs for the CDO and each constructed instance, before level serialization
can override the values. It does not change the global CDO defaults loader.

The scalar source is `tools/ctor_defaults.py --class AFortWorldManager`; its
original constructor is `0x141f7c920`. The register read/modify/write sequence
at `0x141f7cc25..0x141f7cc5c` is not recognized by that scanner. It sets the
saving and streaming bits, and clears the construct and sim-map bits. Masks
are checked against `generated/reflection/bool_bits.json`. The JSON also records
IEEE-754 bits for the two FVector properties and the wind struct. The vector
constants originate from the RIP-relative loads at `0x141f7cca3`,
`0x141f7ccab`, `0x141f7cccd` and the immediate at `0x141f7ccdc`; wind values
are the final overwriting stores at `0x141f7cd7e` and `0x141f7cd88`.

Wire `finalize(model, header, cpp)` after the default-subobjects policy. The
merge requires exactly its `MySceneComponent` constructor; changed declarations,
bit metadata, registry keys, wind fields, or constructor fail generation. This
restores the identified reflected stores, not the entire native constructor or
streaming lifecycle. Validate generated code and live map instances after wiring.
