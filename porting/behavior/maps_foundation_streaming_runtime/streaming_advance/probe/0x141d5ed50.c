
void FUN_141d5ed50(longlong param_1,undefined8 *param_2)

{
  undefined8 *puVar1;
  undefined8 uVar2;
  longlong *plVar3;
  longlong *plVar4;
  undefined8 *puVar5;
  wchar_t *pwStackX_8;
  longlong lStackX_10;
  undefined8 uStack_28;
  undefined4 uStack_20;

  uStack_28 = *param_2;
  uStack_20 = *(undefined4 *)(param_2 + 1);
  FUN_141d58c30((undefined4)uStack_28,&uStack_28);
  FUN_142cb25f0("SignificanceManager");
  pwStackX_8 = L"STAT_FortSignificanceManager_Update";
  plVar4 = (longlong *)&DAT_1478c37b0;
  if (DAT_1482472e4 != '\0') {
    pwStackX_8 = (wchar_t *)0x0;
  }
  do {
    if (*plVar4 != 0) {
      plVar3 = plVar4 + 4;
      if ((longlong *)plVar4[2] != (longlong *)0x0) {
        plVar3 = (longlong *)plVar4[2];
      }
      lStackX_10 = param_1;
      uVar2 = (**(code **)(*plVar3 + 8))();
      (*(code *)*plVar4)(uVar2,&lStackX_10);
    }
    plVar4 = plVar4 + 0x28;
  } while ((longlong)plVar4 < 0x1478c41b0);
  uStack_28 = *(undefined8 *)(param_1 + 0x1d8);
  uStack_20 = *(undefined4 *)(param_1 + 0x1e0);
  FUN_1409f1a90(param_1,&uStack_28);
  plVar4 = (longlong *)&DAT_1478c37f0;
  do {
    if (*plVar4 != 0) {
      plVar3 = plVar4 + 4;
      if ((longlong *)plVar4[2] != (longlong *)0x0) {
        plVar3 = (longlong *)plVar4[2];
      }
      lStackX_10 = param_1;
      uVar2 = (**(code **)(*plVar3 + 8))();
      (*(code *)*plVar4)(uVar2,&lStackX_10);
    }
    plVar4 = plVar4 + 0x28;
  } while ((longlong)plVar4 < 0x1478c41f0);
  *(undefined1 *)(param_1 + 0x159) = 0;
  if ((DAT_1482472e4 != '\0') && (pwStackX_8 != (wchar_t *)0x0)) {
    FUN_142cd3770(&pwStackX_8);
  }
  FUN_142cb7760("SignificanceManager");
  puVar5 = *(undefined8 **)(param_1 + 0x1c8);
  puVar1 = puVar5 + *(int *)(param_1 + 0x1d0);
  for (; puVar5 != puVar1; puVar5 = puVar5 + 1) {
    FMemory_Free((undefined *)*puVar5);
  }
  *(undefined4 *)(param_1 + 0x1d0) = 0;
  if (*(int *)(param_1 + 0x1d4) < 0) {
    FUN_1406d9a30(param_1 + 0x1c8,0);
  }
  return;
}
