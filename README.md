<div align="center">

# Fortnite Chapter 2 Season 2 → Unreal Engine Project

Reconstructing Fortnite Chapter 2 Season 2 as an Unreal Engine project, with recovered C++ source, a dedicated server, and local backend services.

[Wiki](https://github.com/DistilledSlop/fortnite-12.60-reconstruction/wiki) ·
[Source reference](https://github.com/DistilledSlop/fortnite-12.60-reconstruction/wiki/Source-and-API-Index) ·
[Progress](#progress) ·
[Setup](https://github.com/DistilledSlop/fortnite-12.60-reconstruction/wiki/Setup-and-Workflow)

<!-- badges:start -->
<p align="center"><img alt="overall: 29.2%" src="https://img.shields.io/static/v1?label=overall&amp;message=29.2%25&amp;color=2a78d6"> <img alt="evidence: 28.4%" src="https://img.shields.io/static/v1?label=evidence&amp;message=28.4%25&amp;color=3987e5"> <img alt="objective: 28.4%" src="https://img.shields.io/static/v1?label=objective&amp;message=28.4%25&amp;color=5a9cec"> <img alt="milestones: 0/4" src="https://img.shields.io/static/v1?label=milestones&amp;message=0%2F4&amp;color=d6a02a"> <img alt="units: 1.34M/1.38M" src="https://img.shields.io/static/v1?label=units&amp;message=1.34M%2F1.38M&amp;color=0e7490"> <img alt="snapshot: bdce93ea" src="https://img.shields.io/static/v1?label=snapshot&amp;message=bdce93ea&amp;color=9f9f9f&amp;logo=git"> <img alt="updated: 2026-11-07" src="https://img.shields.io/static/v1?label=updated&amp;message=2026-11-07&amp;color=9f9f9f"></p>
<p align="center"><img alt="M1 menu: open" src="https://img.shields.io/static/v1?label=M1%20menu&amp;message=open&amp;color=9f9f9f"> <img alt="M2 lobby: open" src="https://img.shields.io/static/v1?label=M2%20lobby&amp;message=open&amp;color=9f9f9f"> <img alt="M3 matchmaking: open" src="https://img.shields.io/static/v1?label=M3%20matchmaking&amp;message=open&amp;color=9f9f9f"> <img alt="M4 match: open" src="https://img.shields.io/static/v1?label=M4%20match&amp;message=open&amp;color=9f9f9f"> <img alt="client: unstripped" src="https://img.shields.io/static/v1?label=client&amp;message=unstripped&amp;color=7d4cdb"> <img alt="server: reconstructed" src="https://img.shields.io/static/v1?label=server&amp;message=reconstructed&amp;color=e8a317"> <img alt="goal: full A/V" src="https://img.shields.io/static/v1?label=goal&amp;message=full%20A%2FV&amp;color=a41e22"></p>
<p align="center"><img alt="platform: 26.0%" src="https://img.shields.io/static/v1?label=platform&amp;message=26.0%25&amp;color=97ca00"> <img alt="assets: 65.8%" src="https://img.shields.io/static/v1?label=assets&amp;message=65.8%25&amp;color=2e8540"> <img alt="maps: 13.9%" src="https://img.shields.io/static/v1?label=maps&amp;message=13.9%25&amp;color=dfb317"> <img alt="gameplay: 17.4%" src="https://img.shields.io/static/v1?label=gameplay&amp;message=17.4%25&amp;color=dfb317"> <img alt="network: 27.1%" src="https://img.shields.io/static/v1?label=network&amp;message=27.1%25&amp;color=97ca00"> <img alt="backend: 46.3%" src="https://img.shields.io/static/v1?label=backend&amp;message=46.3%25&amp;color=97ca00"> <img alt="audio: 27.3%" src="https://img.shields.io/static/v1?label=audio&amp;message=27.3%25&amp;color=97ca00"></p>
<p align="center"><img alt="visuals: 22.4%" src="https://img.shields.io/static/v1?label=visuals&amp;message=22.4%25&amp;color=dfb317"> <img alt="frontend UI: 23.1%" src="https://img.shields.io/static/v1?label=frontend%20UI&amp;message=23.1%25&amp;color=dfb317"> <img alt="components: 136" src="https://img.shields.io/static/v1?label=components&amp;message=136&amp;color=2a78d6"> <img alt="areas: 9" src="https://img.shields.io/static/v1?label=areas&amp;message=9&amp;color=7d4cdb"> <img alt="class parity: 4,617/4,617" src="https://img.shields.io/static/v1?label=class%20parity&amp;message=4%2C617%2F4%2C617&amp;color=2e8540"> <img alt="git-measured: 61 leaves" src="https://img.shields.io/static/v1?label=git-measured&amp;message=61%20leaves&amp;color=4eaa25"> <img alt="manual: 60 leaves" src="https://img.shields.io/static/v1?label=manual&amp;message=60%20leaves&amp;color=9f9f9f"></p>
<p align="center"><img alt="Fortnite: 12.60" src="https://img.shields.io/static/v1?label=Fortnite&amp;message=12.60&amp;color=7d4cdb&amp;logo=epicgames"> <img alt="build: CL 13477524" src="https://img.shields.io/static/v1?label=build&amp;message=CL%2013477524&amp;color=6a3fc4"> <img alt="UE: 4.25.0" src="https://img.shields.io/static/v1?label=UE&amp;message=4.25.0&amp;color=0e1128&amp;logo=unrealengine"> <img alt="platform: Linux x64" src="https://img.shields.io/static/v1?label=platform&amp;message=Linux%20x64&amp;color=e8a317&amp;logo=linux"> <img alt="renderer: Vulkan" src="https://img.shields.io/static/v1?label=renderer&amp;message=Vulkan&amp;color=a41e22&amp;logo=vulkan"> <img alt="oracle: Wine" src="https://img.shields.io/static/v1?label=oracle&amp;message=Wine&amp;color=722f37&amp;logo=wine"> <img alt="original: Win64" src="https://img.shields.io/static/v1?label=original&amp;message=Win64&amp;color=0078d4"></p>
<p align="center"><img alt="paks: 31 · v10" src="https://img.shields.io/static/v1?label=paks&amp;message=31%20%C2%B7%20v10&amp;color=5b4a8a"> <img alt="pak entries: 963,427" src="https://img.shields.io/static/v1?label=pak%20entries&amp;message=963%2C427&amp;color=0e7490"> <img alt=".uasset: 450,951" src="https://img.shields.io/static/v1?label=.uasset&amp;message=450%2C951&amp;color=2e8540"> <img alt="maps: 3,694" src="https://img.shields.io/static/v1?label=maps&amp;message=3%2C694&amp;color=b08800"> <img alt="plugins: 123" src="https://img.shields.io/static/v1?label=plugins&amp;message=123&amp;color=a41e22"> <img alt="modules: 6" src="https://img.shields.io/static/v1?label=modules&amp;message=6&amp;color=512bd4"> <img alt="API routes: 59" src="https://img.shields.io/static/v1?label=API%20routes&amp;message=59&amp;color=e8a317"></p>
<p align="center"><img alt="extraction: CUE4Parse" src="https://img.shields.io/static/v1?label=extraction&amp;message=CUE4Parse&amp;color=512bd4"> <img alt="decompiler: Ghidra 12.2" src="https://img.shields.io/static/v1?label=decompiler&amp;message=Ghidra%2012.2&amp;color=b5301f"> <img alt="pak index: v10" src="https://img.shields.io/static/v1?label=pak%20index&amp;message=v10&amp;color=5b4a8a"> <img alt="net: IpNetDriver" src="https://img.shields.io/static/v1?label=net&amp;message=IpNetDriver&amp;color=a41e22"> <img alt="UI: CommonUI" src="https://img.shields.io/static/v1?label=UI&amp;message=CommonUI&amp;color=7d4cdb"> <img alt="parity: checksum" src="https://img.shields.io/static/v1?label=parity&amp;message=checksum&amp;color=2e8540"> <img alt="tooling: Python" src="https://img.shields.io/static/v1?label=tooling&amp;message=Python&amp;color=3776ab&amp;logo=python"></p>
<p align="center"><img alt="C++: 4,781 .cpp" src="https://img.shields.io/static/v1?label=C%2B%2B&amp;message=4%2C781%20.cpp&amp;color=00599c&amp;logo=cplusplus"> <img alt="headers: 8,270 .h" src="https://img.shields.io/static/v1?label=headers&amp;message=8%2C270%20.h&amp;color=0e7490"> <img alt="recovered: 3,983 .inc" src="https://img.shields.io/static/v1?label=recovered&amp;message=3%2C983%20.inc&amp;color=d97757"> <img alt="Python: 521 scripts" src="https://img.shields.io/static/v1?label=Python&amp;message=521%20scripts&amp;color=3776ab&amp;logo=python"> <img alt="C#: 24 files" src="https://img.shields.io/static/v1?label=C%23&amp;message=24%20files&amp;color=512bd4"> <img alt="shell: 17 scripts" src="https://img.shields.io/static/v1?label=shell&amp;message=17%20scripts&amp;color=4eaa25&amp;logo=gnubash"> <img alt="patches: 22" src="https://img.shields.io/static/v1?label=patches&amp;message=22&amp;color=6e5494"></p>
<p align="center"><img alt="C++ lines: 454.1k" src="https://img.shields.io/static/v1?label=C%2B%2B%20lines&amp;message=454.1k&amp;color=00599c"> <img alt="Python lines: 76.0k" src="https://img.shields.io/static/v1?label=Python%20lines&amp;message=76.0k&amp;color=3776ab"> <img alt="C# lines: 2,808" src="https://img.shields.io/static/v1?label=C%23%20lines&amp;message=2%2C808&amp;color=512bd4"> <img alt="shell lines: 791" src="https://img.shields.io/static/v1?label=shell%20lines&amp;message=791&amp;color=4eaa25"> <img alt="docs: 25.0k lines" src="https://img.shields.io/static/v1?label=docs&amp;message=25.0k%20lines&amp;color=0e7490"> <img alt="markdown: 303 files" src="https://img.shields.io/static/v1?label=markdown&amp;message=303%20files&amp;color=6e5494"> <img alt="data: 395 JSON" src="https://img.shields.io/static/v1?label=data&amp;message=395%20JSON&amp;color=b08800"></p>
<p align="center"><img alt="native: 12.9k files" src="https://img.shields.io/static/v1?label=native&amp;message=12.9k%20files&amp;color=00599c"> <img alt="behavior: 3,555 files" src="https://img.shields.io/static/v1?label=behavior&amp;message=3%2C555%20files&amp;color=b5301f"> <img alt="frontend: 787 files" src="https://img.shields.io/static/v1?label=frontend&amp;message=787%20files&amp;color=7d4cdb"> <img alt="server: 41 files" src="https://img.shields.io/static/v1?label=server&amp;message=41%20files&amp;color=e8a317"> <img alt="assets: 16 files" src="https://img.shields.io/static/v1?label=assets&amp;message=16%20files&amp;color=2e8540"> <img alt="materials: 159 files" src="https://img.shields.io/static/v1?label=materials&amp;message=159%20files&amp;color=d97757"> <img alt="dashboard: 66 files" src="https://img.shields.io/static/v1?label=dashboard&amp;message=66%20files&amp;color=2a78d6"></p>
<p align="center"><img alt="tracked files: 18.4k" src="https://img.shields.io/static/v1?label=tracked%20files&amp;message=18.4k&amp;color=0e7490"> <img alt="commits: 1,185" src="https://img.shields.io/static/v1?label=commits&amp;message=1%2C185&amp;color=1f6feb&amp;logo=git"> <img alt="branches: 14" src="https://img.shields.io/static/v1?label=branches&amp;message=14&amp;color=7d4cdb"> <img alt="since: 2026-10-30" src="https://img.shields.io/static/v1?label=since&amp;message=2026-10-30&amp;color=9f9f9f"> <img alt="authored by: Codex" src="https://img.shields.io/static/v1?label=authored%20by&amp;message=Codex&amp;color=10a37f"> <img alt="authored by: Claude" src="https://img.shields.io/static/v1?label=authored%20by&amp;message=Claude&amp;color=d97757&amp;logo=anthropic"> <img alt="status: WIP" src="https://img.shields.io/static/v1?label=status&amp;message=WIP&amp;color=dfb317"></p>
<p align="center"><img alt="agents: ≤3 at once" src="https://img.shields.io/static/v1?label=agents&amp;message=%E2%89%A43%20at%20once&amp;color=d97757&amp;logo=anthropic"> <img alt="commits: each batch" src="https://img.shields.io/static/v1?label=commits&amp;message=each%20batch&amp;color=1f6feb"> <img alt="history: per commit" src="https://img.shields.io/static/v1?label=history&amp;message=per%20commit&amp;color=2a78d6"> <img alt="evidence: ledgered" src="https://img.shields.io/static/v1?label=evidence&amp;message=ledgered&amp;color=2e8540"> <img alt="keys: git-ignored" src="https://img.shields.io/static/v1?label=keys&amp;message=git-ignored&amp;color=e05d44"> <img alt="assets: git-ignored" src="https://img.shields.io/static/v1?label=assets&amp;message=git-ignored&amp;color=fe7d37"> <img alt="checks: batched" src="https://img.shields.io/static/v1?label=checks&amp;message=batched&amp;color=dfb317"> <a href="porting/progress/WATER_REPORT.md" title="Central scenario 354.6 US gal; refreshed 2026-11-06T23:51:55+00:00. Partial recorded usage; assumptions and coverage in report."><img alt="AI water est.: ~350 gal (2.9–10,000)" src="https://img.shields.io/static/v1?label=AI%20water%20est.&amp;message=~350%20gal%20%282.9%E2%80%9310%2C000%29&amp;color=2a78d6"></a></p>
<!-- badges:end -->

</div>

The reference build is **12.60, CL 13477524**. The current development environment uses Unreal Engine 4.25 on Linux; the project's goal is to recover the game as an Unreal Engine project. It is under active reconstruction and is not yet a complete, playable release.

## Scope

- Recover the client's classes, function behavior, assets pipeline, frontend, and world systems from the supplied build.
- Reconstruct the dedicated server and local account, profile, matchmaking, session, party, and XMPP services.
- Verify the rebuilt client through the main menu, lobby, matchmaking, and a full match with working audio and visuals.

The original executable is a reference for analysis. The deliverable is the rebuilt Unreal Engine project and its services.

## Documentation

The [wiki](https://github.com/DistilledSlop/fortnite-12.60-reconstruction/wiki) collects project setup, architecture, source and API guides, Maps & World documentation, and subsystem guides linked to detailed source evidence.

| Start here | Contents |
|---|---|
| [Project status](https://github.com/DistilledSlop/fortnite-12.60-reconstruction/wiki/Project-Status) | Current limitations and acceptance milestones |
| [Source and API index](https://github.com/DistilledSlop/fortnite-12.60-reconstruction/wiki/Source-and-API-Index) | Source layout, recovery conventions, and code references |
| [Maps and world](https://github.com/DistilledSlop/fortnite-12.60-reconstruction/wiki/Maps-and-world) | Apollo, streaming, terrain, wind, and remaining integration |
| [Documentation index](https://github.com/DistilledSlop/fortnite-12.60-reconstruction/wiki/Documentation-Index) | Project and subsystem guides |

## Progress

[![Reconstruction progress by area](docs/progress.svg)](docs/progress.svg)

<!-- progress:start -->

**Estimated reconstruction effort: 29.2%** · runtime milestones: 0/4 · snapshot `bdce93ea` (2026-11-07)

| Runtime milestone | Status |
|---|---|
| The rebuilt client reaches the main menu with feature parity to the 12.60 original | Open |
| The client joins a lobby on the reconstructed server | Open |
| The client matchmakes to the reconstructed UE dedicated server | Open |
| A full match is played with audio and visuals working | Open |

Percentages estimate implementation and verification effort. Recovered source, extracted assets, and offline checks do not prove a playable feature. [Dashboard methodology](porting/progress/README.md).

<!-- progress:end -->

The [interactive dashboard](docs/index.html) contains area estimates and evidence. To view it locally, run `porting/progress/serve.sh`. The dashboard refreshes after commits.

## Repository

| Path | Contents |
|---|---|
| [porting/native](porting/native) | Unreal Engine project, generated classes, recovered implementations, and plugins |
| [porting/behavior](porting/behavior) | Function implementations, manifests, and recovery evidence |
| [porting/frontend](porting/frontend) | Frontend and lobby reconstruction |
| [porting/server](porting/server) | Local backend services |
| [porting/asset_pipeline](porting/asset_pipeline) and [porting/material_pipeline](porting/material_pipeline) | Asset conversion tools |
| [porting/progress](porting/progress) | Dashboard and evidence accounting |

The repository contains reconstruction code, tools, and documentation. The supplied game files, Unreal Engine checkout, extracted assets, build products, and encryption keys stay outside Git. See [.gitignore](.gitignore).
