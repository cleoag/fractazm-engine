"""Synthetic IR and driver-table fixtures for native_formula_effects.

These check the analyser, not any formula. The optional compile case uses the
current tree and Metal toolchain; its result is not device evidence.
"""
import hashlib, json, shutil, subprocess, tempfile, unittest
from unittest.mock import patch
from pathlib import Path
import native_formula_effects as fx

STATE = '%"struct.mb3d_native::NativeFormulaState"'
HEADER = (f'define void @effect_probe({STATE} addrspace(1)* nocapture readonly %0, '
          f'{STATE} addrspace(1)* nocapture writeonly %1, i32 addrspace(1)* %7) local_unnamed_addr #3 {{')


def kernel(*lines):
    return "\n".join([HEADER, *("  " + line for line in lines), "  ret void", "}"]) + "\n"


def gep(name, base, field, index=None):
    tail = f", i64 {index}" if index is not None else ""
    return f"{name} = getelementptr inbounds {STATE}, {STATE} addrspace(1)* {base}, i64 0, i32 {field}{tail}"


class IrAnalysis(unittest.TestCase):
    def test_identity_scalar_copy_is_neither_consumed_nor_written(self):
        e = fx.analyze_ir(kernel(gep("%a", "%0", 0, 0), "%v = load float, float addrspace(1)* %a, align 4",
                                 gep("%b", "%1", 0, 0), "store float %v, float addrspace(1)* %b, align 4"))
        self.assertEqual((e["consumed"], e["written"], e["opaque"]), ([], [], False))

    def test_used_load_is_consumed_and_new_value_written(self):
        e = fx.analyze_ir(kernel(gep("%a", "%0", 6), "%r = load float, float addrspace(1)* %a, align 4",
                                 "%s = fadd contract float %r, 1.0", gep("%b", "%1", 0, 0),
                                 "store float %s, float addrspace(1)* %b, align 4"))
        self.assertEqual(e["consumed"], ["rout"])
        self.assertEqual(e["written"], ["xyz[0]"])

    def test_identity_store_plus_other_use_still_consumes(self):
        e = fx.analyze_ir(kernel(gep("%a", "%0", 11), "%v = load float, float addrspace(1)* %a, align 4",
                                 "%f = fcmp ord float %v, 0.0", gep("%b", "%1", 11),
                                 "store float %v, float addrspace(1)* %b, align 4",
                                 "%z = zext i1 %f to i32", "store i32 %z, i32 addrspace(1)* %7, align 4"))
        self.assertEqual((e["consumed"], e["written"]), (["vary_scale"], []))

    def test_lane_rescaled_in_place_is_self_contained(self):
        # _Transform's derivative[0] *= Scale, kept or replaced through a phi.
        e = fx.analyze_ir(kernel(gep("%a", "%0", 4, 0), "%v = load float, float addrspace(1)* %a, align 4",
                                 "%s = fmul contract float %v, 2.0", "%f = tail call fast float @air.fabs.f32(float %s)",
                                 "%p = phi float [ %f, %x ], [ %v, %y ]", "%q = select i1 %c, float %p, float 1.0",
                                 gep("%b", "%1", 4, 0), "store float %q, float addrspace(1)* %b, align 4"))
        self.assertEqual((e["consumed"], e["written"], e["self_only"]),
                         (["derivative[0]"], ["derivative[0]"], ["derivative[0]"]))

    def test_value_reaching_another_lane_or_a_decision_is_not_self_contained(self):
        other = fx.analyze_ir(kernel(gep("%a", "%0", 4, 0), "%v = load float, float addrspace(1)* %a, align 4",
                                     "%s = fmul float %v, 2.0", gep("%b", "%1", 4, 0),
                                     "store float %s, float addrspace(1)* %b, align 4", gep("%c", "%1", 0, 0),
                                     "store float %s, float addrspace(1)* %c, align 4"))
        compare = fx.analyze_ir(kernel(gep("%a", "%0", 4, 0), "%v = load float, float addrspace(1)* %a, align 4",
                                       "%s = fmul float %v, 2.0", "%k = fcmp olt float %s, 1.0",
                                       "%m = select i1 %k, float %s, float 0.0", gep("%b", "%1", 4, 0),
                                       "store float %m, float addrspace(1)* %b, align 4"))
        call = fx.analyze_ir(kernel(gep("%a", "%0", 4, 0), "%v = load float, float addrspace(1)* %a, align 4",
                                    "%s = tail call float @air.sqrt.f32(float %v)", gep("%b", "%1", 4, 0),
                                    "store float %s, float addrspace(1)* %b, align 4"))
        bits = fx.analyze_ir(kernel(gep("%a", "%0", 4, 0), "%v = load float, float addrspace(1)* %a, align 4",
                                    "%i = bitcast float %v to i32", "%f = bitcast i32 %i to float", gep("%b", "%1", 4, 0),
                                    "store float %f, float addrspace(1)* %b, align 4"))
        unstored = fx.analyze_ir(kernel(gep("%a", "%0", 4, 0), "%v = load float, float addrspace(1)* %a, align 4",
                                        "%s = fmul float %v, 2.0"))
        for e in (other, compare, call, bits):
            self.assertIn("derivative[0]", e["consumed"])
            self.assertEqual(e["self_only"], [])
        self.assertEqual((unstored["consumed"], unstored["self_only"]), (["derivative[0]"], []))

    def test_wide_or_parked_reads_are_not_self_contained(self):
        wide = fx.analyze_ir(kernel(gep("%a", "%0", 4, 0), "%p = bitcast float addrspace(1)* %a to i64 addrspace(1)*",
                                    "%v = load i64, i64 addrspace(1)* %p, align 4", "%s = add i64 %v, 1",
                                    gep("%b", "%1", 4, 0), "%q = bitcast float addrspace(1)* %b to i64 addrspace(1)*",
                                    "store i64 %s, i64 addrspace(1)* %q, align 4"))
        parked = fx.analyze_ir(kernel("%s = alloca float, align 4", gep("%a", "%0", 4, 0),
                                      "%v = load float, float addrspace(1)* %a, align 4", "%m = fmul float %v, 2.0",
                                      "store float %m, float* %s, align 4", "%w = load float, float* %s, align 4",
                                      gep("%b", "%1", 4, 0), "store float %w, float addrspace(1)* %b, align 4"))
        self.assertEqual(wide["self_only"], [])
        self.assertIn("derivative[0]", parked["consumed"])
        self.assertEqual(parked["self_only"], [])

    def test_wide_load_forwarded_through_private_alloca(self):
        # {dfree2, smooth} as one i64, parked in a private slot and copied out unchanged.
        e = fx.analyze_ir(kernel("%s = alloca i64, align 8", "%c = bitcast i64* %s to i8*",
                                 "call void @llvm.lifetime.start.p0i8(i64 8, i8* nonnull %c)",
                                 gep("%a", "%0", 13), "%p = bitcast float addrspace(1)* %a to i64 addrspace(1)*",
                                 "%v = load i64, i64 addrspace(1)* %p, align 4", "store i64 %v, i64* %s, align 8",
                                 gep("%b", "%1", 13), "%q = bitcast float addrspace(1)* %b to i64 addrspace(1)*",
                                 "%w = load i64, i64* %s, align 8", "store i64 %w, i64 addrspace(1)* %q, align 4"))
        self.assertEqual((e["consumed"], e["written"], e["imprecise"]), ([], [], []))

    def test_forwarded_value_used_elsewhere_is_consumed(self):
        e = fx.analyze_ir(kernel("%s = alloca float, align 4", gep("%a", "%0", 12),
                                 "%v = load float, float addrspace(1)* %a, align 4", "store float %v, float* %s, align 4",
                                 "%w = load float, float* %s, align 4", "%x = fmul float %w, 2.0",
                                 gep("%b", "%1", 0, 0), "store float %x, float addrspace(1)* %b, align 4"))
        self.assertEqual((e["consumed"], e["written"]), (["dfree1"], ["xyz[0]"]))

    def test_memcpy_region_round_trip_is_identity(self):
        e = fx.analyze_ir(kernel("%s = alloca [4 x float], align 4", "%c = bitcast [4 x float]* %s to i8*",
                                 gep("%a", "%0", 5), "%ac = bitcast [4 x float] addrspace(1)* %a to i8 addrspace(1)*",
                                 "call void @llvm.memcpy.p0i8.p1i8.i64(i8* %c, i8 addrspace(1)* %ac, i64 16, i1 false)",
                                 gep("%b", "%1", 5), "%bc = bitcast [4 x float] addrspace(1)* %b to i8 addrspace(1)*",
                                 "call void @llvm.memcpy.p1i8.p0i8.i64(i8 addrspace(1)* %bc, i8* %c, i64 16, i1 false)"))
        self.assertEqual((e["consumed"], e["written"], e["imprecise"]), ([], [], []))

    def direct_copy(self, out_field):
        return kernel("%ac = bitcast {0} addrspace(1)* %0 to i8 addrspace(1)*".format(STATE), gep("%b", "%1", out_field),
                      "%bc = bitcast float addrspace(1)* %b to i8 addrspace(1)*",
                      "call void @llvm.memmove.p1i8.p1i8.i64(i8 addrspace(1)* %bc, i8 addrspace(1)* %ac, i64 16, i1 false)")

    def test_direct_state_memmove_at_same_offset_is_identity(self):
        e = fx.analyze_ir(self.direct_copy(0))
        self.assertEqual((e["consumed"], e["written"], e["opaque"]), ([], [], False))

    def test_direct_state_memmove_to_other_offset_consumes_and_writes(self):
        e = fx.analyze_ir(self.direct_copy(5))
        self.assertEqual((e["consumed"], e["written"], e["opaque"]),
                         (["xyz[0]", "xyz[1]", "xyz[2]", "xyz[3]"], ["sse[0]", "sse[1]", "sse[2]", "sse[3]"], False))

    def test_lane_also_copied_to_another_lane_is_not_self_contained(self):
        # xyz[0] is rescaled in place but also moved into sse[0] by a state copy.
        e = fx.analyze_ir(kernel(*self.direct_copy(5).splitlines()[1:-2],
                                 gep("%a", "%0", 0, 0), "%v = load float, float addrspace(1)* %a, align 4",
                                 "%s = fmul float %v, 2.0", gep("%x", "%1", 0, 0),
                                 "store float %s, float addrspace(1)* %x, align 4"))
        self.assertIn("xyz[0]", e["consumed"])
        self.assertEqual(e["self_only"], [])

    def region(self, *lines):
        return kernel("%s = alloca [4 x float], align 4", "%c = bitcast [4 x float]* %s to i8*",
                      gep("%a", "%0", 5), "%ac = bitcast [4 x float] addrspace(1)* %a to i8 addrspace(1)*",
                      "call void @llvm.memcpy.p0i8.p1i8.i64(i8* %c, i8 addrspace(1)* %ac, i64 16, i1 false)", *lines)

    def test_interior_load_of_copied_region_consumes_only_that_lane(self):
        e = fx.analyze_ir(self.region("%i = getelementptr inbounds [4 x float], [4 x float]* %s, i64 0, i64 2",
                                      "%v = load float, float* %i, align 4",
                                      gep("%b", "%1", 0, 0), "store float %v, float addrspace(1)* %b, align 4"))
        self.assertEqual((e["consumed"], e["written"], e["imprecise"]), (["sse[2]"], ["xyz[0]"], []))

    def test_partial_overwrite_before_copy_out_writes_only_that_lane(self):
        e = fx.analyze_ir(self.region("%i = getelementptr inbounds [4 x float], [4 x float]* %s, i64 0, i64 1",
                                      "store float 2.0, float* %i, align 4",
                                      gep("%b", "%1", 5), "%bc = bitcast [4 x float] addrspace(1)* %b to i8 addrspace(1)*",
                                      "call void @llvm.memcpy.p1i8.p0i8.i64(i8 addrspace(1)* %bc, i8* %c, i64 16, i1 false)"))
        self.assertEqual((e["consumed"], e["written"], e["imprecise"]), ([], ["sse[1]"], []))

    def test_store_after_copy_replaces_the_copied_lane(self):
        e = fx.analyze_ir(self.region("%i = getelementptr inbounds [4 x float], [4 x float]* %s, i64 0, i64 3",
                                      "store float 2.0, float* %i, align 4", "%v = load float, float* %i, align 4",
                                      gep("%b", "%1", 0, 1), "store float %v, float addrspace(1)* %b, align 4"))
        self.assertEqual((e["consumed"], e["imprecise"], e["written"]), ([], [], ["xyz[1]"]))

    def test_load_of_lane_with_copy_and_conditional_store_is_imprecise(self):
        e = fx.analyze_ir(self.region("%i = getelementptr inbounds [4 x float], [4 x float]* %s, i64 0, i64 3",
                                      "br i1 %7, label %set, label %join", "set:", "store float 2.0, float* %i, align 4",
                                      "br label %join", "join:", "%v = load float, float* %i, align 4",
                                      gep("%b", "%1", 0, 1), "store float %v, float addrspace(1)* %b, align 4"))
        self.assertEqual((e["consumed"], e["imprecise"], e["written"]), (["sse[3]"], ["sse[3]"], ["xyz[1]"]))

    def test_store_in_a_loop_reaches_the_loop_header_load(self):
        # The second iteration's load sees the store from the first.
        e = fx.analyze_ir(self.region("%i = getelementptr inbounds [4 x float], [4 x float]* %s, i64 0, i64 3",
                                      "br label %head", "head:", "%v = load float, float* %i, align 4",
                                      "store float 2.0, float* %i, align 4", "br i1 %7, label %head, label %done",
                                      "done:", gep("%b", "%1", 0, 1), "store float %v, float addrspace(1)* %b, align 4"))
        self.assertEqual((e["consumed"], e["imprecise"]), (["sse[3]"], ["sse[3]"]))

    def test_escaping_private_copy_is_imprecise(self):
        e = fx.analyze_ir(self.region("call void @unknown(i8* %c)"))
        self.assertEqual((e["consumed"], e["imprecise"]), (["sse[0]", "sse[1]", "sse[2]", "sse[3]"],) * 2)

    def test_state_pointer_escaping_to_a_call_is_opaque(self):
        e = fx.analyze_ir(kernel(gep("%a", "%0", 4, 1), "call void @unknown(float addrspace(1)* %a)"))
        self.assertTrue(e["opaque"])

    def param_loop(self, *lines, exit_true=True, park_first=False):
        # The wrapper's FormulaParams copy: memset, a counted loop of slot
        # parameters into a[0..3], and a state lane parked at a[5].
        branch = ("br i1 %done, label %after, label %loop" if exit_true else
                  "br i1 %done, label %loop, label %after")
        park = (gep("%a", "%0", 2, 3), "%j = load float, float addrspace(1)* %a, align 4",
                "%p5 = getelementptr inbounds [8 x float], [8 x float]* %s, i64 0, i64 5",
                "store float %j, float* %p5, align 4")
        return kernel("%s = alloca [8 x float], align 4", "%c = bitcast [8 x float]* %s to i8*",
                      "call void @llvm.memset.p0i8.i64(i8* %c, i8 0, i64 32, i1 false)",
                      *(park if park_first else ()), "br label %loop",
                      "loop:", "%i = phi i32 [ 0, %entry ], [ %n, %loop ]", "%k = zext i32 %i to i64",
                      "%pv = getelementptr inbounds float, float addrspace(2)* %7, i64 %k",
                      "%v = load float, float addrspace(2)* %pv, align 4",
                      "%d = getelementptr inbounds [8 x float], [8 x float]* %s, i64 0, i64 %k",
                      "store float %v, float* %d, align 4", "%n = add nuw nsw i32 %i, 1",
                      "%done = icmp eq i32 %n, 4", branch, "after:", *(() if park_first else park), *lines)

    def use(self, index):
        return (f"%q = getelementptr inbounds [8 x float], [8 x float]* %s, i64 0, i64 {index}",
                "%w = load float, float* %q, align 4", "%x = fmul float %w, 2.0",
                gep("%b", "%1", 0, 0), "store float %x, float addrspace(1)* %b, align 4")

    def test_counted_parameter_loop_and_memset_keep_parked_lanes_precise(self):
        e = fx.analyze_ir(self.param_loop(*self.use(1)))
        self.assertEqual((e["consumed"], e["written"], e["imprecise"], e["opaque"]), ([], ["xyz[0]"], [], False))

    def test_parked_lane_stored_after_memset_is_its_state_value(self):
        e = fx.analyze_ir(self.param_loop(*self.use(5)))
        self.assertEqual((e["consumed"], e["imprecise"]), (["j[3]"], []))

    def test_counted_store_range_covering_a_parked_lane_is_imprecise(self):
        e = fx.analyze_ir(self.param_loop(*self.use(5), park_first=True).replace("icmp eq i32 %n, 4", "icmp eq i32 %n, 6"))
        self.assertEqual((e["consumed"], e["imprecise"]), (["j[3]"], ["j[3]"]))
        below = fx.analyze_ir(self.param_loop(*self.use(5), park_first=True))
        self.assertEqual((below["consumed"], below["imprecise"]), (["j[3]"], []))

    def test_loop_without_a_proven_exit_is_imprecise(self):
        e = fx.analyze_ir(self.param_loop(*self.use(1), exit_true=False))
        self.assertEqual((e["consumed"], e["imprecise"]), (["j[3]"], ["j[3]"]))

    def test_load_at_one_of_two_constant_indices_is_bounded(self):
        def choose(a, b):
            return (f"%k2 = select i1 %7, i64 {a}, i64 {b}",
                    "%r = getelementptr inbounds [8 x float], [8 x float]* %s, i64 0, i64 %k2",
                    "%w = load float, float* %r, align 4", gep("%b", "%1", 0, 0),
                    "store float %w, float addrspace(1)* %b, align 4")
        apart = fx.analyze_ir(self.param_loop(*choose(2, 3)))
        self.assertEqual((apart["consumed"], apart["imprecise"]), ([], []))
        covering = fx.analyze_ir(self.param_loop(*choose(6, 4)))
        self.assertEqual((covering["consumed"], covering["imprecise"]), (["j[3]"], ["j[3]"]))

    def test_counted_load_range_consumes_only_parked_lanes_inside(self):
        dynamic = ("%r = getelementptr inbounds [8 x float], [8 x float]* %s, i64 0, i64 %k",
                   "%w = load float, float* %r, align 4", gep("%b", "%1", 0, 0),
                   "store float %w, float addrspace(1)* %b, align 4")
        inside = fx.analyze_ir(self.param_loop(*dynamic).replace("icmp eq i32 %n, 4", "icmp eq i32 %n, 6"))
        self.assertEqual((inside["consumed"], inside["imprecise"]), (["j[3]"], ["j[3]"]))
        outside = fx.analyze_ir(self.param_loop(*dynamic))
        self.assertEqual((outside["consumed"], outside["written"], outside["imprecise"]), ([], ["xyz[0]"], []))

    def test_dynamic_state_index_is_opaque(self):
        e = fx.analyze_ir(kernel(f"%a = getelementptr inbounds {STATE}, {STATE} addrspace(1)* %0, i64 0, i32 19, i64 %k"))
        self.assertTrue(e["opaque"])


