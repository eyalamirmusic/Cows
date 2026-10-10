#include "Render/QualityGovernor.h"

#include <NanoTest/NanoTest.h>

using namespace nano;
using namespace Cows;

namespace
{
QualityGovernor::Settings settings(int start)
{
    auto made = QualityGovernor::Settings {};
    made.levels = 3;
    made.start = start;
    made.budget = 30.0;
    made.collapse = 3.0;
    made.headroom = 8.0;
    made.window = 10;
    made.windowMilliseconds = 1.0e9;
    made.minimumWindow = 3;
    made.settle = 2;
    made.climbWindows = 3;
    made.decideWindows = 2;
    return made;
}

int feed(QualityGovernor& governor, double milliseconds, int frames)
{
    auto changes = 0;

    for (auto frame = 0; frame < frames; ++frame)
        changes += governor.addFrame(milliseconds) ? 1 : 0;

    return changes;
}
} // namespace

auto tStartsWhereTold = test("QualityGovernor/startsAtItsStartLevel") = []
{
    auto governor = QualityGovernor {settings(1)};
    check(governor.level == 1);
};

auto tSteady = test("QualityGovernor/keepsALevelThatFits") = []
{
    auto governor = QualityGovernor {settings(1)};
    check(feed(governor, 16.0, 200) == 0);
    check(governor.level == 1);
};

auto tDown = test("QualityGovernor/stepsDownWhenOverBudget") = []
{
    auto governor = QualityGovernor {settings(2)};
    feed(governor, 60.0, 12);
    check(governor.level == 1);
    feed(governor, 60.0, 12);
    check(governor.level == 0);
    feed(governor, 60.0, 100);
    check(governor.level == 0);
};

auto tUp = test("QualityGovernor/climbsEarlyWithHeadroom") = []
{
    auto governor = QualityGovernor {settings(1)};
    feed(governor, 4.0, 12);
    check(governor.level == 2);
    feed(governor, 4.0, 100);
    check(governor.level == 2);
};

auto tLateClimb = test("QualityGovernor/climbsOnlyEarlyOn") = []
{
    auto governor = QualityGovernor {settings(1)};
    feed(governor, 16.0, 60);
    feed(governor, 4.0, 100);
    check(governor.level == 1);
};

auto tNoSeeSaw = test("QualityGovernor/neverClimbsBackAfterComingDown") = []
{
    auto governor = QualityGovernor {settings(2)};
    feed(governor, 60.0, 12);
    check(governor.level == 1);
    feed(governor, 4.0, 100);
    check(governor.level == 1);
};

auto tSettles = test("QualityGovernor/ignoresFramesJustAfterAChange") = []
{
    auto governor = QualityGovernor {settings(2)};
    feed(governor, 60.0, 12);
    check(governor.level == 1);
    feed(governor, 1000.0, 2);
    feed(governor, 20.0, 10);
    check(governor.level == 1);
};

auto tNoTimings = test("QualityGovernor/keepsItsLevelWithoutTimings") = []
{
    auto governor = QualityGovernor {settings(1)};
    check(feed(governor, 0.0, 200) == 0);
    check(governor.level == 1);
};

auto tCollapse = test("QualityGovernor/dropsStraightToTheBottomWhenFarOver") = []
{
    auto governor = QualityGovernor {settings(2)};
    feed(governor, 200.0, 12);
    check(governor.level == 0);
};

auto tShortWindow = test("QualityGovernor/judgesSlowFramesInAShorterWindow") = []
{
    auto made = settings(2);
    made.windowMilliseconds = 300.0;
    auto governor = QualityGovernor {made};
    feed(governor, 100.0, 2 + 3);
    check(governor.level == 0);
};

auto tDecides = test("QualityGovernor/decidesOnceTheLevelHolds") = []
{
    auto governor = QualityGovernor {settings(2)};
    feed(governor, 60.0, 12);
    check(!governor.decided());
    feed(governor, 20.0, 2 + 10);
    check(!governor.decided());
    feed(governor, 20.0, 10);
    check(governor.decided());
};
