#include "Settings.h"

#include <eacp/Core/Utils/Environment.h>

#include <sys/system_properties.h>

#include <sstream>
#include <string>

namespace Cows
{
namespace
{
std::string systemProperty(const char* name)
{
    auto* info = __system_property_find(name);
    auto value = std::string {};

    if (info == nullptr)
        return value;

    __system_property_read_callback(
        info,
        [](void* cookie, const char*, const char* value, unsigned)
        { *static_cast<std::string*>(cookie) = value; },
        &value);

    return value;
}
} // namespace

void importSettings()
{
    auto settings = std::istringstream {systemProperty("debug.cows.env")};
    auto setting = std::string {};

    while (settings >> setting)
    {
        auto equals = setting.find('=');

        if (equals != std::string::npos)
            eacp::setEnv(setting.substr(0, equals), setting.substr(equals + 1));
    }
}
} // namespace Cows
