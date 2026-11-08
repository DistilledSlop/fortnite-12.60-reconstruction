// Behaviour snippet: UFortLocalUserEntry::HandleGetMenuContent (body only; spliced into the generated member function).
// Original address: 0x1407c3e20 (thunk 0x140814f70)
// Evidence: leaf; returns the pointer at +0xb78 (LocalUserSettingsMenu, reflected as UFortLocalUserSettingsMenu*) unchanged.
// CONFIDENCE: HIGH
static_assert(sizeof(UFortLocalUserSettingsMenu) > 0, "Generated header include for UFortLocalUserSettingsMenu.");
return LocalUserSettingsMenu;
