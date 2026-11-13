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
