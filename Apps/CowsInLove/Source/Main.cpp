#include "CowsApp.h"

int main()
{
    Cows::Platform::importSettings();
    return Apps::run<Cows::CowsApp>();
}
