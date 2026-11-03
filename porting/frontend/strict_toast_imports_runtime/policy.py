"""Evidence guard: deliberately refuses generation until toast closure is recovered."""
import hashlib
import json
import os
from pathlib import Path

HERE = Path(__file__).resolve().parent
OWNERS = {'UFortFriendSubscriptionToastWidget', 'UFortPlayerPowerRatingToast'}


def validate_source(image_path):
    evidence = json.loads((HERE / 'original-windows.json').read_text())
    base = int(evidence['image_base'], 16)
    fd = os.open(image_path, os.O_RDONLY)
    try:
        for window in evidence['windows']:
            data = os.pread(fd, window['size'], int(window['va'], 16) - base)
            if len(data) != window['size'] or hashlib.sha256(data).hexdigest() != window['sha256']:
                raise ValueError('toast source mismatch: ' + window['label'])
    finally:
        os.close(fd)


def apply(model, block, owner_fns):
    if model.get('name') in OWNERS:
        raise ValueError('toast lifecycle recovery incomplete; refusing reflected export')
    return model
