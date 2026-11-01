# Original `UFortSplashScreenWidget::OnInputMethodChanged` reflection evidence

The normalized PE runtime image and raw in-memory dump are identified and hashed in [login_status_event_evidence.md](login_status_event_evidence.md). The source byte slices below agree in both images. This establishes the original reflection signature, not the interactive input behavior.

The class-name evidence maps FClassParams `0x146fe5e80` to `UFortSplashScreenWidget`, constructed by `0x1452e3dd0`. Its function constructor `0x1452e3e00` points to FFunctionParams `0x147f280b0`, whose original 72 bytes are:

```text
d03d2e450100000000000000000000007032a04501000000000000000000000000000000000000000100000000000000405efe460100000002000000450000000008080800000000
```

This record has the class constructor as outer, one-byte parameter struct, property array `0x146fe5e40`, two property records, object flags `0x45`, and function flags `0x08080800`: protected, Event, BlueprintEvent, without Native or BlueprintCallable. The array contains pointers `0x147f28028` and `0x147f28058`. Their first 48 bytes are:

```text
f0c2a04501000000000000000000000080000000000010001e00000045000000010000000000000020ee814001000000
10a39a450100000000000000000000000000000000000000000000004500000001000000000000000000000000000000
```

The first record is enum parameter `InputType`, property flags `0x0010000000000080`, offset 0, with enum constructor `0x14081ee20`. The second is its underlying Byte record, not another argument. The original enum table identifies that constructor with enum class `ECommonInputType`, `/Script/CommonInput`, block `0x14783dce0`, with underlying byte and values MouseAndKeyboard=0, Gamepad=1, Touch=2, Count=3. Other same-named functions have different owner constructors and flags; owner matching is required.

The source-derived declaration is protected `UFUNCTION(BlueprintImplementableEvent) void OnInputMethodChanged(ECommonInputType InputType);`. The resolver records the exact enum's `ECommonInputType.h` and CommonInput module dependency only for this observed event; ordinary cross-module restrictions remain in force. UHT owns the event forwarder. Generated flags, enum property, and the cooked Blueprint load require controlled-build verification.
