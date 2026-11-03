
void FUN_141c6f610(longlong *param_1,undefined4 param_2)

{
  uint *puVar1;
  undefined8 *puVar2;
  uint uVar3;
  uint uVar4;
  bool bVar5;
  longlong lVar6;
  longlong lVar7;
  UObject *pUVar8;
  undefined *puVar9;
  longlong *plVar10;
  longlong *plVar11;
  undefined4 *puVar12;
  uint uVar13;
  undefined8 *puVar14;
  int iVar15;
  undefined4 uVar16;
  uint *puVar17;
  uint uVar18;
  longlong *plVar19;
  undefined4 uStackX_8;
  undefined4 uStackX_c;
  undefined4 uStackX_10;
  int iStack_a8;
  uint uStack_a4;
  uint uStack_94;
  int iStack_90;
  undefined4 uStack_8c;
  undefined8 uStack_80;
  uint *puStack_78;
  undefined8 uStack_70;
  undefined8 uStack_68;
  longlong *plStack_60;
  undefined8 uStack_58;
  uint *puStack_50;
  undefined8 uStack_48;
  undefined8 uStack_40;
  
  uStackX_10 = param_2;
  if ((char)param_1[0x1e] == '\x03') {
    puVar14 = (undefined8 *)param_1[0x79];
    puVar2 = puVar14 + (int)param_1[0x7a];
    for (; puVar14 != puVar2; puVar14 = puVar14 + 1) {
      FUN_1418dd390(*puVar14);
    }
    lVar6 = (**(code **)(*param_1 + 0x150))(param_1);
    if (lVar6 != 0) {
      if (*(longlong *)(lVar6 + 0x188) == 0) {
        puVar9 = *(undefined **)(lVar6 + 800);
      }
      else {
        puVar9 = *(undefined **)(*(longlong *)(lVar6 + 0x188) + 0xc0);
      }
      lVar7 = FUN_144621fb0(puVar9,param_1 + 0x7f);
      if (lVar7 != 0) {
        FTimerManager_InternalClearTimer(puVar9,param_1[0x7f]);
      }
      param_1[0x7f] = 0;
      if (*(longlong *)(lVar6 + 0x188) == 0) {
        puVar9 = *(undefined **)(lVar6 + 800);
      }
      else {
        puVar9 = *(undefined **)(*(longlong *)(lVar6 + 0x188) + 0xc0);
      }
      lVar6 = FUN_144621fb0(puVar9,param_1 + 0x7e);
      if (lVar6 != 0) {
        FTimerManager_InternalClearTimer(puVar9,param_1[0x7e]);
      }
      param_1[0x7e] = 0;
    }
    plVar10 = param_1 + 99;
    iStack_a8 = 0;
    uVar3 = *(uint *)(param_1 + 0x68);
    puVar1 = (uint *)(param_1 + 0x65);
    uStack_a4 = 1;
    uStack_94 = 0;
    iStack_90 = 0;
    if (uVar3 != 0) {
      puVar17 = puVar1;
      if ((uint *)param_1[0x67] != (uint *)0x0) {
        puVar17 = (uint *)param_1[0x67];
      }
      uVar4 = *puVar17;
      iStack_90 = 0;
      iStack_a8 = 0;
      while (uVar4 == 0) {
        lVar6 = (longlong)iStack_a8;
        iStack_90 = iStack_90 + 0x20;
        iVar15 = iStack_a8 + 1;
        bVar5 = (int)((uVar3 - 1) + ((int)(uVar3 - 1) >> 0x1f & 0x1fU)) >> 5 <= iStack_a8;
        iStack_a8 = iVar15;
        if (bVar5) goto LAB_141c6f7b9;
        uVar4 = puVar17[lVar6 + 1];
      }
      uStack_a4 = uVar4 - 1 & uVar4 ^ uVar4;
      iVar15 = 0x1f;
      if (uStack_a4 != 0) {
        for (; uStack_a4 >> iVar15 == 0; iVar15 = iVar15 + -1) {
        }
      }
      if (uStack_a4 == 0) {
        iVar15 = 0x20;
      }
      else {
        iVar15 = 0x1f - iVar15;
      }
      uStack_94 = (iStack_90 - iVar15) + 0x1f;
      if ((int)uVar3 < (int)uStack_94) {
LAB_141c6f7b9:
        uStack_94 = uVar3;
      }
    }
    uVar4 = *(uint *)(param_1 + 0x68);
    uStack_70 = CONCAT44(uStack_94,0xffffffff);
    uStack_68 = CONCAT44(uStack_8c,iStack_90);
    uStack_80 = CONCAT44(uStack_a4,iStack_a8);
    uVar18 = uVar4 & 0xffffffe0;
    puStack_78 = puVar1;
    plStack_60 = plVar10;
    uStack_58 = uStack_80;
    puStack_50 = puVar1;
    uStack_48 = uStack_70;
    uStack_40 = uStack_68;
    uStack_94 = uVar4;
    if (uVar4 != uVar3) {
      puVar17 = puVar1;
      if ((uint *)param_1[0x67] != (uint *)0x0) {
        puVar17 = (uint *)param_1[0x67];
      }
      uVar13 = puVar17[(int)uVar4 >> 5] & -1 << ((byte)uVar4 & 0x1f);
      iVar15 = (int)uVar4 >> 5;
      while (uVar13 == 0) {
        uVar18 = uVar18 + 0x20;
        if ((int)((uVar3 - 1) + ((int)(uVar3 - 1) >> 0x1f & 0x1fU)) >> 5 <= iVar15)
        goto LAB_141c6f882;
        uVar13 = puVar17[(longlong)iVar15 + 1];
        iVar15 = iVar15 + 1;
      }
      uVar13 = uVar13 - 1 & uVar13 ^ uVar13;
      iVar15 = 0x1f;
      if (uVar13 != 0) {
        for (; uVar13 >> iVar15 == 0; iVar15 = iVar15 + -1) {
        }
      }
      if (uVar13 == 0) {
        iVar15 = 0x20;
      }
      else {
        iVar15 = 0x1f - iVar15;
      }
      uStack_94 = (uVar18 - iVar15) + 0x1f;
      if ((int)uVar3 < (int)uStack_94) {
LAB_141c6f882:
        uStack_94 = uVar3;
      }
    }
    while( true ) {
      if ((uStack_70._4_4_ == uStack_94) && (puStack_78 == puVar1)) {
        bVar5 = true;
      }
      else {
        bVar5 = false;
      }
      if (bVar5) break;
      puVar12 = (undefined4 *)((longlong)(int)uStack_70._4_4_ * 0x10 + *plVar10);
      uStackX_8 = *puVar12;
      uStackX_c = puVar12[1];
      pUVar8 = FWeakObjectPtr_Get((undefined *)&uStackX_8);
      if (pUVar8 != (UObject *)0x0) {
        lVar6 = FUN_1446acb00();
        if ((*(int *)(lVar6 + 0x38) <= *(int *)(*(longlong *)pUVar8->ClassPrivate + 0x38)) &&
           (*(longlong *)
             (*(longlong *)(*(longlong *)pUVar8->ClassPrivate + 0x30) +
             (longlong)*(int *)(lVar6 + 0x38) * 8) == lVar6 + 0x30)) {
          FUN_141c64450(param_1,pUVar8,0);
        }
      }
      uStack_70 = CONCAT44(uStack_70._4_4_,(uint)uStack_70 & ~uStack_80._4_4_);
      FUN_140687d00(&uStack_80);
    }
    uVar16 = uStackX_10;
  }
  else {
    lVar6 = (**(code **)(*param_1 + 0x150))();
    if ((lVar6 != 0) && (lVar6 = *(longlong *)(lVar6 + 0x130), lVar6 != 0)) {
      puVar9 = AFortGameStateAthena_StaticClass();
      if ((*(int *)(puVar9 + 0x38) <= *(int *)(*(longlong *)(lVar6 + 0x10) + 0x38)) &&
         (*(undefined **)
           (*(longlong *)(*(longlong *)(lVar6 + 0x10) + 0x30) +
           (longlong)*(int *)(puVar9 + 0x38) * 8) == puVar9 + 0x30)) {
        FUN_140694550(lVar6 + 0x20e0,param_1);
      }
    }
    lVar6 = param_1[0x80];
    plVar10 = (longlong *)FUN_142ec9f90();
    uVar16 = param_2;
    if (lVar6 != 0) {
      iVar15 = 0;
      if (0 < (int)plVar10[1]) {
        lVar7 = 0;
        do {
          plVar19 = (longlong *)(*plVar10 + lVar7);
          if ((((int)plVar19[1] != 0) && ((longlong *)*plVar19 != (longlong *)0x0)) &&
             (plVar11 = (longlong *)(**(code **)(*(longlong *)*plVar19 + 0x30))(), *plVar11 == lVar6
             )) {
            if (((int)plVar19[1] != 0) && ((longlong *)*plVar19 != (longlong *)0x0)) {
              (**(code **)(*(longlong *)*plVar19 + 0x38))();
              if ((undefined *)*plVar19 != (undefined *)0x0) {
                puVar9 = FMemory_Realloc((undefined *)*plVar19,0,0);
                *plVar19 = (longlong)puVar9;
              }
              *(undefined4 *)(plVar19 + 1) = 0;
            }
            FMulticastDelegateBase_CompactInvocationList((undefined *)plVar10,false);
            break;
          }
          iVar15 = iVar15 + 1;
          lVar7 = lVar7 + 0x10;
        } while (iVar15 < (int)plVar10[1]);
      }
      uVar16 = uStackX_10;
    }
  }
  if (param_1[0x54] != 0) {
    FUN_140690620(param_1[0x54] + 0x478,param_1);
    FUN_140690620(param_1[0x54] + 0x490,param_1);
  }
  FUN_141c92830(param_1);
  FUN_1411ab4c0(param_1,uVar16);
  return;
}

