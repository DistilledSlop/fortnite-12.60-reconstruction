
longlong * FUN_141c74e00(longlong param_1,longlong *param_2,longlong param_3)

{
  longlong *plVar1;
  longlong *plVar2;
  uint *puVar3;
  uint uVar4;
  uint uVar5;
  UObject *pUVar6;
  undefined4 *puVar7;
  uint uVar8;
  longlong lVar9;
  int iVar10;
  uint uVar11;
  uint *puVar12;
  bool bVar13;
  undefined4 uStackX_8;
  undefined4 uStackX_c;
  wchar_t *pwStackX_10;
  int iStack_a8;
  uint uStack_a4;
  uint uStack_94;
  int iStack_90;
  undefined4 uStack_8c;
  int iStack_80;
  uint uStack_7c;
  uint *puStack_78;
  uint uStack_70;
  uint uStack_6c;
  undefined8 uStack_68;
  longlong *plStack_60;
  undefined8 uStack_58;
  uint *puStack_50;
  uint uStack_48;
  uint uStack_44;
  undefined8 uStack_40;
  
  bVar13 = DAT_1482472e4 != '\0';
  plVar2 = (longlong *)(param_1 + 0x318);
  puVar3 = (uint *)(param_1 + 0x328);
  *param_2 = 0;
  pwStackX_10 = L"STAT_Volume_GetActors";
  if (bVar13) {
    pwStackX_10 = (wchar_t *)0x0;
  }
  param_2[1] = 0;
  uVar4 = *(uint *)(param_1 + 0x340);
  iStack_a8 = 0;
  uStack_a4 = 1;
  uStack_94 = 0;
  iStack_90 = 0;
  if (uVar4 != 0) {
    puVar12 = puVar3;
    if (*(uint **)(param_1 + 0x338) != (uint *)0x0) {
      puVar12 = *(uint **)(param_1 + 0x338);
    }
    uVar5 = *puVar12;
    iStack_90 = 0;
    iStack_a8 = 0;
    while (uVar5 == 0) {
      lVar9 = (longlong)iStack_a8;
      iStack_90 = iStack_90 + 0x20;
      iVar10 = iStack_a8 + 1;
      bVar13 = (int)((uVar4 - 1) + ((int)(uVar4 - 1) >> 0x1f & 0x1fU)) >> 5 <= iStack_a8;
      iStack_a8 = iVar10;
      if (bVar13) goto LAB_141c74efc;
      uVar5 = puVar12[lVar9 + 1];
    }
    uStack_a4 = uVar5 - 1 & uVar5 ^ uVar5;
    iVar10 = 0x1f;
    if (uStack_a4 != 0) {
      for (; uStack_a4 >> iVar10 == 0; iVar10 = iVar10 + -1) {
      }
    }
    if (uStack_a4 == 0) {
      iVar10 = 0x20;
    }
    else {
      iVar10 = 0x1f - iVar10;
    }
    uStack_94 = (iStack_90 - iVar10) + 0x1f;
    if ((int)uVar4 < (int)uStack_94) {
LAB_141c74efc:
      uStack_94 = uVar4;
    }
  }
  uVar5 = *(uint *)(param_1 + 0x340);
  uStack_70 = 0xffffffff;
  uStack_6c = uStack_94;
  uStack_68 = CONCAT44(uStack_8c,iStack_90);
  uStack_58 = CONCAT44(uStack_a4,iStack_a8);
  uVar11 = uVar5 & 0xffffffe0;
  iStack_80 = iStack_a8;
  uStack_7c = uStack_a4;
  puStack_78 = puVar3;
  plStack_60 = plVar2;
  puStack_50 = puVar3;
  uStack_40 = uStack_68;
  uStack_48 = uStack_70;
  uStack_44 = uStack_6c;
  uStack_94 = uVar5;
  if (uVar5 != uVar4) {
    puVar12 = puVar3;
    if (*(uint **)(param_1 + 0x338) != (uint *)0x0) {
      puVar12 = *(uint **)(param_1 + 0x338);
    }
    uVar8 = puVar12[(int)uVar5 >> 5] & -1 << ((byte)uVar5 & 0x1f);
    iVar10 = (int)uVar5 >> 5;
    while (uVar8 == 0) {
      uVar11 = uVar11 + 0x20;
      if ((int)((uVar4 - 1) + ((int)(uVar4 - 1) >> 0x1f & 0x1fU)) >> 5 <= iVar10)
      goto LAB_141c74fd4;
      uVar8 = puVar12[(longlong)iVar10 + 1];
      iVar10 = iVar10 + 1;
    }
    uVar8 = uVar8 - 1 & uVar8 ^ uVar8;
    iVar10 = 0x1f;
    if (uVar8 != 0) {
      for (; uVar8 >> iVar10 == 0; iVar10 = iVar10 + -1) {
      }
    }
    if (uVar8 == 0) {
      iVar10 = 0x20;
    }
    else {
      iVar10 = 0x1f - iVar10;
    }
    uStack_94 = (uVar11 - iVar10) + 0x1f;
    if ((int)uVar4 < (int)uStack_94) {
LAB_141c74fd4:
      uStack_94 = uVar4;
    }
  }
  do {
    if ((uStack_6c == uStack_94) && (puStack_78 == puVar3)) {
      bVar13 = true;
    }
    else {
      bVar13 = false;
    }
    if (bVar13) {
      if ((DAT_1482472e4 != '\0') && (pwStackX_10 != (wchar_t *)0x0)) {
        FUN_142cd3770(&pwStackX_10);
      }
      return param_2;
    }
    puVar7 = (undefined4 *)((longlong)(int)uStack_6c * 0x10 + *plVar2);
    uStackX_8 = *puVar7;
    uStackX_c = puVar7[1];
    pUVar6 = FWeakObjectPtr_Get((undefined *)&uStackX_8);
    if (pUVar6 != (UObject *)0x0) {
      lVar9 = FUN_1446acb00();
      if ((*(int *)(lVar9 + 0x38) <= *(int *)(*(longlong *)pUVar6->ClassPrivate + 0x38)) &&
         (*(longlong *)
           (*(longlong *)(*(longlong *)pUVar6->ClassPrivate + 0x30) +
           (longlong)*(int *)(lVar9 + 0x38) * 8) == lVar9 + 0x30)) {
        if ((param_3 != 0) && (lVar9 = FUN_1446acb00(), lVar9 != 0)) {
          plVar1 = (longlong *)(param_3 + 0x30);
          if ((*(int *)(lVar9 + 0x38) <= *(int *)(param_3 + 0x38)) &&
             (*(longlong *)(*plVar1 + (longlong)*(int *)(lVar9 + 0x38) * 8) == lVar9 + 0x30)) {
            lVar9 = FUN_1446acb00();
            if (lVar9 != 0) {
              iVar10 = *(int *)(param_3 + 0x38);
              if ((((*(int *)(lVar9 + 0x38) <= iVar10) &&
                   (*(longlong *)(*plVar1 + (longlong)*(int *)(lVar9 + 0x38) * 8) == lVar9 + 0x30))
                  && (iVar10 <= *(int *)(*(longlong *)pUVar6->ClassPrivate + 0x38))) &&
                 (*(longlong **)
                   (*(longlong *)(*(longlong *)pUVar6->ClassPrivate + 0x30) + (longlong)iVar10 * 8)
                  == plVar1)) goto LAB_141c750c7;
            }
            goto LAB_141c750e7;
          }
        }
LAB_141c750c7:
        iVar10 = (int)param_2[1];
        *(int *)(param_2 + 1) = iVar10 + 1;
        if (*(int *)((longlong)param_2 + 0xc) < iVar10 + 1) {
          TArray8_ResizeGrow((undefined *)param_2,iVar10);
        }
        *(UObject **)(*param_2 + (longlong)iVar10 * 8) = pUVar6;
      }
    }
LAB_141c750e7:
    uStack_70 = uStack_70 & ~uStack_7c;
    FUN_140687d00(&iStack_80);
  } while( true );
}

