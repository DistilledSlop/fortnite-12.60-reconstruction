
longlong * FUN_141c7c5c0(longlong param_1,longlong *param_2)

{
  undefined8 *puVar1;
  int iVar2;
  bool bVar3;
  float fVar4;
  float fVar5;
  float fVar6;
  float fVar7;
  float fVar8;
  float fVar9;
  longlong lVar10;
  float *pfVar11;
  int *piVar12;
  undefined8 uVar13;
  longlong *plVar14;
  longlong *plVar15;
  undefined8 *puVar16;
  char cVar17;
  float fVar18;
  float fVar19;
  float fVar20;
  undefined1 auStackX_10 [8];
  wchar_t *pwStackX_18;
  undefined8 uStackX_20;
  undefined8 uStack_148;
  float fStack_140;
  undefined8 uStack_13c;
  float fStack_134;
  float fStack_130;
  float fStack_128;
  float fStack_124;
  float fStack_120;
  float fStack_11c;
  undefined8 uStack_118;
  undefined8 uStack_108;
  float fStack_100;
  undefined8 uStack_f8;
  float fStack_f0;
  undefined8 auStack_e8 [2];
  float fStack_d8;
  float fStack_cc;
  
  pwStackX_18 = L"STAT_Find_Volume";
  if (DAT_1482472e4 != '\0') {
    pwStackX_18 = (wchar_t *)0x0;
  }
  if ((param_2 != (longlong *)0x0) && (*(int *)(param_1 + 0x280) != 0)) {
    lVar10 = FUN_142752f80();
    if ((*(int *)(param_2[2] + 0x38) < *(int *)(lVar10 + 0x38)) ||
       (*(longlong *)(*(longlong *)(param_2[2] + 0x30) + (longlong)*(int *)(lVar10 + 0x38) * 8) !=
        lVar10 + 0x30)) {
      cVar17 = FUN_141c56eb0(param_2);
      if ((cVar17 == '\0') && (cVar17 = FUN_141c56e70(param_2), cVar17 == '\0')) {
        uVar13 = 1;
      }
      else {
        uVar13 = 0;
      }
LAB_141c7c71e:
      pfVar11 = (float *)(**(code **)(*param_2 + 0x548))(param_2,&uStack_148,uVar13,0);
    }
    else {
      lVar10 = FUN_141c07130(param_2);
      if (lVar10 == 0) {
        uVar13 = 0;
        goto LAB_141c7c71e;
      }
      fStack_130 = (float)CONCAT31(fStack_130._1_3_,1);
      fStack_f0 = *(float *)(lVar10 + 0x108) - *(float *)(lVar10 + 0x114);
      fStack_100 = *(float *)(lVar10 + 0x108) + *(float *)(lVar10 + 0x114);
      uStack_148 = CONCAT44(*(float *)(lVar10 + 0x104) - *(float *)(lVar10 + 0x110),
                            *(float *)(lVar10 + 0x100) - *(float *)(lVar10 + 0x10c));
      fStack_140 = fStack_f0;
      fStack_134 = fStack_100;
      pfVar11 = (float *)&uStack_148;
      uStack_13c = CONCAT44(*(float *)(lVar10 + 0x104) + *(float *)(lVar10 + 0x110),
                            *(float *)(lVar10 + 0x100) + *(float *)(lVar10 + 0x10c));
    }
    uVar13 = *(undefined8 *)(pfVar11 + 4);
    plVar15 = (longlong *)0x0;
    fVar4 = *pfVar11;
    fVar5 = pfVar11[1];
    fVar6 = pfVar11[2];
    fVar7 = pfVar11[3];
    puVar16 = *(undefined8 **)(param_1 + 0x278);
    puVar1 = puVar16 + *(int *)(param_1 + 0x280);
    fStack_128 = fVar4;
    fStack_124 = fVar5;
    fStack_120 = fVar6;
    fStack_11c = fVar7;
    uStack_118 = uVar13;
    if (puVar16 != puVar1) {
      uStack_118._4_4_ = (float)((ulonglong)uVar13 >> 0x20);
      fVar9 = uStack_118._4_4_;
      uStack_118._0_4_ = (float)uVar13;
      fVar8 = (float)uStack_118;
      do {
        plVar14 = (longlong *)*puVar16;
        if (plVar14 != (longlong *)0x0) {
          if (param_2 != plVar14) {
            FUN_142ee3b30(&uStackX_20,param_2);
            auStack_e8[0] = uStackX_20;
            piVar12 = (int *)FUN_140782160(plVar14 + 99,auStackX_10,auStack_e8);
            iVar2 = *piVar12;
            cVar17 = iVar2 != -1;
            lVar10 = FUN_144720ae0();
            if ((*(int *)(param_2[2] + 0x38) < *(int *)(lVar10 + 0x38)) ||
               (*(longlong *)
                 (*(longlong *)(param_2[2] + 0x30) + (longlong)*(int *)(lVar10 + 0x38) * 8) !=
                lVar10 + 0x30)) {
              if (iVar2 == -1) {
                fVar19 = fVar7 - fVar4;
                fVar18 = fVar8 - fVar5;
                fVar20 = fVar9 - fVar6;
                fStack_134 = fVar20 * 0.5;
                uStack_13c = CONCAT44(fVar18 * 0.5,fVar19 * 0.5);
                fStack_140 = fVar6 + fStack_134;
                uStack_148 = CONCAT44(fVar5 + fVar18 * 0.5,fVar19 * 0.5 + fVar4);
                fStack_130 = SQRT((fVar19 * fVar19 + fVar18 * fVar18 + fVar20 * fVar20) * 0.25);
                fStack_d8 = fStack_134;
                fStack_cc = fStack_140;
                cVar17 = FUN_141c6a510(plVar14,param_2,&uStack_148,1);
                goto LAB_141c7c911;
              }
            }
            else {
LAB_141c7c911:
              if (cVar17 == '\0') goto LAB_141c7c964;
            }
            if (plVar15 == (longlong *)0x0) {
              plVar15 = plVar14;
            }
            else {
              lVar10 = FUN_141c75610(plVar15,&uStack_148);
              uVar13 = *(undefined8 *)(lVar10 + 0xc);
              lVar10 = FUN_141c75610(plVar14,&uStack_148);
              uStack_f8 = *(undefined8 *)(lVar10 + 0xc);
              uStack_108 = uVar13;
              if ((float)uStack_f8 < (float)uVar13) {
                uStack_108._4_4_ = (float)((ulonglong)uVar13 >> 0x20);
                uStack_f8._4_4_ = (float)((ulonglong)uStack_f8 >> 0x20);
                bVar3 = uStack_f8._4_4_ < uStack_108._4_4_;
                if (bVar3) {
                  plVar15 = plVar14;
                }
              }
            }
          }
        }
LAB_141c7c964:
        puVar16 = puVar16 + 1;
      } while (puVar16 != puVar1);
      if (plVar15 != (longlong *)0x0) goto LAB_141c7ca17;
    }
    lVar10 = FUN_144720ae0();
    if ((*(int *)(param_2[2] + 0x38) < *(int *)(lVar10 + 0x38)) ||
       (*(longlong *)(*(longlong *)(param_2[2] + 0x30) + (longlong)*(int *)(lVar10 + 0x38) * 8) !=
        lVar10 + 0x30)) goto LAB_141c7ca17;
    if (DAT_14812316c == '\0') {
      uVar13 = FUN_142b39170();
      plVar14 = (longlong *)FUN_142ec9900(param_2,uVar13);
      if (plVar14 != (longlong *)0x0) {
        DAT_14812316c = 1;
        plVar15 = (longlong *)(**(code **)(*plVar14 + 0x10))(plVar14);
        DAT_14812316c = '\0';
      }
      goto LAB_141c7ca17;
    }
  }
  plVar15 = (longlong *)0x0;
LAB_141c7ca17:
  if ((DAT_1482472e4 != '\0') && (pwStackX_18 != (wchar_t *)0x0)) {
    FUN_142cd3770(&pwStackX_18);
  }
  return plVar15;
}

