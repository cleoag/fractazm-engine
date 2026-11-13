// A batch point probe for explicitly selected external formulas. Input rows:
// x y z w J1 J2 J3 option0 option1 option2 option3. Output rows: x y z w.
#include "core/external_formulas.h"
#include "core/formula_validation.h"
#include "core/header_trafos.h"
#include "io/m3p_parser.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>
static double mapU, mapV, mapW;
static int mapNumber, mapCalls;
static void heightMapProbe(TPVec3D result, TPVec3D uv, int32_t number) {
 mapU=(*uv)[0];mapV=(*uv)[1];mapNumber=number;++mapCalls;
 (*result)[0]=mapU;(*result)[1]=mapV;(*result)[2]=number;
}
static void sphereMapProbe(TPVec3D result, TPVec3D xyz, int32_t number) {
 mapU=(*xyz)[0];mapV=(*xyz)[1];mapW=(*xyz)[2];mapNumber=number;++mapCalls;
 (*result)[0]=mapW;(*result)[1]=mapU;(*result)[2]=number;
}
static int runProbe(int argc,char**argv){
 if(argc!=2 && argc!=4 && argc!=5)return 2;
 const bool dumpOnly=argc==5 && strcmp(argv[4],"--dump-only")==0;
 if(argc==5 && !dumpOnly)return 2;
 bool sceneMode=argc>=4;
 bool sseState=sceneMode && std::getenv("MB3D_POINT_SSE01");
 bool counters=sceneMode && std::getenv("MB3D_POINT_COUNTERS");
 bool heightMap=sceneMode && std::getenv("MB3D_POINT_HEIGHTMAP_CALLBACK");
 bool sphereMap=sceneMode && std::getenv("MB3D_POINT_SPHEREMAP_CALLBACK");
 if(sphereMap && strcmp(argv[1],"_MapTranslate")!=0 && strcmp(argv[1],"ABoxMapSFold")!=0 && strcmp(argv[1],"SphereHeightMap")!=0 && strcmp(argv[1],"SphereHMVoid")!=0)return 8;
 if(heightMap && strcmp(argv[1],"HeightMapIFS")!=0 && strcmp(argv[1],"HeightMap2IFS")!=0 && strcmp(argv[1],"CylinderHeightMap")!=0 && strcmp(argv[1],"CylinderHMVoid")!=0 && strcmp(argv[1],"TorusHMVoid")!=0)return 8;
 alignas(16) unsigned char prepared[1024]{};
 THAformula selected{};selected.iFnr=20;
 int selectedSlot=-1;
 memcpy(selected.CustomFname,argv[1],std::min(strlen(argv[1]),sizeof(selected.CustomFname)));
 mb3d::InitExternalFormulaRegistry();auto f=mb3d::FindExternalFormula(argv[1]);
 const char* extra[]={"_BenesiSpheric","_BenesiUnSpheric","_Abs4d","_AbsX","_AbsY","_AbsZ","_conj","_conj2D","_flipC-xyz","_FlipXY","_FlipXZ","_FlipYZ","_FlipXYc","_FlipXZc","_FlipYZc","_Folding1","_FoldingOct","_invcylindrical","_invspherical","_invtorical","_invtorical2","_JuliaSets","_QuadrayTransform","_QuadrayTransform2","_reciprocalX","_reciprocalX2","_reciprocalY2","_reciprocalZ2","_skew","_tocylindrical","_tospherical","_totorical","_updateC","_updateC_4d","_YplusSinZ","_Rpow3","_SinePow2","_SphereInvC","_SquareR","_HopSqrtX","_HopSqrtY","_HopSqrtZ"};
 bool allowed=f && f->id>=95001 && f->id<=95006;
 for(auto n:extra)allowed=allowed||strcmp(argv[1],n)==0;
 if(!f){
  for(const auto& issue:mb3d::ValidateExternalFormulaInput(selected,nullptr))fprintf(stderr,"%s\n",issue.message.c_str());
  return 12;
 }
 if(!allowed && !sceneMode)return 3;
 if(sceneMode){
  // IniCalcMaps also matches the scalar label Log2MapPower. These four
  // compiled formulas never call a map; keep their production metadata intact.
  const bool scalarMap = strcmp(argv[1],"TgladTetra")==0 || strcmp(argv[1],"TgladTetra2")==0 || strcmp(argv[1],"TgladDihed")==0 || strcmp(argv[1],"TgladDihed2")==0;
  if(f->mapValueMask && !heightMap && !sphereMap && !scalarMap)return 4;
  // These original CODE blocks read numeric folding values, never the trailing pointer.
  const bool numericFolding = strcmp(argv[1],"_SphereFolding1")==0 || strcmp(argv[1],"_SphereFolding2")==0 || strcmp(argv[1],"_Folding2")==0;
  for(int i=0;i<f->optionTypeCount;++i)if(f->optionTypes[i]==8 && !numericFolding)return 4;
  auto loaded=mb3d::LoadM3PFile(argv[2]);if(!loaded.success)return 5;
  int slot=-1;for(int i=0;i<6;++i)if(strcmp(reinterpret_cast<const char*>(loaded.addon.Formulas[i].CustomFname),argv[1])==0){slot=i;break;}
  // The loader migrates legacy parameterless flips to their C-capable
  // successors with the C flag disabled. Probe the requested legacy code,
  // using that slot's buffer; these three legacy formulas have no options.
  if(slot<0 && (strcmp(argv[1],"_FlipXY")==0 || strcmp(argv[1],"_FlipXZ")==0 || strcmp(argv[1],"_FlipYZ")==0)) {
   char migrated[32]{};snprintf(migrated,sizeof(migrated),"%sc",argv[1]);
   for(int i=0;i<6;++i)if(strcmp(reinterpret_cast<const char*>(loaded.addon.Formulas[i].CustomFname),migrated)==0){slot=i;break;}
  }
  if(slot<0)return 6;
  mb3d::ApplyExternalFormulaOptionTypes(loaded.addon.Formulas[slot]);
  mb3d::BuildFormulaConstBuffer(slot,&loaded.addon,slot);
  selected=loaded.addon.Formulas[slot];
  selectedSlot=slot;
  memcpy(prepared,mb3d::GetFormulaConstBuffers().buf[slot],1024);
  FILE* out=fopen(argv[3],"wb");if(!out)return 7;fwrite(prepared,1,1024,out);fclose(out);
 }
 // Parameter audits must remain possible for rejected numerical inputs. Empty
 // stdin is also a dump-only request used by existing layout checkers.
 if(dumpOnly)return 0;
 bool inputValidated=false;
 double x,y,z,w,j1,j2,j3,p0,p1,p2,p3;
 while(scanf("%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf",&x,&y,&z,&w,&j1,&j2,&j3,&p0,&p1,&p2,&p3)==11){
  alignas(16) unsigned char params[1024]{};void* var=params+128;
  memcpy(params+112,&p0,8);memcpy(params+104,&p1,8);memcpy(params+96,&p2,8);memcpy(params+88,&p3,8);
  if (strstr(argv[1],"FlipXYc") || strstr(argv[1],"FlipXZc") || strstr(argv[1],"FlipYZc")) { int flip=int(p0); memcpy(params+116,&flip,4); }
  if(sceneMode){memcpy(params,prepared,1024);var=params+512;}
  if(!inputValidated){
   const auto issues=mb3d::ValidateExternalFormulaInput(selected,var,selectedSlot);
   if(!issues.empty()){
    for(const auto& issue:issues)fprintf(stderr,"%s\n",issue.message.c_str());
    return 12; // Unsupported input, not a numerical mismatch or a parity PASS.
   }
   inputValidated=true;
  }
  TIteration3Dext ext{};ext.x=x;ext.y=y;ext.z=z;ext.Rout=x*x+y*y+z*z;ext.w=w;ext.C1=x+.125;ext.C2=y-.25;ext.C3=z+.5;ext.PVar=var;ext.J1=j1;ext.J2=j2;ext.J3=j3;
 if(sceneMode && std::getenv("MB3D_POINT_ROUT"))ext.Rout=p0;
 if(sceneMode){ext.VaryScale=1;ext.Deriv1=1;ext.Rold=.001;ext.RStopD=4;ext.maxIt=20;ext.bFirstIt=-1;}
 if(sphereMap){ext.PMapFunc=sphereMapProbe;mapU=mapV=mapW=0;mapNumber=mapCalls=0;}
 if(heightMap){ext.PMapFunc2=heightMapProbe;ext.Rold=p0;ext.Dfree1=p1;mapU=mapV=0;mapNumber=mapCalls=0;}
 if(sceneMode && std::getenv("MB3D_POINT_MAP_CONTEXT")){ext.Rold=p0;ext.Dfree1=p1;}
 if(sceneMode && std::getenv("MB3D_POINT_FIRST"))ext.bFirstIt=static_cast<int32_t>(p1);
 if(sceneMode && std::getenv("MB3D_POINT_ITERATION"))ext.ItResultI=static_cast<int32_t>(p0);
 if(sceneMode && std::getenv("MB3D_POINT_DERIVATIVES")){ext.Deriv1=p1;ext.Deriv2=p2;ext.Deriv3=p3;}
 if(sceneMode && std::getenv("MB3D_POINT_JULIA_MODE"))ext.DoJulia=static_cast<int32_t>(p3);
 if(counters){ext.bFirstIt=static_cast<int32_t>(p0);for(int k=0;k<6;++k){uint32_t bits=static_cast<uint32_t>(p2)+k; if(static_cast<uint32_t>(p1)&(1u<<k))bits|=0x80000000u;memcpy(&ext.nHybrid[k],&bits,4);}}
 if(sseState && scanf("%lf %lf %lf %lf", &ext.FormulaSse01[0], &ext.FormulaSse01[1], &ext.FormulaSse01[2], &ext.FormulaSse01[3])!=4)return 9;
 auto it=reinterpret_cast<TPIteration3D>(&ext.C1);
  f->wrapperFunc(ext.x,ext.y,ext.z,ext.w,it);printf("%.17g %.17g %.17g %.17g %.17g %.17g %.17g %.17g\n",ext.x,ext.y,ext.z,ext.w,ext.J1,ext.J2,ext.J3,ext.J4);
 if(sceneMode)printf("%.17g %.17g %.17g %.17g %.17g %d %.17g %.17g %.17g %.17g %.17g %d\n",ext.C1,ext.C2,ext.C3,ext.Rout,ext.VaryScale,ext.bFirstIt,ext.Dfree1,ext.Dfree2,ext.Deriv1,ext.Deriv2,ext.Deriv3,ext.ItResultI);
 if(counters)for(int k=0;k<6;++k)printf(k==5?"%d\n":"%d ",ext.nHybrid[k]);
 if(sphereMap)printf("%.17g %.17g %.17g %d %d\n",mapU,mapV,mapW,mapNumber,mapCalls);
 if(sseState)printf("%.17g %.17g %.17g %.17g\n",ext.FormulaSse01[0],ext.FormulaSse01[1],ext.FormulaSse01[2],ext.FormulaSse01[3]);
 if(heightMap)printf("%.17g %.17g %d %d\n",mapU,mapV,mapNumber,mapCalls);
 }
 return 0;
}

int main(int argc, char** argv) {
 try {
  return runProbe(argc, argv);
 } catch (const mb3d::FormulaWorkError& error) {
  // Work exhaustion is an explicit failed evaluation, never a numeric row or
  // unsupported-original-input PASS. Batch callers discard all rows on failure.
  fprintf(stderr, "%s\n", error.what());
  return 13;
 } catch (const std::exception& error) {
  fprintf(stderr, "Formula point probe failed: %s\n", error.what());
  return 1;
 }
}
