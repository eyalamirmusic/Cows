#include "CowsApp.h"
#include "Settings.h"

int main()
{
    Cows::importSettings();
    return Apps::run<Cows::CowsApp>();
}
