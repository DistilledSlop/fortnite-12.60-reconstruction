
char * FUN_141c55d00(longlong *param_1,int param_2,char *param_3)

{
  int iVar1;
  char *pcVar2;
  longlong *plVar3;
  
  if ((int)param_1[1] != *(int *)((longlong)param_1 + 0x34)) {
    plVar3 = param_1 + 7;
    if ((longlong *)param_1[8] != (longlong *)0x0) {
      plVar3 = (longlong *)param_1[8];
    }
    iVar1 = *(int *)((longlong)plVar3 + ((longlong)(int)param_1[9] - 1U & (longlong)param_2) * 4);
    if (iVar1 != -1) {
      do {
        pcVar2 = (char *)((longlong)iVar1 * 0x10 + *param_1);
        if (*pcVar2 == *param_3) {
          if ((iVar1 != -1) && (pcVar2 != (char *)0x0)) {
            return pcVar2 + 4;
          }
          break;
        }
        iVar1 = *(int *)(pcVar2 + 8);
      } while (iVar1 != -1);
    }
  }
  pcVar2 = (char *)FUN_141c55900(param_1,param_2,param_3);
  return pcVar2;
}

