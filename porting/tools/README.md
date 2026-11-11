# Runtime-image lookup tools

Small standard-library scripts for the decrypted client image
(`porting/generated/dump/FortniteClient-Win64-Shipping.runtime.exe`; file offsets equal RVAs). Run with `python3 -I`.

| Script | Use |
|---|---|
| `strx.py "text" ...` | Find ASCII/UTF-16 strings containing the text and the code that loads them: prints string address, reference address and the `.pdata` chunk start. `--secs` lists sections |
| `strs_near.py 0xA 0xB` | Dump UTF-16 strings in an address range (log strings of one source file sit together) |
| `vt.py 0xFUNC` | Find vtables that contain a function pointer and print the neighbouring slots |
| `nameidx.py out.json` | Build the name index: every function that loads a `Class::Function` log string. Output is `server_scope/string_names.json` |

Notes:
- `.pdata` splits large functions into chunks. A reported "function start" can be a chunk in the middle of a function; the real start is one of the few preceding chunk starts. Never measure function size from one `.pdata` entry.
- To decompile: `/opt/fortnite/local-model-package/scripts/evidence.py capture --input <json list of addresses inside the package dir>`; output in `captures/<hash>/<addr>.{c,asm,json}`.
- `server_scope/string_names.json`: address (chunk start) → up to four `[class, function, count]` guesses. 2,536 functions are named this way. A name comes from a log or ensure string inside the function, so it is strong evidence but can be the caller's name when a helper was inlined.
