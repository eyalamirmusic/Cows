# Cows In Love — Google Play Console metadata

Paste these into Play Console (the app's Dashboard walks through each
section). Lines marked **FILL IN** need a value from the publishing account.

## Create app

| Field | Value |
| --- | --- |
| App name (30) | Cows In Love |
| Default language | English (United States) – en-US |
| App or game | Game |
| Free or paid | **FILL IN** (Free cannot later become paid) |
| Package name | com.cowsinlove.play (fixed by the first upload: `COWS_BUNDLE_ID`) |

## Main store listing

**App name (30)**: Cows In Love

**Short description (80)**

Find her in the misty meadow. Moo, listen, hop the fences, and fall in love.

**Full description (4000)**

Somewhere in a foggy meadow, the other cow is waiting.

Cows In Love is a small, gentle 3D game about finding someone. Walk your cow
through hedgerows, groves, hay bales and barns. You can't see far, so moo: she
moos back, and you can hear whether she is close by or far off, and which way.
The nearer you get, the faster both your hearts beat.

Then the meadow gives way. Hop logs and fences, cross a ravine on a narrow
plank bridge while bales of hay roll across it, and find her on the other side.

When you do, the two of you meet nose to nose, the hearts come out, and the
next meadow begins.

• A fresh meadow every round
• Moo for a hint: she answers from where she is
• Jump fences, logs and barn roofs
• A ravine crossing that is all about timing
• No ads, no accounts, no data collected

Based on the "cows in love" terminal animation (ssh ssh.cowsinlove.com).

**Graphics** (from `tools/store-art.sh` and `tools/screenshots.sh`):

| Slot | File | Size |
| --- | --- | --- |
| App icon | `Store/icon_512.png` | 512 x 512, 32-bit PNG, opaque |
| Feature graphic | `Store/feature_graphic_1024x500.png` | 1024 x 500, no alpha |
| Phone screenshots | `Screenshots/phone/*.png` | 1080 x 1920 (9:16), 6 |
| 7-inch tablet screenshots | `Screenshots/tablet-7/*.png` | 1440 x 2560 (9:16), 6 |
| 10-inch tablet screenshots | `Screenshots/tablet-10/*.png` | 2160 x 3840 (9:16), 6 |
| Video | **FILL IN** a YouTube URL, or leave empty | |

Upload order: 1-meadow, 2-moo, 3-ravine, 4-bridge, 5-jump, 6-found.

## Store settings

| Field | Value |
| --- | --- |
| Category | Game > Casual |
| Tags (up to 5) | Casual, Adventure, Relaxing, Cute, Offline (pick the nearest from Play's list) |
| Email address | **FILL IN** support email |
| Phone | optional |
| Website | https://cowsinlove.com |
| External marketing | leave on |

## Release notes (Release > Create new release, 500 per language)

```
<en-US>
First release.
</en-US>
```

## App content (Policy > App content)

**Privacy policy**: **FILL IN** the hosted URL of
`Deploy/Apple-iOS/Metadata/privacy-policy.md` (one policy for every store).

**Ads**: No, my app does not contain ads.

**App access**: All functionality is available without special access (no
login, no accounts).

**Content rating (IARC questionnaire)**: category **Game**. Answer **No** to
violence, blood, fear, sexuality, nudity, language, crude humour, controlled
substances, gambling and simulated gambling, user interaction or user-generated
content, sharing the user's location, digital purchases, and unrestricted
internet. Email: **FILL IN**. Expected result: ESRB Everyone, PEGI 3, USK 0,
IARC 3+.

**Target audience and content**: age groups **13–15, 16–17, 18 and over**;
"Appeals to children": answer honestly — the art is cute, so say it may.
The app is not designed for children unless Jamie decides otherwise. Choosing
any under-13 group instead puts the app under Google Play's Families policy:
a Families self-certification, stricter review, and rules on ads and data
collection. Cows In Love already meets those rules (no ads, no SDKs, no data,
no network), so opting in later is paperwork, not code.

**News app**: No.

**COVID-19 contact tracing and status apps**: My app is not a publicly
available COVID-19 contact tracing or status app.

**Data safety**:

| Question | Answer |
| --- | --- |
| Does your app collect or share any of the required user data types? | **No** |
| Is all of the user data collected by your app encrypted in transit? | not asked once the answer above is No |
| Do you provide a way for users to request that their data is deleted? | not asked |
| Third-party SDKs | none: the app contains no analytics, ads, crash reporting or network code |

The app declares no permissions (not even `INTERNET`); audio output needs
none.

**Government apps**: No.

**Financial features**: My app doesn't provide any financial features.

**Health**: My app does not have any health features.

**Advertising ID**: No, the app does not use advertising ID (it declares no
`com.google.android.gms.permission.AD_ID`).

## Device support

- Vulkan 1.1 is required (`android.hardware.vulkan.version` 0x401000 in the
  manifest), with synchronization2, timeline semaphores and descriptor
  indexing as extensions where the driver is below 1.3. Many 2022 flagships
  still report 1.1 (a Galaxy S22's Adreno 730 does); eacp renders through
  render passes there. Phones whose driver is below 1.1 see "not compatible".
- Android 13 (API 33) and later, arm64-v8a and x86_64 (Chromebooks, emulators).
- Portrait only. Phones and tablets: the game lays itself out for any
  portrait screen, so tablets are supported and the tablet screenshots are
  provided.
