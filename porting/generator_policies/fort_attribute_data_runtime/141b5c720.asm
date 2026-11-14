
porting/generated/agents/fort-attribute-data-runtime22/141b5c720.bin:     file format binary


Disassembly of section .data:

0000000141b5c720 <.data>:
   141b5c720:	40 53                	rex push %rbx
   141b5c722:	48 83 ec 20          	sub    $0x20,%rsp
   141b5c726:	48 8d 05 83 cf e7 03 	lea    0x3e7cf83(%rip),%rax        # 0x1459d96b0
   141b5c72d:	48 8b d9             	mov    %rcx,%rbx
   141b5c730:	48 89 01             	mov    %rax,(%rcx)
   141b5c733:	f6 c2 01             	test   $0x1,%dl
   141b5c736:	74 0a                	je     0x141b5c742
   141b5c738:	ba 28 00 00 00       	mov    $0x28,%edx
   141b5c73d:	e8 6e 85 82 ff       	call   0x141384cb0
   141b5c742:	48 8b c3             	mov    %rbx,%rax
   141b5c745:	48 83 c4 20          	add    $0x20,%rsp
   141b5c749:	5b                   	pop    %rbx
   141b5c74a:	c3                   	ret
   141b5c74b:	90                   	nop
