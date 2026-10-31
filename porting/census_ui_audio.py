#!/usr/bin/env python3
"""Regenerate the Frontend/lobby UI and audio census from local files.

Reads only data already on disk:
  porting/generated/map-native-imports.jsonl   /Script imports per map
  porting/generated/map-scan.jsonl             import/export counts per map
  porting/generated/asset-registry-summary.txt asset class counts
  porting/generated/FortniteGame.uproject      enabled plugins and modules
  porting/generated/extracted/                 recovered cooked packages
  porting/generated/pak-sources.tsv            package -> winning pak (optional; written by
                                               `Cue4Inspect --pak-sources --all-keys --key-file=<path>`)
  porting/generated/widget-blueprints.txt      registry WidgetBlueprint packages (optional; written from
                                               `Cue4Inspect --registry-paths`, filtered to class WidgetBlueprint)
  porting/native/Source/                       project source already written
  <UE engine>/Source, <UE engine>/Plugins     engine headers (class index)
  <UE engine>/Config/Unix/UnixEngine.ini      Linux audio backend selection
  porting/generated/linux-stage/...            staged binary (byte search only)
  host /proc entries and ldconfig              audio devices and libraries

The loose cue4inspect pass reads only recovered packages under
porting/generated/extracted/. It never needs or prompts for an AES key, and it
never reads the pak files. Commands that need the AES key are listed in
porting/ui_audio_census.md.

Outputs (git-ignored):
  porting/generated/ui-audio-census.json
  porting/generated/ui-audio/package-imports.jsonl

Usage:
  python3 porting/census_ui_audio.py                  # full run
  python3 porting/census_ui_audio.py --skip-tool      # reuse cached package imports
  python3 porting/census_ui_audio.py --skip-engine-index
"""

from __future__ import annotations

import argparse
import collections
import concurrent.futures
import json
import mmap
import os
import re
import shutil
import subprocess
import sys
from pathlib import Path

PORTING = Path(__file__).resolve().parent
REPO = PORTING.parent
GEN = PORTING / "generated"
EXTRACTED = GEN / "extracted"
CUE4INSPECT_DLL = PORTING / "cue4inspect/bin/Release/net10.0/Cue4Inspect.dll"
ENGINE = Path(os.environ.get("UE_ENGINE_DIR", REPO.parent / "UnrealEngine" / "Engine"))
STAGED_BINARY = GEN / "linux-stage/LinuxNoEditor/FortniteLinux/Binaries/Linux/FortniteLinux"
SDL2_ARCHIVE = ENGINE / "Source/ThirdParty/SDL2/SDL-gui-backend/lib/Linux/x86_64-unknown-linux-gnu/libSDL2.a"
OUT_DIR = GEN / "ui-audio"
OUT_JSON = GEN / "ui-audio-census.json"
PKG_IMPORTS = OUT_DIR / "package-imports.jsonl"

FRONTEND_MAP_RE = re.compile(r"^FortniteGame/Content/(Maps/(Frontend|FrontEnd|UI/)|FrontEnd/)", re.IGNORECASE)
WIDGET_PREFIXES = ("FortniteGame/Content/UI/",)
UI_MODULES = {"FortniteUI", "CommonUI", "UMG", "Slate", "SlateCore", "NetUI", "SocialUMG", "SubtitlesWidgets"}
UI_NAME_RE = re.compile(r"(Widget|HUD|Hud|Menu|Frontend|FrontEnd|Lobby|Banner|Panel|Screen|Activat|Button|Dialog|Popup|Context|Loadout|Store|UI)")  # case-sensitive: "ui" inside "Building" must not match
AUDIO_PLUGIN_NAMES = {
    "AudioSynesthesia", "AudioCapture", "AmbientAudio", "AudioModulation", "ResonanceAudio",
    "SoundCueTemplates", "FortSoundCueTemplates", "SoundFields", "SoundUtilities",
    "VoiceChatManager", "GVoiceChat", "LiveStreamVoice", "VoiceEmbeddedCommunication",
    "Synthesis", "TimeSynth", "MfMedia", "MediaCompositing",
}
UI_PLUGIN_NAMES = {"CommonUI", "NetUI", "SocialUMG", "SubtitlesWidgets", "UMG", "ImagePlate", "Paper2D"}
CLASS_DECL_RE = re.compile(r"\bclass\s+(?:[A-Z0-9_]+_API\s+)?([UA][A-Za-z0-9_]+)\s*(?::|final\b|\{)")  # UObject and AActor types
SOUND_CLASSES = {"SoundClass", "SoundMix", "SoundCue", "SoundWave", "SoundAttenuation", "AudioComponent", "SoundBase", "SoundNodeWavePlayer", "SoundNodeRandom", "SoundNodeModulator", "SoundNodeMixer", "SoundNodeLooping", "SoundNodeDelay"}
SERVER_PROCESSES = {"pulseaudio", "pipewire", "pipewire-pulse", "wireplumber", "jackd", "pipewire-media-session"}


