#!/usr/bin/env bash
# Usage: COWS_ANDROID_KEYSTORE=<upload.jks> COWS_ANDROID_KEY_ALIAS=<alias>
#        COWS_ANDROID_KEYSTORE_PASSWORD=<password> [COWS_BUILD_NUMBER=n]
#        [COWS_BUNDLE_ID=id] [COWS_ANDROID_ABIS="arm64-v8a x86_64"]
#        tools/release-android.sh [--install]
# Builds the Release libCows.so for each ABI, packages them as an Android App
# Bundle with no Gradle (aapt2 in proto format, bundletool) and signs it with
# the upload key: Deploy/Google-Play/out/CowsInLove-<version>-<build>.aab, plus
# the stripped symbols Play uses for native crash reports (inside the bundle).
# --install also validates it, builds the APK set for the attached device or
# emulator (bundletool --local-testing), installs and launches it.
#
# One-time upload key (keep it and its password; Play App Signing holds the
# real app signing key, so a lost upload key can be reset from Play Console):
#   keytool -genkeypair -v -keystore upload.jks -alias upload -keyalg RSA \
#       -keysize 4096 -validity 10000
#
# Needs the Android SDK at $ANDROID_HOME (default ~/Library/Android/sdk) with
# NDK r30 (eacp builds with the current stable NDK), build-tools 35 and
# platform 35, and a JDK (Homebrew openjdk@21).
# eacp: EACP=<path> (default ~/projects/eacp-android-integration), else CPM
# fetches eyalamirmusic/eacp@jp/android-vulkan-1-1, as in tools/android.sh.
set -euo pipefail

root="$(cd "$(dirname "$0")/.." && pwd)"
cd "$root"

keystore="${COWS_ANDROID_KEYSTORE:?set COWS_ANDROID_KEYSTORE to the upload keystore (.jks)}"
alias="${COWS_ANDROID_KEY_ALIAS:?set COWS_ANDROID_KEY_ALIAS to the upload key alias}"
: "${COWS_ANDROID_KEYSTORE_PASSWORD:?set COWS_ANDROID_KEYSTORE_PASSWORD to the keystore password}"
[[ -f "$keystore" ]] || { echo "no keystore at $keystore" >&2; exit 1; }

build_number="${COWS_BUILD_NUMBER:-1}"
bundle_id="${COWS_BUNDLE_ID:-com.cowsinlove.cows}"
abis=(${COWS_ANDROID_ABIS:-arm64-v8a x86_64})
min_sdk=33
sdk="${ANDROID_HOME:-$HOME/Library/Android/sdk}"
ndk="${ANDROID_NDK:-$sdk/ndk/30.0.16248370}"
build_tools="$sdk/build-tools/35.0.0"
platform_jar="$sdk/platforms/android-35/android.jar"
llvm="$ndk/toolchains/llvm/prebuilt/darwin-x86_64/bin"
eacp="${EACP:-$HOME/projects/eacp-android-integration}"
adb="$sdk/platform-tools/adb"
out=Deploy/Google-Play/out
res=Apps/CowsInLove/Android/res

bundletool_version=1.18.3
bundletool_sha256=a099cfa1543f55593bc2ed16a70a7c67fe54b1747bb7301f37fdfd6d91028e29
bundletool="$sdk/bundletool/bundletool-all-$bundletool_version.jar"

# macOS ships a /usr/bin/java stub that fails without a JDK installed.
if [[ -z "${JAVA_HOME:-}" ]] && ! java -version >/dev/null 2>&1; then
    for candidate in /opt/homebrew/opt/openjdk@21 /opt/homebrew/opt/openjdk; do
        [[ -x "$candidate/bin/java" ]] && export JAVA_HOME="$candidate" && break
    done
fi
[[ -n "${JAVA_HOME:-}" ]] && export PATH="$JAVA_HOME/bin:$PATH"

if [[ ! -f "$bundletool" ]]; then
    mkdir -p "$(dirname "$bundletool")"
    curl -sSLf -o "$bundletool.part" "https://github.com/google/bundletool/releases/download/$bundletool_version/bundletool-all-$bundletool_version.jar"
    echo "$bundletool_sha256  $bundletool.part" | shasum -a 256 -c - >&2
    mv "$bundletool.part" "$bundletool"
