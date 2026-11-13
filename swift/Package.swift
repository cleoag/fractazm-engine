// swift-tools-version:5.9
// Swift wrapper over the mb3d C API. Build the binary first:
//   scripts/build_xcframework.sh
import PackageDescription

let package = Package(
    name: "MB3DKit",
    platforms: [.macOS(.v14), .iOS(.v17), .visionOS(.v1)],
    products: [.library(name: "MB3DKit", targets: ["MB3DKit"])],
    targets: [
        .binaryTarget(name: "CMB3D", path: "../xcframework/MB3D.xcframework"),
        .target(
            name: "MB3DKit",
            dependencies: ["CMB3D"],
            linkerSettings: [.linkedFramework("Metal"), .linkedFramework("Foundation")]
        ),
    ]
)
