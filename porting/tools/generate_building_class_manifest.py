#!/usr/bin/env python3
"""Generate a deterministic PlayerBuildingClass manifest from extracted registry tags.

Consumes the CUE4Parse-extracted ``tagged.json``, verifies the actual SHA-256
of ``AssetRegistry.bin``, validates required tags and BuildingActor parent-class
ancestry as evidence permits, filters to the configured recursive scan
directory boundary, rejects malformed or duplicate records, sorts by ASCII
case-insensitive full generated class path, and writes deterministic JSON.

Note: Extracted registry candidates do not prove original runtime load success,
dynamic scan additions, or final handle assignment parity.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import os
import pathlib
import re
import sys
import tempfile
from typing import Any

DEFAULT_SCAN_DIR = "/Game/Building/ActorBlueprints/Player"
PRIMARY_ASSET_TYPE = "PlayerBuildingClass"
CLASS_CONSTRAINT = "/Script/FortniteGame.BuildingActor"

# Verified direct/transitive native ABuildingActor subclasses from
# porting/native/Source/FortniteGame/Generated/ABuilding*.h.
VERIFIED_NATIVE_BUILDING_CLASSES = frozenset({
    "Class'/Script/FortniteGame.BuildingActor'",
    "Class'/Script/FortniteGame.BuildingSMActor'",
    "Class'/Script/FortniteGame.BuildingAutoNav'",
    "Class'/Script/FortniteGame.BuildingWall'",
    "Class'/Script/FortniteGame.BuildingFloor'",
    "Class'/Script/FortniteGame.BuildingRoof'",
    "Class'/Script/FortniteGame.BuildingStairs'",
    "Class'/Script/FortniteGame.BuildingPillar'",
})

HEX_SHA256_RE = re.compile(r"^[0-9a-fA-F]{64}$")
HEX_SHA1_RE = re.compile(r"^[0-9a-fA-F]{40}$")
NON_NEG_INT_RE = re.compile(r"^\d+$")


class ManifestError(ValueError):
    """Raised when input registry data or manifest invariants fail validation."""


def fold_ascii(value: str) -> str:
    """Fold ASCII uppercase letters A-Z to a-z without Unicode locale effects."""
    return "".join(chr(ord(c) + 32) if "A" <= c <= "Z" else c for c in value)


def compute_file_sha256(path: pathlib.Path) -> str:
    """Stream-compute lowercase hex SHA-256 of an existing non-empty regular file."""
    if not path.is_file():
        raise ManifestError(f"Registry file does not exist or is not a regular file: {path}")
    size = path.stat().st_size
    if size <= 0:
        raise ManifestError(f"Registry file is empty: {path}")
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1 << 20), b""):
            digest.update(chunk)
    return digest.hexdigest()


def _require_clean_str(value: Any, label: str) -> str:
    if not isinstance(value, str) or not value:
        raise ManifestError(f"{label} must be a non-empty string")
    if "\x00" in value or value != value.strip():
        raise ManifestError(f"{label} contains NUL or leading/trailing whitespace: {value!r}")
    return value


def _validate_package_path(package: str, label: str) -> None:
    if not package.startswith("/") or package.endswith("/"):
        raise ManifestError(f"{label} must start with '/' and not end with '/': {package!r}")
    parts = package.split("/")[1:]
    if any(part in ("", ".", "..") for part in parts):
        raise ManifestError(f"{label} contains invalid path segments: {package!r}")


def _normalize_scan_dir(scan_dir: str) -> str:
    cleaned = _require_clean_str(scan_dir, "scan_dir").rstrip("/")
    if not cleaned.startswith("/Game/") or cleaned == "/Game":
        raise ManifestError(f"scan_dir must be a subdirectory under /Game/: {scan_dir!r}")
    _validate_package_path(cleaned, "scan_dir")
    return cleaned


def _resolve_parent_to_building_actor(
    parent_ref: str,
    bp_parent_map: dict[str, str],
    row_label: str,
) -> str:
    """Verify ParentClass resolves to a known BuildingActor-derived native class."""
    current = parent_ref
    visited: set[str] = set()
    while True:
        if current.startswith("Class'") and current.endswith("'"):
            if current not in VERIFIED_NATIVE_BUILDING_CLASSES:
                raise ManifestError(
                    f"{row_label}: native ParentClass {current!r} is not in verified "
                    f"BuildingActor-derived classes"
                )
            return current
        if current.startswith("BlueprintGeneratedClass'") and current.endswith("'"):
            key = fold_ascii(current)
            if key in visited:
                raise ManifestError(f"{row_label}: cycle detected in ParentClass chain at {current!r}")
            visited.add(key)
            nxt = bp_parent_map.get(key)
            if nxt is None:
                raise ManifestError(
                    f"{row_label}: BlueprintGeneratedClass ParentClass {current!r} is not "
                    f"present in tagged.json to verify BuildingActor ancestry"
                )
            current = nxt
            continue
        raise ManifestError(f"{row_label}: malformed ParentClass tag {current!r}")


def _validate_and_index_tagged(
    tagged_data: Any,
    actual_registry_sha256: str,
) -> tuple[int, list[dict[str, Any]], dict[str, str]]:
    """Validate tagged.json structure and all rows; build Blueprint parent lookup."""
    if not isinstance(tagged_data, dict):
        raise ManifestError("tagged.json root must be a JSON object")

    registry_assets = tagged_data.get("registryAssets")
    if isinstance(registry_assets, bool) or not isinstance(registry_assets, int) or registry_assets <= 0:
        raise ManifestError(f"Invalid registryAssets count in tagged.json: {registry_assets!r}")

    for sha_key in ("registrySha256", "sha256"):
        if sha_key in tagged_data:
            embedded_sha = _require_clean_str(tagged_data[sha_key], f"tagged.json.{sha_key}")
            if not HEX_SHA256_RE.fullmatch(embedded_sha):
                raise ManifestError(f"Malformed {sha_key} in tagged.json: {embedded_sha!r}")
            if embedded_sha.lower() != actual_registry_sha256.lower():
                raise ManifestError(
                    f"tagged.json {sha_key} ({embedded_sha.lower()}) does not match actual "
                    f"registry SHA-256 ({actual_registry_sha256.lower()})"
                )

    raw_rows = tagged_data.get("rows")
    if not isinstance(raw_rows, list) or not raw_rows:
        raise ManifestError("tagged.json 'rows' must be a non-empty list")

    seen_object_paths: set[str] = set()
    bp_parent_map: dict[str, str] = {}

    for idx, row in enumerate(raw_rows):
        label = f"rows[{idx}]"
        if not isinstance(row, dict):
            raise ManifestError(f"{label} must be a JSON object")

        package = _require_clean_str(row.get("package"), f"{label}.package")
        _validate_package_path(package, f"{label}.package")
        asset = _require_clean_str(row.get("asset"), f"{label}.asset")
        _require_clean_str(row.get("assetClass"), f"{label}.assetClass")
        object_path = _require_clean_str(row.get("objectPath"), f"{label}.objectPath")

        expected_obj = f"{package}.{asset}"
        if object_path != expected_obj:
            raise ManifestError(
                f"{label}.objectPath ({object_path!r}) != expected {expected_obj!r}"
            )

        obj_key = fold_ascii(object_path)
        if obj_key in seen_object_paths:
            raise ManifestError(f"Duplicate objectPath (case-insensitive) in tagged.json: {object_path!r}")
        seen_object_paths.add(obj_key)

        tags = row.get("tags")
        if not isinstance(tags, dict):
            raise ManifestError(f"{label}.tags must be a JSON object")
        for tag_k, tag_v in tags.items():
            if not isinstance(tag_k, str) or not tag_k or not isinstance(tag_v, str):
                raise ManifestError(f"{label}.tags must map non-empty string keys to string values")

        buildable_raw = tags.get("bIsPlayerBuildable")
        if not isinstance(buildable_raw, str) or buildable_raw.casefold() not in ("true", "false"):
            raise ManifestError(
                f"{label}.tags.bIsPlayerBuildable must be 'True' or 'False', got {buildable_raw!r}"
            )

        if "GeneratedClass" in tags:
            gen_class = _require_clean_str(tags["GeneratedClass"], f"{label}.tags.GeneratedClass")
            parent_class = _require_clean_str(tags.get("ParentClass"), f"{label}.tags.ParentClass")
            gen_key = fold_ascii(gen_class)
            if gen_key in bp_parent_map:
                raise ManifestError(
                    f"Duplicate GeneratedClass (case-insensitive) in tagged.json: {gen_class!r}"
                )
            bp_parent_map[gen_key] = parent_class

    return registry_assets, raw_rows, bp_parent_map


def build_manifest(
    *,
    tagged_path: pathlib.Path,
    registry_path: pathlib.Path,
    content_dir: pathlib.Path,
    scan_dir: str = DEFAULT_SCAN_DIR,
    source_label: str | None = None,
    expected_registry_sha256: str | None = None,
    expected_count: int | None = None,
    expected_output_sha1: str | None = None,
    require_packages_present: bool = False,
) -> tuple[dict[str, Any], bytes, str, str]:
    """Validate inputs and construct deterministic manifest payload and UTF-8 bytes.

    Returns ``(manifest_dict, payload_bytes, actual_registry_sha256, output_sha1)``.
    """
    if expected_registry_sha256 is not None and not HEX_SHA256_RE.fullmatch(expected_registry_sha256):
        raise ManifestError(f"Invalid expected_registry_sha256 format: {expected_registry_sha256!r}")
    if expected_output_sha1 is not None and not HEX_SHA1_RE.fullmatch(expected_output_sha1):
        raise ManifestError(f"Invalid expected_output_sha1 format: {expected_output_sha1!r}")
    if not content_dir.is_dir():
        raise ManifestError(f"Content directory does not exist: {content_dir}")

    normalized_scan_dir = _normalize_scan_dir(scan_dir)
    scan_prefix = normalized_scan_dir + "/"
    scan_prefix_folded = fold_ascii(scan_prefix)

    actual_registry_sha256 = compute_file_sha256(registry_path)
    if expected_registry_sha256 is not None:
        if actual_registry_sha256.lower() != expected_registry_sha256.lower():
            raise ManifestError(
                f"Registry SHA-256 mismatch: expected {expected_registry_sha256.lower()}, "
                f"actual {actual_registry_sha256.lower()}"
            )

    if not tagged_path.is_file():
        raise ManifestError(f"tagged.json does not exist or is not a regular file: {tagged_path}")
    try:
        tagged_data = json.loads(tagged_path.read_text(encoding="utf-8"))
    except (OSError, UnicodeDecodeError, json.JSONDecodeError) as exc:
        raise ManifestError(f"Failed to read/parse tagged.json ({tagged_path}): {exc}") from exc

    registry_assets, raw_rows, bp_parent_map = _validate_and_index_tagged(
        tagged_data, actual_registry_sha256
    )

    selected_rows: list[dict[str, Any]] = []
    seen_class_paths: set[str] = set()
    seen_primary_names: set[str] = set()

    for idx, row in enumerate(raw_rows):
        label = f"rows[{idx}]"
        package = row["package"]
        asset = row["asset"]
        object_path = row["objectPath"]
        tags = row["tags"]

        is_buildable = tags["bIsPlayerBuildable"].casefold() == "true"
        in_scan_ci = fold_ascii(package).startswith(scan_prefix_folded)
        in_scan_exact = package.startswith(scan_prefix)

        if in_scan_ci and not in_scan_exact:
            raise ManifestError(
                f"{label}: package {package!r} matches scan directory case-insensitively "
                f"but not with exact case {scan_prefix!r}"
            )

        if not is_buildable:
            if in_scan_exact and tags.get("PrimaryAssetType") == PRIMARY_ASSET_TYPE:
                raise ManifestError(
                    f"{label}: row inside scan dir has PrimaryAssetType={PRIMARY_ASSET_TYPE!r} "
                    f"but bIsPlayerBuildable={tags['bIsPlayerBuildable']!r}"
                )
            continue

        if row["assetClass"] != "Blueprint":
            raise ManifestError(
                f"{label}: bIsPlayerBuildable==True requires assetClass 'Blueprint', got {row['assetClass']!r}"
            )

        class_path_candidate = f"{object_path}_C"
        expected_generated_class = f"BlueprintGeneratedClass'{class_path_candidate}'"
        actual_generated_class = tags.get("GeneratedClass")
        if actual_generated_class != expected_generated_class:
            raise ManifestError(
                f"{label}: GeneratedClass mismatch: expected {expected_generated_class!r}, "
                f"got {actual_generated_class!r}"
            )

        level_raw = tags.get("CurrentBuildingLevel")
        if not isinstance(level_raw, str) or not NON_NEG_INT_RE.fullmatch(level_raw):
            raise ManifestError(
                f"{label}: CurrentBuildingLevel must be a non-negative integer string, got {level_raw!r}"
            )
        level = int(level_raw)

        parent_class = _require_clean_str(tags.get("ParentClass"), f"{label}.tags.ParentClass")
        _resolve_parent_to_building_actor(parent_class, bp_parent_map, label)

        if not in_scan_exact:
            continue

        if not package.startswith("/Game/"):
            raise ManifestError(f"{label}: scanned package must start with '/Game/': {package!r}")

        primary_asset_type = tags.get("PrimaryAssetType")
        if primary_asset_type != PRIMARY_ASSET_TYPE:
            raise ManifestError(
                f"{label}: expected PrimaryAssetType {PRIMARY_ASSET_TYPE!r}, got {primary_asset_type!r}"
            )

        primary_asset_name = tags.get("PrimaryAssetName")
        if primary_asset_name != asset:
            raise ManifestError(
                f"{label}: PrimaryAssetName ({primary_asset_name!r}) != asset ({asset!r})"
            )

        class_path_key = fold_ascii(class_path_candidate)
        if class_path_key in seen_class_paths:
            raise ManifestError(
                f"{label}: duplicate classPathCandidate (case-insensitive): {class_path_candidate!r}"
            )
        seen_class_paths.add(class_path_key)

        primary_name_key = fold_ascii(primary_asset_name)
        if primary_name_key in seen_primary_names:
            raise ManifestError(
                f"{label}: duplicate PrimaryAssetName (case-insensitive): {primary_asset_name!r}"
            )
        seen_primary_names.add(primary_name_key)

        rel_uasset = pathlib.Path(package.removeprefix("/Game/")).with_suffix(".uasset")
        package_present = (content_dir / rel_uasset).is_file()
        if require_packages_present and not package_present:
            raise ManifestError(
                f"{label}: required .uasset missing under content dir: {content_dir / rel_uasset}"
            )

        selected_rows.append({
            **row,
            "classPathCandidate": class_path_candidate,
            "level": level,
            "generatedClassTagMatches": True,
            "packagePresent": package_present,
        })

    selected_rows.sort(key=lambda item: fold_ascii(item["classPathCandidate"]))

    if expected_count is not None and len(selected_rows) != expected_count:
        raise ManifestError(
            f"Expected {expected_count} manifest rows under {normalized_scan_dir}, got {len(selected_rows)}"
        )

    manifest = {
        "source": str(registry_path) if source_label is None else source_label,
        "sha256": actual_registry_sha256,
        "registryAssets": registry_assets,
        "filter": (
            "bIsPlayerBuildable == True AND default configured PlayerBuildingClass "
            "scan directory; unverified dynamic scans and successful original loads"
        ),
        "sort": (
            "ASCII case-insensitive full generated class path; candidates only, "
            "original successful class loads unverified"
        ),
        "classConstraint": CLASS_CONSTRAINT,
        "rows": selected_rows,
        "scope": "DefaultGame.ini:330, inherited UE 4.25 scan semantics; candidates, not final handles",
    }

    payload_bytes = (json.dumps(manifest, indent=2) + "\n").encode("utf-8")
    output_sha1 = hashlib.sha1(payload_bytes).hexdigest()

    if expected_output_sha1 is not None and output_sha1 != expected_output_sha1.lower():
        raise ManifestError(
            f"Generated manifest SHA-1 mismatch: expected {expected_output_sha1.lower()}, "
            f"actual {output_sha1}"
        )

    return manifest, payload_bytes, actual_registry_sha256, output_sha1


def _atomic_write_bytes(path: pathlib.Path, data: bytes) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    fd, tmp_name = tempfile.mkstemp(prefix=f".{path.name}.", dir=str(path.parent))
    try:
        with os.fdopen(fd, "wb") as handle:
            handle.write(data)
            handle.flush()
            os.fsync(handle.fileno())
        os.replace(tmp_name, path)
    except Exception:
        if os.path.exists(tmp_name):
            os.unlink(tmp_name)
        raise


def parse_args(argv: list[str] | None = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Generate deterministic PlayerBuildingClass manifest JSON from extracted tagged.json."
    )
    parser.add_argument("--tagged", required=True, type=pathlib.Path, help="Input tagged.json from CUE4Parse reader")
    parser.add_argument("--registry", required=True, type=pathlib.Path, help="Input AssetRegistry.bin to hash/verify")
    parser.add_argument("--content-dir", required=True, type=pathlib.Path, help="Extracted FortniteGame/Content dir")
    parser.add_argument("--output", required=True, type=pathlib.Path, help="Output manifest JSON path")
    parser.add_argument(
        "--scan-dir",
        default=DEFAULT_SCAN_DIR,
        help=f"Recursive UE package directory boundary (default: {DEFAULT_SCAN_DIR})",
    )
    parser.add_argument("--source-label", default=None, help="Override JSON 'source' field (defaults to --registry)")
    parser.add_argument("--expected-registry-sha256", default=None, help="Expected 64-hex SHA-256 of AssetRegistry.bin")
    parser.add_argument("--expected-count", type=int, default=None, help="Expected filtered row count")
    parser.add_argument("--expected-output-sha1", default=None, help="Expected 40-hex SHA-1 of generated JSON bytes")
    parser.add_argument("--roots-output", type=pathlib.Path, default=None, help="Optional newline-delimited package roots output")
    parser.add_argument(
        "--require-packages-present",
        action="store_true",
        help="Fail if any scanned row's .uasset is absent under --content-dir",
    )
    return parser.parse_args(argv)


def main(argv: list[str] | None = None) -> int:
    args = parse_args(argv)
    try:
        manifest, payload_bytes, registry_sha256, output_sha1 = build_manifest(
            tagged_path=args.tagged,
            registry_path=args.registry,
            content_dir=args.content_dir,
            scan_dir=args.scan_dir,
            source_label=args.source_label,
            expected_registry_sha256=args.expected_registry_sha256,
            expected_count=args.expected_count,
            expected_output_sha1=args.expected_output_sha1,
            require_packages_present=args.require_packages_present,
        )
        _atomic_write_bytes(args.output, payload_bytes)
        if args.roots_output is not None:
            roots_bytes = "".join(f"{row['package']}\n" for row in manifest["rows"]).encode("utf-8")
            _atomic_write_bytes(args.roots_output, roots_bytes)
    except ManifestError as exc:
        print(f"ERROR: {exc}", file=sys.stderr)
        return 1

    print(
        f"Wrote {args.output} ({len(manifest['rows'])} rows, {len(payload_bytes)} bytes, "
        f"sha1={output_sha1}, registry_sha256={registry_sha256})"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
