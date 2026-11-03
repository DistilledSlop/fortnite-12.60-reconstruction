# Bulk TArray count query proposals

```
python3 -I porting/behavior/bulk_array_query_runtime/prepare.py [--revalidate-own] [--output-dir DIR]
```

Needs Capstone at `/tmp/common-input-capstone` and the pinned image (SHA-256
`338c83a2…de680`, base 0x140000000). Writes `.inc` bodies here (refuses to
overwrite a differing body) and `manifest-entries.json` + `report.json`
(per-match assembly/leaf evidence, rejection counts) to `/tmp/gameplay-bulk-array/`.
Never edits the canonical manifest. `--revalidate-own` rescans entries whose
`inc_path` is in this directory.

Accepted: whole handler = recognized exec frame prefix + exact body, or exact
wrapper calling an exact leaf. Int: `mov eax,[rcx+N]` -> `Field.Num()`.
Bool: `[xor eax,eax;] cmp dword [rcx+N],0; sete|setne al` -> `Num()==0` / `!=0`.
Field = same-class reflected Array at offset N-8 with its adjacent inner record,
unique name, one matching `TArray<..> Name;` header line whose element spelling
agrees with the inner kind. Also requires exact return kind/size, no params,
no net/static/event/reference flags, unique selected non-interface FG/FUI class,
and header signature/const consistency. Inherited fields, Set/Map, others rejected.
