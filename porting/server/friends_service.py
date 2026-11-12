"""Persistent local friends graph with partially observed 12.60 wire contracts.

A directed request becomes an undirected friendship when the recipient requests back.
Blocks are directed and remove both pending requests and any friendship atomically.
The settings PUT and response fields follow public client implementations; 12.60
wire casing and the permission enforcement rules have not been captured. The
default retains this stand-in's original lowercase value.
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
        self.progression = None
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
            self._db.execute('CREATE TABLE IF NOT EXISTS settings (account TEXT PRIMARY KEY, accept_invites TEXT NOT NULL, mutual_privacy TEXT)')

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

    def _emit_pair(self, account, other, fields):
        for recipient, peer in ((account, other), (other, account)):
            self._emit(recipient, dict(fields, accountId=peer))

    def _remove_edge(self, account, other):
        """Remove the relationship in the caller's transaction and return its reason."""
        a, b = sorted((account, other))
        friendship = self._db.execute('DELETE FROM friends WHERE a=? AND b=?', (a, b)).rowcount
        outgoing = self._db.execute('DELETE FROM requests WHERE src=? AND dst=?', (account, other)).rowcount
        incoming = self._db.execute('DELETE FROM requests WHERE src=? AND dst=?', (other, account)).rowcount
        # The client recognizes these reasons; mapping them to actor actions is inferred.
        return 'DELETED' if friendship else 'ABORTED' if outgoing else 'REJECTED' if incoming else None

    def _emit_removal(self, account, other, reason):
        if reason:
            self._emit_pair(account, other, {'type': 'com.epicgames.friends.core.apiobjects.FriendRemoval',
                                             'reason': reason})

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

    def _settings(self, account):
        with self._lock:
            row = self._db.execute('SELECT accept_invites, mutual_privacy FROM settings WHERE account=?', (account,)).fetchone()
        result = {'acceptInvites': row[0] if row else 'public'}
        if row and row[1] is not None:
            result['mutualPrivacy'] = row[1]
        return result

    def summary(self, req, match):
        account = match.group(1)
        self._identity(req, account)
        friends, incoming, outgoing, blocks = self._snapshot(account)
        return 200, {'friends': [self._entry(*x) for x in friends], 'incoming': [self._entry(*x) for x in incoming],
                     'outgoing': [self._entry(*x) for x in outgoing], 'blocklist': [self._entry(*x) for x in blocks],
                     'suggested': [], 'settings': self._settings(account),
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
        return 200, self._settings(match.group(1))

    def set_settings(self, req, match):
        account = match.group(1)
        self._identity(req, account)
        body = req.json_object()
        allowed = {'acceptInvites': {'PUBLIC', 'PRIVATE', 'FRIENDS_OF_FRIENDS'},
                   'mutualPrivacy': {'ALL', 'FRIENDS', 'NONE'}}
        if not body or any(key not in allowed or not isinstance(value, str) or value.upper() not in allowed[key]
                           for key, value in body.items()):
            raise ApiError(400, Err.INVALID_PAYLOAD, 'Invalid friends settings.')
        with self._lock, self._db:
            current = self._settings(account)
            current.update(body)
            self._db.execute('INSERT OR REPLACE INTO settings VALUES (?,?,?)',
                             (account, current['acceptInvites'], current.get('mutualPrivacy')))
        return 200, current

    def recent(self, req, match):
        account, namespace = match.group(1, 2)
        self._identity(req, account)
        # Local policy: only Fortnite results are available, and either user's
        # block hides the peer. No cross-database writes are needed.
        if namespace.lower() != 'fortnite' or self.progression is None:
            return 200, []
        return 200, [entry for entry in self.progression.recent_players(account)
                     if not self.is_blocked(account, entry['accountId'])]

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
                created = self.ctx.iso()
                self._db.execute('INSERT INTO friends VALUES (?,?,?)', (a,b,created))
                event = ('ACCEPTED', created)
            elif not self._db.execute('SELECT 1 FROM requests WHERE src=? AND dst=?', (account,other)).fetchone():
                # Local interpretation of the privacy enum; accepting a pending request remains possible.
                permission = self._settings(other)['acceptInvites'].upper()
                if permission == 'PRIVATE':
                    raise ApiError(403, Err.USER_OPERATION_FORBIDDEN, 'Friend requests are disabled.')
                if permission == 'FRIENDS_OF_FRIENDS':
                    def neighbors(user):
                        return {y if x == user else x for x, y in self._db.execute(
                            'SELECT a,b FROM friends WHERE a=? OR b=?', (user, user))}
                    if not neighbors(account).intersection(neighbors(other)):
                        raise ApiError(403, Err.USER_OPERATION_FORBIDDEN, 'Friend requests require a mutual friend.')
                created = self.ctx.iso()
                self._db.execute('INSERT INTO requests VALUES (?,?,?)', (account,other,created))
                event = ('PENDING', created)
        if event:
            status, created = event
            # Accepted friendships use OUTBOUND as in legacy_list; the client ignores it.
            for recipient, peer, direction in ((account, other, 'OUTBOUND'),
                                                (other, account, 'INBOUND' if status == 'PENDING' else 'OUTBOUND')):
                self._emit(recipient, {'type': 'com.epicgames.friends.core.apiobjects.Friend',
                                       'accountId': peer, 'status': status, 'direction': direction,
                                       'created': created})
        return 204, None

    def delete_friend(self, req, match):
        account, other = match.group(1, 2)
        self._identity(req, account, other)
        with self._lock, self._db:
            reason = self._remove_edge(account, other)
        self._emit_removal(account, other, reason)
        return 204, None

    def block(self, req, match):
        account, other = match.group(1, 2)
        self._identity(req, account, other)
        with self._lock, self._db:
            self._db.execute('INSERT OR IGNORE INTO blocks VALUES (?,?,?)', (account,other,self.ctx.iso()))
            reason = self._remove_edge(account, other)
        self._emit_removal(account, other, reason)
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
        router.add('POST', v1 + '/friends/([^/]+)', self.request_friend,
                   name='friends.request_v1', status='observed')
        for path, handler, name in ((v1 + '/summary', self.summary, 'summary'), (v1 + '/settings', self.settings, 'settings'),
                                    (v1 + '/recent/([^/]+)', self.recent, 'recent'), (v1 + '/blocklist', self.blocklist, 'blocklist'),
                                    (public + '/friends/([^/]+)', self.legacy_list, 'legacy'), (public + '/blocklist/([^/]+)', self.blocklist, 'legacy_blocklist')):
            router.add('GET', path, handler, name='friends.' + name,
                       status='observed' if name == 'recent' else 'inferred')
        for method, path, handler, name in (('POST', public + '/friends/([^/]+)/([^/]+)', self.request_friend, 'request'),
                                            ('PUT', v1 + '/settings', self.set_settings, 'set_settings'),
                                            ('DELETE', public + '/friends/([^/]+)/([^/]+)', self.delete_friend, 'delete'),
                                            ('DELETE', v1 + '/friends/([^/]+)', self.delete_friend, 'delete_v1'),
                                            ('POST', public + '/blocklist/([^/]+)/([^/]+)', self.block, 'block'),
                                            ('DELETE', public + '/blocklist/([^/]+)/([^/]+)', self.unblock, 'unblock'),
                                            ('POST', v1 + '/blocklist/([^/]+)', self.block, 'block_v1'),
                                            ('DELETE', v1 + '/blocklist/([^/]+)', self.unblock, 'unblock_v1')):
            router.add(method, path, handler, name='friends.' + name,
                       status='observed' if name == 'delete_v1' else 'inferred')
