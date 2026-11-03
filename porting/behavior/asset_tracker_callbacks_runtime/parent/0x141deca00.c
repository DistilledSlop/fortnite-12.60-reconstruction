
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_141deca00(longlong *param_1,char param_2)

{
  longlong *plVar1;
  undefined **ppuVar2;
  longlong **pplVar3;
  longlong lVar4;
  longlong *plVar5;
  double dVar6;
  char cVar7;
  int iVar8;
  undefined8 *puVar9;
  undefined8 uVar10;
  undefined **ppuVar11;
  longlong *plVar12;
  int iVar13;
  int *piVar14;
  longlong lVar15;
  int iVar16;
  longlong lVar17;
  int *piVar18;
  undefined *puVar19;
  bool bVar20;
  longlong *plVar21;
  undefined *in_stack_ffffffffffffff30;
  undefined *puVar22;
  longlong *plStack_a8;
  longlong *plStack_a0;
  longlong *aplStack_98 [2];
  longlong *plStack_88;
  longlong *plStack_80;
  undefined *puStack_78;
  longlong *plStack_70;
  undefined *puStack_68;
  int iStack_60;
  undefined4 uStack_5c;
  undefined *puStack_58;
  int iStack_50;
  int iStack_4c;
  
  if (*(char *)((longlong)param_1 + 0x6cc) != param_2) {
    plStack_a8 = param_1;
    if (6 < DAT_1481348e0) {
      in_stack_ffffffffffffff30 =
           (undefined *)((ulonglong)in_stack_ffffffffffffff30 & 0xffffffff00000000);
      FMsg_Logf_Internal((byte *)0x0,0,&DAT_1481348e4,7,
                         (ushort *)
                         L"UFortAssetManager::ChangeGameSubMode - Changing to NewSubMode %d");
    }
    if ((4 < DAT_148126e88) &&
       (FMsg_Logf_Internal((byte *)0x0,0,&DAT_148126e8c,5,(ushort *)L"---------------------"),
       4 < DAT_148126e88)) {
      FUN_141f90530(&puStack_58,param_2);
      puVar9 = (undefined8 *)FUN_141f90530(&puStack_78);
      if (*(int *)(puVar9 + 1) == 0) {
        in_stack_ffffffffffffff30 = &DAT_1459a4b30;
      }
      else {
        in_stack_ffffffffffffff30 = (undefined *)*puVar9;
      }
      FMsg_Logf_Internal((byte *)0x0,0,&DAT_148126e8c,5,
                         (ushort *)
                         L"UFortAssetManager::ChangeGameSubMode is changing mode from %s to %s");
      if (puStack_78 != (undefined *)0x0) {
        FMemory_Free(puStack_78);
      }
      if (puStack_58 != (undefined *)0x0) {
        FMemory_Free(puStack_58);
      }
    }
    bVar20 = false;
    *(int *)((longlong)param_1 + 0x754) = *(int *)((longlong)param_1 + 0x754) + 1;
    iVar13 = *(int *)((longlong)param_1 + 0x754);
    iVar8 = (int)param_1[0xe9] + -1;
    lVar17 = (longlong)iVar8;
    if (-1 < iVar8) {
      lVar15 = lVar17 << 4;
      bVar20 = false;
      do {
        lVar4 = param_1[0xe8];
        if (((*(int *)(lVar15 + 8 + lVar4) == 0) ||
            (plVar5 = *(longlong **)(lVar15 + lVar4), plVar5 == (longlong *)0x0)) ||
           (cVar7 = (**(code **)(*plVar5 + 0x50))(), cVar7 == '\0')) {
          bVar20 = true;
        }
        lVar15 = lVar15 + -0x10;
        lVar17 = lVar17 + -1;
      } while (-1 < lVar17);
      iVar13 = *(int *)((longlong)param_1 + 0x754);
    }
    *(int *)((longlong)param_1 + 0x754) = iVar13 + -1;
    if (bVar20) {
      FMulticastDelegateBase_CompactInvocationList((undefined *)(param_1 + 0xe8),false);
    }
    uVar10 = FUN_141ed2350();
    FUN_141ecaec0(uVar10,*(undefined1 *)((longlong)param_1 + 0x6cc),param_2);
    *(char *)((longlong)param_1 + 0x6cc) = param_2;
    uVar10 = FUN_14463f970(_DAT_148351eb8);
    FUN_141eb5c00(uVar10);
    QueryPerformanceCounter(aplStack_98);
    dVar6 = (double)(longlong)aplStack_98[0] * _DAT_1481d47e8;
    ppuVar11 = (undefined **)FUN_141e0b3e0(param_1);
    ppuVar2 = (undefined **)(param_1 + 0xe4);
    if (ppuVar2 != ppuVar11) {
      *ppuVar2 = *ppuVar11;
      *ppuVar11 = (undefined *)0x0;
      plVar5 = (longlong *)ppuVar11[1];
      plVar12 = (longlong *)param_1[0xe5];
      if (plVar5 != plVar12) {
        ppuVar11[1] = (undefined *)0x0;
        param_1[0xe5] = (longlong)plVar5;
        if (plVar12 != (longlong *)0x0) {
          plVar5 = plVar12 + 1;
          *(int *)plVar5 = (int)*plVar5 + -1;
          if ((int)*plVar5 == 0) {
            (**(code **)*plVar12)(plVar12);
            piVar14 = (int *)((longlong)plVar12 + 0xc);
            *piVar14 = *piVar14 + -1;
            if (*piVar14 == 0) {
              (**(code **)(*plVar12 + 8))(plVar12);
            }
          }
        }
      }
    }
    if (plStack_70 != (longlong *)0x0) {
      plVar5 = plStack_70 + 1;
      *(int *)plVar5 = (int)*plVar5 + -1;
      if ((int)*plVar5 == 0) {
        (**(code **)*plStack_70)(plStack_70);
        piVar14 = (int *)((longlong)plStack_70 + 0xc);
        *piVar14 = *piVar14 + -1;
        if (*piVar14 == 0) {
          (**(code **)(*plStack_70 + 8))(plStack_70);
        }
      }
    }
    if (*ppuVar2 != (undefined *)0x0) {
      FUN_141eef450();
      FUN_1445fb140(*ppuVar2,0,0);
      if (ppuVar2 != &puStack_78) {
        *ppuVar2 = (undefined *)0x0;
        plVar5 = (longlong *)param_1[0xe5];
        if (plVar5 != (longlong *)0x0) {
          param_1[0xe5] = 0;
          plVar12 = plVar5 + 1;
          *(int *)plVar12 = (int)*plVar12 + -1;
          if ((int)*plVar12 == 0) {
            (**(code **)*plVar5)(plVar5);
            piVar14 = (int *)((longlong)plVar5 + 0xc);
            *piVar14 = *piVar14 + -1;
            if (*piVar14 == 0) {
              (**(code **)(*plVar5 + 8))(plVar5);
            }
          }
        }
      }
      if (3 < DAT_1481348e0) {
        QueryPerformanceCounter();
        in_stack_ffffffffffffff30 =
             (undefined *)((double)(longlong)aplStack_98[0] * _DAT_1481d47e8 - dVar6);
        FMsg_Logf_Internal((byte *)0x0,0,&DAT_1481348e4,4,
                           (ushort *)
                           L"ChangeGameSubMode took %.2f seconds to preload startup assets");
      }
    }
    plVar5 = param_1 + 0x93;
    iStack_60 = (int)param_1[0x94];
    lVar15 = (longlong)iStack_60;
    lVar17 = *plVar5;
    puStack_68 = (undefined *)0x0;
    if (iStack_60 == 0) {
      uStack_5c = 0;
    }
    else {
      FUN_1406d93e0(&puStack_68,iStack_60,0);
      memcpy(puStack_68,lVar17,lVar15 << 3);
    }
    aplStack_98[0] = (longlong *)param_1[0x95];
    plVar12 = aplStack_98[0] + (int)param_1[0x96];
    plVar21 = param_1;
    plStack_88 = plVar12;
    if (aplStack_98[0] != plVar12) {
      do {
        lVar17 = (longlong)(int)param_1[0x94];
        if ((int)param_1[0x94] != 0) {
          iVar8 = 0;
          lVar15 = 0;
          bVar20 = ((int *)*plVar5)[1] != *(int *)((longlong)aplStack_98[0] + 4) ||
                   *(int *)*plVar5 != (int)*aplStack_98[0];
          iVar13 = 0;
          do {
            lVar15 = lVar15 + 1;
            iVar16 = iVar13 + 1;
            if (lVar15 < lVar17) {
              piVar14 = (int *)(*plVar5 + lVar15 * 8);
              do {
                if (bVar20 != (piVar14[1] != *(int *)((longlong)aplStack_98[0] + 4) ||
                              *piVar14 != (int)*aplStack_98[0])) break;
                iVar16 = iVar16 + 1;
                lVar15 = lVar15 + 1;
                piVar14 = piVar14 + 2;
              } while (lVar15 < lVar17);
            }
            if (bVar20 != false) {
              if (iVar8 != iVar13) {
                memmove(*plVar5 + (longlong)iVar8 * 8,*plVar5 + (longlong)iVar13 * 8,
                        (longlong)(iVar16 - iVar13) << 3);
              }
              iVar8 = iVar8 + (iVar16 - iVar13);
            }
            bVar20 = (bool)(bVar20 ^ 1);
            iVar13 = iVar16;
          } while (lVar15 < lVar17);
          *(int *)(param_1 + 0x94) = iVar8;
          plVar12 = plStack_88;
        }
        aplStack_98[0] = aplStack_98[0] + 1;
      } while (aplStack_98[0] != plVar12);
      plVar21 = plStack_a8;
    }
    puVar19 = &DAT_1459a4b30;
    FUN_141debbc0(plVar21,param_2);
    iVar13 = (int)plVar21[0x96];
    if (iVar13 == 0) {
      iVar13 = (int)plVar21[0x94];
    }
    else {
      iVar8 = (int)param_1[0x94];
      if (*(int *)((longlong)param_1 + 0x4a4) < iVar8 + iVar13) {
        FUN_1406d9a30(plVar5);
        iVar8 = (int)param_1[0x94];
      }
      memcpy(*plVar5 + (longlong)iVar8 * 8,plVar21[0x95],(longlong)iVar13 << 3);
      *(int *)(param_1 + 0x94) = (int)param_1[0x94] + iVar13;
      iVar13 = (int)param_1[0x94];
    }
    piVar18 = (int *)*plVar5;
    piVar14 = piVar18 + (longlong)iVar13 * 2;
    for (; piVar18 != piVar14; piVar18 = piVar18 + 2) {
      if (*piVar18 == _DAT_14812b6b0 && piVar18[1] == _DAT_14812b6b4) {
        FUN_141e0dd50(plVar5,&DAT_14812b6b8);
        FUN_141e0dd50(plVar5,&DAT_14812b6c0);
        if (*(char *)((longlong)plVar21 + 0x6cc) == '\x01') {
          iVar13 = (int)param_1[0x94];
          *(int *)(param_1 + 0x94) = iVar13 + 1;
          if (*(int *)((longlong)param_1 + 0x4a4) < iVar13 + 1) {
            TArray8_ResizeGrow((undefined *)plVar5,iVar13);
          }
          uVar10 = _DAT_14812b6b8;
        }
        else {
          if (*(char *)((longlong)plVar21 + 0x6cc) != '\0') break;
          iVar13 = (int)param_1[0x94];
          *(int *)(param_1 + 0x94) = iVar13 + 1;
          if (*(int *)((longlong)param_1 + 0x4a4) < iVar13 + 1) {
            TArray8_ResizeGrow((undefined *)plVar5,iVar13);
          }
          uVar10 = _DAT_14812b6c0;
        }
        *(undefined8 *)(*plVar5 + (longlong)iVar13 * 8) = uVar10;
        break;
      }
    }
    puVar22 = puStack_68;
    lVar17 = (longlong)iStack_60;
    puStack_58 = (undefined *)0x0;
    iStack_50 = iStack_60;
    if (iStack_60 == 0) {
      iStack_4c = iStack_60;
    }
    else {
      FUN_1406d93e0(&puStack_58,iStack_60,0);
      memcpy(puStack_58,puVar22,lVar17 << 3);
    }
    puVar22 = (undefined *)((ulonglong)in_stack_ffffffffffffff30 & 0xffffffffffffff00);
    puStack_78 = (undefined *)0x0;
    plStack_70 = (longlong *)0x0;
    plStack_88 = (longlong *)0x0;
    plStack_80 = (longlong *)0x0;
    plStack_a8 = (longlong *)0x0;
    plStack_a0 = (longlong *)0x0;
    cVar7 = FUN_14401ea80(plVar21,&puStack_78,&plStack_a8,&puStack_58,&plStack_88,puVar22);
    if (plStack_a8 != (longlong *)0x0) {
      FMemory_Free((undefined *)plStack_a8);
    }
    if (plStack_88 != (longlong *)0x0) {
      FMemory_Free((undefined *)plStack_88);
    }
    if (cVar7 != '\0') {
      if (4 < DAT_148126e88) {
        puVar9 = (undefined8 *)FUN_141e05950();
        if (*(int *)(puVar9 + 1) != 0) {
          puVar19 = (undefined *)*puVar9;
        }
        FMsg_Logf_Internal((byte *)0x0,0,&DAT_148126e8c,5,
                           (ushort *)
                           L"UFortAssetManager::ChangeGameSubMode() changing bundle state to (%s)");
        puVar22 = puVar19;
        if (plStack_a8 != (longlong *)0x0) {
          FMemory_Free((undefined *)plStack_a8);
          puVar22 = puVar19;
        }
      }
      pplVar3 = (longlong **)(plVar21 + 0xe6);
      if (pplVar3 != &plStack_a8) {
        *pplVar3 = (longlong *)0x0;
        plVar12 = (longlong *)plVar21[0xe7];
        if (plVar12 != (longlong *)0x0) {
          plVar21[0xe7] = 0;
          plVar1 = plVar12 + 1;
          *(int *)plVar1 = (int)*plVar1 + -1;
          if ((int)*plVar1 == 0) {
            (**(code **)*plVar12)();
            piVar14 = (int *)((longlong)plVar12 + 0xc);
            *piVar14 = *piVar14 + -1;
            if (*piVar14 == 0) {
              (**(code **)(*plVar12 + 8))(plVar12,1);
            }
          }
        }
      }
      plStack_a8 = (longlong *)0x0;
      plVar12 = (longlong *)FMemory_Realloc((undefined *)0x0,0x30,0);
      plStack_a0 = (longlong *)CONCAT44(plStack_a0._4_4_,3);
      plStack_a8 = plVar12;
      if (plVar12 != (longlong *)0x0) {
        *plVar12 = (longlong)&DAT_1459bfd10;
        FUN_142ee3b30(plVar12 + 1,plVar21);
        plVar12[2] = (longlong)FUN_141e07c00;
        lVar17 = FUN_142bad740();
        plVar12[4] = lVar17;
        *plVar12 = (longlong)&DAT_1459bfd68;
      }
      (**(code **)(*plVar21 + 0x330))
                (plVar21,&plStack_88,&puStack_78,plVar5,&puStack_68,
                 (ulonglong)puVar22 & 0xffffffffffffff00,&plStack_a8,100);
      if ((plStack_88 != (longlong *)0x0) && ((char)plStack_88[2] == '\0')) {
        plStack_a8 = plStack_88;
        plStack_a0 = plStack_80;
        if (plStack_80 != (longlong *)0x0) {
          *(int *)(plStack_80 + 1) = (int)plStack_80[1] + 1;
        }
        if (&plStack_a8 != pplVar3) {
          plStack_a8 = *pplVar3;
          plStack_a0 = (longlong *)plVar21[0xe7];
          *pplVar3 = plStack_88;
          plVar21[0xe7] = (longlong)plStack_80;
        }
        plVar5 = plStack_a0;
        if (plStack_a0 != (longlong *)0x0) {
          plVar12 = plStack_a0 + 1;
          *(int *)plVar12 = (int)*plVar12 + -1;
          if ((int)*plVar12 == 0) {
            (**(code **)*plStack_a0)(plStack_a0);
            piVar14 = (int *)((longlong)plVar5 + 0xc);
            *piVar14 = *piVar14 + -1;
            if (*piVar14 == 0) {
              (**(code **)(*plVar5 + 8))(plVar5,1);
            }
          }
        }
        FUN_141eef450();
      }
      plVar5 = plStack_80;
      if (plStack_80 != (longlong *)0x0) {
        plVar12 = plStack_80 + 1;
        *(int *)plVar12 = (int)*plVar12 + -1;
        if ((int)*plVar12 == 0) {
          (**(code **)*plStack_80)(plStack_80);
          piVar14 = (int *)((longlong)plVar5 + 0xc);
          *piVar14 = *piVar14 + -1;
          if (*piVar14 == 0) {
            (**(code **)(*plVar5 + 8))(plVar5,1);
          }
        }
      }
    }
    if (puStack_78 != (undefined *)0x0) {
      FMemory_Free(puStack_78);
    }
    if (puStack_58 != (undefined *)0x0) {
      FMemory_Free(puStack_58);
    }
    if (puStack_68 != (undefined *)0x0) {
      FMemory_Free(puStack_68);
    }
    lVar17 = FUN_141df61e0();
    EnterCriticalSection(lVar17 + 0x1a0);
    FUN_1407bb180(lVar17 + 0x100,0);
    FUN_140992250(lVar17 + 0x150,0);
    LeaveCriticalSection(lVar17 + 0x1a0);
    FUN_14463f4b0(_DAT_148351eb8,1);
  }
  return;
}

