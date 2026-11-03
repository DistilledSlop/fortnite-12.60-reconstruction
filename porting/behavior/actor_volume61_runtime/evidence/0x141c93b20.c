
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_141c93b20(longlong *param_1,longlong *param_2,char param_3)

{
  uint *puVar1;
  longlong *plVar2;
  uint uVar3;
  uint uVar4;
  bool bVar5;
  char cVar6;
  bool bVar7;
  longlong lVar8;
  longlong lVar9;
  int iVar10;
  uint uVar11;
  uint uVar12;
  uint *puVar13;
  undefined4 uVar14;
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
  
  if (param_2 != (longlong *)0x0) {
    lVar8 = FUN_142a174f0();
    if ((*(int *)(lVar8 + 0x38) <= *(int *)(param_2[2] + 0x38)) &&
       (*(longlong *)(*(longlong *)(param_2[2] + 0x30) + (longlong)*(int *)(lVar8 + 0x38) * 8) ==
        lVar8 + 0x30)) {
      lVar8 = (**(code **)(*param_1 + 0x150))(param_1);
      if (lVar8 != 0) {
        lVar9 = FUN_142321a70(param_2);
        lVar8 = FUN_14468f240(lVar8);
        if (((lVar8 == 0) || (lVar8 != lVar9)) &&
           (cVar6 = (**(code **)(*param_2 + 0xdb0))(param_2), cVar6 == '\0')) {
          return;
        }
      }
      bVar7 = Stub_ReturnFalse();
      uStack_a4 = 1;
      plVar2 = param_1 + 0x33;
      iStack_a8 = 0;
      uVar3 = *(uint *)(param_1 + 0x38);
      puVar1 = (uint *)(param_1 + 0x35);
      uStack_94 = 0;
      iStack_90 = 0;
      if (uVar3 != 0) {
        puVar13 = puVar1;
        if ((uint *)param_1[0x37] != (uint *)0x0) {
          puVar13 = (uint *)param_1[0x37];
        }
        uVar4 = *puVar13;
        iStack_a8 = 0;
        iStack_90 = 0;
        while (uVar4 == 0) {
          lVar8 = (longlong)iStack_a8;
          iStack_90 = iStack_90 + 0x20;
          iVar10 = iStack_a8 + 1;
          bVar5 = (int)((uVar3 - 1) + ((int)(uVar3 - 1) >> 0x1f & 0x1fU)) >> 5 <= iStack_a8;
          iStack_a8 = iVar10;
          if (bVar5) goto LAB_141c93c8d;
          uVar4 = puVar13[lVar8 + 1];
        }
        uStack_a4 = uVar4 - 1 & uVar4 ^ uVar4;
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
        if ((int)uVar3 < (int)uStack_94) {
LAB_141c93c8d:
          uStack_94 = uVar3;
        }
      }
      uVar4 = *(uint *)(param_1 + 0x38);
      uStack_70 = 0xffffffff;
      uStack_6c = uStack_94;
      uStack_68 = CONCAT44(uStack_8c,iStack_90);
      uStack_58 = CONCAT44(uStack_a4,iStack_a8);
      uVar12 = uVar4 & 0xffffffe0;
      iStack_80 = iStack_a8;
      uStack_7c = uStack_a4;
      puStack_78 = puVar1;
      plStack_60 = plVar2;
      puStack_50 = puVar1;
      uStack_40 = uStack_68;
      uStack_48 = uStack_70;
      uStack_44 = uStack_6c;
      uStack_94 = uVar4;
      if (uVar4 != uVar3) {
        puVar13 = puVar1;
        if ((uint *)param_1[0x37] != (uint *)0x0) {
          puVar13 = (uint *)param_1[0x37];
        }
        uVar11 = puVar13[(int)uVar4 >> 5] & -1 << ((byte)uVar4 & 0x1f);
        iVar10 = (int)uVar4 >> 5;
        while (uVar11 == 0) {
          uVar12 = uVar12 + 0x20;
          if ((int)((uVar3 - 1) + ((int)(uVar3 - 1) >> 0x1f & 0x1fU)) >> 5 <= iVar10)
          goto LAB_141c93d52;
          uVar11 = puVar13[(longlong)iVar10 + 1];
          iVar10 = iVar10 + 1;
        }
        uVar11 = uVar11 - 1 & uVar11 ^ uVar11;
        iVar10 = 0x1f;
        if (uVar11 != 0) {
          for (; uVar11 >> iVar10 == 0; iVar10 = iVar10 + -1) {
          }
        }
        if (uVar11 == 0) {
          iVar10 = 0x20;
        }
        else {
          iVar10 = 0x1f - iVar10;
        }
        uStack_94 = (uVar12 - iVar10) + 0x1f;
        if ((int)uVar3 < (int)uStack_94) {
LAB_141c93d52:
          uStack_94 = uVar3;
        }
      }
      while( true ) {
        if ((uStack_6c == uStack_94) && (puStack_78 == puVar1)) {
          bVar5 = true;
        }
        else {
          bVar5 = false;
        }
        if (bVar5) break;
        lVar8 = *(longlong *)(*plVar2 + (longlong)(int)uStack_6c * 0x10);
        if (lVar8 != 0) {
          lVar9 = FUN_1447c6c10();
          if ((*(int *)(lVar9 + 0x38) <= *(int *)(*(longlong *)(lVar8 + 0x10) + 0x38)) &&
             (*(longlong *)
               (*(longlong *)(*(longlong *)(lVar8 + 0x10) + 0x30) +
               (longlong)*(int *)(lVar9 + 0x38) * 8) == lVar9 + 0x30)) {
            cVar6 = FUN_1440a6ab0(lVar8,_DAT_148122c40);
            if (bVar7) {
              if (cVar6 != '\0') {
                if (param_3 == '\0') {
LAB_141c93df7:
                  uVar14 = 0;
                }
                else {
                  uVar14 = 0x3f800000;
                }
LAB_141c93dfa:
                *(undefined4 *)(lVar8 + 0x738) = uVar14;
              }
            }
            else if (cVar6 == '\0') {
              if (param_3 == '\0') goto LAB_141c93df7;
              uVar14 = 0x3f800000;
              goto LAB_141c93dfa;
            }
          }
        }
        uStack_70 = uStack_70 & ~uStack_7c;
        FUN_140687d00(&iStack_80);
      }
    }
  }
  return;
}

