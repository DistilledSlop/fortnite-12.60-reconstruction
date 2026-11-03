<div align="center">

# Fortnite 12.60 → Linux

**A from-scratch reconstruction of Fortnite 12.60 (CL 13477524) as a buildable Unreal Engine 4.25 project, with a reimplemented dedicated server and local backend.**

[Dashboard](#progress-dashboard) ·
[Treemap SVG](docs/progress.svg) ·
[Data](docs/progress.json) ·
[Handoff](porting/HANDOFF.md) ·
[Master plan](porting/master_plan.md)

<!-- badges:start -->
<table align="center">
<thead><tr>
<th align="center">Progress</th>
<th align="center">Target</th>
<th align="center">Stack</th>
<th align="center">Evidence</th>
<th align="center">History</th>
</tr></thead>
<tbody>
<tr><td align="center" valign="middle"><img alt="overall: 20.7%" src="https://img.shields.io/static/v1?label=overall&amp;message=20.7%25&amp;color=2a78d6"></td><td align="center" valign="middle"><img alt="Fortnite: 12.60" src="https://img.shields.io/static/v1?label=Fortnite&amp;message=12.60&amp;color=7d4cdb&amp;logo=epicgames"></td><td align="center" valign="middle"><img alt="C++: 13.9k files" src="https://img.shields.io/static/v1?label=C%2B%2B&amp;message=13.9k%20files&amp;color=00599c&amp;logo=cplusplus"></td><td align="center" valign="middle"><img alt="areas: 9" src="https://img.shields.io/static/v1?label=areas&amp;message=9&amp;color=0e7490"></td><td align="center" valign="middle"><img alt="commits: 570" src="https://img.shields.io/static/v1?label=commits&amp;message=570&amp;color=1f6feb&amp;logo=git"></td></tr>
<tr><td align="center" valign="middle"><img alt="evidence-based: 19.8%" src="https://img.shields.io/static/v1?label=evidence-based&amp;message=19.8%25&amp;color=3987e5"></td><td align="center" valign="middle"><img alt="build: CL 13477524" src="https://img.shields.io/static/v1?label=build&amp;message=CL%2013477524&amp;color=6a3fc4"></td><td align="center" valign="middle"><img alt="Python: 440 scripts" src="https://img.shields.io/static/v1?label=Python&amp;message=440%20scripts&amp;color=3776ab&amp;logo=python"></td><td align="center" valign="middle"><img alt="components: 135" src="https://img.shields.io/static/v1?label=components&amp;message=135&amp;color=0e7490"></td><td align="center" valign="middle"><img alt="tracked files: 14.9k" src="https://img.shields.io/static/v1?label=tracked%20files&amp;message=14.9k&amp;color=1f6feb"></td></tr>
<tr><td align="center" valign="middle"><img alt="objective: 19.8%" src="https://img.shields.io/static/v1?label=objective&amp;message=19.8%25&amp;color=5a9cec"></td><td align="center" valign="middle"><img alt="Unreal Engine: 4.25.0" src="https://img.shields.io/static/v1?label=Unreal%20Engine&amp;message=4.25.0&amp;color=0e1128&amp;logo=unrealengine"></td><td align="center" valign="middle"><img alt="C#: 22 files" src="https://img.shields.io/static/v1?label=C%23&amp;message=22%20files&amp;color=512bd4"></td><td align="center" valign="middle"><img alt="units done: 1.09M/1.37M" src="https://img.shields.io/static/v1?label=units%20done&amp;message=1.09M%2F1.37M&amp;color=0e7490"></td><td align="center" valign="middle"><img alt="since: 2026-10-30" src="https://img.shields.io/static/v1?label=since&amp;message=2026-10-30&amp;color=9f9f9f"></td></tr>
<tr><td align="center" valign="middle"><img alt="milestones: 0/4" src="https://img.shields.io/static/v1?label=milestones&amp;message=0%2F4&amp;color=d6a02a"></td><td align="center" valign="middle"><img alt="platform: Linux x64" src="https://img.shields.io/static/v1?label=platform&amp;message=Linux%20x64&amp;color=e8a317&amp;logo=linux"></td><td align="center" valign="middle"><img alt="shell: 16 scripts" src="https://img.shields.io/static/v1?label=shell&amp;message=16%20scripts&amp;color=4eaa25&amp;logo=gnubash"></td><td align="center" valign="middle"><img alt="class parity: 4,617/4,617" src="https://img.shields.io/static/v1?label=class%20parity&amp;message=4%2C617%2F4%2C617&amp;color=2e8540"></td><td align="center" valign="middle"><img alt="status: work in progress" src="https://img.shields.io/static/v1?label=status&amp;message=work%20in%20progress&amp;color=d6a02a"></td></tr>
<tr><td align="center" valign="middle"><img alt="snapshot: cc388a07" src="https://img.shields.io/static/v1?label=snapshot&amp;message=cc388a07&amp;color=9f9f9f&amp;logo=git"></td><td align="center" valign="middle"><img alt="renderer: Vulkan" src="https://img.shields.io/static/v1?label=renderer&amp;message=Vulkan&amp;color=a41e22&amp;logo=vulkan"></td><td align="center" valign="middle"><img alt="code: 454.2k lines" src="https://img.shields.io/static/v1?label=code&amp;message=454.2k%20lines&amp;color=2f6f4f"></td><td align="center" valign="middle"></td><td align="center" valign="middle"><img alt="authored by: Codex" src="https://img.shields.io/static/v1?label=authored%20by&amp;message=Codex&amp;color=10a37f"></td></tr>
<tr><td align="center" valign="middle"></td><td align="center" valign="middle"><img alt="paks: 31 · v10" src="https://img.shields.io/static/v1?label=paks&amp;message=31%20%C2%B7%20v10&amp;color=9f9f9f"></td><td align="center" valign="middle"></td><td align="center" valign="middle"></td><td align="center" valign="middle"><img alt="co-authored by: Claude" src="https://img.shields.io/static/v1?label=co-authored%20by&amp;message=Claude&amp;color=d97757&amp;logo=anthropic"></td></tr>
<tr><td align="center" valign="middle"></td><td align="center" valign="middle"><img alt="oracle: Wine" src="https://img.shields.io/static/v1?label=oracle&amp;message=Wine&amp;color=722f37&amp;logo=wine"></td><td align="center" valign="middle"></td><td align="center" valign="middle"></td><td align="center" valign="middle"></td></tr>
</tbody>
</table>
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

**Overall: 20.7%** · evidence-based 19.8% · objective 19.8% · milestones 0/4 · snapshot `cc388a07` (2026-11-03)

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
