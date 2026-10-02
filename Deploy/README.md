# Shipping Cows In Love

Everything a release needs is generated and scripted here; what is left is
the publishing account's credentials. Values marked **FILL IN** come from that
account. Nothing in this folder belongs to any other account.

```
Deploy/
  Art/Renders/        key art, icon and logo rendered by the game (gitignored;
                      tools/store-art.sh regenerates it)
  Apple-macOS/
    Cows.entitlements     App Sandbox, the only entitlement the Mac App Store build needs
    ExportOptions.plist   Mac App Store export
    Metadata/             app-store.md (differences from the iOS listing)
    Screenshots/          2880x1800
    out/                  archive and .pkg (gitignored)
  Apple-iOS/
    ExportOptions.plist   App Store Connect export (team filled in by the script)
    Metadata/             app-store.md (every App Store Connect field),
                          privacy-policy.md
    Screenshots/          iPhone-6.9 (1320x2868), iPhone-6.5 (1284x2778)
    out/                  archive and .ipa (gitignored)
  Steam/
    scripts/              app_build.vdf, depot_build_{windows,macos}.vdf
                          templates (@APP_ID@ etc. filled by steam-upload.sh)
    Store/                capsules, library art, icons, copy.md (store text,
                          system requirements, launch options)
    Screenshots/          1920x1080
    content/              staged depots (gitignored)
    controller_config.vdf Steam Input template (Steam Deck / controllers)
    README.md             Steam Deck and controller notes
  Google-Play/
    Metadata/             store-listing.md (every Play Console field, Data
                          safety, IARC, target audience)
    Store/                512 icon, 1024x500 feature graphic
    Screenshots/          phone (1080x1920), tablet-7 (1440x2560),
                          tablet-10 (2160x3840)
  Microsoft-Store/
    AppxManifest.xml      MSIX manifest template (identity from Partner Center)
    Assets/               MSIX tiles, scale-100/200 and taskbar sizes
    Store/                2:3 poster and 1:1 box art
    Metadata/             store-listing.md (Partner Center fields, IARC answers)
    out/                  .msix (gitignored)
```

App icons live with the app: `Apps/CowsInLove/Resources/Assets.xcassets`
(iOS, 1024 single size) and `AppIcon-Desktop.png` (the macOS .icns and the
Windows .ico are made from it at build time by `eacp_set_app_icon`), and
`Apps/CowsInLove/Android/res` (the Android adaptive icon: foreground, sky
background colour, monochrome hearts for themed icons).

## What to fill in

| Variable | Used by | Value |
| --- | --- | --- |
| `COWS_BUNDLE_ID` | CMake (macOS + iOS), release scripts | the bundle id registered on the Apple account; default `com.cowsinlove.cows` |
| `COWS_COPYRIGHT` | CMake (Info.plist) | e.g. `© 2026 <legal name>`; default `© 2026 Cows In Love` |
| `COWS_TEAM` | tools/release-ios.sh | Apple Developer team id (10 chars) |
| `COWS_SIGN_IDENTITY` | tools/release-macos.sh | `Developer ID Application: <Name> (<TEAM>)` |
| `COWS_NOTARY_PROFILE` | tools/release-macos.sh | a `notarytool store-credentials` profile name |
| `COWS_STEAM_APP_ID` | tools/steam-upload.sh | Steam app id |
| `COWS_STEAM_DEPOT_WINDOWS` | tools/steam-upload.sh | Windows depot id (usually app id + 1) |
| `COWS_STEAM_DEPOT_MACOS` | tools/steam-upload.sh | macOS depot id (usually app id + 2) |
| `COWS_STEAM_USER` | tools/steam-upload.sh | Steam build account login |
| `COWS_BUILD_NUMBER` | tools/release-ios.sh, release-mas.sh | raise for every App Store upload of one version |
| `COWS_BUILD_NUMBER` | CMake (Android) | the Play versionCode: raise for every upload, never reuse |
| `EACP_ANDROID_KEYSTORE` | eacp's `Cows-aab` | path to the upload keystore (.jks), kept out of the repo |
| `EACP_ANDROID_KEY_ALIAS` | eacp's `Cows-aab` | the upload key's alias in it |
| `EACP_ANDROID_KEYSTORE_PASSWORD` | eacp's `Cows-aab` | the keystore (and key) password |
| `COWS_MSIX_IDENTITY` | tools/release-msix.ps1 | Partner Center Package/Identity/Name |
| `COWS_MSIX_PUBLISHER` | tools/release-msix.ps1 | Partner Center Package/Identity/Publisher (`CN=...`) |
| `COWS_MSIX_PUBLISHER_NAME` | tools/release-msix.ps1 | Partner Center PublisherDisplayName |

