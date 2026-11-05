143adc8a3  MOV EBX,EDX
143adc8a5  MOV RSI,RCX
143adc8a8  LEA EAX,[RDI + RDX*0x1]
143adc8ab  MOV dword ptr [RCX + 0x8],EAX
143adc8ae  CMP EAX,dword ptr [RCX + 0xc]
143adc8b1  JLE 0x143adc8ba
143adc8b3  MOV EDX,EDI
143adc8b5  CALL 0x143af0c20
143adc8ba  LEA RCX,[RDI + RDI*0x2]
143adc8be  SHL RCX,0x4
143adc8c2  ADD RCX,qword ptr [RSI]
143adc8c5  TEST EBX,EBX
143adc8c7  JZ 0x143adc8e3
143adc8c9  XOR EAX,EAX
143adc8cb  NOP dword ptr [RAX + RAX*0x1]
143adc8d0  MOV qword ptr [RCX],RAX
143adc8d3  MOV dword ptr [RCX + 0x2c],0xffffffff
143adc8da  LEA RCX,[RCX + 0x30]
143adc8de  SUB EBX,0x1
143adc8e1  JNZ 0x143adc8d0
143adc8e3  MOV RBX,qword ptr [RSP + 0x30]
143adc8e8  MOV EAX,EDI
143adc8ea  MOV RSI,qword ptr [RSP + 0x38]
143adc8ef  ADD RSP,0x20