def log(message: str) -> None:
    print(message, file=sys.stderr)


def read_text(path: str | Path) -> str | None:
    try:
        return Path(path).read_text(encoding="utf-8", errors="replace").strip()
    except OSError:
        return None


# ---------------------------------------------------------------- local inputs

def read_jsonl(path: Path):
    """Yield JSON objects, skipping the 'Mounted N pak(s)' header the tool prints."""
    if not path.exists():
        log(f"missing input: {path}")
        return
    with path.open(encoding="utf-8", errors="replace") as handle:
        for line in handle:
            if line.startswith("{"):
                yield json.loads(line)


def load_maps() -> dict[str, dict]:
    maps: dict[str, dict] = {}
    for row in read_jsonl(GEN / "map-native-imports.jsonl"):
        maps.setdefault(row["path"], {})["native"] = row.get("nativeImports", [])
    for row in read_jsonl(GEN / "map-scan.jsonl"):
        entry = maps.setdefault(row["path"], {"native": []})
        entry.update(
            imports=row.get("imports", 0),
            exports=row.get("exports", 0),
            asset_imports=row.get("assetImports", 0),
            script_modules=row.get("scriptModules", []),
        )
    return maps


def load_uproject() -> tuple[list[dict], list[dict]]:
    data = json.loads((GEN / "FortniteGame.uproject").read_text(encoding="utf-8"))
    plugins = [{"name": p["Name"], "enabled": bool(p.get("Enabled"))} for p in data.get("Plugins", [])]
    return plugins, data.get("Modules", [])


def index_engine_plugins() -> dict[str, Path]:
    """Plugin name -> .uplugin path for every plugin in the engine tree."""
    found: dict[str, Path] = {}
    root = ENGINE / "Plugins"
    for dirpath, _dirs, files in os.walk(root):
        for name in files:
            if name.endswith(".uplugin"):
                found.setdefault(name[: -len(".uplugin")], Path(dirpath) / name)
    return found


def plugin_modules(uplugin: Path | None) -> list[str]:
    if uplugin is None:
        return []
    try:
        data = json.loads(uplugin.read_text(encoding="utf-8"))
    except (OSError, ValueError):
        return []
    return [m["Name"] for m in data.get("Modules", []) if m.get("Name")]


def pak_listing_scope() -> dict | None:
    """Scope of UI and sound content in the 28 mounted paks, from the existing listing (no pak reads)."""
    path = GEN / "assets.json"
    if not path.exists():
        return None
    data = json.loads(path.read_text(encoding="utf-8"))
    files = [f for pak in data.get("paks", []) for f in pak.get("files", [])]
    def under(*needles: str) -> list[str]:
        return [f for f in files if any(n in f for n in needles)]
    def count(paths: list[str], ext: str) -> int:
        return sum(1 for f in paths if f.endswith(ext))
    widgets = under("/Content/UI/", "/Content/Maps/UI/")
    frontend_maps = under("/Content/Maps/Frontend", "/Content/Maps/FrontEnd/", "/Content/FrontEnd/")
    sounds = under("/Content/Sounds/", "/Content/Athena/Sounds/")
    return {
        "mounted_paks": data.get("indexed_paks"),
        "skipped_paks": [s.get("pak") for s in data.get("skipped", [])],
        "total_files": data.get("total_files"),
        "ui_uasset_files": count(widgets, ".uasset"),
        "ui_umap_files": count(widgets, ".umap"),
        "frontend_umap_files": count(frontend_maps, ".umap"),
        "sound_uasset_files": count(sounds, ".uasset"),
        "sound_uasset_by_folder": dict(sorted(collections.Counter(f.rsplit("/", 1)[0].split("/Content/")[1] for f in sounds if f.endswith(".uasset")).items(), key=lambda kv: -kv[1])[:12]),
    }


