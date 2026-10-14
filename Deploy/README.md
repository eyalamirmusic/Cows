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
background colour, monochrome hearts for themed icons; not built in until eacp
takes an app's own res directory again, so Android uses the desktop PNG).

## What to fill in

| Variable | Used by | Value |
| --- | --- | --- |
| `COWS_BUNDLE_ID` | CMake (macOS + iOS), release scripts | the bundle id registered on the Apple account; default `com.cowsinlove.play` |
| `COWS_COPYRIGHT` | CMake (Info.plist) | e.g. `© 2026 <legal name>`; default `© 2026 Cows In Love` |
| `COWS_TEAM` | tools/release-ios.sh | Apple Developer team id (10 chars) |
| `COWS_SIGN_IDENTITY` | tools/release-macos.sh | `Developer ID Application: <Name> (<TEAM>)` |
| `COWS_NOTARY_PROFILE` | tools/release-macos.sh | a `notarytool store-credentials` profile name |
| `COWS_STEAM_APP_ID` | tools/steam-upload.sh | Steam app id |
| `COWS_STEAM_DEPOT_WINDOWS` | tools/steam-upload.sh | Windows depot id (usually app id + 1) |
| `COWS_STEAM_DEPOT_MACOS` | tools/steam-upload.sh | macOS depot id (usually app id + 2) |
| `COWS_STEAM_USER` | tools/steam-upload.sh | Steam build account login |
| `COWS_ANDROID_KEYSTORE` | tools/play-run.sh, Gradle | ephemeral JKS path populated by `op read` |
| `COWS_ANDROID_KEYSTORE_PASSWORD` | Gradle release signing | upload keystore password |
| `COWS_ANDROID_KEY_ALIAS` | Gradle release signing | upload key alias (normally `upload`) |
| `COWS_ANDROID_KEY_PASSWORD` | Gradle release signing | upload key password |
| `COWS_PLAY_JSON_KEY` | tools/play-run.sh, Fastlane | ephemeral service-account JSON path populated by `op read` |
| `COWS_MSIX_IDENTITY` | tools/release-msix.ps1 | Partner Center Package/Identity/Name |
| `COWS_MSIX_PUBLISHER` | tools/release-msix.ps1 | Partner Center Package/Identity/Publisher (`CN=...`) |
| `COWS_MSIX_PUBLISHER_NAME` | tools/release-msix.ps1 | Partner Center PublisherDisplayName |

The version is `project(Cows VERSION x.y.z.n)` in `CMakeLists.txt`: x.y.z
becomes CFBundleShortVersionString, the Android versionName, the Windows
VERSIONINFO and the Steam build description; n is the build (CFBundleVersion,
the Play versionCode), committed, never a `-D`: raise it for every upload and
never reuse one. Release 1.0.0.

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
2. Make an App Store Connect API key (Users and Access > Integrations > Team
   Keys, role App Manager) and put it in the "Cows In Love" vault of the
   personal 1Password account as an item named "App Store Connect API" with
   the fields `team id`, `key id`, `issuer id`, the `.p8` attached as
   `private key`, and `copyright`, `review name`, `review phone`,
   `review email` for the listing. `Deploy/asc.env` maps them to the
   environment for `op run`, which `tools/asc-run.sh` (the justfile's `asc` prefix) runs; automatic
   signing then creates the distribution certificate and profile on first
   export with no Xcode login. No contact details or secrets go in the repo.
3. The listing is filled from `Deploy/Apple-iOS/Metadata/app-store.md` and
   the screenshots by `just asc all --platform ios --write`
   (`tools/asc-listing.py`; without `--write` it only prints). App Privacy
   ("Data Not Collected") and the price are set in the web UI, which the API
   does not cover.

Each release:
```bash
just release-ios --upload
```
This archives Release (iPhone-only, iOS 15+), exports with
`ExportOptions.plist` (app-store-connect, symbols uploaded) and uploads. Without
`--upload` the .ipa lands in `Deploy/Apple-iOS/out/` for Transporter. Then
pick the build on the version page and **Submit for Review**. Without the
vault, `COWS_TEAM=<team id> tools/release-ios.sh --upload` signs and uploads
through the account Xcode is signed into.

Checked on this Mac with Jamie's Personal Team: the archive builds and signs;
export stops at "does not have permission to create iOS App Store
provisioning profiles", which a paid team fixes.

## Mac App Store

One-time: the same bundle id and App Store Connect app as iOS (add the macOS
platform), or its own record; fill it from `Deploy/Apple-macOS/Metadata`.

Each release:
```bash
COWS_TEAM=<team id> just release-mas --upload
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
   (Windows `Cows.exe`, macOS `Cows In Love.app`); Client Icon `client_icon.ico`.
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

Follow `Google-Play/INITIAL-UPLOAD.md` for the app record, upload key, Play App
Signing, service account, listing, policy forms, first bundle, and real-device
test. The first AAB is uploaded manually to bootstrap the app. Google then
holds the app-signing key; the local JKS only authenticates future uploads.

`tools/play-run.sh` fetches the `upload-keystore.jks` and
`service-account.json` file fields from the **Google Play** item in the
personal 1Password **Cows In Love** vault into a mode-0600 temporary directory.
`Deploy/play.env` injects their passwords with `op run`; the wrapper removes
the files as soon as the command exits. The generated Gradle project reads the
four `COWS_ANDROID_*` variables at build time, so no secret or durable key path
is written to the project. `just play-setup-key` creates and verifies the
upload key directly through the 1Password CLI and will not replace one that
already exists. `just play-setup-api` does the same for the dedicated Google
service-account credential. Install Fastlane once with `brew install fastlane`,
then check API access with `just play-validate`.

Build the first, manually uploaded bundle with an unused version code (the
build in `CMakeLists.txt`):
```bash
just release-android
```
That configures the android preset and runs Gradle's `:Cows:bundleRelease` in
`build-android/AndroidStudio`: Release `libCows.so` for arm64-v8a and x86_64,
in `build-android/AndroidStudio/Cows/build/outputs/bundle/release/Cows-release.aab`.
The release wrapper requires the upload key and verifies the resulting AAB's
JAR signature.

After the first manual upload, build and upload to internal testing together:
```bash
just deploy-android
```
After installing that Play build on a real phone, prepare a production draft
with `just play-promote`. A completed production promotion is deliberately
explicit: `just play-promote internal production completed true`. Fastlane
only changes binaries and tracks; the curated listing and screenshots remain
manual, so an upload cannot unexpectedly replace store content.

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
For x64 and arm64 (`-Arch x64` for one), it builds the Release exe if needed
(`COWS_ARCH` on `build-windows.bat`), lays out the package, makes
`resources.pri` and packs `Deploy\Microsoft-Store\out\CowsInLove-<version>.0-<arch>.msix`.
Upload both to the submission's Packages page: Partner Center signs them.
The arm64 package builds on an Arm64 Windows machine; cross-compiling it on
x64 stops at ResEmbed's generator, which comes out arm64 too
(eyalamirmusic/ResEmbed#4).
`-Certificate <pfx> -Password <pw>` signs it locally, only for sideload
testing (the certificate subject must equal the publisher).

Checked on tamby-windows with a test identity: makeappx validates the manifest
and packs the .msix.
