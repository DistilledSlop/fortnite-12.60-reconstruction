
void FUN_141d58c30(longlong param_1,undefined8 *param_2)

{
  longlong *plVar1;
  longlong *plVar2;
  undefined8 *puVar3;
  undefined4 *puVar4;
  int iVar5;
  uint uVar6;
  undefined8 uVar7;
  longlong lVar8;
  bool bVar9;
  undefined4 uVar10;
  undefined4 uVar11;
  undefined4 uVar12;
  int iVar13;
  int iVar14;
  int iVar15;
  longlong lVar16;
  longlong lVar17;
  uint *puVar18;
  int iStack_68;
  uint uStack_64;
  int iStack_60;
  uint uStack_5c;
  undefined8 uStack_58;
  uint uStack_50;
  int iStack_4c;
  int iStack_48;
  undefined4 uStack_44;
  longlong *plStack_40;
  int iStack_38;
  uint uStack_34;
  uint *puStack_30;
  undefined4 uStack_28;
  int iStack_24;
  int iStack_20;
  undefined4 uStack_1c;

  iVar15 = *(int *)(param_2 + 1);
  plVar1 = (longlong *)(param_1 + 0x1d8);
  iVar13 = *(int *)(param_1 + 0x1e4);
  *(undefined4 *)(param_1 + 0x1e0) = 0;
  if (iVar13 < iVar15) {
    FUN_14072cf80(plVar1,iVar15);
    iVar13 = *(int *)(param_1 + 0x1e4);
  }
  uVar7 = *param_2;
  iVar5 = *(int *)(param_1 + 0x1e0);
  *(int *)(param_1 + 0x1e0) = iVar5 + iVar15;
  if (iVar13 < iVar5 + iVar15) {
    FUN_14072b840(plVar1,iVar5);
  }
  memcpy((longlong)iVar5 * 0x30 + *plVar1,uVar7);
  plVar2 = (longlong *)(param_1 + 0x178);
  iStack_68 = 0;
  uStack_64 = 1;
  iVar15 = *(int *)(param_1 + 0x1a0);
  puStack_30 = (uint *)(param_1 + 0x188);
  uStack_58._4_4_ = 0;
  uStack_50 = 0;
  if (iVar15 != 0) {
    puVar18 = puStack_30;
    if (*(uint **)(param_1 + 0x198) != (uint *)0x0) {
      puVar18 = *(uint **)(param_1 + 0x198);
    }
    uVar6 = *puVar18;
    iStack_68 = 0;
    uStack_50 = 0;
    while (uVar6 == 0) {
      iStack_68 = iStack_68 + 1;
      uStack_50 = uStack_50 + 0x20;
      if ((int)((iVar15 + -1 >> 0x1f & 0x1fU) + iVar15 + -1) >> 5 < iStack_68) goto LAB_141d58d5f;
      uVar6 = puVar18[iStack_68];
    }
    uStack_64 = -uVar6 & uVar6;
    iVar13 = 0x1f;
    if (uStack_64 != 0) {
      for (; uStack_64 >> iVar13 == 0; iVar13 = iVar13 + -1) {
      }
    }
    if (uStack_64 == 0) {
      iVar13 = 0x20;
    }
    else {
      iVar13 = 0x1f - iVar13;
    }
    uStack_58._4_4_ = (uStack_50 - iVar13) + 0x1f;
    if (iVar15 < uStack_58._4_4_) {
LAB_141d58d5f:
      uStack_58._4_4_ = iVar15;
    }
  }
  uStack_28 = 0xffffffff;
  iStack_24 = uStack_58._4_4_;
  iStack_20 = uStack_50;
  uStack_1c = iStack_4c;
  iStack_38 = iStack_68;
  uStack_34 = uStack_64;
  iStack_60 = iStack_68;
  uStack_5c = uStack_64;
  iStack_48 = uStack_50;
  uStack_44 = iStack_4c;
  uStack_50 = 0xffffffff;
  iStack_4c = uStack_58._4_4_;
  bVar9 = uStack_58._4_4_ < iVar15;
  plStack_40 = plVar2;
  uStack_58 = puStack_30;
  if (bVar9) {
    do {
      iVar15 = *(int *)(param_1 + 0x1e0);
      lVar17 = (longlong)iVar15;
      lVar16 = (longlong)iStack_4c * 0x50 + *plVar2;
      *(int *)(param_1 + 0x1e0) = iVar15 + 1;
      if (*(int *)(param_1 + 0x1e4) < iVar15 + 1) {
        FUN_14072b840(plVar1,iVar15);
      }
      lVar8 = *plVar1;
      uVar7 = *(undefined8 *)(lVar16 + 0x18);
      puVar3 = (undefined8 *)(lVar8 + lVar17 * 0x30);
      *puVar3 = *(undefined8 *)(lVar16 + 0x10);
      puVar3[1] = uVar7;
      uVar10 = *(undefined4 *)(lVar16 + 0x24);
      uVar11 = *(undefined4 *)(lVar16 + 0x28);
      uVar12 = *(undefined4 *)(lVar16 + 0x2c);
      puVar4 = (undefined4 *)(lVar8 + 0x10 + lVar17 * 0x30);
      *puVar4 = *(undefined4 *)(lVar16 + 0x20);
      puVar4[1] = uVar10;
      puVar4[2] = uVar11;
      puVar4[3] = uVar12;
      uVar10 = *(undefined4 *)(lVar16 + 0x34);
      uVar11 = *(undefined4 *)(lVar16 + 0x38);
      uVar12 = *(undefined4 *)(lVar16 + 0x3c);
      puVar4 = (undefined4 *)(lVar8 + 0x20 + lVar17 * 0x30);
      *puVar4 = *(undefined4 *)(lVar16 + 0x30);
      puVar4[1] = uVar10;
      puVar4[2] = uVar11;
      puVar4[3] = uVar12;
      uStack_50 = uStack_50 & ~uStack_5c;
      FUN_140687d00(&iStack_60);
    } while (iStack_4c < (int)uStack_58[6]);
  }
  iVar15 = *(int *)(param_1 + 0x3c);
  plVar2 = (longlong *)(param_1 + 0x30);
  *(undefined4 *)(param_1 + 0x38) = 0;
  if (iVar15 < *(int *)(param_1 + 0x1e0)) {
    FUN_14072cf80(plVar2);
    iVar15 = *(int *)(param_1 + 0x3c);
  }
  iVar13 = *(int *)(param_1 + 0x1e0);
  lVar16 = *plVar1;
  iVar5 = *(int *)(param_1 + 0x38);
  iVar14 = iVar5 + iVar13;
  *(int *)(param_1 + 0x38) = iVar14;
  if (iVar15 < iVar14) {
    FUN_14072b840(plVar2,iVar5);
  }
  memcpy((longlong)iVar5 * 0x30 + *plVar2,lVar16,(longlong)iVar13 * 0x30);
  return;
}
