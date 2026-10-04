#!/usr/bin/env bash
set -euo pipefail

SERIAL_PORT="${1:-${SERIAL_PORT:-/dev/ttyUSB0}}"
BAUD_RATE="${BAUD_RATE:-115200}"

if [[ ! -e "$SERIAL_PORT" ]]; then
    printf 'Serial device not found: %s\n' "$SERIAL_PORT" >&2
    exit 1
fi

stty -F "$SERIAL_PORT" "$BAUD_RATE" raw -echo
printf 'Reading %s at %s baud (Ctrl+C to stop).\n' "$SERIAL_PORT" "$BAUD_RATE"
exec cat "$SERIAL_PORT"