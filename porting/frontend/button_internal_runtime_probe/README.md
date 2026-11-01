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

## Launching a staged diagnostic

With a successful `FortniteLinux` build results JSON and a freshly staged client,
launch each case into its own previously nonexistent output directory. For example:

```sh
env DISPLAY=:99 VK_ICD_FILENAMES=/path/to/icd.json xvfb-run -a \
  python3 porting/frontend/button_internal_runtime_probe/runner.py \
  --build-results /path/to/results.json --binary /path/to/stage/FortniteLinux \
  --cwd /path/to/stage --out /path/to/new-output --case button --timeout 240
```

The results JSON is a list of build rows with `target`, `exit`,
`source_tree_unchanged`, `source_head`, and `binary` (`path`, `sha256`, `size`).
The runner uses a fixed command, checks the staged ELF against the successful
client build before and after launch, inspects the actual process through
`/proc`, and saves a receipt and full combined output log. Button runs also
require the transcript verifier to pass. For `activation`, `registry`, and
`lifecycle`, a passing receipt only establishes the process binding and exit;
validate each transcript separately. Failures and timeouts after launch retain
the log and receipt. Run under `xvfb-run` externally so the child is the ELF
itself. The recorded build result and source head are limited provenance, not
an exhaustive toolchain or link closure, and do not establish full UI behavior.

## MainButton query diagnostic

Use `--case main-button` with the runner to launch the fixed
`-MainButtonStateProbe` case and record its transcript verification in the
receipt and `verification.json`. It can also be checked independently:

```sh
python3 porting/frontend/button_internal_runtime_probe/main_button_verifier.py path/to/runtime.log --output path/to/result.json
```

The verifier requires the 44 ordered `MAINBTN` rows: begin, defaults, metadata,
40 visibility and native flag combinations, and complete. It checks all three
query results and every success flag against the controlled fixture's expected
values. The fixture checks defaults and UFunction metadata and invokes the
actual UFunction via `ProcessEvent`, alongside direct and alias queries. A
matching transcript alone does not establish its provenance. This controlled
query and native-flags fixture does not establish the state producer, the whole
constructor or `Initialize`, button styling, or acceptance of the main UI.

## Panel intro diagnostic

Use `--case panel-intro` to launch `-PanelIntroProbe`; the runner checks its
ordered transcript and records the result in the receipt and `verification.json`.
It can also be checked independently:

```sh
python3 porting/frontend/button_internal_runtime_probe/panel_intro_verifier.py path/to/runtime.log --output path/to/result.json
```

The controlled fixture checks defaults and metadata, completion and activation
callback order, deferred requests, reentry, and input-method notifications. It
uses C++ `ProcessEvent` interception to defer intro and observe events; this does
not establish Blueprint VM behavior, authentic UI behavior, or deactivation.
The transcript alone does not establish its provenance.

## Panel outro diagnostic

Use `--case panel-outro` to launch `-PanelOutroProbe`; the runner checks its
60 ordered transcript rows and records the result in the receipt and
`verification.json`. It can also be checked independently:

```sh
python3 porting/frontend/button_internal_runtime_probe/panel_outro_verifier.py path/to/runtime.log --output path/to/result.json
```

The controlled fixture checks activation and deactivation order, deferred and
repeated pending outro requests, listener removal, held-action cleanup, and a
manager swap during the progress callback. The fixture uses C++ `ProcessEvent`
interception and does not establish Blueprint VM behavior, authentic UI, or
full interface routing. A matching transcript alone does not establish its
provenance.
