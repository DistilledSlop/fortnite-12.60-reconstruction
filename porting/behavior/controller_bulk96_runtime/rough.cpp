// Rough, uncounted behavioral lifts from the native96 wrappers and helper probe.
// Decompiler types/ABI and U_ calls remain unresolved; this is not buildable C++.
// U_XXXXXXXX denotes the native function at that address; virtual calls stay indirect.

// AFortPlayerControllerAthena_IsTryToFireRestrictedByTypeCooldowns exec 0x142a59050
// Out-of-line dependency 0x1419dde10; parameter ABI remains to be checked.
void U_1419dde10(void)

{
  longlong lVar1;
  
  lVar1 = U_142384300();
  if (lVar1 == 0) {
    return;
  }
  U_1426a5f30(lVar1);
  return;
}

// AFortPlayerControllerAthena_IsPlayerRebootingNow exec 0x142a59020
// Out-of-line dependency 0x1419ddaa0; parameter ABI remains to be checked.
ulonglong U_1419ddaa0(longlong param_1)

{
  int iVar1;
  longlong lVar2;
  ulonglong in_RAX;
  undefined *puVar3;
  UObject *pUVar4;
  ulonglong uVar5;
  
  if (*(longlong *)(param_1 + 0x3620) == 0) {
    return in_RAX & 0xffffffffffffff00;
  }
  lVar2 = *(longlong *)(*(longlong *)(param_1 + 0x3620) + 0xa0);
  if (lVar2 != 0) {
    puVar3 = AFortPlayerControllerAthena_StaticClass();
    iVar1 = *(int *)(puVar3 + 0x38);
    in_RAX = (longlong)iVar1;
    if (((iVar1 <= *(int *)(*(longlong *)(lVar2 + 0x10) + 0x38)) &&
        (in_RAX = *(ulonglong *)(*(longlong *)(lVar2 + 0x10) + 0x30),
        *(undefined **)(in_RAX + (longlong)iVar1 * 8) == puVar3 + 0x30)) &&
       (lVar2 = *(longlong *)(lVar2 + 0x220), lVar2 != 0)) {
      pUVar4 = AFortPlayerStateAthena_GetPrivateStaticClass();
      iVar1 = *(int *)pUVar4[1].ClassPrivate;
      in_RAX = (longlong)iVar1;
      if ((iVar1 <= *(int *)(*(longlong *)(lVar2 + 0x10) + 0x38)) &&
         (in_RAX = *(ulonglong *)(*(longlong *)(lVar2 + 0x10) + 0x30),
         (uint *)*(undefined1 **)(in_RAX + (longlong)iVar1 * 8) == &pUVar4[1].ObjectFlags)) {
        uVar5 = U_141a4b2d0(lVar2);
        return uVar5;
      }
    }
  }
  return in_RAX & 0xffffffffffffff00;
}

// AFortPlayerControllerAthena_IsPlayerInReadOnlyVolume exec 0x142a58ff0
// Out-of-line dependency 0x1419dda50; parameter ABI remains to be checked.
undefined1 U_1419dda50(longlong param_1)

{
  longlong lVar1;
  
  lVar1 = U_141c74820();
  if (lVar1 != 0) {
    lVar1 = U_141c7d420(lVar1,*(undefined8 *)(param_1 + 0x220));
    if ((lVar1 != 0) &&
       ((*(char *)(lVar1 + 0x408) == '\x01' || ((*(byte *)(lVar1 + 0x2d8) & 1) != 0)))) {
      return 1;
    }
  }
  return 0;
}

// AFortPlayerControllerAthena_IsPlayerInPublishedVolume exec 0x142a58fc0
// Out-of-line dependency 0x1419dd9e0; parameter ABI remains to be checked.
undefined1 U_1419dd9e0(longlong param_1)

{
  char cVar1;
  char cVar2;
  longlong lVar3;
  
  lVar3 = U_141c74820();
  if (lVar3 != 0) {
    lVar3 = U_141c7d420(lVar3,*(undefined8 *)(param_1 + 0x220));
    if (lVar3 != 0) {
      cVar1 = U_141c77c90(lVar3);
      cVar2 = U_141c77c90(lVar3);
      if ((cVar1 == '\x03') || (cVar2 == '\x02')) {
        return 1;
      }
    }
  }
  return 0;
}

// AFortPlayerControllerAthena_IsPlayerInFeaturedVolume exec 0x142a58f90
// Out-of-line dependency 0x1419dd990; parameter ABI remains to be checked.
bool U_1419dd990(longlong param_1)

{
  char cVar1;
  longlong lVar2;
  
  lVar2 = U_141c74820();
  if (lVar2 != 0) {
    lVar2 = U_141c7d420(lVar2,*(undefined8 *)(param_1 + 0x220));
    if (lVar2 != 0) {
      cVar1 = U_141c77c90(lVar2);
      return cVar1 == '\x03';
    }
  }
  return false;
}

// AFortPlayerControllerAthena_IsPlayerInAVolumeTheyCanCreateIn exec 0x142a58f60
// Out-of-line dependency 0x1419dd950; parameter ABI remains to be checked.
undefined1 U_1419dd950(undefined8 param_1)

{
  char cVar1;
  longlong alStackX_10 [3];
  
  alStackX_10[0] = 0;
  cVar1 = U_1419c3880(param_1,alStackX_10);
  if ((cVar1 == '\0') || (alStackX_10[0] == 0)) {
    return 0;
  }
  return 1;
}

// AFortPlayerControllerAthena_IsPlayerInAMinigame exec 0x142a58f30
// Out-of-line dependency 0x1419dd8b0; parameter ABI remains to be checked.
undefined8 U_1419dd8b0(longlong param_1)

{
  longlong lVar1;
  char cVar2;
  UObject *pUVar3;
  longlong lVar4;
  
  lVar1 = *(longlong *)(param_1 + 0x220);
  if (lVar1 != 0) {
    pUVar3 = AFortPlayerStateAthena_GetPrivateStaticClass();
    if ((*(int *)pUVar3[1].ClassPrivate <= *(int *)(*(longlong *)(lVar1 + 0x10) + 0x38)) &&
       ((uint *)*(undefined1 **)
                 (*(longlong *)(*(longlong *)(lVar1 + 0x10) + 0x30) +
                 (longlong)*(int *)pUVar3[1].ClassPrivate * 8) == &pUVar3[1].ObjectFlags)) {
      lVar4 = U_141c74820(param_1);
      if (lVar4 != 0) {
        lVar4 = U_141c7d420(lVar4,lVar1);
        if (lVar4 != 0) {
          lVar4 = U_141c795a0(lVar4);
          if ((lVar4 != 0) && (*(char *)(lVar4 + 0x870) != '\0')) {
            cVar2 = U_14209a3d0(lVar4,lVar1,0);
            if (cVar2 != '\0') {
              return 1;
            }
          }
        }
      }
    }
  }
  return 0;
}

// AFortPlayerControllerAthena_IsPartyMemberReadyCheckStatusInProgress exec 0x142a58f00
// Out-of-line dependency 0x1419dd820; parameter ABI remains to be checked.
bool U_1419dd820(void)

{
  undefined8 uVar1;
  longlong lVar2;
  
  lVar2 = U_14237f3b0();
  if (lVar2 == 0) {
    return false;
  }
  uVar1 = U_142290c00(lVar2);
  lVar2 = U_140d91b30(uVar1);
  return *(char *)(lVar2 + 0x408) == '\x01';
}

// AFortPlayerControllerAthena_IsOnPublishedIsland exec 0x142a58ed0
// Out-of-line dependency 0x1419dd760; parameter ABI remains to be checked.
undefined8 U_1419dd760(longlong *param_1)

{
  longlong lVar1;
  longlong lVar2;
  longlong lVar3;
  longlong *plVar4;
  undefined8 uVar5;
  
  lVar1 = U_141c74820();
  if ((lVar1 != 0) && (lVar2 = (**(code **)(*param_1 + 0x14d8))(param_1), lVar2 != 0)) {
    lVar3 = U_142a82010();
    if ((*(int *)(lVar3 + 0x38) <= *(int *)(*(longlong *)(lVar2 + 0x10) + 0x38)) &&
       (*(longlong *)
         (*(longlong *)(*(longlong *)(lVar2 + 0x10) + 0x30) + (longlong)*(int *)(lVar3 + 0x38) * 8)
        == lVar3 + 0x30)) {
      plVar4 = (longlong *)param_1[0x6a1];
      if ((plVar4 == (longlong *)0x0) &&
         (plVar4 = (longlong *)U_141c7c5c0(lVar1,lVar2), plVar4 == (longlong *)0x0)) {
        return 0;
      }
      uVar5 = U_14297b320();
      lVar1 = (**(code **)(*plVar4 + 0x610))(plVar4,uVar5);
      if ((lVar1 != 0) && (1 < *(int *)(lVar1 + 0x2c8))) {
        return 1;
      }
    }
  }
  return 0;
}

// AFortPlayerControllerAthena_GetSquadKillsCount exec 0x142a58ab0
// Out-of-line dependency 0x1419d8e40; parameter ABI remains to be checked.
ulonglong U_1419d8e40(void)

{
  int *piVar1;
  undefined *puVar2;
  int *piVar3;
  int *piVar4;
  int *piVar5;
  int iVar6;
  bool bVar7;
  longlong *plVar8;
  UObject *pUVar9;
  ulonglong uVar10;
  int *piVar11;
  ulonglong uVar12;
  undefined *this;
  int *Ptr;
  int iVar13;
  int iVar14;
  int iVar15;
  int iVar16;
  int iVar17;
  int iVar18;
  int iVar19;
  int iVar20;
  int *piStack_38;
  int iStack_30;
  int iStack_2c;
  
  uVar12 = 0;
  Ptr = (int *)0x0;
  iVar13 = 0;
  piStack_38 = (int *)0x0;
  iStack_2c = 0;
  plVar8 = (longlong *)U_1419d9220();
  this = (undefined *)*plVar8;
  puVar2 = this + (longlong)(int)plVar8[1] * 8;
  iVar6 = 0;
  for (; this != puVar2; this = this + 8) {
    iVar14 = 0;
    bVar7 = FWeakObjectPtr_IsValid(this);
    if (bVar7) {
      pUVar9 = FWeakObjectPtr_Get(this);
      iVar14 = *(int *)(pUVar9[0x5c].ClassPrivate + 4);
    }
    iStack_30 = iVar6 + 1;
    if (iVar13 < iStack_30) {
      U_140690b30(&piStack_38,iVar6);
      Ptr = piStack_38;
      iVar13 = iStack_2c;
    }
    Ptr[iVar6] = iVar14;
    iVar6 = iStack_30;
  }
  uVar10 = (longlong)iVar6 & 0x3fffffffffffffff;
  if (Ptr + iVar6 < Ptr) {
    uVar10 = 0;
  }
  piVar11 = Ptr;
  if ((uVar10 != 0) && (7 < uVar10)) {
    iVar13 = 0;
    iVar15 = 0;
    iVar17 = 0;
    iVar19 = 0;
    iVar14 = 0;
    iVar16 = 0;
    iVar18 = 0;
    iVar20 = 0;
    do {
      uVar12 = uVar12 + 8;
      iVar13 = *piVar11 + iVar13;
      iVar15 = piVar11[1] + iVar15;
      iVar17 = piVar11[2] + iVar17;
      iVar19 = piVar11[3] + iVar19;
      piVar1 = piVar11 + 4;
      piVar3 = piVar11 + 5;
      piVar4 = piVar11 + 6;
      piVar5 = piVar11 + 7;
      piVar11 = piVar11 + 8;
      iVar14 = *piVar1 + iVar14;
      iVar16 = *piVar3 + iVar16;
      iVar18 = *piVar4 + iVar18;
      iVar20 = *piVar5 + iVar20;
    } while (uVar12 != (uVar10 & 0xfffffffffffffff8));
    uVar12 = (ulonglong)
             (uint)(iVar13 + iVar14 + iVar17 + iVar18 + iVar15 + iVar16 + iVar19 + iVar20);
  }
  for (; piVar11 != Ptr + iVar6; piVar11 = piVar11 + 1) {
    uVar12 = (ulonglong)(uint)((int)uVar12 + *piVar11);
  }
  if (Ptr != (int *)0x0) {
    FMemory_Free((undefined *)Ptr);
  }
  return uVar12;
}

// AFortPlayerControllerAthena_GetMinigame exec 0x142a58790
// Out-of-line dependency 0x1419d5470; parameter ABI remains to be checked.
void U_1419d5470(longlong param_1)

{
  U_141c794e0(*(undefined8 *)(param_1 + 0x220));
  return;
}

// AFortPlayerControllerAthena_GetMaxNumberOfPlots exec 0x142a58760
// Out-of-line dependency 0x1419d5380; parameter ABI remains to be checked.
ulonglong U_1419d5380(longlong param_1)

{
  int iVar1;
  ulonglong uVar2;
  
  iVar1 = U_143f7a920();
  if (iVar1 != 1) {
    return (ulonglong)*(uint *)(param_1 + 0x2e7c);
  }
  if (*(longlong *)(param_1 + 0x1b10) == 0) {
    return 0;
  }
  uVar2 = U_1421d7c50();
  *(int *)(param_1 + 0x2e7c) = (int)uVar2;
  return uVar2;
}

// AFortPlayerControllerAthena_GetMatchReport exec 0x142a58730
// Out-of-line dependency 0x1419d5220; parameter ABI remains to be checked.
undefined8 U_1419d5220(longlong param_1)

{
  if (*(char *)(param_1 + 0xf0) == '\x02') {
    return *(undefined8 *)(param_1 + 0x3318);
  }
  return 0;
}

// AFortPlayerControllerAthena_GetCurrentVolumeOrCreativePlotLinkedVolume exec 0x142a586e0
// Out-of-line dependency 0x1419d3790; parameter ABI remains to be checked.
void U_1419d3790(longlong param_1)

{
  longlong lVar1;
  
  lVar1 = U_141c74820();
  if (lVar1 == 0) {
    return;
  }
  U_141c7d420(lVar1,*(undefined8 *)(param_1 + 0x220));
  return;
}

// AFortPlayerControllerAthena_CanFollowSpecialActor exec 0x142a56330
// Out-of-line dependency 0x1419c2de0; parameter ABI remains to be checked.
ulonglong U_1419c2de0(UObject *param_1)

{
  undefined *puVar1;
  bool bVar2;
  undefined4 *puVar3;
  UObject *pUVar4;
  undefined8 uVar5;
  UObject *pUVar6;
  UObject *pUVar7;
  undefined7 extraout_var;
  undefined *this;
  uint uVar8;
  longlong lVar9;
  FName FStackX_10;
  undefined4 uStackX_18;
  undefined4 uStackX_1c;
  
  puVar3 = (undefined4 *)FName_FromEName(&FStackX_10,0x142);
  lVar9 = 0;
  uStackX_1c = 0;
  uStackX_18 = *puVar3;
  pUVar4 = (UObject *)U_14416c3f0(param_1,uStackX_18);
  if ((char)pUVar4 != '\0') {
    FStackX_10.ComparisonIndex = *(int *)(param_1[0x120].ClassPrivate + 0);
    FStackX_10.Number = *(int *)(param_1[0x120].ClassPrivate + 4);
    uVar5 = U_140f95ae0(param_1);
    pUVar4 = (UObject *)U_144621fb0(uVar5,&FStackX_10);
    if ((((pUVar4 == (UObject *)0x0) || (pUVar4->vftable[1] == '\x02')) &&
        (pUVar6 = GetFortGameStateAthena(param_1), pUVar4 = pUVar6, pUVar6 != (UObject *)0x0)) &&
       (((char)pUVar6[0xb7].NamePrivate.ComparisonIndex != '\x05' &&
        (pUVar4 = (UObject *)U_1419d93c0(param_1), (int)pUVar4 < 1)))) {
      uVar8 = 0;
      pUVar7 = GetFortGameStateAthena(param_1);
      pUVar4 = (UObject *)0x0;
      if (pUVar7 != (UObject *)0x0) {
        pUVar4 = (UObject *)U_1419d9220(param_1);
        if ((byte)((char)pUVar7[0xb7].NamePrivate.ComparisonIndex - 1U) < 2) {
          uVar8 = pUVar4->ObjectFlags;
        }
        else {
          this = *(undefined **)pUVar4->vftable;
          pUVar4 = (UObject *)(longlong)(int)pUVar4->ObjectFlags;
          puVar1 = this + (longlong)pUVar4 * 8;
          for (; this != puVar1; this = this + 8) {
            bVar2 = FWeakObjectPtr_IsValid(this);
            pUVar4 = (UObject *)CONCAT71(extraout_var,bVar2);
            if (bVar2) {
              pUVar4 = FWeakObjectPtr_Get(this);
              pUVar4 = (UObject *)(**(code **)(*(longlong *)pUVar4->vftable + 0x840))(pUVar4);
              if ((char)pUVar4 == '\0') {
                uVar8 = uVar8 + 1;
              }
            }
          }
        }
      }
      if ((((int)uVar8 < 1) &&
          (pUVar4 = *(UObject **)&pUVar6[0x73].ObjectFlags, pUVar4 != (UObject *)0x0)) &&
         (pUVar6 = pUVar4 + 0x14, 0 < (int)pUVar6->ObjectFlags)) {
        pUVar4 = (UObject *)(*(longlong *)pUVar4[0x14].vftable + 0x28);
        do {
          if (*(longlong *)pUVar4->vftable != 0) {
            return CONCAT71((int7)((ulonglong)pUVar4 >> 8),1);
          }
          lVar9 = lVar9 + 1;
          pUVar4 = pUVar4 + 0xe;
        } while (lVar9 < (int)pUVar6->ObjectFlags);
      }
    }
  }
  return (ulonglong)pUVar4 & 0xffffffffffffff00;
}

// AFortPlayerControllerAthena_SetCanStreamBuildingFoundationsIn exec 0x142a5cfb0
// Out-of-line dependency 0x1419f3830; parameter ABI remains to be checked.
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void U_1419f3830(longlong *param_1,undefined1 param_2)

{
  longlong lVar1;
  undefined8 uVar2;
  int aiStackX_10 [6];
  
  uVar2 = (**(code **)(*param_1 + 0x150))();
  U_1407bda90(&DAT_14785c5a0,aiStackX_10,uVar2);
  if ((aiStackX_10[0] == -1) ||
     (lVar1 = _DAT_14785c5a0 + (longlong)aiStackX_10[0] * 0x18, lVar1 == 0)) {
    uRam000000000000015a = param_2;
    return;
  }
  *(undefined1 *)(*(longlong *)(lVar1 + 8) + 0x15a) = param_2;
  return;
}

// AFortPlayerControllerAthena_GetSquadMemberPlayerStates exec 0x142a58ae0
// Out-of-line dependency 0x1419d8fc0; parameter ABI remains to be checked.
undefined ** U_1419d8fc0(UObject *param_1,undefined **param_2,char param_3)

{
  undefined *puVar1;
  FName FVar2;
  UObject *pUVar3;
  UObject *pUVar4;
  longlong *plVar5;
  FName *pFVar6;
  FName *pFVar7;
  undefined **ppuVar8;
  ulonglong uVar9;
  longlong lVar10;
  undefined *this;
  longlong lVar11;
  int iVar12;
  int iVar13;
  undefined *Ptr;
  int iVar14;
  bool bVar15;
  undefined *puStack_48;
  int iStack_40;
  int iStack_3c;
  
  iVar14 = 0;
  *param_2 = (undefined *)0x0;
  param_2[1] = (undefined *)0x0;
  ppuVar8 = param_2;
  pUVar3 = GetFortGameStateAthena(param_1);
  if (pUVar3 == (UObject *)0x0) {
    return param_2;
  }
  FVar2 = param_1[0xd].NamePrivate;
  if (FVar2 != (FName)0x0) {
    pUVar4 = AFortPlayerStateAthena_GetPrivateStaticClass();
    ppuVar8 = *(undefined ***)((longlong)FVar2 + 0x10);
    if ((*(int *)pUVar4[1].ClassPrivate <= *(int *)(ppuVar8 + 7)) &&
       ((uint *)*(undefined1 **)(ppuVar8[6] + (longlong)*(int *)pUVar4[1].ClassPrivate * 8) ==
        &pUVar4[1].ObjectFlags)) {
      uVar9 = (ulonglong)*(byte *)((longlong)FVar2 + 0xfdc);
      goto LAB_1419d9040;
    }
  }
  uVar9 = (ulonglong)ppuVar8 & 0xffffffffffffff00;
LAB_1419d9040:
  plVar5 = (longlong *)U_1419982c0(pUVar3,uVar9);
  Ptr = (undefined *)0x0;
  puStack_48 = (undefined *)0x0;
  iVar13 = 0;
  iStack_3c = 0;
  this = (undefined *)*plVar5;
  puVar1 = this + (longlong)(int)plVar5[1] * 8;
  for (; this != puVar1; this = this + 8) {
    pUVar3 = FWeakObjectPtr_Get(this);
    if (pUVar3 != (UObject *)0x0) {
      iStack_40 = iVar13 + 1;
      if (iVar14 < iStack_40) {
        TArray8_ResizeGrow((undefined *)&puStack_48,iVar13);
        Ptr = puStack_48;
        iVar14 = iStack_3c;
      }
      *(UObject **)(Ptr + (longlong)iVar13 * 8) = pUVar3;
      iVar13 = iStack_40;
    }
  }
  if (param_2 == &puStack_48) {
    if (Ptr != (undefined *)0x0) {
      FMemory_Free(Ptr);
    }
  }
  else {
    if (*param_2 != (undefined *)0x0) {
      FMemory_Free(*param_2);
    }
    *param_2 = Ptr;
    *(int *)(param_2 + 1) = iVar13;
    *(int *)((longlong)param_2 + 0xc) = iVar14;
  }
  if ((param_3 == '\0') && (FVar2 = param_1[0xd].NamePrivate, FVar2 != (FName)0x0)) {
    pUVar3 = AFortPlayerStateAthena_GetPrivateStaticClass();
    if ((*(int *)pUVar3[1].ClassPrivate <= *(int *)(*(longlong *)((longlong)FVar2 + 0x10) + 0x38))
       && ((uint *)*(undefined1 **)
                    (*(longlong *)(*(longlong *)((longlong)FVar2 + 0x10) + 0x30) +
                    (longlong)*(int *)pUVar3[1].ClassPrivate * 8) == &pUVar3[1].ObjectFlags)) {
      pFVar7 = (FName *)*param_2;
      lVar10 = (longlong)*(int *)(param_2 + 1);
      for (pFVar6 = pFVar7; pFVar6 != pFVar7 + lVar10; pFVar6 = pFVar6 + 1) {
        if (*pFVar6 == FVar2) {
          if (*(int *)(param_2 + 1) == 0) {
            return param_2;
          }
          iVar13 = 0;
          bVar15 = *pFVar7 != FVar2;
          lVar11 = 0;
          iVar14 = 0;
          do {
            lVar11 = lVar11 + 1;
            iVar12 = iVar14 + 1;
            if (lVar11 < lVar10) {
              pFVar7 = (FName *)(*param_2 + lVar11 * 8);
              do {
                if (bVar15 != (*pFVar7 != FVar2)) break;
                iVar12 = iVar12 + 1;
                lVar11 = lVar11 + 1;
                pFVar7 = pFVar7 + 1;
              } while (lVar11 < lVar10);
            }
            if (bVar15 != false) {
              if (iVar13 != iVar14) {
                memmove(*param_2 + (longlong)iVar13 * 8,*param_2 + (longlong)iVar14 * 8,
                        (longlong)(iVar12 - iVar14) << 3);
              }
              iVar13 = iVar13 + (iVar12 - iVar14);
            }
            bVar15 = (bool)(bVar15 ^ 1);
            iVar14 = iVar12;
            if (lVar10 <= lVar11) {
              *(int *)(param_2 + 1) = iVar13;
              return param_2;
            }
          } while( true );
        }
      }
    }
  }
  return param_2;
}

// AFortPlayerControllerAthena_GetProfilePlotEntries exec 0x142a58970
// Out-of-line dependency 0x1419d60f0; parameter ABI remains to be checked.
ulonglong U_1419d60f0(longlong param_1,char param_2)

{
  int *piVar1;
  undefined8 uVar2;
  int iVar3;
  undefined8 *puVar4;
  longlong *plVar5;
  longlong lVar6;
  ulonglong uVar7;
  longlong lVar8;
  longlong lVar9;
  longlong *plVar10;
  longlong *plVar11;
  uint uVar12;
  uint uVar13;
  longlong *plStack_28;
  undefined8 uStack_20;
  
  iVar3 = U_143f7a920();
  if (iVar3 == 1) {
    if (*(longlong *)(param_1 + 0x1b10) == 0) {
      uVar7 = 0;
    }
    else {
      puVar4 = (undefined8 *)U_1421d91b0();
      plVar11 = (longlong *)0x0;
      plStack_28 = (longlong *)0x0;
      iVar3 = *(int *)(puVar4 + 1);
      uVar7 = (ulonglong)iVar3;
      uVar2 = *puVar4;
      uStack_20 = CONCAT44(uStack_20._4_4_,iVar3);
      if (iVar3 != 0) {
        U_1406d93e0(&plStack_28,iVar3,0);
        plVar11 = plStack_28;
        memcpy(plStack_28,uVar2,uVar7 << 3);
        uVar7 = uStack_20 & 0xffffffff;
      }
      plVar10 = plVar11;
      if (param_2 == '\0') {
        iVar3 = 0;
        plStack_28 = (longlong *)0x0;
        uVar13 = 0;
        uStack_20 = uStack_20 & 0xffffffff;
        plVar10 = (longlong *)0x0;
        for (plVar5 = plVar11; plVar5 != plVar11 + (int)uVar7; plVar5 = plVar5 + 1) {
          lVar6 = *plVar5;
          if ((lVar6 != 0) && (*(longlong *)(lVar6 + 0x198) == 0x2bca2875f4373fff)) {
            lVar8 = (longlong)(int)uVar13;
            uVar12 = uVar13 + 1;
            uStack_20._4_4_ = (int)(uStack_20 >> 0x20);
            uStack_20 = CONCAT44(uStack_20._4_4_,uVar12);
            if (iVar3 < (int)uVar12) {
              TArray8_ResizeGrow((undefined *)&plStack_28,uVar13);
              lVar6 = *plVar5;
              plVar10 = plStack_28;
              iVar3 = uStack_20._4_4_;
              uVar12 = (uint)uStack_20;
            }
            uVar13 = uVar12;
            plVar10[lVar8] = lVar6;
          }
        }
        if (plVar11 != (longlong *)0x0) {
          FMemory_Free((undefined *)plVar11);
        }
        uVar7 = (ulonglong)uVar13;
      }
      if (plVar10 != (longlong *)0x0) {
        FMemory_Free((undefined *)plVar10);
      }
      uVar7 = uVar7 & 0xffffffff;
    }
  }
  else if (param_2 == '\0') {
    lVar6 = *(longlong *)(param_1 + 0x2fc0);
    plVar11 = (longlong *)0x0;
    uVar13 = 0;
    plStack_28 = (longlong *)0x0;
    iVar3 = 0;
    lVar8 = (longlong)*(int *)(param_1 + 0x2fc8) * 0x58 + lVar6;
    uStack_20._0_4_ = 0;
    uStack_20._4_4_ = 0;
    uVar12 = uVar13;
    if (lVar6 != lVar8) {
      do {
        if (*(char *)(lVar6 + 0x50) == '\0') {
          lVar9 = (longlong)(int)uVar13;
          uStack_20._0_4_ = uVar13 + 1;
          if (iVar3 < (int)(uint)uStack_20) {
            U_14072b960(&plStack_28,uVar13);
            plVar11 = plStack_28;
            iVar3 = uStack_20._4_4_;
          }
          uVar13 = (uint)uStack_20;
          U_1419b8fb0(plVar11 + lVar9 * 0xb,lVar6);
        }
        lVar6 = lVar6 + 0x58;
      } while (lVar6 != lVar8);
      uVar12 = 0;
      if (uVar13 != 0) {
        plVar11 = plVar11 + 3;
        do {
          if ((undefined *)plVar11[2] != (undefined *)0x0) {
            FMemory_Free((undefined *)plVar11[2]);
          }
          plVar10 = (longlong *)*plVar11;
          if (plVar10 != (longlong *)0x0) {
            LOCK();
            plVar5 = plVar10 + 1;
            lVar6 = *plVar5;
            *(int *)plVar5 = (int)*plVar5 + -1;
            UNLOCK();
            if ((int)lVar6 == 1) {
              (**(code **)*plVar10)(plVar10);
              LOCK();
              piVar1 = (int *)((longlong)plVar10 + 0xc);
              iVar3 = *piVar1;
              *piVar1 = *piVar1 + -1;
              UNLOCK();
              if (iVar3 == 1) {
                (**(code **)(*plVar10 + 8))(plVar10,1);
              }
            }
          }
          if ((undefined *)plVar11[-3] != (undefined *)0x0) {
            FMemory_Free((undefined *)plVar11[-3]);
          }
          plVar11 = plVar11 + 0xb;
          uVar13 = uVar13 - 1;
        } while (uVar13 != 0);
        plVar11 = plStack_28;
        uVar12 = (uint)uStack_20;
      }
      if (plVar11 != (longlong *)0x0) {
        FMemory_Free((undefined *)plVar11);
      }
    }
    uVar7 = (ulonglong)uVar12;
  }
  else {
    uVar7 = (ulonglong)*(uint *)(param_1 + 0x2fc8);
  }
  return uVar7;
}

// AFortPlayerControllerAthena_GetCurrentFortVolumeType exec 0x142a586b0
// Out-of-line dependency 0x1419d3600; parameter ABI remains to be checked.
undefined1 U_1419d3600(longlong param_1)

{
  undefined1 uVar1;
  longlong lVar2;
  
  lVar2 = U_141c74820();
  if (lVar2 != 0) {
    lVar2 = U_141c7d420(lVar2,*(undefined8 *)(param_1 + 0x220));
    if (lVar2 != 0) {
      uVar1 = U_141c77c90(lVar2);
      return uVar1;
    }
  }
  return 0;
}

// AFortPlayerControllerAthena_GetSquadKills exec 0x142a58a00
// Out-of-line dependency 0x1419d8d90; parameter ABI remains to be checked.
void U_1419d8d90(undefined8 param_1,longlong *param_2)

{
  undefined *puVar1;
  int iVar2;
  bool bVar3;
  longlong *plVar4;
  UObject *pUVar5;
  undefined4 uVar6;
  undefined *this;
  
  *(undefined4 *)(param_2 + 1) = 0;
  if (*(int *)((longlong)param_2 + 0xc) < 0) {
    U_14072d2e0(param_2,0);
  }
  plVar4 = (longlong *)U_1419d9220(param_1);
  this = (undefined *)*plVar4;
  puVar1 = this + (longlong)(int)plVar4[1] * 8;
  for (; this != puVar1; this = this + 8) {
    uVar6 = 0;
    bVar3 = FWeakObjectPtr_IsValid(this);
    if (bVar3) {
      pUVar5 = FWeakObjectPtr_Get(this);
      uVar6 = *(undefined4 *)(pUVar5[0x5c].ClassPrivate + 4);
    }
    iVar2 = (int)param_2[1];
    *(int *)(param_2 + 1) = iVar2 + 1;
    if (*(int *)((longlong)param_2 + 0xc) < iVar2 + 1) {
      U_140690b30(param_2,iVar2);
    }
    *(undefined4 *)(*param_2 + (longlong)iVar2 * 4) = uVar6;
  }
  return;
}

// AFortPlayerControllerAthena_CancelMatchmaking exec 0x142a56420
// Out-of-line dependency 0x1419c3fb0; parameter ABI remains to be checked.
void U_1419c3fb0(longlong *param_1,undefined1 param_2)

{
  longlong lVar1;
  longlong lVar2;
  
  lVar1 = (**(code **)(*param_1 + 0x150))();
  if ((lVar1 != 0) && (lVar1 = *(longlong *)(lVar1 + 0x188), lVar1 != 0)) {
    lVar2 = U_1428ea7f0();
    if ((*(int *)(lVar2 + 0x38) <= *(int *)(*(longlong *)(lVar1 + 0x10) + 0x38)) &&
       (((*(longlong *)
           (*(longlong *)(*(longlong *)(lVar1 + 0x10) + 0x30) + (longlong)*(int *)(lVar2 + 0x38) * 8
           ) == lVar2 + 0x30 && (lVar1 = U_141e613c0(lVar1), lVar1 != 0)) &&
        (*(char *)(lVar1 + 0x118) != '\0')))) {
      U_1421cb250(lVar1,param_2);
    }
  }
  return;
}

// AFortPlayerControllerAthena_SetFocalPoint exec 0x142a5d040
// Out-of-line dependency 0x1419f3e70; parameter ABI remains to be checked.
void U_1419f3e70(longlong param_1,longlong param_2,undefined8 *param_3,undefined4 param_4)

{
  undefined4 uVar1;
  undefined8 uStackX_8;
  undefined8 uStackX_10;
  undefined *puStack_48;
  undefined *apuStack_38 [4];
  
  if (((param_2 != 0) && (*(longlong *)(param_1 + 0x2b40) != 0)) && (2 < DAT_1481348e0)) {
    uStackX_8 = *(undefined8 *)(*(longlong *)(param_1 + 0x2b40) + 0x18);
    U_142ce9f90(&uStackX_8,apuStack_38);
    uStackX_10 = *(undefined8 *)(param_2 + 0x18);
    U_142ce9f90();
    FMsg_Logf_Internal((byte *)0x0,0,&DAT_1481348e4,3,
                       (ushort *)L"SetFocalPoint %s stomping previous focal point %s");
    if (puStack_48 != (undefined *)0x0) {
      FMemory_Free(puStack_48);
    }
    if (apuStack_38[0] != (undefined *)0x0) {
      FMemory_Free(apuStack_38[0]);
    }
  }
  uVar1 = *(undefined4 *)(param_3 + 1);
  *(undefined8 *)(param_1 + 0x2b48) = *param_3;
  *(undefined4 *)(param_1 + 0x2b54) = param_4;
  *(undefined4 *)(param_1 + 0x2b50) = uVar1;
  *(longlong *)(param_1 + 0x2b40) = param_2;
  return;
}