fi

eacp_args=(-DCPM_eacp_SOURCE="$eacp")
[[ -d $eacp ]] || eacp_args=(-DCOWS_EACP_REPOSITORY=eyalamirmusic/eacp
                             -DCOWS_EACP_TAG=jp/android-vulkan-1-1)

work="$(mktemp -d)"
trap 'rm -rf "$work"' EXIT
mkdir -p "$work/base" "$work/symbols"

for abi in "${abis[@]}"; do
    build="build-android-release-$abi"
    cmake -G Ninja -B "$build" -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_TOOLCHAIN_FILE="$ndk/build/cmake/android.toolchain.cmake" \
        -DANDROID_ABI="$abi" -DANDROID_PLATFORM="android-$min_sdk" \
        -DANDROID_SUPPORT_FLEXIBLE_PAGE_SIZES=ON \
        "${eacp_args[@]}" -DCOWS_BUILD_TESTS=OFF \
        -DCOWS_BUILD_NUMBER="$build_number" -DCOWS_BUNDLE_ID="$bundle_id" >&2
    cmake --build "$build" --target Cows >&2

    library="$build/Apps/CowsInLove/libCows.so"
    mkdir -p "$work/base/lib/$abi" "$work/symbols/$abi"
    "$llvm/llvm-strip" --strip-unneeded -o "$work/base/lib/$abi/libCows.so" "$library"
    "$llvm/llvm-objcopy" --strip-debug "$library" "$work/symbols/$abi/libCows.so.sym"
    manifest="$build/Apps/CowsInLove/Cows-AndroidManifest.xml"
done

# The base module: the manifest and resources linked in protobuf format, laid
# out as bundletool wants them (manifest/, res/, resources.pb, lib/).
"$build_tools/aapt2" compile --dir "$res" -o "$work/res.zip"
"$build_tools/aapt2" link --proto-format -o "$work/linked.apk" \
    --manifest "$manifest" -I "$platform_jar" -R "$work/res.zip" --auto-add-overlay
unzip -q "$work/linked.apk" -d "$work/base"
mkdir -p "$work/base/manifest"
mv "$work/base/AndroidManifest.xml" "$work/base/manifest/"
(cd "$work/base" && zip -qr "$work/base.zip" .)

version="$(sed -n 's/.*android:versionName="\([^"]*\)".*/\1/p' "$manifest")"
aab="$out/CowsInLove-$version-$build_number.aab"
mkdir -p "$out"
rm -f "$aab"

metadata=()
for abi in "${abis[@]}"; do
    metadata+=(--metadata-file="com.android.tools.build.debugsymbols/$abi/libCows.so.sym:$work/symbols/$abi/libCows.so.sym")
done

java -jar "$bundletool" build-bundle --modules="$work/base.zip" \
    --config=Deploy/Google-Play/BundleConfig.json "${metadata[@]}" \
    --output="$aab"

jarsigner -sigalg SHA256withRSA -digestalg SHA-256 -keystore "$keystore" \
    -storepass:env COWS_ANDROID_KEYSTORE_PASSWORD "$aab" "$alias" >/dev/null
jarsigner -verify "$aab" | grep -q "jar verified" \
    || { echo "$aab: signature does not verify" >&2; exit 1; }
echo "$aab"

[[ "${1:-}" == --install ]] || exit 0

java -jar "$bundletool" validate --bundle="$aab" >&2
apks="$work/CowsInLove.apks"
(umask 077 && printf '%s' "$COWS_ANDROID_KEYSTORE_PASSWORD" > "$work/ks-pass")
java -jar "$bundletool" build-apks --bundle="$aab" --output="$apks" \
    --local-testing --connected-device --adb="$adb" \
    --ks="$keystore" --ks-key-alias="$alias" \
    --ks-pass="file:$work/ks-pass"
"$adb" uninstall "$bundle_id" >/dev/null 2>&1 || true
java -jar "$bundletool" install-apks --apks="$apks" --adb="$adb"
"$adb" shell am start -n "$bundle_id/android.app.NativeActivity" >&2
