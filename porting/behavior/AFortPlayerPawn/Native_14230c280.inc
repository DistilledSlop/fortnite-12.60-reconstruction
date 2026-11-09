// CONFIDENCE: medium
// Original address: 0x14230c280
// Raw field offsets: 0xF0 state byte, 0x280 pointer, 0x13EC cached byte
if (*(uint8*)((uint8*)self + 0xF0) == 1) {
    return *(uint8*)((uint8*)self + 0x13EC);
}
void* obj = *(void**)((uint8*)self + 0x280);
if (obj != nullptr) {
    uint8 cVar1 = (*(uint8 (**)(void*))(*(uint8**)obj + 0x548))(obj);
    if (cVar1 != 0) {
        cVar1 = (*(uint8 (**)(void*))(*(uint8**)self + 0x7B0))(self);
        if (cVar1 == 0) {
            *(uint8*)((uint8*)self + 0x13EC) = 1;
            return 1;
        }
    }
}
*(uint8*)((uint8*)self + 0x13EC) = 0;
return 0;
