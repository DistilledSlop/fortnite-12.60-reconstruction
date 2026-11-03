141deca00  MOV RAX,RSP
141deca03  PUSH RBP
141deca04  PUSH R12
141deca06  PUSH R15
141deca08  LEA RBP,[RAX + -0x5f]
141deca0c  SUB RSP,0xe0
141deca13  MOVZX R12D,DL
141deca17  MOV R15,RCX
141deca1a  MOV byte ptr [RBP + -0x59],R12B
141deca1e  MOV qword ptr [RBP + -0x49],RCX
141deca22  CMP byte ptr [RCX + 0x6cc],R12B
141deca29  JZ 0x141ded2d0
141deca2f  CMP byte ptr [0x1481348e0],0x7
141deca36  MOV qword ptr [RAX + 0x18],RBX
141deca3a  MOV qword ptr [RAX + -0x20],RSI
141deca3e  MOV qword ptr [RAX + -0x28],RDI
141deca42  MOV qword ptr [RAX + -0x30],R13
141deca46  MOV qword ptr [RAX + -0x38],R14
141deca4a  MOVAPS xmmword ptr [RAX + -0x48],XMM6
141deca4e  JC 0x141deca77
141deca50  LEA RAX,[0x145fddf80]
141deca57  MOV dword ptr [RSP + 0x28],R12D
141deca5c  MOV R9D,0x7
141deca62  MOV qword ptr [RSP + 0x20],RAX
141deca67  LEA R8,[0x1481348e4]
141deca6e  XOR EDX,EDX
141deca70  XOR ECX,ECX
141deca72  CALL 0x142c54b70
141deca77  CMP byte ptr [0x148126e88],0x5
141deca7e  LEA R13,[0x1459a4b30]
141deca85  JC 0x141decb3c
141deca8b  LEA RAX,[0x145fde008]
141deca92  MOV R9D,0x5
141deca98  LEA R8,[0x148126e8c]
141deca9f  MOV qword ptr [RSP + 0x20],RAX
141decaa4  XOR EDX,EDX
141decaa6  XOR ECX,ECX
141decaa8  CALL 0x142c54b70
141decaad  CMP byte ptr [0x148126e88],0x5
141decab4  JC 0x141decb3c
141decaba  MOVZX EDX,R12B
141decabe  LEA RCX,[RBP + 0x7]
141decac2  CALL 0x141f90530
141decac7  CMP dword ptr [RAX + 0x8],0x0
141decacb  JZ 0x141decad2
141decacd  MOV RBX,qword ptr [RAX]
141decad0  JMP 0x141decad5
141decad2  MOV RBX,R13
141decad5  MOVZX EDX,byte ptr [R15 + 0x6cc]
141decadd  LEA RCX,[RBP + -0x19]
141decae1  CALL 0x141f90530
141decae6  CMP dword ptr [RAX + 0x8],0x0
141decaea  JZ 0x141decaf1
141decaec  MOV RCX,qword ptr [RAX]
141decaef  JMP 0x141decaf4
141decaf1  MOV RCX,R13
141decaf4  MOV qword ptr [RSP + 0x30],RBX
141decaf9  LEA RAX,[0x145fde040]
141decb00  MOV qword ptr [RSP + 0x28],RCX
141decb05  LEA R8,[0x148126e8c]
141decb0c  XOR ECX,ECX
141decb0e  MOV qword ptr [RSP + 0x20],RAX
141decb13  MOV R9D,0x5
141decb19  XOR EDX,EDX
141decb1b  CALL 0x142c54b70
141decb20  MOV RCX,qword ptr [RBP + -0x19]
141decb24  TEST RCX,RCX
141decb27  JZ 0x141decb2e
141decb29  CALL 0x142bee710
141decb2e  MOV RCX,qword ptr [RBP + 0x7]
141decb32  TEST RCX,RCX
141decb35  JZ 0x141decb3c
141decb37  CALL 0x142bee710
141decb3c  LEA RSI,[R15 + 0x740]
141decb43  XOR R14B,R14B
141decb46  INC dword ptr [RSI + 0x14]
141decb49  MOV EAX,dword ptr [RSI + 0x8]
141decb4c  MOV ECX,dword ptr [RSI + 0x14]
141decb4f  SUB EAX,0x1
141decb52  MOVSXD RDI,EAX
141decb55  JS 0x141decb91
141decb57  MOV RBX,RDI
141decb5a  SHL RBX,0x4
141decb5e  NOP
141decb60  MOV RCX,qword ptr [RSI]
141decb63  CMP dword ptr [RBX + RCX*0x1 + 0x8],0x0
141decb68  JZ 0x141decb81
141decb6a  MOV RCX,qword ptr [RBX + RCX*0x1]
141decb6e  TEST RCX,RCX
141decb71  JZ 0x141decb81
141decb73  MOV RAX,qword ptr [RCX]
141decb76  MOVZX EDX,R12B
141decb7a  CALL qword ptr [RAX + 0x50]
141decb7d  TEST AL,AL
141decb7f  JNZ 0x141decb84
141decb81  MOV R14B,0x1
141decb84  SUB RBX,0x10
141decb88  SUB RDI,0x1
141decb8c  JNS 0x141decb60
141decb8e  MOV ECX,dword ptr [RSI + 0x14]
141decb91  LEA EAX,[RCX + -0x1]
141decb94  MOV dword ptr [RSI + 0x14],EAX
141decb97  TEST R14B,R14B
141decb9a  JZ 0x141decba6
141decb9c  XOR EDX,EDX
141decb9e  MOV RCX,RSI
141decba1  CALL 0x140685370
141decba6  CALL 0x141ed2350
141decbab  MOVZX EDX,byte ptr [R15 + 0x6cc]
141decbb3  MOV RCX,RAX
141decbb6  MOVZX R8D,R12B
141decbba  CALL 0x141ecaec0
141decbbf  MOV byte ptr [R15 + 0x6cc],R12B
141decbc6  MOV RCX,qword ptr [0x148351eb8]
141decbcd  CALL 0x14463f970
141decbd2  MOV RCX,RAX
141decbd5  CALL 0x141eb5c00
141decbda  LEA RCX,[RBP + -0x39]
141decbde  CALL qword ptr [0x1458fca10]
141decbe4  XORPS XMM6,XMM6
141decbe7  LEA RDX,[RBP + -0x19]
141decbeb  CVTSI2SD XMM6,qword ptr [RBP + -0x39]
141decbf1  MOV RCX,R15
141decbf4  MULSD XMM6,qword ptr [0x1481d47e8]
141decbfc  CALL 0x141e0b3e0
141decc01  LEA RDI,[R15 + 0x720]
141decc08  CMP RDI,RAX
141decc0b  JZ 0x141decc5a
141decc0d  MOV RCX,qword ptr [RAX]
141decc10  MOV qword ptr [RDI],RCX
141decc13  MOV qword ptr [RAX],0x0
141decc1a  MOV RCX,qword ptr [RAX + 0x8]
141decc1e  MOV RBX,qword ptr [RDI + 0x8]
141decc22  CMP RCX,RBX
141decc25  JZ 0x141decc5a
141decc27  MOV qword ptr [RAX + 0x8],0x0
141decc2f  MOV qword ptr [RDI + 0x8],RCX
141decc33  TEST RBX,RBX
141decc36  JZ 0x141decc5a
141decc38  SUB dword ptr [RBX + 0x8],0x1
141decc3c  JNZ 0x141decc5a
141decc3e  MOV RAX,qword ptr [RBX]
141decc41  MOV RCX,RBX
141decc44  CALL qword ptr [RAX]
141decc46  SUB dword ptr [RBX + 0xc],0x1
141decc4a  JNZ 0x141decc5a
141decc4c  MOV RAX,qword ptr [RBX]
141decc4f  MOV EDX,0x1
141decc54  MOV RCX,RBX
141decc57  CALL qword ptr [RAX + 0x8]
141decc5a  MOV RBX,qword ptr [RBP + -0x11]
141decc5e  TEST RBX,RBX
141decc61  JZ 0x141decc85
141decc63  SUB dword ptr [RBX + 0x8],0x1
141decc67  JNZ 0x141decc85
141decc69  MOV RAX,qword ptr [RBX]
141decc6c  MOV RCX,RBX
141decc6f  CALL qword ptr [RAX]
141decc71  SUB dword ptr [RBX + 0xc],0x1
141decc75  JNZ 0x141decc85
141decc77  MOV RAX,qword ptr [RBX]
141decc7a  MOV EDX,0x1
141decc7f  MOV RCX,RBX
141decc82  CALL qword ptr [RAX + 0x8]
141decc85  CMP qword ptr [RDI],0x0
141decc89  JZ 0x141decd35
141decc8f  CALL 0x141eef450
141decc94  MOV RCX,qword ptr [RDI]
141decc97  XOR R8D,R8D
141decc9a  XORPS XMM1,XMM1
141decc9d  CALL 0x1445fb140
141decca2  LEA RAX,[RBP + -0x19]
141decca6  CMP RDI,RAX
141decca9  JZ 0x141decce5
141deccab  MOV qword ptr [RDI],0x0
141deccb2  MOV RBX,qword ptr [RDI + 0x8]
141deccb6  TEST RBX,RBX
141deccb9  JZ 0x141decce5
141deccbb  MOV qword ptr [RDI + 0x8],0x0
141deccc3  SUB dword ptr [RBX + 0x8],0x1
141deccc7  JNZ 0x141decce5
141deccc9  MOV RAX,qword ptr [RBX]
141decccc  MOV RCX,RBX
141decccf  CALL qword ptr [RAX]
141deccd1  SUB dword ptr [RBX + 0xc],0x1
141deccd5  JNZ 0x141decce5
141deccd7  MOV RAX,qword ptr [RBX]
141deccda  MOV EDX,0x1
141deccdf  MOV RCX,RBX
141decce2  CALL qword ptr [RAX + 0x8]
141decce5  CMP byte ptr [0x1481348e0],0x4
141deccec  JC 0x141decd35
141deccee  LEA RCX,[RBP + -0x39]
141deccf2  CALL qword ptr [0x1458fca10]
141deccf8  XORPS XMM0,XMM0
141deccfb  LEA RAX,[0x145fde0d0]
141decd02  CVTSI2SD XMM0,qword ptr [RBP + -0x39]
141decd08  MOV R9D,0x4
141decd0e  LEA R8,[0x1481348e4]
141decd15  XOR EDX,EDX
141decd17  XOR ECX,ECX
141decd19  MULSD XMM0,qword ptr [0x1481d47e8]
141decd21  SUBSD XMM0,XMM6
141decd25  MOVSD qword ptr [RSP + 0x28],XMM0
141decd2b  MOV qword ptr [RSP + 0x20],RAX
141decd30  CALL 0x142c54b70
141decd35  MOVAPS XMM6,xmmword ptr [RSP + 0xb0]
141decd3d  LEA R12,[R15 + 0x498]
141decd44  MOVSXD RBX,dword ptr [R12 + 0x8]
141decd49  MOV RDI,qword ptr [R12]
141decd4d  MOV qword ptr [RBP + -0x9],0x0
141decd55  MOV dword ptr [RBP + -0x1],EBX
141decd58  TEST EBX,EBX
141decd5a  JNZ 0x141decd61
141decd5c  MOV dword ptr [RBP + 0x3],EBX
141decd5f  JMP 0x141decd82
141decd61  XOR R8D,R8D
141decd64  LEA RCX,[RBP + -0x9]
141decd68  MOV EDX,EBX
141decd6a  CALL 0x1406d93e0
141decd6f  MOV RCX,qword ptr [RBP + -0x9]
141decd73  MOV R8,RBX
141decd76  SHL R8,0x3
141decd7a  MOV RDX,RDI
141decd7d  CALL 0x1458092ed
141decd82  MOV R9,qword ptr [R15 + 0x4a8]
141decd89  MOVSXD RAX,dword ptr [R15 + 0x4b0]
141decd90  MOV qword ptr [RBP + -0x39],R9
141decd94  LEA RAX,[R9 + RAX*0x8]
141decd98  MOV qword ptr [RBP + -0x29],RAX
141decd9c  CMP R9,RAX
141decd9f  JZ 0x141dece86
141decda5  NOP word ptr [RAX + RAX*0x1]
141decdb0  MOVSXD RDX,dword ptr [R12 + 0x8]
141decdb5  TEST EDX,EDX
141decdb7  JZ 0x141dece6a
141decdbd  MOV RCX,qword ptr [R12]
141decdc1  XOR R13D,R13D
141decdc4  MOV EAX,dword ptr [R9 + 0x4]
141decdc8  XOR EDI,EDI
141decdca  XOR EBX,EBX
141decdcc  MOV R14,RDX
141decdcf  CMP dword ptr [RCX + 0x4],EAX
141decdd2  MOV EAX,dword ptr [R9]
141decdd5  SETNZ R15B
141decdd9  CMP dword ptr [RCX],EAX
141decddb  SETNZ AL
141decdde  OR R15B,AL
141decde1  MOVSXD R11,EDI
141decde4  INC RBX
141decde7  INC EDI
141decde9  CMP RBX,R14
141decdec  JGE 0x141dece27
141decdee  MOV RAX,qword ptr [R12]
141decdf2  MOV R8D,dword ptr [R9]
141decdf5  MOV R9D,dword ptr [R9 + 0x4]
141decdf9  MOVZX R10D,R15B
141decdfd  LEA RDX,[RAX + RBX*0x8]
141dece01  XOR ECX,ECX
141dece03  CMP dword ptr [RDX + 0x4],R9D
141dece07  SETNZ CL
141dece0a  XOR EAX,EAX
141dece0c  CMP dword ptr [RDX],R8D
141dece0f  SETNZ AL
141dece12  OR ECX,EAX
141dece14  CMP R10D,ECX
141dece17  JNZ 0x141dece27
141dece19  INC EDI
141dece1b  INC RBX
141dece1e  ADD RDX,0x8
141dece22  CMP RBX,R14
141dece25  JL 0x141dece01
141dece27  MOV ESI,EDI
141dece29  SUB ESI,R11D
141dece2c  TEST R15B,R15B
141dece2f  JZ 0x141dece54
141dece31  CMP R13D,R11D
141dece34  JZ 0x141dece51
141dece36  MOV RCX,qword ptr [R12]
141dece3a  MOVSXD RAX,R13D
141dece3d  MOVSXD R8,ESI
141dece40  SHL R8,0x3
141dece44  LEA RDX,[RCX + R11*0x8]
141dece48  LEA RCX,[RCX + RAX*0x8]
141dece4c  CALL 0x1458092f3
141dece51  ADD R13D,ESI
141dece54  MOV R9,qword ptr [RBP + -0x39]
141dece58  XOR R15B,0x1
141dece5c  CMP RBX,R14
141dece5f  JL 0x141decde1
141dece61  MOV RAX,qword ptr [RBP + -0x29]
141dece65  MOV dword ptr [R12 + 0x8],R13D
141dece6a  ADD R9,0x8
141dece6e  MOV qword ptr [RBP + -0x39],R9
141dece72  CMP R9,RAX
141dece75  JNZ 0x141decdb0
141dece7b  MOV R15,qword ptr [RBP + -0x49]
141dece7f  LEA R13,[0x1459a4b30]
141dece86  MOVZX EDX,byte ptr [RBP + -0x59]
141dece8a  MOV RCX,R15
141dece8d  CALL 0x141debbc0
141dece92  MOVSXD RBX,dword ptr [R15 + 0x4b0]
141dece99  MOV R14,qword ptr [RSP + 0xc0]
141decea1  MOV RSI,qword ptr [RSP + 0xd8]
141decea9  TEST EBX,EBX
141deceab  JZ 0x141decef3
141decead  MOV EAX,dword ptr [R12 + 0x8]
141deceb2  LEA EDX,[RAX + RBX*0x1]
141deceb5  CMP EDX,dword ptr [R12 + 0xc]
141deceba  JLE 0x141decec9
141decebc  MOV RCX,R12
141decebf  CALL 0x1406d9a30
141decec4  MOV EAX,dword ptr [R12 + 0x8]
141decec9  MOV RDX,qword ptr [R15 + 0x4a8]
141deced0  MOV R8,RBX
141deced3  MOVSXD RCX,EAX
141deced6  MOV RAX,qword ptr [R12]
141deceda  SHL R8,0x3
141decede  LEA RCX,[RAX + RCX*0x8]
141decee2  CALL 0x1458092ed
141decee7  ADD dword ptr [R12 + 0x8],EBX
141deceec  MOV EAX,dword ptr [R12 + 0x8]
141decef1  JMP 0x141decefa
141decef3  MOV EAX,dword ptr [R15 + 0x4a0]
141decefa  MOV R8,qword ptr [R12]
141decefe  CDQE
141decf00  LEA R9,[R8 + RAX*0x8]
141decf04  CMP R8,R9
141decf07  JZ 0x141decfc1
141decf0d  MOV R10D,dword ptr [0x14812b6b4]
141decf14  MOV RAX,qword ptr [0x14812b6b0]
141decf1b  NOP dword ptr [RAX + RAX*0x1]
141decf20  CMP dword ptr [R8 + 0x4],R10D
141decf24  SETZ DL
141decf27  CMP dword ptr [R8],EAX
141decf2a  SETZ CL
141decf2d  TEST CL,DL
141decf2f  JNZ 0x141decf3f
141decf31  ADD R8,0x8
141decf35  CMP R8,R9
141decf38  JNZ 0x141decf20
141decf3a  JMP 0x141decfc1
141decf3f  LEA RDX,[0x14812b6b8]
141decf46  MOV RCX,R12
141decf49  CALL 0x141e0dd50
141decf4e  LEA RDX,[0x14812b6c0]
141decf55  MOV RCX,R12
141decf58  CALL 0x141e0dd50
141decf5d  MOVZX EAX,byte ptr [R15 + 0x6cc]
141decf65  CMP AL,0x1
141decf67  JNZ 0x141decf90
141decf69  MOVSXD RBX,dword ptr [R12 + 0x8]
141decf6e  LEA EAX,[RBX + 0x1]
141decf71  MOV dword ptr [R12 + 0x8],EAX
141decf76  CMP EAX,dword ptr [R12 + 0xc]
141decf7b  JLE 0x141decf87
141decf7d  MOV EDX,EBX
141decf7f  MOV RCX,R12
141decf82  CALL 0x140690bb0
141decf87  MOV RAX,qword ptr [0x14812b6b8]
141decf8e  JMP 0x141decfb9
141decf90  TEST AL,AL
141decf92  JNZ 0x141decfc1
141decf94  MOVSXD RBX,dword ptr [R12 + 0x8]
141decf99  LEA EAX,[RBX + 0x1]
141decf9c  MOV dword ptr [R12 + 0x8],EAX
141decfa1  CMP EAX,dword ptr [R12 + 0xc]
141decfa6  JLE 0x141decfb2
141decfa8  MOV EDX,EBX
141decfaa  MOV RCX,R12
141decfad  CALL 0x140690bb0
141decfb2  MOV RAX,qword ptr [0x14812b6c0]
141decfb9  MOV RCX,qword ptr [R12]
141decfbd  MOV qword ptr [RCX + RBX*0x8],RAX
141decfc1  MOVSXD RBX,dword ptr [RBP + -0x1]
141decfc5  MOV RDI,qword ptr [RBP + -0x9]
141decfc9  MOV qword ptr [RBP + 0x7],0x0
141decfd1  MOV dword ptr [RBP + 0xf],EBX
141decfd4  TEST EBX,EBX
141decfd6  JNZ 0x141decfdd
141decfd8  MOV dword ptr [RBP + 0x13],EBX
141decfdb  JMP 0x141decffe
141decfdd  XOR R8D,R8D
141decfe0  LEA RCX,[RBP + 0x7]
141decfe4  MOV EDX,EBX
141decfe6  CALL 0x1406d93e0
141decfeb  MOV RCX,qword ptr [RBP + 0x7]
141decfef  MOV R8,RBX
141decff2  SHL R8,0x3
141decff6  MOV RDX,RDI
141decff9  CALL 0x1458092ed
141decffe  LEA RAX,[RBP + -0x29]
141ded002  MOV byte ptr [RSP + 0x28],0x0
141ded007  LEA R9,[RBP + 0x7]
141ded00b  MOV qword ptr [RSP + 0x20],RAX
141ded010  LEA R8,[RBP + -0x49]
141ded014  MOV qword ptr [RBP + -0x19],0x0
141ded01c  LEA RDX,[RBP + -0x19]
141ded020  MOV qword ptr [RBP + -0x11],0x0
141ded028  MOV RCX,R15
141ded02b  MOV qword ptr [RBP + -0x29],0x0
141ded033  MOV qword ptr [RBP + -0x21],0x0
141ded03b  MOV qword ptr [RBP + -0x49],0x0
141ded043  MOV qword ptr [RBP + -0x41],0x0
141ded04b  CALL 0x14401ea80
141ded050  MOV RCX,qword ptr [RBP + -0x49]
141ded054  MOVZX EBX,AL
141ded057  TEST RCX,RCX
141ded05a  JZ 0x141ded061
141ded05c  CALL 0x142bee710
141ded061  MOV RCX,qword ptr [RBP + -0x29]
141ded065  TEST RCX,RCX
141ded068  JZ 0x141ded06f
141ded06a  CALL 0x142bee710
141ded06f  TEST BL,BL
141ded071  JZ 0x141ded242
141ded077  CMP byte ptr [0x148126e88],0x5
141ded07e  JC 0x141ded0ca
141ded080  MOV RDX,R12
141ded083  LEA RCX,[RBP + -0x49]
141ded087  CALL 0x141e05950
141ded08c  CMP dword ptr [RAX + 0x8],0x0
141ded090  JZ 0x141ded095
141ded092  MOV R13,qword ptr [RAX]
141ded095  LEA RAX,[0x145fde150]
141ded09c  MOV qword ptr [RSP + 0x28],R13
141ded0a1  MOV R9D,0x5
141ded0a7  MOV qword ptr [RSP + 0x20],RAX
141ded0ac  LEA R8,[0x148126e8c]
141ded0b3  XOR EDX,EDX
141ded0b5  XOR ECX,ECX
141ded0b7  CALL 0x142c54b70
141ded0bc  MOV RCX,qword ptr [RBP + -0x49]
141ded0c0  TEST RCX,RCX
141ded0c3  JZ 0x141ded0ca
141ded0c5  CALL 0x142bee710
141ded0ca  LEA RDI,[R15 + 0x730]
141ded0d1  LEA RAX,[RBP + -0x49]
141ded0d5  CMP RDI,RAX
141ded0d8  JZ 0x141ded114
141ded0da  MOV qword ptr [RDI],0x0
141ded0e1  MOV RBX,qword ptr [RDI + 0x8]
141ded0e5  TEST RBX,RBX
141ded0e8  JZ 0x141ded114
141ded0ea  MOV qword ptr [RDI + 0x8],0x0
141ded0f2  SUB dword ptr [RBX + 0x8],0x1
141ded0f6  JNZ 0x141ded114
141ded0f8  MOV RAX,qword ptr [RBX]
141ded0fb  MOV RCX,RBX
141ded0fe  CALL qword ptr [RAX]
141ded100  SUB dword ptr [RBX + 0xc],0x1
141ded104  JNZ 0x141ded114
141ded106  MOV RAX,qword ptr [RBX]
141ded109  MOV EDX,0x1
141ded10e  MOV RCX,RBX
141ded111  CALL qword ptr [RAX + 0x8]
141ded114  XOR R8D,R8D
141ded117  MOV qword ptr [RBP + -0x49],0x0
141ded11f  XOR ECX,ECX
141ded121  LEA EDX,[R8 + 0x30]
141ded125  CALL 0x142bfd200
141ded12a  MOV qword ptr [RBP + -0x49],RAX
141ded12e  MOV RBX,RAX
141ded131  MOV dword ptr [RBP + -0x41],0x3
141ded138  TEST RAX,RAX
141ded13b  JZ 0x141ded171
141ded13d  LEA RAX,[0x1459bfd10]
141ded144  MOV RDX,R15
141ded147  LEA RCX,[RBX + 0x8]
141ded14b  MOV qword ptr [RBX],RAX
141ded14e  CALL 0x142ee3b30
141ded153  LEA RAX,[0x141e07c00]
141ded15a  MOV qword ptr [RBX + 0x10],RAX
141ded15e  CALL 0x142bad740
141ded163  MOV qword ptr [RBX + 0x20],RAX
141ded167  LEA RAX,[0x1459bfd68]
141ded16e  MOV qword ptr [RBX],RAX
141ded171  MOV RAX,qword ptr [R15]
141ded174  LEA RCX,[RBP + -0x49]
141ded178  MOV dword ptr [RSP + 0x38],0x64
141ded180  LEA R8,[RBP + -0x19]
141ded184  MOV qword ptr [RSP + 0x30],RCX
141ded189  LEA RDX,[RBP + -0x29]
141ded18d  LEA RCX,[RBP + -0x9]
141ded191  MOV byte ptr [RSP + 0x28],0x0
141ded196  MOV qword ptr [RSP + 0x20],RCX
141ded19b  MOV R9,R12
141ded19e  MOV RCX,R15
141ded1a1  CALL qword ptr [RAX + 0x330]
141ded1a7  MOV RAX,qword ptr [RBP + -0x29]
141ded1ab  TEST RAX,RAX
141ded1ae  JZ 0x141ded217
141ded1b0  CMP byte ptr [RAX + 0x10],0x0
141ded1b4  JNZ 0x141ded217
141ded1b6  MOVAPS XMM1,xmmword ptr [RBP + -0x29]
141ded1ba  MOVDQA XMM0,XMM1
141ded1be  MOVAPS xmmword ptr [RBP + -0x49],XMM1
141ded1c2  PSRLDQ XMM0,0x8
141ded1c7  MOVQ RAX,XMM0
141ded1cc  TEST RAX,RAX
141ded1cf  JZ 0x141ded1d4
141ded1d1  INC dword ptr [RAX + 0x8]
141ded1d4  LEA RAX,[RBP + -0x49]
141ded1d8  CMP RAX,RDI
141ded1db  JZ 0x141ded1e7
141ded1dd  MOVUPS XMM0,xmmword ptr [RDI]
141ded1e0  MOVUPS xmmword ptr [RDI],XMM1
141ded1e3  MOVUPS xmmword ptr [RBP + -0x49],XMM0
141ded1e7  MOV RBX,qword ptr [RBP + -0x41]
141ded1eb  TEST RBX,RBX
141ded1ee  JZ 0x141ded212
141ded1f0  SUB dword ptr [RBX + 0x8],0x1
141ded1f4  JNZ 0x141ded212
141ded1f6  MOV RAX,qword ptr [RBX]
141ded1f9  MOV RCX,RBX
141ded1fc  CALL qword ptr [RAX]
141ded1fe  SUB dword ptr [RBX + 0xc],0x1
141ded202  JNZ 0x141ded212
141ded204  MOV RAX,qword ptr [RBX]
141ded207  MOV EDX,0x1
141ded20c  MOV RCX,RBX
141ded20f  CALL qword ptr [RAX + 0x8]
141ded212  CALL 0x141eef450
141ded217  MOV RBX,qword ptr [RBP + -0x21]
141ded21b  TEST RBX,RBX
141ded21e  JZ 0x141ded242
141ded220  SUB dword ptr [RBX + 0x8],0x1
141ded224  JNZ 0x141ded242
141ded226  MOV RAX,qword ptr [RBX]
141ded229  MOV RCX,RBX
141ded22c  CALL qword ptr [RAX]
141ded22e  SUB dword ptr [RBX + 0xc],0x1
141ded232  JNZ 0x141ded242
141ded234  MOV RAX,qword ptr [RBX]
141ded237  MOV EDX,0x1
141ded23c  MOV RCX,RBX
141ded23f  CALL qword ptr [RAX + 0x8]
141ded242  MOV RCX,qword ptr [RBP + -0x19]
141ded246  MOV R13,qword ptr [RSP + 0xc8]
141ded24e  TEST RCX,RCX
141ded251  JZ 0x141ded258
141ded253  CALL 0x142bee710
141ded258  MOV RCX,qword ptr [RBP + 0x7]
141ded25c  TEST RCX,RCX
141ded25f  JZ 0x141ded266
141ded261  CALL 0x142bee710
141ded266  MOV RCX,qword ptr [RBP + -0x9]
141ded26a  TEST RCX,RCX
141ded26d  JZ 0x141ded274
141ded26f  CALL 0x142bee710
141ded274  CALL 0x141df61e0
141ded279  MOV RDI,RAX
141ded27c  LEA RCX,[RAX + 0x1a0]
141ded283  CALL qword ptr [0x1458fc9f8]
141ded289  LEA RCX,[RDI + 0x100]
141ded290  XOR EDX,EDX
141ded292  CALL 0x1407bb180
141ded297  LEA RCX,[RDI + 0x150]
141ded29e  XOR EDX,EDX
141ded2a0  CALL 0x140992250
141ded2a5  LEA RCX,[RDI + 0x1a0]
141ded2ac  CALL qword ptr [0x1458fca00]
141ded2b2  MOV RCX,qword ptr [0x148351eb8]
141ded2b9  MOV DL,0x1
141ded2bb  CALL 0x14463f4b0
141ded2c0  MOV RDI,qword ptr [RSP + 0xd0]
141ded2c8  MOV RBX,qword ptr [RSP + 0x110]
141ded2d0  ADD RSP,0xe0
141ded2d7  POP R15
141ded2d9  POP R12
141ded2db  POP RBP
141ded2dc  RET
