// Original x87 instruction/store order on Win64; portable fallback is not certified.
constexpr unsigned int FORMULA_ROOMDOORSIFS = 10309;

// ASM trace summary:
// IFS preamble (0x00-0x2D): Non-standard! Uses Version=6 dIFS layout.
//   xmm0 = [esi-0x70] = (y, z)
//   xmm1 = [esi-0x78] = x
//   After the SSE preamble the x87 stack is addressed as:
//     [esp+0x00] = y
//     [esp+0x08] = z
//     [esp+0x10] = x
//   Multiply by Scale [edi-0x30], conditional de*=Scale if PFOrder >= 0
//   Note: PFOrder is at [edi-0x4c] and acts like Apply_scale_add but signed.
//   If PFOrder < 0: skip scale+add of position
//
// 0x38-0x97: Polyfold rotation — snap angle and rotate to face nearest wall:
//   theta = atan2(x, y)  (old dIFS stack order, not the usual x/z pair)
//   snap to nearest wall: round(theta * inv_twopi * |PFOrder|) / |PFOrder| * twopi
//   sincos of snapped angle
//   Rotate (x, y) by snapped angle → get (wall_normal, wall_tangent)
//   Add Door_pos to wall_normal, Wall_pos to wall_tangent
//   Store rotated + offset coordinates
//   Update z with Z_add
//
// 0x9E-0xB9: Also store back to esi if PFOrder >= 0 (update position)
//
// 0xB9-0xE7: Apply Wall_Rho rotation to (z, wall_tangent):
//   Two 2D rotations via DoubleAngle cos/sin pairs
//   rot1: (z, wall_tangent) by Wall_Rho → new local_y, wall_normal_component
//
// 0xE7-0xF6: Wall height check:
//   |wall_tangent| - |WofWall| → neg(result) = wall thickness DE component
//
// 0x101-0x1B2: Door computation:
//   If DoorPeriod != 0: check if current wall index mod DoorPeriod == 1
//     If not door wall → skip door, just use wall DE
//   If door wall:
//     door_width_de = |x_local| * cos(DoorSidesRho) - y_local * sin(DoorSidesRho) - WofDoor
//     door_height = y_local - (-HofDoor)
//     Arc computation for door top:
//       If ArcR <= 0: arc center = ArcR, arc_offset = 0
//       Else: arc center = WofDoor - ArcR, arc_offset = ArcR
//     arc_de = sqrt(door_height² + (|x_local| - arc_offset)²) - arc_center
//     DE = min(max(wall_de, door_width_de, arc_de), ...)
//
// 0x1B2-0x22C: Wall + roof logic:
//   wall_de = negate result from above
//   If WofWall < 0 (roof mode):
//     roof_de = |y + HofWall| + HofWall (distance to roof surface)
//     Check OTrapRoof for coloring
//     DE = max(wall_de, roof_de)
//   Else:
//     wall_y_de = |y + HofWall*0.5| - HofWall*0.5
//     DE = max(DE, wall_y_de)
//
// 0x22C-0x27A: Final wall height:
//   half_wall = HofWall * const[0] (0.5)
//   wall_height_de = |y + half_wall| - half_wall
//   shape_de = max(accumulated_de, wall_height_de)
//   Scale correction if PFOrder < 0

#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void RoomDoorsOriginal_x87(double*, const double*);
#endif

template<>
cpu_double3 execute_formula<FORMULA_ROOMDOORSIFS>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    const int order=int(params[7]), period=int(params[13]), iteration=int(params[ITER_COUNT_SLOT]);
    double state[14]={p.y*params[3],p.z*params[3],p.x*params[3],0.,0.,0.,p.x,p.y,p.z,0.,0.,params[DIFS_ROLD_SLOT]};
    std::memcpy(reinterpret_cast<char*>(state)+96,&iteration,4);
    std::memcpy(reinterpret_cast<char*>(state)+100,&order,4);
    std::memcpy(reinterpret_cast<char*>(state)+104,&period,4);
    if (order>=0) params[DIFS_VARYSCALE_SLOT]*=params[3];
    RoomDoorsOriginal_x87(state,params);
    de=state[9]; params[MAX_FORMULA_PARAMS-1]=state[10];
    return cpu_double3(state[6],state[7],state[8]);
