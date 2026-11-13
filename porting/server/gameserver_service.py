"""[guess] Local dedicated-server registration and admission protocol."""
import base64
import hmac
import os
import secrets
import sqlite3
import threading
from contextlib import closing

from api_common import ApiError, Err

# [guess] A server must refresh state within this interval to remain live.
HEARTBEAT_SECONDS = 30
JOINABLE_PHASES = frozenset(('waiting', 'warmup', 'lobby', 'pending'))


def _invalid(message):
    raise ApiError(400, Err.INVALID_PAYLOAD, message)


def _integer(value, low, high):
    return type(value) is int and low <= value <= high


class GameServerService:
    def __init__(self, ctx, accounts, profiles, party, *, shared_secret=None):
        self.ctx, self.accounts, self.profiles, self.party = ctx, accounts, profiles, party
        secret = os.environ.get('FORTNITE_GAMESERVER_SECRET', '') if shared_secret is None else shared_secret
        self._secret = secret.encode('utf-8') if secret else None
        self._lock = threading.RLock()
        self._servers = {}
        self._database = getattr(profiles, '_database', None)
        self._participants = set()
        if self._database is not None:
            with closing(sqlite3.connect(self._database)) as db, db:
                db.execute("CREATE TABLE IF NOT EXISTS match_participants ("
                           "session_id TEXT NOT NULL, account_id TEXT NOT NULL, "
                           "playlist TEXT NOT NULL, PRIMARY KEY (session_id, account_id, playlist))")

    def authorize(self, req):
        if self._secret is None:
            raise ApiError(503, Err.INVALID_PAYLOAD, 'Game-server API is not configured.')
        token = req.bearer_token()
        if not token or not hmac.compare_digest(self._secret, token.encode('utf-8')):
            raise ApiError(401, Err.INVALID_PAYLOAD, 'Invalid game-server authorization.')

    def register_server(self, req):
        self.authorize(req)
        body = req.json_object()
        address, port = body.get('address'), body.get('port')
        beacon = body.get('beaconPort', port + 1 if _integer(port, 1, 65534) else port)
        playlists, maximum, build = body.get('playlists'), body.get('maxPlayers'), body.get('buildUniqueId')
        if (not isinstance(address, str) or not address.strip() or len(address) > 255 or
                any(c.isspace() or ord(c) < 32 for c in address) or
                not _integer(port, 1, 65535) or not _integer(beacon, 1, 65535) or
                not isinstance(playlists, list) or not playlists or len(playlists) > 100 or
                any(not isinstance(p, str) or not p.strip() or len(p) > 255 for p in playlists) or
                not _integer(maximum, 1, 1000) or
                not isinstance(build, str) or not build.strip() or len(build) > 128):
            _invalid('Invalid game-server registration.')
        sid = secrets.token_hex(16)
        key = secrets.token_bytes(32)
        with self._lock:
            self._servers[sid] = dict(id=sid, address=address, port=port, beaconPort=beacon,
                                      playlists=list(dict.fromkeys(playlists)), maxPlayers=maximum,
                                      buildUniqueId=build, phase='pending', numPlayers=0,
                                      joinable=False, heartbeat=self.ctx.now(), players=[],
                                      connectedPlayerIds=None, key=key)
        return 200, {'sessionId': sid, 'encryptionKey': base64.b64encode(key).decode('ascii')}

    def update_state(self, req, session_id):
        self.authorize(req)
        body = req.json_object()
        phase, count, joinable = body.get('phase'), body.get('numPlayers'), body.get('joinable')
        if (not isinstance(phase, str) or not phase.strip() or len(phase) > 64 or
                not _integer(count, 0, 1000) or type(joinable) is not bool):
            _invalid('Invalid game-server state.')
        connected = body.get('connectedPlayerIds')
        if ('connectedPlayerIds' in body and
                (not isinstance(connected, list) or len(connected) != count or
                 any(not isinstance(p, str) or not p.strip() or len(p) > 255 for p in connected) or
                 len(set(connected)) != len(connected))):
            _invalid('Invalid connected player IDs.')
        with self._lock:
            server = self._servers.get(session_id)
            if server is None:
                raise ApiError(404, Err.NOT_FOUND, 'Unknown game-server session.')
            if count > server['maxPlayers']:
                _invalid('Player count exceeds capacity.')
            if connected is not None:
                previous = set(server['connectedPlayerIds'] or [])
                departed = previous - set(connected)
                server['players'] = [p for p in server['players'] if p not in departed]
                server['connectedPlayerIds'] = list(connected)
            else:
                server['connectedPlayerIds'] = None
            server.update(phase=phase, numPlayers=count, joinable=joinable, heartbeat=self.ctx.now())
        return 204, None

    def _live(self, server):
        return self.ctx.now() - server['heartbeat'] < HEARTBEAT_SECONDS

    def get_session(self, session_id, include_expired=False):
        """Return live session metadata, including a copied roster; never expose the key."""
        with self._lock:
            server = self._servers.get(session_id)
            if server is None or (not include_expired and not self._live(server)):
                return None
            return {k: list(v) if isinstance(v, list) else v for k, v in server.items() if k != 'key'}

    @staticmethod
    def _occupied(server):
        connected = server['connectedPlayerIds']
        if connected is None:
            # Without identities the overlap with reservations is unknown.
            return server['numPlayers'] + len(server['players'])
        return len(set(connected) | set(server['players']))

    def reserve(self, playlist, players, build_unique_id=None):
        """Atomically admit a roster, conservatively counting unidentified players."""
        if not playlist or not players:
            return None
        with self._lock:
            for server in self._servers.values():
                if (not self._live(server) or not server['joinable'] or
                        server['phase'].lower() not in JOINABLE_PHASES or
                        (build_unique_id and build_unique_id != server['buildUniqueId']) or
                        playlist.casefold() not in [p.casefold() for p in server['playlists']]):
                    continue
                added = [p for p in dict.fromkeys(players) if p not in server['players']]
                connected = set(server['connectedPlayerIds'] or [])
                newly_occupied = [p for p in added if p not in connected]
                if self._occupied(server) + len(newly_occupied) > server['maxPlayers']:
                    continue
                # Reservation is the existing trusted admission decision, not proof
                # of a network connection. Persist before publishing the roster.
                records = [(server['id'], p, playlist.casefold()) for p in dict.fromkeys(players)]
                if self._database is None:
                    self._participants.update(records)
                else:
                    with closing(sqlite3.connect(self._database)) as db, db:
                        db.executemany("INSERT OR IGNORE INTO match_participants VALUES (?,?,?)", records)
                server['players'].extend(added)
                return server['id']
        return None

    def result_participant(self, session_id, account_id, playlist):
        """Check immutable reservation history; never grants live admission."""
        record = (session_id, account_id, playlist.casefold())
        if self._database is None:
            with self._lock:
                return record in self._participants
        with closing(sqlite3.connect(self._database)) as db:
            return db.execute("SELECT 1 FROM match_participants WHERE session_id=? "
                              "AND account_id=? AND playlist=?", record).fetchone() is not None

    def session(self, session_id):
        s = self.get_session(session_id)
        if s is None:
            return None
        # MCP requires a valid owning identifier even for a dedicated session.
        # Use the registered server instance's public GUID, independent of its roster.
        return 200, {'id': s['id'], 'ownerId': s['id'], 'ownerName': 'local-dedicated', 'serverName': 'local-dedicated',
                     'serverAddress': s['address'], 'serverPort': s['port'],
                     'maxPublicPlayers': s['maxPlayers'],
                     'openPublicPlayers': max(0, s['maxPlayers'] - self._occupied(s)),
                     'maxPrivatePlayers': 0, 'openPrivatePlayers': 0, 'shouldAdvertise': True,
                     'allowJoinInProgress': s['joinable'], 'isDedicated': True, 'usesStats': False,
                     'allowInvites': False, 'usesPresence': False, 'allowJoinViaPresence': False,
                     'allowJoinViaPresenceFriendsOnly': False, 'publicPlayers': s['players'],
                     'privatePlayers': [], 'attributes': {'BeaconPort_i': s['beaconPort']},
                     'buildUniqueId': s['buildUniqueId'], 'started': s['phase'].lower() not in JOINABLE_PHASES}

    def join(self, session_id, account_id):
        s = self.get_session(session_id)
        return (204, None) if s and account_id in s['players'] else None

    def session_key(self, account_id, session_id):
        with self._lock:
            s = self._servers.get(session_id)
            if s is None or not self._live(s) or account_id not in s['players']:
                return None
            key = base64.b64encode(s['key']).decode('ascii')
        return 200, {'accountId': account_id, 'sessionId': session_id, 'key': key}

    def check_admission(self, req, session_id):
        """Authenticated server lookup; membership is not authentication of a client."""
        self.authorize(req)
        account = req.json_object().get('accountId')
        if not isinstance(account, str) or not account.strip() or len(account) > 255:
            _invalid('Invalid account ID.')
        with self._lock:
            server = self._servers.get(session_id)
            if server is None or not self._live(server) or account not in server['players']:
                raise ApiError(404, Err.NOT_FOUND, 'Admission not found.')
        return 204, None

    def register(self, router):
        router.add('POST', r'/local/gameserver/([^/]+)/admission',
                   lambda req, match: self.check_admission(req, match.group(1)),
                   name='gameserver.admission', status='guess')
        router.add('POST', r'/local/gameserver/register', lambda req, match: self.register_server(req),
                   name='gameserver.register', status='guess')
        router.add('POST', r'/local/gameserver/([^/]+)/state',
                   lambda req, match: self.update_state(req, match.group(1)),
                   name='gameserver.state', status='guess')
