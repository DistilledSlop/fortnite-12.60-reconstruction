"""Evidence-limited Season 12 quest and variant handling.

Cooked uasset SHA256 (Content/Athena/Items; property-report via Cue4Inspect):
ChallengeBundleSchedules/Styles/BundleSchedule_Styles_PaperDoll d188ff9f6ef3fd0ba2a60723e3867d5152a3165e9e7b2b66c9befbf5d3b79df3
ChallengeBundleSchedules/Season12/Season12_Donut_Schedule 597f23e4f3d3626115d699d3187268acb99c8f50b2ba123529e692bc7667dc17
ChallengeBundles/Styles/QuestBundle_Styles_PaperDoll dc9bf8d450b0b7bf40345f4defbe8d495c80eba651d990186209a21299f1be50
ChallengeBundles/Season12/Donut/QuestBundle_S12_Donut_W1 b6852f08b4944abcd4af42399124adbccaa6f5062f9a7ddd78b5181e9927e2c2
Quests/Styles/Season12/quest_s12_styles1_q01_accolade_ar 9b6341c5be3cc6ac8a48b42feb8cbcdd121f60829f859d25f5df211a078eb6e2
Quests/BattlePass/Season12/Donut/Quest_S12_Donut_W1_A a875b8f0784ae776fbd782714d701bef53d622508c65dab45e650cf3baa423f6
Quests/BattlePass/Season12/Donut/Quest_S12_Donut_W1_B c8a47924fb178b2f7096fd8cd42ea346bb826db227ea5f10e5075efca8158c07
CosmeticVariantTokens/VTID_560_BananaAgent_StyleB 7370c42216ba899410101b5b4a8081ff511cf2152823a2716976b972e8f5c47b
CosmeticVariantTokens/VTID_561_BananaAgent_StyleC cbb6da7d9566ccea63164c9bee7db0311cc7e025a487ee90a62e554686c4af84
Cosmetics/Characters/CID_701_Athena_Commando_M_BananaAgent 4a50739682819c09986dd3a4dd39550cf18ba13f78745b3d2f91532c10c004c7

The profile item attributes and trusted event mapping are inferred, not captured.
Calendar starts require explicit configuration. Missing objective completion defaults
to all objectives locally. Daily rotation is an explicit local policy; historical
rotation, bundle rewards and Maya customization remain unresolved.
"""
import copy
import math
from collections.abc import Mapping
from datetime import datetime

from season12_quest_data import (DONUT_BUNDLES, DONUT_QUESTS, DONUT_SCHEDULE,
                                 DONUT_SCHEDULE_CALENDAR_EVENT_TAG)

from season12_maya_data import MAYA_BUNDLE, MAYA_QUESTS
from season12_daily_quest_data import DAILY_QUESTS
from daily_quest_service import DailyService

from api_common import ApiError, Err
from profile_service import CommandContext, _invalid

EVENTS = {'accolade_assault_specialist': 'quest_s12_styles1_q01_accolade_ar',
          'donut_w1_startquest': 'Quest_S12_Donut_W1_A',
          'dont_thank_bus_driver': 'Quest_S12_Donut_W1_B'}
EVENT_QUESTS = {**DONUT_QUESTS, **MAYA_QUESTS, **DAILY_QUESTS}
EVENT_BUNDLES = {**DONUT_BUNDLES, 'QuestBundle_Styles_PaperDoll': MAYA_BUNDLE}
VALID_EVENTS = set(EVENTS) | {stat for quest in EVENT_QUESTS.values() for _, stat, _ in quest['objectives']}
TOKENS = {'cosmeticvarianttoken:vtid_560_bananaagent_styleb': 'Stage2',
          'cosmeticvarianttoken:vtid_561_bananaagent_stylec': 'Stage3'}