def registry_count(class_name: str) -> int | None:
    for line in (read_text(GEN / "asset-registry-summary.txt") or "").splitlines():
        if line.startswith(class_name + ":"):
            return int(line.split(":", 1)[1])
    return None


# ---------------------------------------------------------------- class index

def provider_for(path: Path, root: Path) -> str:
    """Human-readable home of a class: engine module, engine plugin, or project source."""
    rel = path.relative_to(root).parts
    if root == REPO / "porting/native/Source":
        return f"project-source:porting/native/{rel[0]}" if rel else "project-source:porting/native"
    if root == ENGINE / "Plugins":
        if "Source" in rel:
            src = rel.index("Source")
            plugin = rel[src - 1] if src >= 1 else rel[0]
            module = rel[src + 1] if src + 1 < len(rel) else ""
            return f"plugin:{plugin}/{module}"
        return f"plugin:{rel[-1] if rel else ''}"
    # engine Source/<Runtime|Developer|Editor>/<Module>/...
    category = rel[0] if rel else ""
    module = rel[1] if len(rel) > 1 else ""
    return f"engine:{category}/{module}"


def index_native_classes(skip: bool) -> dict[str, str]:
    """Class name without its U/A prefix -> provider, from engine, plugin and project headers.

    ThirdParty trees are skipped: they declare generic names (for example ICU's
    'Object') that would falsely satisfy Unreal class imports."""
    index: dict[str, str] = {}
    if skip:
        return index
    for root in (ENGINE / "Source", ENGINE / "Plugins", REPO / "porting/native/Source"):
        for dirpath, dirs, files in os.walk(root):
            dirs[:] = [d for d in dirs if d != "ThirdParty"]
            for name in files:
                if not name.endswith(".h"):
                    continue
                path = Path(dirpath) / name
                try:
                    text = path.read_text(encoding="utf-8", errors="ignore")
                except OSError:
                    continue
                provider = provider_for(path, root)
                for match in CLASS_DECL_RE.finditer(text):
                    index.setdefault(match.group(1)[1:], provider)
    return index


class Classifier:
    """Decide whether a native import resolves to engine source, a plugin, or nothing local."""

    def __init__(self, class_index: dict[str, str], engine_plugins: dict[str, Path], enabled: set[str]):
        self.index = class_index
        self.engine_plugins = engine_plugins
        self.enabled = enabled
        self.engine_modules = set()
        for sub in ("Runtime", "Developer", "Editor"):
            base = ENGINE / "Source" / sub
            if base.exists():
                self.engine_modules.update(d.name for d in base.iterdir() if d.is_dir())

    def module_status(self, module: str) -> str:
        if module in self.engine_modules:
            return "engine module"
        if module in self.engine_plugins:
            return "plugin enabled in engine tree" if module in self.enabled else "plugin disabled in engine tree"
        if module in ("FortniteGame", "FortniteUI", "FortniteEngineLoadingScreen", "FortnitePatchCheck"):
            return "project module, source not recovered"
        if module in self.enabled:
            return "plugin absent from engine tree (enabled in uproject)"
        return "module not in engine or project"

    def class_status(self, module: str, name: str) -> str:
        provider = self.index.get(name)
        if provider is not None:
            return f"present: {provider}"
        return f"MISSING class; module {self.module_status(module)}"


