cmake_minimum_required(VERSION 3.28)
project(mb3d_render LANGUAGES C CXX)
option(MB3D_NATIVE_TESTS_ONLY "Configure the portable CPU runtime tests without the numerical renderer" OFF)
option(MB3D_BUILD_DIAGNOSTICS "Include diagnostic and test executables in the default build" OFF)
include(cmake/ReleaseChecks.cmake)
# Finalize after all targets, including the portable-runtime early return.
cmake_language(DEFER CALL mb3d_configure_release_checks)
option(MB3D_BUILD_METAL_PROBE "Build the isolated Apple Metal postprocess feasibility probe" OFF)
option(MB3D_BUILD_METAL_FORMULA_PROBE "Build the isolated experimental FP32 Metal formula renderer" OFF)
option(MB3D_BUILD_METAL_EXACT_SCENE "Build the native Metal exact arithmetic and restricted M3P primary stage" OFF)
if(MB3D_BUILD_METAL_EXACT_SCENE AND MB3D_NATIVE_TESTS_ONLY)
    message(FATAL_ERROR "The exact Metal scene stage needs the full native CPU reference renderer")
endif()

# Pointer size alone also matches ARM64, which cannot assemble x86 MASM.
set(MB3D_MSVC_X64 OFF)
if(MSVC AND CMAKE_CXX_COMPILER_ARCHITECTURE_ID STREQUAL "x64")
    set(MB3D_MSVC_X64 ON)
endif()
if(MB3D_MSVC_X64 AND NOT MB3D_NATIVE_TESTS_ONLY)
    enable_language(ASM_MASM)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
option(MB3D_ENABLE_PARITY_TRACE "Enable parity-trace instrumentation" ON)
set(MB3D_DEFAULT_PRECISION "compatible" CACHE STRING "Default render arithmetic: compatible or native")
set_property(CACHE MB3D_DEFAULT_PRECISION PROPERTY STRINGS compatible native)
if(NOT MB3D_DEFAULT_PRECISION MATCHES "^(compatible|native)$")
    message(FATAL_ERROR "MB3D_DEFAULT_PRECISION must be compatible or native")
endif()
if(CMAKE_INTERPROCEDURAL_OPTIMIZATION)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT MB3D_IPO_SUPPORTED OUTPUT MB3D_IPO_ERROR LANGUAGES C CXX)
    if(NOT MB3D_IPO_SUPPORTED)
        message(FATAL_ERROR "Requested IPO is unavailable: ${MB3D_IPO_ERROR}")
    endif()
endif()

# The Apple ARM64 compatibility profile keeps every multiply/add separate.
# Do not silently compile this profile for a translated or universal binary.
set(MB3D_APPLE_ARM64 OFF)
if(APPLE AND (CMAKE_OSX_ARCHITECTURES STREQUAL "arm64" OR
    (NOT CMAKE_OSX_ARCHITECTURES AND CMAKE_SYSTEM_PROCESSOR STREQUAL "arm64")))
    set(MB3D_APPLE_ARM64 ON)
    add_compile_options(-fno-fast-math -ffp-contract=off)
endif()
set(MB3D_METAL_RENDERER_DEFAULT OFF)
if(MB3D_APPLE_ARM64 AND NOT MB3D_NATIVE_TESTS_ONLY)
    set(MB3D_METAL_RENDERER_DEFAULT ON)
endif()
option(MB3D_ENABLE_METAL_RENDERER "Enable native Metal in the main renderer (CPU remains the default backend)" ${MB3D_METAL_RENDERER_DEFAULT})
# Shader SDK follows the target platform, so iOS/visionOS slices get their own metallib.
if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
    set(MB3D_METAL_SDK_DEFAULT iphoneos)
elseif(CMAKE_SYSTEM_NAME STREQUAL "visionOS")
    set(MB3D_METAL_SDK_DEFAULT xros)
else()
    set(MB3D_METAL_SDK_DEFAULT macosx)
endif()
if(CMAKE_OSX_SYSROOT MATCHES "[Ss]imulator")
    string(REPLACE "iphoneos" "iphonesimulator" MB3D_METAL_SDK_DEFAULT "${MB3D_METAL_SDK_DEFAULT}")
    string(REPLACE "xros" "xrsimulator" MB3D_METAL_SDK_DEFAULT "${MB3D_METAL_SDK_DEFAULT}")
endif()
set(MB3D_METAL_SDK "${MB3D_METAL_SDK_DEFAULT}" CACHE STRING "xcrun SDK used to compile Metal shaders")
# Minimum-OS flag for the application shader library, e.g. -mios-version-min=17.0.
# Empty keeps the SDK default, so ordinary builds are unchanged.
set(MB3D_METAL_MIN_OS_FLAG "" CACHE STRING "Minimum OS flag passed to the Metal compiler for metal_native_scene.metallib")
option(MB3D_BUILD_GENERAL_NAVIGATION_EXPERIMENT "Build the opt-in unqualified general sparse navigator" OFF)
if(MB3D_BUILD_GENERAL_NAVIGATION_EXPERIMENT AND NOT MB3D_ENABLE_METAL_RENDERER)
    message(FATAL_ERROR "The general navigation experiment requires Native Metal")
endif()
if(MB3D_ENABLE_METAL_RENDERER AND (NOT MB3D_APPLE_ARM64 OR MB3D_NATIVE_TESTS_ONLY))
    message(FATAL_ERROR "The native Metal renderer requires a full native Apple ARM64 build")
