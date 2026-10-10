# Google Play initial upload checklist

The first bundle is uploaded in Play Console. That creates the artifact history
Fastlane's Publishing API integration expects. After this bootstrap release,
use the repository's `play-upload`, `play-promote`, and `deploy-android`
commands.

## 1. Create and protect the upload identity

- [ ] Create the Play Console app as **Cows In Love**, **Game**, English
  (United States). Decide Free or Paid carefully: a free app cannot later
  become paid.
- [ ] Confirm the package name will be `com.cowsinlove.play`. The first bundle
  permanently binds this package name to the app.
- [ ] Generate the upload key and put its only durable copy in 1Password:

  ```bash
  just play-setup-key
  ```

  The script creates the personal **Cows In Love** vault and **Google Play**
  item if needed, generates the key under `/tmp`, attaches it as
  `upload-keystore.jks`, reads it back to verify its SHA-256 digest, and removes
  both temporary copies. Re-running it never replaces an existing key.
- [ ] This is the upload identity; Google Play App Signing holds the separate
  key installed builds use. Do not export another durable copy unless making a
  deliberate encrypted backup.

## 2. Enable CLI access for later releases

- [ ] Use the dedicated `cows-in-love-play` Google Cloud project, with the
  [Google Play Developer API](https://developers.google.com/android-publisher/getting_started)
  enabled.
- [ ] Create the dedicated service-account JSON and store its only durable
  copy in the same 1Password item:

  ```bash
  just play-setup-api
  ```

  The script creates a key for
  `cows-play-publisher@cows-in-love-play.iam.gserviceaccount.com`, attaches it
  as `service-account.json`, verifies the stored bytes, and removes the local
  copies. `tools/play-run.sh` materializes both credentials only for the
  lifetime of each build or upload command.
- [ ] In Play Console **Users and permissions**, invite the service-account
  email and give it app access to Cows In Love with:
  - View app information (read-only)
  - Release apps to testing tracks
  - Release to production, exclude devices, and use Play App Signing
- [ ] Install Fastlane: `brew install fastlane`.
- [ ] Validate the credential: `just play-validate`.
- [ ] Confirm the service account can read the app: `just play-check`. Play
  permission changes can take time to propagate; retry later if the Console
  permissions are correct but this still reports `caller does not have
  permission`.

## 3. Complete the Play Console forms

- [ ] Fill the main store listing from
  [`Metadata/store-listing.md`](Metadata/store-listing.md).
- [ ] Upload `Store/icon_512.png`, `Store/feature_graphic_1024x500.png`, and
  all phone, 7-inch tablet, and 10-inch tablet screenshots in filename order.
- [ ] Set category to **Game > Casual** and add the support email and website.
- [ ] Publish `https://cowsinlove.com/privacy` and enter it as the privacy
  policy URL.
- [ ] Complete App content using the prepared answers: no ads, unrestricted
  app access, no collected/shared data, no Advertising ID, no financial or
  health features, not a news or government app, and the documented IARC and
  target-audience answers.
- [ ] Finish pricing/countries and any developer-profile tasks still shown on
  the dashboard.

## 4. Build and inspect the first bundle

- [ ] Pick a `versionCode` that has never been uploaded. The repo currently
  defaults to 6; every future upload must be higher/unused.
- [ ] Build the upload-signed bundle:

  ```bash
  just release-android
  ```

- [ ] Confirm the output exists:

  ```text
  build-android/AndroidStudio/Cows/build/outputs/bundle/release/Cows-release.aab
  ```

- [ ] Inspect its signing certificate and save the SHA-256 fingerprint with
  the release record:

  ```bash
  keytool -printcert -jarfile \
    build-android/AndroidStudio/Cows/build/outputs/bundle/release/Cows-release.aab
  ```

## 5. Upload and test the bootstrap release

- [ ] In Play Console, open **Testing > Internal testing > Create new
  release**.
- [ ] Accept **Let Google manage and protect your app signing key** when Play
  App Signing is offered.
- [ ] Upload `Cows-release.aab` and confirm Play reports the intended package,
  version name, version code, API levels, and supported devices.
- [ ] Add the `en-US` release note: `First release.`
- [ ] Save, review, and roll out the internal release.
- [ ] Add the tester account/list, open the opt-in link, and install the Play
  build on a real Android 13+ device. Confirm launch, sound, touch controls,
  rotation lock, background/resume, and a complete game round.
- [ ] Review the pre-launch report. Devices below the declared Vulkan 1.1 or
  Android 13 floors being incompatible is expected; crashes on supported
  devices are not.

## 6. Use the CLI after bootstrap

- [ ] For the next unused version code, build and upload to internal testing:

  ```bash
  just deploy-android 7
  ```

- [ ] After testing, prepare a production draft:

  ```bash
  just play-promote
  ```

- [ ] Inspect the draft in Play Console, then either submit it there or run an
  explicit completed promotion:

  ```bash
  just play-promote internal production completed true
  ```

- [ ] Confirm the release enters review/rollout and retain the upload bundle,
  version code, commit, and signing-certificate fingerprint in the release
  record.
