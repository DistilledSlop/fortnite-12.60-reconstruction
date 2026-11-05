
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

float FUN_141d39910(longlong param_1,longlong param_2)

{
  float fVar1;
  float fVar2;
  float fVar3;
  float fVar4;
  float fVar5;
  float fVar6;
  float fStack_28;
  float fStack_24;
  float fStack_20;
  
  fVar2 = 0.0;
  fVar6 = _DAT_1478c36cc * _DAT_1478c36cc;
  if (0.0 < _DAT_1478c36d0) {
    fVar5 = _DAT_1478c36d0 * _DAT_1478c36d0;
  }
  else {
    fVar5 = 0.0;
  }
  if ((0 < _DAT_148126190) && (0.0 < _DAT_1478c36c0)) {
    fStack_20 = (float)*(undefined8 *)(param_2 + 0x18);
    fVar3 = _DAT_1478c36c4;
    if (_DAT_1478c36c4 <= fStack_20) goto LAB_141d39972;
  }
  fVar3 = 0.0;
LAB_141d39972:
  fVar4 = *(float *)(param_1 + 0xb0);
  fVar1 = 0.0;
  fStack_28 = (float)*(undefined8 *)(param_2 + 0x10);
  if ((fStack_28 < fVar4) || (fVar4 = *(float *)(param_1 + 0xbc), fVar4 < fStack_28)) {
    fVar1 = (fStack_28 - fVar4) * (fStack_28 - fVar4);
  }
  fVar4 = *(float *)(param_1 + 0xb4);
  fStack_24 = (float)((ulonglong)*(undefined8 *)(param_2 + 0x10) >> 0x20);
  if ((fStack_24 < fVar4) || (fVar4 = *(float *)(param_1 + 0xc0), fVar4 < fStack_24)) {
    fVar1 = fVar1 + (fStack_24 - fVar4) * (fStack_24 - fVar4);
  }
  fVar4 = *(float *)(param_1 + 0xb8);
  if ((fVar3 < fVar4) || (fVar4 = *(float *)(param_1 + 0xc4), fVar4 < fVar3)) {
    fVar1 = fVar1 + (fVar3 - fVar4) * (fVar3 - fVar4);
  }
  if (fVar6 < fVar5) {
    fVar2 = ((fVar1 - fVar5) + (fVar1 - fVar5)) / (fVar6 - fVar5);
    if (fVar2 <= 0.0) {
      fVar2 = 0.0;
    }
  }
  else if (fVar1 <= fVar6) {
    fVar2 = fVar1 / fVar6 + 2.0;
  }
  if ((*(char *)(*(longlong *)(param_1 + 0xd0) + 0x15a) == '\0') && (2.0 <= fVar2)) {
    return 1.9998;
  }
  return fVar2;
}

