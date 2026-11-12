"""Single dispatcher for the local Epic service stand-ins. Python standard library only.

    python3 -I backend.py --routes      route table with evidence labels (for the README)
    python3 -I backend.py --commands    MCP command table with evidence labels

The dispatcher resolves method and path, runs the handler under one lock, turns
ApiError into the Epic error body, and records one line per request in memory:
method, path, route name and status. It never records headers, bodies, query values,
tokens or session keys.

Services (each module registers its own routes and keeps its own state):
  oauth_service      token, exchange, verify, session kill
  account_service    account lookup, tryPlayOnPlatform, profile token verify
  profile_service    MCP profile store and command set
  party_service      party v1 and presence
  content_service    lightswitch, version check, storefront, calendar, datarouter
  cloudstorage_service  system files and optional persistent user files
  friends_service    persistent requests, friends and blocks
  xmpp_service       authenticated social notification and presence transport
  frontend_stubs     social ban, receipts, billing and other guess answers
  chat_service       game chat room reservation (empty room lists)
  matchmaking        ticket, session and session-key routes, and the websocket
"""

import itertools
import os
import sys
import threading
import traceback
from collections import deque

# Run with `python3 -I`, which leaves the script directory off sys.path. Add it explicitly.
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import profile_service  # noqa: E402
from account_service import AccountService
from api_common import DEFAULT_SEED, ApiError, Context, Err, Request, Router, error_body, log_safe_path
from chat_service import ChatService
from cloudstorage_service import CloudStorageService
from content_service import ContentService
from frontend_stubs import FrontendStubs
from friends_service import FriendsService
from gameserver_service import GameServerService
from gameplay_service import GameplayService
from gifting_service import GiftingService
from matchmaking import MatchmakingService
from oauth_service import OAuthService
from party_service import PartyService
from profile_service import ProfileService
from progression_service import ProgressionService
from quest_service import QuestService
from xmpp_service import XmppService

INTERNAL_ERROR = "errors.com.epicgames.common.internal_error"  # guess: not in the binary strings


