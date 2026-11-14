
porting/generated/agents/fort-attribute-data-runtime22/141bc47e0.bin:     file format binary


Disassembly of section .data:

0000000141bc47e0 <.data>:
   141bc47e0:	0f 2e 49 1c          	ucomiss 0x1c(%rcx),%xmm1
   141bc47e4:	74 38                	je     0x141bc481e
   141bc47e6:	80 79 1a 00          	cmpb   $0x0,0x1a(%rcx)
   141bc47ea:	f3 0f 11 49 1c       	movss  %xmm1,0x1c(%rcx)
   141bc47ef:	74 15                	je     0x141bc4806
   141bc47f1:	f3 0f 10 41 10       	movss  0x10(%rcx),%xmm0
   141bc47f6:	0f 2f c8             	comiss %xmm0,%xmm1
   141bc47f9:	72 0e                	jb     0x141bc4809
   141bc47fb:	f3 0f 10 41 14       	movss  0x14(%rcx),%xmm0
   141bc4800:	f3 0f 5d c1          	minss  %xmm1,%xmm0
   141bc4804:	eb 03                	jmp    0x141bc4809
   141bc4806:	0f 28 c1             	movaps %xmm1,%xmm0
   141bc4809:	0f 2e c1             	ucomiss %xmm1,%xmm0
   141bc480c:	f3 0f 11 41 08       	movss  %xmm0,0x8(%rcx)
   141bc4811:	74 06                	je     0x141bc4819
   141bc4813:	b0 01                	mov    $0x1,%al
   141bc4815:	88 41 19             	mov    %al,0x19(%rcx)
   141bc4818:	c3                   	ret
   141bc4819:	32 c0                	xor    %al,%al
   141bc481b:	88 41 19             	mov    %al,0x19(%rcx)
   141bc481e:	c3                   	ret
