#include "CommonActivationCore.h"
#include <cassert>
#include <string>
#include <vector>
using namespace CommonActivationCore;
int main()
{
    bool active=false, introed=true;
    std::vector<std::string> events;
    auto is=[&]{return active;};
    auto set=[&](bool value){ active=value; events.push_back(value?"active":"inactive"); };
    auto handler=[&]{
        Notify(true,is,[&]{events.push_back("visibility");},
            [&]{events.push_back("BP");active=false;},
            [&]{events.push_back("native");},[&]{events.push_back("dynamic");});
    };
    Activate(is,[&]{SetActive(true,set,handler);});
    assert((events==std::vector<std::string>{"active","visibility","BP","native","dynamic"}));
    // Reentrant BP state change does not suppress the remaining source broadcasts.
    assert(!active);
    events.clear(); active=true;
    Activate(is,[&]{events.push_back("wrong");}); assert(events.empty());
    Deactivate(is,[&]{SetActive(false,set,[&]{events.push_back("deactivated");});});
    assert((events==std::vector<std::string>{"inactive","deactivated"}));
    Deactivate(is,[&]{events.push_back("wrong");}); assert(events.size()==2);
    events.clear();
    // Completion hook can activate itself; EndIntro must check state after hook.
    EndIntro([&]{events.push_back("complete");active=true;},is,set,[&]{events.push_back("wrong");});
    assert((events==std::vector<std::string>{"complete"}));
    events.clear(); active=false;
    EndIntro([&]{events.push_back("complete");},is,set,handler);
    assert(events.front()=="complete" && events[1]=="active");
    events.clear(); active=false;
    EndOutro([&]{introed=false;events.push_back("clear intro");},
        [&]{assert(!introed);events.push_back("complete");},set,[&]{events.push_back("deactivated");});
    assert((events==std::vector<std::string>{"clear intro","complete","inactive","deactivated"}));
    // Already inactive still completes, clears active and dispatches deactivation.
    events.clear();
    EndOutro([&]{introed=false;},[&]{events.push_back("complete");},set,[&]{events.push_back("deactivated");});
    assert(events.size()==3);
    // Inactive activation handler and active deactivation handler have no effects.
    events.clear();active=false;
    Notify(true,is,[&]{events.push_back("wrong");},[]{},[]{},[]{});
    active=true;Notify(false,is,[&]{events.push_back("wrong");},[]{},[]{},[]{});
    assert(events.empty());
}
