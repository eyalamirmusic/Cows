# Cows In Love — Steam store page

Paste into Steamworks > Store Page. **FILL IN** marks values from the
publishing account.

## Basic info

| Field | Value |
| --- | --- |
| Name | Cows In Love |
| Developer | **FILL IN** |
| Publisher | **FILL IN** |
| Genres | Casual, Adventure, Indie |
| Tags | Cute, Casual, Relaxing, Cozy, 3D, Exploration, Short, Family Friendly, Wholesome, Colorful |
| Supported languages | English (interface, full audio: none needed beyond the moo) |
| Controller | Keyboard and mouse (no controller support declared) |
| Price | **FILL IN** |
| Content descriptors | None |

## Short description (300)

A cow is looking for her love in a misty meadow. Moo and listen for her
answer, hop fences and logs, time your way across a ravine bridge while hay
bales roll by, and find her.

## About this game

Somewhere in a foggy meadow, the other cow is waiting.

Cows In Love is a small, gentle 3D game about finding someone. Walk your cow
through hedgerows, groves, hay bales and barns. You can't see far, so moo: she
moos back, and you can hear whether she is close by or far off, and which way.
The nearer you get, the faster both your hearts beat.

Then the meadow gives way. Hop logs and fences, cross a ravine on a narrow
plank bridge while bales of hay roll across it, and find her on the other side.

When you do, the two of you meet nose to nose, the hearts come out, and the
next meadow begins.

**Features**
- A fresh meadow every round
- Moo for a hint: she answers from where she is, in stereo
- Jump fences, logs and barn roofs
- A ravine crossing that is all about timing
- Walk with WASD, HJKL or the arrow keys; space to jump; M to moo

Based on the "cows in love" terminal animation (ssh ssh.cowsinlove.com).

## System requirements

**Windows**

| | Minimum | Recommended |
| --- | --- | --- |
| OS | Windows 10 64-bit | Windows 11 64-bit |
| Processor | Dual-core 64-bit | Quad-core 64-bit |
| Memory | 4 GB RAM | 8 GB RAM |
| Graphics | DirectX 12 capable GPU (feature level 11_0) | Any GPU from 2018 or later |
| DirectX | Version 12 | Version 12 |
| Storage | 50 MB | 50 MB |
| Sound | Any | Stereo |

**macOS**

| | Minimum | Recommended |
| --- | --- | --- |
| OS | macOS 11 Big Sur | macOS 14 or later |
| Processor | Apple silicon or Intel 64-bit (universal binary) | Apple silicon |
| Memory | 4 GB RAM | 8 GB RAM |
| Graphics | Metal capable GPU | Apple silicon GPU |
| Storage | 50 MB | 50 MB |

## Launch options (Steamworks > Installation > General)

| OS | Executable | Arguments |
| --- | --- | --- |
| Windows | `Cows.exe` | none |
| macOS | `Cows.app` | none |

No installscript and no redistributables: the Windows exe links the C
runtime statically and only needs DLLs that ship with Windows 10 (Direct3D 12,
DXGI, D3DCompiler_47, Direct2D, DirectWrite).

## Store and library art (Deploy/Steam/Store)

| File | Size | Steamworks slot |
| --- | --- | --- |
| header_capsule.png | 920 x 430 | Header capsule |
| small_capsule.png | 462 x 174 | Small capsule |
| main_capsule.png | 1232 x 706 | Main capsule |
| vertical_capsule.png | 748 x 896 | Vertical capsule |
| page_background.png | 1438 x 810 | Page background (optional) |
| library_capsule.png | 600 x 900 | Library capsule |
| library_header.png | 920 x 430 | Library header |
| library_hero.png | 3840 x 1240 | Library hero (no text) |
| library_logo.png | 1280 x 720 | Library logo (transparent) |
| community_icon.jpg | 184 x 184 | Community icon |
| client_icon.ico | 16-256 | Client icon (Installation > Client Icon) |
| ../Screenshots/*.png | 1920 x 1080 | Screenshots (6) |

Regenerate with `tools/store-art.sh` (art) and `tools/screenshots.sh`.
