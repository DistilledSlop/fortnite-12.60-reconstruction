"""Exchange-code lifecycle and local-client compatibility over HTTP."""

from test_services import ServiceTestCase, jcall, LOCAL
from api_common import Err


class ExchangeCodeTests(ServiceTestCase):
    def exchange(self, code):
        return jcall(self.base, "POST", "/account/api/oauth/token",
                     form={"grant_type": "exchange_code", "exchange_code": code})

    def test_unknown_issued_format_and_expired_codes_never_sign_in(self):
        self.assertError(self.exchange("local-exchange:unknown"), 400, Err.OAUTH_INVALID_GRANT)
        account = self.new_account("Exchange owner")
        token = self.sign_in_account(account)["access_token"]
        status, issued, _ = jcall(self.base, "GET", "/account/api/oauth/exchange", token=token)
        self.assertEqual(status, 200)
        self.clock.now += issued["expiresInSeconds"]
        self.assertError(self.exchange(issued["code"]), 400, Err.OAUTH_INVALID_GRANT)
        self.assertError(self.exchange(issued["code"]), 400, Err.OAUTH_INVALID_GRANT)

    def test_legacy_password_and_local_username_still_work(self):
        status, local, _ = self.exchange("arbitrary-original-client-password")
        self.assertEqual(status, 200)
        self.assertEqual(local["account_id"], LOCAL)
        status, named, _ = self.exchange("local-username:ExchangeUser")
        self.assertEqual(status, 200)
        self.assertEqual(named["displayName"], "ExchangeUser")
        self.assertNotEqual(named["account_id"], LOCAL)
