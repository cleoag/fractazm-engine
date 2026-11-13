// Native Monte Carlo integration. See CalcMonteCarlo.pas for reference
// equations. Fixed sample counts make runs bounded and repeatable; this is not
// the GUI's adaptive CalcN scheduling or its packed progressive accumulator
// format.
#include "core/calc_monte_carlo.h"
#include "core/colorizer.h"
#include "core/header_trafos.h"
#include "core/lightmap.h"
#include "core/math3d.h"
#include "core/ray_marcher.h"
#include "core/reflection.h"
#include "core/renderer.h"
#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <cstring>
#include <future>
#include <fstream>
#include <iomanip>
#include <cstdlib>
#include <stdexcept>

namespace mb3d {
namespace {
using V = std::array<double, 3>;
constexpr double tau = 6.2831853071795864769;
V operator+(V a, V b) {
  for (int i = 0; i < 3; ++i)
    a[i] += b[i];
  return a;
}
V operator-(V a, V b) {
  for (int i = 0; i < 3; ++i)
    a[i] -= b[i];
  return a;
}
V operator*(V a, double s) {
  for (auto &v : a)
    v *= s;
  return a;
}
V operator*(V a, V b) {
  for (int i = 0; i < 3; ++i)
    a[i] *= b[i];
  return a;
}
double dot(V a, V b) { return a[0] * b[0] + a[1] * b[1] + a[2] * b[2]; }
double length(V a) { return std::sqrt(dot(a, a)); }
V unit(V a) {
  double l = length(a);
  return l > 1e-100 ? a * (1 / l) : V{0, 0, 1};
}
V cross(V a, V b) {
  return {a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2],
          a[0] * b[1] - a[1] * b[0]};
}
V read(const double *p) { return {p[0], p[1], p[2]}; }
V read(const float *p) { return {p[0], p[1], p[2]}; }
void put(double *p, V v) { std::copy(v.begin(), v.end(), p); }
void put(float *p, V v) {
  for (int i = 0; i < 3; ++i)
    p[i] = static_cast<float>(v[i]);
}
double sat(double x) { return std::clamp(x, 0.0, 1.0); }
double sq(double x) { return x * x; }
double luminance(V v) { return .3 * v[0] + .59 * v[1] + .11 * v[2]; }
V power(V v, double p) {
  for (auto &x : v)
    x = std::pow(std::max(0.0, x), p);
  return v;
}
V rotate(V v, const TMatrix3 &m, bool reverse) {
  TVec3D d;
  put(d, v);
  if (reverse)
    RotateVectorReverse(d, m);
  else
    RotateVector(d, m);
  return read(d);
}
std::array<V, 2> orthoBasis(V n) {
  n = unit(n);
  // Math3D.MakeOrthoVecs: the basis orientation determines finite samples.
  V a;
  if (std::abs(n[0]) > .1) {
    double d = 1 / std::sqrt(n[0] * n[0] + n[2] * n[2]);
    a = {float(n[2] * d), 0, float(-n[0] * d)};
  } else {
    double d = 1 / std::sqrt(n[1] * n[1] + n[2] * n[2]);
    a = {0, float(-n[2] * d), float(n[1] * d)};
  }
  V b = cross(a, n);
  for (auto &v : b) v = float(v);
  return {a, b};
}
V ortho(V n, double x, double y, double z, bool doubleSum = false) {
  auto basis = orthoBasis(n);
  V result;
  // ScaleSVector takes Single arguments and returns Single components.
  // AddSVectors rounds their sum too; VaryVecSphere uses AddSVectorsToDVec.
  for (int i = 0; i < 3; ++i) {
    double transverse = double(float(basis[0][i] * float(x))) +
                        double(float(basis[1][i] * float(y)));
    result[i] = (doubleSum ? transverse : double(float(transverse))) + n[i] * z;
  }
  return result;
}
struct RNG {
  uint32_t state;
  double next() {
    return MCNextRandom(state);
  }
};
std::array<float, 2> disc(float u, float v) {
  MakeDiscFromHalton(u, v);
  return {u, v};
}
// PaintThread.pas: PaintColorBuffer, D65 Lab saturation/gamma/HDR path.
// The SSE Halley root iteration is intentional; cbrt is a different curve.
V displayColor(V value, const TMandHeader10 &h, bool squareColor) {
  double contrast = sq(h.MCcontrast / 256.0 + .5);
  bool soft = h.MCoptions & 1;
  int gamma = (h.Light.TBoptions >> 21) & 0xfc;
  if (soft) {
    gamma = std::max(0, gamma - 20);
    contrast *= 1.1;
  }
  for (auto &c : value) {
    if (squareColor)
      c = std::sqrt(std::max(0.0, c));
    c = sq(std::max(0.0, c * contrast));
  }
  V xyz{
      (.412453 * value[0] + .35758 * value[1] + .180423 * value[2]) * 1.052111,
      .212671 * value[0] + .71516 * value[1] + .072169 * value[2],
      (.019334 * value[0] + .119193 * value[1] + .950227 * value[2]) * .918417};
  for (auto &c : xyz) {
    float threshold = 216.f / 24389.f, r = std::max(float(c), threshold),
          w = .4275f;
    for (int i = 0; i < 3; ++i) {
      float cube = w * w * w;
      w = w * (cube + 2 * r) / (2 * cube + r);
    }
    c = w + (std::min(float(c), threshold) - threshold) * (841.f / 108.f);
  }
  double L = 1.16 * xyz[1] - .16, A = xyz[0] - xyz[1], B = xyz[1] - xyz[2],
         sat = (h.bMCSaturation & 127) / 32.0;
  if (L > 0) {
    double old = L;
    if (gamma > 128) {
      L += (std::sqrt(L) - L) * ((gamma - 128) / 127.0);
      if (old > 1)
        sat *= L / old;
    } else if (gamma < 128)
      L += (L * L - L) * (1 - gamma / 128.0);
    if (soft) {
      old = L;
      L = std::sqrt(L * L / std::sqrt(sq(L * L * .9) + 1));
      sat *= L / old;
    }
  }
  double Y = (L + .16) / 1.16;
  xyz = {Y + A * sat, Y, Y - B * sat};
  for (auto &c : xyz) {
    float t = 6.f / 29.f, r = std::max(float(c), t);
    c = r * r * r + (std::min(float(c), t) - t) * (108.f / 841.f);
  }
  xyz[0] *= .95047;
  xyz[2] *= 1.08883;
  value = {3.240479 * xyz[0] - 1.537150 * xyz[1] - .498535 * xyz[2],
           -.969256 * xyz[0] + 1.875992 * xyz[1] + .041556 * xyz[2],
           .055648 * xyz[0] - .204043 * xyz[1] + 1.057311 * xyz[2]};
  for (auto &c : value)
    c = std::sqrt(std::max(0.0, c));
  return value;
}
struct Material {
  V diffuse{}, specular{};
  double transparency = 0;
};
struct Hit {
  bool hit = false, cutSurface = false, inside = false;
  V position{}, normal{};
  double z = 0, distance = 0;
  float fog = 0;
  TsiLight5 surface{};
  int deMode = 0;
  Material material;
};

class PathTracer {
  const TMandHeader10 &h;
  const RenderOptions &options;
  TMCTparameter m;
  TMCTparameter initialParams;
  TIteration3Dext it{};
  TLightVals lights;
  TLValigned aligned;
  TCalcThreadStats stats{};
  int32_t stop = 0;
  RNG rng{0};
  float hx = 0, hy = 0, dx = 0, dy = 0;
  V primary{};
  int row = 0;
  uint16_t primaryZ = 32768;
  bool initialInside = false;
  uint64_t rayBudget = 0;
  double sr = 0, ambLength = 0;
  std::ofstream diagnostic;
  int sampleIndex = 0;
  void diagnosticEvent(const char* event, int depth, std::initializer_list<double> values) {
    if (!diagnostic.is_open()) return;
    diagnostic << sampleIndex << ' ' << depth << ' ' << event;
    for (double v : values) diagnostic << ' ' << std::setprecision(17) << v;
    diagnostic << '\n';
  }
  V position() { return {it.C1, it.C2, it.C3}; }
  void position(V p) {
    it.C1 = p[0];
    it.C2 = p[1];
    it.C3 = p[2];
  }
  double de(V p, double z) {
    position(p);
    m.mZZ = std::abs(z);
    m.msDEstop = float(m.DEstop * (1 + std::abs(z) * m.mctDEstopFactor));
    double d = m.CalcDE(&it, &m);
    if (!std::isfinite(d))
      throw std::runtime_error("MC: nonfinite distance estimate");
    return d;
  }
  V normals(V p, V direction, double z, float &smooth) {
    it.CalcSIT = 1;
    double base;
    if (m.NormalsOnDE)
      base = de(p, z);
    else {
      position(p);
      m.mMandFunction(reinterpret_cast<TPIteration3D>(&it.C1));
      base = it.SmoothItD;
    }
    smooth = float(it.SmoothItD);
    it.CalcSIT = 0;
    float scale = (rng.state & 128) ? -.15f : .15f;
    for (int retry = 0; retry < 4; ++retry) {
      if (retry)
        scale = (rng.next() - .5f) * .5f;
      double e = std::min(1.0, double(m.DEstop)) *
                 (1 + std::abs(z) * m.mctDEstopFactor) * scale * m.StepWidth;
      V n{};
      for (int axis : {2, 0, 1}) {
        V q = p;
        q[axis] += e;
        if (m.NormalsOnDE)
          n[axis] = (de(q, z) - base) * scale;
        else {
          position(q);
          it.CalcSIT = 1;
          m.mMandFunction(reinterpret_cast<TPIteration3D>(&it.C1));
          n[axis] = (smooth - it.SmoothItD) * scale;
          it.CalcSIT = 0;
        }
      }
      if (dot(n, n) > 1e-100 && dot(n, direction) < 0) {
        position(p);
        return unit(n);
      }
    }
    position(p);
    return unit(direction) * -1;
  }
  Material color(Hit &hit) {
    TSVec diffuse{}, specular{};
    diffuse[0] = float(lights.sColZmul *
                       (std::abs(hit.z) * m.StepWidth + m.sZZstmitDif));
    if (hit.surface.SIgradient > 32767)
      CalcColorsInside(&diffuse, &specular, &hit.surface, &lights);
    else
      CalcColors(&diffuse, &specular, &hit.surface, &lights);
    V saved = read(diffuse);
    if (lights.iColOnOT > 1 && lights.DiffColLightMap) {
      if (lights.iColOnOT > 5) {
        V p = hit.position * lights.lvMapScale, n = hit.normal;
        double total = std::abs(n[0]) + std::abs(n[1]) + std::abs(n[2]);
        for (auto &v : n)
          v = std::abs(v) / total;
        if (lights.iColOnOT > 7) {
          TSVec a{}, b{}, c{};
          GetLightMapPixel(a, float(FracA(p[1] + lights.DCLMapOffX)),
                           float(FracA(p[2] + lights.DCLMapOffY)),
                           lights.DiffColLightMap, lights.bCalcPixColSqr, 1);
          GetLightMapPixel(b, float(FracA(p[0] + lights.DCLMapOffX)),
                           float(FracA(p[2] + lights.DCLMapOffY)),
                           lights.DiffColLightMap, lights.bCalcPixColSqr, 1);
          GetLightMapPixel(c, float(FracA(p[0] + lights.DCLMapOffX)),
                           float(FracA(p[1] + lights.DCLMapOffY)),
                           lights.DiffColLightMap, lights.bCalcPixColSqr, 1);
          put(diffuse, read(a) * n[0] + read(b) * n[1] + read(c) * n[2]);
        } else {
          for (auto &v : p)
            v = std::sin(v);
          GetLightMapPixel(diffuse,
                           float(FracA(p[1] * n[0] + p[0] * (n[1] + n[2]) +
                                       lights.DCLMapOffX)),
                           float(FracA(p[2] * (n[0] + n[1]) + p[1] * n[2] +
                                       lights.DCLMapOffY)),
                           lights.DiffColLightMap, lights.bCalcPixColSqr, 1);
        }
      } else if (lights.iColOnOT > 3) {
        TSVec n{};
        put(n, unit(rotate(hit.normal, m.Vgrads, false)));
        GetLightMapPixelSphere(diffuse, n,
                               &lights.DiffColLightMap->PicRotMatrix,
                               *lights.DiffColLightMap, lights.bCalcPixColSqr);
      } else {
        double x = (hit.surface.OTrap & 32767) * 3.05186851e-5 - .5,
               y = hit.surface.SIgradient * 3.05186851e-5 - .5;
        GetLightMapPixel(
            diffuse,
            float(FracA((lights.DCLMapRotCos * x + lights.DCLMapRotSin * y +
                         lights.DCLMapOffX) *
                        lights.lvMapScale)),
            float(FracA((lights.DCLMapRotCos * y - lights.DCLMapRotSin * x +
                         lights.DCLMapOffY) *
                        lights.lvMapScale)),
            lights.DiffColLightMap, lights.bCalcPixColSqr, 1);
      }
      if (lights.bYCcomb)
        put(diffuse,
            saved * (luminance(read(diffuse)) / (mc_reference::s001 + luminance(saved))));
    }
    Material result{read(diffuse), read(specular), specular[3]};
    if ((h.MCoptions & 4) && sr > 0) {
      double trans = (h.bCalcSRautomatic & 2) ? 1 - result.transparency : 1;
      V sum = result.specular * sr + result.diffuse * trans;
      double maximum = *std::max_element(sum.begin(), sum.end());
      if (maximum > 1) {
        result.specular = result.specular * (1 / maximum);
        result.diffuse = result.diffuse * (1 / maximum);
      }
    }
    return result;
  }
  void refineIterationSurface(V &p, V direction, double &z, double projection) {
      // Literal doBinSearchIt: smooth-iteration refinement is different from
      // the DE threshold search, including its asymmetric finite difference.
      const int savedMaxIt = it.maxIt;
      const float target = float(it.maxIt - .99);
      ++it.maxIt;
      it.CalcSIT = 1;
      double delta = 0, damping = 1, refinedZ = z;
      float lastSI = 0, lastDifference = 0;
      for (int step = 0; step < 11; ++step) {
        refinedZ += delta;
        p = p + direction * delta;
        position(p);
        m.CalcDE(&it, &m);
        if (step && lastDifference < std::abs(target - it.SmoothItD)) {
          refinedZ -= delta;
          p = p - direction * delta;
          it.SmoothItD = lastSI;
          damping *= delta > 0 ? .5 : .7;
        }
        lastDifference = float(std::abs(target - it.SmoothItD));
        lastSI = float(it.SmoothItD);
        if (it.SmoothItD > it.maxIt - .1) delta = -3;
        else {
          refinedZ -= mc_reference::s0001;
          p = p - direction * .001;
          position(p);
          m.CalcDE(&it, &m);
          float slope = float(lastSI - it.SmoothItD);
          delta = std::abs(slope) < 1e-30 ? double(lastSI < it.SmoothItD) - .5
                  : (target - it.SmoothItD) / (slope * (slope < 0 ? 500 : 1000));
          if (delta > 4) delta = std::sqrt(delta) * 2;
          else if (delta < -9) delta = std::sqrt(-delta) * -3;
          delta = delta * damping + .0005;
        }
      }
      it.maxIt = savedMaxIt;
      z += (refinedZ - z) * projection;
  }
  Hit march(V origin, V direction, double z, bool inside, double maximum,
            bool secondary) {
    if (++rayBudget > 1000000)
      throw std::runtime_error("MC: path branching budget exhausted");
    Hit hit;
    hit.inside = inside;
    hit.surface.Zpos = 32768;
    hit.surface.AmbShadow = 5000;
    m.bInsideRendering = inside;
    m.bCalcInside = inside;
    it.CalcSIT = 0;
    double traveled = 0, projection = dot(direction, primary) / sq(m.StepWidth),
           lastDE = 0, lastStep = 0;
    float factor = 1; // Pascal RSFmul is Single, including the secant proposal.
    position(origin);
    // Intersect enabled cutting half-spaces using the shared Pascal port.
    int cut = 0;
    double cutDistance = 0;
    if (m.iCutOptions) {
      if (secondary) {
        V planes{m.dCOX, m.dCOY, m.dCOZ};
        for (int axis = 0; axis < 3; ++axis)
          if ((m.iCutOptions & (1 << axis)) &&
              std::abs(direction[axis]) > 1e-20) {
            double distance = (planes[axis] - origin[axis]) / direction[axis];
            if (distance > .1 && distance < maximum)
              maximum = distance;
          }
      } else {
        m.mZZ = z;
        RMmaxLengthToCutPlane(&m, cutDistance, cut,
                              reinterpret_cast<TPPos3D>(&it.C1));
        if (cutDistance >= maximum) {
          hit.position = origin + direction * maximum;
          hit.z = z + maximum * projection;
          hit.distance = maximum;
          return hit;
        }
        if (cutDistance > 0) {
          origin = origin + direction * cutDistance;
          z += cutDistance * projection;
          traveled += cutDistance;
        }
      }
    }
    double d = de(origin, z);
    V p = origin;
    lastStep = d * m.sZstepDiv;
    auto integrateVolume = [&](double distance) {
      TVec3D q, v;
      put(q, p); put(v, primary);
      // Pascal DoDynFog samples the previous segment using mVgradsFOV,
      // including its initial DE-sized segment and the final partial segment.
      VolLightDoDynFogSubstep(q, v, float(distance), std::abs(z),
          m.mctDEstopFactor, m.VLstepmul, m.VLmul, m.StepWidth, hit.fog);
    };
    for (int step = 0;; ++step) {
      if (step >= options.mc_max_steps)
        throw std::runtime_error("MC: ray march budget exhausted");
      // Primary rays starting in a cut surface are accepted immediately.
      if (step == 0 && !secondary &&
          (d < m.msDEstop || it.ItResultI >= m.MaxItsResult)) {
        if (m.bInAndOutside) {
          inside = !inside;
          hit.inside = inside;
          m.bInsideRendering = inside;
          m.bCalcInside = inside;
          d = de(p, z);
        } else {
          hit.hit = true;
          hit.cutSurface = true;
          break;
        }
      }
      lastDE = d;
      diagnosticEvent("march", secondary ? 2 : 1,
                      {z,d,m.msDEstop,factor,p[0],p[1],p[2],double(it.ItResultI)});
      if (m.DFogOnIt == 65535) integrateVolume(lastStep);
      double ds =
          std::max(.11, (d - m.msDEsub * m.msDEstop) * m.sZstepDiv * factor);
      double limiter = std::max(.4, double(m.msDEstop)) * m.mctMH04ZSD;
      if (m.DFogOnIt != 65535) {
        // CalcRay's MaxCS takes/returns Single; d1 is Single too.
        ds = std::max(.11f, float((d - m.msDEsub * m.msDEstop) * m.sZstepDiv * factor));
        limiter = std::max(.4f, m.msDEstop) * m.mctMH04ZSD;
      }
      if (m.DFogOnIt != 65535 &&
          (m.DFogOnIt == 0 || it.ItResultI == m.DFogOnIt))
        hit.fog += float(std::min(1.0, limiter / ds));
      ds = std::min(ds, limiter);
      if (step == 0)
        ds *= rng.next();
      lastStep = ds;
      if (traveled + ds > maximum) {
        if (m.DFogOnIt == 65535) {
          const double partial = std::max(0.0, maximum - traveled);
          p = p + direction * partial;
          integrateVolume(partial);
        }
        traveled += ds;
        break;
      }
      traveled += ds;
      p = p + direction * ds;
      z += ds * projection;
      d = de(p, z);
      if (it.ItResultI >= m.MaxItsResult ||
          (d < m.msDEstop && it.ItResultI >= m.iMinIt)) {
        hit.hit = true;
        break;
      }
      d = std::min(d, lastDE + lastStep);
      factor =
          lastDE > d + 1e-30 ? std::clamp(lastStep / (lastDE - d), .5, 1.0) : 1;
    }
    const double unrefinedDistance = traveled;
    const bool limited = it.ItResultI < m.MaxItsResult;
    if (hit.hit && lastStep > 0 && !limited) {
      refineIterationSurface(p, direction, z, projection);
    } else if (hit.hit && lastStep > 0) {
      double delta = -lastStep * .5;
      for (int i = 0; i < (h.MCoptions & 2 ? 4 : 10); ++i) {
        if (limited && !MCDERefinementRequired(d, m.msDEstop))
          break;
        diagnosticEvent("refine", secondary ? 2 : 1,
                        {z,d,m.msDEstop,delta,double(10-i)});
        if ((h.MCoptions & 2) && limited) {
          const double denom = NotZero(lastDE - d);
          const float proposal = float(lastStep * (d - m.msDEstop) / denom);
          double a = std::abs(lastStep);
          delta =
              d < m.msDEstop
                  ? ((proposal >= 0 || proposal < -.94 * a) ? -.5 * a
                                                            : proposal)
                  : ((proposal <= 0 || proposal > .94 * a) ? .5 * a : proposal);
        }
        p = p + direction * delta;
        z += delta * projection;
        traveled += delta;
        lastDE = d;
        lastStep = delta;
        m.msDEstop = m.DEstop * (1 + std::abs(z) * m.mctDEstopFactor);
        if (i + 1 == (h.MCoptions & 2 ? 4 : 10)) break;
        d = de(p, z);
        delta = (limited ? d < m.msDEstop : it.ItResultI >= m.MaxItsResult)
                    ? -std::abs(delta) * .55
                    : std::abs(delta) * .55;
      }
    }
    if (inside && m.DFogOnIt < 65535)
      hit.fog = float(hit.fog * 200 * m.DEstop / m.iMandWidth);
    hit.position = p;
    hit.z = z;
    hit.distance = unrefinedDistance;
    position(p);
    m.mZZ = std::abs(z);
    if (hit.hit) {
      if (hit.cutSurface) {
        m.mPsiLight = &hit.surface;
        if (m.ColorOnIt) RMdoColorOnIt(&m);
        RMdoColor(&m);
        V n{0, 0, -1};
        if (cut) {
          const int axis = cut - 1;
          double scale = std::abs(m.Vgrads[2][axis]) < 1e-40 ? -1e40 : -1 / m.Vgrads[2][axis];
          n = {m.Vgrads[0][axis] * scale, m.Vgrads[1][axis] * scale, -1};
        }
        hit.normal = unit(rotate(n, m.Vgrads, true));
        if (m.ColorOption > 4) hit.surface.SIgradient |= 32768;
        else if (inside) {
          it.CalcSIT = 1;
          de(p, z);
          hit.surface.SIgradient = 32768 | uint16_t(std::lrint(std::clamp(double(it.SmoothItD) * m.mctsM, 0.0, 32767.0)));
        } else hit.surface.SIgradient = 32768 + uint16_t(std::lrint(32767 * sat(it.Rout / m.dRStop)));
        hit.material = color(hit);
        CalcZposAndRough(&hit.surface, &m, z);
        return hit;
      }
      float smooth = 0;
      hit.normal = normals(p, direction, z, smooth);
      hit.deMode = m.DEoptionResult;
      double gradient =
          32767 -
          (smooth + (limited ? m.dColPlus +
           m.mctColVarDEstopMul *
               std::log(std::max(1e-100, double(m.msDEstop) * m.StepWidth)) : 0)) *
              m.mctsM;
      hit.surface.SIgradient = uint16_t(std::lrint(std::clamp(gradient, 0.0, 32767.0)));
      m.mPsiLight = &hit.surface;
      if (m.ColorOnIt)
        RMdoColorOnIt(&m);
      RMdoColor(&m);
      CalcZposAndRough(&hit.surface, &m, std::max(0.0, unrefinedDistance));
      hit.surface.NormalX = int16_t(hit.normal[0] * 32767);
      hit.surface.NormalY = int16_t(hit.normal[1] * 32767);
      hit.surface.NormalZ = int16_t(hit.normal[2] * 32767);
      hit.material = color(hit);
    }
    return hit;
  }
  V background(V direction, bool ambient, bool first, bool allowMap = true) {
    V d = unit(direction);
    TSVec out{};
    if (allowMap && lights.bBackBMP && lights.BGLightMap) {
      TSVec v{};
      put(v, d);
      const bool nearest = !first && lights.BGLightMap->LMWidth * int(h.MCdiffReflects) > 5000;
      if (lights.bDirectImageCoord) {
        V camera = unit(rotate(d, m.Vgrads, false));
        float sx = float(m.iMandHeight / (m.FOVy * m.iMandWidth));
        float x, y, t;
        if (first) {
          if (m.MCTCameraOptic == 1) {
            t = float(m.mctPlOpticZ / (std::abs(camera[2]) < 1e-100 ? 1e-100 : camera[2]));
            y = float(camera[1] * t / m.FOVy + .5);
            x = float(camera[0] * sx * t + .5);
          } else if (m.MCTCameraOptic == 2) {
            y = float(std::asin(std::clamp(camera[1], -1.0, 1.0)) / m.FOVy + .5);
            x = float(std::atan2(camera[0], camera[2]) / tau + .5);
          } else {
            t = float(std::sqrt(.5 + std::sqrt(.25 + sq(camera[0] * camera[1]))));
            y = float(std::asin(std::clamp(camera[1] * t, -1.0, 1.0)) / m.FOVy + .5);
            x = float(std::asin(std::clamp(camera[0] * t, -1.0, 1.0)) * sx + .5);
          }
        } else {
          y = float(std::asin(std::clamp(camera[1], -1.0, 1.0)) / m.FOVy + .5);
          t = float(m.FOVy / std::max(.1, tau - m.FOVy));
          if (y > 1) { y = 1 + t * (1 - y); if (y < .75) y = 1.5f - y; }
          else if (y < 0) { y = std::abs(y) * t; if (y > .25) y = .5f - y; }
          x = float(std::atan2(camera[0], camera[2]) * sx + .5);
          t = float(1 / (sx * std::max(.1, tau - 1 / double(sx))));
          if (x > 1) x = 1 + t * (1 - x);
          else if (x < 0) x = std::abs(x) * t;
        }
        if (nearest) GetLightMapPixelNN(out, float(sat(x)), float(sat(y)), lights.BGLightMap, lights.bCalcPixColSqr);
        else GetLightMapPixel(out, float(sat(x)), float(sat(y)), lights.BGLightMap, lights.bCalcPixColSqr, 0);
      } else if (nearest)
        GetLightMapPixelSphereNN(out, v, &lights.BGLightMap->PicRotMatrix,
                                 *lights.BGLightMap, lights.bCalcPixColSqr);
      else
        GetLightMapPixelSphere(out, v, &lights.BGLightMap->PicRotMatrix,
                               *lights.BGLightMap, lights.bCalcPixColSqr);
    } else {
      if (!lights.bAmbRelObj)
        d = unit(rotate(d, m.Vgrads, false));
      double t;
      const float *a, *b;
      if (ambient) {
        t = d[1] * .5 + .5;
        a = aligned.sAmbCol;
        b = aligned.sAmbCol2;
      } else {
        t = !lights.bAmbRelObj
                ? (first ? double(row) / m.iMandHeight
                         : sat(std::asin(std::clamp(d[1], -1.0, 1.0)) / m.FOVy +
                               .5))
                : sat(std::asin(std::clamp(d[1], -1.0, 1.0)) / M_PI + .5);
        if (!lights.bAmbRelObj) {
          if (lights.iDfunc == 1)
            t *= t;
          else if (lights.iDfunc == 2)
            t = std::sqrt(t);
        }
        a = aligned.sDepthCol;
        b = aligned.sDepthCol2;
      }
      put(out, (read(a) * (1 - t) + read(b) * t) * (1.0 / 256));
    }
    if (ambient)
      for (int i = 0; i < 6; ++i)
        if (lights.iLightOption[i] == 2 && lights.LLightMaps[i]) {
          TSVec v{}, c{};
          put(v, rotate(direction, m.Vgrads, true));
          GetLightMapPixelSphereNN(c, v, &lights.LLightMaps[i]->PicRotMatrix,
                                 *lights.LLightMaps[i], lights.bCalcPixColSqr);
          put(out, read(out) + read(c));
        }
    return read(out);
  }
  V visible(const Hit &hit, V origin, V direction, bool ambient, bool first,
            double &decrease, double &depth) {
    double bg = hit.hit ? std::max(0.0, 1 + (int(hit.surface.Zpos) - 28000) *
                                                double(lights.sDepth))
                        : std::max(0.0, 1 - (60768 - int(hit.surface.Zpos)) * double(lights.sDepth));
    if (bg < 1) {
      bg = 1 - bg;
      if (lights.bCalcPixColSqr)
        bg *= bg;
      if (lights.bFarFog)
        bg *= bg;
      bg = 1 - bg;
    }
    depth = bg;
    decrease = 1;
    V result{}, dir = unit(direction);
    if (!ambient)
      for (int i = 0; i < 6; ++i)
        if (lights.iLightPos[i] & 14) {
          double dist, radius = lights.sLmaxL[i] * 1e-8;
          if (lights.iLightPos[i] & 1) {
            V to = read(m.HSvecs[i]) - origin;
            double along = dot(to, dir);
            if (along < 0 ||
                (hit.hit && along > hit.distance * m.StepWidth +
                                        std::sqrt(std::max(0.0, radius))))
              continue;
            dist = std::max(0.0, dot(to, to) - along * along);
          } else {
            if (hit.hit)
              continue;
            dist = std::max(0.0, 1 - dot(read(aligned.LN[i]),
                                         unit(rotate(dir, m.Vgrads, false))));
          }
          if (dist < radius) {
            float flux = float(dist), trans = float(radius);
            CalcPosLightShape(flux, trans, i, &lights);
            result = result + read(aligned.sLCols[i]) *
                                  (flux / 255.0 * (bg * .9 + .1) * decrease);
            decrease *= std::min(1.0, double(trans));
          }
        }
    double fog =
        (hit.fog - lights.sShad -
         lights.sShadZmul * ((hit.hit ? hit.distance : m.Zend) * m.StepWidth + m.sZZstmitDif)) *
        lights.sShadGr;
    if (lights.bDFogOptions & 2)
      fog = std::max(0.0, fog);
    double f2 = std::min(1.0, hit.fog * double(lights.sDynFogMul)) * fog;
    result = result +
             background(direction, false, first, false) * std::max(0.0, 1 - bg);
    if (lights.bDFogOptions & 1) {
      fog = sat(fog);
      f2 = sat(f2);
      result = result * (1 - fog);
      depth *= 1 - fog;
    }
    return result + (read(aligned.sDynFogCol) * (fog - f2) +
                     read(aligned.sDynFogCol2) * f2) *
                        (1.0 / 255);
  }
  struct Shadow { V transmission{1, 1, 1}; double specular = 1; };
  Shadow shadow(const Hit &surface, V light, double limit, double maxLength,
                bool inside) {
    if (options.no_shadows) return {};
    // CalcHSMC starts with a deterministic step. The transparent branch uses
    // the same entry search, then probes backwards from the light through
    // the medium. It is not another primary/ambient ray.
    const auto savedM = m;
    const auto savedIt = it;
    Shadow result;
    V p = surface.position;
    double remaining = limit, z = surface.z;
    const float projection = float(dot(unit(light), unit(primary)));
    if (m.iCutOptions) {
      V planes{m.dCOX, m.dCOY, m.dCOZ};
      for (int axis = 0; axis < 3; ++axis)
        if ((m.iCutOptions & (1 << axis)) && std::abs(light[axis]) > 1e-20) {
          double t = (planes[axis] - p[axis]) / light[axis];
          if (t > 0 && t < remaining) remaining = t;
        }
    }
    const double initialLength = remaining;
    m.bInsideRendering = inside;
    m.bCalcInside = inside;
    float factor = 2, previous = 0, lastStep = 0;
    double d = de(p, z);
    const bool transparent = sr > 0 && (h.bCalcSRautomatic & 2) &&
                            (!(h.bCalcSRautomatic & 4) || surface.deMode == 20);
    if (remaining > mc_reference::s0001) {
      for (int step = 0;; ++step) {
        if (step >= options.mc_max_steps)
          throw std::runtime_error("MC: shadow march budget exhausted");
        previous = float(d);
        double distance = std::min(double(float(d * m.sZstepDiv * factor)),
                         double(float(std::max(double(m.msDEstop), .4) * m.mctMH04ZSD)));
        lastStep = float(distance);
        remaining -= distance;
        p = p + light * distance;
        z += distance * projection;
        d = de(p, z);
        if (it.ItResultI >= m.MaxItsResult || d < m.msDEstop) break;
        d = std::min(d, double(previous + lastStep));
        factor = previous > d + 1e-30
                   ? float(std::clamp(lastStep / (previous - d), .5, 1.0)) : 1;
        if (remaining < 0) break;
      }
      if (remaining > mc_reference::s001) {
        const double opaque = std::min(1.0, std::pow((initialLength - remaining) / maxLength, 8));
        result.transmission = {opaque, opaque, opaque};
        if (transparent) {
          if (it.ItResultI < m.MaxItsResult) {
            for (int step = 0; step < 3 && std::abs(d - m.msDEstop) > mc_reference::s001; ++step) {
              previous = float(previous - d);
              if (std::abs(previous) < 1e-30f) previous = previous < 0 ? -1e-30f : 1e-30f;
              factor = float(lastStep * (d - m.msDEstop) / previous);
              const float extent = std::abs(lastStep);
              lastStep = d < m.msDEstop
                ? ((factor >= 0 || factor < extent * -.94) ? extent * -.5f : factor)
                : ((factor <= 0 || factor > extent * .94) ? extent * .5f : factor);
              previous = float(d);
              z += lastStep * projection;
              p = p + light * lastStep;
              d = de(p, z);
            }
          } else refineIterationSurface(p, light, z, projection);
          Hit blocker;
          blocker.hit = true;
          blocker.position = p;
          blocker.z = z;
          float smooth = 0;
          blocker.normal = normals(p, light, z, smooth);
          double gradient = 32767 - (smooth + (it.ItResultI < m.MaxItsResult
            ? m.dColPlus + m.mctColVarDEstopMul * std::log(std::max(1e-100, double(m.msDEstop) * m.StepWidth)) : 0)) * m.mctsM;
          blocker.surface.SIgradient = uint16_t(std::lrint(std::clamp(gradient, 0.0, 32767.0)));
          m.mPsiLight = &blocker.surface;
          if (m.ColorOnIt) RMdoColorOnIt(&m);
          RMdoColor(&m);
          CalcZposAndRough(&blocker.surface, &m, z);
          blocker.material = color(blocker);
          const int entryMode = m.DEoptionResult;
          const auto &material = blocker.material;
          if (material.transparency > 0 && (!(h.bCalcSRautomatic & 4) || entryMode == 20)) {
            const float absorption = float(h.sTransmissionAbsorption * m.StepWidth);
            if (m.bInAndOutside) result.transmission = power(material.diffuse, absorption);
            else {
              result.specular = sq(dot(blocker.normal, light) / m.StepWidth);
              m.bInsideRendering = !inside;
              m.bCalcInside = !inside;
              p = p + light * remaining;
              d = rng.next() * 8 * std::max(double(m.msDEstop), 1.0);
              for (int step = 0;; ++step) {
                if (step >= options.mc_max_steps)
                  throw std::runtime_error("MC: transparent shadow budget exhausted");
                d = -1 - std::abs(d);
                remaining += d;
                p = p + light * d;
                position(p);
                d = m.CalcDE(&it, &m); // Pascal keeps the entry DE threshold.
                if (!std::isfinite(d)) throw std::runtime_error("MC: nonfinite transparent shadow DE");
                if (d > m.msDEstop || remaining <= 0) break;
              }
              result.transmission = entryMode == m.DEoptionResult
                ? power(material.diffuse, std::max(0.0, remaining) * absorption) *
                    (result.specular * material.transparency *
                     *std::max_element(material.specular.begin(), material.specular.end())) : V{};
            }
          }
        }
      }
    }
    m = savedM;
    it = savedIt;
    return result;
  }
  std::array<V, 2> direct(const Hit &hit, V reflected, bool inside) {
    std::array<V, 2> result{};
    std::array<V, 6> targets{};
    const bool doDOF = !options.no_dof && (h.bCalcDOFtype & 1);
    const double radius = ShortFloatToSingle(h.MCSoftShadowRadius);
    // Pascal varies every light before tracing any shadow. Even zero-radius
    // lights consume their random draws, which determines subsequent paths.
    for (int i = 0; i < 6; ++i)
      if (lights.iLightOption[i] == 0) {
        V original = read(m.HSvecs[i]);
        if (lights.iLightPos[i] & 1) {
          double draw = rng.next();
          double rr = ((lights.iLightPos[i] & 14) == 2 ||
                       (lights.iLightPos[i] & 14) == 8)
                          ? sq(draw) * 1e-4 : std::sqrt(draw) * 3e-4;
          double u = dx, v = dy;
          if (doDOF) { v = rng.next(); u = rng.next(); }
          V sphere{double(float(2 * std::sqrt(v * (1 - v)) * std::cos(tau * u))),
                   double(float(2 * std::sqrt(v * (1 - v)) * std::sin(tau * u))),
                   double(float(1 - 2 * v))};
          targets[i] = original + unit(sphere) *
                         (std::sqrt(lights.sLmaxL[i]) * rr * radius);
        } else {
          double u = doDOF ? rng.next() : dx;
          double v = doDOF ? rng.next() : dy;
          double rr = std::sqrt(u) * std::sqrt(lights.sLmaxL[i]) *
                      ((lights.iLightPos[i] & 14) == 8 ? 1e-5 : 2.5e-5) * radius * m.StepWidth;
          targets[i] = unit(ortho(original, float(std::sin(tau * v)) * rr,
                                 float(std::cos(tau * v)) * rr, 1)) * m.StepWidth;
        }
      }
    for (int i = 0; i < 6; ++i)
      if (lights.iLightOption[i] == 0) {
        V original = read(m.HSvecs[i]), target = targets[i];
        bool positional = (lights.iLightPos[i] & 1) != 0;
        double limit = (m.iMandWidth + m.iMandHeight) * .6 *
                       m.sHSmaxLengthMultiplier *
                       (1 + .5 * std::min(std::abs(hit.z), m.Zend * .4) *
                                std::max(0.0, m.FOVy) / m.iMandHeight);
        V direction = positional ? unit(target - hit.position) * m.StepWidth
                                 : target * -1;
        if (dot(hit.normal, direction) <= 0)
          continue;
        const double maxLength = limit;
        const bool shadowInRange = !positional || dot(target - hit.position, target - hit.position) <= lights.sLmaxL[i] * m.sHSmaxLengthMultiplier;
        if (positional)
          limit = std::min(limit, length(target - hit.position) / m.StepWidth);
        Shadow shading = shadowInRange ? shadow(hit, direction, limit, maxLength, inside) : Shadow{};
        V base = read(aligned.sLCols[i]);
        V ld = positional ? original - hit.position : original * -1;
        double distance2 = dot(ld, ld);
        ld = unit(ld);
        if (positional)
          base = base * (1 / (distance2 + 1e-26));
        base = base * shading.transmission;
        result[0] =
            result[0] + base * (FastIntPow(float(dot(unit(reflected), ld)),
                                           lights.iLightPowFunc[i]) *
                                lights.sSpec / 255.0 * shading.specular);
        result[1] = result[1] + base * (sq(std::max(0.0, dot(hit.normal, ld))) *
                                        lights.sDiff / 255.0);
      }
    return result;
  }
  // CalcMonteCarlo.pas AddLight: scattering along a segment inside material.
  V mediumLight(V origin, V end, V diffuse) {
    V result{}, segment = end - origin;
    for (int i = 0; i < 6; ++i)
      if (lights.iLightOption[i] == 0) {
        double weight = 1;
        if (lights.iLightPos[i] & 1) {
          V light = read(aligned.LN[i]), a = light - origin, b = light - end;
          double epsilon = sq(lights.sStepWidth), d2 = dot(a, a) + epsilon,
                 d3 = dot(b, b) + epsilon, t = dot(a, segment);
          if (t <= 0)
            weight = .25 / d3 + 1 / d2;
          else {
            double len = dot(segment, segment);
            if (len <= t)
              weight = .25 / d2 + 1 / d3;
            else {
              V v = light - (origin + segment * (t / len));
              weight = 1 / (dot(v, v) + epsilon) + .25 / std::max(d2, d3);
            }
          }
        }
        result = result + read(aligned.sLCols[i]) * weight;
      }
    return result * diffuse;
  }
  V trace(V origin, V direction, double z, bool inside, bool ambient, int depth,
          V throughput, Material medium, Hit *firstHit = nullptr) {
    if (luminance({std::abs(throughput[0]), std::abs(throughput[1]),
                   std::abs(throughput[2])}) < 1e-4)
      return {};
    if (depth > 260)
      throw std::runtime_error("MC: recursion budget exhausted");
    diagnosticEvent("start", depth, {origin[0],origin[1],origin[2],direction[0],direction[1],direction[2],z,double(rng.state),throughput[0],throughput[1],throughput[2]});
    m.bCalcAmbShadow = ambient;
    double maximum = ambient ? ambLength * (sq(rng.next()) + .75) : m.Zend;
    Hit hit = march(origin, direction, z, inside, maximum, depth > 1);
    inside = hit.inside;
    if (depth == 1 && hit.hit) primaryZ = hit.surface.Zpos;
    if (!hit.hit && (lights.bFarFog || lights.bCalcPixColSqr) && std::abs(lights.sDepth) > 1e-10) {
      float firstFog = float(1 - std::max(0.0, 1 + (int(primaryZ) - 28000) * double(lights.sDepth)));
      float endFog = float(1 - std::max(0.0, 1 - 28000 * double(lights.sDepth)));
      if (firstFog > 0) { firstFog *= firstFog; if (lights.bFarFog && lights.bCalcPixColSqr) firstFog *= firstFog; }
      if (endFog > 0) { endFog *= endFog; if (lights.bFarFog && lights.bCalcPixColSqr) endFog *= endFog; }
      firstFog = endFog - firstFog;
      if (firstFog > 0) { firstFog = std::sqrt(firstFog); if (lights.bFarFog && lights.bCalcPixColSqr) firstFog = std::sqrt(firstFog); }
      hit.surface.Zpos = uint16_t(std::lrint(std::clamp(60768.0 - firstFog / lights.sDepth, 32768.0, 65535.0)));
    }
    diagnosticEvent("hit", depth, {double(hit.hit),hit.position[0],hit.position[1],hit.position[2],hit.z,hit.normal[0],hit.normal[1],hit.normal[2],double(hit.surface.SIgradient),double(hit.surface.OTrap),double(rng.state),hit.material.diffuse[0],hit.material.diffuse[1],hit.material.diffuse[2]});
    if (firstHit)
      *firstHit = hit;
    double decrease, depthDecay;
    V radiance = visible(hit, origin, direction, ambient, depth == 1, decrease,
                         depthDecay) *
                 throughput;
    bool transmission = sr > 0 && (h.bCalcSRautomatic & 2) &&
                        (!(h.bCalcSRautomatic & 4) || hit.deMode == 20);
    if (transmission && !m.bInAndOutside && (inside != initialInside)) {
      V attenuation =
            power(medium.diffuse, std::min(hit.distance, maximum) *
                                      h.sTransmissionAbsorption * m.StepWidth),
        before = throughput;
      throughput = throughput * attenuation;
      double amount = (1 - luminance(attenuation)) * h.sTRscattering / 330.0;
      if (amount != 0) {
        V light = mediumLight(origin - V{m.Xmit, m.Ymit, m.Zmit},
                              hit.position - V{m.Xmit, m.Ymit, m.Zmit},
                              medium.diffuse);
        radiance = radiance + light *
                                  (hit.hit ? (throughput + before) * .5
                                           : throughput * medium.diffuse) *
                                  amount;
      }
    } else
      throughput = throughput * (decrease * depthDecay);
    if (!hit.hit) {
      if (lights.bBackBMP && lights.bAddBGlight && depthDecay != 0 && !transmission)
        throughput = throughput * (1 / depthDecay);
      return radiance + background(direction, ambient, depth == 1) * throughput;
    }
    V n = hit.normal, reflect = direction - n * (2 * dot(n, direction)),
      refract{};
    Material material = hit.material;
    if (hit.cutSurface) {
      std::array<V, 2> illumination{};
      for (int i = 0; i < 6; ++i) if (lights.iLightOption[i] == 0) {
        V light = read(m.HSvecs[i]), intensity = read(aligned.sLCols[i]);
        if (lights.iLightPos[i] & 1) {
          light = light - hit.position;
          intensity = intensity * (1 / (dot(light, light) + 1e-60));
        } else light = light * -1;
        light = unit(light);
        illumination[0] = illumination[0] + intensity *
          (FastIntPow(float(dot(unit(reflect), light)), lights.iLightPowFunc[i]) * lights.sSpec / 255);
        illumination[1] = illumination[1] + intensity * (sq(std::max(0.0, dot(n, light))) * lights.sDiff / 255);
      }
      V ambientRay = ortho(n, float(std::sin(tau * hx)) * float(std::sqrt(hy)),
                             float(std::cos(tau * hx)) * float(std::sqrt(hy)), std::sqrt(1 - hy), true);
      illumination[1] = illumination[1] + background(ambientRay, true, true) * lights.sDiff;
      radiance = radiance + throughput * (material.specular * illumination[0] + material.diffuse * illumination[1]);
      if (sr > 0) {
        Hit sky; sky.surface.Zpos = 32768;
        double bgDecrease, decay;
        V reflectedLight = visible(sky, hit.position, reflect, false, true, bgDecrease, decay);
        double factor = lights.bBackBMP && lights.bAddBGlight ? bgDecrease : bgDecrease * decay;
        radiance = radiance + throughput * material.specular *
          (reflectedLight + background(reflect, false, true) * factor) * sr;
      }
      return radiance;
    }
    double reflectAmount = 1, fresnel = 0, diffuseAmount = 1;
    if (transmission) {
      double ratio = (inside != initialInside) ? h.sTRIndex : 1.0 / h.sTRIndex,
             cosine = -dot(unit(direction), n),
             k = 1 - sq(ratio) * (1 - sq(cosine));
      if (k <= 0) {
        reflectAmount = 1 - material.transparency +
                        sq(material.transparency) /
                            (*std::max_element(material.specular.begin(),
                                               material.specular.end()) +
                             mc_reference::s001);
        transmission = false;
      } else {
        refract = unit(direction - n * (m.StepWidth / ratio *
                                        (std::sqrt(k) - ratio * cosine))) *
                  m.StepWidth;
        double ct = std::abs(dot(unit(refract), n)), ci = std::abs(cosine);
        if (ct > 1e-16)
          fresnel = .5 * (sq((ci - ratio * ct) / (ci + ratio * ct)) +
                          sq((ct - ratio * ci) / (ct + ratio * ci)));
        reflectAmount = fresnel + (1 - fresnel) * (1 - material.transparency);
        if (m.bInAndOutside)
          refract = direction;
      }
      diffuseAmount = 1 - material.transparency * sr;
    }
    auto lighting = direct(hit, reflect, inside);
    diagnosticEvent("direct", depth, {lighting[0][0],lighting[0][1],lighting[0][2],lighting[1][0],lighting[1][1],lighting[1][2],double(rng.state)});
    radiance = radiance +
               (material.specular * ((.25 + reflectAmount) * .8) * lighting[0] +
                material.diffuse * diffuseAmount * lighting[1]) *
                   throughput;
    if (depthDecay > 1) throughput = throughput * (1 / depthDecay);
    auto roughen = [&](V v) {
      if (!h.MCdiffReflects)
        return v;
      double angle = tau * rng.next(),
             r = rng.next() * h.MCdiffReflects * .00004;
      float radius = std::sqrt(r) * m.StepWidth;
      return unit(ortho(v, std::sin(angle) * radius,
                        std::cos(angle) * radius, std::sqrt(1 - r))) *
             m.StepWidth;
    };
    if (depth <= h.SRreflectioncount && sr > 0) {
      V r = roughen(reflect);
      if (dot(r, n) > 0)
        radiance = radiance +
                   trace(hit.position, r, hit.z, inside, ambient, depth + 1,
                         throughput * material.specular * (reflectAmount * sr),
                         material);
      if (transmission) {
        V t = roughen(refract);
        if (dot(t, n) < 0) {
          double amount =
              (1 - fresnel * *std::max_element(material.specular.begin(),
                                               material.specular.end())) *
              material.transparency * sr;
          radiance = radiance + trace(hit.position, t, hit.z, !inside, ambient,
                                      depth + 1, throughput * amount, material);
        }
      }
    }
    if (!options.no_ao && (depth < h.MCDepth || !ambient)) {
      double u = ambient ? rng.next() : hx;
      float v = ambient ? rng.next() : hy, radius = std::sqrt(v);
      V ray = unit(ortho(n, float(std::cos(tau * u)) * radius,
                         float(std::sin(tau * u)) * radius, std::sqrt(1 - v))) *
              m.StepWidth;
      radiance = radiance +
                 trace(hit.position, ray, hit.z, inside, true, depth + 1,
                       throughput * material.diffuse * diffuseAmount, material);
    }
    return radiance;
  }

public:
  PathTracer(const TMandHeader10 &header, const RenderOptions &opts,
             const TMCTparameter &params, const TLightVals &lv)
      : h(header), options(opts), m(params), initialParams(params), lights(lv),
        aligned(*lv.PLValigned) {
    m.pIt3Dext = &it;
    lights.PLValigned = &aligned;
    m.PLVals = &lights;
    stats.pLBcalcStop = &stop;
    m.PCalcThreadStats = &stats;
    m.iThreadId = 1;
    IniIt3D(&m, &it);
    initialInside = m.bInsideRendering != 0;
    sr =
        options.no_reflections ? 0 : std::clamp(double(h.SRamount), 0.0, 100.0);
    if (!(h.bCalcSRautomatic & 1))
      sr = 0;
    ambLength = std::min(
        double(m.DEAOmaxL) *
            std::hypot(double(m.iMandWidth), double(m.iMandHeight)) * 3,
        m.Zend * 255 / 1.75);
  }
  V sample(int x, int y, int index, Hit &hit) {
    if (index == 0) {
      if (diagnostic.is_open()) diagnostic.close();
      const char* file = std::getenv("MB3D_MC_TRACE");
      const char* px = std::getenv("MB3D_MC_TRACE_X");
      const char* py = std::getenv("MB3D_MC_TRACE_Y");
      if (file && px && py && x == std::atoi(px) && y == std::atoi(py)) {
        diagnostic.open(file, std::ios::trunc);
        if (!diagnostic) throw std::runtime_error("Cannot open MC diagnostic file");
      }
    }
    sampleIndex = index;
    m = initialParams;
    it = {};
    m.pIt3Dext = &it;
    m.PLVals = &lights;
    m.PCalcThreadStats = &stats;
    m.iThreadId = 1;
    IniIt3D(&m, &it);
    // Pixel-local streams make results independent of worker scheduling.
    rng.state = options.deterministic_seed ^ (uint32_t(x + 1) * 0x324594a1u) ^
                (uint32_t(y + 1) * 0x9e3779b9u) ^
                (uint32_t(index) * 0x85ebca6bu);
    rayBudget = 0;
    row = y;
    const auto &halton = GetHaltonSequence()[index];
    auto shift = GetMCHaltonShifts(x + 1, y);
    hx = ShiftMCHalton(halton.HRx, shift[0]);
    hy = ShiftMCHalton(halton.HRy, shift[1]);
    dx = ShiftMCHalton(halton.HDx, shift[0]);
    dy = ShiftMCHalton(halton.HDy, shift[1]);
    float xx, yy;
    if (index < 4) {
      xx = x + 1 - (index & 1) * .5f - .75f;
      yy = y + (index & 2) * .25f - .25f;
    } else {
      GetMCSubPixelShift((h.MCoptions & 8) != 0, [&]() { return rng.next(); }, xx, yy);
      xx += x;
      yy += y;
    }
    const auto cameraAngles = CalcMCCameraAngles(xx, yy, m.iMandWidth,
                                                m.iMandHeight, m.FOVXmul, m.FOVy);
    m.CAFX = cameraAngles[0];
    m.CAFY = cameraAngles[1];
    TVec3D v{};
    if (m.MCTCameraOptic == 1)
      put(v, unit({-m.CAFX, m.CAFY, m.mctPlOpticZ}));
    else if (m.MCTCameraOptic == 2)
      BuildViewVectorDSphereFOV(m.CAFY, m.CAFX, v);
    else
      BuildViewVectorDFOV(m.CAFY, m.CAFX, v);
    RotateVectorReverse(v, m.Vgrads);
    diagnosticEvent("camera", 0, {m.CAFX,m.CAFY,m.FOVXmul,m.FOVy,m.StepWidth,
                                  v[0],v[1],v[2]});
    V direction = read(v), origin = read(m.Ystart);
    if (m.MCTCameraOptic != 2)
      origin = origin + read(m.Vgrads[1]) * yy + read(m.Vgrads[0]) * xx;
    if (!options.no_dof && (h.bCalcDOFtype & 1)) {
      const auto lens = CalcMCLensSample(dx, dy, (h.MCoptions >> 4) & 7,
                                        h.sDOFaperture, h.sDOFZsharp,
                                        m.iMandWidth, m.StepWidth);
      V sharp = origin + direction * lens.focus;
      auto basis = orthoBasis(direction);
      diagnosticEvent("lens", 0, {lens.focus,lens.x,lens.y,sharp[0],sharp[1],sharp[2],
                                  basis[0][0],basis[0][1],basis[0][2],
                                  basis[1][0],basis[1][1],basis[1][2]});
      float sx = lens.x, sy = lens.y;
      for (int i = 0; i < 3; ++i) {
        origin[i] += float(basis[0][i] * sx);
        origin[i] += float(basis[1][i] * sy);
      }
      direction = unit(sharp - origin) * m.StepWidth;
    }
    primaryZ = 32768;
    primary = direction;
    put(m.mVgradsFOV, primary);
    V value = trace(origin, direction, 0, initialInside, false, 1, {1, 1, 1},
                    {}, &hit);
    diagnosticEvent("sample", 0, {value[0],value[1],value[2]});
    for (auto &c : value) {
      if (!std::isfinite(c))
        throw std::runtime_error("MC: nonfinite radiance");
      // Pascal MinMaxSVecMC bounds each ray before accumulation.
      c = std::clamp(c, 0.0, 400.0);
    }
    return value;
  }
};
} // namespace

