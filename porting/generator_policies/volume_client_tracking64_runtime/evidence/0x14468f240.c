
UObject * FUN_14468f240(longlong param_1)

{
  UObject *pUVar1;
  
  if (*(int *)(param_1 + 0x1d0) < 1) {
    return (UObject *)0x0;
  }
  pUVar1 = FWeakObjectPtr_Get(*(undefined **)(param_1 + 0x1c8));
  return pUVar1;
}

