
longlong FUN_141f611f0(longlong param_1)

{
  longlong lVar1;
  longlong lVar2;
  longlong lVar3;
  undefined8 uVar4;
  
  lVar1 = FUN_141ef3ab0();
  if (lVar1 == 0) {
    return 0;
  }
  lVar2 = FUN_1429234b0();
  if (*(int *)(*(longlong *)(lVar1 + 0x10) + 0x38) < *(int *)(lVar2 + 0x38)) {
    return 0;
  }
  if (*(longlong *)
       (*(longlong *)(*(longlong *)(lVar1 + 0x10) + 0x30) + (longlong)*(int *)(lVar2 + 0x38) * 8) !=
      lVar2 + 0x30) {
    return 0;
  }
  lVar2 = FUN_141c74820(param_1);
  if (lVar2 != 0) {
    uVar4 = 0;
    if (param_1 != 0) {
      lVar3 = FUN_1446acb00();
      if ((*(int *)(*(longlong *)(param_1 + 0x10) + 0x38) < *(int *)(lVar3 + 0x38)) ||
         (*(longlong *)
           (*(longlong *)(*(longlong *)(param_1 + 0x10) + 0x30) +
           (longlong)*(int *)(lVar3 + 0x38) * 8) != lVar3 + 0x30)) {
        lVar3 = FUN_142a85850();
        if ((*(int *)(lVar3 + 0x38) <= *(int *)(*(longlong *)(param_1 + 0x10) + 0x38)) &&
           (*(longlong *)
             (*(longlong *)(*(longlong *)(param_1 + 0x10) + 0x30) +
             (longlong)*(int *)(lVar3 + 0x38) * 8) == lVar3 + 0x30)) {
          uVar4 = FUN_141c7d420(lVar2,param_1);
        }
      }
      else {
        uVar4 = FUN_141c7c5c0(lVar2,param_1);
      }
    }
    lVar2 = FUN_141f9f270(uVar4);
    if (lVar2 != 0) {
      return lVar2;
    }
  }
  return *(longlong *)(lVar1 + 0x2d0);
}

