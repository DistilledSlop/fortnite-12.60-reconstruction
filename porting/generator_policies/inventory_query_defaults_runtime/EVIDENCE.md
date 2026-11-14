# Inventory query constructor defaults

Original UFortItemDefinition constructor `1420241d0` ORs 0x0c into +0x57 at `14202463f`; UFortWorldItemDefinition constructor `142042090` ORs 4 at `14204277a`. Both explicitly enable reflected bAllowMultipleStacks (bit 2). UFortAmmoItemDefinition constructor `142622f70` stores word 1 at +0x7c8 at `142623163`, enabling the full-byte bIsConsumed. These are the real shared reflected fields consumed by the two recovered virtual queries; asset serialization can override their native defaults.

The policy preserves existing constructor bodies and projects only these proven fields, rejecting competing initialization or altered class/field identity. Other constructor effects remain separately incomplete. Original flat-image binary windows and standard function capture `1e4ccba90c3a4142ac8c2daae0ca1bb2` are recorded in ignored `generated/agents/reload-inventory209/`, independently reviewed by `inventory-leaf-review216`.
