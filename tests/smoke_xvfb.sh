#!/usr/bin/env bash
# Xvfb integration smoke test for the native MiG Alley port.
#
#  1. starts a private Xvfb display
#  2. launches the game and waits for the intro video (OpenSmack)
#  3. clicks once to skip the intro - regression for the stuck-drag bug:
#     the click used to leave an RDialog in dragstate!=DRAG_NO, making the
#     menu background follow the cursor and later a blank white screen
#  4. moves the mouse with no button held (must not pan the sheet)
#  5. verifies both screenshots are neither blank-white nor black
#
# Exit: 0 pass, 1 fail, 77 skip (missing tool or game data).
set -u
cd "$(dirname "$0")"

LAUNCHER="${MIG_LAUNCHER:-$(cd ../../.. && pwd)/run-migalley.sh}"

for t in Xvfb xdotool import convert; do
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

LOG=$(mktemp /tmp/mig_smoke.XXXXXX.log)
S1=$(mktemp /tmp/mig_smoke1.XXXXXX.png)
S2=$(mktemp /tmp/mig_smoke2.XXXXXX.png)
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

echo "=== launching $LAUNCHER"
DISPLAY="$DISP" sh "$LAUNCHER" >"$LOG" 2>&1 &
GAME_PID=$!

# --- wait for the intro video --------------------------------------------
ok=""
for i in $(seq 1 60); do
    grep -q "OpenSmack" "$LOG" && { ok=1; break; }
    kill -0 "$GAME_PID" 2>/dev/null || break
    sleep 1
done
if [ -z "$ok" ]; then
    echo "FAIL: intro video never started"; tail -20 "$LOG"; exit 1
fi
echo "=== intro playing, clicking to skip"
sleep 1
DISPLAY="$DISP" xdotool mousemove 800 450 click 1

# --- wait for the main menu ----------------------------------------------
sleep 8
DISPLAY="$DISP" import -window root "$S1" || { echo "FAIL: screenshot"; exit 1; }

# --- mouse moves with NO button held (stuck-drag regression) --------------
for xy in "300 300" "1300 600" "800 450" "200 700" "1400 800"; do
    DISPLAY="$DISP" xdotool mousemove $xy; sleep 0.5
done
sleep 2
DISPLAY="$DISP" import -window root "$S2" || { echo "FAIL: screenshot"; exit 1; }

# --- verdicts --------------------------------------------------------------
mean() { convert "$1" -colorspace Gray -format "%[fx:mean*255]" info:; }
M1=$(mean "$S1"); M2=$(mean "$S2")
echo "=== brightness: menu=$M1 after-moves=$M2"

FAIL=""
for pair in "menu:$M1" "moves:$M2"; do
    v=${pair#*:}
    if awk "BEGIN{exit !($v>210)}"; then FAIL="$FAIL ${pair%%:*}=white"; fi
    if awk "BEGIN{exit !($v<8)}";   then FAIL="$FAIL ${pair%%:*}=black"; fi
done

# the down during the intro may legitimately log one OnLButtonDown; a
# second drag-state entry means the sheet is still being dragged
DRAGS=$(grep -c "OnLButtonDown" "$LOG" || true)
if [ "$DRAGS" -gt 1 ]; then FAIL="$FAIL drags=$DRAGS"; fi
if grep -q "stale drag cleared" "$LOG"; then
    echo "=== note: stale drag state was exercised and healed"
fi

if [ -n "$FAIL" ]; then
    echo "FAIL:$FAIL"
    echo "--- last log lines ---"; tail -15 "$LOG"
    echo "--- screenshots kept: $S1 $S2"; exit 1
fi
echo "PASS: intro skipped, menu rendered, no stuck drag, no white screen"
exit 0
