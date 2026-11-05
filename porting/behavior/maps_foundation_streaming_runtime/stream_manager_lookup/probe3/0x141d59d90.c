
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_141d59d90(undefined8 param_1,longlong *param_2,undefined8 param_3,longlong *param_4,
                  undefined1 param_5,longlong *param_6)

{
  char cVar1;
  byte bVar2;
  int iVar3;
  longlong lVar4;
  longlong lVar5;
  undefined8 uVar6;
  undefined8 *puVar7;
  undefined8 *puVar8;
  undefined8 uVar9;
  longlong *plVar10;
  longlong *plVar11;
  int iVar12;
  undefined *puVar13;
  undefined8 uStack_194;
  undefined4 uStack_18c;
  undefined4 uStack_188;
  undefined4 uStack_184;
  undefined4 uStack_180;
  undefined4 uStack_17c;
  undefined4 uStack_178;
  undefined4 uStack_174;
  undefined4 uStack_170;
  longlong *plStack_168;
  longlong *plStack_160;
  longlong *plStack_158;
  longlong *plStack_150;
  undefined1 *apuStack_148 [2];
  longlong lStack_138;
  undefined **ppuStack_128;
  code *pcStack_120;
  longlong lStack_118;
  longlong lStack_110;
  undefined1 *apuStack_108 [2];
  longlong lStack_f8;
  undefined **ppuStack_e8;
  code *pcStack_e0;
  undefined4 uStack_d8;
  undefined4 uStack_d4;
  undefined4 uStack_d0;
  undefined4 uStack_cc;
  undefined1 auStack_c8 [64];
  undefined1 auStack_88 [72];
  
  iVar3 = (int)((ulonglong)param_3 >> 0x20);
  iVar12 = (int)param_3;
  if (iVar12 == _DAT_1478c3a20 && iVar3 == _DAT_1478c3a24) {
    cVar1 = FUN_1443b1f50(param_2);
    if (cVar1 != '\0') {
      lVar4 = param_2[0x15];
      if (lVar4 == 0) {
        lVar4 = FUN_1440b3260(param_2);
      }
      lVar5 = FUN_1409f0710(param_1);
      if (lVar4 == lVar5) {
        plVar10 = (longlong *)param_2[0x14];
        if (plVar10 != (longlong *)0x0) {
          lVar4 = FUN_1447ae380();
          if ((((*(int *)(lVar4 + 0x38) <= *(int *)(plVar10[2] + 0x38)) &&
               (*(longlong *)
                 (*(longlong *)(plVar10[2] + 0x30) + (longlong)*(int *)(lVar4 + 0x38) * 8) ==
                lVar4 + 0x30)) && (cVar1 = (**(code **)(*plVar10 + 0x6b0))(plVar10), cVar1 != '\0'))
             && (cVar1 = (**(code **)(*plVar10 + 0x6b8))(plVar10), cVar1 != '\0'))
          goto LAB_141d5a4c4;
        }
        lVar4 = FUN_144114bf0(param_2);
        if ((lVar4 != 0) && (plVar10 = *(longlong **)(lVar4 + 0xa0), plVar10 != (longlong *)0x0)) {
          lVar4 = FUN_1447ae380();
          if (((*(int *)(lVar4 + 0x38) <= *(int *)(plVar10[2] + 0x38)) &&
              ((*(longlong *)
                 (*(longlong *)(plVar10[2] + 0x30) + (longlong)*(int *)(lVar4 + 0x38) * 8) ==
                lVar4 + 0x30 && (cVar1 = (**(code **)(*plVar10 + 0x6b0))(plVar10), cVar1 != '\0'))))
             && (cVar1 = (**(code **)(*plVar10 + 0x6b8))(plVar10), cVar1 != '\0'))
          goto LAB_141d5a4c4;
        }
        lVar4 = FUN_141384ca0(0xc0);
        if (lVar4 == 0) {
LAB_141d5a480:
          FUN_1409f1030(param_1,0);
        }
        else {
          uVar6 = FUN_141dbe930(lVar4,param_2,param_3);
          FUN_1409f1030(param_1,uVar6);
        }
      }
    }
LAB_141d5a4c4:
    if (*param_4 == 0) goto LAB_141d5a4df;
    plVar10 = (longlong *)param_4[2];
  }
  else {
    if (iVar12 != _DAT_1478c3f20 || iVar3 != _DAT_1478c3f24) {
      if (iVar12 != _DAT_1478c38e0 || iVar3 != _DAT_1478c38e4) {
        if (iVar12 == _DAT_1478c3b60 && iVar3 == _DAT_1478c3b64) {
          puVar7 = (undefined8 *)FUN_141384ca0(0xe0);
          if (puVar7 == (undefined8 *)0x0) goto LAB_141d5a2e0;
          plStack_168 = (longlong *)FUN_141d28c50(auStack_c8,param_6);
          plStack_160 = (longlong *)FUN_141d28c50(auStack_88,param_4);
          lStack_138 = plStack_168[2];
          apuStack_148[0] = (undefined1 *)*plStack_168;
          plStack_168[2] = 0;
          plStack_150 = plStack_168 + 4;
          ppuStack_128 = (undefined **)*plStack_150;
          pcStack_120 = (code *)plStack_168[5];
          lStack_118 = plStack_168[6];
          lStack_110 = plStack_168[7];
          if (apuStack_148[0] != (undefined1 *)0x0) {
            *plStack_168 = 0;
          }
          lStack_f8 = plStack_160[2];
          apuStack_108[0] = (undefined1 *)*plStack_160;
          plStack_160[2] = 0;
          plStack_158 = plStack_160 + 4;
          ppuStack_e8 = (undefined **)*plStack_158;
          pcStack_e0 = (code *)plStack_160[5];
          uStack_d8 = (undefined4)plStack_160[6];
          uStack_d4 = *(undefined4 *)((longlong)plStack_160 + 0x34);
          uStack_d0 = (undefined4)plStack_160[7];
          uStack_cc = *(undefined4 *)((longlong)plStack_160 + 0x3c);
          if (apuStack_108[0] != (undefined1 *)0x0) {
            *plStack_160 = 0;
          }
          FUN_141d2b670(puVar7,param_2,param_3,apuStack_108,param_5,apuStack_148);
          lVar4 = puVar7[2];
          *puVar7 = &PTR_FUN_145f98a28;
          puVar7[0x1a] = param_1;
          lVar5 = FUN_141bdf240(lVar4);
          if (*(char *)(lVar5 + 0x18) == '\0') {
            lVar4 = *(longlong *)(lVar4 + 0x130);
            if (lVar4 == 0) {
              uStack_194 = _DAT_148218630;
              uStack_18c = _DAT_148218638;
            }
            else {
              uStack_194 = *(undefined8 *)(lVar4 + 0x1d0);
              uStack_18c = *(undefined4 *)(lVar4 + 0x1d8);
            }
            uStack_178 = uStack_194._4_4_;
            uStack_174 = uStack_18c;
            uStack_170 = CONCAT31(uStack_170._1_3_,1);
            *(undefined4 *)(puVar7 + 0x16) = (undefined4)uStack_194;
            *(undefined4 *)((longlong)puVar7 + 0xb4) = uStack_194._4_4_;
            *(undefined4 *)(puVar7 + 0x17) = uStack_18c;
            *(undefined4 *)((longlong)puVar7 + 0xbc) = (undefined4)uStack_194;
            uStack_188 = (undefined4)uStack_194;
            uStack_184 = uStack_194._4_4_;
            uStack_180 = uStack_18c;
            uStack_17c = (undefined4)uStack_194;
            puVar7[0x18] = CONCAT44(uStack_18c,uStack_194._4_4_);
          }
          else {
            puVar13 = (undefined *)(*(longlong *)(lVar4 + 0x130) + 0x1c0);
            if (*(longlong *)(lVar4 + 0x130) == 0) {
              puVar13 = &DAT_1482184f0;
            }
            FUN_142c60b50(lVar5,&uStack_188,puVar13);
            *(undefined4 *)(puVar7 + 0x16) = uStack_188;
            *(undefined4 *)((longlong)puVar7 + 0xb4) = uStack_184;
            *(undefined4 *)(puVar7 + 0x17) = uStack_180;
            *(undefined4 *)((longlong)puVar7 + 0xbc) = uStack_17c;
            puVar7[0x18] = CONCAT44(uStack_174,uStack_178);
          }
          *(undefined4 *)(puVar7 + 0x19) = uStack_170;
          *(undefined4 *)((longlong)puVar7 + 0xc4) = 0;
          *(undefined4 *)(puVar7 + 0x17) = 0;
          if (*plStack_160 != 0) {
            plVar10 = plStack_158;
            if ((longlong *)plStack_160[2] != (longlong *)0x0) {
              plVar10 = (longlong *)plStack_160[2];
            }
            (**(code **)(*plVar10 + 0x10))();
          }
          if (*plStack_168 == 0) goto LAB_141d5a2e3;
          plVar10 = plStack_150;
          if ((longlong *)plStack_168[2] != (longlong *)0x0) {
            plVar10 = (longlong *)plStack_168[2];
          }
          (**(code **)(*plVar10 + 0x10))();
          FUN_1409f1030(param_1,puVar7);
        }
        else {
          if (iVar12 == _DAT_1478c3ca0 && iVar3 == _DAT_1478c3ca4) {
            puVar7 = (undefined8 *)FUN_141384ca0(0xe0);
            if (puVar7 == (undefined8 *)0x0) goto LAB_141d5a2e0;
            apuStack_108[0] = (undefined1 *)0x0;
            pcStack_120 = FUN_141d39dd0;
            lStack_f8 = 0;
            ppuStack_128 = &PTR_FUN_1459ccad0;
            lStack_138 = 0;
            apuStack_148[0] = &LAB_1406fbcc0;
            FUN_141d2aea0(puVar7,param_2,param_3,apuStack_148,0,apuStack_108);
            puVar7[0x17] = 0;
            *puVar7 = &PTR_FUN_145f98a28;
            puVar7[0x18] = 0;
            puVar7[0x19] = 0;
            *(undefined4 *)(puVar7 + 0x1a) = 0;
            *(undefined1 *)((longlong)puVar7 + 0xd4) = 0;
            if ((DAT_1481261a5 != '\0') && (lVar4 = FUN_141991c10(param_2,0), lVar4 != 0)) {
              DAT_1481261a5 = '\0';
              if (1e-08 < ABS(*(float *)(lVar4 + 0x6ac) - 1.0)) {
                _DAT_1478c370c = *(float *)(lVar4 + 0x6ac);
                FUN_1409f1030(param_1,puVar7);
                goto LAB_141d5a4c4;
              }
            }
            goto LAB_141d5a2e3;
          }
          if (iVar12 == _DAT_1478c4060 && iVar3 == _DAT_1478c4064) {
            if (0 < _DAT_1481261a0) {
              lVar4 = FUN_141384ca0(0xd0);
              if (lVar4 == 0) goto LAB_141d5a480;
              uVar6 = FUN_141d28c50(auStack_88,param_6);
              uVar9 = FUN_141d28c50(auStack_c8,param_4);
              uVar6 = FUN_141d2b480(lVar4,param_2,param_3,uVar9,param_5,uVar6);
              FUN_1409f1030(param_1,uVar6);
            }
          }
          else {
            uVar6 = FUN_141d28c50(auStack_88,param_6);
            uVar9 = FUN_141d28c50(auStack_c8,param_4);
            FUN_1409f1420(param_1,param_2,param_3,uVar9,param_5,uVar6);
          }
        }
        goto LAB_141d5a4c4;
      }
      puVar7 = (undefined8 *)FUN_141384ca0(0xd0);
      if (puVar7 == (undefined8 *)0x0) {
LAB_141d5a2e0:
        puVar7 = (undefined8 *)0x0;
      }
      else {
        lStack_f8 = 0;
        pcStack_e0 = FUN_141d58800;
        lStack_138 = 0;
        ppuStack_e8 = &PTR_FUN_1459ccad0;
        apuStack_108[0] = &LAB_141d3a240;
        pcStack_120 = FUN_141d3a140;
        ppuStack_128 = &PTR_FUN_1459ccad0;
        apuStack_148[0] = &LAB_1406fbcc0;
        FUN_141d2aea0(puVar7,param_2,param_3,apuStack_148,1,apuStack_108);
        *(undefined1 *)(puVar7 + 0x18) = 0;
        *puVar7 = &PTR_FUN_145f98a30;
        lVar4 = param_2[0x4f];
        if (lVar4 == 0) {
LAB_141d5a023:
          lVar4 = 0;
        }
        else {
          lVar5 = FUN_1409e4e60();
          if ((*(int *)(*(longlong *)(lVar4 + 0x10) + 0x38) < *(int *)(lVar5 + 0x38)) ||
             (*(longlong *)
               (*(longlong *)(*(longlong *)(lVar4 + 0x10) + 0x30) +
               (longlong)*(int *)(lVar5 + 0x38) * 8) != lVar5 + 0x30)) goto LAB_141d5a023;
        }
        puVar7[0x17] = lVar4;
        bVar2 = *(byte *)((longlong)param_2 + 0x1344) & 1;
        *(byte *)((longlong)puVar7 + 0xc1) = bVar2;
        *(undefined1 *)((longlong)puVar7 + 0xc2) = *(undefined1 *)((longlong)param_2 + 0x1e93);
        if ((bVar2 == 0) && (_DAT_1481261b4 != 0)) {
          uVar6 = (**(code **)(*param_2 + 0x150))(param_2);
          puVar8 = (undefined8 *)FUN_1409e3b20(uVar6);
          (**(code **)*puVar8)(puVar8,puVar7[0x17]);
          FUN_1409f1030(param_1,puVar7);
          goto LAB_141d5a4c4;
        }
      }
LAB_141d5a2e3:
      FUN_1409f1030(param_1,puVar7);
      goto LAB_141d5a4c4;
    }
    if (*param_4 == 0) goto LAB_141d5a4df;
    plVar10 = (longlong *)param_4[2];
  }
  plVar11 = param_4 + 4;
  if (plVar10 != (longlong *)0x0) {
    plVar11 = plVar10;
  }
  (**(code **)(*plVar11 + 0x10))();
LAB_141d5a4df:
  if (*param_6 != 0) {
    plVar10 = param_6 + 4;
    if ((longlong *)param_6[2] != (longlong *)0x0) {
      plVar10 = (longlong *)param_6[2];
    }
    (**(code **)(*plVar10 + 0x10))();
  }
  return;
}

