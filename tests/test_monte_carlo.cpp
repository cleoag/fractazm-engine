#include "core/calc_monte_carlo.h"
#include "core/external_formulas.h"
#include "core/renderer.h"
#include "core/lightmap.h"
#include "io/m3p_parser.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <cstring>
#include "fixtures/mc_sampling_reference.h"
#include "fixtures/mc_lens_reference.h"
#include "fixtures/mc_subpixel_reference.h"
#include "fixtures/mc_codec_reference.h"
#include "fixtures/mc_adaptive_reference.h"
#include "fixtures/mc_camera_streams_reference.h"
#include "fixtures/mc_threshold_reference.h"
#include <limits>
#include <stdexcept>
using namespace mb3d;
int main(int argc, char **argv) {
  if (argc != 2)
    return 2;
  InitExternalFormulaRegistry();
  auto input = LoadM3PFile(argv[1]);
  if (!input.success) {
    puts(input.error.c_str());
    return 2;
  }
  int failed = 0;
  auto check = [&](bool ok, const char *msg) {
    if (!ok) {
      printf("FAIL: %s\n", msg);
      ++failed;
    }
  };
  const auto fromBits = [](uint64_t bits) {
    double value; std::memcpy(&value,&bits,8); return value;
  };
  for (const auto& ref : mcThresholdReference) {
    float stop; std::memcpy(&stop,&ref.stop,4);
    check(MCDERefinementRequired(fromBits(ref.distance),stop)==ref.refine,
          "MC refinement boundary matches original Pascal typed threshold");
  }
  {
    const float values[] = {mc_reference::s0001,mc_reference::s001};
    uint32_t bits[2]; std::memcpy(bits,values,8);
    check(bits[0]==981668463u && bits[1]==1008981770u,
          "MC tolerances match original Pascal Single constant bits");
  }
  for (const auto& ref : mcCameraAnglesReference) {
    const auto value = CalcMCCameraAngles(float(ref.x),float(ref.y),ref.width,
                                          ref.height,ref.fx,ref.fy);
    uint64_t bits[2]; std::memcpy(bits,value.data(),sizeof(bits));
    check(bits[0]==ref.expected[0] && bits[1]==ref.expected[1],
          "MC camera angle bits match extracted Pascal");
  }
  for (const auto& ref : mcStreamsReference) {
    const auto shift = GetMCHaltonShifts(ref.x,ref.y);
    const float value = ShiftMCHalton(ref.word,float(ref.shift));
    uint32_t seed = ref.seed, bits[3];
    const double random = MCNextRandom(seed);
    uint64_t randomBits;
    std::memcpy(bits,shift.data(),8); std::memcpy(bits+2,&value,4);
    std::memcpy(&randomBits,&random,8);
    check(bits[0]==ref.expected[0] && bits[1]==ref.expected[1] &&
          bits[2]==ref.expected[2] && randomBits==ref.expected[3] && seed==ref.expected[4],
          "MC shifts, Word conversion, RNG bits/state match extracted Pascal");
  }
  const auto close = [](double a,double b) {
    return std::isfinite(a) && std::isfinite(b) &&
      std::abs(a-b) <= 2e-12*std::max({1.,std::abs(a),std::abs(b)});
  };
  for (const auto& ref : mcCodecReference) {
    uint8_t guarded[5]{0xa5,0,0,0,0x5a};
    EncodeMC24(ref.input,guarded+1);
    const uint32_t code = guarded[1] | (uint32_t(guarded[2])<<8) | (uint32_t(guarded[3])<<16);
    check(code==ref.packed && guarded[0]==0xa5 && guarded[4]==0x5a,
          "MC packing matches Pascal and preserves adjacent bytes");
    MCPackedRecord r{};
    r.ysum[0]=uint8_t(ref.code); r.ysum[1]=uint8_t(ref.code>>8); r.ysum[2]=uint8_t(ref.code>>16);
    std::memcpy(r.ysqr,guarded+1,3); r.rayCount=ref.count;
    check(close(DecodeMC24(r.ysum),fromBits(ref.decoded)) &&
          close(NoiseOfMCRecord(r),fromBits(ref.noise)), "MC decode and noise match independent Pascal");
  }
  for (double invalid : {std::numeric_limits<double>::infinity(),std::numeric_limits<double>::quiet_NaN()}) {
    uint8_t bytes[3]{1,2,3}; bool rejected=false;
    try { EncodeMC24(invalid,bytes); } catch(const std::invalid_argument&) { rejected=true; }
    check(rejected && bytes[0]==1 && bytes[1]==2 && bytes[2]==3,"MC packing rejects nonfinite input without writing");
  }
  for (const auto& ref : mcAdaptiveReference) {
    const auto d=CalcMCAdaptiveSamples(ref.bytes,ref.size,ref.w,ref.h,ref.x,ref.y,
                                     ref.variance,ref.average,ref.random);
    check(d.samples==ref.samples && d.consumedRandom==ref.used,
          "CalcN sample count and random consumption match independent Pascal");
  }
  {
    uint8_t pixels[36]{}; bool rejected=false;
    try { CalcMCAdaptiveSamples(pixels,sizeof(pixels),2,1,0,0,.002,4,.5); }
    catch(const std::out_of_range&) { rejected=true; }
    check(rejected,"CalcN detects Pascal neighbor counter read beyond the image");
    rejected=false;
    try { CalcMCAdaptiveSamples(pixels,sizeof(pixels),2,1,0,0,0,4,.5); }
    catch(const std::invalid_argument&) { rejected=true; }
    check(rejected,"CalcN rejects zero average variance");
  }
  // Boundary and mode regressions from the independently compiled Pascal helper.
  for (const auto& ref : mcSamplingReference) {
    float x = ref.x, y = ref.y;
    float shape = CalcBokeh(x, y, ref.mode);
    MakeDiscFromHalton(x, y);
    uint32_t actual[3];
    std::memcpy(&actual[0], &x, 4);
    std::memcpy(&actual[1], &y, 4);
    std::memcpy(&actual[2], &shape, 4);
    check(std::memcmp(actual, ref.bits, sizeof(actual)) == 0,
          "disc and bokeh match independent Pascal float bits");
  }
  for (const auto& ref : mcLensReference) {
    const auto lens = CalcMCLensSample(ref.x, ref.y, ref.mode, ref.aperture,
                                      ref.focus, ref.width, ref.step);
    const float values[] = {lens.x, lens.y, lens.focus, lens.aperture, lens.scale};
    uint32_t actual[5];
    std::memcpy(actual, values, sizeof(actual));
    check(std::memcmp(actual, ref.bits, sizeof(actual)) == 0,
          "lens initialization and scaling match independent Pascal float bits");
  }
  for (const auto& ref : mcSubpixelReference) {
    int used = 0;
    float x, y;
    GetMCSubPixelShift(ref.gaussian, [&]() {
      if (used >= ref.count) {
        check(false, "subpixel sampler exhausted reference random stream");
        return .5;
      }
      return ref.stream[used++];
    }, x, y);
    uint32_t actual[2];
    std::memcpy(actual, &x, 4);
    std::memcpy(actual + 1, &y, 4);
    check(std::memcmp(actual, ref.bits, sizeof(actual)) == 0 && used == ref.used,
          "subpixel sampling and random consumption match independent Pascal");
  }
  // Asymmetric colors catch channel-order errors; 48-bit maps have a six-byte
  // pixel stride and must not be read through the 32-bit bitmap path.
  TLightMap map{};
  uint32_t pixel32[4]{0x00FF8040, 0, 0, 0};
  map.LMWidth = map.LMHeight = 1;
  map.LMa = pixel32;
  map.sIntensity = 2;
  TSVec sampled{};
  GetLightMapPixelNN(sampled, 0, 0, &map, 0);
  check(std::abs(sampled[0] - 2) < 1e-6 && std::abs(sampled[2] - 128.0 / 255) < 1e-6,
        "nearest 32-bit map preserves RGB and intensity");
  alignas(4) uint16_t pixel48[12]{0, 32768, 65535};
  map.iMapType = 1;
  map.LMa = reinterpret_cast<uint32_t*>(pixel48);
  GetLightMapPixelNN(sampled, 0, 0, &map, 1);
  check(std::abs(sampled[0] - 2) < 1e-6 && sampled[2] == 0 &&
        std::abs(sampled[1] - 2 * std::pow(32768.0 / 65535, 2)) < 1e-6,
        "nearest 48-bit map squares normalized channels before intensity");
  RenderOptions o;
  o.width_override = 24;
  o.height_override = 18;
  o.mc_samples = 8;
  o.thread_count = 1;
  o.deterministic_mode = true;
  auto render = [&](RenderOptions opts, TMandHeader10 h) {
    auto addon = input.addon;
    return Render(h, addon, opts);
  };
  auto a = render(o, input.header);
  check(a.success, a.error.c_str());
  o.thread_count = 4;
  auto b = render(o, input.header);
  check(b.success, b.error.c_str());
  check(a.mc_linear == b.mc_linear,
        "radiance must be exactly independent of thread count");
  check(a.image == b.image,
        "display must be exactly independent of thread count");
  check(a.mc_variance == b.mc_variance,
        "variance must be exactly independent of thread count");
  check(a.mc_linear.size() == 24 * 18 * 3, "linear buffer dimensions");
  check(a.mc_sample_counts.size() == 24 * 18, "count buffer dimensions");
  check(std::all_of(a.mc_sample_counts.begin(), a.mc_sample_counts.end(),
                    [](auto n) { return n == 8; }),
        "every pixel has eight samples");
  check(std::all_of(a.mc_variance.begin(), a.mc_variance.end(),
                    [](float x) { return std::isfinite(x) && x >= 0; }),
        "variance finite and nonnegative");
  check(std::any_of(a.mc_variance.begin(), a.mc_variance.end(),
                    [](float x) { return x > 0; }),
        "samples must vary");
  check(std::any_of(a.silight.begin(), a.silight.end(),
                    [](auto p) { return p.Zpos < 32768; }),
        "test scene must contain geometry");
  ++o.deterministic_seed;
  auto c = render(o, input.header);
  check(c.success, c.error.c_str());
  check(c.mc_linear != a.mc_linear, "seed changes sampling");
  auto dof = input.header;
  dof.bCalcDOFtype |= 1;
  dof.sDOFaperture = .2f;
  dof.sDOFZsharp = .5f;
  auto d = render(o, dof);
  check(d.success, d.error.c_str());
  check(d.mc_linear != c.mc_linear, "lens sampling affects radiance");
  o.mc_samples = 1;
  auto one = render(o, input.header);
  check(one.success, one.error.c_str());
  check(std::all_of(one.mc_variance.begin(), one.mc_variance.end(),
                    [](float v) { return v == 0; }),
        "single sample variance is zero");
  o.mc_samples = -1;
  check(!render(o, input.header).success, "negative sample count rejected");
  o.mc_samples = 65536;
  check(!render(o, input.header).success,
        "sample count beyond Halton table rejected");
  o.mc_samples = 1;
  o.mc_max_steps = 1;
  check(!render(o, input.header).success,
        "exhausted ray budget must fail, not produce a success image");
  o.mc_max_steps = 100000;
  auto bad = input.header;
  bad.bCalcSRautomatic = 3;
  bad.sTRIndex = 0;
  check(!render(o, bad).success, "invalid refractive index rejected");
  bad.sTRIndex = 1.5;
  bad.sTRscattering = 1;
  check(render(o, bad).success, "scattering mode accepts valid medium settings");
  o.mc_samples = 0;
  auto regular = render(o, input.header);
  check(regular.success, regular.error.c_str());
  check(regular.mc_linear.empty(), "regular renderer remains the default");
  printf("Monte Carlo checks: %d failures\n", failed);
  return failed ? 1 : 0;
}
