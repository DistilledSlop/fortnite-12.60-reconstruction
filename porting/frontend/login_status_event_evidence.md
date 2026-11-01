# Original `UFortLoginStatus::SetContinueButtonText` reflection evidence

The normalized PE runtime image `porting/generated/dump/FortniteClient-Win64-Shipping.runtime.exe` is 165,991,936 bytes, SHA-256 `5f056080237b5a0fdab284700d5bd975acf8c30ffeeb37472f5ba5542fa2dc38`. The raw in-memory dump `FortniteClient-Win64-Shipping.runtime.bin` has the same length and SHA-256 `604900985a724144b6e0fc6e033b0cef2adb74a295a65a4fe01aceebb52df93d`; the raw instruction, class-link and parameter slices at the addresses below were compared directly and match the normalized image. These original-data observations are read-only; they do not establish interactive login behavior.

The class evidence `porting/generated/reflection/class_name_map.evidence.json` (SHA-256 `6aecbf0ca6bf466a8cae04a70884635670ad77710577bca57b8fcc0a9ed437e1`) maps `UFortLoginStatus` to FClassParams `0x146f6d830`, class construction function `0x145282560`. The exact original machine code at `0x145282570` is `48 8d 15 b9 b2 ce 01`, loading the class block. That class block's function-link array `0x146f6d7f0` contains function constructor `0x1452825a0` and name pointer `0x146f21d20` (`SetContinueButtonText`). Function constructor bytes at `0x1452825b0` are `48 8d 15 99 3b c6 02`, loading FFunctionParams `0x147ee6150`.

The 72 raw FFunctionParams bytes, in original address order:

```text
60252845010000000000000000000000201df2460100000000000000000000000000000000000000180000000000000088d7f6460100000001000000450000000008480800000000
```

They decode as `OuterFunc=0x145282560`, no super, name `SetContinueButtonText`, parameter struct size `0x18`, property-array pointer `0x146f6d788`, one property, object flags `0x45`, and function flags `0x08480800`. Its array's sole property pointer is `0x147ee5588`; the first 48 raw bytes are:

```text
b8eaf64601000000000000000000000082010008000010001d000000450000000100000000000000801f284501000000
```

The property is `ContinueText`, `EPropertyGenFlags::Text` (`0x1d`), property flags `0x0010000008000182`, array dimension 1 and offset 0. `ConstParm`, `Parm`, `OutParm`, and `ReferenceParm` are set. The same name occurs on a *different* owner `0x14522c030` / function `0x14522c120` with parameter `InText`; matching the function name alone is insufficient. The complete reflection report `porting/generated/reflection/fortnite_game_reflection.json` has SHA-256 `c11cc036439fd2832514681e1bac12c933ee78b6a351f36d44dabf170d8eab1b`; independent decoded `function_blocks.json` has SHA-256 `a91890539be0788e7f333897330be8b65866f7ac57ac5a98bb881efa8b815965`.

UE's `Script.h` identifies the function flags as `Event` (`0x800`), `Protected` (`0x80000`), `HasOutParms` (`0x400000`) and `BlueprintEvent` (`0x8000000`); `BlueprintCallable` (`0x4000000`) is **not set**. Its SHA-256 is `552e828a5409d172d33cecf2ccf46c24cc4a7485fc2073dad810143d7937015f`. `ObjectMacros.h`, SHA-256 `b7efacfde163c7f1d49404380c6182a7a989a282e991441fcbe60861d8d67389`, defines the parameter flags.

Existing UE source `Engine/Classes/Components/TextRenderComponent.h:115` declares `K2_SetText(const FText& Value)`; its generated `TextRenderComponent.gen.cpp:373` has the same text-property flags `0x0010000008000182`. Source and current generated file SHA-256 values are respectively `f1a6f62785f95123bf9733915292f59be3cd299acc033804616c1def9a719ef8` and `29a594e6401e099f3bd93dd271dc920006a98a8466665ca4745cf9858cd79351`. An existing protected `BlueprintImplementableEvent` with const-reference out flags (`UUserDefinedCaptureProtocol::OnPixelsReceived`) has generated UFunction flags `0x08480800`.

The selected declaration is therefore protected `UFUNCTION(BlueprintImplementableEvent) void SetContinueButtonText(const FText& ContinueText);`. It has no `BlueprintCallable` specifier and no hand-written C++ implementation: UHT must generate its ProcessEvent forwarder. Runtime class/Blueprint loading and actual UHT output must still be verified after regeneration and build.
