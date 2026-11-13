"""Durable account interactions in the native client's recovered wire format.

Explicit records describe actions that actually occurred. Co-play is additionally
inferred from committed progression results, using their recorded match end times.
"""
import re
import sqlite3
import threading
from datetime import datetime, timezone
from pathlib import Path

INTERACTION_TYPES = frozenset(('FriendshipCreate', 'FriendshipAccept', 'FriendshipDelete',
                               'PingSent', 'PartyInviteSent', 'PartyJoined', 'PartyLeft',
                               'GamePlayed'))


class InteractionsService:
    def __init__(self, ctx, accounts, oauth, friends, progression, *, state_dir=None):
        self.ctx, self.accounts, self.oauth = ctx, accounts, oauth
        self.friends, self.progression = friends, progression
        self._lock = threading.RLock()
        if state_dir is None:
            path = ':memory:'
        else:
            directory = Path(state_dir)
            directory.mkdir(parents=True, exist_ok=True)
            path = str(directory / 'interactions.sqlite3')
        self._db = sqlite3.connect(path, check_same_thread=False)
        with self._db:
            self._db.execute('CREATE TABLE IF NOT EXISTS interactions ('
                             'id INTEGER PRIMARY KEY, src TEXT NOT NULL, dst TEXT NOT NULL, '
                             'kind TEXT NOT NULL, happened_at INTEGER NOT NULL)')
            self._db.execute('CREATE INDEX IF NOT EXISTS interactions_src ON interactions(src)')
            self._db.execute('CREATE INDEX IF NOT EXISTS interactions_dst ON interactions(dst)')

    def close(self):
        with self._lock:
            self._db.close()

    def record(self, from_id, to_id, interaction_type, happened_at=None):
        """Record one real directed action, with an optional Unix-millisecond time."""
        for account in (from_id, to_id):
            if not isinstance(account, str) or not re.fullmatch(r'[0-9a-f]{32}', account):
                raise ValueError('Invalid interaction account ID.')
            if self.accounts.get(account) is None:
                raise ValueError('Unknown interaction account ID.')
        if from_id == to_id:
            raise ValueError('Interaction endpoints must differ.')
        if not isinstance(interaction_type, str) or interaction_type not in INTERACTION_TYPES:
            raise ValueError('Invalid interaction type.')
        if happened_at is None:
            happened_at = int(self.ctx.now() * 1000)
        if isinstance(happened_at, bool) or not isinstance(happened_at, int) or not -(2**63) <= happened_at < 2**63:
            raise ValueError('Interaction timestamp must be an int64 Unix millisecond value.')
        with self._lock, self._db:
            self._db.execute('INSERT INTO interactions (src,dst,kind,happened_at) VALUES (?,?,?,?)',
                             (from_id, to_id, interaction_type, happened_at))

    def query(self, req, match=None):
        account = self.oauth.account_session(req).account_id
        with self._lock:
            rows = self._db.execute('SELECT src,dst,kind,happened_at FROM interactions '
                                    'WHERE src=? OR dst=? ORDER BY happened_at,id',
                                    (account, account)).fetchall()
        events = [{'fromAccountId': src, 'toAccountId': dst, 'interactionType': kind,
                   'happenedAt': timestamp} for src, dst, kind, timestamp in rows]
        if self.progression is not None:
            for entry in self.progression.recent_players(account):
                # recent_players returns the actual committed result time in UTC.
                when = datetime.fromisoformat(entry['lastPlayed'].replace('Z', '+00:00'))
                epoch = datetime(1970, 1, 1, tzinfo=timezone.utc)
                delta = when - epoch
                milliseconds = (delta.days * 86400 + delta.seconds) * 1000 + delta.microseconds // 1000
                events.append({'fromAccountId': account, 'toAccountId': entry['accountId'],
                               'interactionType': 'GamePlayed', 'happenedAt': milliseconds})
        if self.friends is not None:
            events = [event for event in events if not self.friends.is_blocked(
                account, event['toAccountId'] if event['fromAccountId'] == account else event['fromAccountId'])]
        # The native parser updates a counterpart cache while iterating; older
        # events must precede newer ones so the latest interaction survives.
        events.sort(key=lambda event: event['happenedAt'])
        return 200, {'interactions': events}

    def register(self, router):
        router.add('GET', r'^/api/v1/Fortnite/get$', self.query, name='interactions.query')
