"""Daily quest rotation: LOCAL POLICY, inferred, not captured.

One grant per UTC day, at most three active quests, and one reroll per day.
There is no catch-up. Missing cooked Weight is locally treated as one; zero
is excluded. Claimed instances remain in the profile; recurring templates get
new instance IDs. Selection is deterministic for an account and UTC day.
The caller owns the command transaction and commits only on success.
"""
import hashlib
from dataclasses import dataclass
from datetime import date, datetime, timezone

from profile_service import _invalid
from season12_daily_quest_data import DAILY_QUESTS, daily_quest_name


@dataclass(frozen=True)
class DailyPolicy:
    max_active: int = 3
    candidates: tuple | None = None


class DailyService:
    def __init__(self, profiles, policy=None):
        self.profiles = profiles
        self.policy = policy or DailyPolicy()
        if not isinstance(self.policy, DailyPolicy) or type(self.policy.max_active) is not int or self.policy.max_active < 1:
            raise ValueError('Invalid daily policy.')
        names = self.policy.candidates if self.policy.candidates is not None else tuple(DAILY_QUESTS)
        if not isinstance(names, (tuple, list)) or any(name not in DAILY_QUESTS for name in names):
            raise ValueError('Invalid daily candidates.')
        self.candidates = tuple(sorted({name for name in names if DAILY_QUESTS[name].get('Weight', 1) > 0}))

    def _day(self):
        return datetime.fromtimestamp(self.profiles.ctx.now(), timezone.utc).date().isoformat()

    def _manager(self, tx, day):
        stats = tx.profile['stats']['attributes']
        manager = stats.get('quest_manager')
        if not isinstance(manager, dict):
            raise _invalid('Invalid quest manager.')
        manager = dict(manager)
        reroll_day = stats.get('local_daily_reroll_day', '')
        grant_day = stats.get('local_daily_grant_day', '')
        for value in (reroll_day, grant_day):
            if not isinstance(value, str):
                raise _invalid('Invalid daily state.')
            if value:
                try:
                    valid = date.fromisoformat(value).isoformat() == value
                except ValueError:
                    valid = False
                if not valid:
                    raise _invalid('Invalid daily state.')
        remaining = manager.get('dailyQuestRerolls', 0)
        if type(remaining) is not int or remaining not in (0, 1):
            raise _invalid('Invalid daily reroll count.')
        if not reroll_day or day > reroll_day:
            manager['dailyQuestRerolls'] = 1
            tx.set_stat('local_daily_reroll_day', day)
            tx.set_stat('quest_manager', manager)
        return manager, grant_day

    def _active(self, tx):
        return [(item_id, daily_quest_name(item.get('templateId'))) for item_id, item in tx.profile['items'].items()
                if daily_quest_name(item.get('templateId')) and item.get('attributes', {}).get('quest_state') == 'Active']

    def _choose(self, account, day, purpose, excluded):
        choices = [name for name in self.candidates if name not in excluded]
        if not choices:
            return None
        digest = hashlib.sha256(('daily\0' + account + '\0' + day + '\0' + purpose).encode()).digest()
        return choices[int.from_bytes(digest, 'big') % len(choices)]

    def _grant(self, tx, name):
        attrs = {'quest_state': 'Active', 'last_state_change_time': self.profiles.ctx.iso(),
                 'level': -1, 'item_seen': False}
        attrs.update({'completion_' + objective.lower(): 0 for objective, _, _ in DAILY_QUESTS[name]['objectives']})
        item_id = self.profiles._new_item_id('item', tx.profile)
        tx.add_item(item_id, 'Quest:' + name, attrs)
        return item_id

    def login(self, ctx):
        tx = ctx.tx('athena')
        day = self._day()
        _, grant_day = self._manager(tx, day)
        if grant_day and day <= grant_day:
            return
        active = self._active(tx)
        if len(active) < self.policy.max_active:
            name = self._choose(ctx.account_id, day, 'grant', {name for _, name in active})
            if name is not None:
                self._grant(tx, name)
        tx.set_stat('local_daily_grant_day', day)

    def reroll(self, ctx, quest_id):
        tx = ctx.tx('athena')
        if not isinstance(quest_id, str) or not quest_id:
            raise _invalid('Invalid daily quest ID.')
        active = self._active(tx)
        old = next((name for item_id, name in active if item_id == quest_id), None)
        if old is None:
            raise _invalid('Daily quest must be owned and active.')
        day = self._day()
        manager, _ = self._manager(tx, day)
        if manager.get('dailyQuestRerolls') != 1:
            raise _invalid('No daily rerolls remain.')
        name = self._choose(ctx.account_id, day, 'reroll', {name for _, name in active})
        if name is None:
            raise _invalid('No eligible daily quest remains.')
        tx.remove_item(quest_id)
        item_id = self._grant(tx, name)
        manager['dailyQuestRerolls'] = 0
        tx.set_stat('quest_manager', manager)
        return item_id
