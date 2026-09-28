# Steam

One app, two depots: Windows (`content/windows/Cows.exe`) and macOS
(`content/macos/Cows.app`). `tools/release-windows.sh` and
`tools/release-macos.sh` stage them; `tools/steam-upload.sh` fills in the
`scripts/*.vdf` templates and runs steamcmd. The whole runbook is in
`Deploy/README.md`; store text, system requirements, launch options and the
art slot list are in `Store/copy.md`.

## Steam Deck

There is no separate Deck build: the Deck runs the Windows depot through
Proton (Direct3D 12 via vkd3d-proton). In Steamworks:

1. Installation > Linux / SteamOS: leave empty (no native build).
2. Steam Deck compatibility review questionnaire: default controller
   configuration **yes** (the template below), text legible (the footer is
   13 pt over the scene; mark it as small text if the reviewer asks),
   no launcher, no anti-cheat, works offline.
3. The game reads keyboard and mouse only today, so a default Steam Input
   configuration is required for Deck and controllers:
   `controller_config.vdf` maps the left stick and D-pad to W/A/S/D, A (and B)
   to space (jump), X to M (moo), Y to R (new meadow), the right trackpad to a
   mouse with click for drag-to-look, and Start to Escape (quit). Load it on a
   Deck or with Steam's controller configurator (Big Picture > Controller
   settings for the game > Browse configs > Import), check it, then publish
   it from there and select it as the official default in Steamworks >
   Application > Steam Input. Treat the file as a starting template: Steam
   rewrites it on export.

Proton needs to be verified on a Deck before choosing "Verified" — nothing
in the game (D3D12, audio through miniaudio's WASAPI backend, a plain Win32 window) is
known to be a problem.

Linux native later: eacp has a Vulkan backend and MakeASound builds on ALSA,
so a Linux depot is a matter of a Linux build of eacp's windowing; not
attempted for 1.0.
