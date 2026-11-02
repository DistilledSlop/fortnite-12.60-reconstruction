# Original source evidence

Original Windows memory-layout image SHA256
`338c83a2314ccac1040a2c0bb520b893013cc7b58eda58e550da67f5d84de680`.
`button-initialize-dependency-map-1` pins receiver 0x1407c6b10, base
0x1407d2b90 and the event identities. Receiver clears bit +0xac8 bit 0,
invokes virtual +0x4d8, rereads the bit and returns the corresponding real
FReply. Base dispatches BP_OnDoubleClicked, native +0xaf0 and dynamic
OnButtonDoubleClicked(this) in that order. `button-native-multicast-correction-1`
proves +0xaf0 is a multicast, correcting the earlier singlecast label in the
initial dependency map. `button-click-compaction-proof-1` pins shared native
multicast mutation semantics. `common-button-constructor-defaults-1` pins the
reflected bit and setter 0x1407fd0d0; reflected exec 0x14080d970 jumps to it.
Native reflection record joins the exec to create 0x140807610.

On 2026-11-02 these source replays passed:

```
PYTHONPATH=/tmp/common-input-capstone python3 /mnt/vibrato/fortnite-work/button-initialize-dependency-map-1/replay.py
PYTHONPATH=/tmp/common-input-capstone python3 /mnt/vibrato/fortnite-work/button-native-multicast-correction-1/replay.py
PYTHONPATH=/tmp/common-input-capstone python3 /mnt/vibrato/fortnite-work/common-button-constructor-defaults-1/replay.py
```

These are static source checks, not proof of runtime compatibility. The existing
internal Slate callback can later override the receiver's reply. Initialize and
other receivers remain unimplemented by this isolated candidate.
