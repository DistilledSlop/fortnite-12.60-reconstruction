#!/usr/bin/env python3
"""Self-contained tests for the local Epic service stand-ins.

Runs every service over real HTTP on 127.0.0.1 at a random port, using the same handler
as probe_server.py. Standard library only. No sudo, no root-only ports, no network
namespaces, no subprocess. Files it writes stay under porting/generated (ignored by Git).

    python3 -I server/test_services.py          # or: python3 -I -m unittest -v server/test_services.py

Each test starts a fresh backend with a fixed clock and removes its listener afterwards.
Tokens are random per run and are never printed. The leak test checks that the
request log file contains none of them.
"""

import base64
import json
import shutil
import sys
import threading
import unittest
import urllib.error
import urllib.parse
import urllib.request
from http.server import ThreadingHTTPServer
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))

import api_common  # noqa: E402
import matchmaking  # noqa: E402
import schema_check  # noqa: E402
import probe_server  # noqa: E402
import smoke_matchmaking as ws  # noqa: E402  (websocket client helpers)
from api_common import Err  # noqa: E402
from backend import Backend  # noqa: E402

GENERATED = HERE.parent / "generated"
RECORD = GENERATED / "test-services-requests.jsonl"
CLOUD_DIR = GENERATED / "test-services-cloudstorage"
T0 = 1_767_225_600.0  # 2026-01-01T00:00:00Z
DAY = 86400
LOCAL = "00000000000000000000000000000001"
DEAD_ITEM = "deadbeef" * 4


class FakeClock:
    def __init__(self, start):
        self.now = start

    def __call__(self):
        return self.now


def call(base, method, path, *, token=None, body=None, form=None, headers=None):
    """Return (status, response headers, raw body bytes)."""
    hdrs = dict(headers or {})
    data = None
    if token:
        hdrs["Authorization"] = "bearer " + token
    if form is not None:
        data = urllib.parse.urlencode(form).encode("utf-8")
        hdrs["Content-Type"] = "application/x-www-form-urlencoded"
    elif body is not None:
        data = body if isinstance(body, bytes) else json.dumps(body).encode("utf-8")
        hdrs.setdefault("Content-Type", "application/json")
    request = urllib.request.Request(base + path, data=data, method=method, headers=hdrs)
    try:
        with urllib.request.urlopen(request, timeout=10) as response:
            return response.status, response.headers, response.read()
    except urllib.error.HTTPError as error:
        try:
            return error.code, error.headers, error.read()
        finally:
            error.close()


def jcall(base, method, path, **kwargs):
    """Return (status, parsed JSON or None, raw bytes)."""
    status, headers, raw = call(base, method, path, **kwargs)
    parsed = json.loads(raw) if raw and "json" in headers.get("Content-Type", "") else None
    return status, parsed, raw


class LiveServer:
    """The backend behind probe_server's handler, on a random localhost port."""

    def __init__(self, backend):
        handler = probe_server.make_handler([], RECORD, backend)
        self.httpd = ThreadingHTTPServer(("127.0.0.1", 0), handler)
        self.port = self.httpd.server_address[1]
        self.base = f"http://127.0.0.1:{self.port}"
        self.thread = threading.Thread(target=self.httpd.serve_forever, kwargs={"poll_interval": 0.05},
                                       daemon=True)

    def start(self):
        self.thread.start()
        return self

    def stop(self):
        self.httpd.shutdown()
        self.httpd.server_close()
        self.thread.join(timeout=5)


class ServiceTestCase(unittest.TestCase):
    cloud_files = None  # set by subclasses that need cloud storage files

    def setUp(self):
        GENERATED.mkdir(parents=True, exist_ok=True)
        if RECORD.exists():
            RECORD.unlink()
        self.clock = FakeClock(T0)
        self.issued_tokens = []
        self.start_backend(Backend(clock=self.clock, state_step_sec=0,
                                   cloudstorage_dir=self._cloud_dir()))

    def _cloud_dir(self):
        if self.cloud_files is None:
            return None
        if CLOUD_DIR.exists():
            shutil.rmtree(CLOUD_DIR)
        CLOUD_DIR.mkdir(parents=True)
        for name, data in self.cloud_files.items():
            (CLOUD_DIR / name).write_bytes(data)
        self.addCleanup(shutil.rmtree, CLOUD_DIR, True)
        return CLOUD_DIR

    def start_backend(self, backend):
        if getattr(self, "server", None) is not None:
            self.server.stop()
        self.backend = backend
        self.server = LiveServer(backend).start()
        self.base = self.server.base
        self.addCleanup(self.server.stop)

    def remember(self, body):
        """Keep the access token for the leak test. Returns the body unchanged."""
        if body and body.get("access_token"):
            self.issued_tokens.append(body["access_token"])
        return body

    def sign_in(self, code="local-test-code"):
        status, body, _ = jcall(self.base, "POST", "/account/api/oauth/token",
                                form={"grant_type": "exchange_code", "exchange_code": code})
        self.assertEqual(status, 200)
        return self.remember(body)

    def sign_in_account(self, account_id):
        return self.remember(self.backend.oauth.local_sign_in(account_id))

    def new_account(self, name):
        return self.backend.accounts.create(name)["id"]

    def assertError(self, result, status, code):
        http_status, body, _ = result
        self.assertEqual(http_status, status, body)
        self.assertEqual(body["errorCode"], code, body)


# Dispatcher and request log -------------------------------------------------

class DispatcherTests(ServiceTestCase):
    def test_unknown_route_is_404_with_error_body(self):
        status, body, _ = jcall(self.base, "GET", "/no/such/route")
        self.assertEqual(status, 404)
        self.assertEqual(body["errorCode"], Err.NOT_FOUND)

    def test_wrong_method_on_known_path_is_405(self):
        status, body, _ = jcall(self.base, "GET", "/account/api/oauth/token")
        self.assertEqual(status, 405)
        self.assertEqual(body["errorCode"], Err.METHOD_NOT_ALLOWED)

    def test_health_still_answers(self):
        status, body, _ = jcall(self.base, "GET", "/__health")
        self.assertEqual((status, body), (200, {"status": "ready"}))

    def test_request_log_records_route_and_status_without_secrets(self):
        token = self.sign_in()["access_token"]
        jcall(self.base, "GET", f"/account/api/public/account/{LOCAL}", token=token)
        entries = self.backend.recent_requests()
        names = [entry["route"] for entry in entries]
        self.assertIn("oauth.token", names)
        self.assertIn("account.by_id", names)
        self.assertTrue(all(set(entry) == {"method", "path", "route", "status", "index"} for entry in entries))
        self.assertEqual([entry["index"] for entry in entries], sorted(entry["index"] for entry in entries))

    def test_probe_record_file_holds_no_tokens_or_auth_headers(self):
        token = self.sign_in()["access_token"]
        status, _, _ = call(self.base, "POST", f"/fortnite/api/game/v2/profile/{LOCAL}/client/QueryProfile"
                            "?profileId=athena&rvn=-1", token=token, body={})
        self.assertEqual(status, 200)
        text = RECORD.read_text(encoding="utf-8")
        self.assertTrue(text.strip(), "the probe should have recorded requests")
        for secret in self.issued_tokens:
            self.assertNotIn(secret, text)
        self.assertNotIn("authorization", text.lower())

    def test_same_seed_and_clock_give_same_answers(self):
        def answers(backend):
            server = LiveServer(backend).start()
            try:
                token = self.sign_in_on(server.base)
                _, body, _ = jcall(server.base, "POST",
                                   f"/fortnite/api/game/v2/profile/{LOCAL}/client/QueryProfile?profileId=athena",
                                   token=token, body={})
                return body
            finally:
                server.stop()
        first = answers(Backend(clock=FakeClock(T0), state_step_sec=0))
        second = answers(Backend(clock=FakeClock(T0), state_step_sec=0))
        self.assertEqual(first, second)

    def sign_in_on(self, base):
        status, body, _ = jcall(base, "POST", "/account/api/oauth/token",
                                form={"grant_type": "exchange_code", "exchange_code": "x"})
        self.assertEqual(status, 200)
        return body["access_token"]

    def test_route_names_are_unique_and_labelled(self):
        table = self.backend.route_table()
        names = [row["name"] for row in table]
        self.assertEqual(len(names), len(set(names)), "route names must be unique")
        self.assertTrue(all(row["status"] in ("observed", "inferred", "guess") for row in table))


