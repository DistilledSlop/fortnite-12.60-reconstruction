
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

longlong FUN_141c74820(longlong param_1)

{
  int iVar1;
  uint uVar2;
  longlong lVar3;
  undefined *puVar4;
  longlong lVar5;
  uint uVar6;
  
  if (param_1 != 0) {
    lVar3 = FUN_144640410(_DAT_148351eb8,param_1,1);
    if ((lVar3 != 0) && (lVar3 = *(longlong *)(lVar3 + 0x130), lVar3 != 0)) {
      puVar4 = AFortGameStateAthena_StaticClass();
      if ((*(int *)(puVar4 + 0x38) <= *(int *)(*(longlong *)(lVar3 + 0x10) + 0x38)) &&
         (*(undefined **)
           (*(longlong *)(*(longlong *)(lVar3 + 0x10) + 0x30) +
           (longlong)*(int *)(puVar4 + 0x38) * 8) == puVar4 + 0x30)) {
        lVar3 = FUN_141999ba0(lVar3);
        if (lVar3 != 0) {
          iVar1 = *(int *)(lVar3 + 0xc);
          if (iVar1 < _DAT_148263a2c) {
            uVar6 = iVar1 >> 0x1f & 0xffff;
            uVar2 = iVar1 + uVar6;
            lVar5 = *(longlong *)(_DAT_148263a18 + (longlong)((int)uVar2 >> 0x10) * 8) +
                    (longlong)(int)((uVar2 & 0xffff) - uVar6) * 0x18;
          }
          else {
            lVar5 = 0;
          }
          if ((*(uint *)(lVar5 + 8) >> 0x1d & 1) == 0) {
            return lVar3;
          }
        }
      }
    }
  }
  return 0;
}

