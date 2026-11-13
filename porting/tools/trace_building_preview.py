# Passive trace of the original 12.60 Windows client's placement helper.
# Attach once with a NEW absolute path, then enter `continue` at the GDB prompt:
#   BUILDING_PREVIEW_LOG=/absolute/new/path.jsonl \
#     gdb -q -nx -p "$CLIENT_LINUX_PID" -x porting/tools/trace_building_preview.py
# Stop with Ctrl-C, then `detach` and `quit`. The trace disables itself after
# 100 distinct events. Breakpoints briefly stop the client; no inferior calls.
import datetime
import json
import os
import struct
import time
import gdb

BASE = 0x140000000
ENTRY = 0x14237dbe0
RET = 0x14237e3de
# These signatures are from the decrypted 12.60 runtime image. The PE dump is
# RVA-mapped (file offset == RVA). Check both it and /proc/PID/mem before hooks.
SIGNATURES = (
    (ENTRY, bytes.fromhex('48895c24105556574154415541564157488dac2420ffffff4881ece001000048')),
    (0x14237e3c4, bytes.fromhex('488b9c24280200004881c4e0010000415f415e415d415c5f5e5dc3')),
)
REFERENCE = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))),
                         'generated/dump/FortniteClient-Win64-Shipping.runtime.exe')
path = os.environ.get('BUILDING_PREVIEW_LOG')
if not path or not os.path.isabs(path):
    raise RuntimeError('BUILDING_PREVIEW_LOG must be a new absolute file path')
inferior = gdb.selected_inferior()
pid = inferior.pid
if not pid:
    raise RuntimeError('attach to the original client before loading this script')
with open(REFERENCE, 'rb') as reference, open('/proc/{}/mem'.format(pid), 'rb', buffering=0) as memory:
    for address, expected in SIGNATURES:
        reference.seek(address - BASE)
        memory.seek(address)
        if reference.read(len(expected)) != expected or memory.read(len(expected)) != expected:
            raise RuntimeError('client or reference signature mismatch at {:#x}; no hooks installed'.format(address))
log = open(path, 'x', buffering=1, encoding='utf-8')

# Win64 entry: RCX=controller, RDX=class, R8/R9=vector pointers. At
# 0x14237e32e the validator takes class in RDX and candidate position in R8;
# on success 0x14237e346 copies that position to the outer R8 output. The
# outer function can fail without writing that output: record it only on zero.
# RSP at the return instruction equals the entry RSP, including the return
# address, so (thread, RSP) pairs nested calls without patching return sites.
def read(addr, fmt):
    size = struct.calcsize(fmt)
    return struct.unpack(fmt, bytes(inferior.read_memory(addr, size)))

def maybe(addr, fmt):
    try:
        return read(addr, fmt)
    except (gdb.MemoryError, gdb.error, OverflowError):
        return None

def reg(name):
    return int(gdb.parse_and_eval('$' + name))

def write(record):
    log.write(json.dumps(record, separators=(',', ':'), allow_nan=False) + '\n')

pending = {}
last = {}
last_time = {}
count = 0
hooks = []

def class_name_raw(addr):
    value = maybe(addr + 0x18, '<2I') if addr else None
    return list(value) if value else None

def disable():
    for hook in hooks:
        hook.enabled = False
    pending.clear()

def vec(addr):
    value = maybe(addr, '<3f') if addr else None
    if value is None:
        return None
    # Invalid floats cannot be encoded as JSON, and do not describe a location.
    return list(value) if all(abs(x) < 1e9 for x in value) else None

class EntryBreakpoint(gdb.Breakpoint):
    def stop(self):
        try:
            thread = gdb.selected_thread().global_num
            rsp, pc, cls, location = reg('rsp'), reg('rcx'), reg('rdx'), reg('r8')
            caller = maybe(rsp, '<Q')
            selected = maybe(pc + 0x1350, '<Q') if pc else None
            snap = vec(pc + 0x1464) if pc else None
            # Bound stale frames if an exception unwinds without reaching RET.
            if len(pending) >= 512:
                pending.clear()
            pending[(thread, rsp)] = {'caller': caller[0] if caller else None,
                'controller': pc, 'class_argument': cls,
                'selected_class': selected[0] if selected else None,
                'class_argument_fname_raw': class_name_raw(cls),
                'selected_class_fname_raw': class_name_raw(selected[0]) if selected else None,
                'snap_at_entry': snap, 'location_ptr': location}
        except Exception as exc:
            write({'type': 'entry_error', 'error': str(exc)})
            disable()
        return False

class ReturnBreakpoint(gdb.Breakpoint):
    def stop(self):
        global count
        try:
            thread, rsp = gdb.selected_thread().global_num, reg('rsp')
            event = pending.pop((thread, rsp), None)
            if event is None:
                return False  # call began before attachment or was unwound
            result = reg('eax') & 0xffffffff
            event['result'] = result
            location_ptr = event.pop('location_ptr')
            event['location_output'] = vec(location_ptr) if result == 0 else None
            event['snap_at_return'] = vec(event['controller'] + 0x1464) if event['controller'] else None
            event['thread'] = thread
            # Group by caller/class and preserve changes in validation result.
            key = (thread, event['caller'], event['class_argument'], event['selected_class'])
            now = time.monotonic()
            # A changing camera can change snap every frame. Emit result changes
            # immediately, otherwise at most one context sample per second.
            if last.get(key) == result and now - last_time.get(key, 0) < 1.0:
                return False
            last[key] = result
            last_time[key] = now
            count += 1
            event.update(type='placement', sequence=count,
                         utc=datetime.datetime.now(datetime.timezone.utc).isoformat())
            write(event)
            if count >= 100:
                disable()
                write({'type': 'limit', 'events': count})
        except Exception as exc:
            write({'type': 'return_error', 'error': str(exc)})
            disable()
        return False

try:
    gdb.execute('set pagination off')
    gdb.execute('set confirm off')
    gdb.execute('handle SIGSEGV nostop noprint pass')  # Wine first-chance handling
    gdb.execute('handle SIGTRAP nostop noprint nopass')
    for address, typ in ((ENTRY, EntryBreakpoint), (RET, ReturnBreakpoint)):
        hook = typ('*{:#x}'.format(address), internal=True)
        hook.silent = True
        hooks.append(hook)
    write({'type': 'start', 'pid': pid, 'entry': hex(ENTRY), 'return': hex(RET),
           'note': 'class pointers are raw; snap can be stale; location output is recorded only for result zero'})
except Exception:
    for hook in hooks:
        hook.delete()
    log.close()
    raise
print('Building preview trace installed; enter continue. Log: {}'.format(path))
