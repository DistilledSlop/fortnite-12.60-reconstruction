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
to all objectives locally; suppressed bundle rewards and daily rotation remain unresolved.
"""
import copy
import math
from collections.abc import Mapping
from datetime import datetime

from season12_quest_data import (DONUT_BUNDLES, DONUT_QUESTS, DONUT_SCHEDULE,
                                 DONUT_SCHEDULE_CALENDAR_EVENT_TAG)

from api_common import ApiError, Err
from profile_service import CommandContext, _invalid

SCHEDULES = {
    'challengebundleschedule:bundleschedule_styles_paperdoll': ('QuestBundle_Styles_PaperDoll', 'quest_s12_styles1_q01_accolade_ar'),
    'challengebundleschedule:season12_donut_schedule': ('QuestBundle_S12_Donut_W1', 'Quest_S12_Donut_W1_A'),
}
QUESTS = {
    'quest_s12_styles1_q01_accolade_ar': ('quest_s12_styles1_q01_accolade_ar_obj', 5, 'Token:AthenaToken_BRS12_HeroSelection_01'),
    'Quest_S12_Donut_W1_A': ('Quest_S12_Donut_W1_A', 1, 'Quest:Quest_S12_Donut_W1_B'),
    'Quest_S12_Donut_W1_B': ('Quest_S12_Donut_W1_B', 1, 'ChallengeBundle:QuestBundle_S12_Donut_W2'),
}
EVENTS = {'accolade_assault_specialist': 'quest_s12_styles1_q01_accolade_ar',
          'donut_w1_startquest': 'Quest_S12_Donut_W1_A',
          'dont_thank_bus_driver': 'Quest_S12_Donut_W1_B'}
VALID_EVENTS = set(EVENTS) | {stat for quest in DONUT_QUESTS.values() for _, stat, _ in quest['objectives']}
TOKENS = {'cosmeticvarianttoken:vtid_560_bananaagent_styleb': 'Stage2',
          'cosmeticvarianttoken:vtid_561_bananaagent_stylec': 'Stage3'}


class QuestService:
    def __init__(self, profiles, event_starts=None):
        self.profiles = profiles
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
        objectives = DONUT_QUESTS[name]['objectives'] if name in DONUT_QUESTS else [(QUESTS[name][0], '', QUESTS[name][1])]
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
        definition = DONUT_BUNDLES[name]
        for quest, grant_type, days, _ in definition['quest_infos']:
            if grant_type == 'GrantWithBundle' or (grant_type == 'DaysFromEventStart' and
                    self._date_reached(definition['calendar_event_tag'], days)):
                self._quest(tx, quest, bundle_id)

    def login(self, ctx):
        tx = ctx.tx('athena')
        self.consume_variants(tx)
        items = list(tx.profile['items'].items())
        for schedule_id, item in items:
            template = item['templateId'].casefold()
            if template == 'challengebundleschedule:bundleschedule_styles_paperdoll':
                bundle_id = self._grant(tx, 'ChallengeBundle:QuestBundle_Styles_PaperDoll',
                    {'item_seen': False, 'challenge_bundle_schedule_id': schedule_id, 'num_quests_completed': 0})
                self._quest(tx, 'quest_s12_styles1_q01_accolade_ar', bundle_id)
            elif template == 'challengebundleschedule:season12_donut_schedule':
                for bundle, grant_type, days in DONUT_SCHEDULE:
                    if grant_type == 'OnScheduleGranted' or (grant_type == 'DaysSinceEventStart' and
                            self._date_reached(DONUT_SCHEDULE_CALENDAR_EVENT_TAG, days)):
                        self._bundle(tx, bundle, schedule_id)
        # Manual reward bundles may have been granted before the date gate opened.
        names = {('challengebundle:' + name).casefold(): name for name in DONUT_BUNDLES}
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
        names = {('quest:' + name).casefold(): name for name in DONUT_QUESTS}
        names['quest:quest_s12_styles1_q01_accolade_ar'] = 'quest_s12_styles1_q01_accolade_ar'
        for item_id, item in active:
            name = names.get(item['templateId'].casefold())
            if name is None or item['attributes'].get('quest_state') != 'Active':
                continue
            if name in DONUT_QUESTS:
                definition = DONUT_QUESTS[name]
                objectives = definition['objectives']
                rewards = definition['rewards']
                # Missing cooked property: local policy requires all objectives.
                required = definition.get('ObjectiveCompletionCount', len(objectives))
            else:
                objective, target, reward = QUESTS[name]
                objectives = [(objective, 'accolade_assault_specialist', target)]
                rewards = [(reward.split(':', 1)[0], reward.split(':', 1)[1], 1)]
                required = 1
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
            tx.set_item_attr(item_id, 'quest_state', 'Claimed')
            tx.set_item_attr(item_id, 'last_state_change_time', self.profiles.ctx.iso())
            bundle_id = item['attributes'].get('challenge_bundle_id')
            if name in DONUT_QUESTS and bundle_id in tx.profile['items']:
                bundle = tx.item(bundle_id)
                count = bundle['attributes'].get('num_quests_completed', 0)
                if isinstance(count, bool) or not isinstance(count, int) or count < 0:
                    raise _invalid('Invalid bundle progress.')
                # Local counter policy: count claimed quest items, without applying
                # unverified suppressed-quest or completion-reward semantics.
                tx.set_item_attr(bundle_id, 'num_quests_completed', count + 1)
            for kind, reward, quantity in rewards:
                if quantity != 1:
                    raise _invalid('Unsupported quest reward quantity.')
                if kind == 'Quest':
                    self._quest(tx, reward, bundle_id)
                elif kind == 'ChallengeBundle' and reward in DONUT_BUNDLES:
                    schedule_id = tx.item(bundle_id)['attributes'].get('challenge_bundle_schedule_id') if bundle_id in tx.profile['items'] else None
                    self._bundle(tx, reward, schedule_id)
                else:
                    self._grant(tx, kind + ':' + reward, {'item_seen': False})
        receipts[event_id] = payload
        tx.set_stat('local_quest_event_receipts', receipts)
        self.profiles._store(account_id, ctx.txs)
        return True
