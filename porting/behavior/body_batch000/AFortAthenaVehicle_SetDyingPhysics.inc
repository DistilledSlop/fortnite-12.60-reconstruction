// CONFIDENCE: medium
// Original address: 0x14283d290
// raw slot at param_2+0x20: x += (x != 0) (decomp artifact, kept)
*(uint64 *)((uint8 *)param_2 + 0x20) = (uint64)(*(int64 *)((uint8 *)param_2 + 0x20) != 0) + *(int64 *)((uint8 *)param_2 + 0x20);
// virtual call on param_1 (vtable slot 0xca0)
(*(void (**)(void *))((*(uint8 **)param_1) + 0xca0))(param_1);
