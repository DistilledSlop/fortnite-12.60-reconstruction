#pragma once
#include <cstddef>
#include <cstdint>

// SHA-256 (FIPS 180-4). Caller supplies at most UINT32_MAX bytes.
inline void LegacyManifestSHA256(const uint8_t* Data, uint32_t Size, uint8_t Out[32])
{
    static const uint32_t K[64] = {
        0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
        0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
        0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
        0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
        0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
        0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
        0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
        0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2};
    uint32_t H[8]={0x6a09e667,0xbb67ae85,0x3c6ef372,0xa54ff53a,0x510e527f,0x9b05688c,0x1f83d9ab,0x5be0cd19};
    auto Rot=[](uint32_t X,unsigned N){return (X>>N)|(X<<(32-N));};
    auto Block=[&](const uint8_t* P)
    {
        uint32_t W[64];
        for (unsigned I=0;I<16;++I) W[I]=(uint32_t(P[4*I])<<24)|(uint32_t(P[4*I+1])<<16)|(uint32_t(P[4*I+2])<<8)|P[4*I+3];
        for (unsigned I=16;I<64;++I) W[I]=(Rot(W[I-2],17)^Rot(W[I-2],19)^(W[I-2]>>10))+W[I-7]+(Rot(W[I-15],7)^Rot(W[I-15],18)^(W[I-15]>>3))+W[I-16];
        uint32_t A=H[0],B=H[1],C=H[2],D=H[3],E=H[4],F=H[5],G=H[6],T=H[7];
        for(unsigned I=0;I<64;++I)
        {
            uint32_t T1=T+(Rot(E,6)^Rot(E,11)^Rot(E,25))+((E&F)^(~E&G))+K[I]+W[I];
            uint32_t T2=(Rot(A,2)^Rot(A,13)^Rot(A,22))+((A&B)^(A&C)^(B&C));
            T=G;G=F;F=E;E=D+T1;D=C;C=B;B=A;A=T1+T2;
        }
        H[0]+=A;H[1]+=B;H[2]+=C;H[3]+=D;H[4]+=E;H[5]+=F;H[6]+=G;H[7]+=T;
    };
    uint32_t Offset=0;
    while (Size-Offset>=64) { Block(Data+Offset); Offset+=64; }
    uint8_t Tail[128]={}; const uint32_t Remainder=Size-Offset;
    for(uint32_t I=0;I<Remainder;++I) Tail[I]=Data[Offset+I];
    Tail[Remainder]=0x80;
    const uint32_t Length=(Remainder<56 ? 64 : 128);
    const uint64_t Bits=uint64_t(Size)*8;
    for(unsigned I=0;I<8;++I) Tail[Length-1-I]=uint8_t(Bits>>(8*I));
    Block(Tail); if(Length==128) Block(Tail+64);
    for(unsigned I=0;I<8;++I) for(unsigned J=0;J<4;++J) Out[I*4+J]=uint8_t(H[I]>>(24-8*J));
}
