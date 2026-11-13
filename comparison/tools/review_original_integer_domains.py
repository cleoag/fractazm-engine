"""Apply explicitly reviewed original-x86 INTEGER semantics to the source audit.

This reports input obligations, never parity PASS. Marginal finite partitions do
not imply arbitrary parameter interactions have been verified. Every review is
bound to the immutable original CODE and disassembly hashes in the input audit.
"""
import argparse
from collections import Counter
import hashlib
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
MIN = -(1 << 31)
MAX = (1 << 31)-1


def general_quat(value):
    decremented = (value-1) & 0xffffffff
    negative = bool(decremented & 0x80000000)
    slot = ((-decremented-2) if negative else decremented) & 3
    return negative, slot


def rotation_type(value, sequential=False):
    flipped = value >= 6
    value -= 6 if flipped else 0
    # JNS following SUB observes the wrapped result, not signed less-than.
    if value == MIN:
        return flipped, 5 if sequential else 2
    return flipped, min(5, max(0, value))


def rotation_power(value):
    return 2 if value <= 2 else value if value < 10 else 10 + (value & 1)


CALL_REVIEWS = {
    '_Fold45double': ('0xdd..end', 'EDI stays PVar; callee reads the same three zero/nonzero sign words and floating constant PVar+0. No indexed selector access or integer transformation.'),
    '_Fold45single': ('0x94..0xf0', 'EDI stays PVar; callee reads the same three zero/nonzero sign words -0x18/-0x1c/-0x20 and floating constant PVar+0. Five outer flags only compare zero.'),
    '_FoldingTetraKaleid': ('0xc2..end', 'Callee performs FPU coordinate folding and uses floating constant PVar+0; EDI stays PVar. Selector words only compare zero outside the callee.'),
    '_IntPowRotateC': ('0xb2..0x2d3', 'EDI stays PVar; helper consumes explicitly passed EBX power and EDX type, plus floating constant PVar+0x10. Enable words are read only by caller CMP0.'),
    'GenIntPower': ('0xe2..0x303', 'EDI stays PVar; helper consumes explicitly passed EBX power and EDX type, plus floating constant PVar+0x10. ESI scratch is restored by caller; enable words only CMP0.'),
    'MengerHyper': ('0x6f..end', 'ESI stays PVar; callee only uses FPU coordinates and floating constants -0x50/-0x58. Mode word only reaches the outer direct comparison.'),
    'ATetraVS': ('0x21..0xc9', 'EDI stays PVar. Shared fold helper uses stack Single matrix, floating fold limit -0x28 and CMP dword[-0x5c],1. Outer direction -0x58 compares0/1/2; no other integer selector use.'),
    'TetraFolding Pow2': ('0x21..0xc9;0x37c..0x425', 'EDI stays PVar. Both helpers use stack Single matrix and CMP dword[-0x64],1; extra helper uses floating scale -0x6c. Outer direction -0x60 compares0/1/2; no other integer selector use.'),
    'DodecahedronIFSex': ('0x1c..0x164', 'EDI stays PVar across internal call tree. Helpers use floating matrix/constants and coordinates; sole integer -0x7c is compared with0 in helper0xa7 and1..7 in main. No selector-derived pointer or loop.'),
}


