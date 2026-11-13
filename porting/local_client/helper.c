/* Explicit patched local mode for the original 12.60 CL13477524 process.
 * EXE unchanged on disk; CA trust and pins remain enforced. No TLS bypass.
 * Original layout proof: verifier1438bda50 manager+18/+20, domain stride32,
 * fixed digest stride40 with Num/Max32. X5091438bdc10 calls slot38.
 * This module must be injected by the launcher into its own shipping child.
 */
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <tlhelp32.h>
#include <wincrypt.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>

#define ORIGINAL_BASE ((uintptr_t)0x140000000ULL)
#define IMAGE_SIZE 0x09e4d600UL
#define MAX_DIGESTS 64
#define MAX_THREADS 4096
static uintptr_t image_base;
static WCHAR log_path[32768];
static char local_server[INET_ADDRSTRLEN];
static BYTE local_digest[32];
static const WCHAR *domains[6] = {L".ol.epicgames.net",L".ol.epicgames.com",
    L".on.epicgames.net",L".on.epicgames.com",L".ak.epicgames.com",L".fortnite.qq.com"};
#define AT(x) ((void *)(image_base + ((uintptr_t)(x) - ORIGINAL_BASE)))

static void status(const WCHAR *message) {
    OutputDebugStringW(message); OutputDebugStringW(L"\r\n");
    if (log_path[0]) {
        HANDLE file=CreateFileW(log_path,FILE_APPEND_DATA,FILE_SHARE_READ|FILE_SHARE_WRITE,
                               NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
        if(file!=INVALID_HANDLE_VALUE) {
            char text[256]; DWORD written;
            int size=WideCharToMultiByte(CP_UTF8,0,message,-1,text,sizeof(text)-2,NULL,NULL);
            if(size>0) {text[size-1]='\r';text[size]='\n';WriteFile(file,text,(DWORD)size+1,&written,NULL);}
            CloseHandle(file);
        }
    }
}

static int range_ok(const void *pointer,SIZE_T size,int write) {
    MEMORY_BASIC_INFORMATION m;
    uintptr_t start=(uintptr_t)pointer, end=start+size;
    DWORD p;
    if(start<0x10000 || !size || end<start || !VirtualQuery(pointer,&m,sizeof(m))) return 0;
    p=m.Protect & 0xff;
    if(m.State!=MEM_COMMIT || (m.Protect&(PAGE_GUARD|PAGE_NOACCESS)) ||
       start<(uintptr_t)m.BaseAddress || end>(uintptr_t)m.BaseAddress+m.RegionSize) return 0;
    if(write) return m.Type==MEM_PRIVATE && (p==PAGE_READWRITE || p==PAGE_EXECUTE_READWRITE);
    return p==PAGE_READONLY || p==PAGE_READWRITE || p==PAGE_WRITECOPY ||
           p==PAGE_EXECUTE_READ || p==PAGE_EXECUTE_READWRITE || p==PAGE_EXECUTE_WRITECOPY;
}
static int safe_read(const void *source,void *target,SIZE_T size,int write) {
    SIZE_T got=0;
    return range_ok(source,size,write) && ReadProcessMemory(GetCurrentProcess(),source,target,size,&got) && got==size;
}
static int bytes_equal(const void *pointer,const void *expected,SIZE_T size) {
    return range_ok(pointer,size,0) && !memcmp(pointer,expected,size);
}
static uint64_t u64(const BYTE *p) {uint64_t v;memcpy(&v,p,8);return v;}
static int32_t i32(const BYTE *p) {int32_t v;memcpy(&v,p,4);return v;}
static int image_version(void) {
    IMAGE_DOS_HEADER *dos=(IMAGE_DOS_HEADER *)image_base;
    IMAGE_NT_HEADERS64 *nt;
    WCHAR filename[32768], *name;
    if(!GetModuleFileNameW(NULL,filename,32768)) return 0;
    name=wcsrchr(filename,L'\\');name=name?name+1:filename;
    if(_wcsicmp(name,L"FortniteClient-Win64-Shipping.exe")) return 0;
    if(!range_ok(dos,4096,0)||dos->e_magic!=IMAGE_DOS_SIGNATURE||dos->e_lfanew<64||dos->e_lfanew>3500)return 0;
    nt=(IMAGE_NT_HEADERS64 *)(image_base+dos->e_lfanew);
    return nt->Signature==IMAGE_NT_SIGNATURE && nt->FileHeader.Machine==IMAGE_FILE_MACHINE_AMD64 &&
       nt->FileHeader.TimeDateStamp==0x5ebe1934 && nt->OptionalHeader.Magic==IMAGE_NT_OPTIONAL_HDR64_MAGIC &&
       nt->OptionalHeader.SizeOfImage==IMAGE_SIZE;
}
static int tls_signatures(void) {
    static const BYTE digest[]={0xe9,0x37,0x7f,0x79,0x05};
    static const BYTE x509[]={0xe9,0xca,0x3d,0xbb,0x05};
    static const BYTE call[]={0xff,0x50,0x38,0x90,0xe9,0xcb,0x2a,0x07,0x06};
    static const BYTE wss[]={0xe8,0x83,0xa3,0x40,0xfe,0x45,0x33,0xf6,0x44,0x38,0x74,0x24,0x60,0x75,0x54,0x44,0x8b,0xe6};
    uintptr_t slots[2]={(uintptr_t)AT(0x1438bda50ULL),(uintptr_t)AT(0x1438bdc10ULL)};
    return bytes_equal(AT(0x1438bda50ULL),digest,sizeof(digest)) &&
       bytes_equal(AT(0x1438bdc10ULL),x509,sizeof(x509)) &&
       bytes_equal(AT(0x1438bddb0ULL),call,sizeof(call)) &&
       bytes_equal(AT(0x144866e28ULL),wss,sizeof(wss)) &&
       bytes_equal(AT(0x146761e38ULL),slots,sizeof(slots)) &&
       range_ok(AT(0x147c69478ULL),1,0) && *(BYTE *)AT(0x147c69478ULL)==1;
}

static int enroll_public_ca(const WCHAR *path) {
    HANDLE file=INVALID_HANDLE_VALUE;
    BYTE *pem=NULL,*der=NULL,*spki=NULL;
    DWORD size,got,der_size=0,spki_size=0,digest_size=32,decoded_size=0;
    PCCERT_CONTEXT cert=NULL,added=NULL;
    CERT_BASIC_CONSTRAINTS2_INFO *constraints=NULL;
    HCERTSTORE store=NULL;
    int ok=0;DWORD failure=ERROR_SUCCESS;
    SetLastError(ERROR_SUCCESS);
    file=CreateFileW(path,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
    if(file==INVALID_HANDLE_VALUE)goto done;
    size=GetFileSize(file,NULL);
    if(size<60 || size>262144)goto done;
    pem=HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,size+1);
    if(!pem || !ReadFile(file,pem,size,&got,NULL)||got!=size)goto done;
    /* Reject any private-key/extra PEM blocks before invoking a decoder. */
    {
        char *begin=strstr((char *)pem,"-----BEGIN CERTIFICATE-----");
        char *end=strstr((char *)pem,"-----END CERTIFICATE-----"), *p;
        if(!begin||!end||end<begin||strstr(begin+1,"-----BEGIN")||strstr(end+1,"-----END"))goto done;
        for(p=(char *)pem;p<begin;p++)if(*p!=' '&&*p!='\r'&&*p!='\n'&&*p!='\t')goto done;
        for(p=end+strlen("-----END CERTIFICATE-----");p<(char *)pem+size;p++)
            if(*p!=' '&&*p!='\r'&&*p!='\n'&&*p!='\t')goto done;
        if(memchr(pem,0,size))goto done;
    }
    if(!CryptStringToBinaryA((char *)pem,size,CRYPT_STRING_BASE64HEADER,NULL,&der_size,NULL,NULL))goto done;
    der=HeapAlloc(GetProcessHeap(),0,der_size);
    if(!der || !CryptStringToBinaryA((char *)pem,size,CRYPT_STRING_BASE64HEADER,der,&der_size,NULL,NULL))goto done;
    cert=CertCreateCertificateContext(X509_ASN_ENCODING,der,der_size);
    if(!cert || CertVerifyTimeValidity(NULL,cert->pCertInfo)!=0)goto done;
    {
        PCERT_EXTENSION ext=CertFindExtension(szOID_BASIC_CONSTRAINTS2,cert->pCertInfo->cExtension,cert->pCertInfo->rgExtension);
        if(!ext || !CryptDecodeObjectEx(X509_ASN_ENCODING,X509_BASIC_CONSTRAINTS2,ext->Value.pbData,
          ext->Value.cbData,CRYPT_DECODE_ALLOC_FLAG,NULL,&constraints,&decoded_size)||!constraints->fCA)goto done;
    }
    if(!CertCompareCertificateName(X509_ASN_ENCODING,&cert->pCertInfo->Issuer,&cert->pCertInfo->Subject) ||
       !CryptVerifyCertificateSignatureEx(0,X509_ASN_ENCODING,CRYPT_VERIFY_CERT_SIGN_SUBJECT_CERT,
          (void *)cert,CRYPT_VERIFY_CERT_SIGN_ISSUER_CERT,(void *)cert,0,NULL))goto done;
    if(!CryptEncodeObjectEx(X509_ASN_ENCODING,X509_PUBLIC_KEY_INFO,&cert->pCertInfo->SubjectPublicKeyInfo,
       CRYPT_ENCODE_ALLOC_FLAG,NULL,&spki,&spki_size))goto done;
    if(!CryptHashCertificate2(L"SHA256",0,NULL,spki,spki_size,local_digest,&digest_size)||digest_size!=32)goto done;
    store=CertOpenStore(CERT_STORE_PROV_SYSTEM_W,0,0,CERT_SYSTEM_STORE_CURRENT_USER,L"ROOT");
    if(!store || !CertAddCertificateContextToStore(store,cert,CERT_STORE_ADD_USE_EXISTING,&added))goto done;
    ok=1;
 done:
    if(!ok){failure=GetLastError();if(!failure)failure=ERROR_INVALID_DATA;}
    if(added)CertFreeCertificateContext(added);
    if(store)CertCloseStore(store,0);
    if(spki)LocalFree(spki);
    if(constraints)LocalFree(constraints);
    if(cert)CertFreeCertificateContext(cert);
    if(der)HeapFree(GetProcessHeap(),0,der);
    if(pem)HeapFree(GetProcessHeap(),0,pem);
    if(file!=INVALID_HANDLE_VALUE)CloseHandle(file);
    SetLastError(failure);
    return ok;
}


/* GUI preflight: the parent loads with FN_LOCAL_PREPARE_ONLY=1, calls this
 * export, unloads, clears the flag, then starts the shipping child. This path
 * reads only the public certificate and enrolls CurrentUser ROOT. It never
 * accesses the main image, process memory, DNS hooks or local account config.
 */
__declspec(dllexport) BOOL WINAPI ProjectChronoPrepareCertificate(LPCWSTR certificatePath) {
    if(!certificatePath || !certificatePath[0] ||
       !((certificatePath[1]==L':' && (certificatePath[2]==L'\\' || certificatePath[2]==L'/')) ||
         (certificatePath[0]==L'\\' && certificatePath[1]==L'\\'))) {
        SetLastError(ERROR_BAD_PATHNAME);
        return FALSE;
    }
    SetLastError(ERROR_SUCCESS);
    if(!enroll_public_ca(certificatePath)) {
        if(GetLastError()==ERROR_SUCCESS)SetLastError(ERROR_INVALID_DATA);
        return FALSE;
    }
    return TRUE;
}

typedef struct {BYTE raw[32];WCHAR name[256];BYTE digests[MAX_DIGESTS][40];} PinEntry;
typedef struct {uintptr_t manager;BYTE header[40];PinEntry entry[6];} Snapshot;
static int array_valid(const BYTE *raw,int limit,int stride) {
    uintptr_t p=(uintptr_t)u64(raw);int n=i32(raw+8),cap=i32(raw+12);
    return p>=0x10000 && !(p&7) && n>0 && n<=cap && cap<=limit && range_ok((void *)p,(SIZE_T)cap*stride,1);
}
static int overlaps(uintptr_t a,SIZE_T an,uintptr_t b,SIZE_T bn) {return a<b+bn && b<a+an;}
static int inspect(uintptr_t manager,Snapshot *s) {
    uintptr_t entries,roots;int i,j,k,seen=0;SIZE_T root_size,entry_size;
    memset(s,0,sizeof(*s));s->manager=manager;
    if(!range_ok((void *)manager,40,1))return 0;
    if(!safe_read((void *)manager,s->header,40,1))return 0;
    if(u64(s->header)!=(uintptr_t)AT(0x146761e00ULL)||
       !array_valid(s->header+8,8192,8)||!array_valid(s->header+24,128,32)||i32(s->header+32)!=6)return 0;
    roots=(uintptr_t)u64(s->header+8);root_size=(SIZE_T)i32(s->header+20)*8;
    entries=(uintptr_t)u64(s->header+24);entry_size=(SIZE_T)i32(s->header+36)*32;
    for(i=0;i<6;i++) {
        PinEntry *e=&s->entry[i];uintptr_t name,digests;int n,cap;
        if(!safe_read((void *)(entries+i*32),e->raw,32,1))return 0;
        name=(uintptr_t)u64(e->raw);n=i32(e->raw+8);cap=i32(e->raw+12);
        if(name<0x10000||(name&1)||n<2||n>cap||cap>256||!range_ok((void *)name,(SIZE_T)cap*2,1))return 0;
        if(!safe_read((void *)name,e->name,(SIZE_T)n*2,1))return 0;
        if(e->name[n-1])return 0;
        for(j=0;j<6;j++)if(!wcscmp(e->name,domains[j]))break;
        if(j==6||(seen&(1<<j)))return 0;
        seen|=1<<j;
        if(!array_valid(e->raw+16,MAX_DIGESTS,40))return 0;
        digests=(uintptr_t)u64(e->raw+16);
        for(j=0;j<i32(e->raw+24);j++) {
            if(!safe_read((void *)(digests+j*40),e->digests[j],40,1))return 0;
            if(i32(e->digests[j]+32)!=32||i32(e->digests[j]+36)!=32)return 0;
        }
    }
    /* No digest storage may alias another array, manager, strings or roots. */
    for(i=0;i<6;i++) {
        uintptr_t d=(uintptr_t)u64(s->entry[i].raw+16);SIZE_T ds=(SIZE_T)i32(s->entry[i].raw+28)*40;
        if(overlaps(d,ds,manager,40)||overlaps(d,ds,roots,root_size)||overlaps(d,ds,entries,entry_size))return 0;
        for(j=0;j<6;j++) {
            PinEntry *e=&s->entry[j];
            if(overlaps(d,ds,(uintptr_t)u64(e->raw),(SIZE_T)i32(e->raw+12)*2))return 0;
            if(j>i&&overlaps(d,ds,(uintptr_t)u64(e->raw+16),(SIZE_T)i32(e->raw+28)*40))return 0;
        }
    }
    /* Ensure the copied metadata did not change during this read. */
    {BYTE check[40];
    if(!safe_read((void *)manager,check,40,1)||memcmp(check,s->header,40))return 0;
    for(k=0;k<6;k++)if(!safe_read((void *)(entries+k*32),check,32,1)||memcmp(check,s->entry[k].raw,32))return 0;
    }
    return seen==63;
}
static BYTE scan_buffer[1024*1024+40]; /* DLL MEM_IMAGE, excluded from heap scan. */
static uintptr_t find_manager(void) {
    SYSTEM_INFO info;MEMORY_BASIC_INFORMATION m,stack;uintptr_t p,found=0;SIZE_T scanned=0;
    Snapshot snapshot;GetSystemInfo(&info);
    if(!VirtualQuery(&snapshot,&stack,sizeof(stack)))return 0;
    p=(uintptr_t)info.lpMinimumApplicationAddress;
    while(p<(uintptr_t)info.lpMaximumApplicationAddress && VirtualQuery((void *)p,&m,sizeof(m))) {
        uintptr_t end=(uintptr_t)m.BaseAddress+m.RegionSize;
        if(end<=p)break;
        /* Exclude own stack: snapshots contain valid-looking copied headers. */
        if(m.AllocationBase!=stack.AllocationBase && m.State==MEM_COMMIT&&m.Type==MEM_PRIVATE&&!(m.Protect&PAGE_GUARD)&&
          ((m.Protect&0xff)==PAGE_READWRITE||(m.Protect&0xff)==PAGE_EXECUTE_READWRITE)&&m.RegionSize<=256*1024*1024) {
            scanned+=m.RegionSize;if(scanned>(SIZE_T)4*1024*1024*1024)return 0;
            for(uintptr_t chunk=p;chunk<end;chunk+=1024*1024) {
                SIZE_T amount=end-chunk,got=0;if(amount>sizeof(scan_buffer))amount=sizeof(scan_buffer);
                if(!ReadProcessMemory(GetCurrentProcess(),(void *)chunk,scan_buffer,amount,&got)||got!=amount)continue;
                for(SIZE_T offset=0;offset+40<=amount;offset+=8) {
                    uintptr_t q=chunk+offset;
                    if(u64(scan_buffer+offset)==(uintptr_t)AT(0x146761e00ULL)&&inspect(q,&snapshot)) {
                        if(found&&found!=q)return 0;
                        found=q;
                    }
                }
            }
        }
        p=end;
    }
    return found;
}

/* Native thread enumeration avoids loader/heap allocation while peers are
 * suspended. No user-mode snapshot allocation takes place in the frozen region.
 * A thread created during enumeration is also visited before enumeration ends.
 */
typedef LONG (NTAPI *NextThreadFn)(HANDLE,HANDLE,ACCESS_MASK,ULONG,ULONG,PHANDLE);
static int freeze_other_threads(HANDLE handles[MAX_THREADS],DWORD *number) {
    NextThreadFn next=(NextThreadFn)(uintptr_t)GetProcAddress(GetModuleHandleW(L"ntdll.dll"),"NtGetNextThread");
    HANDLE cursor=NULL,thread=NULL;DWORD self=GetCurrentThreadId();LONG code;
    *number=0;if(!next)return 0;
    while((code=next(GetCurrentProcess(),cursor,THREAD_SUSPEND_RESUME|THREAD_QUERY_LIMITED_INFORMATION,0,0,&thread))>=0) {
        /* Keep cursor handles alive until all threads have been resumed. */
        if(GetThreadId(thread)==self) {
            /* Own worker is the sole unsuspended thread; retain its handle too. */
            if(*number==MAX_THREADS){CloseHandle(thread);goto fail;}
            handles[(*number)++]=thread;cursor=thread;continue;
        }
        if(*number==MAX_THREADS){CloseHandle(thread);goto fail;}
        if(SuspendThread(thread)==(DWORD)-1){CloseHandle(thread);goto fail;}
        handles[(*number)++]=thread;cursor=thread;
    }
    if((ULONG)code!=0x8000001aUL)goto fail; /* STATUS_NO_MORE_ENTRIES */
    return 1;
 fail:
    for(DWORD i=0;i<*number;i++) {
        if(GetThreadId(handles[i])!=self)ResumeThread(handles[i]);
        CloseHandle(handles[i]);
    }
    *number=0;return 0;
}
static void resume_threads(HANDLE *handles,DWORD number) {
    for(DWORD i=0;i<number;i++){if(GetThreadId(handles[i])!=GetCurrentThreadId())ResumeThread(handles[i]);CloseHandle(handles[i]);}
}
/* Return -1 on rollback failure, 0 on rejection, 1 on success. */
static int update_pins(uintptr_t manager,int *changed) {
    Snapshot before,now,after;HANDLE threads[MAX_THREADS];DWORD number=0;
    int modifications[6],count=0,attempted=0,result=0;
    if(!inspect(manager,&before)||!freeze_other_threads(threads,&number))return 0;
    if(!tls_signatures()||!inspect(manager,&now)||memcmp(&before,&now,sizeof(now)))goto done;
    for(int i=0;i<6;i++) {
        int present=0;PinEntry *e=&now.entry[i];
        for(int j=0;j<i32(e->raw+24);j++)if(!memcmp(e->digests[j],local_digest,32))present=1;
        if(!present)modifications[count++]=i;
    }
    for(int i=0;i<count;i++) {
        void *target=(void *)(uintptr_t)u64(now.entry[modifications[i]].raw+16);
        attempted=i+1;memcpy(target,local_digest,32);
        if(memcmp(target,local_digest,32))goto rollback;
    }
    if(!inspect(manager,&after))goto rollback;
    for(int i=0;i<count;i++)memcpy(now.entry[modifications[i]].digests[0],local_digest,32);
    if(memcmp(&after,&now,sizeof(now)))goto rollback;
    *changed=count;result=1;goto done;
 rollback:
    result=0;
    for(int i=0;i<attempted;i++) {
        int index=modifications[i];void *target=(void *)(uintptr_t)u64(before.entry[index].raw+16);
        memcpy(target,before.entry[index].digests[0],32);
        if(memcmp(target,before.entry[index].digests[0],32))result=-1;
    }
 done:
    /* A failed rollback must not leave a silently usable partial policy. */
    if(result<0)TerminateProcess(GetCurrentProcess(),5);
    resume_threads(threads,number);return result;
}

/* Evidence: original main-module IAT1458fd910 = WS2_32.dll!getaddrinfo.
 * Runtime import-name validation is mandatory; no speculative fallback hook. */
typedef int (WSAAPI *GetAddrInfoFn)(PCSTR,PCSTR,const ADDRINFOA *,PADDRINFOA *);
static GetAddrInfoFn real_getaddrinfo;
static int epic_service(const char *host) {
    static const char *suffixes[]={".ol.epicgames.net",".ol.epicgames.com",
        ".on.epicgames.net",".on.epicgames.com",".ak.epicgames.com",".fortnite.qq.com"};
    SIZE_T length;if(!host)return 0;length=strlen(host);
    for(int i=0;i<6;i++){SIZE_T n=strlen(suffixes[i]);if(length>n&&!_stricmp(host+length-n,suffixes[i]))return 1;}
    return 0;
}
static int WSAAPI local_getaddrinfo(PCSTR node,PCSTR service,const ADDRINFOA *hints,PADDRINFOA *result) {
    return real_getaddrinfo(epic_service(node)?local_server:node,service,hints,result);
}
static int image_rva(DWORD rva,SIZE_T size) {return rva<IMAGE_SIZE && size<=IMAGE_SIZE-rva && range_ok((void *)(image_base+rva),size,0);}
static int install_dns(void) {
    IMAGE_DOS_HEADER *dos=(IMAGE_DOS_HEADER *)image_base;
    IMAGE_NT_HEADERS64 *nt=(IMAGE_NT_HEADERS64 *)(image_base+dos->e_lfanew);
    IMAGE_DATA_DIRECTORY imports=nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
    HMODULE winsock=GetModuleHandleW(L"ws2_32.dll");void **slot=NULL;DWORD previous;
    GetAddrInfoFn expected=(GetAddrInfoFn)(uintptr_t)GetProcAddress(winsock,"getaddrinfo");
    if(!expected||!image_rva(imports.VirtualAddress,imports.Size))return 0;
    for(SIZE_T i=0;i<imports.Size/sizeof(IMAGE_IMPORT_DESCRIPTOR);i++) {
        IMAGE_IMPORT_DESCRIPTOR *d=(IMAGE_IMPORT_DESCRIPTOR *)(image_base+imports.VirtualAddress)+i;
        if(!d->Name)break;
        if(!image_rva(d->Name,32)||_strnicmp((char *)(image_base+d->Name),"WS2_32.dll",11))continue;
        if(!d->OriginalFirstThunk)return 0;
        for(DWORD index=0;index<2048;index++) {
            IMAGE_THUNK_DATA64 *name,*target;ULONGLONG name_rva;
            DWORD nr=d->OriginalFirstThunk+index*8,tr=d->FirstThunk+index*8;
            if(!image_rva(nr,8)||!image_rva(tr,8))return 0;
            name=(IMAGE_THUNK_DATA64 *)(image_base+nr);target=(IMAGE_THUNK_DATA64 *)(image_base+tr);
            name_rva=name->u1.AddressOfData;if(!name_rva)break;
            if(IMAGE_SNAP_BY_ORDINAL64(name_rva))continue;
            if(name_rva>0xffffffff||!image_rva((DWORD)name_rva,64))return 0;
            if(!strncmp((char *)(image_base+(DWORD)name_rva+2),"getaddrinfo",12)) {
                if(slot||tr!=0x58fd910||target->u1.Function!=(ULONGLONG)(uintptr_t)expected)return 0;
                slot=(void **)&target->u1.Function;
            }
        }
    }
    if(!slot)return 0;
    real_getaddrinfo=expected;
    if(!VirtualProtect(slot,sizeof(void *),PAGE_READWRITE,&previous))return 0;
    if(InterlockedCompareExchangePointer(slot,(void *)(uintptr_t)local_getaddrinfo,(void *)(uintptr_t)expected)!=(void *)(uintptr_t)expected) {
        DWORD ignored;VirtualProtect(slot,sizeof(void *),previous,&ignored);return 0;
    }
    {DWORD ignored;if(!VirtualProtect(slot,sizeof(void *),previous,&ignored))return 0;}
    return 1;
}

/* Exact pair from patch_appes_diagnostic.py callback mode. This suppresses the
 * handler and every use of its exit callback, not a successful entitlement check. */
static int appes_diagnostic(void) {
    static const BYTE expected[2][6]={{0x0f,0x84,0x7b,0x03,0,0},{0xe9,0x91,0xbb,0x83,0,0x90}};
    static const BYTE replacement[2][6]={{0xe9,0x7c,0x03,0,0,0x90},{0xc3,0x90,0x90,0x90,0x90,0x90}};
    void *addresses[2]={AT(0x142121984ULL),AT(0x142120970ULL)};
    DWORD protection[2],ignored;HANDLE threads[MAX_THREADS];DWORD number=0;int ok=0;
    if(!bytes_equal(addresses[0],expected[0],6)||!bytes_equal(addresses[1],expected[1],6))return 0;
    if(!freeze_other_threads(threads,&number))return 0;
    if(!bytes_equal(addresses[0],expected[0],6)||!bytes_equal(addresses[1],expected[1],6))goto done;
    if(!VirtualProtect(addresses[0],6,PAGE_EXECUTE_READWRITE,&protection[0]))goto done;
    if(!VirtualProtect(addresses[1],6,PAGE_EXECUTE_READWRITE,&protection[1])) {
        VirtualProtect(addresses[0],6,protection[0],&ignored);goto done;
    }
    memcpy(addresses[0],replacement[0],6);memcpy(addresses[1],replacement[1],6);
    ok=bytes_equal(addresses[0],replacement[0],6)&&bytes_equal(addresses[1],replacement[1],6);
    if(!ok){memcpy(addresses[0],expected[0],6);memcpy(addresses[1],expected[1],6);}
    {
        BOOL flush0=FlushInstructionCache(GetCurrentProcess(),addresses[0],6);
        BOOL flush1=FlushInstructionCache(GetCurrentProcess(),addresses[1],6);
        BOOL restore0=VirtualProtect(addresses[0],6,protection[0],&ignored);
        BOOL restore1=VirtualProtect(addresses[1],6,protection[1],&ignored);
        /* Failed instruction/protection synchronization cannot be retried as a
         * successful pair. Fail closed instead of running partially patched code. */
        if(!flush0||!flush1||!restore0||!restore1)TerminateProcess(GetCurrentProcess(),5);
    }
 done:
    resume_threads(threads,number);return ok;
}

static DWORD WINAPI helper_run(LPVOID ignored) {
    WCHAR ca_path[32768]={0},server[64]={0},account[256]={0};IN_ADDR ip;ULONGLONG deadline;
    int dns=0,appes=0,pins=0,changed=0;(void)ignored;
    if(GetEnvironmentVariableW(L"FN_LOCAL_HELPER_LOG",log_path,32768)>=32768)log_path[0]=0;
    DWORD ca_length=GetEnvironmentVariableW(L"FN_LOCAL_CA_CERT",ca_path,32768);
    DWORD server_length=GetEnvironmentVariableW(L"FN_LOCAL_SERVER",server,64);
    DWORD account_length=GetEnvironmentVariableW(L"FN_LOCAL_ACCOUNT",account,256);
    if(!ca_length||ca_length>=32768||!server_length||server_length>=64||!account_length||account_length>=256||
       InetPtonW(AF_INET,server,&ip)!=1 || !(ip.S_un.S_addr) ||
       !((ca_path[0]&&ca_path[1]==L':')||(ca_path[0]==L'\\'&&ca_path[1]==L'\\'))) {
        status(L"local helper rejected: explicit local configuration required");return 1;
    }
    if(!WideCharToMultiByte(CP_UTF8,0,server,-1,local_server,sizeof(local_server),NULL,NULL))return 1;
    image_base=(uintptr_t)GetModuleHandleW(NULL);
    if(!image_version()){status(L"local helper rejected: unsupported shipping image");return 1;}
    if(!enroll_public_ca(ca_path)){status(L"local helper rejected: public CA validation or enrollment failed");return 1;}
    status(L"patched local mode: public CA enrolled in CurrentUser ROOT; TLS verification retained");
    deadline=GetTickCount64()+180000;
    do {
        if(!dns) {dns=install_dns();if(dns)status(L"local DNS: guarded main-module getaddrinfo redirect installed");}
        if(!appes) {appes=appes_diagnostic();if(appes)status(L"local AppES callback diagnostic applied; entitlement checks not established");}
        if(!pins&&tls_signatures()) {
            uintptr_t manager=find_manager();
            if(manager&&update_pins(manager,&changed)>0) {
                pins=1;status(changed?L"local TLS: six domain pins updated; verification enabled":L"local TLS: six local pins already present; verification enabled");
            }
        }
        if(dns&&appes&&pins){status(L"patched local helper ready");return 0;}
        Sleep(250);
    }while(GetTickCount64()<deadline);
    if(!pins)status(L"local TLS pins pending: manager or original signatures unavailable");
    if(!dns)status(L"local DNS pending: original getaddrinfo import binding not proven");
    if(!appes)status(L"local AppES diagnostic pending: original signatures unavailable");
    status(L"local helper incomplete; no readiness claimed");return 1;
}


/* Optional launcher protocol. Sidecar/manual invocation without either event
 * keeps its existing behavior. Only the completed worker can signal readiness;
 * DllMain and certificate preflight never signal either event.
 */
static DWORD WINAPI helper_worker(LPVOID ignored) {
    DWORD result=helper_run(ignored);
    WCHAR ready_name[260]={0},failed_name[260]={0};
    DWORD ready_length=GetEnvironmentVariableW(L"FN_LOCAL_READY_EVENT",ready_name,260);
    DWORD failed_length=GetEnvironmentVariableW(L"FN_LOCAL_FAILED_EVENT",failed_name,260);
    HANDLE ready=NULL,failed=NULL;
    if(!ready_length && !failed_length)return result;
    if(ready_length && ready_length<260)
        ready=OpenEventW(EVENT_MODIFY_STATE,FALSE,ready_name);
    if(failed_length && failed_length<260)
        failed=OpenEventW(EVENT_MODIFY_STATE,FALSE,failed_name);
    /* Both configured events must be usable before any success is announced.
     * Signal the failure event when available, including a malformed pair.
     */
    if(!ready_length || ready_length>=260 || !failed_length || failed_length>=260 || !ready || !failed) {
        status(L"local helper event protocol failed: configured event pair unavailable");
        if(failed) {
            if(!SetEvent(failed))status(L"local helper event protocol failed: failure signal unavailable");
        }
        if(ready)CloseHandle(ready);
        if(failed)CloseHandle(failed);
        return 1;
    }
    if(!SetEvent(result==0?ready:failed)) {
        status(L"local helper event protocol failed: worker result signal unavailable");
        if(result==0 && !SetEvent(failed))
            status(L"local helper event protocol failed: failure signal unavailable");
        result=1;
    }
    CloseHandle(ready);CloseHandle(failed);
    return result;
}

BOOL WINAPI DllMain(HINSTANCE instance,DWORD reason,LPVOID reserved) {
    (void)reserved;
    if(reason==DLL_PROCESS_ATTACH) {
        HANDLE thread;WCHAR prepare_only[2]={0};DWORD length;
        DisableThreadLibraryCalls(instance);
        length=GetEnvironmentVariableW(L"FN_LOCAL_PREPARE_ONLY",prepare_only,2);
        if(length==1 && prepare_only[0]==L'1')return TRUE;
        thread=CreateThread(NULL,0,helper_worker,NULL,0,NULL);
        if(thread)CloseHandle(thread);else return FALSE;
    }
    return TRUE;
}
