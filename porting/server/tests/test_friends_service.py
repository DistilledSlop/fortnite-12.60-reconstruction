"""Friends graph behavior, routing, authorization and durable state."""

# Isolated mode omits the script directory from sys.path.
if __package__:
    from . import _bootstrap
else:
    import sys as _sys
    from pathlib import Path as _Path
    _sys.path.insert(0, str(_Path(__file__).resolve().parent))
    import _bootstrap

import tempfile
import unittest
import sqlite3
from types import SimpleNamespace

from account_service import AccountService
from api_common import ApiError, Router
from friends_service import FriendsService


class FriendsTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.ctx = SimpleNamespace(iso=lambda: '2026-01-01T00:00:00Z')
        self.accounts = AccountService(self.ctx, state_dir=self.tmp.name)
        self.a = '00000000000000000000000000000001'
        self.b = self.accounts.create('Guest')['id']
        self.events = []
        self.restart()

    def restart(self):
        if hasattr(self, "service"):
            self.service.close()
        oauth = SimpleNamespace(account_session=lambda req: SimpleNamespace(account_id=req))
        self.service = FriendsService(self.ctx, self.accounts, oauth, state_dir=self.tmp.name,
                                      notify=lambda recipient, event: self.events.append((recipient, event)))
        self.addCleanup(self.service.close)
        self.router = Router()
        self.service.register(self.router)

    def call(self, who, method, path):
        route, match, _ = self.router.resolve(method, path)
        self.assertIsNotNone(route, path)
        return route.handler(who, match)

    def summary(self, who):
        return self.call(who, 'GET', f'/friends/api/v1/{who}/summary')[1]

    def test_request_accept_delete_and_restart(self):
        ab = f'/friends/api/public/friends/{self.a}/{self.b}'
        ba = f'/friends/api/public/friends/{self.b}/{self.a}'
        self.assertEqual(self.call(self.a, 'POST', ab)[0], 204)
        self.call(self.a, 'POST', ab)
        self.assertEqual(len(self.events), 2)
        self.assertEqual(self.events[1][0], self.b)
        self.assertEqual(self.summary(self.a)['outgoing'][0]['accountId'], self.b)
        self.assertEqual(self.summary(self.b)['incoming'][0]['accountId'], self.a)
        self.restart()
        self.call(self.b, 'POST', ba)
        self.assertEqual(self.summary(self.a)['friends'][0]['accountId'], self.b)
        self.assertEqual(self.summary(self.b)['friends'][0]['accountId'], self.a)
        self.assertTrue(self.service.are_friends(self.a, self.b))
        self.assertEqual(self.summary(self.a)['outgoing'], [])
        self.restart()
        self.assertEqual(len(self.summary(self.b)['friends']), 1)
        self.call(self.a, 'DELETE', ab)
        self.call(self.a, 'DELETE', ab)
        self.assertEqual(self.summary(self.b)['friends'], [])

    def test_blocks_clean_edges_and_prevent_requests(self):
        ab = f'/friends/api/public/friends/{self.a}/{self.b}'
        ba = f'/friends/api/public/friends/{self.b}/{self.a}'
        block = f'/friends/api/v1/{self.a}/blocklist/{self.b}'
        self.call(self.b, 'POST', ba)
        self.call(self.a, 'POST', block)
        self.assertEqual(self.summary(self.b)['outgoing'], [])
        self.assertTrue(self.service.is_blocked(self.b, self.a))
        self.restart()
        self.assertEqual(self.summary(self.a)['blocklist'][0]['accountId'], self.b)
        with self.assertRaises(ApiError) as exc:
            self.call(self.b, 'POST', ba)
        self.assertEqual(exc.exception.status, 403)
        self.call(self.a, 'DELETE', block)
        self.call(self.a, 'POST', ab)
        self.call(self.b, 'POST', ba)
        self.call(self.a, 'POST', block)
        self.assertEqual(self.summary(self.b)['friends'], [])

    def test_authorization_validation_and_notification_failure(self):
        with self.assertRaises(ApiError) as exc:
            self.call(self.b, 'GET', f'/friends/api/v1/{self.a}/summary')
        self.assertEqual(exc.exception.status, 403)
        with self.assertRaises(ApiError):
            self.call(self.a, 'POST', f'/friends/api/public/friends/{self.a}/bad')
        self.service.notify = lambda *_: (_ for _ in ()).throw(RuntimeError())
        self.call(self.a, 'POST', f'/friends/api/public/friends/{self.a}/{self.b}')
        self.assertEqual(len(self.summary(self.b)['incoming']), 1)
        self.assertEqual(self.service.notification_failures, 2)

    def test_notification_payloads_and_committed_relationships(self):
        ab = f'/friends/api/public/friends/{self.a}/{self.b}'
        ba = f'/friends/api/public/friends/{self.b}/{self.a}'
        database = sqlite3.connect(f'{self.tmp.name}/friends.sqlite3')
        self.addCleanup(database.close)
        observations = []
        def notify(recipient, event):
            rows = (database.execute('SELECT src,dst,created FROM requests').fetchall(),
                    database.execute('SELECT a,b,created FROM friends').fetchall())
            observations.append((recipient, event, rows))
        self.service.notify = notify

        self.call(self.a, 'POST', ab)
        self.call(self.a, 'POST', ab)
        created = self.summary(self.a)['outgoing'][0]['created']
        friend_type = 'com.epicgames.friends.core.apiobjects.Friend'
        self.assertEqual([(r, e) for r, e, _ in observations], [
            (self.a, {'type': friend_type, 'accountId': self.b, 'status': 'PENDING', 'direction': 'OUTBOUND', 'created': created}),
            (self.b, {'type': friend_type, 'accountId': self.a, 'status': 'PENDING', 'direction': 'INBOUND', 'created': created})])
        self.assertTrue(all(rows == ([(self.a, self.b, created)], []) for _, _, rows in observations))
        observations.clear()
        self.call(self.b, 'POST', ba)
        self.call(self.b, 'POST', ba)
        created = self.summary(self.a)['friends'][0]['created']
        self.assertEqual([(r, e) for r, e, _ in observations], [
            (self.b, {'type': friend_type, 'accountId': self.a, 'status': 'ACCEPTED', 'direction': 'OUTBOUND', 'created': created}),
            (self.a, {'type': friend_type, 'accountId': self.b, 'status': 'ACCEPTED', 'direction': 'OUTBOUND', 'created': created})])
        self.assertTrue(all(rows == ([], [(*sorted((self.a, self.b)), created)]) for _, _, rows in observations))
        observations.clear()
        self.call(self.a, 'DELETE', ab)
        self.call(self.a, 'DELETE', ab)
        self.assertEqual([(r, e) for r, e, _ in observations], self.removal_events(self.a, self.b, 'DELETED'))
        self.assertTrue(all(rows == ([], []) for _, _, rows in observations))

    def removal_events(self, actor, other, reason):
        kind = 'com.epicgames.friends.core.apiobjects.FriendRemoval'
        return [(actor, {'type': kind, 'accountId': other, 'reason': reason}),
                (other, {'type': kind, 'accountId': actor, 'reason': reason})]

    def test_cancel_reject_and_block_removal_notifications(self):
        for action, reason in (('cancel', 'ABORTED'), ('reject', 'REJECTED'),
                               ('block_outgoing', 'ABORTED'), ('block_incoming', 'REJECTED'),
                               ('block_friend', 'DELETED')):
            with self.subTest(action=action):
                ab = f'/friends/api/public/friends/{self.a}/{self.b}'
                ba = f'/friends/api/public/friends/{self.b}/{self.a}'
                block = f'/friends/api/v1/{self.a}/blocklist/{self.b}'
                if action == 'block_incoming':
                    self.call(self.b, 'POST', ba)
                else:
                    self.call(self.a, 'POST', ab)
                if action == 'block_friend':
                    self.call(self.b, 'POST', ba)
                self.events.clear()
                actor, peer = (self.b, self.a) if action == 'reject' else (self.a, self.b)
                method, path = ('POST', block) if action.startswith('block') else ('DELETE', ba if action == 'reject' else ab)
                self.call(actor, method, path)
                self.call(actor, method, path)
                self.assertEqual(self.events, self.removal_events(actor, peer, reason))
                self.assertEqual(self.summary(self.a)['friends'], [])
                self.assertEqual(self.summary(self.a)['incoming'], [])
                self.assertEqual(self.summary(self.a)['outgoing'], [])
                if action.startswith('block'):
                    self.call(self.a, 'DELETE', block)

    def test_failed_request_does_not_notify(self):
        block = f'/friends/api/v1/{self.a}/blocklist/{self.b}'
        self.call(self.a, 'POST', block)
        with self.assertRaises(ApiError):
            self.call(self.b, 'POST', f'/friends/api/public/friends/{self.b}/{self.a}')
        self.assertEqual(self.events, [])

    def test_v1_friend_mutations_use_same_authorization_and_graph(self):
        ab = f'/friends/api/v1/{self.a}/friends/{self.b}'
        ba = f'/friends/api/v1/{self.b}/friends/{self.a}'
        route, _, _ = self.router.resolve('POST', ab)
        self.assertEqual(route.status, 'observed')
        route, _, _ = self.router.resolve('DELETE', ab)
        self.assertEqual(route.status, 'observed')
        with self.assertRaises(ApiError) as exc:
            self.call(self.b, 'POST', ab)
        self.assertEqual(exc.exception.status, 403)
        self.assertEqual(self.events, [])
        self.call(self.a, 'POST', ab)
        self.call(self.a, 'POST', ab)
        self.assertEqual(len(self.events), 2)
        self.assertEqual(self.events[0][1]['direction'], 'OUTBOUND')
        self.assertEqual(self.events[1][1]['direction'], 'INBOUND')
        self.call(self.b, 'POST', ba)
        self.call(self.b, 'POST', ba)
        self.assertTrue(self.service.are_friends(self.a, self.b))
        self.assertEqual(len(self.events), 4)
        self.assertTrue(all(event['status'] == 'ACCEPTED' for _, event in self.events[2:]))
        with self.assertRaises(ApiError):
            self.call(self.b, 'DELETE', ab)
        self.assertEqual(len(self.events), 4)
        self.call(self.a, 'DELETE', ab)
        self.call(self.a, 'DELETE', ab)
        self.assertEqual(self.events[4:], self.removal_events(self.a, self.b, 'DELETED'))

    def test_mutual_friend_permission_and_accepting_existing_request(self):
        c = self.accounts.create('Mutual')['id']
        def friend(src, dst):
            self.call(src, 'POST', f'/friends/api/public/friends/{src}/{dst}')
        self.service._db.execute('INSERT INTO settings VALUES (?,?,?)', (self.b, 'FRIENDS_OF_FRIENDS', None))
        self.service._db.commit()
        with self.assertRaises(ApiError):
            friend(self.a, self.b)
        friend(self.a, c)
        friend(c, self.a)
        friend(self.b, c)
        friend(c, self.b)
        friend(self.a, self.b)
        self.service._db.execute('UPDATE settings SET accept_invites=? WHERE account=?', ('PRIVATE', self.a))
        self.service._db.commit()
        friend(self.b, self.a)
        self.assertTrue(self.service.are_friends(self.a, self.b))