// AFortPlayerControllerAthena_IsUnderPlotLimit exec 0x142a59080
// Out-of-line dependency 0x1419dde30; parameter ABI remains to be checked.
bool U_1419dde30(longlong param_1,int *param_2,int *param_3,undefined1 param_4)

{
  int iVar1;
  
  iVar1 = U_143f7a920();
  if (iVar1 == 1) {
    if (*(longlong *)(param_1 + 0x1b10) == 0) {
      iVar1 = 0;
    }
    else {
      iVar1 = U_1421d7c50();
      *(int *)(param_1 + 0x2e7c) = iVar1;
    }
  }
  else {
    iVar1 = *(int *)(param_1 + 0x2e7c);
  }
  *param_2 = iVar1;
  iVar1 = U_1419d60f0(param_1,param_4);
  *param_3 = iVar1;
  return iVar1 < *param_2;
}

// AFortPlayerControllerAthena_SetFocalPointWithDuration exec 0x142a5d160
// Out-of-line dependency 0x1419f3fa0; parameter ABI remains to be checked.
void U_1419f3fa0(longlong param_1,longlong param_2,undefined8 *param_3,undefined4 param_4,
                  undefined4 param_5)

{
  undefined4 uVar1;
  undefined8 uStackX_8;
  undefined8 uStackX_10;
  undefined *puStack_48;
  undefined *apuStack_38 [4];
  
  if (((param_2 != 0) && (*(longlong *)(param_1 + 0x2b40) != 0)) && (2 < DAT_1481348e0)) {
    uStackX_8 = *(undefined8 *)(*(longlong *)(param_1 + 0x2b40) + 0x18);
    U_142ce9f90(&uStackX_8,apuStack_38);
    uStackX_10 = *(undefined8 *)(param_2 + 0x18);
    U_142ce9f90();
    FMsg_Logf_Internal((byte *)0x0,0,&DAT_1481348e4,3,
                       (ushort *)L"SetFocalPointWithDuration %s stomping previous focal point %s");
    if (puStack_48 != (undefined *)0x0) {
      FMemory_Free(puStack_48);
    }
    if (apuStack_38[0] != (undefined *)0x0) {
      FMemory_Free(apuStack_38[0]);
    }
  }
  uVar1 = *(undefined4 *)(param_3 + 1);
  *(undefined8 *)(param_1 + 0x2b48) = *param_3;
  *(undefined4 *)(param_1 + 0x2b54) = param_4;
  *(undefined4 *)(param_1 + 0x2b58) = param_5;
  *(longlong *)(param_1 + 0x2b40) = param_2;
  *(undefined4 *)(param_1 + 0x2b50) = uVar1;
  return;
}

// AFortPlayerControllerAthena_ForceSpectatorToCursorMode exec 0x142a58620
// Out-of-line dependency 0x141c794e0; parameter ABI remains to be checked.
undefined8 U_141c794e0(longlong *param_1)

{
  longlong lVar1;
  undefined *puVar2;
  longlong *plVar3;
  undefined8 uVar4;
  
  if (param_1 != (longlong *)0x0) {
    lVar1 = (**(code **)(*param_1 + 0x150))();
    if (lVar1 != 0) {
      lVar1 = *(longlong *)(lVar1 + 0x130);
      if (lVar1 != 0) {
        puVar2 = AFortGameStateAthena_StaticClass();
        if ((*(int *)(puVar2 + 0x38) <= *(int *)(*(longlong *)(lVar1 + 0x10) + 0x38)) &&
           (*(undefined **)
             (*(longlong *)(*(longlong *)(lVar1 + 0x10) + 0x30) +
             (longlong)*(int *)(puVar2 + 0x38) * 8) == puVar2 + 0x30)) {
          lVar1 = U_141999ba0(lVar1);
          if (lVar1 != 0) {
            plVar3 = (longlong *)U_141c7d420(lVar1,param_1);
            if (plVar3 != (longlong *)0x0) {
              uVar4 = U_1429da540();
              lVar1 = (**(code **)(*plVar3 + 0x610))(plVar3,uVar4);
              if (lVar1 != 0) {
                return *(undefined8 *)(lVar1 + 0xd0);
              }
            }
          }
        }
      }
      return 0;
    }
  }
  return 0;
}
// Out-of-line dependency 0x14209aa00; parameter ABI remains to be checked.
undefined1 U_14209aa00(longlong param_1,longlong param_2)

{
  longlong lVar1;
  
  if (param_2 != 0) {
    lVar1 = U_14208d7e0(param_1 + 0x558,param_2 + 0x248);
    if (lVar1 != 0) {
      return 1;
    }
  }
  return 0;
}

// AFortPlayerControllerAthena_CheckIfLevelStreamingCompleteForHandshake exec 0x142a564c0
// Out-of-line dependency 0x1419dbc20; parameter ABI remains to be checked.
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void U_1419dbc20(longlong *param_1)

{
  undefined8 uVar1;
  char *pcVar2;
  longlong *plVar3;
  float fVar4;
  bool bVar5;
  bool bVar6;
  float fVar7;
  float fVar8;
  bool bVar9;
  char cVar10;
  uint uVar11;
  longlong lVar12;
  longlong lVar13;
  longlong lVar14;
  longlong lVar15;
  char *pcVar16;
  longlong lVar17;
  float *pfVar18;
  longlong *plVar19;
  int iVar20;
  uint uVar21;
  int iVar22;
  longlong *plVar23;
  int iVar24;
  float fVar25;
  undefined1 auStack_118 [32];
  undefined8 uStack_f8;
  undefined4 uStack_f0;
  longlong *plStack_e8;
  undefined8 uStack_e0;
  float fStack_d0;
  float fStack_cc;
  float fStack_c8;
  float fStack_c4;
  undefined8 uStack_c0;
  uint uStack_b0;
  longlong lStack_a8;
  undefined *puStack_a0;
  int iStack_98;
  int iStack_90;
  char cStack_8c;
  int iStack_88;
  longlong lStack_80;
  undefined *puStack_78;
  int iStack_70;
  undefined8 uStack_60;
  char cStack_58;
  ulonglong uStack_50;
  
  uStack_50 = _DAT_1480c4288 ^ (ulonglong)auStack_118;
  plStack_e8 = param_1;
  lVar12 = (**(code **)(*param_1 + 0x150))();
  if (lVar12 == 0) goto LAB_1419dc243;
  if ((param_1[0x5ce] == 0) || (_DAT_14811365c == 0)) {
    plVar23 = *(longlong **)(lVar12 + 0x98);
    plVar19 = plVar23 + *(int *)(lVar12 + 0xa0);
    for (; plVar23 != plVar19; plVar23 = plVar23 + 1) {
      plVar3 = (longlong *)*plVar23;
      if (((plVar3 != (longlong *)0x0) && ((char)plVar3[0x17] != '\x02')) &&
         (((cVar10 = U_1442dd540(plVar3), cVar10 == '\0' &&
           (cVar10 = (**(code **)(*plVar3 + 0x270))(plVar3), cVar10 != '\0')) ||
          ((plVar3[0x25] != 0 && (cVar10 = (**(code **)(*plVar3 + 0x268))(plVar3), cVar10 == '\0')))
          ))) goto LAB_1419dc243;
    }
  }
  else {
    lVar13 = *(longlong *)(param_1[0x5ce] + 0x130);
    if (lVar13 == 0) {
      uStack_f8 = _DAT_148218630;
      uStack_f0 = _DAT_148218638;
    }
    else {
      uStack_f8 = *(undefined8 *)(lVar13 + 0x1d0);
      uStack_f0 = *(undefined4 *)(lVar13 + 0x1d8);
    }
    uStack_e0 = uStack_f8;
    lVar13 = U_14273ec00();
    if ((lVar13 == 0) || (lVar14 = U_14273ec00(), lVar14 == 0)) {
LAB_1419dbd1d:
      lVar13 = 0;
    }
    else if ((*(int *)(lVar13 + 0x38) < *(int *)(lVar14 + 0x38)) ||
            (*(longlong *)(*(longlong *)(lVar13 + 0x30) + (longlong)*(int *)(lVar14 + 0x38) * 8) !=
             lVar14 + 0x30)) goto LAB_1419dbd1d;
    uStack_b0 = 5;
    cStack_58 = 0;
    U_140894360(&lStack_a8,lVar12,lVar13);
    lVar12 = lStack_a8;
    iVar22 = iStack_90 + 1;
    cStack_58 = '\x01';
    if (iVar22 < iStack_98 + iStack_70) {
      lVar13 = (longlong)iVar22 * 8;
      iVar20 = iStack_98;
      iVar24 = iStack_70;
      do {
        if (iVar22 < iVar20) {
          lVar14 = *(longlong *)(puStack_a0 + lVar13);
        }
        else {
          lVar14 = *(longlong *)(puStack_78 + (longlong)(iVar22 - iVar20) * 8);
        }
        iStack_88 = iStack_88 + 1;
        if (lVar14 != 0) {
          lVar15 = U_143f76730(lVar14);
          if (lVar15 != 0) {
            if ((uStack_b0 & 1) != 0) {
              iVar20 = *(int *)(lVar14 + 0xc);
              if (iVar20 < _DAT_148263a2c) {
                uVar21 = iVar20 >> 0x1f & 0xffff;
                uVar11 = iVar20 + uVar21;
                lVar17 = *(longlong *)(_DAT_148263a18 + (longlong)((int)uVar11 >> 0x10) * 8) +
                         (longlong)(int)((uVar11 & 0xffff) - uVar21) * 0x18;
              }
              else {
                lVar17 = 0;
              }
              if ((*(uint *)(lVar17 + 8) >> 0x1d & 1) != 0) goto LAB_1419dbeba;
            }
            if (((uStack_b0 >> 1 & 1) == 0) || (bVar9 = Stub_ReturnFalse(), bVar9)) {
              if ((uStack_b0 >> 2 & 1) != 0) {
                if (((*(byte *)(lVar15 + 500) & 0x20) == 0) &&
                   ((*(byte *)(lVar15 + 0x1f5) & 0x40) == 0)) {
                  bVar9 = false;
                }
                else {
                  bVar9 = true;
                }
                pcVar2 = *(char **)(lVar15 + 0x248);
                if (((*(longlong *)(lVar15 + 0xb8) == 0) ||
                    (pcVar16 = (char *)U_14468e190(), pcVar16 == (char *)0x0)) ||
                   (pcVar2 == pcVar16)) {
                  bVar5 = true;
                }
                else {
                  bVar5 = false;
                }
                if ((pcVar2 == (char *)0x0) || (*pcVar2 == '\x02')) {
                  bVar6 = true;
                }
                else {
                  bVar6 = false;
                }
                if ((bVar5) || (bVar5 = false, bVar6)) {
                  bVar5 = true;
                }
                if ((!bVar9) || (!bVar5)) goto LAB_1419dbeba;
              }
              lVar17 = U_1413064f0(lVar15);
              if (lVar17 == lVar12) {
                if (lVar15 != *(longlong *)(lVar12 + 0x30)) {
                  lVar15 = U_14480eb00();
                  if ((*(int *)(lVar15 + 0x38) <= *(int *)(*(longlong *)(lVar14 + 0x10) + 0x38)) &&
                     (*(longlong *)
                       (*(longlong *)(*(longlong *)(lVar14 + 0x10) + 0x30) +
                       (longlong)*(int *)(lVar15 + 0x38) * 8) == lVar15 + 0x30)) goto LAB_1419dbeba;
                }
                iStack_90 = iVar22;
                lStack_80 = lVar14;
                if (cStack_8c != '\0') goto LAB_1419dbedf;
                fVar8 = uStack_e0._4_4_;
                fVar7 = (float)uStack_e0;
                goto LAB_1419dbf86;
              }
            }
          }
LAB_1419dbeba:
          iVar20 = iStack_98;
          iVar24 = iStack_70;
        }
        iVar22 = iVar22 + 1;
        lVar13 = lVar13 + 8;
      } while (iVar22 < iVar20 + iVar24);
    }
LAB_1419dbed3:
    cStack_8c = '\x01';
    lStack_80 = 0;
LAB_1419dbedf:
    if (cStack_58 != '\0') {
      cStack_58 = '\0';
      U_14469a960(lStack_a8,uStack_60);
      if (puStack_78 != (undefined *)0x0) {
        FMemory_Free(puStack_78);
      }
      if (puStack_a0 != (undefined *)0x0) {
        FMemory_Free(puStack_a0);
      }
    }
    param_1 = plStack_e8;
  }
  iVar22 = (int)param_1[0x5b5];
  while (iVar22 = iVar22 + -1, -1 < iVar22) {
    lVar12 = (longlong)iVar22;
    if (((*(int *)(param_1[0x5b4] + 8 + lVar12 * 0x10) == 0) ||
        (plVar19 = *(longlong **)(param_1[0x5b4] + lVar12 * 0x10), plVar19 == (longlong *)0x0)) ||
       (cVar10 = (**(code **)(*plVar19 + 0x28))(), cVar10 == '\0')) {
      bVar9 = false;
    }
    else {
      bVar9 = true;
    }
    if (bVar9) {
      if (*(int *)(param_1[0x5b4] + 8 + lVar12 * 0x10) == 0) {
        plVar19 = (longlong *)0x0;
      }
      else {
        plVar19 = *(longlong **)(param_1[0x5b4] + lVar12 * 0x10);
      }
      cVar10 = (**(code **)(*plVar19 + 0x48))(plVar19,param_1);
      if (cVar10 == '\0') break;
    }
    if ((iVar22 < 0) || ((int)param_1[0x5b5] <= iVar22)) {
      bVar9 = false;
    }
    else {
      bVar9 = true;
    }
    if (!bVar9) {
      iVar22 = (int)param_1[0x5b5];
    }
  }
LAB_1419dc243:
  U_145803c70(uStack_50 ^ (ulonglong)auStack_118);
  return;
LAB_1419dbf86:
  if (((lStack_80 != 0) && (plVar19 = *(longlong **)(lStack_80 + 0xda0), plVar19 != (longlong *)0x0)
      ) && ((char)plVar19[0x17] != '\x02')) {
    if ((plVar19[0x25] != 0) && (cVar10 = (**(code **)(*plVar19 + 0x268))(plVar19), cVar10 == '\0'))
    {
LAB_1419dc20e:
      if (cStack_58 != '\0') {
        cStack_58 = '\0';
        U_14469a960(lStack_a8,uStack_60);
        if (puStack_78 != (undefined *)0x0) {
          FMemory_Free(puStack_78);
        }
        if (puStack_a0 != (undefined *)0x0) {
          FMemory_Free(puStack_a0);
        }
      }
      goto LAB_1419dc243;
    }
    cVar10 = (**(code **)(*plVar19 + 0x270))(plVar19);
    if ((cVar10 != '\0') && (cVar10 = U_1442dd540(plVar19), cVar10 == '\0')) {
      pfVar18 = (float *)U_141bdf240(lStack_80);
      fVar25 = 0.0;
      fStack_d0 = *pfVar18;
      fStack_cc = pfVar18[1];
      fStack_c8 = pfVar18[2];
      fStack_c4 = pfVar18[3];
      uVar1 = *(undefined8 *)(pfVar18 + 4);
      fVar4 = fStack_d0;
      if ((fVar7 < fStack_d0) || (fVar4 = fStack_c4, fStack_c4 < fVar7)) {
        fVar25 = (fVar7 - fVar4) * (fVar7 - fVar4);
      }
      uStack_c0._0_4_ = fStack_cc;
      if ((fVar8 < fStack_cc) || (uStack_c0._0_4_ = (float)uVar1, (float)uStack_c0 < fVar8)) {
        fVar25 = fVar25 + (fVar8 - (float)uStack_c0) * (fVar8 - (float)uStack_c0);
      }
      uStack_c0 = uVar1;
      if (fVar25 <= _DAT_1478c283c * _DAT_1478c283c) goto LAB_1419dc20e;
    }
  }
  lVar12 = lStack_a8;
  iVar22 = iStack_90 + 1;
  if (iVar22 < iStack_98 + iStack_70) {
    lVar13 = (longlong)iVar22 * 8;
    iVar20 = iStack_98;
    iVar24 = iStack_70;
    do {
      if (iVar22 < iVar20) {
        lVar14 = *(longlong *)(puStack_a0 + lVar13);
      }
      else {
        lVar14 = *(longlong *)(puStack_78 + (longlong)(iVar22 - iVar20) * 8);
      }
      iStack_88 = iStack_88 + 1;
      if (lVar14 != 0) {
        lVar15 = U_143f76730(lVar14);
        if (lVar15 != 0) {
          if ((uStack_b0 & 1) != 0) {
            iVar20 = *(int *)(lVar14 + 0xc);
            if (iVar20 < _DAT_148263a2c) {
              uVar21 = iVar20 >> 0x1f & 0xffff;
              uVar11 = iVar20 + uVar21;
              lVar17 = *(longlong *)(_DAT_148263a18 + (longlong)((int)uVar11 >> 0x10) * 8) +
                       (longlong)(int)((uVar11 & 0xffff) - uVar21) * 0x18;
            }
            else {
              lVar17 = 0;
            }
            if ((*(uint *)(lVar17 + 8) >> 0x1d & 1) != 0) goto LAB_1419dc1da;
          }
          if (((uStack_b0 >> 1 & 1) == 0) || (bVar9 = Stub_ReturnFalse(), bVar9)) {
            if ((uStack_b0 >> 2 & 1) != 0) {
              if (((*(byte *)(lVar15 + 500) & 0x20) == 0) &&
                 ((*(byte *)(lVar15 + 0x1f5) & 0x40) == 0)) {
                bVar9 = false;
              }
              else {
                bVar9 = true;
              }
              pcVar2 = *(char **)(lVar15 + 0x248);
              if (((*(longlong *)(lVar15 + 0xb8) == 0) ||
                  (pcVar16 = (char *)U_14468e190(), pcVar16 == (char *)0x0)) ||
                 (pcVar2 == pcVar16)) {
                bVar5 = true;
              }
              else {
                bVar5 = false;
              }
              if ((pcVar2 == (char *)0x0) || (*pcVar2 == '\x02')) {
                bVar6 = true;
              }
              else {
                bVar6 = false;
              }
              if ((bVar5) || (bVar5 = false, bVar6)) {
                bVar5 = true;
              }
              if ((!bVar9) || (!bVar5)) goto LAB_1419dc1da;
            }
            lVar17 = U_1413064f0(lVar15);
            if (lVar17 == lVar12) {
              if (lVar15 == *(longlong *)(lVar12 + 0x30)) goto LAB_1419dc1f8;
              lVar15 = U_14480eb00();
              if ((*(int *)(*(longlong *)(lVar14 + 0x10) + 0x38) < *(int *)(lVar15 + 0x38)) ||
                 (*(longlong *)
                   (*(longlong *)(*(longlong *)(lVar14 + 0x10) + 0x30) +
                   (longlong)*(int *)(lVar15 + 0x38) * 8) != lVar15 + 0x30)) goto LAB_1419dc1f8;
            }
          }
        }
LAB_1419dc1da:
        iVar20 = iStack_98;
        iVar24 = iStack_70;
      }
      iVar22 = iVar22 + 1;
      lVar13 = lVar13 + 8;
      if (iVar20 + iVar24 <= iVar22) break;
    } while( true );
  }
  goto LAB_1419dbed3;
LAB_1419dc1f8:
  iStack_90 = iVar22;
  lStack_80 = lVar14;
  if (cStack_8c != '\0') goto LAB_1419dbedf;
  goto LAB_1419dbf86;
}
// Out-of-line dependency 0x1419c5210; parameter ABI remains to be checked.
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void U_1419c5210(longlong *param_1)

{
  longlong lVar1;
  int aiStackX_8 [8];
  
  if ((char)param_1[0x1e] == '\x03') {
    return;
  }
  if (_DAT_148351eb8 != 0) {
    U_14463f4b0(_DAT_148351eb8,1);
  }
  lVar1 = (**(code **)(*param_1 + 0x150))(param_1);
  if (lVar1 != 0) {
    U_1407bda90(&DAT_14785c5a0,aiStackX_8,lVar1);
    if (aiStackX_8[0] != -1) {
      lVar1 = _DAT_14785c5a0 + (longlong)aiStackX_8[0] * 0x18;
      if ((lVar1 != 0) && (lVar1 = *(longlong *)(lVar1 + 8), lVar1 != 0)) {
        U_141d58ea0(lVar1,0,L"CleanupLevelStreamingOnClient");
      }
    }
  }
  *(undefined1 *)(param_1 + 0x5b1) = 1;
  U_142a54b40(param_1,1);
  return;
}
// Out-of-line dependency 0x144621fb0; parameter ABI remains to be checked.
longlong U_144621fb0(longlong param_1,longlong *param_2)

{
  longlong lVar1;
  longlong lVar2;
  uint uVar3;
  
  lVar1 = *param_2;
  if (lVar1 != 0) {
    uVar3 = (uint)lVar1 & 0xffffff;
    if ((int)uVar3 < *(int *)(param_1 + 0x10)) {
      lVar2 = param_1 + 0x18;
      if (*(longlong *)(param_1 + 0x28) != 0) {
        lVar2 = *(longlong *)(param_1 + 0x28);
      }
      if ((((*(uint *)(lVar2 + (ulonglong)(uVar3 >> 5) * 4) >> ((uint)lVar1 & 0x1f) & 1) != 0) &&
          (lVar2 = (ulonglong)uVar3 * 0x90 + *(longlong *)(param_1 + 8),
          *(longlong *)(lVar2 + 0x70) == lVar1)) && (*(char *)(lVar2 + 1) != '\x04')) {
        return lVar2;
      }
    }
  }
  return 0;
}

// AFortPlayerControllerAthena_StartMatchmakingByLinkCode exec 0x142a5d550
// Out-of-line dependency 0x1419fb9d0; parameter ABI remains to be checked.
void U_1419fb9d0(longlong *param_1,undefined8 param_2)

{
  longlong lVar1;
  longlong lVar2;
  undefined8 uVar3;
  
  lVar1 = (**(code **)(*param_1 + 0x150))();
  if ((lVar1 != 0) && (lVar1 = *(longlong *)(lVar1 + 0x188), lVar1 != 0)) {
    lVar2 = U_1428ea7f0();
    if ((*(int *)(lVar2 + 0x38) <= *(int *)(*(longlong *)(lVar1 + 0x10) + 0x38)) &&
       ((*(longlong *)
          (*(longlong *)(*(longlong *)(lVar1 + 0x10) + 0x30) + (longlong)*(int *)(lVar2 + 0x38) * 8)
         == lVar2 + 0x30 && (lVar2 = U_14447ee40(param_1), lVar2 != 0)))) {
      uVar3 = U_141e613c0(lVar1);
      U_1421fab00(uVar3,*(undefined4 *)(lVar2 + 0xb8),1,param_2);
    }
  }
  return;
}

// AFortPlayerControllerAthena_RespawnPlayerAfterDeath exec 0x142a5a140
// Out-of-line dependency 0x1419ee720; parameter ABI remains to be checked.
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void U_1419ee720(UObject *param_1,char param_2)

{
  FName FVar1;
  char cVar2;
  UObject *pUVar3;
  longlong *Original;
  longlong lVar4;
  undefined8 uVar5;
  undefined *Ptr;
  longlong lVar6;
  undefined8 *puVar7;
  longlong *plVar8;
  FName FVar9;
  undefined1 auStack_c8 [32];
  undefined1 uStack_a8;
  undefined4 uStack_a0;
  undefined8 uStack_98;
  undefined4 uStack_90;
  undefined4 uStack_8c;
  longlong *plStack_88;
  int iStack_80;
  undefined1 auStack_78 [8];
  undefined8 uStack_70;
  longlong lStack_68;
  longlong *plStack_58;
  longlong alStack_48 [4];
  ulonglong uStack_28;
  
  uStack_28 = _DAT_1480c4288 ^ (ulonglong)auStack_c8;
  FVar9 = param_1[0xd].NamePrivate;
  if (FVar9 == (FName)0x0) {
LAB_1419ee914:
    FVar9.ComparisonIndex = 0;
    FVar9.Number = 0;
  }
  else {
    pUVar3 = AFortPlayerStateAthena_GetPrivateStaticClass();
    if ((*(int *)(*(longlong *)((longlong)FVar9 + 0x10) + 0x38) < *(int *)pUVar3[1].ClassPrivate) ||
       ((uint *)*(undefined1 **)
                 (*(longlong *)(*(longlong *)((longlong)FVar9 + 0x10) + 0x30) +
                 (longlong)*(int *)pUVar3[1].ClassPrivate * 8) != &pUVar3[1].ObjectFlags))
    goto LAB_1419ee914;
    if (*(char *)((longlong)FVar9 + 0xd30) != '\0') {
      U_141a5b590(FVar9,CONCAT71((int7)((ulonglong)&pUVar3[1].ObjectFlags >> 8),1));
      cVar2 = U_141a4b2b0();
      if (cVar2 == '\0') {
        uStack_90 = 0;
        uStack_98 = U_1419ee720;
        Original = (longlong *)FMemory_Realloc((undefined *)0x0,0x30,0);
        if (Original != (longlong *)0x0) {
          *Original = (longlong)&DAT_145daca60;
          U_142ee3b30(Original + 1,param_1);
          *(undefined4 *)(Original + 2) = (undefined4)uStack_98;
          *(undefined4 *)((longlong)Original + 0x14) = uStack_98._4_4_;
          *(undefined4 *)(Original + 3) = uStack_90;
          *(undefined4 *)((longlong)Original + 0x1c) = uStack_8c;
          *(char *)(Original + 4) = param_2;
          lVar4 = U_142bad740();
          Original[5] = lVar4;
          *Original = (longlong)&DAT_145dacab8;
        }
        uVar5 = U_140f95ae0(param_1);
        plStack_88 = (longlong *)0x0;
        iStack_80 = 0;
        if (Original != (longlong *)0x0) {
          (**(code **)(*Original + 0x40))(Original,&plStack_88);
        }
        U_142ee3b30(auStack_78,0);
        uStack_a0 = 0xbf800000;
        uStack_a8 = 0;
        uStack_70 = 0;
        lStack_68 = 0;
        plStack_58 = (longlong *)0x0;
        U_1446278d0(uVar5,&param_1[0x129].NamePrivate,&plStack_88,0x3dcccccd);
        if (lStack_68 != 0) {
          plVar8 = alStack_48;
          if (plStack_58 != (longlong *)0x0) {
            plVar8 = plStack_58;
          }
          (**(code **)(*plVar8 + 0x10))();
        }
        if (iStack_80 == 0) {
LAB_1419ee8d0:
          if (plStack_88 != (longlong *)0x0) {
            FMemory_Free((undefined *)plStack_88);
          }
        }
        else if (plStack_88 != (longlong *)0x0) {
          (**(code **)(*plStack_88 + 0x38))();
          if (plStack_88 != (longlong *)0x0) {
            plStack_88 = (longlong *)FMemory_Realloc((undefined *)plStack_88,0,0);
          }
          iStack_80 = 0;
          goto LAB_1419ee8d0;
        }
        if (Original != (longlong *)0x0) {
          (**(code **)(*Original + 0x38))(Original);
          Ptr = FMemory_Realloc((undefined *)Original,0,0);
          if (Ptr != (undefined *)0x0) {
            FMemory_Free(Ptr);
          }
        }
        goto LAB_1419eea7e;
      }
    }
  }
  lVar4 = (**(code **)(*(longlong *)param_1->vftable + 0x150))(param_1);
  if (lVar4 != 0) {
    lVar4 = (**(code **)(*(longlong *)param_1->vftable + 0x150))(param_1);
    lVar4 = *(longlong *)(lVar4 + 0x128);
    if (lVar4 != 0) {
      lVar6 = U_1428ee970();
      if (((*(int *)(lVar6 + 0x38) <= *(int *)(*(longlong *)(lVar4 + 0x10) + 0x38)) &&
          (*(longlong *)
            (*(longlong *)(*(longlong *)(lVar4 + 0x10) + 0x30) +
            (longlong)*(int *)(lVar6 + 0x38) * 8) == lVar6 + 0x30)) &&
         (cVar2 = U_14194cb40(lVar4,param_1), cVar2 != '\0')) goto LAB_1419eea7e;
    }
  }
  (**(code **)(*(longlong *)param_1->vftable + 0x828))(param_1);
  FVar1 = param_1[0xe].NamePrivate;
  if (FVar1 != (FName)0x0) {
    lVar6 = U_142a82010();
    lVar4 = *(longlong *)((longlong)FVar1 + 0x10);
    if ((*(int *)(lVar6 + 0x38) <= *(int *)(lVar4 + 0x38)) &&
       (*(longlong *)(*(longlong *)(lVar4 + 0x30) + (longlong)*(int *)(lVar6 + 0x38) * 8) ==
        lVar6 + 0x30)) {
      if (param_2 != '\0') {
        thunk_FUN_140c31764(FVar1,CONCAT71((int7)((ulonglong)lVar4 >> 8),1));
      }
      U_14232b1f0(FVar1);
    }
  }
  if (FVar9 != (FName)0x0) {
    puVar7 = (undefined8 *)U_141a42550(FVar9);
    *puVar7 = 0;
    puVar7[1] = 0;
    *(undefined2 *)(puVar7 + 2) = 0x3200;
    *(undefined1 *)((longlong)puVar7 + 0x12) = 0xff;
    *(undefined4 *)((longlong)puVar7 + 0x14) = 0;
    puVar7[3] = _DAT_148218630;
    *(undefined4 *)(puVar7 + 4) = _DAT_148218638;
    *(undefined1 *)((longlong)puVar7 + 0x24) = 0;
    *(undefined1 *)(puVar7 + 9) = 0;
    U_143e96ff0(puVar7 + 5,0);
    *(int *)((longlong)FVar9 + 0x129c) = *(int *)((longlong)FVar9 + 0x129c) + 1;
    U_141a3ca60(FVar9);
  }
  param_1[0x18].ObjectFlags = param_1[0x18].ObjectFlags & 0xffffffef;
  *(undefined8 *)param_1[0x33].vftable = 0;
  param_1[0x129].ClassPrivate[0] = 0;
  lVar4 = U_141c74820(param_1);
  if (((lVar4 != 0) && (lVar4 = U_141c7d420(lVar4,param_1[0xd].NamePrivate), lVar4 != 0)) &&
     (cVar2 = U_141c821a0(lVar4), cVar2 != '\0')) {
    *(undefined1 *)&param_1[0x122].NamePrivate.ComparisonIndex = 0;
  }
  pUVar3 = GetFortGameStateAthena(param_1);
  if (pUVar3 != (UObject *)0x0) {
    U_14199e430(pUVar3);
  }
LAB_1419eea7e:
  U_145803c70(uStack_28 ^ (ulonglong)auStack_c8);
  return;
}

// AFortPlayerControllerAthena_RemoveDroppableTaggedItems exec 0x142a5a020
// Out-of-line dependency 0x1409bc4c0; parameter ABI remains to be checked.
void U_1409bc4c0(undefined8 param_1,undefined8 *param_2)

{
  if ((undefined *)param_2[2] != (undefined *)0x0) {
    FMemory_Free((undefined *)param_2[2]);
  }
  if ((undefined *)*param_2 == (undefined *)0x0) {
    return;
  }
  FMemory_Free((undefined *)*param_2);
  return;
}

// AFortPlayerControllerAthena_OnFortPlayerPawnAthenaDied exec 0x142a59580
// Out-of-line dependency 0x1419e17b0; parameter ABI remains to be checked.
void U_1419e17b0(undefined8 param_1,longlong param_2)

{
  longlong lVar1;
  longlong lVar2;
  
  lVar1 = *(longlong *)(param_2 + 0x238);
  if (lVar1 != 0) {
    lVar2 = U_142a85850();
    if ((*(int *)(lVar2 + 0x38) <= *(int *)(*(longlong *)(lVar1 + 0x10) + 0x38)) &&
       (*(longlong *)
         (*(longlong *)(*(longlong *)(lVar1 + 0x10) + 0x30) + (longlong)*(int *)(lVar2 + 0x38) * 8)
        == lVar2 + 0x30)) {
      U_142407c60(lVar1,param_2);
    }
  }
  return;
}

// AFortPlayerControllerAthena_HighlightNumberReturned exec 0x142a58da0
// Out-of-line dependency 0x1419dc720; parameter ABI remains to be checked.
void U_1419dc720(longlong param_1,undefined4 param_2)

{
  if (4 < DAT_1481348e0) {
    FMsg_Logf_Internal((byte *)0x0,0,&DAT_1481348e4,5,
                       (ushort *)L"%d highlights are currently in group:%s");
  }
  U_140f2c470(param_1 + 0x1718,param_2);
  *(undefined4 *)(param_1 + 0x2c90) = param_2;
  return;
}

// AFortPlayerControllerAthena_HandlePlayersLeftChanged exec 0x142a58d20
// Out-of-line dependency 0x1419db530; parameter ABI remains to be checked.
void U_1419db530(undefined8 param_1)

{
  thunk_FUN_142810956(param_1,0,1);
  return;
}

// AFortPlayerControllerAthena_BroadcastDeviceButtonComponentClickedDelegate exec 0x142a562b0
// Out-of-line dependency 0x1419c2480; parameter ABI remains to be checked.
void U_1419c2480(longlong *param_1,longlong param_2)

