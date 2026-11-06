143acfa60 MOV qword ptr [RSP + 0x10],RBX
143acfa65 MOV qword ptr [RSP + 0x18],RDI
143acfa6a PUSH RBP
143acfa6b LEA RBP,[RSP + -0x57]
143acfa70 SUB RSP,0xf0
143acfa77 MOV RAX,qword ptr [0x1480c4288]
143acfa7e XOR RAX,RSP
143acfa81 MOV qword ptr [RBP + 0x47],RAX
143acfa85 MOV RDI,RCX
143acfa88 MOV qword ptr [RBP + -0x39],0x1
143acfa90 XOR EBX,EBX
143acfa92 LEA RAX,[0x145a611c0]
143acfa99 LEA RCX,[RBP + -0x41]
143acfa9d MOV qword ptr [RBP + -0x41],RBX
143acfaa1 XOR EDX,EDX
143acfaa3 MOV qword ptr [RBP + -0x49],RAX
143acfaa7 MOV byte ptr [RBP + -0x31],BL
143acfaaa CALL 0x140690b30
143acfaaf MOV RAX,qword ptr [RBP + -0x41]
143acfab3 LEA RDX,[RBP + 0x7]
143acfab7 MOV qword ptr [RSP + 0x28],RDX
143acfabc LEA R8,[0x148300c18]
143acfac3 LEA RDX,[RBP + -0x69]
143acfac7 MOV dword ptr [RSP + 0x20],0x201
143acfacf MOV dword ptr [RAX],EBX
143acfad1 MOV EAX,0x1
143acfad6 MOV ECX,dword ptr [0x147c6a944]
143acfadc MOVUPS XMM0,xmmword ptr [0x148218618]
143acfae3 MOV R9D,dword ptr [RBP + -0x39]
143acfae7 SHL EAX,CL
143acfae9 MOV RCX,qword ptr [0x1482ff948]
143acfaf0 DEC EAX
143acfaf2 MOV dword ptr [RBP + 0x2b],EAX
143acfaf5 LEA RAX,[RBP + -0x49]
143acfaf9 MOV qword ptr [RBP + 0xf],RAX
143acfafd LEA R9D,[R9*0x4]
143acfb05 MOV qword ptr [RBP + 0x7],RBX
143acfb09 MOV dword ptr [RBP + 0x17],0x1
143acfb10 MOVUPS xmmword ptr [RBP + 0x1b],XMM0
143acfb14 MOV byte ptr [RBP + 0x2f],BL
143acfb17 MOV qword ptr [RBP + 0x37],RBX
143acfb1b MOV dword ptr [RBP + 0x3f],EBX
143acfb1e MOV RAX,qword ptr [RCX]
143acfb21 CALL qword ptr [RAX + 0x4b8]
143acfb27 LEA RDX,[RBP + -0x69]
143acfb2b LEA RCX,[RDI + 0x28]
143acfb2f CALL 0x140828830
143acfb34 LEA RCX,[RBP + -0x69]
143acfb38 CALL 0x140827d40
143acfb3d MOV RCX,qword ptr [0x1482ff948]
143acfb44 LEA R8,[0x148300c18]
143acfb4b MOV R9,qword ptr [RDI + 0x28]
143acfb4f LEA RDX,[RBP + -0x61]
143acfb53 MOV byte ptr [RSP + 0x28],0xd
143acfb58 MOV dword ptr [RSP + 0x20],0x4
143acfb60 MOV RAX,qword ptr [RCX]
143acfb63 CALL qword ptr [RAX + 0x4d8]
143acfb69 LEA RDX,[RBP + -0x61]
143acfb6d LEA RCX,[RDI + 0x30]
143acfb71 CALL 0x140828830
143acfb76 LEA RCX,[RBP + -0x61]
143acfb7a CALL 0x140827d40
143acfb7f MOV RAX,qword ptr [RDI + 0x30]
143acfb83 LEA R9,[0x1483117f0]
143acfb8a MOV RCX,qword ptr [0x1482ff948]
143acfb91 LEA R8,[RBP + -0x29]
143acfb95 MOV dword ptr [RBP + -0x21],0x1
143acfb9c LEA RDX,[RBP + -0x59]
143acfba0 MOV dword ptr [RBP + -0x1d],0x1
143acfba7 MOV qword ptr [RBP + -0x19],RAX
143acfbab MOV qword ptr [RBP + -0x11],RAX
143acfbaf MOV qword ptr [RBP + -0x9],RAX
143acfbb3 MOV qword ptr [RBP + -0x1],RAX
143acfbb7 MOV RAX,qword ptr [RCX]
143acfbba MOV dword ptr [RSP + 0x28],0x1
143acfbc2 MOV dword ptr [RSP + 0x20],0x2
143acfbca CALL qword ptr [RAX + 0x128]
143acfbd0 MOV RAX,qword ptr [RBP + -0x59]
143acfbd4 MOV qword ptr [RBP + -0x51],RAX
143acfbd8 TEST RAX,RAX
143acfbdb JZ 0x143acfbe1
143acfbdd INC.LOCK dword ptr [RAX + 0x8]
143acfbe1 LEA RCX,[RBP + -0x59]
143acfbe5 CALL 0x140827d40
143acfbea LEA RCX,[RDI + 0x38]
143acfbee LEA RDX,[RBP + -0x51]
143acfbf2 CALL 0x140828830
143acfbf7 LEA RCX,[RBP + -0x51]
143acfbfb CALL 0x140827d40
143acfc00 MOV RCX,qword ptr [RBP + -0x41]
143acfc04 TEST RCX,RCX
143acfc07 JZ 0x143acfc0e
143acfc09 CALL 0x142bee710
143acfc0e MOV RCX,qword ptr [RBP + 0x47]
143acfc12 XOR RCX,RSP
143acfc15 CALL 0x145803c70
143acfc1a LEA R11,[RSP + 0xf0]
143acfc22 MOV RBX,qword ptr [R11 + 0x18]
143acfc26 MOV RDI,qword ptr [R11 + 0x20]
143acfc2a MOV RSP,R11
143acfc2d POP RBP
143acfc2e RET