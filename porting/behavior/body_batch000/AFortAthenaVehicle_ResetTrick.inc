// CONFIDENCE: medium
// Original address: 0x14283ccf0
// raw slot at param_2+0x20: x += (x != 0) (decomp artifact, kept)
*(uint64 *)((uint8 *)param_2 + 0x20) = (uint64)(*(int64 *)((uint8 *)param_2 + 0x20) != 0) + *(int64 *)((uint8 *)param_2 + 0x20);
// component at param_1+0x3b0, virtual call slot 0x3e8
uint8 *component = *(uint8 **)((uint8 *)param_1 + 0x3b0);
(*(void (**)(void *))((*(uint8 **)component) + 0x3e8))(component);
