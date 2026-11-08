#!/bin/bash
# usage: run_hits.sh MAP SECONDS -> prints COUNT lines
cd /opt/fortnite/FortniteClient-12.60-CL-13477524/porting/generated/linux-stage/LinuxNoEditor/FortniteLinux/Binaries/Linux
MAP="$1"; T=${2:-40}
gdb -q -batch -x /tmp/claude-1000/-opt-fortnite/46cd2cbf-c73d-4053-ba19-142367ff8af0/scratchpad/hits.gdb --args ./FortniteLinuxServer.gdb "$MAP" -FortniteProbePawnGround -nullrhi -nosound -unattended -port=7777 -log > /tmp/claude-1000/-opt-fortnite/46cd2cbf-c73d-4053-ba19-142367ff8af0/scratchpad/hits_gdb.out 2>&1 &
GP=$!
sleep 15
timeout 25 ./FortniteLinux.new FortniteLinux 127.0.0.1:7777 -nullrhi -nosound -unattended -log > /tmp/claude-1000/-opt-fortnite/46cd2cbf-c73d-4053-ba19-142367ff8af0/scratchpad/hits_client.log 2>&1
sleep $((T-40>0?T-40:1))
kill -INT $(pgrep -f '^./FortniteLinuxServer.gdb' | head -1) 2>/dev/null
wait $GP
grep COUNT /tmp/claude-1000/-opt-fortnite/46cd2cbf-c73d-4053-ba19-142367ff8af0/scratchpad/hits_gdb.out | sort -k2 -n -r > /tmp/claude-1000/-opt-fortnite/46cd2cbf-c73d-4053-ba19-142367ff8af0/scratchpad/hits_counts.txt
wc -l /tmp/claude-1000/-opt-fortnite/46cd2cbf-c73d-4053-ba19-142367ff8af0/scratchpad/hits_counts.txt
