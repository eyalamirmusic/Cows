#include "Platform.h"

#include <sys/system_properties.h>

#include <cstdlib>
#include <sstream>
#include <string>

namespace Cows
{
void Platform::importSettings()
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

void Platform::attach(Graphics::View& root,
                      TouchControls& touchControls,
                      FooterView& footer)
{
    root.addSubview(touchControls);

    hud = std::make_unique<HudLayer>(touchControls, footer);
    surface = std::make_unique<TouchSurface>(touchControls);
    surface->onSafeAreaChanged = [&footer, &touchControls](Graphics::Insets insets)
    {
        touchControls.safeArea = insets;
        touchControls.resized();
        footer.bottomInset = insets.bottom;
    };
}

void Platform::drawOverlay(GPUView& scene, RenderPass& pass)
{
    if (hud != nullptr)
        hud->draw(scene, pass);
}
} // namespace Cows
