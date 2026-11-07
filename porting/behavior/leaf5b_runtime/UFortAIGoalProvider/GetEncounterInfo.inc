// Thunk 0x14279c3e0: ADD RCX,0x28 (thunk_delta 0x28), then virtual call [RAX+0x10] on the sub-object at this+0x28 (wrapper_delta 0).
// Body 0x1407c1e20: MOV RAX,qword [RCX+0x18]; RET. Object offset = 0x28 + 0x18 = 0x40.
// Covered exactly by reflected member UFortAIGoalProvider::EncounterInfo (UFortAIEncounterInfo*, +0x40; UFortAIGoalProvider.h line 31).
// CONFIDENCE: HIGH
return EncounterInfo;
