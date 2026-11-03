# Weapon equip RPC event names

The original capture `original-late-capture-1/image-125.runtime.bin` is a raw mapped image at base `0x140000000`. The globals and their registration initializers directly establish these names; no inference from network role or a guessed FName pool layout is needed.

| Wrapper | FName global (8 captured bytes) | Static initializer | UTF-16LE literal | Event |
|---|---|---|---|---|
| `0x142a16c90` | `0x1481ad458` (`07 de 04 00 00 00 00 00`) | `0x140413fa0` | `0x146485718` | `ClientInternalEquipWeapon` |
| `0x142a19220` | `0x1481ad538` (`9d de 04 00 00 00 00 00`) | `0x140414320` | `0x1464859b0` | `ServerInternalEquipWeapon` |

Each 25-byte initializer loads `r8d = 1`, loads the listed literal address into `rdx`, loads the listed global into `rcx`, and tail-jumps to the same function `0x142cd4ab0`. The initializer window SHA-256 values are respectively `8d6faf72131fa1979de20fb419d4072cb66f1e6af57620ffc3fe98b71a782863` and `67fbe3e5f13f998bf20be6c368d77155dffc29bb7129be94ecb273e8fbbe97e5`. Captured comparison indices are `0x4de07` and `0x4de9d`, with number zero. This directly connects the captured globals to the literal names; it does not require reading the externally allocated name-pool blocks.

Reflection (`porting/generated/reflection/fortnite_game_reflection.json`) identifies both as `AFortPawn` RPCs with one offset-zero object parameter `Weap` of `AFortWeapon*`. The client event has creation function `0x142a16320`, flags `0x1040cc1`, and Reliable/Client roles; the server event has creation function `0x142a159c0`, flags `0x80240cc1`, and Reliable/Server roles. Both have outer function `0x142a168e0`. Generated `porting/native/Source/FortniteGame/Generated/AFortPawn.h` declares the corresponding `UFUNCTION(Client, Reliable)` and `UFUNCTION(Server, Reliable, WithValidation)` signatures. Its generated `_Implementation` bodies are currently unrecovered stubs; the server validation is the existing shared true-return body. The manifest associates both original implementations with `0x14230e590` but marks them skipped due to folded-body ambiguity; these name findings do not establish the event implementations or their effects.

Method: searched existing decoder references and dump metadata, scanned the captured `.text` for RIP-relative references to the two globals, and disassembled 128-byte windows with `objdump -D -b binary -m i386:x86-64 -M intel --adjust-vma=<window VA>`. Windows are `/tmp/gameplay-weapon-event/0x140413f80.bin` and `/tmp/gameplay-weapon-event/0x140414300.bin`. The literals and globals were read at VA minus `0x140000000`. No build or runtime test was run.
