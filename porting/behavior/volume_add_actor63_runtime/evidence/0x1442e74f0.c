
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_1442e74f0(undefined8 param_1,undefined8 param_2,undefined1 (*param_3) [12],longlong param_4
                  ,undefined8 param_5)

{
  undefined4 *puVar1;
  undefined1 auStack_88 [32];
  undefined1 auStack_68 [16];
  undefined4 uStack_58;
  undefined4 uStack_54;
  undefined4 uStack_50;
  uint uStack_4c;
  undefined1 auStack_48 [16];
  uint uStack_38;
  uint uStack_34;
  uint uStack_30;
  uint uStack_2c;
  ulonglong uStack_28;
  
  uStack_28 = _DAT_1480c4288 ^ (ulonglong)auStack_88;
  uStack_38 = _DAT_148333120 & _DAT_1483330f0;
  uStack_34 = uRam0000000148333124 & uRam00000001483330f4;
  uStack_30 = uRam0000000148333128 & uRam00000001483330f8;
  uStack_2c = uRam000000014833312c & uRam00000001483330fc;
  uStack_58 = 0;
  uStack_54 = 0;
  uStack_50 = 0;
  uStack_4c = uRam000000014833312c;
  auStack_48 = ZEXT816(0);
  if (param_3 != (undefined1 (*) [12])0x0) {
    auStack_48._12_4_ = 0;
    auStack_48._0_12_ = *param_3;
  }
  if (param_4 != 0) {
    puVar1 = (undefined4 *)FUN_142c5c060(param_4,auStack_68);
    uStack_58 = *puVar1;
    uStack_54 = puVar1[1];
    uStack_50 = puVar1[2];
    uStack_4c = puVar1[3];
  }
  FUN_1442e6580(param_1,param_2,&uStack_58,param_5);
  FUN_145803c70(uStack_28 ^ (ulonglong)auStack_88);
  return;
}

