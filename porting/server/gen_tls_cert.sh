#!/usr/bin/env bash
# Generate a self-signed cert whose wildcard SANs cover the Epic service hostnames.
# Install it into the Wine prefix with install_wine_root_cert.py so the client trusts it.
# Usage: gen_tls_cert.sh out_dir
set -euo pipefail
out=$1
mkdir -p "$out"
sans="DNS:*.ol.epicgames.com,DNS:*.ol.epicgames.net,DNS:*.ak.epicgames.com,DNS:*.epicgames.com,DNS:epicgames.com,DNS:*.unrealengine.com,DNS:*.akamaized.net,DNS:fnwp.vivox.com,DNS:launcher-resources.s3.amazonaws.com,IP:127.0.0.1"
openssl req -x509 -newkey rsa:2048 -nodes -days 365 -subj "/CN=local-fortnite-mock" \
    -addext "subjectAltName=$sans" -addext "basicConstraints=critical,CA:TRUE" \
    -keyout "$out/key.pem" -out "$out/cert.pem" 2>/dev/null
echo "wrote $out/cert.pem"
