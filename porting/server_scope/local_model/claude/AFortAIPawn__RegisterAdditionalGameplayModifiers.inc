// CONFIDENCE: HIGH. Body is empty: the native function reads the ModifierDefinitions array from the script frame and releases the temporary buffer without any effect (it calls Stub_Empty).
// Original address: 0x14273fcd0 (exec). Empty native body: Stub_Empty (no-op) plus FMemory_Free of the temporary array.
(void)ModifierDefinitions;
