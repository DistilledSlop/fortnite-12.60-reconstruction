"""Season 12 discovery grants and trusted quest-event persistence."""
if __package__:
    from . import _bootstrap
else:
    import sys as _sys
    from pathlib import Path as _Path
    _sys.path.insert(0, str(_Path(__file__).resolve().parent))
    import _bootstrap

import json
import tempfile
import unittest

from api_common import LOCAL_ACCOUNT_ID
from backend import Backend
from season12_discovery_data import DISCOVERY_QUESTS


class DiscoveryQuestTests(unittest.TestCase):
    def test_grants_bridge_replay_and_persistence(self):
        with tempfile.TemporaryDirectory() as directory:
            backend = Backend(clock=lambda: 1700000000, state_dir=directory, gameserver_secret='secret')
            self.addCleanup(lambda: backend.close())
            def post(path, body):
                return backend.respond('POST', path, body=json.dumps(body).encode(),
                                       headers={'Authorization': 'Bearer secret'})
            status, registration = post('/local/gameserver/register', {
                'address': '127.0.0.1', 'port': 7777, 'playlists': ['Playlist_DefaultSolo'],
                'maxPlayers': 2, 'buildUniqueId': 'test'})
            self.assertEqual(status, 200)
            sid = registration['sessionId']
            post('/local/gameserver/' + sid + '/state', {'phase': 'warmup', 'numPlayers': 0, 'joinable': True})
            backend.gameservers.reserve('Playlist_DefaultSolo', [LOCAL_ACCOUNT_ID])
            backend.profiles.run(LOCAL_ACCOUNT_ID, 'athena', 'ClientQuestLogin', {})
            def items():
                return backend.profiles.profile(LOCAL_ACCOUNT_ID, 'athena')['items']
            def quest(name):
                matches = [item for item in items().values() if item['templateId'].casefold() == ('quest:' + name).casefold()]
                self.assertEqual(len(matches), 1)
                return matches[0]['attributes']
            for name, definition in DISCOVERY_QUESTS.items():
                attrs = quest(name)
                self.assertEqual(attrs['quest_state'], 'Active')
                self.assertIsNone(attrs['challenge_bundle_id'])
                for objective, _, _ in definition['objectives']:
                    self.assertEqual(attrs['completion_' + objective.lower()], 0)
            path = '/local/gameserver/' + sid + '/quest-event'
            def send(event, event_id):
                return post(path, {'accountId': LOCAL_ACCOUNT_ID, 'eventId': event_id, 'event': event})
            named = 'Quest_S11_Discover_NamedLocations'
            landmarks = 'Quest_S11_Discover_Landmarks'
            self.assertEqual(send('Visit_Apollo_Location_BeachyBluffs', 'named'), (200, {'accepted': True}))
            self.assertEqual(send('visit_landmark_MilitaryCamp.01', 'landmark'), (200, {'accepted': True}))
            self.assertEqual(send('visit_namedpoi_spybase_oilrig', 'rig1'), (200, {'accepted': True}))
            self.assertEqual(send('visit_namedpoi_spybase_oilrig', 'rig1'), (200, {'accepted': False}))
            self.assertEqual(quest(landmarks)['completion_visit_namedpoi_spybase_oilrig'], 1)
            self.assertEqual(send('visit_namedpoi_spybase_oilrig', 'rig2'), (200, {'accepted': True}))
            self.assertEqual(send('objective:impossible_poi_of_permanence', 'impossible')[0], 400)
            backend.profiles.run(LOCAL_ACCOUNT_ID, 'athena', 'ClientQuestLogin', {})
            before = items()
            self.assertEqual(quest(named)['completion_visit_location_beachybluffs'], 1)
            self.assertEqual(quest(named)['completion_visit_location_dirtydocks'], 0)
            self.assertEqual(quest(landmarks)['completion_visit_landmark_militarycamp_01'], 1)
            self.assertEqual(quest(landmarks)['completion_visit_namedpoi_spybase_oilrig'], 2)
            backend.close()
            backend = Backend(clock=lambda: 1700000000, state_dir=directory, gameserver_secret='secret')
            self.assertEqual(items(), before)
            self.assertEqual(send('visit_namedpoi_spybase_oilrig', 'rig2'), (200, {'accepted': False}))
