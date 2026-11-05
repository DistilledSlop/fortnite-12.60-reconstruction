
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_141d394f0(longlong param_1)

{
  undefined8 *puVar1;
  longlong lVar2;
  undefined8 uVar3;
  undefined8 uVar4;
  undefined8 uVar5;
  undefined8 uVar6;
  undefined8 uVar7;
  undefined8 uVar8;
  undefined8 uVar9;
  longlong *plVar10;
  char *pcVar11;
  int iVar12;
  longlong lVar13;
  int OldNum;
  longlong *plVar14;
  undefined8 *puVar15;
  longlong *Ptr;
  float fVar16;
  float fVar17;
  float fVar18;
  undefined1 auVar19 [16];
  undefined1 auVar20 [16];
  float fVar21;
  float fVar22;
  longlong *plStack_128;
  int iStack_120;
  int iStack_11c;
  undefined8 uStack_118;
  undefined8 uStack_110;
  float fStack_108;
  float fStack_104;
  float fStack_100;
  float fStack_fc;
  float fStack_f8;
  float fStack_f4;
  float fStack_f0;
  undefined4 uStack_ec;
  undefined8 uStack_b8;
  undefined8 uStack_b0;
  
  if ((0 < _DAT_148126190) && (*(int *)(param_1 + 0x1d0) != 0)) {
    iVar12 = 0;
    OldNum = 0;
    puVar15 = *(undefined8 **)(param_1 + 0x1c8);
    Ptr = (longlong *)0x0;
    plStack_128 = (longlong *)0x0;
    puVar1 = puVar15 + *(int *)(param_1 + 0x1d0);
    iStack_11c = 0;
    plVar10 = _DAT_1478c3c70;
    for (; _DAT_1478c3c70 = plVar10, puVar15 != puVar1; puVar15 = puVar15 + 1) {
      pcVar11 = (char *)*puVar15;
      if (*pcVar11 == '\x03') {
        iStack_120 = OldNum + 1;
        if (iVar12 < iStack_120) {
          TArray8_ResizeGrow((undefined *)&plStack_128,OldNum);
          pcVar11 = (char *)*puVar15;
          Ptr = plStack_128;
          iVar12 = iStack_11c;
        }
        Ptr[OldNum] = (longlong)pcVar11;
        OldNum = iStack_120;
      }
      plVar10 = _DAT_1478c3c70;
    }
    *(undefined4 *)(plVar10 + 1) = 0;
    if (*(int *)((longlong)plVar10 + 0xc) < OldNum) {
      FUN_1406d9b10(plVar10,OldNum);
    }
    for (plVar14 = Ptr; plVar14 != Ptr + OldNum; plVar14 = plVar14 + 1) {
      lVar2 = *plVar14;
      plStack_128 = *(longlong **)(lVar2 + 0x70);
      fVar21 = SUB84(plStack_128,0);
      fVar22 = (float)((ulonglong)plStack_128 >> 0x20);
      uVar3 = *(undefined8 *)(lVar2 + 0x50);
      uStack_110 = *(undefined8 *)(lVar2 + 0x58);
      uVar4 = *(undefined8 *)(lVar2 + 0x10);
      uVar5 = *(undefined8 *)(lVar2 + 0x18);
      uVar6 = *(undefined8 *)(lVar2 + 0x20);
      uVar7 = *(undefined8 *)(lVar2 + 0x28);
      uVar8 = *(undefined8 *)(lVar2 + 0x30);
      uVar9 = *(undefined8 *)(lVar2 + 0x38);
      fVar16 = fVar21 * fVar21 + fVar22 * fVar22;
      if (1e-08 < fVar16) {
        auVar19._4_4_ = fVar22;
        auVar19._0_4_ = fVar16;
        auVar19._8_8_ = 0;
        auVar20._4_4_ = fVar22;
        auVar20._0_4_ = fVar16;
        auVar20._8_8_ = 0;
        auVar20 = rsqrtss(auVar19,auVar20);
        fVar18 = auVar20._0_4_;
        fVar18 = fVar18 + fVar18 * (0.5 - fVar16 * 0.5 * fVar18 * fVar18);
        fVar18 = fVar18 + fVar18 * (0.5 - fVar16 * 0.5 * fVar18 * fVar18);
        fVar21 = fVar21 * fVar18;
        fVar22 = fVar22 * fVar18;
      }
      fVar16 = SQRT(fVar16) / _DAT_1478c36c8;
      if (0.0 <= fVar16) {
        if (1.0 <= fVar16) {
          fVar16 = 1.0;
        }
      }
      else {
        fVar16 = 0.0;
      }
      fVar18 = _DAT_1478c36d0 * fVar16 * _DAT_1478c36b4;
      uStack_118._0_4_ = (float)uVar3;
      fStack_f8 = fVar21 * fVar18 + (float)uStack_118;
      uStack_118._4_4_ = (float)((ulonglong)uVar3 >> 0x20);
      fStack_f4 = fVar22 * fVar18 + uStack_118._4_4_;
      fVar18 = (float)uStack_110;
      fStack_108 = (_DAT_1478c36bc - 1.0) * fVar16 + 1.0;
      fVar17 = _DAT_1478c36d0 - fStack_108 * _DAT_1478c36cc;
      fStack_104 = fStack_f8 - fVar21 * fVar17 * _DAT_1478c36b8 * fVar16;
      fVar21 = _DAT_1478c36c4 - (float)uStack_110;
      fStack_100 = fStack_f4 - fVar22 * fVar17 * _DAT_1478c36b8 * fVar16;
      if (0.0 <= fVar21) {
        if (_DAT_1478c36c4 <= fVar21) {
          fVar21 = _DAT_1478c36c4;
        }
      }
      else {
        fVar21 = 0.0;
      }
      uStack_118 = uVar3;
      if (_DAT_1478c36c0 <= 0.0) {
        fStack_fc = 0.0;
      }
      else {
        fStack_fc = (float)powf(fVar21);
        fStack_fc = fVar18 - fStack_fc;
        if (fStack_fc <= 0.0) {
          fStack_fc = 0.0;
        }
      }
      iVar12 = (int)plVar10[1];
      *(int *)(plVar10 + 1) = iVar12 + 1;
      fStack_f0 = fStack_fc;
      if (*(int *)((longlong)plVar10 + 0xc) < iVar12 + 1) {
        FUN_14072b9f0(plVar10,iVar12);
      }
      lVar2 = *plVar10;
      lVar13 = (longlong)iVar12 * 0x60;
      *(undefined8 *)(lVar13 + lVar2) = CONCAT44(fStack_104,fStack_108);
      ((undefined8 *)(lVar13 + lVar2))[1] = CONCAT44(fStack_fc,fStack_100);
      puVar1 = (undefined8 *)(lVar13 + 0x10 + lVar2);
      *puVar1 = CONCAT44(fStack_f4,fStack_f8);
      puVar1[1] = CONCAT44(uStack_ec,fStack_f0);
      puVar1 = (undefined8 *)(lVar13 + 0x20 + lVar2);
      *puVar1 = uVar4;
      puVar1[1] = uVar5;
      puVar1 = (undefined8 *)(lVar13 + 0x30 + lVar2);
      *puVar1 = uVar6;
      puVar1[1] = uVar7;
      puVar1 = (undefined8 *)(lVar13 + 0x40 + lVar2);
      *puVar1 = uVar8;
      puVar1[1] = uVar9;
      puVar1 = (undefined8 *)(lVar13 + 0x50 + lVar2);
      *puVar1 = uStack_b8;
      puVar1[1] = uStack_b0;
    }
    if (Ptr != (longlong *)0x0) {
      FMemory_Free((undefined *)Ptr);
    }
  }
  return;
}

