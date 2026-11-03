
void FUN_141748280(longlong param_1,char param_2)

{
  longlong *plVar1;
  
  plVar1 = *(longlong **)(param_1 + 0x220);
  if (plVar1 != (longlong *)0x0) {
    if (param_2 == '\0') {
      if ((*(byte *)((longlong)plVar1 + 0x86) & 1) != 0) {
                    /* WARNING: Could not recover jumptable at 0x0001417482b1. Too many branches */
                    /* WARNING: Treating indirect jump as call */
        (**(code **)(*plVar1 + 0x270))();
        return;
      }
    }
    else if ((*(byte *)((longlong)plVar1 + 0x86) & 1) == 0) {
                    /* WARNING: Could not recover jumptable at 0x00014174829e. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      (**(code **)(*plVar1 + 0x268))(plVar1,0);
      return;
    }
  }
  return;
}

