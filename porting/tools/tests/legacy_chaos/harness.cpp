#include "FortniteLegacyChaosCookedData.h"
#include <fstream>
#include <iostream>
#include <iterator>
using namespace FortniteLegacyChaos;
int main(int argc,char**argv)
{
 if(argc!=2)return 2;
 std::ifstream F(argv[1],std::ios::binary); std::vector<uint8> B((std::istreambuf_iterator<char>(F)),{});
 FData D; FString E;
 // Output remains untouched on any failure.
 D.FaceRemap.SetNum(1);D.FaceRemap[0]=123456;
 if(!Read(B.data(),B.size(),D,E))
 {if(D.FaceRemap.Num()!=1||D.FaceRemap[0]!=123456)return 3; std::cout<<E.S<<"\n";return 1;}
 std::cout.precision(9);
 std::cout<<"{\"convex_alias\":"<<(D.Convexes.Num()>1 && D.Convexes[0] && D.Convexes[0]==D.Convexes[1])<<",\"mesh_alias\":"<<(D.TriangleMeshes.Num()>1 && D.TriangleMeshes[0] && D.TriangleMeshes[0]==D.TriangleMeshes[1])<<",\"convexes\":[";
 bool Comma=false; auto Vec=[&](FVector V){std::cout<<'['<<V.X<<','<<V.Y<<','<<V.Z<<']';};
 for(auto C:D.Convexes){if(Comma)std::cout<<',';Comma=true;if(!C){std::cout<<"null";continue;}std::cout<<"{\"collide\":"<<C->bDoCollide<<",\"vertices\":[";bool P=false;for(auto V:C->Vertices){if(P)std::cout<<',';P=true;Vec(V);}std::cout<<"]}";}
 std::cout<<"],\"meshes\":[";Comma=false;
 for(auto M:D.TriangleMeshes){if(Comma)std::cout<<',';Comma=true;if(!M){std::cout<<"null";continue;}std::cout<<"{\"collide\":"<<M->bDoCollide<<",\"vertices\":[";bool P=false;for(auto V:M->Vertices){if(P)std::cout<<',';P=true;Vec(V);}std::cout<<"],\"triangles\":[";P=false;for(auto T:M->Triangles){if(P)std::cout<<',';P=true;std::cout<<'['<<T.X<<','<<T.Y<<','<<T.Z<<']';}std::cout<<"],\"materials\":[";P=false;for(auto V:M->MaterialIndices){if(P)std::cout<<',';P=true;std::cout<<V;}std::cout<<"],\"facemap\":[";P=false;for(auto V:M->ExternalFaceIndexMap){if(P)std::cout<<',';P=true;std::cout<<V;}std::cout<<"]}";}
 std::cout<<"],\"uv_indices\":[";Comma=false;for(auto I:D.UVIndices){if(Comma)std::cout<<',';Comma=true;std::cout<<I;}
 std::cout<<"],\"uv_positions\":[";Comma=false;for(auto V:D.UVPositions){if(Comma)std::cout<<',';Comma=true;Vec(V);}
 std::cout<<"],\"uv_channels\":[";Comma=false;for(auto C:D.UVChannels){if(Comma)std::cout<<',';Comma=true;std::cout<<'[';bool P=false;for(auto V:C){if(P)std::cout<<',';P=true;std::cout<<'['<<V.X<<','<<V.Y<<']';}std::cout<<']';}
 std::cout<<"],\"face_remap\":[";Comma=false;for(auto I:D.FaceRemap){if(Comma)std::cout<<',';Comma=true;std::cout<<I;}std::cout<<"]}\n";
 // Check pointer identity where the input contains an alias (the synthetic case
 // only uses two convex objects with identical shared pointer addresses).
 return 0;
}
