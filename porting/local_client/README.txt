ProjectChrono
=============

Lightweight Windows launcher for the original Fortnite 12.60 CL 13477524 client
and the reconstructed local backend. Source lives in porting/local_client/.
This is an explicitly patched local mode; it does not satisfy the unmodified
client acceptance gate. Native Windows end-to-end compatibility is pending.

Build on Linux with MinGW-w64:
    bash porting/local_client/build.sh

The build output is under porting/generated/local-client-tools/. Put the
ProjectChrono.exe, FortniteLocalHelper.dll and local-server.ini beside the original
installation's FortniteGame directory, or choose the game executable in the UI.
Supply your backend's PUBLIC CA certificate as server-ca.pem. Never distribute
the server's private key. Select the backend IPv4 address and local account,
save the settings, then choose Play. The backend must already be running and
serve HTTPS on port 443 with certificate names matching the original service
hostnames. This launcher does not start or install the backend.

Before launch, ProjectChrono validates the public CA and adds it to the current
Windows user's Trusted Root Certification Authorities store. This is a
persistent trust-store change for that account. Certificate chain, hostname
checks and public-key pin checks remain enabled. The helper replaces one
compiled pin for each of the six known domains with the local CA's public-key
hash, preserving the other pins. Use only a CA controlled by your backend.
The current helper pins the CA key: a separate leaf/intermediate deployment
needs runtime verification before it can be considered supported.

The launcher starts the original shipping executable directly, requests no
EAC/BattlEye wrapper, and loads its helper into that child. The helper changes
only process memory; the game executable on disk is unchanged. It includes the
guarded local AppES exit-callback diagnostic. That diagnostic suppresses an
environment exit; it does not establish successful entitlement verification.
The AppES patch waits until this launch reaches the frontend: changing those
bytes during protected engine startup causes an execution fault. ProjectChrono
adds a unique session token to the command line and requires that token before
the frontend marker in the normal client log. Old logs cannot satisfy setup;
missing or disabled logging fails setup. The normal log location is the current
user's LocalAppData\FortniteGame\Saved\Logs\FortniteGame.log. Custom saved/log
locations are not currently supported. Concurrent clients sharing one Windows
user log are unverified; use separate machines or user profiles for social tests.
Unsupported executable layouts are rejected. Service redirection currently
covers the proven main-module getaddrinfo import and six compiled domain
suffixes; other resolver APIs have not been established.

Use --headless for the console-free automated launch path. Process startup and
helper loading do not themselves prove successful login, lobby or matchmaking.
The launcher waits for explicit helper readiness and reports failed setup.
ProjectChrono-helper.log contains fixed setup diagnostics; use Open log to read
it. Play saves the four settings before launch; closing the launcher leaves
an already running game process open.
The original client files and generated binaries are not committed to Git.

Focused log-reader checks (MinGW-w64 and Wine):
    x86_64-w64-mingw32-gcc -std=c11 -Wall -Wextra -Werror -municode \
        porting/local_client/tests/frontend_log_test.c \
        -o porting/generated/local-client-tools/frontend_log_test.exe -lcrypt32 -lws2_32
    wine porting/generated/local-client-tools/frontend_log_test.exe
These checks use temporary files and cover stale, split, replaced and truncated
logs. The full original-client launch remains a separate runtime check.

Wine launcher (Linux)
---------------------
Build the tools as above, configure local-server.ini and provide the backend's
public server-ca.pem alongside ProjectChrono.exe, and start the HTTPS backend
before launching. Select the original game executable in the launcher if it is
not at the configured path. ProjectChrono performs the CA validation and trust
store update described above. Install 64-bit DXVK into the selected Wine
prefix separately; this script does not install it. The script checks for the
expected DLL paths, but their presence does not prove they contain DXVK.

For the normal rendered client on the current desktop:
    bash porting/local_client/run_wine.sh --prefix /path/to/wine-prefix

The prefix defaults to FN_WINEPREFIX, then WINEPREFIX, then ~/.wine. The tools
folder defaults to porting/generated/local-client-tools; use --tools-dir DIR
for another built and configured copy. The script sets Wine's d3d11 and dxgi
overrides to native for DXVK. WINEDEBUG defaults to -all and may be set in the
environment. For a quick UI check, use --ui-only to select Wine's builtin
renderer; its 3D background may appear black under Wine. For automated runs, --xvfb
uses a virtual display and arguments after the wrapper options are forwarded
to ProjectChrono, for example:
    bash porting/local_client/run_wine.sh --prefix /path/to/wine-prefix --xvfb -- --headless