#elif defined(MB3D_SOFTWARE_EXT80)
    using E = mb3d::numeric::Extended80;
    mb3d::numeric::ExtendedScope scope;
    const int32_t order=int(params[7]), period=int(params[13]);
    // Version 6's SSE preamble is in Y/Z/X order, with Double products.
    const double ys=p.y*params[3], zs=p.z*params[3], xs=p.x*params[3];
    const E angle=mb3d::numeric::extendedAtan2(E(xs),E(ys));
    const int32_t sector=((E(params[21])-angle)*params[19]*E(order)).integer();
    const auto sc=mb3d::numeric::extendedSinCos((E(sector)/E(order))*params[20]);
    const double c=double(sc.cos), s=double(sc.sin);
    const double tangent=double((E(xs)*c+(-(E(ys)*s)))+params[6]);
    const double normal=double((E(xs)*s+E(ys)*c)+params[5]);
    const double height=double(E(zs)+params[4]);
    if (order >= 0) {
        params[DIFS_VARYSCALE_SLOT]*=params[3];
        p=cpu_double3(tangent,normal,height);
    }
    // Wall rotation has Double stores, then MOVUPD/MOVSD restore the
    // pre-rotation normal/height/tangent before the door and roof sections.
    const double wall=double(E(height)*params[9]+E(tangent)*params[10]);
    E distance=-(abs(E(wall))-abs(E(params[0])));
    const uint32_t wallIndex=uint32_t(sector)+uint32_t(256);
    if (period <= 0 || wallIndex%uint32_t(period) == 1) {
        const E sides=(abs(E(normal))*params[12]+(-(E(height)*params[11])))-params[8];
        const E top=-E(params[2])-height;
        double center=params[14], offset=0;
        if (E(0) >= E(params[14])) {
            center=double(E(params[8])-params[14]); offset=params[14];
        }
        const E dx=abs(E(normal))-offset;
        const E arc=sqrt(top*top+dx*dx)-center;
        E door=!(arc >= top) ? arc : top;
        door=door >= sides ? door : sides;
        distance=!(door >= distance) ? door : distance;
    }
    distance=-distance;
    bool roofTrap=false;
    if (std::signbit(params[0])) {
        const E heightRoof=abs(E(height)+params[1])+params[0];
        const E wallRoof=E(wall)+params[0];
        const E roof=wallRoof >= heightRoof ? wallRoof : heightRoof;
        if (E(0) >= roof+E(params[22])*params[DIFS_ROLD_SLOT]
            && !(E(0) >= E(params[17]))) {
            roofTrap=true;
            params[MAX_FORMULA_PARAMS-1]=params[17];
        }
        distance=!(roof >= distance) ? roof : distance;
    }
    const E half=E(params[1])*params[18];
    const E heightDistance=abs(E(height)+half)-double(half);
    distance=heightDistance >= distance ? heightDistance : distance;
    if (!roofTrap)
        params[MAX_FORMULA_PARAMS-1]=double(E(int32_t(params[ITER_COUNT_SLOT]))*params[15]+params[16]);
    if (order < 0) distance/=params[3];
    de=double(distance);
    return p;
