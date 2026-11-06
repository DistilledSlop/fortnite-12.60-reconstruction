// CONFIDENCE: high. Native target 0x14238ea50.
const UFortControllerComponent_Aircraft* Component = static_cast<UFortControllerComponent_Aircraft*>(
    FindComponentByClass(UFortControllerComponent_Aircraft::StaticClass()));
return Component ? Component->NativeRecoveredBool_0xf8 : false;
