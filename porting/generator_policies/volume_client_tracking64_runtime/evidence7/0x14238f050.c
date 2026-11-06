
undefined1 thunk_FUN_1444816f0(longlong param_1)

{
  int iVar1;
  
  if (*(char *)(param_1 + 0x54c) == '\0') {
    iVar1 = FUN_143f7a920();
    if (iVar1 == 1) {
      return 0;
    }
    if ((iVar1 != 3) && (iVar1 != 0)) {
      return *(undefined1 *)(param_1 + 0x54c);
    }
    *(undefined1 *)(param_1 + 0x54c) = 1;
  }
  return 1;
}

