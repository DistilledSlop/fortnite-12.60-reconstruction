143aea924  MOVSXD RSI,dword ptr [RDI + 0x8]
143aea928  LEA EAX,[RSI + 0x1]
143aea92b  MOV dword ptr [RDI + 0x8],EAX
143aea92e  CMP EAX,dword ptr [RDI + 0xc]
143aea931  JLE 0x143aea93d
143aea933  MOV EDX,ESI
143aea935  MOV RCX,RDI
143aea938  CALL 0x14072b840
143aea93d  LEA RCX,[RSI + RSI*0x2]
143aea941  MOV RSI,qword ptr [RSP + 0xb8]
143aea949  SHL RCX,0x4
143aea94d  ADD RCX,qword ptr [RDI]
143aea950  JZ 0x143aea95e
143aea952  MOV qword ptr [RCX],R12
143aea955  MOV dword ptr [RCX + 0x2c],0xffffffff
143aea95c  JMP 0x143aea961
143aea95e  MOV RCX,R12
143aea961  AND dword ptr [RBP + -0x29],0xf8000000
143aea968  LEA RAX,[R14 + 0x5e8]
143aea96f  AND byte ptr [RBP + 0x24],0xf0
143aea973  XORPS XMM0,XMM0
143aea976  MOVSS XMM1,dword ptr [0x1459a86f8]
143aea97e  MOV qword ptr [RCX + 0x8],RAX
143aea982  LEA RAX,[R14 + 0x490]