def resolved(status: str) -> bool:
    """True when the import has a local definition (engine source, enabled plugin, or porting source)."""
    return status.startswith(("present", "engine module", "plugin enabled"))


def parse_tool_imports(text: str) -> list[dict]:
    rows = []
    for line in text.splitlines():
        parts = [p.strip() for p in line.split("|")]
        if len(parts) == 3 and parts[0].startswith("/Script/"):
            rows.append({"module": parts[0][len("/Script/"):], "type": parts[1], "name": parts[2]})
    return rows


def parse_tool_export_counts(text: str) -> dict[str, int]:
    counts: dict[str, int] = {}
    for line in text.splitlines():
        match = re.match(r"^  (\w+): (\d+)$", line)
        if match:
            counts[match.group(1)] = int(match.group(2))
    return counts


def run_tool_one(package: str) -> dict:
    base = ["dotnet", str(CUE4INSPECT_DLL), package, "--loose"]
    imports = subprocess.run(base + ["--imports"], capture_output=True, text=True, timeout=300)
    exports = subprocess.run(base, capture_output=True, text=True, timeout=300)
    return {
        "package": package,
        "ok": imports.returncode == 0 and exports.returncode == 0,
        "imports": parse_tool_imports(imports.stdout),
        "export_types": parse_tool_export_counts(exports.stdout),
    }


def recovered_packages() -> list[str]:
    """Package paths (no extension) for every recovered .uasset; classification happens on imports later."""
    return sorted(path.relative_to(EXTRACTED).as_posix()[: -len(".uasset")] for path in EXTRACTED.rglob("*.uasset"))


def run_tool_pass(skip: bool) -> list[dict]:
    if skip:
        return [json.loads(line) for line in read_text(PKG_IMPORTS).splitlines()] if PKG_IMPORTS.exists() else []
    if not CUE4INSPECT_DLL.exists() or shutil.which("dotnet") is None:
        log("cue4inspect DLL or dotnet missing; loose pass skipped")
        return []
    packages = recovered_packages()
    log(f"loose cue4inspect pass over {len(packages)} recovered packages (imports only; no key)")
    with concurrent.futures.ThreadPoolExecutor(max_workers=8) as pool:
        rows = list(pool.map(run_tool_one, packages))
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    with PKG_IMPORTS.open("w", encoding="utf-8") as handle:
        for row in rows:
            handle.write(json.dumps(row, sort_keys=True) + "\n")
    return rows


# ---------------------------------------------------------------- host and backend

def host_audio_probe() -> dict:
    sound_servers = set()
    proc_root = Path("/proc")
    for proc in proc_root.iterdir():
        if proc.name.isdigit():
            comm = read_text(proc / "comm")
            if comm in SERVER_PROCESSES:
                sound_servers.add(comm)
    ldconf = subprocess.run(["ldconfig", "-p"], capture_output=True, text=True).stdout if shutil.which("ldconfig") else ""
    libs = sorted({line.split("=>")[0].strip().split(" ")[0] for line in ldconf.splitlines()
                   if re.search(r"SDL2|openal|libasound|libpulse|pipewire|libjack", line)})
    pcm = read_text("/proc/asound/pcm")
    dev_snd = sorted(p.name for p in Path("/dev/snd").iterdir()) if Path("/dev/snd").exists() else []
    return {
        "proc_asound_cards": read_text("/proc/asound/cards"),
        "proc_asound_pcm": pcm if pcm else "(empty: no PCM playback or capture device registered)",
        "dev_snd_nodes": dev_snd,
        "dev_snd_pcm_nodes": [name for name in dev_snd if name.startswith("pcm")],
        "running_sound_servers": sorted(sound_servers),
        "SDL_AUDIODRIVER_env": os.environ.get("SDL_AUDIODRIVER"),
        "shared_libs_present": libs,
    }


