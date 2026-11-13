#!/usr/bin/env python3
"""Focused unit tests for generate_building_class_manifest.py (no 100MB fixtures)."""

from __future__ import annotations

import hashlib
import json
import pathlib
import subprocess
import sys
import tempfile
import unittest

TOOLS_DIR = pathlib.Path(__file__).resolve().parents[1] / "tools"
if str(TOOLS_DIR) not in sys.path:
    sys.path.insert(0, str(TOOLS_DIR))

import generate_building_class_manifest as gen  # noqa: E402


def make_row(
    *,
    package: str,
    asset: str,
    buildable: str = "True",
    parent: str = "Class'/Script/FortniteGame.BuildingWall'",
    level: str = "0",
    primary_asset_type: str | None = "PlayerBuildingClass",
    primary_asset_name: str | None = None,
    generated_class: str | None = None,
    asset_class: str = "Blueprint",
    object_path: str | None = None,
) -> dict:
    obj = f"{package}.{asset}" if object_path is None else object_path
    gen_cls = f"BlueprintGeneratedClass'{obj}_C'" if generated_class is None else generated_class
    tags = {
        "GeneratedClass": gen_cls,
        "ParentClass": parent,
        "BuildingType": "Wall",
        "bIsPlayerBuildable": buildable,
        "CurrentBuildingLevel": level,
    }
    if primary_asset_type is not None:
        tags["PrimaryAssetType"] = primary_asset_type
    if primary_asset_name is not None or primary_asset_type is not None:
        tags["PrimaryAssetName"] = asset if primary_asset_name is None else primary_asset_name
    return {
        "package": package,
        "asset": asset,
        "assetClass": asset_class,
        "objectPath": obj,
        "tags": tags,
    }


