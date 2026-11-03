
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined * FUN_141c8abe0(longlong *param_1,undefined *param_2)

{
  bool bVar1;
  char cVar2;
  uint uVar3;
  int iVar4;
  longlong lVar5;
  int *piVar6;
  longlong lVar7;
  int *piVar8;
  undefined *puVar9;
  wchar_t *pwStackX_10;
  undefined8 uStackX_18;
  undefined1 auStackX_20 [8];
  undefined8 uStack_48;
  undefined *puStack_40;
  undefined8 uStack_38;
  undefined *puStack_30;
  longlong *plStack_28;
  
  puVar9 = (undefined *)0x0;
  pwStackX_10 = L"STAT_Volume_RemoveActor";
  if (DAT_1482472e4 != '\0') {
    pwStackX_10 = (wchar_t *)0x0;
  }
  if (param_2 == (undefined *)0x0) {
LAB_141c8aff7:
    uVar3 = 0;
  }
  else {
    FUN_141c64450(param_1,param_2,0);
    if ((char)param_1[0x1e] != '\x03') {
      FUN_141c93b20(param_1,param_2,0);
      goto LAB_141c8affb;
    }
    FUN_142ee3b30(&uStackX_18);
    uVar3 = FUN_1407834a0(param_1 + 99,&uStackX_18);
    if ((int)uVar3 < 1) {
      if (2 < DAT_1481348e0) {
        uStackX_18 = *(undefined8 *)(param_2 + 0x18);
        FUN_142ce9f90();
        FMsg_Logf_Internal((byte *)0x0,0,&DAT_1481348e4,3,
                           (ushort *)
                           L"FortVolume::RemoveActor - attempted to remove an actor (%s) that wasn\'t inside this volume"
                          );
        if (puStack_40 != (undefined *)0x0) {
          FMemory_Free(puStack_40);
        }
      }
      goto LAB_141c8aff7;
    }
    lVar5 = FUN_1447ae380();
    puStack_30 = puVar9;
    if ((*(int *)(lVar5 + 0x38) <= *(int *)(*(longlong *)(param_2 + 0x10) + 0x38)) &&
       (*(longlong *)
         (*(longlong *)(*(longlong *)(param_2 + 0x10) + 0x30) + (longlong)*(int *)(lVar5 + 0x38) * 8
         ) == lVar5 + 0x30)) {
      puStack_30 = param_2;
    }
    uStack_38 = *(undefined8 *)(param_2 + 0x130);
    puStack_40 = param_2;
    plStack_28 = param_1;
    FUN_1406a9dc0(param_1 + 0xa6,&puStack_40);
    if (_DAT_1478c33fc == 1) {
      lVar5 = FUN_142732ac0();
      if ((*(int *)(lVar5 + 0x38) <= *(int *)(*(longlong *)(param_2 + 0x10) + 0x38)) &&
         (*(longlong *)
           (*(longlong *)(*(longlong *)(param_2 + 0x10) + 0x30) +
           (longlong)*(int *)(lVar5 + 0x38) * 8) == lVar5 + 0x30)) {
        piVar6 = (int *)FUN_141c55d00(param_1 + 0x6d,param_2[0x3eb]);
        *piVar6 = *piVar6 + -1;
      }
    }
    FUN_142cd49c0(&uStack_48,"AthenaAIRequiresNavigation",1);
    FUN_142cd49c0(auStackX_20,"AthenaAIRequiresAIDirector",1);
    lVar5 = FUN_141ed1800();
    bVar1 = FName_EqualsEName(*(int *)(lVar5 + 0x2050),0);
    iVar4 = *(int *)(lVar5 + 0x2054);
    if (iVar4 != 0 || !bVar1) {
      piVar8 = (int *)param_1[0x4a];
      piVar6 = piVar8 + (longlong)(int)param_1[0x4b] * 2;
      if (piVar8 != piVar6) {
        do {
          if (*piVar8 == *(int *)(lVar5 + 0x2050) && piVar8[1] == iVar4) goto LAB_141c8adef;
          piVar8 = piVar8 + 2;
        } while (piVar8 != piVar6);
      }
      piVar8 = (int *)param_1[0x4c];
      piVar6 = piVar8 + (longlong)(int)param_1[0x4d] * 2;
      if (piVar8 != piVar6) {
        do {
          if (*piVar8 == *(int *)(lVar5 + 0x2050) && piVar8[1] == iVar4) goto LAB_141c8adef;
          piVar8 = piVar8 + 2;
        } while (piVar8 != piVar6);
      }
    }
  }
  goto LAB_141c8aff9;
LAB_141c8aecc:
  lVar5 = (**(code **)(*param_1 + 0x150))(param_1);
  if ((lVar5 != 0) && (lVar5 = *(longlong *)(lVar5 + 0x128), lVar5 != 0)) {
    lVar7 = FUN_1428ee970();
    if ((*(int *)(lVar7 + 0x38) <= *(int *)(*(longlong *)(lVar5 + 0x10) + 0x38)) &&
       ((*(longlong *)
          (*(longlong *)(*(longlong *)(lVar5 + 0x10) + 0x30) + (longlong)*(int *)(lVar7 + 0x38) * 8)
         == lVar7 + 0x30 && (lVar5 = FUN_141e9a660(lVar5), lVar5 != 0)))) {
      *(uint *)(lVar5 + 0xb60) = *(uint *)(lVar5 + 0xb60) | 2;
    }
  }
  goto LAB_141c8af20;
LAB_141c8adef:
  cVar2 = FUN_143f67a70(param_2);
  if ((cVar2 != '\0') &&
     (*(int *)((longlong)param_1 + 0x4ec) = *(int *)((longlong)param_1 + 0x4ec) + -1,
     *(int *)((longlong)param_1 + 0x4ec) < 1)) {
    lVar5 = FUN_141ed1800();
    bVar1 = FName_EqualsEName(*(int *)(lVar5 + 0x2050),0);
    iVar4 = *(int *)(lVar5 + 0x2054);
    if (iVar4 != 0 || !bVar1) {
      piVar8 = (int *)param_1[0x4a];
      piVar6 = piVar8 + (longlong)(int)param_1[0x4b] * 2;
      if (piVar8 != piVar6) {
        do {
          if (*piVar8 == *(int *)(lVar5 + 0x2050) && piVar8[1] == iVar4) goto LAB_141c8aecc;
          piVar8 = piVar8 + 2;
        } while (piVar8 != piVar6);
      }
      piVar8 = (int *)param_1[0x4c];
      piVar6 = piVar8 + (longlong)(int)param_1[0x4d] * 2;
      if (piVar8 != piVar6) {
        do {
          if (*piVar8 == *(int *)(lVar5 + 0x2050) && piVar8[1] == iVar4) goto LAB_141c8aecc;
          piVar8 = piVar8 + 2;
        } while (piVar8 != piVar6);
      }
    }
  }
LAB_141c8af20:
  cVar2 = FUN_143f67a70(param_2,uStack_48);
  if (cVar2 != '\0') {
    iVar4 = (int)param_1[0x9d] + -1;
    *(int *)(param_1 + 0x9d) = iVar4;
    if ((param_1[0x9c] != 0) && (iVar4 < 1)) {
      FUN_141748280(param_1[0x9c],0);
    }
  }
  lVar5 = FUN_1427a1790();
  if ((*(int *)(lVar5 + 0x38) <= *(int *)(*(longlong *)(param_2 + 0x10) + 0x38)) &&
     (*(longlong *)
       (*(longlong *)(*(longlong *)(param_2 + 0x10) + 0x30) + (longlong)*(int *)(lVar5 + 0x38) * 8)
      == lVar5 + 0x30)) {
    FUN_1422c39e0(param_2,0);
  }
LAB_141c8aff9:
  puVar9 = (undefined *)(ulonglong)uVar3;
LAB_141c8affb:
  if ((DAT_1482472e4 != '\0') && (pwStackX_10 != (wchar_t *)0x0)) {
    FUN_142cd3770(&pwStackX_10);
  }
  return puVar9;
}

