
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined8 FUN_141d398d0(longlong param_1,float *param_2)

{
  float fVar1;
  float fVar2;
  longlong lVar3;
  undefined1 auVar4 [16];
  undefined1 auVar5 [16];
  undefined1 auVar6 [16];
  undefined1 auVar7 [16];
  uint uVar8;
  int iVar9;
  ulonglong uVar10;
  float *pfVar11;
  longlong lVar12;
  longlong lVar13;
  float fVar14;
  undefined8 uVar15;
  float fVar16;
  float fVar17;
  float fVar18;
  float fVar19;
  float fVar20;
  float fVar21;
  float fVar22;
  float fVar23;
  
  if ((DAT_1478c36ed == '\0') || (_DAT_1478c36cc <= 0.0)) {
    return 0x40000000;
  }
  if (_DAT_148126190 < 1) {
    uVar15 = FUN_141d39910();
    return uVar15;
  }
  lVar3 = *_DAT_1478c3c70;
  uVar10 = (ulonglong)(int)_DAT_1478c3c70[1];
  lVar13 = uVar10 * 0x60 + lVar3;
  if (lVar3 != lVar13) {
    pfVar11 = (float *)(lVar3 + 0x40);
    lVar12 = lVar3;
    do {
      auVar4._4_4_ = -(uint)(0.0001 < (float)((uint)(pfVar11[-3] - param_2[5]) &
                                             uRam0000000148126b54));
      auVar4._0_4_ = -(uint)(0.0001 < (float)((uint)(pfVar11[-4] - param_2[4]) & _DAT_148126b50));
      auVar4._8_4_ = -(uint)(0.0001 < (float)((uint)(pfVar11[-2] - param_2[6]) &
                                             uRam0000000148126b58));
      auVar4._12_4_ =
           -(uint)(0.0001 < (float)((uint)(pfVar11[-1] - param_2[7]) & uRam0000000148126b5c));
      uVar8 = movmskps((int)uVar10,auVar4);
      uVar10 = (ulonglong)uVar8;
      if (uVar8 == 0) {
        auVar5._4_4_ = -(uint)(0.0001 < (float)((uint)(pfVar11[-7] - param_2[1]) &
                                               uRam0000000148126b54));
        auVar5._0_4_ = -(uint)(0.0001 < (float)((uint)(pfVar11[-8] - *param_2) & _DAT_148126b50));
        auVar5._8_4_ = -(uint)(0.0001 < (float)((uint)(pfVar11[-6] - param_2[2]) &
                                               uRam0000000148126b58));
        auVar5._12_4_ =
             -(uint)(0.0001 < (float)((uint)(pfVar11[-5] - param_2[3]) & uRam0000000148126b5c));
        iVar9 = movmskps(0,auVar5);
        if (iVar9 != 0) {
          auVar6._4_4_ = -(uint)(0.0001 < (float)((uint)(pfVar11[-7] + param_2[1]) &
                                                 uRam0000000148126b54));
          auVar6._0_4_ = -(uint)(0.0001 < (float)((uint)(pfVar11[-8] + *param_2) & _DAT_148126b50));
          auVar6._8_4_ = -(uint)(0.0001 < (float)((uint)(pfVar11[-6] + param_2[2]) &
                                                 uRam0000000148126b58));
          auVar6._12_4_ =
               -(uint)(0.0001 < (float)((uint)(pfVar11[-5] + param_2[3]) & uRam0000000148126b5c));
          uVar8 = movmskps(iVar9,auVar6);
          uVar10 = (ulonglong)uVar8;
          if (uVar8 != 0) goto LAB_141d39b12;
        }
        auVar7._4_4_ = -(uint)(0.0001 < (float)((uint)(pfVar11[1] - param_2[9]) &
                                               uRam0000000148126b54));
        auVar7._0_4_ = -(uint)(0.0001 < (float)((uint)(*pfVar11 - param_2[8]) & _DAT_148126b50));
        auVar7._8_4_ = -(uint)(0.0001 < (float)((uint)(pfVar11[2] - param_2[10]) &
                                               uRam0000000148126b58));
        auVar7._12_4_ =
             -(uint)(0.0001 < (float)((uint)(pfVar11[3] - param_2[0xb]) & uRam0000000148126b5c));
        uVar8 = movmskps(0,auVar7);
        uVar10 = (ulonglong)uVar8;
        if (uVar8 == 0) {
          iVar9 = (int)((lVar12 - lVar3) / 0x60);
          if (iVar9 != -1) {
            fVar14 = 0.0;
            fVar16 = 0.0;
            pfVar11 = (float *)((longlong)iVar9 * 0x60 + lVar3);
            fVar20 = _DAT_1478c36cc * *pfVar11 * _DAT_1478c36cc * *pfVar11;
            if (0.0 < _DAT_1478c36d0) {
              fVar21 = _DAT_1478c36d0 * _DAT_1478c36d0;
            }
            else {
              fVar21 = 0.0;
            }
            fVar1 = pfVar11[0xc];
            fVar18 = pfVar11[0xd];
            fVar2 = pfVar11[0xe];
            fVar17 = 0.0;
            fVar19 = 0.0;
            fVar22 = *(float *)(param_1 + 0xb0);
            fVar23 = fVar22;
            if ((fVar1 < fVar22) ||
               (fVar23 = *(float *)(param_1 + 0xbc), *(float *)(param_1 + 0xbc) < fVar1)) {
              fVar17 = (fVar1 - fVar23) * (fVar1 - fVar23);
              fVar19 = fVar18;
            }
            fVar1 = *(float *)(param_1 + 0xb4);
            fVar23 = fVar1;
            if ((fVar18 < fVar1) ||
               (fVar23 = *(float *)(param_1 + 0xc0), *(float *)(param_1 + 0xc0) < fVar18)) {
              fVar17 = fVar17 + (fVar18 - fVar23) * (fVar18 - fVar23);
            }
            fVar18 = *(float *)(param_1 + 0xb8);
            fVar23 = fVar18;
            if ((fVar2 < fVar18) ||
               (fVar23 = *(float *)(param_1 + 0xc4), *(float *)(param_1 + 0xc4) < fVar2)) {
              fVar17 = fVar17 + (fVar2 - fVar23) * (fVar2 - fVar23);
            }
            if (fVar20 < fVar21) {
              fVar23 = pfVar11[1];
              fVar19 = 0.0;
              fVar2 = fVar22;
              if ((fVar23 < fVar22) ||
                 (fVar2 = *(float *)(param_1 + 0xbc), *(float *)(param_1 + 0xbc) < fVar23)) {
                fVar19 = (fVar23 - fVar2) * (fVar23 - fVar2);
              }
              fVar23 = pfVar11[2];
              fVar2 = fVar1;
              if ((fVar23 < fVar1) ||
                 (fVar2 = *(float *)(param_1 + 0xc0), *(float *)(param_1 + 0xc0) < fVar23)) {
                fVar19 = fVar19 + (fVar23 - fVar2) * (fVar23 - fVar2);
              }
              fVar23 = pfVar11[3];
              if ((fVar23 < fVar18) || (fVar18 = *(float *)(param_1 + 0xc4), fVar18 < fVar23)) {
                fVar19 = fVar19 + (fVar23 - fVar18) * (fVar23 - fVar18);
              }
              fVar23 = pfVar11[4];
              fVar18 = 0.0;
              if ((fVar23 < fVar22) || (fVar22 = *(float *)(param_1 + 0xbc), fVar22 < fVar23)) {
                fVar18 = (fVar23 - fVar22) * (fVar23 - fVar22);
              }
              fVar22 = pfVar11[5];
              if ((fVar22 < fVar1) || (fVar1 = *(float *)(param_1 + 0xc0), fVar1 < fVar22)) {
                fVar18 = fVar18 + (fVar22 - fVar1) * (fVar22 - fVar1);
              }
              fVar22 = pfVar11[6];
              fVar23 = *(float *)(param_1 + 0xb8);
              if ((fVar22 < fVar23) || (fVar23 = *(float *)(param_1 + 0xc4), fVar23 < fVar22)) {
                fVar18 = fVar18 + (fVar22 - fVar23) * (fVar22 - fVar23);
              }
              if (fVar20 <= fVar19) {
                if (fVar18 < fVar21) {
                  fVar14 = 1.0;
                  fVar16 = 0.0;
                }
              }
              else {
                fVar16 = 0.0;
                fVar14 = (fVar19 - fVar21) / (fVar20 - fVar21);
                fVar14 = fVar14 + fVar14;
              }
            }
            else if (fVar17 <= fVar20) {
              fVar14 = fVar17 / fVar20 + 2.0;
              fVar16 = fVar19;
            }
            if (*(char *)(*(longlong *)(param_1 + 0xd0) + 0x15a) != '\0') {
              return CONCAT44(fVar16,fVar14);
            }
            if (fVar14 < 2.0) {
              return CONCAT44(fVar16,fVar14);
            }
            return 0x3ffff972;
          }
          break;
        }
      }
LAB_141d39b12:
      lVar12 = lVar12 + 0x60;
      pfVar11 = pfVar11 + 0x18;
    } while (lVar12 != lVar13);
  }
  uVar15 = FUN_141d39910(param_1,param_2);
  return uVar15;
}

