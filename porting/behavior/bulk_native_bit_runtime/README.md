# Packed native bit getter recovery

`prepare.py` proposes zero-argument Bool getters only when a complete exec handler, or its exact wrapper and called leaf, reads an own-class byte and normalizes one bit with a right shift and `and 1`, or directly uses `and 1`. It requires consistent reflected class and parent sizes, the generated parent to match, and the byte to be unoccupied by reflected or canonical native fields. Other reads, calls, flags, and uncertain layouts are rejected. Compatible getters sharing a byte use one `uint8` field without a default. Construction and writers remain unported; this is source recovery, not class feature credit.

Run `python3 prepare.py` to write proposals and evidence under `/tmp/gameplay-bulk-native-bit`. `--output-dir` chooses another report directory; `--revalidate-own` includes this pass's existing canonical entries for revalidation. Existing body files must match exactly. The script verifies the source image digest.
