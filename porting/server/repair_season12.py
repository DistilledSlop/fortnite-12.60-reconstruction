"""Repair a local account's missing Season 12 level-one rewards while the server is stopped."""
import argparse
import fcntl
from pathlib import Path
from backend import Backend

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("state_dir")
    parser.add_argument("account_id")
    args = parser.parse_args()
    directory = Path(args.state_dir)
    directory.mkdir(parents=True, exist_ok=True)
    with (directory / ".backend.lock").open("a") as lock:
        try:
            fcntl.flock(lock, fcntl.LOCK_EX | fcntl.LOCK_NB)
        except BlockingIOError:
            parser.error("state directory is already in use; stop the backend first")
        count = Backend(state_dir=directory).profiles.repair_season12_level_one(args.account_id)
        print(f"Added {count} missing Season 12 level-one items.")
