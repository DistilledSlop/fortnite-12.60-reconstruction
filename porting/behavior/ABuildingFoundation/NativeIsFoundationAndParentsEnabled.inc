// CONFIDENCE: HIGH
// Original 0x141be2520. All ancestors must have the enabled state.
return FoundationEnabledState == EDynamicFoundationEnabledState::Enabled &&
    (!ParentFoundation || ParentFoundation->NativeIsFoundationAndParentsEnabled());
