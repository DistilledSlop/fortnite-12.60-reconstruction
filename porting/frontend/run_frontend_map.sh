#!/bin/bash
# Run the client in the matched stage with a map as the startup URL, capture an X framebuffer screenshot at the end.
# usage: run_frontend_map.sh /Game/Maps/Frontend OUT.log SHOT.png [SECONDS] [extra engine args]
# Add -FortniteSkipUnconvertedLegacyShaderMaps to list every unconverted /Game material (UNCONVERTED_LEGACY_MATERIAL lines);
# that flag is a diagnostic: later PostLoad can crash because those materials have no shader map.
# GDB_SCRIPT=file.gdb runs the probe under gdb (use 'handle SIGTRAP nostop noprint nopass', 'handle SIGSEGV stop', 'bt').
# Native/engine fixes for unversioned 12.60 packages (field paths, Sequencer v11) are on by default.
set -u
if [ "$#" -lt 3 ]; then
  echo "usage: $0 MAP OUT.log SHOT.png [SECONDS] [extra engine args]" >&2
  exit 2
fi
MAP=$1
O=$(realpath -m -- "$2")
PNG=$(realpath -m -- "$3")
shift 3
SECS=90
if [ "$#" -gt 0 ] && [[ $1 =~ ^[0-9]+([.][0-9]+)?$ ]]; then
  SECS=$1
  shift
fi
if [ -n "${GDB_SCRIPT:-}" ]; then
  GDB_SCRIPT=$(realpath -m -- "$GDB_SCRIPT")
fi
R=/opt/fortnite/FortniteClient-12.60-CL-13477524/porting
G=$R/generated; M=$G/material_pipeline
# Bundles whose hashes are compiled into LegacyMaterialShaderMapProbe.cpp (root-replay copies match the pins).
B="$M/locker-ingress-bundle.json,$M/gradient-panel-ingress-bundle.json,$M/power-toast-ingress-bundle.json,$M/menu-button-ingress-bundle-active-time.json,$M/donut-ingress-bundle.json,$M/mainl-ingress-bundle-checked.json,$M/keybind-ingress-3/bundle.json,$M/flogo-ingress-root-replay-1/bundle.json,$M/google-envelope-1-a/bundle.json,$M/media_button-ingress-2/bundle.json,$M/social-atlas-corrected-ingress-3/bundle.json,$M/hollow-ingress-root-replay-1/bundle.json,$G/reconstruction-visual-2026-11-05/signin-debug-epic-ingress/bundle.json,$M/facebook-ingress-1/bundle.json,$M/login-icon-instances-1/M_TempIcon_Login_PS4.bundle.json,$M/login-icon-instances-1/M_TempIcon_Login_XBox.bundle.json,$M/login-icon-instances-1/M_TempIcon_Login_Epic.bundle.json,$M/externallink-ingress-1/bundle.json,$M/backarrow-ingress-1/bundle.json,$M/skewdf-ingress-1/bundle.json,$M/skewdf-instances-1/M_SkewButtonShape_Chip.bundle.json,$M/tracking-llama-bundle-3/M_UI_TrackingLlama.json"
CM=$(realpath -e -- "${FORTNITE_MATERIAL_MANIFEST:-$M/comicbubble-ingress-3/combined-manifest.json}") || exit 2
SHA=$(sha256sum "$CM" | cut -d' ' -f1)
cd "$G/frontend-probes/paper2d-matched-stage-3/LinuxNoEditor/FortniteLinux/Binaries/Linux" || exit 2
FB=$(mktemp -d) || exit 2
trap 'rm -rf -- "$FB" "$FB.snap.py"' EXIT
ulimit -c 0
RUN=(./FortniteLinux.nov8)
[ -n "${GDB_SCRIPT:-}" ] && RUN=(gdb -q -batch -x "$GDB_SCRIPT" --args ./FortniteLinux.nov8)
timeout "$SECS" xvfb-run -a -s "-screen 0 1280x720x24 -fbdir $FB" "${RUN[@]}" "$MAP" -vulkan -windowed -ResX=1280 -ResY=720 -nosound -unattended \
  -FortniteLegacyMaterialIngress="$B" -log \
  -FortniteLegacyMaterialManifest="$CM" -FortniteLegacyMaterialManifestSHA256="$SHA" "$@" > "$O" 2>&1 &
PID=$!
cat > "$FB.snap.py" <<'PY'
import sys,glob,struct
from PIL import Image
f=glob.glob(sys.argv[1]+'/Xvfb_screen0')
if f:
    try:
        d=open(f[0],'rb').read(); hs=struct.unpack('>I',d[:4])[0]
        w,h=struct.unpack('>II',d[16:24]); nc=struct.unpack('>I',d[76:80])[0]
        off=hs+nc*12
        Image.frombuffer('RGBX',(w,h),d[off:off+w*h*4],'raw','BGRX',0,1).convert('RGB').save(sys.argv[2]+'.tmp.png')
        import os; os.replace(sys.argv[2]+'.tmp.png',sys.argv[2])
    except Exception as e:
        pass
PY
# snapshot every 5 s while the client is alive; the last good frame is kept at $PNG
while kill -0 $PID 2>/dev/null; do sleep 5; python3 "$FB.snap.py" "$FB" "$PNG"; done
wait "$PID"
rc=$?
echo "rc=$rc"
grep -a -n "Fatal error\|Signal 11\|Assertion failed" "$O" | head -3 | cut -c1-250
exit "$rc"
