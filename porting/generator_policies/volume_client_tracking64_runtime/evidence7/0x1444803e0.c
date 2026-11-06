
longlong FUN_1444803e0(longlong param_1)

{
  longlong lVar1;
  
  lVar1 = *(longlong *)(param_1 + 0x2b0);
  if (lVar1 == 0) {
    return 0;
  }
  if (*(longlong *)(lVar1 + 0x1410) != 0) {
    FUN_144475cb0(lVar1 + 0x1410,*(undefined8 *)(lVar1 + 0x218));
    if (*(longlong *)(lVar1 + 0x1410) != 0) {
      return *(longlong *)(lVar1 + 0x1410);
    }
  }
  FUN_144475cb0(lVar1 + 0xe30,*(undefined8 *)(lVar1 + 0x218));
  return *(longlong *)(lVar1 + 0xe30);
}

