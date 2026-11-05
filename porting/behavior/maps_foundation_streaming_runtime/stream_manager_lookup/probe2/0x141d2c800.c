
/* WARNING: Removing unreachable block (ram,0x000141d2d614) */
/* WARNING: Removing unreachable block (ram,0x000141d2d42b) */
/* WARNING: Removing unreachable block (ram,0x000141d2d43a) */
/* WARNING: Removing unreachable block (ram,0x000141d2d448) */
/* WARNING: Removing unreachable block (ram,0x000141d2d41e) */
/* WARNING: Removing unreachable block (ram,0x000141d2d630) */
/* WARNING: Removing unreachable block (ram,0x000141d2d621) */
/* WARNING: Removing unreachable block (ram,0x000141d2d63e) */
/* WARNING: Removing unreachable block (ram,0x000141d2d45a) */
/* WARNING: Removing unreachable block (ram,0x000141d2d650) */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_141d2c800(undefined8 *param_1)

{
  undefined8 uVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  undefined4 uVar5;
  undefined4 uVar6;
  undefined4 uVar7;
  undefined4 uVar8;
  undefined4 uVar9;
  undefined8 *puVar10;
  int iVar11;
  char cVar12;
  undefined8 *puVar13;
  longlong *plVar14;
  longlong lVar15;
  longlong *plVar16;
  undefined *Ptr;
  int iVar17;
  longlong lVar18;
  undefined1 auStack_108 [32];
  undefined *puStack_e8;
  undefined8 uStack_d8;
  longlong *plStack_d0;
  int iStack_c8;
  undefined1 *puStack_b8;
  undefined8 uStack_b0;
  longlong *plStack_a8;
  undefined8 uStack_a0;
  undefined4 uStack_98;
  undefined4 uStack_94;
  undefined4 uStack_90;
  undefined4 uStack_8c;
  undefined4 uStack_88;
  undefined4 uStack_84;
  undefined4 uStack_80;
  undefined4 uStack_7c;
  longlong *plStack_68;
  undefined8 uStack_60;
  ulonglong uStack_38;
  
  uStack_38 = _DAT_1480c4288 ^ (ulonglong)auStack_108;
  FUN_1409ef640();
  *(uint *)(param_1 + 5) = *(uint *)(param_1 + 5) & 0xfffffffd;
  *param_1 = &DAT_145f987a8;
  param_1[0x29] = 0;
  param_1[0x2d] = 0;
  param_1[0x2e] = 0;
  param_1[0x2f] = 0;
  param_1[0x30] = 0;
  param_1[0x33] = 0;
  *(undefined4 *)(param_1 + 0x34) = 0;
  *(undefined4 *)((longlong)param_1 + 0x1a4) = 0x80;
  *(undefined4 *)(param_1 + 0x35) = 0xffffffff;
  *(undefined4 *)((longlong)param_1 + 0x1ac) = 0;
  param_1[0x37] = 0;
  *(undefined4 *)(param_1 + 0x38) = 0;
  param_1[0x39] = 0;
  param_1[0x3a] = 0;
  param_1[0x3b] = 0;
  param_1[0x3c] = 0;
  param_1[0x3d] = 0;
  *(undefined1 *)((longlong)param_1 + 0x15a) = 1;
  cVar12 = FUN_142ecc600(param_1,0x30);
  if ((cVar12 == '\0') && (_DAT_14823d260 != 0)) {
    puStack_e8 = &DAT_148247648;
    FUN_142c71b90(_DAT_14823d260,L"FortSignificanceManager",
                  L"AthenaBuildingFoundationStreamInDistance",&DAT_1478c36cc);
    puStack_e8 = &DAT_148247648;
    FUN_142c71b90(_DAT_14823d260,L"FortSignificanceManager",
                  L"AthenaBuildingFoundationStreamOutDistance",&DAT_1478c36d0);
  }
  cVar12 = FUN_142ecc600(param_1,0x30);
  if (cVar12 != '\0') {
    FUN_142cd49c0(&uStack_d8,"PlayerPawn",1);
    uVar9 = uRam00000001478c386c;
    uVar8 = uRam00000001478c3868;
    uVar7 = uRam00000001478c3864;
    uVar6 = _DAT_1478c3860;
    uVar5 = uRam00000001478c385c;
    uVar4 = uRam00000001478c3858;
    uVar3 = uRam00000001478c3854;
    uVar2 = _DAT_1478c3850;
    uStack_60 = uRam00000001478c3848;
    plStack_68 = _DAT_1478c3840;
    uVar1 = uRam00000001478c3838;
    puStack_b8 = _DAT_1478c3830;
    _DAT_1478c37a0 = uStack_d8;
    _DAT_1478c3830 = &LAB_1406fbcc0;
    uRam00000001478c3838 = uStack_b0;
    _DAT_1478c3840 = (longlong *)0x0;
    uRam00000001478c3848 = uStack_a0;
    _DAT_1478c3850 = 0x459ccad0;
    uRam00000001478c3854 = 1;
    uRam00000001478c3858 = 0x41d38cf0;
    uRam00000001478c385c = 1;
    _DAT_1478c3860 = uStack_88;
    uRam00000001478c3864 = uStack_84;
    uRam00000001478c3868 = uStack_80;
    uRam00000001478c386c = uStack_7c;
    uStack_b0 = uVar1;
    plStack_a8 = plStack_68;
    uStack_a0 = uStack_60;
    _uStack_98 = CONCAT44(uVar3,uVar2);
    _uStack_90 = CONCAT44(uVar5,uVar4);
    uStack_88 = uVar6;
    uStack_84 = uVar7;
    uStack_80 = uVar8;
    uStack_7c = uVar9;
    if (puStack_b8 != (undefined1 *)0x0) {
      plVar14 = (longlong *)&uStack_98;
      if (plStack_68 != (longlong *)0x0) {
        plVar14 = plStack_68;
      }
      (**(code **)(*plVar14 + 0x10))();
    }
    FUN_142cd49c0(&uStack_d8,"AIPawn",1);
    uVar9 = uRam00000001478c396c;
    uVar8 = uRam00000001478c3968;
    uVar7 = uRam00000001478c3964;
    uVar6 = _DAT_1478c3960;
    uVar5 = uRam00000001478c395c;
    uVar4 = uRam00000001478c3958;
    uVar3 = uRam00000001478c3954;
    uVar2 = _DAT_1478c3950;
    uStack_60 = uRam00000001478c3948;
    plStack_68 = _DAT_1478c3940;
    uVar1 = uRam00000001478c3938;
    puStack_b8 = _DAT_1478c3930;
    _DAT_1478c38e0 = uStack_d8;
    _DAT_1478c3930 = &LAB_141d3a260;
    uRam00000001478c3938 = uStack_b0;
    _DAT_1478c3940 = (longlong *)0x0;
    uRam00000001478c3948 = uStack_a0;
    _DAT_1478c3950 = 0x459ccad0;
    uRam00000001478c3954 = 1;
    uRam00000001478c3958 = 0x41d32060;
    uRam00000001478c395c = 1;
    _DAT_1478c3960 = uStack_88;
    uRam00000001478c3964 = uStack_84;
    uRam00000001478c3968 = uStack_80;
    uRam00000001478c396c = uStack_7c;
    uStack_b0 = uVar1;
    plStack_a8 = plStack_68;
    uStack_a0 = uStack_60;
    _uStack_98 = CONCAT44(uVar3,uVar2);
    _uStack_90 = CONCAT44(uVar5,uVar4);
    uStack_88 = uVar6;
    uStack_84 = uVar7;
    uStack_80 = uVar8;
    uStack_7c = uVar9;
    if (puStack_b8 != (undefined1 *)0x0) {
      plVar14 = (longlong *)&uStack_98;
      if (plStack_68 != (longlong *)0x0) {
        plVar14 = plStack_68;
      }
      (**(code **)(*plVar14 + 0x10))();
    }
    FUN_142cd49c0(&uStack_d8,&DAT_145f98c0c,1);
    uVar9 = uRam00000001478c3a6c;
    uVar8 = uRam00000001478c3a68;
    uVar7 = uRam00000001478c3a64;
    uVar6 = _DAT_1478c3a60;
    uVar5 = uRam00000001478c3a5c;
    uVar4 = uRam00000001478c3a58;
    uVar3 = uRam00000001478c3a54;
    uVar2 = _DAT_1478c3a50;
    uStack_60 = uRam00000001478c3a48;
    plStack_68 = _DAT_1478c3a40;
    uVar1 = uRam00000001478c3a38;
    puStack_b8 = _DAT_1478c3a30;
    _DAT_1478c3a20 = uStack_d8;
    _DAT_1478c3a30 = &LAB_141d3a260;
    uRam00000001478c3a38 = uStack_b0;
    _DAT_1478c3a40 = (longlong *)0x0;
    uRam00000001478c3a48 = uStack_a0;
    _DAT_1478c3a50 = 0x459ccad0;
    uRam00000001478c3a54 = 1;
    uRam00000001478c3a58 = 0x41de2550;
    uRam00000001478c3a5c = 1;
    _DAT_1478c3a60 = uStack_88;
    uRam00000001478c3a64 = uStack_84;
    uRam00000001478c3a68 = uStack_80;
    uRam00000001478c3a6c = uStack_7c;
    uStack_b0 = uVar1;
    plStack_a8 = plStack_68;
    uStack_a0 = uStack_60;
    _uStack_98 = CONCAT44(uVar3,uVar2);
    _uStack_90 = CONCAT44(uVar5,uVar4);
    uStack_88 = uVar6;
    uStack_84 = uVar7;
    uStack_80 = uVar8;
    uStack_7c = uVar9;
    if (puStack_b8 != (undefined1 *)0x0) {
      plVar14 = (longlong *)&uStack_98;
      if (plStack_68 != (longlong *)0x0) {
        plVar14 = plStack_68;
      }
      (**(code **)(*plVar14 + 0x10))();
    }
    puVar13 = (undefined8 *)FUN_141384ca0(0x14);
    if (puVar13 == (undefined8 *)0x0) {
      puVar13 = (undefined8 *)0x0;
    }
    else {
      *puVar13 = 0x3f800000;
      puVar13[1] = 0;
      *(undefined4 *)(puVar13 + 2) = 0;
    }
    puVar10 = _DAT_1478c3b30;
    _DAT_1478c3b30 = puVar13;
    FMemory_Free((undefined *)puVar10);
    FMemory_Free((undefined *)0x0);
    FUN_142cd49c0(&uStack_d8,"BuildingFoundation",1);
    uVar9 = uRam00000001478c3bac;
    uVar8 = uRam00000001478c3ba8;
    uVar7 = uRam00000001478c3ba4;
    uVar6 = _DAT_1478c3ba0;
    uVar5 = uRam00000001478c3b9c;
    uVar4 = uRam00000001478c3b98;
    uVar3 = uRam00000001478c3b94;
    uVar2 = _DAT_1478c3b90;
    uStack_60 = uRam00000001478c3b88;
    plStack_68 = _DAT_1478c3b80;
    uVar1 = uRam00000001478c3b78;
    puStack_b8 = _DAT_1478c3b70;
    _DAT_1478c3b60 = uStack_d8;
    _DAT_1478c3b70 = &LAB_141d3a260;
    uRam00000001478c3b78 = uStack_b0;
    _DAT_1478c3b80 = (longlong *)0x0;
    uRam00000001478c3b88 = uStack_a0;
    _DAT_1478c3b90 = 0x459ccad0;
    uRam00000001478c3b94 = 1;
    uRam00000001478c3b98 = 0x41d394f0;
    uRam00000001478c3b9c = 1;
    _DAT_1478c3ba0 = uStack_88;
    uRam00000001478c3ba4 = uStack_84;
    uRam00000001478c3ba8 = uStack_80;
    uRam00000001478c3bac = uStack_7c;
    uStack_b0 = uVar1;
    plStack_a8 = plStack_68;
    uStack_a0 = uStack_60;
    _uStack_98 = CONCAT44(uVar3,uVar2);
    _uStack_90 = CONCAT44(uVar5,uVar4);
    uStack_88 = uVar6;
    uStack_84 = uVar7;
    uStack_80 = uVar8;
    uStack_7c = uVar9;
    if (puStack_b8 != (undefined1 *)0x0) {
      plVar14 = (longlong *)&uStack_98;
      if (plStack_68 != (longlong *)0x0) {
        plVar14 = plStack_68;
      }
      (**(code **)(*plVar14 + 0x10))();
    }
    uVar9 = uRam00000001478c3c2c;
    uVar8 = uRam00000001478c3c28;
    uVar7 = uRam00000001478c3c24;
    uVar6 = _DAT_1478c3c20;
    uVar5 = uRam00000001478c3c1c;
    uVar4 = uRam00000001478c3c18;
    uVar3 = uRam00000001478c3c14;
    uVar2 = _DAT_1478c3c10;
    uStack_60 = uRam00000001478c3c08;
    plStack_68 = _DAT_1478c3c00;
    uVar1 = uRam00000001478c3bf8;
    puStack_b8 = _DAT_1478c3bf0;
    _DAT_1478c3bf0 = &LAB_1406fbcc0;
    uRam00000001478c3bf8 = uStack_b0;
    _DAT_1478c3c00 = (longlong *)0x0;
    uRam00000001478c3c08 = uStack_a0;
    _DAT_1478c3c10 = 0x459ccad0;
    uRam00000001478c3c14 = 1;
    uRam00000001478c3c18 = 0x41d398d0;
    uRam00000001478c3c1c = 1;
    _DAT_1478c3c20 = uStack_88;
    uRam00000001478c3c24 = uStack_84;
    uRam00000001478c3c28 = uStack_80;
    uRam00000001478c3c2c = uStack_7c;
    uStack_b0 = uVar1;
    plStack_a8 = plStack_68;
    uStack_a0 = uStack_60;
    _uStack_98 = CONCAT44(uVar3,uVar2);
    _uStack_90 = CONCAT44(uVar5,uVar4);
    uStack_88 = uVar6;
    uStack_84 = uVar7;
    uStack_80 = uVar8;
    uStack_7c = uVar9;
    if (puStack_b8 != (undefined1 *)0x0) {
      plVar14 = (longlong *)&uStack_98;
      if (plStack_68 != (longlong *)0x0) {
        plVar14 = plStack_68;
      }
      (**(code **)(*plVar14 + 0x10))();
    }
    uVar9 = uRam00000001478c3c6c;
    uVar8 = uRam00000001478c3c68;
    uVar7 = uRam00000001478c3c64;
    uVar6 = _DAT_1478c3c60;
    uVar5 = uRam00000001478c3c5c;
    uVar4 = uRam00000001478c3c58;
    uVar3 = uRam00000001478c3c54;
    uVar2 = _DAT_1478c3c50;
    uStack_60 = uRam00000001478c3c48;
    plStack_68 = _DAT_1478c3c40;
    uVar1 = uRam00000001478c3c38;
    puStack_b8 = _DAT_1478c3c30;
    _DAT_1478c3c30 = &LAB_141d3a240;
    uRam00000001478c3c38 = uStack_b0;
    _DAT_1478c3c40 = (longlong *)0x0;
    uRam00000001478c3c48 = uStack_a0;
    _DAT_1478c3c50 = 0x459ccad0;
    uRam00000001478c3c54 = 1;
    uRam00000001478c3c58 = 0x41d39420;
    uRam00000001478c3c5c = 1;
    DAT_1478c3b68 = 2;
    _DAT_1478c3c60 = uStack_88;
    uRam00000001478c3c64 = uStack_84;
    uRam00000001478c3c68 = uStack_80;
    uRam00000001478c3c6c = uStack_7c;
    uStack_b0 = uVar1;
    plStack_a8 = plStack_68;
    uStack_a0 = uStack_60;
    _uStack_98 = CONCAT44(uVar3,uVar2);
    _uStack_90 = CONCAT44(uVar5,uVar4);
    uStack_88 = uVar6;
    uStack_84 = uVar7;
    uStack_80 = uVar8;
    uStack_7c = uVar9;
    if (puStack_b8 != (undefined1 *)0x0) {
      plVar14 = (longlong *)&uStack_98;
      if (plStack_68 != (longlong *)0x0) {
        plVar14 = plStack_68;
      }
      (**(code **)(*plVar14 + 0x10))();
    }
    puVar13 = (undefined8 *)FUN_141384ca0(0x10);
    if (puVar13 == (undefined8 *)0x0) {
      puVar13 = (undefined8 *)0x0;
    }
    else {
      *puVar13 = 0;
      puVar13[1] = 0;
    }
    puVar10 = _DAT_1478c3c70;
    _DAT_1478c3c70 = puVar13;
    FMemory_Free((undefined *)puVar10);
    FUN_142cd49c0(&uStack_d8,"HLODProxy",1);
    uVar9 = uRam00000001478c3fac;
    uVar8 = uRam00000001478c3fa8;
    uVar7 = uRam00000001478c3fa4;
    uVar6 = _DAT_1478c3fa0;
    uVar5 = uRam00000001478c3f9c;
    uVar4 = uRam00000001478c3f98;
    uVar3 = uRam00000001478c3f94;
    uVar2 = _DAT_1478c3f90;
    uStack_60 = uRam00000001478c3f88;
    plStack_68 = _DAT_1478c3f80;
    uVar1 = uRam00000001478c3f78;
    puStack_b8 = _DAT_1478c3f70;
    _DAT_1478c3f20 = uStack_d8;
    _DAT_1478c3f70 = &LAB_141d3a260;
    uRam00000001478c3f78 = uStack_b0;
    _DAT_1478c3f80 = (longlong *)0x0;
    uRam00000001478c3f88 = uStack_a0;
    _DAT_1478c3f90 = 0x459ccad0;
    uRam00000001478c3f94 = 1;
    uRam00000001478c3f98 = 0x41d519e0;
    uRam00000001478c3f9c = 1;
    _DAT_1478c3fa0 = uStack_88;
    uRam00000001478c3fa4 = uStack_84;
    uRam00000001478c3fa8 = uStack_80;
    uRam00000001478c3fac = uStack_7c;
    uStack_b0 = uVar1;
    plStack_a8 = plStack_68;
    uStack_a0 = uStack_60;
    _uStack_98 = CONCAT44(uVar3,uVar2);
    _uStack_90 = CONCAT44(uVar5,uVar4);
    uStack_88 = uVar6;
    uStack_84 = uVar7;
    uStack_80 = uVar8;
    uStack_7c = uVar9;
    if (puStack_b8 != (undefined1 *)0x0) {
      plVar14 = (longlong *)&uStack_98;
      if (plStack_68 != (longlong *)0x0) {
        plVar14 = plStack_68;
      }
      (**(code **)(*plVar14 + 0x10))();
    }
    FUN_142cd49c0(&uStack_d8,"AthenaPlayerPawn",1);
    uVar9 = uRam00000001478c3cec;
    uVar8 = uRam00000001478c3ce8;
    uVar7 = uRam00000001478c3ce4;
    uVar6 = _DAT_1478c3ce0;
    uVar5 = uRam00000001478c3cdc;
    uVar4 = uRam00000001478c3cd8;
    uVar3 = uRam00000001478c3cd4;
    uVar2 = _DAT_1478c3cd0;
    uStack_60 = uRam00000001478c3cc8;
    plStack_68 = _DAT_1478c3cc0;
    uVar1 = uRam00000001478c3cb8;
    puStack_b8 = _DAT_1478c3cb0;
    _DAT_1478c3ca0 = uStack_d8;
    _DAT_1478c3cb0 = &LAB_141d3a260;
    uRam00000001478c3cb8 = uStack_b0;
    _DAT_1478c3cc0 = (longlong *)0x0;
    uRam00000001478c3cc8 = uStack_a0;
    _DAT_1478c3cd0 = 0x459ccad0;
    uRam00000001478c3cd4 = 1;
    uRam00000001478c3cd8 = 0x41d38860;
    uRam00000001478c3cdc = 1;
    _DAT_1478c3ce0 = uStack_88;
    uRam00000001478c3ce4 = uStack_84;
    uRam00000001478c3ce8 = uStack_80;
    uRam00000001478c3cec = uStack_7c;
    uStack_b0 = uVar1;
    plStack_a8 = plStack_68;
    uStack_a0 = uStack_60;
    _uStack_98 = CONCAT44(uVar3,uVar2);
    _uStack_90 = CONCAT44(uVar5,uVar4);
    uStack_88 = uVar6;
    uStack_84 = uVar7;
    uStack_80 = uVar8;
    uStack_7c = uVar9;
    if (puStack_b8 != (undefined1 *)0x0) {
      plVar14 = (longlong *)&uStack_98;
      if (plStack_68 != (longlong *)0x0) {
        plVar14 = plStack_68;
      }
      (**(code **)(*plVar14 + 0x10))();
    }
    uVar9 = uRam00000001478c3d2c;
    uVar8 = uRam00000001478c3d28;
    uVar7 = uRam00000001478c3d24;
    uVar6 = _DAT_1478c3d20;
    uVar5 = uRam00000001478c3d1c;
    uVar4 = uRam00000001478c3d18;
    uVar3 = uRam00000001478c3d14;
    uVar2 = _DAT_1478c3d10;
    uStack_60 = uRam00000001478c3d08;
    plStack_68 = _DAT_1478c3d00;
    uVar1 = uRam00000001478c3cf8;
    puStack_b8 = _DAT_1478c3cf0;
    _DAT_1478c3cf0 = &LAB_141d3a260;
    uRam00000001478c3cf8 = uStack_b0;
    _DAT_1478c3d00 = (longlong *)0x0;
    uRam00000001478c3d08 = uStack_a0;
    _DAT_1478c3d10 = 0x459ccad0;
    uRam00000001478c3d14 = 1;
    uRam00000001478c3d18 = 0x41d385b0;
    uRam00000001478c3d1c = 1;
    _DAT_1478c3d20 = uStack_88;
    uRam00000001478c3d24 = uStack_84;
    uRam00000001478c3d28 = uStack_80;
    uRam00000001478c3d2c = uStack_7c;
    uStack_b0 = uVar1;
    plStack_a8 = plStack_68;
    uStack_a0 = uStack_60;
    _uStack_98 = CONCAT44(uVar3,uVar2);
    _uStack_90 = CONCAT44(uVar5,uVar4);
    uStack_88 = uVar6;
    uStack_84 = uVar7;
    uStack_80 = uVar8;
    uStack_7c = uVar9;
    if (puStack_b8 != (undefined1 *)0x0) {
      plVar14 = (longlong *)&uStack_98;
      if (plStack_68 != (longlong *)0x0) {
        plVar14 = plStack_68;
      }
      (**(code **)(*plVar14 + 0x10))();
    }
    FUN_142cd49c0(&uStack_d8,"FortAthenaSKVehicle",1);
    uVar9 = uRam00000001478c3eac;
    uVar8 = uRam00000001478c3ea8;
    uVar7 = uRam00000001478c3ea4;
    uVar6 = _DAT_1478c3ea0;
    uVar5 = uRam00000001478c3e9c;
    uVar4 = uRam00000001478c3e98;
    uVar3 = uRam00000001478c3e94;
    uVar2 = _DAT_1478c3e90;
    uStack_60 = uRam00000001478c3e88;
    plStack_68 = _DAT_1478c3e80;
    uVar1 = uRam00000001478c3e78;
    puStack_b8 = _DAT_1478c3e70;
    _DAT_1478c3de0 = uStack_d8;
    _DAT_1478c3e70 = &LAB_1406fbcc0;
    uRam00000001478c3e78 = uStack_b0;
    _DAT_1478c3e80 = (longlong *)0x0;
    uRam00000001478c3e88 = uStack_a0;
    _DAT_1478c3e90 = 0x459ccad0;
    uRam00000001478c3e94 = 1;
    uRam00000001478c3e98 = 0x41d38b20;
    uRam00000001478c3e9c = 1;
    _DAT_1478c3ea0 = uStack_88;
    uRam00000001478c3ea4 = uStack_84;
    uRam00000001478c3ea8 = uStack_80;
    uRam00000001478c3eac = uStack_7c;
    uStack_b0 = uVar1;
    plStack_a8 = plStack_68;
    uStack_a0 = uStack_60;
    _uStack_98 = CONCAT44(uVar3,uVar2);
    _uStack_90 = CONCAT44(uVar5,uVar4);
    uStack_88 = uVar6;
    uStack_84 = uVar7;
    uStack_80 = uVar8;
    uStack_7c = uVar9;
    if (puStack_b8 != (undefined1 *)0x0) {
      plVar14 = (longlong *)&uStack_98;
      if (plStack_68 != (longlong *)0x0) {
        plVar14 = plStack_68;
      }
      (**(code **)(*plVar14 + 0x10))();
    }
    uVar9 = uRam00000001478c3e6c;
    uVar8 = uRam00000001478c3e68;
    uVar7 = uRam00000001478c3e64;
    uVar6 = _DAT_1478c3e60;
    uVar5 = uRam00000001478c3e5c;
    uVar4 = uRam00000001478c3e58;
    uVar3 = uRam00000001478c3e54;
    uVar2 = _DAT_1478c3e50;
    uStack_60 = uRam00000001478c3e48;
    plStack_68 = _DAT_1478c3e40;
    uVar1 = uRam00000001478c3e38;
    puStack_b8 = _DAT_1478c3e30;
    _DAT_1478c3e30 = &LAB_141d3a260;
    uRam00000001478c3e38 = uStack_b0;
    _DAT_1478c3e40 = (longlong *)0x0;
    uRam00000001478c3e48 = uStack_a0;
    _DAT_1478c3e50 = 0x459ccad0;
    uRam00000001478c3e54 = 1;
    uRam00000001478c3e58 = 0x41d38900;
    uRam00000001478c3e5c = 1;
    _DAT_1478c3e60 = uStack_88;
    uRam00000001478c3e64 = uStack_84;
    uRam00000001478c3e68 = uStack_80;
    uRam00000001478c3e6c = uStack_7c;
    uStack_b0 = uVar1;
    plStack_a8 = plStack_68;
    uStack_a0 = uStack_60;
    _uStack_98 = CONCAT44(uVar3,uVar2);
    _uStack_90 = CONCAT44(uVar5,uVar4);
    uStack_88 = uVar6;
    uStack_84 = uVar7;
    uStack_80 = uVar8;
    uStack_7c = uVar9;
    if (puStack_b8 != (undefined1 *)0x0) {
      plVar14 = (longlong *)&uStack_98;
      if (plStack_68 != (longlong *)0x0) {
        plVar14 = plStack_68;
      }
      (**(code **)(*plVar14 + 0x10))();
    }
    FUN_142cd49c0(&uStack_d8,"FortSprayDecal");
    uVar9 = uRam00000001478c412c;
    uVar8 = uRam00000001478c4128;
    uVar7 = uRam00000001478c4124;
    uVar6 = _DAT_1478c4120;
    uVar5 = uRam00000001478c411c;
    uVar4 = uRam00000001478c4118;
    uVar3 = uRam00000001478c4114;
    uVar2 = _DAT_1478c4110;
    uStack_60 = uRam00000001478c4108;
    plStack_68 = _DAT_1478c4100;
    uVar1 = uRam00000001478c40f8;
    puStack_b8 = _DAT_1478c40f0;
    _DAT_1478c4060 = uStack_d8;
    _DAT_1478c40f0 = &LAB_1406fbcc0;
    uRam00000001478c40f8 = uStack_b0;
    _DAT_1478c4100 = (longlong *)0x0;
    uRam00000001478c4108 = uStack_a0;
    _DAT_1478c4110 = 0x459ccad0;
    uRam00000001478c4114 = 1;
    uRam00000001478c4118 = 0x41d38d40;
    uRam00000001478c411c = 1;
    _DAT_1478c4120 = uStack_88;
    uRam00000001478c4124 = uStack_84;
    uRam00000001478c4128 = uStack_80;
    uRam00000001478c412c = uStack_7c;
    uStack_b0 = uVar1;
    plStack_a8 = plStack_68;
    uStack_a0 = uStack_60;
    _uStack_98 = CONCAT44(uVar3,uVar2);
    _uStack_90 = CONCAT44(uVar5,uVar4);
    uStack_88 = uVar6;
    uStack_84 = uVar7;
    uStack_80 = uVar8;
    uStack_7c = uVar9;
    if (puStack_b8 != (undefined1 *)0x0) {
      plVar14 = (longlong *)&uStack_98;
      if (plStack_68 != (longlong *)0x0) {
        plVar14 = plStack_68;
      }
      (**(code **)(*plVar14 + 0x10))();
    }
    uVar9 = uRam00000001478c40ec;
    uVar8 = uRam00000001478c40e8;
    uVar7 = uRam00000001478c40e4;
    uVar6 = _DAT_1478c40e0;
    uVar5 = uRam00000001478c40dc;
    uVar4 = uRam00000001478c40d8;
    uVar3 = uRam00000001478c40d4;
    uVar2 = _DAT_1478c40d0;
    uStack_60 = uRam00000001478c40c8;
    plStack_68 = _DAT_1478c40c0;
    uVar1 = uRam00000001478c40b8;
    puStack_b8 = _DAT_1478c40b0;
    _DAT_1478c40b0 = &LAB_141d3a260;
    uRam00000001478c40b8 = uStack_b0;
    _DAT_1478c40c0 = (longlong *)0x0;
    uRam00000001478c40c8 = uStack_a0;
    _DAT_1478c40d0 = 0x459ccad0;
    uRam00000001478c40d4 = 1;
    uRam00000001478c40d8 = 0x41d57dc0;
    uRam00000001478c40dc = 1;
    _DAT_1478c40e0 = uStack_88;
    uRam00000001478c40e4 = uStack_84;
    uRam00000001478c40e8 = uStack_80;
    uRam00000001478c40ec = uStack_7c;
    uStack_b0 = uVar1;
    plStack_a8 = plStack_68;
    uStack_a0 = uStack_60;
    _uStack_98 = CONCAT44(uVar3,uVar2);
    _uStack_90 = CONCAT44(uVar5,uVar4);
    uStack_88 = uVar6;
    uStack_84 = uVar7;
    uStack_80 = uVar8;
    uStack_7c = uVar9;
    if (puStack_b8 != (undefined1 *)0x0) {
      plVar14 = (longlong *)&uStack_98;
      if (plStack_68 != (longlong *)0x0) {
        plVar14 = plStack_68;
      }
      (**(code **)(*plVar14 + 0x10))();
    }
  }
  *(undefined4 *)(param_1 + 0x2a) = _DAT_1478c36cc;
  *(undefined4 *)((longlong)param_1 + 0x154) = _DAT_1478c36d0;
  _DAT_148127490 = 0;
  if (_DAT_148127494 != 0) {
    FUN_140694750(&DAT_148127488,0);
  }
  plVar14 = (longlong *)FMemory_Realloc((undefined *)0x0,0x30,0);
  if (plVar14 != (longlong *)0x0) {
    *plVar14 = (longlong)&DAT_1459bfd10;
    FUN_142ee3b30(plVar14 + 1);
    plVar14[2] = (longlong)&LAB_141d56ae0;
    lVar15 = FUN_142bad740();
    plVar14[4] = lVar15;
    *plVar14 = (longlong)&DAT_1459bfd68;
  }
  iVar11 = _DAT_1478c41f8;
  lVar15 = 0;
  if (plVar14 != (longlong *)0x0) {
    if ((_DAT_1478c4204 < 1) &&
       (_DAT_1478c4200 = _DAT_1478c4200 + -1, _DAT_1478c4200 <= _DAT_1478c41f8)) {
      iVar17 = 0;
      if (0 < _DAT_1478c41f8) {
        do {
          if (((*(int *)(_DAT_1478c41f0 + 8 + lVar15) == 0) ||
              (*(longlong **)(_DAT_1478c41f0 + lVar15) == (longlong *)0x0)) ||
             (cVar12 = (**(code **)(**(longlong **)(_DAT_1478c41f0 + lVar15) + 0x20))(),
             cVar12 != '\0')) {
            FUN_140690780(&DAT_1478c41f0,iVar17,1,1);
          }
          else {
            iVar17 = iVar17 + 1;
            lVar15 = lVar15 + 0x10;
          }
        } while (iVar17 < _DAT_1478c41f8);
      }
      _DAT_1478c4200 = _DAT_1478c41f8 * 2;
      if (_DAT_1478c4200 < 3) {
        _DAT_1478c4200 = 2;
      }
      if ((_DAT_1478c4200 < iVar11) && (_DAT_1478c41fc != _DAT_1478c41f8)) {
        FUN_1406911b0(&DAT_1478c41f0);
      }
    }
    plVar16 = (longlong *)(**(code **)(*plVar14 + 0x30))(plVar14);
    lVar18 = (longlong)_DAT_1478c41f8;
    lVar15 = *plVar16;
    _DAT_1478c41f8 = _DAT_1478c41f8 + 1;
    if (_DAT_1478c41fc < _DAT_1478c41f8) {
      FUN_140690dd0(&DAT_1478c41f0);
    }
    puVar13 = (undefined8 *)(lVar18 * 0x10 + _DAT_1478c41f0);
    *puVar13 = plVar14;
    *(undefined4 *)(puVar13 + 1) = 3;
  }
  param_1[0x29] = lVar15;
  if (_DAT_1481d47f8 == (longlong *)0x0) {
    FUN_142bcee40();
  }
  plVar14 = _DAT_1481d47f8;
  plStack_d0 = (longlong *)0x0;
  plVar16 = (longlong *)FMemory_Realloc((undefined *)0x0,0x30,0);
  iStack_c8 = 3;
  plStack_d0 = plVar16;
  if (plVar16 != (longlong *)0x0) {
    *plVar16 = (longlong)&DAT_1459bfd10;
    FUN_142ee3b30(plVar16 + 1,param_1);
    plVar16[2] = (longlong)FUN_141d427d0;
    lVar15 = FUN_142bad740();
    plVar16[4] = lVar15;
    *plVar16 = (longlong)&DAT_1459bfd68;
  }
  puVar13 = (undefined8 *)(**(code **)(*plVar14 + 0x58))(plVar14);
  param_1[0x3d] = *puVar13;
  if (iStack_c8 != 0) {
    if (plStack_d0 == (longlong *)0x0) goto LAB_141d2d53a;
    (**(code **)(*plStack_d0 + 0x38))();
    if (plStack_d0 != (longlong *)0x0) {
      plStack_d0 = (longlong *)FMemory_Realloc((undefined *)plStack_d0,0,0);
    }
    iStack_c8 = 0;
  }
  if (plStack_d0 != (longlong *)0x0) {
    FMemory_Free((undefined *)plStack_d0);
  }
LAB_141d2d53a:
  FUN_141d5f000();
  cVar12 = FUN_142ecc600(param_1,0x30);
  if (cVar12 == '\0') {
    plVar14 = (longlong *)FMemory_Realloc((undefined *)0x0,0x30,0);
    if (plVar14 != (longlong *)0x0) {
      *plVar14 = (longlong)&DAT_145b41508;
      FUN_142ee3b30(plVar14 + 1);
      plVar14[2] = (longlong)FUN_141d57ab0;
      lVar15 = FUN_142bad740();
      plVar14[4] = lVar15;
      *plVar14 = (longlong)&DAT_145b41560;
      FMulticastDelegateBase_CompactInvocationList(&DAT_147cdea30,true);
      (**(code **)(*plVar14 + 0x30))(plVar14);
      lVar15 = (longlong)_DAT_147cdea38;
      _DAT_147cdea38 = _DAT_147cdea38 + 1;
      if (_DAT_147cdea3c < _DAT_147cdea38) {
        FUN_140690dd0(&DAT_147cdea30);
      }
      puVar13 = (undefined8 *)(lVar15 * 0x10 + _DAT_147cdea30);
      *puVar13 = plVar14;
      *(undefined4 *)(puVar13 + 1) = 3;
    }
    plVar14 = (longlong *)FMemory_Realloc((undefined *)0x0,0x20,0);
    if (plVar14 == (longlong *)0x0) {
      Ptr = (undefined *)0x0;
      if ((_DAT_1480da210 != 0) && (_DAT_1480da208 != (longlong *)0x0)) {
        (**(code **)(*_DAT_1480da208 + 0x38))();
        if (_DAT_1480da208 != (longlong *)0x0) {
          _DAT_1480da208 = (longlong *)FMemory_Realloc((undefined *)_DAT_1480da208,0,0);
        }
        _DAT_1480da210 = 0;
      }
    }
    else {
      *plVar14 = (longlong)&DAT_1459a8380;
      plVar14[1] = (longlong)FUN_141d55f40;
      lVar15 = FUN_142bad740();
      plVar14[3] = lVar15;
      (**(code **)(*plVar14 + 0x40))(plVar14);
      (**(code **)(*plVar14 + 0x38))(plVar14);
      Ptr = FMemory_Realloc((undefined *)plVar14,0,0);
    }
    if (Ptr != (undefined *)0x0) {
      FMemory_Free(Ptr);
    }
  }
  _DAT_1478c370c = _DAT_1478c36f0;
  DAT_1481261a5 = DAT_1478c36ec;
  FUN_145803c70(uStack_38 ^ (ulonglong)auStack_108);
  return;
}

