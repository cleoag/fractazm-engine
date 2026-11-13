"""Source-qualified finite ordinary feature inventory (never runs a renderer).

Header observations are not dynamic branch coverage. Equivalence below is only
the explicitly stated routing equivalence, not equality for arbitrary inputs.
Every retained status belongs to its recorded executable, never a new candidate.
"""
from __future__ import annotations
import argparse
from collections import Counter, defaultdict
import hashlib
import json
from pathlib import Path
import struct
import zlib

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / 'comparison/results/stage-a-20261018/ordinary-source-audit-v1.json'
LEDGER = ROOT / 'comparison/results/stage-a-20261018/coverage-ledger-v1.json'


def sha(p): return hashlib.sha256(p.read_bytes()).hexdigest()
def rel(p): return p.resolve().relative_to(ROOT).as_posix()
def read(p): return json.loads(p.read_text(encoding='utf-8-sig'))


class Audit:
    def __init__(self):
        self.inputs = {}
        self.texts = {}
        self.axes = []

    def source(self, path, needle):
        p = ROOT / path
        if path not in self.texts:
            self.texts[path] = p.read_text(encoding='utf-8', errors='replace').splitlines()
            self.inputs[path] = sha(p)
        matches = [i + 1 for i, line in enumerate(self.texts[path]) if needle in line]
        if not matches: raise ValueError(f'Missing source anchor {path}: {needle}')
        return dict(path=path, line=matches[0], matching_lines=matches, anchor=needle)

    def axis(self, family, ident, domain, pascal, cpp, equivalence='', gate='', obligations=(), observed=None):
        self.axes.append(dict(family=family, id=ident, classes=[str(x) for x in domain],
            pascal=self.source(*pascal), cpp=self.source(*cpp),
            gate=gate or 'No extra stage gate beyond the family entry point',
            equivalence=equivalence or 'Each listed class follows a distinct selector/semantic branch; no numerical equivalence asserted.',
            boundary_or_coupling_obligations=list(obligations), observation_key=observed,
            accounting='finite classes enumerated; candidate acceptance separate'))

    def catalog(self):
        P='mb3d/'; C='src/core/'
        H=(P+'HeaderTrafos.pas','function GetMCTparasFromHeader')
        HC=(C+'header_trafos.cpp','void GetMCTparasFromHeader')
        # Fallback to stable implementation-wide anchors, while each axis uses
        # a more specific expression wherever the implementation has one.
        def a(f,i,d,p,c,e='',g='',o=(),k=None): self.axis(f,i,d,p,c,e,g,o,k)
        a('scene-loader','file-container',['raw','zlib'],(P+'FileHandling.pas','LoadParameter'),('src/io/m3p_parser.cpp','isCompressed'),k='file-container')
        a('scene-loader','header-layout',['0..3','4..6','7','8..19','20..250'],(P+'FileHandling.pas','MId < 4'),('src/io/m3p_parser.cpp','GetRawHeaderSizeForMandId'),
          'Equivalent disk record sizes within a class only; version-specific migrations are NOT equivalent.',o=['Migration thresholds are listed separately; old layouts need immutable-Pascal fixtures.'],k='header-layout')
        a('scene-loader','cached-pixel-layout',['none<=15','siLight3:16..17','siLight4:18','siLight5:19+'],(P+'FileHandling.pas','ConvertSiLight4To5'),('src/io/m3p_parser.cpp','GetStoredPixelDataSize'),o=['M3I addon position, truncated pixels, MID<20 shadow repair and MID35 fog correction.'])
        a('scene-loader','addon-layout',['synthesized<16','old-addon','modern-addon','missing-truncated'],(P+'FileHandling.pas','LoadParameter'),('src/io/m3p_parser.cpp','kHeaderCustomAddonOldSize'),o=['Roundtrip bytes does not replace Pascal migration comparison.'])
        a('scene-loader','tiling',['off','on'],(P+'HeaderTrafos.pas','TilingOptions'),('src/io/m3p_parser.cpp','tmpHeader.TilingOptions != 0'),o=['countX/Y zero clamp, tile index, crop1..3, M3I pixel offset; raw values cannot be counted as successful loading.'],k='tiling')
        a('formula-identity','slot-identity',['builtin','external-registered','alias','missing-unsupported'],(P+'CustomFormulas.pas','LoadCustomFormula'),('src/io/m3p_parser.cpp','void ParseExternFormulas('),o=['Names are case-insensitive; aliases must preserve typed defaults; all six slots share dispatch but not state.'])
        a('formula-identity','slot-position',range(6),(P+'CustomFormulas.pas','FillCustomVBufWithVars'),(C+'header_trafos.cpp','for (int x = 0; x <= 5; x++)'),
          'Same indexed initialization loop for slots0..5; slot membership/end/repeat indices remain distinct.',o=['Cover first/middle/last active slot and second DE group.'])
        a('integer-loader','integer-round',['negative-half-even','negative-half-odd','positive-half-even','positive-half-odd','integer','near-tie','overflow','nan-inf'],(P+'CustomFormulas.pas','PInteger(p)^ := Round(dOptionValues[i]);'),(C+'header_trafos.cpp','case 20:'),o=['308 original FPC cases compare INTEGER,2INTEGER,FOLDING; preserve invalid-conversion classification.'])
        a('integer-loader','negative-buffer',['fits','exact256','crosses256','skip-compound-cursor','novariable','folding-pointer'],(P+'CustomFormulas.pas','const MemNeeded:'),(C+'header_trafos.cpp','void BuildFormulaConstBuffer('),o=['Seven independent Pascal buffers; native pointer bytes are deliberately normalized, not certified.'])
        a('compound-loader','option-type',range(23),(P+'CustomFormulas.pas','case byOptionTypes[i] of'),(C+'header_trafos.cpp','switch (optTypes[i])'),o=['Declaration index, saved-value cursor and runtime offset differ for compound options.','Type10 consumes no storage; types16/17 repeat one value and do not consume extra saved values.'])
        a('compound-loader','preprocess-boundary',['zero','signed-small','finite-normal','overflow-nonfinite','rotation-singularity'],(P+'CustomFormulas.pas','procedure FillCustomVBufWithVars'),(C+'header_trafos.cpp','case 13:'),o=['Separate reciprocal floor, squared reciprocal floor, box scale and rotation precision; no common blanket equivalence.'])
        a('hybrid','hybrid-mode',range(4),(P+'CustomFormulas.pas','(pHCA.bOptions1 and 3) = 1'),(C+'header_trafos.cpp','LongBool bIsInterpolHybrid'),o=['Mode1 weights are stored as IEEE Single bits in iItCount, not integer25/75.','Mode3 legacy IFS normalization is tested separately from ordinary alternating.'],k='hybrid-mode')
        a('hybrid','de-combination',range(6),(P+'HeaderTrafos.pas','FormulaType := '),(C+'header_trafos.cpp','Result.FormulaType = std::min(6'),g='hybrid mode2',o=['Types0=min,1=max,2=max-inverse,3=linear smooth,4=quadratic smooth,5=mix; smoothing sign/zero boundary and group state matter.'],k='de-combination')
        a('hybrid','mix-color',range(3),(P+'HeaderTrafos.pas','Result.DEmixCol'),(C+'ray_marcher.cpp','switch (mct->DEmixCol)'),g='DE-combination mix type5',o=['Color selection0/1/2, unequal group maxIt, 3D+IFS order.'],k='mix-color')
        a('hybrid','repeat-control',['positive','zero-skip','negative-stop','repeat-from','group-end'],(P+'formulas.pas','procedure doHybridPas('),(C+'formulas.cpp','void doHybridPas('),o=['Empty chain, out-of-range routing, stop boundary and first/stateful formula changes.'])
        a('hybrid','interpolation-weight',['fallback-zero-or-subnormal','interior25-75','interior75-25','endpoint','negative-or-nonfinite'],(P+'HeaderTrafos.pas','bIsInterpolHybrid'),(C+'header_trafos.cpp','LongBool bIsInterpolHybrid'),g='hybrid mode1',o=['Real float weights use interpolation-float-v1; earlier integer25/75 scenes are fallback controls only.'],k='interpolation-weight')
        a('orbit-state','julia',['false','true'],(P+'HeaderTrafos.pas','Result.bDoJulia'),(C+'header_trafos.cpp','Result.bDoJulia'),o=['C/W modifications and persistent formula state are formula-specific; delegate to per-formula ledger.'],k='julia')
        a('orbit-state','orbit-driver',['3D','4D','IFS','interpolated','DE-group2'],(P+'formulas.pas','procedure doHybrid4DPas('),(C+'formulas.cpp','void doHybrid4DPas('),o=['Initial/last iteration, bailout, maxIt0/1, written-state lane masks and caller memory.'])
        a('orbit-state','formula-write-contract',['C-write','W-write','radius-write','DE-write','trap-write','persistent-extra','conditional-next-slot','ordinary-stateless'],(P+'TypeDefinitions.pas','TIteration3D'),(C+'types.h','TIteration3D'),o=['A generic point PASS is finite numerical evidence, not proof of every state/caller coupling.'])
        a('raymarch','inside',range(3),(P+'HeaderTrafos.pas','bInsideRendering :='),(C+'header_trafos.cpp','Result.bInsideRendering ='),
          'Fresh file-load GUI offers only0/1/2; raw3 reads back as outside0. Direct-header raw3 retains distinct refraction semantics and is not file-load coverage.',o=['Outside/inside/both; preserve OTrap high-bit restoration for secondary passes.','Direct-header raw3 requires its own explicitly scoped fixture.'],k='inside')
        a('raymarch','disable-analytic-de',['false','true'],(P+'HeaderTrafos.pas','bDisableADE'),(C+'header_trafos.cpp','LongBool bDisableADE'),o=['Analytic capability compatibility and numerical fallback across mixed formulas.'],k='disable-analytic-de')
        a('raymarch','normals-mode',['orbit','DE-forced','DE-requested'],(P+'HeaderTrafos.pas','NormalsOnDE :='),(C+'header_trafos.cpp','NormalsOnDE'),o=['IFS forces DE normals; smooth normals from stored depth is a separate postprocess. Runtime formula classification is required; do not infer a forced driver from header alone.'])
        a('raymarch','vary-de-stop',['false','true'],(P+'HeaderTrafos.pas','bVaryDEstop'),(C+'header_trafos.cpp','Result.bVaryDEstop'),k='vary-de-stop')
        a('raymarch','color-option',range(6),(P+'HeaderTrafos.pas','ColorOption := Min(5'),(C+'ray_marcher.cpp','switch (pMCTparas->ColorOption)'),
          'Raw byColor2Option >=5 is clamped to5 before all ordinary callers.',o=['ColorOnIt reruns orbit; DE/trap second-group state restoration needs explicit coupling.'],k='color-option')
        a('raymarch','color-on-it',['off','output-input','rerun'],(P+'TypeDefinitions.pas','bColorOnIt:'),(C+'ray_marcher.cpp','RMdoColorOnIt'),o=['Byte0,1,>=2 select distinct setup; iteration number is numerical within rerun class.'],k='color-on-it')
        a('raymarch','termination',['escape','maxIt','minimumIt','step-limit','refine-DE','refine-iteration','unordered'],(P+'CalcThread.pas','procedure TMandCalcThread.Execute'),(C+'ray_marcher.cpp','RMdoBinSearchIt'),o=['DEcomb/IFS plus minIt and exhausted/refined rays; numerical boundary fixtures already exist.'])
        a('camera','optics',[0,1,2,3],(P+'HeaderTrafos.pas','MCTCameraOptic := Min(2'),(C+'header_trafos.cpp','Result.MCTCameraOptic ='),
          'Raw3 maps MCT optic to2 but FOV/pano dispatch test raw==2; raw3 is NOT equivalent to2 across the renderer.',o=['UI ordinary optics0..2; raw3 must be explicitly rejected or qualified, not silently counted as panorama.'],k='optics')
        a('camera','stereo',[0,1,2,3,4],(P+'HeaderTrafos.pas','function CalcXoff'),(C+'header_trafos.cpp','switch (Header->bStereoMode)'),
          'File loading unconditionally resets stereo to0 in Pascal FileHandling and C++ UpdateFormulaAndParameterValues. Direct-header entry can retain1..4;0/2 share default offset and shift.',o=['File scenes cannot qualify stereo1..4; require a separate direct-header C API/kernel fixture.'],k='stereo')
        a('slice','slice-plane',[1,2,3],(P+'HeaderTrafos.pas','if calc3D or (iSliceCalc = 1)'),(C+'header_trafos.cpp','if (Result.calc3D || Result.iSliceCalc == 1)'),
          'For calc3D=false, every raw value except1/3 is normalized to2 before slice execution; raw0/2 independent images agree.',g='calc3D=false',k='slice-plane')
        a('slice','slice-driver',['orbit','de-combination'],(P+'CalcThread2D.pas','FormulaType > 0'),(C+'calc_2d.cpp','if (mcp->FormulaType > 0)'),g='calc3D=false',o=['DEcomb slice and inside coloring need a targeted scene; retained3D flags are suppressed by early return.'],k='slice-driver')
        a('cutplanes','cut-mask',range(8),(P+'HeaderTrafos.pas','iCutOptions := iCutOptions or'),(C+'reflection.cpp','if (mcp.iCutOptions == 0)'),
          'Each axis uses same min-length pattern; combinations change nearest clipping plane and cannot be numerically collapsed.',g='calc3D=true',o=['Axis normal sign follows view direction; zero-direction threshold and clipped primary/shadow/reflected rays.'],k='cut-mask')
        a('hard-shadow','hard-shadow-stage',['off','hard','soft'],(P+'CalcHardShadow.pas','calcHSsoft'),(C+'renderer.cpp','ShouldRunHardShadowPass('),o=['Active lightmask plus auto bit; do not credit disabled lights merely because mask set.'],k='hard-shadow-stage')
        a('hard-shadow','selected-shadow-light',range(6),(P+'CalcHardShadow.pas','HSminLengthToCutPlane'),(C+'renderer.cpp','ShouldRunHardShadowPass('),
          'Same indexed light kernel, different packed shadow bit and sorted identity; permutation needs a controlled assertion.',g='3D auto shadow enabled and selected light active',k='selected-shadow-light')
        a('hard-shadow','completed-shadow-state',['not-computed','completed','replay-restored'],(P+'DivUtils.pas','AllAutoProcessings'),(C+'renderer.cpp','ShouldRunHardShadowPass('),o=['bHScalculated high lightmask must be preserved into paint, without forcing completion when pass disabled.'])
        a('ao','ao-kernel',list(range(10))+['DE'],(P+'AmbHiQ.pas','case ATi of'),(C+'ambient_occlusion.cpp','switch (ATi)'),g='calc3D and AO enable bit0',o=['DE mask is tested before ATi;raw6/7 route DE, so Video dispatch10 is not reachable through ordinary enabled header path.'],k='ao-kernel')
        a('ao','ao-quality',range(4),(P+'CalcAmbShadowDE.pas','Quali :='),(C+'ambient_occlusion.cpp','if (quality == 0)'),g='AO enabled',o=['HiQ wavelet count and DE ray layout use quality differently; observe separately.'],k='ao-quality')
        a('ao','de-ao-quality',range(4),(P+'CalcAmbShadowDE.pas','Quali :='),(C+'ambient_occlusion.cpp','if (quality == 0)'),g='DE AO enabled',k='de-ao-quality')
        a('ao','de-ao-dither',[0,1,2],(P+'CalcAmbShadowDE.pas','Dither := Header.AODEdithering'),(C+'ambient_occlusion.cpp','if (dither > 0)'),
          '0 disables dither;positive d changes row/column modulo.1 and2 are not numerically equivalent.',g='DE AO enabled',o=['Dither2 and quality0 with positive dither are separate loops.'],k='de-ao-dither')
        a('ao','de-ao-first-random',['false','true'],(P+'CalcAmbShadowDE.pas','bMCTFirstStepRandom :='),(C+'ambient_occlusion.cpp','bool firstStepRandom'),g='DE AO enabled',k='de-ao-first-random')
        a('ao','random-ao-repeat',['one','multiple'],(P+'AmbHiQ.pas','aRcount := Header.SSAORcount'),(C+'ambient_occlusion.cpp','const int aoIterations'),g='raw AO4/5 enabled',o=['Count>1 changes accumulation plus repeated workers; wavelet-level RNG seed lifetime cannot be deduced from header presence.'],k='random-ao-repeat')
        a('lighting','light-kind',['normal','disabled','map'],(P+'HeaderTrafos.pas','iLightOption'),(C+'header_trafos.cpp','Lvals->iLightOption[i]   ='),
          'Raw3 becomes disabled1; map2 with mapNr0 becomes disabled1. Resource load failure also disables map light.',k='light-kind')
        a('lighting','light-position',['directional','positional'],(P+'PaintThread.pas','PosLight :='),(C+'colorizer.cpp','bool PosLight ='),g='normal light enabled',k='light-position')
        a('lighting','diffuse-function',range(4),(P+'PaintThread.pas','GetCosTabVal'),(C+'header_trafos.cpp','Lvals->iLightFuncDiff[i]'),g='normal light enabled',o=['Linear/squared output paths and roughness blend; both endpoints and interpolation.'],k='diffuse-function')
        a('lighting','specular-exponent',range(8),(P+'PaintThread.pas','FastIntPow'),(C+'header_trafos.cpp','Lvals->iLightPowFunc[i]'),
          'Same exponentiation path, exponent=2<<(raw&7); values are distinct numeric parameters, not eight algorithms.',g='normal light enabled',k='specular-exponent')
        a('lighting','visible-light-shape',[0,2,4,6,8,'other'],(P+'PaintThread.pas','case iLightPos[LightNr] and 14 of'),(C+'colorizer.cpp','switch (PLVals->iLightPos[LightNr] & 14)'),g='normal light enabled',o=['Shape8 has separate radius and compositing; default10/12/14 requires caller-domain classification.'],k='visible-light-shape')
        a('lighting','color-arithmetic',['linear','squared'],(P+'PaintThread.pas','procedure CalcPixelColorSqr'),(C+'colorizer.cpp','const bool calcPixColSqr'),k='color-arithmetic')
        a('lighting','gamma-class',['below','identity','above'],(P+'HeaderTrafos.pas','iGammaH'),(C+'header_trafos.cpp','if (gammaIdx == 32)'),
          'Gamma0..31 shares square interpolation;32 identity;33..63 sqrt interpolation. Endpoints still numerical boundaries.',k='gamma-class')
        a('lighting','color-source',['smooth-iteration','orbit-trap'],(P+'PaintThread.pas','if (iColOnOT and 1) = 0'),(C+'colorizer.cpp','if ((PLVals->iColOnOT & 1) == 0)'),k='color-source')
        a('lighting','gradient-cycling',['false','true'],(P+'PaintThread.pas','if bColCycling'),(C+'colorizer.cpp','if (PLVals->bColCycling)'),o=['Outside endpoints vs wrapped interval; interior gradient uses distinct table.'],k='gradient-cycling')
        a('lighting','gradient-interpolation',['interpolate','nearest'],(P+'PaintThread.pas','bNoColIpol'),(C+'colorizer.cpp','Lvals->bNoColIpol ='),k='gradient-interpolation')
        a('lighting','total-light-mode',['zero','nonzero'],(P+'PaintThread.pas','iExModes = 0'),(C+'colorizer.cpp','if (PLVals->iExModes == 0)'),e='All nonzero values route CalcTotalLight2; no further iExModes bit tests occur in that function.',k='total-light-mode')
        a('maps','diffuse-map-projection',['iteration-trap','sphere','wrap3D','wrap3D-alt'],(P+'PaintThread.pas','if iColOnOT > 7'),(C+'lightmap.cpp','if (plVals->iColOnOT > 7)'),g='diffuse map requested and load succeeds',o=['Header observations certify request only; actual asset hash/loading and pixel influence must be recorded.'],k='diffuse-map-projection')
        a('maps','background-projection',['screen','sphere'],(P+'PaintThread.pas','bDirectImageCoord'),(C+'colorizer.cpp','Lvals->bDirectImageCoord ='),g='background resource requested and load succeeds',k='background-projection')
        a('maps','resource-contract',['loaded','missing','rgb8','rgb16-words','wrap','clamp','nearest','interpolated'],(P+'PaintThread.pas','GetLightMapPixel'),(C+'lightmap.cpp','GetLightMapPixel('),o=['Image decoding, formula height/sphere callback slots and gamma differ; point synthetic callbacks do not qualify production resources.'])
        a('maps','formula-map-callback',['sphere-spline','direct-xy-spline'],(P+'Calc.pas','It3Dex.PMapFunc := GetMapPixelSphereSpline'),(C+'ray_marcher.cpp','It3Dex->PMapFunc = MapFuncSphereSpline'),e='Ordinary IniIt3D fixes these two callbacks. Legacy linear/bicubic helpers are not credited as selectable ordinary algorithms.',g='formula invokes a map callback and resource load succeeds',o=['Callback dispatch and real resource influence require separate evidence; helper existence is insufficient.'])
        a('fog','fog-driver',['distance','iteration','volumetric'],(P+'CalcThread.pas','if MCTparas.DFogOnIt < 65535'),(C+'header_trafos.cpp','Result.DFogOnIt ='),
          'Volumetric selector overrides stored bDFogIt. Nonzero ordinary fog values select same equality branch with different iteration target.',g='3D',k='fog-driver')
        a('fog','fog-composite',[0,1,2],(P+'PaintThread.pas','bDFogOptions'),(C+'colorizer.cpp','if (Lvals->bDFogOptions == 3)'),
          'Raw3 normalizes to1 before paint;0 additive,1 clipped/translucent,2 positive-only.',k='fog-composite')
        a('fog','far-fog',['false','true'],(P+'PaintThread.pas','bFarFog'),(C+'header_trafos.cpp','Lvals->bFarFog'),o=['Linear color only remaps when enabled; squared arithmetic has its own base remap.'],k='far-fog')
        a('volume','volume-light',range(1,7),(P+'HeaderTrafos.pas','bVolLightNr and 7'),(C+'reflection.cpp','int vlNr ='),g='3D and low3 bVolLightNr nonzero',o=['Raw7 clamps to light6 in some setup but must not be treated as valid UI selection without qualification.'],k='volume-light')
        a('volume','volume-light-position',['directional','positional'],(P+'CalcSR.pas','CalcVolLightMap'),(C+'reflection.cpp','void CalcVolLightMap('),g='volume enabled',o=['Directional/cube-map primary and recursive consumers; cache rebuilt per scene.'],k='volume-light-position')
        a('volume','volume-map-face',range(6),(P+'CalcSR.pas','CalcVolLightMap'),(C+'reflection.cpp','switch (side)'),
          'Same cube-face kernel with six explicit coordinate permutations; numerical face seams need samples.',g='positional volumetric prepass')
        a('reflection','secondary-mode',['off','reflect','transmit-all','transmit-dIFS'],(P+'CalcSR.pas','procedure TSRCalcThread.CalcRay('),(C+'reflection.cpp','if (!st.bCalcTrans)'),g='3D and reflection auto bit',o=['DIFS-only condition uses actual DEoptionResult, not formula name; check both sides of group mix.'],k='secondary-mode')
        a('reflection','bounce-boundary',['zero','one','multiple','last-depth'],(P+'CalcSR.pas','MaxReflections'),(C+'reflection.cpp','st.MaxReflections'),o=['Transmission flips inside flag on entry/exit; total internal reflection and scattering share continuation state.'])
        a('reflection','secondary-medium',['outside','inside','both','total-internal-reflection','absorption'],(P+'CalcSR.pas','bTransFlipInside'),(C+'reflection.cpp','st.bTransFlipInside'),o=['Explicit coupled scenes required for inside x transmit x cut-plane and volume reuse.'])
        a('dof','dof-dispatch',['disabled','sorted0','sorted1','sorted2','plain0','plain1','plain2'],(P+'DivUtils.pas','bCalcDOFtype and 1'),(C+'dof.cpp','if ((header->bCalcDOFtype & 1) == 0)'),
          'Enable bit0 dominates; rawpass0=1, rawpass1=2, rawpass2=3 passes. Rawpass3 reaches4 kernel passes but the original Pascal radio caller rejects it; it is not an alias of rawpass2.',g='3D',k='dof-dispatch')
        a('dof','dof-depth-region',['foreground','focus','background','no-hit','clipped-edge'],(P+'DOF.pas','procedure doDOFsort('),(C+'dof.cpp','bool bDescale ='),o=['Sorted forward occlusion/weight order is not equivalent to plain accumulation; radius0 and depth sentinel boundaries.'])
        a('packed-output','surface-class',['hit-exterior','hit-interior','background','cut-plane'],(P+'TypeDefinitions.pas','TsiLight5 ='),(C+'types.h','struct TsiLight5'),o=['18-byte exact record, packed depth/roughness alias, high-bit tags, saturation.'])
        a('packed-output','normal-source',['DE','orbit','post-depth'],(P+'HeaderTrafos.pas','NormalsOnDE'),(C+'renderer.cpp','NormalsOnDE'),o=['byCalcNsOnZBufAuto has a separate post-stage; existing image corpus may miss it.'])
        a('packed-output','post-depth-normals',['disabled','enabled'],(P+'ImageProcess.pas','procedure NormalsOnZbuf'),(C+'zbuffer_normals.cpp','namespace mb3d'),g='3D and byCalcNsOnZBufAuto bit0',o=['Implemented auto-stage: v7 independent live buffers verify on/off. Primary-captured dumps alone cannot prove a postprocess.'],k='post-depth-normals')
        a('exports','export-kind',['BMP','PNG','depth8','depth16','depth32','normals','C-API-buffer'],(P+'FileHandling.pas','implementation'),(C+'mb3d_api.cpp','mb3d_render'),o=['Per-format unit tests qualify produced data; Pascal file byte identity is not implied.'])
        a('exports','error-class',['missing-input','bad-format','invalid-dimensions','unwritable-output','unsupported-formula','invalid-parameter'],(P+'FileHandling.pas','LoadParameter'),(C+'mb3d_api.cpp','mb3d_render'),o=['Errors must not publish a successful partial image; strict oddFlip/zero-power rejection has dedicated tests.'])
        a('workers-rng','ordinary-schedule',['one-logical-worker','multiple-logical-workers','repeat','trace'],(P+'CalcThread.pas','iThreadCount'),(C+'thread_pool.cpp','namespace mb3d'),o=['Fixed logical schedule reproducibility is required; changing logical workers can change RNG/state.'])
        a('workers-rng','random-state-owner',['primary-row-lane','ao-worker-wavelet','ao-repeat','secondary-ray'],(P+'AmbHiQ.pas','aRcount := Header.SSAORcount'),(C+'ambient_occlusion.cpp','PasLcgInitFromThread'),o=['Pano AO fixed wavelet reseeding and Single arithmetic; non-pano reseeding audit needs explicit multiwavelet control.'])
        a('diagnostics','trace-stage',['primary','AO','paint','reflection','packed-replay'],(P+'DivUtils.pas','AllAutoProcessings'),(C+'renderer.cpp','PaintFromSiLight'),o=['Production trace disabled/enabled must agree at fixed logical schedule; frozen-candidate handoff tests plus retained trace captures.'])
        a('diagnostics','replay-state',['fresh','shadow-complete','AO-complete','reflection-late','DOF-late'],(P+'DivUtils.pas','AllAutoProcessings'),(C+'renderer.cpp','PaintFromSiLight'),o=['Restored headers and packed buffers must refer to same scene/configuration; postpass ordering is shared state.'])

    def decode(self, data):
        u=lambda off: struct.unpack_from('<I',data,off)[0]
        b=lambda off: data[off]
        v={}
        def put(k,x): v[k]=[str(x).lower() if isinstance(x,bool) else str(x)]
        mid=u(0); put('header-layout','0..3' if mid<4 else '4..6' if mid<7 else '7' if mid==7 else '8..19' if mid<20 else '20..250')
        # This decoder models effective selectors only for the exact modern
        # layout used by this corpus. Earlier migrations remain explicit unknowns.
        if len(data)<1976 or mid!=44 or (u(476)>>20)&7!=7 or b(516)!=8:
            put('effective-selector-qualification','unknown-unmodeled-migration'); return v
        put('effective-selector-qualification','MID44-light7-extra8')
        calc3d=bool(b(344)); ao=b(149); hm=b(841)&3; lights=500
        put('tiling','on' if u(428) else 'off'); put('hybrid-mode',hm)
        put('julia',bool(b(190))); raw_inside=(b(842)>>1)&3
        put('raw-inside',raw_inside);put('inside',0 if raw_inside==3 else raw_inside)
        put('disable-analytic-de',bool(b(842)&1)); put('vary-de-stop',bool(b(162)))
        put('color-option',min(5,b(342))); put('color-on-it','off' if b(19)==0 else 'output-input' if b(19)==1 else 'rerun')
        put('optics',b(148)); put('raw-stereo',b(126)); put('stereo',0)
        if hm==2:
            put('de-combination',min(5,b(843)))
            if b(843)>=5: put('mix-color',min(2,b(422)))
        if hm==1:
            x,y=struct.unpack_from('<f',data,848)[0],struct.unpack_from('<f',data,1036)[0]
            put('interpolation-weight','interior25-75' if (x,y)==(.25,.75) else 'interior75-25' if (x,y)==(.75,.25) else 'fallback-zero-or-subnormal' if abs(x)+abs(y)<1e-30 else 'endpoint' if x==0 or y==0 else 'negative-or-nonfinite' if not (x>0 and y>0) else 'other-finite')
        if not calc3d:
            put('slice-plane',b(345) if b(345) in (1,3) else 2)
            put('slice-driver','de-combination' if hm==2 else 'orbit')
        if calc3d:
            put('post-depth-normals','enabled' if b(331)&1 else 'disabled')
            put('cut-mask',b(176)&7)
            shadow=bool(b(133)&1)
            active=[i for i in range(6) if b(lights+32*i)&3==0 and b(133)&(4<<i)]
            put('hard-shadow-stage','off' if not shadow or not active else 'soft' if b(139)&1 else 'hard')
            if shadow: v['selected-shadow-light']=[str(i) for i in active]
            if ao&1:
                at=(ao>>1)&7; de=ao&12==12
                put('ao-kernel','DE' if de else at+4 if b(148)==2 and 2<=at<=5 else at)
                put('ao-quality',(ao>>4)&3)
                if de:
                    put('de-ao-quality',(ao>>4)&3);put('de-ao-dither',b(188));put('de-ao-first-random',bool(ao&128))
                elif at in (4,5): put('random-ao-repeat','multiple' if b(187)>1 else 'one')
            dof=b(181);put('dof-dispatch','disabled' if not dof&1 else ('plain' if dof&8 else 'sorted')+str((dof>>1)&3))
            vl=b(343)&7;put('fog-driver','volumetric' if vl else 'iteration' if b(223) else 'distance')
            if vl:
                put('volume-light',vl);put('volume-light-position','positional' if b(lights+32*min(5,vl-1))&4 else 'directional')
            sr=b(336);put('secondary-mode','off' if not sr&1 else 'reflect' if not sr&2 else 'transmit-dIFS' if sr&4 else 'transmit-all')
        tb=u(476);add=b(439);put('color-arithmetic','squared' if add&1 else 'linear')
        gamma=(tb>>23)&63;put('gamma-class','below' if gamma<32 else 'above' if gamma>32 else 'identity')
        put('color-source','orbit-trap' if tb&0x20000 else 'smooth-iteration');put('gradient-cycling',bool(tb&0x4000))
        put('gradient-interpolation','nearest' if b(lights+3*32+24)&1 else 'interpolate')
        put('total-light-mode','nonzero' if b(lights+2*32+24) else 'zero')
        put('far-fog',bool(tb&0x40000));fog=b(lights+24)&3;put('fog-composite',1 if fog==3 else fog)
        for k in ['light-kind','light-position','diffuse-function','specular-exponent','visible-light-shape']:v[k]=[]
        for i in range(6):
            off=lights+32*i; opt=b(off)&3;fn=b(off+1);mapnr=struct.unpack_from('<H',data,off+7)[0]
            opt=1 if opt==3 or (opt==2 and mapnr==0) else opt
            v['light-kind'].append(['normal','disabled','map'][opt])
            if opt==0:
                v['light-position'].append('positional' if b(off)&4 else 'directional')
                v['diffuse-function'].append(str((fn>>4)&3));v['specular-exponent'].append(str(fn&7))
                shape=((b(off)>>2)&6)|((fn&128)>>4);v['visible-light-shape'].append(str(shape) if shape in (0,2,4,6,8) else 'other')
        if b(435) or b(lights+32+16):
            put('diffuse-map-projection',['iteration-trap','sphere','wrap3D','wrap3D-alt'][b(lights+32+24)&3])
        if b(816): put('background-projection','screen' if tb&0x8000 else 'sphere')
        return {k:sorted(set(vv)) for k,vv in v.items()}

    def scenes(self, ledger):
        scenes=[]
        broad=ledger['ordinary343']
        for x in broad['cases']:
            scenes.append(dict(id=x['scene'],path=x['path'],sha256=x['sha256'],evidence=broad['summary'],status=x['historical_status']))
        for folder in ['additional-modes-v1','interpolation-float-v1','effective-modes-v1','de-mix-controls-v2']:
            d=ROOT/'comparison/results/stage-a-20261018'/folder
            if not (d/'manifest.json').exists():continue
            m=read(d/'manifest.json');s=read(d/'summary.json')
            self.inputs[rel(d/'manifest.json')]=sha(d/'manifest.json');self.inputs[rel(d/'summary.json')]=sha(d/'summary.json')
            status={r.get('scene',r.get('case')):r.get('status',r.get('reference',{}).get('status','unknown')) for r in s['results']}
            for x in m['cases']:
                scenes.append(dict(id=x['id'],path=x['scene'],sha256=x['sha256'],evidence=rel(d/'summary.json'),status=status.get(x['id'],'unknown')))
        for row in scenes:
            p=ROOT/row['path']; raw=p.read_bytes()
            if sha(p)!=row['sha256']:raise ValueError(f'Scene identity changed {p}')
            container='raw'
            try:
                raw=zlib.decompress(raw);container='zlib'
            except zlib.error:pass
            # Deferred rendering is not admitted to this inventory.
            if len(raw)>=840 and 20<=struct.unpack_from('<i',raw)[0]<=44 and raw[124]&1:raise ValueError('Deferred scene encountered')
            row['selectors']=self.decode(raw);row['selectors']['file-container']=[container]
        return scenes

    def build(self):
        ledger=read(LEDGER);self.inputs[rel(LEDGER)]=sha(LEDGER)
        self.catalog();scenes=self.scenes(ledger)
        for axis in self.axes:
            observations=defaultdict(list)
            for scene in scenes:
                if scene['status'].lower() not in ('pass','reference_ready'):continue
                for value in scene['selectors'].get(axis['observation_key'],[]):
                    observations[value].append(dict(scene=scene['id'],status=scene['status'],evidence=scene['evidence']))
            axis['header_observations']={k:dict(count=len(v),examples=v[:8]) for k,v in sorted(observations.items())}
            axis['not_observed_in_qualified_scene_inventory']=[c for c in axis['classes'] if c not in observations] if axis['observation_key'] else None
            axis['unclassified_observed_values']=[c for c in observations if c not in axis['classes']]
        oldfamilies=ledger['ordinary_features'] if 'ordinary_features' in ledger else ledger['features']
        families=[{k:v for k,v in f.items() if k in ('id','pascal','cpp','test_sources')} for f in oldfamilies]
        for family in families:
            family['selector_axes']=[x['id'] for x in self.axes if x['family']==family['id']]
            family['inventory_status']='enumerated_with_explicit_obligations'
            family['semantic_acceptance']='not certified by static inventory'
        self.inputs[rel(Path(__file__))]=sha(Path(__file__))
        # Reject a report assembled across concurrent production edits.
        for path, expected in self.inputs.items():
            if sha(ROOT/path)!=expected: raise ValueError('Input changed during audit: '+path)
        return dict(schema_version=1,scope='ordinary headless only; deferred Monte Carlo/JIT excluded',
            contract='docs/PARITY_TRACE.md Stage A1/A2',
            interpretation=['22 families are accounted for; axis listing is a source review, not a formal proof that every control-flow branch was extracted.',
                'Header values are admitted only under recorded outer gates. A resource request is still not proof of successful load or pixel influence.',
                'Tests linked from the ledger are test source evidence. Their existence does not certify execution on a final candidate.',
                'Equivalence statements are local to the stated routing; numerical parameters and shared state are not automatically equivalent.',
                'Earlier integer25/75 interpolation scenes exercise float-subnormal fallback; interpolation-float-v1 supplies actual .25/.75 weights.',
                'DOF rawpass3 was rejected by original Pascal radio caller; the two reference errors are retained, not marked covered.'],
            totals=dict(families=len(families),selector_axes=len(self.axes),declared_classes=sum(len(x['classes']) for x in self.axes),
                scene_observable_axes=sum(bool(x['observation_key']) for x in self.axes),
                scene_observed_classes=sum(len(set(x['classes'])&set(x['header_observations'])) for x in self.axes),
                observable_declared_classes=sum(len(x['classes']) for x in self.axes if x['observation_key']),
                scenes=len(scenes),qualified_scene_observations=sum(x['status'].lower() in ('pass','reference_ready') for x in scenes),
                numerical_completion_percentage=None),
            features=families,axes=self.axes,scenes=scenes,input_sha256=self.inputs,
            final_candidate_acceptance='pending separate frozen full ordinary, external, point/state and trace verification')


def main():
    ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('--output',type=Path,default=OUT);args=ap.parse_args()
    report=Audit().build();args.output.parent.mkdir(parents=True,exist_ok=True)
    args.output.write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print(json.dumps(report['totals'],ensure_ascii=False))
    for axis in report['axes']:
        if axis['not_observed_in_qualified_scene_inventory']:
            print(axis['family'],axis['id'],','.join(axis['not_observed_in_qualified_scene_inventory']))


if __name__=='__main__':main()