#else
    // Parameters from .m3
    const double WofWall = params[0];              // [PVar-0x10] Double WofWall (- for roof) = 0.1
    const double HofWall = params[1];              // [PVar-0x18] Double HofWall = 1.
    const double HofDoor = params[2];              // [PVar-0x20] Double HofDoor = .65
    const double Scale_val = params[3];            // [PVar-0x30] .2Doubles Scale = 2.
    const double Z_add = params[4];                // [PVar-0x38] Double Z add = 0.
    const double Door_pos = params[5];             // [PVar-0x40] Double Door pos (Y+) = 0.
    const double Wall_pos = params[6];             // [PVar-0x48] Double Wall pos (X+) = -2.
    const int PFOrder = int(params[7]);           // [PVar-0x4c] Integer PFOrder = -4
    const double WofDoor = params[8];              // [PVar-0x54] Double WofDoor = .2
    // Generated value order for .DoubleAngle is (sin, cos) at the value level.
    const double sinWallRho = params[9];           // [PVar-0x5c] DoubleAngle Wall Rho sin
    const double cosWallRho = params[10];          // [PVar-0x64] DoubleAngle Wall Rho cos
    const double sinDoorRho = params[11];          // [PVar-0x6c] DoubleAngle DoorSides Rho sin
    const double cosDoorRho = params[12];          // [PVar-0x74] DoubleAngle DoorSides Rho cos
    const int DoorPeriod = int(params[13]);       // [PVar-0x78] Integer DoorPeriod = 0
    const double ArcR = params[14];                // [PVar-0x80] Double ArcR = 1e-25
    const double OTrapOnIts = params[15];          // [PVar-0x88] Double OTrapOnIts = .1
    const double OTrapOffset = params[16];         // [PVar-0x90] Double OTrapOffset = 0
    const double OTrapRoof = params[17];           // [PVar-0x98] Double OTrapRoof (neg OFF) = -1
    // CONSTANTS:
    // [edi+0x00] = 0.5
    // [edi+0x08] = 0.159154943 (1/2pi)
    // [edi+0x10] = -6.283185307 (-2pi)
    // [edi+0x18] = 1.57079632675 (pi/2)
    // [edi+0x20] = -30

    const double half_val = 0.5;
    const double inv_twopi = 0.159154943;
    const double neg_twopi = -6.283185307;
    const double half_pi = 1.57079632675;

    double state_x = p.x;
    double state_y = p.y;
    double state_z = p.z;

    // Step 1: IFS transform (non-standard Version=6 dIFS preamble)
    // SSE loads [esi-0x70]=(y,z) and [esi-0x78]=x.
    double y_pos = p.y * Scale_val;
    double z_pos = p.z * Scale_val;
    double x_pos = p.x * Scale_val;

    int signedPFOrder = PFOrder;
    if (signedPFOrder == 0) signedPFOrder = 1;

    if (PFOrder >= 0) {
        de *= Scale_val;
        params[DIFS_COMMIT_XYZ_SLOT] = 1.0;
        state_x = x_pos;
        state_y = y_pos;
        state_z = z_pos;
    }

    // Step 2: Polyfold rotation in the old dIFS stack order.
    // ASM 0x38-0x97:
    //   theta = atan2(x_pos, y_pos)
    //   snapped_n = round((half_pi - theta) * inv_twopi * PFOrder)
    //   snapped_angle = (snapped_n / PFOrder) * neg_twopi  (note: neg_twopi = -2pi)
    //   sincos(snapped_angle) → cos_s, sin_s
    //   new_tangent = x_pos * cos_s - y_pos * sin_s + Wall_pos
    //   new_normal = x_pos * sin_s + y_pos * cos_s + Door_pos
    //   z_new = z_pos + Z_add

    double theta = atan2(x_pos, y_pos);
    double snap_idx_f = (half_pi - theta) * inv_twopi * double(signedPFOrder);
    int snap_idx = x87_fistp_int32(snap_idx_f);  // fistp → nearest-even int
    double snapped_angle = (double(snap_idx) / double(signedPFOrder)) * neg_twopi;
    double cos_s = cos(snapped_angle);
    double sin_s = sin(snapped_angle);

    // ASM rotation:
    //   new1 = x*cos - y*sin + Wall_pos  (stored as [esp+0x10] = tangent)
    //   new2 = x*sin + y*cos + Door_pos  (stored as [esp] = normal)
    //   z += Z_add                        (stored as [esp+0x08])
    double wall_tangent = x_pos * cos_s - y_pos * sin_s + Wall_pos;
    double wall_normal = x_pos * sin_s + y_pos * cos_s + Door_pos;
    double wall_z = z_pos + Z_add;

    // Store transformed position
    double sx = wall_normal;   // [esp]
    double sy = wall_z;        // [esp+8]
    double sz = wall_tangent;  // [esp+0x10]

    // Step 3: Apply Wall_Rho rotation to (z, tangent)
    // ASM 0xB9-0xD0:
    //   r1 = z * sinWallRho + tangent * cosWallRho
    //   r2 = z * cosWallRho - tangent * sinWallRho
    double wr1 = sy * sinWallRho + sz * cosWallRho;
    double wr2 = sy * cosWallRho - sz * sinWallRho;
    // ASM stores:
    //   [esp+0x08] = wr2  (local door/wall-height axis)
    //   [esp+0x10] = wr1  (wall-thickness axis)
    //   [esp+0x20] = wr1  (aux copy for roof path)
    double wall_comp = wr1;
    double local_y = wr2;

    // Step 4: Wall thickness DE
    // ASM 0xE7-0xF6:
    //   |wr1| - |WofWall| → negate → wall_thickness_de
    double wall_thick_de = -(fabs(wall_comp) - fabs(WofWall));

    // Step 5: Door computation
    // ASM 0x101-0x1B2: complex door logic
    // Check DoorPeriod: if != 0, check if snap_idx mod DoorPeriod != 1 → skip door
    bool has_door = true;
    if (DoorPeriod > 0) {
        int wall_idx = (snap_idx + 256);  // ASM adds 0x100 to make positive
        int remainder = wall_idx % DoorPeriod;
        if (remainder != 1) {
            has_door = false;
        }
    }

    double shape_de = wall_thick_de;

    if (has_door) {
        // Door width DE:
        //   |sx| * cosDoorRho - wr2 * sinDoorRho → subtract WofDoor → door_sides_de
        double door_sides = fabs(sx) * cosDoorRho - local_y * sinDoorRho - WofDoor;

        // Door height: wr2 - (-HofDoor) = wr2 + HofDoor → then neg → -(wr2 + HofDoor)
        double door_top = -(local_y + HofDoor);

        // Arc computation
        double arc_center, arc_offset;
        if (ArcR <= 0.0) {
            // Acute/round arc
            arc_center = ArcR;      // [esp+0x18]
            arc_offset = 0.0;       // [esp+0x28]
        } else {
            // Depressed/composite arc
            arc_center = WofDoor - ArcR;   // [esp+0x18]
            arc_offset = ArcR;             // [esp+0x28]
        }

        // Arc DE = sqrt(door_top² + (|sx| - arc_offset)²) - arc_center
        double arc_de = sqrt(door_top * door_top +
                           (fabs(sx) - arc_offset) * (fabs(sx) - arc_offset))
                      - arc_center;

        // ASM compare ladder at 0x185..0x1B2:
        // 1. min(arc_de, door_top)
        // 2. max(result, door_sides)
        // 3. min(result, wall_thick_de)
        // 4. negate
        double shape_inner = min(arc_de, door_top);
        shape_inner = max(shape_inner, door_sides);
        shape_de = min(shape_inner, wall_thick_de);
    }

    // Step 7: Negate (ASM at 0x1B2: fchs)
    shape_de = -shape_de;

    // Step 8: Wall height (roof/floor)
    // ASM 0x1BF-0x22C:
    // If WofWall < 0 (sign bit set → roof mode):
    if (WofWall < 0.0) {
        // Roof mode: add height constraint
        // ASM uses [esp+8], which already holds wr2 after Wall_Rho rotation.
        double roof_de = fabs(local_y + HofWall) + HofWall;
        // Compare with wall_comp + HofWall
        double comp_de = wall_comp + HofWall;
        if (comp_de < roof_de) roof_de = comp_de;

        // OTrapRoof coloring check
        // If OTrapRoof > 0: check if roof, set orbit trap

        // shape_de = max(shape_de, roof_de)
        if (roof_de > shape_de) shape_de = roof_de;
    }

    // Step 9: Final wall height constraint
    // half_wall = HofWall * 0.5
    double half_wall = HofWall * half_val;
    // ASM 0x22C loads [esp+8], i.e. wr2 after the Wall_Rho rotation.
    double wall_height_de = fabs(local_y + half_wall) - half_wall;

    // shape_de = max(shape_de, wall_height_de)
    // ASM: fcom, comparison
    if (wall_height_de > shape_de) {
        shape_de = wall_height_de;
    }

    // Orbit trap (default path)
    // ASM at 0x255: iter * OTrapOnIts + OTrapOffset

    // Step 10: Scale correction
    if (PFOrder < 0) {
        shape_de /= Scale_val;
    } else {
        state_x = wall_normal;
        state_y = wall_z;
        state_z = wall_tangent;
    }

    de = shape_de;
    params[MAX_FORMULA_PARAMS - 1] =
        params[ITER_COUNT_SLOT] * OTrapOnIts + OTrapOffset;

    return cpu_double3(state_x, state_y, state_z);
