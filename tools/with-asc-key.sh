#!/usr/bin/env bash
# Usage: tools/with-asc-key.sh <command...>
# Runs the command with the App Store Connect API key from the "Cows In Love"
# 1Password vault (personal account): COWS_TEAM, COWS_ASC_KEY_ID,
# COWS_ASC_ISSUER_ID from the "App Store Connect API" item, and COWS_ASC_KEY
# pointing at the key file written to a private temp dir for the command's run.
set -euo pipefail

vault="Cows In Love"
item="App Store Connect API"
read_field() { op read "op://$vault/$item/$1" --account my.1password.com; }

keydir="$(mktemp -d)"
trap 'rm -rf "$keydir"' EXIT

export OP_SERVICE_ACCOUNT_TOKEN=
export COWS_TEAM="${COWS_TEAM:-$(read_field "team id")}"
export COWS_ASC_KEY_ID="$(read_field "key id")"
export COWS_ASC_ISSUER_ID="$(read_field "issuer id")"
export COWS_ASC_KEY="$keydir/AuthKey_$COWS_ASC_KEY_ID.p8"
op read "op://$vault/$item/private key" --account my.1password.com --out-file "$COWS_ASC_KEY"
chmod 600 "$COWS_ASC_KEY"

"$@"
