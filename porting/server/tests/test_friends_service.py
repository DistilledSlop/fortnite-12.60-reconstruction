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
        self.assertEqual(len(self.events), 1)
        self.assertEqual(self.events[0][0], self.b)
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
        self.assertEqual(self.service.notification_failures, 1)

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
