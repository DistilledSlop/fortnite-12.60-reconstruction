# Dedicated player-name compatibility

This policy restores the recovered dedicated-server name representation for the
original 12.60 client. It does not claim full shared-class/client body recovery.
The manifest helper entries explicitly record that scope. Outside a dedicated
server the methods retain the existing Engine behavior.

Original decode `0x142118d30` and encode `0x142119770` copy counted UTF-16 strings.
For `n` units, seed `(n + int8(salt)) & 3`, add 3 before each unit, and add/subtract
that unit by `seed & 7` with 16-bit wrapping. The constructor at `0x1423fa52b`
enables custom names. Dedicated current-name storage uses salt 0; ordinary getters
decode it. `LocalPlayer` therefore has wire representation `Fn_ZjKl^sdn`.

`SetPlayerName` (`0x142121070`) enables original processing mask 2 around the Engine
setter. Its virtual internal setter (`0x1421210a0`) encodes salt 0. While processing,
the custom getter (`0x14211a390`) decodes salt 0 then encodes salt 1 so Engine's
old-name cache receives the correct representation. `GetOldPlayerName`
(`0x14211a290`) decodes salt 1. The inherited `SetOldPlayerName` remains a raw
assignment, matching original `0x1444b7320`. Spectators bypass transformation.
Client OnRep mask 4 is distinct from setter mask 2; its receiver lifecycle and the
client cover-name/cache branches are not recovered here.

The policy merges constructor changes through generation and rejects competing
helpers/state. Generated files must not be edited manually. Helper snippet bodies
use ordinary C++ target/mode branches because the manifest loader forbids
preprocessor directives; the support codec and constructor have compile guards.

Original machine-code evidence is in
`generated/agents/name-codec-recovery/REPORT.md` and its capture/CFG artifacts.
`oracle-fixtures.json` records seven outputs produced by running original x86
loops in Unicorn, independently of the C++ implementation. The opt-in
`-FortnitePlayerNameSelfTest` exercises the generated class, repeated setters,
old-cache behavior, spectator bypass and actual FString bit archives in a
dedicated game world. FString's U+FFFF loading truncation prevents an arbitrary
UTF-16 transport claim, even when the arithmetic itself is reversible.