class FriendsHttpTests(unittest.TestCase):
    def test_settings_persist_and_control_new_requests(self):
        if __package__:
            from . import test_services as support
        else:
            import test_services as support
        from backend import Backend
        with tempfile.TemporaryDirectory() as directory:
            backend = Backend(state_dir=directory, state_step_sec=0)
            server = support.LiveServer(backend).start()
            try:
                a = '00000000000000000000000000000001'
                b = backend.accounts.create('Guest')['id']
                at = backend.oauth.local_sign_in(a)['access_token']
                bt = backend.oauth.local_sign_in(b)['access_token']
                path = f'/friends/api/v1/{b}/settings'
                request = f'/friends/api/public/friends/{a}/{b}'
                self.assertEqual(support.jcall(server.base, 'PUT', path, token=at, body={'acceptInvites': 'PRIVATE'})[0], 403)
                self.assertEqual(support.jcall(server.base, 'PUT', path, token=bt, body={'acceptInvites': 'bad'})[0], 400)
                status, settings, _ = support.jcall(server.base, 'PUT', path, token=bt, body={'acceptInvites': 'PRIVATE', 'mutualPrivacy': 'FRIENDS'})
                self.assertEqual((status, settings), (200, {'acceptInvites': 'PRIVATE', 'mutualPrivacy': 'FRIENDS'}))
                self.assertEqual(support.call(server.base, 'POST', request, token=at)[0], 403)
            finally:
                server.stop()
                backend.close()
            backend = Backend(state_dir=directory, state_step_sec=0)
            server = support.LiveServer(backend).start()
            try:
                bt = backend.oauth.local_sign_in(b)['access_token']
                at = backend.oauth.local_sign_in(a)['access_token']
                self.assertEqual(support.jcall(server.base, 'GET', path, token=bt)[1], settings)
                self.assertEqual(support.jcall(server.base, 'GET', f'/friends/api/v1/{b}/summary', token=bt)[1]['settings'], settings)
                self.assertEqual(support.jcall(server.base, 'GET', f'/friends/api/v1/{a}/settings', token=at)[1], {'acceptInvites': 'public'})
                self.assertEqual(support.call(server.base, 'POST', request, token=at)[0], 403)
                self.assertEqual(support.jcall(server.base, 'PUT', path, token=bt, body={'acceptInvites': 'public'})[1]['mutualPrivacy'], 'FRIENDS')
                self.assertEqual(support.call(server.base, 'POST', request, token=at)[0], 204)
            finally:
                server.stop()
                backend.close()

    def test_real_oauth_and_http_across_restart(self):
        if __package__:
            from . import test_services as support
        else:
            import test_services as support
        from backend import Backend
        with tempfile.TemporaryDirectory() as directory:
            def start():
                backend = Backend(state_dir=directory, state_step_sec=0)
                return backend, support.LiveServer(backend).start()
            backend, server = start()
            try:
                a = '00000000000000000000000000000001'
                b = backend.accounts.create('Guest')['id']
                at = backend.oauth.local_sign_in(a)['access_token']
                bt = backend.oauth.local_sign_in(b)['access_token']
                path = f'/friends/api/public/friends/{a}/{b}'
                self.assertEqual(support.call(server.base, 'POST', path, token=at)[0], 204)
                self.assertEqual(support.call(server.base, 'GET', f'/friends/api/v1/{a}/summary', token=bt)[0], 403)
                self.assertEqual(support.call(server.base, 'GET', f'/friends/api/v1/{a}/summary')[0], 401)
            finally:
                server.stop()
                backend.close()
            backend, server = start()
            try:
                bt = backend.oauth.local_sign_in(b)['access_token']
                status, summary, _ = support.jcall(server.base, 'GET', f'/friends/api/v1/{b}/summary', token=bt)
                self.assertEqual(status, 200)
                self.assertEqual(summary['incoming'][0]['accountId'], a)
            finally:
                server.stop()
                backend.close()


if __name__ == '__main__':
    unittest.main()
