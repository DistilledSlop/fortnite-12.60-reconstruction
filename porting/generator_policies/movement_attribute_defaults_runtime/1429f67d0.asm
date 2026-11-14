00000001429f67d0: 488b01                   mov rax, qword ptr [rcx]
00000001429f67d3: 4885c0                   test rax, rax
00000001429f67d6: 740b                     je 0x1429f67e3
00000001429f67d8: 488bd1                   mov rdx, rcx
00000001429f67db: 488bc8                   mov rcx, rax
00000001429f67de: e96d5519ff               jmp 0x141b8bd50
00000001429f67e3: c3                       ret
