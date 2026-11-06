
void FUN_140687d00(uint *param_1)

{
  uint uVar1;
  int iVar2;
  ulonglong uVar3;
  uint uVar4;
  longlong lVar5;
  
  lVar5 = *(longlong *)(param_1 + 2);
  uVar3 = (ulonglong)(int)*param_1;
  uVar1 = *(uint *)(lVar5 + 0x18);
  if (*(longlong *)(lVar5 + 0x10) != 0) {
    lVar5 = *(longlong *)(lVar5 + 0x10);
  }
  uVar4 = *(uint *)(lVar5 + uVar3 * 4) & param_1[4];
  while( true ) {
    if (uVar4 != 0) {
      uVar4 = -uVar4 & uVar4;
      iVar2 = 0x1f;
      if (uVar4 != 0) {
        for (; uVar4 >> iVar2 == 0; iVar2 = iVar2 + -1) {
        }
      }
      param_1[1] = uVar4;
      if (uVar4 == 0) {
        iVar2 = 0x20;
      }
      else {
        iVar2 = 0x1f - iVar2;
      }
      uVar4 = (param_1[6] - iVar2) + 0x1f;
      if ((int)uVar1 < (int)uVar4) {
        uVar4 = uVar1;
      }
      param_1[5] = uVar4;
      return;
    }
    param_1[6] = param_1[6] + 0x20;
    uVar4 = (int)uVar3 + 1;
    uVar3 = (ulonglong)uVar4;
    *param_1 = uVar4;
    if ((int)(((int)(uVar1 - 1) >> 0x1f & 0x1fU) + (uVar1 - 1)) >> 5 < (int)uVar4) break;
    uVar4 = *(uint *)(lVar5 + (longlong)(int)uVar4 * 4);
    param_1[4] = 0xffffffff;
  }
  param_1[5] = uVar1;
  return;
}

