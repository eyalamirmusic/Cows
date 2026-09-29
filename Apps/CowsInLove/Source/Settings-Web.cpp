#include "Settings.h"

#include <emscripten/em_js.h>

#include <cstdlib>

// Copies the value of `name` in the page's query string into `value` (at most
// `size` bytes with the terminator) and returns its length, or -1 when it is
// absent. A bare `?freeze` is empty.
EM_JS_DEPS(settingsWeb, "$stringToUTF8,$UTF8ToString");

EM_JS(int, queryParameter, (const char* name, char* value, int size), {
    const found =
        new URLSearchParams(window.location.search).get(UTF8ToString(name));
    return found == null ? -1 : stringToUTF8(found, value, size);
});

// miniaudio resumes its AudioContext on the first click or touchend only, so a
// player who starts with the keys would moo in silence until they clicked. A key
// is a user gesture too: resume on it as well, until the audio runs.
// clang-format off
EM_JS(void, resumeAudioOnKeys, (), {
    const resume = function() {
        const audio = window.miniaudio;

        if (audio === undefined)
            return;

        const suspended = audio.devices.some(function(device) {
            return device != null && device.webaudio != null
                   && device.webaudio.state !== "running";
        });

        if (suspended)
            audio.unlock();
        else
            document.removeEventListener("keydown", resume, true);
    };

    document.addEventListener("keydown", resume, true);
});
// clang-format on

namespace Cows
{
namespace
{
struct Setting final
{
    const char* parameter;
    const char* variable;
};

constexpr Setting settings[] = {{"seed", "COWS_SEED"},
                                {"stage", "COWS_STAGE"},
                                {"time", "COWS_TIME"},
                                {"freeze", "COWS_FREEZE"},
                                {"found", "COWS_FOUND"},
                                {"profile", "COWS_PROFILE"}};
} // namespace

void importSettings()
{
    resumeAudioOnKeys();

    for (const auto& setting: settings)
    {
        char value[64] = {};
        auto length = queryParameter(setting.parameter, value, sizeof(value));

        if (length >= 0)
            setenv(setting.variable, length == 0 ? "1" : value, 1);
    }
}
} // namespace Cows
