#!/usr/bin/env python3
"""Generate a deterministic AthenaCharacter manifest from extracted registry tags.

Consumes CUE4Parse-extracted ``tagged.json`` (using UE4.25+ AssetRegistry
parsing), verifies the actual SHA-256 of ``AssetRegistry.bin``, validates
``AthenaCharacterItemDefinition`` and ``PrimaryAssetType=AthenaCharacter`` tags
inside the five configured AssetManagerSettings scan directories, includes the
two pinned ``PrimaryAssetIdRedirects`` from extracted ``DefaultGame.ini``,
rejects malformed/duplicate records, sorts canonically, and writes deterministic
JSON.
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

DEFAULT_REGISTRY_SHA256 = "8cdd9e893b9e043d45702fb285d312476105b448330493cbb872ba3d58fe47b0"
PRIMARY_ASSET_TYPE = "AthenaCharacter"
ASSET_CLASS = "AthenaCharacterItemDefinition"

# Configured directories and redirects from extracted FortniteGame/Config/DefaultGame.ini
# [/Script/Engine.AssetManagerSettings] PrimaryAssetTypesToScan + PrimaryAssetIdRedirects.
CONFIGURED_SCAN_DIRS = (
    "/Game/Athena/Items/Cosmetics/Characters",
    "/Game/Heroes/Commando/CosmeticCharacterItemDefinitions",
    "/Game/Heroes/Constructor/CosmeticCharacterItemDefinitions",
    "/Game/Heroes/Ninja/CosmeticCharacterItemDefinitions",
    "/Game/Heroes/Outlander/CosmeticCharacterItemDefinitions",
)
CONFIGURED_REDIRECTS = (
    ("AthenaCharacter:cid_136_athena_commando_m_basketball", "AthenaCharacter:cid_136_athena_commando_m_streetbasketball"),
    ("AthenaCharacter:cid_137_athena_commando_f_basketball", "AthenaCharacter:cid_137_athena_commando_f_streetbasketball"),
)

HEX_SHA256_RE = re.compile(r"^[0-9a-fA-F]{64}$")
HEX_SHA1_RE = re.compile(r"^[0-9a-fA-F]{40}$")


class ManifestError(ValueError):
    """Raised when input registry data or manifest invariants fail validation."""


def fold_ascii(value: str) -> str:
    """Fold ASCII uppercase letters A-Z to a-z without Unicode locale effects."""
    return "".join(chr(ord(c) + 32) if "A" <= c <= "Z" else c for c in value)


def compute_file_sha256(path: pathlib.Path) -> str:
    if not path.is_file():
        raise ManifestError(f"Registry file does not exist or is not a regular file: {path}")
    if path.stat().st_size <= 0:
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
    if not package.startswith("/Game/") or package.endswith("/"):
        raise ManifestError(f"{label} must start with '/Game/' and not end with '/': {package!r}")
    parts = package.split("/")[1:]
    if any(part in ("", ".", "..") for part in parts):
        raise ManifestError(f"{label} contains invalid path segments: {package!r}")


def _validate_tagged(tagged_data: Any, actual_registry_sha256: str) -> tuple[int, list[dict[str, Any]]]:
    if not isinstance(tagged_data, dict):
        raise ManifestError("tagged.json root must be a JSON object")
    registry_assets = tagged_data.get("registryAssets")
    if isinstance(registry_assets, bool) or not isinstance(registry_assets, int) or registry_assets <= 0:
        raise ManifestError(f"Invalid registryAssets count in tagged.json: {registry_assets!r}")
    for sha_key in ("registrySha256", "sha256"):
        if sha_key in tagged_data:
            embedded = _require_clean_str(tagged_data[sha_key], f"tagged.json.{sha_key}")
            if not HEX_SHA256_RE.fullmatch(embedded):
                raise ManifestError(f"Malformed {sha_key} in tagged.json: {embedded!r}")
            if embedded.lower() != actual_registry_sha256.lower():
                raise ManifestError(
                    f"tagged.json {sha_key} ({embedded.lower()}) != actual SHA-256 ({actual_registry_sha256.lower()})"
                )
    raw_rows = tagged_data.get("rows")
    if not isinstance(raw_rows, list) or not raw_rows:
        raise ManifestError("tagged.json 'rows' must be a non-empty list")
    return registry_assets, raw_rows


def build_manifest(
    *,
    tagged_path: pathlib.Path,
    registry_path: pathlib.Path,
    content_dir: pathlib.Path,
    source_label: str | None = None,
    expected_registry_sha256: str | None = DEFAULT_REGISTRY_SHA256,
    expected_count: int | None = None,
    expected_output_sha1: str | None = None,
    require_packages_present: bool = False,
) -> tuple[dict[str, Any], bytes, str, str]:
    if expected_registry_sha256 is not None and not HEX_SHA256_RE.fullmatch(expected_registry_sha256):
        raise ManifestError(f"Invalid expected_registry_sha256 format: {expected_registry_sha256!r}")
    if expected_output_sha1 is not None and not HEX_SHA1_RE.fullmatch(expected_output_sha1):
        raise ManifestError(f"Invalid expected_output_sha1 format: {expected_output_sha1!r}")
    if not content_dir.is_dir():
        raise ManifestError(f"Content directory does not exist: {content_dir}")

    actual_registry_sha256 = compute_file_sha256(registry_path)
    if expected_registry_sha256 is not None and actual_registry_sha256.lower() != expected_registry_sha256.lower():
        raise ManifestError(
            f"Registry SHA-256 mismatch: expected {expected_registry_sha256.lower()}, actual {actual_registry_sha256.lower()}"
        )

    if not tagged_path.is_file():
        raise ManifestError(f"tagged.json does not exist or is not a regular file: {tagged_path}")
    try:
        tagged_data = json.loads(tagged_path.read_text(encoding="utf-8"))
    except (OSError, UnicodeDecodeError, json.JSONDecodeError) as exc:
        raise ManifestError(f"Failed to read/parse tagged.json ({tagged_path}): {exc}") from exc

    registry_assets, raw_rows = _validate_tagged(tagged_data, actual_registry_sha256)

    scan_prefixes = [d + "/" for d in CONFIGURED_SCAN_DIRS]
    scan_prefixes_folded = [fold_ascii(p) for p in scan_prefixes]

    selected_rows: list[dict[str, str]] = []
    seen_primary_ids: set[str] = set()
    seen_object_paths: set[str] = set()

    for idx, row in enumerate(raw_rows):
        label = f"rows[{idx}]"
        if not isinstance(row, dict):
            raise ManifestError(f"{label} must be a JSON object")
        package = _require_clean_str(row.get("package"), f"{label}.package")
        _validate_package_path(package, f"{label}.package")
        asset = _require_clean_str(row.get("asset"), f"{label}.asset")
        asset_class = _require_clean_str(row.get("assetClass"), f"{label}.assetClass")
        object_path = _require_clean_str(row.get("objectPath"), f"{label}.objectPath")
        if object_path != f"{package}.{asset}":
            raise ManifestError(f"{label}.objectPath ({object_path!r}) != {package}.{asset!r}")

        tags = row.get("tags")
        if not isinstance(tags, dict):
            raise ManifestError(f"{label}.tags must be a JSON object")

        if asset_class != ASSET_CLASS:
            raise ManifestError(f"{label}: expected assetClass {ASSET_CLASS!r}, got {asset_class!r}")

        pat = tags.get("PrimaryAssetType")
        pan = tags.get("PrimaryAssetName")
        if pat != PRIMARY_ASSET_TYPE:
            raise ManifestError(f"{label}: expected PrimaryAssetType={PRIMARY_ASSET_TYPE!r}, got {pat!r}")
        pan_clean = _require_clean_str(pan, f"{label}.tags.PrimaryAssetName")

        in_exact = any(package.startswith(p) for p in scan_prefixes)
        in_ci = any(fold_ascii(package).startswith(p) for p in scan_prefixes_folded)
        if in_ci and not in_exact:
            raise ManifestError(f"{label}: package {package!r} case-mismatches configured scan directories")
        if not in_exact:
            raise ManifestError(f"{label}: package {package!r} is outside configured scan directories")

        primary_asset_id = f"{PRIMARY_ASSET_TYPE}:{pan_clean}"
        pid_key = fold_ascii(primary_asset_id)
        obj_key = fold_ascii(object_path)
        if pid_key in seen_primary_ids:
            raise ManifestError(f"Duplicate primaryAssetId (case-insensitive): {primary_asset_id!r}")
        if obj_key in seen_object_paths:
            raise ManifestError(f"Duplicate objectPath (case-insensitive): {object_path!r}")
        seen_primary_ids.add(pid_key)
        seen_object_paths.add(obj_key)

        if require_packages_present:
            rel = package.removeprefix("/Game/")
            uasset = (content_dir / (rel + ".uasset")).resolve()
            content_resolved = content_dir.resolve()
            if not uasset.is_relative_to(content_resolved) or not uasset.is_file():
                raise ManifestError(f"{label}: missing .uasset under content_dir for {package!r}")

        selected_rows.append({
            "primaryAssetId": primary_asset_id,
            "package": package,
            "asset": asset,
            "objectPath": object_path,
        })

    if not selected_rows:
        raise ManifestError("No AthenaCharacter rows selected")

    redirects = []
    seen_redirects: set[str] = set()
    for old_id, new_id in CONFIGURED_REDIRECTS:
        old_key = fold_ascii(old_id)
        new_key = fold_ascii(new_id)
        if old_key in seen_redirects:
            raise ManifestError(f"Duplicate redirect source: {old_id!r}")
        if new_key not in seen_primary_ids:
            raise ManifestError(f"Redirect target {new_id!r} missing from selected character rows")
        if old_key == new_key:
            raise ManifestError(f"Self-redirect detected: {old_id!r}")
        seen_redirects.add(old_key)
        redirects.append({"old": old_id, "new": new_id})

    selected_rows.sort(key=lambda r: (fold_ascii(r["primaryAssetId"]), r["objectPath"]))
    redirects.sort(key=lambda r: fold_ascii(r["old"]))

    if expected_count is not None and len(selected_rows) != expected_count:
        raise ManifestError(f"Manifest count mismatch: expected {expected_count}, actual {len(selected_rows)}")

    manifest: dict[str, Any] = {
        "schemaVersion": 1,
        "primaryAssetType": PRIMARY_ASSET_TYPE,
        "assetClass": ASSET_CLASS,
        "registryAssets": registry_assets,
        "registrySha256": actual_registry_sha256,
        "scanDirs": list(CONFIGURED_SCAN_DIRS),
        "redirects": redirects,
        "count": len(selected_rows),
        "rows": selected_rows,
    }
    if source_label:
        manifest["source"] = source_label

    payload_bytes = (json.dumps(manifest, indent=2) + "\n").encode("utf-8")
    output_sha1 = hashlib.sha1(payload_bytes).hexdigest()
    if expected_output_sha1 is not None and output_sha1.lower() != expected_output_sha1.lower():
        raise ManifestError(f"Output SHA-1 mismatch: expected {expected_output_sha1.lower()}, actual {output_sha1}")
    return manifest, payload_bytes, actual_registry_sha256, output_sha1


def _write_atomic(output_path: pathlib.Path, payload_bytes: bytes) -> None:
    output_path.parent.mkdir(parents=True, exist_ok=True)
    fd, tmp_name = tempfile.mkstemp(prefix=".character-manifest-", suffix=".tmp", dir=str(output_path.parent))
    tmp_path = pathlib.Path(tmp_name)
    try:
        with os.fdopen(fd, "wb") as handle:
            handle.write(payload_bytes)
            handle.flush()
            os.fsync(handle.fileno())
        os.replace(tmp_path, output_path)
    finally:
        if tmp_path.exists():
            tmp_path.unlink()


def parse_args(argv: list[str] | None = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Generate deterministic AthenaCharacter manifest from AssetRegistry tags.")
    parser.add_argument("--tagged", required=True, type=pathlib.Path)
    parser.add_argument("--registry", required=True, type=pathlib.Path)
    parser.add_argument("--content-dir", required=True, type=pathlib.Path)
    parser.add_argument("--output", required=True, type=pathlib.Path)
    parser.add_argument("--source", default="AssetRegistry.bin")
    parser.add_argument("--expected-registry-sha256", default=DEFAULT_REGISTRY_SHA256)
    parser.add_argument("--expected-count", type=int, default=None)
    parser.add_argument("--expected-output-sha1", default=None)
    parser.add_argument("--require-packages-present", action="store_true")
    return parser.parse_args(argv)


def main(argv: list[str] | None = None) -> int:
    args = parse_args(argv)
    try:
        _, payload_bytes, reg_sha256, out_sha1 = build_manifest(
            tagged_path=args.tagged,
            registry_path=args.registry,
            content_dir=args.content_dir,
            source_label=args.source,
            expected_registry_sha256=args.expected_registry_sha256,
            expected_count=args.expected_count,
            expected_output_sha1=args.expected_output_sha1,
            require_packages_present=args.require_packages_present,
        )
        _write_atomic(args.output, payload_bytes)
    except ManifestError as exc:
        print(f"ERROR: {exc}", file=sys.stderr)
        return 1
    manifest_summary = json.loads(payload_bytes.decode("utf-8"))
    print(f"Generated {args.output}: count={manifest_summary['count']}, registrySha256={reg_sha256}, sha1={out_sha1}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
