# The release gate reuses existing tests. It neither runs the historical
# numerical corpus nor replaces scene/formula qualification.
function(mb3d_release_test test target)
    if(NOT TEST ${test} OR NOT TARGET ${target})
        message(FATAL_ERROR "Missing release check: ${test} (${target})")
    endif()
    set_property(TEST ${test} APPEND PROPERTY LABELS release)
    add_dependencies(mb3d_release_checks ${target})
endfunction()

function(mb3d_configure_release_checks)
    add_custom_target(mb3d_release_checks)

    # Context ownership and the public control handle's concurrent lifecycle.
    mb3d_release_test(render_context_tests mb3d_tests_render_context)
    mb3d_release_test(render_control_api_lifecycle_tests mb3d_tests_render_control_api_lifecycle)

    if(NOT MB3D_NATIVE_TESTS_ONLY)
        add_dependencies(mb3d_release_checks mb3d_render)
        # Public render API, scene loading, formula registration and rejection
        # of malformed inputs or unbounded formula work.
        mb3d_release_test(render_control_api_tests mb3d_tests_render_control_api)
        mb3d_release_test(m3p_parser_tests mb3d_tests_m3p)
        # Application API: scene editing, stereo, clip volume, progressive passes.
        mb3d_release_test(scene_api_tests mb3d_tests_scene_api)
        mb3d_release_test(external_registry_tests mb3d_tests_external_registry)
        mb3d_release_test(formula_validation_tests mb3d_tests_formula_validation)
        mb3d_release_test(formula_iteration_guard_tests mb3d_tests_formula_iteration_guard)
        mb3d_release_test(formula_work_guard_tests mb3d_tests_formula_work_guard)
        if(MB3D_MSVC_X64 OR MB3D_APPLE_ARM64)
            # Arithmetic mode isolation, conversion and restoration of TLS.
            mb3d_release_test(native_precision_tests mb3d_tests_native_precision)
        endif()
        if(Python3_Interpreter_FOUND)
            # Small CLI renders: cancellation, budget/output failures and both
            # precision modes. The exhaustive backend CLI suite stays ordinary.
            mb3d_release_test(render_control_cli_tests mb3d_render)
            mb3d_release_test(render_profile_cli_tests mb3d_render)
        endif()
        if(MB3D_ENABLE_METAL_RENDERER)
            # Host scheduling/admission and the embedded pixel-state contract.
            mb3d_release_test(native_render_plan_tests mb3d_tests_native_render_plan)
        endif()
    endif()

    # Keep explicit ordinary/native/qualified and individual targets usable.
    # EXCLUDE_FROM_ALL does not remove dependencies requested by those targets.
    # Production libraries, shaders and their required validators are unchanged.
    get_property(targets DIRECTORY PROPERTY BUILDSYSTEM_TARGETS)
    foreach(target IN LISTS targets)
        get_target_property(kind ${target} TYPE)
        if(NOT MB3D_BUILD_DIAGNOSTICS AND
           (kind STREQUAL "EXECUTABLE" OR kind STREQUAL "UTILITY") AND
           NOT target STREQUAL "mb3d_render")
            set_property(TARGET ${target} PROPERTY EXCLUDE_FROM_ALL TRUE)
        endif()
    endforeach()
    # Deferred Monte Carlo/Halton are never pulled in by the diagnostics switch.
    # Fetched zlib examples also remain opt-in; ordinary requests its example.
    foreach(target IN ITEMS mb3d_tests_monte_carlo mb3d_tests_halton mb3d_mc_sampling_probe
                            example example64 minigzip minigzip64)
        if(TARGET ${target})
            set_property(TARGET ${target} PROPERTY EXCLUDE_FROM_ALL TRUE)
        endif()
    endforeach()
endfunction()