RenderResult RenderMonteCarlo(TMandHeader10 &header, THeaderCustomAddon &addon,
                              const RenderOptions &options) {
  RenderResult result;
  try {
    int w = options.width_override ? options.width_override : header.Width,
        h = options.height_override ? options.height_override : header.Height;
    if (w < 1 || h < 1 || w > 16384 || h > 16384 || options.mc_samples < 1 ||
        options.mc_samples > 65535 || options.mc_max_steps < 1)
      throw std::runtime_error(
          "Invalid Monte Carlo dimensions, sample count or step budget");
    if (options.diag_only || !options.parity_trace_dump_path.empty() ||
        !options.ray_setup_dump_path.empty())
      throw std::runtime_error(
          "MC diagnostic traces require a separate MC schema; regular-render "
          "traces are not applicable");
    if ((header.bCalcSRautomatic & 2) &&
        (!(header.sTRIndex > 0) || !std::isfinite(header.sTRIndex)))
      throw std::runtime_error(
          "MC requires a finite positive refraction index");
    header.Width = w;
    header.Height = h;
    header.TilingOptions = 0;
    header.dZend =
        std::max(header.dZend - header.dZstart,
                 std::hypot(double(w), double(h)) * .1 * header.dStepWidth) +
        header.dZstart;
    result.width = w;
    result.height = h;
    ResetVolLightMap();
    TMCTparameter params = GetMCTparasFromHeader(header, &addon, 1);
    if (!params.bMCTisValid || !params.calc3D || !(params.StepWidth > 0) ||
        !(params.Zend > 0))
      throw std::runtime_error(
          "Invalid Monte Carlo scene/formula setup (3D required)");
    TLightVals lights{};
    TLValigned aligned{};
    lights.PLValigned = &aligned;
    MakeLightValsFromHeaderLight(&header, &lights, 1, 0);
    CompleteLightValsSetup(&header, &lights);
    lights.SRLightAmount = std::clamp(header.SRamount, 0.0f, 100.0f);
    CalcHSVecsFromLights(&lights, &params);
    params.iSmNormals = 0;
    params.sRoughness = 0;
    params.calcHardShadow = 0;
    for (int light = 0; light < 6; ++light)
      if ((header.Light.Lights[light].Loption & 3) == 0)
        params.calcHardShadow |= 4 << light;
    int workers = options.thread_count > 0
                      ? options.thread_count
                      : std::max(1u, std::thread::hardware_concurrency());
    workers = std::clamp(workers, 1, h);
    if (header.bVolLightNr & 7)
      CalcVolLightMap(&header, &lights, &params, workers);
    size_t pixels = size_t(w) * h;
    result.image.resize(pixels * 3);
    result.silight.resize(pixels);
    result.mc_linear.resize(pixels * 3);
    result.mc_variance.resize(pixels);
    result.mc_sample_counts.resize(pixels);
    std::atomic<int> nextRow{0};
    std::atomic<bool> failed{false};
    std::vector<std::future<void>> jobs;
    for (int worker = 0; worker < workers; ++worker)
      jobs.push_back(std::async(std::launch::async, [&] {
        try {
          PathTracer tracer(header, options, params, lights);
          for (int y; (y = nextRow.fetch_add(1)) < h && !failed.load();) {
            for (int x = 0; x < w; ++x) {
              V sum{};
              double mean = 0, m2 = 0;
              Hit nearest;
              nearest.surface.Zpos = 32768;
              double nearestDistance = 1e300;
              for (int s = 0; s < options.mc_samples; ++s) {
                Hit hit;
                V value = tracer.sample(x, y, s, hit);
                sum = sum + value;
                double l = luminance(value), delta = l - mean;
                mean += delta / (s + 1);
                m2 += delta * (l - mean);
                if (hit.hit && hit.distance < nearestDistance) {
                  nearest = hit;
                  nearestDistance = hit.distance;
                }
              }
              size_t pixel = size_t(y) * w + x;
              V average = sum * (1.0 / options.mc_samples);
              result.silight[pixel] = nearest.surface;
              result.mc_variance[pixel] = float(
                  options.mc_samples > 1 ? m2 / (options.mc_samples - 1) : 0);
              result.mc_sample_counts[pixel] = options.mc_samples;
              V display = displayColor(average, header, lights.bCalcPixColSqr);
              for (int c = 0; c < 3; ++c) {
                result.mc_linear[pixel * 3 + c] = float(average[c]);
                result.image[pixel * 3 + c] =
                    uint8_t(std::lrint(sat(display[c]) * 255));
              }
            }
          }
        } catch (...) {
          failed = true;
          throw;
        }
      }));
    std::exception_ptr error;
    for (auto &job : jobs) {
      try {
        job.get();
      } catch (...) {
        if (!error)
          error = std::current_exception();
      }
    }
    if (error)
      std::rethrow_exception(error);
    result.success = true;
  } catch (const std::exception &e) {
    result.success = false;
    result.error = e.what();
  }
  return result;
}
} // namespace mb3d