# OAuth ----------------------------------------------------------------------

class OAuthTests(ServiceTestCase):
    def test_client_credentials_token_has_no_account(self):
        status, body, _ = jcall(self.base, "POST", "/account/api/oauth/token",
                                form={"grant_type": "client_credentials"})
        self.assertEqual(status, 200)
        self.remember(body)
        self.assertEqual(body["token_type"], "bearer")
        self.assertEqual(body["expires_in"], 28800)
        self.assertEqual(body["client_service"], "fortnite")
        self.assertNotIn("account_id", body)
        self.assertTrue(body["access_token"].startswith("local-"))

    def test_exchange_code_signs_in_the_local_account(self):
        body = self.sign_in()
        self.assertEqual(body["account_id"], LOCAL)
        self.assertEqual(body["displayName"], "LocalPlayer")
        self.assertEqual(body["in_app_id"], LOCAL)
        self.assertIn("refresh_token", body)

    def test_verify_accepts_a_live_token_and_rejects_bad_ones(self):
        token = self.sign_in()["access_token"]
        status, body, _ = jcall(self.base, "GET", "/account/api/oauth/verify", token=token)
        self.assertEqual(status, 200)
        self.assertEqual(body["account_id"], LOCAL)
        self.assertEqual(body["expires_in"], 28800)
        self.assertNotIn("access_token", body)
        self.assertError(jcall(self.base, "GET", "/account/api/oauth/verify", token="local-bogus"),
                         401, Err.TOKEN_INVALID)
        self.assertError(jcall(self.base, "GET", "/account/api/oauth/verify"), 401, Err.AUTH_REQUIRED)

    def test_bad_grants_are_rejected_with_oauth_codes(self):
        self.assertError(jcall(self.base, "POST", "/account/api/oauth/token", form={}), 400,
                         Err.OAUTH_INVALID_REQUEST)
        self.assertError(jcall(self.base, "POST", "/account/api/oauth/token",
                               form={"grant_type": "exchange_code"}), 400, Err.OAUTH_INVALID_REQUEST)
        self.assertError(jcall(self.base, "POST", "/account/api/oauth/token",
                               form={"grant_type": "device_auth"}), 400, Err.OAUTH_UNSUPPORTED_GRANT)

    def test_refresh_rotates_tokens_and_retires_the_old_ones(self):
        first = self.sign_in()
        status, second, _ = jcall(self.base, "POST", "/account/api/oauth/token",
                                  form={"grant_type": "refresh_token", "refresh_token": first["refresh_token"]})
        self.assertEqual(status, 200)
        self.remember(second)
        self.assertEqual(second["account_id"], LOCAL)
        self.assertNotEqual(second["access_token"], first["access_token"])
        self.assertError(jcall(self.base, "GET", "/account/api/oauth/verify", token=first["access_token"]),
                         401, Err.TOKEN_INVALID)
        self.assertError(jcall(self.base, "POST", "/account/api/oauth/token",
                               form={"grant_type": "refresh_token", "refresh_token": first["refresh_token"]}),
                         400, Err.REFRESH_INVALID)

    def test_exchange_code_from_the_exchange_route_is_single_use_and_maps_to_its_account(self):
        other = self.new_account("Second")
        token = self.sign_in_account(other)["access_token"]
        status, issued, _ = jcall(self.base, "GET", "/account/api/oauth/exchange", token=token)
        self.assertEqual(status, 200)
        self.assertEqual(issued["expiresInSeconds"], 300)
        status, body, _ = jcall(self.base, "POST", "/account/api/oauth/token",
                                form={"grant_type": "exchange_code", "exchange_code": issued["code"]})
        self.assertEqual(body["account_id"], other)
        self.remember(body)
        # Policy: a used code no longer maps to its account, so it falls back to the local account.
        status, again, _ = jcall(self.base, "POST", "/account/api/oauth/token",
                                 form={"grant_type": "exchange_code", "exchange_code": issued["code"]})
        self.assertEqual(again["account_id"], LOCAL)
        self.remember(again)

    def test_kill_all_ends_the_callers_sessions(self):
        token = self.sign_in()["access_token"]
        status, _, _ = call(self.base, "DELETE", "/account/api/oauth/sessions/kill", token=token)
        self.assertEqual(status, 204)
        self.assertError(jcall(self.base, "GET", "/account/api/oauth/verify", token=token), 401, Err.TOKEN_INVALID)

    def test_kill_one_removes_own_session_and_refuses_another_accounts(self):
        mine = self.sign_in()["access_token"]
        other_account = self.new_account("Stranger")
        theirs = self.sign_in_account(other_account)["access_token"]
        status, _, _ = call(self.base, "DELETE", f"/account/api/oauth/sessions/kill/{theirs}", token=mine)
        self.assertEqual(status, 403)
        status, body, _ = jcall(self.base, "GET", "/account/api/oauth/verify", token=theirs)
        self.assertEqual(status, 200, "the other account's session must survive")
        status, _, _ = call(self.base, "DELETE", f"/account/api/oauth/sessions/kill/{mine}", token=mine)
        self.assertEqual(status, 204)
        self.assertError(jcall(self.base, "GET", "/account/api/oauth/verify", token=mine), 401, Err.TOKEN_INVALID)


# Accounts and game-account endpoints ---------------------------------------

class AccountTests(ServiceTestCase):
    def test_account_by_id_and_unknown_id(self):
        token = self.sign_in()["access_token"]
        status, body, _ = jcall(self.base, "GET", f"/account/api/public/account/{LOCAL}", token=token)
        self.assertEqual(status, 200)
        self.assertEqual(body["id"], LOCAL)
        self.assertEqual(body["displayName"], "LocalPlayer")
        self.assertEqual(body["minorStatus"], "NOT_MINOR")
        self.assertError(jcall(self.base, "GET", "/account/api/public/account/" + "f" * 32, token=token),
                         404, Err.ACCOUNT_NOT_FOUND)

    def test_accounts_by_ids_returns_known_accounts_only(self):
        friend = self.new_account("Friend")
        query = urllib.parse.urlencode([("accountId", LOCAL), ("accountId", friend), ("accountId", "f" * 32)])
        status, body, _ = jcall(self.base, "GET", f"/account/api/public/account?{query}",
                                token=self.sign_in()["access_token"])
        self.assertEqual(status, 200)
        self.assertEqual([account["id"] for account in body], [LOCAL, friend])

    def test_accounts_by_ids_needs_an_id(self):
        self.assertError(jcall(self.base, "GET", "/account/api/public/account",
                               token=self.sign_in()["access_token"]), 400, Err.INVALID_ACCOUNT_COUNT)

    def test_display_name_lookup(self):
        friend = self.new_account("Bee")
        token = self.sign_in()["access_token"]
        status, body, _ = jcall(self.base, "GET", "/account/api/public/account/displayName/bee", token=token)
        self.assertEqual((status, body["id"]), (200, friend))
        self.assertError(jcall(self.base, "GET", "/account/api/public/account/displayName/Nobody", token=token),
                         404, Err.ACCOUNT_NOT_FOUND)

    def test_external_auths_user_setting_and_play_on_platform(self):
        token = self.sign_in()["access_token"]
        status, body, _ = jcall(self.base, "GET", f"/account/api/public/account/{LOCAL}/externalAuths", token=token)
        self.assertEqual((status, body), (200, []))
        status, body, _ = jcall(self.base, "POST", "/api/v1/user/setting", token=token, body=[])
        self.assertEqual((status, body), (200, []))
        status, headers, raw = call(self.base, "POST",
                                    f"/fortnite/api/game/v2/tryPlayOnPlatform/account/{LOCAL}?platform=Windows",
                                    token=token, body={})
        self.assertEqual((status, raw), (200, b"true"))
        self.assertTrue(headers["Content-Type"].startswith("text/plain"))

    def test_profile_token_verify_needs_a_known_account(self):
        token = self.sign_in()["access_token"]
        status, _, _ = call(self.base, "POST", f"/fortnite/api/game/v2/profileToken/verify/{LOCAL}", token=token)
        self.assertEqual(status, 204)
        self.assertError(jcall(self.base, "POST", "/fortnite/api/game/v2/profileToken/verify/" + "e" * 32,
                               token=token), 404, Err.ACCOUNT_NOT_FOUND)


