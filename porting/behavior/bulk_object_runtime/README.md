# Bulk object-pointer getter proposals

`python3 porting/behavior/bulk_object_runtime/prepare.py` scans the pinned original
runtime image and reflected native inventory once. It emits individual source
bodies here, and a proposal manifest plus detailed evidence under
`/tmp/gameplay-bulk-object/`. `--output-dir PATH` changes the report destination;
`--revalidate-own` includes entries already ported from this directory. Existing
bodies must match exactly or the scan stops.

Only complete native thunks with a single qword object pointer read and return
store, or a complete matching wrapper and leaf, qualify. The return and
same-class field must have identical reflected Object type references and array
shape; the generated field declaration and the generator's Resolver must agree
on the pointer spelling. The target must be a known non-interface class. Class
properties represented as `TSubclassOf`, soft/weak/interface values, and all
other return kinds are deliberately excluded. Function flags, selected class,
return flags, access, and constness are checked as in the scalar pass. The report
records the matched instructions and reflected offset. Proposals are source
recovery only; they require integration and validation before runtime claims.
