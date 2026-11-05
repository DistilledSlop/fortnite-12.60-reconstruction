141d2c800  MOV qword ptr [RSP + 0x10],RBX
141d2c805  MOV qword ptr [RSP + 0x18],RSI
141d2c80a  MOV qword ptr [RSP + 0x20],RDI
141d2c80f  PUSH RBP
141d2c810  PUSH R12
141d2c812  PUSH R13
141d2c814  PUSH R14
141d2c816  PUSH R15
141d2c818  LEA RBP,[RSP + -0x37]
141d2c81d  SUB RSP,0xe0
141d2c824  MOV RAX,qword ptr [0x1480c4288]
141d2c82b  XOR RAX,RSP
141d2c82e  MOV qword ptr [RBP + 0x27],RAX
141d2c832  MOV RDI,RCX
141d2c835  CALL 0x1409ef640
141d2c83a  AND dword ptr [RDI + 0x28],0xfffffffd
141d2c83e  LEA RAX,[0x145f987a8]
141d2c845  MOV qword ptr [RDI],RAX
141d2c848  XOR R13D,R13D
141d2c84b  MOV RCX,RDI
141d2c84e  MOV qword ptr [RDI + 0x148],R13
141d2c855  MOV qword ptr [RDI + 0x168],R13
141d2c85c  MOV qword ptr [RDI + 0x170],R13
141d2c863  LEA EDX,[R13 + 0x30]
141d2c867  MOV qword ptr [RDI + 0x178],R13
141d2c86e  MOV qword ptr [RDI + 0x180],R13
141d2c875  MOV qword ptr [RDI + 0x198],R13
141d2c87c  MOV dword ptr [RDI + 0x1a0],R13D
141d2c883  MOV dword ptr [RDI + 0x1a4],0x80
141d2c88d  MOV dword ptr [RDI + 0x1a8],0xffffffff
141d2c897  MOV dword ptr [RDI + 0x1ac],R13D
141d2c89e  MOV qword ptr [RDI + 0x1b8],R13
141d2c8a5  MOV dword ptr [RDI + 0x1c0],R13D
141d2c8ac  MOV qword ptr [RDI + 0x1c8],R13
141d2c8b3  MOV qword ptr [RDI + 0x1d0],R13
141d2c8ba  MOV qword ptr [RDI + 0x1d8],R13
141d2c8c1  MOV qword ptr [RDI + 0x1e0],R13
141d2c8c8  MOV qword ptr [RDI + 0x1e8],R13
141d2c8cf  MOV byte ptr [RDI + 0x15a],0x1
141d2c8d6  CALL 0x142ecc600
141d2c8db  TEST AL,AL
141d2c8dd  JNZ 0x141d2c937
141d2c8df  MOV RCX,qword ptr [0x14823d260]
141d2c8e6  TEST RCX,RCX
141d2c8e9  JZ 0x141d2c937
141d2c8eb  LEA RBX,[0x148247648]
141d2c8f2  LEA R9,[0x1478c36cc]
141d2c8f9  MOV qword ptr [RSP + 0x20],RBX
141d2c8fe  LEA R8,[0x145f98b10]
141d2c905  LEA RDX,[0x145f98b68]
141d2c90c  CALL 0x142c71b90
141d2c911  MOV RCX,qword ptr [0x14823d260]
141d2c918  LEA R9,[0x1478c36d0]
141d2c91f  LEA R8,[0x145f98ba0]
141d2c926  MOV qword ptr [RSP + 0x20],RBX
141d2c92b  LEA RDX,[0x145f98b68]
141d2c932  CALL 0x142c71b90
141d2c937  MOV EDX,0x30
141d2c93c  MOV RCX,RDI
141d2c93f  CALL 0x142ecc600
141d2c944  TEST AL,AL
141d2c946  JZ 0x141d2d269
141d2c94c  MOV R8D,0x1
141d2c952  LEA RDX,[0x145f98bf8]
141d2c959  LEA RCX,[RBP + -0x79]
141d2c95d  CALL 0x142cd49c0
141d2c962  MOVAPS XMM5,xmmword ptr [0x1478c3830]
141d2c969  LEA R15,[0x1406fbcc0]
141d2c970  MOVAPS XMM4,xmmword ptr [0x1478c3840]
141d2c977  LEA R14,[0x1459ccad0]
141d2c97e  MOV RAX,qword ptr [RBP + -0x79]
141d2c982  MOVAPS XMM2,xmmword ptr [0x1478c3850]
141d2c989  MOVAPS XMM3,xmmword ptr [0x1478c3860]
141d2c990  MOV qword ptr [0x1478c37a0],RAX
141d2c997  LEA RAX,[0x141d38cf0]
141d2c99e  MOV qword ptr [RBP + -0x31],RAX
141d2c9a2  MOVQ RAX,XMM5
141d2c9a7  MOV qword ptr [RBP + -0x49],R13
141d2c9ab  MOVAPS XMM1,xmmword ptr [RBP + -0x49]
141d2c9af  MOV qword ptr [RBP + -0x59],R15
141d2c9b3  MOVAPS XMM0,xmmword ptr [RBP + -0x59]
141d2c9b7  MOV qword ptr [RBP + -0x39],R14
141d2c9bb  MOVAPS xmmword ptr [0x1478c3830],XMM0
141d2c9c2  MOVAPS XMM0,xmmword ptr [RBP + -0x39]
141d2c9c6  MOVAPS xmmword ptr [0x1478c3840],XMM1
141d2c9cd  MOVAPS XMM1,xmmword ptr [RBP + -0x29]
141d2c9d1  MOVAPS xmmword ptr [RBP + -0x9],XMM4
141d2c9d5  MOVAPS xmmword ptr [0x1478c3850],XMM0
141d2c9dc  MOVAPS xmmword ptr [0x1478c3860],XMM1
141d2c9e3  MOVAPS xmmword ptr [RBP + -0x59],XMM5
141d2c9e7  MOVAPS xmmword ptr [RBP + -0x49],XMM4
141d2c9eb  MOVAPS xmmword ptr [RBP + -0x39],XMM2
141d2c9ef  MOVAPS xmmword ptr [RBP + -0x29],XMM3
141d2c9f3  TEST RAX,RAX
141d2c9f6  JZ 0x141d2ca0d
141d2c9f8  MOV RAX,qword ptr [RBP + -0x9]
141d2c9fc  LEA RCX,[RBP + -0x39]
141d2ca00  TEST RAX,RAX
141d2ca03  CMOVNZ RCX,RAX
141d2ca07  MOV RAX,qword ptr [RCX]
141d2ca0a  CALL qword ptr [RAX + 0x10]
141d2ca0d  MOV R8D,0x1
141d2ca13  LEA RDX,[0x145f98c04]
141d2ca1a  LEA RCX,[RBP + -0x79]
141d2ca1e  CALL 0x142cd49c0
141d2ca23  MOVAPS XMM5,xmmword ptr [0x1478c3930]
141d2ca2a  LEA RSI,[0x141d3a260]
141d2ca31  MOVAPS XMM4,xmmword ptr [0x1478c3940]
141d2ca38  LEA RBX,[0x1459ccad0]
141d2ca3f  MOV RAX,qword ptr [RBP + -0x79]
141d2ca43  MOVAPS XMM2,xmmword ptr [0x1478c3950]
141d2ca4a  MOVAPS XMM3,xmmword ptr [0x1478c3960]
141d2ca51  MOV qword ptr [0x1478c38e0],RAX
141d2ca58  LEA RAX,[0x141d32060]
141d2ca5f  MOV qword ptr [RBP + -0x31],RAX
141d2ca63  MOVQ RAX,XMM5
141d2ca68  MOV qword ptr [RBP + -0x49],R13
141d2ca6c  MOVAPS XMM1,xmmword ptr [RBP + -0x49]
141d2ca70  MOV qword ptr [RBP + -0x59],RSI
141d2ca74  MOVAPS XMM0,xmmword ptr [RBP + -0x59]
141d2ca78  MOV qword ptr [RBP + -0x39],RBX
141d2ca7c  MOVAPS xmmword ptr [0x1478c3930],XMM0
141d2ca83  MOVAPS XMM0,xmmword ptr [RBP + -0x39]
141d2ca87  MOVAPS xmmword ptr [0x1478c3940],XMM1
141d2ca8e  MOVAPS XMM1,xmmword ptr [RBP + -0x29]
141d2ca92  MOVAPS xmmword ptr [RBP + -0x9],XMM4
141d2ca96  MOVAPS xmmword ptr [0x1478c3950],XMM0
141d2ca9d  MOVAPS xmmword ptr [0x1478c3960],XMM1
141d2caa4  MOVAPS xmmword ptr [RBP + -0x59],XMM5
141d2caa8  MOVAPS xmmword ptr [RBP + -0x49],XMM4
141d2caac  MOVAPS xmmword ptr [RBP + -0x39],XMM2
141d2cab0  MOVAPS xmmword ptr [RBP + -0x29],XMM3
141d2cab4  TEST RAX,RAX
141d2cab7  JZ 0x141d2cace
141d2cab9  MOV RAX,qword ptr [RBP + -0x9]
141d2cabd  LEA RCX,[RBP + -0x39]
141d2cac1  TEST RAX,RAX
141d2cac4  CMOVNZ RCX,RAX
141d2cac8  MOV RAX,qword ptr [RCX]
141d2cacb  CALL qword ptr [RAX + 0x10]
141d2cace  MOV R8D,0x1
141d2cad4  LEA RDX,[0x145f98c0c]
141d2cadb  LEA RCX,[RBP + -0x79]
141d2cadf  CALL 0x142cd49c0
141d2cae4  MOVAPS XMM5,xmmword ptr [0x1478c3a30]
141d2caeb  MOVAPS XMM4,xmmword ptr [0x1478c3a40]
141d2caf2  MOV RAX,qword ptr [RBP + -0x79]
141d2caf6  MOVAPS XMM2,xmmword ptr [0x1478c3a50]
141d2cafd  MOVAPS XMM3,xmmword ptr [0x1478c3a60]
141d2cb04  MOV qword ptr [0x1478c3a20],RAX
141d2cb0b  LEA RAX,[0x141de2550]
141d2cb12  MOV qword ptr [RBP + -0x31],RAX
141d2cb16  MOVQ RAX,XMM5
141d2cb1b  MOV qword ptr [RBP + -0x49],R13
141d2cb1f  MOVAPS XMM1,xmmword ptr [RBP + -0x49]
141d2cb23  MOV qword ptr [RBP + -0x59],RSI
141d2cb27  MOVAPS XMM0,xmmword ptr [RBP + -0x59]
141d2cb2b  MOV qword ptr [RBP + -0x39],RBX
141d2cb2f  MOVAPS xmmword ptr [0x1478c3a30],XMM0
141d2cb36  MOVAPS XMM0,xmmword ptr [RBP + -0x39]
141d2cb3a  MOVAPS xmmword ptr [0x1478c3a40],XMM1
141d2cb41  MOVAPS XMM1,xmmword ptr [RBP + -0x29]
141d2cb45  MOVAPS xmmword ptr [RBP + -0x9],XMM4
141d2cb49  MOVAPS xmmword ptr [0x1478c3a50],XMM0
141d2cb50  MOVAPS xmmword ptr [0x1478c3a60],XMM1
141d2cb57  MOVAPS xmmword ptr [RBP + -0x59],XMM5
141d2cb5b  MOVAPS xmmword ptr [RBP + -0x49],XMM4
141d2cb5f  MOVAPS xmmword ptr [RBP + -0x39],XMM2
141d2cb63  MOVAPS xmmword ptr [RBP + -0x29],XMM3
141d2cb67  TEST RAX,RAX
141d2cb6a  JZ 0x141d2cb81
141d2cb6c  MOV RAX,qword ptr [RBP + -0x9]
141d2cb70  LEA RCX,[RBP + -0x39]
141d2cb74  TEST RAX,RAX
141d2cb77  CMOVNZ RCX,RAX
141d2cb7b  MOV RAX,qword ptr [RCX]
141d2cb7e  CALL qword ptr [RAX + 0x10]
141d2cb81  MOV ECX,0x14
141d2cb86  CALL 0x141384ca0
141d2cb8b  TEST RAX,RAX
141d2cb8e  JZ 0x141d2cba1
141d2cb90  MOV qword ptr [RAX],0x3f800000
141d2cb97  MOV qword ptr [RAX + 0x8],R13
141d2cb9b  MOV dword ptr [RAX + 0x10],R13D
141d2cb9f  JMP 0x141d2cba4
141d2cba1  MOV RAX,R13
141d2cba4  MOV RCX,qword ptr [0x1478c3b30]
141d2cbab  MOV EDX,0x1
141d2cbb0  MOV qword ptr [0x1478c3b30],RAX
141d2cbb7  CALL 0x141384cb0
141d2cbbc  MOV EDX,0x14
141d2cbc1  XOR ECX,ECX
141d2cbc3  CALL 0x141384cb0
141d2cbc8  MOV R8D,0x1
141d2cbce  LEA RDX,[0x145f98c10]
141d2cbd5  LEA RCX,[RBP + -0x79]
141d2cbd9  CALL 0x142cd49c0
141d2cbde  MOVAPS XMM5,xmmword ptr [0x1478c3b70]
141d2cbe5  MOVAPS XMM4,xmmword ptr [0x1478c3b80]
141d2cbec  MOV RAX,qword ptr [RBP + -0x79]
141d2cbf0  MOVAPS XMM2,xmmword ptr [0x1478c3b90]
141d2cbf7  MOVAPS XMM3,xmmword ptr [0x1478c3ba0]
141d2cbfe  MOV qword ptr [0x1478c3b60],RAX
141d2cc05  LEA RAX,[0x141d394f0]
141d2cc0c  MOV qword ptr [RBP + -0x31],RAX
141d2cc10  MOVQ RAX,XMM5
141d2cc15  MOV qword ptr [RBP + -0x49],R13
141d2cc19  MOVAPS XMM1,xmmword ptr [RBP + -0x49]
141d2cc1d  MOV qword ptr [RBP + -0x59],RSI
141d2cc21  MOVAPS XMM0,xmmword ptr [RBP + -0x59]
141d2cc25  MOV qword ptr [RBP + -0x39],RBX
141d2cc29  MOVAPS xmmword ptr [0x1478c3b70],XMM0
141d2cc30  MOVAPS XMM0,xmmword ptr [RBP + -0x39]
141d2cc34  MOVAPS xmmword ptr [0x1478c3b80],XMM1
141d2cc3b  MOVAPS XMM1,xmmword ptr [RBP + -0x29]
141d2cc3f  MOVAPS xmmword ptr [RBP + -0x9],XMM4
141d2cc43  MOVAPS xmmword ptr [0x1478c3b90],XMM0
141d2cc4a  MOVAPS xmmword ptr [0x1478c3ba0],XMM1
141d2cc51  MOVAPS xmmword ptr [RBP + -0x59],XMM5
141d2cc55  MOVAPS xmmword ptr [RBP + -0x49],XMM4
141d2cc59  MOVAPS xmmword ptr [RBP + -0x39],XMM2
141d2cc5d  MOVAPS xmmword ptr [RBP + -0x29],XMM3
141d2cc61  TEST RAX,RAX
141d2cc64  JZ 0x141d2cc7b
141d2cc66  MOV RAX,qword ptr [RBP + -0x9]
141d2cc6a  LEA RCX,[RBP + -0x39]
141d2cc6e  TEST RAX,RAX
141d2cc71  CMOVNZ RCX,RAX
141d2cc75  MOV RAX,qword ptr [RCX]
141d2cc78  CALL qword ptr [RAX + 0x10]
141d2cc7b  MOVAPS XMM5,xmmword ptr [0x1478c3bf0]
141d2cc82  LEA RAX,[0x141d398d0]
141d2cc89  MOVAPS XMM4,xmmword ptr [0x1478c3c00]
141d2cc90  MOVAPS XMM2,xmmword ptr [0x1478c3c10]
141d2cc97  MOVAPS XMM3,xmmword ptr [0x1478c3c20]
141d2cc9e  MOV qword ptr [RBP + -0x31],RAX
141d2cca2  MOVQ RAX,XMM5
141d2cca7  MOV qword ptr [RBP + -0x49],R13
141d2ccab  MOVAPS XMM1,xmmword ptr [RBP + -0x49]
141d2ccaf  MOV qword ptr [RBP + -0x59],R15
141d2ccb3  MOVAPS XMM0,xmmword ptr [RBP + -0x59]
141d2ccb7  MOV qword ptr [RBP + -0x39],R14
141d2ccbb  MOVAPS xmmword ptr [0x1478c3bf0],XMM0
141d2ccc2  MOVAPS XMM0,xmmword ptr [RBP + -0x39]
141d2ccc6  MOVAPS xmmword ptr [0x1478c3c00],XMM1
141d2cccd  MOVAPS XMM1,xmmword ptr [RBP + -0x29]
141d2ccd1  MOVAPS xmmword ptr [0x1478c3c10],XMM0
141d2ccd8  MOVAPS xmmword ptr [0x1478c3c20],XMM1
141d2ccdf  MOVAPS xmmword ptr [RBP + -0x9],XMM4
141d2cce3  MOVAPS xmmword ptr [RBP + -0x59],XMM5
141d2cce7  MOVAPS xmmword ptr [RBP + -0x49],XMM4
141d2cceb  MOVAPS xmmword ptr [RBP + -0x39],XMM2
141d2ccef  MOVAPS xmmword ptr [RBP + -0x29],XMM3
141d2ccf3  TEST RAX,RAX
141d2ccf6  JZ 0x141d2cd0d
141d2ccf8  MOV RAX,qword ptr [RBP + -0x9]
141d2ccfc  LEA RCX,[RBP + -0x39]
141d2cd00  TEST RAX,RAX
141d2cd03  CMOVNZ RCX,RAX
141d2cd07  MOV RAX,qword ptr [RCX]
141d2cd0a  CALL qword ptr [RAX + 0x10]
141d2cd0d  MOVAPS XMM5,xmmword ptr [0x1478c3c30]
141d2cd14  LEA RAX,[0x141d39420]
141d2cd1b  MOVAPS XMM4,xmmword ptr [0x1478c3c40]
141d2cd22  MOVAPS XMM2,xmmword ptr [0x1478c3c50]
141d2cd29  MOVAPS XMM3,xmmword ptr [0x1478c3c60]
141d2cd30  MOV qword ptr [RBP + -0x31],RAX
141d2cd34  LEA RAX,[0x1459ccad0]
141d2cd3b  MOV qword ptr [RBP + -0x39],RAX
141d2cd3f  LEA RAX,[0x141d3a240]
141d2cd46  MOV qword ptr [RBP + -0x59],RAX
141d2cd4a  MOVQ RAX,XMM5
141d2cd4f  MOVAPS XMM0,xmmword ptr [RBP + -0x59]
141d2cd53  MOV qword ptr [RBP + -0x49],R13
141d2cd57  MOVAPS XMM1,xmmword ptr [RBP + -0x49]
141d2cd5b  MOVAPS xmmword ptr [0x1478c3c30],XMM0
141d2cd62  MOVAPS XMM0,xmmword ptr [RBP + -0x39]
141d2cd66  MOVAPS xmmword ptr [0x1478c3c40],XMM1
141d2cd6d  MOVAPS XMM1,xmmword ptr [RBP + -0x29]
141d2cd71  MOVAPS xmmword ptr [0x1478c3c50],XMM0
141d2cd78  MOV byte ptr [0x1478c3b68],0x2
141d2cd7f  MOVAPS xmmword ptr [RBP + -0x9],XMM4
141d2cd83  MOVAPS xmmword ptr [0x1478c3c60],XMM1
141d2cd8a  MOVAPS xmmword ptr [RBP + -0x59],XMM5
141d2cd8e  MOVAPS xmmword ptr [RBP + -0x49],XMM4
141d2cd92  MOVAPS xmmword ptr [RBP + -0x39],XMM2
141d2cd96  MOVAPS xmmword ptr [RBP + -0x29],XMM3
141d2cd9a  TEST RAX,RAX
141d2cd9d  JZ 0x141d2cdb4
141d2cd9f  MOV RAX,qword ptr [RBP + -0x9]
141d2cda3  LEA RCX,[RBP + -0x39]
141d2cda7  TEST RAX,RAX
141d2cdaa  CMOVNZ RCX,RAX
141d2cdae  MOV RAX,qword ptr [RCX]
141d2cdb1  CALL qword ptr [RAX + 0x10]
141d2cdb4  MOV ECX,0x10
141d2cdb9  CALL 0x141384ca0
141d2cdbe  TEST RAX,RAX
141d2cdc1  JZ 0x141d2cdcc
141d2cdc3  MOV qword ptr [RAX],R13
141d2cdc6  MOV qword ptr [RAX + 0x8],R13
141d2cdca  JMP 0x141d2cdcf
141d2cdcc  MOV RAX,R13
141d2cdcf  MOV RCX,qword ptr [0x1478c3c70]
141d2cdd6  MOV EDX,0x1
141d2cddb  MOV qword ptr [0x1478c3c70],RAX
141d2cde2  CALL 0x141384cb0
141d2cde7  MOV R8D,0x1
141d2cded  LEA RDX,[0x145f98c28]
141d2cdf4  LEA RCX,[RBP + -0x79]
141d2cdf8  CALL 0x142cd49c0
141d2cdfd  MOVAPS XMM5,xmmword ptr [0x1478c3f70]
141d2ce04  MOVAPS XMM4,xmmword ptr [0x1478c3f80]
141d2ce0b  MOV RAX,qword ptr [RBP + -0x79]
141d2ce0f  MOVAPS XMM2,xmmword ptr [0x1478c3f90]
141d2ce16  MOVAPS XMM3,xmmword ptr [0x1478c3fa0]
141d2ce1d  MOV qword ptr [0x1478c3f20],RAX
141d2ce24  LEA RAX,[0x141d519e0]
141d2ce2b  MOV qword ptr [RBP + -0x31],RAX
141d2ce2f  MOVQ RAX,XMM5
141d2ce34  MOV qword ptr [RBP + -0x49],R13
141d2ce38  MOVAPS XMM1,xmmword ptr [RBP + -0x49]
141d2ce3c  MOV qword ptr [RBP + -0x59],RSI
141d2ce40  MOVAPS XMM0,xmmword ptr [RBP + -0x59]
141d2ce44  MOV qword ptr [RBP + -0x39],RBX
141d2ce48  MOVAPS xmmword ptr [0x1478c3f70],XMM0
141d2ce4f  MOVAPS XMM0,xmmword ptr [RBP + -0x39]
141d2ce53  MOVAPS xmmword ptr [0x1478c3f80],XMM1
141d2ce5a  MOVAPS XMM1,xmmword ptr [RBP + -0x29]
141d2ce5e  MOVAPS xmmword ptr [RBP + -0x9],XMM4
141d2ce62  MOVAPS xmmword ptr [0x1478c3f90],XMM0
141d2ce69  MOVAPS xmmword ptr [0x1478c3fa0],XMM1
141d2ce70  MOVAPS xmmword ptr [RBP + -0x59],XMM5
141d2ce74  MOVAPS xmmword ptr [RBP + -0x49],XMM4
141d2ce78  MOVAPS xmmword ptr [RBP + -0x39],XMM2
141d2ce7c  MOVAPS xmmword ptr [RBP + -0x29],XMM3
141d2ce80  TEST RAX,RAX
141d2ce83  JZ 0x141d2ce9a
141d2ce85  MOV RAX,qword ptr [RBP + -0x9]
141d2ce89  LEA RCX,[RBP + -0x39]
141d2ce8d  TEST RAX,RAX
141d2ce90  CMOVNZ RCX,RAX
141d2ce94  MOV RAX,qword ptr [RCX]
141d2ce97  CALL qword ptr [RAX + 0x10]
141d2ce9a  MOV R8D,0x1
141d2cea0  LEA RDX,[0x145f98c38]
141d2cea7  LEA RCX,[RBP + -0x79]
141d2ceab  CALL 0x142cd49c0
141d2ceb0  MOVAPS XMM5,xmmword ptr [0x1478c3cb0]
141d2ceb7  MOVAPS XMM4,xmmword ptr [0x1478c3cc0]
141d2cebe  MOV RAX,qword ptr [RBP + -0x79]
141d2cec2  MOVAPS XMM2,xmmword ptr [0x1478c3cd0]
141d2cec9  MOVAPS XMM3,xmmword ptr [0x1478c3ce0]
141d2ced0  MOV qword ptr [0x1478c3ca0],RAX
141d2ced7  LEA RAX,[0x141d38860]
141d2cede  MOV qword ptr [RBP + -0x31],RAX
141d2cee2  MOVQ RAX,XMM5
141d2cee7  MOV qword ptr [RBP + -0x49],R13
141d2ceeb  MOVAPS XMM1,xmmword ptr [RBP + -0x49]
141d2ceef  MOV qword ptr [RBP + -0x59],RSI
141d2cef3  MOVAPS XMM0,xmmword ptr [RBP + -0x59]
141d2cef7  MOV qword ptr [RBP + -0x39],RBX
141d2cefb  MOVAPS xmmword ptr [0x1478c3cb0],XMM0
141d2cf02  MOVAPS XMM0,xmmword ptr [RBP + -0x39]
141d2cf06  MOVAPS xmmword ptr [0x1478c3cc0],XMM1
141d2cf0d  MOVAPS XMM1,xmmword ptr [RBP + -0x29]
141d2cf11  MOVAPS xmmword ptr [RBP + -0x9],XMM4
141d2cf15  MOVAPS xmmword ptr [0x1478c3cd0],XMM0
141d2cf1c  MOVAPS xmmword ptr [0x1478c3ce0],XMM1
141d2cf23  MOVAPS xmmword ptr [RBP + -0x59],XMM5
141d2cf27  MOVAPS xmmword ptr [RBP + -0x49],XMM4
141d2cf2b  MOVAPS xmmword ptr [RBP + -0x39],XMM2
141d2cf2f  MOVAPS xmmword ptr [RBP + -0x29],XMM3
141d2cf33  TEST RAX,RAX
141d2cf36  JZ 0x141d2cf4d
141d2cf38  MOV RAX,qword ptr [RBP + -0x9]
141d2cf3c  LEA RCX,[RBP + -0x39]
141d2cf40  TEST RAX,RAX
141d2cf43  CMOVNZ RCX,RAX
141d2cf47  MOV RAX,qword ptr [RCX]
141d2cf4a  CALL qword ptr [RAX + 0x10]
141d2cf4d  MOVAPS XMM5,xmmword ptr [0x1478c3cf0]
141d2cf54  LEA RAX,[0x141d385b0]
141d2cf5b  MOVAPS XMM4,xmmword ptr [0x1478c3d00]
141d2cf62  MOVAPS XMM2,xmmword ptr [0x1478c3d10]
141d2cf69  MOVAPS XMM3,xmmword ptr [0x1478c3d20]
141d2cf70  MOV qword ptr [RBP + -0x31],RAX
141d2cf74  MOVQ RAX,XMM5
141d2cf79  MOV qword ptr [RBP + -0x49],R13
141d2cf7d  MOVAPS XMM1,xmmword ptr [RBP + -0x49]
141d2cf81  MOV qword ptr [RBP + -0x59],RSI
141d2cf85  MOVAPS XMM0,xmmword ptr [RBP + -0x59]
141d2cf89  MOV qword ptr [RBP + -0x39],RBX
141d2cf8d  MOVAPS xmmword ptr [0x1478c3cf0],XMM0
141d2cf94  MOVAPS XMM0,xmmword ptr [RBP + -0x39]
141d2cf98  MOVAPS xmmword ptr [0x1478c3d00],XMM1
141d2cf9f  MOVAPS XMM1,xmmword ptr [RBP + -0x29]
141d2cfa3  MOVAPS xmmword ptr [0x1478c3d10],XMM0
141d2cfaa  MOVAPS xmmword ptr [0x1478c3d20],XMM1
141d2cfb1  MOVAPS xmmword ptr [RBP + -0x9],XMM4
141d2cfb5  MOVAPS xmmword ptr [RBP + -0x59],XMM5
141d2cfb9  MOVAPS xmmword ptr [RBP + -0x49],XMM4
141d2cfbd  MOVAPS xmmword ptr [RBP + -0x39],XMM2
141d2cfc1  MOVAPS xmmword ptr [RBP + -0x29],XMM3
141d2cfc5  TEST RAX,RAX
141d2cfc8  JZ 0x141d2cfdf
141d2cfca  MOV RAX,qword ptr [RBP + -0x9]
141d2cfce  LEA RCX,[RBP + -0x39]
141d2cfd2  TEST RAX,RAX
141d2cfd5  CMOVNZ RCX,RAX
141d2cfd9  MOV RAX,qword ptr [RCX]
141d2cfdc  CALL qword ptr [RAX + 0x10]
141d2cfdf  MOV R8D,0x1
141d2cfe5  LEA RDX,[0x145f98c50]
141d2cfec  LEA RCX,[RBP + -0x79]
141d2cff0  CALL 0x142cd49c0
141d2cff5  MOVAPS XMM5,xmmword ptr [0x1478c3e70]
141d2cffc  MOVAPS XMM4,xmmword ptr [0x1478c3e80]
141d2d003  MOV RAX,qword ptr [RBP + -0x79]
141d2d007  MOVAPS XMM2,xmmword ptr [0x1478c3e90]
141d2d00e  MOVAPS XMM3,xmmword ptr [0x1478c3ea0]
141d2d015  MOV qword ptr [0x1478c3de0],RAX
141d2d01c  LEA RAX,[0x141d38b20]
141d2d023  MOV qword ptr [RBP + -0x31],RAX
141d2d027  MOVQ RAX,XMM5
141d2d02c  MOV qword ptr [RBP + -0x49],R13
141d2d030  MOVAPS XMM1,xmmword ptr [RBP + -0x49]
141d2d034  MOV qword ptr [RBP + -0x59],R15
141d2d038  MOVAPS XMM0,xmmword ptr [RBP + -0x59]
141d2d03c  MOV qword ptr [RBP + -0x39],R14
141d2d040  MOVAPS xmmword ptr [0x1478c3e70],XMM0
141d2d047  MOVAPS XMM0,xmmword ptr [RBP + -0x39]
141d2d04b  MOVAPS xmmword ptr [0x1478c3e80],XMM1
141d2d052  MOVAPS XMM1,xmmword ptr [RBP + -0x29]
141d2d056  MOVAPS xmmword ptr [RBP + -0x9],XMM4
141d2d05a  MOVAPS xmmword ptr [0x1478c3e90],XMM0
141d2d061  MOVAPS xmmword ptr [0x1478c3ea0],XMM1
141d2d068  MOVAPS xmmword ptr [RBP + -0x59],XMM5
141d2d06c  MOVAPS xmmword ptr [RBP + -0x49],XMM4
141d2d070  MOVAPS xmmword ptr [RBP + -0x39],XMM2
141d2d074  MOVAPS xmmword ptr [RBP + -0x29],XMM3
141d2d078  TEST RAX,RAX
141d2d07b  JZ 0x141d2d092
141d2d07d  MOV RAX,qword ptr [RBP + -0x9]
141d2d081  LEA RCX,[RBP + -0x39]
141d2d085  TEST RAX,RAX
141d2d088  CMOVNZ RCX,RAX
141d2d08c  MOV RAX,qword ptr [RCX]
141d2d08f  CALL qword ptr [RAX + 0x10]
141d2d092  MOVAPS XMM5,xmmword ptr [0x1478c3e30]
141d2d099  LEA RAX,[0x141d38900]
141d2d0a0  MOVAPS XMM4,xmmword ptr [0x1478c3e40]
141d2d0a7  MOVAPS XMM2,xmmword ptr [0x1478c3e50]
141d2d0ae  MOVAPS XMM3,xmmword ptr [0x1478c3e60]
141d2d0b5  MOV qword ptr [RBP + -0x31],RAX
141d2d0b9  MOVQ RAX,XMM5
141d2d0be  MOV qword ptr [RBP + -0x49],R13
141d2d0c2  MOVAPS XMM1,xmmword ptr [RBP + -0x49]
141d2d0c6  MOV qword ptr [RBP + -0x59],RSI
141d2d0ca  MOVAPS XMM0,xmmword ptr [RBP + -0x59]
141d2d0ce  MOV qword ptr [RBP + -0x39],RBX
141d2d0d2  MOVAPS xmmword ptr [0x1478c3e30],XMM0
141d2d0d9  MOVAPS XMM0,xmmword ptr [RBP + -0x39]
141d2d0dd  MOVAPS xmmword ptr [0x1478c3e40],XMM1
141d2d0e4  MOVAPS XMM1,xmmword ptr [RBP + -0x29]
141d2d0e8  MOVAPS xmmword ptr [0x1478c3e50],XMM0
141d2d0ef  MOVAPS xmmword ptr [0x1478c3e60],XMM1
141d2d0f6  MOVAPS xmmword ptr [RBP + -0x9],XMM4
141d2d0fa  MOVAPS xmmword ptr [RBP + -0x59],XMM5
141d2d0fe  MOVAPS xmmword ptr [RBP + -0x49],XMM4
141d2d102  MOVAPS xmmword ptr [RBP + -0x39],XMM2
141d2d106  MOVAPS xmmword ptr [RBP + -0x29],XMM3
141d2d10a  TEST RAX,RAX
141d2d10d  JZ 0x141d2d124
141d2d10f  MOV RAX,qword ptr [RBP + -0x9]
141d2d113  LEA RCX,[RBP + -0x39]
141d2d117  TEST RAX,RAX
141d2d11a  CMOVNZ RCX,RAX
141d2d11e  MOV RAX,qword ptr [RCX]
141d2d121  CALL qword ptr [RAX + 0x10]
141d2d124  MOV R8D,0x1
141d2d12a  LEA RDX,[0x145f98c68]
141d2d131  LEA RCX,[RBP + -0x79]
141d2d135  CALL 0x142cd49c0
141d2d13a  MOVAPS XMM5,xmmword ptr [0x1478c40f0]
141d2d141  MOVAPS XMM4,xmmword ptr [0x1478c4100]
141d2d148  MOV RAX,qword ptr [RBP + -0x79]
141d2d14c  MOVAPS XMM2,xmmword ptr [0x1478c4110]
141d2d153  MOVAPS XMM3,xmmword ptr [0x1478c4120]
141d2d15a  MOV qword ptr [0x1478c4060],RAX
141d2d161  LEA RAX,[0x141d38d40]
141d2d168  MOV qword ptr [RBP + -0x31],RAX
141d2d16c  MOVQ RAX,XMM5
141d2d171  MOV qword ptr [RBP + -0x49],R13
141d2d175  MOVAPS XMM1,xmmword ptr [RBP + -0x49]
141d2d179  MOV qword ptr [RBP + -0x59],R15
141d2d17d  MOVAPS XMM0,xmmword ptr [RBP + -0x59]
141d2d181  MOV qword ptr [RBP + -0x39],R14
141d2d185  MOVAPS xmmword ptr [0x1478c40f0],XMM0
141d2d18c  MOVAPS XMM0,xmmword ptr [RBP + -0x39]
141d2d190  MOVAPS xmmword ptr [0x1478c4100],XMM1
141d2d197  MOVAPS XMM1,xmmword ptr [RBP + -0x29]
141d2d19b  MOVAPS xmmword ptr [RBP + -0x9],XMM4
141d2d19f  MOVAPS xmmword ptr [0x1478c4110],XMM0
141d2d1a6  MOVAPS xmmword ptr [0x1478c4120],XMM1
141d2d1ad  MOVAPS xmmword ptr [RBP + -0x59],XMM5
141d2d1b1  MOVAPS xmmword ptr [RBP + -0x49],XMM4
141d2d1b5  MOVAPS xmmword ptr [RBP + -0x39],XMM2
141d2d1b9  MOVAPS xmmword ptr [RBP + -0x29],XMM3
141d2d1bd  TEST RAX,RAX
141d2d1c0  JZ 0x141d2d1d7
141d2d1c2  MOV RAX,qword ptr [RBP + -0x9]
141d2d1c6  LEA RCX,[RBP + -0x39]
141d2d1ca  TEST RAX,RAX
141d2d1cd  CMOVNZ RCX,RAX
141d2d1d1  MOV RAX,qword ptr [RCX]
141d2d1d4  CALL qword ptr [RAX + 0x10]
141d2d1d7  MOVAPS XMM5,xmmword ptr [0x1478c40b0]
141d2d1de  LEA RAX,[0x141d57dc0]
141d2d1e5  MOVAPS XMM4,xmmword ptr [0x1478c40c0]
141d2d1ec  MOVAPS XMM2,xmmword ptr [0x1478c40d0]
141d2d1f3  MOVAPS XMM3,xmmword ptr [0x1478c40e0]
141d2d1fa  MOV qword ptr [RBP + -0x31],RAX
141d2d1fe  MOVQ RAX,XMM5
141d2d203  MOV qword ptr [RBP + -0x49],R13
141d2d207  MOVAPS XMM1,xmmword ptr [RBP + -0x49]
141d2d20b  MOV qword ptr [RBP + -0x59],RSI
141d2d20f  MOVAPS XMM0,xmmword ptr [RBP + -0x59]
141d2d213  MOV qword ptr [RBP + -0x39],RBX
141d2d217  MOVAPS xmmword ptr [0x1478c40b0],XMM0
141d2d21e  MOVAPS XMM0,xmmword ptr [RBP + -0x39]
141d2d222  MOVAPS xmmword ptr [0x1478c40c0],XMM1
141d2d229  MOVAPS XMM1,xmmword ptr [RBP + -0x29]
141d2d22d  MOVAPS xmmword ptr [0x1478c40d0],XMM0
141d2d234  MOVAPS xmmword ptr [0x1478c40e0],XMM1
141d2d23b  MOVAPS xmmword ptr [RBP + -0x9],XMM4
141d2d23f  MOVAPS xmmword ptr [RBP + -0x59],XMM5
141d2d243  MOVAPS xmmword ptr [RBP + -0x49],XMM4
141d2d247  MOVAPS xmmword ptr [RBP + -0x39],XMM2
141d2d24b  MOVAPS xmmword ptr [RBP + -0x29],XMM3
141d2d24f  TEST RAX,RAX
141d2d252  JZ 0x141d2d269
141d2d254  MOV RAX,qword ptr [RBP + -0x9]
141d2d258  LEA RCX,[RBP + -0x39]
141d2d25c  TEST RAX,RAX
141d2d25f  CMOVNZ RCX,RAX
141d2d263  MOV RAX,qword ptr [RCX]
141d2d266  CALL qword ptr [RAX + 0x10]
141d2d269  MOVSS XMM0,dword ptr [0x1478c36cc]
141d2d271  MOVSS dword ptr [RDI + 0x150],XMM0
141d2d279  MOVSS XMM1,dword ptr [0x1478c36d0]
141d2d281  MOVSS dword ptr [RDI + 0x154],XMM1
141d2d289  CMP dword ptr [0x148127494],R13D
141d2d290  MOV dword ptr [0x148127490],R13D
141d2d297  JZ 0x141d2d2a7
141d2d299  XOR EDX,EDX
141d2d29b  LEA RCX,[0x148127488]
141d2d2a2  CALL 0x140694750
141d2d2a7  XOR R8D,R8D
141d2d2aa  XOR ECX,ECX
141d2d2ac  LEA EDX,[R8 + 0x30]
141d2d2b0  CALL 0x142bfd200
141d2d2b5  MOV RSI,RAX
141d2d2b8  LEA RBX,[0x1459bfd68]
141d2d2bf  LEA RAX,[0x1459bfd10]
141d2d2c6  MOV R15D,0x3
141d2d2cc  MOV R12D,R15D
141d2d2cf  TEST RSI,RSI
141d2d2d2  JZ 0x141d2d2fa
141d2d2d4  LEA RCX,[RSI + 0x8]
141d2d2d8  MOV qword ptr [RSI],RAX
141d2d2db  MOV RDX,RDI
141d2d2de  CALL 0x142ee3b30
141d2d2e3  LEA RAX,[0x141d56ae0]
141d2d2ea  MOV qword ptr [RSI + 0x10],RAX
141d2d2ee  CALL 0x142bad740
141d2d2f3  MOV qword ptr [RSI + 0x20],RAX
141d2d2f7  MOV qword ptr [RSI],RBX
141d2d2fa  MOV RBX,R13
141d2d2fd  MOV R14,RSI
141d2d300  TEST RSI,RSI
141d2d303  JZ 0x141d2d40f
141d2d309  CMP dword ptr [0x1478c4204],EBX
141d2d30f  JG 0x141d2d3b9
141d2d315  MOV EAX,dword ptr [0x1478c4200]
141d2d31b  MOV EDX,dword ptr [0x1478c41f8]
141d2d321  DEC EAX
141d2d323  MOV dword ptr [0x1478c4200],EAX
141d2d329  CMP EAX,EDX
141d2d32b  JG 0x141d2d3b9
141d2d331  MOV R12D,EDX
141d2d334  MOV R14D,R13D
141d2d337  TEST EDX,EDX
141d2d339  JLE 0x141d2d38d
141d2d33b  NOP dword ptr [RAX + RAX*0x1]
141d2d340  MOV RCX,qword ptr [0x1478c41f0]
141d2d347  CMP dword ptr [RCX + RBX*0x1 + 0x8],R13D
141d2d34c  JZ 0x141d2d36a
141d2d34e  MOV RCX,qword ptr [RCX + RBX*0x1]
141d2d352  TEST RCX,RCX
141d2d355  JZ 0x141d2d36a
141d2d357  MOV RAX,qword ptr [RCX]
141d2d35a  CALL qword ptr [RAX + 0x20]
141d2d35d  TEST AL,AL
141d2d35f  JNZ 0x141d2d36a
141d2d361  INC R14D
141d2d364  ADD RBX,0x10
141d2d368  JMP 0x141d2d382
141d2d36a  MOV R9B,0x1
141d2d36d  LEA RCX,[0x1478c41f0]
141d2d374  MOV R8D,0x1
141d2d37a  MOV EDX,R14D
141d2d37d  CALL 0x140690780
141d2d382  MOV EDX,dword ptr [0x1478c41f8]
141d2d388  CMP R14D,EDX
141d2d38b  JL 0x141d2d340
141d2d38d  LEA EAX,[RDX + RDX*0x1]
141d2d390  MOV ECX,0x2
141d2d395  CMP EAX,ECX
141d2d397  CMOVLE EAX,ECX
141d2d39a  MOV dword ptr [0x1478c4200],EAX
141d2d3a0  CMP R12D,EAX
141d2d3a3  JLE 0x141d2d3b9
141d2d3a5  CMP dword ptr [0x1478c41fc],EDX
141d2d3ab  JZ 0x141d2d3b9
141d2d3ad  LEA RCX,[0x1478c41f0]
141d2d3b4  CALL 0x1406911b0
141d2d3b9  MOV RAX,qword ptr [RSI]
141d2d3bc  LEA RDX,[RBP + -0x79]
141d2d3c0  MOV RCX,RSI
141d2d3c3  CALL qword ptr [RAX + 0x30]
141d2d3c6  MOVSXD R14,dword ptr [0x1478c41f8]
141d2d3cd  MOV RBX,qword ptr [RAX]
141d2d3d0  LEA EAX,[R14 + 0x1]
141d2d3d4  CMP EAX,dword ptr [0x1478c41fc]
141d2d3da  MOV dword ptr [0x1478c41f8],EAX
141d2d3e0  JLE 0x141d2d3f1
141d2d3e2  MOV EDX,R14D
141d2d3e5  LEA RCX,[0x1478c41f0]
141d2d3ec  CALL 0x140690dd0
141d2d3f1  MOV RAX,R14
141d2d3f4  MOV R12D,R13D
141d2d3f7  SHL RAX,0x4
141d2d3fb  MOV R14,R13
141d2d3fe  ADD RAX,qword ptr [0x1478c41f0]
141d2d405  MOV qword ptr [RAX],RSI
141d2d408  MOV RSI,R13
141d2d40b  MOV dword ptr [RAX + 0x8],R15D
141d2d40f  TEST R12D,R12D
141d2d412  MOV RCX,R14
141d2d415  CMOVZ RCX,R13
141d2d419  TEST RCX,RCX
141d2d41c  JZ 0x141d2d430
141d2d41e  MOV RAX,qword ptr [RCX]
141d2d421  XOR EDX,EDX
141d2d423  CALL qword ptr [RAX + 0x38]
141d2d426  TEST R14,R14
141d2d429  JZ 0x141d2d455
141d2d42b  MOV RCX,R14
141d2d42e  JMP 0x141d2d448
141d2d430  TEST R12D,R12D
141d2d433  JZ 0x141d2d455
141d2d435  TEST RSI,RSI
141d2d438  JZ 0x141d2d462
141d2d43a  MOV RAX,qword ptr [RSI]
141d2d43d  XOR EDX,EDX
141d2d43f  MOV RCX,RSI
141d2d442  CALL qword ptr [RAX + 0x38]
141d2d445  MOV RCX,RSI
141d2d448  XOR R8D,R8D
141d2d44b  XOR EDX,EDX
141d2d44d  CALL 0x142bfd200
141d2d452  MOV RSI,RAX
141d2d455  TEST RSI,RSI
141d2d458  JZ 0x141d2d462
141d2d45a  MOV RCX,RSI
141d2d45d  CALL 0x142bee710
141d2d462  MOV qword ptr [RDI + 0x148],RBX
141d2d469  MOV RSI,qword ptr [0x1481d47f8]
141d2d470  TEST RSI,RSI
141d2d473  JNZ 0x141d2d481
141d2d475  CALL 0x142bcee40
141d2d47a  MOV RSI,qword ptr [0x1481d47f8]
141d2d481  XOR R8D,R8D
141d2d484  MOV qword ptr [RBP + -0x71],R13
141d2d488  XOR ECX,ECX
141d2d48a  LEA EDX,[R8 + 0x30]
141d2d48e  CALL 0x142bfd200
141d2d493  MOV qword ptr [RBP + -0x71],RAX
141d2d497  MOV RBX,RAX
141d2d49a  MOV dword ptr [RBP + -0x69],R15D
141d2d49e  TEST RAX,RAX
141d2d4a1  JZ 0x141d2d4d7
141d2d4a3  LEA RAX,[0x1459bfd10]
141d2d4aa  MOV RDX,RDI
141d2d4ad  LEA RCX,[RBX + 0x8]
141d2d4b1  MOV qword ptr [RBX],RAX
141d2d4b4  CALL 0x142ee3b30
141d2d4b9  LEA RAX,[0x141d427d0]
141d2d4c0  MOV qword ptr [RBX + 0x10],RAX
... (capped at 700)