def archive_members(path: Path) -> list[str]:
    """Member names from a GNU ar archive, read without invoking ar."""
    names: list[str] = []
    if not path.exists():
        return names
    with path.open("rb") as handle:
        if handle.read(8) != b"!<arch>\n":
            return names
        extended = b""
        while True:
            header = handle.read(60)
            if len(header) < 60:
                break
            raw_name = header[:16].decode("ascii", "replace").rstrip()
            size = int(header[48:58].decode("ascii").strip())
            body = handle.read(size)
            if size % 2:
                handle.read(1)
            if raw_name == "//":
                extended = body
            elif raw_name == "/":
                continue
            elif raw_name.startswith("/") and raw_name[1:].isdigit():
                start = int(raw_name[1:])
                names.append(extended[start:].split(b"/\n", 1)[0].decode("ascii", "replace"))
            else:
                names.append(raw_name.rstrip("/"))
    return names


def staged_needed_libs(path: Path) -> list[str]:
    """DT_NEEDED entries from the ELF dynamic section (readelf reads, never executes)."""
    if not path.exists() or shutil.which("readelf") is None:
        return []
    out = subprocess.run(["readelf", "-d", str(path)], capture_output=True, text=True).stdout
    return re.findall(r"\(NEEDED\)\s+Shared library: \[([^\]]+)\]", out)


def binary_contains(path: Path, needles: list[bytes]) -> dict[str, bool]:
    result = {}
    with path.open("rb") as handle, mmap.mmap(handle.fileno(), 0, access=mmap.ACCESS_READ) as mem:
        for needle in needles:
            result[needle.decode()] = mem.find(needle) != -1
    return result


def engine_audio_backend() -> dict:
    section: dict[str, str] = {}
    current = None
    for line in (read_text(ENGINE / "Config/Unix/UnixEngine.ini") or "").splitlines():
        if line.startswith("["):
            current = line.strip("[]")
        elif "=" in line and current == "Audio":
            key, value = line.split("=", 1)
            section[key.strip()] = value.strip()
    members = archive_members(SDL2_ARCHIVE)
    driver_objects = sorted(m for m in members if re.search(r"(alsa|pulseaudio|dummy|disk|jack|pipewire|sndio|nas|sun|paudio)_?audio|_audio\.c\.o", m, re.IGNORECASE) and "audio" in m.lower())
    staged = {}
    if STAGED_BINARY.exists():
        staged = binary_contains(STAGED_BINARY, [b"SDL_OpenAudioDevice", b"SDL_AUDIODRIVER", b"SDL_DISKAUDIOFILE", b"AudioMixerSDL", b"libSDL2", b"libopenal", b"libasound"])
    sdl_version = read_text(ENGINE / "Source/ThirdParty/SDL2/SDL-gui-backend/include/SDL_version.h") or ""
    version = re.findall(r"define SDL_(MAJOR_VERSION|MINOR_VERSION|PATCHLEVEL)\s+(\d+)", sdl_version)
    return {
        "unix_engine_ini_audio_section": section,
        "sdl2_archive": str(SDL2_ARCHIVE) if SDL2_ARCHIVE.exists() else None,
        "sdl2_version": ".".join(v for _k, v in version) if version else None,
        "sdl2_driver_objects": driver_objects,
        "staged_binary_has": staged,
        "staged_binary_needed_libs": staged_needed_libs(STAGED_BINARY),
        "engine_openal_linux_libopenal_bundled": (ENGINE / "Binaries/ThirdParty/OpenAL/Linux/x86_64-unknown-linux-gnu/libopenal.so").exists(),
    }


# ---------------------------------------------------------------- census

