"""Trusted local match results, atomic with MCP profiles.

The result protocol and stats counter names are local inferred contracts, not
captured Epic wire formats. Callers MUST authenticate the dedicated server and
check its session/account membership before calling record_result. Client MCP
commands never call this service. xp/bookXp are trusted server-computed totals;
alternatively accolades supplies original AthenaAccoladeXP row names and counts.
Placement alone does not imply a medal: those trigger rules are not recovered.

xp and book_xp are XP remaining in the current level (inferred MCP semantics).
Purchased book levels are preserved; season/account levels advance independently.
"""

import copy
import json
import re
import sqlite3
import threading
from contextlib import closing
from datetime import datetime
from pathlib import Path

from api_common import ApiError, Err
from profile_service import CommandContext, _grant_season12_rewards
from season12_rewards import FREE, PAID
from season12_progression_data import ACCOLADE_XP, XP_TO_NEXT


def _invalid(message):
    return ApiError(400, Err.INVALID_PAYLOAD, message)


def _integer(value, name, minimum=0, maximum=2**31 - 1):
    if isinstance(value, bool) or not isinstance(value, int) or not minimum <= value <= maximum:
        raise _invalid(f"{name} must be an integer in [{minimum}, {maximum}].")
    return value


def _text(value, name, maximum=128):
    if not isinstance(value, str) or not re.fullmatch(r"[A-Za-z0-9_.:-]{1," + str(maximum) + r"}", value):
        raise _invalid(f"{name} has an invalid format.")
    return value


def _advance(level, remainder, award, cap):
    level = _integer(level, "stored level", 1, cap)
    remainder = _integer(remainder, "stored XP", 0, 2**63 - 1) + award
    while level < cap and remainder >= XP_TO_NEXT[level - 1]:
        remainder -= XP_TO_NEXT[level - 1]
        level += 1
    # At the cap the remainder is retained; no invented next-level threshold.
    return level, remainder


