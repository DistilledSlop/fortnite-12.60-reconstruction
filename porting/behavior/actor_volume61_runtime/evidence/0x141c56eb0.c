
ulonglong FUN_141c56eb0(longlong param_1)

{
  int iVar1;
  longlong lVar2;
  ulonglong uVar3;
  
  lVar2 = FUN_1447ae380();
  iVar1 = *(int *)(lVar2 + 0x38);
  uVar3 = (longlong)iVar1;
  if ((*(int *)(*(longlong *)(param_1 + 0x10) + 0x38) < iVar1) ||
     (uVar3 = *(ulonglong *)(*(longlong *)(param_1 + 0x10) + 0x30),
     *(longlong *)(uVar3 + (longlong)iVar1 * 8) != lVar2 + 0x30)) {
    return uVar3 & 0xffffffffffffff00;
  }
  return CONCAT71((int7)(uVar3 >> 8),1);
}

