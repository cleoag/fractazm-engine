#pragma once
#include "core/math3d.h"
#include <cstring>

namespace mb3d {
// Runtime records retain the original packed layout. Use byte copies at their
// boundaries, so vector arithmetic receives naturally aligned C++ objects.
inline void AddPackedVecWeight(void* destination, const void* add, double weight) {
    TVec3D a, b;
    std::memcpy(a, destination, sizeof(a));
    std::memcpy(b, add, sizeof(b));
    mAddVecWeight(a, b, weight);
    std::memcpy(destination, a, sizeof(a));
}
inline void CopyAddPackedVecWeight(void* destination, const void* source,
                                   const void* add, double weight) {
    TVec3D a, b, result;
    std::memcpy(a, source, sizeof(a));
    std::memcpy(b, add, sizeof(b));
    mCopyAddVecWeight(result, a, b, weight);
    std::memcpy(destination, result, sizeof(result));
}
inline void RotatePackedVectorReverse(void* vector, const void* matrix) {
    TVec3D v; TMatrix3 m;
    std::memcpy(v, vector, sizeof(v));
    std::memcpy(m, matrix, sizeof(m));
    RotateVectorReverse(v, m);
    std::memcpy(vector, v, sizeof(v));
}
inline double DotPackedVec3D(const void* first, const void* second) {
    TVec3D a, b;
    std::memcpy(a, first, sizeof(a));
    std::memcpy(b, second, sizeof(b));
    return DotVec3D(a, b);
}

inline double DotOfPackedVectorsNormalize(const void* first, const void* second) {
    TVec3D a, b;
    std::memcpy(a, first, sizeof(a));
    std::memcpy(b, second, sizeof(b));
    return DotOfVectorsNormalize(a, b);
}
inline void NormalizePackedVectorTo(double scale, void* vector) {
    TVec3D v;
    std::memcpy(v, vector, sizeof(v));
    NormaliseVectorToInPlace(scale, v);
    std::memcpy(vector, v, sizeof(v));
}
inline double SqrLengthOfPackedVec(const void* vector) {
    TVec3D v;
    std::memcpy(v, vector, sizeof(v));
    return SqrLengthOfVec(v);
}
inline void SubtractPackedVec3D(void* destination, const void* first, const void* second) {
    TVec3D a, b, result;
    std::memcpy(a, first, sizeof(a));
    std::memcpy(b, second, sizeof(b));
    SubtractVec3D(result, a, b);
    std::memcpy(destination, result, sizeof(result));
}
inline void AddSVecToPackedVec3D(const void* singles, const void* doubles, void* destination) {
    TSVec s; TVec3D d, result;
    std::memcpy(s, singles, sizeof(s));
    std::memcpy(d, doubles, sizeof(d));
    AddSVec2Vec3(s, d, result);
    std::memcpy(destination, result, sizeof(result));
}

}
