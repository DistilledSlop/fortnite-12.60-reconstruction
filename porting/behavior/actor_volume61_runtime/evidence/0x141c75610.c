
undefined8 * FUN_141c75610(longlong param_1,undefined8 *param_2)

{
  undefined8 uVar1;
  undefined4 uVar2;
  longlong lVar3;
  undefined8 uVar4;
  undefined *puStack_18;
  
  lVar3 = *(longlong *)(param_1 + 0x2a0);
  if (lVar3 != 0) {
    uVar4 = *(undefined8 *)(lVar3 + 0x108);
    uVar1 = *(undefined8 *)(lVar3 + 0x110);
    uVar2 = *(undefined4 *)(lVar3 + 0x118);
    *param_2 = *(undefined8 *)(lVar3 + 0x100);
    param_2[1] = uVar4;
    param_2[2] = uVar1;
    *(undefined4 *)(param_2 + 3) = uVar2;
    return param_2;
  }
  if (1 < DAT_1481348e0) {
    FUN_142ce9f90();
    FMsg_Logf_Internal((byte *)0x0,0,&DAT_1481348e4,2,
                       (ushort *)
                       L"AFortVolume::GetBounds - The static mesh should be initialized for: %s");
    if (puStack_18 != (undefined *)0x0) {
      FMemory_Free(puStack_18);
    }
  }
  return param_2;
}

