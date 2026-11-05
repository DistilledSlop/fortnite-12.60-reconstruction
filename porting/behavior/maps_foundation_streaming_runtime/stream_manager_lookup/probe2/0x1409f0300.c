
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined * FUN_1409f0300(longlong param_1,undefined8 param_2)

{
  undefined *puVar1;
  longlong lVar2;
  longlong unaff_GS_OFFSET;
  int aiStackX_8 [8];
  
  FUN_140709780((longlong *)(param_1 + 0x48),aiStackX_8,param_2);
  if (aiStackX_8[0] == -1) {
    lVar2 = 0;
  }
  else {
    lVar2 = (longlong)aiStackX_8[0] * 0x20 + *(longlong *)(param_1 + 0x48);
  }
  puVar1 = (undefined *)(lVar2 + 8);
  if (lVar2 == 0) {
    puVar1 = (undefined *)0x0;
  }
  if (puVar1 == (undefined *)0x0) {
    if (*(int *)(*(longlong *)(*(longlong *)(unaff_GS_OFFSET + 0x58) + (ulonglong)__tls_index * 8) +
                0x248) < _DAT_1480da538) {
      FUN_145803a44(&DAT_1480da538);
      if (_DAT_1480da538 == -1) {
        FUN_1458038a4(&LAB_1458285d0);
        FUN_1458039e4(&DAT_1480da538);
        return &DAT_1480da528;
      }
    }
    puVar1 = &DAT_1480da528;
  }
  return puVar1;
}