The version is `project(Cows VERSION x.y.z)` in `CMakeLists.txt`: it becomes
CFBundleShortVersionString, the Android versionName, the Windows VERSIONINFO
and the Steam build description. Release 1.0.0.

## Before every release

```bash
just test            # macOS build + every test suite
just store-art       # icons and store art (only when the art changes)
just screenshots     # store screenshots (only when the game looks different)
```

## App Store (iOS)

One-time, on the Apple account:
1. Register the bundle id (`COWS_BUNDLE_ID`) in Certificates, Identifiers &
   Profiles, and create the app in App Store Connect with it.
2. Sign Xcode into the account (Xcode > Settings > Accounts). Automatic signing
   creates the distribution certificate and profile on first export.
3. Fill in App Store Connect from `Deploy/Apple-iOS/Metadata/app-store.md`,
   host `privacy-policy.md`, upload the screenshots.

Each release:
```bash
COWS_TEAM=<team id> COWS_BUILD_NUMBER=<n> just release-ios --upload
```
This archives Release (iPhone-only, iOS 15+), exports with
`ExportOptions.plist` (app-store-connect, symbols uploaded) and uploads. Without
`--upload` the .ipa lands in `Deploy/Apple-iOS/out/` for Transporter. Then
pick the build on the version page and **Submit for Review**.

Checked on this Mac with Jamie's Personal Team: the archive builds and signs;
export stops at "does not have permission to create iOS App Store
provisioning profiles", which a paid team fixes.

## Mac App Store

One-time: the same bundle id and App Store Connect app as iOS (add the macOS
platform), or its own record; fill it from `Deploy/Apple-macOS/Metadata`.

Each release:
```bash
COWS_TEAM=<team id> COWS_BUILD_NUMBER=<n> just release-mas --upload
```
This archives a sandboxed (`Cows.entitlements`), universal Release build with
the Xcode generator and exports it with method app-store-connect: Xcode signs
the app with Apple Distribution and the .pkg with 3rd Party Mac Developer
Installer, creating both on first export. Without `--upload` the .pkg lands
in `Deploy/Apple-macOS/out/` for Transporter.

Checked on this Mac with the Personal Team: the archive builds, is universal
and sandboxed, and runs sandboxed without denials; export stops at "does not
have permission to create Mac App Store provisioning profiles" / no "Mac
Installer Distribution" certificate, which a paid team fixes.

## Steam (macOS + Windows, one app, two depots)

One-time, on the Steamworks account:
1. Create the app; add a Windows depot and a macOS depot (Steamworks > SteamPipe
   > Depots) and note their ids.
2. Installation > General: launch options from `Deploy/Steam/Store/copy.md`
   (Windows `Cows.exe`, macOS `Cows.app`); Client Icon `client_icon.ico`.
3. Store page and library art from `Deploy/Steam/Store/` (copy.md lists every
   slot and size) and `Deploy/Steam/Screenshots/`.
4. On the Mac that notarizes: a Developer ID Application certificate in a
   keychain, and `xcrun notarytool store-credentials <profile> --apple-id <id>
   --team-id <TEAM> --password <app-specific password>`.
5. `brew install steamcmd` (already on this Mac) and log in once
   interactively (`steamcmd +login <user> +quit`) to pass Steam Guard.

Each release:
```bash
COWS_SIGN_IDENTITY="Developer ID Application: <Name> (<TEAM>)" \
COWS_NOTARY_PROFILE=<profile> just release-macos   # universal, signed, notarized
just release-windows                                # builds on the Windows box
COWS_STEAM_APP_ID=<app> COWS_STEAM_DEPOT_WINDOWS=<depot> \
COWS_STEAM_DEPOT_MACOS=<depot> COWS_STEAM_USER=<login> just steam-upload
```
`release-windows` runs `tools/build-windows.bat` directly on Windows (git-bash),
or from the Mac copies the tree to `COWS_WINDOWS` (default
`jamie@tamby-windows`) and copies the exe back. The exe links the C runtime
statically, so there is no redistributable to install.

