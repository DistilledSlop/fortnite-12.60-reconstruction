
UObject * FUN_142321a70(longlong *param_1)

{
  char cVar1;
  longlong lVar2;
  UObject *pUVar3;
  longlong *plVar4;
  longlong *plStack_18;
  int iStack_10;
  
  lVar2 = (**(code **)(*param_1 + 0x150))();
  if (lVar2 != 0) {
    FUN_14468fb70(lVar2,&plStack_18);
    for (; (-1 < iStack_10 && (iStack_10 < (int)plStack_18[1])); iStack_10 = iStack_10 + 1) {
      pUVar3 = FWeakObjectPtr_Get((undefined *)(*plStack_18 + (longlong)iStack_10 * 8));
      if (pUVar3 != (UObject *)0x0) {
        cVar1 = (**(code **)(*(longlong *)pUVar3->vftable + 0x6a8))(pUVar3);
        if (cVar1 != '\0') {
          plVar4 = (longlong *)(**(code **)(*(longlong *)pUVar3->vftable + 0x698))(pUVar3);
          if (plVar4 == param_1) {
            return pUVar3;
          }
        }
      }
    }
  }
  return (UObject *)0x0;
}

