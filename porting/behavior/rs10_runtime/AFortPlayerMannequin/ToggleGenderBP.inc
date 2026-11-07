// Behaviour snippet: AFortPlayerMannequin::ToggleGenderBP (body only; spliced into the generated member function).
// Original body address: 0x141dde960 (thunk 0x142a80eb0, group B, flags 0x4020401).
// Offset evidence: the only work in the body is a verbosity-gated diagnostic block (guard byte at 0x1481349f0 below 2 skips it). That block builds a display string via call 0x142ce9f90, logs it via call 0x142c54b70 and frees the temporary string. The string is used only as a log argument.
// Verbosity-gated diagnostic log at 0x142c54b70 elided (no state change)
// CONFIDENCE: MEDIUM
return;
