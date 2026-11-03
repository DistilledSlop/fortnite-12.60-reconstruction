# Alternate forward-wrapper scan

`prepare.py` checks the pinned runtime image for zero-argument scalar return wrappers that advance the script frame using `rcx`, call a direct leaf, and transfer the complete scalar result. Because that frame sequence overwrites `rcx`, the scan accepts only leaf constants independent of `this`. It emits a scratch proposal and report under `/tmp/gameplay-bulk-forward` and does not edit the canonical manifest. The present scan produced no candidates in the selected class inventory. Branching and call-containing leaves remain unresolved; they are not counted.