class QuestService:
    def __init__(self, profiles, event_starts=None):
        self.profiles = profiles
        self.daily = DailyService(profiles)
        if event_starts is None:
            event_starts = {}
        if not isinstance(event_starts, Mapping):
            raise ValueError('Event starts must be a mapping.')
        self.event_starts = {}
        for tag, start in event_starts.items():
            if tag != DONUT_SCHEDULE_CALENDAR_EVENT_TAG:
                raise ValueError('Unsupported event start tag.')
            if isinstance(start, str):
                try:
                    start = datetime.fromisoformat(start.replace('Z', '+00:00'))
                except ValueError as exc:
                    raise ValueError('Invalid event start timestamp.') from exc
            if isinstance(start, datetime):
                if start.tzinfo is None or start.utcoffset() is None:
                    raise ValueError('Event start must include a timezone.')
                try:
                    start = start.timestamp()
                except (OverflowError, OSError, ValueError) as exc:
                    raise ValueError('Invalid event start timestamp.') from exc
            try:
                valid = not isinstance(start, bool) and isinstance(start, (int, float)) and math.isfinite(start)
            except OverflowError:
                valid = False
            if not valid:
                raise ValueError('Event start must be a finite Unix timestamp or timezone-aware ISO timestamp.')
            self.event_starts[tag] = start

    def _date_reached(self, tag, days):
        start = self.event_starts.get(tag)
        if start is None:
            return False
        return self.profiles.ctx.now() >= start + days * 86400

    def _grant(self, tx, template, attrs):
        for item_id, item in tx.profile['items'].items():
            if item['templateId'].casefold() == template.casefold():
                return item_id
        item_id = self.profiles._new_item_id('item', tx.profile)
        tx.add_item(item_id, template, attrs)
        return item_id

    def _quest(self, tx, name, bundle):
        objectives = EVENT_QUESTS[name]['objectives']
        attrs = {'quest_state': 'Active', 'last_state_change_time': self.profiles.ctx.iso(),
                 'level': -1, 'item_seen': False, 'challenge_bundle_id': bundle}
        attrs.update({'completion_' + objective.lower(): 0 for objective, _, _ in objectives})
        return self._grant(tx, 'Quest:' + name, attrs)

    def _bundle(self, tx, name, schedule_id=None):
        attrs = {'item_seen': False, 'num_quests_completed': 0}
        if schedule_id is not None:
            attrs['challenge_bundle_schedule_id'] = schedule_id
        bundle_id = self._grant(tx, 'ChallengeBundle:' + name, attrs)
        self._reconcile_bundle(tx, name, bundle_id)
        return bundle_id

    def _reconcile_bundle(self, tx, name, bundle_id):
        definition = EVENT_BUNDLES[name]
        for quest, grant_type, days, _ in definition['quest_infos']:
            if grant_type == 'GrantWithBundle' or (grant_type == 'DaysFromEventStart' and
                    self._date_reached(definition['calendar_event_tag'], days)):
                self._quest(tx, quest, bundle_id)

    def login(self, ctx, *, include_daily=True):
        tx = ctx.tx('athena')
        if include_daily:
            self.daily.login(ctx)
        self.consume_variants(tx)
        items = list(tx.profile['items'].items())
        for schedule_id, item in items:
            template = item['templateId'].casefold()
            if template == 'challengebundleschedule:bundleschedule_styles_paperdoll':
                self._bundle(tx, 'QuestBundle_Styles_PaperDoll', schedule_id)
            elif template == 'challengebundleschedule:season12_donut_schedule':
                for bundle, grant_type, days in DONUT_SCHEDULE:
                    if grant_type == 'OnScheduleGranted' or (grant_type == 'DaysSinceEventStart' and
                            self._date_reached(DONUT_SCHEDULE_CALENDAR_EVENT_TAG, days)):
                        self._bundle(tx, bundle, schedule_id)
        # Manual reward bundles may have been granted before the date gate opened.
        names = {('challengebundle:' + name).casefold(): name for name in EVENT_BUNDLES}
        for bundle_id, item in list(tx.profile['items'].items()):
            name = names.get(item['templateId'].casefold())
            if name:
                self._reconcile_bundle(tx, name, bundle_id)

    def consume_variants(self, tx):
        items = list(tx.profile['items'].items())
        character = next(((i, v) for i, v in items if v['templateId'].casefold() ==
                          'athenacharacter:cid_701_athena_commando_m_bananaagent'), None)
        if not character:
            return
        character_id, character_item = character
        variants = copy.deepcopy(character_item['attributes'].get('variants', []))
        for token_id, token in items:
            stage = TOKENS.get(token['templateId'].casefold())
            if stage is None:
                continue
            channel = 'Progressive'
            entry = next((v for v in variants if isinstance(v, dict) and v.get('channel') == channel), None)
            if entry is None:
                entry = {'channel': channel, 'active': 'Stage1', 'owned': ['Stage1']}
                variants.append(entry)
            if not isinstance(entry.get('owned'), list) or not isinstance(entry.get('active'), str):
                raise _invalid('Invalid owned variant data.')
            if stage not in entry['owned']:
                entry['owned'].append(stage)
            tx.remove_item(token_id)
        tx.set_item_attr(character_id, 'variants', variants)

    def _validate_event(self, account_id, event, amount, event_id):
        if not isinstance(account_id, str) or not account_id or self.profiles.accounts.get(account_id) is None:
            raise ApiError(404, Err.PROFILE_ACCOUNT_NOT_FOUND, 'The account was not found.')
        if not isinstance(event, str) or event not in VALID_EVENTS or isinstance(amount, bool) or not isinstance(amount, int) or not 1 <= amount <= 10000:
            raise _invalid('Invalid quest event.')
        if not isinstance(event_id, str) or not event_id or len(event_id) > 200:
            raise _invalid('Invalid quest event ID.')

    def replay_progress(self, account_id, event, amount=1, *, event_id):
        """Return False for an exact receipt, None if absent; never create a profile."""
        self._validate_event(account_id, event, amount, event_id)
        profile = self.profiles.existing_profile(account_id, 'athena')
        if profile is None:
            return None
        receipts = profile['stats']['attributes'].get('local_quest_event_receipts', {})
        if event_id not in receipts:
            return None
        if receipts[event_id] != [event, amount]:
            raise _invalid('Quest event ID already used with different data.')
        return False

    def claim(self, ctx, quest_id, selected_index):
        """Local claim policy for supported fixed-reward quests; retries never award twice.

        The original caller sends -1 when no selectable rewards exist. Missing
        indices and selectable reward quests are unsupported, not defaulted.
        """
        if type(selected_index) is not int or selected_index != -1:
            raise _invalid('Only fixed quest rewards with selectedRewardIndex -1 are supported.')
        tx = ctx.tx('athena')
        item = tx.item(quest_id)
        name = next((name for name in EVENT_QUESTS
                     if item['templateId'].casefold() == ('quest:' + name).casefold()), None)
        if name is None:
            raise _invalid('Unsupported quest.')
        state = item['attributes'].get('quest_state')
        if state == 'Claimed':
            return
        if state != 'Completed':
            raise _invalid('Quest must be completed before claiming rewards.')
        definition = EVENT_QUESTS[name]
        completed = 0
        for objective, _, target in definition['objectives']:
            value = item['attributes'].get('completion_' + objective.lower())
            if type(value) is not int or not 0 <= value <= target:
                raise _invalid('Invalid quest progress.')
            completed += value == target
        if completed < definition.get('ObjectiveCompletionCount', len(definition['objectives'])):
            raise _invalid('Quest objectives are incomplete.')
        self._award_completed(ctx, tx, item_id=quest_id, item=item, name=name)

    def _award_completed(self, ctx, tx, item_id, item, name):
        rewards = EVENT_QUESTS[name]['rewards']
        tx.set_item_attr(item_id, 'quest_state', 'Claimed')
        tx.set_item_attr(item_id, 'last_state_change_time', self.profiles.ctx.iso())
        bundle_id = item['attributes'].get('challenge_bundle_id')
        if name in EVENT_QUESTS and bundle_id in tx.profile['items']:
            bundle = tx.item(bundle_id)
            count = bundle['attributes'].get('num_quests_completed', 0)
            if isinstance(count, bool) or not isinstance(count, int) or count < 0:
                raise _invalid('Invalid bundle progress.')
            # Local counter policy: count claimed quest items, without applying
            # unverified suppressed-quest or completion-reward semantics.
            tx.set_item_attr(bundle_id, 'num_quests_completed', count + 1)
        for kind, reward, quantity in rewards:
            if kind == 'AccountResource' and reward == 'athenaseasonalxp':
                from progression_service import award_profile_xp
                # Local Season 12 policy: seasonal quest XP advances both
                # season and book XP while retaining purchased book levels.
                award_profile_xp(ctx, quantity, quantity)
                continue
            if quantity != 1:
                raise _invalid('Unsupported quest reward quantity.')
            if kind == 'Quest':
                self._quest(tx, reward, bundle_id)
            elif kind == 'ChallengeBundle' and reward in EVENT_BUNDLES:
                schedule_id = tx.item(bundle_id)['attributes'].get('challenge_bundle_schedule_id') if bundle_id in tx.profile['items'] else None
                self._bundle(tx, reward, schedule_id)
            elif name in DAILY_QUESTS and kind == 'Token':
                template = kind + ':' + reward
                found = next(((i, v) for i, v in tx.profile['items'].items()
                              if v['templateId'].casefold() == template.casefold()), None)
                if found:
                    token_id, token = found
                    count = token['quantity']
                    if type(count) is not int or not 1 <= count < 2**31 - 1:
                        raise _invalid('Invalid daily token quantity.')
                    tx.set_item_quantity(token_id, count + quantity)
                else:
                    self._grant(tx, template, {'item_seen': False})
            else:
                self._grant(tx, kind + ':' + reward, {'item_seen': False})

    def progress(self, account_id, event, amount=1, *, event_id):
        """Trusted server-only event, with durable per-account replay protection.

        Event names are objective-stat row names; mapping to actual gameplay is
        unconfirmed. Event IDs must be unique to a gameplay occurrence.
        """
        self._validate_event(account_id, event, amount, event_id)
        ctx = CommandContext(self.profiles, account_id, 'athena', {})
        tx = ctx.tx('athena')
        receipts = dict(tx.profile['stats']['attributes'].get('local_quest_event_receipts', {}))
        payload = [event, amount]
        if event_id in receipts:
            if receipts[event_id] != payload:
                raise _invalid('Quest event ID already used with different data.')
            return False
        # Snapshot active quests before awarding anything: an event cannot advance
        # a quest unlocked by that same event, nor one unlocked in the future.
        active = list(tx.profile['items'].items())
        names = {('quest:' + name).casefold(): name for name in EVENT_QUESTS}
        for item_id, item in active:
            name = names.get(item['templateId'].casefold())
            if name is None or item['attributes'].get('quest_state') != 'Active':
                continue
            definition = EVENT_QUESTS[name]
            objectives = definition['objectives']
            # Missing cooked property: local policy requires all objectives.
            required = definition.get('ObjectiveCompletionCount', len(objectives))
            if not any(stat == event for _, stat, _ in objectives):
                continue
            completed = 0
            for objective, stat, target in objectives:
                key = 'completion_' + objective.lower()
                old = item['attributes'].get(key)
                if isinstance(old, bool) or not isinstance(old, int) or not 0 <= old <= target:
                    raise _invalid('Invalid quest progress.')
                new = min(target, old + amount) if stat == event else old
                if new != old:
                    tx.set_item_attr(item_id, key, new)
                completed += new == target
            if completed < required:
                continue
            self._award_completed(ctx, tx, item_id, item, name)
        receipts[event_id] = payload
        tx.set_stat('local_quest_event_receipts', receipts)
        self.profiles._store(account_id, ctx.txs)
        return True
