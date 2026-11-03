
void FUN_141c8baa0(longlong param_1)

{
  uint uVar1;
  longlong lVar2;
  int iVar3;
  uint *puVar4;
  int iVar5;
  undefined1 auStackX_8 [8];
  uint uStack_a8;
  uint uStack_a4;
  uint *puStack_a0;
  undefined4 uStack_98;
  int iStack_94;
  int iStack_90;
  undefined4 uStack_8c;
  longlong *plStack_88;
  undefined8 uStack_80;
  uint *puStack_78;
  undefined8 uStack_70;
  undefined1 auStack_68 [16];
  longlong *plStack_58;
  char cStack_50;
  char cStack_4f;
  longlong *plStack_48;
  undefined8 uStack_40;
  uint *puStack_38;
  undefined1 auStack_30 [16];
  undefined8 uStack_20;
  
  uStack_a4 = 1;
  plStack_88 = (longlong *)(param_1 + 0x128);
  uStack_a8 = 0;
  iVar5 = *(int *)(param_1 + 0x150);
  puStack_a0 = (uint *)(param_1 + 0x138);
  iStack_94 = 0;
  iStack_90 = 0;
  if (iVar5 != 0) {
    puVar4 = puStack_a0;
    if (*(uint **)(param_1 + 0x148) != (uint *)0x0) {
      puVar4 = *(uint **)(param_1 + 0x148);
    }
    uVar1 = *puVar4;
    iStack_90 = 0;
    uStack_a8 = 0;
    while (uVar1 == 0) {
      uStack_a8 = uStack_a8 + 1;
      iStack_90 = iStack_90 + 0x20;
      if ((int)((iVar5 + -1 >> 0x1f & 0x1fU) + iVar5 + -1) >> 5 < (int)uStack_a8)
      goto LAB_141c8bb61;
      uVar1 = puVar4[(int)uStack_a8];
    }
    uStack_a4 = -uVar1 & uVar1;
    iVar3 = 0x1f;
    if (uStack_a4 != 0) {
      for (; uStack_a4 >> iVar3 == 0; iVar3 = iVar3 + -1) {
      }
    }
    if (uStack_a4 == 0) {
      iVar3 = 0x20;
    }
    else {
      iVar3 = 0x1f - iVar3;
    }
    iStack_94 = (iStack_90 - iVar3) + 0x1f;
    if (iVar5 < iStack_94) {
LAB_141c8bb61:
      iStack_94 = iVar5;
    }
  }
  uStack_98 = 0xffffffff;
  uStack_70 = CONCAT44(iStack_94,0xffffffff);
  auStack_30._8_4_ = iStack_90;
  auStack_30._0_8_ = uStack_70;
  auStack_30._12_4_ = uStack_8c;
  uStack_80 = CONCAT44(uStack_a4,uStack_a8);
  cStack_50 = '\0';
  cStack_4f = '\0';
  uStack_20._0_4_ = SUB84(plStack_88,0);
  uStack_20._4_4_ = (undefined4)((ulonglong)plStack_88 >> 0x20);
  auStack_68._8_4_ = (undefined4)uStack_20;
  auStack_68._0_8_ = auStack_30._8_8_;
  auStack_68._12_4_ = uStack_20._4_4_;
  if (iStack_94 < *(int *)(param_1 + 0x150)) {
    iVar5 = iStack_94;
    puStack_78 = puStack_a0;
    plStack_58 = plStack_88;
    plStack_48 = plStack_88;
    uStack_40 = uStack_80;
    puStack_38 = puStack_a0;
    uStack_20 = plStack_88;
    do {
      lVar2 = *plStack_88;
      if (*(char *)(lVar2 + 0xc + (longlong)iVar5 * 0x18) == '\0') {
        FUN_141541af0(param_1 + 0x318,&uStack_a8);
        if (puStack_a0 == (uint *)0x0) {
          puVar4 = (uint *)0x0;
        }
        else {
          *puStack_a0 = *(uint *)(lVar2 + (longlong)iVar5 * 0x18 + 8);
          puStack_a0[1] = 0xffffffff;
          puVar4 = puStack_a0;
        }
        FUN_1408a3bc0(param_1 + 0x318,auStackX_8,*puVar4,puVar4,uStack_a8,0);
        FUN_14133f630(auStack_68._8_8_,iVar5);
        cStack_50 = '\x01';
      }
      uStack_70 = CONCAT44(uStack_70._4_4_,(uint)uStack_70 & ~uStack_80._4_4_);
      FUN_140687d00(&uStack_80);
      iVar5 = uStack_70._4_4_;
    } while (uStack_70._4_4_ < (int)puStack_78[6]);
    if ((cStack_50 != '\0') && (cStack_4f != '\0')) {
      FUN_141c69450(plStack_58,(int)plStack_58[1] - *(int *)((longlong)plStack_58 + 0x34),1);
    }
  }
  return;
}

