"""Trusted game-server quest events and operator calendar configuration."""
if __package__:
    from . import _bootstrap
else:
    import sys as _sys
    from pathlib import Path as _Path
    _sys.path.insert(0, str(_Path(__file__).resolve().parent))
    import _bootstrap

import argparse
import json
import tempfile
import unittest
from datetime import datetime, timezone

from api_common import LOCAL_ACCOUNT_ID
from backend import Backend
from probe_server import quest_event_start
from profile_service import CommandContext, _grant_season12_rewards
from season12_rewards import PAID

TAG = 'EventFlag.LobbySeason12'

class QuestBridgeTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.now = 1700000000
        self.backend = self.start()
        self.addCleanup(lambda: self.backend.close())
        status, registration = self.post('/local/gameserver/register', dict(address='127.0.0.1', port=7777,
            playlists=['Playlist_DefaultSolo'], maxPlayers=2, buildUniqueId='test'))
        self.assertEqual(status, 200)
        self.sid = registration['sessionId']
        self.post('/local/gameserver/' + self.sid + '/state', dict(phase='warmup', numPlayers=0, joinable=True))
        self.assertEqual(self.backend.gameservers.reserve('Playlist_DefaultSolo', [LOCAL_ACCOUNT_ID]), self.sid)
        ctx = CommandContext(self.backend.profiles, LOCAL_ACCOUNT_ID, 'athena', {})
        _grant_season12_rewards(ctx, ctx.tx('athena'), PAID[1], 0)
        self.backend.profiles._store(LOCAL_ACCOUNT_ID, ctx.txs)
        self.backend.profiles.run(LOCAL_ACCOUNT_ID, 'athena', 'ClientQuestLogin', {})

    def start(self, **kw):
        return Backend(clock=lambda: self.now, state_dir=self.tmp.name, gameserver_secret='secret', **kw)

    def post(self, path, payload, token='secret'):
        return self.backend.respond('POST', path, body=json.dumps(payload).encode(),
                                    headers={'Authorization': 'Bearer ' + token})

    def event(self, **changes):
        payload = dict(accountId=LOCAL_ACCOUNT_ID, eventId='first', event='donut_w1_startquest')
        payload.update(changes)
        return self.post('/local/gameserver/' + self.sid + '/quest-event', payload)

    def profile(self):
        return self.backend.profiles.profile(LOCAL_ACCOUNT_ID, 'athena')

    def test_admission_validation_and_durable_replay(self):
        before = self.profile()
        path = '/local/gameserver/' + self.sid + '/quest-event'
        payload = dict(accountId=LOCAL_ACCOUNT_ID, eventId='first', event='donut_w1_startquest')
        self.assertEqual(self.post(path, payload, 'bad')[0], 401)
        self.assertEqual(self.post('/local/gameserver/wrong/quest-event', payload)[0], 404)
        for change in ({'eventId': ''}, {'eventId': 'x' * 129}, {'amount': True}, {'event': 'unknown'}, {'extra': 1}):
            self.assertEqual(self.event(**change)[0], 400)
        self.assertEqual(before, self.profile())
        self.assertEqual(self.event(), (200, {'accepted': True}))
        after = self.profile()
        quest = next(item for item in after['items'].values() if item['templateId'].lower() == 'quest:quest_s12_donut_w1_a')
        self.assertEqual(quest['attributes']['quest_state'], 'Claimed')
        self.assertEqual(self.event(), (200, {'accepted': False}))
        self.assertEqual(self.event(amount=2)[0], 400)
        self.assertEqual(after, self.profile())
        self.now += 31
        self.assertEqual(self.event(), (200, {'accepted': False}))
        self.assertEqual(self.event(eventId='new')[0], 404)
        self.backend.close()
        self.backend = self.start()
        self.assertEqual(self.event(), (200, {'accepted': False}))
        self.assertEqual(self.event(eventId='new')[0], 404)
        self.assertEqual(after, self.profile())

    def test_registered_session_is_trusted_weekly_match_context(self):
        quests = self.backend.profiles.quests
        ctx = CommandContext(self.backend.profiles, LOCAL_ACCOUNT_ID, 'athena', {})
        quests._grant(ctx.tx('athena'), 'ChallengeBundleSchedule:Season12_Mission_Schedule', {})
        quests.login(ctx, include_daily=False)
        self.backend.profiles._store(LOCAL_ACCOUNT_ID, ctx.txs)
        self.assertEqual(self.event(event='open_faction_locked_doors', amount=3), (200, {'accepted': True}))
        self.assertEqual(self.event(event='open_faction_locked_doors', eventId='second', amount=3),
                         (200, {'accepted': True}))
        quest = next(v for v in self.profile()['items'].values()
                     if v['templateId'] == 'Quest:quest_s12_w1_q03')
        self.assertEqual(quest['attributes']['completion_quest_s12_w1_q03_obj'], 1)
        before = self.profile()
        self.assertEqual(self.event(event='open_faction_locked_doors', eventId='forged', matchId='other')[0], 400)
        self.assertEqual(before, self.profile())
        self.backend.close()
        self.backend = self.start()
        self.assertEqual(self.event(event='open_faction_locked_doors', amount=3), (200, {'accepted': False}))
        self.assertEqual(before, self.profile())

    def test_legacy_event_receipt_replays_after_match_context_upgrade(self):
        quests = self.backend.profiles.quests
        quests.progress(LOCAL_ACCOUNT_ID, 'donut_w1_startquest',
                        event_id='gameserver:' + self.sid + ':first')
        before = self.profile()
        self.now += 31
        self.assertEqual(self.event(), (200, {'accepted': False}))
        self.assertEqual(before, self.profile())

    def test_nonmember_cannot_record(self):
        self.backend.gameservers._servers[self.sid]['players'].clear()
        before = self.profile()
        self.assertEqual(self.event()[0], 403)
        self.assertEqual(before, self.profile())

    def test_replay_lookup_does_not_create_profile(self):
        account = self.backend.accounts.create('NoProfile')['id']
        path = '/local/gameserver/' + self.sid + '/quest-event'
        payload = dict(accountId=account, eventId='unseen', event='donut_w1_startquest')
        self.assertIsNone(self.backend.profiles.existing_profile(account, 'athena'))
        self.assertEqual(self.post(path, payload)[0], 403)
        self.assertIsNone(self.backend.profiles.existing_profile(account, 'athena'))
        self.assertEqual(self.post(path, dict(payload, accountId='missing'))[0], 404)
        self.assertEqual(self.post(path, dict(payload, accountId=[]))[0], 404)

    def test_calendar_validation_and_reconcile(self):
        for starts in ({TAG: float('nan')}, {TAG: float('inf')}, {TAG: True},
                       {TAG: datetime(2020, 1, 1)}, {'unknown': 0}, {TAG: 'bad'}, {TAG: 10**1000}):
            with self.subTest(starts=starts), self.assertRaises(ValueError):
                self.start(quest_event_starts=starts)
        self.assertEqual(quest_event_start(TAG + '=1700000000'), (TAG, 1700000000))
        self.assertEqual(quest_event_start('EventFlag.LTE_StormTheAgency=1700000000'), ('EventFlag.LTE_StormTheAgency', 1700000000))
        self.assertEqual(quest_event_start(TAG + '=2023-11-14T22:13:20Z'), (TAG, 1700000000))
        for value in (TAG + '=NaN', TAG + '=2020-01-01', 'unknown=0'):
            with self.assertRaises(argparse.ArgumentTypeError): quest_event_start(value)
        self.backend.close()
        self.backend = self.start(quest_event_starts={TAG: datetime.fromtimestamp(self.now - 43*86400, timezone.utc)})
        self.backend.profiles.run(LOCAL_ACCOUNT_ID, 'athena', 'ClientQuestLogin', {})
        templates = {x['templateId'].lower() for x in self.profile()['items'].values()}
        self.assertIn('challengebundle:questbundle_s12_donut_w7', templates)
