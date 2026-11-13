# Phase-one scene resources

These 16 image files and the original terms file are the exact approved inputs
of the pinned v4 resource profiles. They make the 77-scene resource set available
from a clean checkout without copying another developer's ignored run directory.
The immutable `mb3d/` reference is unchanged.

Six MI map numbers (18, 24, 31, 55, 71, 129) are user-selected available images,
not authenticated originals for the old scenes. Map 1633 is an explicit copy
of map 3038, not the missing MandelWerk photograph. All bytes, profile inventory
references, source ZIP/member identities and selection limits are recorded in
[selection.json](selection.json). The approved v4 contract remains unchanged.

The source distributions supply [M3Maps terms](<M3Maps/m3maps terms of use.txt>)
and [MI distribution terms](MI-m3maps-terms-of-use.txt), preserved verbatim.
These assets are not relicensed under the C++ source license. The supplied terms
allow their use in personal/commercial projects and prohibit selling unchanged
maps. Preserve those terms and the substitution provenance with redistributed
resource bundles.

Prepare a new runtime package from the repository root:

```sh
python3 -B comparison/tools/prepare_native_scene_resources.py --output comparison/runs/my-scene-package
```

This creates `runtime` (original), `runtime-selected` (available selections),
and `runtime-substitute1633` (the explicitly substituted profile), each with
`M3Maps/`, plus an inventory report. Every file is checked against the committed
contract inventories before copying. No scene, formula, rendering flag or old
runtime is changed. Runtime availability is not rendering qualification.
