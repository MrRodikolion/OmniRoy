#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
SBC_TARGET="${SBC_TARGET:-}"
REMOTE_DIR="${SBC_REMOTE_DIR:-/opt/omniroy/sbc-app}"
JAR_PATH="$ROOT_DIR/sbc-app/target/omniroy-sbc-controller.jar"

if [[ -z "$SBC_TARGET" ]]; then
    printf 'Set SBC_TARGET to user@host before deploying.\n' >&2
    exit 2
fi

mvn -f "$ROOT_DIR/sbc-app/pom.xml" package
ssh "$SBC_TARGET" "mkdir -p '$REMOTE_DIR'"
rsync -av -- "$JAR_PATH" "$SBC_TARGET:$REMOTE_DIR/"

printf 'Deployed %s to %s:%s\n' "$(basename "$JAR_PATH")" "$SBC_TARGET" "$REMOTE_DIR"