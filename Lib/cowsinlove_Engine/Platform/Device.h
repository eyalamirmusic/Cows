#pragma once

namespace Cows
{
// Whether the controls are on screen and the footer teaches them: phones and
// tablets, and on the web whatever the browser says it is running on.
bool touchScreen();

// Whether q / Esc quits. A browser tab is closed by its user, never by the page.
bool canQuit();
} // namespace Cows
