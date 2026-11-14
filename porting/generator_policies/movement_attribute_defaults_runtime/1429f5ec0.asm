00000001429f5ec0: 4c8bdc                   mov r11, rsp
00000001429f5ec3: 4881ec88000000           sub rsp, 0x88
00000001429f5eca: 488b05a7487b05           mov rax, qword ptr [rip + 0x57b48a7]
00000001429f5ed1: 4885c0                   test rax, rax
00000001429f5ed4: 0f8590000000             jne 0x1429f5f6a
00000001429f5eda: 33c9                     xor ecx, ecx
00000001429f5edc: 488d05fd07cafd           lea rax, [rip - 0x235f803]
00000001429f5ee3: 49894bf0                 mov qword ptr [r11 - 0x10], rcx
00000001429f5ee7: 4c8d0d12100000           lea r9, [rip + 0x1012]
00000001429f5eee: 884c2470                 mov byte ptr [rsp + 0x70], cl
00000001429f5ef2: 4c8d057f487b05           lea r8, [rip + 0x57b487f]
00000001429f5ef9: 498943e0                 mov qword ptr [r11 - 0x20], rax
00000001429f5efd: 488d150e3ba703           lea rdx, [rip + 0x3a73b0e]
00000001429f5f04: 488d05f575d9ff           lea rax, [rip - 0x268a0b]
00000001429f5f0b: 498943d8                 mov qword ptr [r11 - 0x28], rax
00000001429f5f0f: 488d05aaa2c8fd           lea rax, [rip - 0x2375d56]
00000001429f5f16: 498943d0                 mov qword ptr [r11 - 0x30], rax
00000001429f5f1a: 488d05dfa3c8fd           lea rax, [rip - 0x2375c21]
00000001429f5f21: 498943c8                 mov qword ptr [r11 - 0x38], rax
00000001429f5f25: 488d05a4080000           lea rax, [rip + 0x8a4]
00000001429f5f2c: 498943c0                 mov qword ptr [r11 - 0x40], rax
00000001429f5f30: 488d05d1ecfa02           lea rax, [rip + 0x2faecd1]
00000001429f5f37: 498943b8                 mov qword ptr [r11 - 0x48], rax
00000001429f5f3b: 49894bb0                 mov qword ptr [r11 - 0x50], rcx
00000001429f5f3f: 488d0d62cf8303           lea rcx, [rip + 0x383cf62]
00000001429f5f46: c744243000000010         mov dword ptr [rsp + 0x30], 0x10000000
00000001429f5f4e: c744242808000000         mov dword ptr [rsp + 0x28], 8
00000001429f5f56: c7442420e8010000         mov dword ptr [rsp + 0x20], 0x1e8
00000001429f5f5e: e84d393700               call 0x142d698b0
00000001429f5f63: 488b050e487b05           mov rax, qword ptr [rip + 0x57b480e]
00000001429f5f6a: 4881c488000000           add rsp, 0x88
00000001429f5f71: c3                       ret
