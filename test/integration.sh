#!/usr/bin/env bash
# Run with: sudo ./test/integration.sh
set -u
cd "$(dirname "$0")/.."

TARGET=build/target_program
SCANNER=build/scanner
TMP=$(mktemp -d)
TPID=""
trap '[ -n "$TPID" ] && kill "$TPID" 2>/dev/null; rm -rf "$TMP"' EXIT

wait_for() {   # wait_for <file> <pattern>
    for _ in $(seq 100); do
        grep -q "$2" "$1" 2>/dev/null && return 0
        sleep 0.1
    done
    echo "FAIL: timed out waiting for '$2' in $1"; exit 1
}

# Feed the target through a FIFO; holding fd 3 open prevents EOF.
mkfifo "$TMP/in"
exec 3<>"$TMP/in"
"$TARGET" <"$TMP/in" >"$TMP/target.out" 2>&1 &
TPID=$!

wait_for "$TMP/target.out" "Health memory address"
ADDR=$(grep -o 'address: 0x[0-9a-f]*' "$TMP/target.out" | head -1 | cut -d' ' -f2)
echo "target pid=$TPID  health address=$ADDR"

# Scanner scans for 100, then we change health to 73 and refine with 73.
{
    wait_for "$TMP/scan.out" "Initial Pass Complete"
    echo 73 >&3
    wait_for "$TMP/target.out" "Health value: 73"
    echo 73
    wait_for "$TMP/scan.out" "Refinement Pass Complete"
    echo -999
} | stdbuf -oL "$SCANNER" "$TPID" 100 >"$TMP/scan.out" 2>&1

if grep -q "Address: $ADDR | Value: 73" "$TMP/scan.out"; then
    echo "PASS: scanner found health at $ADDR"
else
    echo "FAIL: $ADDR not in final candidates"
    tail -n 20 "$TMP/scan.out"
    exit 1
fi