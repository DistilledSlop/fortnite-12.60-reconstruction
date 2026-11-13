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
rotation, some bundle rewards and Maya customization remain unresolved.
"""
import copy
import math
from collections.abc import Mapping
from datetime import datetime

from season12_quest_data import (DONUT_BUNDLES, DONUT_QUESTS, DONUT_SCHEDULE,
                                 DONUT_SCHEDULE_CALENDAR_EVENT_TAG)

from season12_deadpool_reward_data import DONUT_REWARD_TYPES, DONUT_VARIANTS
from season12_maya_data import MAYA_BUNDLE, MAYA_QUESTS
from season12_daily_quest_data import DAILY_QUESTS
from season12_location_data import LOCATION_BUNDLES, LOCATION_QUESTS, LOCATION_SCHEDULE
from season12_weekly_quest_data import (WEEKLY_QUESTS, WEEKLY_BUNDLES, WEEKLY_SCHEDULE,
                                        WEEKLY_SCHEDULE_CALENDAR_EVENT_TAG)
from season12_brutus_faction_data import (QUESTS as BRUTUS_QUESTS, BUNDLES as BRUTUS_BUNDLES,
                                           GRANT_QUEST as BRUTUS_GRANT, TOKEN as BRUTUS_TOKEN,
                                           OBJECTIVE as BRUTUS_OBJECTIVE, SCHEDULE as BRUTUS_SCHEDULE)
from season12_tntina_faction_data import (QUESTS as TNTINA_QUESTS, BUNDLES as TNTINA_BUNDLES,
                                           GRANT_QUEST as TNTINA_GRANT, TOKEN as TNTINA_TOKEN,
                                           OBJECTIVE as TNTINA_OBJECTIVE, SCHEDULE as TNTINA_SCHEDULE)
from season12_remaining_faction_data import (QUESTS as REMAINING_FACTION_QUESTS,
                                            BUNDLES as REMAINING_FACTION_BUNDLES,
                                            FACTIONS as REMAINING_FACTIONS)
from season12_storm_agency_data import STORM_AGENCY_QUESTS, STORM_AGENCY_BUNDLES, STORM_AGENCY_SCHEDULE
from season12_oro_data import ORO_QUESTS, ORO_BUNDLES, ORO_SCHEDULE
from season12_superlevel_data import SUPERLEVEL_QUESTS, SUPERLEVEL_BUNDLES, SUPERLEVEL_SCHEDULE
from superlevel_service import SUPERLEVEL_VARIANTS, reconcile_level
from season12_faction_variant_data import FACTION_VARIANTS, AUTO_EQUIP_VARIANTS
from daily_quest_service import DailyService
from locker_variant_sync import sync_lockers

from api_common import ApiError, Err
from profile_service import CommandContext, _invalid
from banner_reward_service import convert_banner_tokens

EVENTS = {'accolade_assault_specialist': 'quest_s12_styles1_q01_accolade_ar',
          'donut_w1_startquest': 'Quest_S12_Donut_W1_A',
          'dont_thank_bus_driver': 'Quest_S12_Donut_W1_B'}
FACTION_BUNDLES = {**BRUTUS_BUNDLES, **TNTINA_BUNDLES, **REMAINING_FACTION_BUNDLES}
FACTION_GRANTS = {
    BRUTUS_GRANT: (BRUTUS_TOKEN, BRUTUS_OBJECTIVE, BRUTUS_SCHEDULE, 18),
    TNTINA_GRANT: (TNTINA_TOKEN, TNTINA_OBJECTIVE, TNTINA_SCHEDULE, 18),
    **{f['grant_quest']: (f['token'], f['objective'], f['schedule'], f['threshold'])
       for f in REMAINING_FACTIONS.values()},
}
LIMITED_EVENT_BUNDLES = {**STORM_AGENCY_BUNDLES, **ORO_BUNDLES}
LIMITED_EVENT_SCHEDULES = {("challengebundleschedule:" + schedule["name"]).casefold(): schedule
                           for schedule in (STORM_AGENCY_SCHEDULE, ORO_SCHEDULE)}
EVENT_QUESTS = {**DONUT_QUESTS, **MAYA_QUESTS, **DAILY_QUESTS, **WEEKLY_QUESTS, **LOCATION_QUESTS, **BRUTUS_QUESTS, **TNTINA_QUESTS, **REMAINING_FACTION_QUESTS, **STORM_AGENCY_QUESTS, **ORO_QUESTS, **SUPERLEVEL_QUESTS}
EVENT_BUNDLES = {**DONUT_BUNDLES, **WEEKLY_BUNDLES, **LOCATION_BUNDLES, **FACTION_BUNDLES, **LIMITED_EVENT_BUNDLES, **SUPERLEVEL_BUNDLES, 'QuestBundle_Styles_PaperDoll': MAYA_BUNDLE}
ACQUISITION_EVENTS = {'item_acquire_brutus_token', 'item_acquire_tntina_token'} | {
    f['acquisition_event'] for f in REMAINING_FACTIONS.values()}


def objective_event_key(objective, stat):
    # Inline objectives use a locally qualified identifier on the trusted bridge.
    return stat if stat is not None else 'objective:' + objective


# Level-item events are derived from committed season progression, never accepted
# as caller-supplied stat rows or inline-objective events.
VALID_EVENTS = (set(EVENTS) | {objective_event_key(objective, stat)
                             for name, quest in EVENT_QUESTS.items() if name not in SUPERLEVEL_QUESTS
                             for objective, stat, _ in quest['objectives']}) - ACQUISITION_EVENTS
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
            if tag not in ({DONUT_SCHEDULE_CALENDAR_EVENT_TAG} |
                           {schedule['calendar_event_tag'] for schedule in LIMITED_EVENT_SCHEDULES.values()}):
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

    def _quest(self, tx, name, bundle, active=True):
        name = next((key for key in EVENT_QUESTS if key.casefold() == name.casefold()), name)
        objectives = EVENT_QUESTS[name]['objectives']
        attrs = {'quest_state': 'Active' if active else 'Inactive', 'last_state_change_time': self.profiles.ctx.iso(),
                 'level': -1, 'item_seen': False, 'challenge_bundle_id': bundle}
        attrs.update({'completion_' + objective.lower(): 0 for objective, _, _ in objectives})
        quest_id = self._grant(tx, 'Quest:' + name, attrs)
        # Attach an existing unlinked root once its owning bundle is reconciled.
        if (bundle in tx.profile['items'] and
                tx.item(bundle)['templateId'].casefold() in {('challengebundle:' + key).casefold() for key in {**STORM_AGENCY_BUNDLES, **SUPERLEVEL_BUNDLES}} and
                name.casefold() in {q.casefold() for definition in {**STORM_AGENCY_BUNDLES, **SUPERLEVEL_BUNDLES}.values() for q, _, _, _ in definition['quest_infos']} and
                tx.item(quest_id)['attributes'].get('challenge_bundle_id') is None):
            tx.set_item_attr(quest_id, 'challenge_bundle_id', bundle)
        return quest_id

    def _bundle(self, tx, name, schedule_id=None):
        attrs = {'item_seen': False, 'num_quests_completed': 0,
                 'num_progress_quests_completed': 0}
        if schedule_id is not None:
            attrs['challenge_bundle_schedule_id'] = schedule_id
        bundle_id = self._grant(tx, 'ChallengeBundle:' + name, attrs)
        self._reconcile_bundle(tx, name, bundle_id)
        return bundle_id

    def _reconcile_bundle(self, tx, name, bundle_id):
        definition = EVENT_BUNDLES[name]
        for quest, grant_type, days, start_active in definition['quest_infos']:
            if (grant_type == 'GrantWithBundle' or
                    (grant_type == 'RequiresBattlePass' and
                     tx.profile['stats']['attributes'].get('book_purchased') is True and
                     tx.profile['stats']['attributes'].get('book_level', 0) >= days) or
                    (grant_type == 'DaysFromEventStart' and
                     self._date_reached(definition['calendar_event_tag'], days))):
                self._quest(tx, quest, bundle_id, start_active if name in FACTION_BUNDLES else True)
        if name in LOCATION_BUNDLES:
            # Repair a missing successor without replaying the claimed stage's
            # rewards. Restrict the scan to chains belonging to this bundle.
            for root, _, _, _ in definition['quest_infos']:
                current = root.casefold()
                while True:
                    successors = [reward.casefold() for kind, reward, _ in LOCATION_QUESTS[current]['rewards']
                                  if kind == 'Quest']
                    if not successors:
                        break
                    claimed = any(item['templateId'].casefold() == 'quest:' + current and
                                  item['attributes'].get('challenge_bundle_id') == bundle_id and
                                  item['attributes'].get('quest_state') == 'Claimed'
                                  for item in tx.profile['items'].values())
                    if claimed:
                        self._quest(tx, successors[0], bundle_id)
                    current = successors[0]
        self._sync_bundle_progress(tx, name, bundle_id)

    def _sync_bundle_progress(self, tx, name, bundle_id):
        # Original GetAchievedCount/GetPercentageComplete read this separate
        # attribute, not num_quests_completed. Single-stage bundles count
        # claimed QuestInfos; Location Domination uses terminal stages below.
        if name not in WEEKLY_BUNDLES and name not in LOCATION_BUNDLES and name not in LIMITED_EVENT_BUNDLES and name not in SUPERLEVEL_BUNDLES and name != 'QuestBundle_Styles_PaperDoll':
            return
        roots = {quest.casefold() for quest, _, _, _ in EVENT_BUNDLES[name]['quest_infos']}
        if name in LOCATION_BUNDLES:
            # Inferred local policy: UI counts completed terminal chains, while
            # the reward counter counts every claimed stage. Original server
            # increment and completion-reward comparison are not recovered.
            terminals = set()
            for root in roots:
                current = root
                while True:
                    next_quests = [reward.casefold() for kind, reward, _ in LOCATION_QUESTS[current]['rewards'] if kind == 'Quest']
                    if not next_quests:
                        terminals.add(current)
                        break
                    current = next_quests[0]
            roots = terminals
        completed = {item['templateId'].split(':', 1)[-1].casefold()
                     for item in tx.profile['items'].values()
                     if item['templateId'].casefold().startswith('quest:') and
                     item['attributes'].get('challenge_bundle_id') == bundle_id and
                     item['attributes'].get('quest_state') == 'Claimed'}
        tx.set_item_attr(bundle_id, 'num_progress_quests_completed', len(roots & completed))

    def login(self, ctx, *, include_daily=True):
        tx = ctx.tx('athena')
        if include_daily:
            self.daily.login(ctx)
        self.consume_variants(tx)
        # Explicit local opt-in: the original grant source and epoch are unknown.
        if tx.profile['stats']['attributes'].get('season_num') == 12:
            for schedule in LIMITED_EVENT_SCHEDULES.values():
                if self._date_reached(schedule['calendar_event_tag'], 0):
                    self._grant(tx, 'ChallengeBundleSchedule:' + schedule['name'], {})
        items = list(tx.profile['items'].items())
        for schedule_id, item in items:
            template = item['templateId'].casefold()
            if template == 'challengebundleschedule:bundleschedule_styles_paperdoll':
                self._bundle(tx, 'QuestBundle_Styles_PaperDoll', schedule_id)
            elif template == 'challengebundleschedule:season12_mission_schedule':
                for bundle, grant_type, days in WEEKLY_SCHEDULE + LOCATION_SCHEDULE:
                    if grant_type == 'OnScheduleGranted' or self._date_reached(WEEKLY_SCHEDULE_CALENDAR_EVENT_TAG, days):
                        self._bundle(tx, bundle, schedule_id)
            elif template == ('challengebundleschedule:' + SUPERLEVEL_SCHEDULE['name']).casefold():
                for bundle, grant_type, _ in SUPERLEVEL_SCHEDULE['entries']:
                    if grant_type == 'OnScheduleGranted':
                        self._bundle(tx, bundle, schedule_id)
            elif template in LIMITED_EVENT_SCHEDULES:
                for bundle, grant_type, _ in LIMITED_EVENT_SCHEDULES[template]['entries']:
                    if grant_type == 'OnScheduleGranted':
                        self._bundle(tx, bundle, schedule_id)
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
                if name in DONUT_BUNDLES:
                    self._award_donut_bundle(tx, name, bundle_id, repair=True)
                if name == 'QuestBundle_Styles_PaperDoll':
                    self._award_maya_bundle(tx, bundle_id)
        # Purchase/XP reward handling can call login recursively before the
        # current quest's token is awarded. Reconcile only on explicit login.
        if include_daily:
            self._reconcile_faction_entitlements(tx)
        reconcile_level(self, ctx)

    def _reconcile_faction_entitlements(self, tx):
        # Weekly bundle completion rewards were awarded before faction grant
        # quests existed. Reconstruct only from unambiguous retained claims;
        # token balances alone do not prove when or how they were acquired.
        items = list(tx.profile['items'].items())
        by_template = {}
        for item_id, item in items:
            by_template.setdefault(item['templateId'].casefold(), []).append((item_id, item))
        proven = {token.casefold(): 0 for token, _, _, _ in FACTION_GRANTS.values()}
        weekly_quests = {name.casefold(): quest for name, quest in WEEKLY_QUESTS.items()}
        for bundle_name, definition in WEEKLY_BUNDLES.items():
            bundles = by_template.get(('ChallengeBundle:' + bundle_name).casefold(), [])
            if len(bundles) != 1:
                continue
            bundle_id = bundles[0][0]
            claimed = 0
            for quest_name in {q.casefold() for q, _, _, _ in definition['quest_infos']}:
                matches = by_template.get('quest:' + quest_name, [])
                if len(matches) != 1:
                    continue
                attrs = matches[0][1].get('attributes', {})
                if attrs.get('quest_state') != 'Claimed' or attrs.get('challenge_bundle_id') != bundle_id:
                    continue
                quest = weekly_quests[quest_name]
                values = [attrs.get('completion_' + objective.lower())
                          for objective, _, _ in quest['objectives']]
                targets = [target for _, _, target in quest['objectives']]
                if any(type(value) is not int or not 0 <= value <= target
                       for value, target in zip(values, targets)):
                    continue
                if sum(value == target for value, target in zip(values, targets)) < quest.get('ObjectiveCompletionCount', len(targets)):
                    continue
                claimed += 1
            for threshold, rewards in definition['completion_rewards'].items():
                if threshold > claimed:
                    continue
                for kind, reward, quantity in rewards:
                    if kind == 'Token' and reward.casefold() in proven:
                        proven[reward.casefold()] += quantity

        for grant, (token, objective, _, threshold) in FACTION_GRANTS.items():
            grants = by_template.get(('Quest:' + grant).casefold(), [])
            if len(grants) != 1:
                continue
            item_id, item = grants[0]
            if item['attributes'].get('quest_state') != 'Active':
                continue
            key = 'completion_' + objective
            old = item['attributes'].get(key)
            if type(old) is not int or not 0 <= old <= threshold:
                raise _invalid('Invalid faction grant progress.')
            new = max(old, min(threshold, proven[token.casefold()]))
            if new != old:
                tx.set_item_attr(item_id, key, new)
            if new == threshold:
                self._award_completed(None, tx, item_id, item, grant)

    def consume_variants(self, tx):
        items = list(tx.profile['items'].items())
        character = next(((i, v) for i, v in items if v['templateId'].casefold() ==
                          'athenacharacter:cid_701_athena_commando_m_bananaagent'), None)
        if character:
            self._consume_peely_variants(tx, items, character)

        # Local policy: retain a token until its target is owned; apply to every
        # matching owned item, then consume it atomically with the unlock. The
        # original backend token-consumption timing is not recovered.
        items = list(tx.profile['items'].items())
        for token_id, token in items:
            spec = (FACTION_VARIANTS.get(token['templateId'].casefold()) or
                    DONUT_VARIANTS.get(token['templateId'].casefold()) or
                    SUPERLEVEL_VARIANTS.get(token['templateId'].casefold()))
            if spec is None:
                continue
            target, channel, unlocked, default = spec
            targets = [(i, v) for i, v in items if v['templateId'].casefold() == target.casefold()]
            if not targets:
                continue
            for item_id, item in targets:
                variants = copy.deepcopy(item['attributes'].get('variants', []))
                if not isinstance(variants, list):
                    raise _invalid('Invalid owned variant data.')
                entry = next((v for v in variants if isinstance(v, dict) and v.get('channel') == channel), None)
                if entry is None:
                    entry = {'channel': channel, 'active': default, 'owned': [default]}
                    variants.append(entry)
                if not isinstance(entry.get('owned'), list) or not isinstance(entry.get('active'), str):
                    raise _invalid('Invalid owned variant data.')
                if unlocked not in entry['owned']:
                    entry['owned'].append(unlocked)
                if token['templateId'].casefold() in AUTO_EQUIP_VARIANTS:
                    entry['active'] = unlocked
                tx.set_item_attr(item_id, 'variants', variants)
                if token['templateId'].casefold() in AUTO_EQUIP_VARIANTS:
                    sync_lockers(tx, item_id, item['templateId'], variants, channel=channel)
                if token['templateId'].casefold() not in DONUT_VARIANTS:
                    tx.set_item_attr(item_id, 'item_seen', False)
            tx.remove_item(token_id)

    def _consume_peely_variants(self, tx, items, character):
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

    def replay_progress(self, account_id, event, amount=1, *, event_id, match_id=None):
        """Return False for an exact receipt, None if absent; never create a profile."""
        self._validate_event(account_id, event, amount, event_id)
        payload = self._receipt_payload(event, amount, match_id)
        profile = self.profiles.existing_profile(account_id, 'athena')
        if profile is None:
            return None
        receipts = profile['stats']['attributes'].get('local_quest_event_receipts', {})
        if event_id not in receipts:
            return None
        if not self._receipt_matches(receipts[event_id], payload):
            raise _invalid('Quest event ID already used with different data.')
        return False

    @staticmethod
    def _receipt_matches(stored, payload):
        # A context-aware retry of a legacy receipt is already accounted for.
        # The trusted caller must bind its event ID to the actual match.
        return stored == payload or (len(payload) == 3 and stored == payload[:2])

    @staticmethod
    def _receipt_payload(event, amount, match_id):
        if match_id is not None and (not isinstance(match_id, str) or not match_id or len(match_id) > 200):
            raise _invalid('Invalid trusted match ID.')
        return [event, amount] if match_id is None else [event, amount, match_id]

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
        from quest_reward_notification import begin_claim_notification
        begin_claim_notification(ctx, quest_id)
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
            # unverified suppressed-quest semantics. For Location Domination this
            # also drives completion reward thresholds, an inferred local policy.
            tx.set_item_attr(bundle_id, 'num_quests_completed', count + 1)
        if bundle_id in tx.profile['items']:
            bundle_name = next((key for key in {**WEEKLY_BUNDLES, **LOCATION_BUNDLES, **LIMITED_EVENT_BUNDLES, **SUPERLEVEL_BUNDLES} if
                                tx.item(bundle_id)['templateId'].casefold() == ('ChallengeBundle:' + key).casefold()), None)
            if bundle_name:
                self._sync_bundle_progress(tx, bundle_name, bundle_id)
                definition = EVENT_BUNDLES[bundle_name]
                roots = {q.casefold() for q, _, _, _ in definition['quest_infos']}
                limited_event = bundle_name in LIMITED_EVENT_BUNDLES
                reward_count = (tx.item(bundle_id)['attributes']['num_progress_quests_completed']
                                if limited_event else count + 1)
                # An unrelated quest linked to the bundle cannot trigger its reward.
                if not limited_event or name.casefold() in roots:
                    rewards = list(rewards) + definition['completion_rewards'].get(reward_count, [])
                    # As with Maya, -1 means all roots under explicit local policy.
                    # Preserve the cooked sentinel in data instead of rewriting it.
                    if limited_event and reward_count == len(roots):
                        rewards += definition['completion_rewards'].get(-1, [])
        for kind, reward, quantity in rewards:
            if kind == 'AccountResource' and reward == 'athenaseasonalxp':
                from progression_service import award_profile_xp
                # Local Season 12 policy: seasonal quest XP advances both
                # season and book XP while retaining purchased book levels.
                award_profile_xp(ctx, quantity, quantity)
                continue
            if kind == 'Token':
                self._award_token(tx, reward, quantity)
                continue
            if quantity != 1:
                raise _invalid('Unsupported quest reward quantity.')
            if kind == 'Quest':
                self._quest(tx, reward, bundle_id)
            elif kind == 'ChallengeBundle' and reward in EVENT_BUNDLES:
                schedule_id = tx.item(bundle_id)['attributes'].get('challenge_bundle_schedule_id') if bundle_id in tx.profile['items'] else None
                if name in FACTION_GRANTS:
                    schedule = FACTION_GRANTS[name][2]
                    schedule_id = next((i for i, v in tx.profile['items'].items() if v['templateId'].casefold() == ('ChallengeBundleSchedule:' + schedule).casefold()), None)
                self._bundle(tx, reward, schedule_id)
            else:
                self._grant(tx, kind + ':' + reward, {'item_seen': False})
        if bundle_id in tx.profile['items']:
            bundle_name = next((key for key in DONUT_BUNDLES if
                tx.item(bundle_id)['templateId'].casefold() == ('ChallengeBundle:' + key).casefold()), None)
            if bundle_name:
                self._award_donut_bundle(tx, bundle_name, bundle_id, completed_now=name)
        self.consume_variants(tx)
        if bundle_id in tx.profile['items'] and tx.item(bundle_id)['templateId'].casefold() == 'challengebundle:questbundle_styles_paperdoll':
            self._sync_bundle_progress(tx, 'QuestBundle_Styles_PaperDoll', bundle_id)
            self._award_maya_bundle(tx, bundle_id)

    def _award_donut_bundle(self, tx, name, bundle_id, *, repair=False, completed_now=None):
        definition = DONUT_BUNDLES[name]
        # Local policy: each QuestInfo and its linked Quest rewards form one
        # challenge. Count it only when the complete chain is claimed. W2
        # suppresses both stages, so suppression alone cannot select a terminal.
        # Original server accounting remains unknown.
        claimed = 0
        completed_terminals = set()
        for root, _, _, _ in definition['quest_infos']:
            current = root
            visited = set()
            while True:
                key = current.casefold()
                if key in visited or current not in DONUT_QUESTS:
                    raise _invalid('Invalid Donut quest chain.')
                visited.add(key)
                matches = [item for item in tx.profile['items'].values()
                           if item['templateId'].casefold() == ('Quest:' + current).casefold()
                           and item['attributes'].get('challenge_bundle_id') == bundle_id]
                if len(matches) != 1 or matches[0]['attributes'].get('quest_state') != 'Claimed':
                    break
                attrs = matches[0]['attributes']
                quest_def = DONUT_QUESTS[current]
                values = [attrs.get('completion_' + objective.lower())
                          for objective, _, _ in quest_def['objectives']]
                targets = [target for _, _, target in quest_def['objectives']]
                if (not all(type(value) is int and 0 <= value <= target for value, target in zip(values, targets))
                        or sum(value == target for value, target in zip(values, targets)) <
                        quest_def.get('ObjectiveCompletionCount', len(targets))):
                    break
                successors = [reward for kind, reward, _ in quest_def['rewards'] if kind == 'Quest']
                if len(successors) > 1:
                    raise _invalid('Unsupported Donut quest chain.')
                if not successors:
                    claimed += 1
                    completed_terminals.add(key)
                    break
                current = successors[0]
        # Only a terminal completed in this transaction can earn fresh stacked
        # tokens. Older claims may already have received or spent them elsewhere.
        repair = repair or not completed_now or completed_now.casefold() not in completed_terminals
        for threshold, rewards in definition['completion_rewards']:
            if claimed < threshold:
                continue
            receipt = 'local_donut_reward_' + name.lower() + '_' + str(threshold)
            attrs = tx.profile['stats']['attributes']
            if receipt in attrs:
                if attrs[receipt] is not True:
                    raise _invalid('Invalid Donut bundle reward receipt.')
                continue
            for path, quantity, _visibility in rewards:
                spec = DONUT_REWARD_TYPES.get(path)
                if spec is None:
                    raise _invalid('Unsupported Donut bundle reward.')
                kind, reward = spec
                if kind == 'Token':
                    # Older claims predate this implementation; consumed token
                    # balances cannot establish whether an award occurred.
                    if not repair:
                        self._award_token(tx, reward, quantity)
                elif quantity == 1:
                    self._grant(tx, kind + ':' + reward, {'item_seen': False})
                else:
                    raise _invalid('Unsupported Donut bundle reward quantity.')
            tx.set_stat(receipt, True)
        if repair:
            self.consume_variants(tx)

    def _award_maya_bundle(self, tx, bundle_id):
        # A durable receipt prevents a later consumer of the token from causing
        # login repair to grant it again. Never infer completion from the counter.
        attrs = tx.profile['stats']['attributes']
        receipt = 'local_maya_bundle_reward_received'
        if receipt in attrs:
            if attrs[receipt] is not True:
                raise _invalid('Invalid Maya bundle reward receipt.')
            return
        items = tx.profile['items'].values()
        for name, _, _, _ in MAYA_BUNDLE['quest_infos']:
            matches = [item for item in items if
                       item['templateId'].casefold() == ('Quest:' + name).casefold() and
                       item['attributes'].get('challenge_bundle_id') == bundle_id]
            if len(matches) != 1 or matches[0]['attributes'].get('quest_state') != 'Claimed':
                return
            for objective, _, target in MAYA_QUESTS[name]['objectives']:
                value = matches[0]['attributes'].get('completion_' + objective.lower())
                if type(value) is not int or value != target:
                    return
        for kind, reward, quantity in MAYA_BUNDLE['completion_rewards'][-1]:
            if kind != 'Token':
                raise _invalid('Unsupported Maya bundle reward.')
            # A profile may have received the token before this receipt existed.
            if not any(item['templateId'].casefold() == ('Token:' + reward).casefold()
                       for item in tx.profile['items'].values()):
                self._award_token(tx, reward, quantity)
        tx.set_stat(receipt, True)

    def _award_token(self, tx, reward, quantity):
        if type(quantity) is not int or not 1 <= quantity < 2**31:
            raise _invalid('Invalid token quantity.')
        template = 'Token:' + reward
        found = next(((i, v) for i, v in tx.profile['items'].items()
                      if v['templateId'].casefold() == template.casefold()), None)
        if found:
            token_id, token = found
            count = token['quantity']
            if type(count) is not int or not 1 <= count <= 2**31 - 1 - quantity:
                raise _invalid('Invalid token quantity.')
            tx.set_item_quantity(token_id, count + quantity)
        else:
            item_id = self._grant(tx, template, {'item_seen': False})
            if quantity != 1:
                tx.set_item_quantity(item_id, quantity)

        for grant, (token, objective, _, _) in FACTION_GRANTS.items():
            if reward.casefold() == token.casefold():
                self._acquire_faction_token(tx, quantity, grant, objective)

    def _acquire_faction_token(self, tx, quantity, grant, objective):
        threshold = FACTION_GRANTS[grant][3]
        for item_id, item in list(tx.profile['items'].items()):
            if (item['templateId'].casefold() != ('Quest:' + grant).casefold()
                    or item['attributes'].get('quest_state') != 'Active'):
                continue
            key = 'completion_' + objective
            old = item['attributes'].get(key)
            if type(old) is not int or not 0 <= old <= threshold:
                raise _invalid('Invalid faction grant progress.')
            new = min(threshold, old + quantity)
            tx.set_item_attr(item_id, key, new)
            if new == threshold:
                self._award_completed(None, tx, item_id, item, grant)

    def progress(self, account_id, event, amount=1, *, event_id, match_id=None):
        """Trusted server-only event, with durable per-account replay protection.

        Event names are prequalified stat rows or local inline-objective keys;
        mapping actual gameplay to those keys is the trusted caller’s responsibility. Event IDs must be unique to a gameplay occurrence.
        """
        self._validate_event(account_id, event, amount, event_id)
        ctx = CommandContext(self.profiles, account_id, 'athena', {})
        tx = ctx.tx('athena')
        receipts = dict(tx.profile['stats']['attributes'].get('local_quest_event_receipts', {}))
        payload = self._receipt_payload(event, amount, match_id)
        if event_id in receipts:
            if not self._receipt_matches(receipts[event_id], payload):
                raise _invalid('Quest event ID already used with different data.')
            return False
        # Snapshot active quests before awarding anything: an event cannot advance
        # a quest unlocked by that same event, nor one unlocked in the future.
        active = list(tx.profile['items'].items())
        names = {('quest:' + name).casefold(): name for name in EVENT_QUESTS}
        for item_id, item in active:
            name = names.get(item['templateId'].casefold())
            if name is None or name in SUPERLEVEL_QUESTS or item['attributes'].get('quest_state') != 'Active':
                continue
            definition = EVENT_QUESTS[name]
            objectives = definition['objectives']
            # Missing cooked property: local policy requires all objectives.
            required = definition.get('ObjectiveCompletionCount', len(objectives))
            if not any(objective_event_key(objective, stat) == event for objective, stat, _ in objectives):
                continue
            once = definition.get('bAthenaUpdateObjectiveOncePerMatch', False)
            single = definition.get('bAthenaMustCompleteInSingleMatch', False)
            if (once or single) and match_id is None:
                continue
            # Local policy: once-per-match is per objective. For single-match
            # quests, each unseen match supersedes the current one; late events
            # from previously seen matches cannot reset or advance progress.
            state = copy.deepcopy(item['attributes'].get('local_quest_match_state', {}))
            if (once or single) and not isinstance(state, dict):
                raise _invalid('Invalid quest match state.')
            if once:
                seen = state.setdefault('objective_matches', {})
                if (not isinstance(seen, dict) or any(
                        not isinstance(key, str) or not isinstance(matches, list) or
                        any(not isinstance(value, str) or not value or len(value) > 200 for value in matches)
                        for key, matches in seen.items())):
                    raise _invalid('Invalid quest match state.')
            if single:
                seen_matches = state.setdefault('seen_matches', [])
                current = state.get('current_match')
                if (not isinstance(seen_matches, list) or any(
                        not isinstance(value, str) or not value or len(value) > 200 for value in seen_matches)
                        or (current is not None and (not isinstance(current, str) or current not in seen_matches))):
                    raise _invalid('Invalid quest match state.')
                if match_id != current:
                    if match_id in seen_matches:
                        continue
                    seen_matches.append(match_id)
                    state['current_match'] = match_id
                    for objective, _, _ in objectives:
                        tx.set_item_attr(item_id, 'completion_' + objective.lower(), 0)
            completed = 0
            for objective, stat, target in objectives:
                key = 'completion_' + objective.lower()
                old = item['attributes'].get(key)
                if isinstance(old, bool) or not isinstance(old, int) or not 0 <= old <= target:
                    raise _invalid('Invalid quest progress.')
                if single and state['current_match'] == match_id:
                    old = tx.item(item_id)['attributes'].get(key)
                increment = amount
                if once and objective_event_key(objective, stat) == event:
                    matches = seen.setdefault(objective.lower(), [])
                    increment = 0 if match_id in matches else 1
                    if increment:
                        matches.append(match_id)
                new = min(target, old + increment) if objective_event_key(objective, stat) == event else old
                if new != old:
                    tx.set_item_attr(item_id, key, new)
                completed += new == target
            if once or single:
                tx.set_item_attr(item_id, 'local_quest_match_state', state)
            if completed < required:
                continue
            self._award_completed(ctx, tx, item_id, item, name)
        receipts[event_id] = payload
        tx.set_stat('local_quest_event_receipts', receipts)
        convert_banner_tokens(ctx)
        self.profiles._store(account_id, ctx.txs)
        return True
