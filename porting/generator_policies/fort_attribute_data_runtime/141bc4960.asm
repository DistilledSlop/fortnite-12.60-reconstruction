
porting/generated/agents/fort-attribute-data-runtime22/141bc4960.bin:     file format binary


Disassembly of section .data:

0000000141bc4960 <.data>:
   141bc4960:	0f 2e 49 20          	ucomiss 0x20(%rcx),%xmm1
   141bc4964:	74 2d                	je     0x141bc4993
   141bc4966:	f3 0f 10 41 10       	movss  0x10(%rcx),%xmm0
   141bc496b:	0f 2f c8             	comiss %xmm0,%xmm1
   141bc496e:	f3 0f 11 49 20       	movss  %xmm1,0x20(%rcx)
   141bc4973:	72 09                	jb     0x141bc497e
   141bc4975:	f3 0f 10 41 14       	movss  0x14(%rcx),%xmm0
   141bc497a:	f3 0f 5d c1          	minss  %xmm1,%xmm0
   141bc497e:	0f 2e c1             	ucomiss %xmm1,%xmm0
   141bc4981:	f3 0f 11 41 0c       	movss  %xmm0,0xc(%rcx)
   141bc4986:	74 06                	je     0x141bc498e
   141bc4988:	b0 01                	mov    $0x1,%al
   141bc498a:	88 41 18             	mov    %al,0x18(%rcx)
   141bc498d:	c3                   	ret
   141bc498e:	32 c0                	xor    %al,%al
   141bc4990:	88 41 18             	mov    %al,0x18(%rcx)
   141bc4993:	c3                   	ret
