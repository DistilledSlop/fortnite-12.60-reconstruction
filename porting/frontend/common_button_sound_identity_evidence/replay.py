"""Replay bounded original and target engine joins, including mutation checks."""
import os
from pathlib import Path
import shutil
import tempfile
import policy

IMAGE = '/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin'
ENGINE = Path('/opt/fortnite/UnrealEngine')
policy.validate_original_image(IMAGE)
policy.validate_source(ENGINE)
with tempfile.TemporaryDirectory() as tmp:
    root = Path(tmp)
    for relative in policy.SOURCE_SPANS:
        target = root / relative
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(ENGINE / relative, target)
    rel = 'Engine/Source/Runtime/Slate/Private/Framework/Application/SlateApplication.cpp'
    p = root / rel
    p.write_text(p.read_text().replace('SlateSoundDevice->PlaySound(SoundToPlay, UserIndex);', 'SlateSoundDevice->PlaySound(SoundToPlay, 0);'))
    try:
        policy.validate_source(root)
    except ValueError:
        pass
    else:
        raise AssertionError('mutated source accepted')
    # Sparse fixture copies only guarded spans; corrupt one byte in a guarded span.
    image = root / 'image.bin'
    src = os.open(IMAGE, os.O_RDONLY)
    dst = os.open(image, os.O_RDWR | os.O_CREAT, 0o600)
    try:
        for w in policy.WINDOWS:
            offset = int(w['va'], 16) - policy.BASE
            os.pwrite(dst, os.pread(src, w['length'], offset), offset)
        offset = int(policy.WINDOWS[0]['va'], 16) - policy.BASE
        os.pwrite(dst, b'\x00', offset)
    finally:
        os.close(src)
        os.close(dst)
    try:
        policy.validate_original_image(image)
    except ValueError:
        pass
    else:
        raise AssertionError('mutated original accepted')
print('PASS: original windows, engine source and both mutation rejections')