class Backend:
    def __init__(self, *, clock=None, seed=DEFAULT_SEED, cloudstorage_dir=None,
                 state_dir=None, state_step_sec=None, log_size=1000, matchmaking_ws_origin=None,
                 gameserver_secret=None, registered_matchmaking=None,
                 content_dir=None, content_file=None, content_public_origin=None):
        self.ctx = Context(clock=clock, seed=seed)
        self.accounts = AccountService(self.ctx, state_dir=state_dir)
        self.oauth = OAuthService(self.ctx, self.accounts)
        self.profiles = ProfileService(self.ctx, self.accounts, self.oauth, state_dir=state_dir)
        self.profiles.quests = QuestService(self.profiles)
        self.party = PartyService(self.ctx, self.accounts, self.oauth)
        self.friends = FriendsService(self.ctx, self.accounts, self.oauth, state_dir=state_dir)
        self.xmpp = XmppService(self.ctx, self.accounts, self.oauth, self.friends, self.party)
        self.friends.notify = self.xmpp.notify
        self.party.notify = self.xmpp.notify
        self.profiles.gifting = GiftingService(self.ctx, self.accounts, self.profiles, self.friends,
                                              notify=self.xmpp.notify)
        self.progression = ProgressionService(self.ctx, self.accounts, self.profiles, state_dir=state_dir)
        self.gameservers = GameServerService(self.ctx, self.accounts, self.profiles, self.party,
                                            shared_secret=gameserver_secret)
        self.gameplay = GameplayService(self.ctx, self.accounts, self.profiles, self.oauth,
                                        self.party, self.gameservers, self.progression)
        self.content = ContentService(self.ctx, content_dir=content_dir, content_file=content_file,
                                      public_origin=content_public_origin)
        self.cloud = CloudStorageService(self.ctx, self.oauth, cloudstorage_dir, state_dir=state_dir)
        self.frontend = FrontendStubs(self.ctx)
        self.chat = ChatService(self.ctx, self.oauth)
        if registered_matchmaking is None:
            registered_matchmaking = bool(gameserver_secret or os.environ.get("FORTNITE_GAMESERVER_SECRET"))
        matchmaking_options = {"websocket_origin": matchmaking_ws_origin,
                               "gameservers": self.gameservers if registered_matchmaking else None}
        if state_step_sec is not None:
            matchmaking_options["state_step_sec"] = state_step_sec
        self.matchmaking = MatchmakingService(self.ctx, self.accounts, **matchmaking_options)
        self.router = Router()
        for service in (self.oauth, self.accounts, self.profiles, self.party, self.friends, self.content,
                        self.cloud, self.frontend, self.chat, self.matchmaking, self.gameservers,
                        self.gameplay):
            service.register(self.router)
        self.request_log = deque(maxlen=log_size)
        self._sequence = itertools.count(1)
        self._lock = threading.RLock()

    def close(self):
        if getattr(self, "_closed", False):
            return
        self._closed = True
        self.xmpp.close()
        self.friends.close()

    def respond(self, method, path, query=None, form=None, body=b"", headers=None, query_all=None):
        """Answer one request. Returns (status, payload). Payload is JSON-able, None, RawText or Raw."""
        req = Request(method, path, query=query, query_all=query_all, form=form, body=body, headers=headers)
        route, match, allowed = self.router.resolve(req.method, path)
        route_name = None
        if route is None:
            if allowed:
                status, payload = 405, error_body(Err.METHOD_NOT_ALLOWED, "The method is not allowed for this resource.")
            else:
                status, payload = 404, error_body(Err.NOT_FOUND, f"Sorry, the resource {path} was not found", 1004)
        else:
            route_name = route.name
            try:
                with self._lock:
                    status, payload = route.handler(req, match)
            except ApiError as error:
                status, payload = error.status, error.body()
            except Exception:
                traceback.print_exc(file=sys.stderr)
                status, payload = 500, error_body(INTERNAL_ERROR, "The local service failed to answer this request.")
        self._record(req.method, path, route_name, status)
        return status, payload

    def websocket_route(self, path, query):
        serve = self.matchmaking.websocket_route(path, query)
        if serve is not None:
            return serve
        xmpp_serve = self.xmpp.websocket_route(path, query)
        if xmpp_serve is None:
            return None
        def serve_xmpp(conn):
            return xmpp_serve(conn)
        serve_xmpp.subprotocol = "xmpp"
        return serve_xmpp

    def _record(self, method, path, route_name, status):
        with self._lock:
            self.request_log.append({
                "method": method,
                "path": log_safe_path(path),
                "route": route_name,
                "status": status,
                "index": next(self._sequence),
            })

    def recent_requests(self):
        with self._lock:
            return [dict(entry) for entry in self.request_log]

    def route_table(self):
        return [{"method": route.method, "pattern": route.regex.pattern, "name": route.name,
                 "status": route.status} for route in self.router.routes]


def _print_routes(backend):
    print("| Method | Path pattern | Route | Evidence |")
    print("| --- | --- | --- | --- |")
    for row in backend.route_table():
        pattern = row["pattern"].replace("|", "\\|")
        print(f"| {row['method']} | `{pattern}` | {row['name']} | {row['status']} |")


def _print_commands():
    print("| Command | Profiles | Name evidence | Body |")
    print("| --- | --- | --- | --- |")
    print("| QueryProfile | any | observed | none |")
    for name in sorted(profile_service.COMMANDS):
        cmd = profile_service.COMMANDS[name]
        profiles = ", ".join(sorted(cmd.profiles))
        print(f"| {name} | {profiles} | {cmd.name_status} | {cmd.body} |")
    print("| GiftCatalogEntry | common_core | observed | offerId, receiverAccountIds, giftWrapTemplateId, userMessageToRecipients, currency, currencySubType, expectedTotalPrice (inferred). |")
    print("| RemoveGiftBox | common_core | observed | giftBoxItemIds (reflected; JSON casing inferred). |")
    for name in profile_service.UNIMPLEMENTED:
        print(f"| {name} | - | observed | not implemented (operation_not_found) |")


if __name__ == "__main__":
    if "--commands" in sys.argv:
        _print_commands()
    else:
        _print_routes(Backend())
