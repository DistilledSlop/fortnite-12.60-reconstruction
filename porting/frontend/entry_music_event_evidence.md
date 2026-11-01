# Original `AFortEntryMusicController::StartMusic` reflection evidence

The original normalized PE runtime image and raw memory dump are documented and hashed in [login_status_event_evidence.md](login_status_event_evidence.md). The following original bytes agree in both images. This evidence establishes reflection metadata, not the original audio behavior.

The class map identifies FClassParams `0x1463bb8b0` as `AFortEntryMusicController`, with constructor `0x1428dc600`. Its one-entry function-link array at `0x1463bb8a0` contains `30c68d4201000000f8ca3b4601000000`: the constructor is `0x1428dc630` and the name at `0x1463bcaf8` is `StartMusic`. The function constructor loads FFunctionParams at `0x147a32fa0`; its 72 bytes are:

```text
00c68d42010000000000000000000000f8ca3b4601000000000000000000000000000000000000000000000000000000000000000000000000000000450000000008020800000000
```

They give outer `0x1428dc600`, no super, a zero-byte parameter struct, a null property array, zero properties, object flags `0x45`, and function flags `0x08020800`. These are `Public`, `Event`, and `BlueprintEvent`; neither `Native` nor `BlueprintCallable` is set. The independent reflection and class-map files and their relevant hashes are cited in the LoginStatus evidence. The original class name map specifically associates `0x1463bb8b0` and its construction function with `AFortEntryMusicController`.

The source-derived declaration is public `UFUNCTION(BlueprintImplementableEvent) void StartMusic();`. UHT owns the forwarder; no native audio implementation is inferred. The generated UFunction flags and loading behavior require verification in the controlled build.
