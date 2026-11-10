// CONFIDENCE: HIGH that the native body is empty - STRIPPED.
// Original exec 0x142a4fc30 reads TossOverrideData and DropItemDef off the FFrame, writes three dead float locals
// (300, 45, 600) and then calls the shared empty void stub 0x1406801c0 (RET only). No other side effect.
// Empty body on purpose: nothing is done with the parameters.