def build_frontend(maps: dict[str, dict], clf: Classifier) -> tuple[list[dict], list[dict], list[dict], list[dict]]:
    """Return (frontend maps, UI-module imports in frontend maps, all imports in frontend maps, UI imports everywhere)."""
    frontend_maps = {p: m for p, m in maps.items() if FRONTEND_MAP_RE.search(p)}
    ui_refs: collections.Counter = collections.Counter()
    all_ui_refs: collections.Counter = collections.Counter()
    frontend_all: collections.Counter = collections.Counter()
    for path, entry in maps.items():
        for imp in entry.get("native", []):
            ref, _, kind = imp.rpartition("|")
            module, _, name = ref.partition(".")
            module = module.replace("/Script/", "")
            key = (module, name, kind)
            if path in frontend_maps:
                frontend_all[key] += 1
            if module in UI_MODULES or (module == "FortniteGame" and UI_NAME_RE.search(name)):
                all_ui_refs[key] += 1
                if path in frontend_maps:
                    ui_refs[key] += 1

    def row(key, count):
        module, name, kind = key
        status = clf.class_status(module, name) if kind == "Class" else clf.module_status(module)
        return {"module": module, "name": name, "kind": kind, "maps": count, "status": status}

    frontend_native = sorted((row(k, c) for k, c in ui_refs.items()),
                             key=lambda r: (resolved(r["status"]), -r["maps"], r["module"], r["name"]))
    frontend_all_native = sorted((row(k, c) for k, c in frontend_all.items()),
                                 key=lambda r: (resolved(r["status"]), -r["maps"], r["module"], r["name"]))
    all_native = sorted((row(k, c) for k, c in all_ui_refs.items()),
                        key=lambda r: (-r["maps"], r["module"], r["name"]))
    frontend_list = []
    for path, entry in sorted(frontend_maps.items()):
        frontend_list.append({
            "path": path,
            "imports": entry.get("imports", 0),
            "exports": entry.get("exports", 0),
            "asset_imports": entry.get("asset_imports", 0),
            "script_modules": entry.get("script_modules", []),
            "ui_native_refs": sum(1 for imp in entry.get("native", []) if imp.rpartition("|")[0].split(".")[0] in ("/Script/FortniteUI", "/Script/CommonUI", "/Script/UMG")),
        })
    frontend_list.sort(key=lambda r: (-r["asset_imports"], r["path"]))
    return frontend_list, frontend_native, frontend_all_native, all_native


def is_widget(row: dict) -> bool:
    return row["package"].startswith(WIDGET_PREFIXES) or any(i["module"] in UI_MODULES for i in row["imports"])


def is_sound(row: dict) -> bool:
    return "/Sounds/" in row["package"] or any(i["type"] == "Class" and i["module"] == "Engine" and i["name"] in SOUND_CLASSES for i in row["imports"])


def build_widgets(pkg_rows: list[dict], clf: Classifier) -> tuple[list[dict], list[dict]]:
    packages = []
    counts: collections.Counter = collections.Counter()
    for row in pkg_rows:
        if not is_widget(row):
            continue
        native = [imp for imp in row["imports"] if not imp["name"].startswith("Default__") and imp["type"] != "Package"]
        for imp in native:
            counts[(imp["module"], imp["name"], imp["type"])] += 1
        packages.append({"package": row["package"], "ok": row["ok"], "native_imports": len(native)})
    rows = []
    for (module, name, kind), count in counts.items():
        status = clf.class_status(module, name) if kind == "Class" else clf.module_status(module)
        rows.append({"module": module, "name": name, "type": kind, "packages": count, "status": status})
    rows.sort(key=lambda r: (resolved(r["status"]), -r["packages"], r["module"], r["name"]))
    return packages, rows


def build_audio(pkg_rows: list[dict], clf: Classifier) -> dict:
    sound_rows = [r for r in pkg_rows if is_sound(r)]
    folders: collections.Counter = collections.Counter()
    exports: collections.Counter = collections.Counter()
    classes: collections.Counter = collections.Counter()
    for row in sound_rows:
        folders[row["package"].rsplit("/", 1)[0]] += 1
        for typ, count in row["export_types"].items():
            exports[typ] += count
        for imp in row["imports"]:
            if imp["type"] == "Class" and not imp["name"].startswith("Default__"):
                classes[(imp["module"], imp["name"])] += 1
    native = sorted(
        ({"module": m, "class": n, "packages": c, "status": clf.class_status(m, n)} for (m, n), c in classes.items()),
        key=lambda r: (resolved(r["status"]), -r["packages"], r["module"], r["class"]),
    )
    return {
        "recovered_sound_packages": len(sound_rows),
        "packages_by_folder": dict(folders.most_common()),
        "export_types": dict(exports.most_common()),
        "native_classes": native,
        "registry_soundwave_count": registry_count("SoundWave"),
    }


