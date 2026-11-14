0000000141b8bd50: 48895c2408               mov qword ptr [rsp + 8], rbx
0000000141b8bd55: 4889742410               mov qword ptr [rsp + 0x10], rsi
0000000141b8bd5a: 48897c2418               mov qword ptr [rsp + 0x18], rdi
0000000141b8bd5f: 4c89742420               mov qword ptr [rsp + 0x20], r14
0000000141b8bd64: 4157                     push r15
0000000141b8bd66: 4883ec30                 sub rsp, 0x30
0000000141b8bd6a: 0f29742420               movaps xmmword ptr [rsp + 0x20], xmm6
0000000141b8bd6f: 4c8bf9                   mov r15, rcx
0000000141b8bd72: e8991eb6fe               call 0x1406edc10
0000000141b8bd77: 33c9                     xor ecx, ecx
0000000141b8bd79: 4d8db720010000           lea r14, [r15 + 0x120]
0000000141b8bd80: 498db770010000           lea rsi, [r15 + 0x170]
0000000141b8bd87: 498dbf98010000           lea rdi, [r15 + 0x198]
0000000141b8bd8e: 488d05332c3704           lea rax, [rip + 0x4372c33]
0000000141b8bd95: 498907                   mov qword ptr [r15], rax
0000000141b8bd98: 498d9fc0010000           lea rbx, [r15 + 0x1c0]
0000000141b8bd9f: 488d051a553604           lea rax, [rip + 0x436551a]
0000000141b8bda6: 49894f38                 mov qword ptr [r15 + 0x38], rcx
0000000141b8bdaa: 49894730                 mov qword ptr [r15 + 0x30], rax
0000000141b8bdae: 49894758                 mov qword ptr [r15 + 0x58], rax
0000000141b8bdb2: 49898780000000           mov qword ptr [r15 + 0x80], rax
0000000141b8bdb9: 498987a8000000           mov qword ptr [r15 + 0xa8], rax
0000000141b8bdc0: 498987d0000000           mov qword ptr [r15 + 0xd0], rax
0000000141b8bdc7: 498987f8000000           mov qword ptr [r15 + 0xf8], rax
0000000141b8bdce: 498906                   mov qword ptr [r14], rax
0000000141b8bdd1: 49898748010000           mov qword ptr [r15 + 0x148], rax
0000000141b8bdd8: 488906                   mov qword ptr [rsi], rax
0000000141b8bddb: 488907                   mov qword ptr [rdi], rax
0000000141b8bdde: 41c74740ffff7fff         mov dword ptr [r15 + 0x40], 0xff7fffff
0000000141b8bde6: 41c74744ffff7f7f         mov dword ptr [r15 + 0x44], 0x7f7fffff
0000000141b8bdee: 6641894f48               mov word ptr [r15 + 0x48], cx
0000000141b8bdf3: 41884f4a                 mov byte ptr [r15 + 0x4a], cl
0000000141b8bdf7: 49894f4c                 mov qword ptr [r15 + 0x4c], rcx
0000000141b8bdfb: 49894f60                 mov qword ptr [r15 + 0x60], rcx
0000000141b8bdff: 41c74768ffff7fff         mov dword ptr [r15 + 0x68], 0xff7fffff
0000000141b8be07: 41c7476cffff7f7f         mov dword ptr [r15 + 0x6c], 0x7f7fffff
0000000141b8be0f: 6641894f70               mov word ptr [r15 + 0x70], cx
0000000141b8be14: 41884f72                 mov byte ptr [r15 + 0x72], cl
0000000141b8be18: 49894f74                 mov qword ptr [r15 + 0x74], rcx
0000000141b8be1c: 49898f88000000           mov qword ptr [r15 + 0x88], rcx
0000000141b8be23: 41c78790000000ffff7fff   mov dword ptr [r15 + 0x90], 0xff7fffff
0000000141b8be2e: 41c78794000000ffff7f7f   mov dword ptr [r15 + 0x94], 0x7f7fffff
0000000141b8be39: 6641898f98000000         mov word ptr [r15 + 0x98], cx
0000000141b8be41: 41888f9a000000           mov byte ptr [r15 + 0x9a], cl
0000000141b8be48: 49898f9c000000           mov qword ptr [r15 + 0x9c], rcx
0000000141b8be4f: 49898fb0000000           mov qword ptr [r15 + 0xb0], rcx
0000000141b8be56: 41c787b8000000ffff7fff   mov dword ptr [r15 + 0xb8], 0xff7fffff
0000000141b8be61: 41c787bc000000ffff7f7f   mov dword ptr [r15 + 0xbc], 0x7f7fffff
0000000141b8be6c: 6641898fc0000000         mov word ptr [r15 + 0xc0], cx
0000000141b8be74: 41888fc2000000           mov byte ptr [r15 + 0xc2], cl
0000000141b8be7b: 49898fc4000000           mov qword ptr [r15 + 0xc4], rcx
0000000141b8be82: 49898fd8000000           mov qword ptr [r15 + 0xd8], rcx
0000000141b8be89: 41c787e0000000ffff7fff   mov dword ptr [r15 + 0xe0], 0xff7fffff
0000000141b8be94: 41c787e4000000ffff7f7f   mov dword ptr [r15 + 0xe4], 0x7f7fffff
0000000141b8be9f: 6641898fe8000000         mov word ptr [r15 + 0xe8], cx
0000000141b8bea7: 41888fea000000           mov byte ptr [r15 + 0xea], cl
0000000141b8beae: 49898fec000000           mov qword ptr [r15 + 0xec], rcx
0000000141b8beb5: 49898f00010000           mov qword ptr [r15 + 0x100], rcx
0000000141b8bebc: 41c78708010000ffff7fff   mov dword ptr [r15 + 0x108], 0xff7fffff
0000000141b8bec7: 41c7870c010000ffff7f7f   mov dword ptr [r15 + 0x10c], 0x7f7fffff
0000000141b8bed2: 6641898f10010000         mov word ptr [r15 + 0x110], cx
0000000141b8beda: 41888f12010000           mov byte ptr [r15 + 0x112], cl
0000000141b8bee1: 49898f14010000           mov qword ptr [r15 + 0x114], rcx
0000000141b8bee8: 49894e08                 mov qword ptr [r14 + 8], rcx
0000000141b8beec: 41c74610ffff7fff         mov dword ptr [r14 + 0x10], 0xff7fffff
0000000141b8bef4: 41c74614ffff7f7f         mov dword ptr [r14 + 0x14], 0x7f7fffff
0000000141b8befc: 6641894e18               mov word ptr [r14 + 0x18], cx
0000000141b8bf01: 41884e1a                 mov byte ptr [r14 + 0x1a], cl
0000000141b8bf05: 49894e1c                 mov qword ptr [r14 + 0x1c], rcx
0000000141b8bf09: 49898f50010000           mov qword ptr [r15 + 0x150], rcx
0000000141b8bf10: 41c78758010000ffff7fff   mov dword ptr [r15 + 0x158], 0xff7fffff
0000000141b8bf1b: 41c7875c010000ffff7f7f   mov dword ptr [r15 + 0x15c], 0x7f7fffff
0000000141b8bf26: 6641898f60010000         mov word ptr [r15 + 0x160], cx
0000000141b8bf2e: 41888f62010000           mov byte ptr [r15 + 0x162], cl
0000000141b8bf35: 49898f64010000           mov qword ptr [r15 + 0x164], rcx
0000000141b8bf3c: 48894e08                 mov qword ptr [rsi + 8], rcx
0000000141b8bf40: c74610ffff7fff           mov dword ptr [rsi + 0x10], 0xff7fffff
0000000141b8bf47: c74614ffff7f7f           mov dword ptr [rsi + 0x14], 0x7f7fffff
0000000141b8bf4e: 66894e18                 mov word ptr [rsi + 0x18], cx
0000000141b8bf52: 884e1a                   mov byte ptr [rsi + 0x1a], cl
0000000141b8bf55: 48894e1c                 mov qword ptr [rsi + 0x1c], rcx
0000000141b8bf59: 48894f08                 mov qword ptr [rdi + 8], rcx
0000000141b8bf5d: c74710ffff7fff           mov dword ptr [rdi + 0x10], 0xff7fffff
0000000141b8bf64: c74714ffff7f7f           mov dword ptr [rdi + 0x14], 0x7f7fffff
0000000141b8bf6b: 66894f18                 mov word ptr [rdi + 0x18], cx
0000000141b8bf6f: 884f1a                   mov byte ptr [rdi + 0x1a], cl
0000000141b8bf72: 48894f1c                 mov qword ptr [rdi + 0x1c], rcx
0000000141b8bf76: f30f10357ac7e103         movss xmm6, dword ptr [rip + 0x3e1c77a]
0000000141b8bf7e: 48894b08                 mov qword ptr [rbx + 8], rcx
0000000141b8bf82: 0f28ce                   movaps xmm1, xmm6
0000000141b8bf85: 66894b18                 mov word ptr [rbx + 0x18], cx
0000000141b8bf89: 884b1a                   mov byte ptr [rbx + 0x1a], cl
0000000141b8bf8c: 48894b1c                 mov qword ptr [rbx + 0x1c], rcx
0000000141b8bf90: 498bce                   mov rcx, r14
0000000141b8bf93: 488903                   mov qword ptr [rbx], rax
0000000141b8bf96: c74310ffff7fff           mov dword ptr [rbx + 0x10], 0xff7fffff
0000000141b8bf9d: c74314ffff7f7f           mov dword ptr [rbx + 0x14], 0x7f7fffff
0000000141b8bfa4: e837880300               call 0x141bc47e0
0000000141b8bfa9: 498b06                   mov rax, qword ptr [r14]
0000000141b8bfac: 0f28ce                   movaps xmm1, xmm6
0000000141b8bfaf: 498bce                   mov rcx, r14
0000000141b8bfb2: ff5008                   call qword ptr [rax + 8]
0000000141b8bfb5: 488b06                   mov rax, qword ptr [rsi]
0000000141b8bfb8: 0f28ce                   movaps xmm1, xmm6
0000000141b8bfbb: 488bce                   mov rcx, rsi
0000000141b8bfbe: ff5010                   call qword ptr [rax + 0x10]
0000000141b8bfc1: 488b06                   mov rax, qword ptr [rsi]
0000000141b8bfc4: 0f28ce                   movaps xmm1, xmm6
0000000141b8bfc7: 488bce                   mov rcx, rsi
0000000141b8bfca: ff5008                   call qword ptr [rax + 8]
0000000141b8bfcd: 488b07                   mov rax, qword ptr [rdi]
0000000141b8bfd0: 0f28ce                   movaps xmm1, xmm6
0000000141b8bfd3: 488bcf                   mov rcx, rdi
0000000141b8bfd6: ff5010                   call qword ptr [rax + 0x10]
0000000141b8bfd9: 488b07                   mov rax, qword ptr [rdi]
0000000141b8bfdc: 0f28ce                   movaps xmm1, xmm6
0000000141b8bfdf: 488bcf                   mov rcx, rdi
0000000141b8bfe2: ff5008                   call qword ptr [rax + 8]
0000000141b8bfe5: 488b03                   mov rax, qword ptr [rbx]
0000000141b8bfe8: 0f28ce                   movaps xmm1, xmm6
0000000141b8bfeb: 488bcb                   mov rcx, rbx
0000000141b8bfee: ff5010                   call qword ptr [rax + 0x10]
0000000141b8bff1: 488b03                   mov rax, qword ptr [rbx]
0000000141b8bff4: 0f28ce                   movaps xmm1, xmm6
0000000141b8bff7: 488bcb                   mov rcx, rbx
0000000141b8bffa: ff5008                   call qword ptr [rax + 8]
0000000141b8bffd: 488b5c2440               mov rbx, qword ptr [rsp + 0x40]
0000000141b8c002: 498bc7                   mov rax, r15
0000000141b8c005: 488b742448               mov rsi, qword ptr [rsp + 0x48]
0000000141b8c00a: 488b7c2450               mov rdi, qword ptr [rsp + 0x50]
0000000141b8c00f: 4c8b742458               mov r14, qword ptr [rsp + 0x58]
0000000141b8c014: 0f28742420               movaps xmm6, xmmword ptr [rsp + 0x20]
0000000141b8c019: 4883c430                 add rsp, 0x30
0000000141b8c01d: 415f                     pop r15
0000000141b8c01f: c3                       ret
