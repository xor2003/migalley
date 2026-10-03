#!/usr/bin/env bash
# Xvfb integration smoke test for the native MiG Alley port.
#
#  1. starts a private Xvfb display
#  2. launch A: presses a KEY during the intro - regression for
#     "any key should interrupt intro" (InterruptSmack -> introsmk)
#  3. launch B: clicks once to skip the intro - regression for the
#     stuck-drag bug: the click used to leave an RDialog in
#     dragstate!=DRAG_NO, making the menu background follow the cursor
#     and later a blank white screen
#  4. moves the mouse with no button held (must not pan the sheet)
#  5. verifies all screenshots are neither blank-white nor black
#
# Exit: 0 pass, 1 fail, 77 skip (missing tool or game data).
set -u
cd "$(dirname "$0")"

LAUNCHER="${MIG_LAUNCHER:-$(cd ../../.. && pwd)/run-migalley.sh}"

for t in Xvfb xdotool import convert compare; do
    command -v "$t" >/dev/null 2>&1 || { echo "SKIP: $t not found"; exit 77; }
done
[ -x "$LAUNCHER" ] || { echo "SKIP: launcher $LAUNCHER missing"; exit 77; }
GAMEDIR="$(cd ../../.. && pwd)/runtime/migprefix/drive_c/rowan/mig"
[ -f "$GAMEDIR/roots.dir" ] || { echo "SKIP: no installed game data at $GAMEDIR"; exit 77; }

# --- pick a free display -------------------------------------------------
DISP=""
for d in 110 111 112 113 114 115; do
    xdpyinfo -display ":$d" >/dev/null 2>&1 || { DISP=":$d"; break; }
done
[ -n "$DISP" ] || { echo "SKIP: no free X display"; exit 77; }

LOG_A=$(mktemp /tmp/mig_smokeA.XXXXXX.log)
LOG_B=$(mktemp /tmp/mig_smokeB.XXXXXX.log)
S1=$(mktemp /tmp/mig_smoke1.XXXXXX.png)
S1B=$(mktemp /tmp/mig_smoke1b.XXXXXX.png)
S2=$(mktemp /tmp/mig_smoke2.XXXXXX.png)
S3=$(mktemp /tmp/mig_smoke3.XXXXXX.png)
XVFB_PID=""; GAME_PID=""
cleanup() {
    [ -n "$GAME_PID" ] && kill "$GAME_PID" 2>/dev/null
    [ -n "$XVFB_PID" ] && kill "$XVFB_PID" 2>/dev/null
}
trap cleanup EXIT

echo "=== Xvfb on $DISP"
Xvfb "$DISP" -screen 0 1600x900x24 >/dev/null 2>&1 &
XVFB_PID=$!
sleep 1

# launch_game <logfile>: start the game, wait for the intro video
launch_game() {
    echo "=== launching $LAUNCHER"
    DISPLAY="$DISP" sh "$LAUNCHER" >"$1" 2>&1 &
    GAME_PID=$!
    local ok="" i
    for i in $(seq 1 60); do
        grep -q "OpenSmack" "$1" && { ok=1; break; }
        kill -0 "$GAME_PID" 2>/dev/null || break
        sleep 1
    done
    if [ -z "$ok" ]; then
        echo "FAIL: intro video never started"; tail -20 "$1"; exit 1
    fi
}

stop_game() {
    kill "$GAME_PID" 2>/dev/null; wait "$GAME_PID" 2>/dev/null
    GAME_PID=""
}

# ==== launch A: KEY skips the intro =======================================
launch_game "$LOG_A"
echo "=== intro playing, pressing a key to skip"
sleep 1
DISPLAY="$DISP" xdotool key Return
# two shots 3s apart: identical = static menu (skip worked); different =
# the video is still playing (key was ignored)
sleep 5
DISPLAY="$DISP" import -window root "$S1" || { echo "FAIL: screenshot"; exit 1; }
sleep 3
DISPLAY="$DISP" import -window root "$S1B" || { echo "FAIL: screenshot"; exit 1; }
stop_game

# ==== launch B: CLICK skips the intro (stuck-drag regression) =============
launch_game "$LOG_B"
echo "=== intro playing, clicking to skip"
sleep 1
DISPLAY="$DISP" xdotool mousemove 800 450 click 1
sleep 8
DISPLAY="$DISP" import -window root "$S2" || { echo "FAIL: screenshot"; exit 1; }

# --- mouse moves with NO button held (stuck-drag regression) --------------
for xy in "300 300" "1300 600" "800 450" "200 700" "1400 800"; do
    DISPLAY="$DISP" xdotool mousemove $xy; sleep 0.5
done
sleep 2
DISPLAY="$DISP" import -window root "$S3" || { echo "FAIL: screenshot"; exit 1; }
stop_game

# --- verdicts --------------------------------------------------------------
mean() { convert "$1" -colorspace Gray -format "%[fx:mean*255]" info:; }
M1=$(mean "$S1"); M2=$(mean "$S2"); M3=$(mean "$S3")
echo "=== brightness: key-skip=$M1 click-skip=$M2 after-moves=$M3"

FAIL=""
for pair in "keyskip:$M1" "clickskip:$M2" "moves:$M3"; do
    v=${pair#*:}
    if awk "BEGIN{exit !($v>210)}"; then FAIL="$FAIL ${pair%%:*}=white"; fi
    if awk "BEGIN{exit !($v<8)}";   then FAIL="$FAIL ${pair%%:*}=black"; fi
done

# key skip proof: the screen must be static (menu) not playing video.
# compare counts pixels differing between the two post-key shots.
# compare exits nonzero when images differ; the AE count is on stderr
DIFFPX=$(compare -metric AE "$S1" "$S1B" null: 2>&1 || true)
DIFFPX=${DIFFPX%%[!0-9]*}
echo "=== key-skip frame diff: $DIFFPX px"
if ! [[ "$DIFFPX" =~ ^[0-9]+$ ]] || [ "$DIFFPX" -gt 5000 ]; then
    FAIL="$FAIL keystillplaying=${DIFFPX}px"
fi

# the down during the intro may legitimately log one OnLButtonDown; a
# second drag-state entry means the sheet is still being dragged
DRAGS=$(grep -c "OnLButtonDown" "$LOG_B" || true)
if [ "$DRAGS" -gt 1 ]; then FAIL="$FAIL drags=$DRAGS"; fi
if grep -q "stale drag cleared" "$LOG_B"; then
    echo "=== note: stale drag state was exercised and healed"
fi

if [ -n "$FAIL" ]; then
    echo "FAIL:$FAIL"
    echo "--- last log A lines ---"; tail -10 "$LOG_A"
    echo "--- last log B lines ---"; tail -10 "$LOG_B"
    echo "--- screenshots kept: $S1 $S2 $S3"; exit 1
fi
echo "PASS: key and click skip intro, menu rendered, no stuck drag, no white screen"
exit 0
