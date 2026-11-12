"""Print direction, length and the first bits (LSB first, as FBitReader reads) of the UDP payloads in a loopback pcap.
usage: pcap_dump.py capture.pcap [count]   (standard library only, run with python3 -I)"""
import struct,sys
d=open(sys.argv[1],'rb').read()
magic,vm,vn,tz,sf,snap,lt=struct.unpack('<IHHiIII',d[:24]); o=24; i=0; n=int(sys.argv[2]) if len(sys.argv)>2 else 30
print('linktype',lt)
while o+16<=len(d) and i<n:
    ts,tu,cl,ol=struct.unpack('<IIII',d[o:o+16]); o+=16
    p=d[o:o+cl]; o+=cl
    off=14 if lt==1 else 16 if lt==113 else 0
    ip=p[off:]; ihl=(ip[0]&15)*4; sp,dp=struct.unpack('>HH',ip[ihl:ihl+4]); pay=ip[ihl+8:]
    dirn='C>S' if dp==7777 else 'S>C'
    bits=''.join(format(b,'08b')[::-1] for b in pay)  # LSB-first bit order as FBitReader
    print(i,dirn,len(pay),'B', pay[:12].hex(), 'bits:',bits[:40])
    i+=1
