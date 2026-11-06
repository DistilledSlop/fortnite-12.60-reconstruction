// CONFIDENCE: medium
// Original address: 0x140dbabe0
// raw slot at param_2+0x20: x += (x != 0) (decomp artifact, kept)
*(uint64 *)((uint8 *)param_2 + 0x20) = (uint64)(*(int64 *)((uint8 *)param_2 + 0x20) != 0) + *(int64 *)((uint8 *)param_2 + 0x20);
// virtual call on param_1 (vtable slot 0x670)
(*(void (**)(void *))((*(uint8 **)param_1) + 0x670))(param_1);
