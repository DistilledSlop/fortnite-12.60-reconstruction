"""Focused end-to-end backend tests for vanilla 12.60 client flows."""

if __package__:
    from . import _bootstrap
    from .test_services import GENERATED, ServiceTestCase, call, jcall, mcp
    from .test_xmpp_service import Peer
else:
    import sys as _sys
    from pathlib import Path as _Path
    _sys.path.insert(0, str(_Path(__file__).resolve().parent))
    import _bootstrap
    from test_services import GENERATED, ServiceTestCase, call, jcall, mcp
    from test_xmpp_service import Peer

import json
import socket
import tempfile
import urllib.parse
import xml.etree.ElementTree as ET

import smoke_matchmaking as ws
from backend import Backend


class VanillaClientFlowTests(ServiceTestCase):
    def setUp(self):
        super().setUp()
        self.backend.close()
        self.state = tempfile.TemporaryDirectory(dir=GENERATED)
        self.addCleanup(self.state.cleanup)
        self.secret = "vanilla-flow-secret"
        self.restart()

    def restart(self):
        if hasattr(self, "backend"):
            self.backend.close()
        self.start_backend(Backend(clock=self.clock, state_step_sec=0, state_dir=self.state.name,
                                   gameserver_secret=self.secret))
        self.addCleanup(self.backend.close)

    def login(self, username):
        status, body, _ = jcall(self.base, "POST", "/account/api/oauth/token",
                                form={"grant_type": "local_username", "username": username})
        self.assertEqual(status, 200, body)
        return body

    def register_server(self, playlists=("Playlist_DefaultSolo", "Playlist_DefaultDuo")):
        status, reg, _ = jcall(self.base, "POST", "/local/gameserver/register", token=self.secret,
                               body={"address": "127.0.0.1", "port": 7777, "playlists": list(playlists),
                                     "maxPlayers": 4, "buildUniqueId": "12896485"})
        self.assertEqual(status, 200, reg)
        sid = reg["sessionId"]
        status, _, _ = jcall(self.base, "POST", f"/local/gameserver/{sid}/state", token=self.secret,
                             body={"phase": "warmup", "numPlayers": 0, "joinable": True, "connectedPlayerIds": []})
        self.assertEqual(status, 204)
        return sid, reg["encryptionKey"]

    def matchmake(self, account_id, playlist="Playlist_DefaultSolo", party_ids=None):
        query = f"?bucketId=12896485:0:NAE:{playlist}"
        if party_ids:
            query += "&partyPlayerIds=" + ",".join(party_ids)
        status, ticket, _ = jcall(self.base, "GET",
                                  f"/fortnite/api/game/v2/matchmakingservice/ticket/player/{account_id}{query}")
        self.assertEqual(status, 200, ticket)
        sock, stream, status_line, accepted = ws.ws_open(
            self.server.port, urllib.parse.urlsplit(ticket["serviceUrl"]).path, None)
        self.assertTrue(status_line.startswith("HTTP/1.1 101") and accepted)
        try:
            while True:
                _, payload = ws.ws_recv(stream)
                msg = json.loads(payload)
                if msg["name"] == "Play":
                    return msg["payload"]["sessionId"]
        finally:
            stream.close()
            sock.close()

    def xmpp_peer(self, auth):
        sock = socket.create_connection(("127.0.0.1", self.server.port), timeout=2)
        sock.settimeout(2)
        sock.sendall((f"GET /xmpp HTTP/1.1\r\nHost: 127.0.0.1:{self.server.port}\r\n"
                      "Upgrade: websocket\r\nConnection: Upgrade\r\n"
                      "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n"
                      "Sec-WebSocket-Version: 13\r\nSec-WebSocket-Protocol: xmpp\r\n\r\n").encode("ascii"))
        reader = sock.makefile("rb")
        status = reader.readline().decode("ascii").strip()
        self.assertTrue(status.startswith("HTTP/1.1 101"), status)
        while reader.readline().decode("ascii").strip():
            pass
        peer = Peer.__new__(Peer)
        peer.sock, peer.reader = sock, reader
        peer.login(auth["account_id"], auth["access_token"], resource="vanilla")
        self.addCleanup(lambda: (reader.close(), sock.close()))
        return peer

    def test_discovery_progression_return_to_lobby_and_requeue(self):
        alice = self.login("DiscoveryAlice")
        aid, token = alice["account_id"], alice["access_token"]
        self.assertEqual(mcp(self.base, token, aid, "ClientQuestLogin")[0], 200)

        sid1, _ = self.register_server()
        self.assertEqual(self.matchmake(aid), sid1)
        self.assertEqual(jcall(self.base, "POST", f"/fortnite/api/matchmaking/session/{sid1}/join?accountId={aid}")[0], 204)
        self.assertEqual(jcall(self.base, "POST", f"/local/gameserver/{sid1}/state", token=self.secret,
                               body={"phase": "inprogress", "numPlayers": 1, "joinable": False,
                                     "connectedPlayerIds": [aid]})[0], 204)

        # Case-insensitive discovery event persists and syncs via player endpoint.
        status, ev, _ = jcall(self.base, "POST", f"/local/gameserver/{sid1}/quest-event", token=self.secret,
                              body={"accountId": aid, "eventId": "poi-1",
                                    "event": "visit_apollo_location_beachybluffs"})
        self.assertEqual((status, ev), (200, {"accepted": True}))

        # Submit match result with Playlist_DefaultSolo and Battle Pass tier unlock.
        status, res, _ = jcall(self.base, "POST", f"/local/gameserver/{sid1}/result", token=self.secret,
                               body={"accountId": aid, "playlist": "Playlist_DefaultSolo",
                                     "kills": 2, "placement": 1, "xp": 30000,
                                     "endedAt": "2026-01-01T00:00:00Z"})
        self.assertEqual(status, 200, res)

        # Client returns to lobby and server resets to warmup with zero players.
        self.assertEqual(jcall(self.base, "DELETE", f"/fortnite/api/matchmaking/session/{sid1}", token=token)[0], 204)
        self.assertEqual(jcall(self.base, "POST", f"/local/gameserver/{sid1}/state", token=self.secret,
                               body={"phase": "warmup", "numPlayers": 0, "joinable": True,
                                     "connectedPlayerIds": []})[0], 204)

        # Requeue onto the reset server, then verify deregister + fresh server requeue.
        self.assertEqual(self.matchmake(aid), sid1)
        self.assertEqual(jcall(self.base, "POST", f"/fortnite/api/matchmaking/session/{sid1}/join?accountId={aid}")[0], 204)
        status, player, _ = jcall(self.base, "GET", f"/local/gameserver/{sid1}/player/{aid}", token=self.secret)
        self.assertEqual(status, 200, player)
        self.assertIn("Visit_Apollo_Location_BeachyBluffs", player["discoveredLocations"])

        self.assertEqual(jcall(self.base, "DELETE", f"/local/gameserver/{sid1}", token=self.secret)[0], 204)
        sid2, _ = self.register_server()
        self.assertEqual(self.matchmake(aid), sid2)

        self.restart()
        alice2 = self.login("DiscoveryAlice")
        stats = jcall(self.base, "GET", f"/statsproxy/api/statsv2/account/{aid}",
                      token=alice2["access_token"])[1]["stats"]
        self.assertEqual(stats["br_kills_keyboardmouse_m0_playlist_defaultsolo"], 2)
        profile = mcp(self.base, alice2["access_token"], aid, "QueryProfile")[1]["profileChanges"][0]["profile"]
        self.assertEqual(profile["stats"]["attributes"]["book_level"], 3)

    def test_squad_readiness_playlist_xmpp_presets_currency_and_cloudstorage(self):
        cap = self.login("SquadCaptain")
        mate = self.login("SquadMate")
        cid, ctok = cap["account_id"], cap["access_token"]
        mid, mtok = mate["account_id"], mate["access_token"]

        peer_cap = self.xmpp_peer(cap)
        peer_mate = self.xmpp_peer(mate)

        # Create party, join mate, select playlist, toggle readiness, verify XMPP notifications.
        party = jcall(self.base, "POST", "/party/api/v1/Fortnite/parties", token=ctok,
                      body={"join_info": {"connection": {"id": "cap-conn"}}})[1]
        pid = party["id"]
        self.assertEqual(jcall(self.base, "POST", f"/party/api/v1/Fortnite/parties/{pid}/members/{mid}/join",
                               token=mtok, body={"connection": {"id": "mate-conn"}})[0], 200)
        self.assertEqual(jcall(self.base, "PATCH", f"/party/api/v1/Fortnite/parties/{pid}", token=ctok,
                               body={"revision": 2, "meta": {"update": {
                                   "Default:PlaylistData_j": json.dumps({"PlaylistData": {"playlistName": "Playlist_DefaultDuo"}})
                               }}})[0], 204)
        self.assertEqual(jcall(self.base, "PATCH", f"/party/api/v1/Fortnite/parties/{pid}/members/{mid}/meta",
                               token=mtok, body={"revision": 0, "update": {"Default:GameReadiness_s": "Ready"}})[0], 204)

        seen_types = []
        for _ in range(4):
            frame = peer_cap.recv()
            body = ET.fromstring(frame).find("{jabber:client}body")
            if body is not None:
                seen_types.append(json.loads(body.text)["type"])
        self.assertIn("com.epicgames.social.party.notification.v0.PARTY_UPDATED", seen_types)
        self.assertIn("com.epicgames.social.party.notification.v0.MEMBER_STATE_UPDATED", seen_types)
        mate_types = []
        for _ in range(3):
            frame = peer_mate.recv()
            body = ET.fromstring(frame).find("{jabber:client}body")
            if body is not None:
                mate_types.append(json.loads(body.text)["type"])
        self.assertIn("com.epicgames.social.party.notification.v0.PARTY_UPDATED", mate_types)
        self.assertIn("com.epicgames.social.party.notification.v0.MEMBER_STATE_UPDATED", mate_types)

        sid, key = self.register_server(playlists=("Playlist_DefaultDuo",))
        self.assertEqual(self.matchmake(cid, playlist="Playlist_DefaultDuo", party_ids=[cid, mid]), sid)
        for acct in (cid, mid):
            k = jcall(self.base, "GET", f"/fortnite/api/game/v2/matchmaking/account/{acct}/session/{sid}")[1]
            self.assertEqual(k["key"], key)
            p = jcall(self.base, "GET", f"/local/gameserver/{sid}/player/{acct}", token=self.secret)[1]
            self.assertEqual(p["partyId"], pid)
            self.assertEqual(set(p["partyMembers"]), {cid, mid})

        # Presets, random loadout toggle, pinned quests, party assist, currency, and cloudstorage DELETE.
        self.assertEqual(mcp(self.base, ctok, cid, "CopyCosmeticLoadout",
                             body={"sourceIndex": 0, "targetIndex": 1, "optNewNameForTarget": "DuoPreset"})[0], 200)
        self.assertEqual(mcp(self.base, ctok, cid, "SetRandomCosmeticLoadout", body={})[0], 200)
        self.assertEqual(mcp(self.base, ctok, cid, "SetPinnedQuests", body={"pinnedQuestIds": ["q1"]})[0], 200)
        self.assertEqual(mcp(self.base, ctok, cid, "SetPartyAssistQuest",
                             body={"questToPinAsPartyAssist": "q1"})[0], 200)
        self.assertEqual(mcp(self.base, ctok, cid, "PurchaseCatalogEntry", profile="common_core",
                             body={"offerId": "local-free-credits", "purchaseQuantity": 1,
                                   "currency": "MtxCurrency", "currencySubType": "", "expectedTotalPrice": 0})[0], 200)

        cloud_path = f"/fortnite/api/cloudstorage/user/{cid}/ClientSettings.Sav"
        self.assertEqual(call(self.base, "PUT", cloud_path, token=ctok, body=b"cfg-v1")[0], 204)

        self.restart()
        cap2 = self.login("SquadCaptain")
        ctok2 = cap2["access_token"]
        athena = mcp(self.base, ctok2, cid, "QueryProfile")[1]["profileChanges"][0]["profile"]
        attrs = athena["stats"]["attributes"]
        self.assertTrue(attrs["use_random_loadout"])
        self.assertEqual(attrs["client_settings"]["pinnedQuestInstances"], ["q1"])
        self.assertEqual(attrs["party_assist_quest"], "q1")
        self.assertEqual(len(attrs["loadouts"]), 2)
        self.assertEqual(mcp(self.base, ctok2, cid, "SetActiveLoadoutIndex", body={"index": 1})[0], 200)
        athena2 = mcp(self.base, ctok2, cid, "QueryProfile")[1]["profileChanges"][0]["profile"]
        self.assertFalse(athena2["stats"]["attributes"]["use_random_loadout"])

        self.assertEqual(call(self.base, "GET", cloud_path, token=ctok2)[2], b"cfg-v1")
        self.assertEqual(call(self.base, "DELETE", cloud_path, token=ctok2)[0], 204)
        self.assertEqual(call(self.base, "GET", cloud_path, token=ctok2)[0], 404)


if __name__ == "__main__":
    import unittest
    unittest.main()
