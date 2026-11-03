
void FUN_141dedec2(void)

{
  longlong lVar1;
  char unaff_SIL;
  
  lVar1 = FUN_141df61e0();
  EnterCriticalSection();
  if (*(char *)(lVar1 + 0x1c8) == unaff_SIL) {
    if (1 < DAT_1481348e0) {
      FMsg_Logf_Internal((byte *)0x0,0,&DAT_1481348e4,2,
                         (ushort *)
                         L"SetStartupWindowClosedForAlwaysLoadedAssets called with already set value!"
                        );
    }
  }
  else {
    *(char *)(lVar1 + 0x1c8) = unaff_SIL;
  }
                    /* WARNING: Could not recover jumptable at 0x000141dedf2a. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  LeaveCriticalSection(lVar1 + 0x1a0);
  return;
}

