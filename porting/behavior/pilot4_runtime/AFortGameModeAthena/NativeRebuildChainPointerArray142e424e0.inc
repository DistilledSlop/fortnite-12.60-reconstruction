// Helper 0x142e424e0 (45 insns): RCX = &array at node+0x30 (caller 0x142e48430 passes LEA RCX,[RSI+0x30] right after
// writing [RSI+0x40]); the nodes are therefore RCX-0x30, then [node+0x40], ... until null.
// Asm: free old [RCX] (0x141384cb0 = jump to FMemory::Free 0x142bee710); count nodes; buffer = Malloc(count*8, 0)
// (0x141384ca0 = XOR EDX,EDX; JMP 0x142bfb0f0 = FMemory::Malloc); fill from the end: element = node+0x30 for each node in
// chain order, so element order is reversed; [RCX] = buffer; dword [RCX+8] = count - 1 (as in asm); Max (+0xc) untouched.
// CONFIDENCE: MEDIUM (node layout is raw; only the link at +0x40 is used)
FMemory::Free(*reinterpret_cast<void**>(ArrayStorage));
uint8* ChainNode = ArrayStorage - 0x30;
int32 ChainCount = 0;
if (ChainNode)
{
    uint8* CountNode = ChainNode;
    do
    {
        CountNode = *reinterpret_cast<uint8**>(CountNode + 0x40);
        ++ChainCount;
    }
    while (CountNode);
}
uint64 BufferBytes = static_cast<uint64>(static_cast<int64>(ChainCount)) * 8;
void** ChainBuffer = static_cast<void**>(FMemory::Malloc(static_cast<SIZE_T>(BufferBytes), 0));
int32 FillIndex = ChainCount;
while (ChainNode)
{
    --FillIndex;
    ChainBuffer[FillIndex] = ChainNode + 0x30;
    ChainNode = *reinterpret_cast<uint8**>(ChainNode + 0x40);
}
*reinterpret_cast<void***>(ArrayStorage) = ChainBuffer;
*reinterpret_cast<int32*>(ArrayStorage + 0x8) = ChainCount - 1;
