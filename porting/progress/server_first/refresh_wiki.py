#!/usr/bin/env python3
"""Regenerate the server-first wiki treemap; narrative lives on its wiki page.

Usage: python3 -I porting/progress/server_first/refresh_wiki.py /path/to/wiki
The classifier is a snapshot; this does not reclassify functions, build or test.
"""
import subprocess
import sys
from pathlib import Path

here = Path(__file__).resolve().parent
wiki = Path(sys.argv[1]).resolve(strict=True)
subprocess.run([sys.executable, '-I', str(here / 'render_server_first.py'),
                str(wiki / 'server-first-treemap-v4.svg')], check=True)
