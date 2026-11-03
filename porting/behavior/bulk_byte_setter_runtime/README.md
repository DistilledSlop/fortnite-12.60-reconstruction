# Bool setter recovery

`python3 porting/behavior/bulk_byte_setter_runtime/prepare.py` scans the pinned runtime image and reflection inventory, writing proposed `.inc` files here and evidence to `/tmp/gameplay-bulk-byte-setter/{manifest-entries,report}.json`. It requires the exact complete one-bool FFrame extraction, branch targets, stack/register consistency, a complete pure byte-store leaf, and a unique matching reflected and accessible generated C++ `bool` field. Other storage forms, unresolved fields, and writers with side effects are rejected. The SHA-256 of the input image is checked before decoding. Existing canonical files are not edited.

The current scan produces zero proposals: 29 pure-store thunks have no corresponding uniquely reflected field at the store offset, and 479 call writers outside the pure-store shape. No storage or side effects are inferred. No build or runtime test was performed.