# Content, lightswitch and version -----------------------------------------

class ContentTests(ServiceTestCase):
    def test_bulk_lightswitch_defaults_to_fortnite_up(self):
        status, body, _ = jcall(self.base, "GET", "/lightswitch/api/service/bulk/status?serviceId=fortnite")
        self.assertEqual(status, 200)
        self.assertEqual(body[0]["serviceInstanceId"], "fortnite")
        self.assertEqual(body[0]["status"], "UP")
        self.assertEqual(body[0]["allowedActions"], ["PLAY", "DOWNLOAD"])

    def test_maintenance_is_visible_on_both_lightswitch_routes(self):
        self.backend.content.set_service_status("fortnite", "MAINTENANCE", "Down for patching")
        _, body, _ = jcall(self.base, "GET", "/lightswitch/api/service/bulk/status?serviceId=fortnite")
        self.assertEqual((body[0]["status"], body[0]["message"]), ("MAINTENANCE", "Down for patching"))
        status, single, _ = jcall(self.base, "GET", "/lightswitch/api/service/fortnite/status")
        self.assertEqual((status, single["status"]), (200, "MAINTENANCE"))

    def test_version_check_and_enabled_features(self):
        status, body, _ = jcall(self.base, "GET", "/fortnite/api/v2/versioncheck/Windows?version=1")
        self.assertEqual((status, body), (200, {"type": "NO_UPDATE"}))
        status, body, _ = jcall(self.base, "GET", "/fortnite/api/game/v2/enabled_features")
        self.assertEqual((status, body), (200, []))

    def test_storefront_calendar_keychain_and_datarouter(self):
        status, body, _ = jcall(self.base, "GET", "/fortnite/api/calendar/v1/timeline")
        self.assertIn("channels", body)
        self.assertEqual(jcall(self.base, "GET", "/fortnite/api/storefront/v2/catalog")[0], 200)
        status, body, _ = jcall(self.base, "GET", "/fortnite/api/storefront/v2/keychain?numKeysDownloaded=0")
        self.assertEqual(len(body), 1)
        self.assertEqual(len(body[0].split(":")), 3)
        status, _, _ = call(self.base, "POST", "/datarouter/api/v1/public/data", body=b"{}")
        self.assertEqual(status, 204)


# Cloud storage --------------------------------------------------------------

class CloudStorageTests(ServiceTestCase):
    cloud_files = {"DefaultGame.ini": b"[Section]\nKey=Value\n", ".hidden": b"x", "Engine.ini": b"[A]\n"}

    def test_system_listing_comes_from_the_local_directory(self):
        status, body, _ = jcall(self.base, "GET", "/fortnite/api/cloudstorage/system")
        self.assertEqual(status, 200)
        names = [entry["uniqueFilename"] for entry in body]
        self.assertEqual(names, ["DefaultGame.ini", "Engine.ini"], "hidden files are not listed")
        game = body[0]
        self.assertEqual(game["length"], len(self.cloud_files["DefaultGame.ini"]))
        self.assertEqual(len(game["hash256"]), 64)
        self.assertEqual(len(game["hash"]), 40)

    def test_system_file_is_served_raw_and_unknown_names_are_404(self):
        status, headers, raw = call(self.base, "GET", "/fortnite/api/cloudstorage/system/DefaultGame.ini")
        self.assertEqual((status, raw), (200, self.cloud_files["DefaultGame.ini"]))
        self.assertTrue(headers["Content-Type"])
        self.assertError(jcall(self.base, "GET", "/fortnite/api/cloudstorage/system/missing.ini"),
                         404, Err.CLOUD_FILE_NOT_FOUND)
        self.assertError(jcall(self.base, "GET", "/fortnite/api/cloudstorage/system/.hidden"),
                         404, Err.CLOUD_FILE_NOT_FOUND)

    def test_user_storage_round_trip_and_account_checks(self):
        token = self.sign_in()["access_token"]
        path = f"/fortnite/api/cloudstorage/user/{LOCAL}"
        self.assertEqual(jcall(self.base, "GET", path, token=token)[1], [])
        self.assertError(jcall(self.base, "GET", path), 401, Err.AUTH_REQUIRED)
        other = self.sign_in_account(self.new_account("Other"))["access_token"]
        self.assertError(jcall(self.base, "GET", path, token=other), 403, Err.MISSING_PERMISSION)
        status, _, _ = call(self.base, "PUT", f"{path}/GameUserSettings.ini", token=token, body=b"[Video]\n")
        self.assertEqual(status, 204)
        status, listing, _ = jcall(self.base, "GET", path, token=token)
        self.assertEqual([entry["uniqueFilename"] for entry in listing], ["GameUserSettings.ini"])
        status, headers, raw = call(self.base, "GET", f"{path}/GameUserSettings.ini", token=token)
        self.assertEqual((status, raw), (200, b"[Video]\n"))
        self.assertError(jcall(self.base, "GET", f"{path}/absent.ini", token=token),
                         404, Err.CLOUD_FILE_NOT_FOUND)
        self.assertError(jcall(self.base, "PUT", f"{path}/bad%2Fname", token=token, body=b"x"),
                         400, Err.INVALID_PAYLOAD)


# MCP profiles ---------------------------------------------------------------

def mcp(base, token, account, command, profile="athena", body=None, scope="client"):
    query = f"?profileId={profile}" if profile else ""
    return jcall(base, "POST", f"/fortnite/api/game/v2/profile/{account}/{scope}/{command}{query}",
                 token=token, body={} if body is None else body)


