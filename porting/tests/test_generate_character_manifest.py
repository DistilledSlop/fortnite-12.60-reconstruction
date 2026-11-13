#!/usr/bin/env python3
"""Focused unit tests for generate_character_manifest.py (synthetic fixtures)."""

from __future__ import annotations

import hashlib
import json
import pathlib
import sys
import tempfile
import unittest

TOOLS_DIR = pathlib.Path(__file__).resolve().parents[1] / "tools"
if str(TOOLS_DIR) not in sys.path:
    sys.path.insert(0, str(TOOLS_DIR))

import generate_character_manifest as gen  # noqa: E402


def make_row(
    *,
    package: str,
    asset: str,
    asset_class: str = "AthenaCharacterItemDefinition",
    primary_asset_type: str = "AthenaCharacter",
    primary_asset_name: str | None = None,
) -> dict:
    return {
        "package": package,
        "asset": asset,
        "assetClass": asset_class,
        "objectPath": f"{package}.{asset}",
        "tags": {
            "PrimaryAssetType": primary_asset_type,
            "PrimaryAssetName": asset if primary_asset_name is None else primary_asset_name,
        },
    }


def default_rows() -> list[dict]:
    return [
        make_row(
            package="/Game/Athena/Items/Cosmetics/Characters/CID_001_Athena_Commando_F_Default",
            asset="CID_001_Athena_Commando_F_Default",
        ),
        make_row(
            package="/Game/Athena/Items/Cosmetics/Characters/CID_136_Athena_Commando_M_StreetBasketball",
            asset="CID_136_Athena_Commando_M_StreetBasketball",
        ),
        make_row(
            package="/Game/Athena/Items/Cosmetics/Characters/CID_137_Athena_Commando_F_StreetBasketball",
            asset="CID_137_Athena_Commando_F_StreetBasketball",
        ),
    ]


class CharacterManifestTests(unittest.TestCase):
    def setUp(self) -> None:
        self._tmp = tempfile.TemporaryDirectory()
        self.root = pathlib.Path(self._tmp.name)
        self.registry_path = self.root / "AssetRegistry.bin"
        self.registry_bytes = b"synthetic-character-registry"
        self.registry_path.write_bytes(self.registry_bytes)
        self.registry_sha256 = hashlib.sha256(self.registry_bytes).hexdigest()
        self.content_dir = self.root / "Content"
        self.content_dir.mkdir()
        self.tagged_path = self.root / "tagged.json"

    def tearDown(self) -> None:
        self._tmp.cleanup()

    def write_tagged(self, rows: list[dict], *, sha256: str | None = None) -> None:
        payload = {
            "registryAssets": len(rows),
            "registrySha256": self.registry_sha256 if sha256 is None else sha256,
            "rows": rows,
        }
        self.tagged_path.write_text(json.dumps(payload), encoding="utf-8")

    def touch_uasset(self, package: str) -> None:
        rel = package.removeprefix("/Game/") + ".uasset"
        target = self.content_dir / rel
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_bytes(b"\xc1\x83\x2a\x9e")

    def test_deterministic_manifest_and_redirects(self) -> None:
        rows = default_rows()
        for r in rows:
            self.touch_uasset(r["package"])
        self.write_tagged(list(reversed(rows)))

        manifest, _, _, sha1_a = gen.build_manifest(
            tagged_path=self.tagged_path,
            registry_path=self.registry_path,
            content_dir=self.content_dir,
            expected_registry_sha256=self.registry_sha256,
            expected_count=3,
            require_packages_present=True,
        )
        self.write_tagged(rows)
        _, _, _, sha1_b = gen.build_manifest(
            tagged_path=self.tagged_path,
            registry_path=self.registry_path,
            content_dir=self.content_dir,
            expected_registry_sha256=self.registry_sha256,
            expected_count=3,
            expected_output_sha1=sha1_a,
            require_packages_present=True,
        )
        self.assertEqual(sha1_a, sha1_b)
        self.assertEqual(manifest["count"], 3)
        self.assertEqual([r["primaryAssetId"] for r in manifest["rows"]], [
            "AthenaCharacter:CID_001_Athena_Commando_F_Default",
            "AthenaCharacter:CID_136_Athena_Commando_M_StreetBasketball",
            "AthenaCharacter:CID_137_Athena_Commando_F_StreetBasketball",
        ])
        self.assertEqual(len(manifest["redirects"]), 2)

    def test_rejects_outside_configured_scan_dirs(self) -> None:
        rows = default_rows() + [
            make_row(package="/Game/Athena/Outside/CID_999_Test", asset="CID_999_Test")
        ]
        self.write_tagged(rows)
        with self.assertRaises(gen.ManifestError):
            gen.build_manifest(
                tagged_path=self.tagged_path,
                registry_path=self.registry_path,
                content_dir=self.content_dir,
                expected_registry_sha256=self.registry_sha256,
            )

    def test_rejects_case_insensitive_duplicate_id(self) -> None:
        rows = default_rows() + [
            make_row(
                package="/Game/Athena/Items/Cosmetics/Characters/cid_001_athena_commando_f_default_dup",
                asset="cid_001_athena_commando_f_default_dup",
                primary_asset_name="cid_001_athena_commando_f_default",
            )
        ]
        self.write_tagged(rows)
        with self.assertRaises(gen.ManifestError):
            gen.build_manifest(
                tagged_path=self.tagged_path,
                registry_path=self.registry_path,
                content_dir=self.content_dir,
                expected_registry_sha256=self.registry_sha256,
            )

    def test_rejects_missing_uasset_and_registry_hash_mismatch(self) -> None:
        self.write_tagged(default_rows())
        with self.assertRaises(gen.ManifestError):
            gen.build_manifest(
                tagged_path=self.tagged_path,
                registry_path=self.registry_path,
                content_dir=self.content_dir,
                expected_registry_sha256=self.registry_sha256,
                require_packages_present=True,
            )
        with self.assertRaises(gen.ManifestError):
            gen.build_manifest(
                tagged_path=self.tagged_path,
                registry_path=self.registry_path,
                content_dir=self.content_dir,
                expected_registry_sha256="0" * 64,
            )


if __name__ == "__main__":
    unittest.main()
