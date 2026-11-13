# Licence and notices

## This library

This repository is a C++ and Metal port of the rendering engine of
Mandelbulb 3D (MB3D). It is a work based on Mandelbulb 3D and is distributed
under the licence of Mandelbulb 3D, the GNU Lesser General Public License,
version 2.1. The full text is in [LICENSE](LICENSE).

Copyright (C) 2026 Feel Digital (the port).

This library is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License, version 2.1, as published
by the Free Software Foundation. It is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the licence for the
details.

## Mandelbulb 3D

Mandelbulb 3D was created by Jens "Jesse" Dierks and developed further by
Andreas Maschke (thargor6) and its community. It is distributed under the GNU
Lesser General Public License, version 2.1.

- Source of the original program: <https://github.com/thargor6/mb3d>
- `mb3d/` holds the Pascal source tree this port is checked against (the
  Free Pascal / Lazarus fork, version 1.9.9.1.38), with its own
  [licence file](mb3d/LICENSE) and its list of
  [acknowledgments](mb3d/README.md). Nothing in `mb3d/` is changed here.

The people named in `mb3d/README.md` are the authors of the methods this
renderer implements.

## Formulas

`mb3d/M3Formulas` is the formula collection distributed with Mandelbulb 3D,
written by many authors of its community; a formula file names its author
where the author left a name. `external_formulas/` and `src/core/generated`
hold the same formulas translated to C++ and to Metal shaders. The
translations are part of this library and are under its licence.

## Scenes and maps

- `mb3d/M3Parameter` holds the sample scenes distributed with Mandelbulb 3D;
  they belong to their authors, named in the file names.
- The scenes under `comparison/` and `tests/` are the test cases of this
  project.
- The maps under `comparison/resources` come from the M3Maps collection of
  Mandelbulb 3D and keep its terms of use
  ([`mb3d/M3Maps/m3maps terms of use.txt`](mb3d/M3Maps/m3maps%20terms%20of%20use.txt)):
  free to use, not to be sold unchanged.

## Third-party code

| Code | Where | Licence |
|---|---|---|
| Berkeley SoftFloat 3e, John R. Hauser; copyright The Regents of the University of California | `external_formulas/vendored/softfloat` | BSD 3-Clause, [COPYING.txt](external_formulas/vendored/softfloat/COPYING.txt) |
| stb_image and stb_image_write, Sean Barrett | `libs/stb` | MIT or public domain, at the end of each header |
| Cephes mathematical functions, Stephen L. Moshier, as adapted in the Free Pascal run-time library | `external_formulas/vendored/portable_math` | public domain (Cephes) |
| zlib 1.3.1, Jean-loup Gailly and Mark Adler | fetched by the build when the system has none | zlib licence |

A program that includes this library in binary form has to reproduce the
SoftFloat copyright notice and conditions in its documentation.

## The public source

The source is published at <https://github.com/cleoag/fractazm-engine>, one
commit for each released version of the Fractazm application, tagged with that
version. The published tree leaves out `comparison/results/` (dated records of
test runs) and the files that only concern the private working repository;
links into that folder from the documents lead nowhere there.
`scripts/publish_source.sh` makes the commit.
