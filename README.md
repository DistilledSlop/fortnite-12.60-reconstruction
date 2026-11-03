<div align="center">

# Fortnite 12.60 → Linux

**A from-scratch reconstruction of Fortnite 12.60 (CL 13477524) as a buildable Unreal Engine 4.25 project, with a reimplemented dedicated server and local backend.**

[Dashboard](#progress-dashboard) ·
[Treemap SVG](docs/progress.svg) ·
[Data](docs/progress.json) ·
[Handoff](porting/HANDOFF.md) ·
[Master plan](porting/master_plan.md)

<!-- badges:start -->

![overall: 20.7%](https://img.shields.io/static/v1?label=overall&message=20.7%25&color=2a78d6&style=flat-square&labelColor=1f2328) ![evidence-based: 19.8%](https://img.shields.io/static/v1?label=evidence-based&message=19.8%25&color=3987e5&style=flat-square&labelColor=1f2328) ![objective: 19.8%](https://img.shields.io/static/v1?label=objective&message=19.8%25&color=5a9cec&style=flat-square&labelColor=1f2328) ![milestones: 0/4](https://img.shields.io/static/v1?label=milestones&message=0%2F4&color=d6a02a&style=flat-square&labelColor=1f2328) ![snapshot: cf3c54ba](https://img.shields.io/static/v1?label=snapshot&message=cf3c54ba&color=52514e&style=flat-square&labelColor=1f2328&logo=git&logoColor=white)

![Fortnite: 12.60](https://img.shields.io/static/v1?label=Fortnite&message=12.60&color=7d4cdb&style=flat-square&labelColor=1f2328&logo=epicgames&logoColor=white) ![build: CL 13477524](https://img.shields.io/static/v1?label=build&message=CL%2013477524&color=6a3fc4&style=flat-square&labelColor=1f2328) ![Unreal Engine: 4.25.0](https://img.shields.io/static/v1?label=Unreal%20Engine&message=4.25.0&color=0e1128&style=flat-square&labelColor=1f2328&logo=unrealengine&logoColor=white) ![platform: Linux x86_64](https://img.shields.io/static/v1?label=platform&message=Linux%20x86_64&color=e8a317&style=flat-square&labelColor=1f2328&logo=linux&logoColor=white) ![renderer: Vulkan](https://img.shields.io/static/v1?label=renderer&message=Vulkan&color=a41e22&style=flat-square&labelColor=1f2328&logo=vulkan&logoColor=white) ![paks: 31 · v10](https://img.shields.io/static/v1?label=paks&message=31%20%C2%B7%20v10&color=52514e&style=flat-square&labelColor=1f2328) ![oracle: Wine](https://img.shields.io/static/v1?label=oracle&message=Wine&color=722f37&style=flat-square&labelColor=1f2328&logo=wine&logoColor=white)

![C++: 13.9k files](https://img.shields.io/static/v1?label=C%2B%2B&message=13.9k%20files&color=00599c&style=flat-square&labelColor=1f2328&logo=cplusplus&logoColor=white) ![Python: 439 scripts](https://img.shields.io/static/v1?label=Python&message=439%20scripts&color=3776ab&style=flat-square&labelColor=1f2328&logo=python&logoColor=white) ![C#: 22 files](https://img.shields.io/static/v1?label=C%23&message=22%20files&color=512bd4&style=flat-square&labelColor=1f2328) ![shell: 16 scripts](https://img.shields.io/static/v1?label=shell&message=16%20scripts&color=4eaa25&style=flat-square&labelColor=1f2328&logo=gnubash&logoColor=white) ![code: 453.7k lines](https://img.shields.io/static/v1?label=code&message=453.7k%20lines&color=2f6f4f&style=flat-square&labelColor=1f2328)

![commits: 568](https://img.shields.io/static/v1?label=commits&message=568&color=1f6feb&style=flat-square&labelColor=1f2328&logo=git&logoColor=white) ![tracked files: 14.9k](https://img.shields.io/static/v1?label=tracked%20files&message=14.9k&color=1f6feb&style=flat-square&labelColor=1f2328) ![areas: 9](https://img.shields.io/static/v1?label=areas&message=9&color=0e7490&style=flat-square&labelColor=1f2328) ![components: 135](https://img.shields.io/static/v1?label=components&message=135&color=0e7490&style=flat-square&labelColor=1f2328) ![work units done: 1.09M/1.37M](https://img.shields.io/static/v1?label=work%20units%20done&message=1.09M%2F1.37M&color=0e7490&style=flat-square&labelColor=1f2328) ![class parity: 4,617/4,617](https://img.shields.io/static/v1?label=class%20parity&message=4%2C617%2F4%2C617&color=2e8540&style=flat-square&labelColor=1f2328) ![since: 2026-10-30](https://img.shields.io/static/v1?label=since&message=2026-10-30&color=52514e&style=flat-square&labelColor=1f2328)

![status: work in progress](https://img.shields.io/static/v1?label=status&message=work%20in%20progress&color=d6a02a&style=flat-square&labelColor=1f2328) ![authored by: Codex](https://img.shields.io/static/v1?label=authored%20by&message=Codex&color=10a37f&style=flat-square&labelColor=1f2328) ![co-authored by: Claude](https://img.shields.io/static/v1?label=co-authored%20by&message=Claude&color=d97757&style=flat-square&labelColor=1f2328&logo=anthropic&logoColor=white)

<!-- badges:end -->

</div>

---

## Progress dashboard

Area is estimated effort, fill is percent complete. Click the treemap for the full-size SVG. The interactive
page with per-component evidence and the per-commit history chart is [`docs/index.html`](docs/index.html) (download it,
or run `porting/progress/serve.sh`); GitHub only shows HTML as source unless Pages is enabled for the repo.

<p align="center">
  <a href="docs/progress.svg">
    <img src="docs/progress.svg" alt="Treemap of port progress: area is effort weight, fill is percent complete" width="100%">
  </a>
</p>

<!-- progress:start -->

**Overall: 20.7%** · evidence-based 19.8% · objective 19.8% · milestones 0/4 · snapshot `cf3c54ba` (2026-11-03)

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
| Gameplay classes (/Script/FortniteGame) | `███░░░░░░░░░░░░░░░░░` | 13.0% | 33% |
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
| [`docs/`](docs) | Published dashboard snapshot (HTML, SVG, JSON, evidence), ready for GitHub Pages |
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
