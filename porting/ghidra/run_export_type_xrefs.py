#!/usr/bin/env python3
"""Run porting/ghidra/export_type_xrefs.py through PyGhidra against the imported dump project.

Opens the program already imported by analyzeHeadless (no re-import, no analysis rerun) and runs
export_type_xrefs.py read-only (reference sites of parameter blocks, see porting/extract_types.py). Needs porting/.venv (jpype1 + Ghidra's pypkg). The program and project
default to the slim analysis of the Fortnite runtime dump (project fortnite-slim); override with
FN_GHIDRA_PROJECT_DIR, FN_GHIDRA_PROJECT and FN_GHIDRA_PROGRAM. The Ghidra install comes from
GHIDRA_INSTALL_DIR or the default below. The project must not be locked by a running analysis.

Usage: porting/.venv/bin/python -I porting/ghidra/run_export_type_xrefs.py [REFLECTION_JSON OUT_JSON]
"""
import os
import sys

import pyghidra

REPO = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
GENERATED = os.path.join(REPO, "porting", "generated")
# Defaults point at the slim project (porting/generated/ghidra/slim-import.log). Override with
# FN_GHIDRA_PROJECT_DIR / FN_GHIDRA_PROJECT for another project (e.g. a small test project).
PROJECT_DIR = os.environ.get("FN_GHIDRA_PROJECT_DIR", "/opt/fortnite/ghidra-projects")
PROJECT = os.environ.get("FN_GHIDRA_PROJECT", "fortnite-slim")
PROGRAM = os.environ.get("FN_GHIDRA_PROGRAM",
                         os.path.join(GENERATED, "dump", "FortniteClient-Win64-Shipping.runtime.exe"))
SCRIPT = os.path.join(REPO, "porting", "ghidra", "export_type_xrefs.py")


def main():
    args = sys.argv[1:]
    if len(args) != 2:
        raise SystemExit("usage: run_export_type_xrefs.py CANDIDATES_JSON OUT_JSON")
    install = os.environ.get("GHIDRA_INSTALL_DIR",
                             "/opt/fortnite/ghidra-install/ghidra_12.2_PUBLIC")
    pyghidra.start(install_dir=install)
    # nested_project_location=False: the project is PROJECT_DIR/PROJECT.gpr, as analyzeHeadless wrote
    # it. The default (True) looks for PROJECT_DIR/PROJECT/PROJECT.gpr, finds nothing, and imports a
    # fresh unanalyzed copy into a new project.
    pyghidra.run_script(PROGRAM, SCRIPT, project_location=PROJECT_DIR, project_name=PROJECT,
                        script_args=args, analyze=False, nested_project_location=False)


if __name__ == "__main__":
    main()
