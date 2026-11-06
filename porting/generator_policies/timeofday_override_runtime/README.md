# Time-of-day override caller

Pending refresh/spawn closure; not canonical yet. Original 1419642e0 (98 instructions)
uses the existing IsCreativeOrPlaygroundModePlaylist guard and LogFort Warning.
Compares checked old/new subclasses and then stores the original raw class pointer;
TSubclassOf assignment from UClass* retains it. GetWorld is dereferenced without an
invented null guard. World::GetWorldSettings(false,true) is original 144690740;
its persistent-level/streaming logic matches Engine/Private/World.cpp:4681 and
callee143dd18e0 matches ULevel::GetWorldSettings. Refresh occurs after the store.
AFortTimeOfDayManager and AFortWorldSettings class getters identified in the adjacent
investigation. Reflection exec1428f08f0 supplies class argument; helper name assigned.
Do not promote until actual NativeRefreshTimeOfDayManager and its spawn path close.
