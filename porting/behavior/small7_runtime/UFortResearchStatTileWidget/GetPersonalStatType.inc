// Original body address: 0x1450e94e0
// Offset evidence: MOV RAX,[RCX+0x230] is reflected PersonalStatItemDefinition (Object). Null returns 8 (MOV AL,8). Else MOVZX byte at pointee+0x388, pointee class not reflected, read raw.
// CONFIDENCE: MEDIUM
if (PersonalStatItemDefinition == nullptr) { return static_cast<EFortStatType>(8); }
const uint8* Definition = reinterpret_cast<const uint8*>(PersonalStatItemDefinition);
return static_cast<EFortStatType>(Definition[0x388]);
