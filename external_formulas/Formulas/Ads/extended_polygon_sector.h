#pragma once

#ifdef MB3D_SOFTWARE_EXT80
// Included inside the formula namespace. AND AH,41h / JG exits on <= or
// unordered. Repeated extended subtraction and both Double sector stores
// are observable; fmod is not an equivalent replacement.
inline mb3d::numeric::Extended80 extendedPolygonSector(
        mb3d::numeric::Extended80 angle,double half,double step,
        FormulaWorkBudget& budget) {
    while (angle > mb3d::numeric::Extended80(half)) {
        budget.beforeStep();
        angle-=step;
    }
    return angle;
}
#endif
