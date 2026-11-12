"""Persistent local friends graph. Route and response shapes are inferred, not captured.

A directed request becomes an undirected friendship when the recipient requests back.
Blocks are directed and remove both pending requests and any friendship atomically.
"""
import re
import sqlite3
import threading
from pathlib import Path

from api_common import ApiError, Err


class FriendsService:
    def __del__(self):
        database = getattr(self, "_db", None)
        if database is not None:
            database.close()

    def __init__(self, ctx, accounts, oauth, *, state_dir=None, notify=None):
        self.ctx, self.accounts, self.oauth, self.notify = ctx, accounts, oauth, notify
        self._lock = threading.RLock()
        self.notification_failures = 0
        if state_dir is None:
            path = ':memory:'
        else:
            directory = Path(state_dir)
            directory.mkdir(parents=True, exist_ok=True)
            path = str(directory / 'friends.sqlite3')
        self._db = sqlite3.connect(path, check_same_thread=False)
        with self._db:
            self._db.execute('CREATE TABLE IF NOT EXISTS requests (src TEXT NOT NULL, dst TEXT NOT NULL, created TEXT NOT NULL, PRIMARY KEY(src,dst))')
            self._db.execute('CREATE TABLE IF NOT EXISTS friends (a TEXT NOT NULL, b TEXT NOT NULL, created TEXT NOT NULL, PRIMARY KEY(a,b))')
            self._db.execute('CREATE TABLE IF NOT EXISTS blocks (src TEXT NOT NULL, dst TEXT NOT NULL, created TEXT NOT NULL, PRIMARY KEY(src,dst))')

    def close(self):
        with self._lock:
            self._db.close()

    def are_friends(self, account, other):
        a, b = sorted((account, other))
        with self._lock:
            return self._db.execute('SELECT 1 FROM friends WHERE a=? AND b=?', (a, b)).fetchone() is not None

    def is_blocked(self, account, other):
        """True when either account blocks the other."""
        with self._lock:
            return self._db.execute('SELECT 1 FROM blocks WHERE (src=? AND dst=?) OR (src=? AND dst=?)',
                                    (account, other, other, account)).fetchone() is not None

    def _identity(self, req, account, other=None):
        caller = self.oauth.account_session(req).account_id
        if caller != account:
            raise ApiError(403, Err.USER_OPERATION_FORBIDDEN, 'Cannot access another account’s friends.')
        for value in (account,) if other is None else (account, other):
            if not re.fullmatch(r'[0-9a-f]{32}', value):
                raise ApiError(400, Err.INVALID_PAYLOAD, 'Invalid account id.')
            if self.accounts.get(value) is None:
                raise ApiError(404, Err.ACCOUNT_NOT_FOUND, 'Account not found.')
        if other == account:
            raise ApiError(400, Err.INVALID_PAYLOAD, 'An account cannot target itself.')

    def _emit(self, recipient, event):
        if self.notify:
            try:
                self.notify(recipient, event)
            except Exception:
                with self._lock:
                    self.notification_failures += 1  # Best effort after commit; no event data is logged.

    def _snapshot(self, account):
        with self._lock:
            friends = self._db.execute('SELECT a,b,created FROM friends WHERE a=? OR b=? ORDER BY a,b', (account, account)).fetchall()
            incoming = self._db.execute('SELECT src,created FROM requests WHERE dst=? ORDER BY src', (account,)).fetchall()
            outgoing = self._db.execute('SELECT dst,created FROM requests WHERE src=? ORDER BY dst', (account,)).fetchall()
            blocks = self._db.execute('SELECT dst,created FROM blocks WHERE src=? ORDER BY dst', (account,)).fetchall()
        return ([(b if a == account else a, date) for a,b,date in friends], incoming, outgoing, blocks)

    @staticmethod
    def _entry(account, date):
        return {'accountId': account, 'created': date}

    def summary(self, req, match):
        account = match.group(1)
        self._identity(req, account)
        friends, incoming, outgoing, blocks = self._snapshot(account)
        return 200, {'friends': [self._entry(*x) for x in friends], 'incoming': [self._entry(*x) for x in incoming],
                     'outgoing': [self._entry(*x) for x in outgoing], 'blocklist': [self._entry(*x) for x in blocks],
                     'suggested': [], 'settings': {'acceptInvites': 'public'},
                     'limitsReached': {'incoming': False, 'outgoing': False, 'accepted': False}}

    def legacy_list(self, req, match):
        account = match.group(1)
        self._identity(req, account)
        friends, incoming, outgoing, _ = self._snapshot(account)
        return 200, [dict(self._entry(*x), status=status, direction=direction, favorite=False)
                     for rows, status, direction in ((friends, 'ACCEPTED', 'OUTBOUND'), (incoming, 'PENDING', 'INBOUND'), (outgoing, 'PENDING', 'OUTBOUND')) for x in rows]

    def blocklist(self, req, match):
        account = match.group(1)
        self._identity(req, account)
        return 200, [self._entry(*x) for x in self._snapshot(account)[3]]

    def settings(self, req, match):
        self._identity(req, match.group(1))
        return 200, {'acceptInvites': 'public'}

    def recent(self, req, match):
        self._identity(req, match.group(1))
        return 200, []

    def request_friend(self, req, match):
        account, other = match.group(1, 2)
        self._identity(req, account, other)
        a, b = sorted((account, other))
        event = None
        with self._lock, self._db:
            if self._db.execute('SELECT 1 FROM blocks WHERE (src=? AND dst=?) OR (src=? AND dst=?)', (account, other, other, account)).fetchone():
                raise ApiError(403, Err.USER_OPERATION_FORBIDDEN, 'Friend request blocked.')
            if self._db.execute('SELECT 1 FROM friends WHERE a=? AND b=?', (a,b)).fetchone():
                return 204, None
            if self._db.execute('SELECT 1 FROM requests WHERE src=? AND dst=?', (other,account)).fetchone():
                self._db.execute('DELETE FROM requests WHERE src=? AND dst=?', (other,account))
                self._db.execute('INSERT INTO friends VALUES (?,?,?)', (a,b,self.ctx.iso()))
                event = 'ACCEPTED'
            elif not self._db.execute('SELECT 1 FROM requests WHERE src=? AND dst=?', (account,other)).fetchone():
                self._db.execute('INSERT INTO requests VALUES (?,?,?)', (account,other,self.ctx.iso()))
                event = 'PENDING'
        if event:
            self._emit(other, {'type': 'com.epicgames.friends.core.apiobjects.Friend', 'accountId': account, 'status': event})
        return 204, None

    def delete_friend(self, req, match):
        account, other = match.group(1, 2)
        self._identity(req, account, other)
        a, b = sorted((account, other))
        with self._lock, self._db:
            changed = self._db.execute('DELETE FROM friends WHERE a=? AND b=?', (a,b)).rowcount
            changed += self._db.execute('DELETE FROM requests WHERE (src=? AND dst=?) OR (src=? AND dst=?)', (account,other,other,account)).rowcount
        if changed:
            self._emit(other, {'type': 'com.epicgames.friends.core.apiobjects.Friend', 'accountId': account, 'status': 'REMOVED'})
        return 204, None

    def block(self, req, match):
        account, other = match.group(1, 2)
        self._identity(req, account, other)
        a, b = sorted((account,other))
        with self._lock, self._db:
            self._db.execute('INSERT OR IGNORE INTO blocks VALUES (?,?,?)', (account,other,self.ctx.iso()))
            removed = self._db.execute('DELETE FROM friends WHERE a=? AND b=?', (a,b)).rowcount
            removed += self._db.execute('DELETE FROM requests WHERE (src=? AND dst=?) OR (src=? AND dst=?)', (account,other,other,account)).rowcount
        if removed:
            self._emit(other, {'type': 'com.epicgames.friends.core.apiobjects.Friend', 'accountId': account, 'status': 'REMOVED'})
        return 204, None

    def unblock(self, req, match):
        account, other = match.group(1, 2)
        self._identity(req, account, other)
        with self._lock, self._db:
            self._db.execute('DELETE FROM blocks WHERE src=? AND dst=?', (account,other))
        return 204, None

    def register(self, router):
        v1 = r'/friends/api/v1/([^/]+)'
        public = r'/friends/api/public'
        for path, handler, name in ((v1 + '/summary', self.summary, 'summary'), (v1 + '/settings', self.settings, 'settings'),
                                    (v1 + '/recent/[^/]+', self.recent, 'recent'), (v1 + '/blocklist', self.blocklist, 'blocklist'),
                                    (public + '/friends/([^/]+)', self.legacy_list, 'legacy'), (public + '/blocklist/([^/]+)', self.blocklist, 'legacy_blocklist')):
            router.add('GET', path, handler, name='friends.' + name, status='inferred')
        for method, path, handler, name in (('POST', public + '/friends/([^/]+)/([^/]+)', self.request_friend, 'request'),
                                            ('DELETE', public + '/friends/([^/]+)/([^/]+)', self.delete_friend, 'delete'),
                                            ('POST', public + '/blocklist/([^/]+)/([^/]+)', self.block, 'block'),
                                            ('DELETE', public + '/blocklist/([^/]+)/([^/]+)', self.unblock, 'unblock'),
                                            ('POST', v1 + '/blocklist/([^/]+)', self.block, 'block_v1'),
                                            ('DELETE', v1 + '/blocklist/([^/]+)', self.unblock, 'unblock_v1')):
            router.add(method, path, handler, name='friends.' + name, status='inferred')
