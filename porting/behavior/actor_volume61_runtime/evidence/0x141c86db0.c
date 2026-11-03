
void FUN_141c86db0(longlong param_1)

{
  longlong *plVar1;
  int iVar2;
  uint uVar3;
  bool bVar4;
  int iVar5;
  longlong lVar6;
  ulonglong uVar7;
  ulonglong uVar8;
  uint *puVar9;
  ulonglong uVar10;
  undefined1 auStackX_8 [8];
  undefined1 *puStackX_10;
  undefined1 **ppuStackX_18;
  int iStack_78;
  uint uStack_74;
  int iStack_70;
  uint uStack_6c;
  undefined8 uStack_68;
  uint uStack_60;
  int iStack_5c;
  int iStack_58;
  undefined4 uStack_54;
  longlong *plStack_50;
  longlong *plStack_48;
  int iStack_40;
  uint uStack_3c;
  uint *puStack_38;
  undefined4 uStack_30;
  int iStack_2c;
  int iStack_28;
  undefined4 uStack_24;
  
  if (*(int *)(param_1 + 800) != *(int *)(param_1 + 0x34c)) {
    FUN_141c8baa0();
    puStackX_10 = auStackX_8;
    plVar1 = (longlong *)(param_1 + 0x318);
    if (0 < *(int *)(param_1 + 800) - *(int *)(param_1 + 0x34c)) {
      FUN_141c690a0(plVar1);
      ppuStackX_18 = &puStackX_10;
      FUN_141c56b70(*plVar1,*(int *)(param_1 + 800) - *(int *)(param_1 + 0x34c),auStackX_8[0],
                    &ppuStackX_18);
    }
    FUN_140726bb0(plVar1);
    iVar2 = *(int *)(param_1 + 0x340);
    puStack_38 = (uint *)(param_1 + 0x328);
    uVar8 = 0;
    iStack_78 = 0;
    uStack_74 = 1;
    uStack_68._4_4_ = 0;
    uStack_60 = 0;
    if (iVar2 != 0) {
      puVar9 = puStack_38;
      if (*(uint **)(param_1 + 0x338) != (uint *)0x0) {
        puVar9 = *(uint **)(param_1 + 0x338);
      }
      uVar3 = *puVar9;
      iStack_78 = 0;
      uStack_60 = 0;
      while (uVar3 == 0) {
        iStack_78 = iStack_78 + 1;
        uStack_60 = uStack_60 + 0x20;
        if ((int)((iVar2 + -1 >> 0x1f & 0x1fU) + iVar2 + -1) >> 5 < iStack_78) goto LAB_141c86ecf;
        uVar3 = puVar9[iStack_78];
      }
      uStack_74 = -uVar3 & uVar3;
      iVar5 = 0x1f;
      if (uStack_74 != 0) {
        for (; uStack_74 >> iVar5 == 0; iVar5 = iVar5 + -1) {
        }
      }
      if (uStack_74 == 0) {
        iVar5 = 0x20;
      }
      else {
        iVar5 = 0x1f - iVar5;
      }
      uStack_68._4_4_ = (uStack_60 - iVar5) + 0x1f;
      if (iVar2 < uStack_68._4_4_) {
LAB_141c86ecf:
        uStack_68._4_4_ = iVar2;
      }
    }
    uStack_30 = 0xffffffff;
    iStack_2c = uStack_68._4_4_;
    iStack_28 = uStack_60;
    uStack_24 = iStack_5c;
    iStack_40 = iStack_78;
    uStack_3c = uStack_74;
    iStack_70 = iStack_78;
    uStack_6c = uStack_74;
    iStack_58 = uStack_60;
    uStack_54 = iStack_5c;
    uStack_60 = 0xffffffff;
    iStack_5c = uStack_68._4_4_;
    bVar4 = uStack_68._4_4_ < iVar2;
    plStack_50 = plVar1;
    plStack_48 = plVar1;
    uStack_68 = puStack_38;
    if (bVar4) {
      do {
        FUN_141c86ff0(param_1,*(undefined4 *)(*plVar1 + (longlong)iStack_5c * 0xc));
        uStack_60 = uStack_60 & ~uStack_6c;
        FUN_140687d00(&iStack_70);
      } while (iStack_5c < (int)uStack_68[6]);
    }
    *(undefined4 *)(param_1 + 800) = 0;
    if (*(int *)(param_1 + 0x324) != 0) {
      FUN_140694750(plVar1,0);
    }
    *(undefined4 *)(param_1 + 0x348) = 0xffffffff;
    *(undefined4 *)(param_1 + 0x34c) = 0;
    *(undefined4 *)(param_1 + 0x340) = 0;
    if (*(int *)(param_1 + 0x344) != 0) {
      *(undefined4 *)(param_1 + 0x344) = 0;
      FUN_140690310(param_1 + 0x328,0);
    }
    if (0 < (int)*(uint *)(param_1 + 0x360)) {
      uVar10 = (ulonglong)*(uint *)(param_1 + 0x360);
      do {
        uVar7 = (longlong)*(int *)(param_1 + 0x360) - 1U & uVar8;
        lVar6 = param_1 + 0x350;
        if (*(longlong *)(param_1 + 0x358) != 0) {
          lVar6 = *(longlong *)(param_1 + 0x358);
        }
        uVar8 = uVar8 + 1;
        *(undefined4 *)(lVar6 + uVar7 * 4) = 0xffffffff;
        uVar10 = uVar10 - 1;
      } while (uVar10 != 0);
    }
  }
  return;
}

