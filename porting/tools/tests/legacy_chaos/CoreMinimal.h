#pragma once
#include <cstdint>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <cmath>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <utility>
using uint8=uint8_t; using uint16=uint16_t; using uint32=uint32_t; using int32=int32_t; using int64=int64_t; using TCHAR=char;
#define TEXT(x) x
struct FString { std::string S; bool IsEmpty()const{return S.empty();} void Empty(){S.clear();} static FString Printf(const char* F,...){char B[512]; va_list A; va_start(A,F);vsnprintf(B,sizeof B,F,A);va_end(A);return FString{B};} };
struct FVector { float X=0,Y=0,Z=0; FVector()=default; FVector(float A,float B,float C):X(A),Y(B),Z(C){} };
struct FVector2D { float X=0,Y=0; FVector2D()=default; FVector2D(float A,float B):X(A),Y(B){} };
struct FIntVector { int32 X=0,Y=0,Z=0; FIntVector()=default; FIntVector(int32 A,int32 B,int32 C):X(A),Y(B),Z(C){} };
struct FMemory {static void Memcpy(void* A,const void* B,size_t N){memcpy(A,B,N);}};
struct FMath {static bool IsFinite(float X){return std::isfinite(X);}};
template<class T> using TSharedPtr=std::shared_ptr<T>;
template<class T> TSharedPtr<T> MakeShared(){return std::make_shared<T>();}
template<class T> T&& MoveTemp(T& A){return std::move(A);}
template<class T> struct TArray:std::vector<T> {int32 Num()const{return int32(this->size());} void SetNum(int32 N){this->resize(size_t(N));}};
template<class K,class V> struct TMap {std::unordered_map<K,V> M; V* Find(K Key){auto I=M.find(Key);return I==M.end()?nullptr:&I->second;} void Add(K Key,const V& Val){M.emplace(Key,Val);} };
template<class K> struct TSet {std::unordered_set<K> S; bool Contains(K Key)const{return S.count(Key)!=0;}void Add(K Key){S.insert(Key);} };
