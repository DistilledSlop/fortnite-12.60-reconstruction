
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_141c64450(longlong *param_1,longlong param_2,char param_3)

{
  longlong *plVar1;
  int iVar2;
  int *piVar3;
  FName FVar4;
  bool bVar5;
  byte bVar6;
  char cVar7;
  undefined1 uVar8;
  undefined8 uVar9;
  longlong lVar10;
  longlong lVar11;
  UObject *pUVar12;
  longlong lVar13;
  longlong *plVar14;
  undefined8 *puVar15;
  int *piVar16;
  longlong unaff_GS_OFFSET;
  FName FStackX_8;
  undefined8 uStack_58;
  FName FStack_50;
  undefined8 uStack_48;
  FName FStack_40;
  
  cVar7 = '\x01';
  uVar9 = FUN_1429da540();
  lVar10 = (**(code **)(*param_1 + 0x610))(param_1,uVar9);
  if ((lVar10 != 0) && (*(longlong *)(lVar10 + 0xd0) != 0)) {
    cVar7 = *(char *)(*(longlong *)(lVar10 + 0xd0) + 0x688);
  }
  if (param_3 == '\0') {
    lVar11 = wcsstr(L"&AFortVolume::RemoveActorWhenEndPlay",&DAT_1459a98c4);
    lVar10 = 0;
    while (lVar13 = lVar11, lVar13 != 0) {
      lVar11 = wcsstr(lVar13 + 2,&DAT_1459a98c4);
      lVar10 = lVar13;
    }
    FUN_142cd4ab0(&FStackX_8,lVar10 + 4,1);
    FVar4 = FStackX_8;
    if (*(char *)(param_2 + 0x187) != '\0') {
      if ((*(int *)(*(longlong *)
                     (*(longlong *)(unaff_GS_OFFSET + 0x58) + (ulonglong)__tls_index * 8) + 0x248) <
           _DAT_1481232e8) && (FUN_145803a44(&DAT_1481232e8), _DAT_1481232e8 == -1)) {
        FUN_142cd49c0(&DAT_1481232e0,"OnEndPlay",1);
        FUN_1458039e4(&DAT_1481232e8);
      }
      uVar8 = FUN_142ed2f40(param_2,_DAT_1481232e0,param_1,FVar4);
      *(undefined1 *)(param_2 + 0x187) = uVar8;
    }
    if (param_2 != 0) {
      lVar10 = FUN_142732ac0();
      if ((*(int *)(*(longlong *)(param_2 + 0x10) + 0x38) < *(int *)(lVar10 + 0x38)) ||
         (*(longlong *)
           (*(longlong *)(*(longlong *)(param_2 + 0x10) + 0x30) +
           (longlong)*(int *)(lVar10 + 0x38) * 8) != lVar10 + 0x30)) {
        lVar10 = FUN_142a82010();
        if ((*(int *)(lVar10 + 0x38) <= *(int *)(*(longlong *)(param_2 + 0x10) + 0x38)) &&
           (*(longlong *)
             (*(longlong *)(*(longlong *)(param_2 + 0x10) + 0x30) +
             (longlong)*(int *)(lVar10 + 0x38) * 8) == lVar10 + 0x30)) {
          if (cVar7 != '\0') {
            lVar11 = wcsstr(L"&ThisClass::RemoveAthenaPawnWhenDied",&DAT_1459a98c4);
            lVar10 = 0;
            while (lVar13 = lVar11, lVar13 != 0) {
              lVar11 = wcsstr(lVar13 + 2,&DAT_1459a98c4);
              lVar10 = lVar13;
            }
            FUN_142cd4ab0(&FStackX_8,lVar10 + 4,1);
            FUN_1406d80a0(param_2 + 0x2ca8,param_1,FStackX_8);
          }
          FUN_140690620(param_2 + 0x15b8,param_1);
        }
      }
      else {
        lVar11 = wcsstr(L"&AFortVolume::RemoveActorWhenDied",&DAT_1459a98c4);
        lVar10 = 0;
        while (lVar13 = lVar11, lVar13 != 0) {
          lVar11 = wcsstr(lVar13 + 2,&DAT_1459a98c4);
          lVar10 = lVar13;
        }
        FUN_142cd4ab0(&FStackX_8,lVar10 + 4,1);
        FUN_1406d80a0(param_2 + 0x448,param_1,FStackX_8);
      }
    }
  }
  else {
    lVar11 = wcsstr(L"&AFortVolume::RemoveActorWhenEndPlay",&DAT_1459a98c4);
    lVar10 = 0;
    while (lVar13 = lVar11, lVar13 != 0) {
      lVar11 = wcsstr(lVar13 + 2,&DAT_1459a98c4);
      lVar10 = lVar13;
    }
    FUN_142cd4ab0(&FStackX_8,lVar10 + 4,1);
    FVar4 = FStackX_8;
    FUN_142ee3b30(&uStack_58,0);
    FStack_50.ComparisonIndex = 0;
    FStack_50.Number = 0;
    FUN_142ee3b30(&uStack_58);
    FStack_50 = FVar4;
    bVar5 = FName_EqualsEName(FVar4.ComparisonIndex,0);
    if ((FStack_50.Number != 0 || !bVar5) &&
       (pUVar12 = FWeakObjectPtr_Get((undefined *)&uStack_58), pUVar12 != (UObject *)0x0)) {
      UObject_FindFunction(pUVar12,FStack_50);
    }
    FVar4 = FStack_50;
    uVar9 = uStack_58;
    if ((*(int *)(*(longlong *)(*(longlong *)(unaff_GS_OFFSET + 0x58) + (ulonglong)__tls_index * 8)
                 + 0x248) < _DAT_1481232e8) && (FUN_145803a44(&DAT_1481232e8), _DAT_1481232e8 == -1)
       ) {
      FUN_142cd49c0(&DAT_1481232e0,"OnEndPlay",1);
      FUN_1458039e4(&DAT_1481232e8);
    }
    uStack_48 = uVar9;
    FStack_40 = FVar4;
    bVar6 = FUN_142ebfca0(param_2,_DAT_1481232e0,&uStack_48);
    *(byte *)(param_2 + 0x187) = *(byte *)(param_2 + 0x187) | bVar6;
    if (param_2 != 0) {
      lVar10 = FUN_142732ac0();
      if ((*(int *)(*(longlong *)(param_2 + 0x10) + 0x38) < *(int *)(lVar10 + 0x38)) ||
         (*(longlong *)
           (*(longlong *)(*(longlong *)(param_2 + 0x10) + 0x30) +
           (longlong)*(int *)(lVar10 + 0x38) * 8) != lVar10 + 0x30)) {
        lVar10 = FUN_142a82010();
        if ((*(int *)(lVar10 + 0x38) <= *(int *)(*(longlong *)(param_2 + 0x10) + 0x38)) &&
           (*(longlong *)
             (*(longlong *)(*(longlong *)(param_2 + 0x10) + 0x30) +
             (longlong)*(int *)(lVar10 + 0x38) * 8) == lVar10 + 0x30)) {
          if (cVar7 != '\0') {
            lVar11 = wcsstr(L"&ThisClass::RemoveAthenaPawnWhenDied",&DAT_1459a98c4);
            lVar10 = 0;
            while (lVar11 != 0) {
              lVar13 = wcsstr(lVar11 + 2,&DAT_1459a98c4);
              lVar10 = lVar11;
              lVar11 = lVar13;
            }
            FUN_142cd4ab0(&FStackX_8,lVar10 + 4);
            plVar1 = (longlong *)(param_2 + 0x2ca8);
            FUN_142ee3b30(&uStack_58,0);
            FStack_50.ComparisonIndex = 0;
            FStack_50.Number = 0;
            FUN_142ee3b30(&uStack_58);
            FStack_50 = FStackX_8;
            bVar5 = FName_EqualsEName(FStackX_8.ComparisonIndex,0);
            if ((FStack_50.Number != 0 || !bVar5) &&
               (pUVar12 = FWeakObjectPtr_Get((undefined *)&uStack_58), pUVar12 != (UObject *)0x0)) {
              UObject_FindFunction(pUVar12,FStack_50);
            }
            piVar3 = (int *)*plVar1;
            iVar2 = *(int *)(param_2 + 0x2cb0);
            uVar9 = uStack_58;
            for (piVar16 = piVar3; uStack_58 = (code *)uVar9,
                piVar16 != piVar3 + (longlong)iVar2 * 4; piVar16 = piVar16 + 4) {
              uStack_58._0_4_ = (int)uVar9;
              if (((*piVar16 == (int)uStack_58) &&
                  (uStack_58._4_4_ = (int)((ulonglong)uVar9 >> 0x20),
                  bVar5 = piVar16[1] == uStack_58._4_4_, bVar5)) ||
                 ((bVar5 = FWeakObjectPtr_IsValid((undefined *)piVar16), !bVar5 &&
                  (bVar5 = FWeakObjectPtr_IsValid((undefined *)&uStack_58), !bVar5)))) {
                bVar5 = true;
              }
              else {
                bVar5 = false;
              }
              if (bVar5) {
                if (piVar16[3] != FStack_50.Number || piVar16[2] != FStack_50.ComparisonIndex)
                goto LAB_141c6487c;
                bVar5 = true;
              }
              else {
LAB_141c6487c:
                bVar5 = false;
              }
              if (bVar5) {
                if ((int)((longlong)piVar16 - (longlong)piVar3 >> 4) != -1) goto LAB_141c648c5;
                break;
              }
              uVar9 = uStack_58;
            }
            iVar2 = *(int *)(param_2 + 0x2cb0);
            *(int *)(param_2 + 0x2cb0) = iVar2 + 1;
            if (*(int *)(param_2 + 0x2cb4) < iVar2 + 1) {
              FUN_140690dd0(plVar1,iVar2);
            }
            puVar15 = (undefined8 *)(*plVar1 + (longlong)iVar2 * 0x10);
            *puVar15 = uStack_58;
            *(FName *)(puVar15 + 1) = FStack_50;
LAB_141c648c5:
            FUN_1406bd0b0(plVar1);
          }
          plVar1 = (longlong *)(param_2 + 0x15b8);
          cVar7 = FUN_140d97b10(plVar1,param_1);
          if (cVar7 == '\0') {
            FStack_50 = (FName)((ulonglong)FStack_50 & 0xffffffff00000000);
            uStack_58 = FUN_141c5e1d0;
            plVar14 = (longlong *)FMemory_Realloc((undefined *)0x0,0x30,0);
            if (plVar14 != (longlong *)0x0) {
              *plVar14 = (longlong)&DAT_145a88cc8;
              FUN_142ee3b30(plVar14 + 1,param_1);
              *(int *)(plVar14 + 2) = (int)uStack_58;
              *(int *)((longlong)plVar14 + 0x14) = uStack_58._4_4_;
              *(int *)(plVar14 + 3) = FStack_50.ComparisonIndex;
              *(int *)((longlong)plVar14 + 0x1c) = FStack_50.Number;
              lVar10 = FUN_142bad740();
              plVar14[5] = lVar10;
              *plVar14 = (longlong)&DAT_145a88d20;
              FMulticastDelegateBase_CompactInvocationList((undefined *)plVar1,true);
              (**(code **)(*plVar14 + 0x30))(plVar14,&FStackX_8);
              iVar2 = *(int *)(param_2 + 0x15c0);
              *(int *)(param_2 + 0x15c0) = iVar2 + 1;
              if (*(int *)(param_2 + 0x15c4) < iVar2 + 1) {
                FUN_140690dd0(plVar1,iVar2);
              }
              puVar15 = (undefined8 *)((longlong)iVar2 * 0x10 + *plVar1);
              *puVar15 = plVar14;
              *(undefined4 *)(puVar15 + 1) = 3;
            }
          }
        }
      }
      else {
        lVar11 = wcsstr(L"&AFortVolume::RemoveActorWhenDied",&DAT_1459a98c4);
        lVar10 = 0;
        while (lVar11 != 0) {
          lVar13 = wcsstr(lVar11 + 2,&DAT_1459a98c4);
          lVar10 = lVar11;
          lVar11 = lVar13;
        }
        FUN_142cd4ab0(&FStackX_8,lVar10 + 4,1);
        plVar1 = (longlong *)(param_2 + 0x448);
        FUN_142ee3b30(&uStack_58,0);
        FStack_50.ComparisonIndex = 0;
        FStack_50.Number = 0;
        FUN_142ee3b30(&uStack_58);
        FStack_50 = FStackX_8;
        bVar5 = FName_EqualsEName(FStackX_8.ComparisonIndex,0);
        if ((FStack_50.Number != 0 || !bVar5) &&
           (pUVar12 = FWeakObjectPtr_Get((undefined *)&uStack_58), pUVar12 != (UObject *)0x0)) {
          UObject_FindFunction(pUVar12,FStack_50);
        }
        piVar3 = (int *)*plVar1;
        iVar2 = *(int *)(param_2 + 0x450);
        uVar9 = uStack_58;
        for (piVar16 = piVar3; uStack_58 = (code *)uVar9, piVar16 != piVar3 + (longlong)iVar2 * 4;
            piVar16 = piVar16 + 4) {
          uStack_58._0_4_ = (int)uVar9;
          if (((*piVar16 == (int)uStack_58) &&
              (uStack_58._4_4_ = (int)((ulonglong)uVar9 >> 0x20),
              bVar5 = piVar16[1] == uStack_58._4_4_, bVar5)) ||
             ((bVar5 = FWeakObjectPtr_IsValid((undefined *)piVar16), !bVar5 &&
              (bVar5 = FWeakObjectPtr_IsValid((undefined *)&uStack_58), !bVar5)))) {
            bVar5 = true;
          }
          else {
            bVar5 = false;
          }
          if (bVar5) {
            if (piVar16[3] != FStack_50.Number || piVar16[2] != FStack_50.ComparisonIndex)
            goto LAB_141c646dc;
            bVar5 = true;
          }
          else {
LAB_141c646dc:
            bVar5 = false;
          }
          if (bVar5) {
            if ((int)((longlong)piVar16 - (longlong)piVar3 >> 4) != -1) goto LAB_141c64723;
            break;
          }
          uVar9 = uStack_58;
        }
        iVar2 = *(int *)(param_2 + 0x450);
        *(int *)(param_2 + 0x450) = iVar2 + 1;
        if (*(int *)(param_2 + 0x454) < iVar2 + 1) {
          FUN_140690dd0(plVar1,iVar2);
        }
        puVar15 = (undefined8 *)(*plVar1 + (longlong)iVar2 * 0x10);
        *puVar15 = uStack_58;
        *(FName *)(puVar15 + 1) = FStack_50;
LAB_141c64723:
        FUN_1406bd0b0(plVar1);
      }
    }
  }
  return;
}

