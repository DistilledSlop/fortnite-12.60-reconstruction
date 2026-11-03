// Behavioral sketch of 0x1426a8510, original Win64 offsets. Not a native class layout.
// Calls and virtual dispatch use an unresolved ABI bridge; Frame preserves stack outputs
// and implicit arguments which the decompiler did not recover. See HANDOFF.md.
#include <cstdint>
#include <cstddef>
using U = uintptr_t;
using I = int32_t;
struct Frame { alignas(16) unsigned char bytes[0x268]; };
extern U call(U address, Frame&, ...);
extern U virt(U object, unsigned slot, Frame&, ...);
extern U mem(U object, unsigned offset);       // pointer-sized load
extern I dword(U object, unsigned offset);
extern uint8_t byte(U object, unsigned offset);
extern void put64(U object, unsigned offset, U value);
extern void put32(U object, unsigned offset, I value);
extern void put8(U object, unsigned offset, uint8_t value);
extern U slot(Frame&, unsigned);               // typed access to decompiler stack slot
extern void setslot(Frame&, unsigned, U);
extern U alloc(size_t); extern U resize(U, size_t); extern void release(U);
extern void compact_multicast(Frame&, U, bool);
extern void log_invalid_data(Frame&, U category, unsigned level, const wchar_t* format);
extern void set_low32(Frame&, unsigned, I);
// The bridge maps these names to the indicated original stack locations, including
// overlapping temporaries. It must not silently initialize omitted upper halves.
enum { S238=0x238, S230=0x230, S228=0x228, S220=0x220, S218=0x218,
       S210=0x210, S208=0x208, S200=0x200, S1f8=0x1f8, S1f0=0x1f0,
       S1e8=0x1e8, S1e0=0x1e0, S1d8=0x1d8, S1d0=0x1d0,
       S1c8=0x1c8, S1c0=0x1c0, S1b8=0x1b8, S1b0=0x1b0,
       S188=0x188, S180=0x180, S178=0x178, S170=0x170, S168=0x168,
       S158=0x158, S148=0x148, S128=0x128, S11c=0x11c,
       Sd8=0xd8, Sc8=0xc8, Sb0=0xb0, S70=0x70, S68=0x68 };
