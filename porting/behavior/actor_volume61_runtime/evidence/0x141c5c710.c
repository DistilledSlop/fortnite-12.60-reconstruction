
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_141c5c710(longlong *param_1,longlong param_2,undefined8 *param_3)

{
  undefined4 *puVar1;
  undefined1 auVar2 [16];
  float fVar3;
  float fVar4;
  bool bVar5;
  char cVar6;
  int iVar7;
  int *piVar8;
  longlong lVar9;
  longlong lVar10;
  undefined8 *puVar11;
  ulonglong uVar12;
  uint uVar13;
  uint uVar14;
  int *piVar15;
  longlong lVar16;
  uint uVar17;
  uint uVar18;
  undefined4 uVar19;
  undefined4 uVar20;
  char acStackX_8 [8];
  undefined4 uStackX_10;
  undefined4 uStackX_14;
  undefined8 uStackX_20;
  undefined8 in_stack_ffffffffffffff18;
  char *pcVar21;
  undefined8 uVar22;
  wchar_t *apwStack_c8 [2];
  longlong lStack_b8;
  undefined8 uStack_b0;
  longlong lStack_a8;
  longlong *plStack_a0;
  longlong lStack_98;
  undefined8 uStack_90;
  undefined4 uStack_88;
  undefined8 uStack_84;
  undefined4 uStack_7c;
  undefined4 auStack_78 [2];
  int *piStack_70;
  undefined1 auStack_68 [48];
  
  uVar19 = (undefined4)((ulonglong)in_stack_ffffffffffffff18 >> 0x20);
  apwStack_c8[0] = L"STAT_Volume_AddActor";
  acStackX_8[0] = '\0';
  if (DAT_1482472e4 != '\0') {
    apwStack_c8[0] = (wchar_t *)0x0;
  }
  uStackX_20 = 0xffffffff;
  if (param_2 != 0) {
    FUN_142ee3b30(&uStackX_10);
    uStackX_20 = CONCAT44(uStackX_14,uStackX_10);
  }
  piVar8 = (int *)FUN_140782160(param_1 + 99,&uStackX_10,&uStackX_20);
  if (*piVar8 != -1) goto LAB_141c5cd19;
  if (param_2 != 0) {
    lVar9 = FUN_142752f80();
    if (((*(int *)(lVar9 + 0x38) <= *(int *)(*(longlong *)(param_2 + 0x10) + 0x38)) &&
        (*(longlong *)
          (*(longlong *)(*(longlong *)(param_2 + 0x10) + 0x30) +
          (longlong)*(int *)(lVar9 + 0x38) * 8) == lVar9 + 0x30)) &&
       ((*(byte *)(param_2 + 0x6a9) & 4) != 0)) goto LAB_141c5cd19;
  }
  FUN_141c64450(param_1,param_2,1);
  if ((char)param_1[0x1e] != '\x03') {
    FUN_141c93b20(param_1,param_2,1);
    goto LAB_141c5cd19;
  }
  uStackX_10 = 0xffffffff;
  uStackX_14 = 0;
  if (param_2 != 0) {
    FUN_142ee3b30(&uStackX_10,param_2);
  }
  uVar22 = CONCAT44(uStackX_14,uStackX_10);
  FUN_1411b9710(param_1 + 99,auStack_78);
  if (piStack_70 == (int *)0x0) {
    piStack_70 = (int *)0x0;
  }
  else {
    *(undefined8 *)piStack_70 = uVar22;
    piStack_70[2] = -1;
  }
  uVar14 = piStack_70[1];
  uVar13 = *piStack_70 - uVar14 ^ uVar14 >> 0xd;
  uVar17 = (-0x61c88647 - uVar13) - uVar14 ^ uVar13 << 8;
  uVar18 = (uVar14 - uVar17) - uVar13 ^ uVar17 >> 0xd;
  uVar14 = (uVar13 - uVar17) - uVar18 ^ uVar18 >> 0xc;
  uVar13 = (uVar17 - uVar14) - uVar18 ^ uVar14 << 0x10;
  uVar17 = (uVar18 - uVar13) - uVar14 ^ uVar13 >> 5;
  uVar14 = (uVar14 - uVar13) - uVar17 ^ uVar17 >> 3;
  uVar13 = (uVar13 - uVar14) - uVar17 ^ uVar14 << 10;
  pcVar21 = acStackX_8;
  FUN_1407818b0(param_1 + 99,&uStackX_10,uVar13 >> 0xf ^ uVar17 - (uVar13 + uVar14),piStack_70,
                CONCAT44(uVar19,auStack_78[0]),pcVar21);
  if (param_2 == 0) {
LAB_141c5c910:
    lStack_a8 = 0;
  }
  else {
    lVar9 = FUN_1447ae380();
    if ((*(int *)(*(longlong *)(param_2 + 0x10) + 0x38) < *(int *)(lVar9 + 0x38)) ||
       (lStack_a8 = param_2,
       *(longlong *)
        (*(longlong *)(*(longlong *)(param_2 + 0x10) + 0x30) + (longlong)*(int *)(lVar9 + 0x38) * 8)
       != lVar9 + 0x30)) goto LAB_141c5c910;
  }
  uStack_b0 = *(undefined8 *)(param_2 + 0x130);
  lStack_b8 = param_2;
  plStack_a0 = param_1;
  FUN_1406a9dc0(param_1 + 0xa4,&lStack_b8);
  uStack_b0 = *param_3;
  lStack_a8 = CONCAT44(lStack_a8._4_4_,*(undefined4 *)(param_3 + 1));
  lStack_b8 = param_2;
  FUN_1406a9dc0(param_1 + 0x98,&lStack_b8);
  if ((_DAT_1478c33fc == 1) && (acStackX_8[0] == '\0')) {
    lVar9 = FUN_142732ac0();
    if ((*(int *)(lVar9 + 0x38) <= *(int *)(*(longlong *)(param_2 + 0x10) + 0x38)) &&
       (*(longlong *)
         (*(longlong *)(*(longlong *)(param_2 + 0x10) + 0x30) + (longlong)*(int *)(lVar9 + 0x38) * 8
         ) == lVar9 + 0x30)) {
      piVar8 = (int *)FUN_141c55d00(param_1 + 0x6d,*(undefined1 *)(param_2 + 0x3eb));
      *piVar8 = *piVar8 + 1;
    }
  }
  FUN_142cd49c0(&lStack_98,"AthenaAIRequiresNavigation",1);
  FUN_142cd49c0(&uStackX_20,"AthenaAIRequiresAIDirector",1);
  lVar9 = FUN_141ed1800();
  bVar5 = FName_EqualsEName(*(int *)(lVar9 + 0x2050),0);
  iVar7 = *(int *)(lVar9 + 0x2054);
  if (iVar7 != 0 || !bVar5) {
    piVar15 = (int *)param_1[0x4a];
    piVar8 = piVar15 + (longlong)(int)param_1[0x4b] * 2;
    if (piVar15 != piVar8) {
      do {
        if (*piVar15 == *(int *)(lVar9 + 0x2050) && piVar15[1] == iVar7) goto LAB_141c5ca8f;
        piVar15 = piVar15 + 2;
      } while (piVar15 != piVar8);
    }
    piVar15 = (int *)param_1[0x4c];
    piVar8 = piVar15 + (longlong)(int)param_1[0x4d] * 2;
    if (piVar15 != piVar8) {
      do {
        if (*piVar15 == *(int *)(lVar9 + 0x2050) && piVar15[1] == iVar7) goto LAB_141c5ca8f;
        piVar15 = piVar15 + 2;
      } while (piVar15 != piVar8);
    }
  }
LAB_141c5cd19:
  if ((DAT_1482472e4 != '\0') && (apwStack_c8[0] != (wchar_t *)0x0)) {
    FUN_142cd3770(apwStack_c8);
  }
  return;
LAB_141c5ca8f:
  cVar6 = FUN_143f67a70(param_2,uStackX_20);
  if (cVar6 != '\0') {
    if (((*(int *)((longlong)param_1 + 0x4ec) == 0) &&
        (lVar9 = (**(code **)(*param_1 + 0x150))(param_1), lVar9 != 0)) &&
       (lVar9 = *(longlong *)(lVar9 + 0x128), lVar9 != 0)) {
      lVar10 = FUN_1428ee970();
      if (((*(int *)(lVar10 + 0x38) <= *(int *)(*(longlong *)(lVar9 + 0x10) + 0x38)) &&
          (*(longlong *)
            (*(longlong *)(*(longlong *)(lVar9 + 0x10) + 0x30) +
            (longlong)*(int *)(lVar10 + 0x38) * 8) == lVar10 + 0x30)) &&
         (lVar9 = FUN_141e9a660(lVar9), lVar9 != 0)) {
        *(uint *)(lVar9 + 0xb60) = *(uint *)(lVar9 + 0xb60) & 0xfffffffd;
      }
    }
    *(int *)((longlong)param_1 + 0x4ec) = *(int *)((longlong)param_1 + 0x4ec) + 1;
  }
  cVar6 = FUN_143f67a70(param_2,lStack_98);
  if (cVar6 == '\0') goto LAB_141c5cd19;
  *(int *)(param_1 + 0x9d) = (int)param_1[0x9d] + 1;
  if (((param_1[0x9c] == 0) && (lVar9 = (**(code **)(*param_1 + 0x150))(param_1), lVar9 != 0)) &&
     ((param_1[0x9b] != 0 && (lVar10 = FUN_14271f760(), lVar10 != 0)))) {
    lVar16 = param_1[0x9b];
    if (((*(int *)(lVar10 + 0x38) <= *(int *)(lVar16 + 0x38)) &&
        (*(longlong *)(*(longlong *)(lVar16 + 0x30) + (longlong)*(int *)(lVar10 + 0x38) * 8) ==
         lVar10 + 0x30)) && (lVar16 != 0)) {
      lVar10 = param_1[0x26];
      if (lVar10 == 0) {
        uVar19 = (undefined4)_DAT_148218640;
        uVar20 = (undefined4)((ulonglong)_DAT_148218640 >> 0x20);
        uStack_88 = _DAT_148218648;
      }
      else {
        fVar3 = *(float *)(lVar10 + 0x1c0);
        lStack_b8 = *(longlong *)(lVar10 + 0x1c0);
        fVar4 = *(float *)(lVar10 + 0x1c8);
        uStack_b0 = *(undefined8 *)(lVar10 + 0x1c8);
        auVar2._4_4_ = -(uint)(*(float *)(lVar10 + 0x1c4) != *(float *)(lVar10 + 0x184));
        auVar2._0_4_ = -(uint)(fVar3 != *(float *)(lVar10 + 0x180));
        auVar2._8_4_ = -(uint)(fVar4 != *(float *)(lVar10 + 0x188));
        auVar2._12_4_ = -(uint)(*(float *)(lVar10 + 0x1cc) != *(float *)(lVar10 + 0x18c));
        iVar7 = movmskps((int)*(longlong *)(lVar16 + 0x30),auVar2);
        lVar16 = lVar10;
        if (iVar7 != 0) {
          *(float *)(lVar10 + 0x180) = fVar3;
          *(float *)(lVar10 + 0x184) = *(float *)(lVar10 + 0x1c4);
          *(float *)(lVar10 + 0x188) = fVar4;
          *(float *)(lVar10 + 0x18c) = *(float *)(lVar10 + 0x1cc);
          puVar11 = (undefined8 *)FUN_142c5d000(&lStack_b8,auStack_78);
          *(undefined8 *)(lVar10 + 400) = *puVar11;
          *(undefined4 *)(lVar10 + 0x198) = *(undefined4 *)(puVar11 + 1);
          lVar16 = param_1[0x26];
        }
        uVar19 = (undefined4)*(undefined8 *)(lVar10 + 400);
        uVar20 = (undefined4)((ulonglong)*(undefined8 *)(lVar10 + 400) >> 0x20);
        puVar1 = (undefined4 *)(lVar10 + 0x198);
        lVar10 = lVar16;
        uStack_88 = *puVar1;
      }
      uStack_90 = CONCAT44(uVar20,uVar19);
      if (lVar10 == 0) {
        uVar22 = _DAT_148218630;
        uVar19 = _DAT_148218638;
      }
      else {
        uVar22 = *(undefined8 *)(lVar10 + 0x1d0);
        uVar19 = *(undefined4 *)(lVar10 + 0x1d8);
      }
      uStack_84 = uVar22;
      uStack_7c = uVar19;
      if ((param_1[0x9b] == 0) || (lVar10 = FUN_14271f760(), lVar10 == 0)) {
LAB_141c5cc98:
        lVar16 = 0;
      }
      else {
        lVar16 = param_1[0x9b];
        if ((*(int *)(lVar16 + 0x38) < *(int *)(lVar10 + 0x38)) ||
           (*(longlong *)(*(longlong *)(lVar16 + 0x30) + (longlong)*(int *)(lVar10 + 0x38) * 8) !=
            lVar10 + 0x30)) goto LAB_141c5cc98;
      }
      uVar12 = FUN_144680860(auStack_68);
      lVar9 = FUN_1442e74f0(lVar9,lVar16,&uStack_84,&uStack_90,uVar12,pcVar21,uVar22,uVar19);
      param_1[0x9c] = lVar9;
      if (lVar9 != 0) {
        if (param_1[0x54] != 0) {
          FUN_143f840a0(lVar9,param_1[0x54] + 0x1c0,0,0,uVar12 & 0xffffffffffffff00);
        }
      }
    }
  }
  if (param_1[0x9c] != 0) {
    FUN_141748280(param_1[0x9c],1);
  }
  goto LAB_141c5cd19;
}

