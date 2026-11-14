
porting/generated/agents/fort-attribute-data-runtime22/142840460.bin:     file format binary


Disassembly of section .data:

0000000142840460 <.data>:
   142840460:	45 85 c9             	test   %r9d,%r9d
   142840463:	74 5e                	je     0x1428404c3
   142840465:	48 8d 4a 0c          	lea    0xc(%rdx),%rcx
   142840469:	4c 2b c2             	sub    %rdx,%r8
   14284046c:	0f 1f 40 00          	nopl   0x0(%rax)
   142840470:	41 8b 44 08 fc       	mov    -0x4(%r8,%rcx,1),%eax
   142840475:	89 41 fc             	mov    %eax,-0x4(%rcx)
   142840478:	41 8b 04 08          	mov    (%r8,%rcx,1),%eax
   14284047c:	89 01                	mov    %eax,(%rcx)
   14284047e:	41 8b 44 08 04       	mov    0x4(%r8,%rcx,1),%eax
   142840483:	89 41 04             	mov    %eax,0x4(%rcx)
   142840486:	41 8b 44 08 08       	mov    0x8(%r8,%rcx,1),%eax
   14284048b:	89 41 08             	mov    %eax,0x8(%rcx)
   14284048e:	41 0f b6 44 08 0c    	movzbl 0xc(%r8,%rcx,1),%eax
   142840494:	88 41 0c             	mov    %al,0xc(%rcx)
   142840497:	41 0f b6 44 08 0d    	movzbl 0xd(%r8,%rcx,1),%eax
   14284049d:	88 41 0d             	mov    %al,0xd(%rcx)
   1428404a0:	41 0f b6 44 08 0e    	movzbl 0xe(%r8,%rcx,1),%eax
   1428404a6:	88 41 0e             	mov    %al,0xe(%rcx)
   1428404a9:	41 8b 44 08 10       	mov    0x10(%r8,%rcx,1),%eax
   1428404ae:	89 41 10             	mov    %eax,0x10(%rcx)
   1428404b1:	41 8b 44 08 14       	mov    0x14(%r8,%rcx,1),%eax
   1428404b6:	89 41 14             	mov    %eax,0x14(%rcx)
   1428404b9:	48 8d 49 28          	lea    0x28(%rcx),%rcx
   1428404bd:	41 83 e9 01          	sub    $0x1,%r9d
   1428404c1:	75 ad                	jne    0x142840470
   1428404c3:	b0 01                	mov    $0x1,%al
   1428404c5:	c3                   	ret
