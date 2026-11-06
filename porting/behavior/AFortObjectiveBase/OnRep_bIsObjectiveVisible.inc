// Original address: 0x142a05710; native body 0x14213da10 (exec thunk tail-jumps there after Stack.Code bump).
// Decompile source: porting/generated/decomp/AFortObjectiveBase/OnRep_bIsObjectiveVisible__142a05710.c
// Inferred: the target checks the invocation count of OnObjectiveVisibilityChanged (delegate at +0x2c8; its InvocationList count at +0x2d0), which is TMulticastScriptDelegate::IsBound() (ScriptDelegates.h: InvocationList.Num() > 0). When bound it broadcasts (this, bIsObjectiveVisible).
// CONFIDENCE: HIGH
if (OnObjectiveVisibilityChanged.IsBound())
{
    OnObjectiveVisibilityChanged.Broadcast(this, bIsObjectiveVisible);
}
