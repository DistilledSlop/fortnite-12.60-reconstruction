// Original address: 0x142a056f0; native body 0x14213d9d0 (exec thunk tail-jumps there after Stack.Code bump).
// Decompile source: porting/generated/decomp/AFortObjectiveBase/OnRep_ObjectiveVisibilityOverride__142a056f0.c
// Inferred: the target checks the invocation count of OnObjectiveVisibilityOverrideChanged (delegate at +0x2d8; InvocationList count at +0x2e0), which is TMulticastScriptDelegate::IsBound(). When bound it broadcasts (this, VisibilityOverride) with the enum byte at +0x2a2.
// CONFIDENCE: HIGH
if (OnObjectiveVisibilityOverrideChanged.IsBound())
{
    OnObjectiveVisibilityOverrideChanged.Broadcast(this, VisibilityOverride);
}
