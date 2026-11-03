
longlong FUN_141e9a660(longlong *param_1)

{
  longlong lVar1;
  
  lVar1 = param_1[0xdc];
  if (lVar1 == 0) {
    (**(code **)(*param_1 + 0xc68))();
    lVar1 = param_1[0xdc];
  }
  return lVar1;
}

