// Behaviour snippet: AFortCreativeMoveTool::PopCreativeMoveToolEquippedInputComponent (body only; spliced into the generated member function).
// Original address: 0x1428c3e60
// Decompile source: captures/6c9f7a5abea44e54aba70d7728c6d2fd/0x1428c3e60.c (asm: .../0x1428c3e60.asm)
// Inferred: Native (non-RPC) exec thunk. After the shared FFrame prologue (return-value flag
//   touch at [rdx+0x20]) the whole body is a single virtual call with no arguments:
//     mov rax,[rcx];  call [rax+0xc78]
//   i.e. it invokes the class virtual at vtable byte offset 0xc78.
// The vtable candidate 0x1461de5c0 (native_class_index.json: status "unverified", shared by 3
//   classes) has slot 0xc78 = 0x1406801c0, the documented shared bare-return stub used by 15+
//   empty native bodies (see porting/behavior/README.md third pass). That target is `ret` only,
//   so the virtual returns immediately and the whole function has no observable side effects.
// Declared in generated header: no (absent from Generated/AFortCreativeMoveTool.h).
// CONFIDENCE: MEDIUM