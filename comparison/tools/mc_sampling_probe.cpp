#include "core/calc_monte_carlo.h"
#include "core/math3d.h"
#include "core/header_trafos.h"
#include "core/ray_marcher.h"
#include "core/formulas.h"
#include <cstdio>
#include <cstring>
#include <cstdint>
#include <algorithm>
#include <vector>
#include <stdexcept>
#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void Rout3_compiled_pas_sse2(double*, const double*);
extern "C" void CalcRawDe_x87_extcw(double*, double*, const double*);
#endif
static std::vector<float> iterationSequence;
static size_t iterationSequenceIndex;
static double iterationSequenceDE(TPIteration3Dext it, void*) {
  it->SmoothItD = iterationSequence[std::min(iterationSequenceIndex++, iterationSequence.size()-1)];
  return 0;
}
int main(int argc, char** argv) {
#if defined(_MSC_VER) && defined(_M_X64)
  if (argc == 2 && strcmp(argv[1], "--radius-de") == 0) {
    double v[6]; unsigned long long input;
    while (scanf("%llu", &input)==1) {
      memcpy(v,&input,8);
      for (int i=1;i<6;++i) {
        if (scanf("%llu",&input)!=1) return 2;
        memcpy(v+i,&input,8);
      }
      double result[3];
      Rout3_compiled_pas_sse2(result,v);
      CalcRawDe_x87_extcw(result+1,result+2,v);
      for (int i=0;i<3;++i) { memcpy(&input,result+i,8); printf("%llu%c",input,i==2?'\n':' '); }
    }
    return 0;
  }
#endif
  if (argc == 2 && strcmp(argv[1], "--integer-power8") == 0) {
    double v[14]; unsigned long long input;
    while (scanf("%llu",&input)==1) {
      memcpy(v,&input,8);
      for (int i=1;i<14;++i) {
        if (scanf("%llu",&input)!=1) return 2;
        memcpy(v+i,&input,8);
      }
      TIteration3Dext state{};
      auto* it=reinterpret_cast<TPIteration3D>(reinterpret_cast<char*>(&state)+56);
      it->J1=v[3]; it->J2=v[4]; it->J3=v[5];
      alignas(16) double params[32]{}; double* base=params+4;
      base[-2]=v[6]; base[18]=v[7]; base[19]=v[8]; base[20]=v[9];
      base[24]=v[10]; base[26]=v[11]; base[4]=v[12]; base[3]=v[13];
      it->PVar=base; double w=0;
      mb3d::HybridIntP8(v[0],v[1],v[2],w,it);
      for (int i=0;i<3;++i) { memcpy(&input,v+i,8); printf("%llu%c",input,i==2?'\n':' '); }
    }
    return 0;
  }
  if (argc == 2 && strcmp(argv[1], "--smooth-iterations") == 0) {
    unsigned long long radius,oldRadius; unsigned logStop,power; int iterations;
    while (scanf("%llu %llu %u %u %d",&radius,&oldRadius,&logStop,&power,&iterations)==5) {
      TIteration3Dext state{};
      memcpy(&state.Rout,&radius,8); memcpy(&state.Rold,&oldRadius,8);
      auto* it=reinterpret_cast<TPIteration3D>(reinterpret_cast<char*>(&state)+56);
      memcpy(&it->LNRStop,&logStop,4); memcpy(&it->fHln[0],&power,4);
      it->ItResultI=iterations;
      mb3d::CalcSmoothIterations(it,0);
      uint32_t bits; memcpy(&bits,&it->SmoothItD,4); printf("%u\n",bits);
    }
    return 0;
  }
  if (argc == 2 && strcmp(argv[1], "--march-step-limit") == 0) {
    float stop,limiter;
    while (scanf("%f %f",&stop,&limiter)==2) {
      const double result=mb3d::CalcMarchStepLimit(stop,limiter);
      uint64_t bits; memcpy(&bits,&result,8); printf("%llu\n",(unsigned long long)bits);
    }
    return 0;
  }
  if (argc == 2 && strcmp(argv[1], "--refine-ray-step") == 0) {
    double v[8]; float factor,stop;
    while (scanf("%lf %lf %lf %lf %lf %lf %lf %lf %f %f",
        v,v+1,v+2,v+3,v+4,v+5,v+6,v+7,&factor,&stop)==10) {
      TVec3D pos{v[0],v[1],v[2]},dir{v[5],v[6],v[7]};
      const float result=mb3d::RefineRayStep(pos,v[3],dir,v[4],factor,stop);
      for (double x : {pos[0],pos[1],pos[2],v[3]}) {
        uint64_t bits; memcpy(&bits,&x,8); printf("%llu ",(unsigned long long)bits);
      }
      uint32_t bits; memcpy(&bits,&result,4); printf("%u\n",bits);
    }
    return 0;
  }
  if (argc == 2 && strcmp(argv[1], "--iteration-refine") == 0) {
    int steps,maxIt,count; unsigned long long z;
    while (scanf("%d %d %llu %d",&steps,&maxIt,&z,&count)==4) {
      if(count<1 || count>1000 || steps<0 || steps>100) return 2;
      iterationSequence.resize(count); iterationSequenceIndex=0;
      for(auto& v:iterationSequence) { unsigned bits; if(scanf("%u",&bits)!=1)return 2; memcpy(&v,&bits,4); }
      TMCTparameter p{}; TIteration3Dext it{};
      p.pIt3Dext=&it; p.CalcDE=iterationSequenceDE; p.iMaxIt=maxIt; p.iMaxitF2=maxIt;
      p.iDEAddSteps=steps; it.maxIt=maxIt; memcpy(&p.mZZ,&z,8);
      mb3d::RMdoBinSearchIt(&p,p.mZZ); memcpy(&z,&p.mZZ,8);
      unsigned smooth;memcpy(&smooth,&it.SmoothItD,4);
      printf("%llu %u %zu %d %d %d\n",z,smooth,iterationSequenceIndex,it.maxIt,p.iMaxIt,p.iMaxitF2);
    }
    return 0;
  }
  if (argc == 2 && strcmp(argv[1], "--iteration-step") == 0) {
    unsigned a,b,c; unsigned long long m;
    while (scanf("%u %u %u %llu",&a,&b,&c,&m)==4) {
      float target,previous,current; double multiplier;
      memcpy(&target,&a,4); memcpy(&previous,&b,4); memcpy(&current,&c,4); memcpy(&multiplier,&m,8);
      double result=mb3d::CalcIterationBoundaryStep(target,previous,current,multiplier);
      memcpy(&m,&result,8); printf("%llu\n",m);
    }
    return 0;
  }
  if (argc == 2 && strcmp(argv[1], "--depth-codec") == 0) {
    unsigned long long z,c,m; unsigned rough,smooth;
    while (scanf("%llu %llu %llu %u %u", &z,&c,&m,&rough,&smooth)==5) {
      TMCTparameter parameters{}; TsiLight5 light; memset(&light,0xA5,sizeof(light));
      double depth; memcpy(&depth,&z,8); memcpy(&parameters.Zcorr,&c,8);
      memcpy(&parameters.ZcMul,&m,8); memcpy(&parameters.sRoughness,&rough,4);
      parameters.iSmNormals=static_cast<uint16_t>(smooth);
      mb3d::CalcZposAndRough(&light,&parameters,depth);
      uint32_t packed; memcpy(&packed,&light.RoughZposFine,4);
      const auto* bytes=reinterpret_cast<const unsigned char*>(&light);
      bool guard=true;
      for (size_t i=0;i<sizeof(light);++i) if ((i<6 || i>=10) && bytes[i]!=0xA5) guard=false;
      printf("%u %u\n",packed,unsigned(guard));
    }
    return 0;
  }
  if (argc == 2 && strcmp(argv[1], "--camera-row-angle") == 0) {
    int y,height; double fovy;
    while (scanf("%d %d %lf", &y,&height,&fovy)==3) {
      const double result=mb3d::CalcCameraRowAngle(y,height,fovy);
      uint64_t bits; memcpy(&bits,&result,8); printf("%llu\n",(unsigned long long)bits);
    }
    return 0;
  }
  if (argc == 2 && strcmp(argv[1], "--ray-step-limiter") == 0) {
    int width,height; float step,limit;
    while (scanf("%d %d %f %f", &width,&height,&step,&limit)==4) {
      const float result=mb3d::CalcRayStepLimiter(width,height,step,limit);
      uint32_t bits; memcpy(&bits,&result,4); printf("%u\n",bits);
    }
    return 0;
  }
  if (argc == 2 && strcmp(argv[1], "--pack-normals") == 0) {
    unsigned long long a,b,c;
    while (scanf("%llu %llu %llu", &a,&b,&c) == 3) {
      TVec3D input; memcpy(&input[0], &a, 8); memcpy(&input[1], &b, 8); memcpy(&input[2], &c, 8);
      int16_t output[3]; mb3d::MakeWNormalsFromDVec(output, input);
      printf("%d %d %d\n", int(output[0]),int(output[1]),int(output[2]));
    }
    return 0;
  }
  if (argc == 2 && strcmp(argv[1], "--clip15") == 0) {
    unsigned bits;
    while (scanf("%u", &bits) == 1) {
      float input; memcpy(&input, &bits, 4);
      uint16_t output; mb3d::MinMaxClip15bit(input, output);
      printf("%u\n", unsigned(output));
    }
    return 0;
  }
  if (argc == 2 && strcmp(argv[1], "--thresholds") == 0) {
    uint32_t a,b;
    const float small=mb3d::mc_reference::s0001, large=mb3d::mc_reference::s001;
    memcpy(&a,&small,4); memcpy(&b,&large,4);
    printf("%u %u\n",a,b);
    double distance; float stop;
    while (scanf("%lf %f",&distance,&stop)==2)
      printf("%d\n",int(mb3d::MCDERefinementRequired(distance,stop)));
    return 0;
  }
  if (argc == 2 && strcmp(argv[1], "--halton-shifts") == 0) {
    unsigned x,y,seed,word; float shift;
    while (scanf("%u %u %u %u %f", &x,&y,&seed,&word,&shift)==5) {
      if (word>65535) return 2;
      auto result = mb3d::GetMCHaltonShifts(x,y);
      const float shifted = mb3d::ShiftMCHalton(uint16_t(word),shift);
      const double random = mb3d::MCNextRandom(seed);
      uint32_t a,b,c; uint64_t r;
      memcpy(&a,&result[0],4); memcpy(&b,&result[1],4); memcpy(&c,&shifted,4);
      memcpy(&r,&random,8);
      printf("%u %u %u %llu %u\n", a,b,c,(unsigned long long)r,seed);
    }
    return 0;
  }
  if (argc == 2 && strcmp(argv[1], "--camera-angles") == 0) {
    float x,y; int width,height; double fx,fy;
    while (scanf("%f %f %d %d %lf %lf", &x,&y,&width,&height,&fx,&fy)==6) {
      const auto result = mb3d::CalcMCCameraAngles(x,y,width,height,fx,fy);
      uint64_t a,b; memcpy(&a,&result[0],8); memcpy(&b,&result[1],8);
      printf("%llu %llu\n", (unsigned long long)a,(unsigned long long)b);
    }
    return 0;
  }
  if (argc == 2 && strcmp(argv[1], "--halton") == 0) {
    const auto* table = mb3d::GetHaltonSequence();
    for (int i = 0; i < 65536; ++i)
      printf("%u %u %u %u\n", unsigned(table[i].HRx), unsigned(table[i].HRy),
             unsigned(table[i].HDx), unsigned(table[i].HDy));
    int index, base;
    while (scanf("%d %d", &index, &base) == 2) {
      if (base < 2) return 2;
      const double value = mb3d::HaltonRadicalInverse(index, base);
      uint64_t bits; memcpy(&bits, &value, 8);
      printf("%llu\n", (unsigned long long)bits);
    }
    return 0;
  }
  if (argc == 2 && strcmp(argv[1], "--adaptive") == 0) {
    int w,h,x,y,n; double variance,random; float average;
    while (scanf("%d %d %d %d %lf %f %lf %d", &w,&h,&x,&y,&variance,&average,&random,&n)==8) {
      if (n < 18 || n > 4096) return 2;
      std::vector<uint8_t> bytes(n);
      for (auto& b : bytes) { unsigned v; if (scanf("%u",&v)!=1 || v>255) return 2; b=uint8_t(v); }
      try {
        auto d = mb3d::CalcMCAdaptiveSamples(bytes.data(),bytes.size(),w,h,x,y,variance,average,random);
        printf("%d %d\n",d.samples,int(d.consumedRandom));
      } catch (const std::exception&) { return 3; }
    }
    return 0;
  }
  if (argc == 3 && strcmp(argv[1], "--decode-file") == 0) {
    FILE* output = fopen(argv[2], "wb");
    if (!output) return 2;
    double block[4096];
    for (uint32_t base = 0; base < 0x1000000; base += 4096) {
      for (unsigned j = 0; j < 4096; ++j) {
        const uint32_t value = base + j;
        const uint8_t bytes[] = {uint8_t(value), uint8_t(value >> 8), uint8_t(value >> 16)};
        block[j] = mb3d::DecodeMC24(bytes);
      }
      if (fwrite(block, sizeof(block), 1, output) != 1) { fclose(output); return 3; }
    }
    return fclose(output) == 0 ? 0 : 3;
  }
  if (argc == 2 && strcmp(argv[1], "--codec") == 0) {
    double value; unsigned packed, count;
    while (scanf("%lf %u %u", &value, &packed, &count) == 3) {
      if (packed > 0xffffff || count > 65535) return 2;
      uint8_t encoded[5]{0xa5,0,0,0,0x5a};
      mb3d::MCPackedRecord record{};
      record.ysum[0] = uint8_t(packed);
      record.ysum[1] = uint8_t(packed >> 8);
      record.ysum[2] = uint8_t(packed >> 16);
      record.rayCount = uint16_t(count);
      try { mb3d::EncodeMC24(value, encoded + 1); }
      catch (const std::exception&) { return 2; }
      memcpy(record.ysqr, encoded + 1, 3);
      double decoded = mb3d::DecodeMC24(record.ysum);
      double noise = mb3d::NoiseOfMCRecord(record);
      uint64_t db, nb; memcpy(&db, &decoded, 8); memcpy(&nb, &noise, 8);
      unsigned code = encoded[1] | (unsigned(encoded[2]) << 8) | (unsigned(encoded[3]) << 16);
      printf("%u %llu %llu %u %u\n", code, (unsigned long long)db,
             (unsigned long long)nb, unsigned(encoded[0]), unsigned(encoded[4]));
    }
    return 0;
  }
  if (argc == 2 && strcmp(argv[1], "--subpixel") == 0) {
    int gaussian, count;
    while (scanf("%d %d", &gaussian, &count) == 2) {
      if (count < 2 || count > 4096) return 2;
      std::vector<double> stream(count);
      for (double& value : stream) if (scanf("%lf", &value) != 1) return 2;
      size_t used = 0;
      float x, y;
      try {
        mb3d::GetMCSubPixelShift(gaussian != 0, [&]() { return stream.at(used++); }, x, y);
      } catch (const std::out_of_range&) { return 3; }
      uint32_t bits[2]; memcpy(bits, &x, 4); memcpy(bits + 1, &y, 4);
      printf("%u %u %zu\n", bits[0], bits[1], used);
    }
    return 0;
  }
  if (argc == 2 && strcmp(argv[1], "--lens") == 0) {
    float x, y, aperture, focus; int mode, width; double step;
    while (scanf("%f %f %d %f %f %d %lf", &x, &y, &mode, &aperture, &focus, &width, &step) == 7) {
      const auto sample = mb3d::CalcMCLensSample(x, y, mode, aperture, focus, width, step);
      float result[] = {sample.x, sample.y, sample.focus, sample.aperture, sample.scale};
      uint32_t bits[5]; memcpy(bits, result, sizeof(bits));
      printf("%u %u %u %u %u\n", bits[0], bits[1], bits[2], bits[3], bits[4]);
    }
    return 0;
  }
  float x, y; int mode;
  while (scanf("%f %f %d", &x, &y, &mode) == 3) {
    float bokeh = mb3d::CalcBokeh(x, y, mode);
    mb3d::MakeDiscFromHalton(x, y);
    uint32_t bits[3];
    memcpy(&bits[0], &x, 4); memcpy(&bits[1], &y, 4); memcpy(&bits[2], &bokeh, 4);
    printf("%u %u %u\n", bits[0], bits[1], bits[2]);
  }
}
