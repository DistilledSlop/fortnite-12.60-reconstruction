// Original item owner/lifetime behavior; see HANDOFF.md.
// CONFIDENCE: HIGH
const UObject* Outer = GetOuter();
return Outer ? Outer->GetWorld() : nullptr;
