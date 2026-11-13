#!/usr/bin/env bash
# Shared original-client UI driver. Source this and call start_client_actions
# inside the client X display; FN_ACTION_PID identifies the background worker.
start_client_actions() {
    FN_ACTION_PID=
    [[ -n ${FN_ACTIONS:-} ]] || return 0
    (
        set +e
        base=$SECONDS
        read -r -a actions <<< "$FN_ACTIONS"
        for action in "${actions[@]}"; do
            at=${action%%:*}; what=${action#*:}; kind=${what%%,*}; args=${what#*,}
            delay=$(( base + at - SECONDS )); (( delay <= 0 )) || sleep $delay
            case $kind in
                wait) # wait,REGEX[,TIMEOUT]: poll the client log and FN_ACTION_LOG (server log) for REGEX; later times count from the match.
                    pattern=${args%%,*}; limit=600; [[ $args != *,* ]] || limit=${args##*,}
                    deadline=$(( SECONDS + limit ))
                    until grep -aqE "$pattern" "$WINEPREFIX"/drive_c/users/*/AppData/Local/FortniteGame/Saved/Logs/FortniteGame.log ${FN_ACTION_LOG:+"$FN_ACTION_LOG"} 2>/dev/null || (( SECONDS >= deadline )); do sleep 1; done
                    base=$SECONDS ;;
                stopif) # stopif,REGEX: end the action list when REGEX is already in the client or server log.
                    grep -aqE "$args" "$WINEPREFIX"/drive_c/users/*/AppData/Local/FortniteGame/Saved/Logs/FortniteGame.log ${FN_ACTION_LOG:+"$FN_ACTION_LOG"} 2>/dev/null && break ;;
                move) xdotool mousemove ${args%%,*} ${args#*,} ;;
                mhold) xdotool mousedown 1; sleep "$args"; xdotool mouseup 1 ;;
                look) xdotool mousemove_relative -- ${args%%,*} ${args#*,} ;;
                shot) ffmpeg -loglevel error -y -f x11grab -video_size 1280x720 -i "$DISPLAY" -frames:v 1 "$SHOTS_DIR/t$at.png" ;;
                click) xdotool mousemove ${args%%,*} ${args#*,} click 1 ;;
                key) xdotool key "$args" ;;
                hold) xdotool keydown "${args%%,*}"; sleep "${args#*,}"; xdotool keyup "${args%%,*}" ;;
            esac
        done
    ) &
    FN_ACTION_PID=$!
}
