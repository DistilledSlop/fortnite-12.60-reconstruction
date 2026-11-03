
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined1 FUN_141c6a510(longlong param_1,longlong param_2,float *param_3,char param_4)

{
  undefined8 uVar1;
  undefined8 uVar2;
  bool bVar3;
  bool bVar4;
  undefined1 uVar5;
  int *piVar6;
  float *pfVar7;
  undefined8 *puVar8;
  longlong lVar9;
  float fVar10;
  float fVar11;
  float fVar12;
  float fVar13;
  float fVar14;
  float fVar15;
  undefined1 auStackX_10 [8];
  wchar_t *pwStack_f8;
  undefined8 uStack_f0;
  float fStack_e8;
  undefined8 uStack_e0;
  float fStack_d8;
  undefined1 auStack_c4 [172];
  
  pwStack_f8 = L"STAT_Is_Actor_Within_Volume";
  if (DAT_1482472e4 != '\0') {
    pwStack_f8 = (wchar_t *)0x0;
  }
  if (param_2 != 0) {
    if ((1e-08 < SQRT(param_3[3] * param_3[3] + param_3[4] * param_3[4] + param_3[5] * param_3[5]))
       || (1e-08 < ABS(param_3[6]))) {
      bVar3 = false;
    }
    else {
      bVar3 = true;
    }
    FUN_142ee3b30(&uStack_f0);
    uStack_e0 = uStack_f0;
    piVar6 = (int *)FUN_140782160(param_1 + 0x318,auStackX_10,&uStack_e0);
    if (*piVar6 != -1) {
      uVar5 = 1;
      goto LAB_141c6a8ca;
    }
    if ((bVar3) || (param_4 == '\0')) {
      lVar9 = FUN_144720ae0();
      if ((*(int *)(*(longlong *)(param_2 + 0x10) + 0x38) < *(int *)(lVar9 + 0x38)) ||
         (*(longlong *)
           (*(longlong *)(*(longlong *)(param_2 + 0x10) + 0x30) +
           (longlong)*(int *)(lVar9 + 0x38) * 8) != lVar9 + 0x30)) {
        lVar9 = *(longlong *)(param_2 + 0x130);
        if (lVar9 == 0) {
          uStack_e0 = _DAT_148218630;
          fStack_d8 = _DAT_148218638;
        }
        else {
          uStack_e0 = *(undefined8 *)(lVar9 + 0x1d0);
          fStack_d8 = *(float *)(lVar9 + 0x1d8);
        }
        uStack_f0 = uStack_e0;
        fStack_e8 = fStack_d8;
        uVar5 = FUN_141c82210(param_1,&uStack_f0);
        goto LAB_141c6a8ca;
      }
    }
    else if (*(char *)(param_1 + 0x560) == '\x01') {
      pfVar7 = (float *)FUN_141c756c0(param_1,&uStack_e0);
      fVar15 = *param_3 - param_3[3];
      fVar14 = *param_3 + param_3[3];
      fVar13 = param_3[1] + param_3[4];
      fVar11 = param_3[2] + param_3[5];
      fVar10 = param_3[1] - param_3[4];
      fVar12 = param_3[2] - param_3[5];
      if (((((fVar15 < *pfVar7 - 175.0) || (pfVar7[3] + 175.0 < fVar15)) ||
           (fVar10 < pfVar7[1] - 175.0)) ||
          ((pfVar7[4] + 175.0 < fVar10 || (fVar12 < pfVar7[2] - 175.0)))) ||
         (pfVar7[5] + 175.0 < fVar12)) {
        bVar3 = false;
      }
      else {
        bVar3 = true;
      }
      if (((fVar14 < *pfVar7 - 175.0) || (pfVar7[3] + 175.0 < fVar14)) ||
         ((fVar13 < pfVar7[1] - 175.0 ||
          (((pfVar7[4] + 175.0 < fVar13 || (fVar11 < pfVar7[2] - 175.0)) ||
           (pfVar7[5] + 175.0 < fVar11)))))) {
        bVar4 = false;
      }
      else {
        bVar4 = true;
      }
      if ((bVar3) && (bVar4)) {
        uVar5 = 1;
        goto LAB_141c6a8ca;
      }
    }
    else if (*(char *)(param_1 + 0x560) == '\0') {
      puVar8 = (undefined8 *)FUN_141c75610(param_1,auStack_c4);
      uVar1 = *puVar8;
      fStack_e8 = *(float *)(puVar8 + 1);
      uVar2 = *(undefined8 *)param_3;
      fStack_d8 = param_3[2];
      uStack_f0 = uVar1;
      uStack_e0 = uVar2;
      if (param_3[6] <= *(float *)(puVar8 + 3) + 0.0001) {
        uStack_e0._0_4_ = (float)uVar2;
        fVar10 = (*(float *)(puVar8 + 3) + 0.0001) - param_3[6];
        uStack_f0._0_4_ = (float)uVar1;
        uStack_e0._4_4_ = (float)((ulonglong)uVar2 >> 0x20);
        uStack_f0._4_4_ = (float)((ulonglong)uVar1 >> 0x20);
        uVar5 = ((float)uStack_e0 - (float)uStack_f0) * ((float)uStack_e0 - (float)uStack_f0) +
                (uStack_e0._4_4_ - uStack_f0._4_4_) * (uStack_e0._4_4_ - uStack_f0._4_4_) +
                (fStack_d8 - fStack_e8) * (fStack_d8 - fStack_e8) <= fVar10 * fVar10;
        goto LAB_141c6a8ca;
      }
    }
  }
  uVar5 = 0;
LAB_141c6a8ca:
  if ((DAT_1482472e4 != '\0') && (pwStack_f8 != (wchar_t *)0x0)) {
    FUN_142cd3770(&pwStack_f8);
  }
  return uVar5;
}

