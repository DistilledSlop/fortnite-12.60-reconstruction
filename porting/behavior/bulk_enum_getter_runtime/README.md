# Enum byte-field getters

`prepare.py` produces isolated proposals for original zero-argument enum getters. It pins the complete original runtime image SHA-256 and matches the full exec through `ret`, either an inline byte load and result store or the exact single-call wrapper and complete byte-load leaf. Branches, partial wrappers, and additional effects are excluded.

The parameter table must split into exactly one Enum return and its zero-flag Byte underlying record using the generator's `split_properties` and `enum_underlying_record`. A unique reflected enum field at the original byte offset must be accessible through verified generated-class ancestry, with matching enum identity, generator-resolved declaration, zero-flag Byte underlying record, and generated `enum class : uint8` declaration. No new native storage or raw field casts are proposed. Existing canonical and maintained bodies are excluded; the generated signature and any existing declaration are checked; each body returns the typed field directly.

Run `python3 -I porting/behavior/bulk_enum_getter_runtime/prepare.py`. The default output is `/tmp/gameplay-bulk-enum-getter/{manifest-entries,report}.json`; `--output-dir` changes it, and `--revalidate-own` includes canonical entries owned by this directory. Differing existing snippets stop the scan. Proposals and snippets do not modify the canonical manifest or Native. Source recovery is not build or runtime validation; reflected field defaults and writers are outside this scan.

Integrated in native source batch 15: 31 bodies; compilation and runtime checks remain pending at approximately 2,040 bodies.
