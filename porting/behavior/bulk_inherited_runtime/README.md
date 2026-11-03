# Inherited scalar getter recovery

Run `python3 -I porting/behavior/bulk_inherited_runtime/prepare.py` to scan the
native inventory once and write proposals under `/tmp/gameplay-bulk-inherited/`.
`--output-dir` selects another scratch destination; `--revalidate-own` rescans
already canonical entries from this directory. The script never edits the
manifest or Native and refuses differing existing body content.

It retains the scalar matcher's pinned-image hash, complete instruction patterns,
return widths, original flags, constness and normalized Boolean restrictions.
It follows only unique selected FortniteGame/FortniteUI original class ancestry
that agrees with current generated C++ inheritance. Exactly one reflected field
must match across the supported chain, and it must belong to an ancestor with
an accessible generated declaration. The returned member is explicitly qualified
by its owner to avoid hiding. Engine ancestors, pointers, enums, references and
constant returns are outside this pass. Proposals require generation and combined
compilation validation; no larger feature/runtime parity is established.