{
  char cVar1;
  longlong lVar2;
  undefined *puVar3;
  longlong lVar4;
  
  lVar2 = (**(code **)(*param_1 + 0x150))();
  if ((lVar2 != 0) && (lVar2 = *(longlong *)(lVar2 + 0x130), lVar2 != 0)) {
    puVar3 = AFortGameStateAthena_StaticClass();
    if ((*(int *)(puVar3 + 0x38) <= *(int *)(*(longlong *)(lVar2 + 0x10) + 0x38)) &&
       (*(undefined **)
         (*(longlong *)(*(longlong *)(lVar2 + 0x10) + 0x30) + (longlong)*(int *)(puVar3 + 0x38) * 8)
        == puVar3 + 0x30)) goto LAB_1419c24d4;
  }
  lVar2 = 0;
LAB_1419c24d4:
  lVar4 = U_141ed2350();
  if ((*(char *)(lVar4 + 0xc9) != '\0') && (lVar2 != 0)) {
    cVar1 = U_14199e430(lVar2);
    if ((cVar1 != '\0') && (param_2 != 0)) {
      if ((char)param_1[0x1e] != '\x03') {
        U_142a53e70(param_1,param_2);
      }
      (**(code **)(*param_1 + 0x2800))(param_1,param_2);
    }
  }
  return;
}

// AFortPlayerControllerAthena_ApplyOverrideWrapToVehicle exec 0x142a561f0
// Out-of-line dependency 0x1419bfa60; parameter ABI remains to be checked.
void U_1419bfa60(longlong param_1,undefined8 param_2)

{
  longlong lVar1;
  longlong *plVar2;
  
  plVar2 = *(longlong **)(param_1 + 0x248);
  if (plVar2 != (longlong *)0x0) {
    lVar1 = U_142a75f10();
    if ((*(int *)(lVar1 + 0x38) <= *(int *)(plVar2[2] + 0x38)) &&
       (*(longlong *)(*(longlong *)(plVar2[2] + 0x30) + (longlong)*(int *)(lVar1 + 0x38) * 8) ==
        lVar1 + 0x30)) {
      plVar2 = (longlong *)(**(code **)(*plVar2 + 0x1088))(plVar2);
      if (plVar2 != (longlong *)0x0) {
        (**(code **)(*plVar2 + 0x5d0))(plVar2,param_2);
        U_141f8b8e0(plVar2);
      }
    }
  }
  U_142a53d90(param_1,param_2);
  return;
}

// AFortPlayerControllerAthena_OnGameServerNodeTypeChanged exec 0x142a59600
// Out-of-line dependency 0x1419e1810; parameter ABI remains to be checked.
void U_1419e1810(longlong *param_1,char param_2)

{
  longlong *plVar1;
  int *piVar2;
  int iVar3;
  char cVar4;
  longlong lVar5;
  longlong lStack_18;
  longlong *plStack_10;
  
  cVar4 = (**(code **)(*param_1 + 0x6a8))();
  if (cVar4 != '\0') {
    cVar4 = U_1410564b0();
    if ((cVar4 != '\0') && (param_2 == '\0')) {
      lVar5 = (**(code **)(*param_1 + 0x150))(param_1);
      if ((lVar5 != 0) && (*(longlong *)(lVar5 + 0x188) != 0)) {
        lVar5 = U_141053d30();
        if (lVar5 != 0) {
          U_141055600(lVar5,&lStack_18);
          if (lStack_18 != 0) {
            lVar5 = U_14447ee40(param_1);
            if (lVar5 != 0) {
              U_1410582b0(lStack_18,*(undefined4 *)(lVar5 + 0xb8));
            }
          }
          if (plStack_10 != (longlong *)0x0) {
            LOCK();
            plVar1 = plStack_10 + 1;
            lVar5 = *plVar1;
            *(int *)plVar1 = (int)*plVar1 + -1;
            UNLOCK();
            if ((int)lVar5 == 1) {
              (**(code **)*plStack_10)(plStack_10);
              LOCK();
              piVar2 = (int *)((longlong)plStack_10 + 0xc);
              iVar3 = *piVar2;
              *piVar2 = *piVar2 + -1;
              UNLOCK();
              if (iVar3 == 1) {
                (**(code **)(*plStack_10 + 8))(plStack_10,1);
              }
            }
          }
        }
      }
    }
  }
  return;
}

// AFortPlayerControllerAthena_HandleGamePhaseStepChanged exec 0x142a58c40
// Out-of-line dependency 0x1419da970; parameter ABI remains to be checked.
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void U_1419da970(UObject *param_1,undefined8 param_2,char param_3)

{
  int *piVar1;
  longlong lVar2;
  bool bVar3;
  undefined4 uVar4;
  char cVar5;
  int iVar6;
  UObject *pUVar7;
  longlong *plVar8;
  undefined *puVar9;
  int iVar10;
  longlong *plVar11;
  longlong lVar12;
  undefined8 uVar13;
  longlong lVar14;
  undefined1 auStack_d8 [32];
  undefined1 uStack_b8;
  undefined4 uStack_b0;
  undefined8 uStack_a8;
  undefined4 uStack_a0;
  undefined4 uStack_9c;
  longlong *plStack_98;
  int iStack_90;
  undefined1 auStack_88 [8];
  undefined8 uStack_80;
  longlong lStack_78;
  longlong *plStack_68;
  longlong alStack_58 [4];
  ulonglong uStack_38;
  
  if (param_3 != '\x05') {
    return;
  }
  uStack_38 = _DAT_1480c4288 ^ (ulonglong)auStack_d8;
  pUVar7 = GetFortGameStateAthena(param_1);
  if (pUVar7 == (UObject *)0x0) {
    cVar5 = '\0';
  }
  else {
    cVar5 = U_141990c10(pUVar7);
  }
  if (*(char *)((longlong)&param_1[0x143].NamePrivate.Number + 1) == cVar5) goto LAB_1419ff57e;
  *(char *)((longlong)&param_1[0x143].NamePrivate.Number + 1) = cVar5;
  piVar1 = &param_1[0x142].NamePrivate.Number;
  *piVar1 = *piVar1 + 1;
  iVar10 = param_1[0x142].NamePrivate.Number;
  bVar3 = false;
  iVar6 = *(int *)param_1[0x142].ClassPrivate + -1;
  lVar14 = (longlong)iVar6;
  if (-1 < iVar6) {
    lVar12 = lVar14 << 4;
    bVar3 = false;
    do {
      lVar2 = *(longlong *)&param_1[0x142].ObjectFlags;
      if (((*(int *)(lVar12 + 8 + lVar2) == 0) ||
          (plVar8 = *(longlong **)(lVar12 + lVar2), plVar8 == (longlong *)0x0)) ||
         (cVar5 = (**(code **)(*plVar8 + 0x50))(), cVar5 == '\0')) {
        bVar3 = true;
      }
      lVar12 = lVar12 + -0x10;
      lVar14 = lVar14 + -1;
    } while (-1 < lVar14);
    iVar10 = param_1[0x142].NamePrivate.Number;
  }
  param_1[0x142].NamePrivate.Number = iVar10 + -1;
  if (bVar3) {
    FMulticastDelegateBase_CompactInvocationList((undefined *)&param_1[0x142].ObjectFlags,false);
  }
  if (*(char *)((longlong)&param_1[0x143].NamePrivate.Number + 1) != '\x06') {
    if ((*(longlong *)param_1[0x143].OuterPrivate != 0) &&
       (lVar14 = (**(code **)(*(longlong *)param_1->vftable + 0x150))(param_1), lVar14 != 0)) {
      if (*(longlong *)(lVar14 + 0x188) == 0) {
        puVar9 = *(undefined **)(lVar14 + 800);
      }
      else {
        puVar9 = *(undefined **)(*(longlong *)(lVar14 + 0x188) + 0xc0);
      }
      lVar14 = U_144621fb0(puVar9,param_1[0x143].OuterPrivate);
      if (lVar14 != 0) {
        FTimerManager_InternalClearTimer(puVar9,*(ulonglong *)param_1[0x143].OuterPrivate);
      }
      *(undefined8 *)param_1[0x143].OuterPrivate = 0;
    }
    goto LAB_1419ff57e;
  }
  lVar14 = (**(code **)(*(longlong *)param_1->vftable + 0x150))();
  if (lVar14 == 0) goto LAB_1419ff57e;
  uStack_a0 = 0;
  uStack_a8 = U_1419ff260;
  plVar8 = (longlong *)FMemory_Realloc((undefined *)0x0,0x30,0);
  if (plVar8 != (longlong *)0x0) {
    *plVar8 = (longlong)&DAT_1459a9e80;
    U_142ee3b30(plVar8 + 1,param_1);
    *(undefined4 *)(plVar8 + 2) = (undefined4)uStack_a8;
    *(undefined4 *)((longlong)plVar8 + 0x14) = uStack_a8._4_4_;
    *(undefined4 *)(plVar8 + 3) = uStack_a0;
    *(undefined4 *)((longlong)plVar8 + 0x1c) = uStack_9c;
    lVar12 = U_142bad740();
    plVar8[5] = lVar12;
    *plVar8 = (longlong)&DAT_1459a9ed8;
  }
  uVar4 = _DAT_1478c2838;
  if (*(longlong *)(lVar14 + 0x188) == 0) {
    uVar13 = *(undefined8 *)(lVar14 + 800);
  }
  else {
    uVar13 = *(undefined8 *)(*(longlong *)(lVar14 + 0x188) + 0xc0);
  }
  plStack_98 = (longlong *)0x0;
  iStack_90 = 0;
  if (plVar8 != (longlong *)0x0) {
    (**(code **)(*plVar8 + 0x40))(plVar8,&plStack_98);
  }
  U_142ee3b30(auStack_88,0);
  uStack_b0 = 0xbf800000;
  uStack_b8 = 1;
  uStack_80 = 0;
  lStack_78 = 0;
  plStack_68 = (longlong *)0x0;
  U_1446278d0(uVar13,param_1[0x143].OuterPrivate,&plStack_98,uVar4);
  if (lStack_78 != 0) {
    plVar11 = alStack_58;
    if (plStack_68 != (longlong *)0x0) {
      plVar11 = plStack_68;
    }
    (**(code **)(*plVar11 + 0x10))();
  }
  plVar11 = (longlong *)0x0;
  if (iStack_90 != 0) {
    plVar11 = plStack_98;
  }
  if (plVar11 == (longlong *)0x0) {
    if (iStack_90 == 0) goto LAB_1419ff4d1;
    if (plStack_98 != (longlong *)0x0) {
      lVar14 = *plStack_98;
      plVar11 = plStack_98;
      goto LAB_1419ff4ac;
    }
  }
  else {
    lVar14 = *plVar11;
LAB_1419ff4ac:
    (**(code **)(lVar14 + 0x38))(plVar11);
    if (plStack_98 != (longlong *)0x0) {
      plStack_98 = (longlong *)FMemory_Realloc((undefined *)plStack_98,0,0);
    }
    iStack_90 = 0;
LAB_1419ff4d1:
    if (plStack_98 != (longlong *)0x0) {
      FMemory_Free((undefined *)plStack_98);
    }
  }
  if (plVar8 != (longlong *)0x0) {
    (**(code **)(*plVar8 + 0x38))(plVar8);
    puVar9 = FMemory_Realloc((undefined *)plVar8,0,0);
    if (puVar9 != (undefined *)0x0) {
      FMemory_Free(puVar9);
    }
  }
LAB_1419ff57e:
  U_145803c70(uStack_38 ^ (ulonglong)auStack_d8);
  return;
}

// AFortGameStateAthena_IsStormCN exec 0x142920160
// Out-of-line dependency 0x14199e4e0; parameter ABI remains to be checked.
undefined8 U_14199e4e0(longlong param_1)

{
  int *piVar1;
  int iVar2;
  char cVar3;
  bool bVar4;
  longlong lVar5;
  longlong lVar6;
  int *piVar7;
  
  lVar5 = U_141ed2350();
  if (*(char *)(lVar5 + 0x2c8) != '\0') {
    lVar5 = *(longlong *)(param_1 + 0x1df8);
    if (lVar5 == 0) {
      lVar5 = *(longlong *)(param_1 + 0x1df0);
    }
    cVar3 = U_141a49590(lVar5);
    if (cVar3 == '\0') {
      lVar5 = *(longlong *)(param_1 + 0x1df8);
      if (lVar5 == 0) {
        lVar5 = *(longlong *)(param_1 + 0x1df0);
      }
      cVar3 = U_141a4b0d0(lVar5);
      if (cVar3 == '\0') {
        lVar5 = *(longlong *)(param_1 + 0x1df8);
        if ((lVar5 != 0) || (lVar5 = *(longlong *)(param_1 + 0x1df0), lVar5 != 0)) {
          lVar6 = U_141ed1800();
          bVar4 = FName_EqualsEName(*(int *)(lVar6 + 0x1780),0);
          iVar2 = *(int *)(lVar6 + 0x1784);
          if (iVar2 != 0 || !bVar4) {
            piVar7 = *(int **)(lVar5 + 0xd0);
            piVar1 = piVar7 + (longlong)*(int *)(lVar5 + 0xd8) * 2;
            if (piVar7 != piVar1) {
              do {
                if (*piVar7 == *(int *)(lVar6 + 0x1780) && piVar7[1] == iVar2) {
                  return 0;
                }
                piVar7 = piVar7 + 2;
              } while (piVar7 != piVar1);
            }
            piVar7 = *(int **)(lVar5 + 0xe0);
            piVar1 = piVar7 + (longlong)*(int *)(lVar5 + 0xe8) * 2;
            if (piVar7 != piVar1) {
              do {
                if (*piVar7 == *(int *)(lVar6 + 0x1780) && piVar7[1] == iVar2) {
                  return 0;
                }
                piVar7 = piVar7 + 2;
              } while (piVar7 != piVar1);
            }
          }
          if ((*(char *)(lVar5 + 0x467) != '\0') && (*(char *)(lVar5 + 0x467) != '\a')) {
            return 0;
          }
        }
        return 1;
      }
    }
  }
  return 0;
}

// AFortGameStateAthena_IsLastManStandingCN exec 0x14291ffe0
// Out-of-line dependency 0x14199e4e0; parameter ABI remains to be checked.
undefined8 U_14199e4e0(longlong param_1)

{
  int *piVar1;
  int iVar2;
  char cVar3;
  bool bVar4;
  longlong lVar5;
  longlong lVar6;
  int *piVar7;
  
  lVar5 = U_141ed2350();
  if (*(char *)(lVar5 + 0x2c8) != '\0') {
    lVar5 = *(longlong *)(param_1 + 0x1df8);
    if (lVar5 == 0) {
      lVar5 = *(longlong *)(param_1 + 0x1df0);
    }
    cVar3 = U_141a49590(lVar5);
    if (cVar3 == '\0') {
      lVar5 = *(longlong *)(param_1 + 0x1df8);
      if (lVar5 == 0) {
        lVar5 = *(longlong *)(param_1 + 0x1df0);
      }
      cVar3 = U_141a4b0d0(lVar5);
      if (cVar3 == '\0') {
        lVar5 = *(longlong *)(param_1 + 0x1df8);
        if ((lVar5 != 0) || (lVar5 = *(longlong *)(param_1 + 0x1df0), lVar5 != 0)) {
          lVar6 = U_141ed1800();
          bVar4 = FName_EqualsEName(*(int *)(lVar6 + 0x1780),0);
          iVar2 = *(int *)(lVar6 + 0x1784);
          if (iVar2 != 0 || !bVar4) {
            piVar7 = *(int **)(lVar5 + 0xd0);
            piVar1 = piVar7 + (longlong)*(int *)(lVar5 + 0xd8) * 2;
            if (piVar7 != piVar1) {
              do {
                if (*piVar7 == *(int *)(lVar6 + 0x1780) && piVar7[1] == iVar2) {
                  return 0;
                }
                piVar7 = piVar7 + 2;
              } while (piVar7 != piVar1);
            }
            piVar7 = *(int **)(lVar5 + 0xe0);
            piVar1 = piVar7 + (longlong)*(int *)(lVar5 + 0xe8) * 2;
            if (piVar7 != piVar1) {
              do {
                if (*piVar7 == *(int *)(lVar6 + 0x1780) && piVar7[1] == iVar2) {
                  return 0;
                }
                piVar7 = piVar7 + 2;
              } while (piVar7 != piVar1);
            }
          }
          if ((*(char *)(lVar5 + 0x467) != '\0') && (*(char *)(lVar5 + 0x467) != '\a')) {
            return 0;
          }
        }
        return 1;
      }
    }
  }
  return 0;
}

// AFortGameStateAthena_GetTimedWinConditionTime exec 0x14291f650
// Out-of-line dependency 0x1419996f0; parameter ABI remains to be checked.
undefined8 U_1419996f0(longlong param_1)

{
  longlong lVar1;
  undefined8 uVar2;
  
  lVar1 = *(longlong *)(param_1 + 0x1df8);
  if ((lVar1 != 0) || (lVar1 = *(longlong *)(param_1 + 0x1df0), lVar1 != 0)) {
    uVar2 = U_14070fdf0(lVar1 + 0x468,0);
    return uVar2;
  }
  if (2 < DAT_1481348e0) {
    FMsg_Logf_Internal((byte *)0x0,0,&DAT_1481348e4,3,
                       (ushort *)L"PLAYLIST: GetTimedWinConditionTime called with no playlist data")
    ;
  }
  return 0xbf800000;
}

// AFortGameStateAthena_GetShouldPlayRespawnTimerSFX exec 0x14291f420
// Out-of-line dependency 0x141997fe0; parameter ABI remains to be checked.
undefined1 U_141997fe0(undefined8 param_1)

{
  char cVar1;
  undefined1 auStackX_10 [24];
  undefined1 auStack_48 [28];
  int iStack_2c;
  longlong *plStack_18;
  
  auStackX_10[0] = 1;
  U_1419b90a0(auStack_48,0,param_1,1);
  while ((plStack_18 != (longlong *)0x0 &&
         (cVar1 = (**(code **)(*plStack_18 + 0xca0))(plStack_18,auStackX_10), cVar1 != '\x02'))) {
    U_1419f3d90(auStack_48,iStack_2c + 1);
  }
  U_1419bb7e0(auStack_48);
  return auStackX_10[0];
}

// AFortGameStateAthena_GetShouldDisplayCountdownTimerText exec 0x14291f3f0
// Out-of-line dependency 0x141997f70; parameter ABI remains to be checked.
undefined1 U_141997f70(undefined8 param_1)

{
  char cVar1;
  undefined1 auStackX_10 [24];
  undefined1 auStack_48 [28];
  int iStack_2c;
  longlong *plStack_18;
  
  auStackX_10[0] = 1;
  U_1419b90a0(auStack_48,0,param_1,1);
  while ((plStack_18 != (longlong *)0x0 &&
         (cVar1 = (**(code **)(*plStack_18 + 0xc98))(plStack_18,auStackX_10), cVar1 != '\x02'))) {
    U_1419f3d90(auStack_48,iStack_2c + 1);
  }
  U_1419bb7e0(auStack_48);
  return auStackX_10[0];
}

// AFortGameStateAthena_IsTeleportToCreativeHubAllowed exec 0x142920220
// Out-of-line dependency 0x14199e430; parameter ABI remains to be checked.
undefined1 U_14199e430(longlong param_1)

{
  char cVar1;
  longlong lVar2;
  
  lVar2 = *(longlong *)(param_1 + 0x1df8);
  if (lVar2 == 0) {
    lVar2 = *(longlong *)(param_1 + 0x1df0);
  }
  cVar1 = U_141a49590(lVar2);
  if (cVar1 == '\0') {
    lVar2 = *(longlong *)(param_1 + 0x1df8);
    if (lVar2 == 0) {
      lVar2 = *(longlong *)(param_1 + 0x1df0);
    }
    cVar1 = U_141a4b0d0(lVar2);
    if (cVar1 == '\0') {
      return 0;
    }
  }
  return 1;
}

// AFortGameStateAthena_IsTeamSwitchAllowed exec 0x142920190
// Out-of-line dependency 0x14199ee50; parameter ABI remains to be checked.
bool U_14199ee50(longlong param_1,longlong param_2)

{
  bool bVar1;
  char cVar2;
  char cVar3;
  char cVar4;
  longlong lVar5;
  longlong lVar6;
  
  lVar5 = *(longlong *)(param_1 + 0x1df8);
  lVar6 = lVar5;
  if (lVar5 == 0) {
    lVar6 = *(longlong *)(param_1 + 0x1df0);
  }
  if (lVar6 != 0) {
    if (lVar5 == 0) {
      lVar5 = *(longlong *)(param_1 + 0x1df0);
    }
    cVar2 = U_141a4a6f0(lVar5);
    lVar5 = *(longlong *)(param_1 + 0x1df8);
    if (lVar5 == 0) {
      lVar5 = *(longlong *)(param_1 + 0x1df0);
    }
    cVar3 = U_141a4b0d0(lVar5);
    lVar5 = *(longlong *)(param_1 + 0x1df8);
    if (lVar5 == 0) {
      lVar5 = *(longlong *)(param_1 + 0x1df0);
    }
    cVar4 = U_141a4a530(lVar5);
    if (cVar4 == '\0') {
      if (cVar2 == '\0') {
        if (cVar3 == '\0') {
          if ((((*(byte *)(lVar6 + 0x9fc) & 4) != 0) &&
              (*(char *)(lVar6 + 200) != *(char *)(lVar6 + 0xc9))) &&
             ((1 < *(int *)(lVar6 + 0x58) && (1 < *(int *)(param_1 + 0x12f0))))) {
            return true;
          }
        }
        else {
          bVar1 = false;
          if ((param_2 != 0) && (*(longlong *)(param_2 + 0x238) != 0)) {
            lVar5 = U_141c794e0();
            if (lVar5 != 0) {
              cVar2 = U_142099810(lVar5);
              bVar1 = false;
              if (cVar2 != '\0') {
                bVar1 = true;
              }
            }
          }
          if (((((*(byte *)(lVar6 + 0x9fc) & 4) != 0) &&
               (*(char *)(lVar6 + 200) != *(char *)(lVar6 + 0xc9))) && (1 < *(int *)(lVar6 + 0x58)))
             && (!bVar1)) {
            return true;
          }
        }
      }
      else if (((((*(byte *)(lVar6 + 0x9fc) & 4) != 0) &&
                (*(char *)(lVar6 + 200) != *(char *)(lVar6 + 0xc9))) && (1 < *(int *)(lVar6 + 0x58))
               ) && (param_2 != 0)) {
        lVar5 = U_142a82010();
        if ((*(int *)(lVar5 + 0x38) <= *(int *)(*(longlong *)(param_2 + 0x10) + 0x38)) &&
           (*(longlong *)
             (*(longlong *)(*(longlong *)(param_2 + 0x10) + 0x30) +
             (longlong)*(int *)(lVar5 + 0x38) * 8) == lVar5 + 0x30)) {
          cVar2 = U_141a18ce0(param_2);
          return cVar2 == '\0';
        }
      }
    }
  }
  return false;
}

// AFortGameStateAthena_IsResurrectionEnabled exec 0x1429200d0
// Out-of-line dependency 0x14199ea80; parameter ABI remains to be checked.
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

bool U_14199ea80(longlong param_1,longlong param_2)

{
  longlong lVar1;
  undefined8 uVar2;
  undefined1 uVar3;
  char cVar4;
  bool bVar5;
  undefined *puVar6;
  longlong lVar7;
  longlong *plVar8;
  
  if (_DAT_1478c26a0 == 1) {
    if (param_2 != 0) {
      lVar1 = *(longlong *)(param_2 + 0x250);
      if (lVar1 != 0) {
        puVar6 = AFortPlayerControllerAthena_StaticClass();
        if ((*(int *)(puVar6 + 0x38) <= *(int *)(*(longlong *)(lVar1 + 0x10) + 0x38)) &&
           (*(undefined **)
             (*(longlong *)(*(longlong *)(lVar1 + 0x10) + 0x30) +
             (longlong)*(int *)(puVar6 + 0x38) * 8) == puVar6 + 0x30)) {
          lVar7 = U_141680740(lVar1);
          if ((lVar7 == 0) || (*(char *)(lVar7 + 5000) == '\0')) {
            uVar3 = U_1419d8d40(lVar1);
            plVar8 = (longlong *)U_1419982c0(param_1,uVar3);
            if (1 < (int)plVar8[1]) {
              lVar7 = *plVar8;
              uVar2 = *(undefined8 *)(param_2 + 0x238);
              lVar1 = lVar7 + (longlong)(int)plVar8[1] * 8;
              for (; lVar7 != lVar1; lVar7 = lVar7 + 8) {
                cVar4 = U_1406a7e80(lVar7,uVar2);
                if (cVar4 != '\0') goto LAB_14199eb68;
              }
            }
          }
          goto LAB_14199eb4b;
        }
      }
LAB_14199eb68:
      if ((*(byte *)(param_2 + 0x53a) & 2) != 0) goto LAB_14199eb4b;
    }
    bVar5 = ABS(*(float *)(param_1 + 0x20bc) - 1.0) <= 1e-08;
  }
  else {
LAB_14199eb4b:
    bVar5 = false;
  }
  return bVar5;
}

// AFortGameStateAthena_IsPlayerTriggeredRespawnAllowed exec 0x142920010
// Out-of-line dependency 0x14199e700; parameter ABI remains to be checked.
ulonglong U_14199e700(longlong param_1,longlong param_2)

{
  char cVar1;
  ulonglong uVar2;
  longlong lVar3;
  undefined8 uVar4;
  char acStackX_18 [16];
  undefined1 auStack_48 [28];
  int iStack_2c;
  longlong *plStack_18;
  
  uVar2 = U_141ed2350();
  if (*(char *)(uVar2 + 0x78) != '\0') {
    lVar3 = *(longlong *)(param_1 + 0x1df8);
    if (lVar3 == 0) {
      lVar3 = *(longlong *)(param_1 + 0x1df0);
    }
    cVar1 = U_141a49590(lVar3);
    if (cVar1 == '\0') {
      lVar3 = *(longlong *)(param_1 + 0x1df8);
      if (lVar3 == 0) {
        lVar3 = *(longlong *)(param_1 + 0x1df0);
      }
      uVar2 = U_141a4b0d0(lVar3);
      if ((char)uVar2 == '\0') goto LAB_14199e81f;
    }
    lVar3 = U_141c794e0(param_2);
    if ((lVar3 == 0) || (uVar2 = U_142099810(lVar3), (char)uVar2 == '\0')) {
      uVar2 = 1;
      acStackX_18[0] = '\x01';
      uVar4 = 0;
      if (param_2 != 0) {
        uVar4 = U_1419b6d20(param_2);
      }
      U_1419b90a0(auStack_48,uVar4,param_1,1);
      do {
        if (plStack_18 == (longlong *)0x0) {
LAB_14199e801:
          U_1419bb7e0(auStack_48);
          return uVar2;
        }
        cVar1 = (**(code **)(*plStack_18 + 0x748))(plStack_18,uVar4,acStackX_18);
        if ((cVar1 == '\x02') && (acStackX_18[0] == '\0')) {
          uVar2 = 0;
          goto LAB_14199e801;
        }
        U_1419f3d90(auStack_48,iStack_2c + 1);
      } while( true );
    }
  }
LAB_14199e81f:
  return uVar2 & 0xffffffffffffff00;
}

// AFortGameStateAthena_GetTeamPOIName exec 0x14291f560
// Out-of-line dependency 0x1419984d0; parameter ABI remains to be checked.
undefined8 U_1419984d0(undefined8 param_1,undefined8 param_2,undefined1 param_3)

{
  char cVar1;
  undefined1 auStack_48 [28];
  int iStack_2c;
  longlong *plStack_18;
  
  U_142c0c240(param_2);
  U_1419b90a0(auStack_48,0,param_1,1);
  while ((plStack_18 != (longlong *)0x0 &&
         (cVar1 = (**(code **)(*plStack_18 + 0xc90))(plStack_18,param_3,param_2), cVar1 != '\x02')))
  {
    U_1419f3d90(auStack_48,iStack_2c + 1);
  }
  U_1419bb7e0(auStack_48);
  return param_2;
}

// AFortGameStateAthena_GetPoiGridTagsForLocation exec 0x14291f300
// Out-of-line dependency 0x141996820; parameter ABI remains to be checked.
undefined8 U_141996820(longlong *param_1,undefined8 param_2)

{
  (**(code **)(*param_1 + 0x8f8))();
  return param_2;
}
// Out-of-line dependency 0x142c64fe0; parameter ABI remains to be checked.
undefined8 * U_142c64fe0(undefined8 *param_1,undefined8 *param_2)

{
  undefined8 *puVar1;
  undefined8 *puVar2;
  
  if (param_1 != param_2) {
    if ((undefined *)*param_1 != (undefined *)0x0) {
      FMemory_Free((undefined *)*param_1);
    }
    *param_1 = *param_2;
    *param_2 = 0;
    *(undefined4 *)(param_1 + 1) = *(undefined4 *)(param_2 + 1);
    *(undefined4 *)((longlong)param_1 + 0xc) = *(undefined4 *)((longlong)param_2 + 0xc);
    param_2[1] = 0;
  }
  puVar2 = param_2 + 2;
  puVar1 = param_1 + 2;
  if (puVar1 != puVar2) {
    if ((undefined *)*puVar1 != (undefined *)0x0) {
      FMemory_Free((undefined *)*puVar1);
    }
    *puVar1 = *puVar2;
    *puVar2 = 0;
    *(undefined4 *)(param_1 + 3) = *(undefined4 *)(param_2 + 3);
    *(undefined4 *)((longlong)param_1 + 0x1c) = *(undefined4 *)((longlong)param_2 + 0x1c);
    param_2[3] = 0;
  }
  return param_1;
}

// AFortGameStateAthena_GetOverridenSiphonWoodValue exec 0x14291f130
// Out-of-line dependency 0x141995d20; parameter ABI remains to be checked.
undefined4 U_141995d20(undefined8 param_1,undefined8 param_2)

{
  char cVar1;
  undefined4 auStackX_18 [4];
  undefined1 auStack_48 [28];
  int iStack_2c;
  longlong *plStack_18;
  
  auStackX_18[0] = 0;
  U_1419b90a0(auStack_48,param_2,param_1,1);
  while ((plStack_18 != (longlong *)0x0 &&
         (cVar1 = (**(code **)(*plStack_18 + 0xaf8))(plStack_18,auStackX_18), cVar1 == '\0'))) {
    U_1419f3d90(auStack_48,iStack_2c + 1);
  }
  U_1419bb7e0(auStack_48);
  return auStackX_18[0];
}

// AFortGameStateAthena_GetOverridenSiphonStoneValue exec 0x14291f090
// Out-of-line dependency 0x141995ca0; parameter ABI remains to be checked.
undefined4 U_141995ca0(undefined8 param_1,undefined8 param_2)

{
  char cVar1;
  undefined4 auStackX_18 [4];
  undefined1 auStack_48 [28];
  int iStack_2c;
  longlong *plStack_18;
  
  auStackX_18[0] = 0;
  U_1419b90a0(auStack_48,param_2,param_1,1);
  while ((plStack_18 != (longlong *)0x0 &&
         (cVar1 = (**(code **)(*plStack_18 + 0xb00))(plStack_18,auStackX_18), cVar1 == '\0'))) {
    U_1419f3d90(auStack_48,iStack_2c + 1);
  }
  U_1419bb7e0(auStack_48);
  return auStackX_18[0];
}

// AFortGameStateAthena_GetOverridenSiphonMetalValue exec 0x14291eff0
// Out-of-line dependency 0x141995c20; parameter ABI remains to be checked.
undefined4 U_141995c20(undefined8 param_1,undefined8 param_2)

{
  char cVar1;
  undefined4 auStackX_18 [4];
  undefined1 auStack_48 [28];
  int iStack_2c;
  longlong *plStack_18;
  
  auStackX_18[0] = 0;
  U_1419b90a0(auStack_48,param_2,param_1,1);
  while ((plStack_18 != (longlong *)0x0 &&
         (cVar1 = (**(code **)(*plStack_18 + 0xb08))(plStack_18,auStackX_18), cVar1 == '\0'))) {
    U_1419f3d90(auStack_48,iStack_2c + 1);
  }
  U_1419bb7e0(auStack_48);
  return auStackX_18[0];
}

// AFortGameStateAthena_GetOverridenDamageSelfWeaponQuery exec 0x14291edd0
// Out-of-line dependency 0x141995a90; parameter ABI remains to be checked.
undefined8 U_141995a90(undefined8 param_1,undefined8 param_2,undefined8 param_3)

{
  char cVar1;
  undefined1 auStack_48 [28];
  int iStack_2c;
  longlong *plStack_18;
  
  U_143e89f60(param_2,&DAT_148323460);
  U_1419b90a0(auStack_48,param_3,param_1,1);
  while ((plStack_18 != (longlong *)0x0 &&
         (cVar1 = (**(code **)(*plStack_18 + 0xb30))(plStack_18,param_2), cVar1 == '\0'))) {
    U_1419f3d90(auStack_48,iStack_2c + 1);
  }
  U_1419bb7e0(auStack_48);
  return param_2;
}
// Out-of-line dependency 0x143e8a780; parameter ABI remains to be checked.
undefined4 * U_143e8a780(undefined4 *param_1,undefined4 *param_2)

{
  undefined8 *puVar1;
  undefined8 *puVar2;
  
  puVar1 = (undefined8 *)(param_2 + 2);
  puVar2 = (undefined8 *)(param_1 + 2);
  *param_1 = *param_2;
  if (puVar2 != puVar1) {
    if ((undefined *)*puVar2 != (undefined *)0x0) {
      FMemory_Free((undefined *)*puVar2);
    }
    *puVar2 = *puVar1;
    *puVar1 = 0;
    param_1[4] = param_2[4];
    param_1[5] = param_2[5];
    *(undefined8 *)(param_2 + 4) = 0;
  }
  puVar1 = (undefined8 *)(param_2 + 6);
  puVar2 = (undefined8 *)(param_1 + 6);
  if (puVar2 != puVar1) {
    if ((undefined *)*puVar2 != (undefined *)0x0) {
      FMemory_Free((undefined *)*puVar2);
    }
    *puVar2 = *puVar1;
    *puVar1 = 0;
    param_1[8] = param_2[8];
    param_1[9] = param_2[9];
    *(undefined8 *)(param_2 + 8) = 0;
  }
  puVar1 = (undefined8 *)(param_2 + 10);
  puVar2 = (undefined8 *)(param_1 + 10);
  if (puVar2 != puVar1) {
    if ((undefined *)*puVar2 != (undefined *)0x0) {
      FMemory_Free((undefined *)*puVar2);
    }
    *puVar2 = *puVar1;
    *puVar1 = 0;
    param_1[0xc] = param_2[0xc];
    param_1[0xd] = param_2[0xd];
    *(undefined8 *)(param_2 + 0xc) = 0;
  }
  puVar1 = (undefined8 *)(param_2 + 0xe);
  puVar2 = (undefined8 *)(param_1 + 0xe);
  if (puVar2 != puVar1) {
    if ((undefined *)*puVar2 != (undefined *)0x0) {
      FMemory_Free((undefined *)*puVar2);
    }
    *puVar2 = *puVar1;
    *puVar1 = 0;
    param_1[0x10] = param_2[0x10];
    param_1[0x11] = param_2[0x11];
    *(undefined8 *)(param_2 + 0x10) = 0;
  }
  return param_1;
}

