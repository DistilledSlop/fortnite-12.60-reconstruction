
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

bool FUN_141c74950(longlong param_1,longlong *param_2)

{
  longlong *plVar1;
  uint *puVar2;
  uint uVar3;
  uint uVar4;
  undefined8 uVar5;
  bool bVar6;
  UObject *pUVar7;
  undefined8 *puVar8;
  undefined4 *puVar9;
  uint uVar10;
  longlong lVar11;
  int iVar12;
  int *piVar13;
  uint *puVar14;
  undefined8 *puVar15;
  uint uVar16;
  int iVar17;
  longlong unaff_GS_OFFSET;
  undefined4 uStackX_8;
  undefined4 uStackX_c;
  wchar_t *pwStackX_10;
  int iStack_98;
  uint uStack_94;
  uint uStack_84;
  int iStack_80;
  undefined4 uStack_7c;
  int iStack_70;
  uint uStack_6c;
  uint *puStack_68;
  uint uStack_60;
  uint uStack_5c;
  undefined8 uStack_58;
  longlong *plStack_50;
  undefined8 uStack_48;
  uint *puStack_40;
  uint uStack_38;
  uint uStack_34;
  undefined8 uStack_30;
  
  pwStackX_10 = L"STAT_Volume_GetActors";
  plVar1 = (longlong *)(param_1 + 0x318);
  iVar12 = *(int *)(param_1 + 800);
  if (DAT_1482472e4 != '\0') {
    pwStackX_10 = (wchar_t *)0x0;
  }
  iVar17 = *(int *)(param_1 + 0x34c);
  *(undefined4 *)(param_2 + 1) = 0;
  if (*(int *)((longlong)param_2 + 0xc) != iVar12 - iVar17) {
    FUN_1406d9a30(param_2);
  }
  piVar13 = (int *)(*(longlong *)
                     (*(longlong *)(unaff_GS_OFFSET + 0x58) + (ulonglong)__tls_index * 8) + 0x248);
  if ((*piVar13 < _DAT_148123150) && (FUN_145803a44(&DAT_148123150), _DAT_148123150 == -1)) {
    FUN_1458038a4(&LAB_14585b000);
    FUN_1458039e4(&DAT_148123150);
  }
  if ((*piVar13 < _DAT_148123168) && (FUN_145803a44(&DAT_148123168), _DAT_148123168 == -1)) {
    FUN_1458038a4(&LAB_14585afd0);
    FUN_1458039e4(&DAT_148123168);
  }
  _DAT_148123148 = 0;
  if ((_DAT_14812314c < 0) && (_DAT_14812314c != 0)) {
    FUN_1406d9a30(&DAT_148123140,0);
  }
  _DAT_148123160 = 0;
  if ((_DAT_148123164 < 0) && (_DAT_148123164 != 0)) {
    FUN_1406d9a30(&DAT_148123158,0);
  }
  puVar2 = (uint *)(param_1 + 0x328);
  iStack_98 = 0;
  uVar3 = *(uint *)(param_1 + 0x340);
  uStack_94 = 1;
  uStack_84 = 0;
  iStack_80 = 0;
  if (uVar3 != 0) {
    puVar14 = puVar2;
    if (*(uint **)(param_1 + 0x338) != (uint *)0x0) {
      puVar14 = *(uint **)(param_1 + 0x338);
    }
    uVar4 = *puVar14;
    iStack_80 = 0;
    iStack_98 = 0;
    while (uVar4 == 0) {
      lVar11 = (longlong)iStack_98;
      iStack_80 = iStack_80 + 0x20;
      iVar12 = iStack_98 + 1;
      bVar6 = (int)((uVar3 - 1) + ((int)(uVar3 - 1) >> 0x1f & 0x1fU)) >> 5 <= iStack_98;
      iStack_98 = iVar12;
      if (bVar6) goto LAB_141c74ac9;
      uVar4 = puVar14[lVar11 + 1];
    }
    uStack_94 = uVar4 - 1 & uVar4 ^ uVar4;
    iVar12 = 0x1f;
    if (uStack_94 != 0) {
      for (; uStack_94 >> iVar12 == 0; iVar12 = iVar12 + -1) {
      }
    }
    if (uStack_94 == 0) {
      iVar12 = 0x20;
    }
    else {
      iVar12 = 0x1f - iVar12;
    }
    uStack_84 = (iStack_80 - iVar12) + 0x1f;
    if ((int)uVar3 < (int)uStack_84) {
LAB_141c74ac9:
      uStack_84 = uVar3;
    }
  }
  uVar4 = *(uint *)(param_1 + 0x340);
  uStack_60 = 0xffffffff;
  uStack_5c = uStack_84;
  uStack_58 = CONCAT44(uStack_7c,iStack_80);
  uStack_48 = CONCAT44(uStack_94,iStack_98);
  uVar16 = uVar4 & 0xffffffe0;
  iStack_70 = iStack_98;
  uStack_6c = uStack_94;
  puStack_68 = puVar2;
  plStack_50 = plVar1;
  puStack_40 = puVar2;
  uStack_30 = uStack_58;
  uStack_38 = uStack_60;
  uStack_34 = uStack_5c;
  uStack_84 = uVar4;
  if (uVar4 != uVar3) {
    puVar14 = puVar2;
    if (*(uint **)(param_1 + 0x338) != (uint *)0x0) {
      puVar14 = *(uint **)(param_1 + 0x338);
    }
    uVar10 = puVar14[(int)uVar4 >> 5] & -1 << ((byte)uVar4 & 0x1f);
    iVar12 = (int)uVar4 >> 5;
    while (uVar10 == 0) {
      uVar16 = uVar16 + 0x20;
      if ((int)((uVar3 - 1) + ((int)(uVar3 - 1) >> 0x1f & 0x1fU)) >> 5 <= iVar12)
      goto LAB_141c74b92;
      uVar10 = puVar14[(longlong)iVar12 + 1];
      iVar12 = iVar12 + 1;
    }
    uVar10 = uVar10 - 1 & uVar10 ^ uVar10;
    iVar12 = 0x1f;
    if (uVar10 != 0) {
      for (; uVar10 >> iVar12 == 0; iVar12 = iVar12 + -1) {
      }
    }
    if (uVar10 == 0) {
      iVar12 = 0x20;
    }
    else {
      iVar12 = 0x1f - iVar12;
    }
    uStack_84 = (uVar16 - iVar12) + 0x1f;
    if ((int)uVar3 < (int)uStack_84) {
LAB_141c74b92:
      uStack_84 = uVar3;
    }
  }
  while( true ) {
    iVar12 = _DAT_148123148;
    if ((uStack_5c == uStack_84) && (puStack_68 == puVar2)) {
      bVar6 = true;
    }
    else {
      bVar6 = false;
    }
    if (bVar6) break;
    puVar9 = (undefined4 *)((longlong)(int)uStack_5c * 0x10 + *plVar1);
    uStackX_8 = *puVar9;
    uStackX_c = puVar9[1];
    pUVar7 = FWeakObjectPtr_Get((undefined *)&uStackX_8);
    if (pUVar7 != (UObject *)0x0) {
      lVar11 = FUN_1446acb00();
      if ((*(int *)(lVar11 + 0x38) <= *(int *)(*(longlong *)pUVar7->ClassPrivate + 0x38)) &&
         (*(longlong *)
           (*(longlong *)(*(longlong *)pUVar7->ClassPrivate + 0x30) +
           (longlong)*(int *)(lVar11 + 0x38) * 8) == lVar11 + 0x30)) {
        lVar11 = FUN_1447ae380();
        iVar12 = _DAT_148123160;
        iVar17 = _DAT_148123148;
        if ((*(int *)(*(longlong *)pUVar7->ClassPrivate + 0x38) < *(int *)(lVar11 + 0x38)) ||
           (*(longlong *)
             (*(longlong *)(*(longlong *)pUVar7->ClassPrivate + 0x30) +
             (longlong)*(int *)(lVar11 + 0x38) * 8) != lVar11 + 0x30)) {
          _DAT_148123160 = _DAT_148123160 + 1;
          if (_DAT_148123164 < _DAT_148123160) {
            TArray8_ResizeGrow(&DAT_148123158,iVar12);
          }
          puVar8 = _DAT_148123158;
        }
        else {
          _DAT_148123148 = _DAT_148123148 + 1;
          if (_DAT_14812314c < _DAT_148123148) {
            TArray8_ResizeGrow(&DAT_148123140,iVar17);
          }
          puVar8 = _DAT_148123140;
          iVar12 = iVar17;
        }
        puVar8[iVar12] = pUVar7;
      }
    }
    uStack_60 = uStack_60 & ~uStack_6c;
    FUN_140687d00(&iStack_70);
  }
  if (_DAT_148123148 != 0) {
    iVar17 = (int)param_2[1];
    if (*(int *)((longlong)param_2 + 0xc) < _DAT_148123148 + iVar17) {
      FUN_1406d9a30(param_2);
      iVar17 = (int)param_2[1];
    }
    puVar15 = (undefined8 *)(*param_2 + (longlong)iVar17 * 8);
    puVar8 = _DAT_148123140;
    iVar17 = iVar12;
    do {
      uVar5 = *puVar8;
      puVar8 = puVar8 + 1;
      *puVar15 = uVar5;
      puVar15 = puVar15 + 1;
      iVar17 = iVar17 + -1;
    } while (iVar17 != 0);
    *(int *)(param_2 + 1) = (int)param_2[1] + iVar12;
  }
  iVar17 = _DAT_148123160;
  lVar11 = (longlong)_DAT_148123160;
  iVar12 = (int)param_2[1];
  if (_DAT_148123160 != 0) {
    if (*(int *)((longlong)param_2 + 0xc) < _DAT_148123160 + iVar12) {
      FUN_1406d9a30(param_2);
      iVar12 = (int)param_2[1];
    }
    memcpy(*param_2 + (longlong)iVar12 * 8,_DAT_148123158,lVar11 << 3);
    *(int *)(param_2 + 1) = (int)param_2[1] + iVar17;
    iVar12 = (int)param_2[1];
  }
  if ((DAT_1482472e4 != '\0') && (pwStackX_10 != (wchar_t *)0x0)) {
    FUN_142cd3770(&pwStackX_10);
  }
  return 0 < iVar12;
}

