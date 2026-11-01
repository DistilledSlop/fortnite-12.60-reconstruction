#!/usr/bin/env python3
"""Recheck hash-only constructor/lifetime/signature evidence against the original image."""
import argparse
import hashlib
import json
from pathlib import Path


def verify(image, record):
    with image.open('rb') as source:
        if hashlib.file_digest(source, 'sha256').hexdigest() != record['image_sha256']:
            raise ValueError('original image hash mismatch')
        base = int(record['image_base'], 16)
        for span in record['spans']:
            start, end = int(span['start'], 16), int(span['end'], 16)
            if start < base or end <= start:
                raise ValueError('invalid evidence span: ' + span['name'])
            source.seek(start - base)
            data = source.read(end - start)
            if len(data) != end - start or hashlib.sha256(data).hexdigest() != span['sha256']:
                raise ValueError('original source span mismatch: ' + span['name'])
    return len(record['spans'])


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('image', type=Path)
    args = parser.parse_args()
    record = json.loads(Path(__file__).with_name('original-runtime.json').read_text())
    print('verified original source spans:', verify(args.image, record))
