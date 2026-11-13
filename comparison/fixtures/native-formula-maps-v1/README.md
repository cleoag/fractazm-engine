# Authored MapTranslate reference resources

These five small PNGs are synthetic test assets authored for this repository,
not original scene maps or user-selected substitutes. They are canonical RGB8,
64×32, filter-zero, no interlace, with only IHDR/IDAT/IEND chunks. Their exact
bytes are pinned in `comparison/tools/native_group03_map_reference.py`.

`runtime/M3Maps` contains maps 1, 2 and 3. Maps 1 and 2 contain four 16-column
RGB stripes; the second is the channel-wise inverse of the first. Map 3 is
white. Every row is identical, deliberately excluding any claim about VCL
bitmap vertical orientation. Different channels remain visible, so an RGB/BGR
swap does not accidentally pass. The original scalar conversion and the
ColToSVecSSE2 shuffle both produce RGB from BGRA; a symbolic lane test checks
that routing.

`large-id-runtime/M3Maps` contains maps 16777216 (black) and 16777217 (white).
Both IDs are valid original int32 values and have distinct exact binary64
metadata but narrow to the same binary32 value. Both actual files are loaded,
so this control exposes selector narrowing without relying on missing maps or
zero-map fallback. It is a predeclared diagnostic, not a presumption that the
current GPU implementation passes.

Every case declares every file and nonzero ID in its isolated runtime. The
expected upload is RGB values 0/255 widened to float, with the original first
column/row seam duplication. Descriptor words and the entire pixel-buffer hash
are derived from these authored PNGs before any device execution. Runtime
folders contain no README or unlisted search candidates.

The independent oracle uses original MapTranslate CODE and Pascal callback,
parameter layout and B-spline definitions. Wide constant-colour footprints
have exact mathematical partition of unity. A forward bound derived for every
rounding operation is under 0.00103 per sampled channel; final additions at
coordinates ±2^20 have a minimum half-cell width 0.03125. Only cases with one
unique final RN32 word throughout the interval are accepted. Twelve bounded
one-axis orbits also test caller state and strict escape comparison. This does
not establish arbitrary sampling precision, image parity with x87, RGB16
loading, or full-scene qualification.