class ProfileTests(ServiceTestCase):
    def setUp(self):
        super().setUp()
        self.token = self.sign_in()["access_token"]

    def query(self, profile="athena"):
        status, body, _ = mcp(self.base, self.token, LOCAL, "QueryProfile", profile)
        self.assertEqual(status, 200, body)
        return body

    def profile_of(self, profile="athena"):
        return self.query(profile)["profileChanges"][0]["profile"]

    def envelope_keys(self, body):
        for key in ("profileRevision", "profileId", "profileChangesBaseRevision", "profileChanges",
                    "profileCommandRevision", "serverTime", "responseVersion", "notifications", "multiUpdate"):
            self.assertIn(key, body)

    def test_query_profile_returns_a_full_athena_profile_with_a_default_locker(self):
        body = self.query("athena")
        self.envelope_keys(body)
        self.assertEqual(body["profileRevision"], 1)
        self.assertEqual(body["profileCommandRevision"], 1)
        change = body["profileChanges"][0]
        self.assertEqual(change["changeType"], "fullProfileUpdate")
        profile = change["profile"]
        self.assertEqual(profile["accountId"], LOCAL)
        templates = sorted(item["templateId"] for item in profile["items"].values())
        self.assertIn("CosmeticLocker:cosmeticlocker_athena", templates)
        self.assertIn("AthenaCharacter:cid_001_athena_commando_f_default", templates)
        locker_id = profile["stats"]["attributes"]["last_applied_loadout"]
        self.assertIn(locker_id, profile["items"])
        self.assertEqual(profile["stats"]["attributes"]["loadouts"], [locker_id])

    def test_each_profile_id_answers_and_common_core_carries_platform(self):
        for profile in ("common_core", "common_public", "campaign", "creative"):
            self.assertEqual(self.query(profile)["profileId"], profile)
        self.assertEqual(self.profile_of("common_core")["stats"]["attributes"]["current_mtx_platform"], "EpicPC")

    def test_query_profile_on_every_route_variant(self):
        for scope in ("client", "dedicated_server", "public"):
            status, body, _ = mcp(self.base, self.token, LOCAL, "QueryProfile", scope=scope)
            self.assertEqual(status, 200, scope)

    def test_profile_routes_need_a_token_for_that_account(self):
        self.assertError(mcp(self.base, None, LOCAL, "QueryProfile"), 401, Err.AUTH_REQUIRED)
        other = self.sign_in_account(self.new_account("Thief"))["access_token"]
        self.assertError(mcp(self.base, other, LOCAL, "QueryProfile"), 403, Err.MISSING_PERMISSION)
        self.assertError(mcp(self.base, self.token, "a" * 32, "QueryProfile"), 403, Err.MISSING_PERMISSION)

    def test_unknown_profile_unknown_command_and_wrong_profile_are_400(self):
        self.assertError(mcp(self.base, self.token, LOCAL, "QueryProfile", profile="nope"),
                         400, Err.PROFILE_ID_INVALID)
        self.assertError(mcp(self.base, self.token, LOCAL, "NoSuchCommand"), 400, Err.OPERATION_NOT_FOUND)
        self.assertError(mcp(self.base, self.token, LOCAL, "PurchaseCatalogEntry", profile="common_core"),
                         400, Err.OPERATION_NOT_FOUND)
        self.assertError(mcp(self.base, self.token, LOCAL, "SetMtxPlatform", profile="athena",
                             body={"newPlatform": "PSN"}), 400, Err.INVALID_PROFILE_COMMAND)
        self.assertError(mcp(self.base, self.token, "b" * 32, "QueryProfile"), 403, Err.MISSING_PERMISSION)

    def test_client_quest_login_bumps_revision_once_per_day(self):
        status, first, _ = mcp(self.base, self.token, LOCAL, "ClientQuestLogin")
        self.assertEqual(status, 200)
        self.envelope_keys(first)
        self.assertEqual((first["profileChangesBaseRevision"], first["profileRevision"]), (1, 2))
        self.assertEqual((first["profileCommandRevision"]), 2)
        _, second, _ = mcp(self.base, self.token, LOCAL, "ClientQuestLogin")
        self.assertEqual(second["profileChanges"], [])
        self.assertEqual((second["profileRevision"], second["profileCommandRevision"]), (2, 3))
        self.clock.now += DAY
        # The access token lives 8 hours, so a new day needs a new sign-in.
        self.token = self.sign_in(code="next-day")["access_token"]
        _, third, _ = mcp(self.base, self.token, LOCAL, "ClientQuestLogin")
        self.assertEqual((third["profileRevision"], third["profileCommandRevision"]), (3, 4))
        rewards = self.profile_of()["stats"]["attributes"]["daily_rewards"]
        self.assertEqual(rewards["totalDaysLoggedIn"], 2)
        self.assertEqual(self.profile_of()["stats"]["attributes"]["quest_manager"]["dailyLoginInterval"],
                         "2026-01-02")

    def test_mark_item_seen_changes_only_the_named_items_and_is_atomic(self):
        fresh = self.backend.profiles.grant_item(LOCAL, "athena", "AthenaDance:eid_warehouse",
                                                 {"item_seen": False, "favorite": False, "variants": []})
        before = self.query()
        status, body, _ = mcp(self.base, self.token, LOCAL, "MarkItemSeen", body={"itemIds": [fresh]})
        self.assertEqual(status, 200)
        self.assertEqual(body["profileChanges"], [{"changeType": "itemAttrChanged", "itemId": fresh,
                                                   "attributeName": "item_seen", "attributeValue": True}])
        self.assertEqual(body["profileRevision"], before["profileRevision"] + 1)
        again = mcp(self.base, self.token, LOCAL, "MarkItemSeen", body={"itemIds": [fresh]})[1]
        self.assertEqual(again["profileChanges"], [])
        self.assertEqual(again["profileRevision"], body["profileRevision"])
        self.assertEqual(again["profileCommandRevision"], body["profileCommandRevision"] + 1)
        # Atomic: a bad id in the same command leaves the stored profile untouched.
        fresh_two = self.backend.profiles.grant_item(LOCAL, "athena", "AthenaDance:eid_celebration",
                                                     {"item_seen": False, "favorite": False, "variants": []})
        before = self.query()
        self.assertError(mcp(self.base, self.token, LOCAL, "MarkItemSeen",
                             body={"itemIds": [fresh_two, DEAD_ITEM]}), 400, Err.INVALID_PAYLOAD)
        after = self.query()
        self.assertEqual(after["profileRevision"], before["profileRevision"])
        self.assertEqual(after["profileCommandRevision"], before["profileCommandRevision"])
        self.assertFalse(self.profile_of()["items"][fresh_two]["attributes"]["item_seen"])

    def test_set_mtx_platform_validates_and_bumps_only_on_change(self):
        _, first, _ = mcp(self.base, self.token, LOCAL, "SetMtxPlatform", "common_core", {"newPlatform": "PSN"})
        self.assertEqual(first["profileChanges"], [{"changeType": "statModified", "name": "current_mtx_platform",
                                                    "value": "PSN"}])
        self.assertEqual(first["profileRevision"], 2)
        _, same, _ = mcp(self.base, self.token, LOCAL, "SetMtxPlatform", "common_core", {"newPlatform": "PSN"})
        self.assertEqual((same["profileRevision"], same["profileCommandRevision"]), (2, 3))
        self.assertError(mcp(self.base, self.token, LOCAL, "SetMtxPlatform", "common_core",
                             {"newPlatform": "Steam"}), 400, Err.INVALID_PAYLOAD)
        self.assertEqual(self.profile_of("common_core")["stats"]["attributes"]["current_mtx_platform"], "PSN")

    def test_set_affiliate_name_and_gift_flag(self):
        _, body, _ = mcp(self.base, self.token, LOCAL, "SetAffiliateName", "common_core", {"affiliateName": "Bee_1"})
        self.assertEqual(self.profile_of("common_core")["stats"]["attributes"]["mtx_affiliate"], "Bee_1")
        self.assertError(mcp(self.base, self.token, LOCAL, "SetAffiliateName", "common_core",
                             {"affiliateName": "two words"}), 400, Err.INVALID_PAYLOAD)
        mcp(self.base, self.token, LOCAL, "SetReceiveGiftsEnabled", "common_core", {"bReceiveGifts": False})
        self.assertFalse(self.profile_of("common_core")["stats"]["attributes"]["allowed_to_receive_gifts"])
        self.assertError(mcp(self.base, self.token, LOCAL, "SetReceiveGiftsEnabled", "common_core",
                             {"bReceiveGifts": "no"}), 400, Err.INVALID_PAYLOAD)

    def test_favorites_single_and_batch(self):
        item = self.backend.profiles.grant_item(LOCAL, "athena", "AthenaDance:eid_warehouse")
        mcp(self.base, self.token, LOCAL, "SetItemFavoriteStatus", body={"targetItemId": item, "bFavorite": True})
        self.assertTrue(self.profile_of()["items"][item]["attributes"]["favorite"])
        mcp(self.base, self.token, LOCAL, "SetItemFavoriteStatusBatch",
            body={"itemIds": [item], "itemFavStatus": [False]})
        self.assertFalse(self.profile_of()["items"][item]["attributes"]["favorite"])
        self.assertError(mcp(self.base, self.token, LOCAL, "SetItemFavoriteStatusBatch",
                             body={"itemIds": [item], "itemFavStatus": []}), 400, Err.INVALID_PAYLOAD)

    def locker_slots(self):
        profile = self.profile_of()
        locker = profile["stats"]["attributes"]["last_applied_loadout"]
        return profile["items"][locker]["attributes"]["locker_slots_data"]["slots"]

    def test_set_cosmetic_locker_slot_places_owned_items_only_in_their_category(self):
        profile = self.profile_of()
        locker = profile["stats"]["attributes"]["last_applied_loadout"]
        dance = self.backend.profiles.grant_item(LOCAL, "athena", "AthenaDance:eid_warehouse")
        status, body, _ = mcp(self.base, self.token, LOCAL, "SetCosmeticLockerSlot", body={
            "lockerItem": locker, "category": "Dance", "itemToSlot": dance, "slotIndex": 1, "variantUpdates": []})
        self.assertEqual(status, 200)
        self.assertEqual(self.locker_slots()["Dance"]["items"], ["AthenaDance:eid_celebration",
                                                                "AthenaDance:eid_warehouse"])
        self.assertEqual(body["profileChanges"][0]["changeType"], "itemAttrChanged")
        self.assertError(mcp(self.base, self.token, LOCAL, "SetCosmeticLockerSlot", body={
            "lockerItem": locker, "category": "Pickaxe", "itemToSlot": dance, "slotIndex": 0}),
            400, Err.INVALID_PAYLOAD)
        self.assertError(mcp(self.base, self.token, LOCAL, "SetCosmeticLockerSlot", body={
            "lockerItem": locker, "category": "Dance", "itemToSlot": DEAD_ITEM, "slotIndex": 0}),
            400, Err.INVALID_PAYLOAD)
        mcp(self.base, self.token, LOCAL, "SetCosmeticLockerSlot", body={
            "lockerItem": locker, "category": "Dance", "itemToSlot": "", "slotIndex": 1})
        self.assertEqual(self.locker_slots()["Dance"]["items"][1], "")

    def test_equip_battle_royale_customization_targets_the_last_applied_loadout(self):
        dance = self.backend.profiles.grant_item(LOCAL, "athena", "AthenaDance:eid_warehouse")
        status, body, _ = mcp(self.base, self.token, LOCAL, "EquipBattleRoyaleCustomization", body={
            "slotName": "Dance", "itemToSlot": dance, "indexWithinSlot": 2, "variantUpdates": []})
        self.assertEqual(status, 200)
        self.assertEqual(self.locker_slots()["Dance"]["items"][2], "AthenaDance:eid_warehouse")
        self.assertGreater(body["profileRevision"], 1)

    def test_locker_name_and_banner_validation(self):
        locker = self.profile_of()["stats"]["attributes"]["last_applied_loadout"]
        mcp(self.base, self.token, LOCAL, "SetCosmeticLockerName", body={"lockerItem": locker, "name": "Main"})
        self.assertEqual(self.profile_of()["items"][locker]["attributes"]["locker_name"], "Main")
        self.assertError(mcp(self.base, self.token, LOCAL, "SetCosmeticLockerName",
                             body={"lockerItem": locker, "name": "x" * 33}), 400, Err.INVALID_PAYLOAD)
        mcp(self.base, self.token, LOCAL, "SetCosmeticLockerBanner",
            body={"lockerItem": locker, "bannerIconTemplate": "StandardBanner5"})
        self.assertEqual(self.profile_of()["items"][locker]["attributes"]["banner_icon_template"],
                         "StandardBanner5")
        self.assertError(mcp(self.base, self.token, LOCAL, "SetCosmeticLockerBanner",
                             body={"lockerItem": locker}), 400, Err.INVALID_PAYLOAD)

    def test_set_battle_royale_banner_reports_the_mirrored_profile_in_multi_update(self):
        status, body, _ = mcp(self.base, self.token, LOCAL, "SetBattleRoyaleBanner",
                              body={"bannerIconTemplate": "StandardBanner7", "bannerColorTemplate": "DefaultColor3"})
        self.assertEqual(status, 200)
        self.assertEqual(body["profileId"], "athena")
        self.assertEqual(len(body["multiUpdate"]), 1)
        mirrored = body["multiUpdate"][0]
        self.assertEqual(mirrored["profileId"], "common_public")
        self.assertEqual(mirrored["profileRevision"], 2)
        self.assertEqual(mirrored["profileChangesBaseRevision"], 1)
        names = {change["name"] for change in mirrored["profileChanges"]}
        self.assertEqual(names, {"banner_icon", "banner_color"})
        public = self.profile_of("common_public")["stats"]["attributes"]
        self.assertEqual((public["banner_icon"], public["banner_color"]), ("StandardBanner7", "DefaultColor3"))

    def test_copy_cosmetic_loadout_adds_an_item_and_a_slot(self):
        status, body, _ = mcp(self.base, self.token, LOCAL, "CopyCosmeticLoadout", body={
            "sourceIndex": 0, "targetIndex": 1, "optNewNameForTarget": "Second"})
        self.assertEqual(status, 200)
        kinds = [change["changeType"] for change in body["profileChanges"]]
        self.assertIn("itemAdded", kinds)
        self.assertIn("statModified", kinds)
        loadouts = self.profile_of()["stats"]["attributes"]["loadouts"]
        self.assertEqual(len(loadouts), 2)
        copied = self.profile_of()["items"][loadouts[1]]
        self.assertEqual(copied["templateId"], "CosmeticLocker:cosmeticlocker_athena")
        self.assertEqual(copied["attributes"]["locker_name"], "Second")
        self.assertError(mcp(self.base, self.token, LOCAL, "CopyCosmeticLoadout", body={
            "sourceIndex": 5, "targetIndex": 0}), 400, Err.INVALID_PAYLOAD)

    def test_unimplemented_commands_are_named_in_the_error(self):
        status, body, _ = mcp(self.base, self.token, LOCAL, "PurchaseCatalogEntry", "common_core")
        self.assertEqual(status, 400)
        self.assertIn("not implemented", body["errorMessage"])

    def test_revisions_never_go_backwards_across_a_sequence_of_commands(self):
        locker = self.profile_of()["stats"]["attributes"]["last_applied_loadout"]
        sequence = [
            ("athena", "ClientQuestLogin", {}),
            ("athena", "SetCosmeticLockerName", {"lockerItem": locker, "name": "A"}),
            ("athena", "SetCosmeticLockerName", {"lockerItem": locker, "name": "A"}),
            ("common_core", "SetMtxPlatform", {"newPlatform": "XBL"}),
        ]
        last = {}  # per profile: (profileRevision, profileCommandRevision)
        for profile, command, body in sequence:
            _, result, _ = mcp(self.base, self.token, LOCAL, command, profile, body)
            previous_rvn, previous_cmd = last.get(profile, (1, 1))
            self.assertGreaterEqual(result["profileRevision"], previous_rvn)
            self.assertEqual(result["profileCommandRevision"], previous_cmd + 1)
            last[profile] = (result["profileRevision"], result["profileCommandRevision"])


