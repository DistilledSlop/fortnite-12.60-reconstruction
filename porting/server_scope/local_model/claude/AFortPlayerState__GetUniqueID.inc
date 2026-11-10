// CONFIDENCE: HIGH
// Original: exec thunk 0x142a89170. Return-only function (ReturnValue, Struct).
// The thunk builds a temporary FUniqueNetIdRepl with native 0x142419ef0 (dep: deps/AFortPlayerState__GetUniqueID__BuildUniqueNetIdRepl.json),
// then copy-assigns it into the return slot with native 0x140d7c4c0 (dep: deps/AFortPlayerState__GetUniqueID__UniqueNetIdReplCopyAssign.json).
FUniqueNetIdRepl Id = BuildUniqueNetIdRepl(this);
return Id;