// AFortGameStateAthena_GetOverridenDamageSelfTargetQuery exec 0x14291ecf0
// Out-of-line dependency 0x141995a00; parameter ABI remains to be checked.
undefined8 U_141995a00(undefined8 param_1,undefined8 param_2,undefined8 param_3)

{
  char cVar1;
  undefined1 auStack_48 [28];
  int iStack_2c;
  longlong *plStack_18;
  
  U_143e89f60(param_2,&DAT_148323460);
  U_1419b90a0(auStack_48,param_3,param_1,1);
  while ((plStack_18 != (longlong *)0x0 &&
         (cVar1 = (**(code **)(*plStack_18 + 0xb28))(plStack_18,param_2), cVar1 == '\0'))) {
    U_1419f3d90(auStack_48,iStack_2c + 1);
  }
  U_1419bb7e0(auStack_48);
  return param_2;
}
// Out-of-line dependency 0x143e8a780; parameter ABI remains to be checked.
undefined4 * U_143e8a780(undefined4 *param_1,undefined4 *param_2)

{
  undefined8 *puVar1;
  undefined8 *puVar2;
  
  puVar1 = (undefined8 *)(param_2 + 2);
  puVar2 = (undefined8 *)(param_1 + 2);
  *param_1 = *param_2;
  if (puVar2 != puVar1) {
    if ((undefined *)*puVar2 != (undefined *)0x0) {
      FMemory_Free((undefined *)*puVar2);
    }
    *puVar2 = *puVar1;
    *puVar1 = 0;
    param_1[4] = param_2[4];
    param_1[5] = param_2[5];
    *(undefined8 *)(param_2 + 4) = 0;
  }
  puVar1 = (undefined8 *)(param_2 + 6);
  puVar2 = (undefined8 *)(param_1 + 6);
  if (puVar2 != puVar1) {
    if ((undefined *)*puVar2 != (undefined *)0x0) {
      FMemory_Free((undefined *)*puVar2);
    }
    *puVar2 = *puVar1;
    *puVar1 = 0;
    param_1[8] = param_2[8];
    param_1[9] = param_2[9];
    *(undefined8 *)(param_2 + 8) = 0;
  }
  puVar1 = (undefined8 *)(param_2 + 10);
  puVar2 = (undefined8 *)(param_1 + 10);
  if (puVar2 != puVar1) {
    if ((undefined *)*puVar2 != (undefined *)0x0) {
      FMemory_Free((undefined *)*puVar2);
    }
    *puVar2 = *puVar1;
    *puVar1 = 0;
    param_1[0xc] = param_2[0xc];
    param_1[0xd] = param_2[0xd];
    *(undefined8 *)(param_2 + 0xc) = 0;
  }
  puVar1 = (undefined8 *)(param_2 + 0xe);
  puVar2 = (undefined8 *)(param_1 + 0xe);
  if (puVar2 != puVar1) {
    if ((undefined *)*puVar2 != (undefined *)0x0) {
      FMemory_Free((undefined *)*puVar2);
    }
    *puVar2 = *puVar1;
    *puVar1 = 0;
    param_1[0x10] = param_2[0x10];
    param_1[0x11] = param_2[0x11];
    *(undefined8 *)(param_2 + 0x10) = 0;
  }
  return param_1;
}

// AFortGameStateAthena_GetOverridenDamageSelfRequiresNonZeroDamage exec 0x14291ec60
// Out-of-line dependency 0x141995980; parameter ABI remains to be checked.
undefined1 U_141995980(undefined8 param_1,undefined8 param_2)

{
  char cVar1;
  undefined1 auStackX_18 [16];
  undefined1 auStack_48 [28];
  int iStack_2c;
  longlong *plStack_18;
  
  auStackX_18[0] = 1;
  U_1419b90a0(auStack_48,param_2,param_1,1);
  while ((plStack_18 != (longlong *)0x0 &&
         (cVar1 = (**(code **)(*plStack_18 + 0xb20))(plStack_18,auStackX_18), cVar1 == '\0'))) {
    U_1419f3d90(auStack_48,iStack_2c + 1);
  }
  U_1419bb7e0(auStack_48);
  return auStackX_18[0];
}

// AFortGameStateAthena_SetLocationTextWorldPosition exec 0x142921480
// Out-of-line dependency 0x1419aec90; parameter ABI remains to be checked.
void U_1419aec90(longlong *param_1,undefined8 param_2,undefined8 *param_3)

{
  undefined4 uVar1;
  bool bVar2;
  longlong lVar3;
  undefined8 uVar4;
  UObject *pUVar5;
  longlong lVar6;
  longlong *plStack_18;
  int iStack_10;
  
  lVar3 = (**(code **)(*param_1 + 0xaf0))();
  if (lVar3 != 0) {
    uVar4 = (**(code **)(*param_1 + 0x150))(param_1);
    U_14468fb70(uVar4,&plStack_18);
    while( true ) {
      if ((iStack_10 < 0) || ((int)plStack_18[1] <= iStack_10)) {
        bVar2 = false;
      }
      else {
        bVar2 = true;
      }
      if (!bVar2) break;
      pUVar5 = FWeakObjectPtr_Get((undefined *)(*plStack_18 + (longlong)iStack_10 * 8));
      if (pUVar5 == (UObject *)0x0) {
LAB_1419aed32:
        pUVar5 = (UObject *)0x0;
      }
      else {
        lVar6 = U_142a3d9f0();
        if ((*(int *)(*(longlong *)pUVar5->ClassPrivate + 0x38) < *(int *)(lVar6 + 0x38)) ||
           (*(longlong *)
             (*(longlong *)(*(longlong *)pUVar5->ClassPrivate + 0x30) +
             (longlong)*(int *)(lVar6 + 0x38) * 8) != lVar6 + 0x30)) goto LAB_1419aed32;
      }
      lVar6 = U_141ed6bb0(lVar3,param_2,pUVar5);
      if (lVar6 != 0) {
        uVar1 = *(undefined4 *)(param_3 + 1);
        *(undefined8 *)(lVar6 + 0x138) = *param_3;
        *(undefined4 *)(lVar6 + 0x140) = uVar1;
      }
      iStack_10 = iStack_10 + 1;
    }
  }
  return;
}

// AFortGameStateAthena_SetLocationFontInAllStates exec 0x142921080
// Out-of-line dependency 0x142f440c0; parameter ABI remains to be checked.
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined8 * U_142f440c0(undefined8 *param_1)

{
  undefined8 uVar1;
  
  *(undefined4 *)(param_1 + 2) = 0;
  *(undefined2 *)((longlong)param_1 + 0x14) = 0;
  param_1[3] = 0;
  *param_1 = 0;
  param_1[1] = 0;
  uVar1 = _DAT_147c31bf8;
  param_1[4] = _DAT_147c31bf0;
  param_1[5] = uVar1;
  param_1[6] = 0;
  param_1[7] = 0;
  param_1[8] = 0;
  *(undefined4 *)(param_1 + 9) = 0x18;
  *(undefined1 *)((longlong)param_1 + 0x4c) = 3;
  return param_1;
}
// Out-of-line dependency 0x1419aea90; parameter ABI remains to be checked.
void U_1419aea90(longlong *param_1,undefined8 param_2,undefined8 param_3,undefined1 param_4)

{
  bool bVar1;
  longlong lVar2;
  undefined8 uVar3;
  UObject *pUVar4;
  longlong lVar5;
  longlong *plStack_18;
  int iStack_10;
  
  lVar2 = (**(code **)(*param_1 + 0xaf0))();
  if (lVar2 != 0) {
    uVar3 = (**(code **)(*param_1 + 0x150))(param_1);
    U_14468fb70(uVar3,&plStack_18);
    while( true ) {
      if ((iStack_10 < 0) || ((int)plStack_18[1] <= iStack_10)) {
        bVar1 = false;
      }
      else {
        bVar1 = true;
      }
      if (!bVar1) break;
      pUVar4 = FWeakObjectPtr_Get((undefined *)(*plStack_18 + (longlong)iStack_10 * 8));
      if (pUVar4 == (UObject *)0x0) {
LAB_1419aeb42:
        pUVar4 = (UObject *)0x0;
      }
      else {
        lVar5 = U_142a3d9f0();
        if ((*(int *)(*(longlong *)pUVar4->ClassPrivate + 0x38) < *(int *)(lVar5 + 0x38)) ||
           (*(longlong *)
             (*(longlong *)(*(longlong *)pUVar4->ClassPrivate + 0x30) +
             (longlong)*(int *)(lVar5 + 0x38) * 8) != lVar5 + 0x30)) goto LAB_1419aeb42;
      }
      lVar5 = U_141ed6bb0(lVar2,param_2,pUVar4);
      if (lVar5 != 0) {
        U_141edc2e0(lVar5,param_3,param_4);
      }
      iStack_10 = iStack_10 + 1;
    }
  }
  return;
}

// AFortGameStateAthena_SetLocationColor exec 0x142920e70
// Out-of-line dependency 0x1419ae890; parameter ABI remains to be checked.
void U_1419ae890(longlong *param_1,undefined8 param_2,undefined4 *param_3)

{
  bool bVar1;
  longlong lVar2;
  undefined8 uVar3;
  UObject *pUVar4;
  longlong lVar5;
  longlong *plStack_28;
  int iStack_20;
  undefined4 uStack_18;
  undefined4 uStack_14;
  undefined4 uStack_10;
  undefined4 uStack_c;
  
  uStack_18 = *param_3;
  uStack_14 = param_3[1];
  uStack_10 = param_3[2];
  uStack_c = param_3[3];
  lVar2 = (**(code **)(*param_1 + 0xaf0))();
  if (lVar2 != 0) {
    uVar3 = (**(code **)(*param_1 + 0x150))(param_1);
    U_14468fb70(uVar3,&plStack_28);
    while( true ) {
      if ((iStack_20 < 0) || ((int)plStack_28[1] <= iStack_20)) {
        bVar1 = false;
      }
      else {
        bVar1 = true;
      }
      if (!bVar1) break;
      pUVar4 = FWeakObjectPtr_Get((undefined *)(*plStack_28 + (longlong)iStack_20 * 8));
      if (pUVar4 == (UObject *)0x0) {
LAB_1419ae933:
        pUVar4 = (UObject *)0x0;
      }
      else {
        lVar5 = U_142a3d9f0();
        if ((*(int *)(*(longlong *)pUVar4->ClassPrivate + 0x38) < *(int *)(lVar5 + 0x38)) ||
           (*(longlong *)
             (*(longlong *)(*(longlong *)pUVar4->ClassPrivate + 0x30) +
             (longlong)*(int *)(lVar5 + 0x38) * 8) != lVar5 + 0x30)) goto LAB_1419ae933;
      }
      lVar5 = U_141ed6bb0(lVar2,param_2,pUVar4);
      if (lVar5 != 0) {
        U_141edc1f0(lVar5,&uStack_18,0);
      }
      iStack_20 = iStack_20 + 1;
    }
  }
  return;
}

// AFortGameStateAthena_IsWorldResourceWidgetVisible exec 0x1429202b0
// Out-of-line dependency 0x14199f050; parameter ABI remains to be checked.
undefined1 U_14199f050(undefined8 param_1,longlong param_2,uint param_3)

{
  char cVar1;
  longlong lVar2;
  longlong lVar3;
  undefined1 auStackX_18 [8];
  undefined1 auStack_48 [28];
  int iStack_2c;
  longlong *plStack_18;
  
  auStackX_18[0] = param_3 < 3;
  if ((param_2 != 0) && (lVar3 = *(longlong *)(param_2 + 0x220), lVar3 != 0)) {
    lVar2 = U_1447c8410();
    if ((*(int *)(lVar2 + 0x38) <= *(int *)(*(longlong *)(lVar3 + 0x10) + 0x38)) &&
       (*(longlong *)
         (*(longlong *)(*(longlong *)(lVar3 + 0x10) + 0x30) + (longlong)*(int *)(lVar2 + 0x38) * 8)
        == lVar2 + 0x30)) goto LAB_14199f0ab;
  }
  lVar3 = 0;
LAB_14199f0ab:
  U_1419b90a0(auStack_48,lVar3,param_1,1);
  while ((plStack_18 != (longlong *)0x0 &&
         (cVar1 = (**(code **)(*plStack_18 + 0xc48))(plStack_18,param_2,param_3,auStackX_18),
         cVar1 != '\x02'))) {
    U_1419f3d90(auStack_48,iStack_2c + 1);
  }
  U_1419bb7e0(auStack_48);
  return auStackX_18[0];
}

// AFortGameStateAthena_GetSquadMemberPlayerStates exec 0x14291f450
// Out-of-line dependency 0x1419980e0; parameter ABI remains to be checked.
undefined ** U_1419980e0(undefined8 param_1,undefined **param_2,longlong param_3,char param_4)

{
  undefined *puVar1;
  longlong *plVar2;
  UObject *pUVar3;
  longlong lVar4;
  undefined *this;
  longlong lVar5;
  int iVar6;
  int iVar7;
  int iVar8;
  undefined *Ptr;
  bool bVar9;
  undefined *puStack_48;
  int iStack_40;
  int iStack_3c;
  
  *param_2 = (undefined *)0x0;
  param_2[1] = (undefined *)0x0;
  if (param_3 == 0) {
    return param_2;
  }
  iVar6 = 0;
  iVar8 = 0;
  Ptr = (undefined *)0x0;
  puStack_48 = (undefined *)0x0;
  iStack_3c = 0;
  plVar2 = (longlong *)U_1419982c0(param_1,*(undefined1 *)(param_3 + 0xfdc));
  this = (undefined *)*plVar2;
  puVar1 = this + (longlong)(int)plVar2[1] * 8;
  if (this != puVar1) {
    do {
      pUVar3 = FWeakObjectPtr_Get(this);
      if (pUVar3 != (UObject *)0x0) {
        iStack_40 = iVar6 + 1;
        if (iVar8 < iStack_40) {
          TArray8_ResizeGrow((undefined *)&puStack_48,iVar6);
          Ptr = puStack_48;
          iVar8 = iStack_3c;
        }
        *(UObject **)(Ptr + (longlong)iVar6 * 8) = pUVar3;
        iVar6 = iStack_40;
      }
      this = this + 8;
    } while (this != puVar1);
  }
  if (param_2 == &puStack_48) {
    if (Ptr != (undefined *)0x0) {
      FMemory_Free(Ptr);
    }
  }
  else {
    if (*param_2 != (undefined *)0x0) {
      FMemory_Free(*param_2);
    }
    *param_2 = Ptr;
    *(int *)(param_2 + 1) = iVar6;
    *(int *)((longlong)param_2 + 0xc) = iVar8;
  }
  if ((param_4 == '\0') && (lVar4 = (longlong)*(int *)(param_2 + 1), *(int *)(param_2 + 1) != 0)) {
    iVar8 = 0;
    bVar9 = *(longlong *)*param_2 != param_3;
    lVar5 = 0;
    iVar6 = 0;
    do {
      lVar5 = lVar5 + 1;
      iVar7 = iVar6 + 1;
      if (lVar5 < lVar4) {
        plVar2 = (longlong *)(*param_2 + lVar5 * 8);
        do {
          if (bVar9 != (*plVar2 != param_3)) break;
          iVar7 = iVar7 + 1;
          lVar5 = lVar5 + 1;
          plVar2 = plVar2 + 1;
        } while (lVar5 < lVar4);
      }
      if (bVar9 != false) {
        if (iVar8 != iVar6) {
          memmove(*param_2 + (longlong)iVar8 * 8,*param_2 + (longlong)iVar6 * 8,
                  (longlong)(iVar7 - iVar6) << 3);
        }
        iVar8 = iVar8 + (iVar7 - iVar6);
      }
      bVar9 = (bool)(bVar9 ^ 1);
      iVar6 = iVar7;
    } while (lVar5 < lVar4);
    *(int *)(param_2 + 1) = iVar8;
  }
  return param_2;
}

// AFortGameStateAthena_GetPlaylistTeams exec 0x14291f210
// Out-of-line dependency 0x141996680; parameter ABI remains to be checked.
undefined8 U_141996680(longlong param_1,longlong *param_2)

{
  byte bVar1;
  byte bVar2;
  byte bVar3;
  int iVar4;
  int iVar5;
  bool bVar6;
  char cVar7;
  longlong lVar8;
  byte bVar9;
  
  *(undefined4 *)(param_2 + 1) = 0;
  if (*(int *)((longlong)param_2 + 0xc) != 0) {
    U_14072ceb0(param_2,0);
  }
  lVar8 = *(longlong *)(param_1 + 0x1df8);
  if (lVar8 == 0) {
    lVar8 = *(longlong *)(param_1 + 0x1df0);
  }
  if (lVar8 == 0) {
    return 0;
  }
  bVar2 = *(byte *)(lVar8 + 0xc9);
  bVar3 = *(byte *)(lVar8 + 200);
  iVar4 = *(int *)(lVar8 + 0x58);
  if (bVar2 <= bVar3) {
    return 0;
  }
  if (iVar4 < 1) {
    return 0;
  }
  if (*(int *)((longlong)param_2 + 0xc) < iVar4) {
    U_14072ceb0(param_2,iVar4);
  }
  cVar7 = U_14199e430(param_1);
  if (cVar7 == '\0') {
    lVar8 = *(longlong *)(param_1 + 0x1df8);
    if (lVar8 == 0) {
      lVar8 = *(longlong *)(param_1 + 0x1df0);
    }
    cVar7 = U_141a4a530(lVar8);
    if (cVar7 == '\0') {
      bVar6 = false;
      goto LAB_14199674a;
    }
  }
  bVar6 = true;
LAB_14199674a:
  bVar9 = 0;
  while( true ) {
    bVar1 = bVar9 + bVar3;
    if (bVar2 < bVar1) {
      return 1;
    }
    if ((*(int *)(param_1 + 0x12f0) <= (int)(uint)bVar9) && (!bVar6)) break;
    iVar5 = (int)param_2[1];
    *(int *)(param_2 + 1) = iVar5 + 1;
    if (*(int *)((longlong)param_2 + 0xc) < iVar5 + 1) {
      U_14069e940(param_2,iVar5);
    }
    bVar9 = bVar9 + 1;
    *(byte *)((longlong)iVar5 + *param_2) = bVar1;
    if (iVar4 <= (int)(uint)bVar9) {
      return 1;
    }
  }
  return 1;
}

// AFortGameStateAthena_SetAllLocations_StateColor exec 0x142920b40
// Out-of-line dependency 0x1419ae140; parameter ABI remains to be checked.
void U_1419ae140(longlong *param_1,undefined8 param_2)

{
  uint *puVar1;
  uint uVar2;
  uint uVar3;
  bool bVar4;
  longlong *plVar5;
  undefined8 uVar6;
  UObject *pUVar7;
  longlong lVar8;
  longlong *plVar9;
  int iVar10;
  uint uVar11;
  uint uVar12;
  uint *puVar13;
  int iStack_a8;
  uint uStack_a4;
  uint uStack_94;
  int iStack_90;
  undefined4 uStack_8c;
  longlong *plStack_88;
  int iStack_80;
  undefined4 uStack_78;
  undefined4 uStack_74;
  int iStack_70;
  uint uStack_6c;
  uint *puStack_68;
  undefined8 uStack_60;
  undefined8 uStack_58;
  undefined8 uStack_50;
  undefined8 uStack_48;
  uint *puStack_40;
  undefined8 uStack_38;
  undefined8 uStack_30;
  
  plVar5 = (longlong *)(**(code **)(*param_1 + 0xaf0))();
  if (plVar5 != (longlong *)0x0) {
    uVar6 = (**(code **)(*param_1 + 0x150))(param_1);
    U_14468fb70(uVar6,&plStack_88);
    while( true ) {
      if ((iStack_80 < 0) || ((int)plStack_88[1] <= iStack_80)) {
        bVar4 = false;
      }
      else {
        bVar4 = true;
      }
      if (!bVar4) break;
      pUVar7 = FWeakObjectPtr_Get((undefined *)(*plStack_88 + (longlong)iStack_80 * 8));
      if (pUVar7 == (UObject *)0x0) {
LAB_1419ae1fd:
        pUVar7 = (UObject *)0x0;
      }
      else {
        lVar8 = U_142a3d9f0();
        if ((*(int *)(*(longlong *)pUVar7->ClassPrivate + 0x38) < *(int *)(lVar8 + 0x38)) ||
           (*(longlong *)
             (*(longlong *)(*(longlong *)pUVar7->ClassPrivate + 0x30) +
             (longlong)*(int *)(lVar8 + 0x38) * 8) != lVar8 + 0x30)) goto LAB_1419ae1fd;
      }
      plVar9 = (longlong *)(**(code **)(*plVar5 + 0x6a8))(plVar5,pUVar7);
      uVar2 = *(uint *)(plVar9 + 5);
      iStack_a8 = 0;
      uStack_a4 = 1;
      puVar1 = (uint *)(plVar9 + 2);
      uStack_94 = 0;
      iStack_90 = 0;
      if (uVar2 != 0) {
        puVar13 = puVar1;
        if ((uint *)plVar9[4] != (uint *)0x0) {
          puVar13 = (uint *)plVar9[4];
        }
        uVar3 = *puVar13;
        iStack_a8 = 0;
        iStack_90 = 0;
        while (uVar3 == 0) {
          lVar8 = (longlong)iStack_a8;
          iStack_90 = iStack_90 + 0x20;
          iVar10 = iStack_a8 + 1;
          bVar4 = (int)((uVar2 - 1) + ((int)(uVar2 - 1) >> 0x1f & 0x1fU)) >> 5 <= iStack_a8;
          iStack_a8 = iVar10;
          if (bVar4) goto LAB_1419ae2af;
          uVar3 = puVar13[lVar8 + 1];
        }
        uStack_a4 = -uVar3 & uVar3;
        iVar10 = 0x1f;
        if (uStack_a4 != 0) {
          for (; uStack_a4 >> iVar10 == 0; iVar10 = iVar10 + -1) {
          }
        }
        if (uStack_a4 == 0) {
          iVar10 = 0x20;
        }
        else {
          iVar10 = 0x1f - iVar10;
        }
        uStack_94 = (iStack_90 - iVar10) + 0x1f;
        if ((int)uVar2 < (int)uStack_94) {
LAB_1419ae2af:
          uStack_94 = uVar2;
        }
      }
      uVar3 = *(uint *)(plVar9 + 5);
      uStack_60 = CONCAT44(uStack_94,0xffffffff);
      uStack_58 = CONCAT44(uStack_8c,iStack_90);
      uStack_48 = CONCAT44(uStack_a4,iStack_a8);
      uVar12 = uVar3 & 0xffffffe0;
      uStack_50._0_4_ = SUB84(plVar9,0);
      uStack_50._4_4_ = (undefined4)((ulonglong)plVar9 >> 0x20);
      uStack_78 = (undefined4)uStack_50;
      uStack_74 = uStack_50._4_4_;
      iStack_70 = iStack_a8;
      uStack_6c = uStack_a4;
      puStack_68 = puVar1;
      uStack_50 = plVar9;
      puStack_40 = puVar1;
      uStack_38 = uStack_60;
      uStack_30 = uStack_58;
      uStack_94 = uVar3;
      if (uVar3 != uVar2) {
        puVar13 = puVar1;
        if ((uint *)plVar9[4] != (uint *)0x0) {
          puVar13 = (uint *)plVar9[4];
        }
        uVar11 = puVar13[(int)uVar3 >> 5] & -1 << ((byte)uVar3 & 0x1f);
        iVar10 = (int)uVar3 >> 5;
        while (uVar11 == 0) {
          uVar12 = uVar12 + 0x20;
          if ((int)((uVar2 - 1) + ((int)(uVar2 - 1) >> 0x1f & 0x1fU)) >> 5 <= iVar10)
          goto LAB_1419ae376;
          uVar11 = puVar13[(longlong)iVar10 + 1];
          iVar10 = iVar10 + 1;
        }
        uVar11 = -uVar11 & uVar11;
        iVar10 = 0x1f;
        if (uVar11 != 0) {
          for (; uVar11 >> iVar10 == 0; iVar10 = iVar10 + -1) {
          }
        }
        if (uVar11 == 0) {
          iVar10 = 0x20;
        }
        else {
          iVar10 = 0x1f - iVar10;
        }
        uStack_94 = (uVar12 - iVar10) + 0x1f;
        if ((int)uVar2 < (int)uStack_94) {
LAB_1419ae376:
          uStack_94 = uVar2;
        }
      }
      while( true ) {
        if ((uStack_60._4_4_ == uStack_94) && (puStack_68 == puVar1)) {
          bVar4 = true;
        }
        else {
          bVar4 = false;
        }
        if ((bVar4) && ((longlong *)CONCAT44(uStack_74,uStack_78) == plVar9)) {
          bVar4 = false;
        }
        else {
          bVar4 = true;
        }
        if (!bVar4) break;
        U_141edc1f0((longlong)(int)uStack_60._4_4_ * 0x1c8 +
                      *(longlong *)CONCAT44(uStack_74,uStack_78) + 8,param_2);
        uStack_60 = CONCAT44(uStack_60._4_4_,(uint)uStack_60 & ~uStack_6c);
        U_140687d00(&iStack_70);
      }
      iStack_80 = iStack_80 + 1;
    }
  }
  return;
}

// AFortGameStateAthena_SetLocation_StateColor exec 0x142921570
// Out-of-line dependency 0x1419ae980; parameter ABI remains to be checked.
void U_1419ae980(longlong *param_1,undefined8 param_2,undefined8 param_3,undefined1 param_4)

{
  bool bVar1;
  longlong lVar2;
  undefined8 uVar3;
  UObject *pUVar4;
  longlong lVar5;
  longlong *plStack_18;
  int iStack_10;
  
  lVar2 = (**(code **)(*param_1 + 0xaf0))();
  if (lVar2 != 0) {
    uVar3 = (**(code **)(*param_1 + 0x150))(param_1);
    U_14468fb70(uVar3,&plStack_18);
    while( true ) {
      if ((iStack_10 < 0) || ((int)plStack_18[1] <= iStack_10)) {
        bVar1 = false;
      }
      else {
        bVar1 = true;
      }
      if (!bVar1) break;
      pUVar4 = FWeakObjectPtr_Get((undefined *)(*plStack_18 + (longlong)iStack_10 * 8));
      if (pUVar4 == (UObject *)0x0) {
LAB_1419aea32:
        pUVar4 = (UObject *)0x0;
      }
      else {
        lVar5 = U_142a3d9f0();
        if ((*(int *)(*(longlong *)pUVar4->ClassPrivate + 0x38) < *(int *)(lVar5 + 0x38)) ||
           (*(longlong *)
             (*(longlong *)(*(longlong *)pUVar4->ClassPrivate + 0x30) +
             (longlong)*(int *)(lVar5 + 0x38) * 8) != lVar5 + 0x30)) goto LAB_1419aea32;
      }
      lVar5 = U_141ed6bb0(lVar2,param_2,pUVar4);
      if (lVar5 != 0) {
        U_141edc1f0(lVar5,param_3,param_4);
      }
      iStack_10 = iStack_10 + 1;
    }
  }
  return;
}

// AFortGameStateAthena_SetLocationFontInState exec 0x1429211a0
// Out-of-line dependency 0x142f440c0; parameter ABI remains to be checked.
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined8 * U_142f440c0(undefined8 *param_1)

{
  undefined8 uVar1;
  
  *(undefined4 *)(param_1 + 2) = 0;
  *(undefined2 *)((longlong)param_1 + 0x14) = 0;
  param_1[3] = 0;
  *param_1 = 0;
  param_1[1] = 0;
  uVar1 = _DAT_147c31bf8;
  param_1[4] = _DAT_147c31bf0;
  param_1[5] = uVar1;
  param_1[6] = 0;
  param_1[7] = 0;
  param_1[8] = 0;
  *(undefined4 *)(param_1 + 9) = 0x18;
  *(undefined1 *)((longlong)param_1 + 0x4c) = 3;
  return param_1;
}
// Out-of-line dependency 0x1419aea90; parameter ABI remains to be checked.
void U_1419aea90(longlong *param_1,undefined8 param_2,undefined8 param_3,undefined1 param_4)

{
  bool bVar1;
  longlong lVar2;
  undefined8 uVar3;
  UObject *pUVar4;
  longlong lVar5;
  longlong *plStack_18;
  int iStack_10;
  
  lVar2 = (**(code **)(*param_1 + 0xaf0))();
  if (lVar2 != 0) {
    uVar3 = (**(code **)(*param_1 + 0x150))(param_1);
    U_14468fb70(uVar3,&plStack_18);
    while( true ) {
      if ((iStack_10 < 0) || ((int)plStack_18[1] <= iStack_10)) {
        bVar1 = false;
      }
      else {
        bVar1 = true;
      }
      if (!bVar1) break;
      pUVar4 = FWeakObjectPtr_Get((undefined *)(*plStack_18 + (longlong)iStack_10 * 8));
      if (pUVar4 == (UObject *)0x0) {
LAB_1419aeb42:
        pUVar4 = (UObject *)0x0;
      }
      else {
        lVar5 = U_142a3d9f0();
        if ((*(int *)(*(longlong *)pUVar4->ClassPrivate + 0x38) < *(int *)(lVar5 + 0x38)) ||
           (*(longlong *)
             (*(longlong *)(*(longlong *)pUVar4->ClassPrivate + 0x30) +
             (longlong)*(int *)(lVar5 + 0x38) * 8) != lVar5 + 0x30)) goto LAB_1419aeb42;
      }
      lVar5 = U_141ed6bb0(lVar2,param_2,pUVar4);
      if (lVar5 != 0) {
        U_141edc2e0(lVar5,param_3,param_4);
      }
      iStack_10 = iStack_10 + 1;
    }
  }
  return;
}

// AFortGameStateAthena_CanSpawnActor exec 0x14291e220
// Out-of-line dependency 0x14198bb40; parameter ABI remains to be checked.
undefined8 U_14198bb40(longlong param_1,undefined8 param_2)

{
  U_14198ec00(param_1 + 0x20f8);
  return param_2;
}
// Out-of-line dependency 0x142918000; parameter ABI remains to be checked.
undefined1 * U_142918000(undefined1 *param_1,undefined1 *param_2)

{
  undefined8 *puVar1;
  undefined8 *puVar2;
  undefined8 uVar3;
  
  puVar1 = (undefined8 *)(param_2 + 8);
  puVar2 = (undefined8 *)(param_1 + 8);
  *param_1 = *param_2;
  if (puVar2 != puVar1) {
    if ((undefined *)*puVar2 != (undefined *)0x0) {
      FMemory_Free((undefined *)*puVar2);
    }
    *puVar2 = *puVar1;
    *puVar1 = 0;
    *(undefined4 *)(param_1 + 0x10) = *(undefined4 *)(param_2 + 0x10);
    *(undefined4 *)(param_1 + 0x14) = *(undefined4 *)(param_2 + 0x14);
    *(undefined8 *)(param_2 + 0x10) = 0;
  }
  uVar3 = *(undefined8 *)(param_1 + 0x18);
  *(undefined8 *)(param_1 + 0x18) = *(undefined8 *)(param_2 + 0x18);
  *(undefined8 *)(param_2 + 0x18) = uVar3;
  uVar3 = *(undefined8 *)(param_1 + 0x20);
  *(undefined8 *)(param_1 + 0x20) = *(undefined8 *)(param_2 + 0x20);
  *(undefined8 *)(param_2 + 0x20) = uVar3;
  *(undefined4 *)(param_1 + 0x28) = *(undefined4 *)(param_2 + 0x28);
  return param_1;
}

// AFortGameStateAthena_ShouldReturnToMatchmakingOriginOnMatchEnd exec 0x142921790
// Out-of-line dependency 0x1419b0cc0; parameter ABI remains to be checked.
void U_1419b0cc0(longlong param_1)

{
  longlong lVar1;
  
  lVar1 = *(longlong *)(param_1 + 0x1df8);
  if (lVar1 == 0) {
    lVar1 = *(longlong *)(param_1 + 0x1df0);
  }
  U_141a5ba80(lVar1);
  return;
}

// AFortGameStateAthena_ShouldDBNOTeamInfoFlash exec 0x1429216a0
// Out-of-line dependency 0x1419b0350; parameter ABI remains to be checked.
ulonglong U_1419b0350(undefined8 param_1)

{
  byte bVar1;
  char cVar2;
  ulonglong uVar3;
  byte abStackX_10 [24];
  undefined1 auStack_48 [28];
  int iStack_2c;
  longlong *plStack_18;
  
  U_1419b90a0(auStack_48,0,param_1,1);
  while( true ) {
    if (plStack_18 == (longlong *)0x0) {
      uVar3 = U_1419bb7e0(auStack_48);
      return uVar3 & 0xffffffffffffff00;
    }
    abStackX_10[0] = 0;
    cVar2 = (**(code **)(*plStack_18 + 0x908))(plStack_18,abStackX_10);
    bVar1 = abStackX_10[0];
    if (cVar2 != '\0') break;
    U_1419f3d90(auStack_48,iStack_2c + 1);
  }
  U_1419bb7e0(auStack_48);
  return (ulonglong)bVar1;
}

