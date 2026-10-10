#!/usr/bin/env bash
# Creates a JSON key for the dedicated Play publisher service account and
# stores its only durable copy as a 1Password file field.
set -euo pipefail
cd "$(dirname "$0")/.."

unset OP_SERVICE_ACCOUNT_TOKEN

account="my.1password.com"
vault="Cows In Love"
item="Google Play"
project="${COWS_PLAY_GCLOUD_PROJECT:-cows-in-love-play}"
service_account="${COWS_PLAY_SERVICE_ACCOUNT:-cows-play-publisher@$project.iam.gserviceaccount.com}"
credential_field="service-account.json"

gcloud iam service-accounts describe "$service_account" --project "$project" \
    >/dev/null
op item get "$item" --vault "$vault" --account "$account" >/dev/null

if op item get "$item" --vault "$vault" --account "$account" --format json |
        jq -e --arg name "$credential_field" \
            '.files[]? | select(.name == $name)' >/dev/null; then
    echo "Play service-account credential already exists in 1Password; leaving it unchanged."
    echo "Service account email: $service_account"
    exit 0
fi

umask 077
setup_tmp="$(mktemp -d /tmp/cows-play-api.XXXXXX)"
credential="$setup_tmp/$credential_field"
roundtrip="$setup_tmp/roundtrip.json"
key_id=""
verified=false
cleanup() {
    if [[ "$verified" != true && -n "$key_id" ]]; then
        gcloud iam service-accounts keys delete "$key_id" --quiet \
            --iam-account "$service_account" --project "$project" \
            >/dev/null 2>&1 || true
    fi
    rm -f -- "$credential" "$roundtrip"
    rmdir "$setup_tmp" 2>/dev/null || true
}
trap cleanup EXIT

gcloud iam service-accounts keys create "$credential" \
    --iam-account "$service_account" --project "$project"
key_id="$(jq -r '.private_key_id' "$credential")"

op item edit "$item" --vault "$vault" --account "$account" \
    "service-account\\.json[file]=$credential" >/dev/null
op read --account "$account" --force --out-file "$roundtrip" \
    "op://$vault/$item/$credential_field"

if ! cmp -s "$credential" "$roundtrip"; then
    echo "error: the service-account JSON read back from 1Password did not match" >&2
    exit 1
fi

verified=true
echo "Stored and verified $credential_field in 1Password ($vault / $item)."
echo "Service account email: $service_account"