# Party and presence ---------------------------------------------------------

PARTY = "/party/api/v1/Fortnite"


def create_party(base, token, config=None, meta=None):
    body = {"config": config or {}, "meta": meta or {}, "join_info": {"connection": {"id": "conn-a"}}}
    status, party, _ = jcall(base, "POST", f"{PARTY}/parties", token=token, body=body)
    assert status == 200, party
    return party


class PartyTests(ServiceTestCase):
    def setUp(self):
        super().setUp()
        self.a_id = LOCAL
        self.a = self.sign_in()["access_token"]
        self.b_id = self.new_account("Bee")
        self.b = self.sign_in_account(self.b_id)["access_token"]
        self.c_id = self.new_account("Cee")
        self.c = self.sign_in_account(self.c_id)["access_token"]
        self.d_id = self.new_account("Dee")
        self.d = self.sign_in_account(self.d_id)["access_token"]

    def test_create_party_makes_the_caller_captain_and_blocks_a_second_party(self):
        party = create_party(self.base, self.a)
        self.assertEqual(party["revision"], 1)
        self.assertEqual(party["config"]["maxSize"], 16)
        self.assertEqual(party["config"]["joinability"], "OPEN")
        self.assertEqual(len(party["members"]), 1)
        self.assertEqual((party["members"][0]["account_id"], party["members"][0]["role"]), (self.a_id, "CAPTAIN"))
        self.assertError(jcall(self.base, "POST", f"{PARTY}/parties", token=self.a, body={}),
                         409, Err.USER_HAS_PARTY)
        self.assertError(jcall(self.base, "POST", f"{PARTY}/parties", body={}), 401, Err.AUTH_REQUIRED)

    def test_get_party_and_unknown_party(self):
        party = create_party(self.base, self.a)
        status, body, _ = jcall(self.base, "GET", f"{PARTY}/parties/{party['id']}", token=self.b)
        self.assertEqual((status, body["id"]), (200, party["id"]))
        self.assertError(jcall(self.base, "GET", f"{PARTY}/parties/{'0' * 32}", token=self.b),
                         404, Err.PARTY_NOT_FOUND)

    def test_join_adds_a_member_and_bumps_revision(self):
        party = create_party(self.base, self.a)
        pid = party["id"]
        status, joined, _ = jcall(self.base, "POST", f"{PARTY}/parties/{pid}/members/{self.b_id}/join",
                                  token=self.b, body={"connection": {"id": "conn-b"}})
        self.assertEqual(status, 200)
        self.assertEqual(joined["revision"], 2)
        roles = {m["account_id"]: m["role"] for m in joined["members"]}
        self.assertEqual(roles, {self.a_id: "CAPTAIN", self.b_id: "MEMBER"})
        self.assertError(jcall(self.base, "POST", f"{PARTY}/parties/{pid}/members/{self.b_id}/join",
                               token=self.b, body={}), 409, Err.USER_ALREADY_IN_PARTY)
        self.assertError(jcall(self.base, "POST", f"{PARTY}/parties/{pid}/members/{self.b_id}/join",
                               token=self.c, body={}), 403, Err.MISSING_PERMISSION)
        status, presence, _ = jcall(self.base, "GET", f"/presence/api/v1/_/{self.b_id}/last-online", token=self.a)
        self.assertEqual((presence["status"], presence["partyId"]), ("Online", pid))

    def test_party_is_full_and_build_mismatch_is_refused(self):
        party = create_party(self.base, self.a, config={"maxSize": 1})
        self.assertError(jcall(self.base, "POST", f"{PARTY}/parties/{party['id']}/members/{self.b_id}/join",
                               token=self.b, body={}), 409, Err.PARTY_IS_FULL)
        other = create_party(self.base, self.c, meta={"urn:epic:cfg:build-id_s": "683914758"})
        self.assertError(jcall(self.base, "POST", f"{PARTY}/parties/{other['id']}/members/{self.b_id}/join",
                               token=self.b, body={"meta": {"urn:epic:cfg:build-id_s": "1"}}),
                         403, Err.PARTY_JOIN_FORBIDDEN)

    def test_closed_party_needs_an_invite_and_declining_removes_it(self):
        pid = create_party(self.base, self.a, config={"joinability": "CLOSED"})["id"]
        self.assertError(jcall(self.base, "POST", f"{PARTY}/parties/{pid}/members/{self.c_id}/join",
                               token=self.c, body={}), 403, Err.PARTY_JOIN_FORBIDDEN)
        status, _, _ = call(self.base, "POST", f"{PARTY}/parties/{pid}/invites/{self.c_id}", token=self.a,
                            body={"sendPing": True, "ttl": 600, "meta": {"note": "come"}})
        self.assertEqual(status, 204)
        self.assertError(jcall(self.base, "POST", f"{PARTY}/parties/{pid}/invites/{self.c_id}", token=self.a,
                               body={}), 409, Err.INVITE_ALREADY_EXISTS)
        _, state, _ = jcall(self.base, "GET", f"{PARTY}/user/{self.c_id}", token=self.c)
        self.assertEqual([invite["party_id"] for invite in state["invites"]], [pid])
        self.assertEqual([ping["sent_by"] for ping in state["pings"]], [self.a_id])
        self.assertEqual(jcall(self.base, "POST", f"{PARTY}/parties/{pid}/members/{self.c_id}/join",
                               token=self.c, body={})[0], 200)
        _, state, _ = jcall(self.base, "GET", f"{PARTY}/user/{self.c_id}", token=self.c)
        self.assertEqual(state["invites"], [])

    def test_decline_and_invite_checks(self):
        pid = create_party(self.base, self.a)["id"]
        self.assertError(jcall(self.base, "POST", f"{PARTY}/parties/{pid}/invites/{self.d_id}", token=self.b,
                               body={}), 403, Err.INVITE_FORBIDDEN)
        self.assertError(jcall(self.base, "POST", f"{PARTY}/parties/{pid}/invites/{'9' * 32}", token=self.a,
                               body={}), 404, Err.ACCOUNT_NOT_FOUND)
        call(self.base, "POST", f"{PARTY}/parties/{pid}/invites/{self.d_id}", token=self.a, body={})
        status, _, _ = call(self.base, "POST", f"{PARTY}/parties/{pid}/invites/{self.d_id}/{self.a_id}/decline",
                            token=self.d)
        self.assertEqual(status, 204)
        self.assertError(jcall(self.base, "POST", f"{PARTY}/parties/{pid}/invites/{self.d_id}/{self.a_id}/decline",
                               token=self.d), 404, Err.INVITE_NOT_FOUND)

    def test_member_meta_update_uses_revisions(self):
        pid = create_party(self.base, self.a)["id"]
        jcall(self.base, "POST", f"{PARTY}/parties/{pid}/members/{self.b_id}/join", token=self.b, body={})
        path = f"{PARTY}/parties/{pid}/members/{self.b_id}/meta"
        status, _, _ = call(self.base, "PATCH", path, token=self.b,
                            body={"update": {"urn:epic:member:dn_s": "Bee"}, "revision": 0})
        self.assertEqual(status, 204)
        _, party, _ = jcall(self.base, "GET", f"{PARTY}/parties/{pid}", token=self.a)
        bee = [m for m in party["members"] if m["account_id"] == self.b_id][0]
        self.assertEqual(bee["meta"], {"urn:epic:member:dn_s": "Bee"})
        self.assertEqual(bee["revision"], 1)
        self.assertEqual(party["revision"], 3)
        self.assertError(jcall(self.base, "PATCH", path, token=self.b, body={"update": {"x": "1"}, "revision": 0}),
                         409, Err.STALE_REVISION)
        self.assertError(jcall(self.base, "PATCH", path, token=self.a, body={"update": {"x": "1"}}),
                         403, Err.MEMBER_STATE_FORBIDDEN)
        call(self.base, "PATCH", path, token=self.b, body={"delete": ["urn:epic:member:dn_s"]})
        _, party, _ = jcall(self.base, "GET", f"{PARTY}/parties/{pid}", token=self.a)
        self.assertEqual([m["meta"] for m in party["members"] if m["account_id"] == self.b_id], [{}])

    def test_party_meta_is_captain_only(self):
        pid = create_party(self.base, self.a)["id"]
        jcall(self.base, "POST", f"{PARTY}/parties/{pid}/members/{self.b_id}/join", token=self.b, body={})
        self.assertError(jcall(self.base, "PATCH", f"{PARTY}/parties/{pid}/meta", token=self.b,
                               body={"update": {"mode": "x"}}), 403, Err.PARTY_CHANGE_FORBIDDEN)
        status, _, _ = call(self.base, "PATCH", f"{PARTY}/parties/{pid}/meta", token=self.a,
                            body={"update": {"mode": "x"}})
        self.assertEqual(status, 204)
        _, party, _ = jcall(self.base, "GET", f"{PARTY}/parties/{pid}", token=self.a)
        self.assertEqual(party["meta"], {"mode": "x"})

    def test_promote_kick_and_captain_handover_on_leave(self):
        pid = create_party(self.base, self.a)["id"]
        for token, account in ((self.b, self.b_id), (self.c, self.c_id)):
            jcall(self.base, "POST", f"{PARTY}/parties/{pid}/members/{account}/join", token=token, body={})
        self.assertEqual(call(self.base, "POST", f"{PARTY}/parties/{pid}/members/{self.b_id}/promote",
                              token=self.a)[0], 204)
        _, party, _ = jcall(self.base, "GET", f"{PARTY}/parties/{pid}", token=self.a)
        roles = {m["account_id"]: m["role"] for m in party["members"]}
        self.assertEqual(roles[self.b_id], "CAPTAIN")
        self.assertEqual(roles[self.a_id], "MEMBER")
        self.assertError(jcall(self.base, "DELETE", f"{PARTY}/parties/{pid}/members/{self.c_id}", token=self.a),
                         403, Err.USER_OPERATION_FORBIDDEN)
        self.assertEqual(call(self.base, "DELETE", f"{PARTY}/parties/{pid}/members/{self.c_id}", token=self.b)[0],
                         204)
        self.assertEqual(call(self.base, "DELETE", f"{PARTY}/parties/{pid}/members/{self.b_id}", token=self.b)[0],
                         204)
        _, party, _ = jcall(self.base, "GET", f"{PARTY}/parties/{pid}", token=self.a)
        self.assertEqual([(m["account_id"], m["role"]) for m in party["members"]], [(self.a_id, "CAPTAIN")])
        self.assertEqual(call(self.base, "DELETE", f"{PARTY}/parties/{pid}/members/{self.a_id}", token=self.a)[0],
                         204)
        self.assertError(jcall(self.base, "GET", f"{PARTY}/parties/{pid}", token=self.a),
                         404, Err.PARTY_NOT_FOUND)

    def test_ping_rules_and_user_state(self):
        pid = create_party(self.base, self.a)["id"]
        jcall(self.base, "POST", f"{PARTY}/parties/{pid}/members/{self.b_id}/join", token=self.b, body={})
        self.assertEqual(call(self.base, "POST", f"{PARTY}/user/{self.b_id}/pings/{self.a_id}/parties",
                              token=self.a, body={"meta": {"hi": "1"}})[0], 204)
        _, state, _ = jcall(self.base, "GET", f"{PARTY}/user/{self.b_id}", token=self.b)
        self.assertEqual([ping["sent_by"] for ping in state["pings"]], [self.a_id])
        self.assertEqual([p["id"] for p in state["current"]], [pid])
        self.assertEqual((state["pending"], state["invites"]), ([], []))
        self.assertError(jcall(self.base, "POST", f"{PARTY}/user/{self.a_id}/pings/{self.a_id}/parties", token=self.a, body={}),
                         400, Err.SELF_PING)
        # A sender with no party cannot ping.
        self.assertError(jcall(self.base, "POST", f"{PARTY}/user/{self.d_id}/pings/{self.c_id}/parties",
                               token=self.c, body={}), 403, Err.USER_OPERATION_FORBIDDEN)
        # A caller cannot send a ping as somebody else.
        self.assertError(jcall(self.base, "POST", f"{PARTY}/user/{self.d_id}/pings/{self.c_id}/parties",
                               token=self.a, body={}), 403, Err.MISSING_PERMISSION)
        self.assertError(jcall(self.base, "GET", f"{PARTY}/user/{self.b_id}", token=self.a),
                         403, Err.MISSING_PERMISSION)

    def test_disconnect_clears_connections_and_marks_offline(self):
        pid = create_party(self.base, self.a)["id"]
        jcall(self.base, "POST", f"{PARTY}/parties/{pid}/members/{self.b_id}/join", token=self.b, body={})
        self.assertEqual(call(self.base, "POST", f"{PARTY}/parties/{pid}/members/{self.b_id}/disconnect",
                              token=self.b)[0], 204)
        _, party, _ = jcall(self.base, "GET", f"{PARTY}/parties/{pid}", token=self.a)
        bee = [m for m in party["members"] if m["account_id"] == self.b_id][0]
        self.assertEqual(bee["connections"], [])
        _, presence, _ = jcall(self.base, "GET", f"/presence/api/v1/_/{self.b_id}/last-online", token=self.a)
        self.assertEqual(presence["status"], "Offline")

    def test_applicant_decisions_answer_not_found_for_the_captain(self):
        pid = create_party(self.base, self.a)["id"]
        jcall(self.base, "POST", f"{PARTY}/parties/{pid}/members/{self.b_id}/join", token=self.b, body={})
        self.assertError(jcall(self.base, "POST", f"{PARTY}/parties/{pid}/members/{self.c_id}/confirm",
                               token=self.a), 404, Err.APPLICANT_NOT_FOUND)
        self.assertError(jcall(self.base, "POST", f"{PARTY}/parties/{pid}/members/{self.c_id}/reject",
                               token=self.b), 403, Err.MEMBER_STATE_FORBIDDEN)

    def test_presence_subscriptions(self):
        self.assertEqual(jcall(self.base, "GET", f"/presence/api/v1/_/{self.a_id}/subscriptions", token=self.a)[1],
                         [])
        self.assertEqual(call(self.base, "POST", f"/presence/api/v1/_/{self.a_id}/subscriptions/{self.b_id}",
                              token=self.a)[0], 204)
        self.assertEqual(jcall(self.base, "GET", f"/presence/api/v1/_/{self.a_id}/subscriptions", token=self.a)[1],
                         [self.b_id])
        self.assertEqual(call(self.base, "DELETE", f"/presence/api/v1/_/{self.a_id}/subscriptions/{self.b_id}",
                              token=self.a)[0], 204)
        self.assertEqual(jcall(self.base, "GET", f"/presence/api/v1/_/{self.a_id}/subscriptions", token=self.a)[1],
                         [])
        self.assertError(jcall(self.base, "GET", f"/presence/api/v1/_/{'8' * 32}/last-online", token=self.a),
                         404, Err.ACCOUNT_NOT_FOUND)


