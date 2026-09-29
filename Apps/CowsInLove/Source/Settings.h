#pragma once

namespace Cows
{
// An app started by `am start` has no environment of its own, so on Android the
// COWS_* settings come from the `debug.cows.env` system property instead:
// `adb shell setprop debug.cows.env "COWS_PROFILE=1 COWS_SEED=3"`. A web page
// has no environment either, so there they come from the query string, lower
// case and without the prefix: `?seed=3&stage=1&freeze` sets COWS_SEED=3,
// COWS_STAGE=1 and COWS_FREEZE=1 (seed, stage, time, freeze, found, profile).
// Elsewhere the environment already has them and this does nothing.
void importSettings();
} // namespace Cows