class Classification(unittest.TestCase):
    def effects(self, consumed, written=(), imprecise=()):
        return {"status": "ANALYZED", "opaque": False, "consumed": list(consumed), "written": list(written),
                "imprecise": list(imprecise)}

    def test_driver_tables_follow_orbit_entry(self):
        d = fx.DRIVERS
        self.assertNotIn("vary_scale", d[("Hybrid3DAnalytic", False)]["init"])
        self.assertNotIn("vary_scale", d[("Hybrid4DAnalytic", False)]["init"])
        self.assertNotIn("xyz[3]", d[("Hybrid3DAnalytic/xyz-derivative", False)]["init"])
        self.assertNotIn("xyz[3]", d[("Hybrid3DAnalytic/xyz-derivative", False)]["writes"])
        self.assertNotIn("j[3]", d[("Hybrid3DAnalytic", False)]["init"])
        self.assertNotIn("j[3]", d[("Hybrid3DAnalytic", True)]["init"])
        self.assertNotIn("j[3]", d[("HybridDifs", True)]["init"])
        self.assertNotIn("xyz[0]", d[("HybridDifsNoInitialization", False)]["init"])
        self.assertIn("sse[0]", d[("Hybrid3D", False)]["init"])
        self.assertNotIn("sse[0]", d[("Hybrid3DAnalytic", False)]["init"])

    def test_classes(self):
        base = ["xyz[0]", "xyz[1]", "xyz[2]", "xyz[3]", "j[0]", "rout", "vary_scale"]
        self.assertEqual(fx.classify(self.effects(base, ["xyz[0]"]), "Hybrid3DAnalytic")["class"], "B")
        # vary_scale survives every ordinary orbit: invariant unless written.
        self.assertEqual(fx.classify(self.effects(base, ["xyz[0]"]), "Hybrid4DAnalytic")["class"], "B")
        self.assertEqual(fx.classify(self.effects(base, ["vary_scale"]), "Hybrid4DAnalytic")["class"], "C")
        self.assertEqual(fx.classify(self.effects(base + ["sse[0]"], ["sse[0]"]), "Hybrid3DAnalytic")["class"], "C")
        self.assertEqual(fx.classify(self.effects(base, ["hybrid_counts[0]"]), "Hybrid3DAnalytic")["class"], "C")
        self.assertEqual(fx.classify(self.effects(base + ["sse[0]"], ["sse[0]"], ["sse[0]"]), "Hybrid3DAnalytic"),
                         {"class": "U", "imprecise": ["sse[0]"], "bound": "C"})
        # An imprecise lane nobody writes can at worst be an invariant B read.
        self.assertEqual(fx.classify(self.effects(base + ["j[3]"], ["xyz[0]"], ["j[3]"]), "Hybrid3D"),
                         {"class": "U", "imprecise": ["j[3]"], "bound": "B"})
        self.assertEqual(fx.classify({"status": "COMPILE_FAILED"}, "Hybrid3D")["class"], "U")
        # dIFS drivers return carried tmp/dfree2 when no iteration improves the minimum.
        difs = fx.classify(self.effects(["xyz[0]", "xyz[1]", "xyz[2]", "rout", "vary_scale"], ["rout"]), "HybridDifs")
        self.assertEqual((difs["class"], difs["formula_class"], difs["driver_carried"]), ("C", "A", ["dfree2", "tmp"]))
        self.assertEqual(fx.classify(self.effects(base, ["xyz[0]"]), "HybridDifsNoInitialization")["driver_carried"],
                         ["dfree2"])

    def test_ordinary_vary_guard_is_live_only_without_a_writer(self):
        # Even an otherwise stateless body is B: native_step consumes Vary.
        # First=0 does not prove that a conditional body write precedes every
        # consumer; this analysis has no path-sensitive must-write evidence.
        for (driver, julia), variant in fx.DRIVERS.items():
            if driver.startswith("HybridDifs"):
                self.assertIn("vary_scale", variant["init"])
                self.assertIn("vary_scale", variant["writes"])
                continue
            with self.subTest(driver=driver, julia=julia):
                self.assertNotIn("vary_scale", variant["init"])
                self.assertNotIn("vary_scale", variant["writes"])
                live = fx.classify(self.effects([], ["xyz[0]"]), driver, julia)
                self.assertEqual(live["class"], "B")
                self.assertIn("vary_scale", live["invariant_unless_written"])
                carry = fx.classify(self.effects(["first"], ["first", "vary_scale"]), driver, julia)
                self.assertEqual(carry["class"], "C")
                self.assertIn("vary_scale", carry["carried"])

    def test_j4_is_retained_in_3d_for_both_julia_modes(self):
        for driver in ("Hybrid3D", "Hybrid3DAnalytic", "Interpolated3D", "Interpolated3DAnalytic"):
            for julia in (False, True):
                with self.subTest(driver=driver, julia=julia):
                    variant = fx.DRIVERS[(driver, julia)]
                    self.assertNotIn("j[3]", variant["init"] | variant["writes"])
                    live = fx.classify(self.effects(["j[3]"]), driver, julia)
                    self.assertEqual(live["class"], "B")
                    self.assertIn("j[3]", live["invariant_unless_written"])
                    carry = fx.classify(self.effects(["j[3]"], ["j[3]"]), driver, julia)
                    self.assertEqual(carry["class"], "C")
                    self.assertIn("j[3]", carry["carried"])
        for driver in ("Hybrid4D", "Hybrid4DAnalytic", "Interpolated4D", "Interpolated4DAnalytic"):
            for julia in (False, True):
                result = fx.classify(self.effects(["j[3]"], ["j[3]"]), driver, julia)
                self.assertEqual(result["class"], "B")  # only immutable Vary remains
                self.assertNotIn("j[3]", result["invariant_unless_written"])

    def test_interpolation_tmp_is_live_only_without_a_writer(self):
        for (driver, julia), variant in fx.DRIVERS.items():
            if not driver.startswith("Interpolated"):
                continue
            with self.subTest(driver=driver, julia=julia):
                self.assertNotIn("tmp", variant["init"] | variant["writes"])
                live = fx.classify(self.effects(["tmp"]), driver, julia)
                self.assertEqual(live["class"], "B")
                self.assertIn("tmp", live["invariant_unless_written"])
                carry = fx.classify(self.effects(["tmp"], ["tmp"]), driver, julia)
                self.assertEqual(carry["class"], "C")
                self.assertIn("tmp", carry["carried"])
        ordinary = fx.classify(self.effects(["tmp"], ["tmp"]), "Hybrid3D")
        self.assertEqual(ordinary["class"], "B")
        self.assertNotIn("tmp", ordinary["invariant_unless_written"])

    def test_return_sse_may_write_does_not_prove_entry_initialization(self):
        for (driver, julia), variant in fx.DRIVERS.items():
            if driver.startswith("HybridDifs"):
                self.assertFalse(fx.lanes("sse") & (variant["init"] | variant["writes"]))
                continue
            with self.subTest(driver=driver, julia=julia):
                self.assertTrue(fx.lanes("sse") <= variant["writes"])
                # No body SSE write: the preceding query's DRIVER return is
                # already a writer and prevents claiming immutable live input.
                result = fx.classify(self.effects(["sse[2]"]), driver, julia)
                if driver == "Hybrid3D" or driver.startswith("Interpolated"):
                    self.assertEqual(result["class"], "B")
                    self.assertNotIn("sse[2]", result["invariant_unless_written"])
                else:
                    self.assertNotIn("sse[2]", variant["init"])
                    self.assertEqual(result["class"], "C")
                    self.assertIn("sse[2]", result["carried"])



