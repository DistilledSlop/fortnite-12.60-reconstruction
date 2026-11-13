#!/usr/bin/env bash
set -euo pipefail
here=$(cd -- "$(dirname -- "$0")" && pwd)
root=$(cd -- "$here/../.." && pwd)
out="$root/porting/generated/local-client-tools"
cc=${CC:-x86_64-w64-mingw32-gcc}
mkdir -p -- "$out"
"$cc" -std=c11 -O2 -Wall -Wextra -Werror -municode -mwindows -o "$out/ProjectChrono.exe" "$here/launcher.c" -lcomdlg32 -lgdi32 -lgdiplus -lshell32 -lole32
"$cc" -std=c11 -O2 -Wall -Wextra -Werror -shared -o "$out/FortniteLocalHelper.dll" "$here/helper.c" -lcrypt32 -lws2_32
[[ -f $out/local-server.ini ]] || cp -- "$here/local-server.ini" "$out/"
cp -- "$here/README.txt" "$out/"
printf 'Built local client tools in %s\n' "$out"
