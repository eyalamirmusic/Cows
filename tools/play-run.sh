#!/usr/bin/env bash
# Usage: tools/play-run.sh <command...>
set -euo pipefail
cd "$(dirname "$0")/.."

keystore_ref="${COWS_PLAY_KEYSTORE_REF:-op://Cows In Love/Google Play/upload-keystore.jks}"
service_account_ref="${COWS_PLAY_SERVICE_ACCOUNT_REF:-op://Cows In Love/Google Play/service-account.json}"

umask 077
play_tmp="$(mktemp -d /tmp/cows-play.XXXXXX)"
keystore="$play_tmp/upload-keystore.jks"
service_account="$play_tmp/service-account.json"
cleanup() {
    rm -f -- "$keystore" "$service_account"
    rmdir "$play_tmp" 2>/dev/null || true
}
trap cleanup EXIT

op_read() {
    local output="$1"
    local reference="$2"
    if [[ -n "${COWS_OP_TOKEN:-}" ]]; then
        OP_SERVICE_ACCOUNT_TOKEN="$COWS_OP_TOKEN" \
            op read --force --out-file "$output" "$reference"
    else
        env -u OP_SERVICE_ACCOUNT_TOKEN op read --account my.1password.com \
            --force --out-file "$output" "$reference"
    fi
}

op_read "$keystore" "$keystore_ref"
op_read "$service_account" "$service_account_ref"

if [[ -n "${COWS_OP_TOKEN:-}" ]]; then
    COWS_ANDROID_KEYSTORE="$keystore" COWS_PLAY_JSON_KEY="$service_account" \
        OP_SERVICE_ACCOUNT_TOKEN="$COWS_OP_TOKEN" \
        op run --env-file Deploy/play.env -- "$@"
else
    env -u OP_SERVICE_ACCOUNT_TOKEN \
        COWS_ANDROID_KEYSTORE="$keystore" \
        COWS_PLAY_JSON_KEY="$service_account" \
        op run --account my.1password.com --env-file Deploy/play.env -- "$@"
fi