// AFortGameStateAthena_ShouldHaveInfiniteResources exec 0x142921700
// Out-of-line dependency 0x1419b09b0; parameter ABI remains to be checked.
byte U_1419b09b0(undefined8 param_1,undefined8 param_2)

{
  char cVar1;
  byte bVar2;
  byte abStackX_18 [16];
  undefined1 auStack_48 [28];
  int iStack_2c;
  longlong *plStack_18;
  
  bVar2 = 0;
  U_1419b90a0(auStack_48,param_2,param_1,1);
  while (plStack_18 != (longlong *)0x0) {
    abStackX_18[0] = 0;
    cVar1 = (**(code **)(*plStack_18 + 0x9c8))(plStack_18,abStackX_18,param_2);
    bVar2 = bVar2 | abStackX_18[0];
    if (cVar1 == '\x02') break;
    U_1419f3d90(auStack_48,iStack_2c + 1);
  }
  U_1419bb7e0(auStack_48);
  return bVar2;
}

// AFortGameStateAthena_HandleGameModeMessageSuccess exec 0x14291fb80
// Out-of-line dependency 0x14199abb0; parameter ABI remains to be checked.
void U_14199abb0(longlong param_1,undefined8 param_2)

{
  undefined8 auStackX_8 [4];
  
  auStackX_8[0] = param_2;
  U_1406a9dc0(param_1 + 0x10e8,auStackX_8);
  return;
}

// AFortPlayerStateAthena_GetRemainingResurrectionChipExpirationTime exec 0x142a8d7f0
// Out-of-line dependency 0x141a48170; parameter ABI remains to be checked.
float U_141a48170(longlong param_1)

{
  float fVar1;
  
  fVar1 = (float)U_141ef6bc0();
  return *(float *)(param_1 + 0x1390) - fVar1;
}

// AFortPlayerStateAthena_GetPinColor exec 0x142a8d7b0
// Out-of-line dependency 0x141a45bb0; parameter ABI remains to be checked.
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined8 * U_141a45bb0(UObject *param_1,undefined8 *param_2)

{
  undefined8 uVar1;
  longlong lVar2;
  longlong lVar3;
  undefined *puVar4;
  longlong *plVar5;
  UObject *pUVar6;
  longlong *plVar7;
  int iVar8;
  ulonglong uVar9;
  uint uVar10;
  ulonglong uVar11;
  undefined4 auStackX_8 [2];
  
  uVar1 = (**(code **)(*(longlong *)param_1->vftable + 0x150))();
  lVar2 = U_14463f750(_DAT_148351eb8,uVar1);
  if (lVar2 == 0) goto LAB_141a45d3e;
  uVar9 = 0xffffffff;
  lVar3 = U_14447f220(lVar2,uVar1);
  if (lVar3 == 0) {
LAB_141a45c62:
    lVar2 = U_14447f220(lVar2,uVar1);
    if (lVar2 != 0) {
      lVar3 = U_142a69680();
      if ((*(int *)(lVar3 + 0x38) <= *(int *)(*(longlong *)(lVar2 + 0x10) + 0x38)) &&
         (*(longlong *)
           (*(longlong *)(*(longlong *)(lVar2 + 0x10) + 0x30) + (longlong)*(int *)(lVar3 + 0x38) * 8
           ) == lVar3 + 0x30)) {
        plVar5 = *(longlong **)(lVar2 + 0x2d70);
        for (plVar7 = plVar5; plVar7 != plVar5 + *(int *)(lVar2 + 0x2d78); plVar7 = plVar7 + 1) {
          if ((UObject *)*plVar7 == param_1) {
            uVar9 = (longlong)plVar7 - (longlong)plVar5 >> 3;
            break;
          }
        }
      }
    }
  }
  else {
    puVar4 = AFortPlayerControllerAthena_StaticClass();
    if ((*(int *)(*(longlong *)(lVar3 + 0x10) + 0x38) < *(int *)(puVar4 + 0x38)) ||
       (*(undefined **)
         (*(longlong *)(*(longlong *)(lVar3 + 0x10) + 0x30) + (longlong)*(int *)(puVar4 + 0x38) * 8)
        != puVar4 + 0x30)) goto LAB_141a45c62;
    plVar5 = (longlong *)U_1419d9220(lVar3);
    uVar11 = 0;
    if (0 < (int)plVar5[1]) {
      do {
        pUVar6 = FWeakObjectPtr_Get((undefined *)(*plVar5 + uVar11 * 8));
        if (param_1 == pUVar6) {
          uVar9 = uVar11;
          break;
        }
        uVar10 = (int)uVar11 + 1;
        uVar11 = (ulonglong)uVar10;
      } while ((int)uVar10 < (int)plVar5[1]);
    }
  }
  iVar8 = (int)uVar9;
  if (iVar8 == 0) {
    auStackX_8[0] = 0xff167de9;
    U_142c47120(param_2,auStackX_8);
    return param_2;
  }
  if (iVar8 == 1) {
    auStackX_8[0] = 0xffff6994;
    U_142c47120(param_2,auStackX_8);
    return param_2;
  }
  if (iVar8 == 2) {
    auStackX_8[0] = 0xffaa00ff;
    U_142c47120(param_2,auStackX_8);
    return param_2;
  }
  if (iVar8 == 3) {
    auStackX_8[0] = 0xfffed702;
    U_142c47120(param_2,auStackX_8);
    return param_2;
  }
LAB_141a45d3e:
  uVar1 = _DAT_147c31bd8;
  *param_2 = _DAT_147c31bd0;
  param_2[1] = uVar1;
  return param_2;
}

// AFortPlayerStateAthena_GetDeathInfo exec 0x142a8d770
// Out-of-line dependency 0x141a42550; parameter ABI remains to be checked.
longlong U_141a42550(longlong param_1)

{
  return param_1 + 0x12c8;
}
// Out-of-line dependency 0x141a33710; parameter ABI remains to be checked.
undefined8 * U_141a33710(undefined8 *param_1,undefined8 *param_2)

{
  *param_1 = *param_2;
  param_1[1] = param_2[1];
  *(undefined1 *)(param_1 + 2) = *(undefined1 *)(param_2 + 2);
  *(undefined1 *)((longlong)param_1 + 0x11) = *(undefined1 *)((longlong)param_2 + 0x11);
  *(undefined1 *)((longlong)param_1 + 0x12) = *(undefined1 *)((longlong)param_2 + 0x12);
  *(undefined4 *)((longlong)param_1 + 0x14) = *(undefined4 *)((longlong)param_2 + 0x14);
  param_1[3] = param_2[3];
  *(undefined4 *)(param_1 + 4) = *(undefined4 *)(param_2 + 4);
  *(undefined1 *)((longlong)param_1 + 0x24) = *(undefined1 *)((longlong)param_2 + 0x24);
  FGameplayTagContainer_CopyAssign((undefined *)(param_1 + 5),(undefined *)(param_2 + 5));
  *(undefined1 *)(param_1 + 9) = *(undefined1 *)(param_2 + 9);
  return param_1;
}

// AFortPlayerStateAthena_CanEditCreativeIsland exec 0x142a8d310
// Out-of-line dependency 0x141a3c850; parameter ABI remains to be checked.
undefined8 U_141a3c850(longlong param_1)

{
  int *piVar1;
  char cVar2;
  longlong lVar3;
  longlong *plVar4;
  undefined8 uVar5;
  undefined **appuStack_38 [2];
  longlong *plStack_28;
  undefined *puStack_20;
  
  lVar3 = U_141c74820();
  if (lVar3 == 0) {
    plVar4 = (longlong *)0x0;
  }
  else {
    plVar4 = (longlong *)U_141c7d420(lVar3,param_1);
    if ((plVar4 != (longlong *)0x0) && ((*(byte *)(plVar4 + 0x5b) & 1) != 0)) {
      return 0;
    }
  }
  cVar2 = '\0';
  if (plVar4 != (longlong *)0x0) {
    uVar5 = U_14297b320();
    lVar3 = (**(code **)(*plVar4 + 0x610))(plVar4,uVar5);
    if (lVar3 != 0) {
      uVar5 = U_142419ef0(param_1,appuStack_38);
      cVar2 = U_141eeab20(lVar3,uVar5);
      appuStack_38[0] = &PTR_FUN_145a95888;
      if (puStack_20 != (undefined *)0x0) {
        FMemory_Free(puStack_20);
      }
      if (plStack_28 != (longlong *)0x0) {
        plVar4 = plStack_28 + 1;
        *(int *)plVar4 = (int)*plVar4 + -1;
        if ((int)*plVar4 == 0) {
          (**(code **)*plStack_28)(plStack_28);
          piVar1 = (int *)((longlong)plStack_28 + 0xc);
          *piVar1 = *piVar1 + -1;
          if (*piVar1 == 0) {
            (**(code **)(*plStack_28 + 8))(plStack_28,1);
          }
        }
      }
    }
  }
  if (((*(byte *)(param_1 + 0x330) & 1) == 0) && (cVar2 == '\0')) {
    return 0;
  }
  return 1;
}

// AFortPlayerStateAthena_CanBeResurrected exec 0x142a8d2e0
// Out-of-line dependency 0x141a3c6f0; parameter ABI remains to be checked.
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined8 U_141a3c6f0(longlong *param_1)

{
  bool bVar1;
  bool bVar2;
  bool bVar3;
  char cVar4;
  char cVar5;
  longlong lVar6;
  longlong lVar7;
  undefined *puVar8;
  undefined8 uVar9;
  
  bVar3 = false;
  bVar2 = false;
  bVar1 = false;
  lVar6 = (**(code **)(*param_1 + 0x150))();
  if (lVar6 != 0) {
    lVar7 = U_14463f750(_DAT_148351eb8,lVar6);
    bVar2 = bVar3;
    if ((lVar7 != 0) && (lVar6 = U_14447f220(lVar7,lVar6), lVar6 != 0)) {
      puVar8 = AFortPlayerControllerAthena_StaticClass();
      if ((*(int *)(puVar8 + 0x38) <= *(int *)(*(longlong *)(lVar6 + 0x10) + 0x38)) &&
         ((*(undefined **)
            (*(longlong *)(*(longlong *)(lVar6 + 0x10) + 0x30) +
            (longlong)*(int *)(puVar8 + 0x38) * 8) == puVar8 + 0x30 &&
          (lVar6 = *(longlong *)(lVar6 + 0x3620), lVar6 != 0)))) {
        lVar7 = *(longlong *)(lVar6 + 200);
        lVar6 = lVar7 + (longlong)*(int *)(lVar6 + 0xd0) * 0x28;
        for (; lVar7 != lVar6; lVar7 = lVar7 + 0x28) {
          if ((*(longlong **)(lVar7 + 8) == (longlong *)0x0) ||
             (cVar4 = (**(code **)(**(longlong **)(lVar7 + 8) + 0x28))(), cVar4 == '\0')) {
            cVar4 = '\0';
          }
          else {
            cVar4 = '\x01';
          }
          if (((longlong *)param_1[0x4a] == (longlong *)0x0) ||
             (cVar5 = (**(code **)(*(longlong *)param_1[0x4a] + 0x28))(), cVar5 == '\0')) {
            cVar5 = '\0';
          }
          else {
            cVar5 = '\x01';
          }
          if ((cVar4 == cVar5) &&
             ((cVar4 == '\0' ||
              (cVar4 = (**(code **)**(undefined8 **)(lVar7 + 8))
                                 (*(undefined8 **)(lVar7 + 8),param_1[0x4a]), cVar4 != '\0')))) {
            bVar1 = true;
          }
          else {
            bVar1 = false;
          }
          if (bVar1) {
            bVar2 = true;
            goto LAB_141a3c80d;
          }
        }
        bVar2 = false;
      }
    }
LAB_141a3c80d:
    bVar1 = 0 < *(int *)((longlong)param_1 + 0xe6c);
  }
  cVar4 = (**(code **)(*param_1 + 0x840))(param_1);
  if (((cVar4 == '\0') || (!bVar2)) || (bVar1)) {
    uVar9 = 0;
  }
  else {
    uVar9 = 1;
  }
  return uVar9;
}

// AFortPlayerControllerGameplay_IsUsingHideOtherPlayerNames exec 0x142a65770
// Out-of-line dependency 0x142a65770; parameter ABI remains to be checked.
void U_142a65770(undefined8 param_1,longlong param_2,undefined1 *param_3)

{
  undefined1 uVar1;
  
  *(ulonglong *)(param_2 + 0x20) =
       (ulonglong)(*(longlong *)(param_2 + 0x20) != 0) + *(longlong *)(param_2 + 0x20);
  uVar1 = U_1423d97d0();
  *param_3 = uVar1;
  return;
}
// Out-of-line dependency 0x1423d97d0; parameter ABI remains to be checked.
void U_1423d97d0(void)

{
  longlong lVar1;
  
  lVar1 = U_14237e580();
  if (lVar1 == 0) {
    return;
  }
  U_141e272f0(lVar1);
  return;
}

// AFortPlayerControllerGameplay_IsCreativeQuickbarActive exec 0x142a65650
// Out-of-line dependency 0x142a65650; parameter ABI remains to be checked.
void U_142a65650(longlong param_1,longlong param_2,undefined1 *param_3)

{
  *(ulonglong *)(param_2 + 0x20) =
       (ulonglong)(*(longlong *)(param_2 + 0x20) != 0) + *(longlong *)(param_2 + 0x20);
  if ((((*(byte *)(param_1 + 0x25c9) & 2) == 0) || (*(longlong *)(param_1 + 0x22f0) == 0)) ||
     (*(char *)(*(longlong *)(param_1 + 0x22f0) + 0x100) == '\0')) {
    *param_3 = 0;
    return;
  }
  *param_3 = 1;
  return;
}

// AFortPlayerControllerGameplay_IsCreativePreviewScreenshotAvailable exec 0x142a65620
// Out-of-line dependency 0x142a65620; parameter ABI remains to be checked.
void U_142a65620(undefined8 param_1,longlong param_2,undefined1 *param_3)

{
  undefined1 uVar1;
  
  *(ulonglong *)(param_2 + 0x20) =
       (ulonglong)(*(longlong *)(param_2 + 0x20) != 0) + *(longlong *)(param_2 + 0x20);
  uVar1 = U_1423d8f20();
  *param_3 = uVar1;
  return;
}
// Out-of-line dependency 0x1423d8f20; parameter ABI remains to be checked.
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

bool U_1423d8f20(longlong param_1)

{
  char cVar1;
  undefined *puVar2;
  
  if (param_1 != 0) {
    puVar2 = AFortPlayerControllerAthena_StaticClass();
    if ((*(int *)(puVar2 + 0x38) <= *(int *)(*(longlong *)(param_1 + 0x10) + 0x38)) &&
       (*(undefined **)
         (*(longlong *)(*(longlong *)(param_1 + 0x10) + 0x30) +
         (longlong)*(int *)(puVar2 + 0x38) * 8) == puVar2 + 0x30)) {
      cVar1 = U_1419dd8b0(param_1);
      if (cVar1 != '\0') {
        return false;
      }
    }
  }
  return _DAT_1478de924 != 0;
}

// AFortPlayerControllerGameplay_IsCreativeIslandExporterEnabled exec 0x142a65590
// Out-of-line dependency 0x142a65590; parameter ABI remains to be checked.
void U_142a65590(undefined8 param_1,longlong param_2,undefined1 *param_3)

{
  undefined1 uVar1;
  
  *(ulonglong *)(param_2 + 0x20) =
       (ulonglong)(*(longlong *)(param_2 + 0x20) != 0) + *(longlong *)(param_2 + 0x20);
  uVar1 = U_1423d8ec0();
  *param_3 = uVar1;
  return;
}
// Out-of-line dependency 0x1423d8ec0; parameter ABI remains to be checked.
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined8 U_1423d8ec0(longlong param_1)

{
  undefined *puVar1;
  
  if (((*(byte *)(param_1 + 0x25ca) & 4) == 0) && (_DAT_14815c4a0 == 0)) {
    puVar1 = UFortRuntimeOptions_GetDefault();
    if (puVar1[0x199] == '\0') {
      return 0;
    }
  }
  return 1;
}

// AFortPlayerControllerGameplay_IsAllowedToFly exec 0x142a65560
// Out-of-line dependency 0x142a65560; parameter ABI remains to be checked.
void U_142a65560(undefined8 param_1,longlong param_2,undefined1 *param_3)

{
  undefined1 uVar1;
  
  *(ulonglong *)(param_2 + 0x20) =
       (ulonglong)(*(longlong *)(param_2 + 0x20) != 0) + *(longlong *)(param_2 + 0x20);
  uVar1 = U_1423d8ab0();
  *param_3 = uVar1;
  return;
}
// Out-of-line dependency 0x1423d8ab0; parameter ABI remains to be checked.
ulonglong U_1423d8ab0(longlong *param_1)

{
  longlong *plVar1;
  bool bVar2;
  char cVar3;
  char extraout_AL;
  longlong lVar4;
  longlong lVar5;
  ulonglong uVar6;
  ulonglong extraout_RAX;
  ulonglong extraout_RAX_00;
  undefined *puVar7;
  undefined8 uVar8;
  undefined7 uVar11;
  longlong lVar9;
  longlong lVar10;
  undefined7 extraout_var;
  int *piVar12;
  longlong *plVar13;
  ulonglong uStackX_8;
  int *piStack_68;
  undefined8 uStack_60;
  undefined *puStack_58;
  undefined8 uStack_50;
  longlong *plStack_38;
  
  lVar4 = (**(code **)(*param_1 + 0x14d8))();
  if (lVar4 != 0) {
    lVar5 = (**(code **)(*(longlong *)(lVar4 + 0x4c8) + 0x10))();
    if (lVar5 != 0) {
      piStack_68 = (int *)0x0;
      uStack_60 = 0;
      puStack_58 = (undefined *)0x0;
      uStack_50 = 0;
      U_143e8c1b0(&piStack_68);
      lVar5 = U_141ed1800();
      bVar2 = FName_EqualsEName(*(int *)(lVar5 + 0x7a8),0);
      if (*(int *)(lVar5 + 0x7ac) != 0 || !bVar2) {
        uVar6 = (ulonglong)(int)uStack_60;
        lVar9 = uVar6 * 2;
        if (piStack_68 != piStack_68 + lVar9) {
          piVar12 = piStack_68;
          do {
            bVar2 = *piVar12 == *(int *)(lVar5 + 0x7a8);
            uVar6 = CONCAT71((int7)(uVar6 >> 8),bVar2);
            if (bVar2 && piVar12[1] == *(int *)(lVar5 + 0x7ac)) {
              if (puStack_58 != (undefined *)0x0) {
                FMemory_Free(puStack_58);
                uVar6 = extraout_RAX;
              }
              if (piStack_68 != (int *)0x0) {
                FMemory_Free((undefined *)piStack_68);
                return extraout_RAX_00 & 0xffffffffffffff00;
              }
              goto LAB_1423d8ba5;
            }
            piVar12 = piVar12 + 2;
          } while (piVar12 != piStack_68 + lVar9);
        }
      }
      if (puStack_58 != (undefined *)0x0) {
        FMemory_Free(puStack_58);
      }
      if (piStack_68 != (int *)0x0) {
        FMemory_Free((undefined *)piStack_68);
      }
    }
    uVar6 = U_1423318f0(lVar4);
    if ((char)uVar6 != '\0') {
LAB_1423d8ba5:
      return uVar6 & 0xffffffffffffff00;
    }
  }
  lVar4 = (**(code **)(*param_1 + 0x150))(param_1);
  if ((lVar4 != 0) && (lVar4 = *(longlong *)(lVar4 + 0x130), lVar4 != 0)) {
    puVar7 = AFortGameStateAthena_StaticClass();
    if ((*(int *)(puVar7 + 0x38) <= *(int *)(*(longlong *)(lVar4 + 0x10) + 0x38)) &&
       (*(undefined **)
         (*(longlong *)(*(longlong *)(lVar4 + 0x10) + 0x30) + (longlong)*(int *)(puVar7 + 0x38) * 8)
        == puVar7 + 0x30)) goto LAB_1423d8c29;
  }
  lVar4 = 0;
LAB_1423d8c29:
  puVar7 = AFortPlayerControllerAthena_StaticClass();
  if ((((*(int *)(puVar7 + 0x38) <= *(int *)(param_1[2] + 0x38)) &&
       (*(undefined **)(*(longlong *)(param_1[2] + 0x30) + (longlong)*(int *)(puVar7 + 0x38) * 8) ==
        puVar7 + 0x30)) && (lVar4 != 0)) &&
     ((cVar3 = U_14199e430(lVar4), cVar3 != '\0' && (cVar3 = U_14199e2c0(lVar4), cVar3 == '\0'))
     )) {
    uStackX_8 = 0;
    cVar3 = U_1419c3880(param_1,&uStackX_8);
    if ((uStackX_8 == 0) || ((*(byte *)(uStackX_8 + 0x2d8) & 1) != 0)) {
      bVar2 = false;
    }
    else {
      bVar2 = true;
    }
    uVar8 = U_1419dd8b0(param_1);
    uVar11 = (undefined7)((ulonglong)uVar8 >> 8);
    if (((cVar3 != '\0') && (bVar2)) && ((char)uVar8 == '\0')) {
LAB_1423d8db2:
      return CONCAT71(uVar11,1);
    }
  }
  lVar5 = param_1[0x2e7];
  if (lVar5 != 0) {
    plVar13 = *(longlong **)(lVar5 + 0x390);
    plVar1 = plVar13 + *(int *)(lVar5 + 0x398);
    for (; plVar13 != plVar1; plVar13 = plVar13 + 1) {
      if (((longlong *)*plVar13 != (longlong *)0x0) &&
         (lVar5 = (**(code **)(*(longlong *)*plVar13 + 1000))(), lVar5 != 0)) {
        lVar9 = U_142b4a5d0();
        if ((*(int *)(lVar9 + 0x38) <= *(int *)(*(longlong *)(lVar5 + 0x10) + 0x38)) &&
           (*(longlong *)
             (*(longlong *)(*(longlong *)(lVar5 + 0x10) + 0x30) +
             (longlong)*(int *)(lVar9 + 0x38) * 8) == lVar9 + 0x30)) {
          lVar9 = *(longlong *)(lVar5 + 0x30);
          lVar10 = U_141ed1800();
          (**(code **)(lVar9 + 0x18))(lVar5 + 0x30,*(undefined8 *)(lVar10 + 0xba0));
          uVar11 = extraout_var;
          if (extraout_AL != '\0') goto LAB_1423d8db2;
        }
      }
    }
  }
  U_1419b90a0(&piStack_68,param_1[0x49],lVar4,1);
  while( true ) {
    if (plStack_38 == (longlong *)0x0) {
      uVar6 = U_1419bb7e0(&piStack_68);
      return uVar6 & 0xffffffffffffff00;
    }
    uStackX_8 = uStackX_8 & 0xffffffffffffff00;
    cVar3 = (**(code **)(*plStack_38 + 0xb50))(plStack_38,&uStackX_8);
    uVar6 = uStackX_8;
    if (cVar3 != '\0') break;
    U_1419f3d90(&piStack_68,uStack_50._4_4_ + 1);
  }
  U_1419bb7e0(&piStack_68);
  return uVar6 & 0xff;
}

// AFortPlayerControllerGameplay_GetVehicleTrickInfo exec 0x142a651d0
// Out-of-line dependency 0x142a651d0; parameter ABI remains to be checked.
void U_142a651d0(longlong param_1,longlong param_2,undefined8 *param_3)

{
  undefined8 uVar1;
  undefined8 *puVar2;
  undefined8 *puVar3;
  longlong lVar4;
  undefined8 *puVar5;
  
  *(ulonglong *)(param_2 + 0x20) =
       (ulonglong)(*(longlong *)(param_2 + 0x20) != 0) + *(longlong *)(param_2 + 0x20);
  lVar4 = 2;
  puVar2 = (undefined8 *)(param_1 + 0x22f8);
  do {
    puVar5 = param_3;
    puVar3 = puVar2;
    uVar1 = puVar3[1];
    *puVar5 = *puVar3;
    puVar5[1] = uVar1;
    uVar1 = puVar3[3];
    puVar5[2] = puVar3[2];
    puVar5[3] = uVar1;
    uVar1 = puVar3[5];
    puVar5[4] = puVar3[4];
    puVar5[5] = uVar1;
    uVar1 = puVar3[7];
    puVar5[6] = puVar3[6];
    puVar5[7] = uVar1;
    uVar1 = puVar3[9];
    puVar5[8] = puVar3[8];
    puVar5[9] = uVar1;
    uVar1 = puVar3[0xb];
    puVar5[10] = puVar3[10];
    puVar5[0xb] = uVar1;
    uVar1 = puVar3[0xd];
    puVar5[0xc] = puVar3[0xc];
    puVar5[0xd] = uVar1;
    uVar1 = puVar3[0xf];
    puVar5[0xe] = puVar3[0xe];
    puVar5[0xf] = uVar1;
    lVar4 = lVar4 + -1;
    puVar2 = puVar3 + 0x10;
    param_3 = puVar5 + 0x10;
  } while (lVar4 != 0);
  uVar1 = puVar3[0x11];
  puVar5[0x10] = puVar3[0x10];
  puVar5[0x11] = uVar1;
  uVar1 = puVar3[0x13];
  puVar5[0x12] = puVar3[0x12];
  puVar5[0x13] = uVar1;
  uVar1 = puVar3[0x15];
  puVar5[0x14] = puVar3[0x14];
  puVar5[0x15] = uVar1;
  uVar1 = puVar3[0x17];
  puVar5[0x16] = puVar3[0x16];
  puVar5[0x17] = uVar1;
  uVar1 = puVar3[0x19];
  puVar5[0x18] = puVar3[0x18];
  puVar5[0x19] = uVar1;
  return;
}

// AFortPlayerControllerGameplay_GetSpectatingPlayerName exec 0x142a65150
// Out-of-line dependency 0x142a65150; parameter ABI remains to be checked.
void U_142a65150(longlong *param_1,longlong param_2,undefined8 *param_3)

{
  undefined8 *puVar1;
  undefined *apuStack_18 [2];
  
  *(ulonglong *)(param_2 + 0x20) =
       (ulonglong)(*(longlong *)(param_2 + 0x20) != 0) + *(longlong *)(param_2 + 0x20);
  puVar1 = (undefined8 *)(**(code **)(*param_1 + 0x1fd0))(param_1,apuStack_18);
  if (param_3 != puVar1) {
    if ((undefined *)*param_3 != (undefined *)0x0) {
      FMemory_Free((undefined *)*param_3);
    }
    *param_3 = *puVar1;
    *puVar1 = 0;
    *(undefined4 *)(param_3 + 1) = *(undefined4 *)(puVar1 + 1);
    *(undefined4 *)((longlong)param_3 + 0xc) = *(undefined4 *)((longlong)puVar1 + 0xc);
    puVar1[1] = 0;
  }
  if (apuStack_18[0] != (undefined *)0x0) {
    FMemory_Free(apuStack_18[0]);
  }
  return;
}

// AFortPlayerControllerGameplay_CanFollowTeammate exec 0x142a64880
// Out-of-line dependency 0x142a64880; parameter ABI remains to be checked.
void U_142a64880(longlong *param_1,longlong param_2,undefined1 *param_3)

{
  undefined1 uVar1;
  
  *(ulonglong *)(param_2 + 0x20) =
       (ulonglong)(*(longlong *)(param_2 + 0x20) != 0) + *(longlong *)(param_2 + 0x20);
  uVar1 = (**(code **)(*param_1 + 0x1fd8))();
  *param_3 = uVar1;
  return;
}

// AFortPlayerControllerGameplay_SetLastUsedCreativePropetyFilter exec 0x142a673b0
// Out-of-line dependency 0x142a673b0; parameter ABI remains to be checked.
void U_142a673b0(undefined8 param_1,FFrame *param_2)

{
  undefined *Property;
  undefined4 auStackX_10 [2];
  
  auStackX_10[0] = 0;
  if (*(longlong *)param_2->Code == 0) {
    Property = *(undefined **)param_2->PropertyChainForCompiledIn;
    *(undefined8 *)param_2->PropertyChainForCompiledIn = *(undefined8 *)(Property + 0x20);
    FFrame_StepExplicitProperty(param_2,(undefined *)auStackX_10,Property);
  }
  else {
    FFrame_Step(param_2,*(UObject **)param_2->Object,(undefined *)auStackX_10);
  }
  *(ulonglong *)param_2->Code =
       (ulonglong)(*(longlong *)param_2->Code != 0) + *(longlong *)param_2->Code;
  U_1423eb030(param_1,auStackX_10[0]);
  return;
}
// Out-of-line dependency 0x1423eb030; parameter ABI remains to be checked.
void U_1423eb030(longlong param_1,undefined4 param_2)

{
  *(undefined4 *)(param_1 + 0x25c4) = param_2;
  return;
}

// AFortPlayerControllerGameplay_GetSettingsFilteredPlayerName exec 0x142a65070
// Out-of-line dependency 0x142a65070; parameter ABI remains to be checked.
void U_142a65070(undefined8 param_1,FFrame *param_2,undefined8 *param_3)

{
  undefined *Property;
  undefined8 *puVar1;
  undefined8 uStackX_10;
  undefined *apuStack_18 [2];
  
  uStackX_10 = 0;
  if (*(longlong *)param_2->Code == 0) {
    Property = *(undefined **)param_2->PropertyChainForCompiledIn;
    *(undefined8 *)param_2->PropertyChainForCompiledIn = *(undefined8 *)(Property + 0x20);
    FFrame_StepExplicitProperty(param_2,(undefined *)&uStackX_10,Property);
  }
  else {
    FFrame_Step(param_2,*(UObject **)param_2->Object,(undefined *)&uStackX_10);
  }
  *(ulonglong *)param_2->Code =
       (ulonglong)(*(longlong *)param_2->Code != 0) + *(longlong *)param_2->Code;
  puVar1 = (undefined8 *)U_1423d5330(param_1,apuStack_18,uStackX_10);
  if (param_3 != puVar1) {
    if ((undefined *)*param_3 != (undefined *)0x0) {
      FMemory_Free((undefined *)*param_3);
    }
    *param_3 = *puVar1;
    *puVar1 = 0;
    *(undefined4 *)(param_3 + 1) = *(undefined4 *)(puVar1 + 1);
    *(undefined4 *)((longlong)param_3 + 0xc) = *(undefined4 *)((longlong)puVar1 + 0xc);
    puVar1[1] = 0;
  }
  if (apuStack_18[0] != (undefined *)0x0) {
    FMemory_Free(apuStack_18[0]);
  }
  return;
}
// Out-of-line dependency 0x1423d5330; parameter ABI remains to be checked.
undefined8 U_1423d5330(longlong *param_1,undefined8 param_2)

{
  (**(code **)(*param_1 + 0x1fc8))();
  return param_2;
}

// AFortPlayerControllerGameplay_SetHUDElementVisibility exec 0x142a67260
// Out-of-line dependency 0x142a67260; parameter ABI remains to be checked.
void U_142a67260(longlong *param_1,FFrame *param_2)

{
  undefined *puVar1;
  undefined **ppuVar2;
  bool bVar3;
  int aiStackX_8 [2];
  undefined8 uStackX_10;
  undefined *puStack_38;
  undefined8 uStack_30;
  undefined *puStack_28;
  undefined8 uStack_20;
  
  puStack_38 = (undefined *)0x0;
  uStack_30 = 0;
  puStack_28 = (undefined *)0x0;
  uStack_20 = 0;
  *(undefined8 *)param_2->MostRecentPropertyAddress = 0;
  if (*(longlong *)param_2->Code == 0) {
    puVar1 = *(undefined **)param_2->PropertyChainForCompiledIn;
    *(undefined8 *)param_2->PropertyChainForCompiledIn = *(undefined8 *)(puVar1 + 0x20);
    FFrame_StepExplicitProperty(param_2,(undefined *)&puStack_38,puVar1);
  }
  else {
    FFrame_Step(param_2,*(UObject **)param_2->Object,(undefined *)&puStack_38);
  }
  aiStackX_8[0] = 0;
  ppuVar2 = &puStack_38;
  if (*(undefined ***)param_2->MostRecentPropertyAddress != (undefined **)0x0) {
    ppuVar2 = *(undefined ***)param_2->MostRecentPropertyAddress;
  }
  if (*(longlong *)param_2->Code == 0) {
    puVar1 = *(undefined **)param_2->PropertyChainForCompiledIn;
    *(undefined8 *)param_2->PropertyChainForCompiledIn = *(undefined8 *)(puVar1 + 0x20);
    FFrame_StepExplicitProperty(param_2,(undefined *)aiStackX_8,puVar1);
  }
  else {
    FFrame_Step(param_2,*(UObject **)param_2->Object,(undefined *)aiStackX_8);
  }
  bVar3 = aiStackX_8[0] != 0;
  uStackX_10 = 0;
  if (*(longlong *)param_2->Code == 0) {
    puVar1 = *(undefined **)param_2->PropertyChainForCompiledIn;
    *(undefined8 *)param_2->PropertyChainForCompiledIn = *(undefined8 *)(puVar1 + 0x20);
    FFrame_StepExplicitProperty(param_2,(undefined *)&uStackX_10,puVar1);
  }
  else {
    FFrame_Step(param_2,*(UObject **)param_2->Object,(undefined *)&uStackX_10);
  }
  *(ulonglong *)param_2->Code =
       (ulonglong)(*(longlong *)param_2->Code != 0) + *(longlong *)param_2->Code;
  (**(code **)(*param_1 + 0x1f68))(param_1,ppuVar2,bVar3,uStackX_10);
  if (puStack_28 != (undefined *)0x0) {
    FMemory_Free(puStack_28);
  }
  if (puStack_38 != (undefined *)0x0) {
    FMemory_Free(puStack_38);
  }
  return;
}

