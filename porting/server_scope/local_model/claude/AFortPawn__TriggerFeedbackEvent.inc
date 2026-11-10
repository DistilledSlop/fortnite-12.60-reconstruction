// CONFIDENCE: MEDIUM
// Original: exec thunk 0x142a20f00; native body 0x142356fc0; feedback-manager lookup 0x1424ab210 (ported in deps);
// playback 0x1424c21f0 (deps, only its top-level flow was read).
// FFrame read order: EventName, InstigatorPawn, Recipient, OverriddenDelay.
UFortFeedbackManager* Manager = GetFeedbackManagerForGameState(this);
if (Manager)
{
    // Request layout inferred from the native stack block: zero, then EventName, then a constant flag word 0x00050000.
    FFortFeedbackRequest Request{};
    Request.EventName = EventName;
    Request.Flags = 0x00050000;

    // The last two arguments of 0x1424c21f0 are both passed as 1 by this body.
    Manager->PlayFeedbackEvent(Request, InstigatorPawn, Recipient, OverriddenDelay, true, true);
}
