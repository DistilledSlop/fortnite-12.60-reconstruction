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
Only OnScheduleGranted/GrantWithBundle are implemented; calendar, suppressed
bundle completion, quest chains beyond W1 and daily rotation remain unresolved.
"""
import copy

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
TOKENS = {'cosmeticvarianttoken:vtid_560_bananaagent_styleb': 'Stage2',
          'cosmeticvarianttoken:vtid_561_bananaagent_stylec': 'Stage3'}


class QuestService:
    def __init__(self, profiles):
        self.profiles = profiles

    def _grant(self, tx, template, attrs):
        for item_id, item in tx.profile['items'].items():
            if item['templateId'].casefold() == template.casefold():
                return item_id
        item_id = self.profiles._new_item_id('item', tx.profile)
        tx.add_item(item_id, template, attrs)
        return item_id

    def _quest(self, tx, name, bundle):
        objective, _, _ = QUESTS[name]
        return self._grant(tx, 'Quest:' + name, {'quest_state': 'Active', 'last_state_change_time': self.profiles.ctx.iso(),
                            'level': -1, 'item_seen': False, 'challenge_bundle_id': bundle,
                            'completion_' + objective.lower(): 0})

    def login(self, ctx):
        tx = ctx.tx('athena')
        # Reconcile on every login, including same-day purchase or migrated profiles.
        self.consume_variants(tx)
        items = list(tx.profile['items'].items())
        for schedule_id, item in items:
            bundle = SCHEDULES.get(item['templateId'].casefold())
            if bundle is None:
                continue
            bundle_name, quest_name = bundle
            bundle_id = self._grant(tx, 'ChallengeBundle:' + bundle_name,
                                    {'item_seen': False, 'challenge_bundle_schedule_id': schedule_id,
                                     'num_quests_completed': 0})
            self._quest(tx, quest_name, bundle_id)

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

    def progress(self, account_id, event, amount=1, *, event_id):
        """Trusted server-only event, with durable per-account replay protection.

        Event names are objective-stat row names; mapping to actual gameplay is
        unconfirmed. Event IDs must be unique to a gameplay occurrence.
        """
        if self.profiles.accounts.get(account_id) is None:
            raise ApiError(404, Err.PROFILE_ACCOUNT_NOT_FOUND, 'The account was not found.')
        if not isinstance(event, str) or event not in EVENTS or isinstance(amount, bool) or not isinstance(amount, int) or not 1 <= amount <= 1000:
            raise _invalid('Invalid quest event.')
        if not isinstance(event_id, str) or not event_id or len(event_id) > 200:
            raise _invalid('Invalid quest event ID.')
        ctx = CommandContext(self.profiles, account_id, 'athena', {})
        tx = ctx.tx('athena')
        receipts = dict(tx.profile['stats']['attributes'].get('local_quest_event_receipts', {}))
        payload = [event, amount]
        if event_id in receipts:
            if receipts[event_id] != payload:
                raise _invalid('Quest event ID already used with different data.')
            return False
        name = EVENTS[event]
        quest = next(((i, q) for i, q in tx.profile['items'].items() if q['templateId'].casefold() == ('Quest:' + name).casefold()), None)
        if quest and quest[1]['attributes'].get('quest_state') == 'Active':
            item_id, item = quest
            objective, target, reward = QUESTS[name]
            key = 'completion_' + objective.lower()
            old = item['attributes'].get(key)
            if isinstance(old, bool) or not isinstance(old, int) or not 0 <= old <= target:
                raise _invalid('Invalid quest progress.')
            new = min(target, old + amount)
            tx.set_item_attr(item_id, key, new)
            if new == target:
                tx.set_item_attr(item_id, 'quest_state', 'Claimed')
                tx.set_item_attr(item_id, 'last_state_change_time', self.profiles.ctx.iso())
                bundle = item['attributes'].get('challenge_bundle_id')
                if reward.startswith('Quest:'):
                    self._quest(tx, reward.split(':', 1)[1], bundle)
                else:
                    self._grant(tx, reward, {'item_seen': False})
        receipts[event_id] = payload
        tx.set_stat('local_quest_event_receipts', receipts)
        self.profiles._store(account_id, ctx.txs)
        return True
