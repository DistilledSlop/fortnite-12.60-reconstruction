
undefined8 FUN_141f9f270(longlong *param_1)

{
  undefined8 uVar1;
  longlong lVar2;
  
  if (param_1 != (longlong *)0x0) {
    uVar1 = FUN_142b39750();
    lVar2 = (**(code **)(*param_1 + 0x610))(param_1,uVar1);
    if ((lVar2 != 0) && (((*(byte *)(lVar2 + 0x130) & 1) != 0 || (*(char *)(lVar2 + 0xb0) != '\0')))
       ) {
      return *(undefined8 *)(lVar2 + 0x128);
    }
  }
  return 0;
}

