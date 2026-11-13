// Barnsley4Dc.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: Barnsley4Dc
// Hand-ported from ASM: 484 bytes, DEoption=4
// 4D formula: uses de parameter as w coordinate input/output
// Commutative quaternion Barnsley conditional formula

constexpr unsigned int FORMULA_BARNSLEY4DC = 10337;

// Original CODE uses internal quaternion multiplication by C. The reflected
// (tent) and subtract (Barnsley) branches alone apply ConjMode/FlipMode.
// Parameters 0..8 are options, 9..10 constants, 11..14 engine C values.

#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void Barnsley4DcOriginal_x87(double*,const double*,const int*);
#endif

template<>
cpu_double3 execute_formula<FORMULA_BARNSLEY4DC>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    double state[]={p.x,p.y,p.z,de,0.,0.};
    const int flags[]={int(params[6]),int(params[7]),int(params[8])};
    Barnsley4DcOriginal_x87(state,params,flags);
    de=state[3];
    return cpu_double3(state[0],state[1],state[2]);
#elif defined(MB3D_SOFTWARE_EXT80)
    using E=numeric::Extended80;
    const numeric::ExtendedScope scope;
    E x(p.x),y(p.y),z(p.z),w(de);
    const E dx(params[0]),dy(params[1]),dz(params[2]),dw(params[3]),addW(params[4]);
    bool adjustC;
    if(int(params[8])!=0) {
        const E threshold=sqrt(((dx*dx+dy*dy)+dz*dz)+dw*dw)*E(params[9]);
        // AND AH,41h clears CF; JA follows equality/unordered as well.
        adjustC=!(x<threshold);
        const E scale(params[10]);
        if(adjustC) { x=(dx-x)*scale;y=(dy-y)*scale;z=(dz-z)*scale;w=((dw-w)+addW)*scale; }
        else { x*=scale;y*=scale;z*=scale;w=w*scale+addW; }
    } else {
        adjustC=!(x<E(params[5]));
        if(adjustC) { x-=dx;y-=dy;z-=dz;w=(w-dw)+addW; }
        else { x+=dx;y+=dy;z+=dz;w=(w+dw)+addW; }
    }
    // Every branch stores XYZW before reloading for the quaternion product.
    x=E(double(x));y=E(double(y));z=E(double(z));w=E(double(w));
    E cx(params[11]),cy(params[12]);
    const E cz(params[13]),cw(params[14]);
    if(adjustC && int(params[6])!=0) cy=-cy;
    if(adjustC && int(params[7])!=0) {const E old=cx;cx=cy;cy=old;}
    const E nx=((x*cx+-(y*cy))+-(z*cz))+w*cw;
    const E ny=((y*cx+x*cy)+-(w*cz))+-(z*cw);
    const E nz=((z*cx+-(w*cy))+x*cz)+-(y*cw);
    const E nw=((w*cx+z*cy)+y*cz)+x*cw;
    de=double(nw);
    return cpu_double3(double(nx),double(ny),double(nz));
#else
    // Portable arithmetic: branch semantics corrected, rounding parity unverified.
    const double dx = params[0];         // [PVar-0x10]
    const double dy = params[1];         // [PVar-0x18]
    const double dz = params[2];         // [PVar-0x20]
    const double dw = params[3];         // [PVar-0x28]
    const double w_add = params[4];      // [PVar-0x30]
    const double Threshold = params[5];  // [PVar-0x38]
    const int ConjMode = int(params[6]);  // [PVar-0x3C]
    const int FlipMode = int(params[7]);  // [PVar-0x40]
    const int TentMode = int(params[8]);  // [PVar-0x44]

    const double Cx = params[11];  // [esi+0x18]
    const double Cy = params[12];  // [esi+0x20]
    const double Cz = params[13];  // [esi+0x28]
    const double Cw = params[14];  // [esi-0x38]

    double x = p.x, y = p.y, z = p.z, w = de;

    bool adjustC = false;
    if (TentMode != 0) {
        double r = sqrt(dx*dx + dy*dy + dz*dz + dw*dw) * 0.5;
        if (!(x <= r)) {
            adjustC = true;
            x = (dx - x) * 2.0;
            y = (dy - y) * 2.0;
            z = (dz - z) * 2.0;
            w = (dw - w + w_add) * 2.0;
        } else {
            x = x * 2.0;
            y = y * 2.0;
            z = z * 2.0;
            w = w * 2.0 + w_add;
        }
    } else {
        if (!(x <= Threshold)) {
            adjustC = true;
            x = x - dx;
            y = y - dy;
            z = z - dz;
            w = w - dw + w_add;
        } else {
            x = x + dx;
            y = y + dy;
            z = z + dz;
            w = w + dw + w_add;
        }
    }

    double tmp1 = Cy;
    double tmp2 = Cx;

    if (adjustC && ConjMode != 0) {
        tmp1 = -Cy;
    }

    if (adjustC && FlipMode != 0) {
        double t = tmp1;
        tmp1 = tmp2;
        tmp2 = t;
    }


    double new_x = x*tmp2 - y*tmp1 - z*Cz + w*Cw;
    double new_y = y*tmp2 + x*tmp1 - w*Cz - z*Cw;
    double new_z = z*tmp2 - w*tmp1 + x*Cz - y*Cw;
    double new_w = w*tmp2 + z*tmp1 + y*Cz + x*Cw;

    de = new_w;
    return cpu_double3(new_x, new_y, new_z);
#endif
}

// Formula metadata
constant FormulaInfo formula_info_Barnsley4Dc = {
    .id = FORMULA_BARNSLEY4DC,
    .name = "Barnsley4Dc",
    .param_count = 13,
    .complexity = 484,
    .uses_transcendental = false
};

/*
Original M3 Description:
This is a 4-D extension of the (generalized) Barnsley conditional formula.
This implementation uses a commutative quat product.

S is the quaternion (x,y,z,w+wadd), K is (dx,dy,dz,dw), C is (Cx,Cy,Cz,Cw).
Commutative quat product:
  A q* B := (Ax*Bx-Ay*By-Az*Bz+Aw*Bw, Ay*Bx+Ax*By-Aw*Bz-Az*Bw,
             Az*Bx-Aw*By+Ax*Bz-Ay*Bw, Aw*Bx+Az*By+Ay*Bz+Ax*Bw)

Parameters:
  [0] dx: double (default: 1)
  [1] dy: double (default: 0)
  [2] dz: double (default: 0)
  [3] dw: double (default: 0)
  [4] w_add: double (default: 0)
  [5] Threshold: double (default: 0)
  [6] ConjMode: integer (default: 0) - negates Cy
  [7] FlipMode: integer (default: 0) - swaps Cx/Cy
  [8] TentMode: integer (default: 0) - tent map mode
  [9] const 0.5
  [10] const 2.0
  [11..14] Cx, Cy, Cz, Cw (engine-provided)
*/