class ProgressionService:
    def __init__(self, ctx, accounts, profiles, *, state_dir=None):
        self.ctx, self.accounts, self.profiles = ctx, accounts, profiles
        self._database = profiles._database
        self._receipts = {}
        self._lock = threading.RLock()
        if state_dir is not None and self._database != Path(state_dir) / "profiles.sqlite3":
            raise ValueError("Progression must use the profile SQLite database for atomic commits.")
        if self._database is not None:
            with closing(sqlite3.connect(self._database)) as db, db:
                db.execute("CREATE TABLE IF NOT EXISTS match_results ("
                           "session_id TEXT NOT NULL, account_id TEXT NOT NULL, payload TEXT NOT NULL, "
                           "receipt TEXT NOT NULL, ended_at INTEGER NOT NULL, "
                           "PRIMARY KEY (session_id, account_id))")

    def _validate(self, session_id, payload):
        _text(session_id, "sessionId")
        if not isinstance(payload, dict):
            raise _invalid("Result must be an object.")
        if any(not isinstance(key, str) for key in payload):
            raise _invalid("Result field names must be strings.")
        allowed = {"accountId", "placement", "kills", "xp", "bookXp", "accolades", "playlist", "endedAt"}
        if set(payload) - allowed:
            raise _invalid("Unknown result fields: " + ", ".join(sorted(set(payload) - allowed)))
        data = {"accountId": _text(payload.get("accountId"), "accountId"),
                "placement": _integer(payload.get("placement"), "placement", 1, 100),
                "kills": _integer(payload.get("kills"), "kills", 0, 99),
                "playlist": _text(payload.get("playlist", "defaultsolo"), "playlist")}
        if "endedAt" in payload:
            value = payload["endedAt"]
            if not isinstance(value, str) or not re.fullmatch(r"\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}(?:\.\d{1,6})?Z", value):
                raise _invalid("endedAt must be a UTC ISO 8601 timestamp.")
            try:
                moment = datetime.fromisoformat(value.replace("Z", "+00:00"))
                if moment.timestamp() < 0:
                    raise ValueError()
            except ValueError:
                raise _invalid("endedAt must be a valid timestamp.")
            data["endedAt"] = moment.isoformat().replace("+00:00", "Z")
        if "accolades" in payload:
            accolades = payload["accolades"]
            if not isinstance(accolades, dict) or len(accolades) > len(ACCOLADE_XP):
                raise _invalid("accolades must map original row names to counts.")
            for name, count in accolades.items():
                if name not in ACCOLADE_XP:
                    raise _invalid("Unknown AthenaAccoladeXP row.")
                _integer(count, "accolade count", 0, 10000)
            data["accolades"] = dict(accolades)
            computed = sum(ACCOLADE_XP[name] * count for name, count in accolades.items())
            if "xp" in payload and payload["xp"] != computed:
                raise _invalid("xp does not match the accolade award total.")
            data["xp"] = _integer(computed, "xp")
        else:
            data["xp"] = _integer(payload.get("xp"), "xp")
        # Validate explicit XP even if equality with computed XP accepted a bool.
        if "xp" in payload:
            _integer(payload["xp"], "xp")
        data["bookXp"] = _integer(payload.get("bookXp", data["xp"]), "bookXp")
        if self.accounts.get(data["accountId"]) is None:
            raise ApiError(404, Err.ACCOUNT_NOT_FOUND, "The result account was not found.")
        return data, json.dumps(data, sort_keys=True, separators=(",", ":"))

    def _row(self, session_id, account_id, database=None):
        if self._database is None:
            return self._receipts.get((session_id, account_id))
        sql = "SELECT payload, receipt FROM match_results WHERE session_id=? AND account_id=?"
        if database is not None:
            return database.execute(sql, (session_id, account_id)).fetchone()
        with closing(sqlite3.connect(self._database)) as db:
            return db.execute(sql, (session_id, account_id)).fetchone()

    def has_result(self, session_id, account_id):
        return self._row(session_id, account_id) is not None

    @staticmethod
    def _replay(row, canonical):
        if row is None:
            return None
        if row[0] != canonical:
            raise ApiError(409, Err.INVALID_PAYLOAD, "A different result is already recorded for this session/account.")
        return json.loads(row[1])

    def replay_result(self, session_id, payload):
        """Read-only exact replay, including after process-local sessions expire."""
        data, canonical = self._validate(session_id, payload)
        return self._replay(self._row(session_id, data["accountId"]), canonical)

    def _changes(self, data):
        account_id = data["accountId"]
        command = CommandContext(self.profiles, account_id, "athena", {})
        athena = command.tx("athena")
        stats = athena.profile["stats"]["attributes"]
        if stats.get("season_num") != 12 or not isinstance(stats.get("book_purchased"), bool):
            raise _invalid("Season 12 progression is unavailable.")
        old_level = _integer(stats.get("level"), "stored level", 1, 1000)
        old_book_level = _integer(stats.get("book_level"), "stored book level", 1, 100)
        level, xp = _advance(old_level, stats.get("xp", 0), data["xp"], 1000)
        book_level, book_xp = _advance(old_book_level, stats.get("book_xp", 0), data["bookXp"], 100)
        account_level = _integer(stats.get("accountLevel", old_level), "stored accountLevel", 1)
        for name, value in (("level", level), ("xp", xp), ("book_level", book_level),
                            ("book_xp", book_xp), ("accountLevel", account_level + level - old_level)):
            athena.set_stat(name, value)
        for name, value in (("matchesplayed", 1), ("kills", data["kills"]),
                            ("wins", int(data["placement"] == 1))):
            athena.set_stat(name, _integer(stats.get(name, 0), "stored " + name) + value)
        rewards = [reward for n in range(old_book_level + 1, book_level + 1)
                   for schedule in ((FREE, PAID) if stats["book_purchased"] else (FREE,))
                   for reward in schedule.get(n, ())]
        if rewards:
            _grant_season12_rewards(command, athena, rewards, 0)
        return command

    def _receipt(self, session_id, data, command, ended_at):
        athena = command.txs["athena"]
        envelope = self.profiles._envelope(
            self.profiles._entry("athena", athena.profile, athena.base_rvn, athena.changes),
            command.notifications,
            [self.profiles._entry(pid, tx.profile, tx.base_rvn, tx.changes)
             for pid, tx in command.txs.items() if pid != "athena" and tx.changed])
        return {"sessionId": session_id, "accountId": data["accountId"], "recorded": True,
                "xpAwarded": data["xp"], "bookXpAwarded": data["bookXp"],
                "endedAt": ended_at, "profileUpdate": envelope}

    def record_result(self, session_id, payload):
        data, canonical = self._validate(session_id, payload)
        account_id = data["accountId"]
        with self._lock:
            replay = self._replay(self._row(session_id, account_id), canonical)
            if replay is not None:
                return replay
            # Defaults are initialized before BEGIN; all result effects, reward
            # profiles, and the receipt then share one SQLite transaction.
            for pid in ("athena", "common_core"):
                self.profiles.profile(account_id, pid)
            timestamp = data.get("endedAt", self.ctx.iso())
            ended_at = int(datetime.fromisoformat(timestamp.replace("Z", "+00:00")).timestamp())
            if self._database is None:
                command = self._changes(data)
                self.profiles._store(account_id, command.txs, publish=False)
                receipt = self._receipt(session_id, data, command, ended_at)
                encoded = json.dumps(receipt, separators=(",", ":"))
                self._receipts[(session_id, account_id)] = (canonical, encoded, ended_at, copy.deepcopy(data))
            else:
                with closing(sqlite3.connect(self._database)) as db, db:
                    db.execute("BEGIN IMMEDIATE")
                    replay = self._replay(self._row(session_id, account_id, db), canonical)
                    if replay is not None:
                        return replay
                    # Other processes may have committed since this service read
                    # its cache. Read authoritative profiles while holding SQLite's
                    # write lock, before constructing any private ProfileTx copies.
                    for pid, encoded_profile in db.execute(
                            "SELECT profile_id,data FROM profiles WHERE account_id=?", (account_id,)):
                        self.profiles._profiles[(account_id, pid)] = json.loads(encoded_profile)
                    command = self._changes(data)
                    self.profiles._store(account_id, command.txs, database=db, publish=False)
                    receipt = self._receipt(session_id, data, command, ended_at)
                    db.execute("INSERT INTO match_results VALUES (?,?,?,?,?)",
                               (session_id, account_id, canonical,
                                json.dumps(receipt, separators=(",", ":")), ended_at))
            self.profiles._publish(account_id, command.txs)
            return copy.deepcopy(receipt)

    def stats(self, account_id, start_time=None, end_time=None):
        """StatsV2-shaped local counters; optional inclusive Unix-second bounds."""
        if self.accounts.get(account_id) is None:
            raise ApiError(404, Err.ACCOUNT_NOT_FOUND, "The stats account was not found.")
        start = 0 if start_time is None else _integer(start_time, "startTime", 0, 2**63 - 1)
        end = 2**63 - 1 if end_time is None else _integer(end_time, "endTime", 0, 2**63 - 1)
        if start > end:
            raise _invalid("startTime must not exceed endTime.")
        if self._database is None:
            rows = [(row[0], row[2]) for (sid, aid), row in self._receipts.items() if aid == account_id]
        else:
            with closing(sqlite3.connect(self._database)) as db:
                rows = db.execute("SELECT payload,ended_at FROM match_results WHERE account_id=? "
                                  "AND ended_at>=? AND ended_at<=?", (account_id, start, end)).fetchall()
        counters = {}
        for encoded, ended_at in rows:
            if not start <= ended_at <= end:
                continue
            result = json.loads(encoded)
            playlist = result["playlist"].lower()
            for name, amount in (("matchesplayed", 1), ("kills", result["kills"]),
                                 ("placetop1", int(result["placement"] == 1)),
                                 ("placetop10", int(result["placement"] <= 10)),
                                 ("placetop25", int(result["placement"] <= 25))):
                key = f"br_{name}_keyboardmouse_m0_playlist_{playlist}"
                counters[key] = counters.get(key, 0) + amount
        return {"accountId": account_id, "startTime": start, "endTime": end, "stats": counters}
