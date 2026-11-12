"""Owner-private friend aliases through the real backend dispatcher."""
if __package__:
    from . import _bootstrap
else:
    import _bootstrap

import tempfile
import unittest
import contextlib
import io

from backend import Backend


class FriendAliasTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.backend = Backend(state_dir=self.tmp.name)
        self.addCleanup(lambda: self.backend.close())
        self.a = self.login('Alias Alice')
        self.b = self.login('Alias Bob')

    def login(self, name):
        status, auth = self.backend.respond('POST', '/account/api/oauth/token',
                                            form={'grant_type': 'local_username', 'username': name})
        self.assertEqual(status, 200)
        return auth['account_id'], auth['access_token']

    def call(self, who, method, path, body=b'', content_type='text/plain'):
        headers = {'Authorization': 'Bearer ' + who[1]}
        if content_type is not None:
            headers['Content-Type'] = content_type
        return self.backend.respond(method, path, body=body, headers=headers)

    def path(self, owner, peer):
        return f'/friends/api/v1/{owner[0]}/friends/{peer[0]}'

    def befriend(self):
        self.assertEqual(self.call(self.a, 'POST', self.path(self.a, self.b))[0], 204)
        self.assertEqual(self.call(self.b, 'POST', self.path(self.b, self.a))[0], 204)

    def summary(self, who):
        return self.call(who, 'GET', f'/friends/api/v1/{who[0]}/summary')[1]['friends']

    def test_auth_privacy_persistence_update_and_delete(self):
        path = self.path(self.a, self.b) + '/alias'
        self.assertEqual(self.call(self.a, 'PUT', path, b'Buddy')[0], 404)
        self.befriend()
        self.assertEqual(self.backend.respond('PUT', path, body=b'Buddy', headers={'Content-Type': 'text/plain'})[0], 401)
        self.assertEqual(self.call(self.b, 'PUT', path, b'Buddy')[0], 403)
        self.assertEqual(self.call(self.a, 'PUT', path, ' Buddy 😄 '.encode())[0], 204)
        self.assertEqual(self.summary(self.a)[0]['alias'], ' Buddy 😄 ')
        self.assertNotIn('alias', self.summary(self.b)[0])
        self.assertEqual(self.call(self.b, 'PUT', self.path(self.b, self.a) + '/alias', b'Other')[0], 204)
        self.assertEqual(self.summary(self.a)[0]['alias'], ' Buddy 😄 ')
        self.assertEqual(self.summary(self.b)[0]['alias'], 'Other')
        legacy = self.call(self.a, 'GET', f'/friends/api/public/friends/{self.a[0]}')[1]
        self.assertEqual(legacy[0]['alias'], ' Buddy 😄 ')
        self.backend.close()
        self.backend = Backend(state_dir=self.tmp.name)
        # Tokens are process-local, so sign in again after restart.
        self.a = self.login('Alias Alice')
        self.b = self.login('Alias Bob')
        self.assertEqual(self.summary(self.a)[0]['alias'], ' Buddy 😄 ')
        self.assertEqual(self.call(self.a, 'PUT', path, b'New')[0], 204)
        self.assertEqual(self.summary(self.a)[0]['alias'], 'New')
        self.assertEqual(self.call(self.a, 'DELETE', path), (204, None))
        self.assertEqual(self.call(self.a, 'DELETE', path), (204, None))
        self.assertNotIn('alias', self.summary(self.a)[0])

    def test_validation_and_boundaries(self):
        self.befriend()
        path = self.path(self.a, self.b) + '/alias'
        for body, content_type in [(b'\xff', 'text/plain'), (b'', 'text/plain'), (b'  ', 'text/plain'),
                                   (b'a\x00b', 'text/plain'), ('\u0085'.encode(), 'text/plain'),
                                   (b'x' * 17, 'text/plain'), ('😄'.encode() * 9, 'text/plain'),
                                   (b'name', 'application/json'), (b'name', None)]:
            with self.subTest(body=body, content_type=content_type):
                self.assertEqual(self.call(self.a, 'PUT', path, body, content_type)[0], 400)
        for value in ('x' * 16, '😄' * 8):
            self.assertEqual(self.call(self.a, 'PUT', path, value.encode(), 'text/plain; charset=utf-8')[0], 204)
            self.assertEqual(self.summary(self.a)[0]['alias'], value)

    def test_removal_block_and_atomic_rollback(self):
        self.befriend()
        a_path = self.path(self.a, self.b)
        b_path = self.path(self.b, self.a)
        for owner, path in ((self.a, a_path), (self.b, b_path)):
            self.assertEqual(self.call(owner, 'PUT', path + '/alias', b'private')[0], 204)
        db = self.backend.friends._db
        db.execute("CREATE TRIGGER fail_alias_delete BEFORE DELETE ON aliases BEGIN SELECT RAISE(ABORT, 'injected'); END")
        db.commit()
        with contextlib.redirect_stderr(io.StringIO()):
            self.assertEqual(self.call(self.a, 'DELETE', a_path)[0], 500)
        self.assertEqual(len(self.summary(self.a)), 1)
        self.assertEqual(db.execute('SELECT count(*) FROM aliases').fetchone()[0], 2)
        db.execute('DROP TRIGGER fail_alias_delete')
        db.commit()
        self.assertEqual(self.call(self.a, 'DELETE', a_path)[0], 204)
        self.assertEqual(db.execute('SELECT count(*) FROM aliases').fetchone()[0], 0)
        self.befriend()
        for owner, path in ((self.a, a_path), (self.b, b_path)):
            self.assertEqual(self.call(owner, 'PUT', path + '/alias', b'private')[0], 204)
        self.assertEqual(self.call(self.a, 'POST', f'/friends/api/v1/{self.a[0]}/blocklist/{self.b[0]}')[0], 204)
        self.assertEqual(db.execute('SELECT count(*) FROM aliases').fetchone()[0], 0)
        self.assertEqual(self.call(self.a, 'DELETE', a_path + '/alias')[0], 404)


if __name__ == '__main__':
    unittest.main()
