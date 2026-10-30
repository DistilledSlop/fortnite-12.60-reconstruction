#!/usr/bin/env python3
"""Write a hosts file that sends every Epic service hostname to loopback.

Hostnames come from the cooked DefaultEngine.ini plus names the client builds at runtime
(observed in client logs). Usage: gen_hosts.py DefaultEngine.ini out_hosts
"""
import re
import sys

EXTRA = [
    "datarouter.ol.epicgames.com",
    "epicgames.com",
    "fortnite-public-service-prod11.ol.epicgames.com",
    "fortnitecontent-website-prod07.ol.epicgames.com",
    "fortnitecontent-website-prod.ak.epicgames.com",
    "fortnite-vod.akamaized.net",
]

ini, out = sys.argv[1:3]
text = open(ini, encoding="utf-8", errors="ignore").read()
names = set(re.findall(r"[a-z0-9][a-z0-9.-]*\.(?:epicgames\.(?:com|net)|unrealengine\.com)", text, re.I))
names = sorted({n.lower() for n in names} | set(EXTRA))
with open(out, "w") as stream:
    stream.write("127.0.0.1 localhost\n")
    stream.writelines(f"127.0.0.1 {name}\n" for name in names)
print(f"wrote {len(names)} hosts to {out}")