def rule(row):
    name, i = row['formula'], row['stored_value_index']
    def finite(signature, reps, text, anchors):
        return dict(kind='reviewed_finite_original_domain', signature=signature,
                    representatives=list(reps), semantics=text, source_addresses=anchors)
    def algorithm(kind, text, boundaries, anchors):
        return dict(kind=kind, semantics=text, boundary_inputs=boundaries, source_addresses=anchors)
    if name == '_GenCWarp' and i in (5,6):
        reviewed = finite(lambda v:v if 0<=v<=23 else -1, list(range(24))+[-1],
            'Each valid selector0..23 loads a defined FPATAN argument pair. Every other int32 falls through without loading operands, consuming the incoming x87 stack or underflowing it. Production strictly rejects this class; its observed NaN output is not a portable numeric definition.',
            ['0x6c..0x390 theta dispatch', '0x3a1..0x6c5 phi dispatch'])
        reviewed['undefined_representatives'] = [-1]
        reviewed['strict_rejection'] = 'uninitialized_angle_stack'
        return reviewed
    if name == 'GeneralQuat':
        return finite(general_quat, [-4,-3,-2,-1,1,2,3,4],
            'DEC int32; JNS chooses term sign; negative path NEG then SUB 2; AND 3 selects destination. Eight terms are accumulated in original order. INT_MIN is positive slot3; zero is negative slot3.',
            ['0x42','0x69','0x6e','0x86'])
    if name in ('TgladDihed','TgladDihed2','TgladTetra','TgladTetra2') and i == 0:
        return finite(lambda v:v&7, range(8), 'Byte load followed by AND7; only three low bits control absolute-value operations before the fold loop. Upper29 bits cannot affect any use.', ['MOV AH,[EDI-0xc]','AND AH,7','SHL EAX,0x10'])
    if name in ('TgladDihed','TgladDihed2','TgladTetra','TgladTetra2') and i == 1:
        return finite(lambda v:v&7, range(8), 'Log2MapPower is a scalar fold iteration count, not a resource map ID. MOV AL then AND7 limits all uses to0..7; saved through SHL/SHR EAX16 around FPU flags.', ['0x8b..0x9b','0xac','0x148'])
    if (name in ('CylinderHeightMap','CylinderHMVoid','SphereHeightMap','SphereHMVoid','TorusHMVoid') and i == 1) or (name in ('HeightMap2IFS','HeightMapIFS') and i == 10):
        return finite(lambda v:v&3, range(4), 'Map channel is a masked scalar index, not a resource ID. AND ECX,3 selects RGB or original fourth scratch value; upper30 bits ignored. Actual map sampling/callback and scratch-channel state remain independent obligations.', ['MOV ECX,[PVar+channel]','AND ECX,3','indexed qword scratch read'])
    if name == 'TalisTpx' and i == 7:
        return finite(lambda v:v if 2<=v<=22 else 8, range(2,23), 'Signed CMP22/JG and CMP1/JG retain2..22; all other int32 inputs replace the count by8. All21 numerical loop counts retained.', ['0x16..0x28','0xb3 loop counter'])
    if name in ('BT1Pine','BT1Pinegon','BT1Pinehedron') and row['classification'] == 'low_byte_algorithmic_input':
        if i == (5 if name == 'BT1Pine' else 8):
            return algorithm('byte_iteration_count', 'Only low byte is loaded. Bottom-tested DEC AL/JNE performs1..255 iterations, or256 when low byte is0; high24 bits are ignored. Verify byte wrap/count boundaries, not256 unrelated algorithms.', [MIN,-1,0,1,2,127,128,255,256,257,MAX], ['0x16','0xae..0xb0'])
        after = i in (7,10)
        return algorithm('signed_byte_state_threshold' if after else 'byte_state_equality',
            'Low parameter byte compared with low execution-counter byte; '+('signed8 JG controls threshold; equality arm of preceding parameter takes priority.' if after else 'JE/JNE controls equality arm; verify equal and unequal states at byte/sign/wrap boundaries.'),
            [MIN,-129,-128,-1,0,1,127,128,255,256,257,MAX], ['0xb8..0xc6' if i in (6,7) else '0xf5..0x103'])
    if name in ('GenIntPower','_IntPowRotateC') and i in (1,4,7):
        return finite(rotation_power, range(2,12),
            'Complex-power helper always squares; signed powers <=2 use square, 3..9 have separate multiply paths, >=10 select 10/11 by low bit. Other bits do not reach arithmetic.',
            ['0x285..0x303'] if name == 'GenIntPower' else ['0x255..0x2d3'])
    if name in ('GenIntPower','_IntPowRotateC') and i in (2,5,8):
        return finite(lambda v: rotation_type(v, name == '_IntPowRotateC'), range(12),
            'Signed >=6 enables swap and subtracts6; six rotation implementations follow. SUB/JNS int32 wrap maps INT_MIN to type2 in GenIntPower and type5 in _IntPowRotateC; remaining negatives map0 and >=11 map11.',
            ['0xe2..0x284'] if name == 'GenIntPower' else ['0xb2..0x254'])
    if name in ('_SinApprox','_PlusSinApp','_SinY'):
        return finite(lambda v: v & 3, range(4), 'AND 3 bounds coordinate index to0..3 before indexed reads/writes.', ['AND ESI,3'])
    if name in ('ABoxPlatinum','ABoxPlatinumB') and i == 14:
        return finite(lambda v: v & 7, range(8), 'Independent TEST EAX,4/2/1 controls XYZ absolute values; upper bits unused.', ['TEST EAX,4','TEST EAX,2','TEST EAX,1'])
    if name == 'Quad3Db' and i == 5:
        return finite(lambda v: v & 63, range(64), 'Three AX low-two-bit selections, with SHR AX,2/4, select four operations per axis. Full64 mask combinations retained to cover coupling.', ['AND AX,3','SHR AX,2','SHR AX,4'])
    if (name == 'HeightMap2IFS' and i == 14) or (name == 'TilingIFS' and i == 7):
        return finite(lambda v: v & 1, range(2), 'AND1 is the only consumed selector bit; subsequent shifts derive a bounded address/coordinate index.', ['AND ECX,1'] if name.startswith('Height') else ['AND EAX,1'])
    if name in ('NewMengerIFS','NewMenger2IFS','NewFoldCutIFS') and i == 10:
        return finite(lambda v: max(1,v & 31), range(1,32), 'AND31; zero replaced by1. Bounded iteration count1..31. All31 counts retained, not equated by branch direction.', ['0x4b..0x56'])
    if name == 'RandCubesIFS' and i in (10,11):
        return finite(lambda v: v if 1 <= v <= 6 else 0, range(7), 'Cell hash bit0x40 selects parameter; CMP1..6 chooses six shapes, zero and all other int32 values share cube fallback. Both hash-selection outcomes remain a state obligation.', ['0x16d..0x186','0x393..0x401'])
    targets = {'_CondItcountR':(2,), '_CondItcountSwitch':(1,), '_CondItcountSwX2':(1,6),
               '_CondLyapunov':(1,2), '_CondLyapunovEx':(1,2)}
    if i in targets.get(name, ()):
        return finite(lambda v: min(6,max(1,v)), range(1,7), 'Signed clamp1..6 selects nHybrid target; existing target disable bit is preserved. Cross-target alias/order and disabled counts require state tests.', ['CMP EDI,1','CMP EDI,6','LEA EDI,[ESI+EDI*4+0x48]'])
    if (name.startswith('IQ_') or name == 'IQ-bulb') and i in (3,4):
        return algorithm('unused_original_declaration', 'Complete original CODE reads neither declared mode word. No numerical selector branch exists for this declaration; C++ ignored-parameter behavior still needs verification.', [], ['complete CODE: no read of PVar-0x24/-0x28'])
    if name == 'BT1Pinehedron' and i == 10:
        return algorithm('unused_original_declaration', 'PolyIterAfter at PVar-0x4c is not read anywhere in the complete493-byte original CODE; only PolyIter low byte at-0x48 is read.', [], ['0xf5..0xfb','complete CODE'])
    if name == '_CondLyapunov' and i in (3,4):
        return algorithm('unused_original_declaration', 'Declared frequency words are never read by original245-byte CODE; target counts are literal0/1.', [], ['complete CODE'])
    if name in ('_CondLyapunov','_CondLyapunovEx') and i == 0:
        return algorithm('bit_sequence_with_state_index', 'Counter increments with int32 wrap then AND15. Shift loop selects bits0..14 (counter residues0 and1 both select bit0); bits15..31 ignored. Basis vectors plus full16-step cycle qualify bit-selection without32768 blind patterns.', [0]+[1<<b for b in range(15)]+[32767,32768,MIN,MAX], ['0xe..0x30'])
    if name in ('_CondItcountSwitch','_CondItcountSwX2') and i in ((0,5) if name.endswith('SwX2') else (0,)):
        return algorithm('clamped_state_threshold', 'Signed clamp1..100 followed by comparison with incremented execution counter. Exercise counter threshold-1/equal/+1 and wrap; threshold is numerical, not100 unrelated algorithms.', [MIN,0,1,2,99,100,101,MAX], ['CMP EAX,1','CMP EAX,0x64','CMP EDX,EAX'])
    if name.startswith('_Cond') and row['classification'] == 'requires_dataflow_review':
        lo = 1 if name == '_CondLyapunovEx' and i == 4 else 0
        hi = 99999 if name == '_CondItcountR' else 100
        return algorithm('clamped_hybrid_count', f'Signed clamp{lo}..{hi}; write int32 count preserving existing target disable bit. Check each radius/counter/sequence arm and same-target ordering, including zero activation.', [MIN,-1,0,1,hi-1,hi,hi+1,MAX], ['signed count clamp','OR EAX,0x80000000','MOV [EDI],EAX'])
    if (name in ('_RotateRnd','_TranslateRnd') and i in (3,4,5)) or (name == 'RandCubesIFS' and i in (8,9)):
        return algorithm('modular_integer_seed', 'Integer participates in wraparound hash/LCG arithmetic; every seed can produce a distinct numerical value. Verify wrap boundaries and coordinate/cell dependence, not nominal selector modes.', [MIN,MIN+1,-1,0,1,MAX-1,MAX], ['original seed MOV/SUB and hash arithmetic'])
    if name in ('gnarlyIFS','gnarly2IFS','gnarly3IFS','ducksIFS') and i == 8:
        return algorithm('defined_even_undefined_odd_caller_stack', 'Only low bit selects FlipXZ. Odd original path reads caller stack instead of defined parameter data; production rejects odd inputs. Even path is defined. Rejection is not numerical parity PASS.', [MIN,-2,0,2,MAX-1], ['SHR EDX,1','odd caller-stack path'])
    if name == '_recFold' and i == 4:
        return algorithm('guarded_iteration_count', 'DEC int32 then skip when <=0 or >999. Inputs2..1000 execute n-1; all other int32 inputs skip, including INT_MIN wrapping toMAX. Count remains numerical.', [MIN,-1,0,1,2,999,1000,1001,MAX], ['DEC EAX','CMP EAX,0x3e7'])
    if name == 'columnsIFS' and i == 8:
        return algorithm('clamped_iteration_count', 'Loop count uses signed bounds and is limited to1000; verify skip/first/last iteration and clamp boundaries using original loop state.', [MIN,-1,0,1,2,999,1000,1001,MAX], ['original count load, signed guards and backedge'])
    if name == 'Knots2IFS' and i == 11:
        return algorithm('bottom_tested_signed_count', 'Body executes once, then DEC EDX/JG. Values<=1 execute once except INT_MIN: wrapped result isMAX but OF=1 means JG is false, so it too executes once. Positive count controls loop length.', [MIN,-1,0,1,2,256,257,513,MAX], ['0x70','0xbf..0xc0'])
    if name == 'SphCageIFS' and i == 7:
        return algorithm('clamped_numeric_count', 'Signed clamp2..1000 affects spacing via integer division and halved loop count; even/odd, min/max and saturation are numerical obligations.', [MIN,0,1,2,3,999,1000,1001,MAX], ['CMP EDX,2','CMP EDX,0x3e8','SHR EDX,1'])
    if name == 'RoomDoorsIFS' and i == 11:
        return algorithm('positive_divisor', 'Period<=0 bypasses modulo; positive period divides a wrapped unsigned cell index and tests remainder1. Period1 never has remainder1. Test divisor and cell-transition boundaries together.', [MIN,-1,0,1,2,3,MAX], ['0x100 coordinate bias','CMP ECX,0','DIV ECX','CMP EDX,1'])
    if name == '_e_3Dtess' and i == 7:
        return algorithm('geometry_limited_signed_depth', 'Signed iteration limit controls repeated geometric folds, with a separate geometric exit. Large positive limits can require excessive work; there is no new selector mode at INT_MAX. Keep work bounding/cancellation as a separate obligation; do not execute huge counts blindly.', [MIN,-1,0,1,2,MAX], ['original signed depth load/compare and geometric exit'])
    if name == '_IFS_tess' and i == 7:
        return algorithm('geometry_limited_unsigned_depth', 'Unsigned iteration limit controls repeated geometric folds, with a separate geometric exit. Negative saved int32 values are large unsigned limits. No recursive calls or additional selector modes exist; qualify short-loop and geometry-exit behavior separately from a production work bound/cancellation.', [MIN,-1,0,1,2,MAX], ['original unsigned depth compare/JA and geometric exit'])
    if name == 'TrifoxComplexAngles' and i in (0,3):
        return algorithm('unsigned_binary_power_zero_nontermination', 'Unsigned SHR binary exponent consumes up to32 bits. Nonzero int32 values terminate; zero has an original nontermination path and must be separately rejected, never counted as parity PASS.', [MIN,-1,1,2,3,4,7,8,1<<30,MAX], ['binary exponent SHR loop'])
    if name in ('ABoxSmoothFold','ASurfSmoothFold'):
        return algorithm('unsigned_binary_integer_power', 'Logical shifts select repeated-square/multiply steps; bounded by32 bits. Zero/one and bit-boundary exponents need numeric/singular inputs; arbitrary exponents are not equivalent merely because branch coverage is present.', [MIN,-1,0,1,2,3,4,7,8,31,32,MAX], ['SHR EAX,1','JAE','FMUL backedge'])
    if name == '_ngon' and i == 5:
        return finite(lambda v:v!=0,[0,1],'TEST EAX,EAX/JNE is the only selector use; subsequent EAX load is a separate coordinate-derived integer.', ['0x5f..0x66'])
    masks={'Amazing Surf 2':(6,15), 'CommaIFS':(11,7), 'koch_surf':(13,3), 'Mandalex':(9,3), 'waveIFS':(8,1)}
    if name in masks and i == masks[name][0]:
        mask=masks[name][1]
        return finite(lambda v:v&mask,range(mask+1),f'Original masked field consumes only bits in mask{mask}; complete mask combinations retained. Floating comparisons/state controlling branch reachability remain separate.', ['original AND immediate mask uses'])
    if name == 'foldingoctIFS' and i in (6,7,8):
        return finite(lambda v:v&3,range(4),'AND1 controls absolute value; AND2 enables conditional swap. Higher bits are ignored.', ['AND EDX,1','AND EDX,2'])
    if row['label'].startswith('OTrap') and name in ('boxIFS','cylinderIFS','RoundedBox','SphereIFS','CylinderHeightMap','CylinderHMVoid','HeightMap2IFS','HeightMapIFS','SphereHeightMap','SphereHMVoid','TorusHMVoid'):
        return finite(lambda v:v&3,range(4),'AND3 selects one of4 packed scalar channels; channel3 must retain its original scratch/state semantics even when label advertises0..2.', ['AND EAX,3 / AND ECX,3','indexed qword read'])
    if name == 'GenIntPower' and i == 0:
        return finite(lambda v: (2+(v&1)) if v<4 else v if v<10 else 10+(v&1),range(2,12), 'Radius multiply tree tests signed4/6/8/10 then always applies low-bit correction. Negative odd values map power3, unlike the complex-rotation helper.', ['0x35..0x66'])
    if name == 'helixIFS' and i == 10:
        return finite(lambda v:((v-1)&15)+1,range(1,17),'DEC int32 then AND15 bounds count1..16, used for phase spacing and loop length.', ['0x73..0x89','0xc7..0xca'])
    if name in ('PG-bulb','hyperd_1') and i == 1:
        return finite(lambda v:v if 1<=v<=16 else 2,range(1,17),'Signed interval1..16 accepted; all other inputs replaced by2. Bottom-tested multiply body means power1 is retained as a separate conservative class.', ['CMP EAX,0x11','CMP EAX,1','MOV EAX,2'])
    if name == 'KaliLinCombSSE2sm' and i == 0:
        return finite(lambda v:(((v&0xffffffff)>>1)==3,((v&0xffffffff)>>1)>=2,((v&0xffffffff)>>1)>=4,v&1),range(2,10), 'Unsigned half-power drives CMP3 and two SUB2/JS guards; final low bit selects extra multiply. Classes2..9 include every int32 input, including negatives.', ['0x97..0xbd','helpers0x44/0x5e'])
    return None


