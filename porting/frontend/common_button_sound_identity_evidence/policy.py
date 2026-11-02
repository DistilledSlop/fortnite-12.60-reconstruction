"""Bounded original and engine-source gates for selection feedback sound."""
import hashlib
import json
import os
from pathlib import Path

HERE = Path(__file__).resolve().parent
WINDOWS = json.loads((HERE / 'windows.json').read_text())
BASE = 0x140000000


def validate_original_image(path):
    fd = os.open(path, os.O_RDONLY)
    try:
        for w in WINDOWS:
            b = os.pread(fd, w['length'], int(w['va'], 16) - BASE)
            if len(b) != w['length'] or hashlib.sha256(b).hexdigest() != w['sha256']:
                raise ValueError('sound identity original mismatch: ' + w['name'])
    finally:
        os.close(fd)


# Exact bounded source spans include complete dispatch methods, constructor and
# create statements, plus interface slot order. Pass the UnrealEngine root.
SOURCE_SPANS = {
    'Engine/Source/Runtime/Slate/Public/Framework/Application/SlateApplication.h': [
        'static FSlateApplication& Get()\n\t{\n\t\tcheck( IsInGameThread() || IsInSlateThread() || IsInAsyncLoadingThread() );\n\t\treturn *CurrentApplication;\n\t}',
    ],
    'Engine/Source/Runtime/Slate/Private/Framework/Application/SlateApplication.cpp': [
        'CurrentApplication = MakeShareable( new FSlateApplication() );\n\tCurrentBaseApplication = CurrentApplication;\n\n\tPlatformApplication->SetMessageHandler( CurrentApplication.ToSharedRef() );',
        'SlateSoundDevice( MakeShareable(new FNullSlateSoundDevice()) )',
        'void FSlateApplication::InitializeSound( const TSharedRef<ISlateSoundDevice>& InSlateSoundDevice )\n{\n\tSlateSoundDevice = InSlateSoundDevice;\n}',
        'void FSlateApplication::PlaySound( const FSlateSound& SoundToPlay, int32 UserIndex ) const\n{\n\tSlateSoundDevice->PlaySound(SoundToPlay, UserIndex);\n}',
    ],
    'Engine/Source/Runtime/SlateCore/Public/Sound/ISlateSoundDevice.h': [
        'virtual void PlaySound(const FSlateSound& Sound, int32 UserIndex = 0) const = 0;\n\n\t/** @return The duration of the given sound resource */\n\tvirtual float GetSoundDuration(const FSlateSound& Sound) const = 0;',
    ],
    'Engine/Source/Runtime/SlateCore/Private/Sound/NullSlateSoundDevice.cpp': [
        'void FNullSlateSoundDevice::PlaySound(const FSlateSound&, int32) const\n{\n}\n\nfloat FNullSlateSoundDevice::GetSoundDuration(const FSlateSound& Sound) const\n{\n\treturn 0.0f;\n}',
    ],
}


def validate_source(engine_root):
    for relative, spans in SOURCE_SPANS.items():
        source = (Path(engine_root) / relative).read_text()
        for span in spans:
            if source.count(span) != 1:
                raise ValueError('sound identity engine source mismatch: ' + relative)
