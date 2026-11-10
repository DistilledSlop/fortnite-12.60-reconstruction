// CONFIDENCE: HIGH - exec thunk forwards straight to the native helper (no vtable dispatch).
// Original exec 0x142a4f030, helper 0x1423aec80. FFrame read order: SceneComp, bNewHiddenGame, bPropagateToChildren.
return NativeSetComponentHiddenForController(SceneComp, bNewHiddenGame, bPropagateToChildren);
