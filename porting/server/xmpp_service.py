"""Local RFC 7395 XMPP transport. Epic notification envelopes are inferred.

Original-client probes reached authentication, binding, session setup and presence.
Notification envelopes and chat flows still require client validation.
"""
import base64
import binascii
import json
import queue
import re
import secrets
import threading
import time
import xml.etree.ElementTree as ET
from xml.sax.saxutils import escape, quoteattr

from local_ws import OP_TEXT, WebSocketClosed

FRAME = 'urn:ietf:params:xml:ns:xmpp-framing'
SASL = 'urn:ietf:params:xml:ns:xmpp-sasl'
BIND = 'urn:ietf:params:xml:ns:xmpp-bind'
SESSION = 'urn:ietf:params:xml:ns:xmpp-session'
CLIENT = 'jabber:client'
ROSTER = 'jabber:iq:roster'
PING = 'urn:xmpp:ping'
DISCO = 'http://jabber.org/protocol/disco#info'
MUC = 'http://jabber.org/protocol/muc'
MUC_USER = MUC + '#user'
MUC_OWNER = MUC + '#owner'
MAX_XML = 65536
MAX_PENDING = 128
PREAUTH_TIMEOUT = 15


def tag(ns, name):
    return '{%s}%s' % (ns, name)


class XmppService:
    def __init__(self, ctx, accounts, oauth, friends, party):
        self.ctx, self.accounts, self.oauth = ctx, accounts, oauth
        self.friends, self.party = friends, party
        self._lock = threading.RLock()
        self._connections = set()
        self._closed = False

    def websocket_route(self, path, query):
        if path in ('/', '/xmpp'):
            return self.serve
        return None

    def _allowed(self, a, b):
        if a == b:
            return True
        if self.friends.is_blocked(a, b):
            return False
        if self.friends.are_friends(a, b):
            return True
        party = self.party._party_of(a)
        return party is not None and self.party._member(party, b) is not None

    def _live(self, c):
        return bool(c.token and self.oauth.lookup(c.token) is c.session)

    def _enqueue(self, c, text):
        if c.stop.is_set():
            return None
        sent = threading.Event()
        try:
            c.outbound.put_nowait((text, sent))
            return sent
        except queue.Full:
            self._disconnect(c)
            return None

    def _send(self, c, text):
        if not self._live(c):
            self._disconnect(c)
            return
        self._enqueue(c, text)

    def _iq_response(self, c, iqid, kind, payload=''):
        # Server replies use authenticated stream state, never request addresses.
        recipient = ' to=%s' % quoteattr(c.jid) if c.jid else ''
        c.last_sent = self._enqueue(c, '<iq xmlns="jabber:client" type=%s id=%s from=%s%s>%s</iq>' %
                                    (quoteattr(kind), iqid, quoteattr(c.domain), recipient, payload))

    def _disconnect(self, c):
        # shutdown interrupts both the reader and a writer blocked on a slow peer;
        # never take the WebSocket write lock from an HTTP request thread.
        c.stop.set()
        try:
            c.ws.sock.shutdown(2)
        except (OSError, AttributeError):
            pass

    def close(self):
        """Stop all streams and wait until no connection can use friends state."""
        with self._lock:
            self._closed = True
            connections = list(self._connections)
        for c in connections:
            self._disconnect(c)
        for c in connections:
            c.done.wait()

    def notify(self, account_id, event):
        """Deliver a committed backend event to all authenticated resources."""
        if str(event.get("type", "")).startswith("com.epicgames.social.party."):
            self._prune_rooms()
        actor = event.get("account_id")
        if (str(event.get("type", "")).startswith("com.epicgames.social.party.")
                and actor and actor != account_id and self.friends.is_blocked(actor, account_id)):
            return
        with self._lock:
            targets = [c for c in self._connections if c.account == account_id and c.jid]
        body = escape(json.dumps(event, separators=(',', ':'), ensure_ascii=False))
        for c in targets:
            self._send(c, '<message xmlns="jabber:client" from=%s to=%s><body>%s</body></message>' %
                       (quoteattr('xmpp-admin@' + c.domain), quoteattr(c.jid), body))

    def _presence(self, c, unavailable=False):
        if not c.jid:
            return
        with self._lock:
            targets = list(self._connections)
        for other in targets:
            if not other.jid or other is c or not self._allowed(c.account, other.account):
                continue
            kind = ' type="unavailable"' if unavailable else ''
            status = '' if unavailable or c.status is None else '<status>%s</status>' % escape(c.status)
            self._send(other, '<presence xmlns="jabber:client" from=%s to=%s%s>%s</presence>' %
                       (quoteattr(c.jid), quoteattr(other.jid), kind, status))

    def serve(self, ws):
        c = _Connection(ws)
        with self._lock:
            if self._closed:
                self._disconnect(c)
                return
            self._connections.add(c)

        def writer():
            while not c.stop.is_set():
                try:
                    text, sent = c.outbound.get(timeout=0.2)
                except queue.Empty:
                    continue
                try:
                    ws.send_text(text)
                except (OSError, ValueError, WebSocketClosed):
                    self._disconnect(c)
                    return
                finally:
                    sent.set()

        def watch_token():
            deadline = time.monotonic() + PREAUTH_TIMEOUT
            while not c.stop.wait(0.25):
                if (not c.session and time.monotonic() >= deadline) or (c.session and not self._live(c)):
                    self._disconnect(c)
                    return

        writer_thread = threading.Thread(target=writer, daemon=True)
        writer_thread.start()
        threading.Thread(target=watch_token, daemon=True).start()
        try:
            while True:
                opcode, data = ws.recv_message()
                if opcode != OP_TEXT or len(data) > MAX_XML or b'<!' in data:
                    break
                if not data.strip():
                    continue  # RFC 7395 whitespace keepalive
                try:
                    root = ET.fromstring(data.decode('utf-8'))
                except (UnicodeError, ET.ParseError):
                    break
                if c.session and not self._live(c):
                    break
                if not self._handle(c, root):
                    if c.last_sent is not None:
                        c.last_sent.wait(0.5)
                    break
        except (WebSocketClosed, OSError, ValueError):
            pass
        finally:
            self._disconnect(c)
            writer_thread.join()
            # Keep this connection registered through presence cleanup: close()
            # must not return while this thread may still consult friends.
            if c.available:
                self._presence(c, True)
            self._leave_rooms(c)
            with self._lock:
                self._connections.discard(c)
                c.done.set()

    def _handle(self, c, root):
        if root.tag == tag(FRAME, 'close'):
            c.last_sent = self._enqueue(c, '<close xmlns="%s"/>' % FRAME)
            return False
        if root.tag == tag(FRAME, 'open'):
            if c.phase not in ('open', 'restart'):
                return False
            domain = root.get('to', 'xmpp-service-prod.ol.epicgames.com')
            if not re.fullmatch(r'[A-Za-z0-9.-]{1,253}', domain):
                return False
            c.domain = domain
            c.last_sent = self._enqueue(c, '<open xmlns="%s" from=%s id=%s version="1.0"/>' %
                           (FRAME, quoteattr(domain), quoteattr(secrets.token_hex(8))))
            if c.phase == 'open':
                c.phase = 'auth'
                c.last_sent = self._enqueue(c, '<stream:features xmlns:stream="http://etherx.jabber.org/streams"><mechanisms xmlns="%s"><mechanism>PLAIN</mechanism></mechanisms></stream:features>' % SASL)
            else:
                c.phase = 'bind'
                c.last_sent = self._enqueue(c, '<stream:features xmlns:stream="http://etherx.jabber.org/streams"><bind xmlns="%s"/><session xmlns="%s"/></stream:features>' % (BIND, SESSION))
            return True
        if c.phase == 'auth' and root.tag == tag(SASL, 'auth'):
            try:
                if root.get('mechanism') != 'PLAIN':
                    raise ValueError()
                raw = base64.b64decode(root.text or '', validate=True).decode('utf-8')
                authz, account, token = raw.split('\0')
                session = self.oauth.lookup(token)
                if not session or not session.account_id or session.account_id != account or authz not in ('', account):
                    raise ValueError()
            except (ValueError, UnicodeError, binascii.Error):
                c.last_sent = self._enqueue(c, '<failure xmlns="%s"><not-authorized/></failure>' % SASL)
                return False
            c.session, c.account, c.token = session, account, token
            c.phase = 'restart'
            c.last_sent = self._enqueue(c, '<success xmlns="%s"/>' % SASL)
            return True
        # The original client also sends standalone, unqualified stanza fragments.
        if c.phase in ('bind', 'ready') and root.tag in (tag(CLIENT, 'iq'), 'iq'):
            iqid = quoteattr(root.get('id', ''))
            if c.phase == 'ready' and root.get('to') and root.get('to') != c.domain and self._room_iq(c, root, iqid):
                return True
            bind = root.find(tag(BIND, 'bind'))
            if c.phase == 'bind' and root.get('type') == 'set' and bind is not None:
                resource = bind.findtext(tag(BIND, 'resource')) or secrets.token_hex(8)
                if not re.fullmatch(r'[^\x00-\x20/@]{1,128}', resource):
                    return False
                jid = c.account + '@' + c.domain + '/' + resource
                with self._lock:
                    duplicates = [other for other in self._connections if other is not c and other.jid == jid]
                    c.jid = jid
                    c.phase = 'ready'
                    self._iq_response(c, iqid, 'result', '<bind xmlns="%s"><jid>%s</jid></bind>' % (BIND, escape(c.jid)))
                for other in duplicates:
                    self._disconnect(other)
                return True
            if c.phase == 'ready' and root.get('type') == 'set' and root.find(tag(SESSION, 'session')) is not None:
                self._iq_response(c, iqid, 'result')
                return True
            if c.phase == 'ready' and root.get('type') == 'get' and len(root) == 1:
                child = root[0]
                if child.tag == tag(PING, 'ping'):
                    self._iq_response(c, iqid, 'result')
                    return True
                if child.tag == tag(ROSTER, 'query'):
                    friends, _, _, _ = self.friends._snapshot(c.account)
                    items = ''.join('<item jid=%s subscription="both"/>' % quoteattr(account + '@' + c.domain)
                                    for account, _, _ in friends if not self.friends.is_blocked(c.account, account))
                    self._iq_response(c, iqid, 'result', '<query xmlns="%s">%s</query>' % (ROSTER, items))
                    return True
                if child.tag == tag(DISCO, 'query') and not child.get('node'):
                    self._iq_response(c, iqid, 'result', '<query xmlns="%s"><identity category="server" type="im"/><feature var="%s"/><feature var="%s"/></query>' % (DISCO, DISCO, PING))
                    return True
            if root.get('type') in ('result', 'error'):
                return True
            self._iq_response(c, iqid, 'error', '<error type="cancel"><service-unavailable xmlns="urn:ietf:params:xml:ns:xmpp-stanzas"/></error>')
            return True
        if c.phase == 'ready' and root.tag in (tag(CLIENT, 'presence'), 'presence'):
            kind = root.get('type', 'available')
            if root.get('to'):
                self._room_presence(c, root)
                return True
            if kind not in ('available', 'unavailable'):
                return True
            c.available = kind == 'available'
            c.status = root.findtext(tag(CLIENT, 'status')) or root.findtext('status')
            self._presence(c, not c.available)
            if c.available:
                with self._lock:
                    others = list(self._connections)
                for other in others:
                    if other is not c and other.available and self._allowed(c.account, other.account):
                        self._send(c, '<presence xmlns="jabber:client" from=%s to=%s>%s</presence>' % (quoteattr(other.jid), quoteattr(c.jid), '<status>%s</status>' % escape(other.status) if other.status else ''))
            return True
        if c.phase == 'ready' and root.tag in (tag(CLIENT, 'message'), 'message'):
            if root.get('type') == 'groupchat':
                self._room_message(c, root)
                return True
            if root.get('type') == 'chat':
                self._direct_message(c, root)
                return True
        return False

    # Local convention until the original room naming is captured: the bare JID
    # is <party-id>@muc.<stream-domain>; only existing party IDs qualify.
    def _room(self, c, jid):
        bare, sep, nick = (jid or '').partition('/')
        local, at, domain = bare.partition('@')
        if not at or domain != 'muc.' + c.domain or not re.fullmatch(r'[A-Za-z0-9_-]{1,128}', local):
            return None
        party = self.party._parties.get(local)
        if not party or not self.party._member(party, c.account):
            return None
        return bare, nick if sep else '', party

    def _occupant_presence(self, target, occupant, room, nick, unavailable=False, self_presence=False):
        kind = ' type="unavailable"' if unavailable else ''
        role = 'none' if unavailable else 'participant'
        status = '<status code="110"/>' if self_presence else ''
        self._send(target, '<presence xmlns="jabber:client" from=%s to=%s%s><x xmlns="%s"><item affiliation="member" role="%s" jid=%s/>%s</x></presence>' %
                   (quoteattr(room + '/' + nick), quoteattr(target.jid), kind, MUC_USER, role, quoteattr(occupant.jid), status))

    def _leave_rooms(self, c):
        with self._lock:
            for room, nick in list(c.rooms.items()):
                del c.rooms[room]
                for other in list(self._connections):
                    if room in other.rooms and self._allowed(c.account, other.account):
                        self._occupant_presence(other, c, room, nick, True)
                if not c.stop.is_set():
                    self._occupant_presence(c, c, room, nick, True, True)

    def _prune_rooms(self):
        with self._lock:
            for c in list(self._connections):
                for room in list(c.rooms):
                    if not self._room(c, room):
                        nick = c.rooms.pop(room)
                        for other in list(self._connections):
                            if room in other.rooms and self._room(other, room) and not self.friends.is_blocked(c.account, other.account):
                                self._occupant_presence(other, c, room, nick, True)
                        self._occupant_presence(c, c, room, nick, True, True)

    def _room_presence(self, c, root):
        jid = root.get('to', '')
        kind = root.get('type', 'available')
        with self._lock:
            self._prune_rooms()
            resolved = self._room(c, jid)
            if not resolved:
                self._send(c, '<presence xmlns="jabber:client" from=%s to=%s type="error"><error type="auth"><forbidden xmlns="urn:ietf:params:xml:ns:xmpp-stanzas"/></error></presence>' % (quoteattr(jid), quoteattr(c.jid)))
                return
            room, nick, party = resolved
            if kind == 'unavailable':
                if room in c.rooms and c.rooms[room] == nick:
                    old = c.rooms.pop(room)
                    for other in list(self._connections):
                        if room in other.rooms and self._allowed(c.account, other.account):
                            self._occupant_presence(other, c, room, old, True)
                    self._occupant_presence(c, c, room, old, True, True)
                return
            if kind != 'available' or not re.fullmatch(r'[^\x00-\x20/@]{1,128}', nick):
                return
            occupants = [o for o in self._connections if room in o.rooms and o is not c]
            if any(o.rooms[room] == nick for o in occupants):
                self._send(c, '<presence xmlns="jabber:client" from=%s to=%s type="error"><error type="cancel"><conflict xmlns="urn:ietf:params:xml:ns:xmpp-stanzas"/></error></presence>' % (quoteattr(jid), quoteattr(c.jid)))
                return
            if room in c.rooms:
                return
            c.rooms[room] = nick
            for other in occupants:
                if not self.friends.is_blocked(c.account, other.account):
                    self._occupant_presence(c, other, room, other.rooms[room])
                    self._occupant_presence(other, c, room, nick)
            self._occupant_presence(c, c, room, nick, self_presence=True)

    def _direct_message(self, c, root):
        """Relay online local chat; silently drop unsupported or unauthorized targets.

        This local convention does not queue offline messages or disclose whether
        a rejected recipient exists. The authenticated JID supplies the sender.
        """
        target = root.get('to', '')
        bare, sep, resource = target.partition('/')
        account, at, domain = bare.partition('@')
        body = root.find('body' if root.tag == 'message' else tag(CLIENT, 'body'))
        if (not at or not account or domain != c.domain or (sep and not resource)
                or body is None or body.text is None or not self._allowed(c.account, account)):
            return
        with self._lock:
            targets = [other for other in self._connections
                       if other.jid and other.account == account and other.domain == c.domain
                       and (not sep or other.jid == target)]
        for other in targets:
            self._send(other, '<message xmlns="jabber:client" from=%s to=%s type="chat"><body>%s</body></message>' %
                       (quoteattr(c.jid), quoteattr(other.jid), escape(body.text)))

    def _room_message(self, c, root):
        with self._lock:
            self._prune_rooms()
            resolved = self._room(c, root.get('to'))
            body = root.find('body' if root.tag == 'message' else tag(CLIENT, 'body'))
            if not resolved or resolved[1] or resolved[0] not in c.rooms or body is None or body.text is None:
                return
            room = resolved[0]
            for other in list(self._connections):
                if room in other.rooms and self._room(other, room) and not self.friends.is_blocked(c.account, other.account):
                    self._send(other, '<message xmlns="jabber:client" from=%s to=%s type="groupchat"><body>%s</body></message>' %
                               (quoteattr(room + '/' + c.rooms[room]), quoteattr(other.jid), escape(body.text)))

    def _room_iq(self, c, root, iqid):
        resolved = self._room(c, root.get('to'))
        query = root[0] if len(root) == 1 else None
        if query is None or query.tag not in (tag(DISCO, 'query'), tag(MUC_OWNER, 'query')):
            return False
        if not resolved or resolved[1] or (query.tag == tag(MUC_OWNER, 'query') and
                (root.get('type') != 'set' or resolved[0] not in c.rooms or self.party._captain(resolved[2]) != self.party._member(resolved[2], c.account))):
            self._iq_response(c, iqid, 'error', '<error type="auth"><forbidden xmlns="urn:ietf:params:xml:ns:xmpp-stanzas"/></error>')
            return True
        payload = '<query xmlns="%s"><identity category="conference" type="text"/><feature var="%s"/></query>' % (DISCO, MUC) if query.tag == tag(DISCO, 'query') and root.get('type') == 'get' else ''
        self._send(c, '<iq xmlns="jabber:client" from=%s to=%s type="result" id=%s>%s</iq>' % (quoteattr(resolved[0]), quoteattr(c.jid), iqid, payload))
        return True


class _Connection:
    def __init__(self, ws):
        self.ws, self.phase = ws, 'open'
        self.account = self.token = self.session = self.jid = self.domain = self.status = None
        self.available = False
        self.rooms = {}
        self.done = threading.Event()
        self.stop = threading.Event()
        self.outbound = queue.Queue(MAX_PENDING)
        self.last_sent = None
