
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined8 FUN_141c7d270(longlong param_1,longlong param_2)

{
  bool bVar1;
  char cVar2;
  char cVar3;
  undefined8 uVar4;
  longlong *plVar5;
  longlong lVar6;
  longlong lVar7;
  undefined8 uStack_28;
  undefined4 uStack_20;
  undefined8 uStack_18;
  undefined4 uStack_10;
  
  if (param_2 != 0) {
    if ((*(longlong **)(param_2 + 0x250) == (longlong *)0x0) ||
       (cVar2 = (**(code **)(**(longlong **)(param_2 + 0x250) + 0x28))(), cVar2 == '\0')) {
      bVar1 = false;
    }
    else {
      bVar1 = true;
    }
    if (bVar1) {
      lVar6 = *(longlong *)(param_1 + 0x108);
      lVar7 = (longlong)*(int *)(param_1 + 0x110) * 0x40 + lVar6;
      if (lVar6 != lVar7) {
        plVar5 = (longlong *)(lVar6 + 0x18);
        do {
          if (((longlong *)*plVar5 == (longlong *)0x0) ||
             (cVar2 = (**(code **)(*(longlong *)*plVar5 + 0x28))(), cVar2 == '\0')) {
            cVar2 = '\0';
          }
          else {
            cVar2 = '\x01';
          }
          if ((*(longlong **)(param_2 + 0x250) == (longlong *)0x0) ||
             (cVar3 = (**(code **)(**(longlong **)(param_2 + 0x250) + 0x28))(), cVar3 == '\0')) {
            cVar3 = '\0';
          }
          else {
            cVar3 = '\x01';
          }
          if ((cVar2 == cVar3) &&
             ((cVar2 == '\0' ||
              (cVar2 = (*(code *)**(undefined8 **)*plVar5)
                                 ((undefined8 *)*plVar5,*(undefined8 *)(param_2 + 0x250)),
              cVar2 != '\0')))) {
            bVar1 = true;
          }
          else {
            bVar1 = false;
          }
          if (bVar1) {
            return *(undefined8 *)(lVar6 + 0x38);
          }
          lVar6 = lVar6 + 0x40;
          plVar5 = plVar5 + 8;
        } while (lVar6 != lVar7);
      }
    }
    else {
      lVar6 = *(longlong *)(param_2 + 0x278);
      if (lVar6 != 0) {
        lVar7 = FUN_142a174f0();
        if (((*(int *)(lVar7 + 0x38) <= *(int *)(*(longlong *)(lVar6 + 0x10) + 0x38)) &&
            (*(longlong *)
              (*(longlong *)(*(longlong *)(lVar6 + 0x10) + 0x30) +
              (longlong)*(int *)(lVar7 + 0x38) * 8) == lVar7 + 0x30)) &&
           (lVar7 = FUN_141c74820(lVar6), lVar7 != 0)) {
          lVar6 = *(longlong *)(lVar6 + 0x130);
          if (lVar6 == 0) {
            uStack_28 = _DAT_148218630;
            uStack_20 = _DAT_148218638;
          }
          else {
            uStack_28 = *(undefined8 *)(lVar6 + 0x1d0);
            uStack_20 = *(undefined4 *)(lVar6 + 0x1d8);
          }
          uStack_18 = uStack_28;
          uStack_10 = uStack_20;
          uVar4 = FUN_141c7cde0(lVar7,&uStack_18);
          return uVar4;
        }
      }
    }
  }
  return 0;
}

