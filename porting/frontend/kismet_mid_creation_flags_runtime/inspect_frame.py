"""GDB helper for stopped 64-bit non-editor UE4.25 frames; reads memory only.

UObject header offsets and non-case-preserving FName entry encoding match this
project target. No inferior function calls or VM modifications are performed.
"""
import gdb,struct
inferior=gdb.selected_inferior()
def memory(address,size):return inferior.read_memory(address,size).tobytes()
def ptr(address):return struct.unpack('<Q',memory(address,8))[0]
blocks=int(gdb.parse_and_eval('GNameBlocksDebug'))
def fname(address):
    index,number=struct.unpack('<II',memory(address,8))
    block=ptr(blocks+(index>>16)*8);entry=block+(index&65535)*2
    header=struct.unpack('<H',memory(entry,2))[0];length=header>>6;wide=header&1
    text=memory(entry+2,length*(wide+1)).decode('utf-16le' if wide else 'utf-8',errors='replace')
    return text+('_'+str(number-1) if number else '')
def object_path(address):
    names=[]
    for _ in range(8):
        if not address:break
        names.append(fname(address+0x18));address=ptr(address+0x20)
    return '.'.join(reversed(names))
for label,expression in [('node','Stack.Node'),('context','Context')]:
    try:value=int(gdb.parse_and_eval(expression))
    except gdb.error:
        if label!='context':raise
        value=int(gdb.parse_and_eval('Stack.Object'))
    print('ROOT_VM_'+label.upper()+'='+object_path(value))
    if label=='context':print('ROOT_VM_CLASS='+object_path(ptr(value+0x10)))
try:
    code=int(gdb.parse_and_eval('Stack.Code'));start=int(gdb.parse_and_eval('Stack.Node->Script.AllocatorInstance.Data'))
    print('ROOT_VM_CODE_OFFSET='+str(code-start));print('ROOT_VM_BYTES='+memory(code-10,24).hex())
    begin=max(start,code-256);data=memory(begin,code-begin)
    for offset,opcode in enumerate(data[:-8]):
        if opcode not in (0x1c,0x45,0x46,0x68):continue
        candidate=struct.unpack_from('<Q',data,offset+1)[0]
        if candidate<65536 or candidate>0x7fffffffffff:continue
        try:
            if fname(ptr(candidate+0x10)+0x18) == 'Function':
                print('ROOT_VM_CALL='+str(begin+offset-start)+':'+object_path(candidate))
        except (gdb.MemoryError,UnicodeError,struct.error):pass

except gdb.error as error:print('ROOT_VM_OFFSET_UNAVAILABLE='+str(error))
gdb.execute('x/3i $pc');gdb.execute('info registers rax rbx rcx rdx rsi rdi')
