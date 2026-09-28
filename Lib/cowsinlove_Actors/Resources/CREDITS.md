# Sound credits

`moo.f32` — "Single Cow Moo" by MichaeltheFox8621, a cow mooing at a farm in
Pitlochry, UK. <https://commons.wikimedia.org/wiki/File:Single_Cow_Moo.ogg>,
licensed CC BY-SA 4.0 (<https://creativecommons.org/licenses/by-sa/4.0/>).

Changed from the original: mixed to mono, resampled to 44.1 kHz, silence
trimmed from both ends, normalised to -1 dB, and stored as raw little-endian
32-bit float samples:

```bash
ffmpeg -i Single_Cow_Moo.ogg single.wav
sox single.wav -c 1 -r 44100 trimmed.wav \
    silence 1 0.02 2% reverse silence 1 0.02 2% reverse norm -1
sox trimmed.wav -t f32 -e floating-point -r 44100 -c 1 moo.f32
```