class ShadowContracts(unittest.TestCase):
    def test_shadow_preserves_result_contract_and_wrapper_exactly(self):
        with tempfile.TemporaryDirectory() as temp:
            work = Path(temp)
            fx.prepare_shadow(fx.ROOT, work)
            for name in ('formula_wrapper.metal.h', 'generated/formula_result_contracts.metal.h'):
                self.assertEqual((work / 'shadow' / name).read_bytes(),
                                 (fx.ROOT / fx.NATIVE / name).read_bytes())
            self.assertIn('#include "generated/formula_result_contracts.metal.h"',
                          (work / 'shadow/formula_wrapper.metal.h').read_text())


class MathAnalysisIdentity(unittest.TestCase):
    """Synthetic compiler controls prove provenance rejection, not formulas."""
    def run_fixture(self, root, output, change_math=False, keep=False,
                    change_translate=False, missing_translate=False):
        body = fx.NATIVE/'generated/formula_7010.metal.h'
        names = ('formula_math.metal.h', 'translate_add.metal.h', 'formula_wrapper.metal.h',
                 'generated/formula_result_contracts.metal.h', 'generated/formula_metadata.h',
                 'generated/inventory.json', 'orbit_driver.metal.h', 'generated/formula_7010.metal.h')
        for name in names:
            path = root/fx.NATIVE/name
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text('synthetic fixture: '+name)
        math_path = root/fx.NATIVE/'formula_math.metal.h'
        before = hashlib.sha256(math_path.read_bytes()).hexdigest()
        translate_path = root/fx.NATIVE/'translate_add.metal.h'
        translate_before = hashlib.sha256(translate_path.read_bytes()).hexdigest()
        if missing_translate:
            translate_path.unlink()
        entry = {'id': 7010, 'name': 'synthetic-only', 'generated': str(body)}
        def fake_compile(checkout, work, selected, preserve):
            self.assertEqual(checkout, root)
            self.assertEqual(selected, entry)
            self.assertEqual(preserve, keep)
            self.assertEqual(hashlib.sha256(math_path.read_bytes()).hexdigest(), before)
            self.assertEqual(hashlib.sha256(translate_path.read_bytes()).hexdigest(), translate_before)
            if change_math:
                math_path.write_text('synthetic math changed during compile')
            if change_translate:
                translate_path.write_text('synthetic translate add changed during compile')
            return {'status': 'ANALYZED', 'opaque': False, 'consumed': [], 'written': [],
                    'imprecise': [], 'self_only': []}
        with patch.object(fx, 'load_entries', return_value=[entry]), \
             patch.object(fx, 'internal_entries', return_value=[]), \
             patch.object(fx, 'prepare_shadow'), \
             patch.object(fx, 'compile_entry', side_effect=fake_compile) as compile_call, \
             patch.object(fx, 'toolchain', return_value='synthetic-only; no compiler'), \
             patch.object(fx.subprocess, 'run', side_effect=AssertionError('real compiler forbidden in synthetic fixture')):
            try:
                result = fx.run(root, output, jobs=1, ids=None, keep=keep)
            finally:
                self.assertEqual(compile_call.call_count, 0 if missing_translate else 1)
        return result, before

    def test_run_records_both_math_identities_used_by_the_compile(self):
        with tempfile.TemporaryDirectory() as directory:
            root, output = Path(directory)/'checkout', Path(directory)/'result'
            result, before = self.run_fixture(root, output)
            self.assertEqual(result['math_sha256'], before)
            stored = json.loads((output/'effects.json').read_text())
            self.assertEqual(stored['math_sha256'], before)
            translate_before = hashlib.sha256((root/fx.NATIVE/'translate_add.metal.h').read_bytes()).hexdigest()
            self.assertEqual(result['translate_add_sha256'], translate_before)
            self.assertEqual(stored['translate_add_sha256'], translate_before)
            self.assertEqual(stored['status'], 'STATIC_EFFECTS_UNQUALIFIED')
            self.assertIn('no compiler', stored['toolchain'])

    def test_math_mutation_during_compile_cannot_publish_effects(self):
        for keep in (False, True):
            with self.subTest(keep=keep), tempfile.TemporaryDirectory() as directory:
                root, output = Path(directory)/'checkout', Path(directory)/'result'
                with self.assertRaisesRegex(RuntimeError, 'math changed'):
                    self.run_fixture(root, output, change_math=True, keep=keep)
                self.assertFalse((output/'effects.json').exists())

    def test_translate_add_mutation_during_compile_cannot_publish_effects(self):
        for keep in (False, True):
            with self.subTest(keep=keep), tempfile.TemporaryDirectory() as directory:
                root, output = Path(directory)/'checkout', Path(directory)/'result'
                with self.assertRaisesRegex(RuntimeError, 'Translate add helper changed'):
                    self.run_fixture(root, output, change_translate=True, keep=keep)
                self.assertFalse((output/'effects.json').exists())

    def test_missing_translate_add_refuses_before_compile_or_output_creation(self):
        with tempfile.TemporaryDirectory() as directory:
            root, output = Path(directory)/'checkout', Path(directory)/'result'
            with self.assertRaises(FileNotFoundError):
                self.run_fixture(root, output, missing_translate=True)
            self.assertFalse(output.exists())


