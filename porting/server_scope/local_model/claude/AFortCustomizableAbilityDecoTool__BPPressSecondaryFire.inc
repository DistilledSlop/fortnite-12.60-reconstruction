// CONFIDENCE: LOW. Original addresses: exec thunk 0x1428cdee0, virtual slot 0xb60 -> 0x14265fec0.
// The thunk reads FortDecoHelper and forwards to the secondary-fire implementation (dep
// AFortCustomizableAbilityDecoTool__SecondaryFireImpl). Its own body is only the forward.
SecondaryFireImpl();
