// CONFIDENCE: high; native wrapper reads the constructor-initialized field.
{
	FFortPlayerSurveyButtonListMultipleSelectionAnswerWidgetFocusInfo Result;
	static_assert(sizeof(Result) == sizeof(NativeRecoveredUInt64_0x328), "Native focus info is exactly eight bytes");
	FMemory::Memcpy(&Result, &NativeRecoveredUInt64_0x328, sizeof(NativeRecoveredUInt64_0x328));
	return Result;
}