@unittest.skipUnless(shutil.which("xcrun"), "Metal toolchain unavailable")
class CompiledFormula(unittest.TestCase):
    def test_mandelview_adds_julia_inside_its_body(self):
        entry = next(e for e in fx.load_entries(fx.ROOT) if e["id"] == 7098)
        with tempfile.TemporaryDirectory() as directory:
            work = Path(directory)
            fx.prepare_shadow(fx.ROOT, work)
            effects = fx.compile_entry(fx.ROOT, work, entry, keep=False)
        self.assertEqual(effects["status"], "ANALYZED", effects)
        self.assertFalse(effects["opaque"])
        self.assertTrue({"j[0]", "j[1]", "j[2]"} <= set(effects["consumed"]))
        self.assertFalse({"sse[0]", "derivative[1]", "dfree2", "smooth"} & set(effects["consumed"]))
        self.assertEqual(fx.classify(effects, "Hybrid3D")["class"], "B")

    def test_benesi1pow2_reads_julia_through_its_empty_parameter_map(self):
        # ExternalFormulaInfo.paramCount is 3, but the CPU map and the uploaded slot have 0.
        entry = next(e for e in fx.load_entries(fx.ROOT) if e["id"] == 7083)
        self.assertEqual((entry["param_count"], entry["declared_param_count"]), (0, 3))
        with tempfile.TemporaryDirectory() as directory:
            work = Path(directory)
            fx.prepare_shadow(fx.ROOT, work)
            effects = fx.compile_entry(fx.ROOT, work, entry, keep=False)
        self.assertTrue({"j[0]", "j[1]", "j[2]"} <= set(effects["consumed"]), effects["consumed"])

    def test_work_budget_loop_body_survives(self):
        # _e_3Dtess loops under FormulaWorkBudget; a folded exhaustion path left only a state copy.
        entry = next(e for e in fx.load_entries(fx.ROOT) if e["id"] == 10274)
        with tempfile.TemporaryDirectory() as directory:
            work = Path(directory)
            fx.prepare_shadow(fx.ROOT, work)
            effects = fx.compile_entry(fx.ROOT, work, entry, keep=False)
        self.assertEqual(effects["status"], "ANALYZED", effects)
        self.assertFalse(effects["opaque"], effects.get("opaque_instructions"))
        self.assertTrue({"xyz[0]", "xyz[1]", "xyz[2]"} <= set(effects["written"]))

    def test_internal_amazing_box_matches_its_audited_effects(self):
        # pixel_state_contract.h audits internal 4 as reading xyz/w/j.xyz and writing xyz/w.
        entry = next(e for e in fx.internal_entries(fx.ROOT) if e["id"] == 4)
        self.assertEqual(entry["name"], "Amazing Box")
        with tempfile.TemporaryDirectory() as directory:
            work = Path(directory)
            fx.prepare_shadow(fx.ROOT, work)
            effects = fx.compile_entry(fx.ROOT, work, entry, keep=False)
        self.assertEqual((effects["status"], effects["entry"], effects["opaque"]),
                         ("ANALYZED", "native_internal_formula_step", False))
        self.assertTrue(set(effects["consumed"]) <= {"xyz[0]", "xyz[1]", "xyz[2]", "xyz[3]", "j[0]", "j[1]", "j[2]"},
                        effects["consumed"])
        self.assertTrue({"xyz[0]", "xyz[1]", "xyz[2]"} <= set(effects["written"]) <= {"xyz[0]", "xyz[1]", "xyz[2]", "xyz[3]"})

    def test_benesi_t_bodies_keep_two_host_parameters(self):
        entries = {e["id"]: e for e in fx.load_entries(fx.ROOT)}
        self.assertEqual([entries[i]["param_count"] for i in (95001, 95002, 95003)], [2, 2, 2])


if __name__ == "__main__":
    unittest.main()
