# Original client on an X11 display

`run_original_client.sh` starts the original client and the local backend in a
private network namespace. To show the client on an existing X11 display, run
from this directory in a graphical session:

```bash
FN_DISPLAY="$DISPLAY" FN_XAUTHORITY="${XAUTHORITY:-$HOME/.Xauthority}" \
FN_PATCH_APPES_DIAGNOSTIC=1 ./run_original_client.sh 3600
```

The number is the maximum session duration in seconds. Omit `FN_DISPLAY` to
use a private virtual display for automated probes. `--username NAME` before
the duration selects a local profile. The default Wine prefix is
`porting/generated/wine-prefix`; set `FN_WINEPREFIX` to use a prepared prefix.
Optional `FN_WINEDLLOVERRIDES` and `FN_VK_ICD_FILENAMES` select renderer
settings. For example, the software-rendering probe used a prefix with DXVK
installed, `FN_WINEDLLOVERRIDES='d3d11,dxgi=n'`, and
`FN_VK_ICD_FILENAMES=/usr/share/vulkan/icd.d/lvp_icd.json`. DXVK and Vulkan
must already be installed for those settings; software rendering is slow.

The runner requires Wine, X11 tools, Python, and passwordless `sudo` for the
isolated network namespace and runtime patches. Its TLS patch disables client
certificate verification in memory for the local backend. The AppES diagnostic
patch suppresses an unsafe-environment handler and an exit callback in memory;
its consequences are not fully understood. Both are temporary modifications
to the running process, not changes to the executable on disk. The diagnostic
is currently necessary for a persistent tested session. See
[LAUNCHER_PROBE.md](LAUNCHER_PROBE.md) for the supplied-launcher result.
