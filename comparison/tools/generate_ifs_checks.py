"""Finite IFS crossings, with explicit limits rather than exhaustive-coverage claims.

genIFS: all {-1,0,1} fold flags, four signed/zero scales, four rotations.
OctahedronIFS/OctaFullIFS: every pair of the six rotation axes, three angles
per selected axis and four scales, with asymmetric offsets.
foldcut8-IFS: all three normal coefficients in {-1,0,1}, both radius signs
including signed zero, three apply values and four scales.
gearIFS (opt-in): signed/zero tooth widths and periods, three apply values,
four scales and three signed shell thicknesses (576 combinations).
gear2IFS (opt-in): the same crossings with four signed/zero inner-angle shifts
(2304 combinations).
hextgridIFS (opt-in): signed/zero radius and styling, three apply values,
four scales and four rotations (576 combinations).
discoballIFS (opt-in): signed 32-bit facet-count boundaries, -3/-1/0/1/3,
three apply values, four scales and three thicknesses (1764 combinations).
SphCageIFS (opt-in): latitude clamp boundaries and odd/even counts, signed
meridian counts, three apply values, four scales and three radii (2520).
transLogSpIFS (opt-in): both signed 32-bit counter boundaries, -1/0/1,
four tightnesses, four radius factors and three rotations (1200 combinations).
These are one-step probes sharing C++ parameter expansion with the x86 oracle.
CantorIFS (opt-in): four scales, four offset recipes and six rotations for
each of two matrices (576 combinations; explicit expanded saved count).
Amazing Surf 2 (opt-in): all 16 low-bit fold masks, three fold values,
two minimum radii and four scales (384 combinations).
Amazing Surf (opt-in): sphere/cylinder selectors, signed/zero scale, radius,
fold and scale variation with three rotations (1296 combinations).
"""
import argparse
import itertools
import json
import math
import struct
from pathlib import Path
from external_validation import ROOT, decode, digest, slots, write_json, prepare_external_options


