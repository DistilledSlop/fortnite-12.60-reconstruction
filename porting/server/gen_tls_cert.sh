#!/usr/bin/env bash
# Generate a self-signed cert whose SANs cover every host in the generated hosts file.
# Used as the server certificate; the client verification flag is cleared in memory by patch_curl_verify.py.
# Usage: gen_tls_cert.sh hosts_file out_dir
set -euo pipefail
hosts=$1; out=$2
mkdir -p "$out"
sans=$(awk '!/^#/ && NF>=2 {for(i=2;i<=NF;i++) print $i}' "$hosts" | sort -u | grep -v '^localhost$' |
       awk '{printf "%sDNS:%s", (NR>1?",":""), $0}')
sans="$sans,DNS:*.ol.epicgames.com,DNS:*.ol.epicgames.net,DNS:*.epicgames.com,DNS:*.unrealengine.com,IP:127.0.0.1"
openssl req -x509 -newkey rsa:2048 -nodes -days 365 -subj "/CN=local-fortnite-mock" \
    -addext "subjectAltName=$sans" -addext "basicConstraints=critical,CA:TRUE" \
    -keyout "$out/key.pem" -out "$out/cert.pem" 2>/dev/null
echo "wrote $out/cert.pem"