# Matchmaking, websocket and session routes ---------------------------------

class MatchmakingTests(ServiceTestCase):
    def test_ticket_websocket_states_play_session_and_key(self):
        account = LOCAL
        status, ticket, _ = jcall(self.base, "GET",
                                  f"/fortnite/api/game/v2/matchmakingservice/ticket/player/{account}"
                                  "?bucketId=UNKNOWN:0:1&player.platform=Windows")
        self.assertEqual(status, 200)
        self.assertEqual(set(ticket), {"serviceUrl", "ticketType", "payload", "signature"})
        path = urllib.parse.urlsplit(ticket["serviceUrl"]).path
        sock, stream, status_line, accepted = ws.ws_open(self.server.port, path, None)
        self.assertTrue(status_line.startswith("HTTP/1.1 101"))
        self.assertTrue(accepted)
        messages = []
        while True:
            opcode, payload = ws.ws_recv(stream)
            messages.append(json.loads(payload))
            if messages[-1]["name"] == "Play":
                break
        sock.close()
        for message in messages:
            self.assertEqual(set(message), {"name", "payload"})
            self.assertIsInstance(message["payload"], dict)
        statuses = [m["payload"] for m in messages if m["name"] == "StatusUpdate"]
        self.assertEqual([m["state"] for m in statuses], list(matchmaking.STATES))
        ready = next(m for m in statuses if m["state"] == "ReadyCheck")
        self.assertEqual(ready["status"], "ACTIVE")
        self.assertTrue(all(type(ready[k]) is int for k in
                            ("durationSec", "totalPlayers", "readyPlayers", "unreadyPlayers")))
        self.assertTrue(all(type(ready[k]) is bool for k in ("ready", "responded")))
        play = messages[-1]["payload"]
        self.assertEqual(set(play), {"matchId", "sessionId", "joinDelaySec"})
        self.assertTrue(play["matchId"] and play["sessionId"])
        self.assertIsInstance(play["joinDelaySec"], int)
        self.assertTrue(all(m["matchId"] == play["matchId"] for m in statuses))
        session_id = play["sessionId"]
        status, record, _ = jcall(self.base, "GET", f"/fortnite/api/matchmaking/session/{session_id}")
        self.assertEqual((status, record["buildUniqueId"], record["ownerId"]), (200, "683914758", account))
        self.assertEqual(record["ownerName"], "LocalPlayer")
        status, key, _ = jcall(self.base, "GET",
                               f"/fortnite/api/game/v2/matchmaking/account/{account}/session/{session_id}")
        self.assertEqual(status, 200)
        self.assertEqual(set(key), {"accountId", "sessionId", "key"})
        self.assertEqual((key["accountId"], key["sessionId"]), (account, session_id))
        self.assertTrue(all(isinstance(key[name], str) and key[name] for name in key))
        self.assertEqual(len(base64.b64decode(key["key"], validate=True)), 32)
        status, repeated, _ = jcall(self.base, "GET",
                                    f"/api/game/v2/matchmaking/account/{account}/session/{session_id}")
        self.assertEqual(status, 200)
        self.assertEqual(repeated, key)
        self.assertError(jcall(self.base, "GET",
                               f"/api/game/v2/matchmaking/account/{'2' * 32}/session/{session_id}"),
                         404, Err.NOT_FOUND)
        self.assertError(jcall(self.base, "GET", f"/fortnite/api/matchmaking/session/{'1' * 32}"),
                         404, Err.NOT_FOUND)

    def test_party_ticket_session_roster_and_keys(self):
        other = self.new_account("SecondPlayer")
        status, ticket, _ = jcall(
            self.base, "GET", f"/fortnite/api/game/v2/matchmakingservice/ticket/player/{LOCAL}"
            f"?partyPlayerIds={other},{LOCAL},{other}&bucketId=UNKNOWN:0:1")
        self.assertEqual(status, 200)
        path = urllib.parse.urlsplit(ticket["serviceUrl"]).path
        sock, stream, status_line, accepted = ws.ws_open(self.server.port, path, None)
        self.assertTrue(status_line.startswith("HTTP/1.1 101") and accepted)
        try:
            while True:
                opcode, payload = ws.ws_recv(stream)
                self.assertEqual(opcode, 0x1)
                message = json.loads(payload)
                if message["name"] == "Play":
                    session_id = message["payload"]["sessionId"]
                    break
        finally:
            sock.close()
        status, record, _ = jcall(self.base, "GET", f"/fortnite/api/matchmaking/session/{session_id}")
        self.assertEqual(status, 200)
        self.assertEqual(record["publicPlayers"], [LOCAL, other])
        self.assertEqual(record["openPublicPlayers"], record["maxPublicPlayers"] - 2)
        keys = []
        for account in (LOCAL, other):
            status, body, _ = jcall(self.base, "GET",
                                      f"/api/game/v2/matchmaking/account/{account}/session/{session_id}")
            self.assertEqual(status, 200)
            self.assertEqual(body["accountId"], account)
            keys.append(body["key"])
        self.assertEqual(keys[0], keys[1], "party members must receive one session encryption key")
        self.assertError(jcall(self.base, "GET",
                               f"/api/game/v2/matchmaking/account/{'2' * 32}/session/{session_id}"),
                         404, Err.NOT_FOUND)

    def test_unknown_ticket_is_closed_with_1008(self):
        sock, stream, status_line, _ = ws.ws_open(self.server.port,
                                                  matchmaking.MATCHMAKING_WS_PATH + "/" + "0" * 32, None)
        opcode, payload = ws.ws_recv(stream)
        sock.close()
        self.assertEqual(opcode, 0x8)
        self.assertEqual(int.from_bytes(payload[:2], "big"), 1008)


