// Original body address: 0x1406801c0 (shared bare RET, no effect)
// Offset evidence: body is RET 0x0 only; no receiver or memory access; OutVariables is never written.
// CONFIDENCE: HIGH
(void)OutVariables;
return;