#endif
}

cpu_double3 RoomDoorsIFS_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_ROOMDOORSIFS>(p,params,de);
}

constant FormulaInfo formula_info_RoomDoorsIFS = {
    .id = FORMULA_ROOMDOORSIFS,
    .name = "RoomDoorsIFS",
    .param_count = 18,
    .complexity = 635,
    .uses_transcendental = true
};

/*
Original M3 Description:
Tried to make a very flexible tool for generating "realistic" rooms, hope it helps.

- WofWall; If positive, there will be no roof, else there will be one. This is the
  thickness of walls / 2, and the thickness of the roof itself.
- HofWall; Using z as a fixed reference, this will be the total height of the room.
- HofDoor, total height of each door, IF ArcR is left at its default value (positive & near to 0).
- Door,Wall pos; Will also modify x and y values if PFOrder > 0. PForder works as normal
  "Apply scale + add" but also changes room's shape of course.
- WofDoor; Make the doors bigger/smaller.
- Wall rho; Useful to create pyramids or strange walls.
- DS rho; Makes the doors trapezoidal.
- Door period; Instead of a door for each wall, you can see a door for each x walls, 0 to disable.
- ArcR; Has nice effects depending on its value compared to WoD;
  - if AR < 0, the arc will be acute
  - if AR = 0, the arc will be round
  - if AR < WoD, the arc will be composite
  - if AR = WoD, the arc will be round
  - if AR > WoD, the arc will be depressed
- OTrapRoof; You can choose a different color for the roof. Disable with neg values.

Luca GN 2012

The formula uses polyfold (angular snapping) to create repeating walls in a
cylindrical arrangement, then cuts doors into selected walls with configurable
arches. The PFOrder parameter controls both the number of walls and whether
the IFS scale/add is applied.

Parameters:
  [0] WofWall: double (default: 0.1) — wall thickness/2, negative enables roof
  [1] HofWall: double (default: 1.0) — room height
  [2] HofDoor: double (default: 0.65) — door height
  [3] Scale: double (default: 2.0) — IFS scale (.2Doubles)
  [4] Z add: double (default: 0.0)
  [5] Door pos: double (default: 0.0) — door Y offset
  [6] Wall pos: double (default: -2.0) — wall X offset
  [7] PFOrder: int (default: -4) — number of walls, sign controls scale+add
  [8] WofDoor: double (default: 0.2) — door width
  [9] cos(Wall Rho): DoubleAngle
  [10] sin(Wall Rho): DoubleAngle
  [11] cos(DoorSides Rho): DoubleAngle
  [12] sin(DoorSides Rho): DoubleAngle
  [13] DoorPeriod: int (default: 0) — door every N walls, 0=all walls have doors
  [14] ArcR: double (default: 1e-25) — arc radius
  [15] OTrapOnIts: double (default: 0.1)
  [16] OTrapOffset: double (default: 0)
  [17] OTrapRoof: double (default: -1) — roof coloring, negative=off

CONSTANTS:
  [0] 0.5
  [1] 1/(2*pi) = 0.159154943
  [2] -2*pi = -6.283185307
  [3] pi/2 = 1.57079632675
  [4] -30
*/
