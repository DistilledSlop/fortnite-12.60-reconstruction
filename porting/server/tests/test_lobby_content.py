#!/usr/bin/env python3
"""HTTP and configuration coverage for the local CMS service."""
import json
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from api_common import Context, Router
from backend import Backend
from content_service import ContentService
from test_services import LiveServer, call, jcall


class LobbyContentTest(unittest.TestCase):
    def start(self, **options):
        backend = Backend()
        backend.content = ContentService(backend.ctx, **options)
        backend.router = Router()
        backend.content.register(backend.router)
        server = LiveServer(backend).start()
        self.addCleanup(backend.close)
        self.addCleanup(server.stop)
        return backend, server.base

    def test_local_page_and_locale(self):
        backend, base = self.start()
        status, page, _ = jcall(base, "GET", "/content/api/pages/fortnite-game?locale=fr-FR")
        self.assertEqual(status, 200)
        self.assertEqual(page["_locale"], "fr-FR")
        self.assertIn("local", page["battleroyalenews"]["news"]["messages"][0]["body"])
        self.assertEqual(backend.content.lobby_content.provenance, "local-authored")
        self.assertEqual(jcall(base, "GET", "/content/api/pages/fortnite-game?locale=../bad")[0], 400)

    def test_snapshot_assets_and_copy_boundary(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            image = b"\x89PNG\r\n\x1a\nimage"
            (root / "banner.png").write_bytes(image)
            snapshot = {"_locale": "en-US", "battleroyalenews": {"news": {"messages": [
                {"title": "Supplied", "image": "local-asset:banner.png"}]}}}
            (root / "content.json").write_text(json.dumps(snapshot), encoding="utf-8")
            backend, base = self.start(content_dir=root, public_origin="http://127.0.0.1:8000")
            status, page, _ = jcall(base, "GET", "/content/api/pages/fortnite-game")
            self.assertEqual(status, 200)
            self.assertEqual(page["battleroyalenews"]["news"]["messages"][0]["image"],
                             "http://127.0.0.1:8000/content/api/assets/banner.png")
            status, headers, data = call(base, "GET", "/content/api/assets/banner.png")
            self.assertEqual((status, headers["Content-Type"], data), (200, "image/png", image))
            self.assertEqual(call(base, "GET", "/content/api/assets/../content.json")[0], 404)
            page["battleroyalenews"]["news"]["messages"].clear()
            again = backend.content.lobby_content.page("now")
            self.assertEqual(len(again["battleroyalenews"]["news"]["messages"]), 1)
            self.assertEqual(backend.content.lobby_content.provenance, "supplied-snapshot")

    def test_invalid_configuration(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            with self.assertRaises(ValueError):
                ContentService(Context(), content_dir=root)
            path = root / "content.json"
            path.write_text('[]', encoding="utf-8")
            with self.assertRaises(ValueError):
                ContentService(Context(), content_file=path)
            path.write_text('{"image":"local-asset:../secret.png"}', encoding="utf-8")
            with self.assertRaises(ValueError):
                ContentService(Context(), content_file=path, public_origin="http://localhost")
            with self.assertRaises(ValueError):
                ContentService(Context(), public_origin="https://example.com/path")
            with self.assertRaises(ValueError):
                ContentService(Context(), content_file=path, content_dir=root)


if __name__ == "__main__":
    unittest.main()
