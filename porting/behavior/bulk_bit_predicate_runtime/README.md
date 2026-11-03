# Normalized bit predicates

`prepare.py` scans original executable thunks for complete normalized single-bit Boolean reads, either inline or through a fully matched leaf wrapper. It requires a unique same-class reflected Boolean field and generated Boolean declaration. Proposals and evidence are written to the selected output directory; existing canonical ports are excluded. Use `--revalidate-own` to include this pass's canonical entries when replaying it.
