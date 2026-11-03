"""Canonical support-source bundle; no generated Native files are edited here."""
from collections import Counter
import hashlib
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parent
MODULE = "FortniteGame"
DEPENDENCIES = ("Core", "GameplayTags")
HEADER = "FortNativeGameplayTagsRuntime.h"


def render_registration(events):
    lines = ["// Full original constructor order, including helper 0x141ec8c00."]
    for event in events:
        kind = event["kind"]
        if kind == "tag":
            if "map_offset" in event:
                custom = "true" if event["map_offset"] == "0x8d0" else "false"
                line = f'    RegisterMovement({event["offset"]}, {json.dumps(event["name"])}, {event["value"]}, {custom});'
            else:
                line = f'    Register({event["offset"]}, {json.dumps(event["name"])});'
        elif kind == "map":
            line = f'    TagValues{event["map_offset"][2:]}.Add(TagsByOriginalOffset[{event["tag_offset"]} / 8], {event["value"]});'
        elif kind == "done":
            line = "    UGameplayTagsManager::Get().DoneAddingNativeTags();"
        elif kind == "container":
            line = f'    Tags{event["container_offset"][2:]}.AddTag(TagsByOriginalOffset[{event["tag_offset"]} / 8]);'
        else:
            raise ValueError(f"Unclosed constructor effect: {kind}")
        lines.append(line + " // " + event["call"])
    return "\n".join(lines) + "\n"


def sources():
    raw = (ROOT / "constructor_events.json").read_bytes()
    if hashlib.sha256(raw).hexdigest() != "833f573636717591a39379de141741b516ce0604bf5b66c8620dc1d87ef375aa":
        raise ValueError("Original constructor event stream changed; re-close source policy")
    events = json.loads(raw)
    if Counter(e["kind"] for e in events) != {"tag": 1136, "map": 37, "done": 1, "container": 7}:
        raise ValueError("Incomplete constructor event stream")
    tags = [e for e in events if e["kind"] == "tag"]
    if len({e["offset"] for e in tags}) != 1136:
        raise ValueError("Tag field ownership changed")
    done = [i for i, e in enumerate(events) if e["kind"] == "done"]
    if done != [1173] or any(e["kind"] != "container" for e in events[1174:]):
        raise ValueError("Native registration/finalization/container order changed")
    rendered = render_registration(events)
    if (ROOT / "FortNativeGameplayTagsRegistration.inc").read_text() != rendered:
        raise ValueError("Registration include differs from the recovered event stream")
    return {
        HEADER: (ROOT / HEADER).read_text(),
        "FortNativeGameplayTagsRuntime.cpp": (ROOT / "FortNativeGameplayTagsRuntime.cpp").read_text(),
        "FortNativeGameplayTagsRegistration.inc": rendered,
    }


if __name__ == "__main__":
    print(json.dumps({"module": MODULE, "dependencies": DEPENDENCIES, "sources": list(sources())}))
