<div align="center">

# Fortnite 12.60 → Linux

**A from-scratch reconstruction of Fortnite 12.60 (CL 13477524) as a buildable Unreal Engine 4.25 project, with a reimplemented dedicated server and local backend.**

[Dashboard](#progress-dashboard) ·
[Treemap SVG](docs/progress.svg) ·
[Data](docs/progress.json) ·
[Handoff](porting/HANDOFF.md) ·
[Master plan](porting/master_plan.md)

<!-- badges:start -->
<p align="center"><img alt="overall: 20.9%" src="https://img.shields.io/static/v1?label=overall&amp;message=20.9%25&amp;color=2a78d6"> <img alt="evidence: 20.0%" src="https://img.shields.io/static/v1?label=evidence&amp;message=20.0%25&amp;color=3987e5"> <img alt="objective: 20.0%" src="https://img.shields.io/static/v1?label=objective&amp;message=20.0%25&amp;color=5a9cec"> <img alt="milestones: 0/4" src="https://img.shields.io/static/v1?label=milestones&amp;message=0%2F4&amp;color=d6a02a"> <img alt="units: 1.09M/1.37M" src="https://img.shields.io/static/v1?label=units&amp;message=1.09M%2F1.37M&amp;color=0e7490"> <img alt="snapshot: 831a7954" src="https://img.shields.io/static/v1?label=snapshot&amp;message=831a7954&amp;color=9f9f9f&amp;logo=git"> <img alt="updated: 2026-11-03" src="https://img.shields.io/static/v1?label=updated&amp;message=2026-11-03&amp;color=9f9f9f"></p>
<p align="center"><img alt="M1 menu: open" src="https://img.shields.io/static/v1?label=M1%20menu&amp;message=open&amp;color=9f9f9f"> <img alt="M2 lobby: open" src="https://img.shields.io/static/v1?label=M2%20lobby&amp;message=open&amp;color=9f9f9f"> <img alt="M3 matchmaking: open" src="https://img.shields.io/static/v1?label=M3%20matchmaking&amp;message=open&amp;color=9f9f9f"> <img alt="M4 match: open" src="https://img.shields.io/static/v1?label=M4%20match&amp;message=open&amp;color=9f9f9f"> <img alt="client: unstripped" src="https://img.shields.io/static/v1?label=client&amp;message=unstripped&amp;color=7d4cdb"> <img alt="server: reconstructed" src="https://img.shields.io/static/v1?label=server&amp;message=reconstructed&amp;color=e8a317"> <img alt="goal: full A/V" src="https://img.shields.io/static/v1?label=goal&amp;message=full%20A%2FV&amp;color=a41e22"></p>
<p align="center"><img alt="platform: 26.0%" src="https://img.shields.io/static/v1?label=platform&amp;message=26.0%25&amp;color=97ca00"> <img alt="assets: 26.0%" src="https://img.shields.io/static/v1?label=assets&amp;message=26.0%25&amp;color=97ca00"> <img alt="maps: 13.9%" src="https://img.shields.io/static/v1?label=maps&amp;message=13.9%25&amp;color=dfb317"> <img alt="gameplay: 13.7%" src="https://img.shields.io/static/v1?label=gameplay&amp;message=13.7%25&amp;color=dfb317"> <img alt="network: 27.1%" src="https://img.shields.io/static/v1?label=network&amp;message=27.1%25&amp;color=97ca00"> <img alt="backend: 46.3%" src="https://img.shields.io/static/v1?label=backend&amp;message=46.3%25&amp;color=97ca00"> <img alt="audio: 7.4%" src="https://img.shields.io/static/v1?label=audio&amp;message=7.4%25&amp;color=fe7d37"></p>
<p align="center"><img alt="visuals: 2.4%" src="https://img.shields.io/static/v1?label=visuals&amp;message=2.4%25&amp;color=e05d44"> <img alt="frontend UI: 23.1%" src="https://img.shields.io/static/v1?label=frontend%20UI&amp;message=23.1%25&amp;color=dfb317"> <img alt="components: 135" src="https://img.shields.io/static/v1?label=components&amp;message=135&amp;color=2a78d6"> <img alt="areas: 9" src="https://img.shields.io/static/v1?label=areas&amp;message=9&amp;color=7d4cdb"> <img alt="class parity: 4,617/4,617" src="https://img.shields.io/static/v1?label=class%20parity&amp;message=4%2C617%2F4%2C617&amp;color=2e8540"> <img alt="git-measured: 61 leaves" src="https://img.shields.io/static/v1?label=git-measured&amp;message=61%20leaves&amp;color=4eaa25"> <img alt="manual: 60 leaves" src="https://img.shields.io/static/v1?label=manual&amp;message=60%20leaves&amp;color=9f9f9f"></p>
<p align="center"><img alt="Fortnite: 12.60" src="https://img.shields.io/static/v1?label=Fortnite&amp;message=12.60&amp;color=7d4cdb&amp;logo=epicgames"> <img alt="build: CL 13477524" src="https://img.shields.io/static/v1?label=build&amp;message=CL%2013477524&amp;color=6a3fc4"> <img alt="UE: 4.25.0" src="https://img.shields.io/static/v1?label=UE&amp;message=4.25.0&amp;color=0e1128&amp;logo=unrealengine"> <img alt="platform: Linux x64" src="https://img.shields.io/static/v1?label=platform&amp;message=Linux%20x64&amp;color=e8a317&amp;logo=linux"> <img alt="renderer: Vulkan" src="https://img.shields.io/static/v1?label=renderer&amp;message=Vulkan&amp;color=a41e22&amp;logo=vulkan"> <img alt="oracle: Wine" src="https://img.shields.io/static/v1?label=oracle&amp;message=Wine&amp;color=722f37&amp;logo=wine"> <img alt="original: Win64" src="https://img.shields.io/static/v1?label=original&amp;message=Win64&amp;color=0078d4"></p>
<p align="center"><img alt="paks: 31 · v10" src="https://img.shields.io/static/v1?label=paks&amp;message=31%20%C2%B7%20v10&amp;color=5b4a8a"> <img alt="pak entries: 963,427" src="https://img.shields.io/static/v1?label=pak%20entries&amp;message=963%2C427&amp;color=0e7490"> <img alt=".uasset: 450,951" src="https://img.shields.io/static/v1?label=.uasset&amp;message=450%2C951&amp;color=2e8540"> <img alt="maps: 3,694" src="https://img.shields.io/static/v1?label=maps&amp;message=3%2C694&amp;color=b08800"> <img alt="plugins: 123" src="https://img.shields.io/static/v1?label=plugins&amp;message=123&amp;color=a41e22"> <img alt="modules: 6" src="https://img.shields.io/static/v1?label=modules&amp;message=6&amp;color=512bd4"> <img alt="API routes: 59" src="https://img.shields.io/static/v1?label=API%20routes&amp;message=59&amp;color=e8a317"></p>
<p align="center"><img alt="extraction: CUE4Parse" src="https://img.shields.io/static/v1?label=extraction&amp;message=CUE4Parse&amp;color=512bd4"> <img alt="decompiler: Ghidra 12.2" src="https://img.shields.io/static/v1?label=decompiler&amp;message=Ghidra%2012.2&amp;color=b5301f"> <img alt="pak index: v10" src="https://img.shields.io/static/v1?label=pak%20index&amp;message=v10&amp;color=5b4a8a"> <img alt="net: IpNetDriver" src="https://img.shields.io/static/v1?label=net&amp;message=IpNetDriver&amp;color=a41e22"> <img alt="UI: CommonUI" src="https://img.shields.io/static/v1?label=UI&amp;message=CommonUI&amp;color=7d4cdb"> <img alt="parity: checksum" src="https://img.shields.io/static/v1?label=parity&amp;message=checksum&amp;color=2e8540"> <img alt="tooling: Python" src="https://img.shields.io/static/v1?label=tooling&amp;message=Python&amp;color=3776ab&amp;logo=python"></p>
<p align="center"><img alt="C++: 4,742 .cpp" src="https://img.shields.io/static/v1?label=C%2B%2B&amp;message=4%2C742%20.cpp&amp;color=00599c&amp;logo=cplusplus"> <img alt="headers: 8,199 .h" src="https://img.shields.io/static/v1?label=headers&amp;message=8%2C199%20.h&amp;color=0e7490"> <img alt="recovered: 1,690 .inc" src="https://img.shields.io/static/v1?label=recovered&amp;message=1%2C690%20.inc&amp;color=d97757"> <img alt="Python: 521 scripts" src="https://img.shields.io/static/v1?label=Python&amp;message=521%20scripts&amp;color=3776ab&amp;logo=python"> <img alt="C#: 22 files" src="https://img.shields.io/static/v1?label=C%23&amp;message=22%20files&amp;color=512bd4"> <img alt="shell: 16 scripts" src="https://img.shields.io/static/v1?label=shell&amp;message=16%20scripts&amp;color=4eaa25&amp;logo=gnubash"> <img alt="patches: 11" src="https://img.shields.io/static/v1?label=patches&amp;message=11&amp;color=6e5494"></p>
<p align="center"><img alt="C++ lines: 398.5k" src="https://img.shields.io/static/v1?label=C%2B%2B%20lines&amp;message=398.5k&amp;color=00599c"> <img alt="Python lines: 75.1k" src="https://img.shields.io/static/v1?label=Python%20lines&amp;message=75.1k&amp;color=3776ab"> <img alt="C# lines: 2,531" src="https://img.shields.io/static/v1?label=C%23%20lines&amp;message=2%2C531&amp;color=512bd4"> <img alt="shell lines: 754" src="https://img.shields.io/static/v1?label=shell%20lines&amp;message=754&amp;color=4eaa25"> <img alt="docs: 29.9k lines" src="https://img.shields.io/static/v1?label=docs&amp;message=29.9k%20lines&amp;color=0e7490"> <img alt="markdown: 439 files" src="https://img.shields.io/static/v1?label=markdown&amp;message=439%20files&amp;color=6e5494"> <img alt="data: 349 JSON" src="https://img.shields.io/static/v1?label=data&amp;message=349%20JSON&amp;color=b08800"></p>
<p align="center"><img alt="native: 12.8k files" src="https://img.shields.io/static/v1?label=native&amp;message=12.8k%20files&amp;color=00599c"> <img alt="behavior: 2,115 files" src="https://img.shields.io/static/v1?label=behavior&amp;message=2%2C115%20files&amp;color=b5301f"> <img alt="frontend: 783 files" src="https://img.shields.io/static/v1?label=frontend&amp;message=783%20files&amp;color=7d4cdb"> <img alt="server: 28 files" src="https://img.shields.io/static/v1?label=server&amp;message=28%20files&amp;color=e8a317"> <img alt="assets: 6 files" src="https://img.shields.io/static/v1?label=assets&amp;message=6%20files&amp;color=2e8540"> <img alt="materials: 113 files" src="https://img.shields.io/static/v1?label=materials&amp;message=113%20files&amp;color=d97757"> <img alt="dashboard: 54 files" src="https://img.shields.io/static/v1?label=dashboard&amp;message=54%20files&amp;color=2a78d6"></p>
<p align="center"><img alt="tracked files: 16.0k" src="https://img.shields.io/static/v1?label=tracked%20files&amp;message=16.0k&amp;color=0e7490"> <img alt="commits: 649" src="https://img.shields.io/static/v1?label=commits&amp;message=649&amp;color=1f6feb&amp;logo=git"> <img alt="branches: 14" src="https://img.shields.io/static/v1?label=branches&amp;message=14&amp;color=7d4cdb"> <img alt="since: 2026-10-30" src="https://img.shields.io/static/v1?label=since&amp;message=2026-10-30&amp;color=9f9f9f"> <img alt="authored by: Codex" src="https://img.shields.io/static/v1?label=authored%20by&amp;message=Codex&amp;color=10a37f"> <img alt="authored by: Claude" src="https://img.shields.io/static/v1?label=authored%20by&amp;message=Claude&amp;color=d97757&amp;logo=anthropic"> <img alt="status: WIP" src="https://img.shields.io/static/v1?label=status&amp;message=WIP&amp;color=dfb317"></p>
<p align="center"><img alt="agents: ≤3 at once" src="https://img.shields.io/static/v1?label=agents&amp;message=%E2%89%A43%20at%20once&amp;color=d97757&amp;logo=anthropic"> <img alt="commits: each batch" src="https://img.shields.io/static/v1?label=commits&amp;message=each%20batch&amp;color=1f6feb"> <img alt="history: per commit" src="https://img.shields.io/static/v1?label=history&amp;message=per%20commit&amp;color=2a78d6"> <img alt="evidence: ledgered" src="https://img.shields.io/static/v1?label=evidence&amp;message=ledgered&amp;color=2e8540"> <img alt="keys: git-ignored" src="https://img.shields.io/static/v1?label=keys&amp;message=git-ignored&amp;color=e05d44"> <img alt="assets: git-ignored" src="https://img.shields.io/static/v1?label=assets&amp;message=git-ignored&amp;color=fe7d37"> <img alt="checks: batched" src="https://img.shields.io/static/v1?label=checks&amp;message=batched&amp;color=dfb317"> <img alt="water used: 20.6 gal" src="https://img.shields.io/static/v1?label=water%20used&amp;message=20.6%20gal&amp;color=dfb317"></p>
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

**Overall: 20.9%** · evidence-based 20.0% · objective 20.0% · milestones 0/4 · snapshot `831a7954` (2026-11-03)

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
| Gameplay classes (/Script/FortniteGame) | `███░░░░░░░░░░░░░░░░░` | 13.7% | 33% |
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
