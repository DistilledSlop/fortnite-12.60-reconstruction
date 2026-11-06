// CONFIDENCE: HIGH
// Original 0x141be2550. Preserve the recursive parent query and its short circuit.
return DynamicFoundationType != EDynamicFoundationType::Static ||
    (ParentFoundation && ParentFoundation->NativeIsDynamicFoundationOrParent());
