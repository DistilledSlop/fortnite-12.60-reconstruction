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
  frontend_stubs     friends, social ban, receipts, billing and other guess answers
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
from api_common import DEFAULT_SEED, ApiError, Context, Err, Request, Router, error_body
from chat_service import ChatService
from cloudstorage_service import CloudStorageService
from content_service import ContentService
from frontend_stubs import FrontendStubs
from matchmaking import MatchmakingService
from oauth_service import OAuthService
from party_service import PartyService
from profile_service import ProfileService

INTERNAL_ERROR = "errors.com.epicgames.common.internal_error"  # guess: not in the binary strings


class Backend:
    def __init__(self, *, clock=None, seed=DEFAULT_SEED, cloudstorage_dir=None,
                 state_dir=None, state_step_sec=None, log_size=1000):
        self.ctx = Context(clock=clock, seed=seed)
        self.accounts = AccountService(self.ctx, state_dir=state_dir)
        self.oauth = OAuthService(self.ctx, self.accounts)
        self.profiles = ProfileService(self.ctx, self.accounts, self.oauth, state_dir=state_dir)
        self.party = PartyService(self.ctx, self.accounts, self.oauth)
        self.content = ContentService(self.ctx)
        self.cloud = CloudStorageService(self.ctx, self.oauth, cloudstorage_dir, state_dir=state_dir)
        self.frontend = FrontendStubs(self.ctx)
        self.chat = ChatService(self.ctx, self.oauth)
        if state_step_sec is None:
            self.matchmaking = MatchmakingService(self.ctx, self.accounts)
        else:
            self.matchmaking = MatchmakingService(self.ctx, self.accounts, state_step_sec=state_step_sec)
        self.router = Router()
        for service in (self.oauth, self.accounts, self.profiles, self.party, self.content,
                        self.cloud, self.frontend, self.chat, self.matchmaking):
            service.register(self.router)
        self.request_log = deque(maxlen=log_size)
        self._sequence = itertools.count(1)
        self._lock = threading.RLock()

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
        return self.matchmaking.websocket_route(path, query)

    def _record(self, method, path, route_name, status):
        with self._lock:
            self.request_log.append({
                "method": method,
                "path": path,
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
    for name in profile_service.UNIMPLEMENTED:
        print(f"| {name} | - | observed | not implemented (operation_not_found) |")


if __name__ == "__main__":
    if "--commands" in sys.argv:
        _print_commands()
    else:
        _print_routes(Backend())