endif()
if(MB3D_APPLE_ARM64 OR (MB3D_MSVC_X64 AND NOT MB3D_NATIVE_TESTS_ONLY))
    # Numerical DE has the same extended numerator on both hosts, including
    # Windows ARM's x64 translator, which rounds x87 intermediates to binary64.
    set(MB3D_SOFTFLOAT_DIR ${CMAKE_SOURCE_DIR}/external_formulas/vendored/softfloat)
    file(GLOB MB3D_SOFTFLOAT_SOURCES ${MB3D_SOFTFLOAT_DIR}/source/*.c
        ${MB3D_SOFTFLOAT_DIR}/source/8086/*.c)
    add_library(mb3d_softfloat STATIC ${MB3D_SOFTFLOAT_SOURCES})
    set_target_properties(mb3d_softfloat PROPERTIES C_STANDARD 11 C_STANDARD_REQUIRED YES)
    if(MB3D_MSVC_X64)
        target_include_directories(mb3d_softfloat BEFORE PRIVATE
            ${CMAKE_SOURCE_DIR}/src/core/softfloat_msvc)
    endif()
    target_include_directories(mb3d_softfloat PUBLIC ${MB3D_SOFTFLOAT_DIR}/source/include
        PRIVATE ${MB3D_SOFTFLOAT_DIR} ${MB3D_SOFTFLOAT_DIR}/source/8086)
    target_compile_definitions(mb3d_softfloat PUBLIC SOFTFLOAT_FAST_INT64 LITTLEENDIAN=1 MB3D_HAVE_SOFT_EXT80
        $<$<BOOL:${MB3D_APPLE_ARM64}>:MB3D_SOFTWARE_EXT80>
        PRIVATE INLINE_LEVEL=5)
endif()

if(MSVC)
    add_compile_options($<$<COMPILE_LANGUAGE:CXX>:/std:c++latest>)
    # Integer-generated SSE seed tables exceed MSVC's small default constexpr
    # evaluation budget. This changes compile-time work, not floating arithmetic.
    add_compile_options($<$<COMPILE_LANGUAGE:CXX>:/constexpr:steps2000000>)
endif()

if(NOT MB3D_NATIVE_TESTS_ONLY)
    # Find zlib
    find_package(ZLIB QUIET)
    if(NOT ZLIB_FOUND)
        include(FetchContent)
        set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
        FetchContent_Declare(
            zlib
            URL https://github.com/madler/zlib/archive/refs/tags/v1.3.1.zip
            URL_HASH SHA256=50b24b47bf19e1f35d2a21ff36d2a366638cdf958219a66f30ce0861201760e6
            DOWNLOAD_EXTRACT_TIMESTAMP TRUE
        )
        FetchContent_MakeAvailable(zlib)
        if(TARGET zlibstatic)
            add_library(ZLIB::ZLIB ALIAS zlibstatic)
        elseif(TARGET zlib)
            add_library(ZLIB::ZLIB ALIAS zlib)
        else()
            message(FATAL_ERROR "Fetched zlib but no usable target was created")
        endif()
    endif()

endif()

# Find threads
find_package(Threads REQUIRED)

# Include directories
include_directories(src/ include/ libs/)

# Portable production runtime, shared by the renderer and native CPU tests.
# Function-pointer names remain in parity_trace_names.cpp with the numeric core;
# resource ownership, worker/context binding and trace sessions need no kernels.
set(MB3D_RUNTIME_SOURCES
    src/core/numeric_precision.cpp
    src/core/render_context.cpp
    src/core/render_control.cpp
    src/core/mb3d_render_control_api.cpp
    src/core/thread_pool.cpp
    src/core/formula_trace.cpp
    src/core/parity_trace.cpp
    src/core/lightmap_resources.cpp)
add_library(mb3d_runtime STATIC ${MB3D_RUNTIME_SOURCES}
    ${CMAKE_SOURCE_DIR}/external_formulas/vendored/portable_math/portable_math.c)
target_include_directories(mb3d_runtime PUBLIC src/ include/ libs/
    ${CMAKE_SOURCE_DIR}/external_formulas/vendored/portable_math)
target_link_libraries(mb3d_runtime PUBLIC Threads::Threads)
if(MB3D_APPLE_ARM64)
    target_link_libraries(mb3d_runtime PUBLIC mb3d_softfloat)
endif()
target_compile_definitions(mb3d_runtime PUBLIC
    $<$<STREQUAL:${MB3D_DEFAULT_PRECISION},native>:MB3D_DEFAULT_PRECISION_NATIVE>
    _USE_MATH_DEFINES
    M_PI=3.14159265358979323846
    M_LOG2E=1.44269504088896340736
    $<$<BOOL:${MB3D_ENABLE_PARITY_TRACE}>:MB3D_ENABLE_PARITY_TRACE>)
if(MSVC)
    # /fp:strict disables FP reordering/contraction and costs ~30x on
    # numeric-heavy code (measured 2026-04-19 on built-in scene renders).
    # Restrict it to the portable scalar math and mixed-precision DE boundary,
    # which require strict operation order for bit-exact output. Other code uses
    # /fp:precise (MSVC default) — the built-in formulas already match
    # Pascal bit-exactly or within 1 ULP under precise mode.
    set_source_files_properties(
        ${CMAKE_SOURCE_DIR}/external_formulas/vendored/portable_math/portable_math.c
        ${CMAKE_SOURCE_DIR}/src/core/numerical_de.cpp
        PROPERTIES COMPILE_OPTIONS "/fp:strict")
endif()

enable_testing()
# Diagnostic instrumentation can cost orders of magnitude in software ext80
# kernels. Production/default deadlines remain unchanged.
set(MB3D_TEST_TIMEOUT_SCALE 1 CACHE STRING "CTest deadline multiplier for diagnostic builds")
if(NOT MB3D_TEST_TIMEOUT_SCALE MATCHES "^[1-9][0-9]*$")
    message(FATAL_ERROR "MB3D_TEST_TIMEOUT_SCALE must be a positive integer")
endif()
function(mb3d_scale_test_timeouts)
    get_property(tests DIRECTORY PROPERTY TESTS)
    foreach(test IN LISTS tests)
        get_property(timeout TEST ${test} PROPERTY TIMEOUT)
        if(timeout)
            math(EXPR scaled "${timeout} * ${MB3D_TEST_TIMEOUT_SCALE}")
            set_property(TEST ${test} PROPERTY TIMEOUT ${scaled})
        endif()
    endforeach()
endfunction()
cmake_language(DEFER CALL mb3d_scale_test_timeouts)


if(MB3D_BUILD_METAL_PROBE)
    if(NOT MB3D_APPLE_ARM64)
        message(FATAL_ERROR "The Metal probe requires a native Apple ARM64 configuration")
    endif()
    enable_language(OBJCXX)
    find_package(Python3 REQUIRED COMPONENTS Interpreter)
    add_executable(mb3d_metal_downscale_probe comparison/tools/metal_downscale_probe.mm src/core/image_process.cpp)
    target_compile_options(mb3d_metal_downscale_probe PRIVATE $<$<COMPILE_LANGUAGE:OBJCXX>:-fobjc-arc>)
    target_link_libraries(mb3d_metal_downscale_probe PRIVATE mb3d_runtime "-framework Foundation" "-framework Metal")
endif()

if(MB3D_BUILD_METAL_FORMULA_PROBE)
    if(NOT MB3D_APPLE_ARM64)
        message(FATAL_ERROR "The Metal formula probe requires a native Apple ARM64 configuration")
    endif()
    enable_language(OBJCXX)
    find_program(MB3D_XCRUN xcrun REQUIRED)
    set(MB3D_METAL_PROBE_DIR "${CMAKE_SOURCE_DIR}/comparison/metal_probe")
    set(MB3D_METAL_PROBE_AIR "${CMAKE_CURRENT_BINARY_DIR}/metal_formula_probe.air")
    set(MB3D_METAL_PROBE_LIBRARY "${CMAKE_CURRENT_BINARY_DIR}/metal_formula_probe.metallib")
    # Static source compiled ahead of execution; this does not enable formula JIT.
    add_custom_command(OUTPUT "${MB3D_METAL_PROBE_LIBRARY}"
        COMMAND "${MB3D_XCRUN}" -sdk ${MB3D_METAL_SDK} metal -std=metal3.1
            -fno-fast-math -ffp-contract=off -I "${MB3D_METAL_PROBE_DIR}"
            -c "${MB3D_METAL_PROBE_DIR}/kernels.metal" -o "${MB3D_METAL_PROBE_AIR}"
        COMMAND "${MB3D_XCRUN}" -sdk ${MB3D_METAL_SDK} metallib "${MB3D_METAL_PROBE_AIR}"
            -o "${MB3D_METAL_PROBE_LIBRARY}"
        DEPENDS comparison/metal_probe/kernels.metal
            comparison/metal_probe/probe_abi.h comparison/metal_probe/probe_math.h
        BYPRODUCTS "${MB3D_METAL_PROBE_AIR}"
        VERBATIM)
    add_custom_target(mb3d_metal_formula_shaders DEPENDS "${MB3D_METAL_PROBE_LIBRARY}")
    add_executable(mb3d_metal_formula_probe comparison/tools/metal_formula_probe.mm)
    set_target_properties(mb3d_metal_formula_probe PROPERTIES OBJCXX_STANDARD 20 OBJCXX_STANDARD_REQUIRED YES)
    target_include_directories(mb3d_metal_formula_probe PRIVATE "${CMAKE_SOURCE_DIR}")
    target_compile_options(mb3d_metal_formula_probe PRIVATE -fobjc-arc -fno-fast-math -ffp-contract=off)
    target_link_libraries(mb3d_metal_formula_probe PRIVATE Threads::Threads "-framework Foundation" "-framework Metal")
    add_dependencies(mb3d_metal_formula_probe mb3d_metal_formula_shaders)
endif()

add_executable(mb3d_tests_thread_pool tests/test_thread_pool.cpp)
target_link_libraries(mb3d_tests_thread_pool PRIVATE mb3d_runtime)
add_test(NAME thread_pool_tests COMMAND mb3d_tests_thread_pool)
set_tests_properties(thread_pool_tests PROPERTIES TIMEOUT 30)

add_executable(mb3d_tests_render_context tests/test_render_context.cpp)
target_link_libraries(mb3d_tests_render_context PRIVATE mb3d_runtime)
add_test(NAME render_context_tests COMMAND mb3d_tests_render_context)
set_tests_properties(render_context_tests PROPERTIES TIMEOUT 30)

add_executable(mb3d_tests_map_resources tests/test_map_resources.cpp)
target_link_libraries(mb3d_tests_map_resources PRIVATE mb3d_runtime)
add_test(NAME map_resources_tests COMMAND mb3d_tests_map_resources)
set_tests_properties(map_resources_tests PROPERTIES TIMEOUT 30)

add_executable(mb3d_tests_render_control tests/test_render_control.cpp)
target_link_libraries(mb3d_tests_render_control PRIVATE mb3d_runtime)
add_test(NAME render_control_tests COMMAND mb3d_tests_render_control)
set_tests_properties(render_control_tests PROPERTIES TIMEOUT 30)

add_executable(mb3d_tests_render_control_api_lifecycle tests/test_render_control_api_lifecycle.cpp)
target_link_libraries(mb3d_tests_render_control_api_lifecycle PRIVATE mb3d_runtime)
add_test(NAME render_control_api_lifecycle_tests COMMAND mb3d_tests_render_control_api_lifecycle)
set_tests_properties(render_control_api_lifecycle_tests PROPERTIES TIMEOUT 30)

add_executable(mb3d_tests_portable_formula_control tests/test_portable_formula_control.cpp)
target_link_libraries(mb3d_tests_portable_formula_control PRIVATE mb3d_runtime)
add_test(NAME portable_formula_control_tests COMMAND mb3d_tests_portable_formula_control)
set_tests_properties(portable_formula_control_tests PROPERTIES TIMEOUT 30)

add_executable(mb3d_tests_render_control_image_process
    tests/test_render_control_image_process.cpp src/core/image_process.cpp)
target_link_libraries(mb3d_tests_render_control_image_process PRIVATE mb3d_runtime)
add_test(NAME render_control_image_process_tests COMMAND mb3d_tests_render_control_image_process)
set_tests_properties(render_control_image_process_tests PROPERTIES TIMEOUT 30)

# PasLCG is a header-only ordinary deterministic RNG; it has no core dependency.
add_executable(mb3d_tests_pas_lcg tests/test_pas_lcg.cpp)
add_test(NAME pas_lcg_tests COMMAND mb3d_tests_pas_lcg)

# Exercise the actual non-SSE implementation, including on x64 reference hosts.
add_executable(mb3d_tests_portable_sqrt tests/test_portable_sqrt.cpp
    external_formulas/vendored/portable_math/portable_math.c)
target_include_directories(mb3d_tests_portable_sqrt PRIVATE
    external_formulas/vendored/portable_math)
target_compile_definitions(mb3d_tests_portable_sqrt PRIVATE PM_FORCE_PORTABLE_SQRT)
if(UNIX)
    target_link_libraries(mb3d_tests_portable_sqrt PRIVATE m)
endif()
add_test(NAME portable_sqrt_tests COMMAND mb3d_tests_portable_sqrt)

add_executable(mb3d_tests_sse_estimates tests/test_sse_estimates.cpp)
add_test(NAME sse_estimates_tests COMMAND mb3d_tests_sse_estimates)

set(MB3D_NATIVE_TARGETS
    mb3d_tests_thread_pool mb3d_tests_render_context mb3d_tests_map_resources
    mb3d_tests_render_control mb3d_tests_render_control_api_lifecycle
    mb3d_tests_portable_formula_control mb3d_tests_render_control_image_process
    mb3d_tests_pas_lcg mb3d_tests_portable_sqrt mb3d_tests_sse_estimates)
set(MB3D_NATIVE_TESTS
    thread_pool_tests render_context_tests map_resources_tests
    render_control_tests render_control_api_lifecycle_tests
    portable_formula_control_tests render_control_image_process_tests
    pas_lcg_tests portable_sqrt_tests sse_estimates_tests)
if(MB3D_APPLE_ARM64)
    add_executable(mb3d_tests_arm64_numerics tests/test_arm64_numerics.cpp src/core/math3d.cpp)
    target_link_libraries(mb3d_tests_arm64_numerics PRIVATE mb3d_runtime)
    add_test(NAME arm64_numerics_tests COMMAND mb3d_tests_arm64_numerics)
    list(APPEND MB3D_NATIVE_TARGETS mb3d_tests_arm64_numerics)
    list(APPEND MB3D_NATIVE_TESTS arm64_numerics_tests)
    foreach(kernel IN ITEMS extended_sine extended_small_division extended_product extended_series)
        add_executable(mb3d_tests_${kernel} tests/test_${kernel}.cpp)
        target_link_libraries(mb3d_tests_${kernel} PRIVATE mb3d_runtime)
        add_test(NAME ${kernel}_tests COMMAND mb3d_tests_${kernel})
        list(APPEND MB3D_NATIVE_TARGETS mb3d_tests_${kernel})
        list(APPEND MB3D_NATIVE_TESTS ${kernel}_tests)
    endforeach()
endif()
set_tests_properties(${MB3D_NATIVE_TESTS} PROPERTIES LABELS native_cpu TIMEOUT 30)
add_custom_target(mb3d_native_tests DEPENDS ${MB3D_NATIVE_TARGETS})

# A native-runtime configuration neither discovers zlib nor configures the
# Intel-dependent numerical renderer or any deferred targets.
if(MB3D_NATIVE_TESTS_ONLY)
    return()
endif()

# Static library: mb3d_core
file(GLOB MB3D_CORE_SOURCES src/core/*.cpp)
foreach(runtime_source IN LISTS MB3D_RUNTIME_SOURCES)
    list(REMOVE_ITEM MB3D_CORE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/${runtime_source}")
endforeach()
file(GLOB MB3D_CORE_GENERATED_SOURCES src/core/generated/*.cpp)
set(MB3D_CORE_ASM_SOURCES)
if(MB3D_MSVC_X64)
    list(APPEND MB3D_CORE_ASM_SOURCES src/core/win64_de_x87_helpers.asm)
endif()
add_library(mb3d_core STATIC ${MB3D_CORE_SOURCES} ${MB3D_CORE_GENERATED_SOURCES} ${MB3D_CORE_ASM_SOURCES})
target_include_directories(mb3d_core PUBLIC src/ include/ libs/
    ${CMAKE_SOURCE_DIR}/external_formulas/vendored/portable_math)
target_include_directories(mb3d_core PRIVATE ${CMAKE_SOURCE_DIR}/external_formulas/Formulas)
target_link_libraries(mb3d_core PUBLIC mb3d_runtime)
if(TARGET mb3d_softfloat)
    target_link_libraries(mb3d_core PUBLIC mb3d_softfloat)
    target_compile_definitions(mb3d_core PRIVATE MB3D_SOFTWARE_NUMERICAL_DE)
endif()
target_compile_definitions(mb3d_core PUBLIC
    _USE_MATH_DEFINES
    M_PI=3.14159265358979323846
    M_LOG2E=1.44269504088896340736
    $<$<BOOL:${MB3D_ENABLE_PARITY_TRACE}>:MB3D_ENABLE_PARITY_TRACE>)

# Static library: mb3d_io
add_library(mb3d_io STATIC
    src/io/m3p_parser.cpp
    src/io/image_writer.cpp
    src/io/buffer_export.cpp
)
target_include_directories(mb3d_io PUBLIC src/ include/ libs/)
target_link_libraries(mb3d_io PUBLIC mb3d_core ZLIB::ZLIB)
target_compile_definitions(mb3d_io PUBLIC
    $<$<BOOL:${MB3D_ENABLE_PARITY_TRACE}>:MB3D_ENABLE_PARITY_TRACE>)

# Main executable
if(MB3D_BUILD_METAL_EXACT_SCENE OR MB3D_ENABLE_METAL_RENDERER)
    if(NOT MB3D_APPLE_ARM64)
        message(FATAL_ERROR "The exact Metal scene stage requires native Apple ARM64")
    endif()
    enable_language(OBJCXX)
    find_program(MB3D_XCRUN xcrun REQUIRED)
    set(MB3D_EXACT_DIR "${CMAKE_SOURCE_DIR}/comparison/metal_exact")
    set(MB3D_NATIVE_FORMULA_DIR "${CMAKE_SOURCE_DIR}/src/render/native_metal")
    file(GLOB_RECURSE MB3D_NATIVE_FORMULA_HEADERS CONFIGURE_DEPENDS "${MB3D_NATIVE_FORMULA_DIR}/*.h")
    file(GLOB_RECURSE MB3D_NATIVE_FORMULA_INPUTS CONFIGURE_DEPENDS
        "${CMAKE_SOURCE_DIR}/external_formulas/Formulas/*.inc"
        "${CMAKE_SOURCE_DIR}/src/core/external_formulas*.cpp"
        "${CMAKE_SOURCE_DIR}/src/core/external_formula_table*.cpp"
        "${CMAKE_SOURCE_DIR}/src/core/generated/external_formula_table_*.cpp"
        "${CMAKE_SOURCE_DIR}/src/core/formula*.cpp")
    file(GLOB MB3D_EXACT_HEADERS CONFIGURE_DEPENDS "${MB3D_EXACT_DIR}/*.h")
    # Compiler-derived primary state masks admit pixel ownership. They bind the
    # formula bodies and wrapper they analyzed; after a source change, rerun
    # native_formula_effects.py and native_formula_effects_header.py.
    set(MB3D_FORMULA_EFFECTS_HEADER "${MB3D_NATIVE_FORMULA_DIR}/generated/formula_state_effects.h")
    set(MB3D_FORMULA_EFFECTS_MANIFEST "${MB3D_NATIVE_FORMULA_DIR}/generated/formula_state_effects.sources.tsv")
    file(SHA256 "${MB3D_FORMULA_EFFECTS_MANIFEST}" MB3D_FORMULA_EFFECTS_DIGEST)
    file(STRINGS "${MB3D_FORMULA_EFFECTS_HEADER}" MB3D_FORMULA_EFFECTS_RECORDED REGEX "kSourcesSha256")
    if(NOT MB3D_FORMULA_EFFECTS_RECORDED MATCHES "\"${MB3D_FORMULA_EFFECTS_DIGEST}\"")
        message(FATAL_ERROR "formula_state_effects.h does not record its source manifest")
    endif()
    file(STRINGS "${MB3D_FORMULA_EFFECTS_MANIFEST}" MB3D_FORMULA_EFFECTS_SOURCES)
    set(MB3D_FORMULA_EFFECTS_INPUTS "${MB3D_FORMULA_EFFECTS_HEADER}" "${MB3D_FORMULA_EFFECTS_MANIFEST}")
    foreach(effects_source IN LISTS MB3D_FORMULA_EFFECTS_SOURCES)
        if(NOT effects_source MATCHES "^([^\t]+)\t([0-9a-f]+)$")
            message(FATAL_ERROR "Malformed formula effects manifest line: ${effects_source}")
        endif()
        set(effects_path "${CMAKE_MATCH_1}")
        set(effects_digest "${CMAKE_MATCH_2}")
        if(NOT EXISTS "${CMAKE_SOURCE_DIR}/${effects_path}")
            message(FATAL_ERROR "Stale compiler-derived formula masks: ${effects_path} is missing")
        endif()
        file(SHA256 "${CMAKE_SOURCE_DIR}/${effects_path}" effects_actual)
        if(NOT effects_actual STREQUAL effects_digest)
            message(FATAL_ERROR "Stale compiler-derived formula masks: ${effects_path} changed")
        endif()
        list(APPEND MB3D_FORMULA_EFFECTS_INPUTS "${CMAKE_SOURCE_DIR}/${effects_path}")
    endforeach()
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${MB3D_FORMULA_EFFECTS_INPUTS})
    if(MB3D_BUILD_METAL_EXACT_SCENE)
    add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/metal_exact_scene.metallib"
        COMMAND "${MB3D_XCRUN}" -sdk ${MB3D_METAL_SDK} metal -std=metal3.1 -fno-fast-math -ffp-contract=off
            -I "${MB3D_EXACT_DIR}" -c "${MB3D_EXACT_DIR}/kernels.metal"
            -MD -MF "${CMAKE_CURRENT_BINARY_DIR}/metal_exact_scene.d"
            -o "${CMAKE_CURRENT_BINARY_DIR}/metal_exact_scene.air"
        COMMAND "${MB3D_XCRUN}" -sdk ${MB3D_METAL_SDK} metallib "${CMAKE_CURRENT_BINARY_DIR}/metal_exact_scene.air"
            -o "${CMAKE_CURRENT_BINARY_DIR}/metal_exact_scene.metallib"
        DEPENDS comparison/metal_exact/kernels.metal comparison/metal_exact/scene_manifest.inc ${MB3D_EXACT_HEADERS}
        BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/metal_exact_scene.air"
            "${CMAKE_CURRENT_BINARY_DIR}/metal_exact_scene.d" VERBATIM)
    add_custom_target(mb3d_metal_exact_shaders DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/metal_exact_scene.metallib")
    endif()
    # Native arithmetic is a separate offline library: hardware FP32 with FMA
    # contraction, while retaining nonfinite checks (no finite-only fast-math).
    add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/metal_native_scene.metallib"
        COMMAND "${Python3_EXECUTABLE}" -B "${CMAKE_SOURCE_DIR}/scripts/gen_native_metal_formulas.py" --check
        COMMAND "${MB3D_XCRUN}" -sdk ${MB3D_METAL_SDK} metal -std=metal3.1 ${MB3D_METAL_MIN_OS_FLAG} -fno-fast-math -ffp-contract=fast
            -I "${MB3D_EXACT_DIR}" -c "${MB3D_EXACT_DIR}/native_kernels.metal"
            -MD -MF "${CMAKE_CURRENT_BINARY_DIR}/metal_native_scene.d"
            -o "${CMAKE_CURRENT_BINARY_DIR}/metal_native_scene.air"
        COMMAND "${MB3D_XCRUN}" -sdk ${MB3D_METAL_SDK} metal -std=metal3.1 ${MB3D_METAL_MIN_OS_FLAG} -fno-fast-math -ffp-contract=fast
            -I "${CMAKE_SOURCE_DIR}" -I "${MB3D_EXACT_DIR}" -c "${MB3D_NATIVE_FORMULA_DIR}/general_kernels.metal"
            -MD -MF "${CMAKE_CURRENT_BINARY_DIR}/metal_native_general.d"
            -o "${CMAKE_CURRENT_BINARY_DIR}/metal_native_general.air"
        # Ordered DOF accumulation retains explicit FP32 store/grouping order.
        # Keep its AIR separate so it cannot alter existing formula compilation.
        COMMAND "${MB3D_XCRUN}" -sdk ${MB3D_METAL_SDK} metal -std=metal3.1 ${MB3D_METAL_MIN_OS_FLAG} -fno-fast-math -ffp-contract=off
            -I "${CMAKE_SOURCE_DIR}" -c "${MB3D_NATIVE_FORMULA_DIR}/dof_kernels.metal"
            -MD -MF "${CMAKE_CURRENT_BINARY_DIR}/metal_native_dof.d"
            -o "${CMAKE_CURRENT_BINARY_DIR}/metal_native_dof.air"
        COMMAND "${MB3D_XCRUN}" -sdk ${MB3D_METAL_SDK} metal -std=metal3.1 ${MB3D_METAL_MIN_OS_FLAG} -fno-fast-math -ffp-contract=off
            -I "${CMAKE_SOURCE_DIR}" -c "${MB3D_NATIVE_FORMULA_DIR}/zbuffer_kernels.metal"
            -MD -MF "${CMAKE_CURRENT_BINARY_DIR}/metal_native_zbuffer.d"
            -o "${CMAKE_CURRENT_BINARY_DIR}/metal_native_zbuffer.air"
        # Ordered AO uses its own offline FP32 AIR and dependency closure.
        COMMAND "${MB3D_XCRUN}" -sdk ${MB3D_METAL_SDK} metal -std=metal3.1 ${MB3D_METAL_MIN_OS_FLAG} -fno-fast-math -ffp-contract=off
            -I "${CMAKE_SOURCE_DIR}" -I "${MB3D_EXACT_DIR}" -c "${MB3D_NATIVE_FORMULA_DIR}/native_ao_screen_kernels.metal"
            -MD -MF "${CMAKE_CURRENT_BINARY_DIR}/metal_native_ao_screen.d"
            -o "${CMAKE_CURRENT_BINARY_DIR}/metal_native_ao_screen.air"
        # Ordered AO uses its own offline FP32 AIR and dependency closure.
        COMMAND "${MB3D_XCRUN}" -sdk ${MB3D_METAL_SDK} metal -std=metal3.1 ${MB3D_METAL_MIN_OS_FLAG} -fno-fast-math -ffp-contract=off
            -I "${CMAKE_SOURCE_DIR}" -I "${MB3D_EXACT_DIR}" -c "${MB3D_NATIVE_FORMULA_DIR}/native_ao_de_kernels.metal"
            -MD -MF "${CMAKE_CURRENT_BINARY_DIR}/metal_native_ao_de.d"
            -o "${CMAKE_CURRENT_BINARY_DIR}/metal_native_ao_de.air"
        COMMAND "${MB3D_XCRUN}" -sdk ${MB3D_METAL_SDK} metallib "${CMAKE_CURRENT_BINARY_DIR}/metal_native_scene.air"
            "${CMAKE_CURRENT_BINARY_DIR}/metal_native_general.air"
            "${CMAKE_CURRENT_BINARY_DIR}/metal_native_dof.air"
            "${CMAKE_CURRENT_BINARY_DIR}/metal_native_zbuffer.air"
            "${CMAKE_CURRENT_BINARY_DIR}/metal_native_ao_screen.air"
            "${CMAKE_CURRENT_BINARY_DIR}/metal_native_ao_de.air"
            -o "${CMAKE_CURRENT_BINARY_DIR}/metal_native_scene.metallib"
        DEPENDS comparison/metal_exact/native_kernels.metal comparison/metal_exact/kernels.metal
            comparison/metal_exact/scene_manifest.inc ${MB3D_EXACT_HEADERS}
            scripts/gen_native_metal_formulas.py src/render/native_metal/general_kernels.metal
            src/render/native_metal/dof_kernels.metal src/render/native_metal/zbuffer_kernels.metal
            src/render/native_metal/native_ao_screen_kernels.metal src/render/native_metal/native_ao_de_kernels.metal
            ${MB3D_NATIVE_FORMULA_HEADERS}
            ${MB3D_NATIVE_FORMULA_INPUTS}
        BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/metal_native_scene.air" "${CMAKE_CURRENT_BINARY_DIR}/metal_native_general.air"
            "${CMAKE_CURRENT_BINARY_DIR}/metal_native_dof.air" "${CMAKE_CURRENT_BINARY_DIR}/metal_native_zbuffer.air"
            "${CMAKE_CURRENT_BINARY_DIR}/metal_native_scene.d" "${CMAKE_CURRENT_BINARY_DIR}/metal_native_general.d"
            "${CMAKE_CURRENT_BINARY_DIR}/metal_native_dof.d" "${CMAKE_CURRENT_BINARY_DIR}/metal_native_zbuffer.d"
            "${CMAKE_CURRENT_BINARY_DIR}/metal_native_ao_screen.air" "${CMAKE_CURRENT_BINARY_DIR}/metal_native_ao_de.air"
            "${CMAKE_CURRENT_BINARY_DIR}/metal_native_ao_screen.d" "${CMAKE_CURRENT_BINARY_DIR}/metal_native_ao_de.d" VERBATIM)
    add_custom_target(mb3d_metal_native_shaders DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/metal_native_scene.metallib")
endif()
if(MB3D_BUILD_METAL_EXACT_SCENE)
    add_executable(mb3d_tests_metal_native_arithmetic comparison/metal_exact/test_native_arithmetic.cpp)
    target_compile_options(mb3d_tests_metal_native_arithmetic PRIVATE -fno-fast-math -ffp-contract=off)
    add_test(NAME metal_native_arithmetic_tests COMMAND mb3d_tests_metal_native_arithmetic)
    set_tests_properties(metal_native_arithmetic_tests PROPERTIES LABELS "metal_native" TIMEOUT 30)
    add_executable(mb3d_metal_exact_scene comparison/tools/metal_exact_scene.mm comparison/metal_exact/scene_host.cpp
        comparison/metal_exact/shadow_host.cpp comparison/metal_exact/ao_host.cpp comparison/metal_exact/color_host.cpp)
    set_target_properties(mb3d_metal_exact_scene PROPERTIES OBJCXX_STANDARD 20 OBJCXX_STANDARD_REQUIRED YES)
    target_include_directories(mb3d_metal_exact_scene PRIVATE "${CMAKE_SOURCE_DIR}")
    target_compile_options(mb3d_metal_exact_scene PRIVATE $<$<COMPILE_LANGUAGE:OBJCXX>:-fobjc-arc> -fno-fast-math -ffp-contract=off)
    target_link_libraries(mb3d_metal_exact_scene PRIVATE mb3d_io "-framework Foundation" "-framework Metal")
    add_dependencies(mb3d_metal_exact_scene mb3d_metal_exact_shaders mb3d_metal_native_shaders)
    find_package(Python3 REQUIRED COMPONENTS Interpreter)
    add_test(NAME metal_native_cli_tests COMMAND "${Python3_EXECUTABLE}"
        "${CMAKE_SOURCE_DIR}/comparison/metal_exact/test_native_cli.py"
        "$<TARGET_FILE:mb3d_metal_exact_scene>"
        "${CMAKE_CURRENT_BINARY_DIR}/metal_exact_scene.metallib"
        "${CMAKE_CURRENT_BINARY_DIR}/metal_native_scene.metallib"
        "${CMAKE_SOURCE_DIR}/comparison/benchmark_scenes/effects-v1/sphereifs-ao-shadows.m3p")
    set_tests_properties(metal_native_cli_tests PROPERTIES LABELS "metal_native" TIMEOUT 180)
    add_executable(mb3d_tests_metal_exact_arithmetic comparison/metal_exact/test_soft_double.cpp)
    target_compile_options(mb3d_tests_metal_exact_arithmetic PRIVATE -fno-fast-math -ffp-contract=off)
    add_test(NAME metal_exact_arithmetic_tests COMMAND mb3d_tests_metal_exact_arithmetic)
    set_tests_properties(metal_exact_arithmetic_tests PROPERTIES LABELS "metal_exact" TIMEOUT 30)
    add_executable(mb3d_tests_metal_exact_log comparison/metal_exact/test_soft_log.cpp)
    target_compile_options(mb3d_tests_metal_exact_log PRIVATE -fno-fast-math -ffp-contract=off)
    add_test(NAME metal_exact_log_tests COMMAND mb3d_tests_metal_exact_log)
    set_tests_properties(metal_exact_log_tests PROPERTIES LABELS "metal_exact" TIMEOUT 60)
    add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/metal_numerical_de.metallib"
        COMMAND "${MB3D_XCRUN}" -sdk ${MB3D_METAL_SDK} metal -std=metal3.1 -fno-fast-math -ffp-contract=off
            -I "${MB3D_EXACT_DIR}" -c "${MB3D_EXACT_DIR}/numerical_de_kernel.metal"
            -MD -MF "${CMAKE_CURRENT_BINARY_DIR}/metal_numerical_de.d"
            -o "${CMAKE_CURRENT_BINARY_DIR}/metal_numerical_de.air"
        COMMAND "${MB3D_XCRUN}" -sdk ${MB3D_METAL_SDK} metallib "${CMAKE_CURRENT_BINARY_DIR}/metal_numerical_de.air"
            -o "${CMAKE_CURRENT_BINARY_DIR}/metal_numerical_de.metallib"
        DEPENDS comparison/metal_exact/numerical_de_kernel.metal ${MB3D_EXACT_HEADERS}
        BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/metal_numerical_de.air"
            "${CMAKE_CURRENT_BINARY_DIR}/metal_numerical_de.d" VERBATIM)
    add_custom_target(mb3d_metal_numerical_de_shader DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/metal_numerical_de.metallib")
    add_executable(mb3d_tests_metal_numerical_de comparison/metal_exact/test_numerical_de_gpu.mm)
    set_target_properties(mb3d_tests_metal_numerical_de PROPERTIES OBJCXX_STANDARD 20 OBJCXX_STANDARD_REQUIRED YES)
    target_include_directories(mb3d_tests_metal_numerical_de PRIVATE "${CMAKE_SOURCE_DIR}")
    target_compile_options(mb3d_tests_metal_numerical_de PRIVATE -fobjc-arc -fno-fast-math -ffp-contract=off)
    target_link_libraries(mb3d_tests_metal_numerical_de PRIVATE "-framework Foundation" "-framework Metal")
    add_dependencies(mb3d_tests_metal_numerical_de mb3d_metal_numerical_de_shader)
    add_test(NAME metal_numerical_de_tests COMMAND mb3d_tests_metal_numerical_de
        "${CMAKE_CURRENT_BINARY_DIR}/metal_numerical_de.metallib")
    set_tests_properties(metal_numerical_de_tests PROPERTIES LABELS "metal_exact" TIMEOUT 60)
    add_executable(mb3d_tests_metal_exact_bulb comparison/metal_exact/test_bulb_math.cpp)
    target_include_directories(mb3d_tests_metal_exact_bulb PRIVATE "${CMAKE_SOURCE_DIR}")
    target_link_libraries(mb3d_tests_metal_exact_bulb PRIVATE mb3d_io)
    target_compile_options(mb3d_tests_metal_exact_bulb PRIVATE -fno-fast-math -ffp-contract=off)
    add_test(NAME metal_exact_bulb_tests COMMAND mb3d_tests_metal_exact_bulb "${CMAKE_SOURCE_DIR}")
    set_tests_properties(metal_exact_bulb_tests PROPERTIES LABELS "metal_exact" TIMEOUT 120)
    add_executable(mb3d_tests_metal_exact_extended comparison/metal_exact/test_soft_extended.cpp)
    target_include_directories(mb3d_tests_metal_exact_extended PRIVATE "${CMAKE_SOURCE_DIR}")
    target_link_libraries(mb3d_tests_metal_exact_extended PRIVATE mb3d_runtime mb3d_softfloat)
    target_compile_options(mb3d_tests_metal_exact_extended PRIVATE -fno-fast-math -ffp-contract=off)
    add_test(NAME metal_exact_extended_tests COMMAND mb3d_tests_metal_exact_extended)
    set_tests_properties(metal_exact_extended_tests PROPERTIES LABELS "metal_exact" TIMEOUT 30)
    add_executable(mb3d_tests_metal_exact_formula tests/test_metal_exact_formula.cpp)
    target_include_directories(mb3d_tests_metal_exact_formula PRIVATE "${CMAKE_SOURCE_DIR}")
    add_test(NAME metal_exact_formula_tests COMMAND mb3d_tests_metal_exact_formula)
    set_tests_properties(metal_exact_formula_tests PROPERTIES LABELS "metal_exact" TIMEOUT 30)
    add_executable(mb3d_tests_metal_exact_scene_host comparison/metal_exact/test_scene_host.cpp comparison/metal_exact/scene_host.cpp)
    target_include_directories(mb3d_tests_metal_exact_scene_host PRIVATE "${CMAKE_SOURCE_DIR}")
    target_link_libraries(mb3d_tests_metal_exact_scene_host PRIVATE mb3d_io)
    add_test(NAME metal_exact_scene_host_tests COMMAND mb3d_tests_metal_exact_scene_host
        "${CMAKE_SOURCE_DIR}/comparison/scenes/extall_dIFS_SphereIFS.m3p"
        "${CMAKE_SOURCE_DIR}/comparison/scenes/test_04_realpower.m3p")
    set_tests_properties(metal_exact_scene_host_tests PROPERTIES LABELS "metal_exact" TIMEOUT 60)
    add_executable(mb3d_tests_metal_exact_normals comparison/metal_exact/test_normals.cpp
        comparison/metal_exact/scene_host.cpp)
    target_include_directories(mb3d_tests_metal_exact_normals PRIVATE "${CMAKE_SOURCE_DIR}")
    target_link_libraries(mb3d_tests_metal_exact_normals PRIVATE mb3d_io)
    target_compile_options(mb3d_tests_metal_exact_normals PRIVATE -fno-fast-math -ffp-contract=off)
    add_test(NAME metal_exact_normals_tests COMMAND mb3d_tests_metal_exact_normals "${CMAKE_SOURCE_DIR}")
    set_tests_properties(metal_exact_normals_tests PROPERTIES LABELS "metal_exact" TIMEOUT 120)
    add_executable(mb3d_tests_metal_exact_contract tests/test_metal_exact_contract.cpp comparison/metal_exact/scene_host.cpp)
    target_include_directories(mb3d_tests_metal_exact_contract PRIVATE "${CMAKE_SOURCE_DIR}")
    target_link_libraries(mb3d_tests_metal_exact_contract PRIVATE mb3d_io)
    add_test(NAME metal_exact_contract_tests COMMAND mb3d_tests_metal_exact_contract
        "${CMAKE_SOURCE_DIR}/comparison/scenes/extall_dIFS_SphereIFS.m3p")
    set_tests_properties(metal_exact_contract_tests PROPERTIES LABELS "metal_exact" TIMEOUT 60)
    add_executable(mb3d_tests_metal_exact_single comparison/metal_exact/test_soft_single.cpp)
    target_compile_options(mb3d_tests_metal_exact_single PRIVATE -fno-fast-math -ffp-contract=off)
    add_test(NAME metal_exact_single_tests COMMAND mb3d_tests_metal_exact_single)
    set_tests_properties(metal_exact_single_tests PROPERTIES LABELS "metal_exact" TIMEOUT 60)
    foreach(stage IN ITEMS shadow ao color)
        add_executable(mb3d_tests_metal_exact_${stage} comparison/metal_exact/test_${stage}.cpp
            comparison/metal_exact/${stage}_host.cpp comparison/metal_exact/scene_host.cpp)
        if(stage STREQUAL "ao")
            target_sources(mb3d_tests_metal_exact_${stage} PRIVATE comparison/metal_exact/shadow_host.cpp)
        endif()
        target_include_directories(mb3d_tests_metal_exact_${stage} PRIVATE "${CMAKE_SOURCE_DIR}")
        target_link_libraries(mb3d_tests_metal_exact_${stage} PRIVATE mb3d_io)
        target_compile_options(mb3d_tests_metal_exact_${stage} PRIVATE -fno-fast-math -ffp-contract=off)
        add_test(NAME metal_exact_${stage}_tests COMMAND mb3d_tests_metal_exact_${stage}
            "${CMAKE_SOURCE_DIR}/comparison/scenes/extall_dIFS_SphereIFS.m3p")
        set_tests_properties(metal_exact_${stage}_tests PROPERTIES LABELS "metal_exact" TIMEOUT 90)
    endforeach()
    add_executable(mb3d_tests_metal_exact_shadow_trace comparison/metal_exact/test_shadow_trace.cpp
        comparison/metal_exact/shadow_host.cpp comparison/metal_exact/scene_host.cpp)
    target_include_directories(mb3d_tests_metal_exact_shadow_trace PRIVATE "${CMAKE_SOURCE_DIR}")
    target_link_libraries(mb3d_tests_metal_exact_shadow_trace PRIVATE mb3d_io)
    target_compile_options(mb3d_tests_metal_exact_shadow_trace PRIVATE -fno-fast-math -ffp-contract=off)
    add_test(NAME metal_exact_shadow_trace_tests COMMAND mb3d_tests_metal_exact_shadow_trace
        "${CMAKE_SOURCE_DIR}/comparison/scenes/extall_dIFS_SphereIFS.m3p")
    set_tests_properties(metal_exact_shadow_trace_tests PROPERTIES LABELS "metal_exact" TIMEOUT 120)
    foreach(scene_id IN ITEMS box_default box_50 box_asymmetric sphere_varied box_camera sphere_apply0_orbit sphere_existing intpow2 intpow5 intpow8 integer_power3 integer_power4 integer_power6 integer_power7)
        if(scene_id STREQUAL "box_default")
            set(MB3D_EXACT_TEST_SCENE "comparison/scenes/extall_dIFS_boxIFS.m3p")
        elseif(scene_id STREQUAL "box_50")
            set(MB3D_EXACT_TEST_SCENE "comparison/scenes/ext_16_boxifs.m3p")
        elseif(scene_id STREQUAL "sphere_existing")
            set(MB3D_EXACT_TEST_SCENE "comparison/scenes/ext_18_sphereifs.m3p")
        elseif(scene_id STREQUAL "intpow2")
            set(MB3D_EXACT_TEST_SCENE "comparison/scenes/test_01_intpow2.m3p")
        elseif(scene_id STREQUAL "intpow5")
            set(MB3D_EXACT_TEST_SCENE "comparison/scenes/test_02_intpow5.m3p")
        elseif(scene_id STREQUAL "intpow8")
            set(MB3D_EXACT_TEST_SCENE "comparison/scenes/test_03_intpow8.m3p")
        else()
            set(MB3D_EXACT_TEST_SCENE "comparison/metal_exact/fixtures/${scene_id}.m3p")
        endif()
        add_test(NAME metal_exact_${scene_id}_host_tests COMMAND mb3d_tests_metal_exact_scene_host
            "${CMAKE_SOURCE_DIR}/${MB3D_EXACT_TEST_SCENE}"
            "${CMAKE_SOURCE_DIR}/comparison/scenes/test_04_realpower.m3p")
        set_tests_properties(metal_exact_${scene_id}_host_tests PROPERTIES LABELS "metal_exact" TIMEOUT 60)
        # This original zero-axis scene is valid after loader normalization but
        # has no foreground/background and shadow-branch coverage. Exercise
        # admission/CPU export only; it must not be counted as full qualification.
        if(scene_id STREQUAL "sphere_existing")
            continue()
        endif()
        foreach(stage IN ITEMS shadow ao color shadow_trace)
            add_test(NAME metal_exact_${scene_id}_${stage}_tests COMMAND mb3d_tests_metal_exact_${stage}
                "${CMAKE_SOURCE_DIR}/${MB3D_EXACT_TEST_SCENE}")
            set_tests_properties(metal_exact_${scene_id}_${stage}_tests PROPERTIES LABELS "metal_exact" TIMEOUT 300)
        endforeach()
    endforeach()
    add_custom_target(mb3d_metal_exact_checks DEPENDS mb3d_tests_metal_exact_arithmetic
        mb3d_tests_metal_native_arithmetic
        mb3d_tests_metal_numerical_de
        mb3d_tests_metal_exact_log mb3d_tests_metal_exact_bulb mb3d_tests_metal_exact_normals
        mb3d_tests_metal_exact_extended mb3d_tests_metal_exact_formula
        mb3d_tests_metal_exact_scene_host mb3d_tests_metal_exact_contract mb3d_metal_exact_scene
        mb3d_tests_metal_exact_shadow mb3d_tests_metal_exact_ao mb3d_tests_metal_exact_color mb3d_tests_metal_exact_single mb3d_tests_metal_exact_shadow_trace)
endif()

# backend.cpp selects CPU/Metal; mb3d_scene_api.cpp is the application C API.
add_library(mb3d_render_backend STATIC src/render/backend.cpp src/render/mb3d_scene_api.cpp)
target_link_libraries(mb3d_render_backend PUBLIC mb3d_io)
add_executable(mb3d_tests_scene_api tests/test_scene_api.cpp)
target_link_libraries(mb3d_tests_scene_api PRIVATE mb3d_render_backend ZLIB::ZLIB)
if(MB3D_ENABLE_METAL_RENDERER)
    # With Metal the persistent renderer is also compared on the GPU.
    add_test(NAME scene_api_tests COMMAND mb3d_tests_scene_api "${CMAKE_SOURCE_DIR}"
        "${CMAKE_CURRENT_BINARY_DIR}/metal_native_scene.metallib")
else()
    add_test(NAME scene_api_tests COMMAND mb3d_tests_scene_api "${CMAKE_SOURCE_DIR}")
endif()
set_tests_properties(scene_api_tests PROPERTIES TIMEOUT 300)
if(MB3D_ENABLE_METAL_RENDERER)
    add_library(mb3d_native_formula_host STATIC
        src/render/native_metal/formula_host.cpp
        src/render/native_metal/general_scene_host.cpp
        src/render/native_metal/render_plan.cpp
        src/render/native_metal/general_normals_host.cpp
        src/render/native_metal/native_camera_host.cpp
        src/render/native_metal/native_inside_host.cpp
        src/render/native_metal/generic_color_host.cpp
        src/render/native_metal/generic_color_base.cpp
        src/render/native_metal/native_ao_host.cpp
        src/render/native_metal/dof_host.cpp
        src/render/native_metal/zbuffer_host.cpp
        comparison/metal_exact/scene_host.cpp comparison/metal_exact/shadow_host.cpp
        comparison/metal_exact/ao_host.cpp comparison/metal_exact/color_host.cpp
        comparison/metal_exact/native_reflection_host.cpp)
    target_include_directories(mb3d_native_formula_host PRIVATE "${CMAKE_SOURCE_DIR}")
    target_link_libraries(mb3d_native_formula_host PUBLIC mb3d_io)
    # Preparation inventory only: no Metal framework/device or shader dependency.
    add_executable(mb3d_native_scene_context_dump src/cli/native_scene_context_dump.cpp)
    target_include_directories(mb3d_native_scene_context_dump PRIVATE "${CMAKE_SOURCE_DIR}")
    target_link_libraries(mb3d_native_scene_context_dump PRIVATE mb3d_native_formula_host)
    add_executable(mb3d_native_formula_probe src/render/native_metal/formula_probe.mm)
    target_include_directories(mb3d_native_formula_probe PRIVATE "${CMAKE_SOURCE_DIR}")
    set_target_properties(mb3d_native_formula_probe PROPERTIES OBJCXX_STANDARD 20 OBJCXX_STANDARD_REQUIRED YES)
    target_compile_options(mb3d_native_formula_probe PRIVATE -fobjc-arc)
    target_link_libraries(mb3d_native_formula_probe PRIVATE mb3d_native_formula_host "-framework Foundation" "-framework Metal")
    add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/navigation_preview.metallib"
        COMMAND "${MB3D_XCRUN}" -sdk ${MB3D_METAL_SDK} metal -std=metal3.0 -fno-fast-math
            -I "${CMAKE_SOURCE_DIR}" -c "${CMAKE_SOURCE_DIR}/src/render/native_metal/navigation_preview.metal"
            -MD -MF "${CMAKE_BINARY_DIR}/navigation_preview.d"
            -o "${CMAKE_BINARY_DIR}/navigation_preview.air"
        COMMAND "${MB3D_XCRUN}" -sdk ${MB3D_METAL_SDK} metallib "${CMAKE_BINARY_DIR}/navigation_preview.air"
            -o "${CMAKE_BINARY_DIR}/navigation_preview.metallib"
        DEPENDS src/render/native_metal/navigation_preview.metal comparison/metal_exact/scene_manifest.inc ${MB3D_EXACT_HEADERS}
        DEPFILE "${CMAKE_BINARY_DIR}/navigation_preview.d"
        BYPRODUCTS "${CMAKE_BINARY_DIR}/navigation_preview.air"
        VERBATIM)
    add_custom_target(mb3d_navigation_preview DEPENDS "${CMAKE_BINARY_DIR}/navigation_preview.metallib")
    add_dependencies(mb3d_render_backend mb3d_navigation_preview)
    if(MB3D_BUILD_GENERAL_NAVIGATION_EXPERIMENT)
        add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/general_navigation.metallib"
            COMMAND "${MB3D_XCRUN}" -sdk ${MB3D_METAL_SDK} metal -std=metal3.1 -fno-fast-math -ffp-contract=fast
                -I "${CMAKE_SOURCE_DIR}" -c "${CMAKE_SOURCE_DIR}/src/render/native_metal/general_navigation.metal"
                -MD -MF "${CMAKE_BINARY_DIR}/general_navigation.d" -o "${CMAKE_BINARY_DIR}/general_navigation.air"
            COMMAND "${MB3D_XCRUN}" -sdk ${MB3D_METAL_SDK} metallib "${CMAKE_BINARY_DIR}/general_navigation.air"
                -o "${CMAKE_BINARY_DIR}/general_navigation.metallib"
            DEPENDS src/render/native_metal/general_navigation.metal ${MB3D_EXACT_HEADERS}
            DEPFILE "${CMAKE_BINARY_DIR}/general_navigation.d"
            VERBATIM)
        add_custom_target(mb3d_general_navigation_experiment DEPENDS "${CMAKE_BINARY_DIR}/general_navigation.metallib")
        add_dependencies(mb3d_render_backend mb3d_general_navigation_experiment)
        target_sources(mb3d_render_backend PRIVATE src/render/native_metal/general_navigation.mm)
    endif()

    add_executable(mb3d_tests_viewer_admission apps/macos_viewer/test_admission.cpp)
    target_include_directories(mb3d_tests_viewer_admission PRIVATE "${CMAKE_SOURCE_DIR}")
    target_compile_options(mb3d_tests_viewer_admission PRIVATE -UNDEBUG)
    add_test(NAME viewer_admission_tests COMMAND mb3d_tests_viewer_admission)
    set_tests_properties(viewer_admission_tests PROPERTIES LABELS "ordinary;metal_native_host" TIMEOUT 30)
    add_executable(mb3d_tests_viewer_presentation apps/macos_viewer/test_presentation.cpp)
    target_compile_options(mb3d_tests_viewer_presentation PRIVATE -UNDEBUG)
    add_test(NAME viewer_presentation_tests COMMAND mb3d_tests_viewer_presentation)
    set_tests_properties(viewer_presentation_tests PROPERTIES LABELS "ordinary;metal_native_host" TIMEOUT 30)
    target_sources(mb3d_render_backend PRIVATE src/render/metal_renderer.mm
        src/render/native_metal/navigation_preview.mm
        src/render/native_metal/general_renderer.mm
        src/render/native_metal/native_ao_dispatch.mm
        src/render/native_metal/dof_dispatch.mm
        src/render/native_metal/zbuffer_dispatch.mm)
    set_target_properties(mb3d_render_backend PROPERTIES OBJCXX_STANDARD 20 OBJCXX_STANDARD_REQUIRED YES)
    target_include_directories(mb3d_render_backend PRIVATE "${CMAKE_SOURCE_DIR}")
    target_compile_definitions(mb3d_render_backend PRIVATE MB3D_ENABLE_METAL_RENDERER)
    target_compile_options(mb3d_render_backend PRIVATE $<$<COMPILE_LANGUAGE:OBJCXX>:-fobjc-arc>)
    target_link_libraries(mb3d_render_backend PRIVATE "-framework Foundation" "-framework Metal")
    target_link_libraries(mb3d_render_backend PRIVATE mb3d_native_formula_host)
    add_executable(mb3d_tests_native_render_plan tests/test_native_render_plan.cpp tests/test_native_pixel_state_contract.cpp)
    target_compile_definitions(mb3d_tests_native_render_plan PRIVATE MB3D_PIXEL_STATE_CONTRACT_EMBEDDED)
    target_include_directories(mb3d_tests_native_render_plan PRIVATE "${CMAKE_SOURCE_DIR}" "${CMAKE_SOURCE_DIR}/src")
    target_link_libraries(mb3d_tests_native_render_plan PRIVATE mb3d_native_formula_host)
    add_test(NAME native_render_plan_tests COMMAND mb3d_tests_native_render_plan
        "${CMAKE_SOURCE_DIR}" "${CMAKE_SOURCE_DIR}/comparison/scenes/test_01_intpow2.m3p")
    set_tests_properties(native_render_plan_tests PROPERTIES LABELS "ordinary;metal_native_host" TIMEOUT 30)
    add_executable(mb3d_tests_native_resource_entry tests/test_native_resource_entry.cpp)
    target_link_libraries(mb3d_tests_native_resource_entry PRIVATE mb3d_render_backend)
    add_test(NAME native_resource_entry_tests COMMAND mb3d_tests_native_resource_entry "${CMAKE_SOURCE_DIR}")
    set_tests_properties(native_resource_entry_tests PROPERTIES LABELS "ordinary;metal_native_host" TIMEOUT 30)
    add_executable(mb3d_tests_native_formula_resources tests/test_native_formula_resources.cpp)
    target_link_libraries(mb3d_tests_native_formula_resources PRIVATE mb3d_native_formula_host)
    add_test(NAME native_formula_resources_tests COMMAND mb3d_tests_native_formula_resources)
    set_tests_properties(native_formula_resources_tests PROPERTIES LABELS "ordinary;metal_native_host" TIMEOUT 30)
    add_executable(mb3d_tests_native_material comparison/tools/test_native_reflection_material.mm)
    set_target_properties(mb3d_tests_native_material PROPERTIES OBJCXX_STANDARD 20 OBJCXX_STANDARD_REQUIRED YES)
    target_include_directories(mb3d_tests_native_material PRIVATE "${CMAKE_SOURCE_DIR}" "${CMAKE_SOURCE_DIR}/src")
    target_compile_options(mb3d_tests_native_material PRIVATE -fobjc-arc -fno-fast-math -ffp-contract=off)
    target_link_libraries(mb3d_tests_native_material PRIVATE mb3d_native_formula_host "-framework Foundation" "-framework Metal")
    add_executable(mb3d_tests_native_general_normals_gpu tests/test_native_general_normals_gpu.cpp)
    target_include_directories(mb3d_tests_native_general_normals_gpu PRIVATE "${CMAKE_SOURCE_DIR}")
    target_link_libraries(mb3d_tests_native_general_normals_gpu PRIVATE mb3d_render_backend)
    add_executable(mb3d_tests_native_smoothing_readback tests/test_native_smoothing_readback.cpp)
    target_include_directories(mb3d_tests_native_smoothing_readback PRIVATE "${CMAKE_SOURCE_DIR}" "${CMAKE_SOURCE_DIR}/src")
    add_test(NAME metal_native_smoothing_readback_tests COMMAND mb3d_tests_native_smoothing_readback)
    set_tests_properties(metal_native_smoothing_readback_tests PROPERTIES LABELS "metal_native" TIMEOUT 30)
    add_executable(mb3d_tests_native_smoothing_render tests/test_native_smoothing_render.cpp)
    target_include_directories(mb3d_tests_native_smoothing_render PRIVATE "${CMAKE_SOURCE_DIR}")
    target_link_libraries(mb3d_tests_native_smoothing_render PRIVATE mb3d_render_backend)
    add_executable(mb3d_tests_native_general_gpu tests/test_native_general_gpu.cpp)
    target_include_directories(mb3d_tests_native_general_gpu PRIVATE "${CMAKE_SOURCE_DIR}")
    target_link_libraries(mb3d_tests_native_general_gpu PRIVATE mb3d_render_backend)
    add_executable(mb3d_tests_native_logical_lanes_gpu tests/test_native_logical_lanes_gpu.mm)
    add_executable(mb3d_tests_native_pipeline_cache tests/test_native_pipeline_cache.mm)
    set_target_properties(mb3d_tests_native_pipeline_cache PROPERTIES OBJCXX_STANDARD 20 OBJCXX_STANDARD_REQUIRED YES)
    target_include_directories(mb3d_tests_native_pipeline_cache PRIVATE "${CMAKE_SOURCE_DIR}" "${CMAKE_SOURCE_DIR}/src")
    target_compile_options(mb3d_tests_native_pipeline_cache PRIVATE -fobjc-arc)
    target_link_libraries(mb3d_tests_native_pipeline_cache PRIVATE "-framework Foundation" "-framework Metal")
    add_dependencies(mb3d_tests_native_pipeline_cache mb3d_metal_native_shaders)
    add_test(NAME metal_native_pipeline_cache_tests COMMAND mb3d_tests_native_pipeline_cache "${CMAKE_CURRENT_BINARY_DIR}/metal_native_scene.metallib")
    set_tests_properties(metal_native_pipeline_cache_tests PROPERTIES LABELS "metal_native" TIMEOUT 30)
    set_target_properties(mb3d_tests_native_logical_lanes_gpu PROPERTIES OBJCXX_STANDARD 20 OBJCXX_STANDARD_REQUIRED YES)
    target_include_directories(mb3d_tests_native_logical_lanes_gpu PRIVATE "${CMAKE_SOURCE_DIR}" "${CMAKE_SOURCE_DIR}/src")
    target_compile_options(mb3d_tests_native_logical_lanes_gpu PRIVATE -fobjc-arc)
    target_link_libraries(mb3d_tests_native_logical_lanes_gpu PRIVATE "-framework Foundation" "-framework Metal")
    add_dependencies(mb3d_tests_native_logical_lanes_gpu mb3d_metal_native_shaders)
    add_test(NAME metal_native_logical_lanes_tests COMMAND mb3d_tests_native_logical_lanes_gpu "${CMAKE_CURRENT_BINARY_DIR}/metal_native_scene.metallib" "${CMAKE_SOURCE_DIR}/comparison/fixtures/native_gradient_normal_case.json")
    set_tests_properties(metal_native_logical_lanes_tests PROPERTIES LABELS "metal_native" TIMEOUT 300)
    add_executable(mb3d_tests_native_ao_seed_gpu tests/test_native_ao_seed_gpu.mm)
    set_target_properties(mb3d_tests_native_ao_seed_gpu PROPERTIES OBJCXX_STANDARD 20 OBJCXX_STANDARD_REQUIRED YES)
    target_include_directories(mb3d_tests_native_ao_seed_gpu PRIVATE "${CMAKE_SOURCE_DIR}" "${CMAKE_SOURCE_DIR}/src")
    target_compile_options(mb3d_tests_native_ao_seed_gpu PRIVATE -fobjc-arc -fno-fast-math -ffp-contract=off)
    target_link_libraries(mb3d_tests_native_ao_seed_gpu PRIVATE mb3d_render_backend "-framework Foundation" "-framework Metal")
    add_dependencies(mb3d_tests_native_ao_seed_gpu mb3d_metal_native_shaders)
    add_executable(mb3d_tests_native_ao_chunk_gpu tests/test_native_ao_chunk_gpu.mm)
    set_target_properties(mb3d_tests_native_ao_chunk_gpu PROPERTIES OBJCXX_STANDARD 20 OBJCXX_STANDARD_REQUIRED YES)
    target_include_directories(mb3d_tests_native_ao_chunk_gpu PRIVATE "${CMAKE_SOURCE_DIR}" "${CMAKE_SOURCE_DIR}/src")
    target_compile_options(mb3d_tests_native_ao_chunk_gpu PRIVATE -fobjc-arc -fno-fast-math -ffp-contract=off)
    target_link_libraries(mb3d_tests_native_ao_chunk_gpu PRIVATE mb3d_render_backend "-framework Foundation" "-framework Metal")
    add_dependencies(mb3d_tests_native_ao_chunk_gpu mb3d_metal_native_shaders)
    add_executable(mb3d_tests_native_ao_certificate_gpu tests/test_native_ao_certificate_gpu.mm)
    set_target_properties(mb3d_tests_native_ao_certificate_gpu PROPERTIES OBJCXX_STANDARD 20 OBJCXX_STANDARD_REQUIRED YES)
    target_include_directories(mb3d_tests_native_ao_certificate_gpu PRIVATE "${CMAKE_SOURCE_DIR}" "${CMAKE_SOURCE_DIR}/src")
    target_compile_options(mb3d_tests_native_ao_certificate_gpu PRIVATE -fobjc-arc -fno-fast-math -ffp-contract=off)
    target_link_libraries(mb3d_tests_native_ao_certificate_gpu PRIVATE mb3d_render_backend "-framework Foundation" "-framework Metal")
    add_dependencies(mb3d_tests_native_ao_certificate_gpu mb3d_metal_native_shaders)
    add_executable(mb3d_tests_native_seed_scan_gpu tests/test_native_seed_scan_gpu.mm)
    set_target_properties(mb3d_tests_native_seed_scan_gpu PROPERTIES OBJCXX_STANDARD 20 OBJCXX_STANDARD_REQUIRED YES)
    target_include_directories(mb3d_tests_native_seed_scan_gpu PRIVATE "${CMAKE_SOURCE_DIR}" "${CMAKE_SOURCE_DIR}/src")
    target_compile_options(mb3d_tests_native_seed_scan_gpu PRIVATE -fobjc-arc -fno-fast-math -ffp-contract=off)
    target_link_libraries(mb3d_tests_native_seed_scan_gpu PRIVATE mb3d_render_backend "-framework Foundation" "-framework Metal")
    add_dependencies(mb3d_tests_native_seed_scan_gpu mb3d_metal_native_shaders)
    add_test(NAME metal_native_seed_scan_tests COMMAND mb3d_tests_native_seed_scan_gpu "${CMAKE_CURRENT_BINARY_DIR}/metal_native_scene.metallib")
    set_tests_properties(metal_native_seed_scan_tests PROPERTIES LABELS "metal_native" TIMEOUT 180 RUN_SERIAL TRUE)

    add_executable(mb3d_tests_native_command_gpu tests/test_native_command_gpu.mm)
    set_target_properties(mb3d_tests_native_command_gpu PROPERTIES OBJCXX_STANDARD 20 OBJCXX_STANDARD_REQUIRED YES)
    target_include_directories(mb3d_tests_native_command_gpu PRIVATE "${CMAKE_SOURCE_DIR}" "${CMAKE_SOURCE_DIR}/src")
    target_compile_options(mb3d_tests_native_command_gpu PRIVATE -fobjc-arc -fno-fast-math -ffp-contract=off)
    target_link_libraries(mb3d_tests_native_command_gpu PRIVATE mb3d_render_backend "-framework Foundation" "-framework Metal")
    add_dependencies(mb3d_tests_native_command_gpu mb3d_metal_native_shaders)
    add_test(NAME metal_native_command_tests COMMAND mb3d_tests_native_command_gpu "${CMAKE_CURRENT_BINARY_DIR}/metal_native_scene.metallib")
    set_tests_properties(metal_native_command_tests PROPERTIES LABELS "metal_native" TIMEOUT 300 RUN_SERIAL TRUE)
    add_executable(mb3d_tests_native_de_ao_gpu comparison/tools/test_native_de_ao_pixels.mm)
    set_target_properties(mb3d_tests_native_de_ao_gpu PROPERTIES OBJCXX_STANDARD 20 OBJCXX_STANDARD_REQUIRED YES)
    target_include_directories(mb3d_tests_native_de_ao_gpu PRIVATE "${CMAKE_SOURCE_DIR}" "${CMAKE_SOURCE_DIR}/src")
    target_compile_options(mb3d_tests_native_de_ao_gpu PRIVATE -fobjc-arc -fno-fast-math -ffp-contract=off)
    target_link_libraries(mb3d_tests_native_de_ao_gpu PRIVATE mb3d_render_backend "-framework Foundation" "-framework Metal")
    add_dependencies(mb3d_tests_native_de_ao_gpu mb3d_metal_native_shaders)
    # This diagnostic takes an explicit audited depth scene/resource root;
    # the qualification runner supplies them from its hash-pinned contract.
    add_executable(mb3d_tests_native_dof_gpu tests/test_native_dof_gpu.mm)
    set_target_properties(mb3d_tests_native_dof_gpu PROPERTIES OBJCXX_STANDARD 20 OBJCXX_STANDARD_REQUIRED YES)
    target_include_directories(mb3d_tests_native_dof_gpu PRIVATE "${CMAKE_SOURCE_DIR}" "${CMAKE_SOURCE_DIR}/src")
    target_compile_options(mb3d_tests_native_dof_gpu PRIVATE -fobjc-arc -fno-fast-math -ffp-contract=off)
    target_link_libraries(mb3d_tests_native_dof_gpu PRIVATE mb3d_render_backend "-framework Foundation" "-framework Metal")
    add_dependencies(mb3d_tests_native_dof_gpu mb3d_metal_native_shaders)
    add_test(NAME metal_native_dof_tests COMMAND mb3d_tests_native_dof_gpu "${CMAKE_CURRENT_BINARY_DIR}/metal_native_scene.metallib")
    set_tests_properties(metal_native_dof_tests PROPERTIES LABELS "metal_native" TIMEOUT 300)
    add_executable(mb3d_tests_native_dof_render tests/test_native_dof_render.cpp)
    target_include_directories(mb3d_tests_native_dof_render PRIVATE "${CMAKE_SOURCE_DIR}")
    target_link_libraries(mb3d_tests_native_dof_render PRIVATE mb3d_render_backend)
    add_executable(mb3d_tests_native_zbuffer_gpu tests/test_native_zbuffer_gpu.mm)
    set_target_properties(mb3d_tests_native_zbuffer_gpu PROPERTIES OBJCXX_STANDARD 20 OBJCXX_STANDARD_REQUIRED YES)
    target_include_directories(mb3d_tests_native_zbuffer_gpu PRIVATE "${CMAKE_SOURCE_DIR}" "${CMAKE_SOURCE_DIR}/src")
    target_compile_options(mb3d_tests_native_zbuffer_gpu PRIVATE -fobjc-arc -fno-fast-math -ffp-contract=off)
    target_link_libraries(mb3d_tests_native_zbuffer_gpu PRIVATE mb3d_render_backend "-framework Foundation" "-framework Metal")
    add_dependencies(mb3d_tests_native_zbuffer_gpu mb3d_metal_native_shaders)
    add_test(NAME metal_native_zbuffer_tests COMMAND mb3d_tests_native_zbuffer_gpu "${CMAKE_CURRENT_BINARY_DIR}/metal_native_scene.metallib")
    set_tests_properties(metal_native_zbuffer_tests PROPERTIES LABELS "metal_native" TIMEOUT 300)
    add_executable(mb3d_tests_native_zbuffer_render tests/test_native_zbuffer_render.cpp)
    target_include_directories(mb3d_tests_native_zbuffer_render PRIVATE "${CMAKE_SOURCE_DIR}")
    target_link_libraries(mb3d_tests_native_zbuffer_render PRIVATE mb3d_render_backend)
    add_dependencies(mb3d_render_backend mb3d_metal_native_shaders)
endif()

add_executable(mb3d_render src/cli/main.cpp)
target_link_libraries(mb3d_render PRIVATE mb3d_render_backend ZLIB::ZLIB)
if(MB3D_ENABLE_METAL_RENDERER)
    # Stage on every requested renderer build: a shader-only change need not
    # relink the executable, so POST_BUILD would leave a stale adjacent library.
    add_custom_target(mb3d_stage_native_metallib
        COMMAND "${CMAKE_COMMAND}" -E make_directory
            "$<TARGET_FILE_DIR:mb3d_render>"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
            "${CMAKE_CURRENT_BINARY_DIR}/metal_native_scene.metallib"
            "$<TARGET_FILE_DIR:mb3d_render>/metal_native_scene.metallib"
        VERBATIM)
    add_dependencies(mb3d_stage_native_metallib mb3d_metal_native_shaders)
    add_dependencies(mb3d_render mb3d_stage_native_metallib)
endif()
target_compile_definitions(mb3d_render PRIVATE
    $<$<BOOL:${MB3D_ENABLE_PARITY_TRACE}>:MB3D_ENABLE_PARITY_TRACE>)

add_executable(mb3d_cpu_render_benchmark comparison/tools/cpu_render_benchmark.cpp)
target_link_libraries(mb3d_cpu_render_benchmark PRIVATE mb3d_core mb3d_io ZLIB::ZLIB)

add_executable(mb3d_backend_render_benchmark comparison/tools/backend_render_benchmark.cpp)
target_link_libraries(mb3d_backend_render_benchmark PRIVATE mb3d_render_backend ZLIB::ZLIB)

add_executable(mb3d_tests_render_backend tests/test_render_backend.cpp)
target_link_libraries(mb3d_tests_render_backend PRIVATE mb3d_render_backend)
if(MB3D_ENABLE_METAL_RENDERER)
    add_test(NAME render_backend_tests COMMAND mb3d_tests_render_backend "${CMAKE_SOURCE_DIR}"
        --metal-library "${CMAKE_CURRENT_BINARY_DIR}/metal_native_scene.metallib")
else()
    add_test(NAME render_backend_tests COMMAND mb3d_tests_render_backend "${CMAKE_SOURCE_DIR}")
endif()
set_tests_properties(render_backend_tests PROPERTIES TIMEOUT 180 LABELS "render_backend" RUN_SERIAL TRUE)

if(MB3D_ENABLE_METAL_RENDERER)
    add_executable(mb3d_tests_native_reflection_host comparison/metal_exact/test_native_reflection_host.cpp)
    target_include_directories(mb3d_tests_native_reflection_host PRIVATE "${CMAKE_SOURCE_DIR}")
    target_link_libraries(mb3d_tests_native_reflection_host PRIVATE mb3d_render_backend)
    add_test(NAME native_reflection_host_tests COMMAND mb3d_tests_native_reflection_host
        "${CMAKE_SOURCE_DIR}/comparison/benchmark_scenes/effects-v1/integerpower8-reflections.m3p")
    set_tests_properties(native_reflection_host_tests PROPERTIES TIMEOUT 60 LABELS "render_backend")
    add_executable(mb3d_tests_native_reflection_gpu comparison/metal_exact/test_native_reflection_gpu.mm)
    target_include_directories(mb3d_tests_native_reflection_gpu PRIVATE "${CMAKE_SOURCE_DIR}")
    set_target_properties(mb3d_tests_native_reflection_gpu PROPERTIES OBJCXX_STANDARD 20 OBJCXX_STANDARD_REQUIRED YES)
    target_compile_options(mb3d_tests_native_reflection_gpu PRIVATE -fobjc-arc)
    target_link_libraries(mb3d_tests_native_reflection_gpu PRIVATE mb3d_render_backend "-framework Foundation" "-framework Metal")
    add_dependencies(mb3d_tests_native_reflection_gpu mb3d_metal_native_shaders)
    add_test(NAME native_reflection_gpu_tests COMMAND mb3d_tests_native_reflection_gpu
        "${CMAKE_CURRENT_BINARY_DIR}/metal_native_scene.metallib"
        "${CMAKE_SOURCE_DIR}/comparison/benchmark_scenes/effects-v1/integerpower8-reflections.m3p")
    set_tests_properties(native_reflection_gpu_tests PROPERTIES TIMEOUT 60 LABELS "render_backend" RUN_SERIAL TRUE)
endif()

# Test executable: math3d tests
add_executable(mb3d_scene_loader_probe src/cli/scene_loader_probe.cpp)
target_link_libraries(mb3d_scene_loader_probe PRIVATE mb3d_core mb3d_io ZLIB::ZLIB)

add_executable(mb3d_tests tests/test_math3d.cpp)
target_link_libraries(mb3d_tests PRIVATE mb3d_core)
target_compile_definitions(mb3d_tests PRIVATE
    $<$<BOOL:${MB3D_ENABLE_PARITY_TRACE}>:MB3D_ENABLE_PARITY_TRACE>)

# Test executable: m3p parser tests
add_executable(mb3d_tests_m3p tests/test_m3p_parser.cpp)
target_link_libraries(mb3d_tests_m3p PRIVATE mb3d_core mb3d_io ZLIB::ZLIB)
target_compile_definitions(mb3d_tests_m3p PRIVATE
    $<$<BOOL:${MB3D_ENABLE_PARITY_TRACE}>:MB3D_ENABLE_PARITY_TRACE>)

# Independent Pascal cache fixtures and malformed-file checks.
add_executable(mb3d_tests_m3i tests/test_m3i_parser.cpp)
target_link_libraries(mb3d_tests_m3i PRIVATE mb3d_core mb3d_io ZLIB::ZLIB)
target_compile_definitions(mb3d_tests_m3i PRIVATE
    MB3D_TEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

# Test executable: renderer regression tests
add_executable(mb3d_tests_renderer tests/test_renderer.cpp)
if(MB3D_MSVC_X64)
    target_sources(mb3d_tests_renderer PRIVATE tests/fixture_x87_environment.asm)
endif()
target_link_libraries(mb3d_tests_renderer PRIVATE mb3d_core mb3d_io ZLIB::ZLIB)
target_compile_definitions(mb3d_tests_renderer PRIVATE
    $<$<BOOL:${MB3D_ENABLE_PARITY_TRACE}>:MB3D_ENABLE_PARITY_TRACE>)

# Test executable: Halton sequence tests (Monte Carlo precompute parity)
add_executable(mb3d_tests_halton tests/test_halton.cpp)
target_link_libraries(mb3d_tests_halton PRIVATE mb3d_core)
target_compile_definitions(mb3d_tests_halton PRIVATE
    $<$<BOOL:${MB3D_ENABLE_PARITY_TRACE}>:MB3D_ENABLE_PARITY_TRACE>)

# CTest integration
if(MB3D_MSVC_X64 OR MB3D_APPLE_ARM64)
    add_executable(mb3d_tests_numerical_de tests/test_numerical_de.cpp)
    target_link_libraries(mb3d_tests_numerical_de PRIVATE mb3d_core)
    add_test(NAME numerical_de_tests COMMAND mb3d_tests_numerical_de)
    set_tests_properties(numerical_de_tests PROPERTIES TIMEOUT 30)
    add_executable(mb3d_tests_numerical_camera tests/test_numerical_camera.cpp)
    target_link_libraries(mb3d_tests_numerical_camera PRIVATE mb3d_core)
    add_test(NAME numerical_camera_tests COMMAND mb3d_tests_numerical_camera)
    set_tests_properties(numerical_camera_tests PROPERTIES TIMEOUT 30)
    add_executable(mb3d_tests_native_precision tests/test_native_precision.cpp)
    target_link_libraries(mb3d_tests_native_precision PRIVATE mb3d_runtime mb3d_softfloat)
    add_test(NAME native_precision_tests COMMAND mb3d_tests_native_precision)
    set_tests_properties(native_precision_tests PROPERTIES TIMEOUT 30)
    add_executable(mb3d_tests_render_precision tests/test_render_precision.cpp)
    target_link_libraries(mb3d_tests_render_precision PRIVATE mb3d_io)
    add_test(NAME render_precision_tests COMMAND mb3d_tests_render_precision "${CMAKE_SOURCE_DIR}")
    set_tests_properties(render_precision_tests PROPERTIES TIMEOUT 120)
    add_executable(mb3d_tests_manowar_hybrid tests/test_manowar_hybrid.cpp)
    target_link_libraries(mb3d_tests_manowar_hybrid PRIVATE mb3d_core mb3d_io ZLIB::ZLIB)
    target_compile_definitions(mb3d_tests_manowar_hybrid PRIVATE MB3D_TEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
    add_test(NAME manowar_hybrid_tests COMMAND mb3d_tests_manowar_hybrid)
    set_tests_properties(manowar_hybrid_tests PROPERTIES TIMEOUT 30)
    add_executable(mb3d_tests_manowar_sse_producer tests/test_manowar_sse_producer.cpp)
    target_link_libraries(mb3d_tests_manowar_sse_producer PRIVATE mb3d_core mb3d_io ZLIB::ZLIB)
    target_compile_definitions(mb3d_tests_manowar_sse_producer PRIVATE MB3D_TEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
    add_test(NAME manowar_sse_producer_tests COMMAND mb3d_tests_manowar_sse_producer)
    set_tests_properties(manowar_sse_producer_tests PROPERTIES TIMEOUT 30)
endif()
add_test(NAME math3d_tests COMMAND mb3d_tests)
add_test(NAME m3p_parser_tests COMMAND mb3d_tests_m3p)
add_test(NAME m3i_parser_tests COMMAND mb3d_tests_m3i)
set_tests_properties(m3i_parser_tests PROPERTIES TIMEOUT 30)
add_test(NAME renderer_tests COMMAND mb3d_tests_renderer)
add_test(NAME halton_tests COMMAND mb3d_tests_halton)

add_executable(mb3d_tests_external_registry tests/test_external_registry.cpp)
target_link_libraries(mb3d_tests_external_registry PRIVATE mb3d_core)
add_test(NAME external_registry_tests COMMAND mb3d_tests_external_registry)
set_tests_properties(external_registry_tests PROPERTIES TIMEOUT 15)

add_executable(mb3d_tests_integer_options tests/test_integer_options.cpp)
if(MB3D_MSVC_X64)
    target_sources(mb3d_tests_integer_options PRIVATE tests/fixture_x87_environment.asm)
endif()
target_link_libraries(mb3d_tests_integer_options PRIVATE mb3d_core)
add_test(NAME integer_options_tests COMMAND mb3d_tests_integer_options)

add_executable(mb3d_tests_formula_validation tests/test_formula_validation.cpp)
target_link_libraries(mb3d_tests_formula_validation PRIVATE mb3d_core)
add_test(NAME formula_validation_tests COMMAND mb3d_tests_formula_validation)

add_executable(mb3d_tests_formula_iteration_guard tests/test_formula_iteration_guard.cpp)
target_link_libraries(mb3d_tests_formula_iteration_guard PRIVATE mb3d_core)
add_test(NAME formula_iteration_guard_tests COMMAND mb3d_tests_formula_iteration_guard)
set_tests_properties(formula_iteration_guard_tests PROPERTIES TIMEOUT 30)

add_executable(mb3d_tests_formula_work_guard tests/test_formula_work_guard.cpp)
if(MB3D_MSVC_X64)
    target_sources(mb3d_tests_formula_work_guard PRIVATE tests/fixture_x87_environment.asm)
endif()
target_link_libraries(mb3d_tests_formula_work_guard PRIVATE mb3d_core)
add_test(NAME formula_work_guard_tests COMMAND mb3d_tests_formula_work_guard)
set_tests_properties(formula_work_guard_tests PROPERTIES TIMEOUT 30)

add_executable(mb3d_tests_zbuffer_normals tests/test_zbuffer_normals.cpp)
target_link_libraries(mb3d_tests_zbuffer_normals PRIVATE mb3d_core)
add_test(NAME zbuffer_normals_tests COMMAND mb3d_tests_zbuffer_normals)

add_executable(mb3d_tests_native_postprocess_plan tests/test_native_postprocess_plan.cpp)
target_include_directories(mb3d_tests_native_postprocess_plan PRIVATE "${CMAKE_SOURCE_DIR}" "${CMAKE_SOURCE_DIR}/src")
add_test(NAME native_postprocess_plan_tests COMMAND mb3d_tests_native_postprocess_plan)
set_tests_properties(native_postprocess_plan_tests PROPERTIES TIMEOUT 15)

add_executable(mb3d_tests_native_material_palette tests/test_native_material_palette.cpp)
target_include_directories(mb3d_tests_native_material_palette PRIVATE "${CMAKE_SOURCE_DIR}" "${CMAKE_SOURCE_DIR}/src")
if(NOT MSVC)
    target_compile_options(mb3d_tests_native_material_palette PRIVATE -fno-fast-math -ffp-contract=off)
endif()
add_test(NAME native_material_palette_tests COMMAND mb3d_tests_native_material_palette)
set_tests_properties(native_material_palette_tests PROPERTIES TIMEOUT 15)

add_executable(mb3d_tests_camera_tile_setup tests/test_camera_tile_setup.cpp)
target_link_libraries(mb3d_tests_camera_tile_setup PRIVATE mb3d_core mb3d_io ZLIB::ZLIB)
add_test(NAME camera_tile_setup_tests COMMAND mb3d_tests_camera_tile_setup)

add_executable(mb3d_tests_ao_wavelet_lanes tests/test_ao_wavelet_lanes.cpp)
target_link_libraries(mb3d_tests_ao_wavelet_lanes PRIVATE mb3d_core mb3d_io ZLIB::ZLIB)
add_test(NAME ao_wavelet_lanes_tests COMMAND mb3d_tests_ao_wavelet_lanes)

add_executable(mb3d_tests_dof tests/test_dof.cpp)
target_link_libraries(mb3d_tests_dof PRIVATE mb3d_core)
add_test(NAME dof_tests COMMAND mb3d_tests_dof)

add_executable(mb3d_tests_render_failures tests/test_render_failures.cpp)
target_link_libraries(mb3d_tests_render_failures PRIVATE mb3d_core mb3d_io ZLIB::ZLIB)
add_test(NAME render_failures_tests COMMAND mb3d_tests_render_failures
    "${CMAKE_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/render-failure-fixtures")
set_tests_properties(render_failures_tests PROPERTIES TIMEOUT 45)

add_executable(mb3d_tests_render_control_api tests/test_render_control_api.cpp)
target_link_libraries(mb3d_tests_render_control_api PRIVATE mb3d_core mb3d_io ZLIB::ZLIB)
add_test(NAME render_control_api_tests COMMAND mb3d_tests_render_control_api "${CMAKE_SOURCE_DIR}")
set_tests_properties(render_control_api_tests PROPERTIES TIMEOUT 60)

add_executable(mb3d_tests_render_profile tests/test_render_profile.cpp)
target_link_libraries(mb3d_tests_render_profile PRIVATE mb3d_core mb3d_io ZLIB::ZLIB)
add_test(NAME render_profile_tests COMMAND mb3d_tests_render_profile "${CMAKE_SOURCE_DIR}")
set_tests_properties(render_profile_tests PROPERTIES TIMEOUT 60)

find_package(Python3 COMPONENTS Interpreter QUIET)
if(Python3_Interpreter_FOUND)
    add_test(NAME render_control_cli_tests
        COMMAND "${Python3_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/tests/test_render_control_cli.py"
            "$<TARGET_FILE:mb3d_render>" "${CMAKE_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/render-control-cli-fixtures")
    set_tests_properties(render_control_cli_tests PROPERTIES TIMEOUT 90)
    add_test(NAME render_profile_cli_tests
        COMMAND "${Python3_EXECUTABLE}" -B "${CMAKE_SOURCE_DIR}/tests/test_render_profile_cli.py"
            "$<TARGET_FILE:mb3d_render>" "${CMAKE_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/profile-cli-tests")
    set_tests_properties(render_profile_cli_tests PROPERTIES TIMEOUT 90)
    set(MB3D_BACKEND_TEST_ARGS --renderer "$<TARGET_FILE:mb3d_render>"
        --source-root "${CMAKE_SOURCE_DIR}" --scratch "${CMAKE_BINARY_DIR}/backend-cli-tests")
    if(MB3D_ENABLE_METAL_RENDERER)
        list(APPEND MB3D_BACKEND_TEST_ARGS --expect-metal yes
            --native-library "${CMAKE_CURRENT_BINARY_DIR}/metal_native_scene.metallib")
    else()
        list(APPEND MB3D_BACKEND_TEST_ARGS --expect-metal no)
    endif()
    add_test(NAME render_backend_cli_tests COMMAND "${Python3_EXECUTABLE}" -B
        "${CMAKE_SOURCE_DIR}/tests/test_render_backend_cli.py" ${MB3D_BACKEND_TEST_ARGS})
    set_tests_properties(render_backend_cli_tests PROPERTIES TIMEOUT 300 LABELS "render_backend" RUN_SERIAL TRUE)
endif()

add_executable(mb3d_tests_render_isolation tests/test_render_isolation.cpp)
target_link_libraries(mb3d_tests_render_isolation PRIVATE mb3d_core mb3d_io ZLIB::ZLIB)
add_test(NAME render_isolation_tests COMMAND mb3d_tests_render_isolation
    "${CMAKE_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/render-isolation-fixtures")
set_tests_properties(render_isolation_tests PROPERTIES TIMEOUT 120)

add_executable(mb3d_tests_legacy_loader tests/test_legacy_loader.cpp)
target_link_libraries(mb3d_tests_legacy_loader PRIVATE mb3d_core mb3d_io ZLIB::ZLIB)
add_test(NAME legacy_loader_tests COMMAND mb3d_tests_legacy_loader
    "${CMAKE_SOURCE_DIR}/tests/fixtures/legacy_loader" "${CMAKE_BINARY_DIR}/legacy-loader-fixtures")
set_tests_properties(legacy_loader_tests PROPERTIES TIMEOUT 30)

add_executable(mb3d_tests_monte_carlo tests/test_monte_carlo.cpp)
target_link_libraries(mb3d_tests_monte_carlo PRIVATE mb3d_core mb3d_io)
add_test(NAME monte_carlo_tests COMMAND mb3d_tests_monte_carlo "${CMAKE_SOURCE_DIR}/comparison/scenes/test_01_intpow2.m3p")
set_tests_properties(monte_carlo_tests PROPERTIES TIMEOUT 90)

add_executable(mb3d_mc_sampling_probe comparison/tools/mc_sampling_probe.cpp)
target_link_libraries(mb3d_mc_sampling_probe PRIVATE mb3d_core)

add_executable(mb3d_formula_point_probe comparison/tools/formula_point_probe.cpp)
target_link_libraries(mb3d_formula_point_probe PRIVATE mb3d_core mb3d_io)

add_executable(mb3d_normalize_scene comparison/tools/normalize_scene.cpp)
target_link_libraries(mb3d_normalize_scene PRIVATE mb3d_core mb3d_io)

add_executable(mb3d_formula_orbit_probe comparison/tools/formula_orbit_probe.cpp)
target_link_libraries(mb3d_formula_orbit_probe PRIVATE mb3d_core mb3d_io)

add_executable(mb3d_tests_background_resources tests/test_background_resources.cpp)
target_link_libraries(mb3d_tests_background_resources PRIVATE mb3d_core)
add_test(NAME background_resources_tests COMMAND mb3d_tests_background_resources)
set_tests_properties(background_resources_tests PROPERTIES TIMEOUT 60)

# A complete clean-machine build/check entry point for the active CPU scope.
# Deferred Monte Carlo/Halton targets remain available but are not selected.
set(MB3D_ORDINARY_TARGETS
    mb3d_render mb3d_formula_point_probe mb3d_formula_orbit_probe
    mb3d_scene_loader_probe mb3d_normalize_scene
    mb3d_tests mb3d_tests_m3p mb3d_tests_m3i mb3d_tests_renderer
    mb3d_tests_pas_lcg mb3d_tests_thread_pool mb3d_tests_external_registry
    mb3d_tests_portable_sqrt mb3d_tests_sse_estimates mb3d_tests_integer_options
    mb3d_tests_formula_validation mb3d_tests_formula_iteration_guard
    mb3d_tests_formula_work_guard mb3d_tests_zbuffer_normals mb3d_tests_native_postprocess_plan mb3d_tests_native_material_palette
    mb3d_tests_camera_tile_setup mb3d_tests_ao_wavelet_lanes mb3d_tests_dof
    mb3d_tests_background_resources mb3d_tests_render_failures mb3d_tests_render_isolation mb3d_tests_render_context mb3d_tests_legacy_loader
    mb3d_tests_map_resources mb3d_tests_render_control mb3d_tests_render_control_api_lifecycle
    mb3d_tests_portable_formula_control mb3d_tests_render_control_image_process
    mb3d_tests_render_control_api mb3d_tests_render_profile mb3d_tests_render_backend)
set(MB3D_ORDINARY_TESTS
    math3d_tests m3p_parser_tests m3i_parser_tests renderer_tests pas_lcg_tests
    thread_pool_tests external_registry_tests portable_sqrt_tests sse_estimates_tests integer_options_tests
    formula_validation_tests formula_iteration_guard_tests formula_work_guard_tests
    zbuffer_normals_tests native_postprocess_plan_tests native_material_palette_tests camera_tile_setup_tests ao_wavelet_lanes_tests dof_tests
    background_resources_tests render_failures_tests render_isolation_tests render_context_tests legacy_loader_tests
    map_resources_tests render_control_tests render_control_api_lifecycle_tests
    portable_formula_control_tests render_control_image_process_tests render_control_api_tests render_profile_tests render_backend_tests)
if(MB3D_ENABLE_METAL_RENDERER)
    list(APPEND MB3D_ORDINARY_TARGETS mb3d_tests_native_render_plan mb3d_tests_native_resource_entry mb3d_tests_native_formula_resources)
    list(APPEND MB3D_ORDINARY_TESTS native_render_plan_tests native_resource_entry_tests native_formula_resources_tests)
endif()
if(Python3_Interpreter_FOUND)
    list(APPEND MB3D_ORDINARY_TESTS render_control_cli_tests render_profile_cli_tests render_backend_cli_tests)
endif()
if(MB3D_MSVC_X64 OR MB3D_APPLE_ARM64)
    list(APPEND MB3D_ORDINARY_TARGETS mb3d_tests_manowar_hybrid mb3d_tests_manowar_sse_producer
        mb3d_tests_numerical_de mb3d_tests_numerical_camera mb3d_tests_native_precision mb3d_tests_render_precision)
    list(APPEND MB3D_ORDINARY_TESTS manowar_hybrid_tests manowar_sse_producer_tests
        numerical_de_tests numerical_camera_tests native_precision_tests render_precision_tests)
endif()
if(MB3D_APPLE_ARM64)
    list(APPEND MB3D_ORDINARY_TARGETS mb3d_tests_arm64_numerics
        mb3d_tests_extended_sine mb3d_tests_extended_small_division mb3d_tests_extended_product
        mb3d_tests_extended_series)
    list(APPEND MB3D_ORDINARY_TESTS arm64_numerics_tests
        extended_sine_tests extended_small_division_tests extended_product_tests extended_series_tests)
endif()
if(MB3D_ENABLE_METAL_RENDERER)
    list(APPEND MB3D_ORDINARY_TARGETS mb3d_tests_native_reflection_host mb3d_tests_native_reflection_gpu)
    list(APPEND MB3D_ORDINARY_TESTS native_reflection_host_tests native_reflection_gpu_tests)
endif()
set_property(TEST ${MB3D_ORDINARY_TESTS} APPEND PROPERTY LABELS ordinary)
if(TARGET example)
    list(APPEND MB3D_ORDINARY_TARGETS example)
    set_property(TEST example DIRECTORY "${zlib_SOURCE_DIR}" PROPERTY LABELS ordinary)
endif()
add_custom_target(mb3d_ordinary DEPENDS ${MB3D_ORDINARY_TARGETS})

if(MB3D_APPLE_ARM64)
    # A reproducible focused gate. The ordinary label independently exercises
    # the complete tracked renderer corpus; retain both result sets.
    set(MB3D_ARM64_QUALIFIED_TESTS ${MB3D_ORDINARY_TESTS})
    list(REMOVE_ITEM MB3D_ARM64_QUALIFIED_TESTS renderer_tests)
    set_property(TEST ${MB3D_ARM64_QUALIFIED_TESTS} APPEND PROPERTY LABELS arm64_qualified)
    set_property(TEST renderer_tests
        APPEND PROPERTY LABELS arm64_numerical_corpus)
    foreach(formula IN ITEMS Aexion1 GeneralQuat Magnet4D Mandel4DBiC
            _JuliaSets _MapTranslate _SphereFolding1 _hyc3Dinv
            _reciprocalX3 _reciprocalY3 _reciprocalZ3 PiriformIFS columnsIFS
            cubetubeIFS foldcut8-IFS tilingbrickIFS tilingbrick2IFS
            _SinApprox _PlusSinApp ABoxSmoothFold _AmazingBox BT1Pinegon HeightMapIFS SphereHeightMap SphereHMVoid MengerKochV2
            CylinderHeightMap CylinderHMVoid TorusHMVoid QuadricsIFS PG-bulb _Fold45double
            ManowarBulb ATetraVS ABoxVaryScale ABoxVaryScale2 ABoxVaryScale4d
            sliceIFS sliceExIFS TrifoxComplexAngles HeightMap2IFS cayleySurfIFS
            MandelView _helispiral
            ABoxVSWavy ABoxVS_icen1 ABoxVS_icen2 ABoxMod2 Mandalex Mandalay-KIFS
            CrossBiFold OctKoch Benesi1Pow2 MsltoeFoldQuat MsltoeSym4 General Fuzzy
            _Sin4Dplus _setw _invbisp _BStellahedron_F_Sphere _BPolyhedronToSphere
            _BPolygonFromCircle _BPolygonToCircle
            RoomDoorsIFS CommaIFS LimpetIFS RiemSqrtIFS borgIFS greekIFS toupieIFS
            DevilIFS loxodrome2IFS _IFS_tess logspIFS CitrusIFS
            BT1Pine QuatPow Aex-TorusPow IQ-bulb "IQ-bulb old"
            IQ_NormBulb IQ_NormBulb_NoYZ IQ_PineTree IQ_PineTree_NoYZ Ikenagabulb
            dodecatubeIFS tetratubeIFS diniIFS wavespIFS pyramIFS boxIFS gyroidIFS
            ABoxVSShapes "TetraFolding Pow2" _coordvspow tritgridIFS MengerHyper
            Riemann Ruckerbulb TalisTpx KleinBagelIFS "AmazingBox2" UmbrellaIFS
            ABoxMapSFold Barnsley4Dc Barnsley4Dnc ScarabIFS gumdropIFS
            "Amazing Surf 2" ducksIFS gnarlyIFS TgladDihed _RotatePlaneOnIts
            _RotateXYonIts "Amazing Surf" TgladTetra BulboxP-2 TgladDihed2
            TgladTetra2 _recFold schwartzIFS gearIFS hextgrid4IFS _FoldingVar
            ABoxPlatinumB helistairsIFS)
        string(MAKE_C_IDENTIFIER "${formula}" key)
        add_test(NAME arm64_formula_${key} COMMAND mb3d_tests_renderer
            test_external_formula_regressions "${formula}")
        set_tests_properties(arm64_formula_${key} PROPERTIES LABELS arm64_qualified TIMEOUT 30)
    endforeach()
    foreach(kernel IN ITEMS external_exp_range iteration_boundary_original_pascal pinehedron_original_code
            corrected_parity_image_hotspots original_formula_angle_loader)
        add_test(NAME arm64_${kernel} COMMAND mb3d_tests_renderer test_${kernel})
        set_tests_properties(arm64_${kernel} PROPERTIES LABELS arm64_qualified TIMEOUT 120)
    endforeach()
    add_custom_target(mb3d_arm64_qualified DEPENDS ${MB3D_ORDINARY_TARGETS})
endif()

# Native Metal diagnostics share the production renderer and stay out of CPU-only builds.
if(MB3D_ENABLE_METAL_RENDERER)
    add_executable(mb3d_phase1_admission comparison/tools/phase1_admission.cpp)
    target_include_directories(mb3d_phase1_admission PRIVATE "${CMAKE_SOURCE_DIR}" "${CMAKE_SOURCE_DIR}/src")
    target_link_libraries(mb3d_phase1_admission PRIVATE mb3d_render_backend)

    add_executable(mb3d_tests_phase1_color tests/test_phase1_color_gpu.mm)
    set_target_properties(mb3d_tests_phase1_color PROPERTIES OBJCXX_STANDARD 20 OBJCXX_STANDARD_REQUIRED YES)
    target_compile_options(mb3d_tests_phase1_color PRIVATE -fobjc-arc)
    target_include_directories(mb3d_tests_phase1_color PRIVATE "${CMAKE_SOURCE_DIR}" "${CMAKE_SOURCE_DIR}/src")
    target_link_libraries(mb3d_tests_phase1_color PRIVATE mb3d_render_backend "-framework Metal" "-framework Foundation")
    add_dependencies(mb3d_tests_phase1_color mb3d_metal_native_shaders)

    foreach(ao_seed_mode 2 4 5 8 9)
        add_test(NAME metal_native_ao_seed_${ao_seed_mode}_tests COMMAND mb3d_tests_native_ao_seed_gpu "${CMAKE_CURRENT_BINARY_DIR}/metal_native_scene.metallib" ${ao_seed_mode})
        set_tests_properties(metal_native_ao_seed_${ao_seed_mode}_tests PROPERTIES LABELS "metal_native" TIMEOUT 180 RUN_SERIAL TRUE)
    endforeach()
    add_test(NAME metal_native_ao_chunk_tests COMMAND mb3d_tests_native_ao_chunk_gpu "${CMAKE_CURRENT_BINARY_DIR}/metal_native_scene.metallib")
    set_tests_properties(metal_native_ao_chunk_tests PROPERTIES LABELS "metal_native" TIMEOUT 180 RUN_SERIAL TRUE)
    add_test(NAME metal_native_ao_certificate_tests COMMAND mb3d_tests_native_ao_certificate_gpu "${CMAKE_CURRENT_BINARY_DIR}/metal_native_scene.metallib")
    set_tests_properties(metal_native_ao_certificate_tests PROPERTIES LABELS "metal_native" TIMEOUT 300 RUN_SERIAL TRUE)
endif()
