# Cows In Love — Microsoft Store (Partner Center)

**FILL IN** marks values from the Partner Center account.

## Product identity (Product management > Product identity)

Copy these three into the environment before `tools/release-msix.ps1`:

| Partner Center | Variable |
| --- | --- |
| Package/Identity/Name | `COWS_MSIX_IDENTITY` |
| Package/Identity/Publisher | `COWS_MSIX_PUBLISHER` |
| Package/Properties/PublisherDisplayName | `COWS_MSIX_PUBLISHER_NAME` |

## Properties

| Field | Value |
| --- | --- |
| Category | Games > Family & kids (or Games > Other) |
| Privacy policy URL | **FILL IN** (host `Deploy/Apple-iOS/Metadata/privacy-policy.md`) |
| Website | https://cowsinlove.com |
| Support contact | **FILL IN** |
| System requirements | Minimum: Windows 10 version 1809 (17763), x64, DirectX 12 GPU, 4 GB RAM. Input: keyboard and mouse |
| Game settings | Single player; no online features; no cross-device play |
| Accessibility | Not declared |
| Capabilities declared | `runFullTrust` (a desktop game packaged as MSIX) |

`runFullTrust` needs no special justification for a packaged Win32 app; if
Partner Center asks, the answer is "a desktop (Win32) game packaged with the
Desktop Bridge; it uses no other restricted capability".

## Age ratings (IARC questionnaire)

Category: Game. Answer **No** to violence, fear, sexuality, language,
controlled substances, gambling, purchases, user interaction, location sharing
and unrestricted internet. Expected result: ESRB Everyone, PEGI 3.

## Store listing (English)

**Product name**: Cows In Love

**Description**: use "About this game" from `Deploy/Steam/Store/copy.md`.

**Short description**: A cow is looking for her love in a misty meadow. Moo,
listen for her answer, hop the fences, time the bridge, and find her.

**What's new in this version**: First release.

**Product features** (up to 20):
- A fresh meadow every round
- Moo for a hint: she answers from where she is
- Jump fences, logs and barn roofs
- A ravine crossing that is all about timing
- No ads, no accounts, no data collected

**Screenshots**: `Deploy/Steam/Screenshots/*.png` (1920 x 1080; the Store
wants 1366 x 768 or larger), in file order.

**Store logos** (`Deploy/Microsoft-Store/Store`, from `tools/store-art.sh`):
2:3 poster art `poster_1440x2160.png` (minimum 720 x 1080) and 1:1 box art
`boxart_2160x2160.png` (minimum 1080 x 1080).

**Search terms**: cow, cows, love, cute, cozy, casual, meadow, farm

## Pricing and availability

**FILL IN**: price, markets, release date.
