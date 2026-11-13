#!/usr/bin/env bash
set -euo pipefail

here=$(cd -- "$(dirname -- "$0")" && pwd)
root=$(cd -- "$here/../.." && pwd)
prefix=${FN_WINEPREFIX:-${WINEPREFIX:-${HOME:?HOME must be set}/.wine}}
tools_dir="$root/porting/generated/local-client-tools"
renderer=dxvk
xvfb=0

usage() {
    cat <<'HELP'
Usage: run_wine.sh [--prefix DIR] [--tools-dir DIR] [--ui-only] [--xvfb] [--] [PROJECTCHRONO_ARGS...]

Launch ProjectChrono under Wine. By default, use native d3d11 and dxgi for
DXVK. Install DXVK in the selected prefix first; DLL presence alone does not
prove they are DXVK. The prefix defaults to FN_WINEPREFIX, then WINEPREFIX,
then ~/.wine. The tools directory defaults to porting/generated/local-client-tools.
--ui-only uses Wine's builtin d3d11 and dxgi for a quick UI check; the 3D
background is expected to be black. --xvfb runs on a virtual X display.
All remaining arguments are passed to ProjectChrono (for example --headless).
The launcher validates its backend configuration and public CA itself.
HELP
}

while (($#)); do
    case "$1" in
        --prefix|--tools-dir)
            option=$1
            if (($# < 2)) || [[ -z $2 || $2 == --* ]]; then
                printf 'Error: %s requires a directory.\n' "$option" >&2; exit 2
            fi
            if [[ $option == --prefix ]]; then prefix=$2; else tools_dir=$2; fi
            shift 2 ;;
        --ui-only) renderer=ui; shift ;;
        --xvfb) xvfb=1; shift ;;
        --help) usage; exit 0 ;;
        --) shift; break ;;
        *) break ;;
    esac
done

if [[ ! -d $prefix ]]; then
    printf 'Error: Wine prefix does not exist: %s\n' "$prefix" >&2; exit 1
fi
if [[ ! -f $tools_dir/ProjectChrono.exe || ! -f $tools_dir/FortniteLocalHelper.dll ]]; then
    printf 'Error: ProjectChrono.exe and FortniteLocalHelper.dll are required in %s; run porting/local_client/build.sh first.\n' "$tools_dir" >&2; exit 1
fi
command -v wine >/dev/null || { printf 'Error: wine is not installed or not on PATH.\n' >&2; exit 1; }
prefix=$(cd -- "$prefix" && pwd)
tools_dir=$(cd -- "$tools_dir" && pwd)
if [[ $renderer == dxvk ]]; then
    for dll in d3d11 dxgi; do
        if [[ ! -f $prefix/drive_c/windows/system32/$dll.dll ]]; then
            printf 'Error: %s.dll is missing from %s/drive_c/windows/system32. Install 64-bit DXVK in this prefix first.\n' "$dll" "$prefix" >&2; exit 1
        fi
    done
    overrides='d3d11,dxgi=n'
else
    overrides='d3d11,dxgi=b'
fi
if ((xvfb)); then
    command -v xvfb-run >/dev/null || { printf 'Error: --xvfb requires xvfb-run on PATH.\n' >&2; exit 1; }
    runner=(xvfb-run -a)
else
    if [[ -z ${DISPLAY:-} ]]; then
        printf 'Error: DISPLAY is not set; use a desktop display or --xvfb.\n' >&2; exit 1
    fi
    runner=()
fi
export WINEPREFIX="$prefix" WINEDEBUG="${WINEDEBUG:--all}" WINEDLLOVERRIDES="$overrides"
exec "${runner[@]}" wine "$tools_dir/ProjectChrono.exe" "$@"
