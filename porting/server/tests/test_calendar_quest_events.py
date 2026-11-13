"""Calendar event flags agree with configured quest activation."""
if __package__:
    from . import _bootstrap
else:
    import sys as _sys
    from pathlib import Path as _Path
    _sys.path.insert(0, str(_Path(__file__).resolve().parent))
    import _bootstrap

import unittest

from api_common import LOCAL_ACCOUNT_ID
from backend import Backend
from season12_storm_agency_data import STORM_AGENCY_BUNDLES

SEASON = 'EventFlag.LobbySeason12'
STORM = 'EventFlag.LTE_StormTheAgency'
FAR = '2099-01-01T00:00:00.000Z'
START = 1700000000


class CalendarQuestEventsTests(unittest.TestCase):
    def setUp(self):
        self.now = START

    def backend(self, starts=None):
        backend = Backend(clock=lambda: self.now, quest_event_starts=starts)
        self.addCleanup(backend.close)
        return backend

    def calendar(self, backend):
        status, calendar = backend.content.timeline(None, None)
        self.assertEqual(status, 200)
        return calendar['channels']['client-events']

    def has_storm_bundle(self, backend):
        backend.profiles.run(LOCAL_ACCOUNT_ID, 'athena', 'ClientQuestLogin', {})
        items = backend.profiles.profile(LOCAL_ACCOUNT_ID, 'athena')['items'].values()
        bundle = 'ChallengeBundle:' + next(iter(STORM_AGENCY_BUNDLES))
        return any(item['templateId'].casefold() == bundle.casefold() for item in items)

    def test_unconfigured_flags_and_grant_remain_absent(self):
        backend = self.backend()
        events = self.calendar(backend)
        self.assertEqual(events['states'][0]['activeEvents'], [])
        self.assertEqual(events['cacheExpire'], FAR)
        self.assertEqual(events['states'][0]['state']['seasonBegin'], '2020-06-02T00:00:00Z')
        self.assertFalse(self.has_storm_bundle(backend))

    def test_future_boundary_and_quest_grant(self):
        backend = self.backend({STORM: START + 10, SEASON: START + 20})
        events = self.calendar(backend)
        self.assertEqual(events['states'][0]['activeEvents'], [])
        self.assertEqual(events['cacheExpire'], backend.ctx.iso(START + 10))
        self.assertEqual(events['states'][0]['state']['seasonBegin'], backend.ctx.iso(START + 20))
        self.assertFalse(self.has_storm_bundle(backend))
        self.now += 10
        events = self.calendar(backend)
        self.assertEqual(events['states'][0]['activeEvents'], [
            {'eventType': STORM, 'activeSince': backend.ctx.iso(START + 10), 'activeUntil': FAR}])
        self.assertEqual(events['cacheExpire'], backend.ctx.iso(START + 20))
        self.assertTrue(self.has_storm_bundle(backend))
        self.now += 10
        self.assertEqual(self.calendar(backend)['cacheExpire'], FAR)

    def test_iso_and_unix_starts_are_normalized_and_sorted(self):
        backend = self.backend({SEASON: '2023-11-14T17:13:20-05:00', STORM: START})
        events = self.calendar(backend)
        self.assertEqual(events['states'][0]['activeEvents'], [
            {'eventType': tag, 'activeSince': backend.ctx.iso(START), 'activeUntil': FAR}
            for tag in (STORM, SEASON)])
        self.assertEqual(events['states'][0]['state']['seasonBegin'], backend.ctx.iso(START))
        self.assertTrue(self.has_storm_bundle(backend))


if __name__ == '__main__':
    unittest.main()
