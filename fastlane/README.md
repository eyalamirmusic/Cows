fastlane documentation
----

# Installation

Make sure you have the latest version of the Xcode command line tools installed:

```sh
xcode-select --install
```

For _fastlane_ installation instructions, see [Installing _fastlane_](https://docs.fastlane.tools/#installing-fastlane)

# Available Actions

## Android

### android validate_credentials

```sh
[bundle exec] fastlane android validate_credentials
```

Validate the Google Play Publishing API credential

### android upload

```sh
[bundle exec] fastlane android upload
```

Upload the signed AAB to a Play track (internal by default)

### android promote

```sh
[bundle exec] fastlane android promote
```

Promote a release between Play tracks (production draft by default)

----

This README.md is auto-generated and will be re-generated every time [_fastlane_](https://fastlane.tools) is run.

More information about _fastlane_ can be found on [fastlane.tools](https://fastlane.tools).

The documentation of _fastlane_ can be found on [docs.fastlane.tools](https://docs.fastlane.tools).
