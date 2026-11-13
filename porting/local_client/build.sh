#!/usr/bin/env bash
set -euo pipefail
here=$(cd -- "$(dirname -- "$0")" && pwd)
root=$(cd -- "$here/../.." && pwd)
out="$root/porting/generated/local-client-tools"
cc=${CC:-x86_64-w64-mingw32-gcc}
mkdir -p -- "$out"
"$cc" -std=c11 -O2 -Wall -Wextra -Werror -municode -mwindows -o "$out/ProjectChrono.exe" "$here/launcher.c" -lcomdlg32 -lgdi32 -lgdiplus -lshell32 -lole32
"$cc" -std=c11 -O2 -Wall -Wextra -Werror -shared -o "$out/FortniteLocalHelper.dll" "$here/helper.c" -lcrypt32 -lws2_32
# Native Linux build (Qt 6 Widgets). It sits beside ProjectChrono.exe, which it runs inside the Wine/Proton prefix.
if pkg-config --exists Qt6Widgets 2>/dev/null; then
    # shellcheck disable=SC2046
    "${CXX:-g++}" -std=c++17 -O2 -Wall -Wextra -Werror -fPIC -o "$out/ProjectChrono" "$here/launcher_linux.cpp" $(pkg-config --cflags --libs Qt6Widgets)
else
    printf 'Skipping the native Linux launcher: Qt 6 development files not found (Debian/Ubuntu: qt6-base-dev).\n' >&2
fi
[[ -f $out/local-server.ini ]] || cp -- "$here/local-server.ini" "$out/"
cp -- "$here/README.txt" "$out/"
printf 'Built local client tools in %s\n' "$out"
