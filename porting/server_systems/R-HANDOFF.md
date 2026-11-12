# R cards: handoff for the next Fable 5.2 session

Date: 2026-11-12. Written at the user's request when the previous session
wound down. Read this before touching the R cards.

## 1. Division of work (set by the user)

- **Sonnet implements** G6 to G9 in `porting/native/` from cards R1 to R5.
  The Fable session does not implement there.
- **Fable reads and documents**: the remaining unread client functions, and
  the hard questions in section 4. It also reviews Sonnet's re-creation
  designs (death sequence, building, inventory and pickup, bus jump) when
  they exist.
- Workspace rules are in `/opt/AGENTS.md` and the memory index. The ones that
  bit this session: commit with
  `git commit --only --author="Claude Fable 5.2 <noreply@anthropic.com>" -- <paths>`
  plus `GIT_COMMITTER_NAME/EMAIL`, because a plain `git commit` takes other
  sessions' staged files. Push fast-forward only. Never run a directory-wide
  `git checkout`.

## 2. What exists

| File | Content |
|---|---|
| `R1-warmup-and-aircraft-code.md` | tick driver, warmup, ready counter, aircraft phase, bus delegates |
| `R2-safe-zone-code.md` | storm start, update, advance, cached definition table |
| `R3-elimination-and-match-end-code.md` | alive-player lists, win check, end game, match end |
| `R4-readiness-build-interact-code.md` | `bReadyToStartMatch`, the build RPC correction, interact |
| `R5-playlist-setup-phase-handler-receipts.md` | flight path and storm-center setup, phase handler, full match order (section 6), unread list (section 7) |
| `R6-playlist-trigger-storm-table-storm-damage.md` | `OnPlaylistDataLoaded` trigger chain, storm table on the map's MapInfo, per-pawn storm damage, supply drop scheduler, helper table, unread list (section 6) |

`G6_MAPPING_APPROACH.md` and section 0 of `SERVER_FIRST_PLAN.md` link to
them. Last pushed doc commit of the previous session: `e89b15e1`.

Commit `f8742834` wrongly included a Codex session's 33 staged test renames
and a Codex author. The user was told and has not asked for a revert. Leave
it unless asked.

## 3. Findings not yet in the cards

None. Everything that was parked here is in card R6 (helper table in its
section 5; `OnPlaylistDataLoaded` trigger in section 1; MapInfo and storm
table in section 2). Two statements of the first version of this handoff
were wrong and are corrected in R6 section 0: the game mode slot is
`+0xd10`, not `+0xfc8`, and the call at `0x1418f67b0` is a vehicle virtual.

The decompile cache is `/opt/fortnite/local-model-package/captures/`
(persistent; `rd.py` reuses it). Use `rd.py ADDR Class --exact` when the
start is known, because the default start resolution picks the previous
16-aligned chunk and is sometimes wrong.

## 4. Open work for Fable, in priority order

Done on 2026-11-12 (second session), all written into the cards:
`OnPlaylistDataLoaded` trigger, storm table source, `IsMapInfoValid`,
storm damage including the pawn side (R6); `StartEndGamePhaseTeam` and the
end-game helpers (R3 sections 5, 5.1, 6); the build caller `0x1426b0790`
(R4 section 2.3); the five wrong-start helpers (R2, R3).

1. Re-run the RPC sweep with a "real but only a guard" rule (R4 section 5).
   Add a second rule from R4 section 2.3: a body whose only substantive
   call is `0x140680300` (the shared null stub) is also not real.
2. Read the rest of `0x1419b47b0` (playlist values copied into the
   GameState) if Sonnet ports the playlist chain instead of calling the
   game mode directly (R6 section 1.3).
3. Small leftovers: `0x141a5a880` (place setter), `0x141962af0`,
   `0x1419fc560`, `0x141988940`, `0x1419a9580`, `0x1419ae490`,
   `0x1419797b0`, the two plugin callbacks `0x1419a42b0` / `0x1419a4830`.
4. Review Sonnet's re-creation designs when they appear.

## 5. Tools

- `python3 -I porting/tools/rd.py 0xADDR Class --full` (annotated
  decompile), `callers.py`, `vt.py`, `strx.py`, `strs_near.py`,
  `asset_props.py`.
- `objdump -D -b binary -mi386:x86-64 -M intel --adjust-vma=0x140000000` on
  `porting/generated/dump/FortniteClient-Win64-Shipping.runtime.exe` for
  return values and discarded registers.
- Offsets to property names: `python3 -I porting/tools/off.py Class 0xOFF …`
  (whole parent chain; bit names are in `generated/reflection/bool_bits.json`).
- `python3 -I porting/tools/learefs.py 0xADDR …`: code that takes the address
  of a function (delegate bindings, member-pointer thunks). `callers.py` only
  finds direct calls and data references.
- Virtual call sites of a slot: scan for `FF 90+r disp32` / `FF A0+r disp32`
  with the slot offset (done inline in the second session; 47 hits for
  `+0xd10`, the game-mode one is in `0x1419a3fc0`).
