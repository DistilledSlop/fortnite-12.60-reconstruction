
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined8 FUN_141c7d420(longlong param_1,longlong param_2)

{
  int iVar1;
  uint uVar2;
  longlong lVar3;
  undefined8 uVar4;
  uint uVar5;
  wchar_t *apwStackX_10 [3];
  
  apwStackX_10[0] = L"STAT_Find_Volume";
  uVar4 = 0;
  if (DAT_1482472e4 != '\0') {
    apwStackX_10[0] = (wchar_t *)0x0;
  }
  if (param_2 != 0) {
    iVar1 = *(int *)(param_2 + 0xc);
    if (iVar1 < _DAT_148263a2c) {
      uVar5 = iVar1 >> 0x1f & 0xffff;
      uVar2 = iVar1 + uVar5;
      lVar3 = *(longlong *)(_DAT_148263a18 + (longlong)((int)uVar2 >> 0x10) * 8) +
              (longlong)(int)((uVar2 & 0xffff) - uVar5) * 0x18;
    }
    else {
      lVar3 = 0;
    }
    if ((*(uint *)(lVar3 + 8) >> 0x1d & 1) == 0) {
      uVar4 = FUN_141c7d270(param_1 + 0x288,param_2);
    }
  }
  if ((DAT_1482472e4 != '\0') && (apwStackX_10[0] != (wchar_t *)0x0)) {
    FUN_142cd3770(apwStackX_10);
  }
  return uVar4;
}

