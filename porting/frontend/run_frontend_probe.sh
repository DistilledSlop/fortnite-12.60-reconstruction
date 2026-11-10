#!/bin/bash
# Load one original frontend asset class in the matched stage and print the first failure.
# usage: run_frontend_probe.sh /Game/UI/Foundation/StateLayouts/LoginScreen.LoginScreen_C OUT.log [extra engine args]
# Add -FortniteSkipUnconvertedLegacyShaderMaps to list every unconverted /Game material (UNCONVERTED_LEGACY_MATERIAL lines);
# that flag is a diagnostic: later PostLoad can crash because those materials have no shader map.
# GDB_SCRIPT=file.gdb runs the probe under gdb (use 'handle SIGTRAP nostop noprint nopass', 'handle SIGSEGV stop', 'bt').
# Native/engine fixes for unversioned 12.60 packages (field paths, Sequencer v11) are on by default.
set -u
if [ "$#" -lt 2 ]; then
  echo "usage: $0 ASSET OUT.log [extra engine args]" >&2
  exit 2
fi
A=$1
O=$(realpath -m -- "$2")
shift 2
if [ -n "${GDB_SCRIPT:-}" ]; then
  GDB_SCRIPT=$(realpath -m -- "$GDB_SCRIPT")
fi
R=/opt/fortnite/FortniteClient-12.60-CL-13477524/porting
G=$R/generated; M=$G/material_pipeline
# Bundles whose hashes are compiled into LegacyMaterialShaderMapProbe.cpp (root-replay copies match the pins).
B="$M/locker-ingress-bundle.json,$M/gradient-panel-ingress-bundle.json,$M/power-toast-ingress-bundle.json,$M/menu-button-ingress-bundle-active-time.json,$M/donut-ingress-bundle.json,$M/mainl-ingress-bundle-checked.json,$M/keybind-ingress-3/bundle.json,$M/flogo-ingress-root-replay-1/bundle.json,$M/google-envelope-1-a/bundle.json,$M/media_button-ingress-2/bundle.json,$M/social-atlas-corrected-ingress-3/bundle.json,$M/hollow-ingress-root-replay-1/bundle.json,$G/reconstruction-visual-2026-11-05/signin-debug-epic-ingress/bundle.json,$M/facebook-ingress-1/bundle.json,$M/login-icon-instances-1/M_TempIcon_Login_PS4.bundle.json,$M/login-icon-instances-1/M_TempIcon_Login_XBox.bundle.json,$M/login-icon-instances-1/M_TempIcon_Login_Epic.bundle.json,$M/externallink-ingress-1/bundle.json,$M/backarrow-ingress-1/bundle.json,$M/skewdf-ingress-1/bundle.json,$M/skewdf-instances-1/M_SkewButtonShape_Chip.bundle.json,$M/tracking-llama-bundle-3/M_UI_TrackingLlama.json"
CM=$M/comicbubble-ingress-3/combined-manifest.json
SHA=$(sha256sum "$CM" | cut -d' ' -f1)
cd "$G/frontend-probes/paper2d-matched-stage-3/LinuxNoEditor/FortniteLinux/Binaries/Linux" || exit 2
ulimit -c 0
RUN=(./FortniteLinux.nov8)
[ -n "${GDB_SCRIPT:-}" ] && RUN=(gdb -q -batch -x "$GDB_SCRIPT" --args ./FortniteLinux.nov8)
timeout 170 xvfb-run -a -s '-screen 0 1280x720x24' "${RUN[@]}" FortniteLinux -vulkan -windowed -ResX=1280 -ResY=720 -nosound -unattended \
  -FortniteLegacyMaterialIngress="$B" -FortniteProbeAsset="$A" -ExecCmds=Quit -log \
  -FortniteLegacyMaterialManifest="$CM" -FortniteLegacyMaterialManifestSHA256="$SHA" "$@" > "$O" 2>&1
rc=$?
echo "rc=$rc"
grep -a -n "Fatal error\|Signal 11\|Serial size mismatch\|Seeked past\|Assertion failed\|ingress failed\|invalid pinned" "$O" | head -4 | cut -c1-250
exit "$rc"
