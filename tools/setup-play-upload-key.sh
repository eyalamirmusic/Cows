#!/usr/bin/env bash
# Usage: tools/setup-play-upload-key.sh
set -euo pipefail
cd "$(dirname "$0")/.."

# The shell's service-account token is Tamber's; this key is the personal account's.
unset OP_SERVICE_ACCOUNT_TOKEN

account="my.1password.com"
vault="Cows In Love"
item="Google Play"
key_field="upload-keystore.jks"

if ! op vault get "$vault" --account "$account" >/dev/null 2>&1; then
    op vault create "$vault" --account "$account" >/dev/null
    echo "Created 1Password vault: $vault"
fi

if ! op item get "$item" --vault "$vault" --account "$account" >/dev/null 2>&1; then
    op item create --account "$account" --vault "$vault" \
        --category Password --title "$item" \
        --generate-password='letters,digits,32' \
        'upload key alias[text]=upload' >/dev/null
    echo "Created 1Password item: $item"
fi

if op item get "$item" --vault "$vault" --account "$account" --format json |
        jq -e --arg name "$key_field" '.files[]? | select(.name == $name)' \
        >/dev/null; then
    echo "Play upload key already exists in 1Password; leaving it unchanged."
    exit 0
fi

upload_password="$(op read --account "$account" \
    "op://$vault/$item/password")"
if [[ -z "$upload_password" ]]; then
    echo "error: the 1Password item has no generated password" >&2
    exit 1
fi

umask 077
setup_tmp="$(mktemp -d /tmp/cows-play-key.XXXXXX)"
keystore="$setup_tmp/$key_field"
roundtrip="$setup_tmp/roundtrip.jks"
cleanup() {
    unset upload_password COWS_UPLOAD_KEY_PASSWORD
    rm -f -- "$keystore" "$roundtrip"
    rmdir "$setup_tmp" 2>/dev/null || true
}
trap cleanup EXIT

export COWS_UPLOAD_KEY_PASSWORD="$upload_password"
java_home="${JAVA_HOME:-/Applications/Android Studio.app/Contents/jbr/Contents/Home}"
"$java_home/bin/keytool" -genkeypair -v \
    -keystore "$keystore" -storepass:env COWS_UPLOAD_KEY_PASSWORD \
    -alias upload -keypass:env COWS_UPLOAD_KEY_PASSWORD \
    -keyalg RSA -keysize 4096 -validity 10000 \
    -dname "CN=Cows In Love Upload, O=Cows In Love"

op item edit "$item" --vault "$vault" --account "$account" \
    "upload-keystore\\.jks[file]=$keystore" >/dev/null
op read --account "$account" --force --out-file "$roundtrip" \
    "op://$vault/$item/$key_field"

if [[ "$(shasum -a 256 "$keystore" | awk '{print $1}')" != \
      "$(shasum -a 256 "$roundtrip" | awk '{print $1}')" ]]; then
    echo "error: the keystore read back from 1Password did not match" >&2
    exit 1
fi

echo "Stored and verified $key_field in 1Password ($vault / $item)."
