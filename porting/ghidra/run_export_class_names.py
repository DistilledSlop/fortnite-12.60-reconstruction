#!/usr/bin/env python3
"""Run porting/ghidra/export_class_names.py through PyGhidra against the imported dump project.

Opens the program already imported by analyzeHeadless (no re-import, no analysis rerun) and runs
the script read-only. Needs porting/.venv (jpype1 + Ghidra's pypkg). The program and project
default to the Fortnite runtime dump; the Ghidra install comes from GHIDRA_INSTALL_DIR or the
default below.

Usage: porting/.venv/bin/python -I porting/ghidra/run_export_class_names.py [REFLECTION_JSON OUT_JSON]
"""
import os
import sys

import pyghidra

REPO = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
GENERATED = os.path.join(REPO, "porting", "generated")
PROJECT_DIR = "/opt/fortnite/ghidra-projects"
PROJECT = "fortnite"
PROGRAM = os.path.join(GENERATED, "dump", "FortniteClient-Win64-Shipping.runtime.exe")
SCRIPT = os.path.join(REPO, "porting", "ghidra", "export_class_names.py")
DEFAULT_REFLECTION = os.path.join(GENERATED, "reflection", "fortnite_game_reflection.json")
DEFAULT_OUT = os.path.join(GENERATED, "reflection", "class_name_map.json")


def main():
    args = sys.argv[1:] or [DEFAULT_REFLECTION, DEFAULT_OUT]
    install = os.environ.get("GHIDRA_INSTALL_DIR",
                             "/opt/fortnite/ghidra-install/ghidra_12.2_PUBLIC")
    pyghidra.start(install_dir=install)
    pyghidra.run_script(PROGRAM, SCRIPT, project_location=PROJECT_DIR, project_name=PROJECT,
                        script_args=args, analyze=False)


if __name__ == "__main__":
    main()