// AFortPlayerControllerGameplay_Validation_IsFlyingPossible exec 0x142a674d0
// Out-of-line dependency 0x142a674d0; parameter ABI remains to be checked.
void U_142a674d0(undefined8 param_1,longlong param_2,undefined1 *param_3)

{
  undefined1 uVar1;
  
  *(ulonglong *)(param_2 + 0x20) =
       (ulonglong)(*(longlong *)(param_2 + 0x20) != 0) + *(longlong *)(param_2 + 0x20);
  uVar1 = U_1423f5980();
  *param_3 = uVar1;
  return;
}
// Out-of-line dependency 0x1423f5980; parameter ABI remains to be checked.
undefined1 U_1423f5980(longlong *param_1)

{
  longlong *plVar1;
  byte bVar2;
  char cVar3;
  longlong lVar4;
  undefined *puVar5;
  longlong lVar6;
  longlong lVar7;
  longlong lVar8;
  longlong *plVar9;
  byte abStackX_8 [8];
  undefined1 auStack_58 [28];
  int iStack_3c;
  longlong *plStack_28;
  
  lVar4 = (**(code **)(*param_1 + 0x150))();
  if ((lVar4 != 0) && (lVar4 = *(longlong *)(lVar4 + 0x130), lVar4 != 0)) {
    puVar5 = AFortGameStateAthena_StaticClass();
    if ((*(int *)(puVar5 + 0x38) <= *(int *)(*(longlong *)(lVar4 + 0x10) + 0x38)) &&
       (*(undefined **)
         (*(longlong *)(*(longlong *)(lVar4 + 0x10) + 0x30) + (longlong)*(int *)(puVar5 + 0x38) * 8)
        == puVar5 + 0x30)) {
      cVar3 = U_14199e430(lVar4);
      if (cVar3 != '\0') {
        return 1;
      }
      goto LAB_1423f59ed;
    }
  }
  lVar4 = 0;
LAB_1423f59ed:
  lVar6 = param_1[0x2e7];
  if (lVar6 != 0) {
    plVar9 = *(longlong **)(lVar6 + 0x390);
    plVar1 = plVar9 + *(int *)(lVar6 + 0x398);
    for (; plVar9 != plVar1; plVar9 = plVar9 + 1) {
      if (((longlong *)*plVar9 != (longlong *)0x0) &&
         (lVar6 = (**(code **)(*(longlong *)*plVar9 + 1000))(), lVar6 != 0)) {
        lVar7 = U_142b4a5d0();
        if ((*(int *)(lVar7 + 0x38) <= *(int *)(*(longlong *)(lVar6 + 0x10) + 0x38)) &&
           (*(longlong *)
             (*(longlong *)(*(longlong *)(lVar6 + 0x10) + 0x30) +
             (longlong)*(int *)(lVar7 + 0x38) * 8) == lVar7 + 0x30)) {
          lVar7 = *(longlong *)(lVar6 + 0x30);
          lVar8 = U_141ed1800();
          cVar3 = (**(code **)(lVar7 + 0x18))(lVar6 + 0x30,*(undefined8 *)(lVar8 + 0xba0));
          if (cVar3 != '\0') {
            return 1;
          }
        }
      }
    }
  }
  U_1419b90a0(auStack_58,param_1[0x49],lVar4,1);
  while( true ) {
    if (plStack_28 == (longlong *)0x0) {
      U_1419bb7e0(auStack_58);
      return 0;
    }
    abStackX_8[0] = 0;
    cVar3 = (**(code **)(*plStack_28 + 0xb50))(plStack_28,abStackX_8);
    bVar2 = abStackX_8[0];
    if (cVar3 != '\0') break;
    U_1419f3d90(auStack_58,iStack_3c + 1);
  }
  U_1419bb7e0(auStack_58);
  return bVar2;
}

// AFortPlayerControllerGameplay_ShowQuicktimeEventWidget exec 0x142a67430
// Out-of-line dependency 0x142a67430; parameter ABI remains to be checked.
void U_142a67430(longlong *param_1,longlong param_2)

