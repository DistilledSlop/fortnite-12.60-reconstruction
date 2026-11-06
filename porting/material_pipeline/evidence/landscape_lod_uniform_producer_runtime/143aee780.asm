143aee780 PUSH RBP
143aee782 PUSH RSI
143aee783 PUSH R14
143aee785 LEA RBP,[RSP + -0xd0]
143aee78d SUB RSP,0x1d0
143aee794 MOV RAX,qword ptr [0x1480c4288]
143aee79b XOR RAX,RSP
143aee79e MOV qword ptr [RBP + 0xb0],RAX
143aee7a5 MOV R14,RDX
143aee7a8 MOV RSI,RCX
143aee7ab TEST RDX,RDX
143aee7ae JZ 0x143aee7bd
143aee7b0 CMP qword ptr [RCX + 0x248],RDX
143aee7b7 JZ 0x143aeee9c
143aee7bd MOV EAX,dword ptr [0x148218458]
143aee7c3 CMP dword ptr [RCX + 0x10],EAX
143aee7c6 JNZ 0x143aee7d7
143aee7c8 MOV EAX,dword ptr [0x14821845c]
143aee7ce CMP dword ptr [RCX + 0x14],EAX
143aee7d1 JZ 0x143aeee95
143aee7d7 MOV qword ptr [RSP + 0x1f8],RBX
143aee7df MOV qword ptr [RSP + 0x200],RDI
143aee7e7 LEA RDI,[RCX + 0x38]
143aee7eb MOV qword ptr [RSP + 0x1c8],R12
143aee7f3 XOR R12D,R12D
143aee7f6 MOV qword ptr [RSP + 0x1c0],R15
143aee7fe CMP qword ptr [RCX + 0xe8],R12
143aee805 JNZ 0x143aee8e5
143aee80b MOV ECX,dword ptr [0x147c6a944]
143aee811 MOV EAX,0x1
143aee816 MOVUPS XMM0,xmmword ptr [0x148218618]
143aee81d SHL EAX,CL
143aee81f MOV RCX,RDI
143aee822 DEC EAX
143aee824 MOV qword ptr [RBP + -0x50],R12
143aee828 MOV dword ptr [RBP + -0x2c],EAX
143aee82b MOV RAX,qword ptr [RDI]
143aee82e MOV dword ptr [RBP + -0x40],0x1
143aee835 MOVUPS xmmword ptr [RBP + -0x3c],XMM0
143aee839 MOV byte ptr [RBP + -0x28],R12B
143aee83d MOV qword ptr [RBP + -0x20],R12
143aee841 MOV dword ptr [RBP + -0x18],R12D
143aee845 MOV qword ptr [RBP + -0x48],RDI
143aee849 CALL qword ptr [RAX + 0x8]
143aee84c MOV RCX,qword ptr [0x1482ff948]
143aee853 LEA RDX,[RBP + -0x50]
143aee857 MOV qword ptr [RSP + 0x28],RDX
143aee85c LEA R8,[0x148300c18]
143aee863 MOV R9D,EAX
143aee866 MOV dword ptr [RSP + 0x20],0x202
143aee86e LEA RDX,[RSP + 0x30]
143aee873 MOV R10,qword ptr [RCX]
143aee876 CALL qword ptr [R10 + 0x4b8]
143aee87d LEA RDX,[RSP + 0x30]
143aee882 LEA RCX,[RSI + 0xe8]
143aee889 CALL 0x140828830
143aee88e LEA RCX,[RSP + 0x30]
143aee893 CALL 0x140827d40
143aee898 MOV RCX,qword ptr [0x1482ff948]
143aee89f LEA R8,[0x148300c18]
143aee8a6 MOV R9,qword ptr [RSI + 0xe8]
143aee8ad LEA RDX,[RSP + 0x38]
143aee8b2 MOV byte ptr [RSP + 0x28],0xd
143aee8b7 MOV dword ptr [RSP + 0x20],0x4
143aee8bf MOV RAX,qword ptr [RCX]
143aee8c2 CALL qword ptr [RAX + 0x4d8]
143aee8c8 LEA RCX,[RSI + 0xf0]
143aee8cf LEA RDX,[RSP + 0x38]
143aee8d4 CALL 0x140828830
143aee8d9 LEA RCX,[RSP + 0x38]
143aee8de CALL 0x140827d40
143aee8e3 JMP 0x143aee955
143aee8e5 MOV RAX,qword ptr [RDI]
143aee8e8 MOV RCX,RDI
143aee8eb CALL qword ptr [RAX + 0x8]
143aee8ee MOV RCX,qword ptr [0x1482ff948]
143aee8f5 LEA RDX,[0x148300c18]
143aee8fc MOV R8,qword ptr [RSI + 0xe8]
143aee903 XOR R9D,R9D
143aee906 MOV dword ptr [RSP + 0x28],0x1
143aee90e MOV dword ptr [RSP + 0x20],EAX
143aee912 MOV R10,qword ptr [RCX]
143aee915 CALL qword ptr [R10 + 0x160]
143aee91c MOV RDX,qword ptr [RDI]
143aee91f MOV RCX,RDI
143aee922 MOV RBX,RAX
143aee925 CALL qword ptr [RDX + 0x8]
143aee928 MOV RDX,qword ptr [RSI + 0x40]
143aee92c MOV RCX,RBX
143aee92f MOV R8D,EAX
143aee932 CALL 0x1458092ed
143aee937 MOV RCX,qword ptr [0x1482ff948]
143aee93e LEA RDX,[0x148300c18]
143aee945 MOV R8,qword ptr [RSI + 0xe8]
143aee94c MOV RAX,qword ptr [RCX]
143aee94f CALL qword ptr [RAX + 0x168]
143aee955 LEA RDI,[RSI + 0x58]
143aee959 CMP qword ptr [RSI + 0xf8],R12
143aee960 JNZ 0x143aeea40
143aee966 MOV ECX,dword ptr [0x147c6a944]
143aee96c MOV EAX,0x1
143aee971 MOVUPS XMM0,xmmword ptr [0x148218618]
143aee978 SHL EAX,CL
143aee97a MOV RCX,RDI
143aee97d DEC EAX
143aee97f MOV qword ptr [RBP + -0x10],R12
143aee983 MOV dword ptr [RBP + 0x14],EAX
143aee986 MOV RAX,qword ptr [RDI]
143aee989 MOV dword ptr [RBP],0x1
143aee990 MOVUPS xmmword ptr [RBP + 0x4],XMM0
143aee994 MOV byte ptr [RBP + 0x18],R12B
143aee998 MOV qword ptr [RBP + 0x20],R12
143aee99c MOV dword ptr [RBP + 0x28],R12D
143aee9a0 MOV qword ptr [RBP + -0x8],RDI
143aee9a4 CALL qword ptr [RAX + 0x8]
143aee9a7 MOV RCX,qword ptr [0x1482ff948]
143aee9ae LEA RDX,[RBP + -0x10]
143aee9b2 MOV qword ptr [RSP + 0x28],RDX
143aee9b7 LEA R8,[0x148300c18]
143aee9be MOV R9D,EAX
143aee9c1 MOV dword ptr [RSP + 0x20],0x202
143aee9c9 LEA RDX,[RSP + 0x40]
143aee9ce MOV R10,qword ptr [RCX]
143aee9d1 CALL qword ptr [R10 + 0x4b8]
143aee9d8 LEA RDX,[RSP + 0x40]
143aee9dd LEA RCX,[RSI + 0xf8]
143aee9e4 CALL 0x140828830
143aee9e9 LEA RCX,[RSP + 0x40]
143aee9ee CALL 0x140827d40
143aee9f3 MOV RCX,qword ptr [0x1482ff948]
143aee9fa LEA R8,[0x148300c18]
143aeea01 MOV R9,qword ptr [RSI + 0xf8]
143aeea08 LEA RDX,[RSP + 0x48]
143aeea0d MOV byte ptr [RSP + 0x28],0xd
143aeea12 MOV dword ptr [RSP + 0x20],0x4
143aeea1a MOV RAX,qword ptr [RCX]
143aeea1d CALL qword ptr [RAX + 0x4d8]
143aeea23 LEA RCX,[RSI + 0x100]
143aeea2a LEA RDX,[RSP + 0x48]
143aeea2f CALL 0x140828830
143aeea34 LEA RCX,[RSP + 0x48]
143aeea39 CALL 0x140827d40
143aeea3e JMP 0x143aeeab0
143aeea40 MOV RAX,qword ptr [RDI]
143aeea43 MOV RCX,RDI
143aeea46 CALL qword ptr [RAX + 0x8]
143aeea49 MOV RCX,qword ptr [0x1482ff948]
143aeea50 LEA RDX,[0x148300c18]
143aeea57 MOV R8,qword ptr [RSI + 0xf8]
143aeea5e XOR R9D,R9D
143aeea61 MOV dword ptr [RSP + 0x28],0x1
143aeea69 MOV dword ptr [RSP + 0x20],EAX
143aeea6d MOV R10,qword ptr [RCX]
143aeea70 CALL qword ptr [R10 + 0x160]
143aeea77 MOV RDX,qword ptr [RDI]
143aeea7a MOV RCX,RDI
143aeea7d MOV RBX,RAX
143aeea80 CALL qword ptr [RDX + 0x8]
143aeea83 MOV RDX,qword ptr [RSI + 0x60]
143aeea87 MOV RCX,RBX
143aeea8a MOV R8D,EAX
143aeea8d CALL 0x1458092ed
143aeea92 MOV RCX,qword ptr [0x1482ff948]
143aeea99 LEA RDX,[0x148300c18]
143aeeaa0 MOV R8,qword ptr [RSI + 0xf8]
143aeeaa7 MOV RAX,qword ptr [RCX]
143aeeaaa CALL qword ptr [RAX + 0x168]
143aeeab0 CMP qword ptr [RSI + 0x108],R12
143aeeab7 JNZ 0x143aeeb9b
143aeeabd MOV ECX,dword ptr [0x147c6a944]
143aeeac3 MOV EAX,0x1
143aeeac8 MOVUPS XMM0,xmmword ptr [0x148218618]
143aeeacf SHL EAX,CL
143aeead1 LEA RCX,[RSI + 0x78]
143aeead5 DEC EAX
143aeead7 MOV qword ptr [RBP + 0x30],R12
143aeeadb MOV dword ptr [RBP + 0x54],EAX
143aeeade MOV RAX,qword ptr [RCX]
143aeeae1 MOV dword ptr [RBP + 0x40],0x1
143aeeae8 MOVUPS xmmword ptr [RBP + 0x44],XMM0
143aeeaec MOV byte ptr [RBP + 0x58],R12B
143aeeaf0 MOV qword ptr [RBP + 0x60],R12
143aeeaf4 MOV dword ptr [RBP + 0x68],R12D
143aeeaf8 MOV qword ptr [RBP + 0x38],RCX
143aeeafc CALL qword ptr [RAX + 0x8]
143aeeaff MOV RCX,qword ptr [0x1482ff948]
143aeeb06 LEA RDX,[RBP + 0x30]
143aeeb0a MOV qword ptr [RSP + 0x28],RDX
143aeeb0f LEA R8,[0x148300c18]
143aeeb16 MOV R9D,EAX
143aeeb19 MOV dword ptr [RSP + 0x20],0x202
143aeeb21 LEA RDX,[RSP + 0x50]
143aeeb26 MOV R10,qword ptr [RCX]
143aeeb29 CALL qword ptr [R10 + 0x4b8]
143aeeb30 LEA RDX,[RSP + 0x50]
143aeeb35 LEA RCX,[RSI + 0x108]
143aeeb3c CALL 0x140828830
143aeeb41 LEA RCX,[RSP + 0x50]
143aeeb46 CALL 0x140827d40
143aeeb4b MOV RCX,qword ptr [0x1482ff948]
143aeeb52 LEA R8,[0x148300c18]
143aeeb59 MOV R9,qword ptr [RSI + 0x108]
143aeeb60 LEA RDX,[RSP + 0x58]
143aeeb65 MOV byte ptr [RSP + 0x28],0xd
143aeeb6a MOV dword ptr [RSP + 0x20],0x4
143aeeb72 MOV RAX,qword ptr [RCX]
143aeeb75 CALL qword ptr [RAX + 0x4d8]
143aeeb7b LEA RCX,[RSI + 0x110]
143aeeb82 LEA RDX,[RSP + 0x58]
143aeeb87 CALL 0x140828830
143aeeb8c LEA RCX,[RSP + 0x58]
143aeeb91 CALL 0x140827d40
143aeeb96 JMP 0x143aeec1e
143aeeb9b CMP byte ptr [RSI + 0x18],R12B
143aeeb9f JZ 0x143aeec1e
143aeeba1 CMP dword ptr [RSI + 0x4],R12D
143aeeba5 JLE 0x143aeec1e
143aeeba7 MOV RAX,qword ptr [RSI + 0x78]
143aeebab LEA RCX,[RSI + 0x78]
143aeebaf CALL qword ptr [RAX + 0x8]
143aeebb2 MOV RCX,qword ptr [0x1482ff948]
143aeebb9 LEA RDX,[0x148300c18]
143aeebc0 MOV R8,qword ptr [RSI + 0x108]
143aeebc7 XOR R9D,R9D
143aeebca MOV dword ptr [RSP + 0x28],0x1
143aeebd2 MOV dword ptr [RSP + 0x20],EAX
143aeebd6 MOV R10,qword ptr [RCX]
143aeebd9 CALL qword ptr [R10 + 0x160]
143aeebe0 MOV RDX,qword ptr [RSI + 0x78]
143aeebe4 LEA RCX,[RSI + 0x78]
143aeebe8 MOV RBX,RAX
143aeebeb CALL qword ptr [RDX + 0x8]
143aeebee MOV RDX,qword ptr [RSI + 0x80]
143aeebf5 MOV RCX,RBX
143aeebf8 MOV R8D,EAX
143aeebfb CALL 0x1458092ed
143aeec00 MOV RCX,qword ptr [0x1482ff948]
143aeec07 LEA RDX,[0x148300c18]
143aeec0e MOV R8,qword ptr [RSI + 0x108]
143aeec15 MOV RAX,qword ptr [RCX]
143aeec18 CALL qword ptr [RAX + 0x168]
143aeec1e CMP qword ptr [RSI + 0x118],R12
143aeec25 JNZ 0x143aeed1e
143aeec2b MOV ECX,dword ptr [0x147c6a944]
143aeec31 MOV EAX,0x1
143aeec36 MOVUPS XMM0,xmmword ptr [0x148218618]
143aeec3d SHL EAX,CL
143aeec3f LEA RCX,[RSI + 0x98]
143aeec46 DEC EAX
143aeec48 MOV qword ptr [RBP + 0x70],R12
143aeec4c MOV dword ptr [RBP + 0x94],EAX
143aeec52 MOV RAX,qword ptr [RCX]
143aeec55 MOV dword ptr [RBP + 0x80],0x1
143aeec5f MOVUPS xmmword ptr [RBP + 0x84],XMM0
143aeec66 MOV byte ptr [RBP + 0x98],R12B
143aeec6d MOV qword ptr [RBP + 0xa0],R12
143aeec74 MOV dword ptr [RBP + 0xa8],R12D
143aeec7b MOV qword ptr [RBP + 0x78],RCX
143aeec7f CALL qword ptr [RAX + 0x8]
143aeec82 MOV RCX,qword ptr [0x1482ff948]
143aeec89 LEA RDX,[RBP + 0x70]
143aeec8d MOV qword ptr [RSP + 0x28],RDX
143aeec92 LEA R8,[0x148300c18]
143aeec99 MOV R9D,EAX
143aeec9c MOV dword ptr [RSP + 0x20],0x202
143aeeca4 LEA RDX,[RSP + 0x60]
143aeeca9 MOV R10,qword ptr [RCX]
143aeecac CALL qword ptr [R10 + 0x4b8]
143aeecb3 LEA RDX,[RSP + 0x60]
143aeecb8 LEA RCX,[RSI + 0x118]
143aeecbf CALL 0x140828830
143aeecc4 LEA RCX,[RSP + 0x60]
143aeecc9 CALL 0x140827d40
143aeecce MOV RCX,qword ptr [0x1482ff948]
143aeecd5 LEA R8,[0x148300c18]
143aeecdc MOV R9,qword ptr [RSI + 0x118]
143aeece3 LEA RDX,[RSP + 0x68]
143aeece8 MOV byte ptr [RSP + 0x28],0xd
143aeeced MOV dword ptr [RSP + 0x20],0x4
143aeecf5 MOV RAX,qword ptr [RCX]
143aeecf8 CALL qword ptr [RAX + 0x4d8]
143aeecfe LEA RCX,[RSI + 0x120]
143aeed05 LEA RDX,[RSP + 0x68]
143aeed0a CALL 0x140828830
143aeed0f LEA RCX,[RSP + 0x68]
143aeed14 CALL 0x140827d40
143aeed19 JMP 0x143aeedb5
143aeed1e CMP byte ptr [RSI + 0x18],R12B
143aeed22 JZ 0x143aeedb5
143aeed28 CMP dword ptr [RSI + 0x4],R12D
143aeed2c JLE 0x143aeedb5
143aeed32 MOV RAX,qword ptr [RSI + 0x98]
143aeed39 LEA RCX,[RSI + 0x98]
143aeed40 CALL qword ptr [RAX + 0x8]
143aeed43 MOV RCX,qword ptr [0x1482ff948]
143aeed4a LEA RDX,[0x148300c18]
143aeed51 MOV R8,qword ptr [RSI + 0x118]
143aeed58 XOR R9D,R9D
143aeed5b MOV dword ptr [RSP + 0x28],0x1
143aeed63 MOV dword ptr [RSP + 0x20],EAX
143aeed67 MOV R10,qword ptr [RCX]
143aeed6a CALL qword ptr [R10 + 0x160]
143aeed71 MOV RDX,qword ptr [RSI + 0x98]
143aeed78 LEA RCX,[RSI + 0x98]
143aeed7f MOV RBX,RAX
143aeed82 CALL qword ptr [RDX + 0x8]
143aeed85 MOV RDX,qword ptr [RSI + 0xa0]
143aeed8c MOV RCX,RBX
143aeed8f MOV R8D,EAX
143aeed92 CALL 0x1458092ed
143aeed97 MOV RCX,qword ptr [0x1482ff948]
143aeed9e LEA RDX,[0x148300c18]
143aeeda5 MOV R8,qword ptr [RSI + 0x118]
143aeedac MOV RAX,qword ptr [RCX]
143aeedaf CALL qword ptr [RAX + 0x168]
143aeedb5 MOV EAX,dword ptr [RSI + 0x8]
143aeedb8 LEA R8,[RBP + -0x80]
143aeedbc MOV RCX,qword ptr [0x1482ff948]
143aeedc3 MOV RDX,qword ptr [RSI + 0x128]
143aeedca MOV R15,qword ptr [RSP + 0x1c0]
143aeedd2 MOV R12,qword ptr [RSP + 0x1c8]
143aeedda MOV RDI,qword ptr [RSP + 0x200]
143aeede2 MOV dword ptr [RBP + -0x80],EAX
143aeede5 MOV EAX,dword ptr [RSI + 0xc]
143aeede8 MOV dword ptr [RBP + -0x7c],EAX
143aeedeb MOV EAX,dword ptr [RSI + 0x10]
143aeedee MOV dword ptr [RBP + -0x78],EAX
143aeedf1 MOV EAX,dword ptr [RSI + 0x14]
143aeedf4 MOV dword ptr [RBP + -0x74],EAX
143aeedf7 MOV RAX,qword ptr [RSI + 0xf0]
143aeedfe MOV qword ptr [RBP + -0x70],RAX
143aeee02 MOV RAX,qword ptr [RSI + 0x100]
143aeee09 MOV qword ptr [RBP + -0x68],RAX
143aeee0d MOV RAX,qword ptr [RSI + 0x110]
143aeee14 MOV qword ptr [RBP + -0x60],RAX
143aeee18 MOV RAX,qword ptr [RSI + 0x120]
143aeee1f MOV qword ptr [RBP + -0x58],RAX
143aeee23 MOV RAX,qword ptr [RCX]
143aeee26 TEST RDX,RDX
143aeee29 JZ 0x143aeee33
143aeee2b CALL qword ptr [RAX + 0x130]
143aeee31 JMP 0x143aeee8d
143aeee33 MOV dword ptr [RSP + 0x28],0x1
143aeee3b LEA R9,[0x1483117f0]
143aeee42 LEA RDX,[RSP + 0x70]
143aeee47 MOV dword ptr [RSP + 0x20],0x1
143aeee4f CALL qword ptr [RAX + 0x128]
143aeee55 MOV RAX,qword ptr [RSP + 0x70]
143aeee5a MOV qword ptr [RSP + 0x78],RAX
143aeee5f TEST RAX,RAX
143aeee62 JZ 0x143aeee68
143aeee64 INC.LOCK dword ptr [RAX + 0x8]
143aeee68 LEA RCX,[RSP + 0x70]
143aeee6d CALL 0x140827d40
143aeee72 LEA RDX,[RSP + 0x78]
143aeee77 LEA RCX,[RSI + 0x128]
143aeee7e CALL 0x140828830
143aeee83 LEA RCX,[RSP + 0x78]
143aeee88 CALL 0x140827d40
143aeee8d MOV RBX,qword ptr [RSP + 0x1f8]
143aeee95 MOV qword ptr [RSI + 0x248],R14
143aeee9c MOV RCX,qword ptr [RBP + 0xb0]
143aeeea3 XOR RCX,RSP
143aeeea6 CALL 0x145803c70
143aeeeab ADD RSP,0x1d0
143aeeeb2 POP R14
143aeeeb4 POP RSI
143aeeeb5 POP RBP
143aeeeb6 RET