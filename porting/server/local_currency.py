#!/usr/bin/env python3
"""Credit virtual V-Bucks to a local account while its backend is stopped."""
import argparse
import fcntl
from pathlib import Path

from backend import Backend
from profile_service import ProfileTx


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--state-dir", required=True, type=Path)
    parser.add_argument("--username", required=True)
    parser.add_argument("--amount", required=True, type=int, help="positive number of local V-Bucks to add")
    args = parser.parse_args()
    if args.amount <= 0:
        parser.error("amount must be positive")
    args.state_dir.mkdir(parents=True, exist_ok=True)
    with (args.state_dir / ".backend.lock").open("a") as lock:
        try:
            fcntl.flock(lock, fcntl.LOCK_EX | fcntl.LOCK_NB)
        except BlockingIOError:
            parser.error("state directory is already in use; stop the backend first")
        backend = Backend(state_dir=args.state_dir)
        account = backend.accounts.get_or_create_username(args.username)["id"]
        service = backend.profiles
        tx = ProfileTx(service.ctx, service.profile(account, "common_core"))
        currency = next((item_id for item_id, item in tx.profile["items"].items()
                         if item["templateId"] == "Currency:MtxGiveaway"), None)
        if currency is None:
            raise RuntimeError("local currency item missing from profile")
        balance = tx.item(currency)["quantity"] + args.amount
        tx.set_item_quantity(currency, balance)
        service._store(account, {"common_core": tx})
        print(f"Local V-Bucks balance for {args.username}: {balance}")


if __name__ == "__main__":
    main()
