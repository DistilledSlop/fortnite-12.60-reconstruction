# Two argument recoveries for OnRep_InVehicle

Applied by root to `pawn_vehicle70_runtime/OnRep_InVehicle.rough.cpp`
whose SHA-256 is recorded in `HANDOFF.md`. The original runtime image is
`/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin`,
base `0x140000000`, previously pinned SHA-256
`338c83a2314ccac1040a2c0bb520b893013cc7b58eda58e550da67f5d84de680`.
Windows below are image bytes decoded with Capstone x86-64. The image was not
rehashed in this pass. The exec stub jumps from `0x142a7ddc5` to `0x14233a160`.

## Log arguments and preservation of driver result

```asm
14233a1b6  0fb69684150000       movzx edx, byte ptr [rsi + 0x1584]
14233a1bd  41ff9068050000       call  qword ptr [r8 + 0x568]
14233a1c4  0fb6d8               movzx ebx, al
14233a1c7  eb02                 jmp   0x14233a1cb
14233a1c9  32db                 xor   bl, bl
14233a234  e887d8a200           call  0x142d67ac0
14233a239  837c247800           cmp   dword ptr [rsp + 0x78], 0
14233a23e  488d0deba86603       lea   rcx, [rip + 0x366a8eb]
14233a245  0fb6c3               movzx eax, bl
14233a24f  480f454c2470         cmovne rcx, qword ptr [rsp + 0x70]
14233a25b  89442430             mov   dword ptr [rsp + 0x30], eax
14233a261  48894c2428           mov   qword ptr [rsp + 0x28], rcx
14233a266  488d05c35cde03       lea   rax, [rip + 0x3de5cc3]
14233a26f  4889442420           mov   qword ptr [rsp + 0x20], rax
14233a274  e8f7a89100           call  0x142c54b70
```

On Windows x64 the two variadic arguments are the selected string pointer and
zero-extended driver-result byte. The branch at `0x14233a1ae` reaches `1c9`
when the interface is null. At `0x14233a23e` the fallback is address
`0x1459a4b30`; the native selection uses **string count**, not pointer-nullness.
The existing draft discards the vehicle virtual's return. Suggested patch:

```cpp
bool driving = false;
if (current) driving = vehicle_v568(current, state_seat(local_state(p)));
// after enum_name_142d67ac0:
log_role_driving(name_count(name) ? name_data(name) : literal_1459a4b30(),
                 driving ? 1 : 0);
```

Model the exact string layout and format/log ABI before canonical integration;
`bool` above assumes the slot's result is normalized. For full byte parity use
`uint8_t` rather than `bool`, since the machine code zero-extends AL unchanged.
The enum-name allocation is still freed after logging as before.

## Old-driving montage call

```asm
14233a34e  33db                 xor   ebx, ebx
14233bb82  ff90b0000000         call  qword ptr [rax + 0xb0]
14233bb88  488bf8               mov   rdi, rax
14233bb8b  4885c0               test  rax, rax
14233bd43  ff9068050000         call  qword ptr [rax + 0x568]
14233bd49  4c8b06               mov   r8, qword ptr [rsi]
14233bd4c  488bd7               mov   rdx, rdi
14233bd4f  f30f1015a1c96603     movss xmm2, dword ptr [rip + 0x366c9a1]
14233bd57  488bce               mov   rcx, rsi
14233bd5a  48895c2460           mov   qword ptr [rsp + 0x60], rbx
14233bd5f  4c8bcb               mov   r9, rbx
14233bd62  84c0                 test  al, al
14233bd64  0f846d010000         je    0x14233bed7
14233bd6a  41ff90b8070000       call  qword ptr [r8 + 0x7b8]
14233bed7  41ff90b8070000       call  qword ptr [r8 + 0x7b8]
```

Constant at `0x1459a86f8` is bytes `00 00 80 3f` (1.0f). RBX is
zeroed at `0x14233a34e` and no subsequent assignment appears in the decoded
function through this window; Windows x64 calls preserve RBX. Both branches
therefore pass pawn in RCX, montage in RDX, 1.0f in XMM2, zero in R9. Replace
`pawn_v7b8_old_driver_unresolved(p)` with `pawn_v7b8(p,montage,1.0f,0)` and
remove its now-unused extern. Both branches share the setup immediately before
the test and branch. This is register-level evidence, not a resolved prototype
or runtime validation.