def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--audit',type=Path,required=True)
    ap.add_argument('--selectors',type=Path,required=True)
    ap.add_argument('--output',type=Path,required=True)
    ap.add_argument('--delta',type=Path,required=True)
    a=ap.parse_args()
    report=json.loads(a.audit.read_bytes())
    original={(r['formula'],r['stored_value_index']):r for r in json.loads(a.selectors.read_bytes())['options']}
    delta=[]
    for row in report['options']:
        if row['classification']=='original_direct_comparison_partition' and row['formula'] in CALL_REVIEWS:
            if row['blockers'] != ['Calls require interprocedural dataflow qualification']:
                raise ValueError('Unexpected call-review blockers: '+row['formula'])
            anchors, text = CALL_REVIEWS[row['formula']]
            row['call_review'] = dict(source_addresses=anchors, semantics=text,
                original_code_sha256=report['original_sources'][row['formula']]['original_code_sha256'])
            row['blockers'] = []
            row['domain_complete'] = True
        review=rule(row)
        if not review:continue
        source=report['original_sources'][row['formula']]
        if source.get('error'):raise ValueError('Unqualified original CODE: '+row['formula'])
        # Check that the exact source audit is still the file on disk; rules are
        # human-reviewed annotations and must be rerun/reviewed after source edits.
        asm=source['asm']
        if hashlib.sha256((ROOT/asm['path']).read_bytes()).hexdigest()!=asm['sha256']:
            raise ValueError('Original disassembly changed: '+row['formula'])
        values=row['scene_values']+row['fixture_values']
        signature=review.pop('signature',None)
        row['classification']=review.pop('kind')
        row['review']=review
        row['domain_complete']=signature is not None
        row['review_qualification']='Manual original-CODE dataflow; input presence only; C++/state/interaction evidence remains separate.'
        if signature:
            observed={signature(v) for v in values}
            missing=[v for v in review['representatives'] if signature(v) not in observed]
            row['representatives']=review['representatives']
            row['missing_input_classes']=missing
            row['class_count']=len({signature(v) for v in review['representatives']})
            if missing:
                base=dict(original[(row['formula'],row['stored_value_index'])])
                base['missing_representatives_in_scene_catalog']=missing
                base['domain']=dict(kind='reviewed_original_semantics',**review)
                delta.append(base)
        else:
            row['missing_boundary_inputs']=[v for v in review['boundary_inputs'] if v not in values]
    report['totals']=dict(options=len(report['options']), classifications=dict(Counter(r['classification'] for r in report['options'])),
        finite_domains=sum(r['domain_complete'] for r in report['options']),
        finite_missing_input_classes=sum(len(r.get('missing_input_classes',[])) for r in report['options']),
        new_reviewed_finite_domains=sum(r['classification']=='reviewed_finite_original_domain' for r in report['options']),
        new_reviewed_missing_input_classes=sum(len(r['missing_representatives_in_scene_catalog']) for r in delta),
        parity_passes_produced=0)
    report['review_source_sha256']=hashlib.sha256(Path(__file__).read_bytes()).hexdigest()
    a.output.write_text(json.dumps(report,indent=2)+'\n')
    a.delta.write_text(json.dumps(dict(scope='New reviewed finite marginal obligations; never parity PASS.',options=delta),indent=2)+'\n')
    print(json.dumps(report['totals']))


if __name__=='__main__':main()
