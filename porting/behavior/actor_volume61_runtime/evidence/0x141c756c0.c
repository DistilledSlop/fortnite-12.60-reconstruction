
float * FUN_141c756c0(undefined4 param_1,float *param_2)

{
  float fVar1;
  float fVar2;
  float fVar3;
  float fVar4;
  float fVar5;
  float fVar6;
  float fVar7;
  float fVar8;
  float *pfVar9;
  undefined1 auStack_4c [68];
  
  pfVar9 = (float *)FUN_141c75610(param_1,auStack_4c);
  fVar1 = *pfVar9;
  fVar2 = pfVar9[2];
  fVar3 = pfVar9[1];
  fVar4 = pfVar9[5];
  fVar5 = pfVar9[3];
  fVar6 = pfVar9[5];
  fVar7 = pfVar9[4];
  fVar8 = pfVar9[4];
  *param_2 = fVar1 - pfVar9[3];
  param_2[1] = fVar3 - fVar8;
  param_2[2] = fVar2 - fVar6;
  *(ulonglong *)(param_2 + 3) = CONCAT44(fVar3 + fVar7,fVar1 + fVar5);
  param_2[5] = fVar2 + fVar4;
  *(undefined1 *)(param_2 + 6) = 1;
  return param_2;
}

