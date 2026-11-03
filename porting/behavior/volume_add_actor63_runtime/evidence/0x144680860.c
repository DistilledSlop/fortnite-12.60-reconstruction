
undefined8 * FUN_144680860(undefined8 *param_1)

{
  *param_1 = 0;
  *(byte *)((longlong)param_1 + 0x29) = *(byte *)((longlong)param_1 + 0x29) & 0xf0;
  param_1[1] = 0;
  param_1[2] = 0;
  param_1[3] = 0;
  param_1[4] = 0;
  *(undefined1 *)(param_1 + 5) = 0;
  *(undefined4 *)((longlong)param_1 + 0x2c) = 8;
  return param_1;
}

