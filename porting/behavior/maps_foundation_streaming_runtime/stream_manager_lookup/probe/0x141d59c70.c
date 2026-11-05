
void FUN_141d59c70(longlong *param_1,undefined8 param_2,byte param_3)

{
  int OldNum;
  undefined8 *puVar1;
  longlong lVar2;
  longlong alStack_88 [2];
  undefined8 auStack_78 [6];
  longlong alStack_48 [2];
  undefined8 auStack_38 [6];
  
  if (param_3 == 6) {
    OldNum = (int)param_1[0x2e];
    *(int *)(param_1 + 0x2e) = OldNum + 1;
    if (*(int *)((longlong)param_1 + 0x174) < OldNum + 1) {
      TArray8_ResizeGrow((undefined *)(param_1 + 0x2d),OldNum);
    }
    *(undefined8 *)(param_1[0x2d] + (longlong)OldNum * 8) = param_2;
  }
  else {
    auStack_78[0] = 0;
    lVar2 = (ulonglong)param_3 * 0x140;
    alStack_88[0] = *(longlong *)(lVar2 + 0x1478c3870);
    if (alStack_88[0] != 0) {
      puVar1 = (undefined8 *)(lVar2 + 0x1478c3890);
      if (*(undefined8 **)(lVar2 + 0x1478c3880) != (undefined8 *)0x0) {
        puVar1 = *(undefined8 **)(lVar2 + 0x1478c3880);
      }
      (**(code **)*puVar1)(puVar1,auStack_78);
    }
    alStack_48[0] = *(longlong *)(&DAT_1478c3830 + lVar2);
    auStack_38[0] = 0;
    if (alStack_48[0] != 0) {
      puVar1 = (undefined8 *)(&DAT_1478c3850 + lVar2);
      if (*(undefined8 **)(&DAT_1478c3840 + lVar2) != (undefined8 *)0x0) {
        puVar1 = *(undefined8 **)(&DAT_1478c3840 + lVar2);
      }
      (**(code **)*puVar1)(puVar1,auStack_38);
    }
    (**(code **)(*param_1 + 0x268))
              (param_1,param_2,*(undefined8 *)(&DAT_1478c37a0 + lVar2),alStack_48,
               *(undefined1 *)(lVar2 + 0x1478c37a8),alStack_88);
  }
  return;
}

