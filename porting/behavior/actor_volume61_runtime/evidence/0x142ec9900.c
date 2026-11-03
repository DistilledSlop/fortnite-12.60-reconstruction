
longlong FUN_142ec9900(longlong param_1,longlong param_2)

{
  longlong lVar1;
  longlong lVar2;
  char cVar3;
  int iVar4;
  longlong lVar5;
  longlong lVar6;
  longlong lVar7;
  
  lVar6 = 0;
  if (((param_2 != 0) && ((*(uint *)(param_2 + 0xe4) >> 0xe & 1) != 0)) &&
     (lVar5 = FUN_142e5fb70(), param_2 != lVar5)) {
    lVar5 = *(longlong *)(param_1 + 0x10);
    if ((*(uint *)(param_2 + 0xe4) >> 7 & 1) == 0) {
      cVar3 = FUN_142d6a820(lVar5,param_2);
      if (cVar3 != '\0') {
        return param_1;
      }
    }
    else {
      do {
        if (lVar5 == 0) {
          return lVar6;
        }
        iVar4 = 0;
LAB_142ec9980:
        if ((iVar4 < 0) || (*(int *)(lVar5 + 0x1f8) <= iVar4)) goto LAB_142ec99cf;
        lVar1 = *(longlong *)(lVar5 + 0x1f0);
        lVar7 = (longlong)iVar4;
        if (*(char *)(lVar1 + 0xc + lVar7 * 0x10) != '\0') {
LAB_142ec99c0:
          iVar4 = iVar4 + 1;
          goto LAB_142ec9980;
        }
        lVar2 = *(longlong *)(lVar1 + lVar7 * 0x10);
        if ((*(int *)(lVar2 + 0x38) < *(int *)(param_2 + 0x38)) ||
           (*(longlong *)(*(longlong *)(lVar2 + 0x30) + (longlong)*(int *)(param_2 + 0x38) * 8) !=
            param_2 + 0x30)) goto LAB_142ec99c0;
        lVar6 = *(int *)(lVar1 + 8 + lVar7 * 0x10) + param_1;
LAB_142ec99cf:
        lVar5 = *(longlong *)(lVar5 + 0x40);
      } while (lVar6 == 0);
    }
  }
  return lVar6;
}

