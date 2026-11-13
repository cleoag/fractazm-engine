#!/bin/sh
# Build MB3D.xcframework: one merged static library per Apple platform, the
# public C headers and that platform's Metal shader library.
# Minimum OS: macOS 14.0, iOS 17.0, visionOS 1.0 (matches swift/Package.swift).
#   scripts/build_xcframework.sh [macos] [ios] [ios-simulator] [visionos] [visionos-simulator]
# Without arguments every platform is built. Output: xcframework/ (git-ignored).
set -eu
root=$(cd "$(dirname "$0")/.." && pwd)
out="$root/xcframework"
[ $# -gt 0 ] || set -- macos ios ios-simulator visionos visionos-simulator
rm -rf "$out/MB3D.xcframework"
mkdir -p "$out/headers/mb3d"
cp "$root/include/mb3d/mb3d.h" "$root/include/mb3d/mb3d_scene.h" "$out/headers/mb3d/"
cat > "$out/headers/module.modulemap" <<'MAP'
module CMB3D {
    header "mb3d/mb3d.h"
    header "mb3d/mb3d_scene.h"
    link "c++"
    link "z"
    export *
}
MAP
arguments=""
for platform in "$@"; do
    case "$platform" in
        macos) flags="-DCMAKE_OSX_DEPLOYMENT_TARGET=14.0 -DMB3D_METAL_MIN_OS_FLAG=-mmacosx-version-min=14.0" ;;
        ios) flags="-DCMAKE_SYSTEM_NAME=iOS -DCMAKE_OSX_SYSROOT=iphoneos -DCMAKE_OSX_DEPLOYMENT_TARGET=17.0 -DMB3D_METAL_MIN_OS_FLAG=-mios-version-min=17.0" ;;
        ios-simulator) flags="-DCMAKE_SYSTEM_NAME=iOS -DCMAKE_OSX_SYSROOT=iphonesimulator -DCMAKE_OSX_DEPLOYMENT_TARGET=17.0 -DMB3D_METAL_MIN_OS_FLAG=-mios-simulator-version-min=17.0" ;;
        visionos) flags="-DCMAKE_SYSTEM_NAME=visionOS -DCMAKE_OSX_SYSROOT=xros -DCMAKE_OSX_DEPLOYMENT_TARGET=1.0" ;;
        visionos-simulator) flags="-DCMAKE_SYSTEM_NAME=visionOS -DCMAKE_OSX_SYSROOT=xrsimulator -DCMAKE_OSX_DEPLOYMENT_TARGET=1.0" ;;
        *) echo "Unknown platform: $platform" >&2; exit 2 ;;
    esac
    build="$out/$platform"
    # shellcheck disable=SC2086
    cmake -S "$root" -B "$build" -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES=arm64 \
        -DMB3D_ENABLE_PARITY_TRACE=OFF $flags
    cmake --build "$build" --target mb3d_render_backend -j "$(sysctl -n hw.ncpu)"
    libtool -static -o "$build/libmb3d.a" $(find "$build" -maxdepth 1 -name 'libmb3d_*.a')
    arguments="$arguments -library $build/libmb3d.a -headers $out/headers"
    mkdir -p "$out/metallib/$platform"
    cp "$build/metal_native_scene.metallib" "$out/metallib/$platform/"
done
# shellcheck disable=SC2086
xcodebuild -create-xcframework $arguments -output "$out/MB3D.xcframework"
echo "Built $out/MB3D.xcframework; shader libraries are in $out/metallib/<platform>/"