{
  *(ulonglong *)(param_2 + 0x20) =
       (ulonglong)(*(longlong *)(param_2 + 0x20) != 0) + *(longlong *)(param_2 + 0x20);
                    /* WARNING: Could not recover jumptable at 0x000142a67448. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(*param_1 + 0x1ff0))();
  return;
}

// AFortPlayerControllerGameplay_ServerCreativeStopGhost exec 0x142a66c70
// Out-of-line dependency 0x142a66c70; parameter ABI remains to be checked.
void U_142a66c70(longlong *param_1,longlong param_2)

{
  char cVar1;
  
  *(ulonglong *)(param_2 + 0x20) =
       (ulonglong)(*(longlong *)(param_2 + 0x20) != 0) + *(longlong *)(param_2 + 0x20);
  cVar1 = (**(code **)(*param_1 + 0x1ec0))();
  if (cVar1 == '\0') {
    RPC_ValidateFailed((ushort *)L"ServerCreativeStopGhost_Validate");
    return;
  }
                    /* WARNING: Could not recover jumptable at 0x000142a66cb7. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(*param_1 + 0x1ec8))(param_1);
  return;
}

// AFortPlayerControllerGameplay_ServerCreativeStopFlyUp exec 0x142a66c20
// Out-of-line dependency 0x142a66c20; parameter ABI remains to be checked.
void U_142a66c20(longlong *param_1,longlong param_2)

{
  char cVar1;
  
  *(ulonglong *)(param_2 + 0x20) =
       (ulonglong)(*(longlong *)(param_2 + 0x20) != 0) + *(longlong *)(param_2 + 0x20);
  cVar1 = (**(code **)(*param_1 + 0x1f00))();
  if (cVar1 == '\0') {
    RPC_ValidateFailed((ushort *)L"ServerCreativeStopFlyUp_Validate");
    return;
  }
                    /* WARNING: Could not recover jumptable at 0x000142a66c67. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(*param_1 + 0x1f08))(param_1);
  return;
}

// AFortPlayerControllerGameplay_ServerCreativeStopFlyDown exec 0x142a66bd0
// Out-of-line dependency 0x142a66bd0; parameter ABI remains to be checked.
void U_142a66bd0(longlong *param_1,longlong param_2)

{
  char cVar1;
  
  *(ulonglong *)(param_2 + 0x20) =
       (ulonglong)(*(longlong *)(param_2 + 0x20) != 0) + *(longlong *)(param_2 + 0x20);
  cVar1 = (**(code **)(*param_1 + 0x1ee0))();
  if (cVar1 == '\0') {
    RPC_ValidateFailed((ushort *)L"ServerCreativeStopFlyDown_Validate");
    return;
  }
                    /* WARNING: Could not recover jumptable at 0x000142a66c17. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(*param_1 + 0x1ee8))(param_1);
  return;
}

// AFortPlayerControllerGameplay_ServerCreativeStopFly exec 0x142a66b80
// Out-of-line dependency 0x142a66b80; parameter ABI remains to be checked.
void U_142a66b80(longlong *param_1,longlong param_2)

{
  char cVar1;
  
  *(ulonglong *)(param_2 + 0x20) =
       (ulonglong)(*(longlong *)(param_2 + 0x20) != 0) + *(longlong *)(param_2 + 0x20);
  cVar1 = (**(code **)(*param_1 + 0x1f28))();
  if (cVar1 == '\0') {
    RPC_ValidateFailed((ushort *)L"ServerCreativeStopFly_Validate");
    return;
  }
                    /* WARNING: Could not recover jumptable at 0x000142a66bc7. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(*param_1 + 0x1f30))(param_1);
  return;
}

// AFortPlayerControllerGameplay_ServerCreativeStartFlyUp exec 0x142a66b30
// Out-of-line dependency 0x142a66b30; parameter ABI remains to be checked.
void U_142a66b30(longlong *param_1,longlong param_2)

{
  char cVar1;
  
  *(ulonglong *)(param_2 + 0x20) =
       (ulonglong)(*(longlong *)(param_2 + 0x20) != 0) + *(longlong *)(param_2 + 0x20);
  cVar1 = (**(code **)(*param_1 + 0x1f10))();
  if (cVar1 == '\0') {
    RPC_ValidateFailed((ushort *)L"ServerCreativeStartFlyUp_Validate");
    return;
  }
                    /* WARNING: Could not recover jumptable at 0x000142a66b77. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(*param_1 + 0x1f18))(param_1);
  return;
}

// AFortPlayerControllerGameplay_ServerCreativeStartFlyDown exec 0x142a66ae0
// Out-of-line dependency 0x142a66ae0; parameter ABI remains to be checked.
void U_142a66ae0(longlong *param_1,longlong param_2)

{
  char cVar1;
  
  *(ulonglong *)(param_2 + 0x20) =
       (ulonglong)(*(longlong *)(param_2 + 0x20) != 0) + *(longlong *)(param_2 + 0x20);
  cVar1 = (**(code **)(*param_1 + 0x1ef0))();
  if (cVar1 == '\0') {
    RPC_ValidateFailed((ushort *)L"ServerCreativeStartFlyDown_Validate");
    return;
  }
                    /* WARNING: Could not recover jumptable at 0x000142a66b27. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(*param_1 + 0x1ef8))(param_1);
  return;
}

// AFortPlayerControllerGameplay_RefreshHUDElementVisibilitiesToSettings exec 0x142a66640
// Out-of-line dependency 0x142a66640; parameter ABI remains to be checked.
void U_142a66640(longlong *param_1,longlong param_2)

{
  *(ulonglong *)(param_2 + 0x20) =
       (ulonglong)(*(longlong *)(param_2 + 0x20) != 0) + *(longlong *)(param_2 + 0x20);
                    /* WARNING: Could not recover jumptable at 0x000142a66658. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(*param_1 + 0x1f60))();
  return;
}

// AFortPlayerControllerGameplay_PushCreativeReadyCheckInputMode exec 0x142a663e0
// Out-of-line dependency 0x142a663e0; parameter ABI remains to be checked.
void U_142a663e0(longlong *param_1,longlong param_2)

{
  int iVar1;
  
  *(ulonglong *)(param_2 + 0x20) =
       (ulonglong)(*(longlong *)(param_2 + 0x20) != 0) + *(longlong *)(param_2 + 0x20);
  iVar1 = U_143f7a920();
  if ((iVar1 == 1) || (param_1[0x4b4] == 0)) {
    return;
  }
                    /* WARNING: Could not recover jumptable at 0x0001423e381a. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(*param_1 + 0xbd8))(param_1);
  return;
}
// Out-of-line dependency 0x143f7a920; parameter ABI remains to be checked.
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined8 U_143f7a920(longlong *param_1)

{
  bool bVar1;
  longlong lVar2;
  longlong lVar3;
  undefined8 uVar4;
  
  lVar2 = (**(code **)(*param_1 + 0x150))();
  lVar3 = param_1[0x1d];
  bVar1 = FName_EqualsEName((int)lVar3,0x11a);
  if ((int)((ulonglong)lVar3 >> 0x20) == 0 && bVar1) {
    if (lVar2 == 0) {
      return 0;
    }
    lVar3 = *(longlong *)(lVar2 + 0x38);
  }
  else {
    lVar3 = U_14463f3c0(_DAT_148351eb8,lVar2,lVar3);
  }
  if ((lVar3 == 0) && ((lVar2 == 0 || (lVar3 = *(longlong *)(lVar2 + 0xf0), lVar3 == 0)))) {
    return 0;
  }
  uVar4 = U_1443771a0(lVar3);
  return uVar4;
}

// AFortPlayerControllerGameplay_PopCreativeReadyCheckInputMode exec 0x142a663c0
// Out-of-line dependency 0x142a663c0; parameter ABI remains to be checked.
void U_142a663c0(longlong *param_1,longlong param_2)

{
  int iVar1;
  
  *(ulonglong *)(param_2 + 0x20) =
       (ulonglong)(*(longlong *)(param_2 + 0x20) != 0) + *(longlong *)(param_2 + 0x20);
  iVar1 = U_143f7a920();
  if ((iVar1 == 1) || (param_1[0x4b4] == 0)) {
    return;
  }
                    /* WARNING: Could not recover jumptable at 0x0001423e1cba. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(*param_1 + 0xbe0))(param_1);
  return;
}
// Out-of-line dependency 0x143f7a920; parameter ABI remains to be checked.
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined8 U_143f7a920(longlong *param_1)

{
  bool bVar1;
  longlong lVar2;
  longlong lVar3;
  undefined8 uVar4;
  
  lVar2 = (**(code **)(*param_1 + 0x150))();
  lVar3 = param_1[0x1d];
  bVar1 = FName_EqualsEName((int)lVar3,0x11a);
  if ((int)((ulonglong)lVar3 >> 0x20) == 0 && bVar1) {
    if (lVar2 == 0) {
      return 0;
    }
    lVar3 = *(longlong *)(lVar2 + 0x38);
  }
  else {
    lVar3 = U_14463f3c0(_DAT_148351eb8,lVar2,lVar3);
  }
  if ((lVar3 == 0) && ((lVar2 == 0 || (lVar3 = *(longlong *)(lVar2 + 0xf0), lVar3 == 0)))) {
    return 0;
  }
  uVar4 = U_1443771a0(lVar3);
  return uVar4;
}

// AFortPlayerControllerGameplay_InitializeAmbientAudioController exec 0x142a65540
// Out-of-line dependency 0x142a65540; parameter ABI remains to be checked.
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void U_142a65540(longlong *param_1,longlong param_2)

{
  char cVar1;
  bool bVar2;
  longlong lVar3;
  longlong lVar4;
  undefined *puVar5;
  longlong lVar6;
  longlong *plVar7;
  undefined8 uVar8;
  longlong *plVar9;
  undefined1 auStack_f8 [32];
  uint uStack_d8;
  undefined8 uStack_d0;
  undefined1 uStack_c8;
  undefined8 uStack_c0;
  undefined1 uStack_b8;
  undefined8 uStack_a8;
  undefined8 uStack_a0;
  undefined4 uStack_98;
  undefined4 uStack_94;
  longlong *plStack_88;
  int iStack_80;
  undefined1 auStack_78 [8];
  undefined8 uStack_70;
  longlong lStack_68;
  longlong *plStack_58;
  longlong alStack_48 [4];
  ulonglong uStack_28;
  
  *(ulonglong *)(param_2 + 0x20) =
       (ulonglong)(*(longlong *)(param_2 + 0x20) != 0) + *(longlong *)(param_2 + 0x20);
  uStack_28 = _DAT_1480c4288 ^ (ulonglong)auStack_f8;
  lVar3 = (**(code **)(*param_1 + 0x150))();
  if (lVar3 == 0) goto LAB_1423d87c8;
  plVar7 = *(longlong **)(lVar3 + 0x130);
  if (plVar7 != (longlong *)0x0) {
    lVar4 = U_142908ec0();
    if ((*(int *)(lVar4 + 0x38) <= *(int *)(plVar7[2] + 0x38)) &&
       (*(longlong *)(*(longlong *)(plVar7[2] + 0x30) + (longlong)*(int *)(lVar4 + 0x38) * 8) ==
        lVar4 + 0x30)) {
      puVar5 = AFortGameStateAthena_StaticClass();
      if ((*(int *)(plVar7[2] + 0x38) < *(int *)(puVar5 + 0x38)) ||
         (*(undefined **)(*(longlong *)(plVar7[2] + 0x30) + (longlong)*(int *)(puVar5 + 0x38) * 8)
          != puVar5 + 0x30)) {
        if (*(longlong *)(lVar3 + 0x188) == 0) {
          puVar5 = *(undefined **)(lVar3 + 800);
        }
        else {
          puVar5 = *(undefined **)(*(longlong *)(lVar3 + 0x188) + 0xc0);
        }
        lVar3 = U_144621fb0(puVar5,param_1 + 0x497);
        if (lVar3 != 0) {
          FTimerManager_InternalClearTimer(puVar5,param_1[0x497]);
        }
        param_1[0x497] = 0;
        goto LAB_1423d87c8;
      }
      if (((plVar7[0x61] != 0) && (cVar1 = U_141a4b280(), cVar1 != '\0')) &&
         (lVar4 = param_1[0x44], lVar4 != 0)) {
        lVar6 = U_142a85850();
        if ((((*(int *)(lVar6 + 0x38) <= *(int *)(*(longlong *)(lVar4 + 0x10) + 0x38)) &&
             (*(longlong *)
               (*(longlong *)(*(longlong *)(lVar4 + 0x10) + 0x30) +
               (longlong)*(int *)(lVar6 + 0x38) * 8) == lVar6 + 0x30)) &&
            (lVar4 = (**(code **)(*(longlong *)(lVar4 + 800) + 0x10))(), lVar4 != 0)) &&
           (lVar4 = U_141f611f0(), lVar4 != 0)) {
          if (*(longlong *)(lVar3 + 0x188) == 0) {
            puVar5 = *(undefined **)(lVar3 + 800);
          }
          else {
            puVar5 = *(undefined **)(*(longlong *)(lVar3 + 0x188) + 0xc0);
          }
          lVar3 = U_144621fb0(puVar5,param_1 + 0x497);
          if (lVar3 != 0) {
            FTimerManager_InternalClearTimer(puVar5,param_1[0x497]);
          }
          param_1[0x497] = 0;
          cVar1 = U_14199e3c0(plVar7);
          if (cVar1 != '\0') {
            U_142ec0440(param_1,
                          L"NewObject with empty name can\'t be used to create default subobjects (inside of UObject derived class constructor) as it produces inconsistent object names. Use ObjectInitializer.CreateDefaultSuobject<> instead."
                         );
            uStack_a8 = 0;
            uVar8 = U_1427ac710();
            uStack_b8 = 0;
            uStack_c0 = 0;
            uStack_c8 = 0;
            uStack_d0 = 0;
            uStack_d8 = 0;
            lVar3 = U_142ed7360(uVar8,param_1,0,0);
            param_1[0x417] = lVar3;
            U_1424774d0(lVar3);
            U_1440c2bb0(param_1[0x417]);
            goto LAB_1423d87c8;
          }
          uStack_a8 = 0;
          plVar7 = (longlong *)(**(code **)(*plVar7 + 0x768))(plVar7);
          if ((*plVar7 == 0) || (lVar3 = U_1427ac710(), lVar3 == 0)) {
LAB_1423d8556:
            lVar4 = 0;
          }
          else {
            lVar4 = *plVar7;
            if ((*(int *)(lVar4 + 0x38) < *(int *)(lVar3 + 0x38)) ||
               (*(longlong *)(*(longlong *)(lVar4 + 0x30) + (longlong)*(int *)(lVar3 + 0x38) * 8) !=
                lVar3 + 0x30)) goto LAB_1423d8556;
          }
          bVar2 = FName_EqualsEName(0,0);
          if (bVar2) {
            U_142ec0440(param_1,
                          L"NewObject with empty name can\'t be used to create default subobjects (inside of UObject derived class constructor) as it produces inconsistent object names. Use ObjectInitializer.CreateDefaultSuobject<> instead."
                         );
          }
          uStack_b8 = 0;
          uStack_c0 = 0;
          uStack_c8 = 0;
          uStack_d0 = 0;
          uStack_d8 = 0;
          lVar3 = U_142ed7360(lVar4,param_1,uStack_a8,0);
          param_1[0x417] = lVar3;
          U_1440c2bb0(lVar3);
          goto LAB_1423d87c8;
        }
      }
    }
  }
  uStack_a0 = U_1423d8390;
  uStack_98 = 0;
  if (*(longlong *)(lVar3 + 0x188) == 0) {
    uVar8 = *(undefined8 *)(lVar3 + 800);
  }
  else {
    uVar8 = *(undefined8 *)(*(longlong *)(lVar3 + 0x188) + 0xc0);
  }
  plVar7 = (longlong *)FMemory_Realloc((undefined *)0x0,0x30,0);
  if (plVar7 != (longlong *)0x0) {
    *plVar7 = (longlong)&DAT_1459a9e80;
    U_142ee3b30(plVar7 + 1,param_1);
    *(undefined4 *)(plVar7 + 2) = (undefined4)uStack_a0;
    *(undefined4 *)((longlong)plVar7 + 0x14) = uStack_a0._4_4_;
    *(undefined4 *)(plVar7 + 3) = uStack_98;
    *(undefined4 *)((longlong)plVar7 + 0x1c) = uStack_94;
    lVar3 = U_142bad740();
    plVar7[5] = lVar3;
    *plVar7 = (longlong)&DAT_1459a9ed8;
  }
  plStack_88 = (longlong *)0x0;
  iStack_80 = 0;
  if (plVar7 != (longlong *)0x0) {
    (**(code **)(*plVar7 + 0x40))(plVar7,&plStack_88);
  }
  U_142ee3b30(auStack_78,0);
  uStack_d0 = CONCAT44(uStack_d0._4_4_,0xbf800000);
  uStack_d8 = uStack_d8 & 0xffffff00;
  uStack_70 = 0;
  lStack_68 = 0;
  plStack_58 = (longlong *)0x0;
  U_1446278d0(uVar8,param_1 + 0x497,&plStack_88,0x3f800000);
  if (lStack_68 != 0) {
    plVar9 = alStack_48;
    if (plStack_58 != (longlong *)0x0) {
      plVar9 = plStack_58;
    }
    (**(code **)(*plVar9 + 0x10))();
  }
  if (iStack_80 == 0) {
LAB_1423d8781:
    if (plStack_88 != (longlong *)0x0) {
      FMemory_Free((undefined *)plStack_88);
    }
  }
  else if (plStack_88 != (longlong *)0x0) {
    (**(code **)(*plStack_88 + 0x38))();
    if (plStack_88 != (longlong *)0x0) {
      plStack_88 = (longlong *)FMemory_Realloc((undefined *)plStack_88,0,0);
    }
    iStack_80 = 0;
    goto LAB_1423d8781;
  }
  if (plVar7 != (longlong *)0x0) {
    (**(code **)(*plVar7 + 0x38))(plVar7);
    puVar5 = FMemory_Realloc((undefined *)plVar7,0,0);
    if (puVar5 != (undefined *)0x0) {
      FMemory_Free(puVar5);
    }
  }
LAB_1423d87c8:
  U_145803c70(uStack_28 ^ (ulonglong)auStack_f8);
  return;
}
// Out-of-line dependency 0x142908ec0; parameter ABI remains to be checked.
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

longlong U_142908ec0(void)

{
  if (_DAT_148198380 == 0) {
    GetPrivateStaticClassBody
              ((ushort *)L"/Script/FortniteGame",(ushort *)L"FortGameState",&DAT_148198380,
               U_14290b040,0x6b8,8,0x10000004,0,(ushort *)L"Game",&LAB_1429099c0,U_140680300,
               U_143f68ab0,&LAB_14290afc0,&LAB_1406966e0,false);
  }
  return _DAT_148198380;
}
// Out-of-line dependency 0x144621fb0; parameter ABI remains to be checked.
longlong U_144621fb0(longlong param_1,longlong *param_2)

{
  longlong lVar1;
  longlong lVar2;
  uint uVar3;
  
  lVar1 = *param_2;
  if (lVar1 != 0) {
    uVar3 = (uint)lVar1 & 0xffffff;
    if ((int)uVar3 < *(int *)(param_1 + 0x10)) {
      lVar2 = param_1 + 0x18;
      if (*(longlong *)(param_1 + 0x28) != 0) {
        lVar2 = *(longlong *)(param_1 + 0x28);
      }
      if ((((*(uint *)(lVar2 + (ulonglong)(uVar3 >> 5) * 4) >> ((uint)lVar1 & 0x1f) & 1) != 0) &&
          (lVar2 = (ulonglong)uVar3 * 0x90 + *(longlong *)(param_1 + 8),
          *(longlong *)(lVar2 + 0x70) == lVar1)) && (*(char *)(lVar2 + 1) != '\x04')) {
        return lVar2;
      }
    }
  }
  return 0;
}
// Out-of-line dependency 0x141a4b280; parameter ABI remains to be checked.
longlong U_141a4b280(longlong param_1)

{
  int iVar1;
  uint7 uVar2;
  
  iVar1 = *(int *)(param_1 + 0x2c0);
  uVar2 = (uint7)(uint3)((uint)iVar1 >> 8);
  if ((iVar1 < 0) || (*(int *)(param_1 + 0x2b8) != iVar1)) {
    return (ulonglong)uVar2 << 8;
  }
  return CONCAT71(uVar2,1);
}
// Out-of-line dependency 0x142a85850; parameter ABI remains to be checked.
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

longlong U_142a85850(void)

{
  if (_DAT_1481b6550 == 0) {
    GetPrivateStaticClassBody
              ((ushort *)L"/Script/FortniteGame",(ushort *)L"FortPlayerState",&DAT_1481b6550,
               U_142a86600,0x9b8,8,0x10000004,0,(ushort *)L"Engine",&LAB_142a86030,U_140680300,
               U_143f68ab0,&LAB_1407846a0,&LAB_1406966e0,false);
  }
  return _DAT_1481b6550;
}
// Out-of-line dependency 0x141f611f0; parameter ABI remains to be checked.
longlong U_141f611f0(longlong param_1)

{
  longlong lVar1;
  longlong lVar2;
  longlong lVar3;
  undefined8 uVar4;
  
  lVar1 = U_141ef3ab0();
  if (lVar1 == 0) {
    return 0;
  }
  lVar2 = U_1429234b0();
  if (*(int *)(*(longlong *)(lVar1 + 0x10) + 0x38) < *(int *)(lVar2 + 0x38)) {
    return 0;
  }
  if (*(longlong *)
       (*(longlong *)(*(longlong *)(lVar1 + 0x10) + 0x30) + (longlong)*(int *)(lVar2 + 0x38) * 8) !=
      lVar2 + 0x30) {
    return 0;
  }
  lVar2 = U_141c74820(param_1);
  if (lVar2 != 0) {
    uVar4 = 0;
    if (param_1 != 0) {
      lVar3 = U_1446acb00();
      if ((*(int *)(*(longlong *)(param_1 + 0x10) + 0x38) < *(int *)(lVar3 + 0x38)) ||
         (*(longlong *)
           (*(longlong *)(*(longlong *)(param_1 + 0x10) + 0x30) +
           (longlong)*(int *)(lVar3 + 0x38) * 8) != lVar3 + 0x30)) {
        lVar3 = U_142a85850();
        if ((*(int *)(lVar3 + 0x38) <= *(int *)(*(longlong *)(param_1 + 0x10) + 0x38)) &&
           (*(longlong *)
             (*(longlong *)(*(longlong *)(param_1 + 0x10) + 0x30) +
             (longlong)*(int *)(lVar3 + 0x38) * 8) == lVar3 + 0x30)) {
          uVar4 = U_141c7d420(lVar2,param_1);
        }
      }
      else {
        uVar4 = U_141c7c5c0(lVar2,param_1);
      }
    }
    lVar2 = U_141f9f270(uVar4);
    if (lVar2 != 0) {
      return lVar2;
    }
  }
  return *(longlong *)(lVar1 + 0x2d0);
}
// Out-of-line dependency 0x14199e3c0; parameter ABI remains to be checked.
void U_14199e3c0(longlong param_1)

{
  longlong lVar1;
  
  lVar1 = *(longlong *)(param_1 + 0x1df8);
  if (lVar1 == 0) {
    lVar1 = *(longlong *)(param_1 + 0x1df0);
  }
  U_141a4a6f0(lVar1);
  return;
}
// Out-of-line dependency 0x142ec0440; parameter ABI remains to be checked.
void U_142ec0440(longlong param_1)

{
  longlong lVar1;
  undefined1 auStackX_18 [16];
  code *pcStack_18;
  undefined1 *puStack_10;
  
  puStack_10 = auStackX_18;
  auStackX_18[0] = 0;
  pcStack_18 = U_14161f8b0;
  lVar1 = U_142beeca0(&pcStack_18,&DAT_1478c18b8);
  if ((*(int *)(lVar1 + 0x34) != 0) && (param_1 == *(longlong *)(lVar1 + 0x38))) {
    FMsg_Logf_Internal(&DAT_1459a4bc8,0xc73,&DAT_148263594,1,(ushort *)&DAT_1459a4c8c);
    U_142c7b670();
  }
  return;
}
// Out-of-line dependency 0x1427ac710; parameter ABI remains to be checked.
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

longlong U_1427ac710(void)

{
  if (_DAT_14817ea30 == 0) {
    GetPrivateStaticClassBody
              ((ushort *)L"/Script/FortniteGame",(ushort *)L"FortAmbientAudioController",
               &DAT_14817ea30,U_1427adc40,0x208,8,0x10000004,0,(ushort *)L"Engine",&LAB_1427ad0c0,
               U_140680300,Stub_Empty,&LAB_1407846b0,&LAB_1406966e0,false);
  }
  return _DAT_14817ea30;
}
// Out-of-line dependency 0x142ed7360; parameter ABI remains to be checked.
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void U_142ed7360(code *param_1,undefined8 param_2,undefined8 param_3,uint param_4,
                  undefined4 param_5,longlong param_6,undefined1 param_7,undefined8 param_8,
                  char param_9)

{
  int OldNum;
  longlong *plVar1;
  bool bVar2;
  char cVar3;
  longlong lVar4;
  longlong *plVar5;
  code *pcVar6;
  int iStackX_1c;
  undefined1 auStack_218 [32];
  uint uStack_1f8;
  undefined1 uStack_1f0;
  char *pcStack_1e8;
  char acStack_1d8 [8];
  wchar_t *pwStack_1d0;
  code *pcStack_1c8;
  char *pcStack_1c0;
  code *pcStack_1b8;
  char *pcStack_1b0;
  longlong lStack_1a8;
  longlong lStack_1a0;
  undefined1 uStack_198;
  undefined2 uStack_197;
  undefined8 uStack_190;
  undefined *puStack_108;
  undefined4 uStack_100;
  undefined4 uStack_fc;
  undefined *puStack_78;
  undefined4 uStack_70;
  undefined4 uStack_6c;
  undefined8 uStack_68;
  byte bStack_60;
  ulonglong uStack_58;
  
  uStack_58 = _DAT_1480c4288 ^ (ulonglong)auStack_218;
  pwStack_1d0 = L"STAT_ConstructObject";
  if (DAT_1482472e4 != '\0') {
    pwStack_1d0 = (wchar_t *)0x0;
  }
  if ((*(uint *)(param_1 + 0xe4) & 0x10000080) == 0) {
LAB_142ed7455:
    uStack_1f0 = 0;
  }
  else {
    if (param_6 != 0) {
      bVar2 = FName_EqualsEName((int)param_3,0);
      iStackX_1c = (int)((ulonglong)param_3 >> 0x20);
      if (bVar2 && iStackX_1c == 0) goto LAB_142ed7455;
      if (param_9 == '\0') {
        uStack_1f8 = uStack_1f8 & 0xffffff00;
        lVar4 = U_142ec88e0(param_1,param_2,param_3,param_4);
        if (param_6 != lVar4) goto LAB_142ed7455;
      }
    }
    if ((~(byte)(param_4 >> 0x12) & 1) == 0) {
      pcStack_1b0 = acStack_1d8;
      acStack_1d8[0] = '\0';
      pcStack_1b8 = U_14161f8b0;
      lVar4 = U_142beeca0(&pcStack_1b8,&DAT_1478c18b8);
      if (*(int *)(lVar4 + 0x34) != 0) goto LAB_142ed7455;
    }
    uStack_1f0 = 1;
  }
  pcStack_1e8 = acStack_1d8;
  acStack_1d8[0] = '\0';
  uStack_1f8 = param_5;
  lVar4 = U_142ed6a60(param_1,param_2,param_3,param_4);
  if (acStack_1d8[0] == '\0') {
    pcVar6 = param_1;
    if (DAT_1482472e4 != '\0') {
      pcVar6 = (code *)0x0;
    }
    bStack_60 = bStack_60 & 0xfe;
    uStack_190 = param_8;
    pcStack_1c0 = acStack_1d8;
    uStack_198 = param_7;
    puStack_108 = (undefined *)0x0;
    uStack_100 = 0;
    puStack_78 = (undefined *)0x0;
    uStack_70 = 0;
    acStack_1d8[0] = '\0';
    lStack_1a0 = param_6;
    uStack_197 = 0x101;
    uStack_fc = 8;
    uStack_6c = 8;
    pcStack_1c8 = U_14161f8b0;
    pcStack_1b8 = pcVar6;
    lStack_1a8 = lVar4;
    lVar4 = U_142beeca0(&pcStack_1c8,&DAT_1478c18b8);
    uStack_68 = *(undefined8 *)(lVar4 + 0x38);
    *(int *)(lVar4 + 0x34) = *(int *)(lVar4 + 0x34) + 1;
    OldNum = *(int *)(lVar4 + 0x18);
    *(longlong *)(lVar4 + 0x38) = lStack_1a8;
    *(int *)(lVar4 + 0x18) = OldNum + 1;
    if (*(int *)(lVar4 + 0x1c) < OldNum + 1) {
      TArray8_ResizeGrow((undefined *)(lVar4 + 0x10),OldNum);
    }
    *(longlong **)(*(longlong *)(lVar4 + 0x10) + (longlong)OldNum * 8) = &lStack_1a8;
    if (lStack_1a8 != 0) {
      (**(code **)(**(longlong **)(lStack_1a8 + 0x10) + 800))
                (*(longlong **)(lStack_1a8 + 0x10),&lStack_1a8);
    }
    (**(code **)(param_1 + 200))(&lStack_1a8);
    if ((bStack_60 & 1) == 0) {
      pcStack_1c0 = acStack_1d8;
      acStack_1d8[0] = '\0';
      pcStack_1c8 = U_14161f8b0;
      lVar4 = U_142beeca0(&pcStack_1c8,&DAT_1478c18b8);
      *(int *)(lVar4 + 0x18) = *(int *)(lVar4 + 0x18) + -1;
      *(int *)(lVar4 + 0x34) = *(int *)(lVar4 + 0x34) + -1;
      *(undefined8 *)(lVar4 + 0x38) = uStack_68;
LAB_142ed75d6:
      plVar1 = *(longlong **)(lStack_1a8 + 0x10);
      plVar5 = (longlong *)U_142eeab70();
      if (((plVar1 != plVar5) && (lStack_1a0 == 0)) && (plVar1[2] != 0)) {
        lVar4 = plVar1[0x26];
        if (lVar4 == 0) {
          (**(code **)(*plVar1 + 0x390))(plVar1);
          lVar4 = plVar1[0x26];
        }
        lStack_1a0 = lVar4;
      }
      cVar3 = U_142d2fde0();
      if (cVar3 == '\0') {
        lVar4 = U_142d21740(&lStack_1a8);
        if (lVar4 != 0) {
          *(byte *)(lVar4 + 0x148) = *(byte *)(lVar4 + 0x148) | 1;
          goto LAB_142ed7642;
        }
      }
      U_142ecec30(&lStack_1a8);
    }
    else if (lStack_1a8 != 0) goto LAB_142ed75d6;
LAB_142ed7642:
    if (puStack_78 != (undefined *)0x0) {
      FMemory_Free(puStack_78);
    }
    if (puStack_108 != (undefined *)0x0) {
      FMemory_Free(puStack_108);
    }
    if (DAT_1482472e4 == '\0') goto LAB_142ed7694;
    if (pcVar6 != (code *)0x0) {
      U_142ed3b30(&pcStack_1b8);
    }
  }
  if ((DAT_1482472e4 != '\0') && (pwStack_1d0 != (wchar_t *)0x0)) {
    U_142cd3770(&pwStack_1d0);
  }
LAB_142ed7694:
  U_145803c70(uStack_58 ^ (ulonglong)auStack_218);
  return;
}
// Out-of-line dependency 0x1424774d0; parameter ABI remains to be checked.
void U_1424774d0(longlong param_1)

{
  *(int *)(param_1 + 0x204) = *(int *)(param_1 + 0x204) + 1;
  return;
}
// Out-of-line dependency 0x1440c2bb0; parameter ABI remains to be checked.
void U_1440c2bb0(longlong param_1)

{
  longlong lVar1;
  
  if (*(longlong **)(param_1 + 0xa0) != (longlong *)0x0) {
    lVar1 = (**(code **)(**(longlong **)(param_1 + 0xa0) + 0x150))();
    if (lVar1 != 0) {
      U_1440c2ce0(param_1,lVar1);
      return;
    }
  }
  return;
}
// Out-of-line dependency 0x142ee3b30; parameter ABI remains to be checked.
void U_142ee3b30(undefined4 *param_1,longlong param_2)

{
  undefined4 uVar1;
  
  if (param_2 == 0) {
    *param_1 = 0xffffffff;
    param_1[1] = 0;
    return;
  }
  *param_1 = *(undefined4 *)(param_2 + 0xc);
  uVar1 = U_142ec00d0(&DAT_148263810);
  param_1[1] = uVar1;
  return;
}
// Out-of-line dependency 0x142bad740; parameter ABI remains to be checked.
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

longlong U_142bad740(void)

{
  longlong lVar1;
  longlong lVar2;
  
  LOCK();
  UNLOCK();
  lVar2 = _DAT_147c306f0 + 1;
  lVar1 = _DAT_147c306f0 + 1;
  if (lVar2 == 0) {
    LOCK();
    UNLOCK();
    lVar2 = _DAT_147c306f0 + 2;
    lVar1 = _DAT_147c306f0 + 2;
  }
  _DAT_147c306f0 = lVar1;
  return lVar2;
}
// Out-of-line dependency 0x1446278d0; parameter ABI remains to be checked.
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void U_1446278d0(undefined *param_1,ulonglong *param_2,longlong **param_3,float param_4,
                  byte param_5,float param_6)

{
  uint *puVar1;
  uint *puVar2;
  double *pdVar3;
  double dVar4;
  undefined8 uVar5;
  longlong *plVar6;
  longlong *plVar7;
  longlong *plVar8;
  longlong *plVar9;
  longlong *plVar10;
  longlong *plVar11;
  longlong *plVar12;
  char cVar13;
  bool bVar14;
  UObject *Object;
  undefined *puVar15;
  longlong lVar16;
  undefined1 *puVar17;
  ulonglong *puVar18;
  byte bVar19;
  int iVar20;
  longlong lVar21;
  longlong *plVar22;
  longlong **pplVar23;
  ulonglong uVar24;
  uint uVar25;
  undefined1 auStack_138 [32];
  ulonglong uStack_118;
  wchar_t *pwStack_110;
  undefined1 auStack_108 [16];
  byte abStack_f8 [4];
  float fStack_f4;
  double dStack_f0;
  longlong *plStack_e8;
  int iStack_e0;
  longlong *plStack_d8;
  FName FStack_d0;
  longlong *plStack_c8;
  longlong *plStack_c0;
  longlong *plStack_b8;
  longlong *plStack_b0;
  undefined4 uStack_a8;
  undefined4 uStack_a4;
  undefined4 uStack_a0;
  undefined4 uStack_9c;
  longlong *plStack_98;
  longlong *plStack_90;
  undefined8 uStack_88;
  undefined1 uStack_78;
  ulonglong uStack_68;
  
  uStack_68 = _DAT_1480c4288 ^ (ulonglong)auStack_138;
  uVar24 = *param_2;
  pwStack_110 = L"STAT_SetTimer";
  if (DAT_1482472e4 != '\0') {
    pwStack_110 = (wchar_t *)0x0;
  }
  if (uVar24 != 0) {
    uVar25 = (uint)uVar24 & 0xffffff;
    if ((int)uVar25 < *(int *)(param_1 + 0x10)) {
      puVar15 = param_1 + 0x18;
      if (*(undefined **)(param_1 + 0x28) != (undefined *)0x0) {
        puVar15 = *(undefined **)(param_1 + 0x28);
      }
      if ((((*(uint *)(puVar15 + (ulonglong)(uVar25 >> 5) * 4) >> ((uint)uVar24 & 0x1f) & 1) != 0)
          && (lVar21 = (ulonglong)uVar25 * 0x90 + *(longlong *)(param_1 + 8),
             *(ulonglong *)(lVar21 + 0x70) == uVar24)) && (*(char *)(lVar21 + 1) != '\x04')) {
        FTimerManager_InternalClearTimer(param_1,uVar24);
      }
    }
  }
  if (param_4 <= 0.0) {
    *param_2 = 0;
    goto LAB_144627d2d;
  }
  abStack_f8[0] = abStack_f8[0] & 0xfc;
  abStack_f8[1] = 1;
  dStack_f0 = 0.0;
  fStack_f4 = 0.0;
  plStack_e8 = (longlong *)0x0;
  iStack_e0 = 0;
  U_142ee3b30(&plStack_d8);
  FStack_d0.ComparisonIndex = 0;
  FStack_d0.Number = 0;
  plStack_c8 = (longlong *)0x0;
  plStack_b8 = (longlong *)0x0;
  uStack_88 = 0;
  uStack_78 = 0;
  if (param_3 != &plStack_e8) {
    if ((*(int *)(param_3 + 1) == 0) || (*param_3 == (longlong *)0x0)) {
      if ((iStack_e0 != 0) && (plStack_e8 != (longlong *)0x0)) {
        (**(code **)(*plStack_e8 + 0x38))();
        if (plStack_e8 != (longlong *)0x0) {
          plStack_e8 = (longlong *)FMemory_Realloc((undefined *)plStack_e8,0,0);
        }
        iStack_e0 = 0;
      }
    }
    else {
      (**(code **)(**param_3 + 0x40))();
    }
  }
  plStack_d8 = param_3[2];
  FStack_d0 = *(FName *)(param_3 + 3);
  pplVar23 = param_3 + 4;
  if (&plStack_c8 != pplVar23) {
    plVar22 = *pplVar23;
    plVar6 = param_3[5];
    plVar7 = param_3[6];
    plVar8 = param_3[7];
    plVar11 = (longlong *)CONCAT44(uStack_a4,uStack_a8);
    plVar12 = (longlong *)CONCAT44(uStack_9c,uStack_a0);
    uStack_a8 = *(undefined4 *)(param_3 + 8);
    uStack_a4 = *(undefined4 *)((longlong)param_3 + 0x44);
    uStack_a0 = *(undefined4 *)(param_3 + 9);
    uStack_9c = *(undefined4 *)((longlong)param_3 + 0x4c);
    plVar9 = param_3[10];
    plVar10 = param_3[0xb];
    *pplVar23 = plStack_c8;
    param_3[5] = plStack_c0;
    param_3[6] = plStack_b8;
    param_3[7] = plStack_b0;
    param_3[8] = plVar11;
    param_3[9] = plVar12;
    param_3[10] = plStack_98;
    param_3[0xb] = plStack_90;
    plStack_c8 = plVar22;
    plStack_c0 = plVar6;
    plStack_b8 = plVar7;
    plStack_b0 = plVar8;
    plStack_98 = plVar9;
    plStack_90 = plVar10;
  }
  abStack_f8[0] = abStack_f8[0] & 0xfe | param_5;
  fStack_f4 = param_4;
  if (((iStack_e0 == 0) || (plStack_e8 == (longlong *)0x0)) ||
     (cVar13 = (**(code **)(*plStack_e8 + 0x28))(), cVar13 == '\0')) {
    bVar14 = FName_EqualsEName(FStack_d0.ComparisonIndex,0);
    if ((((FStack_d0.Number != 0 || !bVar14) &&
         (Object = FWeakObjectPtr_Get((undefined *)&plStack_d8), Object != (UObject *)0x0)) &&
        (puVar15 = UObject_FindFunction(Object,FStack_d0), puVar15 != (undefined *)0x0)) ||
       (plStack_c8 != (longlong *)0x0)) goto LAB_144627b2a;
    bVar19 = 0;
  }
  else {
LAB_144627b2a:
    bVar19 = 2;
  }
  abStack_f8[0] = abStack_f8[0] & 0xfd | bVar19;
  if (((*(longlong *)(param_1 + 0x158) != 0) && (lVar21 = U_144217e60(), lVar21 != 0)) &&
     (lVar16 = U_14468e190(lVar21), lVar16 != 0)) {
    puVar17 = (undefined1 *)U_14468e190(lVar21);
    uStack_78 = *puVar17;
  }
  if (param_6 < 0.0) {
    param_6 = param_4;
  }
  dStack_f0 = (double)param_6;
  if (*(longlong *)(param_1 + 0x150) == _DAT_1482472b8) {
    dStack_f0 = dStack_f0 + *(double *)(param_1 + 0x140);
    abStack_f8[1] = 1;
    puVar18 = (ulonglong *)U_14461b600(param_1,&uStack_118,abStack_f8);
    iVar20 = *(int *)(param_1 + 0x48);
    uVar24 = *puVar18;
    *(int *)(param_1 + 0x48) = iVar20 + 1;
    if (*(int *)(param_1 + 0x4c) < iVar20 + 1) {
      TArray8_ResizeGrow(param_1 + 0x40,iVar20);
    }
    *(ulonglong *)(*(longlong *)(param_1 + 0x40) + (longlong)iVar20 * 8) = uVar24;
    lVar21 = *(longlong *)(param_1 + 0x40);
    iVar20 = *(int *)(param_1 + 0x48) + -1;
    while (0 < iVar20) {
      lVar16 = (longlong)iVar20;
      iVar20 = (iVar20 + -1) / 2;
      puVar1 = (uint *)(lVar21 + lVar16 * 8);
      puVar2 = (uint *)(lVar21 + (longlong)iVar20 * 8);
      dVar4 = *(double *)(*(longlong *)(param_1 + 8) + 8 + (ulonglong)(*puVar2 & 0xffffff) * 0x90);
      pdVar3 = (double *)
               (*(longlong *)(param_1 + 8) + 8 +
               (ulonglong)(*(uint *)(lVar21 + lVar16 * 8) & 0xffffff) * 0x90);
      if (dVar4 < *pdVar3 || dVar4 == *pdVar3) break;
      if (puVar1 != puVar2) {
        uVar5 = *(undefined8 *)puVar1;
        *(undefined8 *)puVar1 = *(undefined8 *)puVar2;
        *(undefined8 *)puVar2 = uVar5;
      }
    }
  }
  else {
    abStack_f8[1] = 0;
    puVar18 = (ulonglong *)U_14461b600(param_1,&uStack_118,abStack_f8);
    uVar24 = *puVar18;
    uStack_118 = uVar24;
    U_144616360(param_1 + 0xa0,auStack_108,&uStack_118,0);
  }
  *param_2 = uVar24;
  if (plStack_c8 != (longlong *)0x0) {
    plVar22 = (longlong *)&uStack_a8;
    if (plStack_b8 != (longlong *)0x0) {
      plVar22 = plStack_b8;
    }
    (**(code **)(*plVar22 + 0x10))();
  }
  plVar22 = (longlong *)0x0;
  if (iStack_e0 != 0) {
    plVar22 = plStack_e8;
  }
  if (plVar22 == (longlong *)0x0) {
    if (iStack_e0 != 0) {
      if (plStack_e8 == (longlong *)0x0) goto LAB_144627d2d;
      lVar21 = *plStack_e8;
      plVar22 = plStack_e8;
      goto LAB_144627cf8;
    }
  }
  else {
    lVar21 = *plVar22;
LAB_144627cf8:
    (**(code **)(lVar21 + 0x38))(plVar22);
    if (plStack_e8 != (longlong *)0x0) {
      plStack_e8 = (longlong *)FMemory_Realloc((undefined *)plStack_e8,0,0);
    }
    iStack_e0 = 0;
  }
  if (plStack_e8 != (longlong *)0x0) {
    FMemory_Free((undefined *)plStack_e8);
  }
LAB_144627d2d:
  if ((DAT_1482472e4 != '\0') && (pwStack_110 != (wchar_t *)0x0)) {
    U_142cd3770(&pwStack_110);
  }
  U_145803c70(uStack_68 ^ (ulonglong)auStack_138);
  return;
}
// Out-of-line dependency 0x145803c70; parameter ABI remains to be checked.
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void U_145803c70(longlong param_1)

{
  if ((param_1 == _DAT_1480c4288) && ((short)((ulonglong)param_1 >> 0x30) == 0)) {
    return;
  }
  U_14580403c(param_1);
  return;
}

// AFortPlayerControllerGameplay_HideQuicktimeEventWidget exec 0x142a65320
// Out-of-line dependency 0x142a65320; parameter ABI remains to be checked.
void U_142a65320(longlong *param_1,longlong param_2)

{
  *(ulonglong *)(param_2 + 0x20) =
       (ulonglong)(*(longlong *)(param_2 + 0x20) != 0) + *(longlong *)(param_2 + 0x20);
                    /* WARNING: Could not recover jumptable at 0x000142a65338. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(*param_1 + 0x1ff8))();
  return;
}

// AFortPlayerControllerGameplay_FollowPreviousTeammate exec 0x142a65000
// Out-of-line dependency 0x142a65000; parameter ABI remains to be checked.
void U_142a65000(longlong *param_1,longlong param_2)

{
  *(ulonglong *)(param_2 + 0x20) =
       (ulonglong)(*(longlong *)(param_2 + 0x20) != 0) + *(longlong *)(param_2 + 0x20);
                    /* WARNING: Could not recover jumptable at 0x000142a65018. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(*param_1 + 0x1fe8))();
  return;
}

// AFortPlayerControllerGameplay_FollowNextTeammate exec 0x142a64fe0
// Out-of-line dependency 0x142a64fe0; parameter ABI remains to be checked.
void U_142a64fe0(longlong *param_1,longlong param_2)

{
  *(ulonglong *)(param_2 + 0x20) =
       (ulonglong)(*(longlong *)(param_2 + 0x20) != 0) + *(longlong *)(param_2 + 0x20);
                    /* WARNING: Could not recover jumptable at 0x000142a64ff8. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(*param_1 + 0x1fe0))();
  return;
}

// AFortPlayerControllerGameplay_DisplayHUDElementVisibility exec 0x142a64f40
// Out-of-line dependency 0x142a64f40; parameter ABI remains to be checked.
void U_142a64f40(longlong param_1,longlong param_2)

{
  longlong *plVar1;
  uint uVar2;
  bool bVar3;
  undefined *puVar4;
  int iVar5;
  longlong lVar6;
  uint uVar7;
  undefined8 *puVar8;
  uint uVar9;
  uint *puVar10;
  undefined8 *Ptr;
  undefined8 uStackX_8;
  int iStack_b8;
  uint uStack_b4;
  uint uStack_a4;
  int iStack_a0;
  undefined4 uStack_9c;
  undefined *puStack_90;
  ulonglong uStack_88;
  undefined *puStack_80;
  undefined8 uStack_78;
  undefined8 *puStack_70;
  int iStack_68;
  undefined4 uStack_64;
  undefined *apuStack_60 [2];
  longlong *plStack_50;
  undefined8 uStack_48;
  uint *puStack_40;
  uint uStack_38;
  uint uStack_34;
  undefined8 uStack_30;
  
  *(ulonglong *)(param_2 + 0x20) =
       (ulonglong)(*(longlong *)(param_2 + 0x20) != 0) + *(longlong *)(param_2 + 0x20);
  plVar1 = (longlong *)(param_1 + 0x24e0);
  uStack_b4 = 1;
  uVar2 = *(uint *)(param_1 + 0x2508);
  puStack_40 = (uint *)(param_1 + 0x24f0);
  iStack_b8 = 0;
  uStack_a4 = 0;
  iStack_a0 = 0;
  if (uVar2 != 0) {
    puVar10 = puStack_40;
    if (*(uint **)(param_1 + 0x2500) != (uint *)0x0) {
      puVar10 = *(uint **)(param_1 + 0x2500);
    }
    uVar7 = *puVar10;
    iStack_b8 = 0;
    iStack_a0 = 0;
    while (uVar7 == 0) {
      iStack_b8 = iStack_b8 + 1;
      iStack_a0 = iStack_a0 + 0x20;
      if ((int)(((int)(uVar2 - 1) >> 0x1f & 0x1fU) + (uVar2 - 1)) >> 5 < iStack_b8)
      goto LAB_1423cf5fc;
      uVar7 = puVar10[iStack_b8];
    }
    uStack_b4 = -uVar7 & uVar7;
    iVar5 = 0x1f;
    if (uStack_b4 != 0) {
      for (; uStack_b4 >> iVar5 == 0; iVar5 = iVar5 + -1) {
      }
    }
    if (uStack_b4 == 0) {
      iVar5 = 0x20;
    }
    else {
      iVar5 = 0x1f - iVar5;
    }
    uStack_a4 = (iStack_a0 - iVar5) + 0x1f;
    if ((int)uVar2 < (int)uStack_a4) {
LAB_1423cf5fc:
      uStack_a4 = uVar2;
    }
  }
  uVar7 = *(uint *)(param_1 + 0x2508);
  uStack_38 = 0xffffffff;
  uStack_34 = uStack_a4;
  uStack_30 = CONCAT44(uStack_9c,iStack_a0);
  uStack_48 = CONCAT44(uStack_b4,iStack_b8);
  iVar5 = (int)uVar7 >> 5;
  uVar9 = uVar7 & 0xffffffe0;
  plStack_50 = plVar1;
  uStack_a4 = uVar7;
  if (uVar7 != uVar2) {
    puVar10 = puStack_40;
    if (*(uint **)(param_1 + 0x2500) != (uint *)0x0) {
      puVar10 = *(uint **)(param_1 + 0x2500);
    }
    uVar7 = puVar10[iVar5] & -1 << ((byte)uVar7 & 0x1f);
    while (uVar7 == 0) {
      iVar5 = iVar5 + 1;
      uVar9 = uVar9 + 0x20;
      if ((int)(((int)(uVar2 - 1) >> 0x1f & 0x1fU) + (uVar2 - 1)) >> 5 < iVar5) goto LAB_1423cf6c2;
      uVar7 = puVar10[iVar5];
    }
    uVar7 = -uVar7 & uVar7;
    iVar5 = 0x1f;
    if (uVar7 != 0) {
      for (; uVar7 >> iVar5 == 0; iVar5 = iVar5 + -1) {
      }
    }
    if (uVar7 == 0) {
      iVar5 = 0x20;
    }
    else {
      iVar5 = 0x1f - iVar5;
    }
    uStack_a4 = (uVar9 - iVar5) + 0x1f;
    if ((int)uVar2 < (int)uStack_a4) {
LAB_1423cf6c2:
      uStack_a4 = uVar2;
    }
  }
  while( true ) {
    if (((uStack_34 == uStack_a4) && (puStack_40 == (uint *)(param_1 + 0x24f0))) &&
       (plStack_50 == plVar1)) {
      bVar3 = false;
    }
    else {
      bVar3 = true;
    }
    if (!bVar3) break;
    puStack_90 = (undefined *)0x0;
    uStack_88 = 0;
    puStack_80 = (undefined *)0x0;
    uStack_78 = 0;
    FGameplayTagContainer_CopyAssign
              ((undefined *)&puStack_90,
               (undefined *)((longlong)(int)uStack_34 * 0x30 + *plStack_50 + 8));
    puVar4 = puStack_90;
    Ptr = (undefined8 *)0x0;
    puStack_70 = (undefined8 *)0x0;
    uStack_64 = 0;
    iStack_68 = (int)uStack_88;
    if ((int)uStack_88 != 0) {
      U_1406d93e0(&puStack_70,uStack_88 & 0xffffffff,0);
      Ptr = puStack_70;
      memcpy(puStack_70,puVar4);
    }
    lVar6 = (longlong)iStack_68;
    for (puVar8 = Ptr; puVar8 != Ptr + lVar6; puVar8 = puVar8 + 1) {
      uStackX_8 = *puVar8;
      if (3 < DAT_1481348e0) {
        U_142ce9f90(&uStackX_8,apuStack_60);
        U_142ce9f90();
        FMsg_Logf_Internal((byte *)0x0,0,&DAT_1481348e4,4,
                           (ushort *)L"Hidden Reason: \'%s\' : Tag \'%s\'");
        if ((undefined *)CONCAT44(uStack_b4,iStack_b8) != (undefined *)0x0) {
          FMemory_Free((undefined *)CONCAT44(uStack_b4,iStack_b8));
        }
        if (apuStack_60[0] != (undefined *)0x0) {
          FMemory_Free(apuStack_60[0]);
        }
      }
    }
    if (Ptr != (undefined8 *)0x0) {
      FMemory_Free((undefined *)Ptr);
    }
    if (puStack_80 != (undefined *)0x0) {
      FMemory_Free(puStack_80);
    }
    if (puStack_90 != (undefined *)0x0) {
      FMemory_Free(puStack_90);
    }
    uStack_38 = uStack_38 & ~uStack_48._4_4_;
    U_140687d00(&uStack_48);
  }
  return;
}
// Out-of-line dependency 0x1406d93e0; parameter ABI remains to be checked.
void U_1406d93e0(undefined8 *param_1,int param_2,int param_3)

{
  int iVar1;
  ulonglong uVar2;
  undefined *puVar3;
  
  iVar1 = 0;
  if (param_2 != 0) {
    uVar2 = FMemory_QuantizeSize((longlong)param_2 << 3,0);
    iVar1 = (int)(uVar2 >> 3);
    if (iVar1 < param_2) {
      iVar1 = 0x7fffffff;
    }
  }
  if ((iVar1 != param_3) && (((undefined *)*param_1 != (undefined *)0x0 || (iVar1 != 0)))) {
    puVar3 = FMemory_Realloc((undefined *)*param_1,(longlong)iVar1 << 3,0);
    *param_1 = puVar3;
  }
  *(int *)((longlong)param_1 + 0xc) = iVar1;
  return;
}
// Out-of-line dependency 0x142ce9f90; parameter ABI remains to be checked.
longlong * U_142ce9f90(uint *param_1,longlong *param_2)

{
  uint uVar1;
  uint uVar2;
  int iVar3;
  ushort *puVar4;
  undefined *puVar5;
  uint uVar6;
  longlong lStack_28;
  int iStack_20;
  uint uStack_1c;
  
  uVar6 = param_1[1];
  uVar2 = *param_1 >> 0x10;
  uVar1 = *param_1 & 0xffff;
  if (uVar6 == 0) {
    if (DAT_14824b011 == '\0') {
      puVar5 = (undefined *)U_142cd4d80(&DAT_14824b3c0);
      DAT_14824b011 = '\x01';
    }
    else {
      puVar5 = &DAT_14824b3c0;
    }
    U_142ce03a0((ulonglong)(uVar1 * 2) + *(longlong *)(puVar5 + (ulonglong)uVar2 * 8 + 0x10),
                  param_2);
    return param_2;
  }
  lStack_28 = 0;
  uStack_1c = 0;
  if (DAT_14824b011 == '\0') {
    puVar5 = (undefined *)U_142cd4d80(&DAT_14824b3c0);
    uVar6 = param_1[1];
    DAT_14824b011 = '\x01';
  }
  else {
    puVar5 = &DAT_14824b3c0;
  }
  puVar4 = (ushort *)((ulonglong)(uVar1 * 2) + *(longlong *)(puVar5 + (ulonglong)uVar2 * 8 + 0x10));
  uVar1 = (uint)(*puVar4 >> 6);
  iStack_20 = 0;
  if (uVar6 == 0) {
    if (uStack_1c != uVar1) {
      U_140680160(&lStack_28,uVar1);
    }
    U_142cd9f90(puVar4,&lStack_28);
  }
  else {
    if (uStack_1c != uVar1 + 6) {
      U_140680160(&lStack_28,uVar1 + 6);
    }
    U_142cd9f90(puVar4,&lStack_28);
    iVar3 = iStack_20 + -1;
    if (iStack_20 < 1) {
      iVar3 = 0;
    }
    iStack_20 = (iStack_20 < 1) + 1 + iStack_20;
    if ((int)uStack_1c < iStack_20) {
      U_1406800e0(&lStack_28);
    }
    *(undefined2 *)(lStack_28 + (longlong)iVar3 * 2) = 0x5f;
    *(undefined2 *)(lStack_28 + 2 + (longlong)iVar3 * 2) = 0;
    U_142ba93a0(&lStack_28,param_1[1] - 1);
  }
  *param_2 = lStack_28;
  *(int *)(param_2 + 1) = iStack_20;
  *(uint *)((longlong)param_2 + 0xc) = uStack_1c;
  return param_2;
}
// Out-of-line dependency 0x140687d00; parameter ABI remains to be checked.
void U_140687d00(uint *param_1)

{
  uint uVar1;
  int iVar2;
  ulonglong uVar3;
  uint uVar4;
  longlong lVar5;
  
  lVar5 = *(longlong *)(param_1 + 2);
  uVar3 = (ulonglong)(int)*param_1;
  uVar1 = *(uint *)(lVar5 + 0x18);
  if (*(longlong *)(lVar5 + 0x10) != 0) {
    lVar5 = *(longlong *)(lVar5 + 0x10);
  }
  uVar4 = *(uint *)(lVar5 + uVar3 * 4) & param_1[4];
  while( true ) {
    if (uVar4 != 0) {
      uVar4 = -uVar4 & uVar4;
      iVar2 = 0x1f;
      if (uVar4 != 0) {
        for (; uVar4 >> iVar2 == 0; iVar2 = iVar2 + -1) {
        }
      }
      param_1[1] = uVar4;
      if (uVar4 == 0) {
        iVar2 = 0x20;
      }
      else {
        iVar2 = 0x1f - iVar2;
      }
      uVar4 = (param_1[6] - iVar2) + 0x1f;
      if ((int)uVar1 < (int)uVar4) {
        uVar4 = uVar1;
      }
      param_1[5] = uVar4;
      return;
    }
    param_1[6] = param_1[6] + 0x20;
    uVar4 = (int)uVar3 + 1;
    uVar3 = (ulonglong)uVar4;
    *param_1 = uVar4;
    if ((int)(((int)(uVar1 - 1) >> 0x1f & 0x1fU) + (uVar1 - 1)) >> 5 < (int)uVar4) break;
    uVar4 = *(uint *)(lVar5 + (longlong)(int)uVar4 * 4);
    param_1[4] = 0xffffffff;
  }
  param_1[5] = uVar1;
  return;
}

// AFortPlayerControllerGameplay_ClientCreativeStopFly exec 0x142a64a00
// Out-of-line dependency 0x142a64a00; parameter ABI remains to be checked.
void U_142a64a00(longlong *param_1,longlong param_2)

{
  *(ulonglong *)(param_2 + 0x20) =
       (ulonglong)(*(longlong *)(param_2 + 0x20) != 0) + *(longlong *)(param_2 + 0x20);
                    /* WARNING: Could not recover jumptable at 0x000142a64a18. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(*param_1 + 0x1f20))();
  return;
}

// AFortPlayerControllerGameplay_ClientCreativePhoneCreated exec 0x142a64950
// Out-of-line dependency 0x142a64950; parameter ABI remains to be checked.
void U_142a64950(longlong *param_1,longlong param_2)

{
  *(ulonglong *)(param_2 + 0x20) =
       (ulonglong)(*(longlong *)(param_2 + 0x20) != 0) + *(longlong *)(param_2 + 0x20);
                    /* WARNING: Could not recover jumptable at 0x000142a64968. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(*param_1 + 0x1e90))();
  return;
}

// AFortPlayerControllerGameplay_CheckGhostModeItemReplicated exec 0x142a64930
// Out-of-line dependency 0x142a64930; parameter ABI remains to be checked.
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void U_142a64930(longlong *param_1,longlong param_2)

{
  char cVar1;
  longlong *plVar2;
  longlong lVar3;
  longlong *plVar4;
  undefined4 *puVar5;
  undefined *Ptr;
  undefined8 uVar6;
  longlong lVar7;
  undefined1 auStack_e8 [32];
  uint uStack_c8;
  undefined4 uStack_c0;
  char acStack_b8 [4];
  int iStack_b4;
  int aiStack_b0 [2];
  undefined8 *puStack_a8;
  undefined8 uStack_a0;
  undefined8 uStack_98;
  undefined4 uStack_90;
  undefined4 uStack_8c;
  longlong *plStack_88;
  int iStack_80;
  undefined1 auStack_78 [8];
  undefined8 uStack_70;
  longlong lStack_68;
  longlong *plStack_58;
  longlong alStack_48 [4];
  ulonglong uStack_28;
  
  *(ulonglong *)(param_2 + 0x20) =
       (ulonglong)(*(longlong *)(param_2 + 0x20) != 0) + *(longlong *)(param_2 + 0x20);
  uStack_28 = _DAT_1480c4288 ^ (ulonglong)auStack_e8;
  if ((char)param_1[0x489] == '\0') goto LAB_1423cc208;
  plVar4 = param_1 + 0xb3;
  plVar2 = (longlong *)(**(code **)(*plVar4 + 0x50))(plVar4);
  if (plVar2 != (longlong *)0x0) {
    puStack_a8 = (undefined8 *)0x0;
    uStack_a0 = 0;
    (**(code **)(*plVar4 + 0xd0))(plVar4,0x17);
    if (0 < (int)uStack_a0) {
      (**(code **)(*plVar2 + 0x630))(plVar2,*puStack_a8,0,1);
    }
    if (puStack_a8 != (undefined8 *)0x0) {
      FMemory_Free((undefined *)puStack_a8);
    }
  }
  lVar7 = param_1[0x48a];
  if (lVar7 == 0) {
LAB_1423cbf51:
    lVar7 = 0;
  }
  else {
    lVar3 = U_142b5be40();
    if ((*(int *)(*(longlong *)(lVar7 + 0x10) + 0x38) < *(int *)(lVar3 + 0x38)) ||
       (*(longlong *)
         (*(longlong *)(*(longlong *)(lVar7 + 0x10) + 0x30) + (longlong)*(int *)(lVar3 + 0x38) * 8)
        != lVar3 + 0x30)) goto LAB_1423cbf51;
  }
  plVar2 = (longlong *)(**(code **)(*plVar4 + 0x50))(plVar4);
  if ((plVar2 != (longlong *)0x0) &&
     (plVar4 = (longlong *)(**(code **)(*plVar4 + 0xa0))(plVar4,lVar7), plVar4 != (longlong *)0x0))
  {
    acStack_b8[1] = 0;
    iStack_b4 = 0;
    cVar1 = U_142057010(plVar2,plVar4,acStack_b8 + 1,&iStack_b4);
    if ((cVar1 == '\0') || (iStack_b4 == 0)) {
      uStack_c8 = CONCAT31(uStack_c8._1_3_,1);
      (**(code **)(*plVar2 + 0x628))(plVar2,plVar4,0,0);
    }
    else {
      uStack_c8 = 0;
      U_142070440(plVar2,0,iStack_b4,0);
    }
    uStack_c0 = CONCAT31(uStack_c0._1_3_,1);
    uStack_c8 = CONCAT31(uStack_c8._1_3_,1);
    (**(code **)(*plVar2 + 0x640))(plVar2,0,0,0);
    acStack_b8[0] = '\x03';
    aiStack_b0[0] = -1;
    U_142057010(plVar2,plVar4,acStack_b8,aiStack_b0);
    lVar7 = (**(code **)(*param_1 + 0x14d8))(param_1);
    if ((lVar7 != 0) && (lVar7 = (**(code **)(*(longlong *)(lVar7 + 0x4c8) + 0x10))(), lVar7 != 0))
    {
      puVar5 = (undefined4 *)(**(code **)(*plVar4 + 0x480))(plVar4,&puStack_a8);
      lVar7 = U_140708a10(lVar7,*puVar5);
      if ((lVar7 != 0) &&
         (((acStack_b8[0] == '\0' && (aiStack_b0[0] == 0)) &&
          (plVar2 = (longlong *)U_142059e40(plVar2), plVar2 == plVar4)))) goto LAB_1423cc208;
    }
  }
  puStack_a8 = (undefined8 *)0x0;
  uStack_98 = U_1423cbe80;
  uStack_90 = 0;
  lVar7 = (**(code **)(*param_1 + 0x150))(param_1);
  if (*(longlong *)(lVar7 + 0x188) == 0) {
    uVar6 = *(undefined8 *)(lVar7 + 800);
  }
  else {
    uVar6 = *(undefined8 *)(*(longlong *)(lVar7 + 0x188) + 0xc0);
  }
  plVar4 = (longlong *)FMemory_Realloc((undefined *)0x0,0x30,0);
  if (plVar4 != (longlong *)0x0) {
    *plVar4 = (longlong)&DAT_1459a9e80;
    U_142ee3b30(plVar4 + 1,param_1);
    *(undefined4 *)(plVar4 + 2) = (undefined4)uStack_98;
    *(undefined4 *)((longlong)plVar4 + 0x14) = uStack_98._4_4_;
    *(undefined4 *)(plVar4 + 3) = uStack_90;
    *(undefined4 *)((longlong)plVar4 + 0x1c) = uStack_8c;
    lVar7 = U_142bad740();
    plVar4[5] = lVar7;
    *plVar4 = (longlong)&DAT_1459a9ed8;
  }
  plStack_88 = (longlong *)0x0;
  iStack_80 = 0;
  if (plVar4 != (longlong *)0x0) {
    (**(code **)(*plVar4 + 0x40))(plVar4,&plStack_88);
  }
  U_142ee3b30(auStack_78,0);
  uStack_c0 = 0xbf800000;
  uStack_c8 = uStack_c8 & 0xffffff00;
  uStack_70 = 0;
  lStack_68 = 0;
  plStack_58 = (longlong *)0x0;
  U_1446278d0(uVar6,&puStack_a8,&plStack_88,0x3dcccccd);
  if (lStack_68 != 0) {
    plVar2 = alStack_48;
    if (plStack_58 != (longlong *)0x0) {
      plVar2 = plStack_58;
    }
    (**(code **)(*plVar2 + 0x10))();
  }
  if (iStack_80 == 0) {
LAB_1423cc1b1:
    if (plStack_88 != (longlong *)0x0) {
      FMemory_Free((undefined *)plStack_88);
    }
  }
  else if (plStack_88 != (longlong *)0x0) {
    (**(code **)(*plStack_88 + 0x38))();
    if (plStack_88 != (longlong *)0x0) {
      plStack_88 = (longlong *)FMemory_Realloc((undefined *)plStack_88,0,0);
    }
    iStack_80 = 0;
    goto LAB_1423cc1b1;
  }
  if (plVar4 != (longlong *)0x0) {
    (**(code **)(*plVar4 + 0x38))(plVar4);
    Ptr = FMemory_Realloc((undefined *)plVar4,0,0);
    if (Ptr != (undefined *)0x0) {
      FMemory_Free(Ptr);
    }
  }
LAB_1423cc208:
  U_145803c70(uStack_28 ^ (ulonglong)auStack_e8);
  return;
}
// Out-of-line dependency 0x142b5be40; parameter ABI remains to be checked.
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

longlong U_142b5be40(void)

{
  if (_DAT_1481c5660 == 0) {
    GetPrivateStaticClassBody
              ((ushort *)L"/Script/FortniteGame",(ushort *)L"FortWorldItemDefinition",&DAT_1481c5660
               ,U_142b5c5b0,0x740,0x10,0x10000001,0,(ushort *)L"Engine",&LAB_142b59760,
               U_140680300,Stub_Empty,&LAB_142735290,&LAB_1406966e0,false);
  }
  return _DAT_1481c5660;
}
// Out-of-line dependency 0x142057010; parameter ABI remains to be checked.
undefined8 U_142057010(longlong param_1,longlong *param_2,char *param_3,int *param_4)

{
  int iVar1;
  longlong lVar2;
  undefined4 *puVar3;
  char cVar4;
  undefined4 uStack_28;
  undefined4 uStack_24;
  undefined4 uStack_20;
  undefined4 uStack_1c;
  undefined1 auStack_18 [16];
  
  if (param_2 != (longlong *)0x0) {
    lVar2 = U_142b5bd80();
    if ((*(int *)(lVar2 + 0x38) <= *(int *)(param_2[2] + 0x38)) &&
       (*(longlong *)(*(longlong *)(param_2[2] + 0x30) + (longlong)*(int *)(lVar2 + 0x38) * 8) ==
        lVar2 + 0x30)) {
      puVar3 = (undefined4 *)(**(code **)(*param_2 + 0x468))(param_2,auStack_18);
      cVar4 = '\0';
      uStack_28 = *puVar3;
      uStack_24 = puVar3[1];
      uStack_20 = puVar3[2];
      uStack_1c = puVar3[3];
      do {
        if (cVar4 == '\0') {
LAB_142057093:
          lVar2 = 0x220;
        }
        else if (cVar4 == '\x01') {
          lVar2 = 0x2b0;
        }
        else {
          if (cVar4 != '\x02') goto LAB_142057093;
          lVar2 = 0x340;
        }
        iVar1 = U_1420570e0(lVar2 + param_1,&uStack_28);
        if (iVar1 != -1) {
          *param_3 = cVar4;
          *param_4 = iVar1;
          return 1;
        }
        cVar4 = cVar4 + '\x01';
      } while (cVar4 != '\x03');
    }
  }
  return 0;
}
// Out-of-line dependency 0x140708a10; parameter ABI remains to be checked.
longlong U_140708a10(longlong param_1,int param_2)

{
  longlong lVar1;
  longlong lVar2;
  wchar_t *apwStackX_8 [4];
  
  lVar1 = *(longlong *)(param_1 + 0x500);
  apwStackX_8[0] = L"STAT_FindAbilitySpecFromHandle";
  if (DAT_1482472e4 != '\0') {
    apwStackX_8[0] = (wchar_t *)0x0;
  }
  lVar2 = (longlong)*(int *)(param_1 + 0x508) * 200 + lVar1;
  for (; lVar1 != lVar2; lVar1 = lVar1 + 200) {
    if (*(int *)(lVar1 + 0xc) == param_2) goto LAB_140708a65;
  }
  lVar1 = 0;
LAB_140708a65:
  if ((DAT_1482472e4 != '\0') && (apwStackX_8[0] != (wchar_t *)0x0)) {
    U_142cd3770(apwStackX_8);
  }
  return lVar1;
}
// Out-of-line dependency 0x142059e40; parameter ABI remains to be checked.
undefined8 U_142059e40(longlong *param_1)

{
  longlong lVar1;
  undefined8 uVar2;
  byte bVar3;
  
  bVar3 = 0;
  do {
    if (bVar3 < 3) {
      if (bVar3 == 0) {
        lVar1 = 0x220;
      }
      else {
        lVar1 = 0x2b0;
        if (bVar3 != 1) {
          lVar1 = 0x340;
        }
      }
      if (*(int *)(lVar1 + (longlong)param_1) != -1) {
                    /* WARNING: Could not recover jumptable at 0x000142059e7b. Too many branches */
                    /* WARNING: Treating indirect jump as call */
        uVar2 = (**(code **)(*param_1 + 0x648))();
        return uVar2;
      }
    }
    bVar3 = bVar3 + 1;
    if (bVar3 == 3) {
      return 0;
    }
  } while( true );
}
// Out-of-line dependency 0x142ee3b30; parameter ABI remains to be checked.
void U_142ee3b30(undefined4 *param_1,longlong param_2)