def map_modules(entry: dict) -> set[str]:
    """Modules a map touches: its scriptModules plus the module of every native import."""
    modules = set(entry.get("script_modules", []))
    for imp in entry.get("native", []):
        modules.add(imp.rpartition("|")[0].split(".")[0].replace("/Script/", ""))
    return modules


def build_plugins(enabled: list[str], engine_plugins: dict[str, Path], evidence: dict[str, set[str]], maps: dict[str, dict]) -> dict:
    rows = []
    for name in enabled:
        uplugin = engine_plugins.get(name)
        modules = plugin_modules(uplugin) or [name]
        names = set(modules) | {name}
        map_hits = sum(1 for path, entry in maps.items() if names & map_modules(entry))
        ui_use = sorted(names & evidence["frontend"])
        audio_use = sorted(names & evidence["audio"])
        relevant = bool(ui_use or audio_use or name in AUDIO_PLUGIN_NAMES or name in UI_PLUGIN_NAMES)
        rows.append({
            "name": name,
            "present_in_engine": uplugin is not None,
            "engine_dir": str(uplugin.parent.relative_to(ENGINE.parent)) if uplugin else None,
            "declared_modules": modules if uplugin else [],
            "maps_importing_module": map_hits,
            "used_by_frontend_maps": ui_use,
            "used_by_audio_packages": audio_use,
            "ui_or_audio_relevant": relevant,
        })
    rows.sort(key=lambda r: (r["present_in_engine"], not r["ui_or_audio_relevant"], -r["maps_importing_module"], r["name"]))
    return {
        "enabled": len(enabled),
        "present_in_engine": sum(1 for r in rows if r["present_in_engine"]),
        "absent_from_engine": sorted(r["name"] for r in rows if not r["present_in_engine"]),
        "rows": rows,
    }


def pak_attribution(pkg_rows: list[dict], widget_packages: list[dict]) -> dict:
    """Count recovered packages by the pak that supplies them (pak-sources.tsv, optional)."""
    index_path = GEN / "pak-sources.tsv"
    if not index_path.exists():
        return {"available": False, "reason": "run Cue4Inspect --pak-sources to write porting/generated/pak-sources.tsv"}
    source = {}
    for line in read_text(index_path).splitlines():
        package, _, pak = line.partition("\t")
        if pak:
            source[package] = pak
    recovered = collections.Counter(source.get(row["package"], "unknown") for row in pkg_rows)
    widgets = collections.Counter(source.get(row["package"], "unknown") for row in widget_packages)
    return {
        "available": True,
        "index_rows": len(source),
        "recovered_packages_by_pak": dict(recovered.most_common()),
        "widget_packages_by_pak": dict(widgets.most_common()),
        "pakchunk1000_1002_recovered": {pak: recovered.get(pak, 0) for pak in (
            "pakchunk1000-WindowsClient.pak", "pakchunk1001-WindowsClient.pak", "pakchunk1002-WindowsClient.pak")},
    }


