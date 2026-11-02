# CommonButton double-click receiver

The generated source aliases original virtual +0x4d8 as `NativeOnDoubleClicked`
and the FReply native receiver 0x1407c6b10 as `NativeHandleDoubleClicked`. The
shared native multicast `NativeDoubleClicked` represents original +0xaf0.
The receiver clears the reflected stop bit, calls the virtual, and rereads the
bit after callbacks. The base virtual calls the Blueprint event, native multicast,
then bound dynamic multicast in original order. The reflected setter uses the
same bit. Source aliases do not establish binary vtable or object-layout parity.

The policy gates the original class, both property records, both function records,
the complete original dynamic delegate, native exec/create join, member model,
helper collisions, accepted setter splice and rendered header and setter body.
`StopDoubleClickPropagation.inc` is the behavior snippet. Run `python3
porting/frontend/common_button_double_click_runtime/test_policy.py` for its
reflection and fail-closed checks. See source-evidence.md for replay and limits.

This is not Initialize or lifecycle wiring. Only bind the receiver with the
existing internal button's weak UObject delegate as part of a complete, source
supported five-receiver initialization batch. The internal Slate-level callback
can subsequently alter the reply. UE generation/build/runtime verification belongs
to the coherent integration batch; none is claimed here.
