// CONFIDENCE: HIGH - exec thunk forwards to the native helper (no vtable dispatch).
// Original exec 0x142a4f140, helper 0x1423af010. FFrame read order: SceneComp, bNewHiddenGame, bPropagateToChildren.
return NativeSetComponentHiddenForOtherLocalControllers(SceneComp, bNewHiddenGame, bPropagateToChildren);
