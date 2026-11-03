
void FUN_141a24940(longlong *param_1,longlong param_2,undefined8 *param_3)

{
  longlong lVar1;
  longlong lVar2;
  longlong lVar3;
  longlong lVar4;
  
  param_1[0x5be] = param_2;
  FGameplayTagContainer_CopyAssign((undefined *)(param_1 + 0x5bf),(undefined *)param_3);
  lVar2 = (**(code **)(*param_1 + 0x150))(param_1);
  if ((lVar2 == 0) || (lVar1 = param_1[0x4a], lVar1 == 0)) goto LAB_141a249fd;
  lVar3 = FUN_142a3d9f0();
  if ((*(int *)(*(longlong *)(lVar1 + 0x10) + 0x38) < *(int *)(lVar3 + 0x38)) ||
     (*(longlong *)
       (*(longlong *)(*(longlong *)(lVar1 + 0x10) + 0x30) + (longlong)*(int *)(lVar3 + 0x38) * 8) !=
      lVar3 + 0x30)) goto LAB_141a249fd;
  lVar3 = param_1[0x5be];
  if (lVar3 == 0) {
LAB_141a249e3:
    lVar3 = 0;
  }
  else {
    lVar4 = FUN_142a3d9f0();
    if ((*(int *)(*(longlong *)(lVar3 + 0x10) + 0x38) < *(int *)(lVar4 + 0x38)) ||
       (*(longlong *)
         (*(longlong *)(*(longlong *)(lVar3 + 0x10) + 0x30) + (longlong)*(int *)(lVar4 + 0x38) * 8)
        != lVar4 + 0x30)) goto LAB_141a249e3;
  }
  *(longlong *)(lVar1 + 0x808) = lVar3;
  *(undefined4 *)(lVar1 + 0x810) = *(undefined4 *)(lVar2 + 0x540);
LAB_141a249fd:
  if ((undefined *)param_3[2] != (undefined *)0x0) {
    FMemory_Free((undefined *)param_3[2]);
  }
  if ((undefined *)*param_3 == (undefined *)0x0) {
    return;
  }
  FMemory_Free((undefined *)*param_3);
  return;
}

