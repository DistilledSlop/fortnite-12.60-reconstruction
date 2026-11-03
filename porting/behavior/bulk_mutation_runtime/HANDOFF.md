# Mutation sweep handoff

`python3 porting/behavior/bulk_mutation_runtime/prepare.py` completed: five proposals, image digest pinned, source generation only; outputs `/tmp/gameplay-bulk-mutation/{manifest-entries,report}.json`. Five generated `.inc` bodies cover exact immediate stores to reflected integer or float fields. Full-body match, parent metadata and field/header checks are recorded by the scanner. No tests/builds were run under the requested batch cadence; no canonical manifest or Native files changed.

Eight matches remain blocked by absent/ambiguous reflected whole scalar storage, including bitwise mutations. A useful next pass is extraction-aware classification of parameter-bearing setter thunks, matching the full FFrame extraction and complete mutation before source credit. Also recover native member layout for the excluded direct-store offsets. The orchestrator owns promotion and verification.
