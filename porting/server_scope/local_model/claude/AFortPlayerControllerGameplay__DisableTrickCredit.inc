// CONFIDENCE: HIGH for control flow, MEDIUM for names. Original exec 0x142a64ec0 (RPC thunk, one float parameter Duration). No validate, no implementation dispatch: the thunk calls FUN_1423cf4e0 directly.
// Body FUN_1423cf4e0, inlined below. Field_0x22fd is an unresolved bool. TrickCreditEndTime is the float at +0x460 (proposed name). World+0x540 is read as a float time value (proposed name TimeSeconds).
Field_0x22fd = false;
if (Duration > 0.0f)
{
	TrickCreditEndTime = Duration + GetWorld()->Field_0x540; // controller virtual slot 0x150 is GetWorld
}
