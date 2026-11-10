// CONFIDENCE: HIGH
// Original addresses: client RPC thunk 0x142737dd0 (no parameters; virtual slot 0x738 on this) -> _Implementation 0x141e94760 (owner AFortGameState).
UObject* EventCalendarOwner = GetEventCalendarOwner(this);   // 0x141e5b2c0: GetWorldFromContextObject(this)->+0x188, if it is the expected class (0x1428ea7f0)
if (EventCalendarOwner != nullptr)
{
	UObject* EventCalendar = EventCalendarOwner->EventCalendar_7a0;   // 0x141bd9690: +0x7a0
	if (EventCalendar != nullptr)
	{
		RefreshEventCalendar(EventCalendar);                      // 0x14224a780 (35 lines, see evidence)
	}
}
