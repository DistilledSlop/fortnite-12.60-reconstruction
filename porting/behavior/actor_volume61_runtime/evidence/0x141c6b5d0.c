
longlong FUN_141c6b5d0(longlong *param_1,undefined8 param_2)

{
  longlong *plVar1;
  int *piVar2;
  int iVar3;
  longlong lVar4;
  longlong lVar5;
  undefined8 uVar6;
  undefined8 uVar7;
  longlong lVar8;
  undefined8 *puVar9;
  undefined8 uStackX_20;
  undefined *apuStack_48 [2];
  undefined1 auStack_38 [8];
  longlong *plStack_30;
  
  uVar6 = FUN_142e80820();
  FUN_142ec0440(uVar6,
                L"NewObject with empty name can\'t be used to create default subobjects (inside of UObject derived class constructor) as it produces inconsistent object names. Use ObjectInitializer.CreateDefaultSuobject<> instead."
               );
  uVar7 = FUN_142a2c420();
  lVar8 = FUN_142ed7360(uVar7,uVar6,0,0,0,0,0,0,0);
  if (lVar8 == 0) {
    FUN_14079a760(param_2);
    lVar8 = 0;
  }
  else {
    lVar4 = *param_1;
    *(longlong *)(lVar8 + 0x310) = lVar4;
    uStackX_20 = *(undefined8 *)(lVar4 + 0x18);
    FUN_142ce9f90(&uStackX_20,apuStack_48);
    puVar9 = (undefined8 *)FUN_142c26df0(auStack_38,apuStack_48);
    uVar6 = *(undefined8 *)(lVar8 + 0x70);
    *(undefined8 *)(lVar8 + 0x70) = *puVar9;
    *puVar9 = uVar6;
    uVar6 = *(undefined8 *)(lVar8 + 0x78);
    *(undefined8 *)(lVar8 + 0x78) = puVar9[1];
    puVar9[1] = uVar6;
    *(undefined4 *)(lVar8 + 0x80) = *(undefined4 *)(puVar9 + 2);
    if (plStack_30 != (longlong *)0x0) {
      LOCK();
      plVar1 = plStack_30 + 1;
      lVar5 = *plVar1;
      *(int *)plVar1 = (int)*plVar1 + -1;
      UNLOCK();
      if ((int)lVar5 == 1) {
        (**(code **)*plStack_30)(plStack_30);
        LOCK();
        piVar2 = (int *)((longlong)plStack_30 + 0xc);
        iVar3 = *piVar2;
        *piVar2 = *piVar2 + -1;
        UNLOCK();
        if (iVar3 == 1) {
          (**(code **)(*plStack_30 + 8))(plStack_30,1);
        }
      }
    }
    if (apuStack_48[0] != (undefined *)0x0) {
      FMemory_Free(apuStack_48[0]);
    }
    *(undefined8 *)(lVar8 + 0x3a0) = *(undefined8 *)(lVar4 + 0x18);
    FUN_14079cdf0(lVar8 + 0x318,param_2);
    FUN_14079a760(param_2);
  }
  return lVar8;
}

