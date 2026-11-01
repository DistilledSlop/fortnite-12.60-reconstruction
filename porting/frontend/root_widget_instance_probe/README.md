# Original Root widget construction probe

The opt-in `-RootWidgetInstanceProbe` waits for the actual startup world, local
player and controller. It loads the original Root class, brackets `CreateWidget`,
retains the resulting instance until observations finish, and checks its class,
outer, world, player, controller and widget tree. It does not attach to the viewport,
force UI states, intercept callbacks or replace the original class.

The verifier requires eight ordered phase rows and rejects Blueprint initialization
errors. A non-null return does not establish a successful Blueprint VM or UI.
The runner preserves the last observed phase and exit/timeout status on failure,
and binds the launch to the built ELF, actual process arguments and working directory.

Run under the same graphics environment as the retained frontend probes:

```
python3 porting/frontend/root_widget_instance_probe/runner.py --build-results BUILD/results.json --binary STAGE/FortniteLinux --cwd STAGE_ROOT --runtime-args material-runtime-args.json --out FRESH_OUTPUT --timeout 240
```

The runtime-args file is a JSON array containing only the material ingress,
manifest and manifest-SHA256 flags from a successful original Root load receipt.
The runner supplies the Entry map and probe flag. It rejects a conflicting
`-FortniteProbeAsset` or `-ExecCmds=Quit` flag. Each output directory must be fresh.
