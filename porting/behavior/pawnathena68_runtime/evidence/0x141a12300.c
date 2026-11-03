
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_141a12300(longlong param_1,longlong *param_2,char param_3)

{
  bool bVar1;
  char cVar2;
  undefined *puVar3;
  
  if (*param_2 != 0) {
    puVar3 = AFortGameStateAthena_StaticClass();
    if (puVar3 != (undefined *)0x0) {
      if ((*(int *)(puVar3 + 0x38) <= *(int *)(*(longlong *)(*param_2 + 0x10) + 0x38)) &&
         (*(undefined **)
           (*(longlong *)(*(longlong *)(*param_2 + 0x10) + 0x30) +
           (longlong)*(int *)(puVar3 + 0x38) * 8) == puVar3 + 0x30)) {
        bVar1 = (byte)(param_3 - 5U) < 6;
        cVar2 = *(char *)(param_1 + 0x2f90);
        if ((bool)*(char *)(param_1 + 0x2f90) != bVar1) {
          *(bool *)(param_1 + 0x2f90) = bVar1;
          cVar2 = bVar1;
        }
        if ((param_3 == '\v') &&
           (((*(char *)(param_1 + 0x2f75) != '\0' && (cVar2 != '\0')) || (_DAT_148114c00 != 0)))) {
          FUN_141a0b170(param_1);
        }
      }
    }
  }
  return;
}