def widget_presence() -> dict:
    """Registry WidgetBlueprints present under porting/generated/extracted, by folder.

    The list comes from `Cue4Inspect --registry-paths` (porting/generated/widget-blueprints.txt holds the
    registry rows whose class is WidgetBlueprint and whose package resolves to a present .uasset)."""
    listing = GEN / "widget-blueprints.txt"
    if not listing.exists():
        return {"available": False, "reason": "write porting/generated/widget-blueprints.txt from --registry-paths"}
    packages = [line.strip() for line in read_text(listing).splitlines() if line.strip()]
    present = [p for p in packages if (EXTRACTED / (p + ".uasset")).exists()]
    present_set = set(present)
    def folder(package: str) -> str:
        parts = package.split("/")
        return "/".join(parts[2:4]) if len(parts) > 3 else package
    return {
        "available": True,
        "registry_widget_blueprints": len(packages),
        "present_locally": len(present),
        "missing_locally": len(packages) - len(present),
        "present_by_folder": dict(collections.Counter(folder(p) for p in present).most_common()),
        "missing_by_folder": dict(collections.Counter(folder(p) for p in packages if p not in present_set).most_common()),
    }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--skip-tool", action="store_true", help="reuse the cached package imports instead of running cue4inspect")
    parser.add_argument("--skip-engine-index", action="store_true", help="skip the engine header class index")
    parser.add_argument("--out", type=Path, default=OUT_JSON)
    args = parser.parse_args()

    maps = load_maps()
    plugins, modules = load_uproject()
    enabled = [p["name"] for p in plugins if p["enabled"]]
    engine_plugins = index_engine_plugins()
    clf = Classifier(index_native_classes(args.skip_engine_index), engine_plugins, set(enabled))
    log(f"maps {len(maps)}; enabled plugins {len(enabled)}; class index {len(clf.index)}")

    frontend_list, frontend_native, frontend_all_native, all_native = build_frontend(maps, clf)
    pkg_rows = run_tool_pass(args.skip_tool)
    widget_packages, widget_native = build_widgets(pkg_rows, clf)
    audio = build_audio(pkg_rows, clf)

    frontend_modules = set()
    for path, entry in maps.items():
        if FRONTEND_MAP_RE.search(path):
            frontend_modules.update(entry.get("script_modules", []))
    frontend_modules |= {r["module"] for r in frontend_all_native} | {r["module"] for r in widget_native}
    audio_modules = {r["module"] for r in audio["native_classes"]}
    plugins_out = build_plugins(enabled, engine_plugins, {"frontend": frontend_modules, "audio": audio_modules}, maps)

    report = {
        "inputs": {
            "map_count": len(maps),
            "frontend_map_count": len(frontend_list),
            "project_modules": [m["Name"] for m in modules],
            "enabled_plugins": len(enabled),
            "engine_root": str(ENGINE),
            "recovered_packages_scanned": len(pkg_rows),
            "widget_blueprint_registry_count": registry_count("WidgetBlueprint"),
        },
        "frontend_maps": frontend_list,
        "frontend_ui_native_classes": frontend_native,
        "frontend_native_classes_all": frontend_all_native,
        "all_ui_native_classes": all_native[:300],
        "widget_blueprints": {"recovered_packages": widget_packages, "native_imports": widget_native},
        "audio": audio,
        "plugins": plugins_out,
        "host_audio": host_audio_probe(),
        "engine_audio_backend": engine_audio_backend(),
        "pak_listing_scope": pak_listing_scope(),
        "pak_attribution": pak_attribution(pkg_rows, widget_packages),
        "widget_presence": widget_presence(),
    }
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(json.dumps(report, indent=1) + "\n", encoding="utf-8")
    log(f"wrote {args.out}")
    summarize(report)
    return 0


def summarize(report: dict) -> None:
    frontend_missing = [r for r in report["frontend_native_classes_all"] if not resolved(r["status"])]
    print(f"frontend maps: {report['inputs']['frontend_map_count']}")
    print(f"frontend native imports (all): {len(report['frontend_native_classes_all'])} ({len(frontend_missing)} unresolved)")
    for r in frontend_missing[:12]:
        print(f"  {r['maps']:>3} {r['module']}.{r['name']} [{r['kind']}]")
    print(f"widget BP packages recovered: {len(report['widget_blueprints']['recovered_packages'])}")
    print(f"sound packages recovered: {report['audio']['recovered_sound_packages']}")
    print(f"enabled plugins absent from engine: {len(report['plugins']['absent_from_engine'])}")
    print(f"engine Linux audio module: {report['engine_audio_backend']['unix_engine_ini_audio_section'].get('AudioDeviceModuleName')}")
    print(f"host PCM devices: {report['host_audio']['proc_asound_pcm']}")


if __name__ == "__main__":
    sys.exit(main())
