
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_141ded253(undefined *param_1)

{
  longlong lVar1;
  longlong unaff_RBP;
  
  FMemory_Free(param_1);
  if (*(undefined **)(unaff_RBP + 7) != (undefined *)0x0) {
    FMemory_Free(*(undefined **)(unaff_RBP + 7));
  }
  if (*(undefined **)(unaff_RBP + -9) != (undefined *)0x0) {
    FMemory_Free(*(undefined **)(unaff_RBP + -9));
  }
  lVar1 = FUN_141df61e0();
  EnterCriticalSection(lVar1 + 0x1a0);
  FUN_1407bb180(lVar1 + 0x100,0);
  FUN_140992250(lVar1 + 0x150,0);
  LeaveCriticalSection(lVar1 + 0x1a0);
  FUN_14463f4b0(_DAT_148351eb8,1);
  return;
}

