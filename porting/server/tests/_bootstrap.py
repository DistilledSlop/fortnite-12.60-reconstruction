"""Make the sibling server modules importable during test discovery."""
import sys
from pathlib import Path

SERVER = Path(__file__).resolve().parent.parent
if str(SERVER) not in sys.path:
    sys.path.insert(0, str(SERVER))
