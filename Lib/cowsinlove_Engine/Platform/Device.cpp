#include "Platform/Device.h"

#include "Render/Common.h"

namespace Cows
{
bool touchScreen()
{
    return Platform::isIOS() || Platform::isAndroid();
}

bool canQuit()
{
    return true;
}
} // namespace Cows
