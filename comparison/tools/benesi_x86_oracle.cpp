// Windows x86 only: execute original [CODE] from a Benesi .m3f asset.
// Parent must whitelist/hash the asset and enforce a process timeout.
#include <windows.h>
#include <cstdio>
#include <cstring>
#include <float.h>
#include <cstdlib>
#include <cstdint>
static double mapU, mapV, mapW;
static int mapNumber, mapCalls;
static void __cdecl heightMapProbe(double* uv, int number, double* result) {
 mapU=uv[0];mapV=uv[1];mapNumber=number;++mapCalls;
 result[0]=mapU;result[1]=mapV;result[2]=number;
}
// Delphi register ABI for function(PVec3D, Integer): TVec3D.
// EAX=input, EDX=map number, ECX=result (may alias input).
static void __declspec(naked) heightMapCallback() {
 __asm {
  push ecx
  push edx
  push eax
  call heightMapProbe
  add esp,12
  ret
 }
}
static void __cdecl sphereMapProbe(double* xyz, int number, double* result) {
 mapU=xyz[0];mapV=xyz[1];mapW=xyz[2];mapNumber=number;++mapCalls;
 result[0]=mapW;result[1]=mapU;result[2]=number;
}
static void __declspec(naked) sphereMapCallback() {
 __asm {
  push ecx
  push edx
  push eax
  call sphereMapProbe
  add esp,12
  ret
 }
}
int main(int argc,char**argv){
 if(argc!=2 && argc!=3 && argc!=4)return 2;
 bool sceneMode=argc>=3;bool difs=argc==4;unsigned char prepared[1024]={};
 bool counters=sceneMode && std::getenv("MB3D_POINT_COUNTERS");
 bool heightMap=sceneMode && std::getenv("MB3D_POINT_HEIGHTMAP_CALLBACK");
 bool sphereMap=sceneMode && std::getenv("MB3D_POINT_SPHEREMAP_CALLBACK");
 bool sseState=sceneMode && std::getenv("MB3D_POINT_SSE01");
 if(sseState && difs)return 8;
 if(sphereMap && !strstr(argv[1],"_MapTranslate") && !strstr(argv[1],"ABoxMapSFold") && !strstr(argv[1],"SphereHeightMap") && !strstr(argv[1],"SphereHMVoid"))return 8;
 if(heightMap && (!difs || (!strstr(argv[1],"HeightMapIFS") && !strstr(argv[1],"HeightMap2IFS") && !strstr(argv[1],"CylinderHeightMap") && !strstr(argv[1],"CylinderHMVoid") && !strstr(argv[1],"TorusHMVoid"))))return 8;
 if(sceneMode){FILE* p=fopen(argv[2],"rb");if(!p)return 6;size_t n=fread(prepared,1,1024,p);fclose(p);if(n!=1024)return 7;}
 SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);
 FILE*f=fopen(argv[1],"rb");if(!f)return 3;
 unsigned char bytes[65536];size_t size=fread(bytes,1,sizeof(bytes),f);fclose(f);
 if(!size||size==sizeof(bytes))return 4;
 void* code=VirtualAlloc(0,size,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);if(!code)return 5;
 memcpy(code,bytes,size);DWORD old;VirtualProtect(code,size,PAGE_EXECUTE_READ,&old);FlushInstructionCache(GetCurrentProcess(),code,size);
 unsigned int cw;_controlfp_s(&cw,_PC_64,_MCW_PC);
 double x,y,z,w,j1,j2,j3,p0,p1,p2,p3;
 while(scanf("%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf",&x,&y,&z,&w,&j1,&j2,&j3,&p0,&p1,&p2,&p3)==11){
  __declspec(align(16)) double sse01[4]={};
  if(sseState && scanf("%lf %lf %lf %lf",&sse01[0],&sse01[1],&sse01[2],&sse01[3])!=4)return 9;
  __declspec(align(16)) unsigned char params[1024]={};__declspec(align(16)) unsigned char storage[1024]={};unsigned char* it=storage+256;void* var=params+128;
  memcpy(params+112,&p0,8);memcpy(params+104,&p1,8);memcpy(params+96,&p2,8);memcpy(params+88,&p3,8);
  if (strstr(argv[1],"FlipXYc") || strstr(argv[1],"FlipXZc") || strstr(argv[1],"FlipYZc")) { int flip=int(p0); memcpy(params+116,&flip,4); }
  double constants[]={.816496580927726,.5773502691896258,.7071067811865475};memcpy(var,constants,24);
  if(sceneMode){memcpy(params,prepared,1024);var=params+512;double one=1,rold=.001,stop=4;int first=-1,maxit=20;memcpy(it+200,&one,8);memcpy(it+232,&one,8);memcpy(it-48,&rold,8);memcpy(it-40,&stop,8);memcpy(it+208,&first,4);memcpy(it+68,&maxit,4);}
  memcpy(it-32,&x,8);memcpy(it-24,&y,8);memcpy(it-16,&z,8);memcpy(it-8,&w,8);
  if(sphereMap){void* callback=reinterpret_cast<void*>(&sphereMapCallback);memcpy(it+352,&callback,4);mapU=mapV=mapW=0;mapNumber=mapCalls=0;}
  if(heightMap){void* callback=reinterpret_cast<void*>(&heightMapCallback);memcpy(it+356,&callback,4);memcpy(it-48,&p0,8);memcpy(it+216,&p1,8);mapU=mapV=0;mapNumber=mapCalls=0;}
  double radius=x*x+y*y+z*z;memcpy(it+56,&radius,8);
  if(sceneMode && std::getenv("MB3D_POINT_ROUT"))memcpy(it+56,&p0,8);
  if(sceneMode && std::getenv("MB3D_POINT_MAP_CONTEXT")){memcpy(it-48,&p0,8);memcpy(it+216,&p1,8);}
  if(sceneMode && std::getenv("MB3D_POINT_FIRST")){int32_t first=static_cast<int32_t>(p1);memcpy(it+208,&first,4);}
  if(sceneMode && std::getenv("MB3D_POINT_ITERATION")){int32_t iteration=static_cast<int32_t>(p0);memcpy(it+64,&iteration,4);}
  if(sceneMode && std::getenv("MB3D_POINT_DERIVATIVES")){memcpy(it+232,&p1,8);memcpy(it+240,&p2,8);memcpy(it+248,&p3,8);}
  if(sceneMode && std::getenv("MB3D_POINT_JULIA_MODE")){int32_t mode=static_cast<int32_t>(p3);memcpy(it+152,&mode,4);}
  if(counters){int32_t counter=static_cast<int32_t>(p0);memcpy(it+208,&counter,4);for(int k=0;k<6;++k){uint32_t bits=static_cast<uint32_t>(p2)+k;if(static_cast<uint32_t>(p1)&(1u<<k))bits|=0x80000000u;memcpy(it+76+4*k,&bits,4);}}
  double cx=x+.125,cy=y-.25,cz=z+.5;
  memcpy(it,&cx,8);memcpy(it+8,&cy,8);memcpy(it+16,&cz,8);
  memcpy(it+48,&var,4);memcpy(it+24,&j1,8);memcpy(it+32,&j2,8);memcpy(it+40,&j3,8);
  if(difs){
   __asm {
    push ebx
    push esi
    push edi
    mov esi,it
    add esi,88
    mov edi,var
    mov eax,code
    xor ebx,ebx
    mov ecx,1
    call eax
    pop edi
    pop esi
    pop ebx
   }
  }else if(sseState) __asm {
   mov eax,it
   sub eax,8
   push eax
   mov eax,it
   push eax
   mov eax,it
   sub eax,32
   mov edx,it
   sub edx,24
   mov ecx,it
   sub ecx,16
   movupd xmm0,sse01
   movupd xmm1,sse01[16]
   call code
   movupd sse01,xmm0
   movupd sse01[16],xmm1
  }else __asm {
   mov eax,it
   sub eax,8
   push eax
   mov eax,it
   push eax
   mov eax,it
   sub eax,32
   mov edx,it
   sub edx,24
   mov ecx,it
   sub ecx,16
   call code
  }
  memcpy(&x,it-32,8);memcpy(&y,it-24,8);memcpy(&z,it-16,8);memcpy(&w,it-8,8);
  double j4;memcpy(&j1,it+24,8);memcpy(&j2,it+32,8);memcpy(&j3,it+40,8);memcpy(&j4,it-56,8);
  printf("%.17g %.17g %.17g %.17g %.17g %.17g %.17g %.17g\n",x,y,z,w,j1,j2,j3,j4);
  if(sceneMode){
   int offsets[]={0,8,16,56,200,208,216,224,232,240,248,64};
   for(int i=0;i<12;++i){if(i==5||i==11){int v;memcpy(&v,it+offsets[i],4);printf("%d",v);}else{double v;memcpy(&v,it+offsets[i],8);printf("%.17g",v);}printf(i==11?"\n":" ");}
  }
  if(counters)for(int k=0;k<6;++k){int32_t v;memcpy(&v,it+76+4*k,4);printf(k==5?"%d\n":"%d ",v);}
  if(sseState)printf("%.17g %.17g %.17g %.17g\n",sse01[0],sse01[1],sse01[2],sse01[3]);
  if(sphereMap)printf("%.17g %.17g %.17g %d %d\n",mapU,mapV,mapW,mapNumber,mapCalls);
 if(heightMap)printf("%.17g %.17g %d %d\n",mapU,mapV,mapNumber,mapCalls);
 }
 VirtualFree(code,0,MEM_RELEASE);return 0;
}
