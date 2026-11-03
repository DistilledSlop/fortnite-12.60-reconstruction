# FName getter proposals

`prepare.py` scans the native function inventory against the pinned original image for complete zero-argument FName field-copy getter thunks or wrappers calling complete leaf getters. It requires selected, unique FortniteGame/FortniteUI non-interface classes, matching reflected Name return and declared field, offset, eight-byte copy, and generator-resolved FName types. It excludes already-ported functions. Only recognized complete instruction sequences are accepted; constants, globals, constructor calls and inherited fields are not inferred.

The script writes guarded `.inc` source proposals here and a scratch report and proposed manifest entries to `/tmp/gameplay-bulk-name/` by default. `--output-dir` changes the report destination; `--revalidate-own` permits rechecking entries already ported from this directory. It does not change the canonical manifest or generated code.
