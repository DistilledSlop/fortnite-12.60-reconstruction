import http.client
import json
import os
from pathlib import Path
import sys
import tempfile
import threading
import unittest
from http.server import ThreadingHTTPServer
from urllib.parse import urlsplit

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import probe_server

WALLET = 'abcdef0123456789abcdef0123456789'
OTHER = '11111111111111111111111111111111'
PATH = '/fortnite/api/game/v2/profile/private-account/client/'


class WalletTracePrivacyTests(unittest.TestCase):
    def test_filter_and_correlation(self):
        ids = set()
        profile = {'profileRevision': 14, 'profileCommandRevision': 9,
                   'profileChangesBaseRevision': 14, 'private': 'SECRET',
                   'profileChanges': [{'changeType': 'fullProfileUpdate', 'profile': {
                       'accountId': 'SECRET', 'stats': {'secret': 'SECRET'}, 'items': {
                           WALLET: {'templateId': 'Currency:MtxGiveaway', 'quantity': 750,
                                    'attributes': {'platform': 'EpicPC', 'secret': 'SECRET'}},
                           OTHER: {'templateId': 'Athena:Secret', 'quantity': 33}}}}]}
        url = urlsplit(PATH + 'QueryProfile?profileId=common_core&rvn=-1&token=SECRET')
        entry = probe_server.wallet_trace_entry('POST', url, 200, profile, ids)
        self.assertEqual(entry['profileChanges'][0]['currencyItems'],
                         [{'itemId': WALLET, 'quantity': 750, 'platform': 'EpicPC'}])
        self.assertEqual(entry['requestRevision'], -1)
        response = {'profileRevision': 15, 'profileChanges': [
            {'changeType': 'itemQuantityChanged', 'itemId': WALLET, 'quantity': 1750},
            {'changeType': 'itemQuantityChanged', 'itemId': OTHER, 'quantity': 22}],
            'notifications': [{'type': 'CatalogPurchase', 'lootResult': {'items': [
                {'itemType': 'Currency:MtxGiveaway', 'itemGuid': WALLET, 'quantity': 1000,
                 'attributes': {'platform': 'EpicPC', 'secret': 'SECRET'}},
                {'itemType': 'Athena:Secret', 'itemGuid': OTHER, 'quantity': 1}]}}]}
        purchased = probe_server.wallet_trace_entry('POST', urlsplit(PATH + 'PurchaseCatalogEntry?profileId=common_core&rvn=14'), 200, response, ids)
        self.assertEqual(purchased['profileChanges'][0]['quantity'], 1750)
        self.assertEqual(purchased['profileChanges'][1], {'changeType': 'itemQuantityChanged'})
        self.assertEqual(purchased['purchaseRewards'][0]['quantity'], 1000)
        text = json.dumps([entry, purchased])
        for secret in ('SECRET', OTHER, 'private-account', 'Athena:Secret'):
            self.assertNotIn(secret, text)
        self.assertIsNone(probe_server.wallet_trace_entry('POST', urlsplit(PATH + 'QueryProfile?profileId=athena'), 200, profile, ids))
        self.assertIsNone(probe_server.wallet_trace_entry('GET', url, 200, profile, ids))
        self.assertIsNone(probe_server.wallet_trace_entry('POST', urlsplit(PATH + 'Other?profileId=common_core'), 200, profile, ids))

    def test_http_opt_in_permissions_and_response_unchanged(self):
        class Stub:
            def respond(self, *args, **kwargs):
                return 200, {'profileRevision': 2, 'secret': 'PRIVATE', 'profileChanges': []}
        with tempfile.TemporaryDirectory() as tmp:
            trace = Path(tmp) / 'wallet.jsonl'
            trace.write_text('')
            os.chmod(trace, 0o666)
            handler = probe_server.make_handler([], Path(tmp) / 'requests', Stub(), trace)
            server = ThreadingHTTPServer(('127.0.0.1', 0), handler)
            thread = threading.Thread(target=server.serve_forever, daemon=True)
            thread.start()
            try:
                conn = http.client.HTTPConnection('127.0.0.1', server.server_port)
                conn.request('POST', PATH + 'QueryProfile?profileId=common_core&rvn=1',
                             body='SECRET', headers={'Authorization': 'Bearer SECRET'})
                response = conn.getresponse()
                self.assertEqual(json.loads(response.read())['secret'], 'PRIVATE')
                conn.close()
            finally:
                server.shutdown()
                thread.join()
                server.server_close()
            self.assertEqual(trace.stat().st_mode & 0o777, 0o600)
            self.assertNotIn('SECRET', trace.read_text())
            self.assertNotIn('PRIVATE', trace.read_text())
            self.assertEqual(json.loads(trace.read_text())['profileRevision'], 2)


if __name__ == '__main__':
    unittest.main()
