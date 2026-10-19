#!/usr/bin/env bash
# Usage: [COWS_OP_TOKEN=token] tools/asc-run.sh <command...>
# Drops the shell's OP_SERVICE_ACCOUNT_TOKEN: that is Tamber's account, not the personal one.
set -euo pipefail
cd "$(dirname "$0")/.."

if [[ -n "${COWS_OP_TOKEN:-}" ]]; then
    OP_SERVICE_ACCOUNT_TOKEN="$COWS_OP_TOKEN" exec op run --env-file Deploy/asc.env -- "$@"
fi

exec env -u OP_SERVICE_ACCOUNT_TOKEN op run --account my.1password.com \
    --env-file Deploy/asc.env -- "$@"
