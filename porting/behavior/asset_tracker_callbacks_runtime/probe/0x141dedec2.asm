141dedec2  CALL 0x141df61e0
141dedec7  MOV RDI,RAX
141dedeca  LEA RCX,[RAX + 0x1a0]
141deded1  CALL qword ptr [0x1458fc9f8]
141deded7  CMP byte ptr [RDI + 0x1c8],SIL
141dedede  JNZ 0x141dedf0d
141dedee0  CMP byte ptr [0x1481348e0],0x2
141dedee7  JC 0x141dedf14
141dedee9  LEA RAX,[0x145fe0750]
141dedef0  MOV R9D,0x2
141dedef6  LEA R8,[0x1481348e4]
141dedefd  MOV qword ptr [RSP + 0x20],RAX
141dedf02  XOR EDX,EDX
141dedf04  XOR ECX,ECX
141dedf06  CALL 0x142c54b70
141dedf0b  JMP 0x141dedf14
141dedf0d  MOV byte ptr [RDI + 0x1c8],SIL
141dedf14  LEA RCX,[RDI + 0x1a0]
141dedf1b  MOV RBX,qword ptr [RSP + 0x40]
141dedf20  MOV RSI,qword ptr [RSP + 0x48]
141dedf25  ADD RSP,0x30
141dedf29  POP RDI
141dedf2a  JMP qword ptr [0x1458fca00]
