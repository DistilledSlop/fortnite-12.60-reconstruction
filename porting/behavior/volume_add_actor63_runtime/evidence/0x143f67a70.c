
ulonglong FUN_143f67a70(longlong param_1,undefined8 param_2)

{
  int *piVar1;
  bool bVar2;
  undefined7 extraout_var;
  int *piVar3;
  int iStackX_14;
  
  bVar2 = FName_EqualsEName((int)param_2,0);
  iStackX_14 = (int)((ulonglong)param_2 >> 0x20);
  piVar3 = (int *)CONCAT71(extraout_var,iStackX_14 != 0);
  if (!bVar2 || iStackX_14 != 0) {
    piVar3 = *(int **)(param_1 + 0x168);
    piVar1 = piVar3 + (longlong)*(int *)(param_1 + 0x170) * 2;
    for (; piVar3 != piVar1; piVar3 = piVar3 + 2) {
      if (*piVar3 == (int)param_2 && piVar3[1] == iStackX_14) {
        return CONCAT71((int7)((ulonglong)piVar3 >> 8),1);
      }
    }
  }
  return (ulonglong)piVar3 & 0xffffffffffffff00;
}

