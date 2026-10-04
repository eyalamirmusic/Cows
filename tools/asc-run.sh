#!/usr/bin/env bash
# Usage: tools/asc-run.sh <command...>
# Runs the command under `op run` with Deploy/asc.env, from the "Cows In Love"
# vault of the personal 1Password account: through the desktop app, or through
# a service account token in COWS_OP_TOKEN (never the Tamber service account
# the shell may carry).
set -euo pipefail
cd "$(dirname "$0")/.."

if [[ -n "${COWS_OP_TOKEN:-}" ]]; then
    OP_SERVICE_ACCOUNT_TOKEN="$COWS_OP_TOKEN" exec op run --env-file Deploy/asc.env -- "$@"
fi

exec env -u OP_SERVICE_ACCOUNT_TOKEN op run --account my.1password.com \
    --env-file Deploy/asc.env -- "$@"
