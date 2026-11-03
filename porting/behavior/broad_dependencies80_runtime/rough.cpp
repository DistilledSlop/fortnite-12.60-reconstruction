// Uncounted source drafts. Explicit native ABI/layout adapters, not build input.
#include <cstdint>
namespace broad_dependencies80 {
using P = std::uintptr_t; using I = std::int32_t; using U = std::uint32_t;
extern P ptr(P,unsigned); extern I i32(P,unsigned); extern U u32(P,unsigned);
extern std::uint16_t u16(P,unsigned);
extern void put_ptr(P,unsigned,P); extern void put_i32(P,unsigned,I); extern void put_u32(P,unsigned,U);
extern bool fname_none(P low_word,int zero);
extern P service_141e41680(P embedded,int one);
extern void classify_141945ed0(P service,P input,std::uint8_t flag,P output);
extern P ensure_name_142ec0440(P outer,P native_literal);
extern P class_142b5bd80();
extern P construct_142ed7360(P klass,P outer,P name0,P name1,P flags,P a,P b,P c,P d);
extern P empty_name_error_literal();
extern P invalid_engine_literal();
extern P invalid_engine_file_1459a4bc8();
extern void log_invalid_engine(P file,I line,P category,I level,P format);
extern P category_1481348e4();
extern void fatal_flush_142c7b670();
extern P package_142e80820();
extern P engine_global_148351eb8();
extern P class_142930580();
extern I thread_epoch_248(); extern I empty_guard_1480cf3b0();
extern void header_145803a44_1480cf3b0();
extern void empty_text_142c0c150_1480cf398(int zero);
extern void atexit_1458038a4_1458224e0();
extern void footer_1458039e4_1480cf3b0();
extern P empty_text_storage_1480cf398();
extern void atomic_increment_i32(P count);
extern P empty_string_1459a4b30();
extern void replace_142bb18c0(P input,P temporary,P needle,P replacement,I mode);
extern void free_memory(P);
extern void memmove_bytes(P destination,P source,std::int64_t size);
extern void shrink_string_140bc01c0(P input);
extern void text_from_string_142c26f20(P output,P input);

// 141d507d0: first matching non-None FName in an object's variant pointer array.
P variant(P object,std::uint64_t key) {
    P cursor=ptr(object,0x418);
    const P end=cursor+static_cast<std::intptr_t>(i32(object,0x420))*8;
    const U high=static_cast<U>(key>>32);
    while(cursor!=end) {
        const P candidate=ptr(cursor,0);
        const P value=ptr(candidate,0x28); // snapshot before predicate call
        const bool none=fname_none(static_cast<U>(key),0);
        if((!none || high!=0) && static_cast<U>(value>>32)==high &&
           static_cast<U>(value)==static_cast<U>(key)) return candidate;
        cursor+=8;
    }
    return 0;
}
// 141e5f200: conditional forwarding, output untouched when service absent.
void classify(P owner,P input,std::uint8_t flag,P output) {
    const P service=service_141e41680(owner+0x4520,1);
    if(service) classify_141945ed0(service,input,flag,output);
}
// 142040900 is typed void by Ghidra despite its caller consuming RAX. Its
// assembly epilogue after CALL142ed7360 preserves RAX through RET142040958.
P fallback_item(P outer) {
    ensure_name_142ec0440(outer,empty_name_error_literal());
    const P klass=class_142b5bd80();
    return construct_142ed7360(klass,outer,0,0,0,0,0,0,0);
}
static bool class_match(P candidate,P required) {
    const I index=i32(required,0x38);
    return index<=i32(candidate,0x38) &&
        ptr(ptr(candidate,0x30)+static_cast<std::intptr_t>(index)*8,0)==required+0x30;
}
// 141ed2350: engine object selection and explicit invalid-engine recovery path.
P engine_context() {
    P object=ptr(engine_global_148351eb8(),0x288);
    if(object) {
        const P required=class_142930580();
        if(class_match(ptr(object,0x10),required)) return object;
    }
    const P literal=invalid_engine_literal();
    log_invalid_engine(invalid_engine_file_1459a4bc8(),0xa93,category_1481348e4(),1,literal);
    fatal_flush_142c7b670();
    const P outer=package_142e80820();
    ensure_name_142ec0440(outer,empty_name_error_literal());
    const P klass=class_142930580();
    // Native preserves the upper half of the literal pointer in this argument.
    return construct_142ed7360(klass,outer,0,0,literal & P{0xffffffff00000000},0,0,0,0);
}

// 142706020: consumes the input string allocation, stripping configured strings
// then configured UTF-16 characters before constructing the text output.
P filter_text(P unused_runtime,P output,P input,P patterns,P characters) {
    if(i32(input,8)<2) {
        if(thread_epoch_248()<empty_guard_1480cf3b0()) {
            header_145803a44_1480cf3b0();
            if(empty_guard_1480cf3b0()==-1) {
                empty_text_142c0c150_1480cf398(0);
                atexit_1458038a4_1458224e0();
                footer_1458039e4_1480cf3b0();
            }
        }
        const P value=empty_text_storage_1480cf398();
        put_ptr(output,0,ptr(value,0));
        const P control=ptr(value,8);
        put_ptr(output,8,control);
        if(control) atomic_increment_i32(control+8);
        put_u32(output,16,u32(value,16));
    } else {
        if(i32(patterns,8)>0) {
            P cursor=ptr(patterns,0);
            const P end=cursor+static_cast<std::intptr_t>(i32(patterns,8))*16;
            for(;cursor!=end;cursor+=16) {
                const P needle=i32(cursor,8)==0 ? empty_string_1459a4b30() : ptr(cursor,0);
                alignas(8) std::uint8_t temporary[16]; // no native pre-zeroing
                replace_142bb18c0(input,reinterpret_cast<P>(temporary),needle,empty_string_1459a4b30(),1);
                const P allocation=ptr(reinterpret_cast<P>(temporary),0);
                if(allocation) free_memory(allocation);
            }
        }
        if(i32(characters,8)>0) {
            I index=0;
            std::intptr_t byte_offset=0;
            while(true) {
                I count=i32(input,8);
                I length=static_cast<I>(static_cast<U>(count)-1U);
                if(count==0) length=0;
                if(index>=length) break;
                P character=ptr(characters,0);
                const P string=ptr(input,0);
                const P end=character+static_cast<std::intptr_t>(i32(characters,8))*4;
                bool removed=false;
                for(;character!=end;character+=4) {
                    if(u32(character,0)!=u16(string+byte_offset,0)) continue;
                    I amount=length-index;
                    if(amount>1) amount=1;
                    if(amount!=0) {
                        const I tail=(count-amount)-index;
                        if(tail!=0) {
                            memmove_bytes(string+static_cast<std::intptr_t>(index)*2,
                                string+static_cast<std::intptr_t>(amount+index)*2,
                                static_cast<std::int64_t>(tail)*2);
                            count=i32(input,8);
                        }
                        put_i32(input,8,count-amount);
                        shrink_string_140bc01c0(input);
                    }
                    // Exact decompiled back edge decrements BOTH values before
                    // retry. Do not normalize it into a conventional erase loop.
                    byte_offset-=2; --index; removed=true; break;
                }
                if(!removed) { byte_offset+=2; ++index; }
            }
        }
        text_from_string_142c26f20(output,input);
    }
    const P allocation=ptr(input,0);
    if(allocation) free_memory(allocation);
    return output;
}
}
