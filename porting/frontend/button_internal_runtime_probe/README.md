# Button internal diagnostic transcript verifier

From the repository root, run:

```sh
python3 porting/frontend/button_internal_runtime_probe/verifier.py path/to/runtime.log --output path/to/result.json
python3 -m unittest discover -s porting/frontend/button_internal_runtime_probe -p 'test_*.py'
```

The verifier requires exactly 34 `BTNINTERNAL` version 1 rows, in the defined
order, with consecutive sequence numbers and `ok=1` for every row. Unrelated log
lines are allowed. Malformed, duplicate, missing, extra, reordered, or failed
marker rows fail verification. The JSON report records the result; the command
exits nonzero on a verification failure. The tests use synthetic transcripts.

A passing transcript establishes only that the supplied text matches the
controlled `UCommonButtonInternal` diagnostic contract. This tool does not bind
the log to a process, establish which executable or sources produced it, verify
the build, or prove full UI behavior or product readiness. Build and process
provenance must be established independently before treating a transcript as
runtime evidence.