{
  undefined4 uVar1;
  
  if (param_2 == 0) {
    *param_1 = 0xffffffff;
    param_1[1] = 0;
    return;
  }
  *param_1 = *(undefined4 *)(param_2 + 0xc);
  uVar1 = U_142ec00d0(&DAT_148263810);
  param_1[1] = uVar1;
  return;
}
// Out-of-line dependency 0x142bad740; parameter ABI remains to be checked.
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

longlong U_142bad740(void)

{
  longlong lVar1;
  longlong lVar2;
  
  LOCK();
  UNLOCK();
  lVar2 = _DAT_147c306f0 + 1;
  lVar1 = _DAT_147c306f0 + 1;
  if (lVar2 == 0) {
    LOCK();
    UNLOCK();
    lVar2 = _DAT_147c306f0 + 2;
    lVar1 = _DAT_147c306f0 + 2;
  }
  _DAT_147c306f0 = lVar1;
  return lVar2;
}
// Out-of-line dependency 0x1446278d0; parameter ABI remains to be checked.
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void U_1446278d0(undefined *param_1,ulonglong *param_2,longlong **param_3,float param_4,
                  byte param_5,float param_6)

{
  uint *puVar1;
  uint *puVar2;
  double *pdVar3;
  double dVar4;
  undefined8 uVar5;
  longlong *plVar6;
  longlong *plVar7;
  longlong *plVar8;
  longlong *plVar9;
  longlong *plVar10;
  longlong *plVar11;
  longlong *plVar12;
  char cVar13;
  bool bVar14;
  UObject *Object;
  undefined *puVar15;
  longlong lVar16;
  undefined1 *puVar17;
  ulonglong *puVar18;
  byte bVar19;
  int iVar20;
  longlong lVar21;
  longlong *plVar22;
  longlong **pplVar23;
  ulonglong uVar24;
  uint uVar25;
  undefined1 auStack_138 [32];
  ulonglong uStack_118;
  wchar_t *pwStack_110;
  undefined1 auStack_108 [16];
  byte abStack_f8 [4];
  float fStack_f4;
  double dStack_f0;
  longlong *plStack_e8;
  int iStack_e0;
  longlong *plStack_d8;
  FName FStack_d0;
  longlong *plStack_c8;
  longlong *plStack_c0;
  longlong *plStack_b8;
  longlong *plStack_b0;
  undefined4 uStack_a8;
  undefined4 uStack_a4;
  undefined4 uStack_a0;
  undefined4 uStack_9c;
  longlong *plStack_98;
  longlong *plStack_90;
  undefined8 uStack_88;
  undefined1 uStack_78;
  ulonglong uStack_68;
  
  uStack_68 = _DAT_1480c4288 ^ (ulonglong)auStack_138;
  uVar24 = *param_2;
  pwStack_110 = L"STAT_SetTimer";
  if (DAT_1482472e4 != '\0') {
    pwStack_110 = (wchar_t *)0x0;
  }
  if (uVar24 != 0) {
    uVar25 = (uint)uVar24 & 0xffffff;
    if ((int)uVar25 < *(int *)(param_1 + 0x10)) {
      puVar15 = param_1 + 0x18;
      if (*(undefined **)(param_1 + 0x28) != (undefined *)0x0) {
        puVar15 = *(undefined **)(param_1 + 0x28);
      }
      if ((((*(uint *)(puVar15 + (ulonglong)(uVar25 >> 5) * 4) >> ((uint)uVar24 & 0x1f) & 1) != 0)
          && (lVar21 = (ulonglong)uVar25 * 0x90 + *(longlong *)(param_1 + 8),
             *(ulonglong *)(lVar21 + 0x70) == uVar24)) && (*(char *)(lVar21 + 1) != '\x04')) {
        FTimerManager_InternalClearTimer(param_1,uVar24);
      }
    }
  }
  if (param_4 <= 0.0) {
    *param_2 = 0;
    goto LAB_144627d2d;
  }
  abStack_f8[0] = abStack_f8[0] & 0xfc;
  abStack_f8[1] = 1;
  dStack_f0 = 0.0;
  fStack_f4 = 0.0;
  plStack_e8 = (longlong *)0x0;
  iStack_e0 = 0;
  U_142ee3b30(&plStack_d8);
  FStack_d0.ComparisonIndex = 0;
  FStack_d0.Number = 0;
  plStack_c8 = (longlong *)0x0;
  plStack_b8 = (longlong *)0x0;
  uStack_88 = 0;
  uStack_78 = 0;
  if (param_3 != &plStack_e8) {
    if ((*(int *)(param_3 + 1) == 0) || (*param_3 == (longlong *)0x0)) {
      if ((iStack_e0 != 0) && (plStack_e8 != (longlong *)0x0)) {
        (**(code **)(*plStack_e8 + 0x38))();
        if (plStack_e8 != (longlong *)0x0) {
          plStack_e8 = (longlong *)FMemory_Realloc((undefined *)plStack_e8,0,0);
        }
        iStack_e0 = 0;
      }
    }
    else {
      (**(code **)(**param_3 + 0x40))();
    }
  }
  plStack_d8 = param_3[2];
  FStack_d0 = *(FName *)(param_3 + 3);
  pplVar23 = param_3 + 4;
  if (&plStack_c8 != pplVar23) {
    plVar22 = *pplVar23;
    plVar6 = param_3[5];
    plVar7 = param_3[6];
    plVar8 = param_3[7];
    plVar11 = (longlong *)CONCAT44(uStack_a4,uStack_a8);
    plVar12 = (longlong *)CONCAT44(uStack_9c,uStack_a0);
    uStack_a8 = *(undefined4 *)(param_3 + 8);
    uStack_a4 = *(undefined4 *)((longlong)param_3 + 0x44);
    uStack_a0 = *(undefined4 *)(param_3 + 9);
    uStack_9c = *(undefined4 *)((longlong)param_3 + 0x4c);
    plVar9 = param_3[10];
    plVar10 = param_3[0xb];
    *pplVar23 = plStack_c8;
    param_3[5] = plStack_c0;
    param_3[6] = plStack_b8;
    param_3[7] = plStack_b0;
    param_3[8] = plVar11;
    param_3[9] = plVar12;
    param_3[10] = plStack_98;
    param_3[0xb] = plStack_90;
    plStack_c8 = plVar22;
    plStack_c0 = plVar6;
    plStack_b8 = plVar7;
    plStack_b0 = plVar8;
    plStack_98 = plVar9;
    plStack_90 = plVar10;
  }
  abStack_f8[0] = abStack_f8[0] & 0xfe | param_5;
  fStack_f4 = param_4;
  if (((iStack_e0 == 0) || (plStack_e8 == (longlong *)0x0)) ||
     (cVar13 = (**(code **)(*plStack_e8 + 0x28))(), cVar13 == '\0')) {
    bVar14 = FName_EqualsEName(FStack_d0.ComparisonIndex,0);
    if ((((FStack_d0.Number != 0 || !bVar14) &&
         (Object = FWeakObjectPtr_Get((undefined *)&plStack_d8), Object != (UObject *)0x0)) &&
        (puVar15 = UObject_FindFunction(Object,FStack_d0), puVar15 != (undefined *)0x0)) ||
       (plStack_c8 != (longlong *)0x0)) goto LAB_144627b2a;
    bVar19 = 0;
  }
  else {
LAB_144627b2a:
    bVar19 = 2;
  }
  abStack_f8[0] = abStack_f8[0] & 0xfd | bVar19;
  if (((*(longlong *)(param_1 + 0x158) != 0) && (lVar21 = U_144217e60(), lVar21 != 0)) &&
     (lVar16 = U_14468e190(lVar21), lVar16 != 0)) {
    puVar17 = (undefined1 *)U_14468e190(lVar21);
    uStack_78 = *puVar17;
  }
  if (param_6 < 0.0) {
    param_6 = param_4;
  }
  dStack_f0 = (double)param_6;
  if (*(longlong *)(param_1 + 0x150) == _DAT_1482472b8) {
    dStack_f0 = dStack_f0 + *(double *)(param_1 + 0x140);
    abStack_f8[1] = 1;
    puVar18 = (ulonglong *)U_14461b600(param_1,&uStack_118,abStack_f8);
    iVar20 = *(int *)(param_1 + 0x48);
    uVar24 = *puVar18;
    *(int *)(param_1 + 0x48) = iVar20 + 1;
    if (*(int *)(param_1 + 0x4c) < iVar20 + 1) {
      TArray8_ResizeGrow(param_1 + 0x40,iVar20);
    }
    *(ulonglong *)(*(longlong *)(param_1 + 0x40) + (longlong)iVar20 * 8) = uVar24;
    lVar21 = *(longlong *)(param_1 + 0x40);
    iVar20 = *(int *)(param_1 + 0x48) + -1;
    while (0 < iVar20) {
      lVar16 = (longlong)iVar20;
      iVar20 = (iVar20 + -1) / 2;
      puVar1 = (uint *)(lVar21 + lVar16 * 8);
      puVar2 = (uint *)(lVar21 + (longlong)iVar20 * 8);
      dVar4 = *(double *)(*(longlong *)(param_1 + 8) + 8 + (ulonglong)(*puVar2 & 0xffffff) * 0x90);
      pdVar3 = (double *)
               (*(longlong *)(param_1 + 8) + 8 +
               (ulonglong)(*(uint *)(lVar21 + lVar16 * 8) & 0xffffff) * 0x90);
      if (dVar4 < *pdVar3 || dVar4 == *pdVar3) break;
      if (puVar1 != puVar2) {
        uVar5 = *(undefined8 *)puVar1;
        *(undefined8 *)puVar1 = *(undefined8 *)puVar2;
        *(undefined8 *)puVar2 = uVar5;
      }
    }
  }
  else {
    abStack_f8[1] = 0;
    puVar18 = (ulonglong *)U_14461b600(param_1,&uStack_118,abStack_f8);
    uVar24 = *puVar18;
    uStack_118 = uVar24;
    U_144616360(param_1 + 0xa0,auStack_108,&uStack_118,0);
  }
  *param_2 = uVar24;
  if (plStack_c8 != (longlong *)0x0) {
    plVar22 = (longlong *)&uStack_a8;
    if (plStack_b8 != (longlong *)0x0) {
      plVar22 = plStack_b8;
    }
    (**(code **)(*plVar22 + 0x10))();
  }
  plVar22 = (longlong *)0x0;
  if (iStack_e0 != 0) {
    plVar22 = plStack_e8;
  }
  if (plVar22 == (longlong *)0x0) {
    if (iStack_e0 != 0) {
      if (plStack_e8 == (longlong *)0x0) goto LAB_144627d2d;
      lVar21 = *plStack_e8;
      plVar22 = plStack_e8;
      goto LAB_144627cf8;
    }
  }
  else {
    lVar21 = *plVar22;
LAB_144627cf8:
    (**(code **)(lVar21 + 0x38))(plVar22);
    if (plStack_e8 != (longlong *)0x0) {
      plStack_e8 = (longlong *)FMemory_Realloc((undefined *)plStack_e8,0,0);
    }
    iStack_e0 = 0;
  }
  if (plStack_e8 != (longlong *)0x0) {
    FMemory_Free((undefined *)plStack_e8);
  }
LAB_144627d2d:
  if ((DAT_1482472e4 != '\0') && (pwStack_110 != (wchar_t *)0x0)) {
    U_142cd3770(&pwStack_110);
  }
  U_145803c70(uStack_68 ^ (ulonglong)auStack_138);
  return;
}
// Out-of-line dependency 0x145803c70; parameter ABI remains to be checked.
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void U_145803c70(longlong param_1)

{
  if ((param_1 == _DAT_1480c4288) && ((short)((ulonglong)param_1 >> 0x30) == 0)) {
    return;
  }
  U_14580403c(param_1);
  return;
}

// AFortGameModeZone_IsSpectatingAllowedAfterDeath exec 0x1428f3510
// Out-of-line dependency 0x1428f3510; parameter ABI remains to be checked.
void U_1428f3510(longlong *param_1,longlong param_2,undefined1 *param_3)

{
  undefined1 uVar1;
  
  *(ulonglong *)(param_2 + 0x20) =
       (ulonglong)(*(longlong *)(param_2 + 0x20) != 0) + *(longlong *)(param_2 + 0x20);
  uVar1 = (**(code **)(*param_1 + 0xbd8))();
  *param_3 = uVar1;
  return;
}

// AFortGameModeZone_GetTeammateReviveGameplayEffectOverride exec 0x1428f3420
// Out-of-line dependency 0x1428f3420; parameter ABI remains to be checked.
void U_1428f3420(longlong *param_1,longlong param_2,undefined8 *param_3)

{
  undefined8 *puVar1;
  undefined1 auStackX_8 [32];
  
  *(ulonglong *)(param_2 + 0x20) =
       (ulonglong)(*(longlong *)(param_2 + 0x20) != 0) + *(longlong *)(param_2 + 0x20);
  puVar1 = (undefined8 *)(**(code **)(*param_1 + 0xbd0))(param_1,auStackX_8);
  *param_3 = *puVar1;
  return;
}

// AFortGameModeZone_GetPlayerStartupController exec 0x1428f33e0
// Out-of-line dependency 0x1428f33e0; parameter ABI remains to be checked.
void U_1428f33e0(longlong param_1,longlong param_2,undefined8 *param_3)

{
  UObject *pUVar1;
  
  *(ulonglong *)(param_2 + 0x20) =
       (ulonglong)(*(longlong *)(param_2 + 0x20) != 0) + *(longlong *)(param_2 + 0x20);
  pUVar1 = FWeakObjectPtr_Get((undefined *)(param_1 + 0x678));
  *param_3 = pUVar1;
  return;
}

// AFortGameModeZone_SetJoinability exec 0x1428f39c0
// Out-of-line dependency 0x1428f39c0; parameter ABI remains to be checked.
void U_1428f39c0(longlong *param_1,FFrame *param_2)

{
  undefined *Property;
  int aiStackX_8 [2];
  
  aiStackX_8[0] = 0;
  if (*(longlong *)param_2->Code == 0) {
    Property = *(undefined **)param_2->PropertyChainForCompiledIn;
    *(undefined8 *)param_2->PropertyChainForCompiledIn = *(undefined8 *)(Property + 0x20);
    FFrame_StepExplicitProperty(param_2,(undefined *)aiStackX_8,Property);
  }
  else {
    FFrame_Step(param_2,*(UObject **)param_2->Object,(undefined *)aiStackX_8);
  }
  *(ulonglong *)param_2->Code =
       (ulonglong)(*(longlong *)param_2->Code != 0) + *(longlong *)param_2->Code;
  (**(code **)(*param_1 + 0xbe8))(param_1,aiStackX_8[0] != 0);
  return;
}

// AFortGameModeZone_SetEnemyAIItemCacheLootDrops exec 0x1428f36a0
// Out-of-line dependency 0x1428f36a0; parameter ABI remains to be checked.
void U_1428f36a0(undefined8 param_1,FFrame *param_2)

{
  undefined *Property;
  int aiStackX_10 [2];
  
  aiStackX_10[0] = 0;
  if (*(longlong *)param_2->Code == 0) {
    Property = *(undefined **)param_2->PropertyChainForCompiledIn;
    *(undefined8 *)param_2->PropertyChainForCompiledIn = *(undefined8 *)(Property + 0x20);
    FFrame_StepExplicitProperty(param_2,(undefined *)aiStackX_10,Property);
  }
  else {
    FFrame_Step(param_2,*(UObject **)param_2->Object,(undefined *)aiStackX_10);
  }
  *(ulonglong *)param_2->Code =
       (ulonglong)(*(longlong *)param_2->Code != 0) + *(longlong *)param_2->Code;
  U_141eaed80(param_1,aiStackX_10[0] != 0);
  return;
}
// Out-of-line dependency 0x141eaed80; parameter ABI remains to be checked.
void U_141eaed80(longlong param_1,byte param_2)

{
  *(uint *)(param_1 + 0x820) = *(uint *)(param_1 + 0x820) & 0xfffffffd;
  *(uint *)(param_1 + 0x820) = *(uint *)(param_1 + 0x820) | (uint)param_2 * 2;
  return;
}

// AFortGameModeZone_SetZoneLootNamedWeightsMapOverride exec 0x1428f3c00
// Out-of-line dependency 0x1428f3c00; parameter ABI remains to be checked.
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void U_1428f3c00(undefined8 param_1,FFrame *param_2)

{
  undefined *Property;
  ulonglong uVar1;
  undefined **ppuVar2;
  undefined1 auStack_98 [32];
  undefined *puStack_78;
  ulonglong uStack_70;
  undefined1 auStack_68 [16];
  undefined *puStack_58;
  undefined4 uStack_50;
  int iStack_4c;
  undefined4 uStack_48;
  undefined4 uStack_44;
  undefined *puStack_38;
  undefined4 uStack_30;
  ulonglong uStack_28;
  
  uStack_28 = _DAT_1480c4288 ^ (ulonglong)auStack_98;
  iStack_4c = 0x80;
  puStack_78 = (undefined *)0x0;
  uStack_70 = 0;
  puStack_58 = (undefined *)0x0;
  uStack_50 = 0;
  uStack_48 = 0xffffffff;
  uStack_44 = 0;
  puStack_38 = (undefined *)0x0;
  uStack_30 = 0;
  *(undefined8 *)param_2->MostRecentPropertyAddress = 0;
  if (*(longlong *)param_2->Code == 0) {
    Property = *(undefined **)param_2->PropertyChainForCompiledIn;
    *(undefined8 *)param_2->PropertyChainForCompiledIn = *(undefined8 *)(Property + 0x20);
    FFrame_StepExplicitProperty(param_2,(undefined *)&puStack_78,Property);
  }
  else {
    FFrame_Step(param_2,*(UObject **)param_2->Object,(undefined *)&puStack_78);
  }
  ppuVar2 = &puStack_78;
  if (*(undefined ***)param_2->MostRecentPropertyAddress != (undefined **)0x0) {
    ppuVar2 = *(undefined ***)param_2->MostRecentPropertyAddress;
  }
  *(ulonglong *)param_2->Code =
       (ulonglong)(*(longlong *)param_2->Code != 0) + *(longlong *)param_2->Code;
  U_141eb2450(param_1,ppuVar2);
  uStack_30 = 0;
  if (puStack_38 != (undefined *)0x0) {
    FMemory_Free(puStack_38);
  }
  uVar1 = uStack_70 >> 0x20;
  uStack_70 = uStack_70 & 0xffffffff00000000;
  if ((int)uVar1 != 0) {
    U_14069eb50(&puStack_78,0);
  }
  uStack_48 = 0xffffffff;
  uStack_44 = 0;
  uStack_50 = 0;
  if (iStack_4c != 0) {
    iStack_4c = 0;
    U_140690310(auStack_68,0);
  }
  if (puStack_58 != (undefined *)0x0) {
    FMemory_Free(puStack_58);
  }
  if (puStack_78 != (undefined *)0x0) {
    FMemory_Free(puStack_78);
  }
  U_145803c70(uStack_28 ^ (ulonglong)auStack_98);
  return;
}
// Out-of-line dependency 0x141eb2450; parameter ABI remains to be checked.
void U_141eb2450(longlong param_1)

{
  U_1406af2d0(param_1 + 0x7b8);
  return;
}
// Out-of-line dependency 0x14069eb50; parameter ABI remains to be checked.
void U_14069eb50(undefined8 *param_1,int param_2)

{
  ulonglong uVar1;
  undefined *puVar2;
  int iVar3;
  
  iVar3 = 0;
  if (param_2 != 0) {
    uVar1 = FMemory_QuantizeSize((longlong)param_2 * 0x14,0);
    iVar3 = (int)(uVar1 / 0x14);
    if (iVar3 < param_2) {
      iVar3 = 0x7fffffff;
    }
  }
  if (iVar3 != *(int *)((longlong)param_1 + 0xc)) {
    *(int *)((longlong)param_1 + 0xc) = iVar3;
    if (((undefined *)*param_1 != (undefined *)0x0) || (iVar3 != 0)) {
      puVar2 = FMemory_Realloc((undefined *)*param_1,(longlong)iVar3 * 0x14,0);
      *param_1 = puVar2;
    }
  }
  return;
}
// Out-of-line dependency 0x140690310; parameter ABI remains to be checked.
void U_140690310(longlong param_1,int param_2)

{
  int iVar1;
  int iVar2;
  uint uVar3;
  undefined *puVar4;
  ulonglong Count;
  
  iVar1 = (int)(param_2 + 0x1f + (param_2 + 0x1f >> 0x1f & 0x1fU)) >> 5;
  puVar4 = *(undefined **)(param_1 + 0x10);
  iVar2 = *(int *)(param_1 + 0x1c) + 0x1f;
  uVar3 = (int)(iVar2 + (iVar2 >> 0x1f & 0x1fU)) >> 5;
  if (uVar3 < 5) {
    if (puVar4 == (undefined *)0x0) goto LAB_1406903b3;
    memmove(param_1);
    puVar4 = *(undefined **)(param_1 + 0x10);
    if (puVar4 == (undefined *)0x0) goto LAB_1406903b3;
    Count = 0;
  }
  else {
    if (puVar4 == (undefined *)0x0) {
      puVar4 = (undefined *)0x0;
      if (uVar3 != 0) {
        puVar4 = FMemory_Realloc((undefined *)0x0,(longlong)(int)uVar3 << 2,0);
        *(undefined **)(param_1 + 0x10) = puVar4;
      }
      memmove(puVar4,param_1,(longlong)iVar1 << 2);
      goto LAB_1406903b3;
    }
    Count = (longlong)(int)uVar3 << 2;
  }
  puVar4 = FMemory_Realloc(puVar4,Count,0);
  *(undefined **)(param_1 + 0x10) = puVar4;
LAB_1406903b3:
  if (uVar3 != 0) {
    if (*(longlong *)(param_1 + 0x10) != 0) {
      param_1 = *(longlong *)(param_1 + 0x10);
    }
    memset(param_1 + (longlong)iVar1 * 4,0,(longlong)(int)(uVar3 - iVar1) << 2);
  }
  return;
}
// Out-of-line dependency 0x145803c70; parameter ABI remains to be checked.
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void U_145803c70(longlong param_1)

{
  if ((param_1 == _DAT_1480c4288) && ((short)((ulonglong)param_1 >> 0x30) == 0)) {
    return;
  }
  U_14580403c(param_1);
  return;
}

// AFortGameModeZone_SetPlayerStartLocationOverrides exec 0x1428f3a50
// Out-of-line dependency 0x1428f3a50; parameter ABI remains to be checked.
void U_1428f3a50(undefined8 param_1,FFrame *param_2)

{
  undefined *puVar1;
  longlong lVar2;
  undefined8 uStackX_10;
  undefined *puStack_28;
  ulonglong uStack_20;
  undefined8 uStack_18;
  int iStack_10;
  undefined4 uStack_c;
  
  puStack_28 = (undefined *)0x0;
  uStack_20 = 0;
  if (*(longlong *)param_2->Code == 0) {
    puVar1 = *(undefined **)param_2->PropertyChainForCompiledIn;
    *(undefined8 *)param_2->PropertyChainForCompiledIn = *(undefined8 *)(puVar1 + 0x20);
    FFrame_StepExplicitProperty(param_2,(undefined *)&puStack_28,puVar1);
  }
  else {
    FFrame_Step(param_2,*(UObject **)param_2->Object,(undefined *)&puStack_28);
  }
  uStackX_10 = 0;
  if (*(longlong *)param_2->Code == 0) {
    puVar1 = *(undefined **)param_2->PropertyChainForCompiledIn;
    *(undefined8 *)param_2->PropertyChainForCompiledIn = *(undefined8 *)(puVar1 + 0x20);
    FFrame_StepExplicitProperty(param_2,(undefined *)&uStackX_10,puVar1);
  }
  else {
    FFrame_Step(param_2,*(UObject **)param_2->Object,(undefined *)&uStackX_10);
  }
  puVar1 = puStack_28;
  uStack_18 = 0;
  *(ulonglong *)param_2->Code =
       (ulonglong)(*(longlong *)param_2->Code != 0) + *(longlong *)param_2->Code;
  lVar2 = (longlong)(int)uStack_20;
  iStack_10 = (int)uStack_20;
  if ((int)uStack_20 == 0) {
    uStack_c = 0;
  }
  else {
    U_14072b300(&uStack_18,uStack_20 & 0xffffffff,0);
    memcpy(uStack_18,puVar1,lVar2 * 0xc);
  }
  U_14072e780(param_1,&uStack_18,uStackX_10);
  if (puStack_28 != (undefined *)0x0) {
    FMemory_Free(puStack_28);
  }
  return;
}
// Out-of-line dependency 0x14072b300; parameter ABI remains to be checked.
void U_14072b300(undefined8 *param_1,int param_2,int param_3)

{
  ulonglong uVar1;
  undefined *puVar2;
  int iVar3;
  
  iVar3 = 0;
  if (param_2 != 0) {
    uVar1 = FMemory_QuantizeSize((longlong)param_2 * 0xc,0);
    iVar3 = (int)(uVar1 / 0xc);
    if (iVar3 < param_2) {
      iVar3 = 0x7fffffff;
    }
  }
  if ((iVar3 != param_3) && (((undefined *)*param_1 != (undefined *)0x0 || (iVar3 != 0)))) {
    puVar2 = FMemory_Realloc((undefined *)*param_1,(longlong)iVar3 * 0xc,0);
    *param_1 = puVar2;
  }
  *(int *)((longlong)param_1 + 0xc) = iVar3;
  return;
}
// Out-of-line dependency 0x14072e780; parameter ABI remains to be checked.
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void U_14072e780(undefined8 param_1,longlong *param_2)

{
  longlong lVar1;
  
  lVar1 = *param_2;
  if (lVar1 == 0) {
    return;
  }
  if (lVar1 != 0) {
    if (_GMalloc == (longlong *)0x0) {
      U_142bee820();
    }
    (**(code **)(*_GMalloc + 0x30))(_GMalloc,lVar1);
  }
  return;
}

// AFortGameModeZone_OnMorningReached exec 0x1428f3560
// Out-of-line dependency 0x1428f3560; parameter ABI remains to be checked.
void U_1428f3560(longlong *param_1,longlong param_2)

{
  *(ulonglong *)(param_2 + 0x20) =
       (ulonglong)(*(longlong *)(param_2 + 0x20) != 0) + *(longlong *)(param_2 + 0x20);
                    /* WARNING: Could not recover jumptable at 0x0001428f3578. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(*param_1 + 0xc08))();
  return;
}

// AFortGameModeZone_OnEveningReached exec 0x1428f3540
// Out-of-line dependency 0x1428f3540; parameter ABI remains to be checked.
void U_1428f3540(longlong *param_1,longlong param_2)

{
  *(ulonglong *)(param_2 + 0x20) =
       (ulonglong)(*(longlong *)(param_2 + 0x20) != 0) + *(longlong *)(param_2 + 0x20);
                    /* WARNING: Could not recover jumptable at 0x0001428f3558. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(*param_1 + 0xc00))();
  return;
}
