#!/usr/bin/env python3
"""Render the server acceptance checklist with the original progress treemap."""
import argparse
from pathlib import Path

import progress

ROOT = Path(__file__).resolve().parents[2]
OUTPUT = ROOT / "docs/progress/server/progress.svg"

# Completed checks cite recorded evidence. Open checks specify what still needs
# a result. Each check counts once and contributes one unit of tile area.
CHECKLIST = {
    "Hosting and match flow": {
        "Linux build and launch": (
            ["Linux dedicated target builds (build_server.sh)",
             "Staged content boots and runs headless automation (417bfc45)"],
            ["Clean-host build, staging and launch"],
        ),
        "Client join and island drop": (
            ["Original 12.60 client joins a local server (November 13 island capture)",
             "Original client enters Apollo after the bus drop (README local match evidence)"],
            ["Two clients independently possess pawns and drop onto Apollo"],
        ),
        "Storm and elimination": (
            ["Squad wipe and last-standing elimination pass headless checks (417bfc45)"],
            ["Full storm sequence applies damage and phases correctly",
             "Multiple clients receive correct placement and spectator targets"],
        ),
        "Results and requeue": (
            ["Original client reaches local results screen (November 15 results capture)"],
            ["Multiplayer results agree across clients",
             "Return to lobby, requeue and clear previous match state"],
        ),
    },
    "Authoritative gameplay": {
        "Damage, health and shield": (
            ["Point and radial damage and shield overflow pass headless checks (1c459c7e)",
             "Cooked explosive and recovery effects pass headless checks (7224cefa, 8a616111)"],
            ["Live weapon families and consumables resolve correctly",
             "Environmental hazards and PvE damage resolve correctly"],
        ),
        "Team knock and revive": (
            ["Team knock, crawl stance and bleed pass headless checks (417bfc45, c6645a88)",
             "Carry, drop and throw pass headless checks (b3295a17, d714d0e5)",
             "Revive start, cancellation and completion pass headless checks (2e88d5f1, 2b0c4d87)"],
            ["Original clients agree on crawl, carry and throw movement and animations",
             "Original clients show and clear revive prompts and progress correctly"],
        ),
        "Loot and inventory": (
            ["Loot selection has automated checks (FortAthenaLootRollTests.cpp)"],
            ["Live chest search and contested pickups replicate",
             "Inventory swaps, stacking and elimination drops replicate"],
        ),
        "Weapons and building": (
            [],
            ["Weapon families fire and reload correctly across clients",
             "Projectile travel, impact and damage agree across clients",
             "Building placement, edits, destruction and costs agree across clients"],
        ),
        "World actors": (
            ["AI and structure damage paths have headless coverage (ServerDamageExecutionTests.cpp)"],
            ["AI spawn, behavior and combat in a match",
             "Vehicle entry, driving, exit and replication",
             "Interactable props and reboot stations in a match"],
        ),
    },
    "Networking and backend": {
        "Identity, profiles and lobby": (
            ["Original client exercises backend routes in local probes (porting/server/EVIDENCE.md)"],
            ["Fresh and returning accounts and profile mutations pass without diagnostic bypasses",
             "Party transitions and reconnects pass without diagnostic bypasses"],
        ),
        "Matchmaking and sessions": (
            [],
            ["A live party queues and receives the same assigned session",
             "Party members join together and can requeue after a match"],
        ),
        "Replication and stability": (
            ["A local client receives enough replicated state to enter Apollo (November 13 island capture)"],
            ["Actor and gameplay state agree across clients in a sustained match",
             "Server tick times and movement corrections stay within measured targets"],
        ),
    },
}


def measure():
    groups = []
    for group_name, entries in CHECKLIST.items():
        children = []
        for name, (passed, pending) in entries.items():
            done, total = len(passed), len(passed) + len(pending)
            evidence = [*(f"Recorded: {item}" for item in passed),
                        *(f"Open: {item}" for item in pending)]
            children.append(dict(id=name, name=name, weight=total, pct=100 * done / total,
                                 done=done, total=total, basis="server acceptance checklist",
                                 evidence="\n".join(evidence)))
        done = sum(child["done"] for child in children)
        total = sum(child["total"] for child in children)
        groups.append(dict(name=group_name, weight=total, pct=100 * done / total,
                           done=done, total=total, children=children,
                           basis="server acceptance checklist"))
    return groups


def render():
    groups = measure()
    done = sum(group["done"] for group in groups)
    total = sum(group["total"] for group in groups)
    width, height = progress.STANDALONE_W, progress.STANDALONE_H
    body = progress.render_treemap(groups, width, height - 88, prefix="server")
    return f'''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {width} {height}" width="{width}" height="{height}" role="img" aria-label="Dedicated-server progress treemap" font-family="system-ui,-apple-system,Segoe UI,sans-serif">
<style>{progress.TREEMAP_CSS}</style>
<rect class="tm-bg" width="{width}" height="{height}"/>
<text x="12" y="28" font-size="20" font-weight="700" class="tm-head">Fortnite 12.60 server progress: {done / total:.1%} verified</text>
<text x="12" y="48" font-size="12" class="tm-head">{done} of {total} server acceptance checks have recorded evidence</text>
<g transform="translate(0,60)">{body}</g>
<text x="12" y="{height - 10}" font-size="11" class="tm-head">Each check counts once. Tile area reflects check count. Blue fill shows verified checks.</text>
</svg>
'''


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true", help="fail if the README treemap is stale")
    args = parser.parse_args()
    svg = render()
    if args.check:
        if not OUTPUT.is_file() or OUTPUT.read_text() != svg:
            parser.exit(1, f"Stale server treemap: {OUTPUT}\n")
    else:
        OUTPUT.write_text(svg)


if __name__ == "__main__":
    main()