extern U stack_address(Frame&, unsigned);
static bool derives(U objectClass, U target) {
    return dword(objectClass,0x38) >= dword(target,0x38) &&
        mem(mem(objectClass,0x30), unsigned(dword(target,0x38))*8) == target+0x30;
}
static U cast_object(U object, U target) {
    return object && target && derives(mem(object,0x10),target) ? object : 0;
}
static U cast_class(U cls, U target) {
    return cls && target && derives(cls,target) ? cls : 0;
}
static U world_manager(U world) {
    U owner=mem(world,0x188); return owner ? mem(owner,0xc0) : mem(world,0x320);
}
// These routines only expand delegate construction and destruction, not gameplay paths.
static U make_delegate(Frame& f, U self, U callback, U initialVtable, U finalVtable) {
    U p=alloc(0x30);
    if (p) {
        put64(p,0,initialVtable); call(0x142ee3b30,f,p+8,self);
        put64(p,0x10,callback); put32(p,0x18,0);
        // +0x1c is the unrecovered upper half of the original stack member pointer.
        put32(p,0x1c,static_cast<I>(slot(f,S228)>>32));
        put64(p,0x28,call(0x142bad740,f)); put64(p,0,finalVtable);
    }
    return p;
}
static void free_delegate(Frame& f,U p) {
    if (p) { virt(p,0x38,f); U q=resize(p,0); if(q) release(q); }
}
static void prepare_timer_delegate(Frame& f,U p) {
    setslot(f,S188,0); setslot(f,S180,slot(f,S180)&~U(0xffffffff));
    if(p) virt(p,0x40,f,stack_address(f,S188));
    call(0x142ee3b30,f,stack_address(f,S178),0);
    setslot(f,S170,0); setslot(f,S168,0); setslot(f,S158,0);
}
static void cleanup_timer_delegate(Frame& f) {
    if(slot(f,S168)) virt(slot(f,S158)?slot(f,S158):stack_address(f,S148),0x10,f);
    U p=slot(f,S188);
    if (I(slot(f,S180))) {
        if (!p) return;
        virt(p,0x38,f); p=slot(f,S188);
        if(p) { p=resize(p,0); setslot(f,S188,p); }
        setslot(f,S180,slot(f,S180)&~U(0xffffffff));
    }
    if(p) release(p);
}
static void append_delegate(Frame& f,U list,U instance,U* result) {
    compact_multicast(f,list,true);
    U handle=virt(instance,0x30,f,stack_address(f,S238));
    if(result) *result=mem(handle,0);
    I count=dword(list,8); put32(list,8,count+1);
    if(dword(list,0xc)<count+1) call(0x140690dd0,f,list,count);
    U entry=mem(list,0)+U(count)*0x10; put64(entry,0,instance); put32(entry,8,3);
}
static void cleanup_spec(Frame& f) {
    setslot(f,S68,0); if(slot(f,S70)) release(slot(f,S70));
    call(0x14069a5b0,f,stack_address(f,Sb0));
    if(slot(f,Sc8)) release(slot(f,Sc8)); if(slot(f,Sd8)) release(slot(f,Sd8));
}
void weapon_virtual_780(U self,U oldWeapon) {
    Frame f; setslot(f,S208,oldWeapon);
    call(0x142ee3b30,f,self+0xb28); put64(self,0x8b0,0); put8(self,0x289,0);
    put32(self,0x8d8,dword(self,0x8d4)); put32(self,0x8dc,0);
    call(0x142690db0,f,self); virt(self,0x5e8,f,self);
    if(oldWeapon) {
        put8(self,0xb48,(byte(self,0xb48)&~1)|(byte(oldWeapon,0xb48)&1));
        U world=virt(self,0x150,f,self);
        if(world) {
            setslot(f,S230,0x1426afaa0); set_low32(f,S228,0);
            U oldWorld=virt(oldWeapon,0x150,f); U remaining=0;
            if(oldWorld) {
                U manager=world_manager(oldWorld); setslot(f,S238,mem(oldWeapon,0xb38));
                U timer=call(0x144621fb0,f,manager,stack_address(f,S238));
                if(timer && byte(timer,1)!=2) {
                    setslot(f,S210,mem(oldWeapon,0xb38));
                    timer=call(0x144621fb0,f,manager,stack_address(f,S210));
                    remaining=call(0x144627890,f,manager,timer);
                }
            }
            U manager=world_manager(world);
            U delegate=make_delegate(f,self,0x1426afaa0,0x1459a9e80,0x1459a9ed8);
            prepare_timer_delegate(f,delegate);
            call(0x1446278d0,f,manager,self+0xb38,stack_address(f,S188),remaining);
            cleanup_timer_delegate(f); free_delegate(f,delegate);
        }
    }
    U ownerCandidate=call(0x1426a07c0,f,self);
    U owner=0;
    if(ownerCandidate) { U ownerType=call(0x142a3d9f0,f); owner=cast_object(ownerCandidate,ownerType); }
    U pawn=call(0x141125940,f,self);
    if(pawn) { U type=call(0x142a174f0,f); pawn=cast_object(pawn,type); }
    if(pawn) {
        U asc=virt(pawn+0x4c8,0x10,f,pawn+0x4c8); setslot(f,S218,asc);
        if(asc) {
            U type=call(0x142b493b0,f);
            if(call(0x1406c7d70,f,asc,type)) {
                call(0x141629b80,f,stack_address(f,S188));
                set_low32(f,S228,0); setslot(f,S230,0x1426aa040);
                U key=call(0x1406aade0,f,stack_address(f,S1c8),stack_address(f,S188));
                U list=call(0x1406c90d0,f,asc,key);
                U delegate=make_delegate(f,self,0x1426aa040,0x1459bffd0,0x1459c0028);
                U handle=0; if(delegate) append_delegate(f,list,delegate,&handle);
                put64(self,0x9d0,handle);
                if(slot(f,S168)) release(slot(f,S168)); if(slot(f,S188)) release(slot(f,S188));
            }
            if(!mem(self,0x350)) {
                // Logging threshold is a live global read; formatting/temporary cleanup remain calls.
                extern I global_dword(U);
                if(global_dword(0x1481348e0)>2) {
                    setslot(f,S238,mem(self,0x18)); call(0x142ce9f90,f);
                    log_invalid_data(f,0x1481348e4,3,
                         L"Weapon [%s] - Invalid WeaponData in GetWeaponDataPlayerGrantedGameplayTags");
                    if(slot(f,S230)) release(slot(f,S230));
                }
            } else call(0x14266faf0,f,mem(self,0x350),self+0x388);
            asc=slot(f,S218);
            for(I i=0;i>=0 && i<dword(self,0x390);++i)
                if(call(0x140744d30,f,asc+0x1040)) virt(asc,0x810,f,asc);
            if(mem(self,0x350) && byte(pawn,0xf0)==3) {
                // Four distinct getters and handle locations; each cast is deliberately repeated.
                const U getters[]={0x14266fd90,0x142670540,0x142670230,0x14266eff0};
                const unsigned outputs[]={S1f8,S200,S210,S238};
                const unsigned handles[]={0x928,0x92c,0x930,0x934};
                for(unsigned k=0;k<4;++k) {
                    call(getters[k],f,mem(self,0x350),stack_address(f,outputs[k]));
                    U cls=slot(f,outputs[k]); U base=cls?call(0x1428f53b0,f):0;
                    if(cls && base && cast_class(cls,base) && dword(self,handles[k])==-1) {
                        base=call(0x1428f53b0,f); U cast=cast_class(slot(f,outputs[k]),base);
                        U value=call(0x140dce6c0,f,cast);
                        if(k==0) call(0x1406eb990,f,stack_address(f,S128),value,1);
                        else call(0x1406eb990,f,stack_address(f,S128),value);
                        put32(self,handles[k],I(slot(f,S11c)));
                        if(k==0) {
                            U currentAsc=virt(pawn+0x4c8,0x10,f);
                            call(0x14070ffa0,f,currentAsc,stack_address(f,S220));
                            U spec=slot(f,S218)?call(0x140708a10,f,slot(f,S218),dword(self,0x928)):0;
                            U ability=spec?call(0x14070f400,f,spec):0;
                            if(ability) {
                                U target=call(0x1428f53b0,f);
                                if(derives(mem(ability,0x10),target)) {
                                    U value2=call(0x1428f52b0,f,ability);
                                    for(unsigned n=0;n<5;++n) call(0x141601410,f,ability+0x420+n*0xb8,value2);
                                    if(virt(mem(self,0x350),0x298,f)==0x17) call(0x1426b2bf0,f,self);
                                }
                            }
                        } else if(slot(f,S218)) call(0x14070ffa0,f,slot(f,S218),stack_address(f,S220));
                        cleanup_spec(f);
                    }
                }
                extern I global_dword(U);
                if(global_dword(0x1478de4dc)) {
                    U type=call(0x14278c330,f); U component=virt(self,0x610,f,self,type);
                    if(component) call(0x141cf6440,f,component,stack_address(f,S218));
                }
                if(dword(self,0x940)==0 && dword(self,0x950)==0) {
                    call(0x14266bff0,f,mem(self,0x350));
                    for(I i=0;i<I(slot(f,S228));++i) {
                        U cls=mem(slot(f,S230),unsigned(i)*8);
                        U base=cls?call(0x1428f53b0,f):0;
                        if(cast_class(cls,base)) {
                            cls=mem(slot(f,S230),unsigned(i)*8);
                            base=cls?call(0x1428f53b0,f):0;
                            U abilityClass=cast_class(cls,base);
                            if(!mem(abilityClass,0x130)) virt(abilityClass,0x390,f,abilityClass);
                            call(0x1406eb990,f,stack_address(f,S128));
                            I count=dword(self,0x940); put32(self,0x940,count+1);
                            if(dword(self,0x944)<count+1) call(0x140690b30,f,self+0x938);
                            put32(mem(self,0x938),unsigned(count)*4,I(slot(f,S11c)));
                            if(slot(f,S218)) call(0x14070ffa0,f);
                            cleanup_spec(f);
                        }
                    }
                    U item=call(0x1415e26b0,f,mem(self,0x350)+0x940,1);
                    if(item) {
                        U value=call(0x1415f3960,f,item,stack_address(f,S188));
                        I count=dword(self,0x950); put32(self,0x950,count+1);
                        if(dword(self,0x954)<count+1) call(0x14069ea30,f,self+0x948,count);
                        call(0x1415e4f10,f,mem(self,0x948)+U(count)*0x28,value);
                        if(slot(f,S170)) release(slot(f,S170)); if(slot(f,S180)) release(slot(f,S180));
                    }
                    if(slot(f,S230)) release(slot(f,S230));
                }
                if(!(dword(self,0x900)&1)) put32(self,0x900,dword(self,0x900)|2);
                else call(0x1426a25b0,f,self,pawn);
            }
        }
        if(!call(0x14268efc0,f,self) && owner) virt(self,0x8b8,f,self,0);
        if(byte(self,0xf0)==3) {
            U other=mem(pawn,0x250);
            if(other) { U type=call(0x142a3d9f0,f); if(cast_object(other,type)) call(0x1423bebe0,f,other); }
        }
        if(mem(self,0x350) && virt(mem(self,0x350),0x4e8,f) && owner &&
           (virt(owner,0x6a8,f,owner) || byte(self,0xf0)==3)) {
            set_low32(f,S228,0); setslot(f,S230,0x1426aba90);
            U list=call(0x14239b310,f,owner,0);
            U delegate=make_delegate(f,self,0x1426aba90,0x145a88cc8,0x145a88d20);
            if(delegate) append_delegate(f,list,delegate,nullptr);
        }
        extern I global_dword(U);
        if(call(0x1440402d0,f) && global_dword(0x1478df2c8)) {
            call(0x1426ad8b0,f,mem(self,0x4b0)); call(0x1426ad8b0,f,mem(self,0x4b8));
            for(unsigned off=0x4c0;off!=0x4d8;off+=8) call(0x1426ad8b0,f,mem(self,off));
            call(0x1426ad8b0,f,mem(self,0x4d8));
            for(unsigned off=0x4e0;off!=0x4f8;off+=8) call(0x1426ad8b0,f,mem(self,off));
            call(0x1426ad8b0,f,mem(self,0x4d8));
            for(unsigned off=0x500;off!=0x518;off+=8) call(0x1426ad8b0,f,mem(self,off));
            for(unsigned off=0x518;off!=0x530;off+=8) call(0x1426ad8b0,f,mem(self,off));
            call(0x1426ad8b0,f,mem(self,0x548)); call(0x1426ad8b0,f,mem(self,0x550));
            for(unsigned off=0x560;off!=0x630;off+=8) call(0x1426ad8b0,f,mem(self,off));
        }
        if(mem(self,0x350) && call(0x1409f35d0,f,pawn)) {
            setslot(f,S188,0); setslot(f,S180,0); setslot(f,S178,0); setslot(f,S170,0);
            setslot(f,S1e8,0); setslot(f,S1e0,0); setslot(f,S1d8,0); setslot(f,S1d0,0);
            virt(mem(self,0x350)+0x30,0x10,f,mem(self,0x350)+0x30,stack_address(f,S188));
            U x=call(0x142323510,f,pawn,0); x=call(0x141d4ab00,f,x,stack_address(f,S188));
            call(0x141d4dc30,f,x,stack_address(f,S1e8));
            U component=virt(pawn+0x4c8,0x10,f);
            if(I(slot(f,S1e0)) && component) {
                setslot(f,S1c8,0); setslot(f,S1c0,0); setslot(f,S1b8,0); setslot(f,S1b0,0);
                for(I i=0;i>=0 && i<I(slot(f,S1e0));++i) {
                    setslot(f,S238,mem(slot(f,S1e8),unsigned(i)*8));
                    call(0x141ed2350,f); U tagRoot=call(0x141ed1800,f);
                    if(call(0x143e93680,f,stack_address(f,S238),tagRoot+0x1918)) {
                        call(0x142ce9f90,f,stack_address(f,S238),stack_address(f,S1f8));
                        U name=I(slot(f,S1f0))?slot(f,S1f8):0x1459a4b30;
                        call(0x142bafb80,f,stack_address(f,S230),L"%s.Weapon",name);
                        U formatted=I(slot(f,S228))?slot(f,S230):0x1459a4b30;
                        U tag=call(0x142cd4ab0,f,stack_address(f,S210),formatted,1);
                        tag=call(0x143e96810,f,stack_address(f,S200),tag);
                        call(0x143e8b3b0,f,stack_address(f,S1c8),tag);
                        if(slot(f,S230)) release(slot(f,S230)); if(slot(f,S1f8)) release(slot(f,S1f8));
                    }
                }
                call(0x143e8c1b0,f,stack_address(f,S1e8),stack_address(f,S1c8));
                for(I i=0;i>=0 && i<I(slot(f,S1e0));++i)
                    if(call(0x140744d30,f,component+0x1040)) virt(component,0x810,f,component);
                if(slot(f,S1b8)) release(slot(f,S1b8)); if(slot(f,S1c8)) release(slot(f,S1c8));
            }
            if(slot(f,S1d8)) release(slot(f,S1d8)); if(slot(f,S1e8)) release(slot(f,S1e8));
            if(slot(f,S178)) release(slot(f,S178)); if(slot(f,S188)) release(slot(f,S188));
        }
        oldWeapon=slot(f,S208);
    }
    if(!(dword(self,0x900)&8)) {
        put32(self,0x900,dword(self,0x900)|8); U currentPawn=call(0x141125940,f,self);
        if(oldWeapon) { virt(self,0x8b8,f,self,0); virt(oldWeapon,0x8b8,f,oldWeapon,0); }
        virt(self,0xa60,f,self,1);
        if(currentPawn) {
            U object=call(0x14268c4c0,f,self);
            if(!object || !call(0x142331940,f,object)) {
                call(0x142694ec0,f,self); call(0x1426a27c0,f,self); call(0x1426abce0,f,self,oldWeapon);
            }
        }
    }
    if(byte(self,0x288) && owner && byte(owner,0xf0)==3 && pawn && mem(self,0x350) &&
       dword(mem(self,0x350),0x970)>0) {
        U component=mem(owner,0x220);
        if(component && call(0x141edfd80,f,component)) {
            call(0x1409f35d0,f,pawn); call(0x142408830,f,component,owner); virt(component,0x5e8,f,component);
        }
    }
    setslot(f,S238,self); call(0x1406a9dc0,f,self+0x2a0,stack_address(f,S238));
    set_low32(f,S228,0); setslot(f,S230,0x1426ad0e0); U manager=call(0x140f95ae0,f);
    U delegate=make_delegate(f,self,0x1426ad0e0,0x1459a9e80,0x1459a9ed8);
    prepare_timer_delegate(f,delegate); call(0x144627d90,f,manager);
    cleanup_timer_delegate(f); free_delegate(f,delegate);
    // 0x145803c70 is the compiler security-cookie check, omitted from gameplay sketch.
}
