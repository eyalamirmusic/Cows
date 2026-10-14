#!/usr/bin/env bash
# Usage: tools/release-android.sh [cmake args]
# The versionCode is the fourth number of project(Cows VERSION) in CMakeLists.txt.
# Builds a release AAB signed by the Play upload key supplied in the environment.
set -euo pipefail
cd "$(dirname "$0")/.."

required=(
    COWS_ANDROID_KEYSTORE
    COWS_ANDROID_KEYSTORE_PASSWORD
    COWS_ANDROID_KEY_ALIAS
    COWS_ANDROID_KEY_PASSWORD
)
for name in "${required[@]}"; do
    if [[ -z "${!name:-}" ]]; then
        echo "error: $name is required for a Play-signed bundle" >&2
        exit 2
    fi
done

if [[ "$COWS_ANDROID_KEYSTORE" != /* ]]; then
    echo "error: COWS_ANDROID_KEYSTORE must be an absolute path" >&2
    exit 2
fi
if [[ ! -f "$COWS_ANDROID_KEYSTORE" ]]; then
    echo "error: upload keystore not found: $COWS_ANDROID_KEYSTORE" >&2
    exit 2
fi

configure=(cmake --preset android)
if [[ -n "${EACP:-}" ]]; then
    configure+=("-DCPM_eacp_SOURCE=$EACP")
fi
configure+=("$@")
"${configure[@]}"

export JAVA_HOME="${JAVA_HOME:-/Applications/Android Studio.app/Contents/jbr/Contents/Home}"
(
    cd build-android/AndroidStudio
    ./gradlew :Cows:bundleRelease
)

aab="build-android/AndroidStudio/Cows/build/outputs/bundle/release/Cows-release.aab"
if [[ ! -f "$aab" ]]; then
    echo "error: Gradle did not produce $aab" >&2
    exit 1
fi

"$JAVA_HOME/bin/jarsigner" -verify "$aab" >/dev/null

expected_fingerprint="$(
    "$JAVA_HOME/bin/keytool" -J-Duser.language=en -J-Duser.country=US \
        -list -v -keystore "$COWS_ANDROID_KEYSTORE" \
        -storepass:env COWS_ANDROID_KEYSTORE_PASSWORD \
        -alias "$COWS_ANDROID_KEY_ALIAS" |
        awk -F': ' '/SHA256:/{ print $2; exit }'
)"
actual_fingerprint="$(
    "$JAVA_HOME/bin/keytool" -J-Duser.language=en -J-Duser.country=US \
        -printcert -jarfile "$aab" |
        awk -F': ' '/SHA256:/{ print $2; exit }'
)"
if [[ -z "$expected_fingerprint" || "$actual_fingerprint" != "$expected_fingerprint" ]]; then
    echo "error: bundle certificate does not match the configured upload key" >&2
    exit 1
fi

echo "Play-signed bundle: $aab"
