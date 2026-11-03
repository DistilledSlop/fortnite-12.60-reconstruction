<div align="center">

# Fortnite 12.60 → Linux

**A from-scratch reconstruction of Fortnite 12.60 (CL 13477524) as a buildable Unreal Engine 4.25 project, with a reimplemented dedicated server and local backend.**

[Live dashboard](https://distilledslop.github.io/fortnite-12.60-reconstruction/) ·
[History](https://distilledslop.github.io/fortnite-12.60-reconstruction/history.html) ·
[Handoff](porting/HANDOFF.md) ·
[Master plan](porting/master_plan.md)

</div>

---

## Progress dashboard

Area is estimated effort, fill is percent complete. Click the treemap for the interactive dashboard with per-component evidence.

<p align="center">
  <a href="https://distilledslop.github.io/fortnite-12.60-reconstruction/">
    <img src="docs/progress.svg" alt="Treemap of port progress: area is effort weight, fill is percent complete" width="100%">
  </a>
</p>

<!-- progress:start -->

![overall](https://img.shields.io/badge/overall-20.5%25-2a78d6) ![milestones](https://img.shields.io/badge/milestones-0%2F4-d6a02a) ![snapshot](https://img.shields.io/badge/snapshot-8654e768-52514e)

**Overall: 20.5%** · evidence-based 19.7% · objective 19.7% · milestones 0/4 · snapshot `8654e768` (2026-11-03)

| Milestone | Status |
|---|---|
| The rebuilt client reaches the main menu with feature parity to the 12.60 original | ⬜ open |
| The client joins a lobby on the reconstructed server | ⬜ open |
| The client matchmakes to the reconstructed UE dedicated server | ⬜ open |
| A full match is played with audio and visuals working | ⬜ open |

| Area | Progress | % | Effort weight |
|---|---|--:|--:|
| Platform and Linux runtime | `█████░░░░░░░░░░░░░░░` | 26.0% | 5% |
| Asset pipeline | `█████░░░░░░░░░░░░░░░` | 26.0% | 14% |
| Maps and world (Apollo island) | `███░░░░░░░░░░░░░░░░░` | 13.9% | 9% |
| Gameplay classes (/Script/FortniteGame) | `███░░░░░░░░░░░░░░░░░` | 12.6% | 33% |
| Network protocol and replication | `█████░░░░░░░░░░░░░░░` | 27.1% | 10% |
| Backend services (reconstructed server) | `█████████░░░░░░░░░░░` | 46.3% | 11% |
| Audio | `█░░░░░░░░░░░░░░░░░░░` | 7.4% | 4% |
| Visuals parity | `░░░░░░░░░░░░░░░░░░░░` | 2.4% | 5% |
| Frontend and lobby UI | `█████░░░░░░░░░░░░░░░` | 23.1% | 10% |

<sub>Percentages are effort-weighted estimates of reconstruction work, not a measure of the playable-game definition, which is far from met. `objective` counts only computed evidence; `evidence-based` adds documented counts; the rest is manual estimate. Method: [porting/progress/README.md](porting/progress/README.md).</sub>

<!-- progress:end -->

> The dashboard is generated from tracked files by [`porting/progress/progress.py`](porting/progress/progress.py)
> and refreshed after every commit. Each figure carries its basis (`git`, `local`, `documented`, `manual`), so the
> subjective part is always visible.

## The goal

Done means: **login, lobby, matchmaking and a full match, with audio and visuals, verified end to end.**

| Piece | What it has to be |
|---|---|
| **Client** | The exact same client as the supplied 12.60 original, every feature and structure, rebuilt into a UE project. Decompiling it into source is the point, not a fallback. Nothing is stripped to make networking work. |
| **Server** | Every structure and feature the original dedicated server would have. Epic never shipped it, so it is rebuilt from the engine source plus local stand-ins for the Epic backend (account, profile, matchmaking, session, party, XMPP). |
| **Reference** | The supplied 12.60 build defines completeness. The original binary under Wine is an analysis oracle for decrypted code, requests and gates, not the deliverable. |

## How it is built

```
12.60 paks ──► decrypt + index ──► asset extraction ──► reflection dump ──► generated UClasses
 (31, v10)      (cue4inspect)       (963k files)        (types, vtables)         │
                                                                                 ▼
 Wine oracle ──► code recovery ──► decompiled bodies ──► native module ──► UE 4.25 Linux
 (analysis)      (Ghidra)          (spliced, checked)    (client + server)  build + stage
                                                                                 │
                              reconstructed backend  ◄── direct join / smokes ◄─┘
```

- **Native classes** are generated from the game's own reflection data and checked against an offline network-checksum model, so replication layouts match the original.
- **Function bodies** are recovered from the binary and spliced into the generated classes, each with a behavior record. Source coverage is tracked separately from runtime parity.
- **Server** (`porting/server/`) is a local stand-in for Epic's services, with a test suite run on every regression.
- **Assets** are extracted from the supplied paks into ignored local storage and converted (meshes, textures, materials) for the Linux cook.

## Repository layout

| Path | Contents |
|---|---|
| [`porting/native/`](porting/native) | The UE project: generated classes, recovered bodies, plugins, build targets |
| [`porting/behavior/`](porting/behavior) | Recovered function behavior, manifests, and checkers |
| [`porting/frontend/`](porting/frontend) | Frontend/lobby UI recovery: CommonUI, Slate, and per-widget runtime probes |
| [`porting/server/`](porting/server) | Reimplemented backend services and evidence |
| [`porting/asset_pipeline/`](porting/asset_pipeline), [`material_pipeline/`](porting/material_pipeline) | Pak extraction, mesh/texture/material conversion |
| [`porting/progress/`](porting/progress) | The dashboard generator, effort model, evidence ledger |
| [`docs/`](docs) | Published dashboard snapshot (GitHub Pages) |
| [`porting/README.md`](porting/README.md) | Detailed evidence log and the work still needed |
| [`porting/HANDOFF.md`](porting/HANDOFF.md) | Current state, build recipe, next steps |

## What is in and out of Git

This repository tracks **only new porting code, notes, and the dashboard**. The supplied Windows client, the
Unreal Engine source checkout, extracted assets, build products and every encryption key stay outside Git
(see [`.gitignore`](.gitignore)). Nothing here redistributes Fortnite content.

## Working with the dashboard

```sh
python3 porting/progress/progress.py --no-record   # render HEAD into porting/progress/out/
python3 porting/progress/publish.py                # copy it to docs/ and refresh the block above
python3 porting/progress/publish.py --check        # CI-style staleness check
porting/progress/serve.sh                          # serve out/ on the LAN (port 8765)
```

Details of the model, bases and evidence ledger are in [`porting/progress/README.md`](porting/progress/README.md).
