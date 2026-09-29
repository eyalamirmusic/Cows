#include "Settings.h"

#include <sys/system_properties.h>

#include <cstdlib>
#include <sstream>
#include <string>

namespace Cows
{
void importSettings()
{
    char value[PROP_VALUE_MAX] = {};

    if (__system_property_get("debug.cows.env", value) <= 0)
        return;

    auto settings = std::istringstream {value};
    auto setting = std::string {};

    while (settings >> setting)
    {
        auto equals = setting.find('=');

        if (equals != std::string::npos)
            setenv(setting.substr(0, equals).c_str(),
                   setting.substr(equals + 1).c_str(),
                   1);
    }
}
} // namespace Cows
