// Evidence lift, NOT compilable source: exact delegate argument types and native member names
// are unresolved. See HANDOFF.md before using this as a source implementation.
// Each operation uses Unreal's weak UObject binding semantics, preserving virtual dispatch.
// ReplaceSinglecast replaces the previous binding; AddMulticast appends a binding.
// The set must be reloaded for each call: the native function does so.
void ABuildingActor::BindAttributeDelegates_EvidenceLift()
{
    ReplaceSinglecast(BuildingAttributeSet->Delegate_2D8, this, &ABuildingActor::Virtual_850);
    AddMulticast(BuildingAttributeSet->Delegate_2E8, this, &ABuildingActor::Virtual_838);
    AddMulticast(BuildingAttributeSet->Delegate_300, this, &ABuildingActor::Virtual_838);
    AddMulticast(BuildingAttributeSet->Delegate_318, this, &ABuildingActor::Virtual_840);
    ReplaceSinglecast(BuildingAttributeSet->Delegate_348, this, &ABuildingActor::Virtual_858);
    AddMulticast(BuildingAttributeSet->Delegate_358, this, &ABuildingActor::Virtual_848);
    AddMulticast(BuildingAttributeSet->Delegate_388, this, &ABuildingActor::Virtual_9E0);
}

// Evidence-only ABI inventory, Windows x64, target method RCX = actor:
// Virtual_850 returns float; Virtual_858 returns a boolean-like value. XMM1 is a float,
// R8 points to a tag-container-shaped object, R9 and stack arguments are unresolved.
// Virtual_838 and Virtual_848 receive XMM1 float, R8 pointer, R9 and at least
// three stack arguments; exact parameter count/types and return must be recovered.
// Virtual_840 tail-dispatches +9E0 on authority and ignores the other arguments.
// Virtual_9E0 is a void no-argument callback.
// The executor preserves argument registers and stack, but signatures differ:
// the four multicast broadcasts have six explicit parameters, and the two
// singlecast call sites have five. Do not discard trailing parameters.

// Incomplete declaration sketch; placeholder types deliberately have no definition.
// Ref/pointer constness and object class identities must be established from the
// call sites before this can become valid source or a native-field proposal.
struct UnresolvedSourceTagsReference;
struct UnresolvedEffectSpecReference;
struct UnresolvedEffectContextReference;
struct UnresolvedActorArgument;
struct UnresolvedSinglecastThirdArgument;
using DamageAdjustmentCallback = float (*)(ABuildingActor*, float,
    UnresolvedSourceTagsReference, UnresolvedEffectContextReference,
    UnresolvedActorArgument, UnresolvedActorArgument);
using FatalDamageDecisionCallback = bool (*)(ABuildingActor*, float,
    UnresolvedSourceTagsReference, UnresolvedSinglecastThirdArgument,
    UnresolvedActorArgument, UnresolvedActorArgument);
using AttributeEventCallback = void (*)(ABuildingActor*, float,
    UnresolvedSourceTagsReference, UnresolvedEffectSpecReference,
    UnresolvedEffectContextReference, UnresolvedActorArgument, UnresolvedActorArgument);
using AttributeZeroArgumentCallback = void (*)(ABuildingActor*);