class BuildingManifestTests(unittest.TestCase):
    def setUp(self) -> None:
        self._tmp = tempfile.TemporaryDirectory()
        self.root = pathlib.Path(self._tmp.name)
        self.registry_path = self.root / "AssetRegistry.bin"
        self.registry_bytes = b"synthetic-ue-asset-registry-bytes"
        self.registry_path.write_bytes(self.registry_bytes)
        self.registry_sha256 = hashlib.sha256(self.registry_bytes).hexdigest()
        self.content_dir = self.root / "Content"
        self.content_dir.mkdir()
        self.tagged_path = self.root / "tagged.json"
        self.output_path = self.root / "out.json"

    def tearDown(self) -> None:
        self._tmp.cleanup()

    def _touch_uasset(self, package: str) -> None:
        rel = pathlib.Path(package.removeprefix("/Game/")).with_suffix(".uasset")
        target = self.content_dir / rel
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_bytes(b"uasset")

    def _write_tagged(self, rows: list, *, registry_assets: object = 42, extra: dict | None = None) -> None:
        payload: dict = {"registryAssets": registry_assets, "rows": rows}
        if extra:
            payload.update(extra)
        self.tagged_path.write_text(json.dumps(payload, indent=2), encoding="utf-8")

    def test_determinism_sort_and_directory_boundary(self) -> None:
        # Include mixed-case ASCII sort verification:
        # '_' (ASCII 95) must sort BEFORE 'a' (ASCII 97) after ASCII A-Z folding.
        pkg_a = "/Game/Building/ActorBlueprints/Player/Wood/A_Wall"
        pkg_aa = "/Game/Building/ActorBlueprints/Player/Wood/aa_wall"
        pkg_b = "/Game/Building/ActorBlueprints/Player/Wood/B_Wall"
        # Outside and similarly-prefixed paths must be excluded without error.
        pkg_similar = "/Game/Building/ActorBlueprints/Player2/Wood/P2_Wall"
        pkg_similar_under = "/Game/Building/ActorBlueprints/Player_Old/Wood/Old_Wall"
        pkg_outside = "/Game/Athena/Building/Outside_Wall"
        pkg_false = "/Game/Props/Prop_Chair"

        for pkg in (pkg_a, pkg_aa, pkg_b, pkg_similar, pkg_similar_under, pkg_outside):
            self._touch_uasset(pkg)

        rows = [
            make_row(package=pkg_b, asset="B_Wall", level="2", parent="Class'/Script/FortniteGame.BuildingRoof'"),
            make_row(package=pkg_aa, asset="aa_wall", level="1", parent="Class'/Script/FortniteGame.BuildingFloor'"),
            make_row(package=pkg_similar, asset="P2_Wall"),
            make_row(package=pkg_similar_under, asset="Old_Wall"),
            make_row(
                package=pkg_outside,
                asset="Outside_Wall",
                parent=f"BlueprintGeneratedClass'{pkg_a}.A_Wall_C'",
                primary_asset_type=None,
            ),
            make_row(
                package=pkg_false,
                asset="Prop_Chair",
                buildable="False",
                parent="Class'/Script/FortniteGame.BuildingProp'",
                primary_asset_type=None,
            ),
            make_row(package=pkg_a, asset="A_Wall", level="0"),
        ]
        self._write_tagged(rows, extra={"registrySha256": self.registry_sha256})

        _, bytes1, sha256_1, sha1_1 = gen.build_manifest(
            tagged_path=self.tagged_path,
            registry_path=self.registry_path,
            content_dir=self.content_dir,
            expected_registry_sha256=self.registry_sha256,
            expected_count=3,
            require_packages_present=True,
        )
        manifest2, bytes2, sha256_2, sha1_2 = gen.build_manifest(
            tagged_path=self.tagged_path,
            registry_path=self.registry_path,
            content_dir=self.content_dir,
            expected_registry_sha256=self.registry_sha256,
            expected_count=3,
            expected_output_sha1=sha1_1,
            require_packages_present=True,
        )

        self.assertEqual(bytes1, bytes2)
        self.assertEqual(sha256_1, self.registry_sha256)
        self.assertEqual(sha256_2, self.registry_sha256)
        self.assertEqual(sha1_1, sha1_2)
        self.assertNotIn(b"\r\n", bytes1)
        self.assertTrue(bytes1.endswith(b"\n"))
        self.assertEqual(
            [r["asset"] for r in manifest2["rows"]],
            ["A_Wall", "aa_wall", "B_Wall"],
        )
        self.assertEqual(
            list(manifest2.keys()),
            ["source", "sha256", "registryAssets", "filter", "sort", "classConstraint", "rows", "scope"],
        )

    def test_cli_invocation_writes_manifest_and_roots(self) -> None:
        pkg = "/Game/Building/ActorBlueprints/Player/Stone/PBWA_S1_Solid"
        self._touch_uasset(pkg)
        self._write_tagged([make_row(package=pkg, asset="PBWA_S1_Solid")])
        roots_path = self.root / "roots.txt"

        proc = subprocess.run(
            [
                sys.executable,
                "-I",
                str(TOOLS_DIR / "generate_building_class_manifest.py"),
                "--tagged",
                str(self.tagged_path),
                "--registry",
                str(self.registry_path),
                "--content-dir",
                str(self.content_dir),
                "--output",
                str(self.output_path),
                "--roots-output",
                str(roots_path),
                "--expected-registry-sha256",
                self.registry_sha256,
                "--expected-count",
                "1",
                "--require-packages-present",
            ],
            capture_output=True,
            text=True,
            check=False,
        )
        self.assertEqual(proc.returncode, 0, msg=proc.stderr)
        self.assertTrue(self.output_path.is_file())
        self.assertEqual(roots_path.read_text(encoding="utf-8"), f"{pkg}\n")

    def test_registry_sha256_verification_failures(self) -> None:
        pkg = "/Game/Building/ActorBlueprints/Player/Wood/W1"
        self._touch_uasset(pkg)
        self._write_tagged([make_row(package=pkg, asset="W1")])

        with self.assertRaises(gen.ManifestError):
            gen.build_manifest(
                tagged_path=self.tagged_path,
                registry_path=self.registry_path,
                content_dir=self.content_dir,
                expected_registry_sha256="0" * 64,
            )

        self._write_tagged([make_row(package=pkg, asset="W1")], extra={"registrySha256": "f" * 64})
        with self.assertRaises(gen.ManifestError):
            gen.build_manifest(
                tagged_path=self.tagged_path,
                registry_path=self.registry_path,
                content_dir=self.content_dir,
            )

        empty_reg = self.root / "empty.bin"
        empty_reg.write_bytes(b"")
        with self.assertRaises(gen.ManifestError):
            gen.build_manifest(
                tagged_path=self.tagged_path,
                registry_path=empty_reg,
                content_dir=self.content_dir,
            )

    def test_malformed_records_fail_closed(self) -> None:
        pkg = "/Game/Building/ActorBlueprints/Player/Wood/W1"
        self._touch_uasset(pkg)

        cases = [
            # Invalid buildable flag even on an unrelated non-scan row.
            [make_row(package=pkg, asset="W1"), make_row(package="/Game/Props/X", asset="X", buildable="maybe", primary_asset_type=None)],
            # Missing GeneratedClass on buildable row.
            [{"package": pkg, "asset": "W1", "assetClass": "Blueprint", "objectPath": f"{pkg}.W1", "tags": {"ParentClass": "Class'/Script/FortniteGame.BuildingWall'", "PrimaryAssetType": "PlayerBuildingClass", "PrimaryAssetName": "W1", "bIsPlayerBuildable": "True", "CurrentBuildingLevel": "0"}}],
            # Mismatched GeneratedClass.
            [make_row(package=pkg, asset="W1", generated_class=f"BlueprintGeneratedClass'{pkg}.Wrong_C'")],
            # Mismatched objectPath.
            [make_row(package=pkg, asset="W1", object_path=f"{pkg}.Other")],
            # Invalid level.
            [make_row(package=pkg, asset="W1", level="-1")],
            [make_row(package=pkg, asset="W1", level="abc")],
            # Wrong assetClass.
            [make_row(package=pkg, asset="W1", asset_class="DataAsset")],
            # Wrong PrimaryAssetType or PrimaryAssetName inside scan dir.
            [make_row(package=pkg, asset="W1", primary_asset_type="OtherType")],
            [make_row(package=pkg, asset="W1", primary_asset_name="DifferentName")],
            # Conflicting PrimaryAssetType on non-buildable row inside scan dir.
            [make_row(package=pkg, asset="W1", buildable="False")],
            # Case-divergent scan directory path must not be silently dropped.
            [make_row(package="/game/building/actorblueprints/player/Wood/W1", asset="W1")],
        ]

        for bad_rows in cases:
            self._write_tagged(bad_rows)
            with self.subTest(bad_rows=bad_rows):
                with self.assertRaises(gen.ManifestError):
                    gen.build_manifest(
                        tagged_path=self.tagged_path,
                        registry_path=self.registry_path,
                        content_dir=self.content_dir,
                    )

        # Bad registryAssets bool / zero.
        self._write_tagged([make_row(package=pkg, asset="W1")], registry_assets=True)
        with self.assertRaises(gen.ManifestError):
            gen.build_manifest(
                tagged_path=self.tagged_path,
                registry_path=self.registry_path,
                content_dir=self.content_dir,
            )

    def test_parent_class_constraints(self) -> None:
        base_pkg = "/Game/Building/ActorBlueprints/Player/Wood/BaseWall"
        child_pkg = "/Game/Building/ActorBlueprints/Player/Wood/ChildWall"
        self._touch_uasset(base_pkg)
        self._touch_uasset(child_pkg)

        # Multi-hop BlueprintGeneratedClass parent within tagged.json resolves cleanly.
        self._write_tagged([
            make_row(package=base_pkg, asset="BaseWall", parent="Class'/Script/FortniteGame.BuildingPillar'"),
            make_row(package=child_pkg, asset="ChildWall", parent=f"BlueprintGeneratedClass'{base_pkg}.BaseWall_C'"),
        ])
        manifest, _, _, _ = gen.build_manifest(
            tagged_path=self.tagged_path,
            registry_path=self.registry_path,
            content_dir=self.content_dir,
        )
        self.assertEqual(len(manifest["rows"]), 2)

        # Disallowed native parent fails closed.
        self._write_tagged([
            make_row(package=base_pkg, asset="BaseWall", parent="Class'/Script/Engine.Actor'"),
        ])
        with self.assertRaises(gen.ManifestError):
            gen.build_manifest(
                tagged_path=self.tagged_path,
                registry_path=self.registry_path,
                content_dir=self.content_dir,
            )

        # Unresolved BlueprintGeneratedClass parent fails closed.
        self._write_tagged([
            make_row(
                package=child_pkg,
                asset="ChildWall",
                parent="BlueprintGeneratedClass'/Game/Missing/Parent.Parent_C'",
            ),
        ])
        with self.assertRaises(gen.ManifestError):
            gen.build_manifest(
                tagged_path=self.tagged_path,
                registry_path=self.registry_path,
                content_dir=self.content_dir,
            )

        # Cyclic BlueprintGeneratedClass parent chain fails closed.
        self._write_tagged([
            make_row(package=base_pkg, asset="BaseWall", parent=f"BlueprintGeneratedClass'{child_pkg}.ChildWall_C'"),
            make_row(package=child_pkg, asset="ChildWall", parent=f"BlueprintGeneratedClass'{base_pkg}.BaseWall_C'"),
        ])
        with self.assertRaises(gen.ManifestError):
            gen.build_manifest(
                tagged_path=self.tagged_path,
                registry_path=self.registry_path,
                content_dir=self.content_dir,
            )

    def test_duplicate_detection(self) -> None:
        pkg1 = "/Game/Building/ActorBlueprints/Player/Wood/WallOne"
        pkg1_ci = "/Game/Building/ActorBlueprints/Player/wood/wallone"
        pkg2 = "/Game/Building/ActorBlueprints/Player/Stone/WallOne"

        # Case-insensitive duplicate objectPath / classPathCandidate.
        self._write_tagged([
            make_row(package=pkg1, asset="WallOne"),
            make_row(package=pkg1_ci, asset="wallone"),
        ])
        with self.assertRaises(gen.ManifestError):
            gen.build_manifest(
                tagged_path=self.tagged_path,
                registry_path=self.registry_path,
                content_dir=self.content_dir,
            )

        # Distinct package paths with duplicate PrimaryAssetName inside scan dir.
        self._write_tagged([
            make_row(package=pkg1, asset="WallOne"),
            make_row(package=pkg2, asset="wallone", primary_asset_name="wallone"),
        ])
        with self.assertRaises(gen.ManifestError):
            gen.build_manifest(
                tagged_path=self.tagged_path,
                registry_path=self.registry_path,
                content_dir=self.content_dir,
            )

    def test_package_present_and_require_flag(self) -> None:
        pkg = "/Game/Building/ActorBlueprints/Player/Metal/M1"
        self._write_tagged([make_row(package=pkg, asset="M1")])

        manifest, _, _, _ = gen.build_manifest(
            tagged_path=self.tagged_path,
            registry_path=self.registry_path,
            content_dir=self.content_dir,
            require_packages_present=False,
        )
        self.assertFalse(manifest["rows"][0]["packagePresent"])

        with self.assertRaises(gen.ManifestError):
            gen.build_manifest(
                tagged_path=self.tagged_path,
                registry_path=self.registry_path,
                content_dir=self.content_dir,
                require_packages_present=True,
            )


if __name__ == "__main__":
    unittest.main()
