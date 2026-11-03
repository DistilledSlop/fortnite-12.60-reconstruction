
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined * FUN_14029ec5d(undefined8 param_1,ushort *param_2,undefined4 param_3)

{
  ushort uVar1;
  undefined4 uVar2;
  ushort *puVar3;
  ushort uVar4;
  undefined4 uStack_18;
  undefined4 uStack_14;
  int iStack_10;
  bool bStack_c;
  undefined3 uStack_b;
  
  uVar4 = 0;
  puVar3 = param_2;
  if (param_2 != (ushort *)0x0) {
    uVar1 = *param_2;
    while (uVar1 != 0) {
      puVar3 = puVar3 + 1;
      uVar4 = uVar4 | uVar1;
      uVar1 = *puVar3;
    }
  }
  _bStack_c = CONCAT31(uStack_b,(uVar4 & 0xff80) != 0);
  iStack_10 = (int)((longlong)puVar3 - (longlong)param_2 >> 1);
  uStack_14 = (undefined4)((ulonglong)param_2 >> 0x20);
  uStack_18 = SUB84(param_2,0);
  if (iStack_10 == 0) {
    _DAT_148122c40 = 0;
    return &DAT_148122c40;
  }
  uVar2 = FUN_142cd4630(param_2,&iStack_10);
  FUN_142ce4890(&DAT_148122c40,&uStack_18,param_3,uVar2);
  return &DAT_148122c40;
}