def variants(name, original):
    scales = (-2., 0., 1e-12, 2.)
    if name in ('CylinderHeightMap','CylinderHMVoid','TorusHMVoid'):
        # All masked channel pairs with signed/zero heights, then signed aliases.
        cases=list(itertools.product(range(4),range(4),(-.5,0.,.5)))
        cases += [(v,v,.5) for v in (-2147483648,-2,-1,4,2147483647)]
        for index,(channel,trap,height) in enumerate(cases):
            rotation=((0.,0.,0.),(13.,-27.,41.),(90.,0.,-90.))[index%3]
            values=[(-1,0,10)[index%3],channel,.125,-.25,.375,1.,.75,height,
                    (-1.,0.,1.,2.)[index%4],*rotation,trap,-.125,-2.]
            if name=='CylinderHMVoid':values += [(-1.,0.,.5,1.,2.)[index%5]]
            yield values+original[len(values):]
        # Degenerate and negative shape dimensions, independently of channels.
        for length,radius in itertools.product((-1.,0.,1.),(-1.,-0.,1e-12,1.)):
            values=[10,0,0.,0.,0.,length,radius,.5,1.,0.,0.,0.,1,0.,1.]
            if name=='CylinderHMVoid':values += [-.5]
            yield values+original[len(values):]
    elif name in ('HeightMap2IFS','SphereHeightMap','SphereHMVoid'):
        # Cross all masked channel pairs and signed/zero heights. Additional
        # rows exercise signed aliases and odd/even NoPlane, including limits.
        cases=list(itertools.product(range(4),range(4),(-.5,0.,.5)))
        cases += [(v,v,.5) for v in (-2147483648,-2,-1,4,2147483647)]
        for index,(channel,trap,height) in enumerate(cases):
            rotation=((0.,0.,0.),(13.,-27.,41.),(90.,0.,-90.))[index%3]
            number=(-1,0,10)[index%3]
            if name=='HeightMap2IFS':
                flags=(-2147483648,-2,-1,0,1,2,2147483647)[index%7]
                values=[number,.125,-.25,.375,-.2,.3,height,*rotation,channel,trap,-.125,-2.,flags]
            else:
                scale=(-1.,0.,1.,2.)[index%4]
                values=[number,channel,.125,-.25,.375,-.2,.3,height,scale,*rotation,trap,-.125,-2.]
                if name=='SphereHMVoid':values += [(-1.,0.,.5,1.,2.)[index%5]]
            yield values+original[len(values):]
    elif name in ('MandelView','Riemann'):
        for power in (-2.,-.5,0.,.5,1.,2.,8.,100.,1e20):
            if name == 'MandelView':
                for z in (-1.,-0.,0.,2.):
                    yield [power,z]+original[2:]
            else:
                yield [power]+original[1:]
    elif name == 'Magnet4D':
        # Cross fix flag and three signed/zero multipliers; cycle x_Add.
        for index,(fix,z,up,down) in enumerate(itertools.product(
                (-1,0,1),(-1.,0.,2.),(-1.,0.,1.),(-1.,0.,1.))):
            yield [z,up,down,(-1.,0.,1.,.375)[index%4],fix]+original[5:]
        for fix in (-2147483648,2147483647):
            yield [-1.,1.,1.,-1.,fix]+original[5:]
    elif name == 'Mandel4DBiC':
        for w in (-2.,-1e-12,-0.,0.,1e-12,.375,2.):
            yield [w]+original[1:]
    elif name in ('DevilIFS','CitrusIFS'):
        # Cross apply/scale/shape settings; cycle rotations and asymmetric
        # offsets. Explicit saved count includes all three rotation angles.
        for index,(apply,scale,shape,clip) in enumerate(itertools.product(
                (-2147483648,-1,0,1,2147483647),(-2.,-0.,0.,1e-12,2.),(-1.,0.,3.3),(-1.,0.,8.))):
            rotation=((0.,0.,0.),(37.,-61.,23.),(-90.,0.,90.))[index%3]
            offsets=((0.,0.,0.),(.125,-.25,.5),(-.5,.25,-.125))[(index//3)%3]
            yield [shape,scale,*offsets,clip,apply,*rotation]+original[10:]
    elif name in ('_BPolygonFromCircle','_BPolygonToCircle'):
        # Original subtraction loops require nonnegative finite orders here.
        for sides in (0.,.5,1.,3.,4.,17.,257.):
            yield [sides]+original[1:]
    elif name == '_BPolyhedronToSphere':
        for a,b in itertools.product((0.,1.,4.,17.),repeat=2):
            yield [a,b]+original[2:]
    elif name == '_BStellahedron_F_Sphere':
        for a,b,angle1,angle2 in itertools.product((0.,3.,17.),(0.,4.,17.),(-1.,0.,1.),(-1.,0.,1.)):
            yield [a,b,angle1,angle2]+original[4:]
    elif name == '_Fold45double':
        for flags,k,fix in itertools.product(itertools.product((0,1),repeat=4),(-1.,0.,1.),(-1.,0.,1.)):
            yield [k,*flags,fix]+original[6:]
        for flags in itertools.product((-2147483648,2147483647),repeat=4):
            yield [1.,*flags,-.5]+original[6:]
    elif name in ('_reciprocalX3','_reciprocalY3','_reciprocalZ3'):
        for limiter in (-1.,-1e-300,-0.,0.,1e-300,.25,.5,1.,1e300):
            yield [limiter]+original[1:]
    elif name == '_AmazingBoxSSE2':
        # Cross all three saved parameters, including signed zero.
        for scale,radius,fold in itertools.product((-2.,-0.,0.,1e-12,2.),(-1.,-0.,0.,1e-12,.5,1.),(-1.,-0.,0.,.5,1.)):
            yield [scale,radius,fold]+original[3:]
    elif name in ('ABoxVaryScale','ABoxVaryScale4d'):
        # Finite crossed domain: signed/zero scale and minimum radius,
        # variation and radius powers. Fold and W add cycle independently.
        for index,(scale,radius,vary,power) in enumerate(itertools.product(
                (-2.,0.,1e-12,2.),(0.,1e-12,.5),(-.5,0.,.25),(-.5,0.,1.,2.))):
            values=[scale,radius,(-1.,0.,1.)[index%3],vary,power]
            if name=='ABoxVaryScale4d':values += [(-.375,0.,.375)[(index//3)%3]]
            yield values+original[len(values):]
    elif name in ('_Folding2','_SphereFolding2'):
        for fold in (-2.,-1e-12,-0.,0.,1e-12,.5,1.,2.,1e150):
            yield [fold]+original[1:]
    elif name in ('KaliDucks','Kalisets1'):
        for scale,radius in itertools.product((-2.,-0.,0.,1e-12,2.),(-1.,-1e-12,-0.,0.,1e-12,.5,1.)):
            yield [scale,radius]+original[2:]
    elif name in ('dodecatubeIFS','tetratubeIFS'):
        for index,(apply,rotation,radius,sphere) in enumerate(itertools.product(
                (-1,0,1),((0.,0.),(37.,-61.),(-90.,90.)),(-.1,0.,.1),(-.2,0.,.2))):
            values=[radius,1.,-.387298,scales[index%4],.125,-.25,.5,*rotation,apply,.414214,sphere,1.,2.]
            yield values+original[len(values):]
    elif name == 'logspIFS':
        for index,(apply,style,radius,tightness) in enumerate(itertools.product(
                (-1,0,1),(-1,0,1,2,3,4),(-1.,-0.,0.,1.),(-.5,0.,.5))):
            rotation=((0.,0.,0.),(37.,-61.,23.),(-90.,0.,90.))[index%3]
            values=[tightness,scales[index%4],.125,-.25,.5,radius,apply,(-2,0,2)[(index//3)%3],.3,style,*rotation,37.]
            yield values+original[len(values):]
    elif name == 'ducksIFS':
        # Odd Flip XZ needs caller-stack context. INT_MIN Iters wraps to a
        # two-billion-iteration loop in the original and is excluded here.
        for index,(apply,iterations,flip,rotation) in enumerate(itertools.product(
                (-1,0,1),(-1,0,1,2,8),(-2,0,2),(-90.,0.,37.))):
            values=[.7,scales[index%4],.125,-.25,.5,4.,apply,iterations,flip,.5602,-.4039,rotation,(-1.,0.,1.)[index%3]]
            yield values+original[len(values):]
    elif name == 'sliceIFS':
        for index,(apply,angle,period,thickness) in enumerate(itertools.product(
                (-1,0,1),(-30.,-0.,0.,30.),(-120.,-0.,0.,120.),(-.1,0.,.1))):
            values=[angle,scales[index%4],.125,-.25,.5,1.5,apply,period,thickness]
            yield values+original[len(values):]
    elif name == 'QuadricsIFS':
        for index,(apply,half,flip,zmul,hyp) in enumerate(itertools.product(
                (-1,0,1),(-1,0,1),(-1,0,1),(-1.,0.,1.),(-1.,1.))):
            values=[(-1.,0.,1.)[index%3],scales[index%4],.125,-.25,.5,.1,apply,zmul,hyp,half,flip]
            yield values+original[len(values):]
    elif name == 'diniIFS':
        for index,(apply,flip,upside,tightness) in enumerate(itertools.product(
                (-1,0,1),(-2,-1,0,1,2),(-1,0,1),(-.2,0.,.2))):
            radius=(-1.,0.,1.)[index%3]
            values=[radius,scales[index%4],.125,-.25,.5,1.5,apply,(-.1,0.,.1)[(index//3)%3],-.25,flip,upside,tightness]
            yield values+original[len(values):]
    elif name == 'Aex-TorusPow':
        for index,(mode,power,donut,zmul) in enumerate(itertools.product(
                (-1,0,1),(-2.,0.,1.,2.,8.),(-1.5,0.,1.5),(-1.,0.,1.))):
            values=[power,donut,zmul,(-1.,0.,1.)[index%3],mode]
            yield values+original[len(values):]
    elif name == '_coordvspow':
        # All 64 enabled/disabled component masks; signed weights and powers
        # are stratified, not a claim of exhaustive continuous combinations.
        for mask in range(64):
            weights=[(-100 if (mask+i)%2 else 100) if mask&(1<<i) else 0 for i in range(6)]
            values=[weights[0],(-2.,0.,1.,2.,8.)[mask%5],*weights[1:]]
            yield values+original[len(values):]
        for weight,power in itertools.product((-2147483648,0,2147483647),(-2.,0.,1.,2.,8.)):
            values=[weight,power,0,0,0,0,0];yield values+original[len(values):]
        for component,weight in itertools.product(range(1,6),(-2147483648,-100,-1,1,100,2147483647)):
            weights=[0]*6;weights[component]=weight
            values=[weights[0],1.,*weights[1:]];yield values+original[len(values):]
    elif name in ('IQ-bulb','IQ_PineTree','IQ_PineTree_NoYZ'):
        # Three independent numerical axes; saved integer aliases are separate.
        for power,angle,zmul in itertools.product((-2.,0.,1.,2.,8.),(-2.,0.,8.),(-1.,0.,1.)):
            yield [power,angle,zmul,0.,0.]+original[5:]
        for mode,atanmode in itertools.product((-2147483648,0,2147483647),repeat=2):
            yield [8.,8.,1.,mode,atanmode]+original[5:]
    elif name == 'IQ-bulb old':
        # Cross both real branch selectors and numerical power/angle signs.
        for index,(mode,atanmode,power,angle) in enumerate(itertools.product(
                (-2147483648,0,1,2147483647),(-2147483648,0,1,2147483647),(-2.,0.,8.),(-2.,0.,8.))):
            yield [power,angle,(-1.,0.,1.)[index%3],mode,atanmode]+original[5:]
    elif name == 'hexgridIFS':
        for index,(apply,rotation,spacing,styling) in enumerate(itertools.product(
                (-1,0,1),((0.,0.,0.),(37.,-61.,23.),(-90.,0.,90.),(180.,-180.,45.)),(-.3,0.,.3),(-1.5,0.,1.5))):
            values=[spacing,(-2.,0.,2.)[index%3],.125,-.25,.5,styling,apply,*rotation,(-180.,0.,37.)[(index//3)%3],.6,.1]
            yield values+original[len(values):]
    elif name == 'ABoxVaryScale2':
        for index,(radius,scale,rpower,xyzpower) in enumerate(itertools.product((0.,.5,1.),(-2.,0.,2.),(-1.,0.,1/3),(-2.,0.,6.))):
            values=[scale,radius,(-1.,0.,1.)[index%3],(-.1,0.,.1)[(index//3)%3],rpower,xyzpower]
            yield values+original[len(values):]
    elif name == 'Fuzzy':
        for power,ymul in itertools.product((-2.,0.,1.,2.,8.),(-1.,0.,1.)):
            yield [power,ymul]+original[2:]
    elif name == '_Sin4Dplus':
        for phase,strength in itertools.product(((0.,0.,0.,0.),(.125,-.25,.5,-.375),(1e12,-1e12,1e12,-1e12)),(-2.,0.,1.5)):
            values=[*phase,strength];yield values+original[len(values):]
    elif name == 'OctKoch':
        for index,(mode,scale,fold,rotation) in enumerate(itertools.product((-2147483648,0,2147483647),(-2.,0.,2.),(-1.,0.,1.),((0.,0.,0.),(37.,-61.,23.),(-90.,0.,90.),(180.,-180.,45.)))):
            values=[scale,.125,-.25,.5,*rotation,23.,-17.,61.,fold,(-2.,0.,2.)[index%3],mode]
            yield values+original[len(values):]
    elif name == 'loxodrome2IFS':
        for index,(style,apply,period,angle) in enumerate(itertools.product((-2147483648,-2,-1,0,1,2147483647),(-1,0,1),(-120.,0.,120.),(-30.,0.,30.))):
            values=[angle,(-2.,0.,2.)[index%3],.125,-.25,.5,(-1.,0.,1.)[(index//3)%3],apply,period,.125,.1,-1.,.5,style]
            yield values+original[len(values):]
    elif name == 'ABoxMapSFold':
        # Every map identifier/offset/scale crossing; fold cycles independently.
        # This verifies callback semantics, not map projection/interpolation.
        for index,(number,offset,maps,fold) in enumerate(itertools.product(
                (-2147483648,-1,0,1,7,2147483647),(-.5,0.,.5),(-1.,0.,1.),(-1.,0.,1.))):
            values=[(-2.,0.,2.)[index%3],(0.,.5,1.)[(index//3)%3],fold,number,offset,maps]
            yield values+original[len(values):]
    elif name in ('ABoxVS_icen1','ABoxVS_icen2'):
        # Both swap branches and aliases; independent scale/radius and inversion
        # radius or power. Signed fold/vary and asymmetric centers cycle.
        for index,(mode,scale,radius,inversion) in enumerate(itertools.product(
                (-2147483648,0,1,2147483647),(-2.,0.,2.),(0.,.5,1.),(-1.,0.,1.))):
            values=[scale,radius,(-1.,0.,1.)[index%3],(-.1,0.,.1)[(index//3)%3],
                    inversion,.125,-.25,.5,mode]
            yield values+original[len(values):]
    elif name in ('IQ_NormBulb','IQ_NormBulb_NoYZ'):
        # Original CODE ignores both saved integer options. Exercise aliases
        # separately from power/angle/multiplier boundaries.
        for power,angle,zmul in itertools.product((-2.,0.,1.,2.,8.),(-2.,0.,8.),(-1.,0.,1.)):
            yield [power,angle,zmul,0.,0.]+original[5:]
        for mode,atanmode in itertools.product((-2147483648,0,2147483647),repeat=2):
            yield [8.,8.,1.,mode,atanmode]+original[5:]
    elif name == 'Ikenagabulb':
        # Zero and axial coefficients exercise both product degeneracy paths;
        # asymmetric signed coefficients expose component/order errors.
        recipes=((0.,0.,0.),(1.,0.,0.),(0.,1.,0.),(0.,0.,1.),
                 (.3,-.2,.7),(-1.,2.,-3.))
        for a,b in itertools.product(recipes,repeat=2):
            yield list(a+b)+original[6:]
    elif name == 'wavespIFS':
        # Both radial modes plus aliases; commit and divide branches; exponent
        # boundaries. Geometry cycles through signed/zero values, not exhaustive.
        for index,(mode,apply,scale,exponent) in enumerate(itertools.product(
                (-1,0,1,2),(-1,0,1),(-2.,0.,2.),(-1.,0.,.5,.8,1.))):
            values=[(-2.,0.,3.)[index%3],scale,.125,-.25,.5,
                    (-.1,0.,.1)[(index//3)%3],apply,exponent,mode]
            yield values+original[len(values):]
    elif name == 'ABoxMod2':
        # Independent XY/Z fold signs and sphere/scale boundaries. Cylinder
        # caps cycle; this finite stratum does not cross every possible tuple.
        for index,(scale,radius,xy,z) in enumerate(itertools.product(
                (-2.,0.,2.),(0.,.5,1.),(-1.,0.,1.),(-1.,0.,1.))):
            values=[scale,radius,xy,z,(-.5,0.,.5)[(index//3)%3]]
            yield values+original[len(values):]
    elif name == 'DodecahedronIFSex':
        # All eight insertion points and signed/out-of-range aliases. Each has
        # asymmetric offsets, regular/singular normals and independent matrices.
        for index,(style,phi,rotated) in enumerate(itertools.product(
                (-2147483648,-1,*range(9),2147483647),(-1.,0.,1.,1.618),(False,True))):
            angles=(13.,-27.,41.,-61.,23.,37.) if rotated else (0.,)*6
            values=[(-2.,0.,2.)[index%3],.125,-.25,.5,*angles,phi,style]
            yield values+original[len(values):]
    elif name == 'ABoxVSWavy':
        # Both radius-expression branches and aliases; sphere-fold boundaries,
        # signed/zero scale and scale variation. No arbitrary hybrid claim.
        for index,(mode,scale,radius,vary) in enumerate(itertools.product(
                (-1,0,1,2),(-2.,0.,2.),(0.,.5,1.),(-.1,0.,.1))):
            values=[scale,radius,(-1.,0.,1.)[index%3],vary,
                    (-1.,0.,1.)[(index//3)%3],(.0,.5,1e12)[index%3],
                    (-.5,0.,.5)[(index//9)%3],(-1.,0.,1.)[(index//2)%3],mode]
            yield values+original[len(values):]
    elif name in ('gnarlyIFS','gnarly2IFS','gnarly3IFS'):
        # All loop/commit branches, signed scale and representative even aliases.
        # Odd Flip needs measured Pascal caller-stack context; not a point oracle.
        for index,(count,apply,scale) in enumerate(itertools.product(
                (-2147483648,-2,0,1,2,3,16,32),(-1,0,1),(-2.,0.,2.))):
            values=original.copy()
            values[1:9]=[scale,.125,-.25,.5,(-.1,0.,.1)[index%3],apply,count,
                         (-2147483648,-2,0,2,2147483646)[index%5]]
            values[12]=(-1.,0.,1.)[(index//3)%3]
            yield values
    elif name == 'BT1Pine':
        # Exhaust every byte value, then signed/high-bit aliases. Original loop
        # decrements after the body, so zero wraps and executes 256 times.
        for count in (*range(256),-2147483648,-257,-256,-1,256,257,2147483647):
            values=[.75,.125,1.,-.25,.5,count]
            yield values+original[len(values):]
    elif name == 'AmazingBox2':
        # Sphere fold boundaries and both independent pre-fold angle rotations.
        for index,(scale,radius,fold,rotation) in enumerate(itertools.product(
                (-2.,0.,2.),(0.,.5,1.),(-1.,0.,1.),((0.,0.,0.),(13.,-27.,41.)))):
            values=[scale,radius,fold,*rotation,.125,-.25,.5,
                    (-1.,0.,1.)[index%3],37.,-61.]
            yield values+original[len(values):]
    elif name == 'TetraFolding Pow2':
        # All mode dispatch arms and aliases; both folding arms; HRadius signs.
        for index,(mode,tglad,radius,rotation) in enumerate(itertools.product(
                (-1,0,1,2,3,4),(-1,0,1,2),(-1.,0.,1.),
                ((0.,0.,0.),(13.,-27.,41.)))):
            values=[1.,2.,-2.,radius,1.,(.0,.8660254037844386)[index%2],
                    *rotation,mode,tglad,(-1.,0.,1.)[index%3]]
            yield values+original[len(values):]
    elif name == 'ABoxPlatinum':
        # All low abs-mask bits, signed aliases, and sphere-fold boundaries.
        for index,(mask,scale,radius) in enumerate(itertools.product(
                (-2147483648,-1,*range(9),2147483647),(-2.,0.,2.),(0.,.5,1.))):
            values=[scale,radius,(-1.,0.,1.)[index%3],13.,-27.,41.,
                    .125,-.25,.5,(-1.,0.,1.)[(index//3)%3],37.,-61.,23.,-45.,mask]
            yield values+original[len(values):]
    elif name == 'CrossBiFold':
        for scale,fold,sub in itertools.product((-2.,0.,1.,2.),(-1.,0.,1.),(-3.,0.,3.)):
            values=[scale,.125,-.25,.5,13.,-27.,41.,-61.,23.,37.,fold,sub]
            yield values+original[len(values):]
    elif name == '_kamtorcut':
        # Compact polynomial recipes and signed zero/radial feedback.
        for s1,s2,recipe in itertools.product((-1.,0.,.3,1.),(-1.,0.,1.),range(3)):
            values=[s1,s2]+[[1.,0.,0.,0.,0.,0.,0.,-1.,1.,0.,0.,0.],
                           [.3,-.2,.7,-.5,.8,.1,-.4,.6,-.9,.2,-.3,.5],
                           [0.]*12][recipe]
            yield values+original[len(values):]
    elif name == 'RoomDoorsIFS':
        # Every sign/apply/door branch and acute/round/composite/depressed arc.
        for index,(order,period,wall,arc) in enumerate(itertools.product(
                (-4,0,4),(0,1,2,3),(-.1,.1),(-.3,0.,.2,.4))):
            yield [wall,1.,.65,(-2.,0.,2.)[index%3],.125,-.25,-2.,order,
                   .2,(0.,37.,-61.)[index%3],(0.,-23.,45.)[(index//3)%3],
                   period,arc,.1,.3,1.5]
    elif name == 'gyroidIFS':
        for index,(apply,scale,density) in enumerate(itertools.product(
                (-1,0,1),(-2.,0.,2.),(-1.,0.,1.,1e12))):
            values=[1.,scale,.125,-.25,.5,density,apply,13.,-27.,41.,-.75]
            yield values+original[len(values):]
    elif name == 'MsltoeSym4':
        for values in itertools.product((-1.,0.,1.,1e30),repeat=3):
            yield list(values)+original[3:]
    elif name == 'TalisTpx':
        for index,power in enumerate((-2147483648,-1,0,1,*range(2,24),2147483647)):
            values=[(-1.,0.,1.)[index%3],.3,-.2,.7,.8,-.3,.4,power]
            yield values+original[len(values):]
    elif name == 'ABoxVSShapes':
        # Every reachable shape arm plus invalid dispatch, independently of
        # sign/zero scale and shape coefficients. Compact boundary recipes.
        for index,(mode,recipe) in enumerate(itertools.product(range(-1,18),range(4))):
            values=[(-2.,0.,1e-12,2.)[recipe],(.25,0.,1.,-.25)[recipe],
                    1.,(-.1,0.,.1,1.)[recipe],mode,
                    (0.,.5,1.,-1.)[recipe],(1.,0.,.5,-1.)[recipe]]
            yield values+original[len(values):]
    elif name == 'sliceExIFS':
        for index,(boolean,apply,recipe) in enumerate(itertools.product(
                (-1,0,1,2,3),(-1,0,1),range(4))):
            values=[(-30.,0.,30.,360.)[recipe],(-2.,0.,1e-12,2.)[recipe],
                    .125,-.25,.5,1.,apply,(-120.,0.,120.,720.)[recipe],
                    .1,.2,(0.,-45.,45.,360.)[recipe],(0.,120.,-120.,720.)[recipe],boolean]
            yield values+original[len(values):]
    elif name == '_e_3Dtess':
        for index,(axiom,depth) in enumerate(itertools.product((-1,0,1,2,3,4),(-1,0,1,2,5,20))):
            values=[.125,-.25,.5,(-2.,0.,.5,2.)[index%4],
                    (-1.,0.,1.,2.)[(index//4)%4],(.5,1.,2.)[index%3],axiom,depth]
            yield values+original[len(values):]
    elif name == 'columnsIFS':
        # Both sign-bit branches (including -0), all apply outcomes, and
        # iteration-loop entry/limit boundaries without a huge cross product.
        for index,(radius,barrel,apply) in enumerate(itertools.product(
                (-.1,-0.,0.,.1),(-.6,-0.,0.,.6),(-1,0,1))):
            count=(-2147483648,-1,0,1,2,999,1000,1001,2147483647)[index%9]
            values=[radius,.25,1.,(-2.,0.,2.)[index%3],.125,-.25,.375,barrel,count,1.,3.,-.2,apply]
            yield values+original[len(values):]
        for count in (-2147483648,-1,0,1,2,999,1000,1001,2147483647):
            values=original[:];values[8]=count;yield values
    elif name == 'PiriformIFS':
        for index,(curve,flip,mirror,apply) in enumerate(itertools.product(
                (-1,0,1,2,3),(-1,0,1),(-1,0,1),(-1,0,1))):
            values=[-.3,(-2.,0.,2.)[index%3],.125,-.25,.375,.2,apply,curve,
                    (-1.,0.,.75)[(index//3)%3],flip,mirror]
            yield values+original[len(values):]
    elif name == '_helispiral':
        for index,(helix,spiral,rotation) in enumerate(itertools.product(
                (-1,0,1,2),(-1,0,1,2),((0.,0.,0.),(13.,-27.,41.)))):
            values=[*rotation,-.03,.07,helix,spiral,37.]
            yield values+original[len(values):]
    elif name == 'MengerKochV2':
        for index,(edge1,edge2,rotation) in enumerate(itertools.product(
                (-1.,0.,1.),(-1.,0.,1.),((0.,0.,0.),(13.,-27.,41.)))):
            values=[(-3.,0.,3.)[index%3],.2,-.4,.7,*rotation,41.,13.,-27.,edge1,edge2]
            yield values+original[len(values):]
    elif name == 'KaliLinCombSSE2sm':
        for power in (-2147483648,-3,-1,*range(11),2147483647):
            yield [power]+original[1:]
    elif name == '_AmazingBox':
        for scale, radius in itertools.product((-2.,0.,2.),(-1.,-0.,0.,.5,1.,2.)):
            yield [scale,radius]+original[2:]
    elif name == 'cubetubeIFS':
        # Every apply branch, signed/zero scale and two unequal rotations.
        for index, (apply, scale, angles) in enumerate(itertools.product(
                (-1,0,1), (-2.,0.,2.), ((0.,0.),(37.,-61.)))):
            values=[-.1 if index%2 else .1, .7, -1.3, scale,
                    .125,-.25,.375,*angles,apply,-.4,.6,-2.,3.]
            yield values+original[len(values):]
    elif name == '_MapTranslate':
        for number, scale in itertools.product((-1,0,1,10),(-.5,0.,.5)):
            values=[number,.125,-.25,.375,scale,-2.*scale,3.*scale]
            yield values+original[len(values):]
    elif name == 'ATetraVS':
        # Cover each dispatch branch and signed aliases, with two matrices.
        # Geometry varies by row instead of multiplying another Cartesian grid.
        for index, (mode, tglad, rotation) in enumerate(itertools.product(
                (-1, 0, 1, 2, 3, 4), (-1, 0, 1, 2),
                ((45., 45., 45.), (13., -27., 41.)))):
            values = [-1.7 if index % 3 == 0 else 1.7,
                      0. if index % 5 == 0 else .6, .8, -.2,
                      *rotation, mode, tglad]
            yield values + original[len(values):]
    elif name == 'CommaIFS':
        for index, (signs, flip_xz, flip_yz, apply) in enumerate(
                itertools.product(range(8), (0, 1), (0, 1), (0, 1))):
            values = [-.8, -2. if index % 3 == 0 else 2., .125, -.25, .375,
                      .24, apply, 1.5, .37, flip_xz, flip_yz, signs]
            yield values + original[len(values):]
    elif name == '_SphereFolding1':
        values=(-1e300,-4.,-2.,-1.,-1e-300,-0.,0.,1e-300,1.,2.,4.,1e300)
        for v in values:
            for value in (math.nextafter(v,-math.inf),v,math.nextafter(v,math.inf)):
                yield [value]+original[1:]
    elif name == 'ABoxSmoothFold':
        sharpness=(-2147483648,-1,*range(17),31,32,63,64,127,128,2147483647)
        for box,ball in itertools.product(sharpness,repeat=2):
            values=original[:]
            values[4]=box;values[6]=ball
            yield values
        for scale,radius,fold,vary,boxfix,ballfix in itertools.product(
                (-2.,0.,2.), (0.,.5,1.), (-1.,0.,1.), (-.25,0.,.25),
                (-1.,0.,1.), (-.3,0.,.3)):
            values=[scale,radius,fold,vary,6,boxfix,5,ballfix]
            yield values+original[len(values):]
    elif name == 'HeightMapIFS':
        # All masked channels plus signed aliases, signed map IDs/heights and
        # simultaneous rotations. Offsets/scales/trap settings are asymmetric.
        for height, trap, scale, rotation, number in itertools.product(
                (-2147483648,-1,0,1,2,3,2147483647),
                (-2147483648,-1,0,1,2,3,2147483647), (-.5,0.,.5),
                ((0.,0.,0.), (13.,-27.,41.), (90.,0.,-90.)), (-1,0,10)):
            values=[number,.125,-.25,.375,-.2,.3,scale,*rotation,height,trap,-.125,-2.]
            yield values+original[len(values):]
    elif name == 'CantorIFS':
        rotations = ((0.,0.,0.), (90.,0.,0.), (0.,90.,0.), (0.,0.,90.),
                     (13.,-27.,41.), (-180.,360.,-450.))
        for scale, offset, first, second in itertools.product(
                (-3.,0.,1.,3.), ((1.,1.,1.), (0.,1.,2.), (-1.,2.,-3.), (.125,-.25,.5)),
                rotations, rotations):
            values = [scale, *offset, *first, *second]
            yield values + original[len(values):]
    elif name == 'Amazing Surf':
        for selector, scale, radius, fold, vary, rotation in itertools.product(
                (-1,0,1,2147483647), (-2.,0.,2.), (0.,.5,1.), (-1.,0.,1.),
                (-.25,0.,.25,1.), ((0.,0.,0.), (13.,-27.,41.), (90.,0.,-90.))):
            values = [scale, radius, fold, *rotation, vary, selector]
            yield values + original[len(values):]
    elif name == 'Amazing Surf 2':
        for flags, fold, radius, scale in itertools.product(
                range(16), (-.5,0.,.5), (0.,.5), (-2.,0.,1e-12,2.)):
            values = [scale, radius, fold, 0., 0., 0., flags]
            yield values + original[len(values):]
    elif name == '_IFS_tess':
        variations=(-2147483648, -1, 0, 1, 2, 3, 4, 5, 2147483647)
        for variant, depth, angle, atom in itertools.product(variations,
                (-2147483648, -1, 0, 1, 49, 50, 51, 2147483647),
                (-90., -45., 0., 45., 90.), (-1., 0., 1.)):
            values=[.125, -.25, .5, 1., 1.707, atom, angle, depth, variant]
            yield values+original[len(values):]
        for variant, scale, disp, offsets, atom in itertools.product(variations,
                (-2., -0., 0., 2.), (-1.707, 0., 1.707),
                ((0.,0.,0.), (.125,-.25,.5), (-1.,2.,-3.)), (-1., 0., 1.)):
            values=[*offsets, scale, disp, atom, -45., 10, variant]
            yield values+original[len(values):]
    elif name == 'General':
        # Full crossing of both selectors and both tweaks (9216 scenes).
        for th, ph, tt, pt in itertools.product(range(24), range(24), range(4), range(4)):
            values = [8., 8., -1., th, tt, ph, pt, 0., 0., 0., 0.]
            yield values + original[len(values):]
        # Every selector, recipe and signed abs/C mode, with asymmetric powers.
        for th, recipe, flags in itertools.product(range(24), range(3), itertools.product((-1, 0, 1), repeat=3)):
            values = [2.5, -3., .75, th, th % 4, (th+7) % 24, (th+1) % 4, *flags, recipe]
            yield values + original[len(values):]
        for power, zpower, zmul, recipe in itertools.product((-2., -0., 0., .5, 8.), (-3., 0., 2.5), (-1., 0., 1.), range(3)):
            values = [power, zpower, zmul, 2, 3, 8, 2, -1, 0, 1, recipe]
            yield values + original[len(values):]
    elif name == 'KleinBotTIFS':
        for outer, inner, scale, thickness, apply, offsets in itertools.product(
                (-.6, -0., 0., .2, .6), (-.2, -0., 0., .2, .6),
                (-2., -0., 0., 2.), (-.05, -0., 0., .05), (-1, 0, 1),
                ((0.,0.,0.), (.125,-.25,.5), (-1.,2.,-3.))):
            values = [outer, scale, *offsets, inner, apply, thickness]
            yield values + original[len(values):]
    elif name == 'KnotsIFS':
        for order, twist1, twist2, apply in itertools.product(
                (-2147483648, -3, -1, 0, 1, 2, 3, 2147483647),
                (-2147483648, -3, -1, 0, 1, 3, 2147483647),
                (-2147483648, -3, -1, 0, 1, 3, 2147483647), (-1, 0, 1)):
            values = [2., 2., .125, -.25, .5, .4, apply, order, twist1, twist2, -.5]
            yield values + original[len(values):]
        for radius, scale, tube, inner, apply in itertools.product(
                (-2., 0., 2.), (-2., -0., 0., 2.), (-.4, 0., .4), (-.5, 0., .5), (-1, 0, 1)):
            values = [radius, scale, .125, -.25, .5, tube, apply, 3, -1, 2, inner]
            yield values + original[len(values):]
    elif name == 'ABoxPlatinumB':
        # Every low-three-bit mask, signed integer limits and simultaneous
        # nonzero fold angles; then signed sphere/fold/center/rotation strata.
        for angles, mask in itertools.product(itertools.product((-45., 0., 30.), repeat=4),
                (*range(8), -2147483648, -1, 2147483647)):
            values = [2., .5, 1., 37., -61., 23., .125, -.25, .5, 1., *angles, mask]
            yield values + original[len(values):]
        for scale, radius, fold, invradius, center, rotation in itertools.product(
                (-2., -0., 0., 2.), (-.5, 0., .5, 1., 2.), (-1., 0., 1.),
                (-1., 0., 1.), ((0., 0., 0.), (.125, -.25, .5), (-1., 2., -3.)),
                ((0.,0.,0.), (37.,-61.,23.), (-90.,0.,90.))):
            values = [scale, radius, fold, *rotation, *center, invradius, 17., -23., 31., -47., 5.]
            yield values + original[len(values):]
    elif name == 'ABoxSphereOffset4d':
        for scale, radius, fold, offsets, wadd in itertools.product(
                (-2., 0., 2.), (-.5, 0., .5, 1., 2.), (-1., 0., 1.),
                ((0., 0., 0., 0.), (.125, -.25, .5, -.75), (-1., 2., -3., 4.)),
                (-1., 0., 1.)):
            values = [scale, radius, fold, *offsets, wadd]
            yield values + original[len(values):]
    elif name == '_CelticMode':
        # Four independent runes, both zero signs and nonzero signed branches.
        for values in itertools.product((-2., -0., 0., .5, 2.), repeat=4):
            yield list(values) + original[4:]
    elif name == 'genIFS':
        for flags, scale, angles in itertools.product(
                itertools.product((-1, 0, 1), repeat=3), scales,
                ((0., 0., 0.), (37., -61., 23.), (-90., 0., 90.), (180., -180., 45.))):
            values = [scale, *flags, .125, -.25, .5, *angles]
            yield values + original[len(values):]
    elif name in ('OctahedronIFS', 'OctaFullIFS'):
        for axes, angles, scale in itertools.product(itertools.combinations(range(6), 2),
                itertools.product((-61., 0., 37.), repeat=2), scales):
            rotation = [0.] * 6
            for axis, angle in zip(axes, angles):
                rotation[axis] = angle
            values = [scale, .125, -.25, .5, *rotation]
            yield values + original[len(values):]
    elif name == 'BulboxP-2':
        # Two finite strata cover every option, including branch equality r=1.
        for scale, fold, radius, inner_scale, unsharp, disable in itertools.product(
                (-2., 0., 2.), (-1., 0., 1.), (-.6, 0., .6, 1., 2.),
                (-.5, 0., .5), (-.1, 0., .1), (-1, 0, 1)):
            values = [scale, fold, radius, inner_scale, unsharp, .125, -.25, .5, 1., disable]
            yield values + original[len(values):]
        for center, zmul, disable in itertools.product(
                itertools.product((-1., 0., 1.), repeat=3), (-1., -0., 0., 1.),
                (-2147483648, 0, 2147483647)):
            values = [2., 1., .6, -.5, .1, *center, zmul, disable]
            yield values + original[len(values):]
    elif name == 'Mandalay-KIFS':
        for scale, radius, fold, xy, z, rdiv, rotation in itertools.product(
                (-2., -0., 0., 2.), (-.5, 0., .5), (-1., 0., 1.),
                (-1., 0., 1.), (-1., 0., 1.), (-1., 0., 1.),
                ((0.,0.,0.), (37.,-61.,23.), (-90.,0.,90.))):
            values = [scale, radius, fold, *rotation, xy, z, rdiv]
            yield values + original[len(values):]
    elif name == 'Mandalex':
        # Independent finite strata: all pull/count combinations, then geometry.
        # Integer boundaries verify the mask, including negative/high-bit values.
        for pre, post, effect, fiddler in itertools.product(
                (-2., -0., 0., .5, 2.), (-2., -0., 0., .5, 2.),
                (-1., 0., 1.), (-2147483648, -1, 0, 1, 2, 3, 4, 2147483647)):
            values = [2., .25, 1., 37., -61., 23., pre, post, effect, fiddler]
            yield values + original[len(values):]
        for scale, radius, fold, rotation, fiddler in itertools.product(
                (-2., -0., 0., 2.), (-.5, 0., .5), (-1., 0., 1.),
                ((0.,0.,0.), (37.,-61.,23.), (-90.,0.,90.)), range(4)):
            values = [scale, radius, fold, *rotation, .5, 2., -1., fiddler]
            yield values + original[len(values):]
    elif name == 'MengerIFS':
        for scale, offsets, rotation in itertools.product(
                (-3., -1., -0., 0., .5, 1., 3.),
                itertools.product((-1., 0., .5, 1.), repeat=3),
                ((0.,0.,0.), (37.,-61.,23.), (-90.,0.,90.), (180.,-180.,45.))):
            values = [scale, *offsets, *rotation]
            yield values + original[len(values):]
    elif name in ('hextgridIFS', 'hextgrid2IFS', 'hextgrid3IFS', 'hextgrid4IFS'):
        for radius, styling, apply, scale, rotation in itertools.product(
                (-1., 0., 1.), (-.12, 0., .12, 1.), (-1, 0, 1), scales,
                ((0., 0., 0.), (37., -61., 23.), (-90., 0., 90.), (180., -180., 45.))):
            values = [radius, scale, .125, -.25, .5, styling, apply, *rotation]
            yield values + original[len(values):]
    elif name == 'SphCageIFS':
        for parallels, meridians, apply, scale, radius in itertools.product(
                (-2147483648, -1, 0, 1, 2, 3, 999, 1000, 1001, 2147483647),
                (-2147483648, -3, -1, 0, 1, 3, 2147483647),
                (-1, 0, 1), scales, (-.5, 0., .5)):
            values = [radius, scale, .125, -.25, .5, .01, apply, parallels, meridians]
            yield values + original[len(values):]
    elif name == 'discoballIFS':
        for xy, xz, apply, scale, thickness in itertools.product(
                (-2147483648, -3, -1, 0, 1, 3, 2147483647),
                (-2147483648, -3, -1, 0, 1, 3, 2147483647),
                (-1, 0, 1), scales, (-.2, 0., .2)):
            values = [xy, xz, scale, .125, -.25, .5, apply, 1., thickness]
            yield values + original[len(values):]
    elif name == 'transLogSpIFS':
        counters = (-2147483648, -1, 0, 1, 2147483647)
        for bend, arms, tightness, radius, rotation in itertools.product(
                counters, counters, (-1., 0., 1e-12, .4), (-1., 0., 1e-12, 1.),
                ((0., 0., 0.), (37., -61., 23.), (-90., 0., 90.))):
            values = [.125, -.25, .5, *rotation, bend, arms, tightness, radius]
            yield values + original[len(values):]
    elif name in ('gearIFS', 'gear2IFS'):
        for angle, period, apply, scale, thickness in itertools.product(
                (-30., 0., 30., 360.), (-120., 0., 120., 720.), (-1, 0, 1),
                scales, (-.2, 0., .2)):
            for second in ((.8,) if name == 'gearIFS' else (-60., 0., 60., 360.)):
                values = [angle, scale, .125, -.25, .5, 1., apply,
                          period, thickness, .2, second, thickness * .5]
                yield values + original[len(values):]
    else:
        for normals, radius, apply, scale in itertools.product(
                itertools.product((-1., 0., 1.), repeat=3), (-1.5, -0., 0., 1.5),
                (-1, 0, 1), scales):
            values = [.75, scale, .125, -.25, .5, apply, radius, *normals]
            yield values + original[len(values):]


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--manifest', type=Path, required=True)
    ap.add_argument('--output', type=Path, required=True)
    ap.add_argument('--formula', action='append', choices=(
        'CylinderHeightMap', 'CylinderHMVoid', 'TorusHMVoid',
        'HeightMap2IFS', 'SphereHeightMap', 'SphereHMVoid', 'MandelView', 'Riemann', 'Magnet4D', 'Mandel4DBiC', 'DevilIFS', 'CitrusIFS', '_BPolygonFromCircle', '_BPolygonToCircle', '_BPolyhedronToSphere', '_BStellahedron_F_Sphere', '_Fold45double', '_reciprocalX3', '_reciprocalY3', '_reciprocalZ3', '_AmazingBoxSSE2', 'ABoxVaryScale', 'ABoxVaryScale4d', '_Folding2', '_SphereFolding2', 'KaliDucks', 'Kalisets1', 'dodecatubeIFS', 'tetratubeIFS', 'logspIFS', 'ducksIFS',
        'sliceIFS', '_coordvspow', 'Aex-TorusPow', 'diniIFS', 'QuadricsIFS',
        'IQ-bulb', 'IQ-bulb old', 'IQ_PineTree', 'IQ_PineTree_NoYZ', 'hexgridIFS', 'ABoxVaryScale2', 'Fuzzy', '_Sin4Dplus', 'OctKoch', 'loxodrome2IFS', 'ABoxMapSFold', 'ABoxVS_icen1', 'ABoxVS_icen2', 'IQ_NormBulb', 'IQ_NormBulb_NoYZ', 'Ikenagabulb', 'wavespIFS', 'ABoxMod2', 'DodecahedronIFSex', 'ABoxVSWavy', 'gnarlyIFS', 'gnarly2IFS', 'gnarly3IFS', 'BT1Pine', 'AmazingBox2', 'TetraFolding Pow2', 'ABoxPlatinum', 'CrossBiFold', '_kamtorcut',
        'RoomDoorsIFS', 'gyroidIFS', 'MsltoeSym4', 'TalisTpx', 'ABoxVSShapes', 'sliceExIFS', '_e_3Dtess', 'columnsIFS', 'PiriformIFS', '_helispiral', 'MengerKochV2', 'KaliLinCombSSE2sm', '_AmazingBox', 'cubetubeIFS', '_MapTranslate', 'ATetraVS', 'CommaIFS', '_SphereFolding1', 'ABoxSmoothFold', 'HeightMapIFS', 'CantorIFS', 'Amazing Surf', 'Amazing Surf 2',
        'General',
        '_IFS_tess',
        'KleinBotTIFS', 'KnotsIFS', 'ABoxPlatinumB', 'ABoxSphereOffset4d', '_CelticMode', 'genIFS', 'OctahedronIFS', 'OctaFullIFS', 'MengerIFS', 'Mandalex', 'Mandalay-KIFS', 'BulboxP-2', 'foldcut8-IFS', 'gearIFS', 'gear2IFS', 'hextgridIFS', 'hextgrid2IFS', 'hextgrid3IFS', 'hextgrid4IFS', 'discoballIFS', 'SphCageIFS', 'transLogSpIFS'))
    a = ap.parse_args()
    bases = json.loads(a.manifest.read_bytes())['cases']
    a.output.mkdir(parents=True, exist_ok=False)
    cases = []
    for name in a.formula or ('genIFS', 'OctahedronIFS', 'OctaFullIFS', 'foldcut8-IFS'):
        base = next(c for c in bases if c['id'] == 'challenge_' + name + '_early')
        source = ROOT / base['scene']
        if digest(source.read_bytes()) != base['sha256']:
            raise ValueError('Changed source')
        data = decode(source)
        slot = next(s for s in slots(data) if s['name'] == name)
        slot = prepare_external_options(data, slot)
        for index, options in enumerate(variants(name, slot['options'])):
            scene = bytearray(data)
            struct.pack_into('<16d', scene, 848 + slot['slot'] * 188 + 60, *options)
            case = f'ifs_parameters_{name}_{index:04d}'
            path = a.output / (case + '.m3p')
            path.write_bytes(scene)
            cases.append(dict(id=case, target_formulas=[name], options=options,
                scene=str(path.resolve().relative_to(ROOT)), sha256=digest(scene),
                source=base['scene'], source_sha256=base['sha256']))
    write_json(a.output / 'manifest.json', dict(scope=__doc__, cases=cases))
    print(len(cases), 'scenes')


if __name__ == '__main__':
    main()
