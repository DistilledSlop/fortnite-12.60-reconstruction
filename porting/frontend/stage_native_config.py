#!/usr/bin/env python3
"""Copy a committed native config into a stage, preserving auditable evidence."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import stat
import subprocess
import tempfile

EXPECTED = {
    ("/script/enginesettings.gamemapssettings", "gameinstanceclass"): "/Script/FortniteGame.FortGameInstance",
    ("/script/engine.engine", "gameviewportclientclassname"): "/Script/FortniteGame.FortGameViewportClient",
    ("/script/engine.engine", "localplayerclassname"): "/Script/FortniteGame.FortLocalPlayer",
}


def digest(data):
    return hashlib.sha256(data).hexdigest()


def settings(data):
    """Find exactly one occurrence of each requested setting in its section."""
    section = None
    found = {}
    for line in data.decode("utf-8-sig").splitlines():
        line = line.strip()
        if not line or line.startswith((";", "#")):
            continue
        if line.startswith("[") and line.endswith("]"):
            section = line[1:-1].strip().lower()
        elif "=" in line:
            key, value = line.split("=", 1)
            item = (section, key.strip().lower())
            if item in EXPECTED:
                if item in found:
                    raise ValueError("duplicate required config setting: " + item[1])
                found[item] = value.strip()
    if found != EXPECTED:
        raise ValueError("required class settings missing or incorrect")
    return {f"[{section}] {key}": value for (section, key), value in found.items()}


def git(repo, *args):
    return subprocess.check_output(["git", "-C", str(repo), *args], stderr=subprocess.PIPE)


def stage(source, target, out, source_commit, original_config=None):
    source, target, out = map(lambda p: Path(p).absolute(), (source, target, out))
    if source.is_symlink() or target.is_symlink() or not source.is_file() or not target.is_file():
        raise ValueError("source and target must be existing regular files, not symlinks")
    source, target, out = source.resolve(), target.resolve(), out.resolve()
    if source.name != "DefaultEngine.ini" or target.name != "DefaultEngine.ini":
        raise ValueError("source and target must be named DefaultEngine.ini")
    if source == target or out in source.parents or out in target.parents or out == source or out == target:
        raise ValueError("source and target must be distinct and outside evidence directory")
    if out.exists():
        raise ValueError("evidence directory already exists")
    repo = Path(git(source.parent, "rev-parse", "--show-toplevel").decode().strip()).resolve()
    relative = source.relative_to(repo).as_posix()
    commit = git(repo, "rev-parse", "--verify", source_commit + "^{commit}").decode().strip()
    committed = git(repo, "show", f"{commit}:{relative}")
    source_bytes = source.read_bytes()
    if source_bytes != committed:
        raise ValueError("source bytes differ from specified commit")
    proof = settings(source_bytes)
    original_hash = None
    if original_config is not None:
        original = Path(original_config).read_bytes()
        settings(original)
        original_hash = digest(original)
    before = target.read_bytes()
    mode = stat.S_IMODE(target.stat().st_mode)
    out.mkdir(parents=True, exist_ok=False)
    backup = out / "DefaultEngine.ini.before"
    backup.write_bytes(before)
    if digest(backup.read_bytes()) != digest(before):
        raise ValueError("backup verification failed")
    tmp = None
    try:
        with tempfile.NamedTemporaryFile(dir=target.parent, prefix=".DefaultEngine.ini.", delete=False) as f:
            tmp = Path(f.name)
            f.write(source_bytes)
            f.flush()
            os.fsync(f.fileno())
        tmp.chmod(mode)
        # Refuse to overwrite if the target changed during evidence capture.
        if target.read_bytes() != before:
            raise ValueError("target changed during staging")
        os.replace(tmp, target)
        tmp = None
    finally:
        if tmp is not None:
            tmp.unlink(missing_ok=True)
    after = target.read_bytes()
    if after != source_bytes:
        raise ValueError("staged config verification failed")
    receipt = {
        "schema": "native-config-stage-v1",
        "source": str(source), "target": str(target), "backup": str(backup),
        "source_commit": commit, "source_repo": str(repo), "source_repo_path": relative,
        "source_sha256": digest(source_bytes), "committed_source_sha256": digest(committed),
        "before_sha256": digest(before), "backup_sha256": digest(backup.read_bytes()),
        "after_sha256": digest(after), "required_settings": proof,
        "original_config_sha256": original_hash,
        "original_config_path": str(Path(original_config).resolve()) if original_config is not None else None,
        "original_settings_validated": original_config is not None,
        "scope": "config bytes only; effective runtime classes are not verified",
    }
    (out / "receipt.json").write_text(json.dumps(receipt, indent=2, sort_keys=True) + "\n")
    return receipt


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ("source", "target", "out", "source-commit"):
        parser.add_argument("--" + name, required=True)
    parser.add_argument("--original-config")
    args = parser.parse_args()
    try:
        receipt = stage(args.source, args.target, args.out, args.source_commit, args.original_config)
    except (ValueError, OSError, UnicodeError, subprocess.CalledProcessError) as exc:
        parser.exit(1, f"staging failed: {exc}\n")
    print(json.dumps({"receipt": str(Path(args.out).resolve() / "receipt.json"), "after_sha256": receipt["after_sha256"]}))


if __name__ == "__main__":
    main()
