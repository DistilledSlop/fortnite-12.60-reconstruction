# Blocking callees (work list for the next Ghidra identification pass)

Generated from the screen of `porting/generated/decomp/` (exec records) and `porting/generated/decomp_improved/` (RPC records). The screen covers the 750 `portable` and `needs names` records. Counts use the rule `skip reason = unnamed_call` (the record calls at least one `FUN_*` that is not identified).

Identified helpers from `porting/ghidra/name_tables/helpers.json` (30 entries) are excluded from this list, so resolved callees are removed.

Columns: **blocked** = number of functions (of the 750) whose unnamed_call reason includes this callee; **alone** = number of those functions whose only unnamed callee is this one. Guesses come from the callee's own bytes (disassembled from the runtime dump with objdump, call targets, vtable slots, data references) and from call sites. Entries with no guess were not examined.

| # | callee | blocked | alone | example call site | guess (from the callee's code) |
|---:|---|---:|---:|---|---|
| 1 | 0x1406a9dc0 | 20 | 15 | `FUN_1406a9dc0(param_1 + 0x1250,auStackX_8);` | large helper (about 108 instructions, stack cookie, 7 calls, virtual 0x210); not classified |
| 2 | 0x141991bf0 | 7 | 0 | `lVar2 = FUN_141991bf0(lVar2);` | accessor: pointer at +0x1df8 if non-null, else +0x1df0 |
| 3 | 0x1419d3ff0 | 7 | 0 | `lVar2 = FUN_1419d3ff0();` | virtual call slot 0x150 then null test, then virtual 0xc50; lookup helper, not classified |
| 4 | 0x141a3c2b0 | 7 | 0 | `uVar1 = FUN_141a3c2b0(param_1,3,*(undefined4 *)(param_1 + 0xec0),` | large helper (about 110 instructions, virtual 0x770, calls 0x1406a9dc0); not classified |
| 5 | 0x141a3c390 | 7 | 0 | `FUN_141a3c390(param_1);` | helper calling 0x1407ac640 and virtuals 0x150/0x630/0x770; not classified |
| 6 | 0x1406aaec0 | 3 | 0 | `uVar2 = FUN_1406aaec0(auStack_c8,GameplayCueTag._tail_0x0);` | copies a 16-byte struct argument into a local, then calls 0x143e8a690 (low confidence) |
| 7 | 0x1406ae340 | 3 | 0 | `FUN_1406ae340(GameplayCueTag._tail_0x0);` | releases the buffer at +0x60 with FMemory_Free (destructor-style cleanup) |
| 8 | 0x142877fd0 | 3 | 0 | `lVar2 = FUN_142877fd0();` | not examined (call site only) |
| 9 | 0x142c64fe0 | 3 | 0 | `FUN_142c64fe0(param_3,uVar1);` | calls FMemory_Free and 0x142ba63b0/0x142c6d8a0; not classified |
| 10 | 0x143e8a690 | 3 | 0 | `FUN_143e8a690(&uStack_28);` | clears a TArray-like local (FMemory_Free on its data), calls 0x1406d9a30 (low confidence) |
| 11 | 0x14199e430 | 2 | 2 | `uVar1 = FUN_14199e430();` | playlist-info accessor: object at +0x1df8, calls 0x141ed2350, tests byte +0x2c8 |
| 12 | 0x1419ae980 | 2 | 2 | `FUN_1419ae980(param_1,puVar2,puVar3,auStackX_10[0]);` | location/state-colour helper, calls 0x141ed6bb0 and 0x142a3d9f0; not classified |
| 13 | 0x1440c99f0 | 2 | 2 | `FUN_1440c99f0(lVar1,*(undefined4 *)(lVar1 + 0x404),*(undefined4 *)(param_1 + 0x2964),1);` | float computation with virtuals 0x468/0x498/0x4c8/0x860; not classified |
| 14 | 0x14199e4e0 | 2 | 1 | `uVar1 = FUN_14199e4e0();` | same playlist-info accessor family as 0x14199e430 (flag test on the same object); low-medium |
| 15 | 0x1419c3880 | 2 | 1 | `uVar1 = FUN_1419c3880(param_1,puVar2);` | CanPlayerCreateInCurrentVolume helper: calls 0x141c74820 and virtuals; not classified |
| 16 | 0x1420ee770 | 2 | 1 | `FUN_1420ee770(*(longlong *)(param_1 + 0x280),&uStack_18);` | copies a 3-float vector from its argument into two member vectors (+0x22c, +0x1e2c) |
| 17 | 0x140f95ae0 | 2 | 0 | `uVar2 = FUN_140f95ae0();` | accessor chain: virtual 0x150 on [rcx], then the pointer at +0x188 of the result |
| 18 | 0x142872f60 | 2 | 0 | `lVar2 = FUN_142872f60();` | not examined (call site only) |
| 19 | 0x143e8a780 | 2 | 0 | `FUN_143e8a780(param_3,uVar1);` | builds a query/result object with FMemory_Free on a temporary; not classified |
| 20 | 0x144621fb0 | 2 | 0 | `lVar3 = FUN_144621fb0(uVar2,param_1 + 0x2250);` | gliding/redeploy helper with virtual 0x68; not classified |
| 21 | 0x140680300 | 1 | 1 | `uVar1 = FUN_140680300();` | not examined (call site only) |
| 22 | 0x1406f7390 | 1 | 1 | `uVar1 = FUN_1406f7390(param_1 + 0xbe0,0,0);` | not examined (call site only) |
| 23 | 0x14070c910 | 1 | 1 | `if ((this->AbilitySystemComponent != 0) && (lVar1 = FUN_14070c910(), lVar1 != 0)) {` | not examined (call site only) |
| 24 | 0x1417256f0 | 1 | 1 | `FUN_1417256f0(lVar1,(int)in_XMM1_Da,&FStack_38);` | not examined (call site only) |
| 25 | 0x141909530 | 1 | 1 | `FUN_141909530(auStackX_10);` | not examined (call site only) |
| 26 | 0x1419331a0 | 1 | 1 | `FUN_1419331a0(param_1,uStackX_10);` | not examined (call site only) |
| 27 | 0x14193dc90 | 1 | 1 | `FUN_14193dc90(param_1,uStackX_20,aiStackX_10[0] != 0);` | not examined (call site only) |
| 28 | 0x141946800 | 1 | 1 | `uVar1 = FUN_141946800(param_1,uStackX_10);` | not examined (call site only) |
| 29 | 0x141949f60 | 1 | 1 | `puVar1 = (undefined8 *)FUN_141949f60(param_1,auStackX_18,uStackX_10);` | not examined (call site only) |
| 30 | 0x1419520f0 | 1 | 1 | `FUN_1419520f0(param_1,uStackX_10);` | not examined (call site only) |
| 31 | 0x141952190 | 1 | 1 | `FUN_141952190(param_1,uStackX_10);` | not examined (call site only) |
| 32 | 0x141952500 | 1 | 1 | `FUN_141952500(param_1,uStackX_10);` | not examined (call site only) |
| 33 | 0x141952900 | 1 | 1 | `FUN_141952900(param_1,aiStackX_10[0] != 0);` | not examined (call site only) |
| 34 | 0x141953840 | 1 | 1 | `FUN_141953840(param_1,puVar1,auStackX_10[0]);` | not examined (call site only) |
| 35 | 0x14195e060 | 1 | 1 | `FUN_14195e060(uStack_10,uStack_14,uStack_18,auStackX_20[0],aiStackX_10[0] != 0);` | not examined (call site only) |
| 36 | 0x1419642e0 | 1 | 1 | `FUN_1419642e0(param_1,uStackX_10);` | not examined (call site only) |
| 37 | 0x141972f90 | 1 | 1 | `FUN_141972f90(uStackX_20,aiStackX_10[0] != 0);` | not examined (call site only) |
| 38 | 0x141989c50 | 1 | 1 | `uVar1 = FUN_141989c50();` | not examined (call site only) |
| 39 | 0x14198bac0 | 1 | 1 | `uVar1 = FUN_14198bac0();` | not examined (call site only) |
| 40 | 0x14198bce0 | 1 | 1 | `uVar1 = FUN_14198bce0();` | not examined (call site only) |
| 41 | 0x14198cd70 | 1 | 1 | `FUN_14198cd70(param_1,auStack_28[0],puVar2,puVar3);` | not examined (call site only) |
| 42 | 0x141990d90 | 1 | 1 | `uVar1 = FUN_141990d90(param_1,auStackX_10[0]);` | not examined (call site only) |
| 43 | 0x1419918d0 | 1 | 1 | `uVar1 = FUN_1419918d0(param_1,uStackX_10);` | not examined (call site only) |
| 44 | 0x141991c10 | 1 | 1 | `lVar1 = FUN_141991c10(this,1);` | not examined (call site only) |
| 45 | 0x141991ce0 | 1 | 1 | `puVar1 = (undefined8 *)FUN_141991ce0(param_1,auStackX_10);` | not examined (call site only) |
| 46 | 0x1419920d0 | 1 | 1 | `uVar1 = FUN_1419920d0();` | not examined (call site only) |
| 47 | 0x141995550 | 1 | 1 | `uVar1 = FUN_141995550();` | not examined (call site only) |
| 48 | 0x141995820 | 1 | 1 | `uVar1 = FUN_141995820(param_1,auStackX_18[0],auStackX_10[0]);` | not examined (call site only) |
| 49 | 0x141995900 | 1 | 1 | `uVar1 = FUN_141995900(param_1,uStackX_10);` | not examined (call site only) |
| 50 | 0x141995980 | 1 | 1 | `uVar1 = FUN_141995980(param_1,uStackX_10);` | not examined (call site only) |
| 51 | 0x141995b20 | 1 | 1 | `uVar1 = FUN_141995b20(param_1,uStackX_10);` | not examined (call site only) |
| 52 | 0x141995ba0 | 1 | 1 | `uVar1 = FUN_141995ba0(param_1,uStackX_10);` | not examined (call site only) |
| 53 | 0x141995c20 | 1 | 1 | `uVar1 = FUN_141995c20(param_1,uStackX_10);` | not examined (call site only) |
| 54 | 0x141995ca0 | 1 | 1 | `uVar1 = FUN_141995ca0(param_1,uStackX_10);` | not examined (call site only) |
| 55 | 0x141995d20 | 1 | 1 | `uVar1 = FUN_141995d20(param_1,uStackX_10);` | not examined (call site only) |
| 56 | 0x1419964d0 | 1 | 1 | `puVar1 = (undefined8 *)FUN_1419964d0(param_1,auStack_18);` | not examined (call site only) |
| 57 | 0x141996680 | 1 | 1 | `uVar1 = FUN_141996680(param_1,plVar2);` | not examined (call site only) |
| 58 | 0x1419967d0 | 1 | 1 | `uVar1 = FUN_1419967d0();` | not examined (call site only) |
| 59 | 0x141997f70 | 1 | 1 | `uVar1 = FUN_141997f70();` | not examined (call site only) |
| 60 | 0x141997fe0 | 1 | 1 | `uVar1 = FUN_141997fe0();` | not examined (call site only) |
| 61 | 0x1419980e0 | 1 | 1 | `plVar1 = (longlong *)FUN_1419980e0(param_1,alStack_28,auStackX_18[0],aiStackX_10[0] != 0);` | not examined (call site only) |
| 62 | 0x1419996f0 | 1 | 1 | `uVar1 = FUN_1419996f0();` | not examined (call site only) |
| 63 | 0x1419997b0 | 1 | 1 | `puVar1 = (undefined8 *)FUN_1419997b0(param_1,auStack_18);` | not examined (call site only) |
| 64 | 0x14199a6f0 | 1 | 1 | `FUN_14199a6f0(param_1,auStackX_20[0],auStackX_10[0]);` | not examined (call site only) |
| 65 | 0x14199a730 | 1 | 1 | `FUN_14199a730(param_1,auStack_18[0],auStackX_20[0],auStackX_10[0]);` | not examined (call site only) |
| 66 | 0x14199abb0 | 1 | 1 | `FUN_14199abb0(param_1,uStackX_10);` | not examined (call site only) |
| 67 | 0x14199abd0 | 1 | 1 | `FUN_14199abd0(param_1,auStackX_20[0],auStackX_10[0]);` | not examined (call site only) |
| 68 | 0x14199cab0 | 1 | 1 | `FUN_14199cab0(param_1,auStackX_10[0],bVar1,aiStack_28[0] != 0);` | not examined (call site only) |
| 69 | 0x14199e2c0 | 1 | 1 | `uVar1 = FUN_14199e2c0();` | not examined (call site only) |
| 70 | 0x14199e380 | 1 | 1 | `uVar1 = FUN_14199e380();` | not examined (call site only) |
| 71 | 0x14199e3a0 | 1 | 1 | `uVar1 = FUN_14199e3a0();` | not examined (call site only) |
| 72 | 0x14199e3e0 | 1 | 1 | `uVar1 = FUN_14199e3e0();` | not examined (call site only) |
| 73 | 0x14199e480 | 1 | 1 | `uVar1 = FUN_14199e480();` | not examined (call site only) |
| 74 | 0x14199e4a0 | 1 | 1 | `uVar1 = FUN_14199e4a0(param_1,auStackX_10[0]);` | not examined (call site only) |
| 75 | 0x14199e4c0 | 1 | 1 | `uVar1 = FUN_14199e4c0();` | not examined (call site only) |
| 76 | 0x14199e700 | 1 | 1 | `uVar1 = FUN_14199e700(param_1,uStackX_10);` | not examined (call site only) |
| 77 | 0x14199e830 | 1 | 1 | `uVar1 = FUN_14199e830();` | not examined (call site only) |
| 78 | 0x14199ea80 | 1 | 1 | `uVar1 = FUN_14199ea80(param_1,uStackX_10);` | not examined (call site only) |
| 79 | 0x14199ee50 | 1 | 1 | `uVar1 = FUN_14199ee50(param_1,uStackX_10);` | not examined (call site only) |
| 80 | 0x14199f050 | 1 | 1 | `uVar1 = FUN_14199f050(param_1,auStackX_18[0],auStackX_10[0]);` | not examined (call site only) |
| 81 | 0x1419a65b0 | 1 | 1 | `FUN_1419a65b0(param_1,auStackX_10[0]);` | not examined (call site only) |
| 82 | 0x1419a81d0 | 1 | 1 | `FUN_1419a81d0(param_1,puVar1);` | not examined (call site only) |
| 83 | 0x1419a8740 | 1 | 1 | `FUN_1419a8740(param_1,uStackX_20,auStackX_10[0]);` | not examined (call site only) |
| 84 | 0x1419a8b50 | 1 | 1 | `FUN_1419a8b50(param_1,puVar1,auStack_28[0],auStackX_10[0]);` | not examined (call site only) |
| 85 | 0x1419ae140 | 1 | 1 | `FUN_1419ae140(param_1,puVar1,auStackX_10[0]);` | not examined (call site only) |
| 86 | 0x1419ae6f0 | 1 | 1 | `FUN_1419ae6f0(param_1,uStack_38,&uStack_18,auStackX_20[0],auStackX_10[0]);` | not examined (call site only) |
| 87 | 0x1419ae890 | 1 | 1 | `FUN_1419ae890(param_1,puVar1,&uStack_18);` | not examined (call site only) |
| 88 | 0x1419aec90 | 1 | 1 | `FUN_1419aec90(param_1,puVar1,&uStack_18);` | not examined (call site only) |
| 89 | 0x1419b0350 | 1 | 1 | `uVar1 = FUN_1419b0350();` | not examined (call site only) |
| 90 | 0x1419b0720 | 1 | 1 | `uVar1 = FUN_1419b0720();` | not examined (call site only) |
| 91 | 0x1419b09b0 | 1 | 1 | `uVar1 = FUN_1419b09b0(param_1,uStackX_10);` | not examined (call site only) |
| 92 | 0x1419b0cc0 | 1 | 1 | `uVar1 = FUN_1419b0cc0();` | not examined (call site only) |
| 93 | 0x1419bfa60 | 1 | 1 | `FUN_1419bfa60(param_1,puVar1);` | not examined (call site only) |
| 94 | 0x1419c2480 | 1 | 1 | `FUN_1419c2480(param_1,uStackX_10);` | not examined (call site only) |
| 95 | 0x1419c2de0 | 1 | 1 | `uVar1 = FUN_1419c2de0();` | not examined (call site only) |
| 96 | 0x1419c3fb0 | 1 | 1 | `FUN_1419c3fb0(param_1,auStackX_10[0]);` | not examined (call site only) |
| 97 | 0x1419d3010 | 1 | 1 | `uVar1 = FUN_1419d3010();` | not examined (call site only) |
| 98 | 0x1419d3600 | 1 | 1 | `uVar1 = FUN_1419d3600();` | not examined (call site only) |
| 99 | 0x1419d5220 | 1 | 1 | `uVar1 = FUN_1419d5220();` | not examined (call site only) |
| 100 | 0x1419d5380 | 1 | 1 | `uVar1 = FUN_1419d5380();` | not examined (call site only) |
| 101 | 0x1419d5470 | 1 | 1 | `uVar1 = FUN_1419d5470();` | not examined (call site only) |
| 102 | 0x1419d5f70 | 1 | 1 | `uVar1 = FUN_1419d5f70();` | not examined (call site only) |
| 103 | 0x1419d5f80 | 1 | 1 | `FUN_1419d5f80(param_1,puVar1,puVar2,puVar3);` | not examined (call site only) |
| 104 | 0x1419d60f0 | 1 | 1 | `uVar1 = FUN_1419d60f0(param_1,aiStackX_10[0] != 0);` | not examined (call site only) |
| 105 | 0x1419d8d90 | 1 | 1 | `FUN_1419d8d90(param_1,plVar1);` | not examined (call site only) |
| 106 | 0x1419d8e40 | 1 | 1 | `uVar1 = FUN_1419d8e40();` | not examined (call site only) |
| 107 | 0x1419d8fc0 | 1 | 1 | `plVar1 = (longlong *)FUN_1419d8fc0(param_1,alStack_18,aiStackX_10[0] != 0);` | not examined (call site only) |
| 108 | 0x1419da900 | 1 | 1 | `FUN_1419da900(param_1,auStackX_10[0]);` | not examined (call site only) |
| 109 | 0x1419da970 | 1 | 1 | `FUN_1419da970(param_1,puVar1,auStackX_10[0]);` | not examined (call site only) |
| 110 | 0x1419db530 | 1 | 1 | `FUN_1419db530(param_1,auStackX_10[0]);` | not examined (call site only) |
| 111 | 0x1419dc720 | 1 | 1 | `FUN_1419dc720(param_1,auStackX_10[0]);` | not examined (call site only) |
| 112 | 0x1419dd6b0 | 1 | 1 | `uVar1 = FUN_1419dd6b0();` | not examined (call site only) |
| 113 | 0x1419dd760 | 1 | 1 | `uVar1 = FUN_1419dd760();` | not examined (call site only) |
| 114 | 0x1419dd820 | 1 | 1 | `uVar1 = FUN_1419dd820();` | not examined (call site only) |
| 115 | 0x1419dd8b0 | 1 | 1 | `uVar1 = FUN_1419dd8b0();` | not examined (call site only) |
| 116 | 0x1419dd950 | 1 | 1 | `uVar1 = FUN_1419dd950();` | not examined (call site only) |
| 117 | 0x1419dd990 | 1 | 1 | `uVar1 = FUN_1419dd990();` | not examined (call site only) |
| 118 | 0x1419dd9e0 | 1 | 1 | `uVar1 = FUN_1419dd9e0();` | not examined (call site only) |
| 119 | 0x1419dda50 | 1 | 1 | `uVar1 = FUN_1419dda50();` | not examined (call site only) |
| 120 | 0x1419ddaa0 | 1 | 1 | `uVar1 = FUN_1419ddaa0();` | not examined (call site only) |
| 121 | 0x1419dddb0 | 1 | 1 | `uVar1 = FUN_1419dddb0();` | not examined (call site only) |
| 122 | 0x1419dde10 | 1 | 1 | `uVar1 = FUN_1419dde10();` | not examined (call site only) |
| 123 | 0x1419dde30 | 1 | 1 | `uVar1 = FUN_1419dde30(param_1,puVar2,puVar3,aiStackX_20[0] != 0);` | not examined (call site only) |
| 124 | 0x1419e0370 | 1 | 1 | `FUN_1419e0370(param_1,uStackX_20,uStackX_10);` | not examined (call site only) |
| 125 | 0x1419e17b0 | 1 | 1 | `FUN_1419e17b0(param_1,uStackX_10);` | not examined (call site only) |
| 126 | 0x1419e1810 | 1 | 1 | `FUN_1419e1810(param_1,auStackX_10[0]);` | not examined (call site only) |
| 127 | 0x1419ee720 | 1 | 1 | `FUN_1419ee720(param_1,aiStackX_10[0] != 0);` | not examined (call site only) |
| 128 | 0x1419f2490 | 1 | 1 | `FUN_1419f2490(param_1,&uStack_18,&uStack_28,auStackX_10[0]);` | not examined (call site only) |
| 129 | 0x1419f3830 | 1 | 1 | `FUN_1419f3830(param_1,aiStackX_10[0] != 0);` | not examined (call site only) |
| 130 | 0x1419f3e70 | 1 | 1 | `FUN_1419f3e70(param_1,uStackX_20,&uStack_18,auStackX_10[0]);` | not examined (call site only) |
| 131 | 0x1419f3fa0 | 1 | 1 | `FUN_1419f3fa0(param_1,uStack_38,&uStack_18,auStackX_20[0],auStackX_10[0]);` | not examined (call site only) |
| 132 | 0x1419f5920 | 1 | 1 | `FUN_1419f5920(param_1,&uStack_18,&uStack_28,&uStack_38,aiStackX_10[0] != 0);` | not examined (call site only) |
| 133 | 0x1419fb550 | 1 | 1 | `FUN_1419fb550(param_1,alStack_18,puVar1);` | not examined (call site only) |
| 134 | 0x1419fb9d0 | 1 | 1 | `FUN_1419fb9d0(param_1,alStack_18);` | not examined (call site only) |
| 135 | 0x141a0d850 | 1 | 1 | `uVar1 = FUN_141a0d850(param_1,puVar2);` | not examined (call site only) |
| 136 | 0x141a0faa0 | 1 | 1 | `FUN_141a0faa0(param_1,aiStackX_10[0] != 0);` | not examined (call site only) |
| 137 | 0x141a12300 | 1 | 1 | `FUN_141a12300(param_1,puVar1,auStackX_10[0]);` | not examined (call site only) |
| 138 | 0x141a13080 | 1 | 1 | `uVar1 = FUN_141a13080();` | not examined (call site only) |
| 139 | 0x141a132b0 | 1 | 1 | `uVar1 = FUN_141a132b0();` | not examined (call site only) |
| 140 | 0x141a189c0 | 1 | 1 | `uVar1 = FUN_141a189c0(param_1,auStackX_10[0]);` | not examined (call site only) |
| 141 | 0x141a18ce0 | 1 | 1 | `uVar1 = FUN_141a18ce0();` | not examined (call site only) |
| 142 | 0x141a23e00 | 1 | 1 | `FUN_141a23e00(param_1,auStackX_10[0]);` | not examined (call site only) |
| 143 | 0x141a23e40 | 1 | 1 | `FUN_141a23e40(param_1,auStackX_10[0]);` | not examined (call site only) |
| 144 | 0x141a23e80 | 1 | 1 | `uVar1 = FUN_141a23e80(param_1,aiStackX_10[0] != 0);` | not examined (call site only) |
| 145 | 0x141a24c60 | 1 | 1 | `FUN_141a24c60(param_1,auStackX_10[0]);` | not examined (call site only) |
| 146 | 0x141a25830 | 1 | 1 | `FUN_141a25830(param_1,uStackX_10);` | not examined (call site only) |
| 147 | 0x141a267c0 | 1 | 1 | `FUN_141a267c0(param_1,auStackX_10[0]);` | not examined (call site only) |
| 148 | 0x141a3c6f0 | 1 | 1 | `uVar1 = FUN_141a3c6f0();` | not examined (call site only) |
| 149 | 0x141a3c850 | 1 | 1 | `uVar1 = FUN_141a3c850();` | not examined (call site only) |
| 150 | 0x141a45bb0 | 1 | 1 | `puVar2 = (undefined8 *)FUN_141a45bb0(param_1,auStack_18);` | not examined (call site only) |
| 151 | 0x141a48170 | 1 | 1 | `uVar1 = FUN_141a48170();` | not examined (call site only) |
| 152 | 0x141a4aa10 | 1 | 1 | `uVar1 = FUN_141a4aa10();` | not examined (call site only) |
| 153 | 0x141a4b2d0 | 1 | 1 | `uVar1 = FUN_141a4b2d0();` | not examined (call site only) |
| 154 | 0x141a4b2e0 | 1 | 1 | `uVar1 = FUN_141a4b2e0();` | not examined (call site only) |
| 155 | 0x141a4b3a0 | 1 | 1 | `uVar1 = FUN_141a4b3a0();` | not examined (call site only) |
| 156 | 0x141a519d0 | 1 | 1 | `FUN_141a519d0(param_1,auStackX_10[0]);` | not examined (call site only) |
| 157 | 0x141a52fa0 | 1 | 1 | `FUN_141a52fa0(param_1,aiStackX_10[0] != 0);` | not examined (call site only) |
| 158 | 0x141a5f1c0 | 1 | 1 | `uVar1 = FUN_141a5f1c0(plVar2,aiStackX_10[0] != 0);` | not examined (call site only) |
| 159 | 0x142301790 | 1 | 1 | `FUN_142301790(param_1,uStack_60,auStackX_10[0],uStack_68,uStack_70,&uStack_28,uStack_78...` | not examined (call site only) |
| 160 | 0x142301b90 | 1 | 1 | `FUN_142301b90(param_1,uStackX_10);` | not examined (call site only) |
| 161 | 0x142301d90 | 1 | 1 | `FUN_142301d90(param_1,uStackX_20,uStackX_10);` | not examined (call site only) |
| 162 | 0x142302250 | 1 | 1 | `FUN_142302250(param_1,uStackX_20,auStackX_10[0]);` | not examined (call site only) |
| 163 | 0x142302420 | 1 | 1 | `FUN_142302420(param_1,puVar1);` | not examined (call site only) |
| 164 | 0x142302e10 | 1 | 1 | `FUN_142302e10(param_1,uStackX_10);` | not examined (call site only) |
| 165 | 0x142303f70 | 1 | 1 | `FUN_142303f70(param_1,puVar1);` | not examined (call site only) |
| 166 | 0x142304350 | 1 | 1 | `uVar1 = FUN_142304350();` | not examined (call site only) |
| 167 | 0x1423043f0 | 1 | 1 | `uVar1 = FUN_1423043f0(param_1,uStackX_10);` | not examined (call site only) |
| 168 | 0x142304420 | 1 | 1 | `FUN_142304420(param_1,auStack_18[0],uStackX_20,uStackX_10);` | not examined (call site only) |
| 169 | 0x1423055b0 | 1 | 1 | `FUN_1423055b0(param_1,auStackX_20[0],auStackX_10[0],&uStack_18);` | not examined (call site only) |
| 170 | 0x142305f70 | 1 | 1 | `FUN_142305f70(param_1,puVar1);` | not examined (call site only) |
| 171 | 0x1423061d0 | 1 | 1 | `FUN_1423061d0(param_1,aiStackX_10[0] != 0);` | not examined (call site only) |
| 172 | 0x1423074a0 | 1 | 1 | `FUN_1423074a0(param_1,uStackX_20,uStackX_10);` | not examined (call site only) |
| 173 | 0x14230c280 | 1 | 1 | `uVar1 = FUN_14230c280();` | not examined (call site only) |
| 174 | 0x14230c630 | 1 | 1 | `uVar1 = FUN_14230c630();` | not examined (call site only) |
| 175 | 0x14230c690 | 1 | 1 | `uVar1 = FUN_14230c690();` | not examined (call site only) |
| 176 | 0x14230c8b0 | 1 | 1 | `uVar1 = FUN_14230c8b0();` | not examined (call site only) |
| 177 | 0x142310b50 | 1 | 1 | `FUN_142310b50(param_1,uStackX_10);` | not examined (call site only) |
| 178 | 0x1423110f0 | 1 | 1 | `FUN_1423110f0(param_1,auStackX_20[0],aiStackX_10[0] != 0);` | not examined (call site only) |
| 179 | 0x142312070 | 1 | 1 | `FUN_142312070(param_1,uStackX_10);` | not examined (call site only) |
| 180 | 0x142314600 | 1 | 1 | `FUN_142314600(param_1,aiStackX_10[0] != 0);` | not examined (call site only) |
| 181 | 0x142314d70 | 1 | 1 | `cVar1 = FUN_142314d70();` | not examined (call site only) |
| 182 | 0x1423150b0 | 1 | 1 | `uVar1 = FUN_1423150b0(param_1,uStackX_10,&uStack_18);` | not examined (call site only) |
| 183 | 0x1423178a0 | 1 | 1 | `FUN_1423178a0(param_1,uStackX_10,auStack_18[0],uStackX_20);` | not examined (call site only) |
| 184 | 0x142317bb0 | 1 | 1 | `FUN_142317bb0(param_1,auStack_28[0],bVar1,aiStackX_20[0] != 0);` | not examined (call site only) |
| 185 | 0x142318e80 | 1 | 1 | `uVar1 = FUN_142318e80();` | not examined (call site only) |
| 186 | 0x142318ec0 | 1 | 1 | `uVar1 = FUN_142318ec0();` | not examined (call site only) |
| 187 | 0x1423192c0 | 1 | 1 | `uVar1 = FUN_1423192c0(param_1,uStackX_10);` | not examined (call site only) |
| 188 | 0x142319870 | 1 | 1 | `uVar1 = FUN_142319870();` | not examined (call site only) |
| 189 | 0x142319f60 | 1 | 1 | `uVar1 = FUN_142319f60();` | not examined (call site only) |
| 190 | 0x14231a050 | 1 | 1 | `uVar1 = FUN_14231a050();` | not examined (call site only) |
| 191 | 0x14231a240 | 1 | 1 | `plVar1 = (longlong *)FUN_14231a240(param_1,alStack_28,auStackX_18[0],auStackX_10[0]);` | not examined (call site only) |
| 192 | 0x14231a330 | 1 | 1 | `plVar1 = (longlong *)FUN_14231a330(param_1,alStack_18,uStackX_10);` | not examined (call site only) |
| 193 | 0x14231a530 | 1 | 1 | `uVar1 = FUN_14231a530();` | not examined (call site only) |
| 194 | 0x14231a940 | 1 | 1 | `lVar1 = FUN_14231a940();` | not examined (call site only) |
| 195 | 0x14231a960 | 1 | 1 | `uVar1 = FUN_14231a960();` | not examined (call site only) |
| 196 | 0x14231b600 | 1 | 1 | `uVar1 = FUN_14231b600();` | not examined (call site only) |
| 197 | 0x14231c0e0 | 1 | 1 | `FUN_14231c0e0(param_1,puVar1,puVar2);` | not examined (call site only) |
| 198 | 0x14231e630 | 1 | 1 | `uVar1 = FUN_14231e630();` | not examined (call site only) |
| 199 | 0x14231e680 | 1 | 1 | `puVar1 = (undefined8 *)FUN_14231e680(param_1,auStackX_10);` | not examined (call site only) |
| 200 | 0x14231e6e0 | 1 | 1 | `uVar1 = FUN_14231e6e0();` | not examined (call site only) |
| 201 | 0x14231f1f0 | 1 | 1 | `uVar1 = FUN_14231f1f0();` | not examined (call site only) |
| 202 | 0x14231f210 | 1 | 1 | `uVar1 = FUN_14231f210();` | not examined (call site only) |
| 203 | 0x14231f2a0 | 1 | 1 | `FUN_14231f2a0(param_1,puVar1,puVar2);` | not examined (call site only) |
| 204 | 0x142321a70 | 1 | 1 | `uVar1 = FUN_142321a70();` | not examined (call site only) |
| 205 | 0x142321b10 | 1 | 1 | `uVar1 = FUN_142321b10();` | not examined (call site only) |
| 206 | 0x142321b30 | 1 | 1 | `uVar1 = FUN_142321b30();` | not examined (call site only) |
| 207 | 0x142321e10 | 1 | 1 | `uVar1 = FUN_142321e10();` | not examined (call site only) |
| 208 | 0x142321e70 | 1 | 1 | `uVar1 = FUN_142321e70();` | not examined (call site only) |
| 209 | 0x142321eb0 | 1 | 1 | `uVar1 = FUN_142321eb0(param_1,aiStackX_10[0] != 0);` | not examined (call site only) |
| 210 | 0x142322e60 | 1 | 1 | `puVar1 = (undefined8 *)FUN_142322e60(param_1,auStack_18,auStackX_10[0]);` | not examined (call site only) |
| 211 | 0x142322ef0 | 1 | 1 | `uVar1 = FUN_142322ef0();` | not examined (call site only) |
| 212 | 0x142323330 | 1 | 1 | `uVar1 = FUN_142323330();` | not examined (call site only) |
| 213 | 0x142323570 | 1 | 1 | `uVar1 = FUN_142323570(param_1,auStackX_10[0]);` | not examined (call site only) |
| 214 | 0x142323620 | 1 | 1 | `uVar1 = FUN_142323620();` | not examined (call site only) |
| 215 | 0x142323a90 | 1 | 1 | `uVar1 = FUN_142323a90();` | not examined (call site only) |
| 216 | 0x142323e00 | 1 | 1 | `uVar2 = FUN_142323e00(param_1,plVar1,auStackX_10[0]);` | not examined (call site only) |
| 217 | 0x142324750 | 1 | 1 | `FUN_142324750(param_1,puVar1,puVar2);` | not examined (call site only) |
| 218 | 0x1423247c0 | 1 | 1 | `puVar1 = (undefined8 *)FUN_1423247c0(param_1,auStack_18);` | not examined (call site only) |
| 219 | 0x1423249d0 | 1 | 1 | `FUN_1423249d0(param_1,puVar2,puVar3,puVar4);` | not examined (call site only) |
| 220 | 0x142324c10 | 1 | 1 | `uVar1 = FUN_142324c10();` | not examined (call site only) |
| 221 | 0x1423254c0 | 1 | 1 | `uVar1 = FUN_1423254c0();` | not examined (call site only) |
| 222 | 0x142325510 | 1 | 1 | `uVar1 = FUN_142325510();` | not examined (call site only) |
| 223 | 0x1423256f0 | 1 | 1 | `uVar1 = FUN_1423256f0();` | not examined (call site only) |
| 224 | 0x142325700 | 1 | 1 | `uVar1 = FUN_142325700();` | not examined (call site only) |
| 225 | 0x142325930 | 1 | 1 | `uVar1 = FUN_142325930();` | not examined (call site only) |
| 226 | 0x14232ca60 | 1 | 1 | `uVar1 = FUN_14232ca60();` | not examined (call site only) |
| 227 | 0x14232d220 | 1 | 1 | `FUN_14232d220(InDBNOHoistee,this,1);` | not examined (call site only) |
| 228 | 0x14232f830 | 1 | 1 | `FUN_14232f830(param_1,&uStack_18,&uStack_28);` | not examined (call site only) |
| 229 | 0x1423308a0 | 1 | 1 | `FUN_1423308a0(param_1,puVar1);` | not examined (call site only) |
| 230 | 0x142330b00 | 1 | 1 | `FUN_142330b00(param_1,uStackX_20,uStackX_10);` | not examined (call site only) |
| 231 | 0x142330d30 | 1 | 1 | `FUN_142330d30(param_1,puVar1,auStackX_10[0]);` | not examined (call site only) |
| 232 | 0x142331000 | 1 | 1 | `uVar1 = FUN_142331000();` | not examined (call site only) |
| 233 | 0x142331040 | 1 | 1 | `uVar1 = FUN_142331040();` | not examined (call site only) |
| 234 | 0x142331080 | 1 | 1 | `uVar1 = FUN_142331080();` | not examined (call site only) |
| 235 | 0x142331100 | 1 | 1 | `uVar1 = FUN_142331100();` | not examined (call site only) |
| 236 | 0x142331180 | 1 | 1 | `uVar1 = FUN_142331180();` | not examined (call site only) |
| 237 | 0x142331230 | 1 | 1 | `uVar1 = FUN_142331230();` | not examined (call site only) |
| 238 | 0x1423312f0 | 1 | 1 | `uVar1 = FUN_1423312f0(param_1,puVar2);` | not examined (call site only) |
| 239 | 0x1423318f0 | 1 | 1 | `uVar1 = FUN_1423318f0();` | not examined (call site only) |
| 240 | 0x142331900 | 1 | 1 | `uVar1 = FUN_142331900();` | not examined (call site only) |
| 241 | 0x142331920 | 1 | 1 | `uVar1 = FUN_142331920();` | not examined (call site only) |
| 242 | 0x142331940 | 1 | 1 | `uVar1 = FUN_142331940();` | not examined (call site only) |
| 243 | 0x142331bc0 | 1 | 1 | `uVar1 = FUN_142331bc0();` | not examined (call site only) |
| 244 | 0x142331c20 | 1 | 1 | `uVar1 = FUN_142331c20();` | not examined (call site only) |
| 245 | 0x142331fc0 | 1 | 1 | `uVar1 = FUN_142331fc0();` | not examined (call site only) |
| 246 | 0x1423320a0 | 1 | 1 | `uVar1 = FUN_1423320a0();` | not examined (call site only) |
| 247 | 0x1423320c0 | 1 | 1 | `uVar1 = FUN_1423320c0();` | not examined (call site only) |
| 248 | 0x142332130 | 1 | 1 | `uVar1 = FUN_142332130();` | not examined (call site only) |
| 249 | 0x142332260 | 1 | 1 | `uVar1 = FUN_142332260();` | not examined (call site only) |
| 250 | 0x142332270 | 1 | 1 | `uVar1 = FUN_142332270();` | not examined (call site only) |
| 251 | 0x142332c40 | 1 | 1 | `FUN_142332c40(param_1,&uStack_38,bVar1,bVar2,bVar3,iStack_54 != 0);` | not examined (call site only) |
| 252 | 0x142332da0 | 1 | 1 | `FUN_142332da0(param_1,alStack_18);` | not examined (call site only) |
| 253 | 0x142333b40 | 1 | 1 | `FUN_142333b40(param_1,uStackX_10);` | not examined (call site only) |
| 254 | 0x1423355c0 | 1 | 1 | `FUN_1423355c0();` | not examined (call site only) |
| 255 | 0x1423366f0 | 1 | 1 | `FUN_1423366f0(param_1,auStack_b8[0],uStack_c0,uStack_c8,auStackX_20[0],bVar2,puVar1);` | not examined (call site only) |
| 256 | 0x142339d20 | 1 | 1 | `FUN_142339d20(param_1,puVar1);` | not examined (call site only) |
| 257 | 0x14233c8a0 | 1 | 1 | `FUN_14233c8a0(param_1,aiStackX_10[0] != 0);` | not examined (call site only) |
| 258 | 0x14233cd80 | 1 | 1 | `FUN_14233cd80(param_1,aiStackX_10[0] != 0);` | not examined (call site only) |
| 259 | 0x142341760 | 1 | 1 | `uVar1 = FUN_142341760(param_1,auStack_28[0],auStackX_18[0],uStackX_20,aiStackX_10[0] !=...` | not examined (call site only) |
| 260 | 0x142342190 | 1 | 1 | `FUN_142342190(param_1,auStackX_10[0]);` | not examined (call site only) |
| 261 | 0x142342300 | 1 | 1 | `FUN_142342300(param_1,uStackX_10);` | not examined (call site only) |
| 262 | 0x142345ca0 | 1 | 1 | `FUN_142345ca0(param_1,auStackX_10[0],&uStack_18);` | not examined (call site only) |
| 263 | 0x142345f30 | 1 | 1 | `FUN_142345f30(param_1,alStack_18);` | not examined (call site only) |
| 264 | 0x142347a60 | 1 | 1 | `FUN_142347a60(param_1,uStackX_10);` | not examined (call site only) |
| 265 | 0x142347cf0 | 1 | 1 | `FUN_142347cf0(param_1,puVar1);` | not examined (call site only) |
| 266 | 0x1423483e0 | 1 | 1 | `FUN_1423483e0(param_1,puVar1);` | not examined (call site only) |
| 267 | 0x1423486d0 | 1 | 1 | `FUN_1423486d0(param_1,uStackX_10);` | not examined (call site only) |
| 268 | 0x14234a5e0 | 1 | 1 | `uVar1 = FUN_14234a5e0(param_1,auStackX_10[0]);` | not examined (call site only) |
| 269 | 0x14234b110 | 1 | 1 | `FUN_14234b110(param_1,uStackX_10);` | not examined (call site only) |
| 270 | 0x14234b520 | 1 | 1 | `FUN_14234b520(param_1,uStackX_10);` | not examined (call site only) |
| 271 | 0x14234b600 | 1 | 1 | `FUN_14234b600(param_1,puVar1,auStackX_10[0]);` | not examined (call site only) |
| 272 | 0x14234beb0 | 1 | 1 | `FUN_14234beb0(param_1,auStackX_10[0]);` | not examined (call site only) |
| 273 | 0x14234c790 | 1 | 1 | `FUN_14234c790(param_1,&lStack_18);` | not examined (call site only) |
| 274 | 0x14234c7b0 | 1 | 1 | `FUN_14234c7b0(param_1,puVar3,puVar2,uStack_48,uStack_50,uStack_58,uStackX_20);` | not examined (call site only) |
| 275 | 0x14234c9d0 | 1 | 1 | `FUN_14234c9d0(param_1,auStackX_10[0]);` | not examined (call site only) |
| 276 | 0x14234cbd0 | 1 | 1 | `FUN_14234cbd0(param_1,auStackX_10[0]);` | not examined (call site only) |
| 277 | 0x14234cc80 | 1 | 1 | `FUN_14234cc80(param_1,auStack_10[0],uStack_14,uStack_18,auStackX_20[0],auStackX_10[0]);` | not examined (call site only) |
| 278 | 0x14234cde0 | 1 | 1 | `FUN_14234cde0(param_1,auStackX_10[0]);` | not examined (call site only) |
| 279 | 0x14234ce40 | 1 | 1 | `FUN_14234ce40(param_1,auStackX_10[0]);` | not examined (call site only) |
| 280 | 0x14234d380 | 1 | 1 | `FUN_14234d380(param_1,uStackX_10);` | not examined (call site only) |
| 281 | 0x14234d620 | 1 | 1 | `FUN_14234d620(param_1,aiStackX_10[0] != 0);` | not examined (call site only) |
| 282 | 0x14234dac0 | 1 | 1 | `FUN_14234dac0(param_1,aiStackX_10[0] != 0);` | not examined (call site only) |
| 283 | 0x14234db60 | 1 | 1 | `FUN_14234db60(param_1,auStackX_10[0]);` | not examined (call site only) |
| 284 | 0x14234dbc0 | 1 | 1 | `FUN_14234dbc0(param_1,auStackX_10[0]);` | not examined (call site only) |
| 285 | 0x14234dc20 | 1 | 1 | `FUN_14234dc20(param_1,auStackX_10[0]);` | not examined (call site only) |
| 286 | 0x14234df40 | 1 | 1 | `FUN_14234df40(param_1,auStackX_10[0]);` | not examined (call site only) |
| 287 | 0x14234dfa0 | 1 | 1 | `FUN_14234dfa0(param_1,auStackX_10[0]);` | not examined (call site only) |
| 288 | 0x14234e050 | 1 | 1 | `FUN_14234e050(param_1,&uStack_18);` | not examined (call site only) |
| 289 | 0x14234e8b0 | 1 | 1 | `FUN_14234e8b0(param_1,aiStackX_10[0] != 0);` | not examined (call site only) |
| 290 | 0x14234e8d0 | 1 | 1 | `FUN_14234e8d0(param_1,uStackX_10);` | not examined (call site only) |
| 291 | 0x14234ec60 | 1 | 1 | `FUN_14234ec60(param_1,uStackX_10);` | not examined (call site only) |
| 292 | 0x14234eee0 | 1 | 1 | `FUN_14234eee0(param_1,auStackX_10[0]);` | not examined (call site only) |
| 293 | 0x14234ef40 | 1 | 1 | `FUN_14234ef40(param_1,auStackX_10[0]);` | not examined (call site only) |
| 294 | 0x14234efe0 | 1 | 1 | `FUN_14234efe0(param_1,auStackX_10[0]);` | not examined (call site only) |
| 295 | 0x14234f070 | 1 | 1 | `FUN_14234f070(param_1,aiStackX_10[0] != 0);` | not examined (call site only) |
| 296 | 0x142350610 | 1 | 1 | `uVar1 = FUN_142350610();` | not examined (call site only) |
| 297 | 0x142350960 | 1 | 1 | `uVar1 = FUN_142350960();` | not examined (call site only) |
| 298 | 0x142351740 | 1 | 1 | `uVar1 = FUN_142351740(param_1,uStackX_10);` | not examined (call site only) |
| 299 | 0x1423517d0 | 1 | 1 | `uVar1 = FUN_1423517d0(param_1,uStackX_20,uStackX_18,puVar2,aiStackX_10[0] != 0);` | not examined (call site only) |
| 300 | 0x142351a10 | 1 | 1 | `uVar1 = FUN_142351a10(param_1,uStackX_20,uStackX_18,puVar2,aiStackX_10[0] != 0);` | not examined (call site only) |
| 301 | 0x142351d50 | 1 | 1 | `uVar1 = FUN_142351d50(param_1,uStack_70,&uStack_58,auStackX_18[0],auStackX_10[0],uStack...` | not examined (call site only) |
| 302 | 0x142352d70 | 1 | 1 | `FUN_142352d70(param_1,auStackX_10[0]);` | not examined (call site only) |
| 303 | 0x142353080 | 1 | 1 | `FUN_142353080(param_1,&uStack_18,auStackX_10[0]);` | not examined (call site only) |
| 304 | 0x1423532a0 | 1 | 1 | `FUN_1423532a0(param_1,auStack_28[0],bVar1,aiStackX_20[0] != 0);` | not examined (call site only) |
| 305 | 0x1423535a0 | 1 | 1 | `FUN_1423535a0(param_1,aiStackX_10[0] != 0);` | not examined (call site only) |
| 306 | 0x1423562b0 | 1 | 1 | `uVar1 = FUN_1423562b0();` | not examined (call site only) |
| 307 | 0x142356fc0 | 1 | 1 | `FUN_142356fc0(param_1,uStackX_20,uStack_10,uStack_18,auStackX_10[0]);` | not examined (call site only) |
| 308 | 0x142358750 | 1 | 1 | `FUN_142358750(param_1,bVar2,plVar1);` | not examined (call site only) |
| 309 | 0x14235bd20 | 1 | 1 | `FUN_14235bd20(param_1,auStackX_10[0],&uStack_18,&uStack_28);` | not examined (call site only) |
| 310 | 0x14235cae0 | 1 | 1 | `uVar1 = FUN_14235cae0(param_1,auStackX_10[0]);` | not examined (call site only) |
| 311 | 0x14243a470 | 1 | 1 | `FUN_14243a470(uVar2);` | not examined (call site only) |
| 312 | 0x14277c110 | 1 | 1 | `FUN_14277c110();` | not examined (call site only) |
| 313 | 0x142a18a00 | 1 | 1 | `FUN_142a18a00(param_1,iVar2);` | not examined (call site only) |
| 314 | 0x142a75f10 | 1 | 1 | `lVar1 = FUN_142a75f10();` | not examined (call site only) |
| 315 | 0x142ba3400 | 1 | 1 | `uVar1 = FUN_142ba3400();` | not examined (call site only) |
| 316 | 0x144224f50 | 1 | 1 | `FUN_144224f50(this,PitchMultiplier,&uStack_18,&uStack_28,VolumeMultiplier);` | not examined (call site only) |
| 317 | 0x140684de0 | 1 | 0 | `FUN_140684de0(uVar3);` | not examined (call site only) |
| 318 | 0x1406ae6c0 | 1 | 0 | `FUN_1406ae6c0(PredictionKey);` | not examined (call site only) |
| 319 | 0x1406ed110 | 1 | 0 | `uVar2 = FUN_1406ed110(auStack_88);` | not examined (call site only) |
| 320 | 0x14079d060 | 1 | 0 | `FUN_14079d060(param_3,uVar1);` | not examined (call site only) |
| 321 | 0x1407ac640 | 1 | 0 | `FUN_1407ac640(param_1 + 0x1150,param_1,*(undefined4 *)(param_1 + 0xdbc));` | not examined (call site only) |
| 322 | 0x1409bc4c0 | 1 | 0 | `FUN_1409bc4c0(param_1,alStack_48 + 4);` | not examined (call site only) |
| 323 | 0x140f2c470 | 1 | 0 | `FUN_140f2c470(uVar3,TeamKills);` | not examined (call site only) |
| 324 | 0x1419297f0 | 1 | 0 | `FUN_1419297f0(this->CrouchMeshOffset + 4,aSStackX_10);` | not examined (call site only) |
| 325 | 0x14192a490 | 1 | 0 | `FUN_14192a490(this->CrouchMeshOffset + 4);` | not examined (call site only) |
| 326 | 0x14196c820 | 1 | 0 | `FUN_14196c820(lVar1,&uStack_18);` | not examined (call site only) |
| 327 | 0x141972e00 | 1 | 0 | `FUN_141972e00(lVar1);` | not examined (call site only) |
| 328 | 0x141991730 | 1 | 0 | `uVar1 = FUN_141991730(param_1,alStack_28);` | not examined (call site only) |
| 329 | 0x141995a00 | 1 | 0 | `uVar1 = FUN_141995a00(param_1,auStack_58,uStackX_10);` | not examined (call site only) |
| 330 | 0x141995a90 | 1 | 0 | `uVar1 = FUN_141995a90(param_1,auStack_58,uStackX_10);` | not examined (call site only) |
| 331 | 0x141996820 | 1 | 0 | `uVar1 = FUN_141996820(param_1,alStack_28,puVar2);` | not examined (call site only) |
| 332 | 0x1419a3890 | 1 | 0 | `FUN_1419a3890();` | not examined (call site only) |
| 333 | 0x1419c9fc0 | 1 | 0 | `FUN_1419c9fc0(this,&uStack_28);` | not examined (call site only) |
| 334 | 0x1419d5f10 | 1 | 0 | `uVar1 = FUN_1419d5f10();` | not examined (call site only) |
| 335 | 0x1419e4f10 | 1 | 0 | `uVar3 = FUN_1419e4f10(lVar1);` | not examined (call site only) |
| 336 | 0x1419e8cd0 | 1 | 0 | `uVar3 = FUN_1419e8cd0(lVar1);` | not examined (call site only) |
| 337 | 0x141a24940 | 1 | 0 | `FUN_141a24940(param_1,uStackX_10,alStack_48 + 4);` | not examined (call site only) |
| 338 | 0x141a33710 | 1 | 0 | `FUN_141a33710(param_3,uVar1);` | not examined (call site only) |
| 339 | 0x141a33980 | 1 | 0 | `FUN_141a33980(alStack_70,param_1 + 0x1228);` | not examined (call site only) |
| 340 | 0x141a42550 | 1 | 0 | `uVar1 = FUN_141a42550();` | not examined (call site only) |
| 341 | 0x141a59620 | 1 | 0 | `FUN_141a59620(lVar2,CONCAT71((int7)(int3)((uint)iVar1 >> 8),1));` | not examined (call site only) |
| 342 | 0x141c74820 | 1 | 0 | `lVar1 = FUN_141c74820();` | not examined (call site only) |
| 343 | 0x141c795a0 | 1 | 0 | `lVar4 = FUN_141c795a0();` | not examined (call site only) |
| 344 | 0x141c94380 | 1 | 0 | `FUN_141c94380(lVar1,CONCAT71(in_register_00000011,bRestricted),(ulonglong)Volume & 0xff);` | not examined (call site only) |
| 345 | 0x141cd9580 | 1 | 0 | `FUN_141cd9580(lVar1,Result->_tail_0x0[0]);` | not examined (call site only) |
| 346 | 0x141cd95e0 | 1 | 0 | `FUN_141cd95e0(lVar1,PlotID);` | not examined (call site only) |
| 347 | 0x141ceda90 | 1 | 0 | `FUN_141ceda90(lVar1);` | not examined (call site only) |
| 348 | 0x141da28f0 | 1 | 0 | `FUN_141da28f0(lVar1,CONCAT71(in_register_00000011,Gender) & 0xffffffff);` | not examined (call site only) |
| 349 | 0x141e639d0 | 1 | 0 | `uVar3 = FUN_141e639d0(lVar1);` | not examined (call site only) |
| 350 | 0x141ee5f70 | 1 | 0 | `lVar1 = FUN_141ee5f70();` | not examined (call site only) |
| 351 | 0x141ef7100 | 1 | 0 | `pcVar1 = (char *)FUN_141ef7100(auStackX_8,Killer);` | not examined (call site only) |
| 352 | 0x14208b4f0 | 1 | 0 | `FUN_14208b4f0(lVar4,this,!bAbandon);` | not examined (call site only) |
| 353 | 0x1420edbc0 | 1 | 0 | `FUN_1420edbc0(lVar1);` | not examined (call site only) |
| 354 | 0x1422aea50 | 1 | 0 | `FUN_1422aea50(uVar3,EventWindowId,EventId);` | not examined (call site only) |
| 355 | 0x142318c50 | 1 | 0 | `uVar1 = FUN_142318c50(param_1,alStack_28,uStackX_10);` | not examined (call site only) |
| 356 | 0x14233d3c0 | 1 | 0 | `FUN_14233d3c0(param_1);` | not examined (call site only) |
| 357 | 0x1423bd380 | 1 | 0 | `FUN_1423bd380(this,_DAT_148113a00);` | not examined (call site only) |
| 358 | 0x1428381e0 | 1 | 0 | `lVar2 = FUN_1428381e0();` | not examined (call site only) |
| 359 | 0x1428ea7f0 | 1 | 0 | `lVar2 = FUN_1428ea7f0();` | not examined (call site only) |
| 360 | 0x142919d10 | 1 | 0 | `FUN_142919d10();` | not examined (call site only) |
| 361 | 0x1429f5d30 | 1 | 0 | `lVar2 = FUN_1429f5d30();` | not examined (call site only) |
| 362 | 0x142a62c80 | 1 | 0 | `lVar2 = FUN_142a62c80();` | not examined (call site only) |
| 363 | 0x142a755c0 | 1 | 0 | `FUN_142a755c0(this);` | not examined (call site only) |
| 364 | 0x142a76960 | 1 | 0 | `FUN_142a76960(param_1);` | not examined (call site only) |
| 365 | 0x142a76a30 | 1 | 0 | `FUN_142a76a30(param_1);` | not examined (call site only) |
| 366 | 0x142a774a0 | 1 | 0 | `FUN_142a774a0(param_1,param_2);` | not examined (call site only) |
| 367 | 0x142a8ae70 | 1 | 0 | `lVar3 = FUN_142a8ae70();` | not examined (call site only) |
| 368 | 0x142af06a0 | 1 | 0 | `uVar4 = FUN_142af06a0();` | not examined (call site only) |
| 369 | 0x142cbd3c0 | 1 | 0 | `plVar3 = (longlong *)FUN_142cbd3c0(uVar2,uVar4);` | not examined (call site only) |
| 370 | 0x142cd3770 | 1 | 0 | `FUN_142cd3770(apwStackX_10);` | not examined (call site only) |
| 371 | 0x142cd4ab0 | 1 | 0 | `puVar1 = (undefined8 *)FUN_142cd4ab0(auStackX_18,L"Gauntlet",1);` | not examined (call site only) |
| 372 | 0x143f76150 | 1 | 0 | `fVar2 = (float)FUN_143f76150();` | not examined (call site only) |
| 373 | 0x143fd79c0 | 1 | 0 | `FUN_143fd79c0(lVar1,*(undefined4 *)(param_2 + 0xe8),param_2);` | not examined (call site only) |
| 374 | 0x1441205f0 | 1 | 0 | `FUN_1441205f0(lVar1,2);` | not examined (call site only) |
| 375 | 0x1442bcd20 | 1 | 0 | `cVar1 = FUN_1442bcd20(param_1);` | not examined (call site only) |
| 376 | 0x144517e00 | 1 | 0 | `FUN_144517e00(lVar1);` | not examined (call site only) |
| 377 | 0x145803c70 | 1 | 0 | `FUN_145803c70(uStack_18 ^ (ulonglong)auStack_48);` | not examined (call site only) |
| 378 | 0x145803d30 | 1 | 0 | `uVar1 = FUN_145803d30((float)*(byte *)(param_1 + 0x201c) * 0.024639944);` | not examined (call site only) |

## RPC exec thunks and vtable-call records (274 records)

Each thunk calls its `_Validate` and `_Implementation` (or only `_Implementation`) through vtable slots (`(**(code **)(*param_1 + 0xOFF))`, slot = OFF / 8). The slot names come from `porting/ghidra/name_tables/vtable_slots_*.json` (Athena tables only). The native bodies are the separate `_Implementation`/`_Validate` records, so the thunks themselves are not ported.

| Slot status (per called slot) | count |
|---|---:|
| ported | 124 |
| unnamed_slot | 97 |
| verdict_not_usable | 73 |
| unnamed_call | 28 |
| folded_identical_body | 18 |
| other | 8 |
| global_constant | 4 |
| vtable_call | 3 |

Distinct named RPC targets with **no decompile record**: 0. Every named RPC target now has a decompile record (improved set, 1,130 records).

| target | dump address | thunks calling it |
|---|---|---:|
