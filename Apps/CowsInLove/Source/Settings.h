#pragma once

namespace Cows
{
// An app started by `am start` has no environment of its own, so on Android the
// COWS_* settings come from the `debug.cows.env` system property instead:
// `adb shell setprop debug.cows.env "COWS_PROFILE=1 COWS_SEED=3"`. Elsewhere
// the environment already has them and this does nothing.
void importSettings();
} // namespace Cows
