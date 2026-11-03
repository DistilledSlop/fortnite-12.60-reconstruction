
void FUN_141c8cfd0(longlong param_1,longlong *param_2,longlong *param_3)

{
  bool bVar1;
  char cVar2;
  undefined8 uVar3;
  longlong lVar4;
  longlong lVar5;
  int *piVar6;
  undefined8 uStackX_8;
  undefined8 uStackX_20;
  
  if ((*(char *)(param_1 + 0xf0) == '\x03') && (param_3 != (longlong *)0x0)) {
    uVar3 = FUN_142b6ac80();
    lVar4 = (**(code **)(*param_3 + 0x610))(param_3,uVar3);
    if (((char)param_3[0x81] == '\x01') || ((*(byte *)(param_3 + 0x5b) & 1) != 0)) {
      bVar1 = true;
    }
    else {
      bVar1 = false;
    }
    if ((param_2 == (longlong *)0x0) ||
       (lVar5 = (**(code **)(*param_2 + 0x150))(param_2), lVar5 == 0)) {
      if (!bVar1) {
        return;
      }
    }
    else if (!bVar1) {
      FUN_142ee3b30(&uStackX_8,param_2);
      uStackX_20 = uStackX_8;
      piVar6 = (int *)FUN_140782160(param_3 + 99,&uStackX_8,&uStackX_20);
      if (*piVar6 == -1) {
        return;
      }
      cVar2 = FUN_141c90870(param_2);
      if (cVar2 == '\0') {
        return;
      }
      if (lVar4 == 0) {
        return;
      }
      FUN_142068fd0(lVar4,param_2);
      return;
    }
    if ((lVar4 != 0) && (cVar2 = FUN_141c662c0(param_2), cVar2 != '\0')) {
      FUN_142063d20(lVar4,param_2);
    }
  }
  return;
}

