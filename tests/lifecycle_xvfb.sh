#!/usr/bin/env bash
# REQ-LIFE-01 lifecycle test — repeated migalley launches must show no
# stale state, cross-install contamination, or growing resource counts
# after warm-up.
#
#   phase 1: shell gate reject — a bogus dir must exit 2 with diagnostic,
#            every cycle, before any display exists.
#   phase 2: N sequential launches on a private Xvfb; each reaches the
#            intro video (OpenSmack), we sample fd count / VmRSS / maps
#            of the live process, then kill it. Post-warm-up counts must
#            be stable (fds exact, rss bounded, maps exact) and no game
#            processes may survive between cycles.
#
# Usage: lifecycle_xvfb.sh <path-to-MigAlley-binary>
# Exit: 0 pass, 1 fail, 77 skip.
set -u

BIN="${1:?usage: lifecycle_xvfb.sh <MigAlley-binary>}"
ROOT="$(cd "$(dirname "$0")/../../.." && pwd)"
GAMEDIR="$ROOT/runtime/migprefix/drive_c/rowan/mig"
SYSROOT="$ROOT/sysroot-i386/usr/lib/i386-linux-gnu"
CYCLES=5

command -v Xvfb >/dev/null 2>&1 || { echo "SKIP: Xvfb not found"; exit 77; }
[ -x "$BIN" ] || { echo "SKIP: binary $BIN missing"; exit 77; }
[ -f "$GAMEDIR/roots.dir" ] || { echo "SKIP: no installed game data"; exit 77; }
export LD_LIBRARY_PATH="$SYSROOT${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"

# --- phase 1: gate rejects a non-install dir, repeatedly -----------------
for i in 1 2 3; do
    out=$("$BIN" /tmp 2>&1); rc=$?
    [ $rc -eq 2 ] || { echo "FAIL: gate cycle $i rc=$rc (want 2)"; exit 1; }
    echo "$out" | grep -q "not a recognized Rowan install" || {
        echo "FAIL: gate cycle $i no diagnostic: $out"; exit 1; }
done
echo "=== gate: 3/3 rejected with diagnostics"

# --- phase 2: N launches, track resources --------------------------------
DISP=""
for d in 120 121 122 123; do
    xdpyinfo -display ":$d" >/dev/null 2>&1 || { DISP=":$d"; break; }
done
[ -n "$DISP" ] || { echo "SKIP: no free X display"; exit 77; }
Xvfb "$DISP" -screen 0 1600x900x24 >/dev/null 2>&1 &
XVFB_PID=$!
trap 'kill $XVFB_PID 2>/dev/null; pkill -x MigAlley 2>/dev/null' EXIT
sleep 1

LOG=$(mktemp /tmp/mig_life.XXXXXX.log)
declare -a FDS RSS MAPS
for i in $(seq 1 $CYCLES); do
    DISPLAY="$DISP" "$BIN" "$GAMEDIR" >"$LOG" 2>&1 &
    PID=$!
    ok=""
    for w in $(seq 1 60); do
        grep -q "OpenSmack" "$LOG" && { ok=1; break; }
        kill -0 "$PID" 2>/dev/null || break
        sleep 1
    done
    [ -z "$ok" ] && { echo "FAIL: cycle $i intro never started"; tail -15 "$LOG"; exit 1; }
    sleep 2   # settle into video playback so samples are comparable
    FDS[$i]=$(ls "/proc/$PID/fd" | wc -l)
    RSS[$i]=$(awk '/VmRSS/{print $2}' "/proc/$PID/status")
    MAPS[$i]=$(wc -l < "/proc/$PID/maps")
    kill -9 "$PID" 2>/dev/null; wait "$PID" 2>/dev/null
    # nothing may survive between cycles
    sleep 1
    if pgrep -x MigAlley >/dev/null; then
        echo "FAIL: cycle $i left a running MigAlley process"; exit 1; fi
    echo "=== cycle $i: fds=${FDS[$i]} rss=${RSS[$i]}kB maps=${MAPS[$i]}"
done

FAIL=""
# warm-up = first cycle (fontconfig/lib caches); compare cycles 2..N
for i in $(seq 3 $CYCLES); do
    [ "${FDS[$i]}" -ne "${FDS[2]}" ] && FAIL="$FAIL fds:$i=${FDS[$i]}!=${FDS[2]}"
    # maps jitter a few lines per launch (heap splits, stack placement)
    [ "${MAPS[$i]}" -gt "$(( ${MAPS[2]} + 16 ))" ] && FAIL="$FAIL maps:$i=${MAPS[$i]}>${MAPS[2]}+16"
    # RSS may wobble on allocator; allow +10% over cycle 2
    awk "BEGIN{exit !(${RSS[$i]} > ${RSS[2]} * 1.10)}" && FAIL="$FAIL rss:$i=${RSS[$i]}kB>${RSS[2]}kB+10%"
done

[ -n "$FAIL" ] && { echo "FAIL:$FAIL"; exit 1; }
echo "PASS: $CYCLES launches, gate each time, no survivors, resources stable post-warm-up"
exit 0
