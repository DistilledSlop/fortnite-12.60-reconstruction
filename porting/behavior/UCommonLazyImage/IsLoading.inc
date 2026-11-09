// CONFIDENCE: medium
// Original address: 0x140810300
*(ulonglong *)Stack->Code = (ulonglong)(*(longlong *)Stack->Code != 0) + *(longlong *)Stack->Code;
uint8 uVar1 = (uint8)Native_1407cfb50(this);
*(uint8*)Result = uVar1;