class SchemaTests(ServiceTestCase):
    """Responses checked against the reflection-derived schemas (schema_check.py).

    The MCP profile shape comes from the McpProfileSys headers. The other shapes come from
    reflection_schemas.json (chat, storefront catalog, calendar). Each check fails on a missing member,
    a JSON type the reflected member cannot hold, or a key the struct does not declare.
    """

    def setUp(self):
        super().setUp()
        self.token = self.sign_in()["access_token"]
        self.structs = schema_check.parse_structs()
        self.others = schema_check.load_reflected_schemas()

    def query(self, profile="athena"):
        status, body, _ = mcp(self.base, self.token, LOCAL, "QueryProfile", profile)
        self.assertEqual(status, 200, body)
        return body

    def test_query_profile_root_matches_reflected_profile_update(self):
        body = self.query("athena")
        self.assertEqual(schema_check.check_profile_response(body, self.structs), [])

    def test_command_and_multi_update_entries_match_reflected_profile_update(self):
        status, body, _ = mcp(self.base, self.token, LOCAL, "ClientQuestLogin", "athena")
        self.assertEqual(status, 200, body)
        self.assertEqual(schema_check.check_profile_response(body, self.structs), [])
        # SetBattleRoyaleBanner also changes common_public, so the response has a multiUpdate entry.
        status, body, _ = mcp(self.base, self.token, LOCAL, "SetBattleRoyaleBanner", "athena",
                              body={"bannerIconTemplate": "BannerIcon_Check", "bannerColorTemplate": "BannerColor_Check"})
        self.assertEqual(status, 200, body)
        self.assertTrue(body["multiUpdate"], "expected a multiUpdate entry")
        self.assertEqual(schema_check.check_profile_response(body, self.structs), [])

    def test_profile_update_members_are_the_reflected_ones(self):
        names = sorted(member["json_name"] for member in self.structs["FProfileUpdateSingle"]["members"])
        self.assertEqual(names, sorted(["profileRevision", "profileId", "profileChangesBaseRevision",
                                        "profileChanges", "lockExpiration", "notifications",
                                        "profileCommandRevision"]))

    def test_checker_flags_a_missing_and_a_mistyped_member(self):
        broken = dict(self.query("athena"))
        del broken["lockExpiration"]
        broken["profileRevision"] = "2"
        issues = schema_check.check_profile_response(broken, self.structs)
        self.assertTrue(any("lockExpiration" in issue and "missing" in issue for issue in issues), issues)
        self.assertTrue(any("profileRevision" in issue and "integer" in issue for issue in issues), issues)

    def test_headers_agree_with_reflection_struct_blocks(self):
        reflection = HERE.parent / "generated" / "reflection" / "struct_blocks.json"
        if not reflection.exists():
            self.skipTest("porting/generated/reflection/struct_blocks.json is git-ignored and not present")
        blocks = json.loads(reflection.read_text(encoding="utf-8"))["structs"]
        by_name = {block["name"]: block for block in blocks if block.get("name")}
        for header_name, reflected_name in (("FProfileUpdateSingle", "ProfileUpdateSingle"),
                                            ("FMcpLootResult", "McpLootResult"),
                                            ("FGiftBoxInfo", "GiftBoxInfo")):
            with self.subTest(struct=header_name):
                self.assertIn(reflected_name, by_name)
                reflected = {prop["name"] for prop in by_name[reflected_name]["properties"]}
                declared = {member["name"] for member in self.structs[header_name]["members"]}
                self.assertEqual(reflected, declared)

    def test_profile_update_envelope_matches_reflection(self):
        reflection = HERE.parent / "generated" / "reflection" / "struct_blocks.json"
        if not reflection.exists():
            self.skipTest("porting/generated/reflection/struct_blocks.json is git-ignored and not present")
        blocks = json.loads(reflection.read_text(encoding="utf-8"))["structs"]
        by_name = {block["name"]: block for block in blocks if block.get("name")}
        root, single, date = (by_name[name] for name in ("ProfileUpdate", "ProfileUpdateSingle", "DateTime"))
        self.assertEqual(root["super_fn"], single["construct_fns"][0])
        outer = {p["name"]: p for p in root["properties"] if not p.get("array_inner_of")}
        self.assertEqual({schema_check.standardize_case(name) for name in outer}, set(schema_check.ENVELOPE_FIELDS))
        self.assertEqual({name: p["kind"] for name, p in outer.items()},
                         {"MultiUpdate": "Array", "ServerTime": "Struct", "ResponseVersion": "Int"})
        self.assertEqual(outer["ServerTime"]["type_refs"]["struct"], date["construct_fns"][0])
        inner = [p for p in root["properties"] if p.get("array_inner_of") == "MultiUpdate"]
        self.assertEqual(len(inner), 1)
        self.assertEqual(inner[0]["kind"], "Struct")
        self.assertEqual(inner[0]["type_refs"]["struct"], single["construct_fns"][0])
        self.assertEqual(schema_check.ENVELOPE_FIELDS["multiUpdate"]["type"], ("array", "FProfileUpdateSingle"))

    def test_chat_reserve_matches_reflected_general_chat_return(self):
        status, body, _ = jcall(self.base, "POST",
                                f"/fortnite/api/game/v2/chat/{LOCAL}/reserveGeneralChatRooms/fortnite/Windows",
                                token=self.token, body={})
        self.assertEqual(status, 200, body)
        self.assertEqual(schema_check.check_reflected_object(body, "GeneralChatReturn", self.others), [])

    def test_storefront_catalog_matches_reflected_catalog_download(self):
        status, body, _ = jcall(self.base, "GET", "/fortnite/api/storefront/v2/catalog", token=self.token)
        self.assertEqual(status, 200, body)
        self.assertEqual(schema_check.check_reflected_object(body, "CatalogDownload", self.others), [])

    def test_calendar_timeline_matches_reflected_calendar_download(self):
        status, body, _ = jcall(self.base, "GET", "/fortnite/api/calendar/v1/timeline", token=self.token)
        self.assertEqual(status, 200, body)
        self.assertEqual(schema_check.check_reflected_object(body, "CalendarDownload", self.others), [])
        self.assertTrue(body["channels"])
        for name, channel in body["channels"].items():
            with self.subTest(channel=name):
                self.assertEqual(schema_check.check_reflected_object(channel, "EventsTimeline", self.others), [])

    def test_reflected_checker_flags_a_missing_catalog_member(self):
        status, body, _ = jcall(self.base, "GET", "/fortnite/api/storefront/v2/catalog", token=self.token)
        partial = {key: value for key, value in body.items() if key != "expiration"}
        issues = schema_check.check_reflected_object(partial, "CatalogDownload", self.others)
        self.assertTrue(any("expiration" in issue and "missing" in issue for issue in issues), issues)


class ServiceFacadeTests(unittest.TestCase):
    def test_compat_facade_answers_with_the_same_signature(self):
        import fortnite_stub
        status, body = fortnite_stub.respond("GET", "/__missing__", {}, {})
        self.assertEqual(status, 404)
        self.assertEqual(body["errorCode"], Err.NOT_FOUND)
        self.assertEqual(fortnite_stub.ACCOUNT_ID, LOCAL)
        self.assertIsInstance(api_common.RawText("true"), str)


if __name__ == "__main__":
    unittest.main(verbosity=2)
