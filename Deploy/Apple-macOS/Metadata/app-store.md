# Cows In Love — Mac App Store metadata

The Mac App Store listing is a macOS platform on the same App Store Connect
app as iOS (or its own app record): the name, subtitle, description,
keywords, URLs, age rating and App Privacy answers are the ones in
`Deploy/Apple-iOS/Metadata/app-store.md`, with these differences.

| Field | macOS value |
| --- | --- |
| Category | Games (Casual, Adventure); `LSApplicationCategoryType` is `public.app-category.games` |
| Minimum macOS | 11.0 (universal: Apple silicon and Intel) |
| Sandbox | on (`Deploy/Apple-macOS/Cows.entitlements`); no other entitlements: output-only audio, no network, no files |
| Encryption | `ITSAppUsesNonExemptEncryption` false |
| Screenshots | `Deploy/Apple-macOS/Screenshots`, 2880 x 1800 (16:10, one of the accepted 1280x800, 1440x900, 2560x1600, 2880x1800) |

Description differences: replace the touch controls line with

> Walk with WASD, HJKL or the arrow keys, jump with space, moo with M, drag to
> look around and scroll to zoom.

Review notes:

> Single player, no accounts, purchases, ads, network access or data
> collection. WASD / arrows to walk, space to jump, M to moo (she moos back
> from where she is), drag to look. Walking up to her plays the ending and
> the next level starts. Q or Escape quits.
