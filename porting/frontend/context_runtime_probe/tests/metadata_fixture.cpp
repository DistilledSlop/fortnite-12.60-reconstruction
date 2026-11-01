#include <cstdint>
using u64=uint64_t;using u32=uint32_t;
void* Z_Construct_UClass_UBlueprintContextLibrary(){return nullptr;}
void* Z_Construct_UClass_USubsystem_NoRegister(){return nullptr;}
void* Z_Construct_UClass_UObject_NoRegister(){return nullptr;}
void* Z_Construct_UClass_UClass(){return nullptr;}
struct UBlueprintContextLibrary {static void execGetContext() {} static void StaticRegisterNativesUBlueprintContextLibrary();};
struct Pair {const char* name;void (*fn)();};
volatile const void* registry_sink;
void UBlueprintContextLibrary::StaticRegisterNativesUBlueprintContextLibrary(){static const Pair Funcs[]={{"GetContext", &UBlueprintContextLibrary::execGetContext}};registry_sink=Funcs;}
struct ObjProp {const char* name; const char* rep; u64 flags; u32 kind,object,dim,offset; void* (*type)();};
struct ClassProp {const char* name;const char* rep;u64 flags;u32 kind,object,dim,offset;void* (*meta)();void* (*type)();};
struct Fn {void* (*owner)();void* (*super)();const char* name;const char* owning;const char* delegate;u64 size;const void* const* props;u32 count,object,flags;uint16_t rpc,reply;};
struct Z_Construct_UFunction_UBlueprintContextLibrary_GetContext_Statics {
 static const ObjProp NewProp_ReturnValue;
 static const ClassProp NewProp_Class;
 static const ObjProp NewProp_ContextObject;
 static const void* const PropPointers[];
 static const Fn FuncParams;
};
const ObjProp Z_Construct_UFunction_UBlueprintContextLibrary_GetContext_Statics::NewProp_ReturnValue={"ReturnValue",nullptr,0x10000000000580,0x12,0x45,1,16,Z_Construct_UClass_USubsystem_NoRegister};
const ClassProp Z_Construct_UFunction_UBlueprintContextLibrary_GetContext_Statics::NewProp_Class={"Class",nullptr,0x14000000000080,0x11,0x45,1,8,Z_Construct_UClass_USubsystem_NoRegister,Z_Construct_UClass_UClass};
const ObjProp Z_Construct_UFunction_UBlueprintContextLibrary_GetContext_Statics::NewProp_ContextObject={"ContextObject",nullptr,0x10000000000080,0x12,0x45,1,0,Z_Construct_UClass_UObject_NoRegister};
const void* const Z_Construct_UFunction_UBlueprintContextLibrary_GetContext_Statics::PropPointers[]={&NewProp_ReturnValue,&NewProp_Class,&NewProp_ContextObject};
const Fn Z_Construct_UFunction_UBlueprintContextLibrary_GetContext_Statics::FuncParams={Z_Construct_UClass_UBlueprintContextLibrary,nullptr,"GetContext",nullptr,nullptr,24,PropPointers,3,0x45,0x14022409,0,0};
volatile const void* sink;
void* Z_Construct_UFunction_UBlueprintContextLibrary_GetContext(){sink=&Z_Construct_UFunction_UBlueprintContextLibrary_GetContext_Statics::FuncParams;return nullptr;}
int main(){ Z_Construct_UFunction_UBlueprintContextLibrary_GetContext();UBlueprintContextLibrary::execGetContext();UBlueprintContextLibrary::StaticRegisterNativesUBlueprintContextLibrary();}
