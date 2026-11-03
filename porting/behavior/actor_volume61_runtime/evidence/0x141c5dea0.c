
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_141c5dea0(longlong param_1,longlong param_2)

{
  int iVar1;
  longlong lVar2;
  char cVar3;
  int *piVar4;
  longlong lVar5;
  undefined8 *puVar6;
  longlong *plVar7;
  longlong *plVar8;
  int OldNum;
  longlong *Ptr;
  longlong *plVar9;
  int OldNum_00;
  undefined1 auStackX_10 [8];
  longlong *plStackX_18;
  undefined8 uStackX_20;
  undefined8 uStack_a8;
  undefined8 uStack_a0;
  undefined4 uStack_98;
  undefined4 uStack_90;
  undefined4 uStack_8c;
  undefined4 uStack_88;
  undefined4 uStack_84;
  undefined4 uStack_80;
  undefined4 uStack_7c;
  undefined8 uStack_78;
  undefined4 uStack_70;
  undefined8 uStack_6c;
  undefined4 uStack_64;
  longlong *plStack_60;
  int iStack_58;
  int iStack_54;
  longlong *plStack_50;
  int iStack_48;
  int iStack_44;
  
  if (param_2 != 0) {
    Ptr = (longlong *)0x0;
    plVar8 = (longlong *)0x0;
    plVar9 = *(longlong **)(param_1 + 0x278);
    OldNum = 0;
    OldNum_00 = 0;
    plStack_50 = (longlong *)0x0;
    iStack_44 = 0;
    plStack_60 = (longlong *)0x0;
    plVar7 = plVar9 + *(int *)(param_1 + 0x280);
    iStack_54 = 0;
    plStackX_18 = plVar7;
    if (plVar9 != plVar7) {
      do {
        lVar2 = *plVar9;
        if (lVar2 != 0) {
          FUN_142ee3b30(&uStackX_20,param_2);
          uStack_a8 = uStackX_20;
          piVar4 = (int *)FUN_140782160(lVar2 + 0x318,auStackX_10,&uStack_a8);
          iVar1 = *piVar4;
          lVar5 = *(longlong *)(param_2 + 0x130);
          if (lVar5 == 0) {
            uStack_70 = _DAT_148218638;
            puVar6 = &uStack_78;
            uStack_78 = _DAT_148218630;
          }
          else {
            puVar6 = (undefined8 *)&uStack_90;
            uStack_90 = *(undefined4 *)(lVar5 + 0x1d0);
            uStack_88 = *(undefined4 *)(lVar5 + 0x1d8);
            uStack_8c = *(undefined4 *)(lVar5 + 0x1d4);
          }
          uStack_a0 = *puVar6;
          uStack_98 = *(undefined4 *)(puVar6 + 1);
          lVar5 = FUN_142a75f10();
          if (((*(int *)(lVar5 + 0x38) <= *(int *)(*(longlong *)(param_2 + 0x10) + 0x38)) &&
              (*(longlong *)
                (*(longlong *)(*(longlong *)(param_2 + 0x10) + 0x30) +
                (longlong)*(int *)(lVar5 + 0x38) * 8) == lVar5 + 0x30)) &&
             (lVar5 = FUN_142325510(param_2), lVar5 != 0)) {
            lVar5 = *(longlong *)(lVar5 + 0x130);
            if (lVar5 == 0) {
              uStack_64 = _DAT_148218638;
              puVar6 = &uStack_6c;
              uStack_6c = _DAT_148218630;
            }
            else {
              puVar6 = (undefined8 *)&uStack_84;
              uStack_84 = *(undefined4 *)(lVar5 + 0x1d0);
              uStack_7c = *(undefined4 *)(lVar5 + 0x1d8);
              uStack_80 = *(undefined4 *)(lVar5 + 0x1d4);
            }
            uStack_a0 = *puVar6;
            uStack_98 = *(undefined4 *)(puVar6 + 1);
          }
          cVar3 = FUN_141c82210(lVar2,&uStack_a0);
          if (iVar1 == -1) {
            if (cVar3 != '\0') {
              iStack_48 = OldNum + 1;
              if (iStack_44 < iStack_48) {
                TArray8_ResizeGrow((undefined *)&plStack_50,OldNum);
                Ptr = plStack_50;
              }
              Ptr[OldNum] = lVar2;
              OldNum = iStack_48;
            }
          }
          else if (cVar3 == '\0') {
            iStack_58 = OldNum_00 + 1;
            if (iStack_54 < iStack_58) {
              TArray8_ResizeGrow((undefined *)&plStack_60,OldNum_00);
            }
            plStack_60[OldNum_00] = lVar2;
            OldNum_00 = iStack_58;
          }
          plVar7 = plStackX_18;
          plVar8 = plStack_60;
        }
        plVar9 = plVar9 + 1;
      } while (plVar9 != plVar7);
    }
    plVar7 = plVar8 + OldNum_00;
    for (; plVar8 != plVar7; plVar8 = plVar8 + 1) {
      if (*plVar8 != 0) {
        FUN_141c8abe0(*plVar8,param_2);
      }
    }
    for (plVar7 = Ptr; plVar7 != Ptr + OldNum; plVar7 = plVar7 + 1) {
      if (*plVar7 != 0) {
        lVar2 = *(longlong *)(param_2 + 0x130);
        if (lVar2 == 0) {
          uStack_64 = _DAT_148218638;
          puVar6 = &uStack_6c;
          uStack_6c = _DAT_148218630;
        }
        else {
          puVar6 = (undefined8 *)&uStack_84;
          uStack_84 = *(undefined4 *)(lVar2 + 0x1d0);
          uStack_7c = *(undefined4 *)(lVar2 + 0x1d8);
          uStack_80 = *(undefined4 *)(lVar2 + 0x1d4);
        }
        uStack_78 = *puVar6;
        uStack_70 = *(undefined4 *)(puVar6 + 1);
        FUN_141c5c710(*plVar7,param_2,&uStack_78);
      }
    }
    if (plStack_60 != (longlong *)0x0) {
      FMemory_Free((undefined *)plStack_60);
    }
    if (Ptr != (longlong *)0x0) {
      FMemory_Free((undefined *)Ptr);
    }
  }
  return;
}