Steam Deck runs the Windows depot through Proton; see `Deploy/Steam/README.md`
for the compatibility answers and the default controller configuration
(`controller_config.vdf`), which the Deck needs because the game reads keyboard
and mouse only.

`steam-upload` refuses a macOS app that is not notarized. The build is
uploaded without going live; set it live on the default branch in Steamworks
(Builds), or pass `COWS_STEAM_BRANCH=<beta>` to set a beta branch live.

## Google Play

One-time, on the Play Console account:
1. Create the app (Game, the name and default language from
   `Deploy/Google-Play/Metadata/store-listing.md`). The package name is fixed
   by the first upload: `COWS_BUNDLE_ID` (default `com.cowsinlove.cows`).
2. Make the upload key and keep it (and its password) outside the repo:
   ```bash
   keytool -genkeypair -v -keystore ~/keys/cows-upload.jks -alias upload \
       -keyalg RSA -keysize 4096 -validity 10000
   ```
3. Play App Signing: on the first release accept "Let Google manage and
   protect your app signing key" (the default). Google holds the key that
   signs what users install; the key above only proves uploads are yours, and
   Play Console can reset it if it is lost.
4. Fill in the listing and App content from `Metadata/store-listing.md`, host
   the privacy policy, upload `Store/` and `Screenshots/`.

Each release:
```bash
cmake --preset android -B build-android -DCOWS_BUILD_NUMBER=<n>
EACP_ANDROID_KEYSTORE=~/keys/cows-upload.jks EACP_ANDROID_KEY_ALIAS=upload \
EACP_ANDROID_KEYSTORE_PASSWORD=<password> \
    cmake --build build-android --target Cows-aab
```
eacp's `Cows-aab` builds Release `libCows.so` for arm64-v8a and x86_64 (each
in `build-android/aab/<abi>`, stripped, with the symbol tables in the bundle for
Play's native crash reports), packages them with a pinned `bundletool`, signs
with `jarsigner` and validates: `build-android/Apps/CowsInLove/Cows.aab`. No
Gradle. To try it on the emulator, `bundletool build-apks --local-testing
--connected-device` and `install-apks` with the same key.

Then, the one manual step: in Play Console, **Testing > Internal testing >
Create new release**, upload the .aab, paste the release notes, roll out.
Install from the internal testing link on a real phone, then promote the
release to Production and **Send for review**. Start with internal testing:
it is available within minutes, needs no review, and the pre-launch report
runs the app on real devices (the Vulkan 1.1 floor means only very old test
devices report "not compatible", which is expected).

`jarsigner` warns that the upload certificate is self-signed and has no
timestamp: that is normal for an upload key.

Checked on this Mac with a throwaway upload key: the bundle builds (11 MB,
both ABIs), validates, installs through `--local-testing` on the `cows`
emulator and runs (`docs/shots/play-aab.png`); the generated APKs pass
`zipalign -c -P 16` and `aapt2 dump badging` shows minSdk 33, targetSdk 35,
isGame, no permissions.

## Microsoft Store (MSIX)

One-time: reserve the name in Partner Center, and fill in the listing from
`Deploy/Microsoft-Store/Metadata/store-listing.md`.

Each release, on a Windows machine with the Windows SDK (tamby-windows has
10.0.26100):
```powershell
$env:COWS_MSIX_IDENTITY = '<Package/Identity/Name>'
$env:COWS_MSIX_PUBLISHER = '<Package/Identity/Publisher>'
$env:COWS_MSIX_PUBLISHER_NAME = '<PublisherDisplayName>'
powershell -ExecutionPolicy Bypass -File tools\release-msix.ps1
```
It builds the Release exe if needed, lays out the package, makes
`resources.pri` and packs `Deploy\Microsoft-Store\out\CowsInLove-<version>.0-x64.msix`.
Upload that to the submission's Packages page: Partner Center signs it.
`-Certificate <pfx> -Password <pw>` signs it locally, only for sideload
testing (the certificate subject must equal the publisher).

Checked on tamby-windows with a test identity: makeappx validates the manifest
and packs the .msix.
