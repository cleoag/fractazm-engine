OPTION CASEMAP:NONE

.code

; int64_t FormulaIntegerOption_x87(const double* value)
; Pascal Round returns Int64; the caller narrows or clamps as needed.
; Use the caller's rounding mode and preserve its control word.
FormulaIntegerOption_x87 PROC
    sub rsp, 8
    fld qword ptr [rcx]
    fistp qword ptr [rsp]
    mov rax, qword ptr [rsp]
    add rsp, 8
    ret
FormulaIntegerOption_x87 ENDP

; void BuildViewVectorDFOV_x87(double* out_xyz, const double* in)
; in[0] = CAFY
; in[1] = CAFX
BuildViewVectorDFOV_x87 PROC
    sub     rsp, 16
    fnstcw  word ptr [rsp]
    mov     ax, word ptr [rsp]
    and     ax, 0FCFFh
    or      ax, 0300h
    mov     word ptr [rsp + 2], ax
    fldcw   word ptr [rsp + 2]

    ; Literal Pascal Math3D.pas BuildViewVectorDFOV contract:
    ;   x = -sin(ya)
    ;   y = sin(xa)
    ;   z = cos(xa) * cos(ya)
    ; then normalize the full vector in x87 before storing outputs.
    fld     qword ptr [rdx]           ; xa = CAFY
    fsincos                           ; cosX, sinX
    fld     qword ptr [rdx + 8]       ; ya = CAFX, cosX, sinX
    fsincos                           ; cosY, sinY, cosX, sinX
    fmulp   st(2), st(0)              ; sinY, cosX*cosY, sinX
    fchs                              ; -sinY, cosX*cosY, sinX
    fld     st(0)                     ; x, x, z, y
    fmul    st(0), st(1)              ; x*x, x, z, y
    fld     st(2)                     ; z, x*x, x, z, y
    fmul    st(0), st(3)              ; z*z, x*x, x, z, y
    faddp   st(1), st(0)              ; x*x+z*z, x, z, y
    fld     st(3)                     ; y, x*x+z*z, x, z, y
    fmul    st(0), st(4)              ; y*y, x*x+z*z, x, z, y
    faddp   st(1), st(0)              ; sumsq, x, z, y
    fsqrt                             ; sqrt(sumsq), x, z, y
    fld1                              ; 1, sqrt(sumsq), x, z, y
    fdivrp                            ; invlen, x, z, y
    fmul    st(3), st(0)              ; invlen, x, z, y*invlen
    fmul    st(2), st(0)              ; invlen, x, z*invlen, y*invlen
    fmulp                             ; x*invlen, z*invlen, y*invlen
    fstp    qword ptr [rcx]           ; x
    fstp    qword ptr [rcx + 16]      ; z
    fstp    qword ptr [rcx + 8]       ; y

    fldcw   word ptr [rsp]
    add     rsp, 16
    ret
BuildViewVectorDFOV_x87 ENDP

; void BuildViewVectorDFOV_x87_passthru(double* out_xyz, const double* in)
; Same literal Pascal asm contract as BuildViewVectorDFOV_x87, but uses ambient x87 control word.
BuildViewVectorDFOV_x87_passthru PROC
    fld     qword ptr [rdx]           ; xa = CAFY
    fsincos                           ; cosX, sinX
    fld     qword ptr [rdx + 8]       ; ya = CAFX, cosX, sinX
    fsincos                           ; cosY, sinY, cosX, sinX
    fmulp   st(2), st(0)              ; sinY, cosX*cosY, sinX
    fchs                              ; -sinY, cosX*cosY, sinX
    fld     st(0)                     ; x, x, z, y
    fmul    st(0), st(1)              ; x*x, x, z, y
    fld     st(2)                     ; z, x*x, x, z, y
    fmul    st(0), st(3)              ; z*z, x*x, x, z, y
    faddp   st(1), st(0)              ; x*x+z*z, x, z, y
    fld     st(3)                     ; y, x*x+z*z, x, z, y
    fmul    st(0), st(4)              ; y*y, x*x+z*z, x, z, y
    faddp   st(1), st(0)              ; sumsq, x, z, y
    fsqrt                             ; sqrt(sumsq), x, z, y
    fld1                              ; 1, sqrt(sumsq), x, z, y
    fdivrp                            ; invlen, x, z, y
    fmul    st(3), st(0)              ; invlen, x, z, y*invlen
    fmul    st(2), st(0)              ; invlen, x, z*invlen, y*invlen
    fmulp                             ; x*invlen, z*invlen, y*invlen
    fstp    qword ptr [rcx]           ; x
    fstp    qword ptr [rcx + 16]      ; z
    fstp    qword ptr [rcx + 8]       ; y
    ret
BuildViewVectorDFOV_x87_passthru ENDP

; void BuildRotateViewVectorDFOV_x87_passthru(double* out_xyz, const double* in, const double* matrix9)
; in[0] = CAFY
; in[1] = CAFX
; Builds the perspective view vector with Pascal's x87 contract, stores it to out_xyz,
; then applies Pascal's RotateVectorReverse contract under the same ambient x87 state.
BuildRotateViewVectorDFOV_x87_passthru PROC
    ; BuildViewVectorDFOV literal Pascal contract into out_xyz.
    fld     qword ptr [rdx]           ; xa = CAFY
    fsincos                           ; cosX, sinX
    fld     qword ptr [rdx + 8]       ; ya = CAFX, cosX, sinX
    fsincos                           ; cosY, sinY, cosX, sinX
    fmulp   st(2), st(0)              ; sinY, cosX*cosY, sinX
    fchs                              ; -sinY, cosX*cosY, sinX
    fld     st(0)                     ; x, x, z, y
    fmul    st(0), st(1)              ; x*x, x, z, y
    fld     st(2)                     ; z, x*x, x, z, y
    fmul    st(0), st(3)              ; z*z, x*x, x, z, y
    faddp   st(1), st(0)              ; x*x+z*z, x, z, y
    fld     st(3)                     ; y, x*x+z*z, x, z, y
    fmul    st(0), st(4)              ; y*y, x*x+z*z, x, z, y
    faddp   st(1), st(0)              ; sumsq, x, z, y
    fsqrt                             ; sqrt(sumsq), x, z, y
    fld1                              ; 1, sqrt(sumsq), x, z, y
    fdivrp                            ; invlen, x, z, y
    fmul    st(3), st(0)              ; invlen, x, z, y*invlen
    fmul    st(2), st(0)              ; invlen, x, z*invlen, y*invlen
    fmulp                             ; x*invlen, z*invlen, y*invlen
    fstp    qword ptr [rcx]           ; x
    fstp    qword ptr [rcx + 16]      ; z
    fstp    qword ptr [rcx + 8]       ; y

    ; RotateVectorReverse literal Pascal contract on out_xyz using matrix9.
    fld     qword ptr [r8]            ; m00
    fld     qword ptr [r8 + 8]        ; m01, m00
    fld     qword ptr [r8 + 16]       ; m02, m01, m00
    fld     qword ptr [rcx]           ; v0, m02, m01, m00
    fmul    st(1), st(0)              ; v0, m02*v0, m01, m00
    fmul    st(2), st(0)              ; v0, m02*v0, m01*v0, m00
    fmulp   st(3), st(0)              ; m02*v0, m01*v0, m00*v0
    fld     qword ptr [r8 + 24]       ; m10, ...
    fld     qword ptr [r8 + 32]       ; m11, m10, ...
    fld     qword ptr [r8 + 40]       ; m12, m11, m10, ...
    fld     qword ptr [rcx + 8]       ; v1, ...
    fmul    st(1), st(0)              ; v1, m12*v1, ...
    fmul    st(2), st(0)              ; v1, m12*v1, m11*v1, ...
    fmulp   st(3), st(0)              ; m12*v1, m11*v1, m10*v1, ...
    faddp   st(3), st(0)
    faddp   st(3), st(0)
    faddp   st(3), st(0)              ; sum2, sum1, sum0
    fld     qword ptr [r8 + 48]       ; m20, ...
    fld     qword ptr [r8 + 56]       ; m21, m20, ...
    fld     qword ptr [r8 + 64]       ; m22, m21, m20, ...
    fld     qword ptr [rcx + 16]      ; v2, ...
    fmul    st(1), st(0)              ; v2, m22*v2, ...
    fmul    st(2), st(0)              ; v2, m22*v2, m21*v2, ...
    fmulp   st(3), st(0)              ; m22*v2, m21*v2, m20*v2, ...
    faddp   st(3), st(0)
    faddp   st(3), st(0)
    faddp   st(3), st(0)              ; z, y, x
    fstp    qword ptr [rcx + 16]
    fstp    qword ptr [rcx + 8]
    fstp    qword ptr [rcx]
    ret
BuildRotateViewVectorDFOV_x87_passthru ENDP

; void BuildRotateViewVectorDFOV_x87_pascw(double* out_xyz, const double* in, const double* matrix9)
; Same as BuildRotateViewVectorDFOV_x87_passthru but forces the exact Pascal/FPC x87 CW ($137F).
BuildRotateViewVectorDFOV_x87_pascw PROC
    sub     rsp, 16
    fnstcw  word ptr [rsp]
    mov     word ptr [rsp + 2], 0137Fh
    fldcw   word ptr [rsp + 2]

    ; BuildViewVectorDFOV literal Pascal contract into out_xyz.
    fld     qword ptr [rdx]           ; xa = CAFY
    fsincos                           ; cosX, sinX
    fld     qword ptr [rdx + 8]       ; ya = CAFX, cosX, sinX
    fsincos                           ; cosY, sinY, cosX, sinX
    fmulp   st(2), st(0)              ; sinY, cosX*cosY, sinX
    fchs                              ; -sinY, cosX*cosY, sinX
    fld     st(0)                     ; x, x, z, y
    fmul    st(0), st(1)              ; x*x, x, z, y
    fld     st(2)                     ; z, x*x, x, z, y
    fmul    st(0), st(3)              ; z*z, x*x, x, z, y
    faddp   st(1), st(0)              ; x*x+z*z, x, z, y
    fld     st(3)                     ; y, x*x+z*z, x, z, y
    fmul    st(0), st(4)              ; y*y, x*x+z*z, x, z, y
    faddp   st(1), st(0)              ; sumsq, x, z, y
    fsqrt                             ; sqrt(sumsq), x, z, y
    fld1                              ; 1, sqrt(sumsq), x, z, y
    fdivrp                            ; invlen, x, z, y
    fmul    st(3), st(0)              ; invlen, x, z, y*invlen
    fmul    st(2), st(0)              ; invlen, x, z*invlen, y*invlen
    fmulp                             ; x*invlen, z*invlen, y*invlen
    fstp    qword ptr [rcx]           ; x
    fstp    qword ptr [rcx + 16]      ; z
    fstp    qword ptr [rcx + 8]       ; y

    ; RotateVectorReverse literal Pascal contract on out_xyz using matrix9.
    fld     qword ptr [r8]            ; m00
    fld     qword ptr [r8 + 8]        ; m01, m00
    fld     qword ptr [r8 + 16]       ; m02, m01, m00
    fld     qword ptr [rcx]           ; v0, m02, m01, m00
    fmul    st(1), st(0)              ; v0, m02*v0, m01, m00
    fmul    st(2), st(0)              ; v0, m02*v0, m01*v0, m00
    fmulp   st(3), st(0)              ; m02*v0, m01*v0, m00*v0
    fld     qword ptr [r8 + 24]       ; m10, ...
    fld     qword ptr [r8 + 32]       ; m11, m10, ...
    fld     qword ptr [r8 + 40]       ; m12, m11, m10, ...
    fld     qword ptr [rcx + 8]       ; v1, ...
    fmul    st(1), st(0)              ; v1, m12*v1, ...
    fmul    st(2), st(0)              ; v1, m12*v1, m11*v1, ...
    fmulp   st(3), st(0)              ; m12*v1, m11*v1, m10*v1, ...
    faddp   st(3), st(0)
    faddp   st(3), st(0)
    faddp   st(3), st(0)              ; sum2, sum1, sum0
    fld     qword ptr [r8 + 48]       ; m20, ...
    fld     qword ptr [r8 + 56]       ; m21, m20, ...
    fld     qword ptr [r8 + 64]       ; m22, m21, m20, ...
    fld     qword ptr [rcx + 16]      ; v2, ...
    fmul    st(1), st(0)              ; v2, m22*v2, ...
    fmul    st(2), st(0)              ; v2, m22*v2, m21*v2, ...
    fmulp   st(3), st(0)              ; m22*v2, m21*v2, m20*v2, ...
    faddp   st(3), st(0)
    faddp   st(3), st(0)
    faddp   st(3), st(0)              ; z, y, x
    fstp    qword ptr [rcx + 16]
    fstp    qword ptr [rcx + 8]
    fstp    qword ptr [rcx]

    fldcw   word ptr [rsp]
    add     rsp, 16
    ret
BuildRotateViewVectorDFOV_x87_pascw ENDP

; void RotateVectorReverse_x87(double* vec_io, const double* matrix9)
; vec_io points to 3 doubles
; matrix9 points to 9 doubles in row-major order
RotateVectorReverse_x87 PROC
    fld     qword ptr [rdx]           ; m00
    fld     qword ptr [rdx + 8]       ; m01, m00
    fld     qword ptr [rdx + 16]      ; m02, m01, m00
    fld     qword ptr [rcx]           ; v0, m02, m01, m00
    fmul    st(1), st(0)              ; v0, m02*v0, m01, m00
    fmul    st(2), st(0)              ; v0, m02*v0, m01*v0, m00
    fmulp   st(3), st(0)              ; m02*v0, m01*v0, m00*v0
    fld     qword ptr [rdx + 24]      ; m10, ...
    fld     qword ptr [rdx + 32]      ; m11, m10, ...
    fld     qword ptr [rdx + 40]      ; m12, m11, m10, ...
    fld     qword ptr [rcx + 8]       ; v1, ...
    fmul    st(1), st(0)              ; v1, m12*v1, ...
    fmul    st(2), st(0)              ; v1, m12*v1, m11*v1, ...
    fmulp   st(3), st(0)              ; m12*v1, m11*v1, m10*v1, ...
    faddp   st(3), st(0)
    faddp   st(3), st(0)
    faddp   st(3), st(0)              ; sum2, sum1, sum0
    fld     qword ptr [rdx + 48]      ; m20, ...
    fld     qword ptr [rdx + 56]      ; m21, m20, ...
    fld     qword ptr [rdx + 64]      ; m22, m21, m20, ...
    fld     qword ptr [rcx + 16]      ; v2, ...
    fmul    st(1), st(0)              ; v2, m22*v2, ...
    fmul    st(2), st(0)              ; v2, m22*v2, m21*v2, ...
    fmulp   st(3), st(0)              ; m22*v2, m21*v2, m20*v2, ...
    faddp   st(3), st(0)
    faddp   st(3), st(0)
    faddp   st(3), st(0)              ; z, y, x
    fstp    qword ptr [rcx + 16]
    fstp    qword ptr [rcx + 8]
    fstp    qword ptr [rcx]
    ret
RotateVectorReverse_x87 ENDP

; void RotateVectorReverse_x87_extcw(double* vec_io, const double* matrix9)
; Same arithmetic contract as RotateVectorReverse_x87 but forces x87 extended precision.
RotateVectorReverse_x87_extcw PROC
    sub     rsp, 16
    fnstcw  word ptr [rsp]
    mov     ax, word ptr [rsp]
    and     ax, 0FCFFh
    or      ax, 0300h
    mov     word ptr [rsp + 2], ax
    fldcw   word ptr [rsp + 2]

    fld     qword ptr [rdx]           ; m00
    fld     qword ptr [rdx + 8]       ; m01, m00
    fld     qword ptr [rdx + 16]      ; m02, m01, m00
    fld     qword ptr [rcx]           ; v0, m02, m01, m00
    fmul    st(1), st(0)              ; v0, m02*v0, m01, m00
    fmul    st(2), st(0)              ; v0, m02*v0, m01*v0, m00
    fmulp   st(3), st(0)              ; m02*v0, m01*v0, m00*v0
    fld     qword ptr [rdx + 24]      ; m10, ...
    fld     qword ptr [rdx + 32]      ; m11, m10, ...
    fld     qword ptr [rdx + 40]      ; m12, m11, m10, ...
    fld     qword ptr [rcx + 8]       ; v1, ...
    fmul    st(1), st(0)              ; v1, m12*v1, ...
    fmul    st(2), st(0)              ; v1, m12*v1, m11*v1, ...
    fmulp   st(3), st(0)              ; m12*v1, m11*v1, m10*v1, ...
    faddp   st(3), st(0)
    faddp   st(3), st(0)
    faddp   st(3), st(0)              ; sum2, sum1, sum0
    fld     qword ptr [rdx + 48]      ; m20, ...
    fld     qword ptr [rdx + 56]      ; m21, m20, ...
    fld     qword ptr [rdx + 64]      ; m22, m21, m20, ...
    fld     qword ptr [rcx + 16]      ; v2, ...
    fmul    st(1), st(0)              ; v2, m22*v2, ...
    fmul    st(2), st(0)              ; v2, m22*v2, m21*v2, ...
    fmulp   st(3), st(0)              ; m22*v2, m21*v2, m20*v2, ...
    faddp   st(3), st(0)
    faddp   st(3), st(0)
    faddp   st(3), st(0)              ; z, y, x
    fstp    qword ptr [rcx + 16]
    fstp    qword ptr [rcx + 8]
    fstp    qword ptr [rcx]

    fldcw   word ptr [rsp]
    add     rsp, 16
    ret
RotateVectorReverse_x87_extcw ENDP

; unsigned short GetX87ControlWord()
GetX87ControlWord PROC
    sub     rsp, 8
    fnstcw  word ptr [rsp]
    movzx   eax, word ptr [rsp]
    add     rsp, 8
    ret
GetX87ControlWord ENDP

; void Rout3_x87(double* out, const double* xyz)
; xyz[0] = x
; xyz[1] = y
; xyz[2] = z
Rout3_x87 PROC
    fld     qword ptr [rdx]           ; x
    fmul    st(0), st(0)              ; x*x
    fld     qword ptr [rdx + 8]       ; y, xx
    fmul    st(0), st(0)              ; y*y, xx
    faddp   st(1), st(0)              ; xx+yy
    fld     qword ptr [rdx + 16]      ; z, xx+yy
    fmul    st(0), st(0)              ; z*z, xx+yy
    faddp   st(1), st(0)              ; xx+yy+zz
    fstp    qword ptr [rcx]
    ret
Rout3_x87 ENDP

; void Rout3_x87_extcw(double* out, const double* xyz)
; same as Rout3_x87 but forces extended precision control word
; void SinCosD_x87_pascw(double* sinOut, double* cosOut, const double* a)
; Computes sin(a) and cos(a) via fsincos under FPC x87 CW ($137F).
SinCosD_x87_pascw PROC
    sub     rsp, 16
    fnstcw  word ptr [rsp]
    mov     word ptr [rsp + 2], 0137Fh
    fldcw   word ptr [rsp + 2]

    fld     qword ptr [r8]              ; a
    fsincos                              ; cos(a), sin(a)
    fstp    qword ptr [rdx]              ; store cos
    fstp    qword ptr [rcx]              ; store sin

    fldcw   word ptr [rsp]
    add     rsp, 16
    ret
SinCosD_x87_pascw ENDP

; void Rout3_x87_pascw(double* out, const double* xyz)
; Sums x*x + y*y + z*z using x87 under the exact FPC x87 CW ($137F =
; 53-bit precision). This mirrors what doHybridPas compiles to on FPC
; i386 byte-for-byte.
Rout3_x87_pascw PROC
    sub     rsp, 16
    fnstcw  word ptr [rsp]
    mov     word ptr [rsp + 2], 0137Fh
    fldcw   word ptr [rsp + 2]

    fld     qword ptr [rdx]            ; x
    fmul    st(0), st(0)               ; x*x
    fld     qword ptr [rdx + 8]        ; y, xx
    fmul    st(0), st(0)               ; y*y, xx
    faddp   st(1), st(0)               ; xx+yy
    fld     qword ptr [rdx + 16]       ; z, xx+yy
    fmul    st(0), st(0)               ; z*z, xx+yy
    faddp   st(1), st(0)               ; xx+yy+zz
    fstp    qword ptr [rcx]

    fldcw   word ptr [rsp]
    add     rsp, 16
    ret
Rout3_x87_pascw ENDP

; void Rout3_pas_sse2(double* out, const double* xyz)
; Mirrors the SSE2 sequence used inside Pascal's doHybridSSE2 for the
; per-iteration Rout update:
;   xmm7 := z*z
;   xmm6 := (x*x, y*y)        (packed)
;   xmm7 += xmm6.low           -> z*z + x*x
;   shufpd xmm6                -> (y*y, x*x)
;   xmm7 += xmm6.low           -> z*z + x*x + y*y
; The exact operation order matters â€” (z*z + x*x) + y*y is NOT the same
; as (x*x + y*y) + z*z at 1-ULP.
Rout3_pas_sse2 PROC
    movupd  xmm6, [rdx]              ; (x, y)
    movsd   xmm7, qword ptr [rdx + 16] ; z (low); high = 0
    mulpd   xmm6, xmm6               ; (x*x, y*y)
    mulsd   xmm7, xmm7               ; z*z
    addsd   xmm7, xmm6               ; z*z + x*x
    shufpd  xmm6, xmm6, 1            ; (y*y, x*x)
    addsd   xmm7, xmm6               ; z*z + x*x + y*y
    movsd   qword ptr [rcx], xmm7
    ret
Rout3_pas_sse2 ENDP

Rout3_x87_extcw PROC
    sub     rsp, 16
    fnstcw  word ptr [rsp]
    mov     ax, word ptr [rsp]
    and     ax, 0FCFFh
    or      ax, 0300h
    mov     word ptr [rsp + 2], ax
    fldcw   word ptr [rsp + 2]

    fld     qword ptr [rdx]           ; x
    fmul    st(0), st(0)              ; x*x
    fld     qword ptr [rdx + 8]       ; y, xx
    fmul    st(0), st(0)              ; y*y, xx
    faddp   st(1), st(0)              ; xx+yy
    fld     qword ptr [rdx + 16]      ; z, xx+yy
    fmul    st(0), st(0)              ; z*z, xx+yy
    faddp   st(1), st(0)              ; xx+yy+zz
    fstp    qword ptr [rcx]

    fldcw   word ptr [rsp]
    add     rsp, 16
    ret
Rout3_x87_extcw ENDP

; void DeltaSq_x87(double* out, const double* a, const double* b)
DeltaSq_x87 PROC
    fld     qword ptr [rdx]
    fsub    qword ptr [r8]
    fmul    st(0), st(0)
    fstp    qword ptr [rcx]
    ret
DeltaSq_x87 ENDP

; void CalcRawDe_x87(double* out_raw, double* out_denom, const double* params)
; params[0] = bufRout
; params[1] = dt
; params[2] = wt
; params[3] = rst
; params[4] = dDEscale
; params[5] = mctDEoffset006
CalcRawDe_x87 PROC
    ; denom = sqrt(rst + wt + dt) + off
    fld     qword ptr [r8 + 24]
    fadd    qword ptr [r8 + 16]
    fadd    qword ptr [r8 + 8]
    fsqrt
    fadd    qword ptr [r8 + 40]
    fstp    qword ptr [rdx]

    ; raw = bufRout * ln(bufRout) * scale / denom
    fld     qword ptr [r8]
    fldln2
    fxch    st(1)
    fyl2x
    fmul    qword ptr [r8]
    fmul    qword ptr [r8 + 32]
    fdiv    qword ptr [rdx]
    fstp    qword ptr [rcx]
    ret
CalcRawDe_x87 ENDP

; void CalcRawDe_x87_extcw(double* out_raw, double* out_denom, const double* params)
; Same as CalcRawDe_x87 but forces x87 extended precision control word.
; void CalcRawDe_x87_pascw(double* out_raw, double* out_denom, const double* params)
; Same as CalcRawDe_x87 but forces FPC's default x87 CW ($137F = 53-bit).
CalcRawDe_x87_pascw PROC
    sub     rsp, 16
    fnstcw  word ptr [rsp]
    mov     word ptr [rsp + 2], 0137Fh
    fldcw   word ptr [rsp + 2]

    ; denom = sqrt(rst + wt + dt) + off
    fld     qword ptr [r8 + 24]
    fadd    qword ptr [r8 + 16]
    fadd    qword ptr [r8 + 8]
    fsqrt
    fadd    qword ptr [r8 + 40]
    fstp    qword ptr [rdx]

    ; raw = bufRout * ln(bufRout) * scale / denom
    fld     qword ptr [r8]
    fldln2
    fxch    st(1)
    fyl2x
    fmul    qword ptr [r8]
    fmul    qword ptr [r8 + 32]
    fdiv    qword ptr [rdx]
    fstp    qword ptr [rcx]

    fldcw   word ptr [rsp]
    add     rsp, 16
    ret
CalcRawDe_x87_pascw ENDP

CalcRawDe_x87_extcw PROC
    sub     rsp, 16
    fnstcw  word ptr [rsp]
    mov     ax, word ptr [rsp]
    and     ax, 0FCFFh
    or      ax, 0300h
    mov     word ptr [rsp + 2], ax
    fldcw   word ptr [rsp + 2]

    ; Calc.pas compiled with -CfSSE2: each denominator operation is Double.
    movsd   xmm0, qword ptr [r8 + 24]
    addsd   xmm0, qword ptr [r8 + 16]
    addsd   xmm0, qword ptr [r8 + 8]
    sqrtsd  xmm0, xmm0
    addsd   xmm0, qword ptr [r8 + 40]
    movsd   qword ptr [rdx], xmm0

    ; raw = bufRout * ln(bufRout) * scale / denom
    fld     qword ptr [r8]
    fldln2
    fxch    st(1)
    fyl2x
    fmul    qword ptr [r8]
    fmul    qword ptr [r8 + 32]
    fdiv    qword ptr [rdx]
    fstp    qword ptr [rcx]

    fldcw   word ptr [rsp]
    add     rsp, 16
    ret
CalcRawDe_x87_extcw ENDP

; struct DynFogStepX87Params {
;   double actDE;
;   float  msDEsub;
;   float  msDEstop;
;   float  sZstepDiv;
;   float  RSFmul;
;   float  s011;
; }
; void DynFogStep_x87(float* out, const DynFogStepX87Params* p)
DynFogStep_x87 PROC
    sub     rsp, 16
    fnstcw  word ptr [rsp]
    mov     ax, word ptr [rsp]
    and     ax, 0FCFFh                ; clear precision-control bits
    or      ax, 0300h                 ; set extended precision
    mov     word ptr [rsp + 2], ax
    fldcw   word ptr [rsp + 2]

    fld     qword ptr [rdx]           ; actDE
    fld     dword ptr [rdx + 8]       ; msDEsub, actDE
    fmul    dword ptr [rdx + 12]      ; msDEsub * msDEstop
    fsubp   st(1), st(0)              ; actDE - msDEsub*msDEstop
    fmul    dword ptr [rdx + 16]      ; * sZstepDiv
    fmul    dword ptr [rdx + 20]      ; * RSFmul
    fld     dword ptr [rdx + 24]      ; s011, expr
    fcomi   st(0), st(1)
    jb      dynfog_use_expr

    fstp    dword ptr [rcx]           ; store s011
    fstp    st(0)                     ; pop expr
    jmp     dynfog_done

dynfog_use_expr:
    fxch    st(1)                     ; expr, s011
    fstp    dword ptr [rcx]           ; store expr
    fstp    st(0)                     ; pop s011

dynfog_done:
    fldcw   word ptr [rsp]
    add     rsp, 16
    ret
DynFogStep_x87 ENDP

; struct HybridItIntPow2X87Params {
;   double x;
;   double y;
;   double z;
;   double j1;
;   double j2;
;   double j3;
;   double dZmul;
; }
; void HybridItIntPow2_x87(HybridItIntPow2X87Params* p)
HybridItIntPow2_x87 PROC
    fld     qword ptr [rcx + 16]      ; z
    fld     qword ptr [rcx + 8]       ; y, z
    fld     qword ptr [rcx]           ; x, y, z
    fld     st(1)                     ; y, x, y, z
    fmul    st(0), st(2)              ; yy, x, y, z
    fld     st(1)                     ; x, yy, x, y, z
    fmul    st(0), st(2)              ; xx, yy, x, y, z
    fld     st(0)                     ; xx, xx, yy, x, y, z
    fadd    st(0), st(2)              ; xt, xx, yy, x, y, z
    fld     st(0)                     ; xt, xt, xx, yy, x, y, z
    fsqrt
    fmul    qword ptr [rcx + 48]      ; * dZmul
    fmul    st(0), st(6)              ; * z
    fadd    st(0), st(0)              ; * 2
    fadd    qword ptr [rcx + 40]      ; + J3
    fstp    qword ptr [rcx + 16]      ; store z
    fld     st(5)                     ; z, xt, xx, yy, x, y, z
    fmulp   st(6), st(0)              ; xt, xx, yy, x, y, zz
    fld     st(0)                     ; xt, xt, xx, yy, x, y, zz
    fsubrp  st(6), st(0)              ; xt, xx, yy, x, y, xt-zz
    fdivp   st(5), st(0)              ; xx, yy, x, y, a
    fsubrp                             ; xx-yy, x, y, a
    fmul    st(0), st(3)              ; a * (xx-yy), x, y, a
    fadd    qword ptr [rcx + 24]      ; + J1
    fstp    qword ptr [rcx]           ; store x
    fmulp                             ; x*y, a
    fmulp                             ; x*y*a
    fadd    st(0), st(0)              ; * 2
    fadd    qword ptr [rcx + 32]      ; + J2
    fstp    qword ptr [rcx + 8]       ; store y
    ret
HybridItIntPow2_x87 ENDP

; void HybridItIntPow2_x87_extcw(HybridItIntPow2X87Params* p)
; Forces x87 precision-control bits to extended precision (64-bit significand)
; for the duration of the helper and restores the previous control word after.
HybridItIntPow2_x87_extcw PROC
    sub     rsp, 16
    fnstcw  word ptr [rsp]
    mov     ax, word ptr [rsp]
    and     ax, 0FCFFh                ; clear precision-control bits
    or      ax, 0300h                 ; set extended precision
    mov     word ptr [rsp + 2], ax
    fldcw   word ptr [rsp + 2]

    fld     qword ptr [rcx + 16]      ; z
    fld     qword ptr [rcx + 8]       ; y, z
    fld     qword ptr [rcx]           ; x, y, z
    fld     st(1)                     ; y, x, y, z
    fmul    st(0), st(2)              ; yy, x, y, z
    fld     st(1)                     ; x, yy, x, y, z
    fmul    st(0), st(2)              ; xx, yy, x, y, z
    fld     st(0)                     ; xx, xx, yy, x, y, z
    fadd    st(0), st(2)              ; xt, xx, yy, x, y, z
    fld     st(0)                     ; xt, xt, xx, yy, x, y, z
    fsqrt
    fmul    qword ptr [rcx + 48]      ; * dZmul
    fmul    st(0), st(6)              ; * z
    fadd    st(0), st(0)              ; * 2
    fadd    qword ptr [rcx + 40]      ; + J3
    fstp    qword ptr [rcx + 16]      ; store z
    fld     st(5)                     ; z, xt, xx, yy, x, y, z
    fmulp   st(6), st(0)              ; xt, xx, yy, x, y, zz
    fld     st(0)                     ; xt, xt, xx, yy, x, y, zz
    fsubrp  st(6), st(0)              ; xt, xx, yy, x, y, xt-zz
    fdivp   st(5), st(0)              ; xx, yy, x, y, a
    fsubrp                             ; xx-yy, x, y, a
    fmul    st(0), st(3)              ; a * (xx-yy), x, y, a
    fadd    qword ptr [rcx + 24]      ; + J1
    fstp    qword ptr [rcx]           ; store x
    fmulp                             ; x*y, a
    fmulp                             ; x*y*a
    fadd    st(0), st(0)              ; * 2
    fadd    qword ptr [rcx + 32]      ; + J2
    fstp    qword ptr [rcx + 8]       ; store y

    fldcw   word ptr [rsp]
    add     rsp, 16
    ret
HybridItIntPow2_x87_extcw ENDP

; void Atan_x87_pascw(double* out, const double* in)
; Returns ArcTan(in[0]) under the exact Pascal/FPC x87 CW ($137F).
Atan_x87_pascw PROC
    sub     rsp, 16
    fnstcw  word ptr [rsp]
    mov     word ptr [rsp + 2], 0137Fh
    fldcw   word ptr [rsp + 2]

    fld     qword ptr [rdx]
    fld1
    fpatan
    fstp    qword ptr [rcx]

    fldcw   word ptr [rsp]
    add     rsp, 16
    ret
Atan_x87_pascw ENDP

; void AtanScaledDouble_x87_pascw(double* out, const double* in)
; Returns ArcTan(in[0] * 1/4096) under the exact Pascal/FPC x87 CW ($137F).
AtanScaledDouble_x87_pascw PROC
    sub     rsp, 16
    fnstcw  word ptr [rsp]
    mov     word ptr [rsp + 2], 0137Fh
    fldcw   word ptr [rsp + 2]

    fld     qword ptr [rdx]
    fmul    qword ptr kInv4096
    fld1
    fpatan
    fstp    qword ptr [rcx]

    fldcw   word ptr [rsp]
    add     rsp, 16
    ret
AtanScaledDouble_x87_pascw ENDP

; void CalcAOHiQSum_x87_pascw(float* out, const int16_t* psi32)
; Sum over 32 packed directions:
;   st := 0
;   if psi[i] > -32768 then st := st + ArcTan(psi[i] * 1/4096)
; Uses Pascal/FPC x87 control word ($137F) for the full accumulation.
CalcAOHiQSum_x87_pascw PROC
    sub     rsp, 24
    fnstcw  word ptr [rsp]
    mov     word ptr [rsp + 2], 0137Fh
    fldcw   word ptr [rsp + 2]

    fldz
    fstp    qword ptr [rsp + 8]          ; acc = 0
    xor     r8d, r8d

aohiq_sum_loop2:
    cmp     word ptr [rdx + r8*2], 08000h
    je      aohiq_sum_next2

    fild    word ptr [rdx + r8*2]
    fmul    qword ptr kInv4096
    fld1
    fpatan
    fadd    qword ptr [rsp + 8]          ; atan + acc
    fstp    qword ptr [rsp + 8]          ; acc = atan + acc

aohiq_sum_next2:
    inc     r8d
    cmp     r8d, 32
    jl      aohiq_sum_loop2

    fld     qword ptr [rsp + 8]
    fstp    dword ptr [rcx]
    fldcw   word ptr [rsp]
    add     rsp, 24
    ret
CalcAOHiQSum_x87_pascw ENDP

; void Sum32DoublesToFloat_x87_pascw(float* out, const double* values32)
; Sums 32 double values in x87 under the exact Pascal/FPC CW ($137F),
; then stores the final accumulator as float.
Sum32DoublesToFloat_x87_pascw PROC
    sub     rsp, 16
    fnstcw  word ptr [rsp]
    mov     word ptr [rsp + 2], 0137Fh
    fldcw   word ptr [rsp + 2]

    fldz
    mov     r8d, 32

sum32_loop:
    fadd    qword ptr [rdx]
    add     rdx, 8
    dec     r8d
    jnz     sum32_loop

    fstp    dword ptr [rcx]
    fldcw   word ptr [rsp]
    add     rsp, 16
    ret
Sum32DoublesToFloat_x87_pascw ENDP

; void Atan2_x87_pascw(double* out, const double* y, const double* x)
; Returns ArcTan2(y, x) under the exact Pascal/FPC x87 CW ($137F).
; fpatan pops ST(0) and ST(1), pushes ArcTan(ST(1) / ST(0)) quadrant-corrected,
; so we need ST(0) = x and ST(1) = y before issuing fpatan.
Atan2_x87_pascw PROC
    sub     rsp, 16
    fnstcw  word ptr [rsp]
    mov     word ptr [rsp + 2], 0137Fh
    fldcw   word ptr [rsp + 2]

    fld     qword ptr [rdx]         ; y at ST(0)
    fld     qword ptr [r8]          ; x at ST(0), y at ST(1)
    fpatan                           ; ArcTan(ST(1)/ST(0)) = atan2(y, x)
    fstp    qword ptr [rcx]

    fldcw   word ptr [rsp]
    add     rsp, 16
    ret
Atan2_x87_pascw ENDP

; void Pow_x87_pascw(double* out, const double* base, const double* expo)
; Returns base^expo via the fyl2x / f2xm1 / fscale sequence the FPC i386
; `Power` intrinsic emits. Under Pascal/FPC x87 CW ($137F). The instruction
; order mirrors formulas.pas AexionC exactly so the transcendental yields
; byte-identical bits to the Pascal reference.
Pow_x87_pascw PROC
    sub     rsp, 16
    fnstcw  word ptr [rsp]
    mov     word ptr [rsp + 2], 0137Fh
    fldcw   word ptr [rsp + 2]

    fld     qword ptr [r8]          ; expo
    fld     qword ptr [rdx]         ; base, expo
    fldln2                           ; ln2, base, expo
    fxch                             ; base, ln2, expo
    fyl2x                            ; ln2*log2(base) = ln(base), expo
    fxch                             ; expo, ln(base)
    fmulp                            ; expo*ln(base)
    fldl2e                           ; log2(e), expo*ln(base)
    fmulp                            ; expo*log2(base)
    fld     st(0)                    ; full, full
    frndint                          ; trunc, full
    fsub    st(1), st(0)             ; trunc, fraction
    fxch                             ; fraction, trunc
    f2xm1                            ; 2^fraction - 1, trunc
    fld1
    faddp                            ; 2^fraction, trunc
    fscale                           ; 2^fraction * 2^trunc = base^expo, trunc
    fstp    st(1)                    ; discard trunc
    fstp    qword ptr [rcx]

    fldcw   word ptr [rsp]
    add     rsp, 16
    ret
Pow_x87_pascw ENDP

; Complete formulas.pas HybridFloatPow sequence, Pascal CW restored on return.
; params: x,y,z,J1,J2,J3,power,zScale,half,Rout (ten doubles).
HybridFloatPow_x87 PROC
    sub rsp, 16
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 0137Fh
    fldcw word ptr [rsp + 2]
  fld    qword ptr [rcx + 48]
  fld    qword ptr [rcx + 8]
  fld    qword ptr [rcx]
  fld    st(1)
  fld    st(1)
  fpatan                    ;theta, x, y, pow
  fmul   st(0), st(3)
  fsincos                   ;Costheta, Sintheta, x, y, pow
  fld    qword ptr [rcx + 16]        ;z,Costheta, Sintheta,x,y,pow
  fxch   st(3)              ;x,Costheta, Sintheta,z,y,pow
  fmul   st(0), st(0)
  fxch   st(4)              ;y,Costheta, Sintheta,z,xx,pow
  fmul   st(0), st(0)
  faddp  st(4), st(0)          ;Costheta,Sintheta,z,xx+yy,pow
  fxch   st(2)              ;z,Sintheta,Costheta,xx+yy,pow
  fxch                      ;Sintheta,z,Costheta,xx+yy,pow
  fxch   st(3)              ;xx+yy,z,Costheta,Sintheta,pow
  fsqrt
  fpatan                    ;phi,Costheta, Sintheta,pow
  fmul   st(0), st(3)
  fsincos                   ;Cosphi,Sinphi,Costheta,Sintheta,pow
  fxch   st(4)              ;pow,Sinphi,Costheta,Sintheta,Cosphi
  fmul   qword ptr [rcx + 64]    ;*0.5 because of Rout=sqr(R)
  fld    qword ptr [rcx + 72]   ;SqrRadius, pow*0.5,Sinphi,Costheta,Sintheta,Cosphi
  fldln2                    ;power function  x,pow
  fxch
  fyl2x
  fxch
  fmulp
  fldl2e
  fmulp
  fld    st(0)
  frndint
  fsub   st(1), st(0)
  fxch
  f2xm1
  fld1
  faddp
  fscale
  fstp   st(1)              ;NewRadius,Sinphi,Costheta,Sintheta,Cosphi
  fxch   st(2)              ;Costheta,Sinphi,NewRadius,Sintheta,Cosphi
  fmul   st(0), st(4)
  fmul   st(0), st(2)
  fadd   qword ptr [rcx + 24]
  fstp   qword ptr [rcx]        ;Sinphi,NewRadius,Sintheta,Cosphi
  fxch   st(3)              ;Cosphi,NewRadius,Sintheta,Sinphi
  fmulp  st(2), st(0)          ;NewRadius,Sintheta*Cosphi,Sinphi
  fmul   st(1), st(0)
  fmulp  st(2), st(0)          ;Sintheta*Cosphi*r,Sinphi*r
  fadd   qword ptr [rcx + 32]
  fstp   qword ptr [rcx + 8]
  fmul   qword ptr [rcx + 56]
  fadd   qword ptr [rcx + 40]
  fstp   qword ptr [rcx + 16]
    fldcw word ptr [rsp]
    add rsp, 16
    ret
HybridFloatPow_x87 ENDP

; Literal ABoxMod1 fold/DE sequence, with extended x87 intermediates.
; state: xyz,w,scale,scaleOverMinR2,minR2,fold,scaleVary,unused,
;        foldMods[3],J[3],varyScale.
ABoxMod1_x87 PROC
    sub rsp, 16
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 0137Fh
    fldcw word ptr [rsp + 2]
    mov r9, rcx
    fld qword ptr [r9 + 56]
    fld qword ptr [r9]
    fadd qword ptr [r9 + 80]
    fabs
    fsub st(0), st(1)
    nop
    fabs
    fchs
    fadd st(0), st(1)
    fld qword ptr [r9 + 80]
    fabs
    fchs
    faddp st(1), st(0)
    fld qword ptr [r9 + 8]
    fadd qword ptr [r9 + 88]
    fabs
    fsub st(0), st(2)
    nop
    fabs
    fchs
    fadd st(0), st(2)
    fld qword ptr [r9 + 88]
    fabs
    fchs
    faddp st(1), st(0)
    fld qword ptr [r9 + 16]
    fadd qword ptr [r9 + 96]
    fabs
    fsub st(0), st(3)
    nop
    fabs
    fchs
    fadd st(0), st(3)
    fld qword ptr [r9 + 96]
    fabs
    fchs
    faddp st(1), st(0)
    nop
    fstp st(3)
    fxch st(1)
    fxch st(2)
    fld st(0)
    fmul st(0), st(1)
    fld st(2)
    fmul st(0), st(3)
    faddp st(1), st(0)
    fld st(3)
    fmul st(0), st(4)
    faddp st(1), st(0)
    fcom qword ptr [r9 + 48]
    fnstsw ax
    shr ah, 1
    jae abox_ab
    fstp st(0)
    fld qword ptr [r9 + 40]
    jmp abox_c1
abox_ab:
    fld1
    fcom st(1)
    fnstsw ax
    shr ah, 1
    jb abox_bc
    fstp st(0)
    fdivr qword ptr [r9 + 128]
    jmp abox_c1
abox_bc:
    fcompp
    fld qword ptr [r9 + 128]
abox_c1:
    fld qword ptr [r9 + 24]
    fmul st(0), st(1)
    fstp qword ptr [r9 + 24]
    fmul st(3), st(0)
    fmul st(2), st(0)
    fmulp st(1), st(0)
    fadd qword ptr [r9 + 120]
    fstp qword ptr [r9 + 16]
    fadd qword ptr [r9 + 104]
    fstp qword ptr [r9 + 8]
    fadd qword ptr [r9 + 112]
    fstp qword ptr [r9]
    fldcw word ptr [rsp]
    add rsp, 16
    ret
ABoxMod1_x87 ENDP

; Literal ASurfMod1 x87 body, source offsets 0036..0101.
; state: xyz,w,20 expanded parameters (including exact float matrix values),VaryScale.
ASurfMod1_x87 PROC
    sub rsp, 16
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 0137Fh
    fldcw word ptr [rsp + 2]
    mov r9, rcx
    fld qword ptr [r9 + 56]
    fld qword ptr [r9 + 0]
    fadd qword ptr [r9 + 72]
    fabs
    fsub st(0), st(1)
    nop
    fabs
    fchs
    fadd st(0), st(1)
    fld qword ptr [r9 + 72]
    fabs
    fchs
    faddp st(1), st(0)
    fld qword ptr [r9 + 8]
    fadd qword ptr [r9 + 80]
    fabs
    fsub st(0), st(2)
    nop
    fabs
    fchs
    fadd st(0), st(2)
    fld qword ptr [r9 + 80]
    fabs
    fchs
    faddp st(1), st(0)
    fld qword ptr [r9 + 16]
    fstp st(3)
    fxch st(1)
    fxch st(2)
    fld st(0)
    fmul st(0), st(1)
    fld st(2)
    fmul st(0), st(3)
    faddp st(1), st(0)
    fld st(3)
    fmul st(0), st(4)
    faddp st(1), st(0)
    fmul qword ptr [r9 + 88]
    fcom qword ptr [r9 + 48]
    fnstsw ax
    shr ah, 1
    jae asurf_96
    fstp st(0)
    fld qword ptr [r9 + 40]
    jmp asurf_ac
asurf_96:
    fld1
    fcom st(1)
    fnstsw ax
    shr ah, 1
    jb asurf_a7
    fstp st(0)
    fdivr qword ptr [r9 + 192]
    jmp asurf_ac
asurf_a7:
    fcompp
    fld qword ptr [r9 + 192]
asurf_ac:
    fld qword ptr [r9 + 24]
    fmul st(0), st(1)
    fstp qword ptr [r9 + 24]
    fmul st(3), st(0)
    fmul st(2), st(0)
    fmulp st(1), st(0)
    fld st(0)
    fmul qword ptr [r9 + 144]
    fld st(2)
    fmul qword ptr [r9 + 152]
    faddp st(1), st(0)
    fld st(3)
    fmul qword ptr [r9 + 160]
    faddp st(1), st(0)
    fld st(1)
    fmul qword ptr [r9 + 96]
    fld st(3)
    fmul qword ptr [r9 + 104]
    faddp st(1), st(0)
    fld st(4)
    fmul qword ptr [r9 + 112]
    faddp st(1), st(0)
    fxch st(2)
    fmul qword ptr [r9 + 120]
    fxch st(3)
    fmul qword ptr [r9 + 128]
    faddp st(3), st(0)
    fxch st(3)
    fmul qword ptr [r9 + 136]
    faddp st(2), st(0)
    nop
    fadd qword ptr [r9 + 184]
    fstp qword ptr [r9 + 16]
    fadd qword ptr [r9 + 168]
    fstp qword ptr [r9 + 8]
    fadd qword ptr [r9 + 176]
    fstp qword ptr [r9 + 0]
    fldcw word ptr [rsp]
    add rsp, 16
    ret
ASurfMod1_x87 ENDP

; loxodromeIFS offsets 011D..0131: keep both products and the intervening
; addition in Pascal extended precision. Large counters can cancel to ~0.1.
; void LoxodromeTrap_x87(double* out, const double* values5)
LoxodromeTrap_x87 PROC
    sub rsp, 16
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 0137Fh
    fldcw word ptr [rsp + 2]
    fld qword ptr [rdx]
    fmul qword ptr [rdx + 8]
    fadd qword ptr [rdx + 16]
    fld qword ptr [rdx + 24]
    fmul qword ptr [rdx + 32]
    faddp st(1), st(0)
    fabs
    fstp qword ptr [rcx]
    fldcw word ptr [rsp]
    add rsp, 16
    ret
LoxodromeTrap_x87 ENDP

; BT1Pinehedron / BT1Pinegon identical offsets 0019..00AD, one T1 iteration.
; State: xyz,sqrt(2/3),sqrt(1/3),sqrt(1/2),scale,xOff.
; Keep cancellation tails in x87 precision before the original Double stores.
BT1PinehedronT1_x87 PROC
    sub rsp, 16
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 0137Fh
    fldcw word ptr [rsp + 2]
    mov r9, rcx
    fld      qword ptr [r9]
    fmul     qword ptr [r9 + 24]
    fld      qword ptr [r9 + 16]
    fmul     qword ptr [r9 + 32]
    fchs
    faddp st(1), st(0)
    fmul     qword ptr [r9 + 40]
    fstp     qword ptr [rsp + 8]
    fld      qword ptr [r9]
    fmul     qword ptr [r9 + 32]
    fld      qword ptr [r9 + 16]
    fmul     qword ptr [r9 + 24]
    faddp st(1), st(0)
    fabs
    fld      qword ptr [rsp + 8]
    fld      qword ptr [r9 + 8]
    fmul     qword ptr [r9 + 40]
    fchs
    faddp st(1), st(0)
    fabs
    fld      qword ptr [rsp + 8]
    fld      qword ptr [r9 + 8]
    fmul     qword ptr [r9 + 40]
    faddp st(1), st(0)
    fabs
    fstp     qword ptr [r9 + 8]
    fstp     qword ptr [r9]
    fstp     qword ptr [r9 + 16]
    fld      qword ptr [r9]
    fadd     qword ptr [r9 + 8]
    fmul     qword ptr [r9 + 40]
    fstp     qword ptr [rsp + 8]
    fld      qword ptr [r9]
    fchs
    fadd     qword ptr [r9 + 8]
    fmul     qword ptr [r9 + 40]
    fld      qword ptr [rsp + 8]
    fmul     qword ptr [r9 + 24]
    fld      qword ptr [r9 + 16]
    fmul     qword ptr [r9 + 32]
    faddp st(1), st(0)
    fld      qword ptr [rsp + 8]
    fmul     qword ptr [r9 + 32]
    fchs
    fld      qword ptr [r9 + 16]
    fmul     qword ptr [r9 + 24]
    faddp st(1), st(0)
    fstp     qword ptr [r9 + 16]
    fstp     qword ptr [r9]
    fstp     qword ptr [r9 + 8]
    fld      qword ptr [r9]
    fmul     qword ptr [r9 + 48]
    fsub     qword ptr [r9 + 56]
    fstp     qword ptr [r9]
    fld      qword ptr [r9 + 8]
    fmul     qword ptr [r9 + 48]
    fstp     qword ptr [r9 + 8]
    fld      qword ptr [r9 + 16]
    fmul     qword ptr [r9 + 48]
    fstp     qword ptr [r9 + 16]
    fldcw word ptr [rsp]
    add rsp, 16
    ret
BT1PinehedronT1_x87 ENDP

; CalcMonteCarlo.CalcN: GetRand * 1.1 - 0.1 > Clamp0D(dT2).
; Both untyped real literals and the comparison remain Extended in Pascal.
; int MCAdaptiveRandomSkip_x87(const double* random_and_threshold)
MCAdaptiveRandomSkip_x87 PROC
    sub rsp, 16
    fnstcw word ptr [rsp]
    mov ax, word ptr [rsp]
    and ax, 0FCFFh
    or ax, 0300h
    mov word ptr [rsp + 2], ax
    fldcw word ptr [rsp + 2]
    fld tbyte ptr kMCOnePointOne
    fmul qword ptr [rcx]
    fld tbyte ptr kMCPointOne
    fsubp st(1), st(0)
    fcomp qword ptr [rcx + 8]
    fnstsw ax
    sahf
    seta al
    movzx eax, al
    fldcw word ptr [rsp]
    add rsp, 16
    ret
MCAdaptiveRandomSkip_x87 ENDP

; helistairsIFS angular FIST and height FRNDINT must keep x87 intermediates.
; state: x,y,z,n,height,invTwoPi,twoPi; returns u,zPeriodic in first two lanes.
HelistairsSnap_x87 PROC
    sub rsp, 32
    fnstcw word ptr [rsp]
    mov word ptr [rsp+2], 0137Fh
    fldcw word ptr [rsp+2]
    fld qword ptr [rcx+8]
    fld qword ptr [rcx]
    fpatan
    fmul qword ptr [rcx+40]
    fmul qword ptr [rcx+24]
    fist dword ptr [rsp+4]
    fild dword ptr [rsp+4]
    fsubp st(1), st(0)
    fmul qword ptr [rcx+48]
    fdiv qword ptr [rcx+24]
    fstp qword ptr [rcx]
    fld qword ptr [rcx+32]
    fabs
    fchs
    fmul qword ptr [rcx+24]
    fabs
    fstp qword ptr [rsp+8]
    fld qword ptr [rcx+16]
    fild dword ptr [rsp+4]
    fld qword ptr [rcx+32]
    fabs
    fchs
    fmulp st(1), st(0)
    faddp st(1), st(0)
    fld st(0)
    fdiv qword ptr [rsp+8]
    frndint
    fmul qword ptr [rsp+8]
    fsubp st(1), st(0)
    fstp qword ptr [rcx+8]
    fldcw word ptr [rsp]
    add rsp, 32
    ret
HelistairsSnap_x87 ENDP

; state: coordinate, offset1, scale1, scale2, offset2, inv2pi, pi/2,
; coefficient, accumulator. EDX enables the final addition (_PlusSinApp).
; Literal arithmetic order from the original 32-bit formula assets.
SineApproxTransform_x87 PROC
    sub rsp, 16
    fnstcw word ptr [rsp]
    mov word ptr [rsp+2], 0137Fh
    fldcw word ptr [rsp+2]
    xor r8d, r8d
    fld qword ptr [rcx]
    fsub qword ptr [rcx+8]
    fmul qword ptr [rcx+16]
    fmul qword ptr [rcx+40]
    fld st(0)
    fsub qword ptr kHalf
    frndint
    fsubp st(1), st(0)
    fadd st(0), st(0)
    fld1
    fcomp st(1)
    fnstsw ax
    shr ah, 1
    jae sine_approx_pi
    fld1
    fadd st(0), st(0)
    fsubrp st(1), st(0)
    xor r8d, 1
sine_approx_pi:
    fldpi
    fmulp st(1), st(0)
    fcom qword ptr [rcx+48]
    fnstsw ax
    shr ah, 1
    jb sine_approx_poly
    fldpi
    fsubp st(1), st(0)
    xor r8d, 1
sine_approx_poly:
    fld st(0)
    fmul st(0), st(0)
    fmul st(0), st(1)
    fmul qword ptr [rcx+56]
    fsubp st(1), st(0)
    test r8d, r8d
    jz sine_approx_output
    fchs
sine_approx_output:
    fmul qword ptr [rcx+24]
    fadd qword ptr [rcx+32]
    test edx, edx
    jz sine_approx_store
    fadd qword ptr [rcx+64]
sine_approx_store:
    fstp qword ptr [rcx]
    fldcw word ptr [rsp]
    add rsp, 16
    ret
SineApproxTransform_x87 ENDP

; state: x, y, z, power, xMul, yMul, zMul. Output overwrites x.
; Literal _setw asset ordering, including the Double stores after x and y.
SetWTransform_x87 PROC
    sub rsp, 16
    fnstcw word ptr [rsp]
    mov word ptr [rsp+2], 0137Fh
    fldcw word ptr [rsp+2]
    test edx, edx
    jz setw_linear
    fld qword ptr [rcx+24]
    fld qword ptr [rcx+0]
    fabs
    fyl2x
    fld1
    fld st(1)
    fprem
    f2xm1
    faddp st(1), st(0)
    fscale
    fxch st(1)
    fstp st(0)
    fmul qword ptr [rcx+32]
    fstp qword ptr [rsp+8]
    fld qword ptr [rcx+24]
    fld qword ptr [rcx+8]
    fabs
    fyl2x
    fld1
    fld st(1)
    fprem
    f2xm1
    faddp st(1), st(0)
    fscale
    fxch st(1)
    fstp st(0)
    fmul qword ptr [rcx+40]
    fadd qword ptr [rsp+8]
    fstp qword ptr [rsp+8]
    fld qword ptr [rcx+24]
    fld qword ptr [rcx+16]
    fabs
    fyl2x
    fld1
    fld st(1)
    fprem
    f2xm1
    faddp st(1), st(0)
    fscale
    fxch st(1)
    fstp st(0)
    fmul qword ptr [rcx+48]
    fadd qword ptr [rsp+8]
    fld qword ptr [rcx+32]
    fadd qword ptr [rcx+40]
    fadd qword ptr [rcx+48]
    fdivp st(1), st(0)
    fld qword ptr [rcx+24]
    fld1
    fdivp st(1), st(0)
    fxch st(1)
    fyl2x
    fld1
    fld st(1)
    fprem
    f2xm1
    faddp st(1), st(0)
    fscale
    fxch st(1)
    fstp st(0)
    jmp setw_store
setw_linear:
    fld qword ptr [rcx]
    fmul qword ptr [rcx+32]
    fld qword ptr [rcx+8]
    fmul qword ptr [rcx+40]
    fld qword ptr [rcx+16]
    fmul qword ptr [rcx+48]
    faddp st(1), st(0)
    faddp st(1), st(0)
    fld qword ptr [rcx+32]
    fadd qword ptr [rcx+40]
    fadd qword ptr [rcx+48]
    fdivp st(1), st(0)
setw_store:
    fstp qword ptr [rcx]
    fldcw word ptr [rsp]
    add rsp, 16
    ret
SetWTransform_x87 ENDP

; BT1Pinegon 00C8..019F. State: xyz,angle,sides,pi,px,py,pz,Cxyz.
; flags: rotation=1, polygon=2. Branch selection uses original signed bytes.
BT1PinegonTail_x87 PROC
    sub rsp, 48
    fnstcw word ptr [rsp]
    mov word ptr [rsp+2], 0137Fh
    fldcw word ptr [rsp+2]
    test edx, 1
    jz pinegon_polygon
    fld qword ptr [rcx+24]
    fsincos
    fstp qword ptr [rsp+8]
    fst qword ptr [rsp+16]
    fld qword ptr [rcx+16]
    fmulp st(1), st(0)
    fld qword ptr [rcx+8]
    fmul qword ptr [rsp+8]
    faddp st(1), st(0)
    fld qword ptr [rcx+8]
    fmul qword ptr [rsp+16]
    fld qword ptr [rcx+16]
    fmul qword ptr [rsp+8]
    fchs
    faddp st(1), st(0)
    fstp qword ptr [rcx+8]
    fstp qword ptr [rcx+16]
pinegon_polygon:
    test edx, 2
    jz pinegon_pine
    fld qword ptr [rcx+40]
    fdiv qword ptr [rcx+32]
    fst qword ptr [rsp+8]
    fadd st(0), st(0)
    fstp qword ptr [rsp+16]
    fld qword ptr [rcx+16]
    fld qword ptr [rcx+8]
    fpatan
    fabs
pinegon_reduce:
    fcom qword ptr [rsp+8]
    fnstsw ax
    and ah, 41h
    jg pinegon_cos
    fsub qword ptr [rsp+16]
    jmp pinegon_reduce
pinegon_cos:
    fcos
    fld st(0)
    fmul qword ptr [rcx+8]
    fstp qword ptr [rcx+8]
    fmul qword ptr [rcx+16]
    fstp qword ptr [rcx+16]
pinegon_pine:
    fld qword ptr [rcx]
    fmul st(0), st(0)
    fstp qword ptr [rsp+8]
    fld qword ptr [rcx+8]
    fmul st(0), st(0)
    fstp qword ptr [rsp+16]
    fld qword ptr [rcx+16]
    fmul st(0), st(0)
    fstp qword ptr [rsp+24]
    fld qword ptr [rcx]
    fadd st(0), st(0)
    fld qword ptr [rsp+16]
    fadd qword ptr [rsp+24]
    fsqrt
    fdivp st(1), st(0)
    fstp qword ptr [rsp+32]
    fld qword ptr [rsp+8]
    fsub qword ptr [rsp+16]
    fsub qword ptr [rsp+24]
    fld qword ptr [rsp+32]
    fld qword ptr [rsp+16]
    fsub qword ptr [rsp+24]
    fmulp st(1), st(0)
    fld qword ptr [rsp+32]
    fadd st(0), st(0)
    fmul qword ptr [rcx+16]
    fmul qword ptr [rcx+8]
    fld qword ptr [rcx+80]
    fmul qword ptr [rcx+64]
    faddp st(1), st(0)
    fstp qword ptr [rcx+8]
    fld qword ptr [rcx+88]
    fmul qword ptr [rcx+56]
    faddp st(1), st(0)
    fstp qword ptr [rcx+16]
    fld qword ptr [rcx+72]
    fmul qword ptr [rcx+48]
    faddp st(1), st(0)
    fstp qword ptr [rcx]
    fldcw word ptr [rsp]
    add rsp, 48
    ret
BT1PinegonTail_x87 ENDP

; Math3D.MakeWNormalsFromDVec, including FISTP m16 indefinite results.
PackNormals_x87 PROC
    sub rsp, 16
    fnstcw word ptr [rsp]
    mov word ptr [rsp+2], 0137Fh
    fldcw word ptr [rsp+2]
    fld qword ptr [rdx]
    fld st(0)
    fmul st(0), st(0)
    fld qword ptr [rdx+8]
    fld st(0)
    fmul st(0), st(0)
    faddp st(2), st(0)
    fld qword ptr [rdx+16]
    fld st(0)
    fmul st(0), st(0)
    faddp st(3), st(0)
    fxch st(2)
    fadd qword ptr kNormalEpsilon
    fsqrt
    fdivr qword ptr kNormalScale
    fmul st(3), st(0)
    fmul st(2), st(0)
    fmulp st(1), st(0)
    fistp word ptr [rcx+2]
    fistp word ptr [rcx+4]
    fistp word ptr [rcx]
    fldcw word ptr [rsp]
    add rsp, 16
    ret
PackNormals_x87 ENDP

; cayleySurfIFS original 004B..0261, with expanded parameter addressing.
; RCX = transformed XYZ and output DE; RDX = mapped params; R8D = apply.
; Preserve original x87 intermediate precision and every Double store.
CayleySurface_x87 PROC
    sub rsp, 64
    fnstcw word ptr [rsp+48]
    mov word ptr [rsp+50], 0137Fh
    fldcw word ptr [rsp+50]
    mov rax, qword ptr [rcx+0]
    mov qword ptr [rsp+0], rax
    mov rax, qword ptr [rcx+8]
    mov qword ptr [rsp+8], rax
    mov rax, qword ptr [rcx+16]
    mov qword ptr [rsp+16], rax
    fldz
    fst qword ptr [rsp + 24]
    fstp qword ptr [rsp + 32]
    fld qword ptr [rsp + 16]
    fld qword ptr [rsp + 8]
    fld qword ptr [rsp]
    fld st(0)
    fmul qword ptr [rdx+104]
    fld st(2)
    fmul qword ptr [rdx+112]
    faddp st(1), st(0)
    fld st(3)
    fmul qword ptr [rdx+120]
    faddp st(1), st(0)
    fld st(1)
    fmul qword ptr [rdx+56]
    fld st(3)
    fmul qword ptr [rdx+64]
    faddp st(1), st(0)
    fld st(4)
    fmul qword ptr [rdx+72]
    faddp st(1), st(0)
    fxch st(2)
    fmul qword ptr [rdx+80]
    fxch st(3)
    fmul qword ptr [rdx+88]
    faddp st(3), st(0)
    fxch st(3)
    fmul qword ptr [rdx+96]
    faddp st(2), st(0)
    fstp qword ptr [rsp]
    fstp qword ptr [rsp + 8]
    fstp qword ptr [rsp + 16]
    fld qword ptr [rsp]
    fmul qword ptr [rsp]
    fmul qword ptr [rsp + 8]
    ftst
    wait
    fnstsw ax
    shr ah, 1
    jae cayley_c7
    fsubr qword ptr [rsp + 32]
    fstp qword ptr [rsp + 32]
    jmp cayley_cf
cayley_c7:
    fadd qword ptr [rsp + 24]
    fstp qword ptr [rsp + 24]
cayley_cf:
    fld qword ptr [rsp]
    fmul qword ptr [rsp]
    fmul qword ptr [rsp + 16]
    ftst
    wait
    fnstsw ax
    shr ah, 1
    jae cayley_ec
    fsubr qword ptr [rsp + 32]
    fstp qword ptr [rsp + 32]
    jmp cayley_f4
cayley_ec:
    fadd qword ptr [rsp + 24]
    fstp qword ptr [rsp + 24]
cayley_f4:
    fld qword ptr [rsp + 8]
    fmul qword ptr [rsp + 8]
    fmul qword ptr [rsp]
    ftst
    wait
    fnstsw ax
    shr ah, 1
    jae cayley_112
    fsubr qword ptr [rsp + 32]
    fstp qword ptr [rsp + 32]
    jmp cayley_11a
cayley_112:
    fadd qword ptr [rsp + 24]
    fstp qword ptr [rsp + 24]
cayley_11a:
    fld qword ptr [rsp + 8]
    fmul qword ptr [rsp + 8]
    fmul qword ptr [rsp + 16]
    ftst
    wait
    fnstsw ax
    shr ah, 1
    jae cayley_139
    fsubr qword ptr [rsp + 32]
    fstp qword ptr [rsp + 32]
    jmp cayley_141
cayley_139:
    fadd qword ptr [rsp + 24]
    fstp qword ptr [rsp + 24]
cayley_141:
    fld qword ptr [rsp + 16]
    fmul qword ptr [rsp + 16]
    fmul qword ptr [rsp + 8]
    ftst
    wait
    fnstsw ax
    shr ah, 1
    jae cayley_160
    fsubr qword ptr [rsp + 32]
    fstp qword ptr [rsp + 32]
    jmp cayley_168
cayley_160:
    fadd qword ptr [rsp + 24]
    fstp qword ptr [rsp + 24]
cayley_168:
    fld qword ptr [rsp + 16]
    fmul qword ptr [rsp + 16]
    fmul qword ptr [rsp]
    ftst
    wait
    fnstsw ax
    shr ah, 1
    jae cayley_186
    fsubr qword ptr [rsp + 32]
    fstp qword ptr [rsp + 32]
    jmp cayley_18e
cayley_186:
    fadd qword ptr [rsp + 24]
    fstp qword ptr [rsp + 24]
cayley_18e:
    fld qword ptr [rsp + 24]
    fmul qword ptr [rdx+40]
    fabs
    fstp qword ptr [rsp + 24]
    fld qword ptr [rsp + 32]
    fmul qword ptr [rdx+40]
    fabs
    fstp qword ptr [rsp + 32]
    fnop
    fld qword ptr [rsp]
    fmul qword ptr [rsp + 8]
    fadd st(0), st(0)
    ftst
    wait
    fnstsw ax
    shr ah, 1
    jae cayley_1c6
    fsubr qword ptr [rsp + 24]
    fstp qword ptr [rsp + 24]
    jmp cayley_1ce
cayley_1c6:
    fadd qword ptr [rsp + 32]
    fstp qword ptr [rsp + 32]
cayley_1ce:
    fld qword ptr [rsp]
    fmul qword ptr [rsp + 16]
    fadd st(0), st(0)
    ftst
    wait
    fnstsw ax
    shr ah, 1
    jae cayley_1ea
    fsubr qword ptr [rsp + 24]
    fstp qword ptr [rsp + 24]
    jmp cayley_1f2
cayley_1ea:
    fadd qword ptr [rsp + 32]
    fstp qword ptr [rsp + 32]
cayley_1f2:
    fld qword ptr [rsp + 8]
    fmul qword ptr [rsp + 16]
    fadd st(0), st(0)
    ftst
    wait
    fnstsw ax
    shr ah, 1
    jae cayley_20f
    fsubr qword ptr [rsp + 24]
    fstp qword ptr [rsp + 24]
    jmp cayley_217
cayley_20f:
    fadd qword ptr [rsp + 32]
    fstp qword ptr [rsp + 32]
cayley_217:
    fnop
    fld qword ptr [rsp + 24]
    fsqrt
    fsqrt
    fld qword ptr [rsp + 32]
    fsqrt
    fsqrt
    fchs
    faddp st(1), st(0)
    fmul qword ptr [rdx+128]
    fld qword ptr [rsp]
    fmul st(0), st(0)
    fld qword ptr [rsp + 8]
    fmul st(0), st(0)
    fld qword ptr [rsp + 16]
    fmul st(0), st(0)
    faddp st(1), st(0)
    faddp st(1), st(0)
    fsqrt
    fsub qword ptr [rdx+0]
    fcom st(1)
    wait
    fnstsw ax
    shr ah, 1
    jae cayley_256
    fstp st(0)
    jmp cayley_258
cayley_256:
    fstp st(1)
cayley_258:
    cmp r8d, 0
    jne cayley_261
    fdiv qword ptr [rdx+8]
cayley_261:
    fstp qword ptr [rcx+24]
    fldcw word ptr [rsp+48]
    add rsp, 64
    ret
CayleySurface_x87 ENDP

; Original foldcut8-IFS.m3f 004B..00F2, expanded parameter offsets.
; Avoid a Double spill before small-scale division after plane cancellation.
Foldcut8Distance_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp+2], 137Fh
    fldcw word ptr [rsp+2]
    fld qword ptr [rcx]
    fabs
    fld qword ptr [rcx+8]
    fabs
    fld qword ptr [rcx+16]
    fabs
    fcom st(1)
    fnstsw ax
    and ah, 41h
    je foldcut_068
    fxch st(1)
foldcut_068:
    fcom st(2)
    fnstsw ax
    and ah, 41h
    je foldcut_074
    fxch st(2)
foldcut_074:
    fxch st(1)
    fcom st(2)
    fnstsw ax
    and ah, 41h
    je foldcut_082
    fxch st(2)
foldcut_082:
    fld qword ptr [rdx+64]
    fmul st(3), st(0)
    fstp st(0)
    fld qword ptr [rdx+56]
    fmul st(2), st(0)
    fstp st(0)
    fld qword ptr [rdx+72]
    fmul st(1), st(0)
    fstp st(0)
    faddp st(1), st(0)
    faddp st(1), st(0)
    fsub qword ptr [rdx]
    fld qword ptr [rcx]
    fmul st(0), st(0)
    fld qword ptr [rcx+8]
    fmul st(0), st(0)
    faddp st(1), st(0)
    fld qword ptr [rcx+16]
    fmul st(0), st(0)
    faddp st(1), st(0)
    fsqrt
    fld qword ptr [rdx+48]
    fabs
    fsubp st(1), st(0)
    test dword ptr [rdx+52], 80000000h
    jne foldcut_0d9
    fcom st(1)
    fnstsw ax
    and ah, 41h
    je foldcut_0d5
    fstp st(1)
    jmp foldcut_0e9
foldcut_0d5:
    fstp st(0)
    jmp foldcut_0e9
foldcut_0d9:
    fcom st(1)
    fnstsw ax
    and ah, 41h
    je foldcut_0e7
    fstp st(0)
    jmp foldcut_0e9
foldcut_0e7:
    fstp st(1)
foldcut_0e9:
    test r8d, r8d
    jne foldcut_0f2
    fdiv qword ptr [rdx+8]
foldcut_0f2:
    fstp qword ptr [rcx+24]
    fldcw word ptr [rsp]
    add rsp, 8
    ret
Foldcut8Distance_x87 ENDP

; Original gearIFS.m3f 004B..00DF, after its SSE2 position transform.
; state[0..2] = transformed XYZ, state[3] = Rout; params are expanded doubles.
; Keep the Angle division (including zero), FRNDINT, radius Double spill,
; and unordered branch behavior. Restore the caller's x87 control word.
GearDistance_x87 PROC
    sub rsp, 16
    fnstcw word ptr [rsp+8]
    mov word ptr [rsp+10], 137Fh
    fldcw word ptr [rsp+10]
    fld qword ptr [rcx+8]
    fld qword ptr [rcx]
    fpatan
    fmul qword ptr [rdx+96]
    fdiv qword ptr [rdx+56]
    fld st(0)
    frndint
    fsubp st(1), st(0)
    fmul qword ptr [rdx+56]
    fabs
    fdiv qword ptr [rdx]
    fld1
    fsubp st(1), st(0)
    fmul qword ptr [rdx]
    fmul qword ptr [rdx+104]
    fchs
    fld qword ptr [rcx]
    fmul st(0), st(0)
    fld qword ptr [rcx+8]
    fmul st(0), st(0)
    faddp st(1), st(0)
    fsqrt
    fst qword ptr [rsp]
    fsub qword ptr [rdx+40]
    fadd qword ptr [rdx+64]
    fabs
    fsub qword ptr [rdx+64]
    fcom st(1)
    fnstsw ax
    shr ah, 1
    jae gear_09e
    fstp st(0)
    jmp gear_0a0
gear_09e:
    fstp st(1)
gear_0a0:
    fld qword ptr [rsp]
    fsub qword ptr [rdx+80]
    fadd qword ptr [rdx+88]
    fabs
    fsub qword ptr [rdx+88]
    fcom st(1)
    fnstsw ax
    shr ah, 1
    jb gear_0bc
    fstp st(0)
    jmp gear_0be
gear_0bc:
    fstp st(1)
gear_0be:
    fld qword ptr [rcx+16]
    fabs
    fsub qword ptr [rdx+72]
    fcom st(1)
    fnstsw ax
    shr ah, 1
    jae gear_0d4
    fstp st(0)
    jmp gear_0d6
gear_0d4:
    fstp st(1)
gear_0d6:
    test r8d, r8d
    jne gear_0df
    fdiv qword ptr [rdx+8]
gear_0df:
    fstp qword ptr [rcx+24]
    fldcw word ptr [rsp+8]
    add rsp, 16
    ret
GearDistance_x87 ENDP

; gear2IFS original 004B..00EF; radius subtraction spills before angle.
Gear2Distance_x87 PROC
    sub rsp, 16
    fnstcw word ptr [rsp+8]
    mov word ptr [rsp+10], 137Fh
    fldcw word ptr [rsp+10]
    fld qword ptr [rcx]
    fmul st(0), st(0)
    fld qword ptr [rcx+8]
    fmul st(0), st(0)
    faddp st(1), st(0)
    fsqrt
    fsub qword ptr [rdx+40]
    fstp qword ptr [rsp]
    fld qword ptr [rcx+8]
    fld qword ptr [rcx]
    fpatan
    fmul qword ptr [rdx+96]
    fld qword ptr [rsp]
    ftst
    fnstsw ax
    sahf
    jae gear2_angle_ready
    fxch st(1)
    fsub qword ptr [rdx+80]
    fxch st(1)
gear2_angle_ready:
    fstp st(0)
    fdiv qword ptr [rdx+56]
    fld st(0)
    frndint
    fsubp st(1), st(0)
    fmul qword ptr [rdx+56]
    fabs
    fdiv qword ptr [rdx]
    fld1
    fsubp st(1), st(0)
    fmul qword ptr [rdx]
    fmul qword ptr [rdx+104]
    fchs
    fld qword ptr [rsp]
    fabs
    fsub qword ptr [rdx+64]
    fcom st(1)
    fnstsw ax
    shr ah, 1
    jae gear2_09e
    fstp st(0)
    jmp gear2_0a0
gear2_09e:
    fstp st(1)
gear2_0a0:
    fld qword ptr [rsp]
    fabs
    fsub qword ptr [rdx+88]
    fcom st(1)
    fnstsw ax
    shr ah, 1
    jb gear2_0bc
    fstp st(0)
    jmp gear2_0be
gear2_0bc:
    fstp st(1)
gear2_0be:
    fld qword ptr [rcx+16]
    fabs
    fsub qword ptr [rdx+72]
    fcom st(1)
    fnstsw ax
    shr ah, 1
    jae gear2_0d4
    fstp st(0)
    jmp gear2_0d6
gear2_0d4:
    fstp st(1)
gear2_0d6:
    test r8d, r8d
    jne gear2_0df
    fdiv qword ptr [rdx+8]
gear2_0df:
    fstp qword ptr [rcx+24]
    fldcw word ptr [rsp+8]
    add rsp, 16
    ret
Gear2Distance_x87 ENDP

; transLogSpIFS.m3f: original straight-line x87 code with expanded parameters.
; Integer options and Single matrix entries expand exactly to Double.
TransLogSpiral_x87 PROC
    sub rsp, 48
    fnstcw word ptr [rsp+40]
    mov word ptr [rsp+42], 137Fh
    fldcw word ptr [rsp+42]
    fld qword ptr [rcx+16] ; 0000
    fadd qword ptr [rdx+16] ; 0003
    fld qword ptr [rcx+8] ; 0006
    fadd qword ptr [rdx+8] ; 0009
    fld qword ptr [rcx] ; 000C
    fadd qword ptr [rdx+0] ; 000F
    fld st(0) ; 0012
    fmul qword ptr [rdx+72] ; 0014
    fld st(2) ; 0017
    fmul qword ptr [rdx+80] ; 0019
    faddp st(1), st(0) ; 001C
    fld st(3) ; 001E
    fmul qword ptr [rdx+88] ; 0020
    faddp st(1), st(0) ; 0023
    fld st(1) ; 0025
    fmul qword ptr [rdx+24] ; 0027
    fld st(3) ; 002A
    fmul qword ptr [rdx+32] ; 002C
    faddp st(1), st(0) ; 002F
    fld st(4) ; 0031
    fmul qword ptr [rdx+40] ; 0033
    faddp st(1), st(0) ; 0036
    fxch st(2) ; 0038
    fmul qword ptr [rdx+48] ; 003A
    fxch st(3) ; 003D
    fmul qword ptr [rdx+56] ; 003F
    faddp st(3), st(0) ; 0042
    fxch st(3) ; 0044
    fmul qword ptr [rdx+64] ; 0046
    faddp st(2), st(0) ; 0049
    fstp qword ptr [rcx] ; 004B
    fstp qword ptr [rcx+8] ; 004E
    fstp qword ptr [rcx+16] ; 0051
    fld qword ptr [rcx] ; 0057
    fmul st(0), st(0) ; 005A
    fld qword ptr [rcx+8] ; 005C
    fmul st(0), st(0) ; 005F
    faddp st(1), st(0) ; 0061
    fsqrt  ; 0063
    fmul qword ptr [rdx+120] ; 0065
    fst qword ptr [rsp + 16] ; 0068
    fld qword ptr [rdx+112] ; 006C
    fxch st(1) ; 006F
    fyl2x  ; 0071
    fmul qword ptr [rdx+128] ; 0073
    fstp qword ptr [rsp] ; 0075
    fld qword ptr [rcx+8] ; 0078
    fld qword ptr [rcx] ; 007B
    fpatan  ; 007E
    fmul qword ptr [rdx+128] ; 0080
    fstp qword ptr [rsp + 8] ; 0082
    fld qword ptr [rdx+96] ; 0086
    fmul qword ptr [rsp] ; 0089
    fld qword ptr [rdx+104] ; 008C
    fmul qword ptr [rsp + 8] ; 008F
    faddp st(1), st(0) ; 0093
    fld st(0) ; 0095
    frndint  ; 0097
    fsubrp st(1), st(0) ; 0099
    fmul qword ptr [rdx+136] ; 009B
    fstp qword ptr [rcx] ; 009E
    fld qword ptr [rdx+104] ; 00A1
    fmul qword ptr [rsp] ; 00A4
    fld qword ptr [rdx+96] ; 00A7
    fmul qword ptr [rsp + 8] ; 00AA
    fchs  ; 00AE
    faddp st(1), st(0) ; 00B0
    fld st(0) ; 00B2
    frndint  ; 00B4
    fsubrp st(1), st(0) ; 00B6
    fmul qword ptr [rdx+136] ; 00B8
    fstp qword ptr [rcx+8] ; 00BB
    fld qword ptr [rcx+24] ; 00BE
    fld qword ptr [rsp + 16] ; 00C1
    fsqrt  ; 00C5
    fadd qword ptr [rdx+144] ; 00CA
    fdivrp st(1), st(0) ; 00CD
    fld qword ptr [rdx+104] ; 00CF
    fmul st(0), st(0) ; 00D2
    fld qword ptr [rdx+96] ; 00D4
    fmul st(0), st(0) ; 00D7
    faddp st(1), st(0) ; 00D9
    fld qword ptr [rdx+112] ; 00DB
    fmul st(0), st(0) ; 00DE
    faddp st(1), st(0) ; 00E0
    fsqrt  ; 00E2
    fmulp st(1), st(0) ; 00E4
    fmul qword ptr [rdx+152] ; 00E6
    fstp qword ptr [rcx+24] ; 00E9
    fldcw word ptr [rsp+40]
    add rsp, 48
    ret
TransLogSpiral_x87 ENDP

; _RotateXYonItsP.m3f x87 sequence; non-power aliases skip only the power block.
IterationRotation_x87 PROC
    sub rsp, 16
    fnstcw word ptr [rsp]
    mov word ptr [rsp+2], 137Fh
    fldcw word ptr [rsp+2]
    movsd qword ptr [rsp+8], xmm2
    fld qword ptr [rcx+16] ; 000E
    fld qword ptr [rcx+8] ; 0010
    fld qword ptr [rcx] ; 0012
    fld st(0) ; 0014
    fmul qword ptr [rdx+48] ; 0016
    fld st(2) ; 0019
    fmul qword ptr [rdx+56] ; 001B
    faddp st(1), st(0) ; 001E
    fld st(3) ; 0020
    fmul qword ptr [rdx+64] ; 0022
    faddp st(1), st(0) ; 0025
    fld st(1) ; 0027
    fmul qword ptr [rdx+0] ; 0029
    fld st(3) ; 002C
    fmul qword ptr [rdx+8] ; 002E
    faddp st(1), st(0) ; 0031
    fld st(4) ; 0033
    fmul qword ptr [rdx+16] ; 0035
    faddp st(1), st(0) ; 0038
    fxch st(2) ; 003A
    fmul qword ptr [rdx+24] ; 003C
    fxch st(3) ; 003F
    fmul qword ptr [rdx+32] ; 0041
    faddp st(3), st(0) ; 0044
    fxch st(3) ; 0046
    fmul qword ptr [rdx+40] ; 0048
    faddp st(2), st(0) ; 004B
    fstp qword ptr [rcx] ; 004D
    fstp qword ptr [rcx+8] ; 004F
    fstp qword ptr [rcx+16] ; 0051
    fnop  ; 0053
    fld qword ptr [rcx+8] ; 0055
    fld qword ptr [rcx] ; 0057
    fld qword ptr [rsp+8]
    test r9d, r9d
    jz iteration_rotation_linear
    fsub qword ptr [rdx+80] ; 0065
    fld qword ptr [rdx+88] ; 0068
    fxch st(1) ; 00DD
    fabs  ; 00DF
    fldln2  ; 00E1
    fxch st(1) ; 00E3
    fyl2x  ; 00E5
    fxch st(1) ; 00E7
    fmulp st(1), st(0) ; 00E9
    fldl2e  ; 00EB
    fmulp st(1), st(0) ; 00ED
    fld st(0) ; 00EF
    frndint  ; 00F1
    fsub st(1), st(0) ; 00F3
    fxch st(1) ; 00F5
    f2xm1  ; 00F7
    fld1  ; 00F9
    faddp st(1), st(0) ; 00FB
    fscale  ; 00FD
    fstp st(1) ; 00FF
    nop  ; 0070
iteration_rotation_linear:
    fmul qword ptr [rdx+72] ; 0071
    test r9d, r9d
    jz iteration_rotation_linear_constant
    fmul qword ptr [rdx+96] ; 0074
    jmp iteration_rotation_sincos
iteration_rotation_linear_constant:
    fmul qword ptr [rdx+80]
iteration_rotation_sincos:
    fsincos  ; 0076
    fld st(0) ; 0078
    fmul st(0), st(3) ; 007A
    fld st(2) ; 007C
    fmul st(0), st(5) ; 007E
    faddp st(1), st(0) ; 0080
    fstp qword ptr [rcx] ; 0082
    fmulp st(3), st(0) ; 0084
    fmulp st(1), st(0) ; 0086
    fsubp st(1), st(0) ; 0088
    fstp qword ptr [rcx+8] ; 008A
    nop  ; 008C
    fnop  ; 008D
    fld qword ptr [rcx+16] ; 008F
    fld qword ptr [rcx+8] ; 0091
    fld qword ptr [rcx] ; 0093
    fld st(0) ; 0095
    fmul qword ptr [rdx+16] ; 0097
    fld st(2) ; 009A
    fmul qword ptr [rdx+40] ; 009C
    faddp st(1), st(0) ; 009F
    fld st(3) ; 00A1
    fmul qword ptr [rdx+64] ; 00A3
    faddp st(1), st(0) ; 00A6
    fld st(1) ; 00A8
    fmul qword ptr [rdx+0] ; 00AA
    fld st(3) ; 00AD
    fmul qword ptr [rdx+24] ; 00AF
    faddp st(1), st(0) ; 00B2
    fld st(4) ; 00B4
    fmul qword ptr [rdx+48] ; 00B6
    faddp st(1), st(0) ; 00B9
    fxch st(2) ; 00BB
    fmul qword ptr [rdx+8] ; 00BD
    fxch st(3) ; 00C0
    fmul qword ptr [rdx+32] ; 00C2
    faddp st(3), st(0) ; 00C5
    fxch st(3) ; 00C7
    fmul qword ptr [rdx+56] ; 00C9
    faddp st(2), st(0) ; 00CC
    fstp qword ptr [rcx] ; 00CE
    fstp qword ptr [rcx+8] ; 00D0
    fstp qword ptr [rcx+16] ; 00D2
    fldcw word ptr [rsp]
    add rsp, 16
    ret
IterationRotation_x87 ENDP

; _TranslateRnd: preserve the extended multiply/subtract/add until the final store.
TranslateRandom_x87 PROC
    sub rsp, 32
    fnstcw word ptr [rsp]
    mov word ptr [rsp+2], 137Fh
    fldcw word ptr [rsp+2]
    movsd qword ptr [rsp+8], xmm0
    movsd qword ptr [rsp+16], xmm1
    movsd qword ptr [rsp+24], xmm2
    fld qword ptr [rsp+24]
    fmul qword ptr [r9+8]
    fsub qword ptr [r9]
    fmul qword ptr [rsp+16]
    fadd qword ptr [rsp+8]
    fstp qword ptr [rsp+8]
    movsd xmm0, qword ptr [rsp+8]
    fldcw word ptr [rsp]
    add rsp, 32
    ret
TranslateRandom_x87 ENDP

; _RotateRnd 005F..00DA: one plane, including unordered comparisons.
RotateRandomPlane_x87 PROC
    sub rsp, 16
    fnstcw word ptr [rsp]
    mov word ptr [rsp+2], 137Fh
    fldcw word ptr [rsp+2]
    movsd qword ptr [rsp+8], xmm2
    fld qword ptr [rdx] ; 005F
    fabs  ; 0062
    fld qword ptr [rdx+8] ; 0064
    fcompp  ; 0066
    fnstsw ax ; 0068
    shr ah, 1 ; 006A
    jb random_plane_dc ; 006C
    mov r8d, 0 ; 006E
    fld qword ptr [rsp+8] ; 0072
    fmul qword ptr [rdx+16] ; 0075
    fsub qword ptr [rdx+24] ; 0078
    fmul qword ptr [rdx] ; 007B
    fld1  ; 007E
    fcomp st(1) ; 0080
    fnstsw ax ; 0082
    shr ah, 1 ; 0084
    jae random_plane_92 ; 0086
    fld qword ptr [rdx+8] ; 0088
    fsubrp st(1), st(0) ; 008A
    mov r8d, 1 ; 008C
    jmp random_plane_a8 ; 0090
random_plane_92:
    fld1  ; 0092
    fchs  ; 0094
    fcomp st(1) ; 0096
    fnstsw ax ; 0098
    shr ah, 1 ; 009A
    jb random_plane_a8 ; 009C
    fld qword ptr [rdx+8] ; 009E
    faddp st(1), st(0) ; 00A0
    fchs  ; 00A2
    mov r8d, 1 ; 00A4
random_plane_a8:
    fld st(0) ; 00A8
    fmul st(0), st(0) ; 00AA
    fld1  ; 00AC
    fsubrp st(1), st(0) ; 00AE
    fsqrt  ; 00B0
    cmp r8d, 0 ; 00B2
    je random_plane_ba ; 00B6
    fchs  ; 00B8
random_plane_ba:
    fld qword ptr [rcx] ; 00BE
    fmul st(0), st(2) ; 00C0
    fld qword ptr [rcx+8] ; 00C2
    fmul st(0), st(2) ; 00C4
    fsubrp st(1), st(0) ; 00C6
    fld qword ptr [rcx] ; 00C8
    fmul st(0), st(2) ; 00CA
    fld qword ptr [rcx+8] ; 00CC
    fmul st(0), st(4) ; 00CE
    faddp st(1), st(0) ; 00D0
    fstp qword ptr [rcx] ; 00D2
    fstp qword ptr [rcx+8] ; 00D4
    fstp st(0) ; 00D6
    fstp st(0) ; 00D8
    fstp st(0) ; 00DA
random_plane_dc:
    fldcw word ptr [rsp]
    add rsp, 16
    ret
RotateRandomPlane_x87 ENDP

.const
kNormalEpsilon real8 1.0e-100
kNormalScale real8 32767.0
kMCOnePointOne db 0CDh,0CCh,0CCh,0CCh,0CCh,0CCh,0CCh,08Ch,0FFh,03Fh
kMCPointOne db 0CDh,0CCh,0CCh,0CCh,0CCh,0CCh,0CCh,0CCh,0FBh,03Fh
kHalf dq 03FE0000000000000h
kInv4096 dq 3F2FFFFFC906404Ch

.code

; hextgridIFS 004B..01DC, original x87 operations and Double stores.
; Expanded Single matrix values are exact doubles; shape rotation is local.
HexTubeGrid_x87 PROC
    sub rsp, 64
    fnstcw word ptr [rsp+48]
    mov word ptr [rsp+50], 137Fh
    fldcw word ptr [rsp+50]
    mov rax, qword ptr [rcx]
    mov qword ptr [rsp], rax
    mov rax, qword ptr [rcx+8]
    mov qword ptr [rsp+8], rax
    mov rax, qword ptr [rcx+16]
    mov qword ptr [rsp+16], rax
hexgrid_4b:
    fldz
hexgrid_4d:
    fstp qword ptr [rsp+32]
hexgrid_54:
    fld qword ptr [rsp+16]
hexgrid_58:
    fld qword ptr [rsp+8]
hexgrid_5c:
    fld qword ptr [rsp]
hexgrid_5f:
    fld st(0)
hexgrid_61:
    fmul qword ptr [rdx+104]
hexgrid_64:
    fld st(2)
hexgrid_66:
    fmul qword ptr [rdx+112]
hexgrid_69:
    faddp st(1), st(0)
hexgrid_6b:
    fld st(3)
hexgrid_6d:
    fmul qword ptr [rdx+120]
hexgrid_70:
    faddp st(1), st(0)
hexgrid_72:
    fld st(1)
hexgrid_74:
    fmul qword ptr [rdx+56]
hexgrid_77:
    fld st(3)
hexgrid_79:
    fmul qword ptr [rdx+64]
hexgrid_7c:
    faddp st(1), st(0)
hexgrid_7e:
    fld st(4)
hexgrid_80:
    fmul qword ptr [rdx+72]
hexgrid_83:
    faddp st(1), st(0)
hexgrid_85:
    fxch st(2)
hexgrid_87:
    fmul qword ptr [rdx+80]
hexgrid_8a:
    fxch st(3)
hexgrid_8c:
    fmul qword ptr [rdx+88]
hexgrid_8f:
    faddp st(3), st(0)
hexgrid_91:
    fxch st(3)
hexgrid_93:
    fmul qword ptr [rdx+96]
hexgrid_96:
    faddp st(2), st(0)
hexgrid_98:
    fstp qword ptr [rsp]
hexgrid_9b:
    fstp qword ptr [rsp+8]
hexgrid_9f:
    fstp qword ptr [rsp+16]
hexgrid_a6:
    fld qword ptr [rsp+16]
hexgrid_aa:
    fmul st(0), st(0)
hexgrid_ac:
    fmul qword ptr [rdx+128]
hexgrid_ae:
    fadd qword ptr [rdx+0]
hexgrid_b1:
    fabs
hexgrid_b3:
    fstp qword ptr [rsp+24]
hexgrid_b7:
    fld qword ptr [rsp+8]
hexgrid_bb:
    fmul qword ptr [rdx+144]
hexgrid_be:
    fcos
hexgrid_c1:
    ftst
hexgrid_c3:
    wait
hexgrid_c4:
    fnstsw ax
hexgrid_c6:
    shr ah, 1
hexgrid_c8:
    jae hexgrid_d4
hexgrid_ca:
    fsubr qword ptr [rsp+32]
hexgrid_ce:
    fstp qword ptr [rsp+32]
hexgrid_d2:
    jmp hexgrid_dc
hexgrid_d4:
    fadd qword ptr [rsp+24]
hexgrid_d8:
    fstp qword ptr [rsp+24]
hexgrid_dc:
    fld qword ptr [rsp]
hexgrid_df:
    fmul qword ptr [rdx+160]
hexgrid_e2:
    fld qword ptr [rsp+8]
hexgrid_e6:
    fmul qword ptr [rdx+152]
hexgrid_e9:
    fsubp st(1), st(0)
hexgrid_eb:
    fcos
hexgrid_ee:
    ftst
hexgrid_f0:
    wait
hexgrid_f1:
    fnstsw ax
hexgrid_f3:
    shr ah, 1
hexgrid_f5:
    jae hexgrid_101
hexgrid_f7:
    fsubr qword ptr [rsp+32]
hexgrid_fb:
    fstp qword ptr [rsp+32]
hexgrid_ff:
    jmp hexgrid_109
hexgrid_101:
    fadd qword ptr [rsp+24]
hexgrid_105:
    fstp qword ptr [rsp+24]
hexgrid_109:
    fld qword ptr [rsp]
hexgrid_10c:
    fmul qword ptr [rdx+160]
hexgrid_10f:
    fld qword ptr [rsp+8]
hexgrid_113:
    fmul qword ptr [rdx+152]
hexgrid_116:
    faddp st(1), st(0)
hexgrid_118:
    fcos
hexgrid_11b:
    ftst
hexgrid_11d:
    wait
hexgrid_11e:
    fnstsw ax
hexgrid_120:
    shr ah, 1
hexgrid_122:
    jae hexgrid_12e
hexgrid_124:
    fsubr qword ptr [rsp+32]
hexgrid_128:
    fstp qword ptr [rsp+32]
hexgrid_12c:
    jmp hexgrid_136
hexgrid_12e:
    fadd qword ptr [rsp+24]
hexgrid_132:
    fstp qword ptr [rsp+24]
hexgrid_136:
    fld qword ptr [rsp]
hexgrid_139:
    fmul qword ptr [rdx+168]
hexgrid_13c:
    fcos
hexgrid_13e:
    fmul qword ptr [rdx+40]
hexgrid_142:
    ftst
hexgrid_144:
    wait
hexgrid_145:
    fnstsw ax
hexgrid_147:
    shr ah, 1
hexgrid_149:
    jae hexgrid_155
hexgrid_14b:
    fsubr qword ptr [rsp+32]
hexgrid_14f:
    fstp qword ptr [rsp+32]
hexgrid_153:
    jmp hexgrid_15d
hexgrid_155:
    fadd qword ptr [rsp+24]
hexgrid_159:
    fstp qword ptr [rsp+24]
hexgrid_15d:
    fld qword ptr [rsp]
hexgrid_160:
    fmul qword ptr [rdx+160]
hexgrid_163:
    fld qword ptr [rsp+8]
hexgrid_167:
    fmul qword ptr [rdx+136]
hexgrid_16a:
    fsubrp st(1), st(0)
hexgrid_16c:
    fcos
hexgrid_16e:
    fmul qword ptr [rdx+40]
hexgrid_172:
    ftst
hexgrid_174:
    wait
hexgrid_175:
    fnstsw ax
hexgrid_177:
    shr ah, 1
hexgrid_179:
    jae hexgrid_185
hexgrid_17b:
    fsubr qword ptr [rsp+32]
hexgrid_17f:
    fstp qword ptr [rsp+32]
hexgrid_183:
    jmp hexgrid_18d
hexgrid_185:
    fadd qword ptr [rsp+24]
hexgrid_189:
    fstp qword ptr [rsp+24]
hexgrid_18d:
    fld qword ptr [rsp]
hexgrid_190:
    fmul qword ptr [rdx+160]
hexgrid_193:
    fld qword ptr [rsp+8]
hexgrid_197:
    fmul qword ptr [rdx+136]
hexgrid_19a:
    faddp st(1), st(0)
hexgrid_19c:
    fcos
hexgrid_19e:
    fmul qword ptr [rdx+40]
hexgrid_1a2:
    ftst
hexgrid_1a4:
    wait
hexgrid_1a5:
    fnstsw ax
hexgrid_1a7:
    shr ah, 1
hexgrid_1a9:
    jae hexgrid_1b5
hexgrid_1ab:
    fsubr qword ptr [rsp+32]
hexgrid_1af:
    fstp qword ptr [rsp+32]
hexgrid_1b3:
    jmp hexgrid_1bd
hexgrid_1b5:
    fadd qword ptr [rsp+24]
hexgrid_1b9:
    fstp qword ptr [rsp+24]
hexgrid_1bd:
    fld qword ptr [rsp+24]
hexgrid_1c1:
    fsqrt
hexgrid_1c5:
    fld qword ptr [rsp+32]
hexgrid_1c9:
    fsqrt
hexgrid_1cd:
    fchs
hexgrid_1cf:
    faddp st(1), st(0)
    test r8d, r8d
    jne hexgrid_store
    fdiv qword ptr [rdx+8]
hexgrid_store:
    fstp qword ptr [rcx+24]
    fldcw word ptr [rsp+48]
    add rsp, 64
    ret
HexTubeGrid_x87 ENDP


; hextgrid2IFS: original x87 body, mapped params, original local spills.
HexTubeGrid2_x87 PROC
    sub rsp, 128
    fnstcw word ptr [rsp+112]
    mov word ptr [rsp+114], 137Fh
    fldcw word ptr [rsp+114]
    mov rax, qword ptr [rcx]
    mov qword ptr [rsp], rax
    mov rax, qword ptr [rcx+8]
    mov qword ptr [rsp+8], rax
    mov rax, qword ptr [rcx+16]
    mov qword ptr [rsp+16], rax
hexgrid2_4b:
    fldz
hexgrid2_4d:
    fstp qword ptr [rsp+32]
hexgrid2_54:
    fld qword ptr [rsp+16]
hexgrid2_58:
    fld qword ptr [rsp+8]
hexgrid2_5c:
    fld qword ptr [rsp]
hexgrid2_5f:
    fld st(0)
hexgrid2_61:
    fmul qword ptr [rdx+104]
hexgrid2_64:
    fld st(2)
hexgrid2_66:
    fmul qword ptr [rdx+112]
hexgrid2_69:
    faddp st(1), st(0)
hexgrid2_6b:
    fld st(3)
hexgrid2_6d:
    fmul qword ptr [rdx+120]
hexgrid2_70:
    faddp st(1), st(0)
hexgrid2_72:
    fld st(1)
hexgrid2_74:
    fmul qword ptr [rdx+56]
hexgrid2_77:
    fld st(3)
hexgrid2_79:
    fmul qword ptr [rdx+64]
hexgrid2_7c:
    faddp st(1), st(0)
hexgrid2_7e:
    fld st(4)
hexgrid2_80:
    fmul qword ptr [rdx+72]
hexgrid2_83:
    faddp st(1), st(0)
hexgrid2_85:
    fxch st(2)
hexgrid2_87:
    fmul qword ptr [rdx+80]
hexgrid2_8a:
    fxch st(3)
hexgrid2_8c:
    fmul qword ptr [rdx+88]
hexgrid2_8f:
    faddp st(3), st(0)
hexgrid2_91:
    fxch st(3)
hexgrid2_93:
    fmul qword ptr [rdx+96]
hexgrid2_96:
    faddp st(2), st(0)
hexgrid2_98:
    fstp qword ptr [rsp]
hexgrid2_9b:
    fstp qword ptr [rsp+8]
hexgrid2_9f:
    fstp qword ptr [rsp+16]
hexgrid2_a6:
    fld qword ptr [rsp+8]
hexgrid2_aa:
    fmul qword ptr [rdx+136]
hexgrid2_ad:
    frndint
hexgrid2_af:
    fmul qword ptr [rdx+128]
hexgrid2_b1:
    fadd qword ptr [rsp+8]
hexgrid2_b5:
    fabs
hexgrid2_b7:
    fst qword ptr [rsp+24]
hexgrid2_bb:
    fmul qword ptr [rdx+136]
hexgrid2_be:
    fld qword ptr [rsp]
hexgrid2_c1:
    fmul qword ptr [rdx+160]
hexgrid2_c4:
    frndint
hexgrid2_c6:
    fmul qword ptr [rdx+152]
hexgrid2_c9:
    fld qword ptr [rsp]
hexgrid2_cc:
    fmul qword ptr [rdx+144]
hexgrid2_cf:
    faddp st(1), st(0)
hexgrid2_d1:
    fabs
hexgrid2_d3:
    faddp st(1), st(0)
hexgrid2_d5:
    fld qword ptr [rsp+24]
hexgrid2_d9:
    fcom st(1)
hexgrid2_db:
    wait
hexgrid2_dc:
    fnstsw ax
hexgrid2_de:
    shr ah, 1
hexgrid2_e0:
    jae hexgrid2_e6
hexgrid2_e2:
    fstp st(0)
hexgrid2_e4:
    jmp hexgrid2_e8
hexgrid2_e6:
    fstp st(1)
hexgrid2_e8:
    fsub qword ptr [rdx+40]
hexgrid2_ee:
    fld qword ptr [rsp+24]
hexgrid2_f2:
    fcom st(1)
hexgrid2_f4:
    wait
hexgrid2_f5:
    fnstsw ax
hexgrid2_f7:
    shr ah, 1
hexgrid2_f9:
    jbe hexgrid2_ff
hexgrid2_fb:
    fstp st(0)
hexgrid2_fd:
    jmp hexgrid2_101
hexgrid2_ff:
    fstp st(1)
hexgrid2_101:
    fmul st(0), st(0)
hexgrid2_103:
    fld qword ptr [rsp+16]
hexgrid2_107:
    fmul st(0), st(0)
hexgrid2_109:
    faddp st(1), st(0)
hexgrid2_10b:
    fsqrt
hexgrid2_10d:
    fsub qword ptr [rdx+0]
    test r8d, r8d
    jne hexgrid2_store
    fdiv qword ptr [rdx+8]
hexgrid2_store:
    fstp qword ptr [rcx+24]
    fldcw word ptr [rsp+112]
    add rsp, 128
    ret
HexTubeGrid2_x87 ENDP


; hextgrid3IFS: original x87 body, mapped params, original local spills.
HexTubeGrid3_x87 PROC
    sub rsp, 128
    fnstcw word ptr [rsp+112]
    mov word ptr [rsp+114], 137Fh
    fldcw word ptr [rsp+114]
    mov rax, qword ptr [rcx]
    mov qword ptr [rsp], rax
    mov rax, qword ptr [rcx+8]
    mov qword ptr [rsp+8], rax
    mov rax, qword ptr [rcx+16]
    mov qword ptr [rsp+16], rax
hexgrid3_4e:
    fld qword ptr [rsp+16]
hexgrid3_52:
    fld qword ptr [rsp+8]
hexgrid3_56:
    fld qword ptr [rsp]
hexgrid3_59:
    fld st(0)
hexgrid3_5b:
    fmul qword ptr [rdx+104]
hexgrid3_5e:
    fld st(2)
hexgrid3_60:
    fmul qword ptr [rdx+112]
hexgrid3_63:
    faddp st(1), st(0)
hexgrid3_65:
    fld st(3)
hexgrid3_67:
    fmul qword ptr [rdx+120]
hexgrid3_6a:
    faddp st(1), st(0)
hexgrid3_6c:
    fld st(1)
hexgrid3_6e:
    fmul qword ptr [rdx+56]
hexgrid3_71:
    fld st(3)
hexgrid3_73:
    fmul qword ptr [rdx+64]
hexgrid3_76:
    faddp st(1), st(0)
hexgrid3_78:
    fld st(4)
hexgrid3_7a:
    fmul qword ptr [rdx+72]
hexgrid3_7d:
    faddp st(1), st(0)
hexgrid3_7f:
    fxch st(2)
hexgrid3_81:
    fmul qword ptr [rdx+80]
hexgrid3_84:
    fxch st(3)
hexgrid3_86:
    fmul qword ptr [rdx+88]
hexgrid3_89:
    faddp st(3), st(0)
hexgrid3_8b:
    fxch st(3)
hexgrid3_8d:
    fmul qword ptr [rdx+96]
hexgrid3_90:
    faddp st(2), st(0)
hexgrid3_92:
    fstp qword ptr [rsp]
hexgrid3_95:
    fstp qword ptr [rsp+8]
hexgrid3_99:
    fstp qword ptr [rsp+16]
hexgrid3_a0:
    fld qword ptr [rsp]
hexgrid3_a3:
    fsub qword ptr [rdx+40]
hexgrid3_a6:
    fabs
hexgrid3_a8:
    fstp qword ptr [rsp]
hexgrid3_ab:
    fld qword ptr [rsp]
hexgrid3_ae:
    fdiv qword ptr [rdx+40]
hexgrid3_b1:
    fmul qword ptr [rdx+152]
hexgrid3_b3:
    fsub qword ptr [rdx+152]
hexgrid3_b5:
    frndint
hexgrid3_b7:
    fadd st(0), st(0)
hexgrid3_b9:
    fld1
hexgrid3_bb:
    faddp st(1), st(0)
hexgrid3_bd:
    fmul qword ptr [rdx+40]
hexgrid3_c0:
    fchs
hexgrid3_c2:
    fadd qword ptr [rsp]
hexgrid3_c5:
    fabs
hexgrid3_c7:
    fstp qword ptr [rsp]
hexgrid3_ca:
    fld qword ptr [rdx+40]
hexgrid3_cd:
    fmul qword ptr [rdx+160]
hexgrid3_d0:
    fstp qword ptr [rsp+24]
hexgrid3_d4:
    fld qword ptr [rsp+8]
hexgrid3_d8:
    fsub qword ptr [rsp+24]
hexgrid3_dc:
    fabs
hexgrid3_de:
    fstp qword ptr [rsp+8]
hexgrid3_e2:
    fld qword ptr [rsp+8]
hexgrid3_e6:
    fdiv qword ptr [rsp+24]
hexgrid3_ea:
    fmul qword ptr [rdx+152]
hexgrid3_ec:
    fsub qword ptr [rdx+152]
hexgrid3_ee:
    frndint
hexgrid3_f0:
    fadd st(0), st(0)
hexgrid3_f2:
    fld1
hexgrid3_f4:
    faddp st(1), st(0)
hexgrid3_f6:
    fmul qword ptr [rsp+24]
hexgrid3_fa:
    fchs
hexgrid3_fc:
    fadd qword ptr [rsp+8]
hexgrid3_100:
    fabs
hexgrid3_102:
    fstp qword ptr [rsp+8]
hexgrid3_106:
    fld qword ptr [rsp]
hexgrid3_109:
    fmul qword ptr [rdx+168]
hexgrid3_10c:
    fld qword ptr [rsp+8]
hexgrid3_110:
    fmul qword ptr [rdx+176]
hexgrid3_113:
    faddp st(1), st(0)
hexgrid3_115:
    fadd qword ptr [rdx+184]
hexgrid3_118:
    fld st(0)
hexgrid3_11a:
    fabs
hexgrid3_11c:
    fchs
hexgrid3_11e:
    faddp st(1), st(0)
hexgrid3_120:
    fst qword ptr [rsp+24]
hexgrid3_124:
    fld st(0)
hexgrid3_126:
    fmul qword ptr [rdx+168]
hexgrid3_129:
    fchs
hexgrid3_12b:
    fadd qword ptr [rsp]
hexgrid3_12e:
    fabs
hexgrid3_130:
    fstp qword ptr [rsp]
hexgrid3_133:
    fmul qword ptr [rdx+176]
hexgrid3_136:
    fchs
hexgrid3_138:
    fadd qword ptr [rsp+8]
hexgrid3_13c:
    fabs
hexgrid3_13e:
    fstp qword ptr [rsp+8]
hexgrid3_142:
    fld qword ptr [rsp]
hexgrid3_145:
    fmul qword ptr [rdx+168]
hexgrid3_148:
    fld qword ptr [rsp+8]
hexgrid3_14c:
    fmul qword ptr [rdx+176]
hexgrid3_14f:
    faddp st(1), st(0)
hexgrid3_151:
    fadd qword ptr [rdx+184]
hexgrid3_154:
    fld st(0)
hexgrid3_156:
    fabs
hexgrid3_158:
    fchs
hexgrid3_15a:
    faddp st(1), st(0)
hexgrid3_15c:
    fst qword ptr [rsp+24]
hexgrid3_160:
    fld st(0)
hexgrid3_162:
    fmul qword ptr [rdx+168]
hexgrid3_165:
    fchs
hexgrid3_167:
    fadd qword ptr [rsp]
hexgrid3_16c:
    fstp qword ptr [rsp]
hexgrid3_16f:
    fmul qword ptr [rdx+176]
hexgrid3_172:
    fchs
hexgrid3_174:
    fadd qword ptr [rsp+8]
hexgrid3_178:
    fabs
hexgrid3_17a:
    fstp qword ptr [rsp+8]
hexgrid3_17e:
    fld qword ptr [rsp]
hexgrid3_181:
    fmul qword ptr [rdx+128]
hexgrid3_184:
    fld qword ptr [rsp+8]
hexgrid3_188:
    fmul qword ptr [rdx+136]
hexgrid3_18b:
    faddp st(1), st(0)
hexgrid3_18d:
    fadd qword ptr [rdx+144]
hexgrid3_190:
    fmul st(0), st(0)
hexgrid3_192:
    fld qword ptr [rsp+16]
hexgrid3_196:
    fmul st(0), st(0)
hexgrid3_198:
    faddp st(1), st(0)
hexgrid3_19a:
    fsqrt
hexgrid3_19c:
    fsub qword ptr [rdx+0]
    test r8d, r8d
    jne hexgrid3_store
    fdiv qword ptr [rdx+8]
hexgrid3_store:
    fstp qword ptr [rcx+24]
    fldcw word ptr [rsp+112]
    add rsp, 128
    ret
HexTubeGrid3_x87 ENDP


; hextgrid4IFS: original x87 body, mapped params, original local spills.
HexTubeGrid4_x87 PROC
    sub rsp, 128
    fnstcw word ptr [rsp+112]
    mov word ptr [rsp+114], 137Fh
    fldcw word ptr [rsp+114]
    mov rax, qword ptr [rcx]
    mov qword ptr [rsp], rax
    mov rax, qword ptr [rcx+8]
    mov qword ptr [rsp+8], rax
    mov rax, qword ptr [rcx+16]
    mov qword ptr [rsp+16], rax
hexgrid4_4b:
    fld qword ptr [rdx+152]
hexgrid4_4e:
    fmul qword ptr [rdx+40]
hexgrid4_51:
    fst qword ptr [rsp+68]
hexgrid4_55:
    fld st(0)
hexgrid4_57:
    fmul qword ptr [rdx+136]
hexgrid4_5a:
    fstp qword ptr [rsp+84]
hexgrid4_5e:
    fmul qword ptr [rdx+128]
hexgrid4_61:
    fstp qword ptr [rsp+76]
hexgrid4_65:
    fld qword ptr [rsp+16]
hexgrid4_69:
    fld qword ptr [rsp+8]
hexgrid4_6d:
    fld qword ptr [rsp]
hexgrid4_70:
    fld st(0)
hexgrid4_72:
    fmul qword ptr [rdx+104]
hexgrid4_75:
    fld st(2)
hexgrid4_77:
    fmul qword ptr [rdx+112]
hexgrid4_7a:
    faddp st(1), st(0)
hexgrid4_7c:
    fld st(3)
hexgrid4_7e:
    fmul qword ptr [rdx+120]
hexgrid4_81:
    faddp st(1), st(0)
hexgrid4_83:
    fld st(1)
hexgrid4_85:
    fmul qword ptr [rdx+56]
hexgrid4_88:
    fld st(3)
hexgrid4_8a:
    fmul qword ptr [rdx+64]
hexgrid4_8d:
    faddp st(1), st(0)
hexgrid4_8f:
    fld st(4)
hexgrid4_91:
    fmul qword ptr [rdx+72]
hexgrid4_94:
    faddp st(1), st(0)
hexgrid4_96:
    fxch st(2)
hexgrid4_98:
    fmul qword ptr [rdx+80]
hexgrid4_9b:
    fxch st(3)
hexgrid4_9d:
    fmul qword ptr [rdx+88]
hexgrid4_a0:
    faddp st(3), st(0)
hexgrid4_a2:
    fxch st(3)
hexgrid4_a4:
    fmul qword ptr [rdx+96]
hexgrid4_a7:
    faddp st(2), st(0)
hexgrid4_a9:
    fstp qword ptr [rsp]
hexgrid4_ac:
    fstp qword ptr [rsp+8]
hexgrid4_b0:
    fstp qword ptr [rsp+16]
hexgrid4_b4:
    fld qword ptr [rsp]
hexgrid4_b7:
    fabs
hexgrid4_b9:
    fst qword ptr [rsp+24]
hexgrid4_bd:
    fst qword ptr [rsp]
hexgrid4_c0:
    fistp dword ptr [rsp+44]
hexgrid4_c4:
    fld qword ptr [rsp+24]
hexgrid4_c8:
    fild dword ptr [rsp+44]
hexgrid4_cc:
    fsubp st(1), st(0)
hexgrid4_ce:
    fabs
hexgrid4_d0:
    fstp qword ptr [rsp+24]
hexgrid4_d4:
    fild dword ptr [rsp+44]
hexgrid4_d8:
    fmul qword ptr [rsp+76]
hexgrid4_dc:
    fsub qword ptr [rsp+8]
hexgrid4_e0:
    fabs
hexgrid4_e2:
    fst qword ptr [rsp+60]
hexgrid4_e6:
    fdiv qword ptr [rsp+68]
hexgrid4_ea:
    frndint
hexgrid4_ec:
    fmul qword ptr [rsp+68]
hexgrid4_f0:
    fchs
hexgrid4_f2:
    fadd qword ptr [rsp+60]
hexgrid4_f6:
    fabs
hexgrid4_f8:
    fadd qword ptr [rsp+84]
hexgrid4_fc:
    fld qword ptr [rsp+24]
hexgrid4_100:
    fcom st(1)
hexgrid4_102:
    wait
hexgrid4_103:
    fnstsw ax
hexgrid4_105:
    shr ah, 1
hexgrid4_107:
    jae hexgrid4_10d
hexgrid4_109:
    fstp st(0)
hexgrid4_10b:
    jmp hexgrid4_10f
hexgrid4_10d:
    fstp st(1)
hexgrid4_10f:
    fstp qword ptr [rsp+24]
hexgrid4_113:
    fld qword ptr [rsp]
hexgrid4_116:
    fmul qword ptr [rdx+144]
hexgrid4_118:
    fld qword ptr [rsp+8]
hexgrid4_11c:
    fmul qword ptr [rdx+160]
hexgrid4_11f:
    fsubp st(1), st(0)
hexgrid4_121:
    fabs
hexgrid4_123:
    fst qword ptr [rsp+32]
hexgrid4_127:
    fistp dword ptr [rsp+48]
hexgrid4_12b:
    fld qword ptr [rsp+32]
hexgrid4_12f:
    fild dword ptr [rsp+48]
hexgrid4_133:
    fsubp st(1), st(0)
hexgrid4_135:
    fabs
hexgrid4_137:
    fstp qword ptr [rsp+32]
hexgrid4_13b:
    fild dword ptr [rsp+48]
hexgrid4_13f:
    fmul qword ptr [rsp+76]
hexgrid4_143:
    fld qword ptr [rsp+8]
hexgrid4_147:
    fmul qword ptr [rdx+144]
hexgrid4_149:
    fld qword ptr [rsp]
hexgrid4_14c:
    fmul qword ptr [rdx+160]
hexgrid4_14f:
    faddp st(1), st(0)
hexgrid4_151:
    faddp st(1), st(0)
hexgrid4_153:
    fabs
hexgrid4_155:
    fst qword ptr [rsp+60]
hexgrid4_159:
    fdiv qword ptr [rsp+68]
hexgrid4_15d:
    frndint
hexgrid4_15f:
    fmul qword ptr [rsp+68]
hexgrid4_163:
    fchs
hexgrid4_165:
    fadd qword ptr [rsp+60]
hexgrid4_169:
    fabs
hexgrid4_16b:
    fadd qword ptr [rsp+84]
hexgrid4_16f:
    fld qword ptr [rsp+32]
hexgrid4_173:
    fcom st(1)
hexgrid4_175:
    wait
hexgrid4_176:
    fnstsw ax
hexgrid4_178:
    shr ah, 1
hexgrid4_17a:
    jae hexgrid4_180
hexgrid4_17c:
    fstp st(0)
hexgrid4_17e:
    jmp hexgrid4_182
hexgrid4_180:
    fstp st(1)
hexgrid4_182:
    fstp qword ptr [rsp+32]
hexgrid4_186:
    fld qword ptr [rsp]
hexgrid4_189:
    fmul qword ptr [rdx+144]
hexgrid4_18b:
    fld qword ptr [rsp+8]
hexgrid4_18f:
    fmul qword ptr [rdx+160]
hexgrid4_192:
    faddp st(1), st(0)
hexgrid4_194:
    fabs
hexgrid4_196:
    fst qword ptr [rsp+40]
hexgrid4_19a:
    fistp dword ptr [rsp+52]
hexgrid4_19e:
    fld qword ptr [rsp+40]
hexgrid4_1a2:
    fild dword ptr [rsp+52]
hexgrid4_1a6:
    fsubp st(1), st(0)
hexgrid4_1a8:
    fabs
hexgrid4_1aa:
    fstp qword ptr [rsp+40]
hexgrid4_1ae:
    fild dword ptr [rsp+52]
hexgrid4_1b2:
    fmul qword ptr [rsp+76]
hexgrid4_1b6:
    fld qword ptr [rsp+8]
hexgrid4_1ba:
    fmul qword ptr [rdx+144]
hexgrid4_1bc:
    fld qword ptr [rsp]
hexgrid4_1bf:
    fmul qword ptr [rdx+160]
hexgrid4_1c2:
    fchs
hexgrid4_1c4:
    faddp st(1), st(0)
hexgrid4_1c6:
    faddp st(1), st(0)
hexgrid4_1c8:
    fabs
hexgrid4_1ca:
    fst qword ptr [rsp+60]
hexgrid4_1ce:
    fdiv qword ptr [rsp+68]
hexgrid4_1d2:
    frndint
hexgrid4_1d4:
    fmul qword ptr [rsp+68]
hexgrid4_1d8:
    fchs
hexgrid4_1da:
    fadd qword ptr [rsp+60]
hexgrid4_1de:
    fabs
hexgrid4_1e0:
    fadd qword ptr [rsp+84]
hexgrid4_1e4:
    fld qword ptr [rsp+40]
hexgrid4_1e8:
    fcom st(1)
hexgrid4_1ea:
    wait
hexgrid4_1eb:
    fnstsw ax
hexgrid4_1ed:
    shr ah, 1
hexgrid4_1ef:
    jae hexgrid4_1f5
hexgrid4_1f1:
    fstp st(0)
hexgrid4_1f3:
    jmp hexgrid4_1f7
hexgrid4_1f5:
    fstp st(1)
hexgrid4_1f7:
    fstp qword ptr [rsp+40]
hexgrid4_1fb:
    fld qword ptr [rsp+24]
hexgrid4_1ff:
    fld qword ptr [rsp+32]
hexgrid4_203:
    fld qword ptr [rsp+40]
hexgrid4_207:
    fcom st(1)
hexgrid4_209:
    wait
hexgrid4_20a:
    fnstsw ax
hexgrid4_20c:
    shr ah, 1
hexgrid4_20e:
    jae hexgrid4_214
hexgrid4_210:
    fstp st(1)
hexgrid4_212:
    jmp hexgrid4_216
hexgrid4_214:
    fstp st(0)
hexgrid4_216:
    fcom st(1)
hexgrid4_218:
    wait
hexgrid4_219:
    fnstsw ax
hexgrid4_21b:
    shr ah, 1
hexgrid4_21d:
    jae hexgrid4_223
hexgrid4_21f:
    fstp st(1)
hexgrid4_221:
    jmp hexgrid4_225
hexgrid4_223:
    fstp st(0)
hexgrid4_225:
    fmul st(0), st(0)
hexgrid4_227:
    fld qword ptr [rsp+16]
hexgrid4_22b:
    fmul st(0), st(0)
hexgrid4_22d:
    faddp st(1), st(0)
hexgrid4_22f:
    fsqrt
hexgrid4_231:
    fsub qword ptr [rdx+0]
    test r8d, r8d
    jne hexgrid4_store
    fdiv qword ptr [rdx+8]
hexgrid4_store:
    fstp qword ptr [rcx+24]
    fldcw word ptr [rsp+112]
    add rsp, 128
    ret
HexTubeGrid4_x87 ENDP


; discoballIFS original 004B..00DE. Integer options are exactly expanded doubles.
; Preserve pi/2 - atan2(x,y), FSINCOS Double spills and the folded-X spill.
DiscoBall_x87 PROC
    sub rsp, 64
    fnstcw word ptr [rsp+48]
    mov word ptr [rsp+50], 137Fh
    fldcw word ptr [rsp+50]
    mov rax, qword ptr [rcx]
    mov qword ptr [rsp], rax
    mov rax, qword ptr [rcx+8]
    mov qword ptr [rsp+8], rax
    mov rax, qword ptr [rcx+16]
    mov qword ptr [rsp+16], rax
    fld qword ptr [rsp] ; 004B
    fld qword ptr [rsp+8] ; 004E
    fpatan ; 0052
    fsubr qword ptr [rdx+88] ; 0054
    fmul qword ptr [rdx+72] ; 0057
    fmul qword ptr [rdx+0] ; 0059
    frndint ; 005C
    fdiv qword ptr [rdx+0] ; 005E
    fmul qword ptr [rdx+80] ; 0061
    fsincos ; 0064
    fstp qword ptr [rsp+32] ; 0066
    fstp qword ptr [rsp+24] ; 006A
    fld qword ptr [rsp] ; 006E
    fmul qword ptr [rsp+24] ; 0071
    fld qword ptr [rsp+8] ; 0075
    fmul qword ptr [rsp+32] ; 0079
    fchs ; 007D
    faddp st(1), st(0) ; 007F
    fld qword ptr [rsp] ; 0081
    fmul qword ptr [rsp+32] ; 0084
    fld qword ptr [rsp+8] ; 0088
    fmul qword ptr [rsp+24] ; 008C
    faddp st(1), st(0) ; 0090
    fstp qword ptr [rsp] ; 0092
    fstp qword ptr [rsp+8] ; 0095
    fld qword ptr [rsp] ; 0099
    fld qword ptr [rsp+16] ; 009C
    fpatan ; 00A0
    fsubr qword ptr [rdx+88] ; 00A2
    fmul qword ptr [rdx+72] ; 00A5
    fmul qword ptr [rdx+8] ; 00A7
    frndint ; 00AA
    fdiv qword ptr [rdx+8] ; 00AC
    fmul qword ptr [rdx+80] ; 00AF
    fsincos ; 00B2
    fstp qword ptr [rsp+32] ; 00B4
    fstp qword ptr [rsp+24] ; 00B8
    fld qword ptr [rsp] ; 00BC
    fmul qword ptr [rsp+32] ; 00BF
    fld qword ptr [rsp+16] ; 00C3
    fmul qword ptr [rsp+24] ; 00C7
    faddp st(1), st(0) ; 00CB
    fsub qword ptr [rdx+56] ; 00CD
    fabs ; 00D0
    fsub qword ptr [rdx+64] ; 00D2
    test r8d, r8d
    jne disco_store
    fdiv qword ptr [rdx+16]
disco_store:
    fstp qword ptr [rcx+24]
    fldcw word ptr [rsp+48]
    add rsp, 64
    ret
DiscoBall_x87 ENDP


; SphCageIFS original 0069..0151, preserving Double loop spills and unordered min.
; rcx=transformed xyz/distance, rdx=expanded params, r8d=apply, r9d=clamped parallels.
SphCage_x87 PROC
    sub rsp, 96
    fnstcw word ptr [rsp+80]
    mov word ptr [rsp+82], 137Fh
    fldcw word ptr [rsp+82]
    mov rax, qword ptr [rcx]
    mov qword ptr [rsp], rax
    mov rax, qword ptr [rcx+8]
    mov qword ptr [rsp+8], rax
    mov rax, qword ptr [rcx+16]
    mov qword ptr [rsp+16], rax
    mov dword ptr [rsp+52], r9d
sphcage_069:
    fld qword ptr [rdx+0]
sphcage_06c:
    fld st(0)
sphcage_06e:
    fchs 
sphcage_070:
    fstp qword ptr [rsp + 32]
sphcage_074:
    fidiv dword ptr [rsp + 52]
sphcage_078:
    fmul qword ptr [rdx+96]
sphcage_07b:
    fstp qword ptr [rsp + 24]
sphcage_07f:
    shr r9d, 1
sphcage_081:
    fld qword ptr [rsp]
sphcage_084:
    fld qword ptr [rsp + 8]
sphcage_088:
    fpatan 
sphcage_08a:
    fsubr qword ptr [rdx+88]
sphcage_08d:
    fmul qword ptr [rdx+72]
sphcage_08f:
    fmul qword ptr [rdx+64]
sphcage_092:
    frndint 
sphcage_094:
    fdiv qword ptr [rdx+64]
sphcage_097:
    fmul qword ptr [rdx+80]
sphcage_09a:
    fsincos 
sphcage_09c:
    fstp qword ptr [rsp + 40]
sphcage_0a0:
    fstp qword ptr [rsp + 48]
sphcage_0a4:
    fld qword ptr [rsp]
sphcage_0a7:
    fmul qword ptr [rsp + 40]
sphcage_0ab:
    fld qword ptr [rsp + 8]
sphcage_0af:
    fmul qword ptr [rsp + 48]
sphcage_0b3:
    faddp st(1), st(0)
sphcage_0b5:
    fmul st(0), st(0)
sphcage_0b7:
    fld qword ptr [rsp + 16]
sphcage_0bb:
    fmul st(0), st(0)
sphcage_0bd:
    faddp st(1), st(0)
sphcage_0bf:
    fsqrt 
sphcage_0c1:
    fsub qword ptr [rdx+0]
sphcage_0c4:
    fmul st(0), st(0)
sphcage_0c6:
    fld qword ptr [rsp]
sphcage_0c9:
    fmul qword ptr [rsp + 48]
sphcage_0cd:
    fld qword ptr [rsp + 8]
sphcage_0d1:
    fmul qword ptr [rsp + 40]
sphcage_0d5:
    fchs 
sphcage_0d7:
    faddp st(1), st(0)
sphcage_0d9:
    fmul st(0), st(0)
sphcage_0db:
    faddp st(1), st(0)
sphcage_0dd:
    fsqrt 
sphcage_0df:
    fsub qword ptr [rdx+40]
sphcage_0e2:
    fld qword ptr [rsp + 16]
sphcage_0e6:
    fabs 
sphcage_0e8:
    fsub qword ptr [rdx+0]
sphcage_0eb:
    fstp qword ptr [rsp + 16]
sphcage_0ef:
    fstp qword ptr [rcx+24]
sphcage_0f2:
    dec r9d
sphcage_0f3:
    fld qword ptr [rsp + 24]
sphcage_0f7:
    fld st(0)
sphcage_0f9:
    fadd qword ptr [rsp + 16]
sphcage_0fd:
    fstp qword ptr [rsp + 16]
sphcage_101:
    fadd qword ptr [rsp + 32]
sphcage_105:
    fstp qword ptr [rsp + 32]
sphcage_109:
    fld qword ptr [rsp]
sphcage_10c:
    fmul st(0), st(0)
sphcage_10e:
    fld qword ptr [rsp + 8]
sphcage_112:
    fmul st(0), st(0)
sphcage_114:
    faddp st(1), st(0)
sphcage_116:
    fsqrt 
sphcage_118:
    fld qword ptr [rdx+0]
sphcage_11b:
    fmul st(0), st(0)
sphcage_11d:
    fld qword ptr [rsp + 32]
sphcage_121:
    fmul st(0), st(0)
sphcage_123:
    fchs 
sphcage_125:
    faddp st(1), st(0)
sphcage_127:
    fsqrt 
sphcage_129:
    fchs 
sphcage_12b:
    faddp st(1), st(0)
sphcage_12d:
    fmul st(0), st(0)
sphcage_12f:
    fld qword ptr [rsp + 16]
sphcage_133:
    fmul st(0), st(0)
sphcage_135:
    faddp st(1), st(0)
sphcage_137:
    fsqrt 
sphcage_139:
    fsub qword ptr [rdx+40]
sphcage_13c:
    fld qword ptr [rcx+24]
sphcage_13f:
    fcom st(1)
sphcage_141:
    wait 
sphcage_142:
    fnstsw ax
sphcage_144:
    shr ah, 1
sphcage_146:
    jae sphcage_14c
sphcage_148:
    fstp st(1)
sphcage_14a:
    jmp sphcage_14e
sphcage_14c:
    fstp st(0)
sphcage_14e:
    cmp r9d, 0
sphcage_151:
    jg sphcage_0ef
    test r8d, r8d
    jne sphcage_store
    fdiv qword ptr [rdx+8]
sphcage_store:
    fstp qword ptr [rcx+24]
    fldcw word ptr [rsp+80]
    add rsp, 96
    ret
SphCage_x87 ENDP


; Riemann2 original 0016..0166, with original Single matrix expanded exactly.
; rcx=xyz, rdx=expanded parameters. Includes FP rem/power singularities.
Riemann2_x87 PROC
    sub rsp, 64
    fnstcw word ptr [rsp+48]
    mov word ptr [rsp+50], 137Fh
    fldcw word ptr [rsp+50]
riemann2_016:
    fld qword ptr [rcx+16]
riemann2_018:
    fld qword ptr [rcx+8]
riemann2_01a:
    fld qword ptr [rcx]
riemann2_01c:
    fld st(0)
riemann2_01e:
    fmul qword ptr [rdx+48]
riemann2_021:
    fld st(2)
riemann2_023:
    fmul qword ptr [rdx+56]
riemann2_026:
    faddp st(1), st(0)
riemann2_028:
    fld st(3)
riemann2_02a:
    fmul qword ptr [rdx+64]
riemann2_02d:
    faddp st(1), st(0)
riemann2_02f:
    fld st(1)
riemann2_031:
    fmul qword ptr [rdx+0]
riemann2_034:
    fld st(3)
riemann2_036:
    fmul qword ptr [rdx+8]
riemann2_039:
    faddp st(1), st(0)
riemann2_03b:
    fld st(4)
riemann2_03d:
    fmul qword ptr [rdx+16]
riemann2_040:
    faddp st(1), st(0)
riemann2_042:
    fxch st(2)
riemann2_044:
    fmul qword ptr [rdx+24]
riemann2_047:
    fxch st(3)
riemann2_049:
    fmul qword ptr [rdx+32]
riemann2_04c:
    faddp st(3), st(0)
riemann2_04e:
    fxch st(3)
riemann2_050:
    fmul qword ptr [rdx+40]
riemann2_053:
    faddp st(2), st(0)
riemann2_055:
    fstp qword ptr [rcx]
riemann2_057:
    fstp qword ptr [rcx+8]
riemann2_059:
    fstp qword ptr [rcx+16]
riemann2_05b:
    fnop 
riemann2_05d:
    fld qword ptr [rcx]
riemann2_05f:
    fmul st(0), st(0)
riemann2_061:
    fld qword ptr [rcx+8]
riemann2_063:
    fmul st(0), st(0)
riemann2_065:
    fld qword ptr [rcx+16]
riemann2_067:
    fmul st(0), st(0)
riemann2_069:
    faddp st(1), st(0)
riemann2_06b:
    faddp st(1), st(0)
riemann2_06d:
    fsqrt 
riemann2_06f:
    fst qword ptr [rsp+0]
riemann2_072:
    fld qword ptr [rdx+72]
riemann2_075:
    fdivrp st(1), st(0)
riemann2_077:
    nop 
riemann2_078:
    fld qword ptr [rcx]
riemann2_07a:
    fmul st(0), st(1)
riemann2_07c:
    fstp qword ptr [rcx]
riemann2_07e:
    fld qword ptr [rcx+8]
riemann2_080:
    fmul st(0), st(1)
riemann2_082:
    fstp qword ptr [rcx+8]
riemann2_084:
    fld qword ptr [rcx+16]
riemann2_086:
    fmul st(0), st(1)
riemann2_088:
    fstp qword ptr [rcx+16]
riemann2_08a:
    fstp st(0)
riemann2_08c:
    nop 
riemann2_08d:
    nop 
riemann2_08e:
    nop 
riemann2_08f:
    nop 
riemann2_090:
    fld1 
riemann2_092:
    fsub qword ptr [rcx]
riemann2_094:
    fld st(0)
riemann2_096:
    fld qword ptr [rdx+128]
riemann2_099:
    fucompp 
riemann2_09b:
    fnstsw ax
riemann2_09d:
    shr ah, 1
riemann2_09f:
    jb riemann2_0aa
riemann2_0a1:
    fstp st(0)
riemann2_0a3:
    fld1 
riemann2_0a5:
    nop 
riemann2_0a6:
    nop 
riemann2_0a7:
    nop 
riemann2_0a8:
    jmp riemann2_0af
riemann2_0aa:
    fld1 
riemann2_0ac:
    fdivrp st(1), st(0)
riemann2_0ae:
    nop 
riemann2_0af:
    fld qword ptr [rcx+16]
riemann2_0b1:
    fmul st(0), st(1)
riemann2_0b3:
    fstp qword ptr [rsp+8]
riemann2_0b6:
    fld qword ptr [rcx+8]
riemann2_0b8:
    fmul st(0), st(1)
riemann2_0ba:
    fstp qword ptr [rsp+16]
riemann2_0bd:
    fstp st(0)
riemann2_0bf:
    fld qword ptr [rsp+8]
riemann2_0c2:
    fmul st(0), st(0)
riemann2_0c4:
    fld qword ptr [rsp+16]
riemann2_0c7:
    fmul st(0), st(0)
riemann2_0c9:
    faddp st(1), st(0)
riemann2_0cb:
    fld1 
riemann2_0cd:
    faddp st(1), st(0)
riemann2_0cf:
    fstp qword ptr [rsp+24]
riemann2_0d2:
    nop 
riemann2_0d3:
    fld qword ptr [rsp+8]
riemann2_0d6:
    fmul qword ptr [rdx+136]
riemann2_0d8:
    fadd qword ptr [rdx+96]
riemann2_0db:
    fsin 
riemann2_0dd:
    fabs 
riemann2_0df:
    fld qword ptr [rdx+120]
riemann2_0e2:
    faddp st(1), st(0)
riemann2_0e4:
    fabs 
riemann2_0e6:
    fstp qword ptr [rsp+8]
riemann2_0e9:
    fld qword ptr [rsp+16]
riemann2_0ec:
    fmul qword ptr [rdx+136]
riemann2_0ee:
    fadd qword ptr [rdx+104]
riemann2_0f1:
    fsin 
riemann2_0f3:
    fabs 
riemann2_0f5:
    fld qword ptr [rdx+120]
riemann2_0f8:
    faddp st(1), st(0)
riemann2_0fa:
    fabs 
riemann2_0fc:
    fstp qword ptr [rsp+16]
riemann2_0ff:
    nop 
riemann2_100:
    fld qword ptr [rsp+0]
riemann2_103:
    fld qword ptr [rsp+24]
riemann2_106:
    fmul qword ptr [rdx+80]
riemann2_109:
    fxch st(1)
riemann2_10b:
    fabs 
riemann2_10d:
    fyl2x 
riemann2_10f:
    fld1 
riemann2_111:
    fld st(1)
riemann2_113:
    fprem 
riemann2_115:
    f2xm1 
riemann2_117:
    faddp st(1), st(0)
riemann2_119:
    fscale 
riemann2_11b:
    fxch st(1)
riemann2_11d:
    fstp st(0)
riemann2_11f:
    fadd qword ptr [rdx+88]
riemann2_122:
    fstp qword ptr [rsp+0]
riemann2_125:
    nop 
riemann2_126:
    fld qword ptr [rdx+144]
riemann2_129:
    fld qword ptr [rsp+8]
riemann2_12c:
    fmul st(0), st(0)
riemann2_12e:
    fld qword ptr [rsp+16]
riemann2_131:
    fmul st(0), st(0)
riemann2_133:
    faddp st(1), st(0)
riemann2_135:
    fld1 
riemann2_137:
    faddp st(1), st(0)
riemann2_139:
    fdivp st(1), st(0)
riemann2_13b:
    fst qword ptr [rsp+24]
riemann2_13e:
    nop 
riemann2_13f:
    fchs 
riemann2_141:
    fld1 
riemann2_143:
    faddp st(1), st(0)
riemann2_145:
    fmul qword ptr [rsp+0]
riemann2_148:
    nop 
riemann2_149:
    fld qword ptr [rsp+0]
riemann2_14c:
    fmul qword ptr [rsp+16]
riemann2_14f:
    fmul qword ptr [rsp+24]
riemann2_152:
    nop 
riemann2_153:
    fld qword ptr [rsp+0]
riemann2_156:
    fmul qword ptr [rsp+8]
riemann2_159:
    fmul qword ptr [rsp+24]
riemann2_15c:
    nop 
riemann2_15d:
    fstp qword ptr [rcx+16]
riemann2_15f:
    fstp qword ptr [rcx+8]
riemann2_161:
    fadd qword ptr [rdx+112]
riemann2_164:
    fstp qword ptr [rcx]
riemann2_166:
    fnop 
    fldcw word ptr [rsp+48]
    add rsp, 64
    ret
Riemann2_x87 ENDP


; _hyc3Dinv 000A..008D. FDIVRP ST(3) intentionally leaves quotient in ST(2):
; final radius/center instructions affect the old Y result, not that quotient.
HycInv_x87 PROC
    sub rsp, 16
    fnstcw word ptr [rsp]
    mov word ptr [rsp+2], 137Fh
    fldcw word ptr [rsp+2]
    fld qword ptr [rcx+16] ; 000A
    fld qword ptr [rcx+8] ; 000C
    fld qword ptr [rcx] ; 000E
    fadd qword ptr [rdx+0] ; 0010
    fstp qword ptr [rcx] ; 0013
    fadd qword ptr [rdx+8] ; 0015
    fstp qword ptr [rcx+8] ; 0018
    fadd qword ptr [rdx+16] ; 001A
    fstp qword ptr [rcx+16] ; 001D
    fld qword ptr [rcx] ; 001F
    fmul st(0), st(0) ; 0021
    fld qword ptr [rcx] ; 0023
    fmulp st(1), st(0) ; 0025
    fld qword ptr [rcx+8] ; 0027
    fmul st(0), st(0) ; 0029
    fld qword ptr [rcx+8] ; 002B
    fmulp st(1), st(0) ; 002D
    faddp st(1), st(0) ; 002F
    fld qword ptr [rcx+16] ; 0031
    fmul st(0), st(0) ; 0033
    fld qword ptr [rcx+16] ; 0035
    fmulp st(1), st(0) ; 0037
    faddp st(1), st(0) ; 0039
    fld qword ptr [rcx] ; 003B
    fmul qword ptr [rcx+16] ; 003D
    fmul qword ptr [rcx+8] ; 003F
    fxch st(1) ; 0041
    fsub st(0), st(1) ; 0043
    fsub st(0), st(1) ; 0045
    fxch st(1) ; 0047
    fsubrp st(1), st(0) ; 0049
    fchs  ; 004B
    fld qword ptr [rcx] ; 004D
    fmul st(0), st(0) ; 004F
    fld qword ptr [rcx+8] ; 0051
    fld qword ptr [rcx+16] ; 0053
    fmulp st(1), st(0) ; 0055
    fsubp st(1), st(0) ; 0057
    fdiv st(0), st(1) ; 0059
    fmul qword ptr [rdx+24] ; 005B
    fsub qword ptr [rdx+0] ; 005E
    fld qword ptr [rcx+8] ; 0061
    fmul st(0), st(0) ; 0063
    fld qword ptr [rcx] ; 0065
    fld qword ptr [rcx+16] ; 0067
    fmulp st(1), st(0) ; 0069
    fsubp st(1), st(0) ; 006B
    fdiv st(0), st(2) ; 006D
    fmul qword ptr [rdx+24] ; 006F
    fsub qword ptr [rdx+8] ; 0072
    fld qword ptr [rcx+16] ; 0075
    fmul st(0), st(0) ; 0077
    fld qword ptr [rcx] ; 0079
    fld qword ptr [rcx+8] ; 007B
    fmulp st(1), st(0) ; 007D
    fsubp st(1), st(0) ; 007F
    fdivrp st(3), st(0) ; 0081
    fmul qword ptr [rdx+24] ; 0083
    fsub qword ptr [rdx+16] ; 0086
    fstp qword ptr [rcx+16] ; 0089
    fstp qword ptr [rcx+8] ; 008B
    fstp qword ptr [rcx] ; 008D
    fldcw word ptr [rsp]
    add rsp, 16
    ret
HycInv_x87 ENDP


; tilingbrickIFS: literal x87 operations; expanded Single matrix values are exact Doubles.
TilingBrick_x87 PROC
    sub rsp, 16
    fnstcw word ptr [rsp+8]
    mov word ptr [rsp+10], 137Fh
    fldcw word ptr [rsp+10]
    fld qword ptr [rcx+16] ; 0000
    fadd qword ptr [rdx+16] ; 0003
    fld qword ptr [rcx+8] ; 0006
    fadd qword ptr [rdx+8] ; 0009
    fld qword ptr [rcx+0] ; 000C
    fadd qword ptr [rdx+0] ; 000F
    fld st(0) ; 0012
    fmul qword ptr [rdx+72] ; 0014
    fld st(2) ; 0017
    fmul qword ptr [rdx+80] ; 0019
    faddp st(1), st(0) ; 001C
    fld st(3) ; 001E
    fmul qword ptr [rdx+88] ; 0020
    faddp st(1), st(0) ; 0023
    fld st(1) ; 0025
    fmul qword ptr [rdx+24] ; 0027
    fld st(3) ; 002A
    fmul qword ptr [rdx+32] ; 002C
    faddp st(1), st(0) ; 002F
    fld st(4) ; 0031
    fmul qword ptr [rdx+40] ; 0033
    faddp st(1), st(0) ; 0036
    fxch st(2) ; 0038
    fmul qword ptr [rdx+48] ; 003A
    fxch st(3) ; 003D
    fmul qword ptr [rdx+56] ; 003F
    faddp st(3), st(0) ; 0042
    fxch st(3) ; 0044
    fmul qword ptr [rdx+64] ; 0046
    faddp st(2), st(0) ; 0049
    fstp qword ptr [rcx+0] ; 004B
    fstp qword ptr [rcx+8] ; 004E
    fstp qword ptr [rcx+16] ; 0051
    fld qword ptr [rdx+104] ; 0055
    fld qword ptr [rcx+8] ; 0058
    fld st(0) ; 005C
    fdiv st(0), st(2) ; 005E
    fistp dword ptr [rsp] ; 0063
    fild dword ptr [rsp] ; 0066
    mov r8d, dword ptr [rsp] ; 0069
    fmul st(0), st(2) ; 006C
    fsubp st(1), st(0) ; 006F
    fstp st(1) ; 0071
    fld qword ptr [rdx+96] ; 0073
    fld qword ptr [rcx+0] ; 0076
    mov r8d, dword ptr [rsp] ; 0079
    and r8d, 1 ; 007C
    jne TilingBrick_x87_0084 ; 007F
    fadd qword ptr [rdx+112] ; 0081
TilingBrick_x87_0084:
    fld st(0) ; 0084
    fdiv st(0), st(2) ; 0086
    frndint  ; 0089
    fmul st(0), st(2) ; 008E
    fsubp st(1), st(0) ; 0091
    fstp st(1) ; 0093
    fstp qword ptr [rcx+0] ; 0095
    fstp qword ptr [rcx+8] ; 0098
    fldcw word ptr [rsp+8]
    add rsp, 16
    ret
TilingBrick_x87 ENDP

; tilingbrick2IFS: literal x87 operations; expanded Single matrix values are exact Doubles.
TilingBrick2_x87 PROC
    sub rsp, 16
    fnstcw word ptr [rsp+8]
    mov word ptr [rsp+10], 137Fh
    fldcw word ptr [rsp+10]
    fld qword ptr [rcx+16] ; 0000
    fadd qword ptr [rdx+16] ; 0003
    fld qword ptr [rcx+8] ; 0006
    fadd qword ptr [rdx+8] ; 0009
    fld qword ptr [rcx+0] ; 000C
    fadd qword ptr [rdx+0] ; 000F
    fld st(0) ; 0012
    fmul qword ptr [rdx+72] ; 0014
    fld st(2) ; 0017
    fmul qword ptr [rdx+80] ; 0019
    faddp st(1), st(0) ; 001C
    fld st(3) ; 001E
    fmul qword ptr [rdx+88] ; 0020
    faddp st(1), st(0) ; 0023
    fld st(1) ; 0025
    fmul qword ptr [rdx+24] ; 0027
    fld st(3) ; 002A
    fmul qword ptr [rdx+32] ; 002C
    faddp st(1), st(0) ; 002F
    fld st(4) ; 0031
    fmul qword ptr [rdx+40] ; 0033
    faddp st(1), st(0) ; 0036
    fxch st(2) ; 0038
    fmul qword ptr [rdx+48] ; 003A
    fxch st(3) ; 003D
    fmul qword ptr [rdx+56] ; 003F
    faddp st(3), st(0) ; 0042
    fxch st(3) ; 0044
    fmul qword ptr [rdx+64] ; 0046
    faddp st(2), st(0) ; 0049
    fstp qword ptr [rcx+0] ; 004B
    fstp qword ptr [rcx+8] ; 004E
    fstp qword ptr [rcx+16] ; 0051
    fld qword ptr [rdx+104] ; 0055
    fld qword ptr [rcx+8] ; 0058
    fld st(0) ; 005C
    fdiv st(0), st(2) ; 005E
    fistp dword ptr [rsp] ; 0063
    fild dword ptr [rsp] ; 0066
    fmul st(0), st(2) ; 0069
    fsubp st(1), st(0) ; 006B
    fstp st(1) ; 006D
    fld qword ptr [rdx+96] ; 006F
    fld qword ptr [rdx+112] ; 0072
    fimul dword ptr [rsp] ; 0075
    fadd qword ptr [rcx+0] ; 0078
    fld st(0) ; 007B
    fdiv st(0), st(2) ; 007D
    frndint  ; 0080
    fmul st(0), st(2) ; 0085
    fsubp st(1), st(0) ; 0088
    fstp st(1) ; 008A
    fstp qword ptr [rcx+0] ; 008C
    fstp qword ptr [rcx+8] ; 008F
    fldcw word ptr [rsp+8]
    add rsp, 16
    ret
TilingBrick2_x87 ENDP

; Ruckerbulb.m3f 0016..0110, literal x87 order and Double store points.
; rcx = XYZ, rdx = expanded parameters including Julia C.
Ruckerbulb_x87 PROC
    sub rsp, 72
    fnstcw word ptr [rsp+64]
    mov word ptr [rsp+66], 137Fh
    fldcw word ptr [rsp+66]
    fld qword ptr [rcx] ; 0016
    fmul st(0), st(0) ; 0018
    fld qword ptr [rcx+8] ; 001A
    fmul st(0), st(0) ; 001C
    fld qword ptr [rcx+16] ; 001E
    fmul st(0), st(0) ; 0020
    faddp st(1), st(0) ; 0022
    faddp st(1), st(0) ; 0024
    fsqrt  ; 0026
    fst qword ptr [rsp+0] ; 0028
    fld qword ptr [rdx] ; 002B
    fxch st(1) ; 002E
    fabs  ; 0030
    fyl2x  ; 0032
    fld1  ; 0034
    fld st(1) ; 0036
    fprem  ; 0038
    f2xm1  ; 003A
    faddp st(1), st(0) ; 003C
    fscale  ; 003E
    fxch st(1) ; 0040
    fstp st(0) ; 0042
    fstp qword ptr [rsp+56] ; 0044
    fnop  ; 0047
    cmp qword ptr [rdx+24], 0 ; 0049
    jne Ruckerbulb_0057 ; 004D
    fld qword ptr [rsp+56] ; 004F
    fstp qword ptr [rsp+0] ; 0052
    jmp Ruckerbulb_0076 ; 0055
Ruckerbulb_0057:
    fld qword ptr [rsp+0] ; 0057
    fld qword ptr [rdx+8] ; 005A
    fxch st(1) ; 005D
    fabs  ; 005F
    fyl2x  ; 0061
    fld1  ; 0063
    fld st(1) ; 0065
    fprem  ; 0067
    f2xm1  ; 0069
    faddp st(1), st(0) ; 006B
    fscale  ; 006D
    fxch st(1) ; 006F
    fstp st(0) ; 0071
    fstp qword ptr [rsp+0] ; 0073
Ruckerbulb_0076:
    fld qword ptr [rcx] ; 0076
    fchs  ; 0078
    fld qword ptr [rcx+16] ; 007A
    fpatan  ; 007C
    cmp qword ptr [rdx+32], 0 ; 007E
    jne Ruckerbulb_00ae ; 0082
    fld qword ptr [rcx] ; 0084
    nop  ; 0086
    fldz  ; 0087
    fcompp  ; 0089
    fnstsw ax ; 008B
    and ah, 41h ; 008D
    jg Ruckerbulb_00ae ; 0090
    fld qword ptr [rcx+16] ; 0092
    nop  ; 0094
    fldz  ; 0095
    fcompp  ; 0097
    fnstsw ax ; 0099
    and ah, 41h ; 009B
    jl Ruckerbulb_00a8 ; 009E
    fldpi  ; 00A0
    fmul qword ptr [rdx+40] ; 00A2
    fsubp st(1), st(0) ; 00A4
    jmp Ruckerbulb_00ae ; 00A6
Ruckerbulb_00a8:
    fldpi  ; 00A8
    fmul qword ptr [rdx+40] ; 00AA
    faddp st(1), st(0) ; 00AC
Ruckerbulb_00ae:
    fmul qword ptr [rdx+8] ; 00AE
    fsincos  ; 00B1
    fstp qword ptr [rsp+48] ; 00B3
    fstp qword ptr [rsp+40] ; 00B6
    fld qword ptr [rcx+8] ; 00B9
    fld qword ptr [rcx] ; 00BB
    fpatan  ; 00BD
    fmul qword ptr [rdx] ; 00BF
    fsincos  ; 00C2
    fstp qword ptr [rsp+32] ; 00C4
    fstp qword ptr [rsp+24] ; 00C7
    fnop  ; 00CA
    fld qword ptr [rsp+56] ; 00CC
    fmul qword ptr [rsp+48] ; 00CF
    fmul qword ptr [rsp+32] ; 00D2
    fchs  ; 00D5
    fadd qword ptr [rdx+48] ; 00D7
    fstp qword ptr [rsp+16] ; 00DA
    fnop  ; 00DD
    fld qword ptr [rsp+56] ; 00DF
    fmul qword ptr [rsp+48] ; 00E2
    fmul qword ptr [rsp+24] ; 00E5
    fchs  ; 00E8
    fadd qword ptr [rdx+56] ; 00EA
    fstp qword ptr [rsp+8] ; 00ED
    fnop  ; 00F0
    fld qword ptr [rsp+0] ; 00F2
    fmul qword ptr [rsp+40] ; 00F5
    fmul qword ptr [rdx+16] ; 00F8
    fadd qword ptr [rdx+64] ; 00FB
    fstp qword ptr [rsp+0] ; 00FE
    fnop  ; 0101
    fld qword ptr [rsp+16] ; 0103
    fld qword ptr [rsp+8] ; 0106
    fld qword ptr [rsp+0] ; 0109
    fstp qword ptr [rcx+16] ; 010C
    fstp qword ptr [rcx+8] ; 010E
    fstp qword ptr [rcx] ; 0110
    fldcw word ptr [rsp+64]
    add rsp, 72
    ret
Ruckerbulb_x87 ENDP

; Original boxIFS 0051..0074; rcx=transformed XYZ, rdx=expanded Single Twist.
.code
BoxIFSTwist_x87 PROC
    cmp qword ptr [rdx], 0
    je boxifs_twist_done
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp+2], 137Fh
    fldcw word ptr [rsp+2]
    fld qword ptr [rdx]
    fmul qword ptr [rcx+16]
    fsincos
    fld qword ptr [rcx+8]
    fld qword ptr [rcx]
    fld st(3)
    fmul st(0), st(1)
    fld st(2)
    fmul st(0), st(4)
    faddp st(1), st(0)
    fstp qword ptr [rcx]
    fmulp st(2), st(0)
    fmulp st(2), st(0)
    fsubrp st(1), st(0)
    fstp qword ptr [rcx+8]
    fldcw word ptr [rsp]
    add rsp, 8
boxifs_twist_done:
    ret
BoxIFSTwist_x87 ENDP

; Original MengerIFS 000D..00A8; rcx=XYZW, rdx=13 expanded parameters.
; Singles in the original matrix are exactly represented in Double slots.
.code
MengerIFSOriginal_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp+2], 137Fh
    fldcw word ptr [rsp+2]
    fld qword ptr [rcx + 16] ; 000D
    fabs  ; 000F
    fld qword ptr [rcx + 8] ; 0011
    fabs  ; 0013
    fld qword ptr [rcx] ; 0015
    fabs  ; 0017
    fcom st(1) ; 0019
    fnstsw ax ; 001B
    shr ah, 1 ; 001D
    jae menger_original_0023 ; 001F
    fxch st(1) ; 0021
menger_original_0023:
    fcom st(2) ; 0023
    fnstsw ax ; 0025
    shr ah, 1 ; 0027
    jae menger_original_002d ; 0029
    fxch st(2) ; 002B
menger_original_002d:
    fxch st(1) ; 002D
    fcom st(2) ; 002F
    fnstsw ax ; 0031
    shr ah, 1 ; 0033
    jae menger_original_0039 ; 0035
    fxch st(2) ; 0037
menger_original_0039:
    fld st(1) ; 0039
    fmul qword ptr [rdx + 80] ; 003B
    fld st(1) ; 003E
    fmul qword ptr [rdx + 88] ; 0040
    faddp st(1), st(0) ; 0043
    fld st(3) ; 0045
    fmul qword ptr [rdx + 96] ; 0047
    faddp st(1), st(0) ; 004A
    fld st(2) ; 004C
    fmul qword ptr [rdx + 32] ; 004E
    fld st(2) ; 0051
    fmul qword ptr [rdx + 40] ; 0053
    faddp st(1), st(0) ; 0056
    fld st(4) ; 0058
    fmul qword ptr [rdx + 48] ; 005A
    faddp st(1), st(0) ; 005D
    fxch st(3) ; 005F
    fmul qword ptr [rdx + 56] ; 0061
    fxch st(2) ; 0064
    fmul qword ptr [rdx + 64] ; 0066
    faddp st(2), st(0) ; 0069
    fxch st(3) ; 006B
    fmul qword ptr [rdx + 72] ; 006D
    faddp st(1), st(0) ; 0070
    fld qword ptr [rdx + 0] ; 0072
    fld qword ptr [rcx + 24] ; 0075
    fmul st(0), st(1) ; 0078
    fstp qword ptr [rcx + 24] ; 007A
    fmul st(2), st(0) ; 007D
    fmul st(1), st(0) ; 007F
    fld st(0) ; 0081
    fld1  ; 0083
    fsubp st(1), st(0) ; 0085
    fld st(0) ; 0087
    fld st(0) ; 0089
    fmul qword ptr [rdx + 8] ; 008B
    fsubp st(5), st(0) ; 008E
    fmul qword ptr [rdx + 16] ; 0090
    fsubp st(3), st(0) ; 0093
    fdiv st(0), st(1) ; 0095
    fmul qword ptr [rdx + 24] ; 0097
    fsub st(4), st(0) ; 009A
    fxch st(4) ; 009C
    fabs  ; 009E
    fsubp st(4), st(0) ; 00A0
    fmulp st(3), st(0) ; 00A2
    fstp qword ptr [rcx + 8] ; 00A4
    fstp qword ptr [rcx] ; 00A6
    fstp qword ptr [rcx + 16] ; 00A8
    fldcw word ptr [rsp]
    add rsp, 8
    ret
MengerIFSOriginal_x87 ENDP

; Mandalex original arithmetic; rcx=XYZW, rdx=17 expanded params + Julia XYZ.
; Inline both original local calls; the only local is a 32-bit pull counter.
.code
MandalexOriginal_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp+2], 137Fh
    fldcw word ptr [rsp+2]
    fld qword ptr [rdx+104]
    mov eax, 0 ; 002A
    mov dword ptr [rsp + 4], eax ; 002F
    fld qword ptr [rcx] ; 0032
    fabs  ; 0034
    fcomp st(1) ; 0036
    fnstsw ax ; 0038
    shr ah, 1 ; 003A
    jae mandalex_pre_0041 ; 003C
    inc dword ptr [rsp + 4] ; 003E
mandalex_pre_0041:
    fld qword ptr [rcx + 8] ; 0041
    fabs  ; 0043
    fcomp st(1) ; 0045
    fnstsw ax ; 0047
    shr ah, 1 ; 0049
    jae mandalex_pre_0050 ; 004B
    inc dword ptr [rsp + 4] ; 004D
mandalex_pre_0050:
    fld qword ptr [rcx + 16] ; 0050
    fabs  ; 0052
    fcomp st(1) ; 0054
    fnstsw ax ; 0056
    shr ah, 1 ; 0058
    jae mandalex_pre_005f ; 005A
    inc dword ptr [rsp + 4] ; 005C
mandalex_pre_005f:
    cvttsd2si eax, qword ptr [rdx + 128] ; 005F
    and eax, 3 ; 0062
    xor eax, dword ptr [rsp + 4] ; 0065
    jne mandalex_pre_00bf ; 0068
    fmul qword ptr [rdx + 120] ; 006E
    fld qword ptr [rcx] ; 0071
    fldz  ; 0073
    fcomp st(1) ; 0075
    fnstsw ax ; 0077
    shr ah, 1 ; 0079
    jae mandalex_pre_0081 ; 007B
    fld1  ; 007D
    jmp mandalex_pre_0085 ; 007F
mandalex_pre_0081:
    fld1  ; 0081
    fchs  ; 0083
mandalex_pre_0085:
    fmul st(0), st(2) ; 0085
    fsubp st(1), st(0) ; 0087
    fstp qword ptr [rcx] ; 0089
    fld qword ptr [rcx + 8] ; 008B
    fldz  ; 008D
    fcomp st(1) ; 008F
    fnstsw ax ; 0091
    shr ah, 1 ; 0093
    jae mandalex_pre_009b ; 0095
    fld1  ; 0097
    jmp mandalex_pre_009f ; 0099
mandalex_pre_009b:
    fld1  ; 009B
    fchs  ; 009D
mandalex_pre_009f:
    fmul st(0), st(2) ; 009F
    fsubp st(1), st(0) ; 00A1
    fstp qword ptr [rcx + 8] ; 00A3
    fld qword ptr [rcx + 16] ; 00A5
    fldz  ; 00A7
    fcomp st(1) ; 00A9
    fnstsw ax ; 00AB
    shr ah, 1 ; 00AD
    jae mandalex_pre_00b5 ; 00AF
    fld1  ; 00B1
    jmp mandalex_pre_00b9 ; 00B3
mandalex_pre_00b5:
    fld1  ; 00B5
    fchs  ; 00B7
mandalex_pre_00b9:
    fmul st(0), st(2) ; 00B9
    fsubp st(1), st(0) ; 00BB
    fstp qword ptr [rcx + 16] ; 00BD
mandalex_pre_00bf:
    fstp st(0) ; 00BF
    fld qword ptr [rcx] ; 00C5
    fld qword ptr [rcx + 8] ; 00C7
    fld qword ptr [rcx + 16] ; 00C9
    nop  ; 00CB
    fld st(0) ; 00CC
    fmul qword ptr [rdx + 80] ; 00CE
    fld st(2) ; 00D1
    fmul qword ptr [rdx + 88] ; 00D3
    faddp st(1), st(0) ; 00D6
    fld st(3) ; 00D8
    fmul qword ptr [rdx + 96] ; 00DA
    faddp st(1), st(0) ; 00DD
    fld st(1) ; 00DF
    fmul qword ptr [rdx + 32] ; 00E1
    fld st(3) ; 00E4
    fmul qword ptr [rdx + 40] ; 00E6
    faddp st(1), st(0) ; 00E9
    fld st(4) ; 00EB
    fmul qword ptr [rdx + 48] ; 00ED
    faddp st(1), st(0) ; 00F0
    fxch st(2) ; 00F2
    fmul qword ptr [rdx + 56] ; 00F4
    fxch st(3) ; 00F7
    fmul qword ptr [rdx + 64] ; 00F9
    faddp st(3), st(0) ; 00FC
    fxch st(3) ; 00FE
    fmul qword ptr [rdx + 72] ; 0100
    faddp st(2), st(0) ; 0103
    fstp qword ptr [rcx + 16] ; 0105
    fstp qword ptr [rcx + 8] ; 0107
    fstp qword ptr [rcx] ; 0109
    nop  ; 010B
    fld qword ptr [rdx + 24] ; 010C
    fld st(0) ; 010F
    fld st(0) ; 0111
    fadd st(2), st(0) ; 0113
    fchs  ; 0115
    fld qword ptr [rcx] ; 0117
    fcom st(2) ; 0119
    fnstsw ax ; 011B
    shr ah, 1 ; 011D
    jb mandalex_main_0127 ; 011F
    fchs  ; 0121
    fadd st(0), st(3) ; 0123
    jmp mandalex_main_0133 ; 0125
mandalex_main_0127:
    fcom st(1) ; 0127
    fnstsw ax ; 0129
    shr ah, 1 ; 012B
    jae mandalex_main_0133 ; 012D
    fchs  ; 012F
    fsub st(0), st(3) ; 0131
mandalex_main_0133:
    fld qword ptr [rcx + 8] ; 0133
    fcom st(3) ; 0135
    fnstsw ax ; 0137
    shr ah, 1 ; 0139
    jb mandalex_main_0143 ; 013B
    fchs  ; 013D
    fadd st(0), st(4) ; 013F
    jmp mandalex_main_014f ; 0141
mandalex_main_0143:
    fcom st(2) ; 0143
    fnstsw ax ; 0145
    shr ah, 1 ; 0147
    jae mandalex_main_014f ; 0149
    fchs  ; 014B
    fsub st(0), st(4) ; 014D
mandalex_main_014f:
    fld qword ptr [rcx + 16] ; 014F
    fcom st(4) ; 0151
    fnstsw ax ; 0153
    shr ah, 1 ; 0155
    jb mandalex_main_015f ; 0157
    fchs  ; 0159
    fadd st(0), st(5) ; 015B
    jmp mandalex_main_016b ; 015D
mandalex_main_015f:
    fcom st(3) ; 015F
    fnstsw ax ; 0161
    shr ah, 1 ; 0163
    jae mandalex_main_016b ; 0165
    fchs  ; 0167
    fsub st(0), st(5) ; 0169
mandalex_main_016b:
    fxch st(3) ; 016B
    fstp st(0) ; 016D
    fxch st(3) ; 016F
    fstp st(0) ; 0171
    fxch st(3) ; 0173
    fstp st(0) ; 0175
    fld st(0) ; 0177
    fmul qword ptr [rdx + 80] ; 0179
    fld st(2) ; 017C
    fmul qword ptr [rdx + 88] ; 017E
    faddp st(1), st(0) ; 0181
    fld st(3) ; 0183
    fmul qword ptr [rdx + 96] ; 0185
    faddp st(1), st(0) ; 0188
    fld st(1) ; 018A
    fmul qword ptr [rdx + 32] ; 018C
    fld st(3) ; 018F
    fmul qword ptr [rdx + 40] ; 0191
    faddp st(1), st(0) ; 0194
    fld st(4) ; 0196
    fmul qword ptr [rdx + 48] ; 0198
    faddp st(1), st(0) ; 019B
    fxch st(2) ; 019D
    fmul qword ptr [rdx + 56] ; 019F
    fxch st(3) ; 01A2
    fmul qword ptr [rdx + 64] ; 01A4
    faddp st(3), st(0) ; 01A7
    fxch st(3) ; 01A9
    fmul qword ptr [rdx + 72] ; 01AB
    faddp st(2), st(0) ; 01AE
    nop  ; 01B0
    fld st(0) ; 01B1
    fmul st(0), st(1) ; 01B3
    fld st(2) ; 01B5
    fmul st(0), st(3) ; 01B7
    faddp st(1), st(0) ; 01B9
    fld st(3) ; 01BB
    fmul st(0), st(4) ; 01BD
    faddp st(1), st(0) ; 01BF
    fcom qword ptr [rdx + 16] ; 01C1
    fnstsw ax ; 01C4
    shr ah, 1 ; 01C6
    jae mandalex_main_01d1 ; 01C8
    fstp st(0) ; 01CA
    fld qword ptr [rdx + 8] ; 01CC
    jmp mandalex_main_01e7 ; 01CF
mandalex_main_01d1:
    fld1  ; 01D1
    fcom st(1) ; 01D3
    fnstsw ax ; 01D5
    shr ah, 1 ; 01D7
    jb mandalex_main_01e2 ; 01D9
    fstp st(0) ; 01DB
    fdivr qword ptr [rdx + 0] ; 01DD
    jmp mandalex_main_01e7 ; 01E0
mandalex_main_01e2:
    fcompp  ; 01E2
    fld qword ptr [rdx + 0] ; 01E4
mandalex_main_01e7:
    fld qword ptr [rcx + 24] ; 01E7
    fmul st(0), st(1) ; 01EA
    fstp qword ptr [rcx + 24] ; 01EC
    fmul st(3), st(0) ; 01EF
    fmul st(2), st(0) ; 01F1
    fmulp st(1), st(0) ; 01F3
    fadd qword ptr [rdx + 152] ; 01F5
    fstp qword ptr [rcx + 16] ; 01F8
    fadd qword ptr [rdx + 144] ; 01FA
    fstp qword ptr [rcx + 8] ; 01FD
    fadd qword ptr [rdx + 136] ; 01FF
    fstp qword ptr [rcx] ; 0202
    nop  ; 0204
    fld qword ptr [rdx + 112] ; 0205
    mov eax, 0 ; 002A
    mov dword ptr [rsp + 4], eax ; 002F
    fld qword ptr [rcx] ; 0032
    fabs  ; 0034
    fcomp st(1) ; 0036
    fnstsw ax ; 0038
    shr ah, 1 ; 003A
    jae mandalex_post_0041 ; 003C
    inc dword ptr [rsp + 4] ; 003E
mandalex_post_0041:
    fld qword ptr [rcx + 8] ; 0041
    fabs  ; 0043
    fcomp st(1) ; 0045
    fnstsw ax ; 0047
    shr ah, 1 ; 0049
    jae mandalex_post_0050 ; 004B
    inc dword ptr [rsp + 4] ; 004D
mandalex_post_0050:
    fld qword ptr [rcx + 16] ; 0050
    fabs  ; 0052
    fcomp st(1) ; 0054
    fnstsw ax ; 0056
    shr ah, 1 ; 0058
    jae mandalex_post_005f ; 005A
    inc dword ptr [rsp + 4] ; 005C
mandalex_post_005f:
    cvttsd2si eax, qword ptr [rdx + 128] ; 005F
    and eax, 3 ; 0062
    xor eax, dword ptr [rsp + 4] ; 0065
    jne mandalex_post_00bf ; 0068
    fmul qword ptr [rdx + 120] ; 006E
    fld qword ptr [rcx] ; 0071
    fldz  ; 0073
    fcomp st(1) ; 0075
    fnstsw ax ; 0077
    shr ah, 1 ; 0079
    jae mandalex_post_0081 ; 007B
    fld1  ; 007D
    jmp mandalex_post_0085 ; 007F
mandalex_post_0081:
    fld1  ; 0081
    fchs  ; 0083
mandalex_post_0085:
    fmul st(0), st(2) ; 0085
    fsubp st(1), st(0) ; 0087
    fstp qword ptr [rcx] ; 0089
    fld qword ptr [rcx + 8] ; 008B
    fldz  ; 008D
    fcomp st(1) ; 008F
    fnstsw ax ; 0091
    shr ah, 1 ; 0093
    jae mandalex_post_009b ; 0095
    fld1  ; 0097
    jmp mandalex_post_009f ; 0099
mandalex_post_009b:
    fld1  ; 009B
    fchs  ; 009D
mandalex_post_009f:
    fmul st(0), st(2) ; 009F
    fsubp st(1), st(0) ; 00A1
    fstp qword ptr [rcx + 8] ; 00A3
    fld qword ptr [rcx + 16] ; 00A5
    fldz  ; 00A7
    fcomp st(1) ; 00A9
    fnstsw ax ; 00AB
    shr ah, 1 ; 00AD
    jae mandalex_post_00b5 ; 00AF
    fld1  ; 00B1
    jmp mandalex_post_00b9 ; 00B3
mandalex_post_00b5:
    fld1  ; 00B5
    fchs  ; 00B7
mandalex_post_00b9:
    fmul st(0), st(2) ; 00B9
    fsubp st(1), st(0) ; 00BB
    fstp qword ptr [rcx + 16] ; 00BD
mandalex_post_00bf:
    fstp st(0) ; 00BF
    fldcw word ptr [rsp]
    add rsp, 8
    ret
MandalexOriginal_x87 ENDP

; Literal Mandalay-KIFS.m3f 001E..0191; original Single matrix entries
; are supplied exactly expanded to Double. rcx=XYZ/DE, rdx=params/C.
.code
MandalayKIFSOriginal_x87 PROC
    sub rsp, 40
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    fld qword ptr [rcx + 16] ; 001E
    fabs  ; 0020
    fld qword ptr [rcx + 8] ; 0022
    fabs  ; 0024
    fld qword ptr [rcx] ; 0026
    fabs  ; 0028
    fcom st(1) ; 002A
    fnstsw ax ; 002C
    shr ah, 1 ; 002E
    jae mandalay_0034 ; 0030
    fxch st(1) ; 0032
mandalay_0034:
    fcom st(2) ; 0034
    fnstsw ax ; 0036
    shr ah, 1 ; 0038
    jae mandalay_003e ; 003A
    fxch st(2) ; 003C
mandalay_003e:
    fxch st(1) ; 003E
    fcom st(2) ; 0040
    fnstsw ax ; 0042
    shr ah, 1 ; 0044
    jae mandalay_004a ; 0046
    fxch st(2) ; 0048
mandalay_004a:
    fstp qword ptr [rcx + 8] ; 004A
    fstp qword ptr [rcx] ; 004C
    fstp qword ptr [rcx + 16] ; 004E
    nop  ; 0050
    fld qword ptr [rdx + 24] ; 0051
    fadd st(0), st(0) ; 0054
    fld qword ptr [rcx] ; 0056
    fsub st(0), st(1) ; 0058
    fstp qword ptr [rsp + 8] ; 005A
    fstp st(0) ; 005D
    nop  ; 005F
    fld qword ptr [rdx + 24] ; 0060
    fld qword ptr [rcx] ; 0063
    fsub st(0), st(1) ; 0065
    fabs  ; 0067
    fsubr st(0), st(1) ; 0069
    fstp qword ptr [rsp + 16] ; 006B
    fld qword ptr [rcx + 8] ; 006E
    fsub st(0), st(1) ; 0070
    fabs  ; 0072
    fsubr st(0), st(1) ; 0074
    fstp qword ptr [rsp + 24] ; 0076
    fstp st(0) ; 0079
    fld qword ptr [rdx + 112] ; 007B
    fldz  ; 007E
    fcomp st(1) ; 0080
    fnstsw ax ; 0082
    shr ah, 1 ; 0084
    jae mandalay_0092 ; 0086
    fld qword ptr [rcx + 16] ; 0088
    fsub st(0), st(1) ; 008A
    fabs  ; 008C
    fsubr st(0), st(1) ; 008E
    jmp mandalay_0094 ; 0090
mandalay_0092:
    fadd qword ptr [rcx + 16] ; 0092
mandalay_0094:
    fstp qword ptr [rsp + 32] ; 0094
    fstp st(0) ; 0097
    nop  ; 0099
    nop  ; 009A
    nop  ; 009B
    fld qword ptr [rsp + 8] ; 009C
    fldz  ; 009F
    fcomp st(1) ; 00A1
    fnstsw ax ; 00A3
    shr ah, 1 ; 00A5
    jae mandalay_00f6 ; 00A7
    fld qword ptr [rcx + 8] ; 00A9
    fcomp st(1) ; 00AB
    fnstsw ax ; 00AD
    shr ah, 1 ; 00AF
    jae mandalay_00f6 ; 00B1
    fld qword ptr [rdx + 104] ; 00B3
    fadd qword ptr [rcx + 8] ; 00B6
    fcomp st(1) ; 00B8
    fnstsw ax ; 00BA
    shr ah, 1 ; 00BC
    jae mandalay_00db ; 00BE
    fld qword ptr [rsp + 16] ; 00C0
    fadd qword ptr [rdx + 104] ; 00C3
    fstp qword ptr [rsp + 16] ; 00C6
    fld qword ptr [rdx + 104] ; 00C9
    fsub qword ptr [rdx + 24] ; 00CC
    fadd qword ptr [rcx + 8] ; 00CF
    fabs  ; 00D1
    fsubr qword ptr [rdx + 24] ; 00D3
    fstp qword ptr [rsp + 24] ; 00D6
    jmp mandalay_00f6 ; 00D9
mandalay_00db:
    fld qword ptr [rcx + 8] ; 00DB
    fchs  ; 00DD
    fstp qword ptr [rsp + 16] ; 00DF
    fld qword ptr [rcx] ; 00E2
    fsub qword ptr [rdx + 24] ; 00E4
    fsub qword ptr [rdx + 24] ; 00E7
    fsub qword ptr [rdx + 24] ; 00EA
    fabs  ; 00ED
    fsubr qword ptr [rdx + 24] ; 00EF
    fstp qword ptr [rsp + 24] ; 00F2
    nop  ; 00F5
mandalay_00f6:
    fstp st(0) ; 00F6
    nop  ; 00F8
    fld qword ptr [rsp + 16] ; 00F9
    fld qword ptr [rsp + 24] ; 00FC
    fld qword ptr [rsp + 32] ; 00FF
    fld st(0) ; 0102
    fmul qword ptr [rdx + 80] ; 0104
    fld st(2) ; 0107
    fmul qword ptr [rdx + 88] ; 0109
    faddp st(1), st(0) ; 010C
    fld st(3) ; 010E
    fmul qword ptr [rdx + 96] ; 0110
    faddp st(1), st(0) ; 0113
    fld st(1) ; 0115
    fmul qword ptr [rdx + 32] ; 0117
    fld st(3) ; 011A
    fmul qword ptr [rdx + 40] ; 011C
    faddp st(1), st(0) ; 011F
    fld st(4) ; 0121
    fmul qword ptr [rdx + 48] ; 0123
    faddp st(1), st(0) ; 0126
    fxch st(2) ; 0128
    fmul qword ptr [rdx + 56] ; 012A
    fxch st(3) ; 012D
    fmul qword ptr [rdx + 64] ; 012F
    faddp st(3), st(0) ; 0132
    fxch st(3) ; 0134
    fmul qword ptr [rdx + 72] ; 0136
    faddp st(2), st(0) ; 0139
    nop  ; 013B
    fld st(0) ; 013C
    fmul st(0), st(1) ; 013E
    fld st(2) ; 0140
    fmul st(0), st(3) ; 0142
    faddp st(1), st(0) ; 0144
    fld st(3) ; 0146
    fmul st(0), st(4) ; 0148
    faddp st(1), st(0) ; 014A
    fmul qword ptr [rdx + 120] ; 014C
    nop  ; 014F
    fcom qword ptr [rdx + 16] ; 0150
    fnstsw ax ; 0153
    shr ah, 1 ; 0155
    jae mandalay_0160 ; 0157
    fstp st(0) ; 0159
    fld qword ptr [rdx + 8] ; 015B
    jmp mandalay_0176 ; 015E
mandalay_0160:
    fld1  ; 0160
    fcom st(1) ; 0162
    fnstsw ax ; 0164
    shr ah, 1 ; 0166
    jb mandalay_0171 ; 0168
    fstp st(0) ; 016A
    fdivr qword ptr [rdx + 0] ; 016C
    jmp mandalay_0176 ; 016F
mandalay_0171:
    fcompp  ; 0171
    fld qword ptr [rdx + 0] ; 0173
mandalay_0176:
    fld qword ptr [rcx + 24] ; 0176
    fmul st(0), st(1) ; 0179
    fstp qword ptr [rcx + 24] ; 017B
    fmul st(3), st(0) ; 017E
    fmul st(2), st(0) ; 0180
    fmulp st(1), st(0) ; 0182
    fadd qword ptr [rdx + 144] ; 0184
    fstp qword ptr [rcx + 16] ; 0187
    fadd qword ptr [rdx + 136] ; 0189
    fstp qword ptr [rcx + 8] ; 018C
    fadd qword ptr [rdx + 128] ; 018E
    fstp qword ptr [rcx] ; 0191
    fldcw word ptr [rsp]
    add rsp, 40
    ret
MandalayKIFSOriginal_x87 ENDP

; Literal BulboxP-2.m3f 0016..021A. rcx=XYZ/DE; rdx=10 params + Julia C.
.code
BulboxP2Original_x87 PROC
    sub rsp, 56
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    fld qword ptr [rcx] ; 0016
    fadd qword ptr [rdx + 8] ; 0018
    fabs  ; 001B
    fld qword ptr [rcx] ; 001D
    fsub qword ptr [rdx + 8] ; 001F
    fabs  ; 0022
    fchs  ; 0024
    fsub qword ptr [rcx] ; 0026
    faddp st(1), st(0) ; 0028
    fstp qword ptr [rcx] ; 002A
    fld qword ptr [rcx + 8] ; 002C
    fadd qword ptr [rdx + 8] ; 002E
    fabs  ; 0031
    fld qword ptr [rcx + 8] ; 0033
    fsub qword ptr [rdx + 8] ; 0035
    fabs  ; 0038
    fchs  ; 003A
    fsub qword ptr [rcx + 8] ; 003C
    faddp st(1), st(0) ; 003E
    fstp qword ptr [rcx + 8] ; 0040
    fld qword ptr [rcx + 16] ; 0042
    fadd qword ptr [rdx + 8] ; 0044
    fabs  ; 0047
    fld qword ptr [rcx + 16] ; 0049
    fsub qword ptr [rdx + 8] ; 004B
    fabs  ; 004E
    fchs  ; 0050
    fsub qword ptr [rcx + 16] ; 0052
    faddp st(1), st(0) ; 0054
    fstp qword ptr [rcx + 16] ; 0056
    fld qword ptr [rcx] ; 0058
    fmul st(0), st(0) ; 005A
    fld qword ptr [rcx + 8] ; 005C
    fmul st(0), st(0) ; 005E
    fld qword ptr [rcx + 16] ; 0060
    fmul st(0), st(0) ; 0062
    faddp st(1), st(0) ; 0064
    faddp st(1), st(0) ; 0066
    fst qword ptr [rsp + 24] ; 0068
    fsqrt  ; 006B
    fstp qword ptr [rsp + 32] ; 006D
    fld qword ptr [rdx + 0] ; 0070
    fld qword ptr [rcx] ; 0073
    fmul st(0), st(1) ; 0075
    fsub qword ptr [rdx + 40] ; 0077
    fstp qword ptr [rcx] ; 007A
    fld qword ptr [rcx + 8] ; 007C
    fmul st(0), st(1) ; 007E
    fsub qword ptr [rdx + 48] ; 0080
    fstp qword ptr [rcx + 8] ; 0083
    fld qword ptr [rcx + 16] ; 0085
    fmul st(0), st(1) ; 0087
    fsub qword ptr [rdx + 56] ; 0089
    fstp qword ptr [rcx + 16] ; 008C
    fstp st(0) ; 008E
    cvttsd2si eax, qword ptr [rdx + 72]
    cmp eax, 0 ; 0090
    jne bulbox_00ab ; 0094
    nop  ; 0096
    fld qword ptr [rsp + 32] ; 0097
    fld1  ; 009A
    fxch st(1) ; 009C
    fcompp  ; 009E
    fnstsw ax ; 00A0
    shr ah, 1 ; 00A2
    jae bulbox_0173 ; 00A4
    nop  ; 00AA
bulbox_00ab:
    fld qword ptr [rsp + 32] ; 00AB
    fld qword ptr [rdx + 16] ; 00AE
    fcompp  ; 00B1
    fnstsw ax ; 00B3
    shr ah, 1 ; 00B5
    jae bulbox_018e ; 00B7
    nop  ; 00BD
    nop  ; 00BE
    fld qword ptr [rsp + 32] ; 00BF
    fsub qword ptr [rdx + 16] ; 00C2
    fld1  ; 00C5
    fsub qword ptr [rdx + 16] ; 00C7
    fdivp st(1), st(0) ; 00CA
    fst qword ptr [rsp + 40] ; 00CC
    fld1  ; 00CF
    fsub st(0), st(1) ; 00D1
    fstp qword ptr [rsp + 48] ; 00D3
    fstp st(0) ; 00D6
    nop  ; 00D8
    fld qword ptr [rcx + 8] ; 00D9
    fmul st(0), st(0) ; 00DB
    fld qword ptr [rcx] ; 00DD
    fmul st(0), st(0) ; 00DF
    faddp st(1), st(0) ; 00E1
    fld qword ptr [rdx + 32] ; 00E3
    fabs  ; 00E6
    faddp st(1), st(0) ; 00E8
    fsqrt  ; 00EA
    fstp qword ptr [rsp + 8] ; 00EC
    nop  ; 00EF
    nop  ; 00F0
    fld qword ptr [rsp + 24] ; 00F1
    fmul st(0), st(0) ; 00F4
    fld qword ptr [rdx + 24] ; 00F6
    fdivrp st(1), st(0) ; 00F9
    fstp qword ptr [rsp + 24] ; 00FB
    nop  ; 00FE
    fld qword ptr [rcx + 16] ; 00FF
    fdiv qword ptr [rsp + 8] ; 0101
    fmul st(0), st(0) ; 0104
    fchs  ; 0106
    fld1  ; 0108
    faddp st(1), st(0) ; 010A
    fstp qword ptr [rsp + 16] ; 010C
    nop  ; 010F
    fld qword ptr [rcx] ; 0110
    fmul st(0), st(0) ; 0112
    fld qword ptr [rcx + 8] ; 0114
    fmul st(0), st(0) ; 0116
    fchs  ; 0118
    faddp st(1), st(0) ; 011A
    fmul qword ptr [rsp + 16] ; 011C
    fmul qword ptr [rsp + 24] ; 011F
    nop  ; 0122
    fld qword ptr [rcx] ; 0123
    fmul qword ptr [rcx + 8] ; 0125
    fchs  ; 0127
    fmul qword ptr [rsp + 16] ; 0129
    fmul qword ptr [rsp + 24] ; 012C
    nop  ; 012F
    fld qword ptr [rcx + 16] ; 0130
    fadd st(0), st(0) ; 0132
    fchs  ; 0134
    fmul qword ptr [rdx + 64] ; 0136
    fmul qword ptr [rsp + 8] ; 0139
    fmul qword ptr [rsp + 24] ; 013C
    nop  ; 013F
    fmul qword ptr [rsp + 48] ; 0140
    fld qword ptr [rcx + 16] ; 0143
    fmul qword ptr [rsp + 40] ; 0145
    faddp st(1), st(0) ; 0148
    fadd qword ptr [rdx + 96] ; 014A
    fstp qword ptr [rcx + 16] ; 014D
    fmul qword ptr [rsp + 48] ; 014F
    fld qword ptr [rcx + 8] ; 0152
    fmul qword ptr [rsp + 40] ; 0154
    faddp st(1), st(0) ; 0157
    fadd qword ptr [rdx + 88] ; 0159
    fstp qword ptr [rcx + 8] ; 015C
    fmul qword ptr [rsp + 48] ; 015E
    fld qword ptr [rcx] ; 0161
    fmul qword ptr [rsp + 40] ; 0163
    faddp st(1), st(0) ; 0166
    fadd qword ptr [rdx + 80] ; 0168
    fstp qword ptr [rcx] ; 016B
    jmp bulbox_0207 ; 016D
    nop  ; 0172
bulbox_0173:
    fld qword ptr [rcx] ; 0173
    fadd qword ptr [rdx + 80] ; 0175
    fstp qword ptr [rcx] ; 0178
    fld qword ptr [rcx + 8] ; 017A
    fadd qword ptr [rdx + 88] ; 017C
    fstp qword ptr [rcx + 8] ; 017F
    fld qword ptr [rcx + 16] ; 0181
    fadd qword ptr [rdx + 96] ; 0183
    fstp qword ptr [rcx + 16] ; 0186
    jmp bulbox_0207 ; 0188
    nop  ; 018D
bulbox_018e:
    fld qword ptr [rcx + 8] ; 018E
    fmul st(0), st(0) ; 0190
    fld qword ptr [rcx] ; 0192
    fmul st(0), st(0) ; 0194
    faddp st(1), st(0) ; 0196
    fld qword ptr [rdx + 32] ; 0198
    fabs  ; 019B
    faddp st(1), st(0) ; 019D
    fsqrt  ; 019F
    fstp qword ptr [rsp + 8] ; 01A1
    nop  ; 01A4
    fld qword ptr [rsp + 24] ; 01A5
    fmul st(0), st(0) ; 01A8
    fld qword ptr [rdx + 24] ; 01AA
    fdivrp st(1), st(0) ; 01AD
    fstp qword ptr [rsp + 24] ; 01AF
    nop  ; 01B2
    fld qword ptr [rcx + 16] ; 01B3
    fdiv qword ptr [rsp + 8] ; 01B5
    fmul st(0), st(0) ; 01B8
    fchs  ; 01BA
    fld1  ; 01BC
    faddp st(1), st(0) ; 01BE
    fstp qword ptr [rsp + 16] ; 01C0
    nop  ; 01C3
    fld qword ptr [rcx] ; 01C4
    fmul st(0), st(0) ; 01C6
    fld qword ptr [rcx + 8] ; 01C8
    fmul st(0), st(0) ; 01CA
    fchs  ; 01CC
    faddp st(1), st(0) ; 01CE
    fmul qword ptr [rsp + 16] ; 01D0
    fmul qword ptr [rsp + 24] ; 01D3
    fadd qword ptr [rdx + 80] ; 01D6
    nop  ; 01D9
    fld qword ptr [rcx] ; 01DA
    fmul qword ptr [rcx + 8] ; 01DC
    fchs  ; 01DE
    fmul qword ptr [rsp + 16] ; 01E0
    fmul qword ptr [rsp + 24] ; 01E3
    fadd qword ptr [rdx + 88] ; 01E6
    nop  ; 01E9
    fld qword ptr [rcx + 16] ; 01EA
    fadd st(0), st(0) ; 01EC
    fchs  ; 01EE
    fmul qword ptr [rsp + 8] ; 01F0
    fmul qword ptr [rsp + 24] ; 01F3
    fmul qword ptr [rdx + 64] ; 01F6
    fadd qword ptr [rdx + 96] ; 01F9
    nop  ; 01FC
    fstp qword ptr [rcx + 16] ; 01FD
    fstp qword ptr [rcx + 8] ; 01FF
    fstp qword ptr [rcx] ; 0201
    nop  ; 0203
    nop  ; 0204
    nop  ; 0205
    nop  ; 0206
bulbox_0207:
    fld qword ptr [rcx] ; 0207
    fadd qword ptr [rdx + 40] ; 0209
    fstp qword ptr [rcx] ; 020C
    fld qword ptr [rcx + 8] ; 020E
    fadd qword ptr [rdx + 48] ; 0210
    fstp qword ptr [rcx + 8] ; 0213
    fld qword ptr [rcx + 16] ; 0215
    fadd qword ptr [rdx + 56] ; 0217
    fstp qword ptr [rcx + 16] ; 021A
    fldcw word ptr [rsp]
    add rsp, 56
    ret
BulboxP2Original_x87 ENDP

 ; _CelticMode.m3f offsets 0010..005A. rcx=XYZ, rdx=options + Julia C.
; Preserve the original x87 operations, including unordered Mystic != 0.
.code
CelticModeOriginal_x87 PROC
    sub rsp, 16
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 0137Fh
    fldcw word ptr [rsp + 2]
    fld qword ptr [rcx]
    fld qword ptr [rcx]
    fld qword ptr [rdx + 32]
    fmul qword ptr [rdx]
    faddp st(1), st(0)
    fabs
    fld qword ptr [rdx + 32]
    fmul qword ptr [rdx]
    fchs
    faddp st(1), st(0)
    fxch st(1)
    fmul qword ptr [rdx + 8]
    faddp st(1), st(0)
    fstp qword ptr [rcx]
    fld qword ptr [rdx + 16]
    fldz
    fcompp
    fnstsw ax
    and ah, 041h
    xor ah, 040h
    je celtic_done
    fld qword ptr [rcx + 16]
    fld qword ptr [rdx + 48]
    fmul qword ptr [rdx + 16]
    faddp st(1), st(0)
    fabs
    fld qword ptr [rdx + 48]
    fmul qword ptr [rdx + 24]
    fchs
    faddp st(1), st(0)
    fstp qword ptr [rcx + 16]
celtic_done:
    fldcw word ptr [rsp]
    add rsp, 16
    ret
CelticModeOriginal_x87 ENDP


.code
; ABoxSphereOffset4d original 0011..00DB; rcx=XYZW,Deriv1, rdx=9 options+C.
ABoxSphereOffset4dOriginal_x87 PROC
    sub rsp, 16
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 0137Fh
    fldcw word ptr [rsp + 2]
    fld qword ptr [rdx + 24] ; 0011
    fld qword ptr [rcx] ; 0014
    fld st(0) ; 0016
    fsub st(0), st(2) ; 0018
    fabs  ; 001A
    fadd st(0), st(1) ; 001C
    fxch st(1) ; 001E
    fadd st(0), st(2) ; 0020
    fabs  ; 0022
    fsubrp st(1), st(0) ; 0024
    fsub qword ptr [rdx + 32] ; 0026
    fld qword ptr [rcx + 8] ; 0029
    fld st(0) ; 002B
    fsub st(0), st(3) ; 002D
    fabs  ; 002F
    fadd st(0), st(1) ; 0031
    fxch st(1) ; 0033
    fadd st(0), st(3) ; 0035
    fabs  ; 0037
    fsubrp st(1), st(0) ; 0039
    fsub qword ptr [rdx + 40] ; 003B
    fld qword ptr [rcx + 16] ; 003E
    fld st(0) ; 0040
    fsub st(0), st(4) ; 0042
    fabs  ; 0044
    fadd st(0), st(1) ; 0046
    fxch st(1) ; 0048
    fadd st(0), st(4) ; 004A
    fabs  ; 004C
    fsubrp st(1), st(0) ; 004E
    fsub qword ptr [rdx + 48] ; 0050
    fld qword ptr [rcx + 24] ; 0053
    fld st(0) ; 0056
    fsub st(0), st(5) ; 0058
    fabs  ; 005A
    fadd st(0), st(1) ; 005C
    fxch st(1) ; 005E
    fadd st(0), st(5) ; 0060
    fabs  ; 0062
    fsubrp st(1), st(0) ; 0064
    fsub qword ptr [rdx + 56] ; 0066
    fxch st(4) ; 0069
    fstp st(0) ; 006B
    fld st(0) ; 006D
    fmul st(0), st(1) ; 006F
    fld st(2) ; 0071
    fmul st(0), st(3) ; 0073
    faddp st(1), st(0) ; 0075
    fld st(3) ; 0077
    fmul st(0), st(4) ; 0079
    faddp st(1), st(0) ; 007B
    fld st(4) ; 007D
    fmul st(0), st(5) ; 007F
    faddp st(1), st(0) ; 0081
    fcom qword ptr [rdx + 16] ; 0083
    fnstsw ax ; 0086
    shr ah, 1 ; 0088
    jae sphereoff_0093 ; 008A
    fstp st(0) ; 008C
    fld qword ptr [rdx + 8] ; 008E
    jmp sphereoff_00a6 ; 0091
sphereoff_0093:
    fld1  ; 0093
    fcomp st(1) ; 0095
    fnstsw ax ; 0097
    fld qword ptr [rdx + 0] ; 0099
    shr ah, 1 ; 009C
    jb sphereoff_00a2 ; 009E
    fdiv st(0), st(1) ; 00A0
sphereoff_00a2:
    fxch st(1) ; 00A2
    fstp st(0) ; 00A4
sphereoff_00a6:
    fld qword ptr [rcx + 32] ; 00A6
    fmul st(0), st(1) ; 00A9
    fstp qword ptr [rcx + 32] ; 00AB
    fmul st(4), st(0) ; 00AE
    fmul st(3), st(0) ; 00B0
    fmul st(2), st(0) ; 00B2
    fmulp st(1), st(0) ; 00B4
    fadd qword ptr [rdx + 88] ; 00B6
    fadd qword ptr [rdx + 48] ; 00B9
    fstp qword ptr [rcx + 16] ; 00BC
    fadd qword ptr [rdx + 80] ; 00BE
    fadd qword ptr [rdx + 40] ; 00C1
    fstp qword ptr [rcx + 8] ; 00C4
    fadd qword ptr [rdx + 72] ; 00C6
    fadd qword ptr [rdx + 32] ; 00C9
    fstp qword ptr [rcx] ; 00CC
    fadd qword ptr [rdx + 64] ; 00CF
    fadd qword ptr [rdx + 56] ; 00D2
    fadd qword ptr [rdx + 96] ; 00D5
    fstp qword ptr [rcx + 24] ; 00DB
    fldcw word ptr [rsp]
    add rsp, 16
    ret
ABoxSphereOffset4dOriginal_x87 ENDP


.code
; ABoxPlatinumB.m3f 0010..0231, literal operation/store order.
; rcx=XYZ and signed DE; rdx=26 expanded options followed by Julia C.
; Matrix entries are exact Double promotions of the original Single values.
ABoxPlatinumBOriginal_x87 PROC
    sub rsp, 40
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 0137Fh
    fldcw word ptr [rsp + 2]
    fld qword ptr [rdx + 24] ; 0010
    fld qword ptr [rcx] ; 0013
    fmul qword ptr [rdx + 160] ; 0015
    fld qword ptr [rcx + 16] ; 001B
    fmul qword ptr [rdx + 152] ; 001D
    fsubp st(1), st(0) ; 0023
    fstp qword ptr [rsp + 8] ; 0025
    fld qword ptr [rcx] ; 0028
    fmul qword ptr [rdx + 152] ; 002A
    fld qword ptr [rcx + 16] ; 0030
    fmul qword ptr [rdx + 160] ; 0032
    faddp st(1), st(0) ; 0038
    fstp qword ptr [rsp + 16] ; 003A
    fld qword ptr [rsp + 8] ; 0040
    fmul qword ptr [rdx + 144] ; 0043
    fld qword ptr [rcx + 8] ; 0046
    fmul qword ptr [rdx + 136] ; 0048
    fsubp st(1), st(0) ; 004B
    fld qword ptr [rsp + 8] ; 004E
    fmul qword ptr [rdx + 136] ; 0051
    fld qword ptr [rcx + 8] ; 0054
    fmul qword ptr [rdx + 144] ; 0056
    faddp st(1), st(0) ; 0059
    fstp qword ptr [rsp + 24] ; 005B
    fld st(0) ; 0060
    fsub st(0), st(2) ; 0062
    fabs  ; 0064
    fadd st(0), st(1) ; 0066
    fxch st(1) ; 0068
    fadd st(0), st(2) ; 006A
    fabs  ; 006C
    fsubrp st(1), st(0) ; 006E
    fmul qword ptr [rdx + 144] ; 0070
    fld qword ptr [rsp + 24] ; 0073
    fmul qword ptr [rdx + 136] ; 0077
    fsubp st(1), st(0) ; 007A
    fmul qword ptr [rdx + 160] ; 007D
    fld qword ptr [rsp + 16] ; 0083
    fmul qword ptr [rdx + 152] ; 0087
    fsubp st(1), st(0) ; 008D
    fld qword ptr [rcx + 8] ; 0090
    fmul qword ptr [rdx + 192] ; 0092
    fld qword ptr [rcx + 16] ; 0098
    fmul qword ptr [rdx + 184] ; 009A
    fsubp st(1), st(0) ; 00A0
    fstp qword ptr [rsp + 8] ; 00A2
    fld qword ptr [rcx + 8] ; 00A5
    fmul qword ptr [rdx + 184] ; 00A7
    fld qword ptr [rcx + 16] ; 00AD
    fmul qword ptr [rdx + 192] ; 00AF
    faddp st(1), st(0) ; 00B5
    fstp qword ptr [rsp + 16] ; 00B7
    fld qword ptr [rsp + 8] ; 00BD
    fmul qword ptr [rdx + 176] ; 00C0
    fld qword ptr [rcx] ; 00C6
    fmul qword ptr [rdx + 168] ; 00C8
    fsubp st(1), st(0) ; 00CE
    fld qword ptr [rsp + 8] ; 00D1
    fmul qword ptr [rdx + 168] ; 00D4
    fld qword ptr [rcx] ; 00DA
    fmul qword ptr [rdx + 176] ; 00DC
    faddp st(1), st(0) ; 00E2
    fstp qword ptr [rsp + 24] ; 00E4
    fld st(0) ; 00E8
    fsub st(0), st(3) ; 00EA
    fabs  ; 00EC
    fadd st(0), st(1) ; 00EE
    fxch st(1) ; 00F0
    fadd st(0), st(3) ; 00F2
    fabs  ; 00F4
    fsubrp st(1), st(0) ; 00F6
    fmul qword ptr [rdx + 176] ; 00F8
    fld qword ptr [rsp + 24] ; 00FF
    fmul qword ptr [rdx + 168] ; 0103
    fsubp st(1), st(0) ; 0109
    fmul qword ptr [rdx + 192] ; 010C
    fld qword ptr [rsp + 16] ; 0112
    fmul qword ptr [rdx + 184] ; 0116
    fsubp st(1), st(0) ; 011C
    fld qword ptr [rcx + 16] ; 011E
    fld st(0) ; 0120
    fsub st(0), st(4) ; 0122
    fabs  ; 0124
    fadd st(0), st(1) ; 0126
    fxch st(1) ; 0128
    fadd st(0), st(4) ; 012A
    fabs  ; 012C
    fsubrp st(1), st(0) ; 012E
    cvttsd2si eax, qword ptr [rdx + 200] ; 0131
    and eax, 4 ; 0137
    je platinumb_013e ; 013A
    fabs  ; 013C
platinumb_013e:
    cvttsd2si eax, qword ptr [rdx + 200] ; 013E
    and eax, 2 ; 0144
    je platinumb_014f ; 0147
    fxch st(1) ; 0149
    fabs  ; 014B
    fxch st(1) ; 014D
platinumb_014f:
    cvttsd2si eax, qword ptr [rdx + 200] ; 014F
    and eax, 1 ; 0155
    je platinumb_0160 ; 0158
    fxch st(2) ; 015A
    fabs  ; 015C
    fxch st(2) ; 015E
platinumb_0160:
    fld qword ptr [rdx + 120] ; 0161
    fadd st(1), st(0) ; 0164
    fstp st(0) ; 0166
    fld qword ptr [rdx + 112] ; 0169
    fadd st(2), st(0) ; 016C
    fstp st(0) ; 016E
    fld qword ptr [rdx + 104] ; 0171
    fadd st(3), st(0) ; 0174
    fstp st(0) ; 0176
    fld st(0) ; 0178
    fmul st(0), st(0) ; 017A
    fld st(2) ; 017C
    fmul st(0), st(0) ; 017E
    faddp st(1), st(0) ; 0180
    fld st(3) ; 0182
    fmul st(0), st(0) ; 0184
    faddp st(1), st(0) ; 0186
    fmul qword ptr [rdx + 128] ; 0188
    fcom qword ptr [rdx + 16] ; 018C
    fnstsw ax ; 018F
    shr ah, 1 ; 0191
    jae platinumb_019c ; 0193
    fstp st(0) ; 0195
    fld qword ptr [rdx + 8] ; 0197
    jmp platinumb_01b2 ; 019A
platinumb_019c:
    fld1  ; 019C
    fcom st(1) ; 019E
    fnstsw ax ; 01A0
    shr ah, 1 ; 01A2
    jb platinumb_01ad ; 01A4
    fstp st(0) ; 01A6
    fdivr qword ptr [rdx + 0] ; 01A8
    jmp platinumb_01b2 ; 01AB
platinumb_01ad:
    fcompp  ; 01AD
    fld qword ptr [rdx + 0] ; 01AF
platinumb_01b2:
    fld qword ptr [rcx + 24] ; 01B2
    fmul st(0), st(1) ; 01B5
    fstp qword ptr [rcx + 24] ; 01B7
    fmul st(3), st(0) ; 01BA
    fmul st(2), st(0) ; 01BC
    fmulp st(1), st(0) ; 01BE
    fld qword ptr [rdx + 120] ; 01C1
    fmul qword ptr [rdx + 0] ; 01C4
    fsub st(1), st(0) ; 01C7
    fstp st(0) ; 01C9
    fld qword ptr [rdx + 112] ; 01CC
    fmul qword ptr [rdx + 0] ; 01CF
    fsub st(2), st(0) ; 01D2
    fstp st(0) ; 01D4
    fld qword ptr [rdx + 104] ; 01D7
    fmul qword ptr [rdx + 0] ; 01DA
    fsub st(3), st(0) ; 01DD
    fstp st(0) ; 01DF
    fxch st(2) ; 01E2
    fld st(0) ; 01E4
    fmul qword ptr [rdx + 80] ; 01E6
    fld st(2) ; 01E9
    fmul qword ptr [rdx + 88] ; 01EB
    faddp st(1), st(0) ; 01EE
    fld st(3) ; 01F0
    fmul qword ptr [rdx + 96] ; 01F2
    faddp st(1), st(0) ; 01F5
    fld st(1) ; 01F7
    fmul qword ptr [rdx + 32] ; 01F9
    fld st(3) ; 01FC
    fmul qword ptr [rdx + 40] ; 01FE
    faddp st(1), st(0) ; 0201
    fld st(4) ; 0203
    fmul qword ptr [rdx + 48] ; 0205
    faddp st(1), st(0) ; 0208
    fxch st(2) ; 020A
    fmul qword ptr [rdx + 56] ; 020C
    fxch st(3) ; 020F
    fmul qword ptr [rdx + 64] ; 0211
    faddp st(3), st(0) ; 0214
    fxch st(3) ; 0216
    fmul qword ptr [rdx + 72] ; 0218
    faddp st(2), st(0) ; 021B
    fxch st(2) ; 021D
    fadd qword ptr [rdx + 224] ; 0222
    fstp qword ptr [rcx + 16] ; 0225
    fadd qword ptr [rdx + 216] ; 0227
    fstp qword ptr [rcx + 8] ; 022A
    fadd qword ptr [rdx + 208] ; 022C
    fstp qword ptr [rcx] ; 022F
    fstp st(0) ; 0231
    fldcw word ptr [rsp]
    add rsp, 40
    ret
ABoxPlatinumBOriginal_x87 ENDP


.code
; KnotsIFS original 004B..0109 after SSE2 affine transform.
; rcx=local XYZ + Rout, rdx=expanded options, r8d=Apply.
KnotsIFSOriginal_x87 PROC
    sub rsp, 72
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 0137Fh
    fldcw word ptr [rsp + 2]
    movsd xmm0, qword ptr [rcx + 0]
    movsd qword ptr [rsp + 8], xmm0
    cvttsd2si eax, qword ptr [rdx + 56]
    mov dword ptr [rsp + 48], eax
    movsd xmm0, qword ptr [rcx + 8]
    movsd qword ptr [rsp + 16], xmm0
    cvttsd2si eax, qword ptr [rdx + 64]
    mov dword ptr [rsp + 52], eax
    movsd xmm0, qword ptr [rcx + 16]
    movsd qword ptr [rsp + 24], xmm0
    cvttsd2si eax, qword ptr [rdx + 72]
    mov dword ptr [rsp + 56], eax
    fld qword ptr [rsp + 8] ; 004B
    fld qword ptr [rsp + 16] ; 004E
    fpatan  ; 0052
    fild dword ptr [rsp + 52] ; 0054
    fild dword ptr [rsp + 56] ; 0057
    fidiv dword ptr [rsp + 48] ; 005A
    faddp st(1), st(0) ; 005D
    fmulp st(1), st(0) ; 005F
    fsincos  ; 0061
    fstp qword ptr [rsp + 40] ; 0063
    fstp qword ptr [rsp + 32] ; 0067
    fld qword ptr [rsp + 8] ; 006B
    fmul st(0), st(0) ; 006E
    fld qword ptr [rsp + 16] ; 0070
    fmul st(0), st(0) ; 0074
    faddp st(1), st(0) ; 0076
    fsqrt  ; 0078
    fsub qword ptr [rdx + 0] ; 007A
    fstp qword ptr [rsp + 8] ; 007D
    fld qword ptr [rsp + 8] ; 0080
    fmul qword ptr [rsp + 32] ; 0083
    fld qword ptr [rsp + 24] ; 0087
    fmul qword ptr [rsp + 40] ; 008B
    fchs  ; 008F
    faddp st(1), st(0) ; 0091
    fld qword ptr [rsp + 8] ; 0093
    fmul qword ptr [rsp + 40] ; 0096
    fld qword ptr [rsp + 24] ; 009A
    fmul qword ptr [rsp + 32] ; 009E
    faddp st(1), st(0) ; 00A2
    fstp qword ptr [rsp + 8] ; 00A4
    fstp qword ptr [rsp + 24] ; 00A7
    fld qword ptr [rsp + 8] ; 00AB
    fld qword ptr [rsp + 24] ; 00AE
    fpatan  ; 00B2
    fsubr qword ptr [rdx + 104] ; 00B4
    fmul qword ptr [rdx + 88] ; 00B7
    fimul dword ptr [rsp + 48] ; 00B9
    frndint  ; 00BC
    fidiv dword ptr [rsp + 48] ; 00BE
    fmul qword ptr [rdx + 96] ; 00C1
    fsincos  ; 00C4
    fstp qword ptr [rsp + 32] ; 00C6
    fstp qword ptr [rsp + 40] ; 00CA
    fld qword ptr [rsp + 8] ; 00CE
    fmul qword ptr [rsp + 32] ; 00D1
    fld qword ptr [rsp + 24] ; 00D5
    fmul qword ptr [rsp + 40] ; 00D9
    fchs  ; 00DD
    faddp st(1), st(0) ; 00DF
    fadd qword ptr [rdx + 80] ; 00E1
    fmul st(0), st(0) ; 00E4
    fld qword ptr [rsp + 8] ; 00E6
    fmul qword ptr [rsp + 40] ; 00E9
    fld qword ptr [rsp + 24] ; 00ED
    fmul qword ptr [rsp + 32] ; 00F1
    faddp st(1), st(0) ; 00F5
    fmul st(0), st(0) ; 00F7
    faddp st(1), st(0) ; 00F9
    fsqrt  ; 00FB
    fsub qword ptr [rdx + 40] ; 00FD
    cmp r8d, 0 ; 0100
    jne knots_0109 ; 0104
    fdiv qword ptr [rdx + 8] ; 0106
knots_0109:
    fstp qword ptr [rcx + 24] ; 0109
    fldcw word ptr [rsp]
    add rsp, 72
    ret
KnotsIFSOriginal_x87 ENDP


.code
; KleinBotTIFS original 004B..0167; rcx=local XYZ/Rout, rdx=options, r8d=Apply.
KleinBotTIFSOriginal_x87 PROC
    sub rsp, 56
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 0137Fh
    fldcw word ptr [rsp + 2]
    mov r9, rdx
    movupd xmm0, xmmword ptr [rcx]
    movsd xmm1, qword ptr [rcx + 16]
    movupd xmmword ptr [rsp + 8], xmm0
    movsd qword ptr [rsp + 24], xmm1
    fldz  ; 004B
    fstp qword ptr [rsp + 32] ; 004D
    fld qword ptr [rsp + 8] ; 0051
    fld qword ptr [rsp + 16] ; 0054
    ftst  ; 0058
    wait  ; 005A
    fnstsw ax ; 005B
    shr ah, 1 ; 005D
    jae kleinbot_006a ; 005F
    fld qword ptr [r9 + 64] ; 0061
    mov edx, 1 ; 0063
    jmp kleinbot_0073 ; 0068
kleinbot_006a:
    fld qword ptr [r9 + 64] ; 006A
    fchs  ; 006C
    mov edx, -1 ; 006E
kleinbot_0073:
    fstp st(1) ; 0073
    faddp st(1), st(0) ; 0075
    fstp qword ptr [rsp + 8] ; 0077
    fld qword ptr [rsp + 8] ; 007A
    fmul st(0), st(0) ; 007D
    fld qword ptr [rsp + 16] ; 007F
    fmul st(0), st(0) ; 0083
    faddp st(1), st(0) ; 0085
    fsqrt  ; 0087
    fsub qword ptr [r9 + 64] ; 0089
    fmul st(0), st(0) ; 008B
    fld qword ptr [rsp + 24] ; 008D
    fmul st(0), st(0) ; 0091
    faddp st(1), st(0) ; 0093
    fsqrt  ; 0095
    fsub qword ptr [r9 + 0] ; 0097
    fabs  ; 009A
    fsub qword ptr [r9 + 56] ; 009C
    fstp qword ptr [rcx + 24] ; 009F
    movupd xmmword ptr [rsp + 8], xmm0 ; 00A2
    movsd qword ptr [rsp + 24], xmm1 ; 00A7
    cmp edx, 1 ; 00AD
    jne kleinbot_00f9 ; 00B0
    fld qword ptr [rsp + 8] ; 00B2
    fmul st(0), st(0) ; 00B5
    fld qword ptr [rsp + 16] ; 00B7
    fmul st(0), st(0) ; 00BB
    faddp st(1), st(0) ; 00BD
    fsqrt  ; 00BF
    fld qword ptr [rsp + 16] ; 00C1
    fld qword ptr [rsp + 8] ; 00C5
    fpatan  ; 00C8
    fmul qword ptr [r9 + 96] ; 00CA
    fld qword ptr [r9 + 40] ; 00CD
    fadd st(0), st(0) ; 00D0
    fmulp st(1), st(0) ; 00D2
    fadd qword ptr [r9 + 0] ; 00D4
    fstp qword ptr [rsp + 16] ; 00D7
    fsub qword ptr [r9 + 72] ; 00DB
    fmul st(0), st(0) ; 00DE
    fld qword ptr [rsp + 24] ; 00E0
    fmul st(0), st(0) ; 00E4
    faddp st(1), st(0) ; 00E6
    fsqrt  ; 00E8
    fsub qword ptr [rsp + 16] ; 00EA
    fst qword ptr [rsp + 32] ; 00EE
    fabs  ; 00F2
    fsub qword ptr [r9 + 56] ; 00F4
    jmp kleinbot_013b ; 00F7
kleinbot_00f9:
    fld qword ptr [rsp + 16] ; 00F9
    fld qword ptr [rsp + 24] ; 00FD
    fld qword ptr [rsp + 8] ; 0101
    fadd qword ptr [r9 + 72] ; 0104
    fstp qword ptr [rsp + 8] ; 0107
    fstp qword ptr [rsp + 16] ; 010A
    fstp qword ptr [rsp + 24] ; 010E
    fld qword ptr [rsp + 8] ; 0112
    fmul st(0), st(0) ; 0115
    fld qword ptr [rsp + 16] ; 0117
    fmul st(0), st(0) ; 011B
    faddp st(1), st(0) ; 011D
    fsqrt  ; 011F
    fadd qword ptr [r9 + 40] ; 0121
    fsub qword ptr [r9 + 0] ; 0124
    fmul st(0), st(0) ; 0127
    fld qword ptr [rsp + 24] ; 0129
    fmul st(0), st(0) ; 012D
    faddp st(1), st(0) ; 012F
    fsqrt  ; 0131
    fsub qword ptr [r9 + 40] ; 0133
    fabs  ; 0136
    fsub qword ptr [r9 + 56] ; 0138
kleinbot_013b:
    fld qword ptr [rcx + 24] ; 013B
    fld qword ptr [rsp + 32] ; 013E
    fld st(0) ; 0142
    fabs  ; 0144
    fsubrp st(1), st(0) ; 0146
    faddp st(1), st(0) ; 0148
    fstp qword ptr [rcx + 24] ; 014A
    fcom qword ptr [rcx + 24] ; 014D
    wait  ; 0150
    fnstsw ax ; 0151
    shr ah, 1 ; 0153
    jae kleinbot_0159 ; 0155
    jmp kleinbot_015e ; 0157
kleinbot_0159:
    fstp st(0) ; 0159
    fld qword ptr [rcx + 24] ; 015B
kleinbot_015e:
    cmp r8d, 0 ; 015E
    jne kleinbot_0167 ; 0162
    fdiv qword ptr [r9 + 8] ; 0164
kleinbot_0167:
    fstp qword ptr [rcx + 24] ; 0167
    fldcw word ptr [rsp]
    add rsp, 56
    ret
KleinBotTIFSOriginal_x87 ENDP


.code
; General original 0014..07C6, including original flag tests.
; rcx=XYZ/CXYZ, rdx=expanded options/constants. Integer options are materialized.
GeneralOriginal_x87 PROC
    sub rsp, 136
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 0137Fh
    fldcw word ptr [rsp + 2]
    cvttsd2si eax, qword ptr [rdx + 24]
    mov dword ptr [rsp + 88], eax
    cvttsd2si eax, qword ptr [rdx + 32]
    mov dword ptr [rsp + 92], eax
    cvttsd2si eax, qword ptr [rdx + 40]
    mov dword ptr [rsp + 96], eax
    cvttsd2si eax, qword ptr [rdx + 48]
    mov dword ptr [rsp + 100], eax
    cvttsd2si eax, qword ptr [rdx + 56]
    mov dword ptr [rsp + 104], eax
    cvttsd2si eax, qword ptr [rdx + 64]
    mov dword ptr [rsp + 108], eax
    cvttsd2si eax, qword ptr [rdx + 72]
    mov dword ptr [rsp + 112], eax
    cvttsd2si eax, qword ptr [rdx + 80]
    mov dword ptr [rsp + 116], eax
    cmp dword ptr [rsp + 104], 0 ; 0016
    je general_0026 ; 001A
    fld qword ptr [rcx + 0] ; 0020
    fabs  ; 0022
    fstp qword ptr [rcx + 0] ; 0024
general_0026:
    cmp dword ptr [rsp + 108], 0 ; 0026
    je general_0036 ; 002A
    fld qword ptr [rcx + 8] ; 0030
    fabs  ; 0032
    fstp qword ptr [rcx + 8] ; 0034
general_0036:
    cmp dword ptr [rsp + 112], 0 ; 0036
    je general_0046 ; 003A
    fld qword ptr [rcx + 16] ; 0040
    fabs  ; 0042
    fstp qword ptr [rcx + 16] ; 0044
general_0046:
    cmp dword ptr [rsp + 88], 0 ; 0046
    jne general_0057 ; 004A
    fld qword ptr [rcx + 0] ; 004C
    fld qword ptr [rcx + 8] ; 004E
    jmp general_0316 ; 0050
general_0057:
    cmp dword ptr [rsp + 88], 1 ; 0057
    jne general_0068 ; 005B
    fld qword ptr [rcx + 0] ; 005D
    fld qword ptr [rcx + 16] ; 005F
    jmp general_0316 ; 0061
general_0068:
    cmp dword ptr [rsp + 88], 2 ; 0068
    jne general_0079 ; 006C
    fld qword ptr [rcx + 8] ; 006E
    fld qword ptr [rcx + 0] ; 0070
    jmp general_0316 ; 0072
general_0079:
    cmp dword ptr [rsp + 88], 3 ; 0079
    jne general_008a ; 007D
    fld qword ptr [rcx + 8] ; 007F
    fld qword ptr [rcx + 16] ; 0081
    jmp general_0316 ; 0083
general_008a:
    cmp dword ptr [rsp + 88], 4 ; 008A
    jne general_009b ; 008E
    fld qword ptr [rcx + 16] ; 0090
    fld qword ptr [rcx + 0] ; 0092
    jmp general_0316 ; 0094
general_009b:
    cmp dword ptr [rsp + 88], 5 ; 009B
    jne general_00ac ; 009F
    fld qword ptr [rcx + 16] ; 00A1
    fld qword ptr [rcx + 8] ; 00A3
    jmp general_0316 ; 00A5
general_00ac:
    cmp dword ptr [rsp + 88], 6 ; 00AC
    jne general_00c7 ; 00B0
    fld qword ptr [rcx + 0] ; 00B2
    fld qword ptr [rcx + 16] ; 00B4
    fmul st(0), st(0) ; 00B6
    fld qword ptr [rcx + 8] ; 00B8
    fmul st(0), st(0) ; 00BA
    faddp st(1), st(0) ; 00BC
    fsqrt  ; 00BE
    jmp general_0316 ; 00C0
general_00c7:
    cmp dword ptr [rsp + 88], 7 ; 00C7
    jne general_00e2 ; 00CB
    fld qword ptr [rcx + 8] ; 00CD
    fld qword ptr [rcx + 0] ; 00CF
    fmul st(0), st(0) ; 00D1
    fld qword ptr [rcx + 16] ; 00D3
    fmul st(0), st(0) ; 00D5
    faddp st(1), st(0) ; 00D7
    fsqrt  ; 00D9
    jmp general_0316 ; 00DB
general_00e2:
    cmp dword ptr [rsp + 88], 8 ; 00E2
    jne general_00fd ; 00E6
    fld qword ptr [rcx + 16] ; 00E8
    fld qword ptr [rcx + 0] ; 00EA
    fmul st(0), st(0) ; 00EC
    fld qword ptr [rcx + 8] ; 00EE
    fmul st(0), st(0) ; 00F0
    faddp st(1), st(0) ; 00F2
    fsqrt  ; 00F4
    jmp general_0316 ; 00F6
general_00fd:
    cmp dword ptr [rsp + 88], 9 ; 00FD
    jne general_0118 ; 0101
    fld qword ptr [rcx + 0] ; 0103
    fld qword ptr [rcx + 16] ; 0105
    fmul st(0), st(0) ; 0107
    fld qword ptr [rcx + 8] ; 0109
    fmul st(0), st(0) ; 010B
    faddp st(1), st(0) ; 010D
    fsqrt  ; 010F
    fxch st(1) ; 0111
    jmp general_0316 ; 0113
general_0118:
    cmp dword ptr [rsp + 88], 10 ; 0118
    jne general_0133 ; 011C
    fld qword ptr [rcx + 8] ; 011E
    fld qword ptr [rcx + 0] ; 0120
    fmul st(0), st(0) ; 0122
    fld qword ptr [rcx + 16] ; 0124
    fmul st(0), st(0) ; 0126
    faddp st(1), st(0) ; 0128
    fsqrt  ; 012A
    fxch st(1) ; 012C
    jmp general_0316 ; 012E
general_0133:
    cmp dword ptr [rsp + 88], 11 ; 0133
    jne general_014e ; 0137
    fld qword ptr [rcx + 16] ; 0139
    fld qword ptr [rcx + 0] ; 013B
    fmul st(0), st(0) ; 013D
    fld qword ptr [rcx + 8] ; 013F
    fmul st(0), st(0) ; 0141
    faddp st(1), st(0) ; 0143
    fsqrt  ; 0145
    fxch st(1) ; 0147
    jmp general_0316 ; 0149
general_014e:
    cmp dword ptr [rsp + 88], 12 ; 014E
    jne general_0174 ; 0152
    fld qword ptr [rcx + 0] ; 0154
    fld qword ptr [rcx + 16] ; 0156
    fabs  ; 0158
    fld qword ptr [rcx + 8] ; 015A
    fabs  ; 015C
    fcom st(1) ; 015E
    fnstsw ax ; 0160
    and ah, 65 ; 0162
    ja general_016b ; 0165
    fstp st(1) ; 0167
    jmp general_016d ; 0169
general_016b:
    fstp st(0) ; 016B
general_016d:
    jmp general_0316 ; 016D
general_0174:
    cmp dword ptr [rsp + 88], 13 ; 0174
    jne general_019a ; 0178
    fld qword ptr [rcx + 8] ; 017A
    fld qword ptr [rcx + 16] ; 017C
    fabs  ; 017E
    fld qword ptr [rcx + 0] ; 0180
    fabs  ; 0182
    fcom st(1) ; 0184
    fnstsw ax ; 0186
    and ah, 65 ; 0188
    ja general_0191 ; 018B
    fstp st(1) ; 018D
    jmp general_0193 ; 018F
general_0191:
    fstp st(0) ; 0191
general_0193:
    jmp general_0316 ; 0193
general_019a:
    cmp dword ptr [rsp + 88], 14 ; 019A
    jne general_01c0 ; 019E
    fld qword ptr [rcx + 16] ; 01A0
    fld qword ptr [rcx + 0] ; 01A2
    fabs  ; 01A4
    fld qword ptr [rcx + 8] ; 01A6
    fabs  ; 01A8
    fcom st(1) ; 01AA
    fnstsw ax ; 01AC
    and ah, 65 ; 01AE
    ja general_01b7 ; 01B1
    fstp st(1) ; 01B3
    jmp general_01b9 ; 01B5
general_01b7:
    fstp st(0) ; 01B7
general_01b9:
    jmp general_0316 ; 01B9
general_01c0:
    cmp dword ptr [rsp + 88], 15 ; 01C0
    jne general_01e6 ; 01C4
    fld qword ptr [rcx + 0] ; 01C6
    fld qword ptr [rcx + 16] ; 01C8
    fabs  ; 01CA
    fld qword ptr [rcx + 8] ; 01CC
    fabs  ; 01CE
    fcom st(1) ; 01D0
    fnstsw ax ; 01D2
    and ah, 65 ; 01D4
    ja general_01dd ; 01D7
    fstp st(1) ; 01D9
    jmp general_01df ; 01DB
general_01dd:
    fstp st(0) ; 01DD
general_01df:
    fxch st(1) ; 01DF
    jmp general_0316 ; 01E1
general_01e6:
    cmp dword ptr [rsp + 88], 16 ; 01E6
    jne general_020c ; 01EA
    fld qword ptr [rcx + 8] ; 01EC
    fld qword ptr [rcx + 16] ; 01EE
    fabs  ; 01F0
    fld qword ptr [rcx + 0] ; 01F2
    fabs  ; 01F4
    fcom st(1) ; 01F6
    fnstsw ax ; 01F8
    and ah, 65 ; 01FA
    ja general_0203 ; 01FD
    fstp st(1) ; 01FF
    jmp general_0205 ; 0201
general_0203:
    fstp st(0) ; 0203
general_0205:
    fxch st(1) ; 0205
    jmp general_0316 ; 0207
general_020c:
    cmp dword ptr [rsp + 88], 17 ; 020C
    jne general_0232 ; 0210
    fld qword ptr [rcx + 16] ; 0212
    fld qword ptr [rcx + 0] ; 0214
    fabs  ; 0216
    fld qword ptr [rcx + 8] ; 0218
    fabs  ; 021A
    fcom st(1) ; 021C
    fnstsw ax ; 021E
    and ah, 65 ; 0220
    ja general_0229 ; 0223
    fstp st(1) ; 0225
    jmp general_022b ; 0227
general_0229:
    fstp st(0) ; 0229
general_022b:
    fxch st(1) ; 022B
    jmp general_0316 ; 022D
general_0232:
    cmp dword ptr [rsp + 88], 18 ; 0232
    jne general_0258 ; 0236
    fld qword ptr [rcx + 0] ; 0238
    fld qword ptr [rcx + 16] ; 023A
    fabs  ; 023C
    fld qword ptr [rcx + 8] ; 023E
    fabs  ; 0240
    fcom st(1) ; 0242
    fnstsw ax ; 0244
    and ah, 65 ; 0246
    jb general_024f ; 0249
    fstp st(1) ; 024B
    jmp general_0251 ; 024D
general_024f:
    fstp st(0) ; 024F
general_0251:
    jmp general_0316 ; 0251
general_0258:
    cmp dword ptr [rsp + 88], 19 ; 0258
    jne general_027e ; 025C
    fld qword ptr [rcx + 8] ; 025E
    fld qword ptr [rcx + 16] ; 0260
    fabs  ; 0262
    fld qword ptr [rcx + 0] ; 0264
    fabs  ; 0266
    fcom st(1) ; 0268
    fnstsw ax ; 026A
    and ah, 65 ; 026C
    jb general_0275 ; 026F
    fstp st(1) ; 0271
    jmp general_0277 ; 0273
general_0275:
    fstp st(0) ; 0275
general_0277:
    jmp general_0316 ; 0277
general_027e:
    cmp dword ptr [rsp + 88], 20 ; 027E
    jne general_02a4 ; 0282
    fld qword ptr [rcx + 16] ; 0284
    fld qword ptr [rcx + 0] ; 0286
    fabs  ; 0288
    fld qword ptr [rcx + 8] ; 028A
    fabs  ; 028C
    fcom st(1) ; 028E
    fnstsw ax ; 0290
    and ah, 65 ; 0292
    jb general_029b ; 0295
    fstp st(1) ; 0297
    jmp general_029d ; 0299
general_029b:
    fstp st(0) ; 029B
general_029d:
    jmp general_0316 ; 029D
general_02a4:
    cmp dword ptr [rsp + 88], 21 ; 02A4
    jne general_02ca ; 02A8
    fld qword ptr [rcx + 0] ; 02AA
    fld qword ptr [rcx + 16] ; 02AC
    fabs  ; 02AE
    fld qword ptr [rcx + 8] ; 02B0
    fabs  ; 02B2
    fcom st(1) ; 02B4
    fnstsw ax ; 02B6
    and ah, 65 ; 02B8
    jb general_02c1 ; 02BB
    fstp st(1) ; 02BD
    jmp general_02c3 ; 02BF
general_02c1:
    fstp st(0) ; 02C1
general_02c3:
    fxch st(1) ; 02C3
    jmp general_0316 ; 02C5
general_02ca:
    cmp dword ptr [rsp + 88], 22 ; 02CA
    jne general_02f0 ; 02CE
    fld qword ptr [rcx + 8] ; 02D0
    fld qword ptr [rcx + 16] ; 02D2
    fabs  ; 02D4
    fld qword ptr [rcx + 0] ; 02D6
    fabs  ; 02D8
    fcom st(1) ; 02DA
    fnstsw ax ; 02DC
    and ah, 65 ; 02DE
    jb general_02e7 ; 02E1
    fstp st(1) ; 02E3
    jmp general_02e9 ; 02E5
general_02e7:
    fstp st(0) ; 02E7
general_02e9:
    fxch st(1) ; 02E9
    jmp general_0316 ; 02EB
general_02f0:
    cmp dword ptr [rsp + 88], 23 ; 02F0
    jne general_0316 ; 02F4
    fld qword ptr [rcx + 16] ; 02F6
    fld qword ptr [rcx + 0] ; 02F8
    fabs  ; 02FA
    fld qword ptr [rcx + 8] ; 02FC
    fabs  ; 02FE
    fcom st(1) ; 0300
    fnstsw ax ; 0302
    and ah, 65 ; 0304
    jb general_030d ; 0307
    fstp st(1) ; 0309
    jmp general_030f ; 030B
general_030d:
    fstp st(0) ; 030D
general_030f:
    fxch st(1) ; 030F
general_0316:
    fld qword ptr [rdx + 0] ; 0316
    fstp qword ptr [rsp + 64] ; 0319
    cmp dword ptr [rsp + 92], 1 ; 031C
    jne general_0324 ; 0320
    jmp general_032a ; 0322
general_0324:
    cmp dword ptr [rsp + 92], 3 ; 0324
    jne general_0341 ; 0328
general_032a:
    fld st(0) ; 032A
    fldz  ; 032C
    fcompp  ; 032E
    fnstsw ax ; 0330
    and ah, 65 ; 0332
    jb general_033f ; 0335
    fld qword ptr [rsp + 64] ; 0337
    fchs  ; 033A
    fstp qword ptr [rsp + 64] ; 033C
general_033f:
    fabs  ; 033F
general_0341:
    fpatan  ; 0343
    cmp dword ptr [rsp + 92], 2 ; 0347
    jne general_034f ; 034B
    jmp general_0355 ; 034D
general_034f:
    cmp dword ptr [rsp + 92], 3 ; 034F
    jne general_0367 ; 0353
general_0355:
    fld qword ptr [rdx + 88] ; 0355
    fadd st(0), st(1) ; 0357
    fabs  ; 0359
    fld qword ptr [rdx + 88] ; 035B
    fsub st(0), st(2) ; 035D
    fabs  ; 035F
    fsubp st(1), st(0) ; 0361
    fsubp st(1), st(0) ; 0363
general_0367:
    fmul qword ptr [rsp + 64] ; 0367
    fsincos  ; 036A
    fstp qword ptr [rsp + 32] ; 036C
    fstp qword ptr [rsp + 40] ; 036F
    cmp dword ptr [rsp + 96], 0 ; 0373
    jne general_0384 ; 0377
    fld qword ptr [rcx + 0] ; 0379
    fld qword ptr [rcx + 8] ; 037B
    jmp general_0643 ; 037D
general_0384:
    cmp dword ptr [rsp + 96], 1 ; 0384
    jne general_0395 ; 0388
    fld qword ptr [rcx + 0] ; 038A
    fld qword ptr [rcx + 16] ; 038C
    jmp general_0643 ; 038E
general_0395:
    cmp dword ptr [rsp + 96], 2 ; 0395
    jne general_03a6 ; 0399
    fld qword ptr [rcx + 8] ; 039B
    fld qword ptr [rcx + 0] ; 039D
    jmp general_0643 ; 039F
general_03a6:
    cmp dword ptr [rsp + 96], 3 ; 03A6
    jne general_03b7 ; 03AA
    fld qword ptr [rcx + 8] ; 03AC
    fld qword ptr [rcx + 16] ; 03AE
    jmp general_0643 ; 03B0
general_03b7:
    cmp dword ptr [rsp + 96], 4 ; 03B7
    jne general_03c8 ; 03BB
    fld qword ptr [rcx + 16] ; 03BD
    fld qword ptr [rcx + 0] ; 03BF
    jmp general_0643 ; 03C1
general_03c8:
    cmp dword ptr [rsp + 96], 5 ; 03C8
    jne general_03d9 ; 03CC
    fld qword ptr [rcx + 16] ; 03CE
    fld qword ptr [rcx + 8] ; 03D0
    jmp general_0643 ; 03D2
general_03d9:
    cmp dword ptr [rsp + 96], 6 ; 03D9
    jne general_03f4 ; 03DD
    fld qword ptr [rcx + 0] ; 03DF
    fld qword ptr [rcx + 16] ; 03E1
    fmul st(0), st(0) ; 03E3
    fld qword ptr [rcx + 8] ; 03E5
    fmul st(0), st(0) ; 03E7
    faddp st(1), st(0) ; 03E9
    fsqrt  ; 03EB
    jmp general_0643 ; 03ED
general_03f4:
    cmp dword ptr [rsp + 96], 7 ; 03F4
    jne general_040f ; 03F8
    fld qword ptr [rcx + 8] ; 03FA
    fld qword ptr [rcx + 0] ; 03FC
    fmul st(0), st(0) ; 03FE
    fld qword ptr [rcx + 16] ; 0400
    fmul st(0), st(0) ; 0402
    faddp st(1), st(0) ; 0404
    fsqrt  ; 0406
    jmp general_0643 ; 0408
general_040f:
    cmp dword ptr [rsp + 96], 8 ; 040F
    jne general_042a ; 0413
    fld qword ptr [rcx + 16] ; 0415
    fld qword ptr [rcx + 0] ; 0417
    fmul st(0), st(0) ; 0419
    fld qword ptr [rcx + 8] ; 041B
    fmul st(0), st(0) ; 041D
    faddp st(1), st(0) ; 041F
    fsqrt  ; 0421
    jmp general_0643 ; 0423
general_042a:
    cmp dword ptr [rsp + 96], 9 ; 042A
    jne general_0445 ; 042E
    fld qword ptr [rcx + 0] ; 0430
    fld qword ptr [rcx + 16] ; 0432
    fmul st(0), st(0) ; 0434
    fld qword ptr [rcx + 8] ; 0436
    fmul st(0), st(0) ; 0438
    faddp st(1), st(0) ; 043A
    fsqrt  ; 043C
    fxch st(1) ; 043E
    jmp general_0643 ; 0440
general_0445:
    cmp dword ptr [rsp + 96], 10 ; 0445
    jne general_0460 ; 0449
    fld qword ptr [rcx + 8] ; 044B
    fld qword ptr [rcx + 0] ; 044D
    fmul st(0), st(0) ; 044F
    fld qword ptr [rcx + 16] ; 0451
    fmul st(0), st(0) ; 0453
    faddp st(1), st(0) ; 0455
    fsqrt  ; 0457
    fxch st(1) ; 0459
    jmp general_0643 ; 045B
general_0460:
    cmp dword ptr [rsp + 96], 11 ; 0460
    jne general_047b ; 0464
    fld qword ptr [rcx + 16] ; 0466
    fld qword ptr [rcx + 0] ; 0468
    fmul st(0), st(0) ; 046A
    fld qword ptr [rcx + 8] ; 046C
    fmul st(0), st(0) ; 046E
    faddp st(1), st(0) ; 0470
    fsqrt  ; 0472
    fxch st(1) ; 0474
    jmp general_0643 ; 0476
general_047b:
    cmp dword ptr [rsp + 96], 12 ; 047B
    jne general_04a1 ; 047F
    fld qword ptr [rcx + 0] ; 0481
    fld qword ptr [rcx + 16] ; 0483
    fabs  ; 0485
    fld qword ptr [rcx + 8] ; 0487
    fabs  ; 0489
    fcom st(1) ; 048B
    fnstsw ax ; 048D
    and ah, 65 ; 048F
    ja general_0498 ; 0492
    fstp st(1) ; 0494
    jmp general_049a ; 0496
general_0498:
    fstp st(0) ; 0498
general_049a:
    jmp general_0643 ; 049A
general_04a1:
    cmp dword ptr [rsp + 96], 13 ; 04A1
    jne general_04c7 ; 04A5
    fld qword ptr [rcx + 8] ; 04A7
    fld qword ptr [rcx + 16] ; 04A9
    fabs  ; 04AB
    fld qword ptr [rcx + 0] ; 04AD
    fabs  ; 04AF
    fcom st(1) ; 04B1
    fnstsw ax ; 04B3
    and ah, 65 ; 04B5
    ja general_04be ; 04B8
    fstp st(1) ; 04BA
    jmp general_04c0 ; 04BC
general_04be:
    fstp st(0) ; 04BE
general_04c0:
    jmp general_0643 ; 04C0
general_04c7:
    cmp dword ptr [rsp + 96], 14 ; 04C7
    jne general_04ed ; 04CB
    fld qword ptr [rcx + 16] ; 04CD
    fld qword ptr [rcx + 0] ; 04CF
    fabs  ; 04D1
    fld qword ptr [rcx + 8] ; 04D3
    fabs  ; 04D5
    fcom st(1) ; 04D7
    fnstsw ax ; 04D9
    and ah, 65 ; 04DB
    ja general_04e4 ; 04DE
    fstp st(1) ; 04E0
    jmp general_04e6 ; 04E2
general_04e4:
    fstp st(0) ; 04E4
general_04e6:
    jmp general_0643 ; 04E6
general_04ed:
    cmp dword ptr [rsp + 96], 15 ; 04ED
    jne general_0513 ; 04F1
    fld qword ptr [rcx + 0] ; 04F3
    fld qword ptr [rcx + 16] ; 04F5
    fabs  ; 04F7
    fld qword ptr [rcx + 8] ; 04F9
    fabs  ; 04FB
    fcom st(1) ; 04FD
    fnstsw ax ; 04FF
    and ah, 65 ; 0501
    ja general_050a ; 0504
    fstp st(1) ; 0506
    jmp general_050c ; 0508
general_050a:
    fstp st(0) ; 050A
general_050c:
    fxch st(1) ; 050C
    jmp general_0643 ; 050E
general_0513:
    cmp dword ptr [rsp + 96], 16 ; 0513
    jne general_0539 ; 0517
    fld qword ptr [rcx + 8] ; 0519
    fld qword ptr [rcx + 16] ; 051B
    fabs  ; 051D
    fld qword ptr [rcx + 0] ; 051F
    fabs  ; 0521
    fcom st(1) ; 0523
    fnstsw ax ; 0525
    and ah, 65 ; 0527
    ja general_0530 ; 052A
    fstp st(1) ; 052C
    jmp general_0532 ; 052E
general_0530:
    fstp st(0) ; 0530
general_0532:
    fxch st(1) ; 0532
    jmp general_0643 ; 0534
general_0539:
    cmp dword ptr [rsp + 96], 17 ; 0539
    jne general_055f ; 053D
    fld qword ptr [rcx + 16] ; 053F
    fld qword ptr [rcx + 0] ; 0541
    fabs  ; 0543
    fld qword ptr [rcx + 8] ; 0545
    fabs  ; 0547
    fcom st(1) ; 0549
    fnstsw ax ; 054B
    and ah, 65 ; 054D
    ja general_0556 ; 0550
    fstp st(1) ; 0552
    jmp general_0558 ; 0554
general_0556:
    fstp st(0) ; 0556
general_0558:
    fxch st(1) ; 0558
    jmp general_0643 ; 055A
general_055f:
    cmp dword ptr [rsp + 96], 18 ; 055F
    jne general_0585 ; 0563
    fld qword ptr [rcx + 0] ; 0565
    fld qword ptr [rcx + 16] ; 0567
    fabs  ; 0569
    fld qword ptr [rcx + 8] ; 056B
    fabs  ; 056D
    fcom st(1) ; 056F
    fnstsw ax ; 0571
    and ah, 65 ; 0573
    jb general_057c ; 0576
    fstp st(1) ; 0578
    jmp general_057e ; 057A
general_057c:
    fstp st(0) ; 057C
general_057e:
    jmp general_0643 ; 057E
general_0585:
    cmp dword ptr [rsp + 96], 19 ; 0585
    jne general_05ab ; 0589
    fld qword ptr [rcx + 8] ; 058B
    fld qword ptr [rcx + 16] ; 058D
    fabs  ; 058F
    fld qword ptr [rcx + 0] ; 0591
    fabs  ; 0593
    fcom st(1) ; 0595
    fnstsw ax ; 0597
    and ah, 65 ; 0599
    jb general_05a2 ; 059C
    fstp st(1) ; 059E
    jmp general_05a4 ; 05A0
general_05a2:
    fstp st(0) ; 05A2
general_05a4:
    jmp general_0643 ; 05A4
general_05ab:
    cmp dword ptr [rsp + 96], 20 ; 05AB
    jne general_05d1 ; 05AF
    fld qword ptr [rcx + 16] ; 05B1
    fld qword ptr [rcx + 0] ; 05B3
    fabs  ; 05B5
    fld qword ptr [rcx + 8] ; 05B7
    fabs  ; 05B9
    fcom st(1) ; 05BB
    fnstsw ax ; 05BD
    and ah, 65 ; 05BF
    jb general_05c8 ; 05C2
    fstp st(1) ; 05C4
    jmp general_05ca ; 05C6
general_05c8:
    fstp st(0) ; 05C8
general_05ca:
    jmp general_0643 ; 05CA
general_05d1:
    cmp dword ptr [rsp + 96], 21 ; 05D1
    jne general_05f7 ; 05D5
    fld qword ptr [rcx + 0] ; 05D7
    fld qword ptr [rcx + 16] ; 05D9
    fabs  ; 05DB
    fld qword ptr [rcx + 8] ; 05DD
    fabs  ; 05DF
    fcom st(1) ; 05E1
    fnstsw ax ; 05E3
    and ah, 65 ; 05E5
    jb general_05ee ; 05E8
    fstp st(1) ; 05EA
    jmp general_05f0 ; 05EC
general_05ee:
    fstp st(0) ; 05EE
general_05f0:
    fxch st(1) ; 05F0
    jmp general_0643 ; 05F2
general_05f7:
    cmp dword ptr [rsp + 96], 22 ; 05F7
    jne general_061d ; 05FB
    fld qword ptr [rcx + 8] ; 05FD
    fld qword ptr [rcx + 16] ; 05FF
    fabs  ; 0601
    fld qword ptr [rcx + 0] ; 0603
    fabs  ; 0605
    fcom st(1) ; 0607
    fnstsw ax ; 0609
    and ah, 65 ; 060B
    jb general_0614 ; 060E
    fstp st(1) ; 0610
    jmp general_0616 ; 0612
general_0614:
    fstp st(0) ; 0614
general_0616:
    fxch st(1) ; 0616
    jmp general_0643 ; 0618
general_061d:
    cmp dword ptr [rsp + 96], 23 ; 061D
    jne general_0643 ; 0621
    fld qword ptr [rcx + 16] ; 0623
    fld qword ptr [rcx + 0] ; 0625
    fabs  ; 0627
    fld qword ptr [rcx + 8] ; 0629
    fabs  ; 062B
    fcom st(1) ; 062D
    fnstsw ax ; 062F
    and ah, 65 ; 0631
    jb general_063a ; 0634
    fstp st(1) ; 0636
    jmp general_063c ; 0638
general_063a:
    fstp st(0) ; 063A
general_063c:
    fxch st(1) ; 063C
general_0643:
    fld qword ptr [rdx + 8] ; 0643
    fstp qword ptr [rsp + 64] ; 0646
    cmp dword ptr [rsp + 100], 1 ; 0649
    jne general_0651 ; 064D
    jmp general_0657 ; 064F
general_0651:
    cmp dword ptr [rsp + 100], 3 ; 0651
    jne general_066e ; 0655
general_0657:
    fld st(0) ; 0657
    fldz  ; 0659
    fcompp  ; 065B
    fnstsw ax ; 065D
    and ah, 65 ; 065F
    jb general_066c ; 0662
    fld qword ptr [rsp + 64] ; 0664
    fchs  ; 0667
    fstp qword ptr [rsp + 64] ; 0669
general_066c:
    fabs  ; 066C
general_066e:
    fpatan  ; 0670
    cmp dword ptr [rsp + 100], 2 ; 0672
    jne general_067a ; 0676
    jmp general_0680 ; 0678
general_067a:
    cmp dword ptr [rsp + 100], 3 ; 067A
    jne general_0692 ; 067E
general_0680:
    fld qword ptr [rdx + 88] ; 0680
    fadd st(0), st(1) ; 0682
    fabs  ; 0684
    fld qword ptr [rdx + 88] ; 0686
    fsub st(0), st(2) ; 0688
    fabs  ; 068A
    fsubp st(1), st(0) ; 068C
    fsubp st(1), st(0) ; 068E
general_0692:
    fmul qword ptr [rsp + 64] ; 0692
    fsincos  ; 0695
    fstp qword ptr [rsp + 16] ; 0697
    fstp qword ptr [rsp + 24] ; 069A
    fld qword ptr [rcx + 0] ; 06A1
    fmul st(0), st(0) ; 06A3
    fld qword ptr [rcx + 8] ; 06A5
    fmul st(0), st(0) ; 06A7
    fld qword ptr [rcx + 16] ; 06A9
    fmul st(0), st(0) ; 06AB
    faddp st(1), st(0) ; 06AD
    faddp st(1), st(0) ; 06AF
    fsqrt  ; 06B1
    fst qword ptr [rsp + 56] ; 06B3
    fld qword ptr [rdx + 0] ; 06B6
    fxch st(1) ; 06B9
    fabs  ; 06BB
    fyl2x  ; 06BD
    fld1  ; 06BF
    fld st(1) ; 06C1
    fprem  ; 06C3
    f2xm1  ; 06C5
    faddp st(1), st(0) ; 06C7
    fscale  ; 06C9
    fxch st(1) ; 06CB
    fstp st(0) ; 06CD
    fstp qword ptr [rsp + 48] ; 06CF
    cmp dword ptr [rsp + 116], 0 ; 06D4
    jne general_0729 ; 06D8
    fld qword ptr [rsp + 32] ; 06DA
    fmul qword ptr [rsp + 16] ; 06DD
    fmul qword ptr [rsp + 48] ; 06E0
    fld qword ptr [rcx + 24] ; 06E3
    cmp dword ptr [rsp + 104], -1 ; 06E6
    jne general_06ee ; 06EA
    fabs  ; 06EC
general_06ee:
    faddp st(1), st(0) ; 06EE
    fstp qword ptr [rcx + 0] ; 06F0
    fld qword ptr [rsp + 40] ; 06F2
    fmul qword ptr [rsp + 16] ; 06F5
    fmul qword ptr [rsp + 48] ; 06F8
    fld qword ptr [rcx + 32] ; 06FB
    cmp dword ptr [rsp + 108], -1 ; 06FE
    jne general_0706 ; 0702
    fabs  ; 0704
general_0706:
    faddp st(1), st(0) ; 0706
    fstp qword ptr [rcx + 8] ; 0708
    fld qword ptr [rdx + 16] ; 070A
    fmul qword ptr [rsp + 24] ; 070D
    fmul qword ptr [rsp + 48] ; 0710
    fld qword ptr [rcx + 40] ; 0713
    cmp dword ptr [rsp + 112], -1 ; 0716
    jne general_071e ; 071A
    fabs  ; 071C
general_071e:
    faddp st(1), st(0) ; 071E
    fstp qword ptr [rcx + 16] ; 0720
    jmp general_07c8 ; 0722
general_0729:
    cmp dword ptr [rsp + 116], 1 ; 0729
    jne general_077e ; 072D
    fld qword ptr [rsp + 32] ; 072F
    fmul qword ptr [rsp + 24] ; 0732
    fmul qword ptr [rsp + 48] ; 0735
    fld qword ptr [rcx + 24] ; 0738
    cmp dword ptr [rsp + 104], -1 ; 073B
    jne general_0743 ; 073F
    fabs  ; 0741
general_0743:
    faddp st(1), st(0) ; 0743
    fstp qword ptr [rcx + 0] ; 0745
    fld qword ptr [rsp + 40] ; 0747
    fmul qword ptr [rsp + 24] ; 074A
    fmul qword ptr [rsp + 48] ; 074D
    fld qword ptr [rcx + 32] ; 0750
    cmp dword ptr [rsp + 108], -1 ; 0753
    jne general_075b ; 0757
    fabs  ; 0759
general_075b:
    faddp st(1), st(0) ; 075B
    fstp qword ptr [rcx + 8] ; 075D
    fld qword ptr [rdx + 16] ; 075F
    fmul qword ptr [rsp + 16] ; 0762
    fmul qword ptr [rsp + 48] ; 0765
    fld qword ptr [rcx + 40] ; 0768
    cmp dword ptr [rsp + 112], -1 ; 076B
    jne general_0773 ; 076F
    fabs  ; 0771
general_0773:
    faddp st(1), st(0) ; 0773
    fstp qword ptr [rcx + 16] ; 0775
    jmp general_07c8 ; 0777
general_077e:
    fld qword ptr [rsp + 32] ; 077E
    fmul qword ptr [rsp + 16] ; 0781
    fmul qword ptr [rsp + 48] ; 0784
    fld qword ptr [rcx + 24] ; 0787
    cmp dword ptr [rsp + 104], -1 ; 078A
    jne general_0792 ; 078E
    fabs  ; 0790
general_0792:
    faddp st(1), st(0) ; 0792
    fstp qword ptr [rcx + 0] ; 0794
    fld qword ptr [rsp + 40] ; 0796
    fmul qword ptr [rsp + 48] ; 0799
    fld qword ptr [rcx + 32] ; 079C
    cmp dword ptr [rsp + 108], -1 ; 079F
    jne general_07a7 ; 07A3
    fabs  ; 07A5
general_07a7:
    faddp st(1), st(0) ; 07A7
    fstp qword ptr [rcx + 8] ; 07A9
    fld qword ptr [rdx + 16] ; 07AB
    fmul qword ptr [rsp + 32] ; 07AE
    fmul qword ptr [rsp + 24] ; 07B1
    fmul qword ptr [rsp + 48] ; 07B4
    fld qword ptr [rcx + 40] ; 07B7
    cmp dword ptr [rsp + 112], -1 ; 07BA
    jne general_07c2 ; 07BE
    fabs  ; 07C0
general_07c2:
    faddp st(1), st(0) ; 07C2
    fstp qword ptr [rcx + 16] ; 07C4
general_07c8:
    fldcw word ptr [rsp]
    add rsp, 136
    ret
GeneralOriginal_x87 ENDP


.code
; _IFS_tess original 0014..030F. Preserve unsigned counter and raw constant reads.
; r8: FormulaWorkCounter. Return eax=0 complete, eax=1 work exhausted.
IFSTessOriginal_x87 PROC
    mov r10, r8
    sub rsp, 120
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 0137Fh
    fldcw word ptr [rsp + 2]
    cvttsd2si eax, qword ptr [rdx + 56]
    mov dword ptr [rsp + 96], eax
    cvttsd2si eax, qword ptr [rdx + 64]
    mov dword ptr [rsp + 100], eax
    mov rax, qword ptr [rdx + 96]
    mov qword ptr [rsp + 104], rax
    fld qword ptr [rcx + 0] ; 0016
    fadd qword ptr [rdx + 0] ; 0018
    fstp qword ptr [rcx + 0] ; 001B
    fld qword ptr [rcx + 8] ; 001D
    fadd qword ptr [rdx + 8] ; 001F
    fstp qword ptr [rcx + 8] ; 0022
    fld qword ptr [rcx + 16] ; 0024
    fadd qword ptr [rdx + 16] ; 0026
    fstp qword ptr [rcx + 16] ; 0029
    fld qword ptr [rcx + 8] ; 002D
    fstp qword ptr [rsp + 80] ; 002F
    fld qword ptr [rcx + 16] ; 0032
    fstp qword ptr [rsp + 72] ; 0034
    fldz  ; 0037
    fistp dword ptr [rsp + 36] ; 0039
    fldz  ; 003C
    fistp dword ptr [rsp + 16] ; 003E
    fld qword ptr [rdx + 48] ; 0041
    fmul qword ptr [rdx + 88] ; 0044
    fsincos  ; 0047
    fstp qword ptr [rsp + 40] ; 0049
    fstp qword ptr [rsp + 48] ; 004C
    fld qword ptr [rcx + 0] ; 0050
    fst qword ptr [rsp + 28] ; 0052
    fabs  ; 0055
    fld qword ptr [rdx + 40] ; 0057
    fcompp  ; 005A
    fnstsw ax ; 005C
    and ah, 65 ; 005E
    ja ifstess_02ee ; 0061
    fld qword ptr [rsp + 80] ; 0069
    fabs  ; 006C
    fld qword ptr [rdx + 40] ; 006E
    fcompp  ; 0071
    fnstsw ax ; 0073
    and ah, 65 ; 0075
    ja ifstess_0095 ; 0078
    fld qword ptr [rsp + 72] ; 0080
    fabs  ; 0083
    fld qword ptr [rdx + 40] ; 0085
    fcompp  ; 0088
    fnstsw ax ; 008A
    and ah, 65 ; 008C
    jbe ifstess_02ee ; 008F
ifstess_0095:
    ; The original body starts with an empty x87 stack. Count only entered
    ; bodies; do not reject a large saved MAXDEPTH before a geometric exit.
    mov eax, dword ptr [r10]
    cmp eax, dword ptr [r10 + 4]
    jae ifstess_work_exhausted
    inc dword ptr [r10]
    cmp dword ptr [rsp + 100], 4 ; 0095
    jl ifstess_0182 ; 0099
    fld qword ptr [rsp + 80] ; 009F
    fabs  ; 00A2
    fld qword ptr [rdx + 40] ; 00A4
    fcompp  ; 00A7
    fnstsw ax ; 00A9
    and ah, 65 ; 00AB
    ja ifstess_00cb ; 00AE
    fld qword ptr [rsp + 72] ; 00B6
    fabs  ; 00B9
    fld qword ptr [rdx + 40] ; 00BB
    fcompp  ; 00BE
    fnstsw ax ; 00C0
    and ah, 65 ; 00C2
    jbe ifstess_0211 ; 00C5
ifstess_00cb:
    fld qword ptr [rsp + 80] ; 00CD
    fabs  ; 00D0
    fst qword ptr [rsp + 88] ; 00D2
    fldz  ; 00D5
    fcompp  ; 00D7
    fnstsw ax ; 00D9
    and ah, 65 ; 00DB
    xor ah, 64 ; 00DE
    je ifstess_00ec ; 00E1
    fld qword ptr [rsp + 88] ; 00E3
    fdiv qword ptr [rsp + 80] ; 00E6
    fstp qword ptr [rsp + 88] ; 00E9
ifstess_00ec:
    fld qword ptr [rsp + 88] ; 00ED
    fmul qword ptr [rdx + 32] ; 00F0
    fchs  ; 00F3
    fadd qword ptr [rsp + 80] ; 00F5
    fstp qword ptr [rsp + 64] ; 00F8
    fld qword ptr [rdx + 32] ; 00FD
    fmul qword ptr [rdx + 72] ; 0100
    fchs  ; 0102
    fadd qword ptr [rsp + 72] ; 0104
    fstp qword ptr [rsp + 56] ; 0107
    fld qword ptr [rsp + 88] ; 010A
    fldz  ; 010D
    fcompp  ; 010F
    fnstsw ax ; 0111
    and ah, 65 ; 0113
    jbe ifstess_014f ; 0116
    fld qword ptr [rsp + 64] ; 011C
    fmul qword ptr [rsp + 40] ; 011F
    fld qword ptr [rsp + 56] ; 0124
    fmul qword ptr [rsp + 48] ; 0127
    fchs  ; 012A
    faddp st(1), st(0) ; 012C
    fstp qword ptr [rsp + 80] ; 012E
    fld qword ptr [rsp + 64] ; 0133
    fmul qword ptr [rsp + 48] ; 0136
    fld qword ptr [rsp + 56] ; 013B
    fmul qword ptr [rsp + 40] ; 013E
    faddp st(1), st(0) ; 0143
    fstp qword ptr [rsp + 72] ; 0145
    jmp ifstess_017b ; 014A
ifstess_014f:
    fld qword ptr [rsp + 64] ; 014F
    fmul qword ptr [rsp + 40] ; 0152
    fld qword ptr [rsp + 56] ; 0157
    fmul qword ptr [rsp + 48] ; 015A
    faddp st(1), st(0) ; 015F
    fstp qword ptr [rsp + 80] ; 0161
    fld qword ptr [rsp + 64] ; 0166
    fmul qword ptr [rsp + 48] ; 0169
    fchs  ; 016C
    fld qword ptr [rsp + 56] ; 016E
    fmul qword ptr [rsp + 40] ; 0171
    faddp st(1), st(0) ; 0176
    fstp qword ptr [rsp + 72] ; 0178
ifstess_017b:
    jmp ifstess_024a ; 017D
ifstess_0182:
    fld1  ; 0182
    fstp qword ptr [rsp + 88] ; 0184
    fld qword ptr [rsp + 80] ; 0189
    fld qword ptr [rsp + 72] ; 018C
    fcompp  ; 018F
    fnstsw ax ; 0191
    and ah, 65 ; 0193
    jbe ifstess_01ae ; 0196
    fld qword ptr [rsp + 80] ; 019C
    fld qword ptr [rdx + 32] ; 019F
    fmul qword ptr [rdx + 80] ; 01A2
    fchs  ; 01A5
    faddp st(1), st(0) ; 01A7
    fstp qword ptr [rsp + 80] ; 01A9
    jmp ifstess_01c6 ; 01AC
ifstess_01ae:
    fld qword ptr [rsp + 72] ; 01AE
    fld qword ptr [rdx + 32] ; 01B1
    fmul qword ptr [rdx + 80] ; 01B4
    fchs  ; 01B7
    faddp st(1), st(0) ; 01B9
    fstp qword ptr [rsp + 72] ; 01BB
    fld qword ptr [rsp + 88] ; 01BE
    fchs  ; 01C1
    fstp qword ptr [rsp + 88] ; 01C3
ifstess_01c6:
    fld qword ptr [rsp + 80] ; 01C6
    fmul qword ptr [rsp + 48] ; 01C9
    fchs  ; 01CC
    fld qword ptr [rsp + 72] ; 01CE
    fmul qword ptr [rsp + 40] ; 01D1
    fchs  ; 01D4
    faddp st(1), st(0) ; 01D6
    fld qword ptr [rsp + 80] ; 01DA
    fmul qword ptr [rsp + 40] ; 01DD
    fld qword ptr [rsp + 72] ; 01E2
    fmul qword ptr [rsp + 48] ; 01E5
    fchs  ; 01E8
    faddp st(1), st(0) ; 01EA
    fstp qword ptr [rsp + 72] ; 01EC
    fstp qword ptr [rsp + 80] ; 01F1
    fld qword ptr [rsp + 88] ; 01F4
    fldz  ; 01F7
    fcompp  ; 01F9
    fnstsw ax ; 01FB
    and ah, 65 ; 01FD
    ja ifstess_0248 ; 0200
    cmp dword ptr [rsp + 100], 1 ; 0206
    jne ifstess_021e ; 020A
    fld qword ptr [rsp + 80] ; 020C
    fchs  ; 020F
ifstess_0211:
    fstp qword ptr [rsp + 80] ; 0211
    fld qword ptr [rsp + 72] ; 0214
    fchs  ; 0217
    fstp qword ptr [rsp + 72] ; 0219
    jmp ifstess_0248 ; 021C
ifstess_021e:
    cmp dword ptr [rsp + 100], 2 ; 021E
    jne ifstess_0232 ; 0222
    fld qword ptr [rsp + 80] ; 0224
    fld qword ptr [rsp + 72] ; 0227
    fstp qword ptr [rsp + 80] ; 022A
    fstp qword ptr [rsp + 72] ; 022D
    jmp ifstess_0248 ; 0230
ifstess_0232:
    cmp dword ptr [rsp + 100], 3 ; 0232
    jne ifstess_0248 ; 0236
    fld qword ptr [rsp + 80] ; 0238
    fld qword ptr [rsp + 72] ; 023B
    fchs  ; 023E
    fstp qword ptr [rsp + 80] ; 0240
    fchs  ; 0243
    fstp qword ptr [rsp + 72] ; 0245
ifstess_0248:
ifstess_024a:
    fld qword ptr [rsp + 80] ; 024A
    fdiv qword ptr [rdx + 80] ; 024D
    fstp qword ptr [rsp + 64] ; 0250
    fld qword ptr [rsp + 72] ; 0253
    fdiv qword ptr [rdx + 80] ; 0256
    fstp qword ptr [rsp + 56] ; 0259
    fld qword ptr [rsp + 64] ; 025E
    fabs  ; 0261
    fld qword ptr [rdx + 40] ; 0263
    fcompp  ; 0266
    fnstsw ax ; 0268
    and ah, 65 ; 026A
    ja ifstess_029b ; 026D
    fld qword ptr [rsp + 56] ; 026F
    fabs  ; 0272
    fld qword ptr [rdx + 40] ; 0274
    fcompp  ; 0277
    fnstsw ax ; 0279
    and ah, 65 ; 027B
    ja ifstess_029b ; 027E
    fld qword ptr [rsp + 64] ; 0280
    fstp qword ptr [rcx + 8] ; 0283
    fld qword ptr [rsp + 56] ; 0285
    fstp qword ptr [rcx + 16] ; 0288
    fld qword ptr [rsp + 28] ; 028A
    fdiv qword ptr [rdx + 80] ; 028D
    fstp qword ptr [rsp + 28] ; 0290
    fld1  ; 0293
    fistp dword ptr [rsp + 16] ; 0295
    jmp ifstess_02c8 ; 0298
ifstess_029b:
    fld qword ptr [rsp + 64] ; 029B
    fstp qword ptr [rsp + 80] ; 029E
    fld qword ptr [rsp + 56] ; 02A1
    fstp qword ptr [rsp + 72] ; 02A4
    mov eax, dword ptr [rsp + 36] ; 02A7
    cmp dword ptr [rsp + 108], eax ; 02AA
    jbe ifstess_02c8 ; 02AD
    cmp dword ptr [rsp + 96], eax ; 02AF
    jbe ifstess_02c8 ; 02B3
    add dword ptr [rsp + 36], 1 ; 02B5
    fld qword ptr [rsp + 28] ; 02B9
    fdiv qword ptr [rdx + 80] ; 02BC
    fstp qword ptr [rsp + 28] ; 02BF
    jmp ifstess_0095 ; 02C2
ifstess_02c8:
    cmp dword ptr [rsp + 16], 1 ; 02C8
    je ifstess_02ee ; 02CC
    fld qword ptr [rsp + 104] ; 02D2
    fmul qword ptr [rcx + 8] ; 02D5
    fstp qword ptr [rcx + 8] ; 02D7
    fld qword ptr [rsp + 104] ; 02D9
    fmul qword ptr [rcx + 16] ; 02DC
    fstp qword ptr [rcx + 16] ; 02DE
    fld qword ptr [rsp + 104] ; 02E0
    fmul qword ptr [rcx + 0] ; 02E3
    fstp qword ptr [rcx + 0] ; 02E6
    jmp ifstess_0311 ; 02E9
ifstess_02ee:
    fld qword ptr [rsp + 28] ; 02EE
    fmul qword ptr [rdx + 24] ; 02F1
    fsub qword ptr [rdx + 0] ; 02F4
    fstp qword ptr [rcx + 0] ; 02F7
    fld qword ptr [rcx + 8] ; 02F9
    fmul qword ptr [rdx + 24] ; 02FB
    fsub qword ptr [rdx + 8] ; 02FE
    fstp qword ptr [rcx + 8] ; 0301
    fld qword ptr [rcx + 16] ; 0303
    fmul qword ptr [rdx + 24] ; 0305
    fsub qword ptr [rdx + 16] ; 0308
    fstp qword ptr [rcx + 16] ; 030B
ifstess_0311:
    xor eax, eax
    jmp ifstess_work_done
ifstess_work_exhausted:
    mov eax, 1
ifstess_work_done:
    fldcw word ptr [rsp]
    add rsp, 120
    ret
IFSTessOriginal_x87 ENDP

; Calc.pas CalcZposAndRough: preserve sign-bit clipping, x87 conversion,
; signed 32-bit subtraction/clamps and the signed LOW byte of iSmNormals.
; rcx: uint32 out[3] = packed, before-clamp depth, roughness bits
; rdx: double in[4] = ZZ, Zcorr, ZcMul, promoted Single roughness
; r8d: iSmNormals
EncodeDepthRough_x87 PROC
    sub rsp, 24
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    mov dword ptr [rsp + 8], 255
    fld1
    test byte ptr [rdx + 7], 128
    jns depth_rough_positive
    fldz
    jmp depth_rough_calc
depth_rough_positive:
    fld qword ptr [rdx]
depth_rough_calc:
    fmul qword ptr [rdx + 8]
    fadd st(0), st(1)
    fsqrt
    fsubrp st(1), st(0)
    fmul qword ptr [rdx + 16]
    fistp dword ptr [rsp + 4]
    mov eax, 8388352
    sub eax, dword ptr [rsp + 4]
    mov dword ptr [rcx + 4], eax
    test eax, eax
    jnl depth_rough_upper
    xor eax, eax
depth_rough_upper:
    cmp eax, 8388352
    jle depth_rough_pack
    mov eax, 8388352
depth_rough_pack:
    shl eax, 8
    mov dword ptr [rcx + 8], 0
    cmp r8b, 0
    jle depth_rough_done
    fld qword ptr [rdx + 24]
    fimul dword ptr [rsp + 8]
    fistp dword ptr [rsp + 4]
    mov r9d, dword ptr [rsp + 4]
    mov dword ptr [rcx + 8], r9d
    or eax, r9d
depth_rough_done:
    mov dword ptr [rcx], eax
    fldcw word ptr [rsp]
    add rsp, 24
    ret
EncodeDepthRough_x87 ENDP


; CantorIFS original 000D..00E1, including matrix order and x87 flags.
; rcx: state[4] = x,y,z,de; rdx: expanded parameter doubles[22].
CantorIFSOriginal_x87 PROC
    sub rsp, 88
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    movsd xmm0, qword ptr [rdx + 32]
    cvtsd2ss xmm0, xmm0
    movss dword ptr [rsp + 8], xmm0
    movsd xmm0, qword ptr [rdx + 40]
    cvtsd2ss xmm0, xmm0
    movss dword ptr [rsp + 12], xmm0
    movsd xmm0, qword ptr [rdx + 48]
    cvtsd2ss xmm0, xmm0
    movss dword ptr [rsp + 16], xmm0
    movsd xmm0, qword ptr [rdx + 56]
    cvtsd2ss xmm0, xmm0
    movss dword ptr [rsp + 20], xmm0
    movsd xmm0, qword ptr [rdx + 64]
    cvtsd2ss xmm0, xmm0
    movss dword ptr [rsp + 24], xmm0
    movsd xmm0, qword ptr [rdx + 72]
    cvtsd2ss xmm0, xmm0
    movss dword ptr [rsp + 28], xmm0
    movsd xmm0, qword ptr [rdx + 80]
    cvtsd2ss xmm0, xmm0
    movss dword ptr [rsp + 32], xmm0
    movsd xmm0, qword ptr [rdx + 88]
    cvtsd2ss xmm0, xmm0
    movss dword ptr [rsp + 36], xmm0
    movsd xmm0, qword ptr [rdx + 96]
    cvtsd2ss xmm0, xmm0
    movss dword ptr [rsp + 40], xmm0
    movsd xmm0, qword ptr [rdx + 104]
    cvtsd2ss xmm0, xmm0
    movss dword ptr [rsp + 44], xmm0
    movsd xmm0, qword ptr [rdx + 112]
    cvtsd2ss xmm0, xmm0
    movss dword ptr [rsp + 48], xmm0
    movsd xmm0, qword ptr [rdx + 120]
    cvtsd2ss xmm0, xmm0
    movss dword ptr [rsp + 52], xmm0
    movsd xmm0, qword ptr [rdx + 128]
    cvtsd2ss xmm0, xmm0
    movss dword ptr [rsp + 56], xmm0
    movsd xmm0, qword ptr [rdx + 136]
    cvtsd2ss xmm0, xmm0
    movss dword ptr [rsp + 60], xmm0
    movsd xmm0, qword ptr [rdx + 144]
    cvtsd2ss xmm0, xmm0
    movss dword ptr [rsp + 64], xmm0
    movsd xmm0, qword ptr [rdx + 152]
    cvtsd2ss xmm0, xmm0
    movss dword ptr [rsp + 68], xmm0
    movsd xmm0, qword ptr [rdx + 160]
    cvtsd2ss xmm0, xmm0
    movss dword ptr [rsp + 72], xmm0
    movsd xmm0, qword ptr [rdx + 168]
    cvtsd2ss xmm0, xmm0
    movss dword ptr [rsp + 76], xmm0
    fld qword ptr [rcx + 16] ; 000D
    fabs  ; 000F
    fld qword ptr [rcx + 8] ; 0011
    fabs  ; 0013
    fld qword ptr [rcx + 0] ; 0015
    fabs  ; 0017
    fld st(0) ; 0019
    fmul dword ptr [rsp + 32] ; 001B
    fld st(2) ; 001E
    fmul dword ptr [rsp + 36] ; 0020
    faddp st(1), st(0) ; 0023
    fld st(3) ; 0025
    fmul dword ptr [rsp + 40] ; 0027
    faddp st(1), st(0) ; 002A
    fld st(1) ; 002C
    fmul dword ptr [rsp + 8] ; 002E
    fld st(3) ; 0031
    fmul dword ptr [rsp + 12] ; 0033
    faddp st(1), st(0) ; 0036
    fld st(4) ; 0038
    fmul dword ptr [rsp + 16] ; 003A
    faddp st(1), st(0) ; 003D
    fxch st(2) ; 003F
    fmul dword ptr [rsp + 20] ; 0041
    fxch st(3) ; 0044
    fmul dword ptr [rsp + 24] ; 0046
    faddp st(3), st(0) ; 0049
    fxch st(3) ; 004B
    fmul dword ptr [rsp + 28] ; 004D
    faddp st(2), st(0) ; 0050
    fchs  ; 0052
    fcom st(1) ; 0054
    fnstsw ax ; 0056
    shr ah, 1 ; 0058
    jb cantor_005e ; 005A
    fxch st(1) ; 005C
cantor_005e:
    fcom st(2) ; 005E
    fnstsw ax ; 0060
    shr ah, 1 ; 0062
    jb cantor_0068 ; 0064
    fxch st(2) ; 0066
cantor_0068:
    fchs  ; 0068
    fxch st(1) ; 006A
    fchs  ; 006C
    fcom st(2) ; 006E
    fnstsw ax ; 0070
    shr ah, 1 ; 0072
    jb cantor_0078 ; 0074
    fxch st(2) ; 0076
cantor_0078:
    fchs  ; 0078
    fxch st(1) ; 007A
    fld st(1) ; 007C
    fmul dword ptr [rsp + 68] ; 007E
    fld st(1) ; 0081
    fmul dword ptr [rsp + 72] ; 0083
    faddp st(1), st(0) ; 0086
    fld st(3) ; 0088
    fmul dword ptr [rsp + 76] ; 008A
    faddp st(1), st(0) ; 008D
    fld st(2) ; 008F
    fmul dword ptr [rsp + 44] ; 0091
    fld st(2) ; 0094
    fmul dword ptr [rsp + 48] ; 0096
    faddp st(1), st(0) ; 0099
    fld st(4) ; 009B
    fmul dword ptr [rsp + 52] ; 009D
    faddp st(1), st(0) ; 00A0
    fxch st(3) ; 00A2
    fmul dword ptr [rsp + 56] ; 00A4
    fxch st(2) ; 00A7
    fmul dword ptr [rsp + 60] ; 00A9
    faddp st(2), st(0) ; 00AC
    fxch st(3) ; 00AE
    fmul dword ptr [rsp + 64] ; 00B0
    faddp st(1), st(0) ; 00B3
    fld qword ptr [rdx + 0] ; 00B5
    fld qword ptr [rcx + 24] ; 00B8
    fmul st(0), st(1) ; 00BB
    fstp qword ptr [rcx + 24] ; 00BD
    fmul st(3), st(0) ; 00C0
    fmul st(2), st(0) ; 00C2
    fmul st(1), st(0) ; 00C4
    fld1  ; 00C6
    fsubp st(1), st(0) ; 00C8
    fld st(0) ; 00CA
    fld st(0) ; 00CC
    fmul qword ptr [rdx + 16] ; 00CE
    fsubp st(4), st(0) ; 00D1
    fmul qword ptr [rdx + 8] ; 00D3
    fsubp st(2), st(0) ; 00D6
    fmul qword ptr [rdx + 24] ; 00D8
    fsubp st(3), st(0) ; 00DB
    fstp qword ptr [rcx + 0] ; 00DD
    fstp qword ptr [rcx + 8] ; 00DF
    fstp qword ptr [rcx + 16] ; 00E1
    fldcw word ptr [rsp]
    add rsp, 88
    ret
CantorIFSOriginal_x87 ENDP


; Original Amazing Surf 0023..00F9. Original Double stores are significant.
; rcx: x,y,z,de,VaryScale; rdx: expanded params; r8d: cylinder selector.
; Caller implements original signed bFirstIt initialization before this block.
AmazingSurfOriginal_x87 PROC
    sub rsp, 56
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    movsd xmm0, qword ptr [rdx + 32]
    cvtsd2ss xmm0, xmm0
    movss dword ptr [rsp + 8], xmm0
    movsd xmm0, qword ptr [rdx + 40]
    cvtsd2ss xmm0, xmm0
    movss dword ptr [rsp + 12], xmm0
    movsd xmm0, qword ptr [rdx + 48]
    cvtsd2ss xmm0, xmm0
    movss dword ptr [rsp + 16], xmm0
    movsd xmm0, qword ptr [rdx + 56]
    cvtsd2ss xmm0, xmm0
    movss dword ptr [rsp + 20], xmm0
    movsd xmm0, qword ptr [rdx + 64]
    cvtsd2ss xmm0, xmm0
    movss dword ptr [rsp + 24], xmm0
    movsd xmm0, qword ptr [rdx + 72]
    cvtsd2ss xmm0, xmm0
    movss dword ptr [rsp + 28], xmm0
    movsd xmm0, qword ptr [rdx + 80]
    cvtsd2ss xmm0, xmm0
    movss dword ptr [rsp + 32], xmm0
    movsd xmm0, qword ptr [rdx + 88]
    cvtsd2ss xmm0, xmm0
    movss dword ptr [rsp + 36], xmm0
    movsd xmm0, qword ptr [rdx + 96]
    cvtsd2ss xmm0, xmm0
    movss dword ptr [rsp + 40], xmm0
    fld qword ptr [rcx + 32] ; 0023
    fld1 ; 0026
    fld st(1) ; 0028
    fabs ; 002A
    fsubrp st(1), st(0) ; 002C
    fmul qword ptr [rdx + 104] ; 002E
    faddp st(1), st(0) ; 0031
    fstp qword ptr [rcx + 32] ; 0033
    fld qword ptr [rdx + 24] ; 0036
    fld qword ptr [rcx] ; 0039
    fld st(0) ; 003B
    fsub st(0), st(2) ; 003D
    fabs ; 003F
    fadd st(0), st(1) ; 0041
    fxch st(1) ; 0043
    fadd st(0), st(2) ; 0045
    fabs ; 0047
    fsubrp st(1), st(0) ; 0049
    fld qword ptr [rcx + 8] ; 004B
    fld st(0) ; 004D
    fsub st(0), st(3) ; 004F
    fabs ; 0051
    fadd st(0), st(1) ; 0053
    fxch st(1) ; 0055
    fadd st(0), st(3) ; 0057
    fabs ; 0059
    fsubrp st(1), st(0) ; 005B
    fst qword ptr [rcx + 8] ; 005D
    fmul st(0), st(0) ; 005F
    fxch st(1) ; 0061
    fst qword ptr [rcx] ; 0063
    fmul st(0), st(0) ; 0065
    faddp st(1), st(0) ; 0067
    fstp st(1) ; 0069
    cmp r8d, 0 ; 006B
    jne asurf_0077 ; 006F
    fld qword ptr [rcx + 16] ; 0071
    fmul st(0), st(0) ; 0073
    faddp st(1), st(0) ; 0075
asurf_0077:
    fcom qword ptr [rdx + 16] ; 0077
    fnstsw ax ; 007A
    shr ah, 1 ; 007C
    jae asurf_0087 ; 007E
    fstp st(0) ; 0080
    fld qword ptr [rdx + 8] ; 0082
    jmp asurf_009d ; 0085
asurf_0087:
    fld1 ; 0087
    fcom st(1) ; 0089
    fnstsw ax ; 008B
    shr ah, 1 ; 008D
    jb asurf_0098 ; 008F
    fstp st(0) ; 0091
    fdivr qword ptr [rcx + 32] ; 0093
    jmp asurf_009d ; 0096
asurf_0098:
    fcompp ; 0098
    fld qword ptr [rcx + 32] ; 009A
asurf_009d:
    fld qword ptr [rcx + 24] ; 009D
    fmul st(0), st(1) ; 00A0
    fstp qword ptr [rcx + 24] ; 00A2
    fld qword ptr [rcx + 16] ; 00A5
    fmul st(0), st(1) ; 00A7
    fadd qword ptr [rdx + 136] ; 00A9
    fld qword ptr [rcx + 8] ; 00AC
    fmul st(0), st(2) ; 00AE
    fadd qword ptr [rdx + 128] ; 00B0
    fld qword ptr [rcx] ; 00B3
    fmul st(0), st(3) ; 00B5
    fadd qword ptr [rdx + 120] ; 00B7
    fld st(0) ; 00BA
    fmul dword ptr [rsp + 32] ; 00BC
    fld st(2) ; 00BF
    fmul dword ptr [rsp + 36] ; 00C1
    faddp st(1), st(0) ; 00C4
    fld st(3) ; 00C6
    fmul dword ptr [rsp + 40] ; 00C8
    faddp st(1), st(0) ; 00CB
    fld st(1) ; 00CD
    fmul dword ptr [rsp + 8] ; 00CF
    fld st(3) ; 00D2
    fmul dword ptr [rsp + 12] ; 00D4
    faddp st(1), st(0) ; 00D7
    fld st(4) ; 00D9
    fmul dword ptr [rsp + 16] ; 00DB
    faddp st(1), st(0) ; 00DE
    fxch st(2) ; 00E0
    fmul dword ptr [rsp + 20] ; 00E2
    fxch st(3) ; 00E5
    fmul dword ptr [rsp + 24] ; 00E7
    faddp st(3), st(0) ; 00EA
    fxch st(3) ; 00EC
    fmul dword ptr [rsp + 28] ; 00EE
    faddp st(2), st(0) ; 00F1
    fstp qword ptr [rcx] ; 00F3
    fstp qword ptr [rcx + 8] ; 00F5
    fstp qword ptr [rcx + 16] ; 00F7
    fstp st(0) ; 00F9
    fldcw word ptr [rsp]
    add rsp, 56
    ret
AmazingSurfOriginal_x87 ENDP


; HeightMapIFS original 0004..0090, preserving x87 order and Double stores.
; rcx: xyz,DE,Rold,Dfree1,u,v,blue,rotatedZ; rdx: expanded parameters.
HeightMapPrepare_x87 PROC
    sub rsp, 24
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    mov rax, 03FE0000000000000h
    mov qword ptr [rsp + 8], rax
    fld qword ptr [rcx + 16] ; 0004
    fld qword ptr [rcx + 8] ; 0007
    fld qword ptr [rcx] ; 000A
    fld st(0) ; 000D
    fmul qword ptr [rdx + 104] ; 000F
    fld st(2) ; 0012
    fmul qword ptr [rdx + 112] ; 0014
    faddp st(1), st(0) ; 0017
    fld st(3) ; 0019
    fmul qword ptr [rdx + 120] ; 001B
    faddp st(1), st(0) ; 001E
    fld st(1) ; 0020
    fmul qword ptr [rdx + 56] ; 0022
    fld st(3) ; 0025
    fmul qword ptr [rdx + 64] ; 0027
    faddp st(1), st(0) ; 002A
    fld st(4) ; 002C
    fmul qword ptr [rdx + 72] ; 002E
    faddp st(1), st(0) ; 0031
    fxch st(2) ; 0033
    fmul qword ptr [rdx + 80] ; 0035
    fxch st(3) ; 0038
    fmul qword ptr [rdx + 88] ; 003A
    faddp st(3), st(0) ; 003D
    fxch st(3) ; 003F
    fmul qword ptr [rdx + 96] ; 0041
    faddp st(2), st(0) ; 0044
    fld st(2) ; 0046
    fadd qword ptr [rdx + 24] ; 0048
    fld qword ptr [rdx + 48] ; 004B
    fmul qword ptr [rsp + 8] ; 004E
    fsubp st(1), st(0) ; 0051
    fabs ; 0053
    fld qword ptr [rdx + 48] ; 0055
    fabs ; 0058
    fadd qword ptr [rcx + 32] ; 005A
    fcompp ; 0060
    fnstsw ax ; 0062
    shr ah, 1 ; 0064
    jae heightmap_sample ; 0066
    fcompp ; 0068
    fld qword ptr [rsp + 8] ; 006A
    fmul qword ptr [rdx + 48] ; 006D
    fsub qword ptr [rdx + 24] ; 0070
    fsubrp st(1), st(0) ; 0073
    fstp qword ptr [rcx + 24] ; 0075
    xor eax, eax
    jmp heightmap_prepare_done
heightmap_sample:
    fmul qword ptr [rdx + 32] ; 007D
    fsub qword ptr [rdx + 8] ; 0080
    fstp qword ptr [rcx + 48] ; 0083
    fmul qword ptr [rdx + 40] ; 0086
    fsub qword ptr [rdx + 16] ; 0089
    fstp qword ptr [rcx + 56] ; 008C
    fstp qword ptr [rcx + 72] ; 0090
    mov eax, 1
heightmap_prepare_done:
    fldcw word ptr [rsp]
    add rsp, 24
    ret
HeightMapPrepare_x87 ENDP

; Original 00A7..00C6. Channel 3 reads the saved rotated Z (not blue).
HeightMapFinish_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    fld qword ptr [rcx + r8*8 + 48]
    fmul qword ptr [rdx + 48]
    fsub qword ptr [rdx + 24]
    fsub qword ptr [rcx + 72]
    fstp qword ptr [rcx + 24]
    fld qword ptr [rcx + r9*8 + 48]
    fadd qword ptr [rdx + 144]
    fmul qword ptr [rdx + 152]
    fstp qword ptr [rcx + 40]
    fldcw word ptr [rsp]
    add rsp, 8
    ret
HeightMapFinish_x87 ENDP


; Original ABoxSmoothFold power/box/ball paths 0016..0120.
; rcx: xyz,w,VaryScale; rdx: params. Signed first-state init is in caller.
ABoxSmoothFoldOriginal_x87 PROC
    sub rsp, 24
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    mov rax, 03FE0000000000000h
    mov qword ptr [rsp + 8], rax
    mov r10, rcx
    mov r11, rdx
    lea rax, [r10]
    jmp aboxsf_0061
aboxsf_0016:
    shr eax, 1 ; 0016
    je aboxsf_0030 ; 0018
    jae aboxsf_0020 ; 001A
    fld st(0) ; 001C
    jmp aboxsf_0024 ; 001E
aboxsf_0020:
    fld1 ; 0020
    fxch st(1) ; 0022
aboxsf_0024:
    fmul st(0), st(0) ; 0024
    shr eax, 1 ; 0026
    jae aboxsf_0024 ; 0028
    fmul st(1), st(0) ; 002A
    jne aboxsf_0024 ; 002C
    fstp st(0) ; 002E
aboxsf_0030:
    ret ; 0030
aboxsf_0031:
    fld qword ptr [rax] ; 0031
    fabs ; 0033
    push rax ; 0035
    cvttsd2si eax, qword ptr [r11 + 40] ; 0036
    fld st(0) ; 0039
    call aboxsf_0016 ; 003B
    pop rax ; 0040
    fmul qword ptr [r11 + 48] ; 0041
    fxch st(1) ; 0044
    fld qword ptr [r11 + 24] ; 0046
    fadd st(0), st(0) ; 0049
    fsub st(0), st(1) ; 004B
    fmul st(0), st(2) ; 004D
    faddp st(1), st(0) ; 004F
    fld1 ; 0051
    faddp st(2), st(0) ; 0053
    fdivrp st(1), st(0) ; 0055
    test byte ptr [rax + 7], 80h ; 0057
    jns aboxsf_005f ; 005B
    fchs ; 005D
aboxsf_005f:
    ret ; 005F
    nop ; 0060
aboxsf_0061:
    call aboxsf_0031 ; 0061
    lea rax, [r10 + 8] ; 0066
    call aboxsf_0031 ; 0068
    lea rax, [r10 + 16] ; 006D
    call aboxsf_0031 ; 006F
aboxsf_0083:
    fld qword ptr [r10 + 32] ; 0083
    fld1 ; 0086
    fld st(1) ; 0088
    fabs ; 008A
    fsubrp st(1), st(0) ; 008C
    fmul qword ptr [r11 + 32] ; 008E
    faddp st(1), st(0) ; 0091
    fstp qword ptr [r10 + 32] ; 0093
    fld qword ptr [r11 + 16] ; 0096
    fcomp qword ptr [r11 + 72] ; 0099
    wait ; 009B
    fnstsw ax ; 009C
    shr ah, 1 ; 009E
    jb aboxsf_00a6 ; 00A0
    fld1 ; 00A2
    jmp aboxsf_0100 ; 00A4
aboxsf_00a6:
    fld st(2) ; 00A6
    fmul st(0), st(0) ; 00A8
    fld st(2) ; 00AA
    fmul st(0), st(0) ; 00AC
    faddp st(1), st(0) ; 00AE
    fld st(1) ; 00B0
    fmul st(0), st(0) ; 00B2
    faddp st(1), st(0) ; 00B4
    fld1 ; 00B6
    fadd qword ptr [r11 + 16] ; 00B8
    fmul qword ptr [rsp + 8] ; 00BB
    fld1 ; 00BE
    fsub qword ptr [r11 + 16] ; 00C0
    fmul qword ptr [rsp + 8] ; 00C3
    fxch st(2) ; 00C6
    fsub st(0), st(1) ; 00C8
    ftst ; 00CA
    wait ; 00CC
    fnstsw ax ; 00CD
    push rax ; 00CF
    fabs ; 00D0
    fdiv st(0), st(2) ; 00D2
    fld st(0) ; 00D4
    cvttsd2si eax, qword ptr [r11 + 56] ; 00D6
    shr eax, 1 ; 00D9
    jae aboxsf_00e1 ; 00DB
    fsqrt ; 00DD
    rcl eax, 1 ; 00DF
aboxsf_00e1:
    call aboxsf_0016 ; 00E1
    fmul qword ptr [r11 + 64] ; 00E6
    fld1 ; 00E9
    fadd st(0), st(1) ; 00EB
    fxch st(2) ; 00ED
    faddp st(1), st(0) ; 00EF
    fdivrp st(1), st(0) ; 00F1
    pop rax ; 00F3
    fmulp st(2), st(0) ; 00F4
    fxch st(1) ; 00F6
    shr ah, 1 ; 00F8
    jae aboxsf_00fe ; 00FA
    fchs ; 00FC
aboxsf_00fe:
    faddp st(1), st(0) ; 00FE
aboxsf_0100:
    fld qword ptr [r10 + 32] ; 0100
    fdivrp st(1), st(0) ; 0103
    fld qword ptr [r10 + 24] ; 0105
    fmul st(0), st(1) ; 0108
    fstp qword ptr [r10 + 24] ; 010A
    fmul st(3), st(0) ; 010D
    fmul st(2), st(0) ; 010F
    fmulp st(1), st(0) ; 0111
    fadd qword ptr [r11 + 96] ; 0113
    fstp qword ptr [r10 + 16] ; 0116
    fadd qword ptr [r11 + 88] ; 0118
    fstp qword ptr [r10 + 8] ; 011B
    fadd qword ptr [r11 + 80] ; 011D
    fstp qword ptr [r10] ; 0120
    fldcw word ptr [rsp]
    add rsp, 24
    ret
ABoxSmoothFoldOriginal_x87 ENDP


; Original _SphereFolding1 000E..0042, retaining stored Rout and x87 branches.
; rcx: xyz,Rout; rdx: four expanded numeric folding values. No pointer call.
SphereFolding1Original_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    fld qword ptr [rcx + 24] ; 000E
    fcom qword ptr [rdx] ; 0011
    fnstsw ax ; 0014
    shr ah, 1 ; 0016
    jb spherefold_001f ; 0018
    fsubr qword ptr [rdx + 8] ; 001A
    jmp spherefold_002b ; 001D
spherefold_001f:
    fcom qword ptr [rdx + 16] ; 001F
    fnstsw ax ; 0022
    shr ah, 1 ; 0024
    jae spherefold_0042 ; 0026
    fsubr qword ptr [rdx + 24] ; 0028
spherefold_002b:
    fdiv qword ptr [rcx + 24] ; 002B
    fld qword ptr [rcx + 16] ; 002E
    fld qword ptr [rcx + 8] ; 0030
    fld qword ptr [rcx] ; 0032
    fmul st(0), st(3) ; 0034
    fstp qword ptr [rcx] ; 0036
    fmul st(0), st(2) ; 0038
    fstp qword ptr [rcx + 8] ; 003A
    fmul st(0), st(1) ; 003C
    fstp qword ptr [rcx + 16] ; 0040
spherefold_0042:
    fstp st(0) ; 0042
    fldcw word ptr [rsp]
    add rsp, 8
    ret
SphereFolding1Original_x87 ENDP


; Original Aexion1 001A..00D8. rcx: XYZW,J1..J4,C_add; edx: bFirstIt.
; Preserve the Double stores after initialization and x87 intermediates.
Aexion1Original_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    test edx, edx
    jne aexion_main
    fld qword ptr [rcx + 64] ; 001A
    fld qword ptr [rcx + 16] ; 001D
    fld qword ptr [rcx + 8] ; 001F
    fld qword ptr [rcx] ; 0021
    fld st(0) ; 0023
    fadd st(0), st(2) ; 0025
    fadd st(0), st(3) ; 0027
    fabs ; 0029
    fadd st(0), st(4) ; 002B
    fstp qword ptr [rcx] ; 002D
    fld st(2) ; 002F
    fsub st(0), st(1) ; 0031
    fsub st(0), st(2) ; 0033
    fabs ; 0035
    fadd st(0), st(4) ; 0037
    fstp qword ptr [rcx + 8] ; 0039
    fld st(1) ; 003B
    fsub st(0), st(1) ; 003D
    fsub st(0), st(3) ; 003F
    fabs ; 0041
    fadd st(0), st(4) ; 0043
    fstp qword ptr [rcx + 16] ; 0045
    fsubrp st(1), st(0) ; 0047
    fsubrp st(1), st(0) ; 0049
    fabs ; 004B
    fadd st(0), st(1) ; 004D
    fstp qword ptr [rcx + 24] ; 004F
    fld qword ptr [rcx + 48] ; 0052
    fld qword ptr [rcx + 40] ; 0055
    fld qword ptr [rcx + 32] ; 0058
    fld st(0) ; 005B
    fadd st(0), st(2) ; 005D
    fadd st(0), st(3) ; 005F
    fabs ; 0061
    fadd st(0), st(4) ; 0063
    fstp qword ptr [rcx + 32] ; 0065
    fld st(2) ; 0068
    fsub st(0), st(1) ; 006A
    fsub st(0), st(2) ; 006C
    fabs ; 006E
    fadd st(0), st(4) ; 0070
    fstp qword ptr [rcx + 40] ; 0072
    fld st(1) ; 0075
    fsub st(0), st(1) ; 0077
    fsub st(0), st(3) ; 0079
    fabs ; 007B
    fadd st(0), st(4) ; 007D
    fstp qword ptr [rcx + 48] ; 007F
    fsubrp st(1), st(0) ; 0082
    fsubrp st(1), st(0) ; 0084
    fabs ; 0086
    faddp st(1), st(0) ; 0088
    fstp qword ptr [rcx + 56] ; 008A
aexion_main:
    fld qword ptr [rcx + 24] ; 008D
    fld qword ptr [rcx + 16] ; 0090
    fld qword ptr [rcx + 8] ; 0092
    fld qword ptr [rcx] ; 0094
    fld st(2) ; 0096
    fmul st(0), st(4) ; 0098
    fadd st(0), st(0) ; 009A
    fld st(1) ; 009C
    fmul st(0), st(3) ; 009E
    fadd st(0), st(0) ; 00A0
    fld st(2) ; 00A2
    fmul st(0), st(0) ; 00A4
    fld st(4) ; 00A6
    fmul st(0), st(0) ; 00A8
    fsubp st(1), st(0) ; 00AA
    fld st(0) ; 00AC
    fadd st(0), st(3) ; 00AE
    fadd qword ptr [rcx + 32] ; 00B0
    fstp qword ptr [rcx] ; 00B3
    fsubp st(2), st(0) ; 00B5
    fxch st(1) ; 00B7
    fadd qword ptr [rcx + 40] ; 00B9
    fstp qword ptr [rcx + 8] ; 00BC
    fxch st(3) ; 00BE
    fmul st(0), st(0) ; 00C0
    fxch st(4) ; 00C2
    fmul st(0), st(0) ; 00C4
    fsubp st(4), st(0) ; 00C6
    fcompp ; 00C8
    fld st(1) ; 00CA
    fadd st(0), st(1) ; 00CC
    fadd qword ptr [rcx + 48] ; 00CE
    fstp qword ptr [rcx + 16] ; 00D1
    fsubrp st(1), st(0) ; 00D3
    fadd qword ptr [rcx + 56] ; 00D5
    fstp qword ptr [rcx + 24] ; 00D8
    fldcw word ptr [rsp]
    add rsp, 8
    ret
Aexion1Original_x87 ENDP


; Original MengerHyper, including unordered comparisons. rcx: XYZW; rdx: params; r8d: Fold again.
MengerHyperOriginal_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    fld qword ptr [rcx + 16] ; 000D
    fabs ; 000F
    fld qword ptr [rcx + 8] ; 0011
    fabs ; 0013
    fld qword ptr [rcx] ; 0015
    fabs ; 0017
    fld st(0) ; 0019
    fmul st(0), st(0) ; 001B
    fld st(2) ; 001D
    fmul st(0), st(0) ; 001F
    faddp st(1), st(0) ; 0021
    fld st(3) ; 0023
    fmul st(0), st(0) ; 0025
    faddp st(1), st(0) ; 0027
    nop ; 0029
    fld qword ptr [rdx + 56] ; 002A
    fmul qword ptr [rdx + 72] ; 002D
    fcomp st(1) ; 0030
    fnstsw ax ; 0032
    shr ah, 1 ; 0034
    jae mengerhyper_0045 ; 0036
    nop ; 0038
    nop ; 0039
    nop ; 003A
    nop ; 003B
    fstp st(0) ; 003C
    fld1 ; 003E
    jmp mengerhyper_0061 ; 0040
    nop ; 0042
    nop ; 0043
    nop ; 0044
mengerhyper_0045:
    fld qword ptr [rdx + 64] ; 0045
    fmul qword ptr [rdx + 72] ; 0048
    fcomp st(1) ; 004B
    fnstsw ax ; 004D
    shr ah, 1 ; 004F
    jbe mengerhyper_0059 ; 0051
    call mengerhyper_006f ; 0053
    nop ; 0058
mengerhyper_0059:
    fld qword ptr [rdx + 56] ; 0059
    fmul qword ptr [rdx + 72] ; 005C
    fdivrp st(1), st(0) ; 005F
mengerhyper_0061:
    fxch st(1) ; 0061
    fmul st(0), st(1) ; 0063
    fxch st(2) ; 0065
    fmul st(0), st(1) ; 0067
    fxch st(3) ; 0069
    fmul st(0), st(1) ; 006B
    jmp mengerhyper_0078 ; 006D
mengerhyper_006f:
    fstp st(0) ; 006F
    fld qword ptr [rdx + 64] ; 0071
    fmul qword ptr [rdx + 72] ; 0074
    ret ; 0077
mengerhyper_0078:
    fstp st(1) ; 0078
    fcom st(1) ; 007A
    fnstsw ax ; 007C
    shr ah, 1 ; 007E
    jae mengerhyper_0084 ; 0080
    fxch st(1) ; 0082
mengerhyper_0084:
    fcom st(2) ; 0084
    fnstsw ax ; 0086
    shr ah, 1 ; 0088
    jae mengerhyper_008e ; 008A
    fxch st(2) ; 008C
mengerhyper_008e:
    fxch st(1) ; 008E
    fcom st(2) ; 0090
    fnstsw ax ; 0092
    shr ah, 1 ; 0094
    jae mengerhyper_009a ; 0096
    fxch st(2) ; 0098
mengerhyper_009a:
    nop ; 009A
    fadd qword ptr [rdx + 40] ; 009B
    fxch st(1) ; 009E
    fadd qword ptr [rdx + 32] ; 00A0
    fxch st(2) ; 00A3
    fadd qword ptr [rdx + 48] ; 00A5
    fxch st(2) ; 00A8
    fxch st(1) ; 00AA
    fld st(0) ; 00AC
    fmul st(0), st(0) ; 00AE
    fld st(2) ; 00B0
    fmul st(0), st(0) ; 00B2
    faddp st(1), st(0) ; 00B4
    fld st(3) ; 00B6
    fmul st(0), st(0) ; 00B8
    faddp st(1), st(0) ; 00BA
    nop ; 00BC
    fcom qword ptr [rdx + 56] ; 00BD
    fnstsw ax ; 00C0
    shr ah, 1 ; 00C2
    ja mengerhyper_0174 ; 00C4
    fcom qword ptr [rdx + 64] ; 00CA
    fnstsw ax ; 00CD
    shr ah, 1 ; 00CF
    ja mengerhyper_00dc ; 00D1
    nop ; 00D3
    nop ; 00D4
    nop ; 00D5
    nop ; 00D6
    jmp mengerhyper_017e ; 00D7
mengerhyper_00dc:
    fld qword ptr [rdx + 56] ; 00DC
    fdivrp st(1), st(0) ; 00DF
mengerhyper_00e1:
    fxch st(1) ; 00E1
    fmul st(0), st(1) ; 00E3
    fsub qword ptr [rdx + 40] ; 00E5
    fxch st(2) ; 00E8
    fmul st(0), st(1) ; 00EA
    fsub qword ptr [rdx + 32] ; 00EC
    fxch st(3) ; 00EF
    fmul st(0), st(1) ; 00F1
    fsub qword ptr [rdx + 48] ; 00F3
    test r8d, r8d ; Fold again
    je mengerhyper_018b ; 00FA
    fxch st(2) ; 0100
    fxch st(1) ; 0102
    fxch st(3) ; 0104
    fxch st(1) ; 0106
    nop ; 0108
    fcom st(1) ; 0109
    fnstsw ax ; 010B
    shr ah, 1 ; 010D
    jae mengerhyper_0113 ; 010F
    fxch st(1) ; 0111
mengerhyper_0113:
    fcom st(2) ; 0113
    fnstsw ax ; 0115
    shr ah, 1 ; 0117
    jae mengerhyper_011d ; 0119
    fxch st(2) ; 011B
mengerhyper_011d:
    fxch st(1) ; 011D
    fcom st(2) ; 011F
    fnstsw ax ; 0121
    shr ah, 1 ; 0123
    jae mengerhyper_0129 ; 0125
    fxch st(2) ; 0127
mengerhyper_0129:
    nop ; 0129
    fxch st(1) ; 012A
    fxch st(2) ; 012C
    fxch st(3) ; 012E
mengerhyper_0130:
    fstp st(0) ; 0130
    fld qword ptr [rdx + 0] ; 0132
    fld st(0) ; 0135
    fmul qword ptr [rcx + 24] ; 0137
    fstp qword ptr [rcx + 24] ; 013A
    nop ; 013D
    nop ; 013E
    fmul st(2), st(0) ; 013F
    fmul st(1), st(0) ; 0141
    fld st(0) ; 0143
    fld1 ; 0145
    fsubp st(1), st(0) ; 0147
    fld st(0) ; 0149
    fld st(0) ; 014B
    fmul qword ptr [rdx + 8] ; 014D
    fsubp st(5), st(0) ; 0150
    fmul qword ptr [rdx + 16] ; 0152
    fsubp st(3), st(0) ; 0155
    fdiv st(0), st(1) ; 0157
    fmul qword ptr [rdx + 24] ; 0159
    fsub st(4), st(0) ; 015C
    fxch st(4) ; 015E
    fabs ; 0160
    fsubp st(4), st(0) ; 0162
    fmulp st(3), st(0) ; 0164
    fstp qword ptr [rcx + 8] ; 0166
    fstp qword ptr [rcx] ; 0168
    fstp qword ptr [rcx + 16] ; 016A
    jmp MengerHyperOriginal_x87_done
mengerhyper_0174:
    fstp st(0) ; 0174
    fld1 ; 0176
    jmp mengerhyper_00e1 ; 0178
    nop ; 017D
mengerhyper_017e:
    fstp st(0) ; 017E
    nop ; 0180
    nop ; 0181
    nop ; 0182
    fld qword ptr [rdx + 64] ; 0183
    jmp mengerhyper_00dc ; 0186
mengerhyper_018b:
    fxch st(3) ; 018B
    fxch st(2) ; 018D
    fxch st(1) ; 018F
    jmp mengerhyper_0130 ; 0191
MengerHyperOriginal_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
MengerHyperOriginal_x87 ENDP


; Original CommaIFS 00A9..0125. rcx: signed local XYZ, radius scratch, DE, constant; rdx: params; r8d: Apply.
CommaIFSDistance_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
commaifs_00a9:
    fld qword ptr [rcx] ; 00A9
    fmul st(0), st(0) ; 00AC
    fld qword ptr [rcx + 8] ; 00AE
    fmul st(0), st(0) ; 00B2
    faddp st(1), st(0) ; 00B4
    fsqrt ; 00B6
    fld qword ptr [rcx + 8] ; 00B8
    fld qword ptr [rcx] ; 00BC
    fpatan ; 00BF
    fmul qword ptr [rcx + 40] ; 00C1
    fld st(0) ; 00C3
    fmul st(0), st(0) ; 00C5
    fld st(0) ; 00C7
    fmul st(0), st(2) ; 00C9
    fchs ; 00CB
    faddp st(1), st(0) ; 00CD
    faddp st(1), st(0) ; 00CF
    fld qword ptr [rdx] ; 00D1
    fmulp st(1), st(0) ; 00D4
    fadd qword ptr [rdx + 40] ; 00D6
    fstp qword ptr [rcx + 24] ; 00D9
    fsub qword ptr [rdx + 56] ; 00DD
    fmul st(0), st(0) ; 00E0
    fld qword ptr [rcx + 16] ; 00E2
    fmul st(0), st(0) ; 00E6
    faddp st(1), st(0) ; 00E8
    fsqrt ; 00EA
    fsub qword ptr [rcx + 24] ; 00EC
    fld qword ptr [rcx] ; 00F0
    fadd qword ptr [rdx + 56] ; 00F3
    fmul st(0), st(0) ; 00F6
    fld qword ptr [rcx + 8] ; 00F8
    fmul st(0), st(0) ; 00FC
    faddp st(1), st(0) ; 00FE
    fld qword ptr [rcx + 16] ; 0100
    fmul st(0), st(0) ; 0104
    faddp st(1), st(0) ; 0106
    fsqrt ; 0108
    fsub qword ptr [rdx + 64] ; 010A
    fcom st(1) ; 010D
    wait ; 010F
    fnstsw ax ; 0110
    shr ah, 1 ; 0112
    jae commaifs_011a ; 0114
    fstp st(1) ; 0116
    jmp commaifs_011c ; 0118
commaifs_011a:
    fstp st(0) ; 011A
commaifs_011c:
    test r8d, r8d ; Apply scale/add
    jne commaifs_0125 ; 0120
    fdiv qword ptr [rdx + 8] ; 0122
commaifs_0125:
    fstp qword ptr [rcx + 32] ; 0125
CommaIFSDistance_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
CommaIFSDistance_x87 ENDP


; Original _JuliaSets 000B..0050; rcx: saved point -> XYZ,J1..J3; rdx: separation,factor.
JuliaSetsOriginal_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    fld qword ptr [rdx] ; 000B
    fld1 ; 000E
    fdiv st(0), st(1) ; 0010
    fld qword ptr [rcx + 8] ; 0012
    fmul st(0), st(1) ; 0015
    fld qword ptr [rcx] ; 0017
    fmul st(0), st(2) ; 0019
    fld qword ptr [rcx + 16] ; 001B
    fmulp st(3), st(0) ; 001E
    fld st(0) ; 0020
    frndint ; 0022
    fld st(2) ; 0024
    frndint ; 0026
    fld st(4) ; 0028
    frndint ; 002A
    fsub st(5), st(0) ; 002C
    fmul qword ptr [rdx + 8] ; 002E
    fstp qword ptr [rcx + 40] ; 0031
    fsub st(3), st(0) ; 0034
    fmul qword ptr [rdx + 8] ; 0036
    fstp qword ptr [rcx + 32] ; 0039
    fsub st(1), st(0) ; 003C
    fmul qword ptr [rdx + 8] ; 003E
    fstp qword ptr [rcx + 24] ; 0041
    fmul st(0), st(3) ; 0044
    fstp qword ptr [rcx] ; 0046
    fmul st(0), st(2) ; 0049
    fstp qword ptr [rcx + 8] ; 004B
    fmulp st(1), st(0) ; 004E
    fstp qword ptr [rcx + 16] ; 0050
JuliaSetsOriginal_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
JuliaSetsOriginal_x87 ENDP


; Original ATetraVS 001C..0367. rcx: XYZW,VaryScale,J1..J3; rdx: params.
; r8: input Single matrix; r9: mode,Tglad,bFirstIt. Internal calls use r10 locals.
ATetraVSOriginal_x87 PROC
    sub rsp, 88
    fnstcw word ptr [rsp + 72]
    mov word ptr [rsp + 74], 137Fh
    fldcw word ptr [rsp + 74]
    lea r10, [rsp + 64]
    jmp atetravs_00cb ; 001C
atetravs_0021:
    fld qword ptr [rcx + 16] ; 0021
    fld qword ptr [rcx + 8] ; 0023
    fld qword ptr [rcx] ; 0025
    fld st(0) ; 0027
    fmul dword ptr [r10 - 52] ; 0029
    fld st(2) ; 002C
    fmul dword ptr [r10 - 56] ; 002E
    faddp st(1), st(0) ; 0031
    fld st(3) ; 0033
    fmul dword ptr [r10 - 60] ; 0035
    faddp st(1), st(0) ; 0038
    fld st(1) ; 003A
    fmul dword ptr [r10 - 28] ; 003C
    fld st(3) ; 003F
    fmul dword ptr [r10 - 32] ; 0041
    faddp st(1), st(0) ; 0044
    fld st(4) ; 0046
    fmul dword ptr [r10 - 36] ; 0048
    faddp st(1), st(0) ; 004B
    fxch st(2) ; 004D
    fmul dword ptr [r10 - 40] ; 004F
    fxch st(3) ; 0052
    fmul dword ptr [r10 - 44] ; 0054
    faddp st(3), st(0) ; 0057
    fxch st(3) ; 0059
    fmul dword ptr [r10 - 48] ; 005B
    faddp st(2), st(0) ; 005E
    cmp dword ptr [r9 + 4], 1 ; 0060
    je atetravs_0072 ; 0064
    fsub qword ptr [rdx + 24] ; 0066
    fabs ; 0069
    fchs ; 006B
    fadd qword ptr [rdx + 24] ; 006D
    jmp atetravs_008a ; 0070
atetravs_0072:
    fld st(0) ; 0072
    fadd qword ptr [rdx + 24] ; 0074
    fabs ; 0077
    fld st(1) ; 0079
    fsub qword ptr [rdx + 24] ; 007B
    fabs ; 007E
    fchs ; 0080
    faddp st(1), st(0) ; 0082
    fsubp st(1), st(0) ; 0084
    fchs ; 0086
    fnop ; 0088
atetravs_008a:
    fld st(0) ; 008A
    fmul dword ptr [r10 - 36] ; 008C
    fld st(2) ; 008F
    fmul dword ptr [r10 - 48] ; 0091
    faddp st(1), st(0) ; 0094
    fld st(3) ; 0096
    fmul dword ptr [r10 - 60] ; 0098
    faddp st(1), st(0) ; 009B
    fld st(1) ; 009D
    fmul dword ptr [r10 - 28] ; 009F
    fld st(3) ; 00A2
    fmul dword ptr [r10 - 40] ; 00A4
    faddp st(1), st(0) ; 00A7
    fld st(4) ; 00A9
    fmul dword ptr [r10 - 52] ; 00AB
    faddp st(1), st(0) ; 00AE
    fxch st(2) ; 00B0
    fmul dword ptr [r10 - 32] ; 00B2
    fxch st(3) ; 00B5
    fmul dword ptr [r10 - 44] ; 00B7
    faddp st(3), st(0) ; 00BA
    fxch st(3) ; 00BC
    fmul dword ptr [r10 - 56] ; 00BE
    faddp st(2), st(0) ; 00C1
    fstp qword ptr [rcx] ; 00C3
    fstp qword ptr [rcx + 8] ; 00C5
    fstp qword ptr [rcx + 16] ; 00C7
    ret ; 00C9
    nop ; 00CA
atetravs_00cb:
    fld dword ptr [r8 + 0] ; 00CB
    fstp dword ptr [r10 - 28] ; 00CE
    fld dword ptr [r8 + 4] ; 00D1
    fstp dword ptr [r10 - 32] ; 00D4
    fld dword ptr [r8 + 8] ; 00D7
    fstp dword ptr [r10 - 36] ; 00DA
    fld dword ptr [r8 + 12] ; 00DD
    fstp dword ptr [r10 - 40] ; 00E0
    fld dword ptr [r8 + 16] ; 00E3
    fstp dword ptr [r10 - 44] ; 00E6
    fld dword ptr [r8 + 20] ; 00E9
    fstp dword ptr [r10 - 48] ; 00EC
    fld dword ptr [r8 + 24] ; 00EF
    fstp dword ptr [r10 - 52] ; 00F2
    fld dword ptr [r8 + 28] ; 00F5
    fstp dword ptr [r10 - 56] ; 00F8
    fld dword ptr [r8 + 32] ; 00FB
    fstp dword ptr [r10 - 60] ; 00FE
    call atetravs_0021 ; 0101
    cmp dword ptr [r9], 0 ; 0106
    jne atetravs_0180 ; 010A
    fld dword ptr [r10 - 36] ; 010C
    fchs ; 010F
    fstp dword ptr [r10 - 36] ; 0111
    fld dword ptr [r10 - 48] ; 0114
    fchs ; 0117
    fstp dword ptr [r10 - 48] ; 0119
    fld dword ptr [r10 - 52] ; 011C
    fchs ; 011F
    fstp dword ptr [r10 - 52] ; 0121
    fld dword ptr [r10 - 56] ; 0124
    fchs ; 0127
    fstp dword ptr [r10 - 56] ; 0129
    call atetravs_0021 ; 012C
    fld dword ptr [r10 - 32] ; 0131
    fchs ; 0134
    fstp dword ptr [r10 - 32] ; 0136
    fld dword ptr [r10 - 48] ; 0139
    fchs ; 013C
    fstp dword ptr [r10 - 48] ; 013E
    fld dword ptr [r10 - 40] ; 0141
    fchs ; 0144
    fstp dword ptr [r10 - 40] ; 0146
    fld dword ptr [r10 - 56] ; 0149
    fchs ; 014C
    fstp dword ptr [r10 - 56] ; 014E
    call atetravs_0021 ; 0151
    fld dword ptr [r10 - 36] ; 0156
    fchs ; 0159
    fstp dword ptr [r10 - 36] ; 015B
    fld dword ptr [r10 - 48] ; 015E
    fchs ; 0161
    fstp dword ptr [r10 - 48] ; 0163
    fld dword ptr [r10 - 52] ; 0166
    fchs ; 0169
    fstp dword ptr [r10 - 52] ; 016B
    fld dword ptr [r10 - 56] ; 016E
    fchs ; 0171
    fstp dword ptr [r10 - 56] ; 0173
    call atetravs_0021 ; 0176
    jmp atetravs_02e8 ; 017B
atetravs_0180:
    cmp dword ptr [r9], 1 ; 0180
    jne atetravs_01fa ; 0184
    fld dword ptr [r10 - 32] ; 0186
    fchs ; 0189
    fstp dword ptr [r10 - 32] ; 018B
    fld dword ptr [r10 - 48] ; 018E
    fchs ; 0191
    fstp dword ptr [r10 - 48] ; 0193
    fld dword ptr [r10 - 40] ; 0196
    fchs ; 0199
    fstp dword ptr [r10 - 40] ; 019B
    fld dword ptr [r10 - 56] ; 019E
    fchs ; 01A1
    fstp dword ptr [r10 - 56] ; 01A3
    call atetravs_0021 ; 01A6
    fld dword ptr [r10 - 36] ; 01AB
    fchs ; 01AE
    fstp dword ptr [r10 - 36] ; 01B0
    fld dword ptr [r10 - 48] ; 01B3
    fchs ; 01B6
    fstp dword ptr [r10 - 48] ; 01B8
    fld dword ptr [r10 - 52] ; 01BB
    fchs ; 01BE
    fstp dword ptr [r10 - 52] ; 01C0
    fld dword ptr [r10 - 56] ; 01C3
    fchs ; 01C6
    fstp dword ptr [r10 - 56] ; 01C8
    call atetravs_0021 ; 01CB
    fld dword ptr [r10 - 32] ; 01D0
    fchs ; 01D3
    fstp dword ptr [r10 - 32] ; 01D5
    fld dword ptr [r10 - 48] ; 01D8
    fchs ; 01DB
    fstp dword ptr [r10 - 48] ; 01DD
    fld dword ptr [r10 - 40] ; 01E0
    fchs ; 01E3
    fstp dword ptr [r10 - 40] ; 01E5
    fld dword ptr [r10 - 56] ; 01E8
    fchs ; 01EB
    fstp dword ptr [r10 - 56] ; 01ED
    call atetravs_0021 ; 01F0
    jmp atetravs_02e8 ; 01F5
atetravs_01fa:
    cmp dword ptr [r9], 2 ; 01FA
    jne atetravs_0271 ; 01FE
    fld dword ptr [r10 - 36] ; 0200
    fchs ; 0203
    fstp dword ptr [r10 - 36] ; 0205
    fld dword ptr [r10 - 32] ; 0208
    fchs ; 020B
    fstp dword ptr [r10 - 32] ; 020D
    fld dword ptr [r10 - 52] ; 0210
    fchs ; 0213
    fstp dword ptr [r10 - 52] ; 0215
    fld dword ptr [r10 - 40] ; 0218
    fchs ; 021B
    fstp dword ptr [r10 - 40] ; 021D
    call atetravs_0021 ; 0220
    fld dword ptr [r10 - 32] ; 0225
    fchs ; 0228
    fstp dword ptr [r10 - 32] ; 022A
    fld dword ptr [r10 - 48] ; 022D
    fchs ; 0230
    fstp dword ptr [r10 - 48] ; 0232
    fld dword ptr [r10 - 40] ; 0235
    fchs ; 0238
    fstp dword ptr [r10 - 40] ; 023A
    fld dword ptr [r10 - 56] ; 023D
    fchs ; 0240
    fstp dword ptr [r10 - 56] ; 0242
    call atetravs_0021 ; 0245
    fld dword ptr [r10 - 36] ; 024A
    fchs ; 024D
    fstp dword ptr [r10 - 36] ; 024F
    fld dword ptr [r10 - 32] ; 0252
    fchs ; 0255
    fstp dword ptr [r10 - 32] ; 0257
    fld dword ptr [r10 - 52] ; 025A
    fchs ; 025D
    fstp dword ptr [r10 - 52] ; 025F
    fld dword ptr [r10 - 40] ; 0262
    fchs ; 0265
    fstp dword ptr [r10 - 40] ; 0267
    call atetravs_0021 ; 026A
    jmp atetravs_02e8 ; 026F
atetravs_0271:
    nop ; 0271
    nop ; 0272
    nop ; 0273
    nop ; 0274
    nop ; 0275
    nop ; 0276
    fld dword ptr [r10 - 32] ; 0277
    fchs ; 027A
    fstp dword ptr [r10 - 32] ; 027C
    fld dword ptr [r10 - 48] ; 027F
    fchs ; 0282
    fstp dword ptr [r10 - 48] ; 0284
    fld dword ptr [r10 - 40] ; 0287
    fchs ; 028A
    fstp dword ptr [r10 - 40] ; 028C
    fld dword ptr [r10 - 56] ; 028F
    fchs ; 0292
    fstp dword ptr [r10 - 56] ; 0294
    call atetravs_0021 ; 0297
    fld dword ptr [r10 - 36] ; 029C
    fchs ; 029F
    fstp dword ptr [r10 - 36] ; 02A1
    fld dword ptr [r10 - 32] ; 02A4
    fchs ; 02A7
    fstp dword ptr [r10 - 32] ; 02A9
    fld dword ptr [r10 - 52] ; 02AC
    fchs ; 02AF
    fstp dword ptr [r10 - 52] ; 02B1
    fld dword ptr [r10 - 40] ; 02B4
    fchs ; 02B7
    fstp dword ptr [r10 - 40] ; 02B9
    call atetravs_0021 ; 02BC
    fld dword ptr [r10 - 32] ; 02C1
    fchs ; 02C4
    fstp dword ptr [r10 - 32] ; 02C6
    fld dword ptr [r10 - 48] ; 02C9
    fchs ; 02CC
    fstp dword ptr [r10 - 48] ; 02CE
    fld dword ptr [r10 - 40] ; 02D1
    fchs ; 02D4
    fstp dword ptr [r10 - 40] ; 02D6
    fld dword ptr [r10 - 56] ; 02D9
    fchs ; 02DC
    fstp dword ptr [r10 - 56] ; 02DE
    call atetravs_0021 ; 02E1
    fnop ; 02E6
atetravs_02e8:
    cmp dword ptr [r9 + 8], 0 ; 02E8
    jg atetravs_02f7 ; 02EC
    inc dword ptr [r9 + 8] ; 02EE
    fld qword ptr [rdx + 0] ; 02F1
    fstp qword ptr [rcx + 32] ; 02F4
atetravs_02f7:
    fld qword ptr [rcx + 32] ; 02F7
    fld1 ; 02FA
    fld st(1) ; 02FC
    fabs ; 02FE
    fsubrp st(1), st(0) ; 0300
    fmul qword ptr [rdx + 32] ; 0302
    faddp st(1), st(0) ; 0305
    fstp qword ptr [rcx + 32] ; 0307
    nop ; 030A
    nop ; 030B
    fld qword ptr [rcx] ; 030C
    fld qword ptr [rcx + 8] ; 030E
    fld qword ptr [rcx + 16] ; 0310
    nop ; 0312
    fld qword ptr [rcx] ; 0313
    fmul st(0), st(0) ; 0315
    fld qword ptr [rcx + 8] ; 0317
    fmul st(0), st(0) ; 0319
    fld qword ptr [rcx + 16] ; 031B
    fmul st(0), st(0) ; 031D
    faddp st(1), st(0) ; 031F
    faddp st(1), st(0) ; 0321
    nop ; 0323
    fcom qword ptr [rdx + 16] ; 0324
    fnstsw ax ; 0327
    shr ah, 1 ; 0329
    jae atetravs_0334 ; 032B
    fstp st(0) ; 032D
    fld qword ptr [rdx + 8] ; 032F
    jmp atetravs_034a ; 0332
atetravs_0334:
    fld1 ; 0334
    fcom st(1) ; 0336
    fnstsw ax ; 0338
    shr ah, 1 ; 033A
    jb atetravs_0345 ; 033C
    fstp st(0) ; 033E
    fdivr qword ptr [rcx + 32] ; 0340
    jmp atetravs_034a ; 0343
atetravs_0345:
    fcompp ; 0345
    fld qword ptr [rcx + 32] ; 0347
atetravs_034a:
    fld qword ptr [rcx + 24] ; 034A
    fmul st(0), st(1) ; 034D
    fstp qword ptr [rcx + 24] ; 034F
    fmul st(3), st(0) ; 0352
    fmul st(2), st(0) ; 0354
    fmulp st(1), st(0) ; 0356
    fadd qword ptr [rcx + 56] ; 0358
    fstp qword ptr [rcx + 16] ; 035B
    fadd qword ptr [rcx + 48] ; 035D
    fstp qword ptr [rcx + 8] ; 0360
    fadd qword ptr [rcx + 40] ; 0362
    fstp qword ptr [rcx] ; 0365
    fnop ; 0367
    fldcw word ptr [rsp + 72]
    add rsp, 88
    ret
ATetraVSOriginal_x87 ENDP


; Original cubetubeIFS 004B..0146. rcx: local YZX, trap, DE; rdx: params; r8d: Apply.
CubetubeIFSDistance_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    fld qword ptr [rcx] ; 004B
    fabs ; 004E
    fld qword ptr [rcx + 8] ; 0050
    fabs ; 0054
    fld qword ptr [rcx + 16] ; 0056
    fabs ; 005A
    fcom st(1) ; 005C
    wait ; 005E
    fnstsw ax ; 005F
    and ah, 041h ; 0061
    je cubetubeifs_0068 ; 0064
    fxch st(1) ; 0066
cubetubeifs_0068:
    fcom st(2) ; 0068
    wait ; 006A
    fnstsw ax ; 006B
    and ah, 041h ; 006D
    je cubetubeifs_0074 ; 0070
    fxch st(2) ; 0072
cubetubeifs_0074:
    fxch st(1) ; 0074
    fcom st(2) ; 0076
    wait ; 0078
    fnstsw ax ; 0079
    and ah, 041h ; 007B
    je cubetubeifs_0082 ; 007E
    fxch st(2) ; 0080
cubetubeifs_0082:
    fstp qword ptr [rcx + 16] ; 0082
    fstp qword ptr [rcx + 8] ; 0086
    fstp qword ptr [rcx] ; 008A
    fld qword ptr [rcx] ; 008D
    fmul qword ptr [rdx + 56] ; 0090
    fld qword ptr [rcx + 16] ; 0093
    fmul qword ptr [rdx + 64] ; 0097
    faddp st(1), st(0) ; 009A
    fld qword ptr [rcx] ; 009C
    fmul qword ptr [rdx + 64] ; 009F
    fld qword ptr [rcx + 16] ; 00A2
    fmul qword ptr [rdx + 56] ; 00A6
    fchs ; 00A9
    faddp st(1), st(0) ; 00AB
    fstp qword ptr [rcx] ; 00AD
    fstp qword ptr [rcx + 16] ; 00B0
    fld qword ptr [rcx] ; 00B4
    fmul qword ptr [rdx + 72] ; 00B7
    fld qword ptr [rcx + 8] ; 00BA
    fmul qword ptr [rdx + 80] ; 00BE
    faddp st(1), st(0) ; 00C1
    fld qword ptr [rcx] ; 00C3
    fmul qword ptr [rdx + 80] ; 00C6
    fld qword ptr [rcx + 8] ; 00C9
    fmul qword ptr [rdx + 72] ; 00CD
    fchs ; 00D0
    faddp st(1), st(0) ; 00D2
    fstp qword ptr [rcx] ; 00D4
    fstp qword ptr [rcx + 8] ; 00D7
    fld qword ptr [rcx + 8] ; 00DB
    fsub qword ptr [rdx + 8] ; 00DF
    fst qword ptr [rcx + 8] ; 00E2
    fmul st(0), st(0) ; 00E6
    fld qword ptr [rcx + 16] ; 00E8
    fsub qword ptr [rdx + 16] ; 00EC
    fst qword ptr [rcx + 16] ; 00EF
    fmul st(0), st(0) ; 00F3
    faddp st(1), st(0) ; 00F5
    fsqrt ; 00F7
    fsub qword ptr [rdx + 0] ; 00F9
    fld qword ptr [rcx] ; 00FC
    fabs ; 00FF
    fsub qword ptr [rdx + 96] ; 0101
    fmul st(0), st(0) ; 0104
    fld qword ptr [rcx + 8] ; 0106
    fmul st(0), st(0) ; 010A
    faddp st(1), st(0) ; 010C
    fld qword ptr [rcx + 16] ; 010E
    fmul st(0), st(0) ; 0112
    faddp st(1), st(0) ; 0114
    fsqrt ; 0116
    fsub qword ptr [rdx + 104] ; 0118
    fcom st(1) ; 011B
    wait ; 011D
    fnstsw ax ; 011E
    and ah, 041h ; 0120
    je cubetubeifs_012f ; 0123
    fstp st(1) ; 0125
    fld qword ptr [rdx + 120] ; 0127
    jmp cubetubeifs_0137 ; 012D
cubetubeifs_012f:
    fstp st(0) ; 012F
    fld qword ptr [rdx + 112] ; 0131
cubetubeifs_0137:
    fstp qword ptr [rcx + 24] ; 0137
    test r8d, r8d ; Apply scale/add
    jne cubetubeifs_0146 ; 0141
    fdiv qword ptr [rdx + 24] ; 0143
cubetubeifs_0146:
    fstp qword ptr [rcx + 32] ; 0146
CubetubeIFSDistance_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
CubetubeIFSDistance_x87 ENDP


; Original _MapTranslate component arithmetic. rcx: original XYZ, sample RGB; rdx: expanded Single parameters.
MapTranslateFinish_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    fld qword ptr [rcx + 24]
    fadd qword ptr [rdx + 8]
    fmul qword ptr [rdx + 32]
    fadd qword ptr [rcx + 0]
    fstp qword ptr [rcx + 0]
    fld qword ptr [rcx + 32]
    fadd qword ptr [rdx + 16]
    fmul qword ptr [rdx + 40]
    fadd qword ptr [rcx + 8]
    fstp qword ptr [rcx + 8]
    fld qword ptr [rcx + 40]
    fadd qword ptr [rdx + 24]
    fmul qword ptr [rdx + 48]
    fadd qword ptr [rcx + 16]
    fstp qword ptr [rcx + 16]
MapTranslateFinish_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
MapTranslateFinish_x87 ENDP


; Original _AmazingBox 000D..008F, signed DE multiplier and independent Scale.
AmazingBoxTransform_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    fld1 ; 000D
    fld qword ptr [rcx] ; 000F
    fld st(0) ; 0011
    fsub st(0), st(2) ; 0013
    fabs ; 0015
    fadd st(0), st(1) ; 0017
    fxch st(1) ; 0019
    fadd st(0), st(2) ; 001B
    fabs ; 001D
    fsubrp st(1), st(0) ; 001F
    fld qword ptr [rcx + 8] ; 0021
    fld st(0) ; 0023
    fsub st(0), st(3) ; 0025
    fabs ; 0027
    fadd st(0), st(1) ; 0029
    fxch st(1) ; 002B
    fadd st(0), st(3) ; 002D
    fabs ; 002F
    fsubrp st(1), st(0) ; 0031
    fld qword ptr [rcx + 16] ; 0033
    fld st(0) ; 0035
    fsub st(0), st(4) ; 0037
    fabs ; 0039
    fadd st(0), st(1) ; 003B
    fxch st(1) ; 003D
    fadd st(0), st(4) ; 003F
    fabs ; 0041
    fsubrp st(1), st(0) ; 0043
    fld st(0) ; 0045
    fmul st(0), st(1) ; 0047
    fld st(2) ; 0049
    fmul st(0), st(3) ; 004B
    faddp st(1), st(0) ; 004D
    fld st(3) ; 004F
    fmul st(0), st(4) ; 0051
    faddp st(1), st(0) ; 0053
    fcom qword ptr [rdx + 8] ; 0055
    fnstsw ax ; 0058
    shr ah, 1 ; 005A
    jae amazingboxtransform_0065 ; 005C
    fstp st(0) ; 005E
    fld qword ptr [rdx] ; 0060
    jmp amazingboxtransform_007b ; 0063
amazingboxtransform_0065:
    fld1 ; 0065
    fcom st(1) ; 0067
    fnstsw ax ; 0069
    shr ah, 1 ; 006B
    jb amazingboxtransform_0076 ; 006D
    fstp st(0) ; 006F
    fdivr qword ptr [rdx + 16] ; 0071
    jmp amazingboxtransform_007b ; 0074
amazingboxtransform_0076:
    fcompp ; 0076
    fld qword ptr [rdx + 16] ; 0078
amazingboxtransform_007b:
    fld qword ptr [rcx + 24] ; 007B
    fmul st(0), st(1) ; 007E
    fstp qword ptr [rcx + 24] ; 0080
    fmul st(3), st(0) ; 0083
    fmul st(2), st(0) ; 0085
    fmulp st(1), st(0) ; 0087
    fstp qword ptr [rcx + 16] ; 0089
    fstp qword ptr [rcx + 8] ; 008B
    fstp qword ptr [rcx] ; 008D
    fstp st(0) ; 008F
AmazingBoxTransform_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
AmazingBoxTransform_x87 ENDP


; Original columnsIFS 004B..0166. rcx: local YZX,DE; rdx: params; r8d: iterations; r9d: Apply.
ColumnsIFSDistance_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    fld qword ptr [rcx] ; 004B
    fabs ; 004E
    fld qword ptr [rcx + 16] ; 0050
    fabs ; 0054
    fld st(0) ; 0056
    fadd st(0), st(2) ; 0058
    fld st(1) ; 005A
    fsub st(0), st(3) ; 005C
    fsub qword ptr [rdx + 72] ; 005E
    fstp qword ptr [rcx] ; 0061
    fsub qword ptr [rdx + 80] ; 0064
    fstp qword ptr [rcx + 16] ; 0067
    fstp st(0) ; 006B
    fstp st(0) ; 006D
    fld qword ptr [rcx] ; 006F
    fabs ; 0072
    fchs ; 0074
    fld qword ptr [rcx + 16] ; 0076
    fld st(0) ; 007A
    fsubr st(0), st(2) ; 007C
    fld st(1) ; 007E
    fadd st(0), st(3) ; 0080
    fchs ; 0082
    fsub qword ptr [rdx + 88] ; 0084
    fmul qword ptr [rdx + 104] ; 0087
    fstp qword ptr [rcx] ; 0089
    fsub qword ptr [rdx + 88] ; 008C
    fmul qword ptr [rdx + 104] ; 008F
    fstp qword ptr [rcx + 16] ; 0091
    fstp st(0) ; 0095
    fstp st(0) ; 0097
    fld qword ptr [rdx + 8] ; 0099
    fld qword ptr [rcx] ; 009C
    ftst ; 009F
    wait ; 00A1
    fnstsw ax ; 00A2
    shr ah, 1 ; 00A4
    jbe columnsifs_00c1 ; 00A6
    mov r10d, r8d ; original integer iteration count
    dec r10d ; 00AB
    jle columnsifs_00c1 ; 00AC
    cmp r10d, 03e7h ; 00AE
    jg columnsifs_00c1 ; 00B4
columnsifs_00b6:
    fsubr st(0), st(1) ; 00B6
    fabs ; 00B8
    fsubr st(0), st(1) ; 00BA
    fabs ; 00BC
    dec r10d ; 00BE
    jg columnsifs_00b6 ; 00BF
columnsifs_00c1:
    fstp st(1) ; 00C1
    mov r10d, dword ptr [rdx + 4] ; 00C3
    and r10d, 080000000h ; 00C6
    jne columnsifs_0106 ; 00CC
    fmul st(0), st(0) ; 00CE
    fld qword ptr [rcx + 16] ; 00D0
    mov r10d, dword ptr [rdx + 60] ; 00D4
    and r10d, 080000000h ; 00D7
    jne columnsifs_00e7 ; 00DD
    fsubr qword ptr [rdx + 56] ; 00DF
    fabs ; 00E2
    fsubr qword ptr [rdx + 56] ; 00E4
columnsifs_00e7:
    fmul st(0), st(0) ; 00E7
    faddp st(1), st(0) ; 00E9
    fsqrt ; 00EB
    fsub qword ptr [rdx + 0] ; 00ED
    test r10d, r10d ; 00F0
    je columnsifs_0104 ; 00F2
    fld qword ptr [rcx + 8] ; 00F4
    fmul st(0), st(0) ; 00F8
    fld qword ptr [rdx + 56] ; 00FA
    fadd qword ptr [rdx + 8] ; 00FD
    fmulp st(1), st(0) ; 0100
    fsubp st(1), st(0) ; 0102
columnsifs_0104:
    jmp columnsifs_0145 ; 0104
columnsifs_0106:
    fabs ; 0106
    fld qword ptr [rcx + 16] ; 0108
    mov r10d, dword ptr [rdx + 60] ; 010C
    and r10d, 080000000h ; 010F
    jne columnsifs_011f ; 0115
    fsubr qword ptr [rdx + 56] ; 0117
    fabs ; 011A
    fsubr qword ptr [rdx + 56] ; 011C
columnsifs_011f:
    fabs ; 011F
    fcom st(1) ; 0121
    wait ; 0123
    fnstsw ax ; 0124
    shr ah, 1 ; 0126
    jae columnsifs_012e ; 0128
    fstp st(0) ; 012A
    jmp columnsifs_0130 ; 012C
columnsifs_012e:
    fstp st(1) ; 012E
columnsifs_0130:
    fadd qword ptr [rdx + 0] ; 0130
    test r10d, r10d ; 0133
    je columnsifs_0145 ; 0135
    fld qword ptr [rcx + 8] ; 0137
    fld qword ptr [rdx + 56] ; 013B
    fadd qword ptr [rdx + 8] ; 013E
    fmulp st(1), st(0) ; 0141
    faddp st(1), st(0) ; 0143
columnsifs_0145:
    fld qword ptr [rcx + 8] ; 0145
    fabs ; 0149
    fsub qword ptr [rdx + 16] ; 014B
    fcom st(1) ; 014E
    wait ; 0150
    fnstsw ax ; 0151
    shr ah, 1 ; 0153
    jae columnsifs_015b ; 0155
    fstp st(0) ; 0157
    jmp columnsifs_015d ; 0159
columnsifs_015b:
    fstp st(1) ; 015B
columnsifs_015d:
    test r9d, r9d ; Apply
    jne columnsifs_0166 ; 0161
    fdiv qword ptr [rdx + 24] ; 0163
columnsifs_0166:
    fstp qword ptr [rcx + 24] ; 0166
ColumnsIFSDistance_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
ColumnsIFSDistance_x87 ENDP


; Original PiriformIFS local power/distance. rcx: mirrored XYZ,scratch,DE; rdx: params; r8d: Apply; r9d: CurveType.
PiriformIFSDistance_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
piriformifs_005f:
    fld qword ptr [rcx] ; 005F
piriformifs_006e:
    fst qword ptr [rcx] ; 006E
    fst qword ptr [rcx + 24] ; 0071
    fmul st(0), st(0) ; 0075
    cmp r9d, 0
    jne piriformifs_0080 ; 007B
    fmul qword ptr [rcx] ; 007D
piriformifs_0080:
    cmp r9d, 2
    je piriformifs_0089 ; 0084
    fmul qword ptr [rcx] ; 0086
piriformifs_0089:
    fld st(0) ; 0089
    fmul qword ptr [rcx] ; 008B
    fxch st(1) ; 008E
    fstp qword ptr [rcx] ; 0090
    fld qword ptr [rcx + 8] ; 0093
    fmul st(0), st(0) ; 0097
    faddp st(1), st(0) ; 0099
    fld qword ptr [rcx + 16] ; 009B
    fmul st(0), st(0) ; 009F
    faddp st(1), st(0) ; 00A1
    fadd qword ptr [rdx + 40] ; 00A3
    fld st(0) ; 00A6
    fabs ; 00A8
    faddp st(1), st(0) ; 00AA
    fmul qword ptr [rdx + 88] ; 00AC
    fsqrt ; 00AE
    fld qword ptr [rcx] ; 00B0
    fadd qword ptr [rdx + 0] ; 00B3
    fld st(0) ; 00B6
    fabs ; 00B8
    faddp st(1), st(0) ; 00BA
    fmul qword ptr [rdx + 88] ; 00BC
    fsqrt ; 00BE
    fchs ; 00C0
    faddp st(1), st(0) ; 00C2
    fld qword ptr [rdx + 64] ; 00C4
    fldz ; 00C7
    fcomp st(1) ; 00C9
    wait ; 00CB
    fnstsw ax ; 00CC
    shr ah, 1 ; 00CE
    jae piriformifs_00e5 ; 00D0
    fld qword ptr [rcx + 24] ; 00D2
    fabs ; 00D6
    fsub st(0), st(1) ; 00D8
    fstp st(1) ; 00DA
    fcom st(1) ; 00DC
    wait ; 00DE
    fnstsw ax ; 00DF
    shr ah, 1 ; 00E1
    jae piriformifs_00e9 ; 00E3
piriformifs_00e5:
    fstp st(0) ; 00E5
    jmp piriformifs_00eb ; 00E7
piriformifs_00e9:
    fstp st(1) ; 00E9
piriformifs_00eb:
    test r8d, r8d
    jne piriformifs_00f4 ; 00EF
    fdiv qword ptr [rdx + 8] ; 00F1
piriformifs_00f4:
    fstp qword ptr [rcx + 32] ; 00F4
PiriformIFSDistance_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
PiriformIFSDistance_x87 ENDP


; Original MengerKochV2 001B..0129. rcx: absolute XYZ,DE; rdx: expanded params.
MengerKochV2Original_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    fld qword ptr [rcx + 16] ; 001B
    fld qword ptr [rcx + 8] ; 001D
    fld qword ptr [rcx] ; 001F
    fld st(0) ; 0021
    fmul qword ptr [rdx + 80] ; 0023
    fld st(2) ; 0026
    fmul qword ptr [rdx + 88] ; 0028
    faddp st(1), st(0) ; 002B
    fld st(3) ; 002D
    fmul qword ptr [rdx + 96] ; 002F
    faddp st(1), st(0) ; 0032
    fld st(1) ; 0034
    fmul qword ptr [rdx + 32] ; 0036
    fld st(3) ; 0039
    fmul qword ptr [rdx + 40] ; 003B
    faddp st(1), st(0) ; 003E
    fld st(4) ; 0040
    fmul qword ptr [rdx + 48] ; 0042
    faddp st(1), st(0) ; 0045
    fxch st(2) ; 0047
    fmul qword ptr [rdx + 56] ; 0049
    fxch st(3) ; 004C
    fmul qword ptr [rdx + 64] ; 004E
    faddp st(3), st(0) ; 0051
    fxch st(3) ; 0053
    fmul qword ptr [rdx + 72] ; 0055
    faddp st(2), st(0) ; 0058
    fcom st(1) ; 005A
    fnstsw ax ; 005C
    shr ah, 1 ; 005E
    jae mengerkochv2_0064 ; 0060
    fxch st(1) ; 0062
mengerkochv2_0064:
    fcom st(2) ; 0064
    fnstsw ax ; 0066
    shr ah, 1 ; 0068
    jae mengerkochv2_006e ; 006A
    fxch st(2) ; 006C
mengerkochv2_006e:
    fxch st(1) ; 006E
    fcom st(2) ; 0070
    fnstsw ax ; 0072
    shr ah, 1 ; 0074
    jae mengerkochv2_007a ; 0076
    fxch st(2) ; 0078
mengerkochv2_007a:
    fld qword ptr [rdx + 176] ; 007A
    fldz ; 007D
    fcomp st(1) ; 007F
    fnstsw ax ; 0081
    shr ah, 1 ; 0083
    jae mengerkochv2_0092 ; 0085
    fsub st(0), st(2) ; 0087
    fabs ; 0089
    fchs ; 008B
    fadd qword ptr [rdx + 176] ; 008D
    fxch st(2) ; 0090
mengerkochv2_0092:
    fstp st(0) ; 0092
    nop ; 0094
    nop ; 0095
    fld qword ptr [rdx + 184] ; 0096
    fldz ; 0099
    fcomp st(1) ; 009B
    fnstsw ax ; 009D
    shr ah, 1 ; 009F
    jae mengerkochv2_00ae ; 00A1
    fsub st(0), st(1) ; 00A3
    fabs ; 00A5
    fchs ; 00A7
    fadd qword ptr [rdx + 184] ; 00A9
    fxch st(1) ; 00AC
mengerkochv2_00ae:
    fstp st(0) ; 00AE
    nop ; 00B0
    nop ; 00B1
    fnop ; 00B2
    fld st(1) ; 00B4
    fmul qword ptr [rdx + 152] ; 00B6
    fld st(1) ; 00B9
    fmul qword ptr [rdx + 160] ; 00BB
    faddp st(1), st(0) ; 00BE
    fld st(3) ; 00C0
    fmul qword ptr [rdx + 168] ; 00C2
    faddp st(1), st(0) ; 00C5
    fld st(2) ; 00C7
    fmul qword ptr [rdx + 104] ; 00C9
    fld st(2) ; 00CC
    fmul qword ptr [rdx + 112] ; 00CE
    faddp st(1), st(0) ; 00D1
    fld st(4) ; 00D3
    fmul qword ptr [rdx + 120] ; 00D5
    faddp st(1), st(0) ; 00D8
    fxch st(3) ; 00DA
    fmul qword ptr [rdx + 128] ; 00DC
    fxch st(2) ; 00DF
    fmul qword ptr [rdx + 136] ; 00E1
    faddp st(2), st(0) ; 00E4
    fxch st(3) ; 00E6
    fmul qword ptr [rdx + 144] ; 00E8
    faddp st(1), st(0) ; 00EB
    fld qword ptr [rdx + 0] ; 00ED
    fld qword ptr [rcx + 24] ; 00F0
    fmul st(0), st(1) ; 00F3
    fstp qword ptr [rcx + 24] ; 00F5
    fmul st(3), st(0) ; 00F8
    fmul st(2), st(0) ; 00FA
    fmul st(1), st(0) ; 00FC
    fld1 ; 00FE
    fsubp st(1), st(0) ; 0100
    fld st(0) ; 0102
    fld st(0) ; 0104
    fmul qword ptr [rdx + 8] ; 0106
    fsubp st(4), st(0) ; 0109
    fmul qword ptr [rdx + 16] ; 010B
    fsubp st(2), st(0) ; 010E
    fmul qword ptr [rdx + 24] ; 0110
    fld st(0) ; 0113
    fmul qword ptr [rdx + 192] ; 0115
    fcomp st(4) ; 0118
    fnstsw ax ; 011A
    and ah, 041h ; 011C
    je mengerkochv2_0123 ; 011F
    fsub st(3), st(0) ; 0121
mengerkochv2_0123:
    fstp st(0) ; 0123
    fstp qword ptr [rcx + 8] ; 0125
    fstp qword ptr [rcx] ; 0127
    fstp qword ptr [rcx + 16] ; 0129
MengerKochV2Original_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
MengerKochV2Original_x87 ENDP


; Original _helispiral 000E..00F1. rcx: XYZ; rdx: params including Julia C; r8d: H selector; r9d: S selector.
HelispiralOriginal_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    fld qword ptr [rcx + 16] ; 000E
    fld qword ptr [rcx + 8] ; 0010
    fld qword ptr [rcx] ; 0012
    fld st(0) ; 0014
    fmul qword ptr [rdx + 48] ; 0016
    fld st(2) ; 0019
    fmul qword ptr [rdx + 56] ; 001B
    faddp st(1), st(0) ; 001E
    fld st(3) ; 0020
    fmul qword ptr [rdx + 64] ; 0022
    faddp st(1), st(0) ; 0025
    fld st(1) ; 0027
    fmul qword ptr [rdx + 0] ; 0029
    fld st(3) ; 002C
    fmul qword ptr [rdx + 8] ; 002E
    faddp st(1), st(0) ; 0031
    fld st(4) ; 0033
    fmul qword ptr [rdx + 16] ; 0035
    faddp st(1), st(0) ; 0038
    fxch st(2) ; 003A
    fmul qword ptr [rdx + 24] ; 003C
    fxch st(3) ; 003F
    fmul qword ptr [rdx + 32] ; 0041
    faddp st(3), st(0) ; 0044
    fxch st(3) ; 0046
    fmul qword ptr [rdx + 40] ; 0048
    faddp st(2), st(0) ; 004B
    fstp qword ptr [rcx] ; 004D
    fstp qword ptr [rcx + 8] ; 004F
    fstp qword ptr [rcx + 16] ; 0051
    fnop ; 0053
    fld qword ptr [rcx + 8] ; 0055
    fld qword ptr [rcx] ; 0057
    cmp r9d, 1
    jne helispiral_0069 ; 005D
    fld qword ptr [rdx + 120] ; 005F
    fmul st(0), st(0) ; 0062
    fld qword ptr [rdx + 128] ; 0064
    jmp helispiral_006f ; 0067
helispiral_0069:
    fld st(0) ; 0069
    fmul st(0), st(0) ; 006B
    fld qword ptr [rcx + 8] ; 006D
helispiral_006f:
    fmul st(0), st(0) ; 006F
    faddp st(1), st(0) ; 0071
    fsqrt ; 0073
    fld qword ptr [rdx + 80] ; 0075
    fmulp st(1), st(0) ; 0078
    cmp r8d, 1
    jne helispiral_0085 ; 007E
    fld qword ptr [rdx + 136] ; 0080
    jmp helispiral_0087 ; 0083
helispiral_0085:
    fld qword ptr [rcx + 16] ; 0085
helispiral_0087:
    fld qword ptr [rdx + 72] ; 0087
    fmulp st(1), st(0) ; 008A
    faddp st(1), st(0) ; 008C
    fld qword ptr [rdx + 104] ; 008E
    fmul qword ptr [rdx + 112] ; 0091
    nop ; 0093
    faddp st(1), st(0) ; 0094
    fsincos ; 0096
    fld st(0) ; 0098
    fmul st(0), st(3) ; 009A
    fld st(2) ; 009C
    fmul st(0), st(5) ; 009E
    faddp st(1), st(0) ; 00A0
    fstp qword ptr [rcx] ; 00A2
    fmulp st(3), st(0) ; 00A4
    fmulp st(1), st(0) ; 00A6
    fsubp st(1), st(0) ; 00A8
    fstp qword ptr [rcx + 8] ; 00AA
    fnop ; 00AC
    fld qword ptr [rcx + 16] ; 00AE
    fld qword ptr [rcx + 8] ; 00B0
    fld qword ptr [rcx] ; 00B2
    fld st(0) ; 00B4
    fmul qword ptr [rdx + 16] ; 00B6
    fld st(2) ; 00B9
    fmul qword ptr [rdx + 40] ; 00BB
    faddp st(1), st(0) ; 00BE
    fld st(3) ; 00C0
    fmul qword ptr [rdx + 64] ; 00C2
    faddp st(1), st(0) ; 00C5
    fld st(1) ; 00C7
    fmul qword ptr [rdx + 0] ; 00C9
    fld st(3) ; 00CC
    fmul qword ptr [rdx + 24] ; 00CE
    faddp st(1), st(0) ; 00D1
    fld st(4) ; 00D3
    fmul qword ptr [rdx + 48] ; 00D5
    faddp st(1), st(0) ; 00D8
    fxch st(2) ; 00DA
    fmul qword ptr [rdx + 8] ; 00DC
    fxch st(3) ; 00DF
    fmul qword ptr [rdx + 32] ; 00E1
    faddp st(3), st(0) ; 00E4
    fxch st(3) ; 00E6
    fmul qword ptr [rdx + 56] ; 00E8
    faddp st(2), st(0) ; 00EB
    fstp qword ptr [rcx] ; 00ED
    fstp qword ptr [rcx + 8] ; 00EF
    fstp qword ptr [rcx + 16] ; 00F1
HelispiralOriginal_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
HelispiralOriginal_x87 ENDP


; Original sliceExIFS 004B..0147. rcx: local XYZ,sin,cos,DE; rdx: params; r8d: Apply; r9d: Boolean.
SliceExIFSDistance_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    fld qword ptr [rcx + 16] ; 004B
    fmul qword ptr [rdx + 72] ; 004F
    fsincos ; 0052
    fstp qword ptr [rcx + 32] ; 0054
    fstp qword ptr [rcx + 24] ; 0058
    fld qword ptr [rcx] ; 005C
    fmul qword ptr [rcx + 24] ; 005F
    fld qword ptr [rcx + 8] ; 0063
    fmul qword ptr [rcx + 32] ; 0067
    fchs ; 006B
    faddp st(1), st(0) ; 006D
    fld qword ptr [rcx] ; 006F
    fmul qword ptr [rcx + 32] ; 0072
    fld qword ptr [rcx + 8] ; 0076
    fmul qword ptr [rcx + 24] ; 007A
    faddp st(1), st(0) ; 007E
    fpatan ; 0080
    fmul qword ptr [rdx + 104] ; 0082
    fdiv qword ptr [rdx + 56] ; 0084
    fld st(0) ; 0087
    frndint ; 0089
    fsubp st(1), st(0) ; 008B
    fmul qword ptr [rdx + 56] ; 008D
    fabs ; 0090
    fdiv qword ptr [rdx + 0] ; 0092
    fld1 ; 0095
    fsubp st(1), st(0) ; 0097
    fmul qword ptr [rdx + 0] ; 0099
    fmul qword ptr [rdx + 112] ; 009C
    cmp r9d, 2
    je sliceexifs_00a7 ; 00A3
    fchs ; 00A5
sliceexifs_00a7:
    fld qword ptr [rcx] ; 00A7
    fmul st(0), st(0) ; 00AA
    fld qword ptr [rcx + 8] ; 00AC
    fmul st(0), st(0) ; 00B0
    faddp st(1), st(0) ; 00B2
    fsqrt ; 00B4
    fld qword ptr [rcx + 16] ; 00B6
    fpatan ; 00BA
    fmul qword ptr [rdx + 104] ; 00BC
    fdiv qword ptr [rdx + 88] ; 00BE
    fld st(0) ; 00C1
    frndint ; 00C3
    fsubp st(1), st(0) ; 00C5
    fmul qword ptr [rdx + 88] ; 00C7
    fabs ; 00CA
    fdiv qword ptr [rdx + 80] ; 00CC
    fld1 ; 00CF
    fsubp st(1), st(0) ; 00D1
    fmul qword ptr [rdx + 80] ; 00D3
    fmul qword ptr [rdx + 112] ; 00D6
    cmp r9d, 2
    je sliceexifs_00e1 ; 00DD
    fchs ; 00DF
sliceexifs_00e1:
    cmp r9d, 1
    je sliceexifs_00fe ; 00E5
    cmp r9d, 2
    je sliceexifs_00fe ; 00EB
    fcom st(1) ; 00ED
    wait ; 00EF
    fnstsw ax ; 00F0
    shr ah, 1 ; 00F2
    jae sliceexifs_00fa ; 00F4
    fstp st(0) ; 00F6
    jmp sliceexifs_00fc ; 00F8
sliceexifs_00fa:
    fstp st(1) ; 00FA
sliceexifs_00fc:
    jmp sliceexifs_010d ; 00FC
sliceexifs_00fe:
    fcom st(1) ; 00FE
    wait ; 0100
    fnstsw ax ; 0101
    shr ah, 1 ; 0103
    jae sliceexifs_010b ; 0105
    fstp st(1) ; 0107
    jmp sliceexifs_010d ; 0109
sliceexifs_010b:
    fstp st(0) ; 010B
sliceexifs_010d:
    fld qword ptr [rcx] ; 010D
    fmul st(0), st(0) ; 0110
    fld qword ptr [rcx + 8] ; 0112
    fmul st(0), st(0) ; 0116
    faddp st(1), st(0) ; 0118
    fld qword ptr [rcx + 16] ; 011A
    fmul st(0), st(0) ; 011E
    faddp st(1), st(0) ; 0120
    fsqrt ; 0122
    fsub qword ptr [rdx + 40] ; 0124
    fadd qword ptr [rdx + 64] ; 0127
    fabs ; 012A
    fsub qword ptr [rdx + 64] ; 012C
    fcom st(1) ; 012F
    wait ; 0131
    fnstsw ax ; 0132
    shr ah, 1 ; 0134
    jae sliceexifs_013c ; 0136
    fstp st(0) ; 0138
    jmp sliceexifs_013e ; 013A
sliceexifs_013c:
    fstp st(1) ; 013C
sliceexifs_013e:
    test r8d, r8d
    jne sliceexifs_0147 ; 0142
    fdiv qword ptr [rdx + 8] ; 0144
sliceexifs_0147:
    fstp qword ptr [rcx + 40] ; 0147
SliceExIFSDistance_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
SliceExIFSDistance_x87 ENDP


; Original _e_3Dtess 0016..02F3. rcx: XYZ and 44-byte original scratch; rdx: params; r8d: axiom; r9d: depth.
; Fifth argument: FormulaWorkCounter. Return eax=0 complete, eax=1 exhausted.
E3DtessOriginal_x87 PROC
    sub rsp, 8
    mov r10, qword ptr [rsp + 48]
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    fld qword ptr [rcx] ; 0016
    fmul qword ptr [rdx + 40] ; 0018
    fadd qword ptr [rdx + 0] ; 001B
    fstp qword ptr [rcx] ; 001E
    fld qword ptr [rcx + 8] ; 0020
    fmul qword ptr [rdx + 40] ; 0022
    fadd qword ptr [rdx + 8] ; 0025
    fstp qword ptr [rcx + 8] ; 0028
    fld qword ptr [rcx + 16] ; 002A
    fmul qword ptr [rdx + 40] ; 002C
    fadd qword ptr [rdx + 16] ; 002F
    fstp qword ptr [rcx + 16] ; 0032
    fnop ; 0034
    cmp r8d, 1
    jne e3dtess_0046 ; 003A
    fld qword ptr [rdx + 64] ; 003C
    fstp qword ptr [rcx + 28] ; 003E
    jmp e3dtess_006e ; 0041
e3dtess_0046:
    cmp r8d, 2
    jne e3dtess_0057 ; 004A
    fld qword ptr [rdx + 80] ; 004C
    fstp qword ptr [rcx + 28] ; 004F
    jmp e3dtess_006e ; 0052
e3dtess_0057:
    cmp r8d, 3
    jne e3dtess_0068 ; 005B
    fld qword ptr [rdx + 88] ; 005D
    fstp qword ptr [rcx + 28] ; 0060
    jmp e3dtess_006e ; 0063
e3dtess_0068:
    fld1 ; 0068
    fstp qword ptr [rcx + 28] ; 006A
    nop ; 006D
e3dtess_006e:
    fldz ; 006E
    fistp dword ptr [rcx + 44] ; 0070
    fld qword ptr [rcx + 28] ; 0073
    fld1 ; 0076
    faddp st(1), st(0) ; 0078
    fmul qword ptr [rcx + 28] ; 007A
    fstp qword ptr [rcx + 36] ; 007D
    fldz ; 0080
    fistp dword ptr [rcx + 24] ; 0082
    nop ; 0085
e3dtess_0086:
    mov eax, dword ptr [r10]
    cmp eax, dword ptr [r10 + 4]
    jae e3dtess_work_exhausted
    inc dword ptr [r10]
    fnop ; 0086
    add dword ptr [rcx + 24], 1 ; 0088
    fld qword ptr [rcx] ; 008C
    fabs ; 008E
    fstp qword ptr [rcx + 60] ; 0090
    fnop ; 0093
    fldz ; 0095
    fistp dword ptr [rcx + 48] ; 0097
    fld qword ptr [rcx + 8] ; 009A
    fabs ; 009C
    fld qword ptr [rcx + 60] ; 009E
    fcompp ; 00A1
    fnstsw ax ; 00A3
    and ah, 041h ; 00A5
    jle e3dtess_00b8 ; 00A8
    fld qword ptr [rcx + 8] ; 00AA
    fabs ; 00AC
    fstp qword ptr [rcx + 60] ; 00AE
    fld1 ; 00B1
    fistp dword ptr [rcx + 48] ; 00B3
    fnop ; 00B6
e3dtess_00b8:
    fld qword ptr [rcx + 16] ; 00B8
    fabs ; 00BA
    fld qword ptr [rcx + 60] ; 00BC
    fcompp ; 00BF
    fnstsw ax ; 00C1
    and ah, 041h ; 00C3
    jle e3dtess_00d7 ; 00C6
    fld qword ptr [rcx + 16] ; 00C8
    fabs ; 00CA
    fstp qword ptr [rcx + 60] ; 00CC
    fld qword ptr [rdx + 72] ; 00CF
    fistp dword ptr [rcx + 48] ; 00D2
    fnop ; 00D5
e3dtess_00d7:
    fld qword ptr [rcx + 60] ; 00D7
    fld qword ptr [rcx + 36] ; 00DA
    fcompp ; 00DD
    fnstsw ax ; 00DF
    and ah, 041h ; 00E1
    jg e3dtess_0291 ; 00E4
    fld qword ptr [rcx + 60] ; 00EA
    fld qword ptr [rcx + 28] ; 00ED
    fcompp ; 00F0
    fnstsw ax ; 00F2
    and ah, 041h ; 00F4
    jle e3dtess_0291 ; 00F7
    fld1 ; 00FD
    fistp dword ptr [rcx + 44] ; 00FF
    cmp dword ptr [rcx + 48], 0 ; 0102
    jne e3dtess_0187 ; 0106
    nop ; 010C
    fld qword ptr [rcx + 60] ; 010D
    fdiv qword ptr [rcx] ; 0110
    fstp qword ptr [rcx + 52] ; 0112
    fld qword ptr [rcx + 36] ; 0115
    fadd qword ptr [rcx + 28] ; 0118
    fmul qword ptr [rcx + 52] ; 011B
    fmul qword ptr [rdx + 64] ; 011E
    fchs ; 0120
    fadd qword ptr [rcx] ; 0122
    fmul qword ptr [rdx + 72] ; 0124
    fmul qword ptr [rdx + 32] ; 0127
    fld qword ptr [rcx + 36] ; 012A
    fsub qword ptr [rcx + 28] ; 012D
    fdivp st(1), st(0) ; 0130
    fstp qword ptr [rcx] ; 0132
    fnop ; 0134
    fld qword ptr [rcx + 8] ; 0136
    fld qword ptr [rcx + 36] ; 0138
    fsub qword ptr [rcx + 28] ; 013B
    fdivp st(1), st(0) ; 013E
    fstp qword ptr [rcx + 8] ; 0140
    fnop ; 0142
    fld qword ptr [rcx + 8] ; 0144
    fadd qword ptr [rdx + 64] ; 0146
    frndint ; 0148
    fchs ; 014A
    fadd qword ptr [rdx + 64] ; 014C
    fadd qword ptr [rcx + 8] ; 014E
    fmul qword ptr [rdx + 72] ; 0150
    fmul qword ptr [rdx + 32] ; 0153
    fstp qword ptr [rcx + 8] ; 0156
    fnop ; 0158
    fnop ; 015A
    fld qword ptr [rcx + 16] ; 015C
    fld qword ptr [rcx + 36] ; 015E
    fsub qword ptr [rcx + 28] ; 0161
    fdivp st(1), st(0) ; 0164
    fstp qword ptr [rcx + 16] ; 0166
    fnop ; 0168
    fld qword ptr [rcx + 16] ; 016A
    fadd qword ptr [rdx + 64] ; 016C
    frndint ; 016E
    fchs ; 0170
    fadd qword ptr [rdx + 64] ; 0172
    fadd qword ptr [rcx + 16] ; 0174
    fmul qword ptr [rdx + 72] ; 0176
    fmul qword ptr [rdx + 32] ; 0179
    fstp qword ptr [rcx + 16] ; 017C
    fnop ; 017E
    fnop ; 0180
    jmp e3dtess_02d3 ; 0182
e3dtess_0187:
    cmp dword ptr [rcx + 48], 1 ; 0187
    jne e3dtess_020c ; 018B
    nop ; 0191
    fld qword ptr [rcx + 60] ; 0192
    fdiv qword ptr [rcx + 8] ; 0195
    fstp qword ptr [rcx + 52] ; 0197
    fld qword ptr [rcx + 36] ; 019A
    fadd qword ptr [rcx + 28] ; 019D
    fmul qword ptr [rcx + 52] ; 01A0
    fmul qword ptr [rdx + 64] ; 01A3
    fchs ; 01A5
    fadd qword ptr [rcx + 8] ; 01A7
    fmul qword ptr [rdx + 72] ; 01A9
    fmul qword ptr [rdx + 32] ; 01AC
    fld qword ptr [rcx + 36] ; 01AF
    fsub qword ptr [rcx + 28] ; 01B2
    fdivp st(1), st(0) ; 01B5
    fstp qword ptr [rcx + 8] ; 01B7
    fnop ; 01B9
    fld qword ptr [rcx] ; 01BB
    fld qword ptr [rcx + 36] ; 01BD
    fsub qword ptr [rcx + 28] ; 01C0
    fdivp st(1), st(0) ; 01C3
    fstp qword ptr [rcx] ; 01C5
    fnop ; 01C7
    fld qword ptr [rcx] ; 01C9
    fadd qword ptr [rdx + 64] ; 01CB
    frndint ; 01CD
    fchs ; 01CF
    fadd qword ptr [rdx + 64] ; 01D1
    fadd qword ptr [rcx] ; 01D3
    fmul qword ptr [rdx + 72] ; 01D5
    fmul qword ptr [rdx + 32] ; 01D8
    fstp qword ptr [rcx] ; 01DB
    fnop ; 01DD
    fnop ; 01DF
    fld qword ptr [rcx + 16] ; 01E1
    fld qword ptr [rcx + 36] ; 01E3
    fsub qword ptr [rcx + 28] ; 01E6
    fdivp st(1), st(0) ; 01E9
    fstp qword ptr [rcx + 16] ; 01EB
    fnop ; 01ED
    fld qword ptr [rcx + 16] ; 01EF
    fadd qword ptr [rdx + 64] ; 01F1
    frndint ; 01F3
    fchs ; 01F5
    fadd qword ptr [rdx + 64] ; 01F7
    fadd qword ptr [rcx + 16] ; 01F9
    fmul qword ptr [rdx + 72] ; 01FB
    fmul qword ptr [rdx + 32] ; 01FE
    fstp qword ptr [rcx + 16] ; 0201
    fnop ; 0203
    fnop ; 0205
    jmp e3dtess_02d3 ; 0207
e3dtess_020c:
    nop ; 020C
    nop ; 020D
    nop ; 020E
    nop ; 020F
    nop ; 0210
    nop ; 0211
    nop ; 0212
    nop ; 0213
    nop ; 0214
    nop ; 0215
    nop ; 0216
    fld qword ptr [rcx + 60] ; 0217
    fdiv qword ptr [rcx + 16] ; 021A
    fstp qword ptr [rcx + 52] ; 021C
    fld qword ptr [rcx + 36] ; 021F
    fadd qword ptr [rcx + 28] ; 0222
    fmul qword ptr [rcx + 52] ; 0225
    fmul qword ptr [rdx + 64] ; 0228
    fchs ; 022A
    fadd qword ptr [rcx + 16] ; 022C
    fmul qword ptr [rdx + 72] ; 022E
    fmul qword ptr [rdx + 32] ; 0231
    fld qword ptr [rcx + 36] ; 0234
    fsub qword ptr [rcx + 28] ; 0237
    fdivp st(1), st(0) ; 023A
    fstp qword ptr [rcx + 16] ; 023C
    fnop ; 023E
    fld qword ptr [rcx + 8] ; 0240
    fld qword ptr [rcx + 36] ; 0242
    fsub qword ptr [rcx + 28] ; 0245
    fdivp st(1), st(0) ; 0248
    fstp qword ptr [rcx + 8] ; 024A
    fnop ; 024C
    fld qword ptr [rcx + 8] ; 024E
    fadd qword ptr [rdx + 64] ; 0250
    frndint ; 0252
    fchs ; 0254
    fadd qword ptr [rdx + 64] ; 0256
    fadd qword ptr [rcx + 8] ; 0258
    fmul qword ptr [rdx + 72] ; 025A
    fmul qword ptr [rdx + 32] ; 025D
    fstp qword ptr [rcx + 8] ; 0260
    fnop ; 0262
    fnop ; 0264
    fld qword ptr [rcx] ; 0266
    fld qword ptr [rcx + 36] ; 0268
    fsub qword ptr [rcx + 28] ; 026B
    fdivp st(1), st(0) ; 026E
    fstp qword ptr [rcx] ; 0270
    fnop ; 0272
    fld qword ptr [rcx] ; 0274
    fadd qword ptr [rdx + 64] ; 0276
    frndint ; 0278
    fchs ; 027A
    fadd qword ptr [rdx + 64] ; 027C
    fadd qword ptr [rcx] ; 027E
    fmul qword ptr [rdx + 72] ; 0280
    fmul qword ptr [rdx + 32] ; 0283
    fstp qword ptr [rcx] ; 0286
    fnop ; 0288
    fnop ; 028A
    jmp e3dtess_02d3 ; 028C
    nop ; 028E
    nop ; 028F
    nop ; 0290
e3dtess_0291:
    fld qword ptr [rcx + 28] ; 0291
    nop ; 0294
    fld qword ptr [rcx + 36] ; 0295
    fstp qword ptr [rcx + 28] ; 0298
    nop ; 029B
    fld qword ptr [rcx + 36] ; 029C
    fsubrp st(1), st(0) ; 029F
    fmul qword ptr [rdx + 64] ; 02A1
    fadd qword ptr [rcx + 36] ; 02A3
    fstp qword ptr [rcx + 36] ; 02A6
    nop ; 02A9
    cmp dword ptr [rcx + 44], 0 ; 02AA
    jne e3dtess_02c7 ; 02AE
    mov eax, r9d
    cmp eax, dword ptr [rcx + 24] ; 02B3
    jle e3dtess_02d3 ; 02B6
    nop ; 02B8
    nop ; 02B9
    nop ; 02BA
    nop ; 02BB
    fld qword ptr [rcx + 36] ; 02BC
    fsub qword ptr [rcx + 28] ; 02BF
    fld qword ptr [rdx + 96] ; 02C2
    fcompp ; 02C5
e3dtess_02c7:
    fnstsw ax ; 02C7
    and ah, 041h ; 02C9
    jg e3dtess_0086 ; 02CC
    nop ; 02D2
e3dtess_02d3:
    fld qword ptr [rcx] ; 02D3
    fmul qword ptr [rdx + 24] ; 02D5
    fsub qword ptr [rdx + 0] ; 02D8
    fstp qword ptr [rcx] ; 02DB
    fld qword ptr [rcx + 8] ; 02DD
    fmul qword ptr [rdx + 24] ; 02DF
    fsub qword ptr [rdx + 8] ; 02E2
    fstp qword ptr [rcx + 8] ; 02E5
    fld qword ptr [rcx + 16] ; 02E7
    fmul qword ptr [rdx + 24] ; 02E9
    fsub qword ptr [rdx + 16] ; 02EC
    fstp qword ptr [rcx + 16] ; 02EF
    fnop ; 02F1
    fnop ; 02F3
E3DtessOriginal_x87_done:
    xor eax, eax
    jmp e3dtess_work_done
e3dtess_work_exhausted:
    mov eax, 1
e3dtess_work_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
E3DtessOriginal_x87 ENDP


; Original ABoxVSShapes. rcx: XYZ,DE,VaryScale,32-byte scratch,first(int32); rdx: params and Julia C; r8d: shape.
ABoxVSShapesOriginal_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    cmp dword ptr [rcx + 72], 0 ; 001A
    jg aboxvsshapes_0029 ; 001E
    inc dword ptr [rcx + 72] ; 0020
    fld qword ptr [rdx + 0] ; 0023
    fstp qword ptr [rcx + 32] ; 0026
aboxvsshapes_0029:
    fld qword ptr [rcx + 32] ; 0029
    fld1 ; 002C
    fld st(1) ; 002E
    fabs ; 0030
    fsubrp st(1), st(0) ; 0032
    fmul qword ptr [rdx + 32] ; 0034
    faddp st(1), st(0) ; 0037
    fstp qword ptr [rcx + 32] ; 0039
    fld qword ptr [rdx + 24] ; 003C
    fld st(0) ; 003F
    fld st(0) ; 0041
    fadd st(2), st(0) ; 0043
    fchs ; 0045
    fld qword ptr [rcx] ; 0047
    fcom st(2) ; 0049
    fnstsw ax ; 004B
    shr ah, 1 ; 004D
    jb aboxvsshapes_0057 ; 004F
    fchs ; 0051
    fadd st(0), st(3) ; 0053
    jmp aboxvsshapes_0063 ; 0055
aboxvsshapes_0057:
    fcom st(1) ; 0057
    fnstsw ax ; 0059
    shr ah, 1 ; 005B
    jae aboxvsshapes_0063 ; 005D
    fchs ; 005F
    fsub st(0), st(3) ; 0061
aboxvsshapes_0063:
    fld qword ptr [rcx + 8] ; 0063
    fcom st(3) ; 0065
    fnstsw ax ; 0067
    shr ah, 1 ; 0069
    jb aboxvsshapes_0073 ; 006B
    fchs ; 006D
    fadd st(0), st(4) ; 006F
    jmp aboxvsshapes_007f ; 0071
aboxvsshapes_0073:
    fcom st(2) ; 0073
    fnstsw ax ; 0075
    shr ah, 1 ; 0077
    jae aboxvsshapes_007f ; 0079
    fchs ; 007B
    fsub st(0), st(4) ; 007D
aboxvsshapes_007f:
    fld qword ptr [rcx + 16] ; 007F
    fcom st(4) ; 0081
    fnstsw ax ; 0083
    shr ah, 1 ; 0085
    jb aboxvsshapes_008f ; 0087
    fchs ; 0089
    fadd st(0), st(5) ; 008B
    jmp aboxvsshapes_009b ; 008D
aboxvsshapes_008f:
    fcom st(3) ; 008F
    fnstsw ax ; 0091
    shr ah, 1 ; 0093
    jae aboxvsshapes_009b ; 0095
    fchs ; 0097
    fsub st(0), st(5) ; 0099
aboxvsshapes_009b:
    fxch st(3) ; 009B
    fstp st(0) ; 009D
    fxch st(3) ; 009F
    fstp st(0) ; 00A1
    fxch st(3) ; 00A3
    fstp st(0) ; 00A5
    nop ; 00A7
    cmp r8d, 1
    jne aboxvsshapes_00cd ; 00AC
    fld st(0) ; 00AE
    fmul st(0), st(0) ; 00B0
    fld st(2) ; 00B2
    fmul st(0), st(0) ; 00B4
    faddp st(1), st(0) ; 00B6
    fsqrt ; 00B8
    fsub qword ptr [rdx + 48] ; 00BA
    fmul st(0), st(0) ; 00BD
    fld st(3) ; 00BF
    fmul st(0), st(0) ; 00C1
    faddp st(1), st(0) ; 00C3
    fmul qword ptr [rdx + 56] ; 00C5
    jmp aboxvsshapes_0441 ; 00C8
aboxvsshapes_00cd:
    nop ; 00CD
    cmp r8d, 2
    jne aboxvsshapes_0115 ; 00D2
    fld st(2) ; 00D4
    fabs ; 00D6
    fld qword ptr [rdx + 48] ; 00D8
    fcom st(1) ; 00DB
    fnstsw ax ; 00DD
    shr ah, 1 ; 00DF
    jbe aboxvsshapes_010d ; 00E1
    fsubrp st(1), st(0) ; 00E3
    fmul st(0), st(0) ; 00E5
    nop ; 00E7
aboxvsshapes_00e8:
    fld st(1) ; 00E8
    fmul st(0), st(0) ; 00EA
    fld st(3) ; 00EC
    fmul st(0), st(0) ; 00EE
    faddp st(1), st(0) ; 00F0
    fmul qword ptr [rdx + 56] ; 00F2
    nop ; 00F5
    nop ; 00F6
    fcom st(1) ; 00F7
    fnstsw ax ; 00F9
    shr ah, 1 ; 00FB
    jae aboxvsshapes_0106 ; 00FD
    fstp st(0) ; 00FF
    jmp aboxvsshapes_0441 ; 0101
aboxvsshapes_0106:
    fstp st(1) ; 0106
    jmp aboxvsshapes_0441 ; 0108
aboxvsshapes_010d:
    fstp st(0) ; 010D
    fstp st(0) ; 010F
    fldz ; 0111
    jmp aboxvsshapes_00e8 ; 0113
aboxvsshapes_0115:
    nop ; 0115
    cmp r8d, 3
    jne aboxvsshapes_015d ; 011A
    fld st(2) ; 011C
    fabs ; 011E
    fld qword ptr [rdx + 48] ; 0120
    fcom st(1) ; 0123
    fnstsw ax ; 0125
    shr ah, 1 ; 0127
    jbe aboxvsshapes_0155 ; 0129
    fsubrp st(1), st(0) ; 012B
    fmul st(0), st(0) ; 012D
    nop ; 012F
aboxvsshapes_0130:
    fld st(1) ; 0130
    fmul st(0), st(0) ; 0132
    fld st(3) ; 0134
    fmul st(0), st(0) ; 0136
    faddp st(1), st(0) ; 0138
    fmul qword ptr [rdx + 56] ; 013A
    nop ; 013D
    nop ; 013E
    fcom st(1) ; 013F
    fnstsw ax ; 0141
    shr ah, 1 ; 0143
    jbe aboxvsshapes_014e ; 0145
    fstp st(0) ; 0147
    jmp aboxvsshapes_0441 ; 0149
aboxvsshapes_014e:
    fstp st(1) ; 014E
    jmp aboxvsshapes_0441 ; 0150
aboxvsshapes_0155:
    fstp st(0) ; 0155
    fstp st(0) ; 0157
    fldz ; 0159
    jmp aboxvsshapes_0130 ; 015B
aboxvsshapes_015d:
    nop ; 015D
    cmp r8d, 4
    jne aboxvsshapes_01a5 ; 0162
    fld st(2) ; 0164
    fabs ; 0166
    fld qword ptr [rdx + 48] ; 0168
    fcom st(1) ; 016B
    fnstsw ax ; 016D
    shr ah, 1 ; 016F
    jbe aboxvsshapes_019d ; 0171
    fsubrp st(1), st(0) ; 0173
    nop ; 0175
    nop ; 0176
    nop ; 0177
aboxvsshapes_0178:
    fld st(1) ; 0178
    fmul st(0), st(0) ; 017A
    fld st(3) ; 017C
    fmul st(0), st(0) ; 017E
    faddp st(1), st(0) ; 0180
    fmul qword ptr [rdx + 56] ; 0182
    fxch st(1) ; 0185
    fcom st(1) ; 0187
    fnstsw ax ; 0189
    shr ah, 1 ; 018B
    jae aboxvsshapes_0196 ; 018D
    fstp st(0) ; 018F
    jmp aboxvsshapes_0441 ; 0191
aboxvsshapes_0196:
    fdivp st(1), st(0) ; 0196
    jmp aboxvsshapes_0441 ; 0198
aboxvsshapes_019d:
    fstp st(0) ; 019D
    fstp st(0) ; 019F
    fld1 ; 01A1
    jmp aboxvsshapes_0178 ; 01A3
aboxvsshapes_01a5:
    nop ; 01A5
    cmp r8d, 5
    jne aboxvsshapes_01ed ; 01AA
    fld st(2) ; 01AC
    fabs ; 01AE
    fld qword ptr [rdx + 48] ; 01B0
    fcom st(1) ; 01B3
    fnstsw ax ; 01B5
    shr ah, 1 ; 01B7
    jbe aboxvsshapes_01e5 ; 01B9
    fsubrp st(1), st(0) ; 01BB
    nop ; 01BD
    nop ; 01BE
    nop ; 01BF
aboxvsshapes_01c0:
    fld st(1) ; 01C0
    fmul st(0), st(0) ; 01C2
    fld st(3) ; 01C4
    fmul st(0), st(0) ; 01C6
    faddp st(1), st(0) ; 01C8
    fmul qword ptr [rdx + 56] ; 01CA
    fxch st(1) ; 01CD
    fcom st(1) ; 01CF
    fnstsw ax ; 01D1
    shr ah, 1 ; 01D3
    jbe aboxvsshapes_01de ; 01D5
    fstp st(0) ; 01D7
    jmp aboxvsshapes_0441 ; 01D9
aboxvsshapes_01de:
    fdivp st(1), st(0) ; 01DE
    jmp aboxvsshapes_0441 ; 01E0
aboxvsshapes_01e5:
    fstp st(0) ; 01E5
    fstp st(0) ; 01E7
    fld1 ; 01E9
    jmp aboxvsshapes_01c0 ; 01EB
aboxvsshapes_01ed:
    nop ; 01ED
    cmp r8d, 6
    jne aboxvsshapes_023e ; 01F2
    fld st(2) ; 01F4
    fmul st(0), st(0) ; 01F6
    fld qword ptr [rdx + 48] ; 01F8
    fcom st(1) ; 01FB
    fnstsw ax ; 01FD
    shr ah, 1 ; 01FF
    jbe aboxvsshapes_0236 ; 0201
    fsubrp st(1), st(0) ; 0203
aboxvsshapes_0205:
    fld st(1) ; 0205
    fmul st(0), st(0) ; 0207
    fld st(3) ; 0209
    fmul st(0), st(0) ; 020B
    fcom st(1) ; 020D
    fnstsw ax ; 020F
    shr ah, 1 ; 0211
    jae aboxvsshapes_0219 ; 0213
    fstp st(0) ; 0215
    jmp aboxvsshapes_021b ; 0217
aboxvsshapes_0219:
    fstp st(1) ; 0219
aboxvsshapes_021b:
    fmul qword ptr [rdx + 56] ; 021B
    fxch st(1) ; 021E
    fcom st(1) ; 0220
    fnstsw ax ; 0222
    shr ah, 1 ; 0224
    jae aboxvsshapes_022f ; 0226
    fstp st(0) ; 0228
    jmp aboxvsshapes_0441 ; 022A
aboxvsshapes_022f:
    fdivp st(1), st(0) ; 022F
    jmp aboxvsshapes_0441 ; 0231
aboxvsshapes_0236:
    fstp st(0) ; 0236
    fstp st(0) ; 0238
    fld1 ; 023A
    jmp aboxvsshapes_0205 ; 023C
aboxvsshapes_023e:
    nop ; 023E
    cmp r8d, 7
    jne aboxvsshapes_028f ; 0243
    fld st(2) ; 0245
    fmul st(0), st(0) ; 0247
    fld qword ptr [rdx + 48] ; 0249
    fcom st(1) ; 024C
    fnstsw ax ; 024E
    shr ah, 1 ; 0250
    jbe aboxvsshapes_0287 ; 0252
    fsubrp st(1), st(0) ; 0254
aboxvsshapes_0256:
    fld st(1) ; 0256
    fmul st(0), st(0) ; 0258
    fld st(3) ; 025A
    fmul st(0), st(0) ; 025C
    fcom st(1) ; 025E
    fnstsw ax ; 0260
    shr ah, 1 ; 0262
    jae aboxvsshapes_026a ; 0264
    fstp st(0) ; 0266
    jmp aboxvsshapes_026c ; 0268
aboxvsshapes_026a:
    fstp st(1) ; 026A
aboxvsshapes_026c:
    fmul qword ptr [rdx + 56] ; 026C
    fxch st(1) ; 026F
    fcom st(1) ; 0271
    fnstsw ax ; 0273
    shr ah, 1 ; 0275
    jbe aboxvsshapes_0280 ; 0277
    fstp st(0) ; 0279
    jmp aboxvsshapes_0441 ; 027B
aboxvsshapes_0280:
    fdivp st(1), st(0) ; 0280
    jmp aboxvsshapes_0441 ; 0282
aboxvsshapes_0287:
    fstp st(0) ; 0287
    fstp st(0) ; 0289
    fld1 ; 028B
    jmp aboxvsshapes_0256 ; 028D
aboxvsshapes_028f:
    nop ; 028F
    cmp r8d, 8
    jne aboxvsshapes_02b6 ; 0294
    fld st(2) ; 0296
    fmul st(0), st(0) ; 0298
    fld st(0) ; 029A
    fsub qword ptr [rdx + 48] ; 029C
    fmulp st(1), st(0) ; 029F
    fld st(1) ; 02A1
    fmul st(0), st(0) ; 02A3
    fld st(3) ; 02A5
    fmul st(0), st(0) ; 02A7
    faddp st(1), st(0) ; 02A9
    faddp st(1), st(0) ; 02AB
    fmul qword ptr [rdx + 56] ; 02AD
    jmp aboxvsshapes_0441 ; 02B0
    nop ; 02B5
aboxvsshapes_02b6:
    cmp r8d, 0ah
    jne aboxvsshapes_02dd ; 02BA
    fld st(2) ; 02BC
    fmul st(0), st(0) ; 02BE
    fmul st(0), st(0) ; 02C0
    fld st(2) ; 02C2
    fmul st(0), st(0) ; 02C4
    fmul st(0), st(0) ; 02C6
    faddp st(1), st(0) ; 02C8
    fld st(1) ; 02CA
    fmul st(0), st(0) ; 02CC
    fmul st(0), st(0) ; 02CE
    faddp st(1), st(0) ; 02D0
    fmul qword ptr [rdx + 56] ; 02D2
    fsub qword ptr [rdx + 48] ; 02D5
    jmp aboxvsshapes_0441 ; 02D8
aboxvsshapes_02dd:
    nop ; 02DD
    cmp r8d, 0bh
    jne aboxvsshapes_0343 ; 02E2
    fld st(2) ; 02E4
    fmul st(0), st(0) ; 02E6
    fstp qword ptr [rcx + 64] ; 02E8
    fld st(1) ; 02EB
    fmul st(0), st(0) ; 02ED
    fstp qword ptr [rcx + 56] ; 02EF
    fld st(0) ; 02F2
    fmul st(0), st(0) ; 02F4
    fstp qword ptr [rcx + 48] ; 02F6
    fld st(2) ; 02F9
    fmul st(0), st(2) ; 02FB
    fmul st(0), st(1) ; 02FD
    fmul qword ptr [rdx + 64] ; 02FF
    fld qword ptr [rcx + 64] ; 0301
    fmul qword ptr [rcx + 56] ; 0304
    fmul qword ptr [rcx + 48] ; 0307
    faddp st(1), st(0) ; 030A
    fld qword ptr [rcx + 64] ; 030C
    fmul qword ptr [rcx + 56] ; 030F
    fld qword ptr [rcx + 64] ; 0312
    fmul qword ptr [rcx + 48] ; 0315
    faddp st(1), st(0) ; 0318
    fld qword ptr [rcx + 48] ; 031A
    fmul qword ptr [rcx + 56] ; 031D
    faddp st(1), st(0) ; 0320
    fmul qword ptr [rdx + 48] ; 0322
    faddp st(1), st(0) ; 0325
    fld qword ptr [rcx + 64] ; 0327
    fadd qword ptr [rcx + 56] ; 032A
    fadd qword ptr [rcx + 48] ; 032D
    fmul qword ptr [rdx + 48] ; 0330
    fmul qword ptr [rdx + 48] ; 0333
    faddp st(1), st(0) ; 0336
    fadd qword ptr [rdx + 56] ; 0338
    fmul qword ptr [rdx + 72] ; 033B
    jmp aboxvsshapes_0441 ; 033E
aboxvsshapes_0343:
    nop ; 0343
    cmp r8d, 9
    jne aboxvsshapes_0381 ; 0348
    fld st(2) ; 034A
    fmul st(0), st(0) ; 034C
    fmul st(0), st(0) ; 034E
    fld st(2) ; 0350
    fmul st(0), st(0) ; 0352
    fmul st(0), st(0) ; 0354
    faddp st(1), st(0) ; 0356
    fld st(1) ; 0358
    fmul st(0), st(0) ; 035A
    fmul st(0), st(0) ; 035C
    faddp st(1), st(0) ; 035E
    fstp qword ptr [rcx + 56] ; 0360
    fld st(2) ; 0363
    fmul st(0), st(0) ; 0365
    fld st(2) ; 0367
    fmul st(0), st(0) ; 0369
    faddp st(1), st(0) ; 036B
    fld st(1) ; 036D
    fmul st(0), st(0) ; 036F
    faddp st(1), st(0) ; 0371
    fmul qword ptr [rdx + 56] ; 0373
    fadd qword ptr [rcx + 56] ; 0376
    fadd qword ptr [rdx + 48] ; 0379
    jmp aboxvsshapes_0441 ; 037C
aboxvsshapes_0381:
    nop ; 0381
    cmp r8d, 0ch
    jne aboxvsshapes_03b5 ; 0386
    fld st(2) ; 0388
    fchs ; 038A
    fmul qword ptr [rdx + 48] ; 038C
    fld st(1) ; 038F
    fmul st(0), st(0) ; 0391
    fld st(3) ; 0393
    fmul st(0), st(0) ; 0395
    faddp st(1), st(0) ; 0397
    fsqrt ; 0399
    fmul qword ptr [rdx + 56] ; 039B
    faddp st(1), st(0) ; 039E
    fldz ; 03A0
    fcomp st(1) ; 03A2
    fnstsw ax ; 03A4
    shr ah, 1 ; 03A6
    jbe aboxvsshapes_03ae ; 03A8
    fstp st(0) ; 03AA
    fldz ; 03AC
aboxvsshapes_03ae:
    fmul st(0), st(0) ; 03AE
    jmp aboxvsshapes_0441 ; 03B0
aboxvsshapes_03b5:
    nop ; 03B5
    cmp r8d, 0dh
    jne aboxvsshapes_03e7 ; 03BA
    fld st(2) ; 03BC
    fchs ; 03BE
    fld1 ; 03C0
    fcomp st(1) ; 03C2
    fnstsw ax ; 03C4
    shr ah, 1 ; 03C6
    jae aboxvsshapes_03d0 ; 03C8
    fstp st(0) ; 03CA
    fld1 ; 03CC
    nop ; 03CE
    nop ; 03CF
aboxvsshapes_03d0:
    fmul qword ptr [rdx + 48] ; 03D0
    fld st(1) ; 03D3
    fmul st(0), st(0) ; 03D5
    fld st(3) ; 03D7
    fmul st(0), st(0) ; 03D9
    faddp st(1), st(0) ; 03DB
    faddp st(1), st(0) ; 03DD
    fmul qword ptr [rdx + 56] ; 03DF
    jmp aboxvsshapes_0441 ; 03E2
aboxvsshapes_03e7:
    nop ; 03E7
    cmp r8d, 0eh
    jne aboxvsshapes_0426 ; 03EC
    fld st(2) ; 03EE
    fmul st(0), st(0) ; 03F0
    fmul qword ptr [rdx + 48] ; 03F2
    fld st(2) ; 03F5
    fmul st(0), st(0) ; 03F7
    faddp st(1), st(0) ; 03F9
    nop ; 03FB
    fld st(3) ; 03FC
    fmul st(0), st(0) ; 03FE
    fmul qword ptr [rdx + 48] ; 0400
    fld st(2) ; 0403
    fmul st(0), st(0) ; 0405
    faddp st(1), st(0) ; 0407
    nop ; 0409
    fcom st(1) ; 040A
    fnstsw ax ; 040C
    shr ah, 1 ; 040E
    jae aboxvsshapes_041c ; 0410
    fstp st(0) ; 0412
    fmul qword ptr [rdx + 56] ; 0414
    jmp aboxvsshapes_0441 ; 0417
aboxvsshapes_041c:
    fstp st(1) ; 041C
    fmul qword ptr [rdx + 56] ; 041E
    jmp aboxvsshapes_0441 ; 0421
aboxvsshapes_0426:
    nop ; 0426
    jmp aboxvsshapes_048f ; 0427
    nop ; 042C
    nop ; 042D
    nop ; 042E
    nop ; 042F
    nop ; 0430
    nop ; 0431
    nop ; 0432
    nop ; 0433
    nop ; 0434
    nop ; 0435
    nop ; 0436
    nop ; 0437
    nop ; 0438
    nop ; 0439
    nop ; 043A
    nop ; 043B
    nop ; 043C
    nop ; 043D
    nop ; 043E
    nop ; 043F
    nop ; 0440
aboxvsshapes_0441:
    fcom qword ptr [rdx + 16] ; 0441
    fnstsw ax ; 0444
    shr ah, 1 ; 0446
    jae aboxvsshapes_0451 ; 0448
    fstp st(0) ; 044A
    fld qword ptr [rdx + 8] ; 044C
    jmp aboxvsshapes_0467 ; 044F
aboxvsshapes_0451:
    fld1 ; 0451
    fcom st(1) ; 0453
    fnstsw ax ; 0455
    shr ah, 1 ; 0457
    jb aboxvsshapes_0462 ; 0459
    fstp st(0) ; 045B
    fdivr qword ptr [rcx + 32] ; 045D
    jmp aboxvsshapes_0467 ; 0460
aboxvsshapes_0462:
    fcompp ; 0462
    fld qword ptr [rcx + 32] ; 0464
aboxvsshapes_0467:
    fld qword ptr [rcx + 24] ; 0467
    fmul st(0), st(1) ; 046A
    fstp qword ptr [rcx + 24] ; 046C
    fmul st(3), st(0) ; 046F
    fmul st(2), st(0) ; 0471
    fmulp st(1), st(0) ; 0473
    fadd qword ptr [rdx + 104] ; 0475
    fstp qword ptr [rcx + 16] ; 0478
    fadd qword ptr [rdx + 96] ; 047A
    fstp qword ptr [rcx + 8] ; 047D
    fadd qword ptr [rdx + 88] ; 047F
    fstp qword ptr [rcx] ; 0482
    jmp ABoxVSShapesOriginal_x87_done
aboxvsshapes_048f:
    nop ; 048F
    cmp r8d, 0fh
    jne aboxvsshapes_04c2 ; 0494
    fld st(0) ; 0496
    fmul st(0), st(0) ; 0498
    fld st(2) ; 049A
    fmul st(0), st(0) ; 049C
    faddp st(1), st(0) ; 049E
    fmul qword ptr [rdx + 56] ; 04A0
    fld st(3) ; 04A3
    fabs ; 04A5
    fcom qword ptr [rdx + 48] ; 04A7
    fnstsw ax ; 04AA
    shr ah, 1 ; 04AC
    jae aboxvsshapes_04b6 ; 04AE
    fstp st(0) ; 04B0
    fldz ; 04B2
    jmp aboxvsshapes_04bb ; 04B4
aboxvsshapes_04b6:
    fsub qword ptr [rdx + 48] ; 04B6
    fmul st(0), st(0) ; 04B9
aboxvsshapes_04bb:
    faddp st(1), st(0) ; 04BB
    jmp aboxvsshapes_0441 ; 04BD
aboxvsshapes_04c2:
    nop ; 04C2
    cmp r8d, 010h
    jne aboxvsshapes_0576 ; 04C7
    fld st(0) ; 04CD
    fmul st(0), st(0) ; 04CF
    fld st(2) ; 04D1
    fsub qword ptr [rdx + 48] ; 04D3
    fmul st(0), st(0) ; 04D6
    faddp st(1), st(0) ; 04D8
    fld st(3) ; 04DA
    fsub qword ptr [rdx + 48] ; 04DC
    fmul st(0), st(0) ; 04DF
    faddp st(1), st(0) ; 04E1
    fstp qword ptr [rcx + 64] ; 04E3
    nop ; 04E6
    fld st(0) ; 04E7
    fsub qword ptr [rdx + 48] ; 04E9
    fmul st(0), st(0) ; 04EC
    fld st(2) ; 04EE
    fmul st(0), st(0) ; 04F0
    faddp st(1), st(0) ; 04F2
    fld st(3) ; 04F4
    fsub qword ptr [rdx + 48] ; 04F6
    fmul st(0), st(0) ; 04F9
    faddp st(1), st(0) ; 04FB
    fstp qword ptr [rcx + 56] ; 04FD
    nop ; 0500
    fld st(0) ; 0501
    fsub qword ptr [rdx + 48] ; 0503
    fmul st(0), st(0) ; 0506
    fld st(2) ; 0508
    fsub qword ptr [rdx + 48] ; 050A
    fmul st(0), st(0) ; 050D
    faddp st(1), st(0) ; 050F
    fld st(3) ; 0511
    fmul st(0), st(0) ; 0513
    faddp st(1), st(0) ; 0515
    fstp qword ptr [rcx + 48] ; 0517
    nop ; 051A
    fld st(0) ; 051B
    fadd qword ptr [rdx + 48] ; 051D
    fmul st(0), st(0) ; 0520
    fld st(2) ; 0522
    fadd qword ptr [rdx + 48] ; 0524
    fmul st(0), st(0) ; 0527
    faddp st(1), st(0) ; 0529
    fld st(3) ; 052B
    fadd qword ptr [rdx + 48] ; 052D
    fmul st(0), st(0) ; 0530
    faddp st(1), st(0) ; 0532
    fstp qword ptr [rcx + 40] ; 0534
    nop ; 0537
    fld qword ptr [rcx + 64] ; 0538
    fld qword ptr [rcx + 56] ; 053B
    fcom st(1) ; 053E
    fnstsw ax ; 0540
    shr ah, 1 ; 0542
    jae aboxvsshapes_054a ; 0544
    fstp st(0) ; 0546
    jmp aboxvsshapes_054c ; 0548
aboxvsshapes_054a:
    fstp st(1) ; 054A
aboxvsshapes_054c:
    fld qword ptr [rcx + 48] ; 054C
    fcom st(1) ; 054F
    fnstsw ax ; 0551
    shr ah, 1 ; 0553
    jae aboxvsshapes_055b ; 0555
    fstp st(0) ; 0557
    jmp aboxvsshapes_055d ; 0559
aboxvsshapes_055b:
    fstp st(1) ; 055B
aboxvsshapes_055d:
    fld qword ptr [rcx + 40] ; 055D
    fcom st(1) ; 0560
    fnstsw ax ; 0562
    shr ah, 1 ; 0564
    jae aboxvsshapes_056c ; 0566
    fstp st(0) ; 0568
    jmp aboxvsshapes_056e ; 056A
aboxvsshapes_056c:
    fstp st(1) ; 056C
aboxvsshapes_056e:
    fmul qword ptr [rdx + 56] ; 056E
    jmp aboxvsshapes_0441 ; 0571
aboxvsshapes_0576:
    cmp r8d, 011h
    jne aboxvsshapes_05a5 ; 057A
    fld st(2) ; 057C
    fabs ; 057E
    fadd st(0), st(0) ; 0580
    fadd st(0), st(0) ; 0582
    fld st(3) ; 0584
    fmul st(0), st(0) ; 0586
    fadd st(0), st(0) ; 0588
    fld st(3) ; 058A
    fmul st(0), st(0) ; 058C
    faddp st(1), st(0) ; 058E
    fld st(2) ; 0590
    fmul st(0), st(0) ; 0592
    faddp st(1), st(0) ; 0594
    fdivrp st(1), st(0) ; 0596
    fmul st(0), st(0) ; 0598
    fmul qword ptr [rdx + 56] ; 059A
    fadd qword ptr [rdx + 48] ; 059D
    jmp aboxvsshapes_0441 ; 05A0
aboxvsshapes_05a5:
    jmp aboxvsshapes_0441 ; 05A5
ABoxVSShapesOriginal_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
ABoxVSShapesOriginal_x87 ENDP


; Calc.pas RMdoBinSearch inline x87 update, which deliberately differs
; from the SSE2 mAddVecWeight used for normal marching.
; rcx: XYZ, ZZ, step, direction XYZ, factor, stop; rdx: Single DEstop.
RefineRayStep_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137fh
    fldcw word ptr [rsp + 2]
    fld qword ptr [rcx + 24]
    fadd qword ptr [rcx + 32]
    fstp qword ptr [rcx + 24]
    fld qword ptr [rcx + 40]
    fld qword ptr [rcx + 48]
    fld qword ptr [rcx + 56]
    fld qword ptr [rcx + 32]
    fmul st(3), st(0)
    fmul st(2), st(0)
    fmulp st(1), st(0)
    fadd qword ptr [rcx + 16]
    fstp qword ptr [rcx + 16]
    fadd qword ptr [rcx + 8]
    fstp qword ptr [rcx + 8]
    fadd qword ptr [rcx]
    fstp qword ptr [rcx]
    fld qword ptr [rcx + 24]
    fmul qword ptr [rcx + 64]
    fld1
    faddp st(1), st(0)
    fmul qword ptr [rcx + 72]
    fstp dword ptr [rdx]
    fldcw word ptr [rsp]
    add rsp, 8
    ret
RefineRayStep_x87 ENDP


; Original ManowarBulb 003B..0153. rcx: XYZW,SSE01,scratch[10]; rdx: expanded params and Julia C.
ManowarBulbOriginal_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    movupd xmm0, xmmword ptr [rcx + 32]
    movupd xmmword ptr [rcx + 88], xmm0
    movupd xmm1, xmmword ptr [rcx + 48]
    movupd xmmword ptr [rcx + 72], xmm1
    fld qword ptr [rcx] ; 0045
    fmul st(0), st(0) ; 0047
    fld qword ptr [rcx + 8] ; 0049
    fmul st(0), st(0) ; 004B
    fld qword ptr [rcx + 16] ; 004D
    fmul st(0), st(0) ; 004F
    faddp st(1), st(0) ; 0051
    faddp st(1), st(0) ; 0053
    fsqrt ; 0055
    fld qword ptr [rdx + 0] ; 0057
    fxch st(1) ; 005A
    fabs ; 005C
    fyl2x ; 005E
    fld1 ; 0060
    fld st(1) ; 0062
    fprem ; 0064
    f2xm1 ; 0066
    faddp st(1), st(0) ; 0068
    fscale ; 006A
    fxch st(1) ; 006C
    fstp st(0) ; 006E
    fstp qword ptr [rcx + 104] ; 0070
    fld qword ptr [rcx + 8] ; 0073
    fld qword ptr [rcx] ; 0075
    fpatan ; 0077
    fmul qword ptr [rdx + 0] ; 0079
    fsincos ; 007C
    fstp qword ptr [rcx + 128] ; 007E
    fstp qword ptr [rcx + 136] ; 0081
    fld qword ptr [rcx + 16] ; 0084
    fld qword ptr [rcx + 8] ; 0086
    fmul st(0), st(0) ; 0088
    fld qword ptr [rcx] ; 008A
    fmul st(0), st(0) ; 008C
    faddp st(1), st(0) ; 008E
    fsqrt ; 0090
    fpatan ; 0092
    fmul qword ptr [rdx + 0] ; 0094
    fsincos ; 0097
    fstp qword ptr [rcx + 112] ; 0099
    fstp qword ptr [rcx + 120] ; 009C
    fld qword ptr [rdx + 16] ; 009F
    fld qword ptr [rcx + 80] ; 00A2
    fmul st(0), st(1) ; 00A5
    fld qword ptr [rcx + 96] ; 00A7
    fmul st(0), st(2) ; 00AA
    fld qword ptr [rcx + 88] ; 00AC
    fmul st(0), st(3) ; 00AF
    fld st(0) ; 00B1
    fmul qword ptr [rdx + 80] ; 00B3
    fld st(2) ; 00B6
    fmul qword ptr [rdx + 88] ; 00B8
    faddp st(1), st(0) ; 00BB
    fld st(3) ; 00BD
    fmul qword ptr [rdx + 96] ; 00BF
    faddp st(1), st(0) ; 00C2
    fld st(1) ; 00C4
    fmul qword ptr [rdx + 32] ; 00C6
    fld st(3) ; 00C9
    fmul qword ptr [rdx + 40] ; 00CB
    faddp st(1), st(0) ; 00CE
    fld st(4) ; 00D0
    fmul qword ptr [rdx + 48] ; 00D2
    faddp st(1), st(0) ; 00D5
    fxch st(2) ; 00D7
    fmul qword ptr [rdx + 56] ; 00D9
    fxch st(3) ; 00DC
    fmul qword ptr [rdx + 64] ; 00DE
    faddp st(3), st(0) ; 00E1
    fxch st(3) ; 00E3
    fmul qword ptr [rdx + 72] ; 00E5
    faddp st(2), st(0) ; 00E8
    fadd qword ptr [rdx + 104] ; 00EA
    fstp qword ptr [rcx + 96] ; 00ED
    fadd qword ptr [rdx + 112] ; 00F0
    fstp qword ptr [rcx + 88] ; 00F3
    fadd qword ptr [rdx + 120] ; 00F6
    fstp qword ptr [rcx + 80] ; 00F9
    fstp st(0) ; 00FC
    fld qword ptr [rdx + 24] ; 00FE
    fld qword ptr [rcx] ; 0101
    fmul st(0), st(1) ; 0103
    fld qword ptr [rcx + 8] ; 0105
    fmul st(0), st(2) ; 0107
    fld qword ptr [rcx + 16] ; 0109
    fmul st(0), st(3) ; 010B
    fadd qword ptr [rcx + 80] ; 010D
    fstp qword ptr [rcx + 80] ; 0110
    fadd qword ptr [rcx + 88] ; 0113
    fstp qword ptr [rcx + 88] ; 0116
    fadd qword ptr [rcx + 96] ; 0119
    fstp qword ptr [rcx + 96] ; 011C
    fstp st(0) ; 011F
    movupd xmm0, xmmword ptr [rcx]
    movupd xmmword ptr [rcx + 32], xmm0
    movupd xmm1, xmmword ptr [rcx + 16]
    movupd xmmword ptr [rcx + 48], xmm1
    fld qword ptr [rcx + 128] ; 0129
    fmul qword ptr [rcx + 112] ; 012C
    fmul qword ptr [rcx + 104] ; 012F
    fadd qword ptr [rcx + 96] ; 0132
    fstp qword ptr [rcx] ; 0135
    fld qword ptr [rcx + 136] ; 0137
    fmul qword ptr [rcx + 112] ; 013A
    fmul qword ptr [rcx + 104] ; 013D
    fadd qword ptr [rcx + 88] ; 0140
    fstp qword ptr [rcx + 8] ; 0143
    fld qword ptr [rcx + 120] ; 0145
    fmul qword ptr [rcx + 104] ; 0148
    fmul qword ptr [rdx + 8] ; 014B
    fadd qword ptr [rcx + 80] ; 014E
    fstp qword ptr [rcx + 16] ; 0151
    fnop ; 0153
ManowarBulbOriginal_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
ManowarBulbOriginal_x87 ENDP


; Original MsltoeSym4 000E..009F. rcx XYZ; rdx params and Julia C.
MsltoeSym4Original_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    fld qword ptr [rcx] ; 000E
    fld qword ptr [rcx + 16] ; 0010
    fld st(0) ; 0012
    fabs ; 0014
    fmul qword ptr [rdx + 0] ; 0016
    fld st(2) ; 0019
    fabs ; 001B
    fcompp ; 001D
    fnstsw ax ; 001F
    and ah, 041h ; 0021
    je msltoe4_0028 ; 0024
    fxch st(1) ; 0026
msltoe4_0028:
    fld qword ptr [rcx + 8] ; 0028
    fld st(0) ; 002A
    fabs ; 002C
    fmul qword ptr [rdx + 8] ; 002E
    fld st(3) ; 0031
    fabs ; 0033
    fcompp ; 0035
    fnstsw ax ; 0037
    and ah, 041h ; 0039
    je msltoe4_0040 ; 003C
    fxch st(2) ; 003E
msltoe4_0040:
    fld st(1) ; 0040
    fabs ; 0042
    fmul qword ptr [rdx + 16] ; 0044
    fld st(2) ; 0047
    fabs ; 0049
    fcompp ; 004B
    fnstsw ax ; 004D
    and ah, 041h ; 004F
    je msltoe4_0056 ; 0052
    fxch st(1) ; 0054
msltoe4_0056:
    fld st(1) ; 0056
    fmul st(0), st(3) ; 0058
    ftst ; 005A
    fnstsw ax ; 005C
    fstp st(0) ; 005E
    shr ah, 1 ; 0060
    jb msltoe4_006a ; 0062
    fxch st(1) ; 0064
    fchs ; 0066
    fxch st(1) ; 0068
msltoe4_006a:
    fld st(0) ; 006A
    fmul st(0), st(3) ; 006C
    ftst ; 006E
    fnstsw ax ; 0070
    fstp st(0) ; 0072
    shr ah, 1 ; 0074
    jb msltoe4_007a ; 0076
    fchs ; 0078
msltoe4_007a:
    fld st(2) ; 007A
    fmul st(0), st(0) ; 007C
    fld st(1) ; 007E
    fmul st(0), st(0) ; 0080
    fsubp st(1), st(0) ; 0082
    fld st(2) ; 0084
    fmul st(0), st(0) ; 0086
    fsubp st(1), st(0) ; 0088
    fadd qword ptr [rdx + 24] ; 008A
    fstp qword ptr [rcx] ; 008D
    fmul st(0), st(2) ; 008F
    fadd st(0), st(0) ; 0091
    fadd qword ptr [rdx + 32] ; 0093
    fstp qword ptr [rcx + 8] ; 0096
    fmulp st(1), st(0) ; 0098
    fadd st(0), st(0) ; 009A
    fadd qword ptr [rdx + 40] ; 009C
    fstp qword ptr [rcx + 16] ; 009F
MsltoeSym4Original_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
MsltoeSym4Original_x87 ENDP


; Original TalisTpx 0016..01DC. rcx XYZ and scratch[9]; rdx params/C; r8d integer power.
TalisTpxOriginal_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    mov eax, r8d
    cmp eax, 016h ; 0019
    jg talistpx_0023 ; 001C
    cmp eax, 1 ; 001E
    jg talistpx_0028 ; 0021
talistpx_0023:
    mov eax, 8 ; 0023
talistpx_0028:
    fnop ; 0028
    fld qword ptr [rcx] ; 002A
    fld qword ptr [rcx + 8] ; 002C
    fld qword ptr [rcx + 16] ; 002E
    fstp qword ptr [rcx + 72] ; 0030
    fstp qword ptr [rcx + 80] ; 0033
    fstp qword ptr [rcx + 88] ; 0036
    dec eax ; 0039
    fnop ; 003A
    fld qword ptr [rcx] ; 003C
    fmul st(0), st(0) ; 003E
    fld qword ptr [rcx + 8] ; 0040
    fmul st(0), st(0) ; 0042
    faddp st(1), st(0) ; 0044
    fsqrt ; 0046
    fstp qword ptr [rcx + 40] ; 0048
    nop ; 004B
    nop ; 004C
talistpx_004d:
    fnop ; 004D
    fld qword ptr [rcx + 88] ; 004F
    fmul st(0), st(0) ; 0052
    fld qword ptr [rcx + 80] ; 0054
    fmul st(0), st(0) ; 0057
    faddp st(1), st(0) ; 0059
    fsqrt ; 005B
    fst qword ptr [rcx + 32] ; 005D
    fnop ; 0060
    fmul qword ptr [rcx + 16] ; 0062
    fld qword ptr [rcx + 40] ; 0064
    fmul qword ptr [rcx + 72] ; 0067
    fsubp st(1), st(0) ; 006A
    fmul qword ptr [rdx + 0] ; 006C
    fnop ; 006F
    fld qword ptr [rcx + 72] ; 0071
    fmul qword ptr [rcx + 16] ; 0074
    fdiv qword ptr [rcx + 40] ; 0076
    fdiv qword ptr [rcx + 32] ; 0079
    fchs ; 007C
    fld1 ; 007E
    faddp st(1), st(0) ; 0080
    fstp qword ptr [rcx + 32] ; 0082
    fnop ; 0085
    fld qword ptr [rcx] ; 0087
    fmul qword ptr [rcx + 88] ; 0089
    fld qword ptr [rcx + 8] ; 008C
    fmul qword ptr [rcx + 80] ; 008E
    fsubp st(1), st(0) ; 0091
    fmul qword ptr [rcx + 32] ; 0093
    fnop ; 0096
    fld qword ptr [rcx + 8] ; 0098
    fmul qword ptr [rcx + 88] ; 009A
    fld qword ptr [rcx] ; 009D
    fmul qword ptr [rcx + 80] ; 009F
    faddp st(1), st(0) ; 00A2
    fmul qword ptr [rcx + 32] ; 00A4
    fnop ; 00A7
    fstp qword ptr [rcx + 80] ; 00A9
    fstp qword ptr [rcx + 88] ; 00AC
    fstp qword ptr [rcx + 72] ; 00AF
    dec eax ; 00B2
    cmp eax, 0 ; 00B3
    jg talistpx_004d ; 00B6
    nop ; 00B8
    nop ; 00B9
    nop ; 00BA
    nop ; 00BB
    nop ; 00BC
    nop ; 00BD
    nop ; 00BE
    fld qword ptr [rcx + 88] ; 00BF
    fmul qword ptr [rdx + 32] ; 00C2
    fld qword ptr [rcx] ; 00C5
    fmul qword ptr [rdx + 40] ; 00C7
    fld qword ptr [rdx + 64] ; 00CA
    fmul qword ptr [rdx + 48] ; 00CD
    faddp st(1), st(0) ; 00D0
    faddp st(1), st(0) ; 00D2
    fstp qword ptr [rcx + 88] ; 00D4
    nop ; 00D7
    fld qword ptr [rcx + 80] ; 00D8
    fmul qword ptr [rdx + 32] ; 00DB
    fld qword ptr [rcx + 8] ; 00DE
    fmul qword ptr [rdx + 40] ; 00E0
    fld qword ptr [rdx + 72] ; 00E3
    fmul qword ptr [rdx + 48] ; 00E6
    faddp st(1), st(0) ; 00E9
    faddp st(1), st(0) ; 00EB
    fstp qword ptr [rcx + 80] ; 00ED
    nop ; 00F0
    fld qword ptr [rcx + 72] ; 00F1
    fmul qword ptr [rdx + 32] ; 00F4
    fld qword ptr [rcx + 16] ; 00F7
    fmul qword ptr [rdx + 40] ; 00F9
    fld qword ptr [rdx + 80] ; 00FC
    fmul qword ptr [rdx + 48] ; 00FF
    faddp st(1), st(0) ; 0102
    faddp st(1), st(0) ; 0104
    fstp qword ptr [rcx + 72] ; 0106
    nop ; 0109
    fld qword ptr [rdx + 8] ; 010A
    fadd qword ptr [rcx] ; 010D
    nop ; 010F
    fstp qword ptr [rcx + 64] ; 0110
    nop ; 0113
    fld qword ptr [rdx + 16] ; 0114
    fadd qword ptr [rcx + 8] ; 0117
    nop ; 0119
    fstp qword ptr [rcx + 56] ; 011A
    nop ; 011D
    fld qword ptr [rdx + 24] ; 011E
    fadd qword ptr [rcx + 16] ; 0121
    nop ; 0123
    fstp qword ptr [rcx + 48] ; 0124
    nop ; 0127
    fnop ; 0128
    fld qword ptr [rcx + 64] ; 012A
    fmul st(0), st(0) ; 012D
    fld qword ptr [rcx + 56] ; 012F
    fmul st(0), st(0) ; 0132
    fld qword ptr [rcx + 48] ; 0134
    fmul st(0), st(0) ; 0137
    faddp st(1), st(0) ; 0139
    faddp st(1), st(0) ; 013B
    fstp qword ptr [rcx + 24] ; 013D
    nop ; 0140
    fld qword ptr [rcx + 88] ; 0141
    fmul st(0), st(0) ; 0144
    fld qword ptr [rcx + 80] ; 0146
    fmul st(0), st(0) ; 0149
    faddp st(1), st(0) ; 014B
    fsqrt ; 014D
    fstp qword ptr [rcx + 40] ; 014F
    nop ; 0152
    fld qword ptr [rcx + 64] ; 0153
    fmul st(0), st(0) ; 0156
    fld qword ptr [rcx + 56] ; 0158
    fmul st(0), st(0) ; 015B
    faddp st(1), st(0) ; 015D
    fsqrt ; 015F
    fstp qword ptr [rcx + 32] ; 0161
    nop ; 0164
    fld qword ptr [rcx + 32] ; 0165
    fmul qword ptr [rcx + 72] ; 0168
    fld qword ptr [rcx + 40] ; 016B
    fmul qword ptr [rcx + 48] ; 016E
    fsubp st(1), st(0) ; 0171
    fdiv qword ptr [rcx + 24] ; 0173
    fmul qword ptr [rdx + 0] ; 0176
    fstp qword ptr [rcx + 16] ; 0179
    fnop ; 017B
    fld qword ptr [rcx + 72] ; 017D
    fmul qword ptr [rcx + 48] ; 0180
    fdiv qword ptr [rcx + 32] ; 0183
    fdiv qword ptr [rcx + 40] ; 0186
    fld1 ; 0189
    faddp st(1), st(0) ; 018B
    fdiv qword ptr [rcx + 24] ; 018D
    fstp qword ptr [rcx + 24] ; 0190
    nop ; 0193
    fld qword ptr [rcx + 88] ; 0194
    fmul qword ptr [rcx + 64] ; 0197
    fld qword ptr [rcx + 80] ; 019A
    fmul qword ptr [rcx + 56] ; 019D
    faddp st(1), st(0) ; 01A0
    fmul qword ptr [rcx + 24] ; 01A2
    fstp qword ptr [rcx] ; 01A5
    fnop ; 01A7
    fld qword ptr [rcx + 88] ; 01A9
    fmul qword ptr [rcx + 56] ; 01AC
    fld qword ptr [rcx + 64] ; 01AF
    fmul qword ptr [rcx + 80] ; 01B2
    fsubrp st(1), st(0) ; 01B5
    fmul qword ptr [rcx + 24] ; 01B7
    fstp qword ptr [rcx + 8] ; 01BA
    fnop ; 01BC
    fld1 ; 01BE
    fsub qword ptr [rdx + 48] ; 01C0
    fld qword ptr [rdx + 64] ; 01C3
    fmul st(0), st(1) ; 01C6
    fadd qword ptr [rcx] ; 01C8
    fstp qword ptr [rcx] ; 01CA
    fld qword ptr [rdx + 72] ; 01CC
    fmul st(0), st(1) ; 01CF
    fadd qword ptr [rcx + 8] ; 01D1
    fstp qword ptr [rcx + 8] ; 01D3
    fld qword ptr [rdx + 80] ; 01D5
    fmulp st(1), st(0) ; 01D8
    fadd qword ptr [rcx + 16] ; 01DA
    fstp qword ptr [rcx + 16] ; 01DC
TalisTpxOriginal_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
TalisTpxOriginal_x87 ENDP


; Original RoomDoorsIFS 0038..0273. r8 local scratch/output, r9 params; preserves original unsigned door division and roof trap.
RoomDoorsOriginal_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    mov r8, rcx
    mov r9, rdx
    fld qword ptr [r8 + 16] ; 0038
    fld qword ptr [r8 + 0] ; 003C
    fpatan ; 003F
    fsubr qword ptr [r9 + 168] ; 0041
    fmul qword ptr [r9 + 152] ; 0044
    fimul dword ptr [r8 + 100] ; 0047
    fistp dword ptr [r8 + 40] ; 004A
    fild dword ptr [r8 + 40] ; 004E
    fidiv dword ptr [r8 + 100] ; 0052
    fmul qword ptr [r9 + 160] ; 0055
    fsincos ; 0058
    fstp qword ptr [r8 + 24] ; 005A
    fstp qword ptr [r8 + 32] ; 005E
    fld qword ptr [r8 + 16] ; 0062
    fmul qword ptr [r8 + 24] ; 0066
    fld qword ptr [r8 + 0] ; 006A
    fmul qword ptr [r8 + 32] ; 006D
    fchs ; 0071
    faddp st(1), st(0) ; 0073
    fadd qword ptr [r9 + 48] ; 0075
    fld qword ptr [r8 + 16] ; 0078
    fmul qword ptr [r8 + 32] ; 007C
    fld qword ptr [r8 + 0] ; 0080
    fmul qword ptr [r8 + 24] ; 0083
    faddp st(1), st(0) ; 0087
    fadd qword ptr [r9 + 40] ; 0089
    fstp qword ptr [r8 + 0] ; 008C
    fstp qword ptr [r8 + 16] ; 008F
    fld qword ptr [r8 + 8] ; 0093
    fadd qword ptr [r9 + 32] ; 0097
    fstp qword ptr [r8 + 8] ; 009A
    movupd xmm0, xmmword ptr [r8 + 0] ; 009E
    movsd xmm1, qword ptr [r8 + 16] ; 00A3
    cmp dword ptr [r8 + 100], 0 ; 00A9
    jl roomdoors_00b9 ; 00AD
    movupd xmmword ptr [r8 + 56], xmm0 ; 00AF
    movsd qword ptr [r8 + 48], xmm1 ; 00B4
roomdoors_00b9:
    fld qword ptr [r8 + 8] ; 00B9
    fmul qword ptr [r9 + 72] ; 00BD
    fld qword ptr [r8 + 16] ; 00C0
    fmul qword ptr [r9 + 80] ; 00C4
    faddp st(1), st(0) ; 00C7
    fld qword ptr [r8 + 8] ; 00C9
    fmul qword ptr [r9 + 80] ; 00CD
    fld qword ptr [r8 + 16] ; 00D0
    fmul qword ptr [r9 + 72] ; 00D4
    fchs ; 00D7
    faddp st(1), st(0) ; 00D9
    fstp qword ptr [r8 + 8] ; 00DB
    fst qword ptr [r8 + 32] ; 00DF
    fstp qword ptr [r8 + 16] ; 00E3
    fld qword ptr [r8 + 16] ; 00E7
    fabs ; 00EB
    fld qword ptr [r9 + 0] ; 00ED
    fabs ; 00F0
    fsubp st(1), st(0) ; 00F2
    fchs ; 00F4
    movupd xmmword ptr [r8 + 0], xmm0 ; 00F6
    movsd qword ptr [r8 + 16], xmm1 ; 00FB
    mov eax, dword ptr [r8 + 40] ; 0101
    add eax, 0100h ; 0105
    xor edx, edx ; 010A
    mov ecx, dword ptr [r8 + 104] ; 010C
    cmp ecx, 0 ; 010F
    jle roomdoors_011f ; 0112
    div ecx ; 0114
    cmp edx, 1 ; 0116
    jne roomdoors_01b2 ; 0119
roomdoors_011f:
    fld qword ptr [r8 + 0] ; 011F
    fabs ; 0122
    fmul qword ptr [r9 + 96] ; 0124
    fld qword ptr [r8 + 8] ; 0127
    fmul qword ptr [r9 + 88] ; 012B
    fchs ; 012E
    faddp st(1), st(0) ; 0130
    fsub qword ptr [r9 + 64] ; 0132
    fld qword ptr [r8 + 8] ; 0135
    fld qword ptr [r9 + 16] ; 0139
    fchs ; 013C
    fsubrp st(1), st(0) ; 013E
    fld st(0) ; 0140
    fld qword ptr [r9 + 112] ; 0142
    fldz ; 0145
    fcompp ; 0147
    wait ; 0149
    fnstsw ax ; 014A
    shr ah, 1 ; 014C
    ja roomdoors_015f ; 014E
    fld qword ptr [r9 + 112] ; 0150
    fstp qword ptr [r8 + 24] ; 0153
    fldz ; 0157
    fstp qword ptr [r8 + 40] ; 0159
    jmp roomdoors_0170 ; 015D
roomdoors_015f:
    fld qword ptr [r9 + 64] ; 015F
    fsub qword ptr [r9 + 112] ; 0162
    fstp qword ptr [r8 + 24] ; 0165
    fld qword ptr [r9 + 112] ; 0169
    fstp qword ptr [r8 + 40] ; 016C
roomdoors_0170:
    fmul st(0), st(0) ; 0170
    fld qword ptr [r8 + 0] ; 0172
    fabs ; 0175
    fsub qword ptr [r8 + 40] ; 0177
    fmul st(0), st(0) ; 017B
    faddp st(1), st(0) ; 017D
    fsqrt ; 017F
    fsub qword ptr [r8 + 24] ; 0181
    fcom st(1) ; 0185
    wait ; 0187
    fnstsw ax ; 0188
    shr ah, 1 ; 018A
    jb roomdoors_0192 ; 018C
    fstp st(0) ; 018E
    jmp roomdoors_0194 ; 0190
roomdoors_0192:
    fstp st(1) ; 0192
roomdoors_0194:
    fcom st(1) ; 0194
    wait ; 0196
    fnstsw ax ; 0197
    shr ah, 1 ; 0199
    jae roomdoors_01a1 ; 019B
    fstp st(0) ; 019D
    jmp roomdoors_01a3 ; 019F
roomdoors_01a1:
    fstp st(1) ; 01A1
roomdoors_01a3:
    fcom st(1) ; 01A3
    wait ; 01A5
    fnstsw ax ; 01A6
    shr ah, 1 ; 01A8
    jae roomdoors_01b0 ; 01AA
    fstp st(1) ; 01AC
    jmp roomdoors_01b2 ; 01AE
roomdoors_01b0:
    fstp st(0) ; 01B0
roomdoors_01b2:
    fchs ; 01B2
    xor ecx, ecx ; 01B4
    mov edx, dword ptr [r9 + 4] ; 01B6
    and edx, 080000000h ; 01B9
    je roomdoors_022c ; 01BF
    fld qword ptr [r8 + 8] ; 01C1
    fadd qword ptr [r9 + 8] ; 01C5
    fabs ; 01C8
    fadd qword ptr [r9 + 0] ; 01CA
    fld qword ptr [r8 + 32] ; 01CD
    fadd qword ptr [r9 + 0] ; 01D1
    fcom st(1) ; 01D4
    wait ; 01D6
    fnstsw ax ; 01D7
    shr ah, 1 ; 01D9
    jb roomdoors_01e1 ; 01DB
    fstp st(1) ; 01DD
    jmp roomdoors_01e3 ; 01DF
roomdoors_01e1:
    fstp st(0) ; 01E1
roomdoors_01e3:
    fld st(0) ; 01E3
    fld qword ptr [r9 + 176] ; 01E5
    fmul qword ptr [r8 + 88] ; 01E8
    faddp st(1), st(0) ; 01EE
    fldz ; 01F0
    fcompp ; 01F2
    wait ; 01F4
    fnstsw ax ; 01F5
    shr ah, 1 ; 01F7
    jb roomdoors_021d ; 01F9
    fld qword ptr [r9 + 136] ; 01FB
    fldz ; 0201
    fcomp st(1) ; 0203
    wait ; 0205
    fnstsw ax ; 0206
    shr ah, 1 ; 0208
    jb roomdoors_0212 ; 020A
    xor ecx, ecx ; 020C
    fstp st(0) ; 020E
    jmp roomdoors_021d ; 0210
roomdoors_0212:
    mov ecx, 1 ; 0212
    fstp qword ptr [r8 + 80] ; 0217
roomdoors_021d:
    fcom st(1) ; 021D
    wait ; 021F
    fnstsw ax ; 0220
    shr ah, 1 ; 0222
    jae roomdoors_022a ; 0224
    fstp st(1) ; 0226
    jmp roomdoors_022c ; 0228
roomdoors_022a:
    fstp st(0) ; 022A
roomdoors_022c:
    fld qword ptr [r8 + 8] ; 022C
    fld qword ptr [r9 + 8] ; 0230
    fmul qword ptr [r9 + 144] ; 0233
    fst qword ptr [r8 + 24] ; 0235
    faddp st(1), st(0) ; 0239
    fabs ; 023B
    fsub qword ptr [r8 + 24] ; 023D
    fcom st(1) ; 0241
    wait ; 0243
    fnstsw ax ; 0244
    shr ah, 1 ; 0246
    jae roomdoors_024e ; 0248
    fstp st(0) ; 024A
    jmp roomdoors_0250 ; 024C
roomdoors_024e:
    fstp st(1) ; 024E
roomdoors_0250:
    cmp ecx, 0 ; 0250
    jne roomdoors_026a ; 0253
    fild dword ptr [r8 + 96] ; 0255
    fmul qword ptr [r9 + 120] ; 0258
    fadd qword ptr [r9 + 128] ; 025E
    fstp qword ptr [r8 + 80] ; 0264
roomdoors_026a:
    cmp dword ptr [r8 + 100], 0 ; 026A
    jge roomdoors_0273 ; 026E
    fdiv qword ptr [r9 + 24] ; 0270
roomdoors_0273:
    fstp qword ptr [r8 + 72] ; 0273
RoomDoorsOriginal_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
RoomDoorsOriginal_x87 ENDP


; Original gyroidIFS 004F..0124; shifted rotation base folded into mapping. rcx local XYZ,DE,trap; rdx params; r8d Apply.
GyroidDistance_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    fld qword ptr [rcx + 16] ; 004F
    fld qword ptr [rcx + 8] ; 0053
    fld qword ptr [rcx] ; 0057
    fld st(0) ; 005A
    fmul qword ptr [rdx + 104] ; 005C
    fld st(2) ; 005F
    fmul qword ptr [rdx + 112] ; 0061
    faddp st(1), st(0) ; 0064
    fld st(3) ; 0066
    fmul qword ptr [rdx + 120] ; 0068
    faddp st(1), st(0) ; 006B
    fld st(1) ; 006D
    fmul qword ptr [rdx + 56] ; 006F
    fld st(3) ; 0072
    fmul qword ptr [rdx + 64] ; 0074
    faddp st(1), st(0) ; 0077
    fld st(4) ; 0079
    fmul qword ptr [rdx + 72] ; 007B
    faddp st(1), st(0) ; 007E
    fxch st(2) ; 0080
    fmul qword ptr [rdx + 80] ; 0082
    fxch st(3) ; 0085
    fmul qword ptr [rdx + 88] ; 0087
    faddp st(3), st(0) ; 008A
    fxch st(3) ; 008C
    fmul qword ptr [rdx + 96] ; 008E
    faddp st(2), st(0) ; 0091
    fstp qword ptr [rcx] ; 0093
    fstp qword ptr [rcx + 8] ; 0096
    fstp qword ptr [rcx + 16] ; 009A
    fld qword ptr [rcx + 16] ; 00A1
    fmul qword ptr [rdx + 40] ; 00A5
    fsincos ; 00A8
    fld qword ptr [rcx + 8] ; 00AA
    fmul qword ptr [rdx + 40] ; 00AE
    fsincos ; 00B1
    fld qword ptr [rcx] ; 00B3
    fmul qword ptr [rdx + 40] ; 00B6
    fsincos ; 00B9
    fmulp st(3), st(0) ; 00BB
    fmulp st(3), st(0) ; 00BD
    fmulp st(3), st(0) ; 00BF
    fld st(0) ; 00C1
    fsub st(0), st(2) ; 00C3
    fsub st(0), st(3) ; 00C5
    fmul qword ptr [rdx + 128] ; 00C7
    fabs ; 00CA
    fstp qword ptr [rcx + 32] ; 00CC
    faddp st(1), st(0) ; 00D2
    faddp st(1), st(0) ; 00D4
    fld qword ptr [rcx + 16] ; 00D6
    fabs ; 00DA
    fld qword ptr [rcx + 8] ; 00DC
    fabs ; 00E0
    fcom st(1) ; 00E2
    wait ; 00E4
    fnstsw ax ; 00E5
    shr ah, 1 ; 00E7
    jae gyroidifs_00ef ; 00E9
    fstp st(0) ; 00EB
    jmp gyroidifs_00f1 ; 00ED
gyroidifs_00ef:
    fstp st(1) ; 00EF
gyroidifs_00f1:
    fld qword ptr [rcx] ; 00F1
    nop ; 00F4
    fabs ; 00F5
    fcom st(1) ; 00F7
    wait ; 00F9
    fnstsw ax ; 00FA
    shr ah, 1 ; 00FC
    jae gyroidifs_0104 ; 00FE
    fstp st(0) ; 0100
    jmp gyroidifs_0106 ; 0102
gyroidifs_0104:
    fstp st(1) ; 0104
gyroidifs_0106:
    fsub qword ptr [rdx] ; 0106
    fcom st(1) ; 0109
    wait ; 010B
    fnstsw ax ; 010C
    shr ah, 1 ; 010E
    jae gyroidifs_0116 ; 0110
    fstp st(0) ; 0112
    jmp gyroidifs_0118 ; 0114
gyroidifs_0116:
    fstp st(1) ; 0116
gyroidifs_0118:
    fdiv qword ptr [rdx + 40] ; 0118
    test r8d, r8d
    jne gyroidifs_0124 ; 011F
    fdiv qword ptr [rdx + 8] ; 0121
gyroidifs_0124:
    fstp qword ptr [rcx + 24] ; 0124
GyroidDistance_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
GyroidDistance_x87 ENDP


; Original _kamtorcut 000D..0156. rcx XYZ; rdx expanded parameters/constants.
KamtorcutOriginal_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    fld qword ptr [rcx] ; 000D
    fmul st(0), st(0) ; 000F
    fld qword ptr [rcx + 8] ; 0011
    fmul st(0), st(0) ; 0013
    faddp st(1), st(0) ; 0015
    fsqrt ; 0017
    fmul qword ptr [rdx + 8] ; 0019
    fld qword ptr [rcx + 16] ; 001C
    fmul qword ptr [rdx + 0] ; 001E
    faddp st(1), st(0) ; 0021
    fstp qword ptr [rcx + 16] ; 0023
    fld qword ptr [rcx + 16] ; 0025
    fabs ; 0027
    fld qword ptr [rdx + 120] ; 0029
    nop ; 002C
    fcompp ; 002D
    fnstsw ax ; 002F
    and ah, 041h ; 0031
    jg kamtorcut_0046 ; 0034
    fld qword ptr [rcx + 16] ; 0036
    fld st(0) ; 0038
    fmul st(0), st(0) ; 003A
    fchs ; 003C
    fld1 ; 003E
    faddp st(1), st(0) ; 0040
    fsqrt ; 0042
    jmp kamtorcut_0056 ; 0044
kamtorcut_0046:
    fld qword ptr [rdx + 112] ; 0046
    fld qword ptr [rdx + 112] ; 0048
    fstp qword ptr [rcx + 8] ; 004A
    fstp qword ptr [rcx] ; 004C
    jmp KamtorcutOriginal_x87_done
kamtorcut_0056:
    fnop ; 0056
    fld qword ptr [rcx] ; 0058
    fmul qword ptr [rdx + 16] ; 005A
    fld qword ptr [rcx + 8] ; 005D
    fmul qword ptr [rdx + 24] ; 005F
    faddp st(1), st(0) ; 0062
    fld qword ptr [rcx] ; 0064
    fmul st(0), st(0) ; 0066
    fmul qword ptr [rdx + 32] ; 0068
    faddp st(1), st(0) ; 006B
    fld qword ptr [rcx + 8] ; 006D
    fmul st(0), st(0) ; 006F
    fmul qword ptr [rdx + 40] ; 0071
    faddp st(1), st(0) ; 0074
    fld qword ptr [rcx] ; 0076
    fmul qword ptr [rcx + 8] ; 0078
    fmul qword ptr [rdx + 48] ; 007A
    faddp st(1), st(0) ; 007D
    fld qword ptr [rcx] ; 007F
    fmul st(0), st(0) ; 0081
    fld qword ptr [rcx + 8] ; 0083
    fmul st(0), st(0) ; 0085
    faddp st(1), st(0) ; 0087
    fsqrt ; 0089
    fmul qword ptr [rdx + 56] ; 008B
    faddp st(1), st(0) ; 008E
    fmul st(0), st(2) ; 0090
    nop ; 0092
    nop ; 0093
    fld qword ptr [rcx] ; 0094
    fmul qword ptr [rdx + 64] ; 0096
    fld qword ptr [rcx + 8] ; 0099
    fmul qword ptr [rdx + 72] ; 009B
    faddp st(1), st(0) ; 009E
    fld qword ptr [rcx] ; 00A0
    fmul st(0), st(0) ; 00A2
    fmul qword ptr [rdx + 80] ; 00A4
    faddp st(1), st(0) ; 00A7
    fld qword ptr [rcx + 8] ; 00A9
    fmul st(0), st(0) ; 00AB
    fmul qword ptr [rdx + 88] ; 00AD
    faddp st(1), st(0) ; 00B0
    fld qword ptr [rcx] ; 00B2
    fmul qword ptr [rcx + 8] ; 00B4
    fmul qword ptr [rdx + 96] ; 00B6
    faddp st(1), st(0) ; 00B9
    fld qword ptr [rcx] ; 00BB
    fmul st(0), st(0) ; 00BD
    fld qword ptr [rcx + 8] ; 00BF
    fmul st(0), st(0) ; 00C1
    faddp st(1), st(0) ; 00C3
    fsqrt ; 00C5
    fmul qword ptr [rdx + 104] ; 00C7
    faddp st(1), st(0) ; 00CA
    fmul st(0), st(2) ; 00CC
    faddp st(1), st(0) ; 00CE
    fnop ; 00D0
    fld qword ptr [rcx] ; 00D2
    fmul qword ptr [rdx + 16] ; 00D4
    fld qword ptr [rcx + 8] ; 00D7
    fmul qword ptr [rdx + 24] ; 00D9
    faddp st(1), st(0) ; 00DC
    fld qword ptr [rcx] ; 00DE
    fmul st(0), st(0) ; 00E0
    fmul qword ptr [rdx + 32] ; 00E2
    faddp st(1), st(0) ; 00E5
    fld qword ptr [rcx + 8] ; 00E7
    fmul st(0), st(0) ; 00E9
    fmul qword ptr [rdx + 40] ; 00EB
    faddp st(1), st(0) ; 00EE
    fld qword ptr [rcx] ; 00F0
    fmul qword ptr [rcx + 8] ; 00F2
    fmul qword ptr [rdx + 48] ; 00F4
    faddp st(1), st(0) ; 00F7
    fld qword ptr [rcx] ; 00F9
    fmul st(0), st(0) ; 00FB
    fld qword ptr [rcx + 8] ; 00FD
    fmul st(0), st(0) ; 00FF
    faddp st(1), st(0) ; 0101
    fsqrt ; 0103
    fmul qword ptr [rdx + 56] ; 0105
    faddp st(1), st(0) ; 0108
    fmul st(0), st(2) ; 010A
    nop ; 010C
    nop ; 010D
    fld qword ptr [rcx] ; 010E
    fmul qword ptr [rdx + 64] ; 0110
    fld qword ptr [rcx + 8] ; 0113
    fmul qword ptr [rdx + 72] ; 0115
    faddp st(1), st(0) ; 0118
    fld qword ptr [rcx] ; 011A
    fmul st(0), st(0) ; 011C
    fmul qword ptr [rdx + 80] ; 011E
    faddp st(1), st(0) ; 0121
    fld qword ptr [rcx + 8] ; 0123
    fmul st(0), st(0) ; 0125
    fmul qword ptr [rdx + 88] ; 0127
    faddp st(1), st(0) ; 012A
    fld qword ptr [rcx] ; 012C
    fmul qword ptr [rcx + 8] ; 012E
    fmul qword ptr [rdx + 96] ; 0130
    faddp st(1), st(0) ; 0133
    fld qword ptr [rcx] ; 0135
    fmul st(0), st(0) ; 0137
    fld qword ptr [rcx + 8] ; 0139
    fmul st(0), st(0) ; 013B
    faddp st(1), st(0) ; 013D
    fsqrt ; 013F
    fmul qword ptr [rdx + 104] ; 0141
    faddp st(1), st(0) ; 0144
    fmul st(0), st(4) ; 0146
    faddp st(1), st(0) ; 0148
    fnop ; 014A
    fstp qword ptr [rcx + 8] ; 014C
    fstp qword ptr [rcx] ; 014E
    ffree st(0) ; 0150
    ffree st(1) ; 0152
    fincstp ; 0154
    fincstp ; 0156
KamtorcutOriginal_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
KamtorcutOriginal_x87 ENDP


; Original ABoxPlatinum 0013..01F3. rcx XYZW and scratch; rdx params/C; r8d abs bits.
ABoxPlatinumOriginal_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    fld qword ptr [rdx + 24] ; 0013
    fld qword ptr [rcx] ; 0016
    fmul qword ptr [rdx + 160] ; 0018
    fld qword ptr [rcx + 16] ; 001B
    fmul qword ptr [rdx + 152] ; 001D
    fsubp st(1), st(0) ; 0020
    fld qword ptr [rcx] ; 0022
    fmul qword ptr [rdx + 152] ; 0024
    fld qword ptr [rcx + 16] ; 0027
    fmul qword ptr [rdx + 160] ; 0029
    faddp st(1), st(0) ; 002C
    fstp qword ptr [rcx + 32] ; 002E
    fld st(0) ; 0032
    fmul qword ptr [rdx + 144] ; 0034
    fld qword ptr [rcx + 8] ; 0037
    fmul qword ptr [rdx + 136] ; 0039
    fsubp st(1), st(0) ; 003C
    fxch st(1) ; 003E
    fmul qword ptr [rdx + 136] ; 0040
    fld qword ptr [rcx + 8] ; 0043
    fmul qword ptr [rdx + 144] ; 0045
    faddp st(1), st(0) ; 0048
    fxch st(1) ; 004A
    fld st(0) ; 004C
    fsub st(0), st(3) ; 004E
    fabs ; 0050
    fadd st(0), st(1) ; 0052
    fxch st(1) ; 0054
    fadd st(0), st(3) ; 0056
    fabs ; 0058
    fsubrp st(1), st(0) ; 005A
    fld st(0) ; 005C
    fmul qword ptr [rdx + 144] ; 005E
    fld st(2) ; 0061
    fmul qword ptr [rdx + 136] ; 0063
    fsubp st(1), st(0) ; 0066
    fld st(0) ; 0068
    fmul qword ptr [rdx + 160] ; 006A
    fld qword ptr [rcx + 32] ; 006D
    fmul qword ptr [rdx + 152] ; 0071
    fsubp st(1), st(0) ; 0074
    fxch st(2) ; 0076
    fmul qword ptr [rdx + 136] ; 0078
    fxch st(1) ; 007B
    fxch st(3) ; 007D
    fmul qword ptr [rdx + 144] ; 007F
    faddp st(1), st(0) ; 0082
    fxch st(2) ; 0084
    fmul qword ptr [rdx + 152] ; 0086
    fld qword ptr [rcx + 32] ; 0089
    fmul qword ptr [rdx + 160] ; 008D
    faddp st(1), st(0) ; 0090
    fld st(2) ; 0092
    fmul qword ptr [rdx + 192] ; 0094
    fld st(1) ; 0097
    fmul qword ptr [rdx + 184] ; 0099
    fsubp st(1), st(0) ; 009C
    fxch st(1) ; 009E
    fmul qword ptr [rdx + 192] ; 00A0
    fxch st(3) ; 00A3
    fmul qword ptr [rdx + 184] ; 00A5
    faddp st(3), st(0) ; 00A8
    fld st(0) ; 00AA
    fmul qword ptr [rdx + 176] ; 00AC
    fld st(2) ; 00AF
    fmul qword ptr [rdx + 168] ; 00B1
    fsubp st(1), st(0) ; 00B4
    fxch st(2) ; 00B6
    fmul qword ptr [rdx + 176] ; 00B8
    fxch st(1) ; 00BB
    fmul qword ptr [rdx + 168] ; 00BD
    faddp st(1), st(0) ; 00C0
    fxch st(1) ; 00C2
    fld st(0) ; 00C4
    fsub st(0), st(4) ; 00C6
    fabs ; 00C8
    fadd st(0), st(1) ; 00CA
    fxch st(1) ; 00CC
    fadd st(0), st(4) ; 00CE
    fabs ; 00D0
    fsubrp st(1), st(0) ; 00D2
    fld st(0) ; 00D4
    fmul qword ptr [rdx + 176] ; 00D6
    fld st(2) ; 00D9
    fmul qword ptr [rdx + 168] ; 00DB
    fsubp st(1), st(0) ; 00DE
    fld st(0) ; 00E0
    fmul qword ptr [rdx + 192] ; 00E2
    fld st(4) ; 00E5
    fmul qword ptr [rdx + 184] ; 00E7
    fsubp st(1), st(0) ; 00EA
    fld qword ptr [rdx + 168] ; 00EC
    fmulp st(3), st(0) ; 00EF
    fxch st(3) ; 00F1
    fmul qword ptr [rdx + 176] ; 00F3
    faddp st(2), st(0) ; 00F6
    fmul qword ptr [rdx + 184] ; 00F8
    fld qword ptr [rdx + 192] ; 00FB
    fmulp st(4), st(0) ; 00FE
    faddp st(3), st(0) ; 0100
    fxch st(2) ; 0102
    fld st(0) ; 0104
    fsub st(0), st(4) ; 0106
    fabs ; 0108
    fadd st(0), st(1) ; 010A
    fxch st(1) ; 010C
    fadd st(0), st(4) ; 010E
    fabs ; 0110
    fsubrp st(1), st(0) ; 0112
    mov eax, r8d
    nop ; 0117
    test eax, 4 ; 0118
    je aboxplatinum_0121 ; 011D
    fabs ; 011F
aboxplatinum_0121:
    test eax, 2 ; 0121
    je aboxplatinum_012e ; 0126
    fxch st(1) ; 0128
    fabs ; 012A
    fxch st(1) ; 012C
aboxplatinum_012e:
    test eax, 1 ; 012E
    je aboxplatinum_013b ; 0133
    fxch st(2) ; 0135
    fabs ; 0137
    fxch st(2) ; 0139
aboxplatinum_013b:
    fadd qword ptr [rdx + 120] ; 013B
    fld qword ptr [rdx + 112] ; 013E
    faddp st(2), st(0) ; 0141
    fld qword ptr [rdx + 104] ; 0143
    faddp st(3), st(0) ; 0146
    fld st(0) ; 0148
    fmul st(0), st(0) ; 014A
    fld st(2) ; 014C
    fmul st(0), st(0) ; 014E
    faddp st(1), st(0) ; 0150
    fld st(3) ; 0152
    fmul st(0), st(0) ; 0154
    faddp st(1), st(0) ; 0156
    fmul qword ptr [rdx + 128] ; 0158
    fcom qword ptr [rdx + 16] ; 015B
    wait ; 015E
    fnstsw ax ; 015F
    shr ah, 1 ; 0161
    jae aboxplatinum_016c ; 0163
    fstp st(0) ; 0165
    fld qword ptr [rdx + 8] ; 0167
    jmp aboxplatinum_0183 ; 016A
aboxplatinum_016c:
    fld1 ; 016C
    fcom st(1) ; 016E
    wait ; 0170
    fnstsw ax ; 0171
    shr ah, 1 ; 0173
    jb aboxplatinum_017e ; 0175
    fstp st(0) ; 0177
    fdivr qword ptr [rdx + 0] ; 0179
    jmp aboxplatinum_0183 ; 017C
aboxplatinum_017e:
    fcompp ; 017E
    fld qword ptr [rdx + 0] ; 0180
aboxplatinum_0183:
    fld qword ptr [rcx + 24] ; 0183
    fmul st(0), st(1) ; 0186
    fstp qword ptr [rcx + 24] ; 0188
    fmul st(3), st(0) ; 018B
    fmul st(2), st(0) ; 018D
    fmulp st(1), st(0) ; 018F
    fld qword ptr [rdx + 0] ; 0191
    fld qword ptr [rdx + 120] ; 0194
    fmul st(0), st(1) ; 0197
    fsubp st(2), st(0) ; 0199
    fld qword ptr [rdx + 112] ; 019B
    fmul st(0), st(1) ; 019E
    fsubp st(3), st(0) ; 01A0
    fmul qword ptr [rdx + 104] ; 01A2
    fsubp st(3), st(0) ; 01A5
    fxch st(2) ; 01A7
    fld st(0) ; 01A9
    fmul qword ptr [rdx + 80] ; 01AB
    fld st(2) ; 01AE
    fmul qword ptr [rdx + 88] ; 01B0
    faddp st(1), st(0) ; 01B3
    fld st(3) ; 01B5
    fmul qword ptr [rdx + 96] ; 01B7
    faddp st(1), st(0) ; 01BA
    fld st(1) ; 01BC
    fmul qword ptr [rdx + 32] ; 01BE
    fld st(3) ; 01C1
    fmul qword ptr [rdx + 40] ; 01C3
    faddp st(1), st(0) ; 01C6
    fld st(4) ; 01C8
    fmul qword ptr [rdx + 48] ; 01CA
    faddp st(1), st(0) ; 01CD
    fxch st(2) ; 01CF
    fmul qword ptr [rdx + 56] ; 01D1
    fxch st(3) ; 01D4
    fmul qword ptr [rdx + 64] ; 01D6
    faddp st(3), st(0) ; 01D9
    fxch st(3) ; 01DB
    fmul qword ptr [rdx + 72] ; 01DD
    faddp st(2), st(0) ; 01E2
    fadd qword ptr [rdx + 208]
    fstp qword ptr [rcx] ; 01E7
    fadd qword ptr [rdx + 216]
    fstp qword ptr [rcx + 8] ; 01EC
    fadd qword ptr [rdx + 224]
    fstp qword ptr [rcx + 16] ; 01F1
    fstp st(0) ; 01F3
ABoxPlatinumOriginal_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
ABoxPlatinumOriginal_x87 ENDP


; Original CrossBiFold 001B..013C after sign-bit clearing. rcx XYZW; rdx parameters.
CrossBiFoldOriginal_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    fld qword ptr [rcx + 16] ; 001B
    fld qword ptr [rcx + 8] ; 001D
    fld qword ptr [rcx] ; 001F
    fld st(0) ; 0021
    fmul qword ptr [rdx + 80] ; 0023
    fld st(2) ; 0026
    fmul qword ptr [rdx + 88] ; 0028
    faddp st(1), st(0) ; 002B
    fld st(3) ; 002D
    fmul qword ptr [rdx + 96] ; 002F
    faddp st(1), st(0) ; 0032
    fld st(1) ; 0034
    fmul qword ptr [rdx + 32] ; 0036
    fld st(3) ; 0039
    fmul qword ptr [rdx + 40] ; 003B
    faddp st(1), st(0) ; 003E
    fld st(4) ; 0040
    fmul qword ptr [rdx + 48] ; 0042
    faddp st(1), st(0) ; 0045
    fxch st(2) ; 0047
    fmul qword ptr [rdx + 56] ; 0049
    fxch st(3) ; 004C
    fmul qword ptr [rdx + 64] ; 004E
    faddp st(3), st(0) ; 0051
    fxch st(3) ; 0053
    fmul qword ptr [rdx + 72] ; 0055
    faddp st(2), st(0) ; 0058
    fcom st(1) ; 005A
    fnstsw ax ; 005C
    shr ah, 1 ; 005E
    jae crossbifold_0064 ; 0060
    fxch st(1) ; 0062
crossbifold_0064:
    fcom st(2) ; 0064
    fnstsw ax ; 0066
    shr ah, 1 ; 0068
    jae crossbifold_006e ; 006A
    fxch st(2) ; 006C
crossbifold_006e:
    fxch st(1) ; 006E
    fcom st(2) ; 0070
    fnstsw ax ; 0072
    shr ah, 1 ; 0074
    jae crossbifold_007a ; 0076
    fxch st(2) ; 0078
crossbifold_007a:
    fld qword ptr [rdx + 0] ; 007A
    fld qword ptr [rcx + 24] ; 007D
    fmul st(0), st(1) ; 0080
    fstp qword ptr [rcx + 24] ; 0082
    fmul st(3), st(0) ; 0085
    fmul st(2), st(0) ; 0087
    fmul st(1), st(0) ; 0089
    fstp st(0) ; 008B
    fstp qword ptr [rcx + 8] ; 008D
    fstp qword ptr [rcx] ; 008F
    fstp qword ptr [rcx + 16] ; 0091
    fld1 ; 0093
    fsubr qword ptr [rdx + 0] ; 0095
    fmul qword ptr [rdx + 184] ; 0098
    fld qword ptr [rcx] ; 009B
    fsubrp st(1), st(0) ; 009D
    fstp qword ptr [rcx] ; 009F
    fld1 ; 00A1
    fsubr qword ptr [rdx + 0] ; 00A3
    fmul qword ptr [rdx + 176] ; 00A6
    fld qword ptr [rcx] ; 00A9
    fadd st(0), st(1) ; 00AB
    fabs ; 00AD
    fld qword ptr [rcx] ; 00AF
    fsub st(0), st(2) ; 00B1
    fabs ; 00B3
    fchs ; 00B5
    fsub qword ptr [rcx] ; 00B7
    faddp st(1), st(0) ; 00B9
    fstp qword ptr [rcx] ; 00BB
    fstp st(0) ; 00BD
    fld qword ptr [rcx + 16] ; 00BF
    fld qword ptr [rcx] ; 00C1
    fld qword ptr [rcx + 8] ; 00C3
    fcom st(1) ; 00C5
    fnstsw ax ; 00C7
    shr ah, 1 ; 00C9
    jae crossbifold_00cf ; 00CB
    fxch st(1) ; 00CD
crossbifold_00cf:
    fcom st(2) ; 00CF
    fnstsw ax ; 00D1
    shr ah, 1 ; 00D3
    jae crossbifold_00d9 ; 00D5
    fxch st(2) ; 00D7
crossbifold_00d9:
    fxch st(1) ; 00D9
    fcom st(2) ; 00DB
    fnstsw ax ; 00DD
    shr ah, 1 ; 00DF
    jae crossbifold_00e5 ; 00E1
    fxch st(2) ; 00E3
crossbifold_00e5:
    fld st(1) ; 00E5
    fmul qword ptr [rdx + 152] ; 00E7
    fld st(1) ; 00EA
    fmul qword ptr [rdx + 160] ; 00EC
    faddp st(1), st(0) ; 00EF
    fld st(3) ; 00F1
    fmul qword ptr [rdx + 168] ; 00F3
    faddp st(1), st(0) ; 00F6
    fld st(2) ; 00F8
    fmul qword ptr [rdx + 104] ; 00FA
    fld st(2) ; 00FD
    fmul qword ptr [rdx + 112] ; 00FF
    faddp st(1), st(0) ; 0102
    fld st(4) ; 0104
    fmul qword ptr [rdx + 120] ; 0106
    faddp st(1), st(0) ; 0109
    fxch st(3) ; 010B
    fmul qword ptr [rdx + 128] ; 010D
    fxch st(2) ; 0110
    fmul qword ptr [rdx + 136] ; 0112
    faddp st(2), st(0) ; 0115
    fxch st(3) ; 0117
    fmul qword ptr [rdx + 144] ; 0119
    faddp st(1), st(0) ; 011C
    fld qword ptr [rdx + 0] ; 011E
    fld1 ; 0121
    fsubp st(1), st(0) ; 0123
    fld st(0) ; 0125
    fld st(0) ; 0127
    fmul qword ptr [rdx + 24] ; 0129
    fsubp st(4), st(0) ; 012C
    fmul qword ptr [rdx + 16] ; 012E
    fsubp st(2), st(0) ; 0131
    fmul qword ptr [rdx + 8] ; 0133
    fsubrp st(3), st(0) ; 0136
    fstp qword ptr [rcx] ; 0138
    fstp qword ptr [rcx + 8] ; 013A
    fstp qword ptr [rcx + 16] ; 013C
CrossBiFoldOriginal_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
CrossBiFoldOriginal_x87 ENDP

; formulas.pas CalcSmoothIterations executable assembly. Keep intermediate
; logarithms on the x87 stack: rounding them to double changes cancellation.
CalcSmoothIterationsOriginal_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp+2], 137Fh
    fldcw word ptr [rsp+2]
    cmp dword ptr [rcx+4], 3FF00000h
    jg smooth_radius
    fild dword ptr [rcx+40]
    fstp dword ptr [rcx+44]
    jmp smooth_done
smooth_radius:
    fld qword ptr [rcx]
    cmp dword ptr [rcx+12], 3FF00000h
    jnb smooth_old_radius
    fldln2
    fxch st(1)
    fyl2x
    fmul qword ptr [rcx+16]
    fldln2
    fxch st(1)
    fyl2x
    fmul dword ptr [rcx+36]
    fild dword ptr [rcx+40]
    fadd dword ptr [rcx+32]
    fsubrp st(1), st(0)
    fstp dword ptr [rcx+44]
    jmp smooth_done
smooth_old_radius:
    fldln2
    fxch st(1)
    fyl2x
    fmul qword ptr [rcx+16]
    fldln2
    fxch st(1)
    fyl2x
    fldln2
    fld qword ptr [rcx+8]
    fyl2x
    fmul qword ptr [rcx+16]
    fldln2
    fxch st(1)
    fyl2x
    fsubr st(0), st(1)
    fld dword ptr [rcx+32]
    fsubrp st(2), st(0)
    fadd qword ptr [rcx+24]
    fdivp st(1), st(0)
    fiadd dword ptr [rcx+40]
    fstp dword ptr [rcx+44]
smooth_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
CalcSmoothIterationsOriginal_x87 ENDP

; Original formulas.pas HybridIntP8. XYZ/J/Zmul and seven expanded constants.
HybridIntP8Original_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp+2], 137Fh
    fldcw word ptr [rsp+2]

  fld   qword ptr [rcx+0]       
  fmul  st(0), st(0)      
  fld   qword ptr [rcx+8]       
  fmul  st(0), st(0)      
  fld   qword ptr [rcx+16]       
  fmul  st(0), st(0)      
  fld   st(2)             
  fadd  st(0), st(2)      
  fld   st(0)             
  fmul  st(0), st(1)      
  fld   st(2)
  fmul  st(0), st(0)      
  fld   st(2)             
  fmul  st(0), st(4)      
  fmul  qword ptr [rcx+56]  
  fsubr st(0), st(1)      
  fadd  st(0), st(2)      
  fld   st(4)             
  fsub  st(0), st(4)      
    fmulp st(1), st(0)
  fld   st(3)             
  fsqrt
    fmulp st(1), st(0)
  fmul  qword ptr [rcx+16]       
  fmul  qword ptr [rcx+72]  
  fmul  qword ptr [rcx+48] 
  fchs
  fadd  qword ptr [rcx+40]  
  fstp  qword ptr [rcx+16]       
  fld   st(0)             
  fadd  st(0), st(2)      
  fmulp st(3), st(0)      
  fld   st(1)             
  fmul  qword ptr [rcx+88]  
  fadd  st(0), st(1)
    fmulp st(1), st(0)
  fxch  st(2)             
  fmulp st(3), st(0)      
  fxch  st(2)             
  fmul  qword ptr [rcx+80]  
    fsubp st(1), st(0)
  fxch  st(1)
  fmul  st(0), st(0)      
  fadd  qword ptr [rcx+104]  
    fdivp st(1), st(0)
  fadd  qword ptr [rcx+96]  
  fld   st(1)             
  fmul  qword ptr [rcx+64]  
  fld   st(3)             
  fmul  qword ptr [rcx+64]  
  fsub  st(0), st(3)      
  fld   st(4)             
  fsubr st(2), st(0)      
  fmul  st(0), st(0)      
  fmul  st(2), st(0)      
  fld   st(4)             
  fmul  st(0), st(0)      
  fmul  st(2), st(0)      
  fxch  st(2)             
  faddp st(3), st(0)      
  fxch  st(2)             
  fmul  qword ptr [rcx+72]  
  fmul  qword ptr [rcx+0]       
  fmul  qword ptr [rcx+8]       
  fmul  st(0), st(3)      
  fadd  qword ptr [rcx+32]  
  fstp  qword ptr [rcx+8]       
  fld   st(1)             
  fmul  qword ptr [rcx+88]  
  fadd  st(0), st(1)      
  fmul  st(0), st(1)      
  fxch  st(1)             
  fadd  st(0), st(2)      
  fmulp st(4), st(0)      
  fxch  st(4)             
  fmulp st(3), st(0)      
  fmul  st(0), st(0)      
  faddp st(3), st(0)      
  fxch  st(1)             
  fmul  qword ptr [rcx+80]
  fsubp st(2), st(0)      
    fmulp st(1), st(0)
  fadd  qword ptr [rcx+24]
  fstp  qword ptr [rcx+0]
    fldcw word ptr [rsp]
    add rsp, 8
    ret
HybridIntP8Original_x87 ENDP

; FPC -CfSSE2 compilation of doHybridPas radius expression. This differs
; from the hand-written doHybridSSE2 driver above, including NaN priority.
Rout3_compiled_pas_sse2 PROC
    movsd xmm1, qword ptr [rdx]
    mulsd xmm1, xmm1
    movsd xmm0, qword ptr [rdx+8]
    mulsd xmm0, xmm0
    addsd xmm0, xmm1
    movsd xmm1, qword ptr [rdx+16]
    mulsd xmm1, xmm1
    addsd xmm1, xmm0
    movsd qword ptr [rcx], xmm1
    ret
Rout3_compiled_pas_sse2 ENDP


; Original AmazingBox2 0010..018F. rcx XYZW and three Double spills; rdx expanded params and Julia C.
AmazingBox2Original_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    fld qword ptr [rdx + 24] ; 0010
    fld qword ptr [rcx] ; 0013
    fmul qword ptr [rdx + 160] ; 0015
    fld qword ptr [rcx + 16] ; 001B
    fmul qword ptr [rdx + 152] ; 001D
    fsubp st(1), st(0) ; 0023
    fstp qword ptr [rcx + 32] ; 0025
    fld qword ptr [rcx] ; 0028
    fmul qword ptr [rdx + 152] ; 002A
    fld qword ptr [rcx + 16] ; 0030
    fmul qword ptr [rdx + 160] ; 0032
    faddp st(1), st(0) ; 0038
    fstp qword ptr [rcx + 40] ; 003A
    nop ; 003E
    nop ; 003F
    fld qword ptr [rcx + 32] ; 0040
    fmul qword ptr [rdx + 144] ; 0043
    fld qword ptr [rcx + 8] ; 0046
    fmul qword ptr [rdx + 136] ; 0048
    fsubp st(1), st(0) ; 004B
    nop ; 004D
    fld qword ptr [rcx + 32] ; 004E
    fmul qword ptr [rdx + 136] ; 0051
    fld qword ptr [rcx + 8] ; 0054
    fmul qword ptr [rdx + 144] ; 0056
    faddp st(1), st(0) ; 0059
    fstp qword ptr [rcx + 48] ; 005B
    nop ; 005F
    fld st(0) ; 0060
    fsub st(0), st(2) ; 0062
    fabs ; 0064
    fadd st(0), st(1) ; 0066
    fxch st(1) ; 0068
    fadd st(0), st(2) ; 006A
    fabs ; 006C
    fsubrp st(1), st(0) ; 006E
    fmul qword ptr [rdx + 144] ; 0070
    fld qword ptr [rcx + 48] ; 0073
    fmul qword ptr [rdx + 136] ; 0077
    fsubp st(1), st(0) ; 007A
    nop ; 007C
    fmul qword ptr [rdx + 160] ; 007D
    fld qword ptr [rcx + 40] ; 0083
    nop ; 0087
    fmul qword ptr [rdx + 152] ; 0088
    fsubp st(1), st(0) ; 008E
    nop ; 0090
    nop ; 0091
    nop ; 0092
    nop ; 0093
    nop ; 0094
    nop ; 0095
    nop ; 0096
    nop ; 0097
    nop ; 0098
    nop ; 0099
    fld qword ptr [rcx + 8] ; 009A
    fld st(0) ; 009C
    fsub st(0), st(3) ; 009E
    fabs ; 00A0
    fadd st(0), st(1) ; 00A2
    fxch st(1) ; 00A4
    fadd st(0), st(3) ; 00A6
    fabs ; 00A8
    fsubrp st(1), st(0) ; 00AA
    fld qword ptr [rcx + 16] ; 00AC
    fld st(0) ; 00AE
    fsub st(0), st(4) ; 00B0
    fabs ; 00B2
    fadd st(0), st(1) ; 00B4
    fxch st(1) ; 00B6
    fadd st(0), st(4) ; 00B8
    fabs ; 00BA
    fsubrp st(1), st(0) ; 00BC
    nop ; 00BE
    fld qword ptr [rdx + 120] ; 00BF
    fadd st(1), st(0) ; 00C2
    fstp st(0) ; 00C4
    nop ; 00C6
    fld qword ptr [rdx + 112] ; 00C7
    fadd st(2), st(0) ; 00CA
    fstp st(0) ; 00CC
    nop ; 00CE
    fld qword ptr [rdx + 104] ; 00CF
    fadd st(3), st(0) ; 00D2
    fstp st(0) ; 00D4
    fld st(0) ; 00D6
    fmul st(0), st(0) ; 00D8
    fld st(2) ; 00DA
    fmul st(0), st(0) ; 00DC
    faddp st(1), st(0) ; 00DE
    fld st(3) ; 00E0
    fmul st(0), st(0) ; 00E2
    faddp st(1), st(0) ; 00E4
    fmul qword ptr [rdx + 128] ; 00E6
    nop ; 00E9
    fcom qword ptr [rdx + 16] ; 00EA
    fnstsw ax ; 00ED
    shr ah, 1 ; 00EF
    jae amazingbox2_00fa ; 00F1
    fstp st(0) ; 00F3
    fld qword ptr [rdx + 8] ; 00F5
    jmp amazingbox2_0110 ; 00F8
amazingbox2_00fa:
    fld1 ; 00FA
    fcom st(1) ; 00FC
    fnstsw ax ; 00FE
    shr ah, 1 ; 0100
    jb amazingbox2_010b ; 0102
    fstp st(0) ; 0104
    fdivr qword ptr [rdx + 0] ; 0106
    jmp amazingbox2_0110 ; 0109
amazingbox2_010b:
    fcompp ; 010B
    fld qword ptr [rdx + 0] ; 010D
amazingbox2_0110:
    fld qword ptr [rcx + 24] ; 0110
    fmul st(0), st(1) ; 0113
    fstp qword ptr [rcx + 24] ; 0115
    fmul st(3), st(0) ; 0118
    fmul st(2), st(0) ; 011A
    fmulp st(1), st(0) ; 011C
    nop ; 011E
    fld qword ptr [rdx + 120] ; 011F
    fmul qword ptr [rdx + 0] ; 0122
    fsub st(1), st(0) ; 0125
    fstp st(0) ; 0127
    nop ; 0129
    fld qword ptr [rdx + 112] ; 012A
    fmul qword ptr [rdx + 0] ; 012D
    fsub st(2), st(0) ; 0130
    fstp st(0) ; 0132
    nop ; 0134
    fld qword ptr [rdx + 104] ; 0135
    fmul qword ptr [rdx + 0] ; 0138
    fsub st(3), st(0) ; 013B
    fstp st(0) ; 013D
    nop ; 013F
    fxch st(2) ; 0140
    fld st(0) ; 0142
    fmul qword ptr [rdx + 80] ; 0144
    fld st(2) ; 0147
    fmul qword ptr [rdx + 88] ; 0149
    faddp st(1), st(0) ; 014C
    fld st(3) ; 014E
    fmul qword ptr [rdx + 96] ; 0150
    faddp st(1), st(0) ; 0153
    fld st(1) ; 0155
    fmul qword ptr [rdx + 32] ; 0157
    fld st(3) ; 015A
    fmul qword ptr [rdx + 40] ; 015C
    faddp st(1), st(0) ; 015F
    fld st(4) ; 0161
    fmul qword ptr [rdx + 48] ; 0163
    faddp st(1), st(0) ; 0166
    fxch st(2) ; 0168
    fmul qword ptr [rdx + 56] ; 016A
    fxch st(3) ; 016D
    fmul qword ptr [rdx + 64] ; 016F
    faddp st(3), st(0) ; 0172
    fxch st(3) ; 0174
    fmul qword ptr [rdx + 72] ; 0176
    faddp st(2), st(0) ; 0179
    fxch st(2) ; 017B
    fadd qword ptr [rdx + 184] ; 0180
    fstp qword ptr [rcx + 16] ; 0183
    fadd qword ptr [rdx + 176] ; 0185
    fstp qword ptr [rcx + 8] ; 0188
    fadd qword ptr [rdx + 168] ; 018A
    fstp qword ptr [rcx] ; 018D
    fstp st(0) ; 018F
AmazingBox2Original_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
AmazingBox2Original_x87 ENDP


; Original TetraFolding Pow2 001C..0425. rcx XYZ + nine Single spills; rdx params + C; r8d mode; r9d Tglad.
TetraFoldingPow2Original_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    jmp tetrapow2_00cb ; 001C
tetrapow2_0021:
    fld qword ptr [rcx + 16] ; 0021
    fld qword ptr [rcx + 8] ; 0023
    fld qword ptr [rcx] ; 0025
    fld st(0) ; 0027
    fmul dword ptr [rcx + 48] ; 0029
    fld st(2) ; 002C
    fmul dword ptr [rcx + 52] ; 002E
    faddp st(1), st(0) ; 0031
    fld st(3) ; 0033
    fmul dword ptr [rcx + 56] ; 0035
    faddp st(1), st(0) ; 0038
    fld st(1) ; 003A
    fmul dword ptr [rcx + 24] ; 003C
    fld st(3) ; 003F
    fmul dword ptr [rcx + 28] ; 0041
    faddp st(1), st(0) ; 0044
    fld st(4) ; 0046
    fmul dword ptr [rcx + 32] ; 0048
    faddp st(1), st(0) ; 004B
    fxch st(2) ; 004D
    fmul dword ptr [rcx + 36] ; 004F
    fxch st(3) ; 0052
    fmul dword ptr [rcx + 40] ; 0054
    faddp st(3), st(0) ; 0057
    fxch st(3) ; 0059
    fmul dword ptr [rcx + 44] ; 005B
    faddp st(2), st(0) ; 005E
    cmp r9d, 1
    je tetrapow2_0072 ; 0064
    fsub qword ptr [rdx + 40] ; 0066
    fabs ; 0069
    fchs ; 006B
    fadd qword ptr [rdx + 40] ; 006D
    jmp tetrapow2_008a ; 0070
tetrapow2_0072:
    fld st(0) ; 0072
    fadd qword ptr [rdx + 40] ; 0074
    fabs ; 0077
    fld st(1) ; 0079
    fsub qword ptr [rdx + 40] ; 007B
    fabs ; 007E
    fchs ; 0080
    faddp st(1), st(0) ; 0082
    fsubp st(1), st(0) ; 0084
    fchs ; 0086
    fnop ; 0088
tetrapow2_008a:
    fld st(0) ; 008A
    fmul dword ptr [rcx + 32] ; 008C
    fld st(2) ; 008F
    fmul dword ptr [rcx + 44] ; 0091
    faddp st(1), st(0) ; 0094
    fld st(3) ; 0096
    fmul dword ptr [rcx + 56] ; 0098
    faddp st(1), st(0) ; 009B
    fld st(1) ; 009D
    fmul dword ptr [rcx + 24] ; 009F
    fld st(3) ; 00A2
    fmul dword ptr [rcx + 36] ; 00A4
    faddp st(1), st(0) ; 00A7
    fld st(4) ; 00A9
    fmul dword ptr [rcx + 48] ; 00AB
    faddp st(1), st(0) ; 00AE
    fxch st(2) ; 00B0
    fmul dword ptr [rcx + 28] ; 00B2
    fxch st(3) ; 00B5
    fmul dword ptr [rcx + 40] ; 00B7
    faddp st(3), st(0) ; 00BA
    fxch st(3) ; 00BC
    fmul dword ptr [rcx + 52] ; 00BE
    faddp st(2), st(0) ; 00C1
    fstp qword ptr [rcx] ; 00C3
    fstp qword ptr [rcx + 8] ; 00C5
    fstp qword ptr [rcx + 16] ; 00C7
    ret ; 00C9
    nop ; 00CA
tetrapow2_00cb:
    fld qword ptr [rdx + 48] ; 00CB
    fstp dword ptr [rcx + 24] ; 00CE
    fld qword ptr [rdx + 56] ; 00D1
    fstp dword ptr [rcx + 28] ; 00D4
    fld qword ptr [rdx + 64] ; 00D7
    fstp dword ptr [rcx + 32] ; 00DA
    fld qword ptr [rdx + 72] ; 00DD
    fstp dword ptr [rcx + 36] ; 00E0
    fld qword ptr [rdx + 80] ; 00E3
    fstp dword ptr [rcx + 40] ; 00E6
    fld qword ptr [rdx + 88] ; 00E9
    fstp dword ptr [rcx + 44] ; 00EC
    fld qword ptr [rdx + 96] ; 00EF
    fstp dword ptr [rcx + 48] ; 00F2
    fld qword ptr [rdx + 104] ; 00F5
    fstp dword ptr [rcx + 52] ; 00F8
    fld qword ptr [rdx + 112] ; 00FB
    fstp dword ptr [rcx + 56] ; 00FE
    call tetrapow2_037c ; 0101
    cmp r8d, 0
    jne tetrapow2_0180 ; 010A
    fld dword ptr [rcx + 32] ; 010C
    fchs ; 010F
    fstp dword ptr [rcx + 32] ; 0111
    fld dword ptr [rcx + 44] ; 0114
    fchs ; 0117
    fstp dword ptr [rcx + 44] ; 0119
    fld dword ptr [rcx + 48] ; 011C
    fchs ; 011F
    fstp dword ptr [rcx + 48] ; 0121
    fld dword ptr [rcx + 52] ; 0124
    fchs ; 0127
    fstp dword ptr [rcx + 52] ; 0129
    call tetrapow2_0021 ; 012C
    fld dword ptr [rcx + 28] ; 0131
    fchs ; 0134
    fstp dword ptr [rcx + 28] ; 0136
    fld dword ptr [rcx + 44] ; 0139
    fchs ; 013C
    fstp dword ptr [rcx + 44] ; 013E
    fld dword ptr [rcx + 36] ; 0141
    fchs ; 0144
    fstp dword ptr [rcx + 36] ; 0146
    fld dword ptr [rcx + 52] ; 0149
    fchs ; 014C
    fstp dword ptr [rcx + 52] ; 014E
    call tetrapow2_0021 ; 0151
    fld dword ptr [rcx + 32] ; 0156
    fchs ; 0159
    fstp dword ptr [rcx + 32] ; 015B
    fld dword ptr [rcx + 44] ; 015E
    fchs ; 0161
    fstp dword ptr [rcx + 44] ; 0163
    fld dword ptr [rcx + 48] ; 0166
    fchs ; 0169
    fstp dword ptr [rcx + 48] ; 016B
    fld dword ptr [rcx + 52] ; 016E
    fchs ; 0171
    fstp dword ptr [rcx + 52] ; 0173
    call tetrapow2_0021 ; 0176
    jmp tetrapow2_02e8 ; 017B
tetrapow2_0180:
    cmp r8d, 1
    jne tetrapow2_01fa ; 0184
    fld dword ptr [rcx + 28] ; 0186
    fchs ; 0189
    fstp dword ptr [rcx + 28] ; 018B
    fld dword ptr [rcx + 44] ; 018E
    fchs ; 0191
    fstp dword ptr [rcx + 44] ; 0193
    fld dword ptr [rcx + 36] ; 0196
    fchs ; 0199
    fstp dword ptr [rcx + 36] ; 019B
    fld dword ptr [rcx + 52] ; 019E
    fchs ; 01A1
    fstp dword ptr [rcx + 52] ; 01A3
    call tetrapow2_0021 ; 01A6
    fld dword ptr [rcx + 32] ; 01AB
    fchs ; 01AE
    fstp dword ptr [rcx + 32] ; 01B0
    fld dword ptr [rcx + 44] ; 01B3
    fchs ; 01B6
    fstp dword ptr [rcx + 44] ; 01B8
    fld dword ptr [rcx + 48] ; 01BB
    fchs ; 01BE
    fstp dword ptr [rcx + 48] ; 01C0
    fld dword ptr [rcx + 52] ; 01C3
    fchs ; 01C6
    fstp dword ptr [rcx + 52] ; 01C8
    call tetrapow2_0021 ; 01CB
    fld dword ptr [rcx + 28] ; 01D0
    fchs ; 01D3
    fstp dword ptr [rcx + 28] ; 01D5
    fld dword ptr [rcx + 44] ; 01D8
    fchs ; 01DB
    fstp dword ptr [rcx + 44] ; 01DD
    fld dword ptr [rcx + 36] ; 01E0
    fchs ; 01E3
    fstp dword ptr [rcx + 36] ; 01E5
    fld dword ptr [rcx + 52] ; 01E8
    fchs ; 01EB
    fstp dword ptr [rcx + 52] ; 01ED
    call tetrapow2_0021 ; 01F0
    jmp tetrapow2_02e8 ; 01F5
tetrapow2_01fa:
    cmp r8d, 2
    jne tetrapow2_0271 ; 01FE
    fld dword ptr [rcx + 32] ; 0200
    fchs ; 0203
    fstp dword ptr [rcx + 32] ; 0205
    fld dword ptr [rcx + 28] ; 0208
    fchs ; 020B
    fstp dword ptr [rcx + 28] ; 020D
    fld dword ptr [rcx + 48] ; 0210
    fchs ; 0213
    fstp dword ptr [rcx + 48] ; 0215
    fld dword ptr [rcx + 36] ; 0218
    fchs ; 021B
    fstp dword ptr [rcx + 36] ; 021D
    call tetrapow2_0021 ; 0220
    fld dword ptr [rcx + 28] ; 0225
    fchs ; 0228
    fstp dword ptr [rcx + 28] ; 022A
    fld dword ptr [rcx + 44] ; 022D
    fchs ; 0230
    fstp dword ptr [rcx + 44] ; 0232
    fld dword ptr [rcx + 36] ; 0235
    fchs ; 0238
    fstp dword ptr [rcx + 36] ; 023A
    fld dword ptr [rcx + 52] ; 023D
    fchs ; 0240
    fstp dword ptr [rcx + 52] ; 0242
    call tetrapow2_0021 ; 0245
    fld dword ptr [rcx + 32] ; 024A
    fchs ; 024D
    fstp dword ptr [rcx + 32] ; 024F
    fld dword ptr [rcx + 28] ; 0252
    fchs ; 0255
    fstp dword ptr [rcx + 28] ; 0257
    fld dword ptr [rcx + 48] ; 025A
    fchs ; 025D
    fstp dword ptr [rcx + 48] ; 025F
    fld dword ptr [rcx + 36] ; 0262
    fchs ; 0265
    fstp dword ptr [rcx + 36] ; 0267
    call tetrapow2_0021 ; 026A
    jmp tetrapow2_02e8 ; 026F
tetrapow2_0271:
    nop ; 0271
    nop ; 0272
    nop ; 0273
    nop ; 0274
    nop ; 0275
    nop ; 0276
    fld dword ptr [rcx + 28] ; 0277
    fchs ; 027A
    fstp dword ptr [rcx + 28] ; 027C
    fld dword ptr [rcx + 44] ; 027F
    fchs ; 0282
    fstp dword ptr [rcx + 44] ; 0284
    fld dword ptr [rcx + 36] ; 0287
    fchs ; 028A
    fstp dword ptr [rcx + 36] ; 028C
    fld dword ptr [rcx + 52] ; 028F
    fchs ; 0292
    fstp dword ptr [rcx + 52] ; 0294
    call tetrapow2_0021 ; 0297
    fld dword ptr [rcx + 32] ; 029C
    fchs ; 029F
    fstp dword ptr [rcx + 32] ; 02A1
    fld dword ptr [rcx + 28] ; 02A4
    fchs ; 02A7
    fstp dword ptr [rcx + 28] ; 02A9
    fld dword ptr [rcx + 48] ; 02AC
    fchs ; 02AF
    fstp dword ptr [rcx + 48] ; 02B1
    fld dword ptr [rcx + 36] ; 02B4
    fchs ; 02B7
    fstp dword ptr [rcx + 36] ; 02B9
    call tetrapow2_0021 ; 02BC
    fld dword ptr [rcx + 28] ; 02C1
    fchs ; 02C4
    fstp dword ptr [rcx + 28] ; 02C6
    fld dword ptr [rcx + 44] ; 02C9
    fchs ; 02CC
    fstp dword ptr [rcx + 44] ; 02CE
    fld dword ptr [rcx + 36] ; 02D1
    fchs ; 02D4
    fstp dword ptr [rcx + 36] ; 02D6
    fld dword ptr [rcx + 52] ; 02D9
    fchs ; 02DC
    fstp dword ptr [rcx + 52] ; 02DE
    call tetrapow2_0021 ; 02E1
    fnop ; 02E6
tetrapow2_02e8:
    fnop ; 02E8
    fld qword ptr [rcx] ; 02EA
    fmul st(0), st(0) ; 02EC
    fld qword ptr [rcx + 8] ; 02EE
    fmul st(0), st(0) ; 02F0
    fld qword ptr [rdx + 24] ; 02F2
    faddp st(1), st(0) ; 02F5
    faddp st(1), st(0) ; 02F7
    fldz ; 02F9
    fcomp st(1) ; 02FB
    fnstsw ax ; 02FD
    sahf ; 02FF
    ja tetrapow2_0368 ; 0300
    je tetrapow2_0370 ; 0302
    nop ; 0304
    nop ; 0305
    fld qword ptr [rcx] ; 0306
    fmul st(0), st(0) ; 0308
    fld qword ptr [rcx + 8] ; 030A
    fmul st(0), st(0) ; 030C
    fchs ; 030E
    faddp st(1), st(0) ; 0310
    fmul qword ptr [rdx + 0] ; 0312
    fld qword ptr [rdx + 32] ; 0315
    fld qword ptr [rcx + 16] ; 0318
    fmul st(0), st(0) ; 031A
    fdiv st(0), st(3) ; 031C
    fchs ; 031E
    faddp st(1), st(0) ; 0320
    fmulp st(1), st(0) ; 0322
    fadd qword ptr [rdx + 144] ; 0324
    fnop ; 0327
    fld qword ptr [rcx] ; 0329
    fmul qword ptr [rcx + 8] ; 032B
    fmul qword ptr [rdx + 8] ; 032D
    fld qword ptr [rdx + 32] ; 0330
    fld qword ptr [rcx + 16] ; 0333
    fmul st(0), st(0) ; 0335
    fdiv st(0), st(4) ; 0337
    fchs ; 0339
    faddp st(1), st(0) ; 033B
    fmulp st(1), st(0) ; 033D
    fadd qword ptr [rdx + 152] ; 033F
    fnop ; 0342
    fld qword ptr [rcx + 16] ; 0344
    fld st(3) ; 0346
    fsqrt ; 0348
    fmulp st(1), st(0) ; 034A
    fmul qword ptr [rdx + 16] ; 034C
    fadd qword ptr [rdx + 160] ; 034F
tetrapow2_0352:
    fnop ; 0352
    fstp qword ptr [rcx + 16] ; 0354
    fstp qword ptr [rcx + 8] ; 0356
    fstp qword ptr [rcx] ; 0358
    fstp st(0) ; 035A
    jmp TetraFoldingPow2Original_x87_done
    nop ; 0367
tetrapow2_0368:
    fldz ; 0368
    fldz ; 036A
    fldz ; 036C
    jmp tetrapow2_0352 ; 036E
tetrapow2_0370:
    fld qword ptr [rdx + 144] ; 0370
    fld qword ptr [rdx + 152] ; 0373
    fld qword ptr [rdx + 160] ; 0376
    jmp tetrapow2_0352 ; 0379
    nop ; 037B
tetrapow2_037c:
    fld qword ptr [rcx + 16] ; 037C
    fld qword ptr [rcx + 8] ; 037E
    fld qword ptr [rcx] ; 0380
    fld st(0) ; 0382
    fmul dword ptr [rcx + 48] ; 0384
    fld st(2) ; 0387
    fmul dword ptr [rcx + 52] ; 0389
    faddp st(1), st(0) ; 038C
    fld st(3) ; 038E
    fmul dword ptr [rcx + 56] ; 0390
    faddp st(1), st(0) ; 0393
    fld st(1) ; 0395
    fmul dword ptr [rcx + 24] ; 0397
    fld st(3) ; 039A
    fmul dword ptr [rcx + 28] ; 039C
    faddp st(1), st(0) ; 039F
    fld st(4) ; 03A1
    fmul dword ptr [rcx + 32] ; 03A3
    faddp st(1), st(0) ; 03A6
    fxch st(2) ; 03A8
    fmul dword ptr [rcx + 36] ; 03AA
    fxch st(3) ; 03AD
    fmul dword ptr [rcx + 40] ; 03AF
    faddp st(3), st(0) ; 03B2
    fxch st(3) ; 03B4
    fmul dword ptr [rcx + 44] ; 03B6
    faddp st(2), st(0) ; 03B9
    cmp r9d, 1
    je tetrapow2_03cd ; 03BF
    fsub qword ptr [rdx + 40] ; 03C1
    fabs ; 03C4
    fchs ; 03C6
    fadd qword ptr [rdx + 40] ; 03C8
    jmp tetrapow2_03e3 ; 03CB
tetrapow2_03cd:
    fld st(0) ; 03CD
    fadd qword ptr [rdx + 40] ; 03CF
    fabs ; 03D2
    fld st(1) ; 03D4
    fsub qword ptr [rdx + 40] ; 03D6
    fabs ; 03D9
    fchs ; 03DB
    faddp st(1), st(0) ; 03DD
    fsubp st(1), st(0) ; 03DF
    fchs ; 03E1
tetrapow2_03e3:
    fmul qword ptr [rdx + 136] ; 03E3
    fld st(0) ; 03E6
    fmul dword ptr [rcx + 32] ; 03E8
    fld st(2) ; 03EB
    fmul dword ptr [rcx + 44] ; 03ED
    faddp st(1), st(0) ; 03F0
    fld st(3) ; 03F2
    fmul dword ptr [rcx + 56] ; 03F4
    faddp st(1), st(0) ; 03F7
    fld st(1) ; 03F9
    fmul dword ptr [rcx + 24] ; 03FB
    fld st(3) ; 03FE
    fmul dword ptr [rcx + 36] ; 0400
    faddp st(1), st(0) ; 0403
    fld st(4) ; 0405
    fmul dword ptr [rcx + 48] ; 0407
    faddp st(1), st(0) ; 040A
    fxch st(2) ; 040C
    fmul dword ptr [rcx + 28] ; 040E
    fxch st(3) ; 0411
    fmul dword ptr [rcx + 40] ; 0413
    faddp st(3), st(0) ; 0416
    fxch st(3) ; 0418
    fmul dword ptr [rcx + 52] ; 041A
    faddp st(2), st(0) ; 041D
    fstp qword ptr [rcx] ; 041F
    fstp qword ptr [rcx + 8] ; 0421
    fstp qword ptr [rcx + 16] ; 0423
    ret ; 0425
TetraFoldingPow2Original_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
TetraFoldingPow2Original_x87 ENDP


; Original BT1Pine 0016..011E. rcx XYZ + four Double spills; rdx params + C; r8b loop count.
BT1PineOriginal_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    mov al, r8b ; original byte counter; zero performs 256 iterations
bt1pine_0019:
    fld qword ptr [rcx] ; 0019
    fmul qword ptr [rdx + 48] ; 001B
    fld qword ptr [rcx + 16] ; 001D
    fmul qword ptr [rdx + 56] ; 001F
    fchs ; 0022
    faddp st(1), st(0) ; 0024
    fmul qword ptr [rdx + 64] ; 0026
    fstp qword ptr [rcx + 24] ; 0029
    fld qword ptr [rcx] ; 002C
    fmul qword ptr [rdx + 56] ; 002E
    fld qword ptr [rcx + 16] ; 0031
    fmul qword ptr [rdx + 48] ; 0033
    faddp st(1), st(0) ; 0035
    fabs ; 0037
    nop ; 0039
    fld qword ptr [rcx + 24] ; 003A
    fld qword ptr [rcx + 8] ; 003D
    fmul qword ptr [rdx + 64] ; 003F
    fchs ; 0042
    faddp st(1), st(0) ; 0044
    fabs ; 0046
    nop ; 0048
    fld qword ptr [rcx + 24] ; 0049
    fld qword ptr [rcx + 8] ; 004C
    fmul qword ptr [rdx + 64] ; 004E
    faddp st(1), st(0) ; 0051
    fabs ; 0053
    nop ; 0055
    fstp qword ptr [rcx + 8] ; 0056
    fstp qword ptr [rcx] ; 0058
    fstp qword ptr [rcx + 16] ; 005A
    nop ; 005C
    fld qword ptr [rcx] ; 005D
    fadd qword ptr [rcx + 8] ; 005F
    fmul qword ptr [rdx + 64] ; 0061
    fstp qword ptr [rcx + 24] ; 0064
    nop ; 0067
    fld qword ptr [rcx] ; 0068
    fchs ; 006A
    fadd qword ptr [rcx + 8] ; 006C
    fmul qword ptr [rdx + 64] ; 006E
    nop ; 0071
    fld qword ptr [rcx + 24] ; 0072
    fmul qword ptr [rdx + 48] ; 0075
    fld qword ptr [rcx + 16] ; 0077
    fmul qword ptr [rdx + 56] ; 0079
    faddp st(1), st(0) ; 007C
    nop ; 007E
    fld qword ptr [rcx + 24] ; 007F
    fmul qword ptr [rdx + 56] ; 0082
    fchs ; 0085
    fld qword ptr [rcx + 16] ; 0087
    fmul qword ptr [rdx + 48] ; 0089
    faddp st(1), st(0) ; 008B
    nop ; 008D
    fstp qword ptr [rcx + 16] ; 008E
    fstp qword ptr [rcx] ; 0090
    fstp qword ptr [rcx + 8] ; 0092
    nop ; 0094
    fld qword ptr [rcx] ; 0095
    fmul qword ptr [rdx + 0] ; 0097
    fsub qword ptr [rdx + 8] ; 009A
    fstp qword ptr [rcx] ; 009D
    fld qword ptr [rcx + 8] ; 009F
    fmul qword ptr [rdx + 0] ; 00A1
    fstp qword ptr [rcx + 8] ; 00A4
    fld qword ptr [rcx + 16] ; 00A6
    fmul qword ptr [rdx + 0] ; 00A8
    fstp qword ptr [rcx + 16] ; 00AB
    nop ; 00AD
    dec al ; 00AE
    jne bt1pine_0019 ; 00B0
    fld qword ptr [rcx] ; 00B6
    fmul st(0), st(0) ; 00B8
    fstp qword ptr [rcx + 24] ; 00BA
    fld qword ptr [rcx + 8] ; 00BD
    fmul st(0), st(0) ; 00BF
    fstp qword ptr [rcx + 32] ; 00C1
    fld qword ptr [rcx + 16] ; 00C4
    fmul st(0), st(0) ; 00C6
    fstp qword ptr [rcx + 40] ; 00C8
    fld qword ptr [rcx] ; 00CB
    fadd st(0), st(0) ; 00CD
    fld qword ptr [rcx + 32] ; 00CF
    fadd qword ptr [rcx + 40] ; 00D2
    fsqrt ; 00D5
    fdivp st(1), st(0) ; 00D7
    fstp qword ptr [rcx + 48] ; 00D9
    nop ; 00DC
    fld qword ptr [rcx + 24] ; 00DD
    fsub qword ptr [rcx + 32] ; 00E0
    fsub qword ptr [rcx + 40] ; 00E3
    nop ; 00E6
    fld qword ptr [rcx + 48] ; 00E7
    fld qword ptr [rcx + 32] ; 00EA
    fsub qword ptr [rcx + 40] ; 00ED
    fmulp st(1), st(0) ; 00F0
    nop ; 00F2
    fld qword ptr [rcx + 48] ; 00F3
    fadd st(0), st(0) ; 00F6
    fmul qword ptr [rcx + 16] ; 00F8
    fmul qword ptr [rcx + 8] ; 00FA
    nop ; 00FC
    fld qword ptr [rdx + 80] ; 00FD
    fmul qword ptr [rdx + 32] ; 0100
    faddp st(1), st(0) ; 0103
    fstp qword ptr [rcx + 8] ; 0105
    nop ; 0107
    fld qword ptr [rdx + 88] ; 0108
    fmul qword ptr [rdx + 24] ; 010B
    faddp st(1), st(0) ; 010E
    fstp qword ptr [rcx + 16] ; 0110
    nop ; 0112
    fld qword ptr [rdx + 72] ; 0113
    fmul qword ptr [rdx + 16] ; 0116
    faddp st(1), st(0) ; 0119
    fstp qword ptr [rcx] ; 011B
    nop ; 011D
    fnop ; 011E
BT1PineOriginal_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
BT1PineOriginal_x87 ENDP


; Original gnarlyIFS, even Flip only: exact FSIN stack order and DEC/JG. rcx state/local/constants; rdx params; r8 int apply/iters.
GnarlyIFSEvenOriginal_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    movsd xmm0, qword ptr [rcx + 0]
    mulsd xmm0, qword ptr [rdx + 8]
    addsd xmm0, qword ptr [rdx + 32]
    movsd qword ptr [rcx + 56], xmm0
    movsd xmm1, qword ptr [rcx + 8]
    mulsd xmm1, qword ptr [rdx + 8]
    addsd xmm1, qword ptr [rdx + 24]
    movsd qword ptr [rcx + 64], xmm1
    movsd xmm2, qword ptr [rcx + 16]
    mulsd xmm2, qword ptr [rdx + 8]
    addsd xmm2, qword ptr [rdx + 16]
    movsd qword ptr [rcx + 72], xmm2
    cmp dword ptr [r8], 0
    je gnarly_even_local
    movsd xmm5, qword ptr [rcx + 32]
    mulsd xmm5, qword ptr [rdx + 8]
    movsd qword ptr [rcx], xmm0
    movsd qword ptr [rcx + 8], xmm1
    movsd qword ptr [rcx + 16], xmm2
    movsd qword ptr [rcx + 32], xmm5
gnarly_even_local:
    fild dword ptr [r8 + 4] ; 0054
    fistp dword ptr [rcx + 80] ; 0057
    nop ; 005B
    fld qword ptr [rcx + 72] ; 005C
    fabs ; 0060
    fld qword ptr [rdx + 40] ; 0062
    fabs ; 0065
    fadd qword ptr [rcx + 96] ; 0067
    fmul qword ptr [rcx + 88] ; 006A
    fadd qword ptr [rcx + 48] ; 006C
    fcompp ; 0072
    fnstsw ax ; 0074
    shr ah, 1 ; 0076
    jae gnarly_even_0085 ; 0078
    fld qword ptr [rcx + 72] ; 007A
    fchs ; 007E
    jmp gnarly_even_015f ; 0080
gnarly_even_0085:
    fld qword ptr [rcx + 64] ; 0085
    fld qword ptr [rcx + 56] ; 0089
gnarly_even_008c:
    fld st(0) ; 008C
    fmul qword ptr [rdx] ; 008E
    fsin ; 0091
    fadd st(0), st(1) ; 0093
    fmul qword ptr [rdx + 72] ; 0095
    fsin ; 0098
    fadd st(0), st(1) ; 009A
    fsin ; 009C
    fmul qword ptr [rdx + 80] ; 009E
    fadd st(0), st(2) ; 00A1
    fld st(2) ; 00A3
    fmul qword ptr [rdx] ; 00A5
    fsin ; 00A8
    fadd st(0), st(3) ; 00AA
    fmul qword ptr [rdx + 72] ; 00AC
    fsin ; 00AF
    fadd st(0), st(3) ; 00B1
    fsin ; 00B3
    fmul qword ptr [rdx + 88] ; 00B5
    fadd st(0), st(2) ; 00B8
    fstp st(3) ; 00BA
    fstp st(1) ; 00BC
    dec dword ptr [rcx + 80] ; 00BE
    jg gnarly_even_008c ; 00C2
    fmul st(0), st(0) ; 00C4
    fxch st(1) ; 00C6
    fmul st(0), st(0) ; 00C8
    faddp st(1), st(0) ; 00CA
    fsqrt ; 00CC
    fld qword ptr [rcx + 64] ; 00CE
    fmul st(0), st(0) ; 00D2
    fld qword ptr [rcx + 56] ; 00D4
    fmul st(0), st(0) ; 00D7
    faddp st(1), st(0) ; 00D9
    fsqrt ; 00DB
    fchs ; 00DD
    faddp st(1), st(0) ; 00DF
    fld st(0) ; 00E3
    fld qword ptr [rdx + 96] ; 00E5
    fldz ; 00E8
    fcompp ; 00EA
    fnstsw ax ; 00EC
    shr ah, 1 ; 00EE
    jb gnarly_even_00fc ; 00F0
    fld1 ; 00F2
    fadd st(1), st(0) ; 00F4
    faddp st(1), st(0) ; 00F6
    fabs ; 00F8
    fsqrt ; 00FA
gnarly_even_00fc:
    fmul qword ptr [rdx + 96] ; 00FC
    fabs ; 00FF
    fstp qword ptr [rcx + 40] ; 0101
    fmul qword ptr [rdx + 40] ; 0107
    fsub qword ptr [rcx + 72] ; 010A
    cmp dword ptr [r8], 0 ; 010E
    jne gnarly_even_0117 ; 0112
    fdiv qword ptr [rdx + 8] ; 0114
gnarly_even_0117:
    fstp qword ptr [rcx + 24] ; 0117
    jmp GnarlyIFSEvenOriginal_x87_done
gnarly_even_015f:
    cmp dword ptr [r8], 0 ; 015F
    jne gnarly_even_0168 ; 0163
    fdiv qword ptr [rdx + 8] ; 0165
gnarly_even_0168:
    fstp qword ptr [rcx + 24] ; 0168
GnarlyIFSEvenOriginal_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
GnarlyIFSEvenOriginal_x87 ENDP


; Original gnarly2IFS even Flip; same state ABI as GnarlyIFSEvenOriginal_x87.
gnarly2IFSEvenOriginal_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    movsd xmm0, qword ptr [rcx + 0]
    mulsd xmm0, qword ptr [rdx + 8]
    addsd xmm0, qword ptr [rdx + 32]
    movsd qword ptr [rcx + 56], xmm0
    movsd xmm1, qword ptr [rcx + 8]
    mulsd xmm1, qword ptr [rdx + 8]
    addsd xmm1, qword ptr [rdx + 24]
    movsd qword ptr [rcx + 64], xmm1
    movsd xmm2, qword ptr [rcx + 16]
    mulsd xmm2, qword ptr [rdx + 8]
    addsd xmm2, qword ptr [rdx + 16]
    movsd qword ptr [rcx + 72], xmm2
    cmp dword ptr [r8], 0
    je gnarly2IFS_even_local
    movsd xmm5, qword ptr [rcx + 32]
    mulsd xmm5, qword ptr [rdx + 8]
    movsd qword ptr [rcx], xmm0
    movsd qword ptr [rcx + 8], xmm1
    movsd qword ptr [rcx + 16], xmm2
    movsd qword ptr [rcx + 32], xmm5
gnarly2IFS_even_local:
    fild dword ptr [r8 + 4] ; 0054
    fistp dword ptr [rcx + 80] ; 0057
    nop ; 005B
    fld qword ptr [rcx + 72] ; 005C
    fabs ; 0060
    fld qword ptr [rdx + 40] ; 0062
    fabs ; 0065
    fadd qword ptr [rcx + 96] ; 0067
    fmul qword ptr [rcx + 88] ; 006A
    fadd qword ptr [rcx + 48] ; 006C
    fcompp ; 0072
    fnstsw ax ; 0074
    shr ah, 1 ; 0076
    jae gnarly2IFS_even_0085 ; 0078
    fld qword ptr [rcx + 72] ; 007A
    fchs ; 007E
    jmp gnarly2IFS_even_011e ; 0080
gnarly2IFS_even_0085:
    fld qword ptr [rcx + 64] ; 0085
    fld qword ptr [rcx + 56] ; 0089
gnarly2IFS_even_008c:
    fld st(0) ; 008C
    fcos ; 008E
    fmul qword ptr [rdx] ; 0090
    fadd st(0), st(1) ; 0093
    fmul qword ptr [rdx + 72] ; 0095
    fsin ; 0098
    fld st(1) ; 009A
    fsub st(0), st(3) ; 009C
    fmul qword ptr [rdx + 88] ; 009E
    faddp st(1), st(0) ; 00A1
    fsin ; 00A3
    fmul qword ptr [rdx + 80] ; 00A5
    fadd st(0), st(2) ; 00A8
    fnop ; 00AA
    fld st(2) ; 00AC
    fcos ; 00AE
    fmul qword ptr [rdx] ; 00B0
    fadd st(0), st(3) ; 00B3
    fmul qword ptr [rdx + 72] ; 00B5
    fsin ; 00B8
    fld st(2) ; 00BA
    fadd st(0), st(4) ; 00BC
    fmul qword ptr [rdx + 88] ; 00BE
    faddp st(1), st(0) ; 00C1
    fsin ; 00C3
    fmul qword ptr [rdx + 80] ; 00C5
    fadd st(0), st(2) ; 00C8
    fstp st(2) ; 00CA
    fstp st(2) ; 00CC
    dec dword ptr [rcx + 80] ; 00CE
    jg gnarly2IFS_even_008c ; 00D2
    fmul st(0), st(0) ; 00D4
    fxch st(1) ; 00D6
    fmul st(0), st(0) ; 00D8
    faddp st(1), st(0) ; 00DA
    fsqrt ; 00DC
    fld qword ptr [rcx + 64] ; 00DE
    fmul st(0), st(0) ; 00E2
    fld qword ptr [rcx + 56] ; 00E4
    fmul st(0), st(0) ; 00E7
    faddp st(1), st(0) ; 00E9
    fsqrt ; 00EB
    fchs ; 00ED
    faddp st(1), st(0) ; 00EF
    fld st(0) ; 00F3
    fld qword ptr [rdx + 96] ; 00F5
    fldz ; 00F8
    fcompp ; 00FA
    fnstsw ax ; 00FC
    shr ah, 1 ; 00FE
    jb gnarly2IFS_even_010c ; 0100
    fld1 ; 0102
    fadd st(1), st(0) ; 0104
    faddp st(1), st(0) ; 0106
    fabs ; 0108
    fsqrt ; 010A
gnarly2IFS_even_010c:
    fmul qword ptr [rdx + 96] ; 010C
    fabs ; 010F
    fstp qword ptr [rcx + 40] ; 0111
    fmul qword ptr [rdx + 40] ; 0117
    fsub qword ptr [rcx + 72] ; 011A
gnarly2IFS_even_011e:
    cmp dword ptr [r8], 0 ; 011E
    jne gnarly2IFS_even_0127 ; 0122
    fdiv qword ptr [rdx + 8] ; 0124
gnarly2IFS_even_0127:
    fstp qword ptr [rcx + 24] ; 0127
gnarly2IFSEvenOriginal_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
gnarly2IFSEvenOriginal_x87 ENDP


; Original gnarly3IFS even Flip; same state ABI as GnarlyIFSEvenOriginal_x87.
gnarly3IFSEvenOriginal_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    movsd xmm0, qword ptr [rcx + 0]
    mulsd xmm0, qword ptr [rdx + 8]
    addsd xmm0, qword ptr [rdx + 32]
    movsd qword ptr [rcx + 56], xmm0
    movsd xmm1, qword ptr [rcx + 8]
    mulsd xmm1, qword ptr [rdx + 8]
    addsd xmm1, qword ptr [rdx + 24]
    movsd qword ptr [rcx + 64], xmm1
    movsd xmm2, qword ptr [rcx + 16]
    mulsd xmm2, qword ptr [rdx + 8]
    addsd xmm2, qword ptr [rdx + 16]
    movsd qword ptr [rcx + 72], xmm2
    cmp dword ptr [r8], 0
    je gnarly3IFS_even_local
    movsd xmm5, qword ptr [rcx + 32]
    mulsd xmm5, qword ptr [rdx + 8]
    movsd qword ptr [rcx], xmm0
    movsd qword ptr [rcx + 8], xmm1
    movsd qword ptr [rcx + 16], xmm2
    movsd qword ptr [rcx + 32], xmm5
gnarly3IFS_even_local:
    fild dword ptr [r8 + 4] ; 0054
    fistp dword ptr [rcx + 80] ; 0057
    nop ; 005B
    fld qword ptr [rcx + 72] ; 005C
    fabs ; 0060
    fld qword ptr [rdx + 40] ; 0062
    fabs ; 0065
    fadd qword ptr [rcx + 96] ; 0067
    fmul qword ptr [rcx + 88] ; 006A
    fadd qword ptr [rcx + 48] ; 006C
    fcompp ; 0072
    fnstsw ax ; 0074
    shr ah, 1 ; 0076
    jae gnarly3IFS_even_0085 ; 0078
    fld qword ptr [rcx + 72] ; 007A
    fchs ; 007E
    jmp gnarly3IFS_even_0128 ; 0080
gnarly3IFS_even_0085:
    fld qword ptr [rcx + 64] ; 0085
    fld qword ptr [rcx + 56] ; 0089
gnarly3IFS_even_008c:
    fld st(0) ; 008C
    fmul qword ptr [rdx] ; 008E
    fsin ; 0091
    fadd st(0), st(1) ; 0093
    fsin ; 0095
    fmul qword ptr [rdx + 72] ; 0097
    fchs ; 009A
    fld st(1) ; 009C
    fld st(0) ; 009E
    fabs ; 00A0
    fsqrt ; 00A2
    fmulp st(1), st(0) ; 00A4
    faddp st(1), st(0) ; 00A6
    fsin ; 00A8
    fmul qword ptr [rdx + 80] ; 00AA
    fchs ; 00AD
    fadd st(0), st(2) ; 00AF
    fnop ; 00B1
    fld st(2) ; 00B3
    fmul qword ptr [rdx] ; 00B5
    fsin ; 00B8
    fadd st(0), st(3) ; 00BA
    fmul qword ptr [rdx + 72] ; 00BC
    fsin ; 00BF
    fld st(3) ; 00C1
    fld st(0) ; 00C3
    fabs ; 00C5
    fsqrt ; 00C7
    fmulp st(1), st(0) ; 00C9
    faddp st(1), st(0) ; 00CB
    fsin ; 00CD
    fmul qword ptr [rdx + 88] ; 00CF
    fadd st(0), st(2) ; 00D2
    fstp st(3) ; 00D4
    fstp st(1) ; 00D6
    dec dword ptr [rcx + 80] ; 00D8
    jg gnarly3IFS_even_008c ; 00DC
    fmul st(0), st(0) ; 00DE
    fxch st(1) ; 00E0
    fmul st(0), st(0) ; 00E2
    faddp st(1), st(0) ; 00E4
    fsqrt ; 00E6
    fld qword ptr [rcx + 64] ; 00E8
    fmul st(0), st(0) ; 00EC
    fld qword ptr [rcx + 56] ; 00EE
    fmul st(0), st(0) ; 00F1
    faddp st(1), st(0) ; 00F3
    fsqrt ; 00F5
    fchs ; 00F7
    faddp st(1), st(0) ; 00F9
    fld st(0) ; 00FD
    fld qword ptr [rdx + 96] ; 00FF
    fldz ; 0102
    fcompp ; 0104
    fnstsw ax ; 0106
    shr ah, 1 ; 0108
    jb gnarly3IFS_even_0116 ; 010A
    fld1 ; 010C
    fadd st(1), st(0) ; 010E
    faddp st(1), st(0) ; 0110
    fabs ; 0112
    fsqrt ; 0114
gnarly3IFS_even_0116:
    fmul qword ptr [rdx + 96] ; 0116
    fabs ; 0119
    fstp qword ptr [rcx + 40] ; 011B
    fmul qword ptr [rdx + 40] ; 0121
    fsub qword ptr [rcx + 72] ; 0124
gnarly3IFS_even_0128:
    cmp dword ptr [r8], 0 ; 0128
    jne gnarly3IFS_even_0131 ; 012C
    fdiv qword ptr [rdx + 8] ; 012E
gnarly3IFS_even_0131:
    fstp qword ptr [rcx + 24] ; 0131
gnarly3IFSEvenOriginal_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
gnarly3IFSEvenOriginal_x87 ENDP


; Original CODE 001C..0205. rcx XYZ/DE + normal spills; rdx widened loader parameters; r8d rotation style.
DodecahedronIFSexOriginal_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    jmp dodecaex_00a7
dodecaex_001c:
    fld qword ptr [rcx] ; 001C
    fmul qword ptr [rcx + 32] ; 001E
    fld qword ptr [rcx + 8] ; 0021
    fmul qword ptr [rcx + 40] ; 0023
    fld qword ptr [rcx + 16] ; 0026
    fmul qword ptr [rcx + 48] ; 0028
    faddp st(1), st(0) ; 002B
    faddp st(1), st(0) ; 002D
    ftst ; 002F
    fnstsw ax ; 0031
    shr ah, 1 ; 0033
    jbe dodecaex_003b ; 0035
    fstp st(0) ; 0037
    ret ; 0039
    nop ; 003A
dodecaex_003b:
    fadd st(0), st(0) ; 003B
    fchs ; 003D
    fld st(0) ; 003F
    fmul qword ptr [rcx + 32] ; 0041
    fadd qword ptr [rcx] ; 0044
    fstp qword ptr [rcx] ; 0046
    fld st(0) ; 0048
    fmul qword ptr [rcx + 40] ; 004A
    fadd qword ptr [rcx + 8] ; 004D
    fstp qword ptr [rcx + 8] ; 004F
    fmul qword ptr [rcx + 48] ; 0051
    fadd qword ptr [rcx + 16] ; 0054
    fstp qword ptr [rcx + 16] ; 0056
    ret ; 0058
    nop ; 0059
dodecaex_005a:
    fld1 ; 005A
    fsub qword ptr [rdx + 176] ; 005C
    fchs ; 005F
    fst qword ptr [rcx + 32] ; 0061
    fld1 ; 0064
    fdivrp st(1), st(0) ; 0066
    fstp qword ptr [rcx + 40] ; 0068
    fld1 ; 006B
    fchs ; 006D
    fstp qword ptr [rcx + 48] ; 006F
    fld qword ptr [rcx + 32] ; 0072
    fmul st(0), st(0) ; 0075
    fld qword ptr [rcx + 40] ; 0077
    fmul st(0), st(0) ; 007A
    fld qword ptr [rcx + 48] ; 007C
    fmul st(0), st(0) ; 007F
    faddp st(1), st(0) ; 0081
    faddp st(1), st(0) ; 0083
    fsqrt ; 0085
    fld1 ; 0087
    fdivrp st(1), st(0) ; 0089
    fld qword ptr [rcx + 32] ; 008B
    fmul st(0), st(1) ; 008E
    fstp qword ptr [rcx + 32] ; 0090
    fld qword ptr [rcx + 40] ; 0093
    fmul st(0), st(1) ; 0096
    fstp qword ptr [rcx + 40] ; 0098
    fld qword ptr [rcx + 48] ; 009B
    fmul st(0), st(1) ; 009E
    fstp qword ptr [rcx + 48] ; 00A0
    fstp st(0) ; 00A3
    ret ; 00A5
    nop ; 00A6
dodecaex_00a7:
    fld qword ptr [rcx + 16] ; 00A7
    fld qword ptr [rcx + 8] ; 00A9
    fld qword ptr [rcx] ; 00AB
    fld st(0) ; 00AD
    fmul qword ptr [rdx + 80] ; 00AF
    fld st(2) ; 00B2
    fmul qword ptr [rdx + 88] ; 00B4
    faddp st(1), st(0) ; 00B7
    fld st(3) ; 00B9
    fmul qword ptr [rdx + 96] ; 00BB
    faddp st(1), st(0) ; 00BE
    nop ; 00C0
    nop ; 00C1
    fld st(1) ; 00C2
    fmul qword ptr [rdx + 32] ; 00C4
    fld st(3) ; 00C7
    fmul qword ptr [rdx + 40] ; 00C9
    faddp st(1), st(0) ; 00CC
    fld st(4) ; 00CE
    fmul qword ptr [rdx + 48] ; 00D0
    faddp st(1), st(0) ; 00D3
    nop ; 00D5
    nop ; 00D6
    fxch st(2) ; 00D7
    fmul qword ptr [rdx + 56] ; 00D9
    fxch st(3) ; 00DC
    fmul qword ptr [rdx + 64] ; 00DE
    faddp st(3), st(0) ; 00E1
    fxch st(3) ; 00E3
    fmul qword ptr [rdx + 72] ; 00E5
    faddp st(2), st(0) ; 00E8
    fstp qword ptr [rcx] ; 00EA
    fstp qword ptr [rcx + 8] ; 00EC
    fstp qword ptr [rcx + 16] ; 00EE
    call dodecaex_005a ; 00F0
    call dodecaex_001c ; 00F5
    cmp r8d, 0
    jne dodecaex_0105 ; 00FE
    call dodecaex_011f ; 0100
dodecaex_0105:
    jmp dodecaex_0166 ; 0105
dodecaex_0107:
    fld qword ptr [rcx + 32] ; 0107
    fld qword ptr [rcx + 40] ; 010A
    fld qword ptr [rcx + 48] ; 010D
    fstp qword ptr [rcx + 32] ; 0110
    fstp qword ptr [rcx + 48] ; 0113
    fstp qword ptr [rcx + 40] ; 0116
    call dodecaex_001c ; 0119
    ret ; 011E
dodecaex_011f:
    fld qword ptr [rcx + 16] ; 011F
    fld qword ptr [rcx + 8] ; 0121
    fld qword ptr [rcx] ; 0123
    fld st(1) ; 0125
    fmul qword ptr [rdx + 152] ; 0127
    fld st(1) ; 012A
    fmul qword ptr [rdx + 160] ; 012C
    faddp st(1), st(0) ; 012F
    fld st(3) ; 0131
    fmul qword ptr [rdx + 168] ; 0133
    faddp st(1), st(0) ; 0136
    fld st(2) ; 0138
    fmul qword ptr [rdx + 104] ; 013A
    fld st(2) ; 013D
    fmul qword ptr [rdx + 112] ; 013F
    faddp st(1), st(0) ; 0142
    fld st(4) ; 0144
    fmul qword ptr [rdx + 120] ; 0146
    faddp st(1), st(0) ; 0149
    fxch st(3) ; 014B
    fmul qword ptr [rdx + 128] ; 014D
    fxch st(2) ; 0150
    fmul qword ptr [rdx + 136] ; 0152
    faddp st(2), st(0) ; 0155
    fxch st(3) ; 0157
    fmul qword ptr [rdx + 144] ; 0159
    faddp st(1), st(0) ; 015C
    fstp qword ptr [rcx] ; 015E
    fstp qword ptr [rcx + 8] ; 0160
    fstp qword ptr [rcx + 16] ; 0162
    ret ; 0164
    nop ; 0165
dodecaex_0166:
    call dodecaex_0107 ; 0166
    cmp r8d, 1
    jne dodecaex_0176 ; 016F
    call dodecaex_011f ; 0171
dodecaex_0176:
    call dodecaex_0107 ; 0176
    cmp r8d, 2
    jne dodecaex_0186 ; 017F
    call dodecaex_011f ; 0181
dodecaex_0186:
    call dodecaex_0107 ; 0186
    cmp r8d, 3
    jne dodecaex_0196 ; 018F
    call dodecaex_011f ; 0191
dodecaex_0196:
    call dodecaex_0107 ; 0196
    cmp r8d, 4
    jne dodecaex_01a6 ; 019F
    call dodecaex_011f ; 01A1
dodecaex_01a6:
    call dodecaex_0107 ; 01A6
    cmp r8d, 5
    jne dodecaex_01b6 ; 01AF
    call dodecaex_011f ; 01B1
dodecaex_01b6:
    call dodecaex_0107 ; 01B6
    cmp r8d, 6
    jne dodecaex_01c6 ; 01BF
    call dodecaex_011f ; 01C1
dodecaex_01c6:
    fld qword ptr [rcx + 16] ; 01C6
    fld qword ptr [rcx + 8] ; 01C8
    fld qword ptr [rcx] ; 01CA
    fld qword ptr [rdx + 0] ; 01CC
    fld qword ptr [rcx + 24] ; 01CF
    fmul st(0), st(1) ; 01D2
    fstp qword ptr [rcx + 24] ; 01D4
    fmul st(3), st(0) ; 01D7
    fmul st(2), st(0) ; 01D9
    fmul st(1), st(0) ; 01DB
    fld1 ; 01DD
    fsubp st(1), st(0) ; 01DF
    fld st(0) ; 01E1
    fld st(0) ; 01E3
    fmul qword ptr [rdx + 8] ; 01E5
    fsubp st(4), st(0) ; 01E8
    fmul qword ptr [rdx + 16] ; 01EA
    fsubp st(2), st(0) ; 01ED
    fmul qword ptr [rdx + 24] ; 01EF
    fsubp st(3), st(0) ; 01F2
    fstp qword ptr [rcx] ; 01F4
    fstp qword ptr [rcx + 8] ; 01F6
    fstp qword ptr [rcx + 16] ; 01F8
    cmp r8d, 7
    jne dodecaex_0205 ; 01FE
    call dodecaex_011f ; 0200
dodecaex_0205:
    jmp DodecahedronIFSexOriginal_x87_done
DodecahedronIFSexOriginal_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
DodecahedronIFSexOriginal_x87 ENDP


; Original CODE 001A..016A. rcx XYZ/DE/VaryScale + seven spills; rdx params/C; r8d alt mode; r9 first-counter pointer.
ABoxVSWavyOriginal_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    cmp dword ptr [r9], 0 ; 001A
    jg aboxwavy_0029 ; 001E
    inc dword ptr [r9] ; 0020
    fld qword ptr [rdx + 0] ; 0023
    fstp qword ptr [rcx + 32] ; 0026
aboxwavy_0029:
    fld qword ptr [rcx + 32] ; 0029
    fld1 ; 002C
    fld st(1) ; 002E
    fabs ; 0030
    fsubrp st(1), st(0) ; 0032
    fmul qword ptr [rdx + 32] ; 0034
    faddp st(1), st(0) ; 0037
    fstp qword ptr [rcx + 32] ; 0039
    fld qword ptr [rdx + 24] ; 003C
    fld st(0) ; 003F
    fld st(0) ; 0041
    fadd st(2), st(0) ; 0043
    fchs ; 0045
    fld qword ptr [rcx] ; 0047
    fcom st(2) ; 0049
    fnstsw ax ; 004B
    shr ah, 1 ; 004D
    jb aboxwavy_0057 ; 004F
    fchs ; 0051
    fadd st(0), st(3) ; 0053
    jmp aboxwavy_0063 ; 0055
aboxwavy_0057:
    fcom st(1) ; 0057
    fnstsw ax ; 0059
    shr ah, 1 ; 005B
    jae aboxwavy_0063 ; 005D
    fchs ; 005F
    fsub st(0), st(3) ; 0061
aboxwavy_0063:
    fld qword ptr [rcx + 8] ; 0063
    fcom st(3) ; 0065
    fnstsw ax ; 0067
    shr ah, 1 ; 0069
    jb aboxwavy_0073 ; 006B
    fchs ; 006D
    fadd st(0), st(4) ; 006F
    jmp aboxwavy_007f ; 0071
aboxwavy_0073:
    fcom st(2) ; 0073
    fnstsw ax ; 0075
    shr ah, 1 ; 0077
    jae aboxwavy_007f ; 0079
    fchs ; 007B
    fsub st(0), st(4) ; 007D
aboxwavy_007f:
    fld qword ptr [rcx + 16] ; 007F
    fcom st(4) ; 0081
    fnstsw ax ; 0083
    shr ah, 1 ; 0085
    jb aboxwavy_008f ; 0087
    fchs ; 0089
    fadd st(0), st(5) ; 008B
    jmp aboxwavy_009b ; 008D
aboxwavy_008f:
    fcom st(3) ; 008F
    fnstsw ax ; 0091
    shr ah, 1 ; 0093
    jae aboxwavy_009b ; 0095
    fchs ; 0097
    fsub st(0), st(5) ; 0099
aboxwavy_009b:
    fxch st(3) ; 009B
    fstp st(0) ; 009D
    fxch st(3) ; 009F
    fstp st(0) ; 00A1
    fxch st(3) ; 00A3
    fstp st(0) ; 00A5
    fld st(2) ; 00A7
    fmul qword ptr [rdx + 48] ; 00A9
    fsincos ; 00AC
    fstp qword ptr [rcx + 40] ; 00AE
    fstp qword ptr [rcx + 48] ; 00B1
    fld st(1) ; 00B4
    fmul qword ptr [rdx + 48] ; 00B6
    fsincos ; 00B9
    fstp qword ptr [rcx + 56] ; 00BB
    fstp qword ptr [rcx + 64] ; 00BE
    fld st(0) ; 00C1
    fmul qword ptr [rdx + 48] ; 00C3
    fsincos ; 00C6
    fstp qword ptr [rcx + 72] ; 00C8
    fstp qword ptr [rcx + 80] ; 00CB
    nop ; 00CE
    fld st(0) ; 00CF
    fmul st(0), st(0) ; 00D1
    fld st(2) ; 00D3
    fmul st(0), st(0) ; 00D5
    faddp st(1), st(0) ; 00D7
    fld st(3) ; 00D9
    fmul st(0), st(0) ; 00DB
    faddp st(1), st(0) ; 00DD
    fsqrt ; 00DF
    fstp qword ptr [rcx + 88] ; 00E1
    nop ; 00E4
    fld qword ptr [rcx + 40] ; 00E5
    fmul qword ptr [rcx + 64] ; 00E8
    fld qword ptr [rcx + 56] ; 00EB
    fmul qword ptr [rcx + 80] ; 00EE
    faddp st(1), st(0) ; 00F1
    fld qword ptr [rcx + 72] ; 00F3
    fmul qword ptr [rcx + 48] ; 00F6
    faddp st(1), st(0) ; 00F9
    fmul qword ptr [rdx + 40] ; 00FB
    nop ; 00FE
    fld qword ptr [rcx + 40] ; 00FF
    fadd qword ptr [rcx + 56] ; 0102
    fadd qword ptr [rcx + 72] ; 0105
    fmul qword ptr [rdx + 56] ; 0108
    cmp r8d, 1
    je aboxwavy_011f ; 010F
    faddp st(1), st(0) ; 0111
    fld qword ptr [rcx + 88] ; 0113
    fmul qword ptr [rdx + 64] ; 0116
    faddp st(1), st(0) ; 0119
    nop ; 011B
    nop ; 011C
    jmp aboxwavy_0127 ; 011D
aboxwavy_011f:
    fadd qword ptr [rdx + 64] ; 011F
    fmul qword ptr [rcx + 88] ; 0122
    faddp st(1), st(0) ; 0125
aboxwavy_0127:
    fmul st(0), st(0) ; 0127
    fcom qword ptr [rdx + 16] ; 0129
    fnstsw ax ; 012C
    shr ah, 1 ; 012E
    jae aboxwavy_0139 ; 0130
    fstp st(0) ; 0132
    fld qword ptr [rdx + 8] ; 0134
    jmp aboxwavy_014f ; 0137
aboxwavy_0139:
    fld1 ; 0139
    fcom st(1) ; 013B
    fnstsw ax ; 013D
    shr ah, 1 ; 013F
    jb aboxwavy_014a ; 0141
    fstp st(0) ; 0143
    fdivr qword ptr [rcx + 32] ; 0145
    jmp aboxwavy_014f ; 0148
aboxwavy_014a:
    fcompp ; 014A
    fld qword ptr [rcx + 32] ; 014C
aboxwavy_014f:
    fld qword ptr [rcx + 24] ; 014F
    fmul st(0), st(1) ; 0152
    fstp qword ptr [rcx + 24] ; 0154
    fmul st(3), st(0) ; 0157
    fmul st(2), st(0) ; 0159
    fmulp st(1), st(0) ; 015B
    fadd qword ptr [rdx + 96] ; 015D
    fstp qword ptr [rcx + 16] ; 0160
    fadd qword ptr [rdx + 88] ; 0162
    fstp qword ptr [rcx + 8] ; 0165
    fadd qword ptr [rdx + 80] ; 0167
    fstp qword ptr [rcx] ; 016A
ABoxVSWavyOriginal_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
ABoxVSWavyOriginal_x87 ENDP


; Original CODE 0014..0439. rcx XYZ + local Double/high-word spills; rdx params/K/C.
IkenagabulbOriginal_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    fld qword ptr [rcx] ; 0014
    fstp qword ptr [rcx + 32] ; 0016
    fld qword ptr [rcx + 8] ; 001C
    fstp qword ptr [rcx + 40] ; 001E
    fld qword ptr [rcx + 16] ; 0024
    fstp qword ptr [rcx + 48] ; 0026
    fnop ; 002C
    fld qword ptr [rdx + 56] ; 002E
    fstp qword ptr [rcx + 56] ; 0031
    fld qword ptr [rdx + 64] ; 0037
    fstp qword ptr [rcx + 64] ; 003A
    fld qword ptr [rdx + 72] ; 0040
    fstp qword ptr [rcx + 72] ; 0043
    fnop ; 0049
    fld qword ptr [rdx + 0] ; 004B
    fstp qword ptr [rcx + 80] ; 004E
    fld qword ptr [rdx + 8] ; 0054
    fstp qword ptr [rcx + 88] ; 0057
    fld qword ptr [rdx + 16] ; 005D
    fstp qword ptr [rcx + 96] ; 0060
    fnop ; 0066
    fld qword ptr [rdx + 24] ; 0068
    fstp qword ptr [rcx + 104] ; 006B
    fld qword ptr [rdx + 32] ; 0071
    fstp qword ptr [rcx + 112] ; 0074
    fld qword ptr [rdx + 40] ; 007A
    fstp qword ptr [rcx + 120] ; 007D
    fnop ; 0083
    fld qword ptr [rcx + 32] ; 0085
    fmul qword ptr [rcx + 32] ; 008B
    fld qword ptr [rcx + 40] ; 0091
    fmul qword ptr [rcx + 40] ; 0097
    faddp st(1), st(0) ; 009D
    fsqrt ; 009F
    fstp qword ptr [rcx + 168] ; 00A1
    fld qword ptr [rcx + 80] ; 00A4
    fmul qword ptr [rcx + 80] ; 00A7
    fld qword ptr [rcx + 88] ; 00AA
    fmul qword ptr [rcx + 88] ; 00AD
    faddp st(1), st(0) ; 00B0
    fsqrt ; 00B2
    fstp qword ptr [rcx + 176] ; 00B4
    fld qword ptr [rcx + 168] ; 00B7
    fmul qword ptr [rcx + 176] ; 00BA
    fldz ; 00BD
    fcompp ; 00BF
    fnstsw ax ; 00C1
    sahf ; 00C3
    jne ikenaga_00e5 ; 00C4
    fld qword ptr [rcx + 48] ; 00C6
    fchs ; 00CC
    fmul qword ptr [rcx + 96] ; 00CE
    fstp qword ptr [rcx + 128] ; 00D1
    fld qword ptr [rcx + 48] ; 00D4
    fmul qword ptr [rcx + 96] ; 00DA
    fstp qword ptr [rcx + 24] ; 00DD
    jmp ikenaga_0144 ; 00E3
ikenaga_00e5:
    fld qword ptr [rcx + 48] ; 00E5
    fmul qword ptr [rcx + 96] ; 00EB
    fld qword ptr [rcx + 168] ; 00EE
    fmul qword ptr [rcx + 176] ; 00F1
    fdivp st(1), st(0) ; 00F4
    fld1 ; 00F6
    fsubrp st(1), st(0) ; 00F8
    fld qword ptr [rcx + 32] ; 00FA
    fmul qword ptr [rcx + 80] ; 0100
    fld qword ptr [rcx + 40] ; 0103
    fmul qword ptr [rcx + 88] ; 0109
    fsubp st(1), st(0) ; 010C
    fmulp st(1), st(0) ; 010E
    fstp qword ptr [rcx + 128] ; 0110
    fld qword ptr [rcx + 48] ; 0113
    fmul qword ptr [rcx + 96] ; 0119
    fld qword ptr [rcx + 168] ; 011C
    fmul qword ptr [rcx + 176] ; 011F
    fdivp st(1), st(0) ; 0122
    fld1 ; 0124
    fsubrp st(1), st(0) ; 0126
    fld qword ptr [rcx + 80] ; 0128
    fmul qword ptr [rcx + 40] ; 012B
    fld qword ptr [rcx + 32] ; 0131
    fmul qword ptr [rcx + 88] ; 0137
    faddp st(1), st(0) ; 013A
    fmulp st(1), st(0) ; 013C
    fstp qword ptr [rcx + 24] ; 013E
ikenaga_0144:
    fld qword ptr [rcx + 168] ; 0144
    fmul qword ptr [rcx + 96] ; 0147
    fld qword ptr [rcx + 176] ; 014A
    fmul qword ptr [rcx + 48] ; 014D
    faddp st(1), st(0) ; 0153
    fstp qword ptr [rcx + 136] ; 0155
    fld qword ptr [rcx + 128] ; 0158
    fmul qword ptr [rcx + 128] ; 015B
    fld qword ptr [rcx + 24] ; 015E
    fmul qword ptr [rcx + 24] ; 0164
    faddp st(1), st(0) ; 016A
    fsqrt ; 016C
    fstp qword ptr [rcx + 168] ; 016E
    fld qword ptr [rcx + 56] ; 0171
    fmul qword ptr [rcx + 56] ; 0177
    fld qword ptr [rcx + 64] ; 017D
    fmul qword ptr [rcx + 64] ; 0180
    faddp st(1), st(0) ; 0183
    fsqrt ; 0185
    fstp qword ptr [rcx + 176] ; 0187
    fld qword ptr [rcx + 168] ; 018A
    fmul qword ptr [rcx + 176] ; 018D
    fldz ; 0190
    fcompp ; 0192
    fnstsw ax ; 0194
    sahf ; 0196
    jne ikenaga_01af ; 0197
    fld qword ptr [rcx + 136] ; 0199
    fchs ; 019C
    fmul qword ptr [rcx + 72] ; 019E
    fstp qword ptr [rcx + 144] ; 01A1
    fld qword ptr [rcx + 136] ; 01A4
    fmul qword ptr [rcx + 72] ; 01A7
    fstp qword ptr [rcx + 152] ; 01AA
    jmp ikenaga_0205 ; 01AD
ikenaga_01af:
    fld qword ptr [rcx + 136] ; 01AF
    fmul qword ptr [rcx + 72] ; 01B2
    fld qword ptr [rcx + 168] ; 01B5
    fmul qword ptr [rcx + 176] ; 01B8
    fdivp st(1), st(0) ; 01BB
    fld1 ; 01BD
    fsubrp st(1), st(0) ; 01BF
    fld qword ptr [rcx + 128] ; 01C1
    fmul qword ptr [rcx + 56] ; 01C4
    fld qword ptr [rcx + 24] ; 01CA
    fmul qword ptr [rcx + 64] ; 01D0
    fsubp st(1), st(0) ; 01D3
    fmulp st(1), st(0) ; 01D5
    fstp qword ptr [rcx + 144] ; 01D7
    fld qword ptr [rcx + 136] ; 01DA
    fmul qword ptr [rcx + 72] ; 01DD
    fld qword ptr [rcx + 168] ; 01E0
    fmul qword ptr [rcx + 176] ; 01E3
    fdivp st(1), st(0) ; 01E6
    fld1 ; 01E8
    fsubrp st(1), st(0) ; 01EA
    fld qword ptr [rcx + 56] ; 01EC
    fmul qword ptr [rcx + 24] ; 01F2
    fld qword ptr [rcx + 136] ; 01F8
    fmul qword ptr [rcx + 64] ; 01FB
    faddp st(1), st(0) ; 01FE
    fmulp st(1), st(0) ; 0200
    fstp qword ptr [rcx + 152] ; 0202
ikenaga_0205:
    fld qword ptr [rcx + 168] ; 0205
    fmul qword ptr [rcx + 72] ; 0208
    fld qword ptr [rcx + 176] ; 020B
    fmul qword ptr [rcx + 136] ; 020E
    faddp st(1), st(0) ; 0211
    fstp qword ptr [rcx + 160] ; 0213
    fld qword ptr [rcx + 32] ; 0216
    fmul qword ptr [rcx + 32] ; 021C
    fld qword ptr [rcx + 40] ; 0222
    fmul qword ptr [rcx + 40] ; 0228
    faddp st(1), st(0) ; 022E
    fsqrt ; 0230
    fstp qword ptr [rcx + 168] ; 0232
    fld qword ptr [rcx + 104] ; 0235
    fmul qword ptr [rcx + 104] ; 0238
    fld qword ptr [rcx + 112] ; 023B
    fmul qword ptr [rcx + 112] ; 023E
    faddp st(1), st(0) ; 0241
    fsqrt ; 0243
    fstp qword ptr [rcx + 176] ; 0245
    fld qword ptr [rcx + 168] ; 0248
    fmul qword ptr [rcx + 176] ; 024B
    fldz ; 024E
    fcompp ; 0250
    fnstsw ax ; 0252
    sahf ; 0254
    jne ikenaga_0276 ; 0255
    fld qword ptr [rcx + 48] ; 0257
    fchs ; 025D
    fmul qword ptr [rcx + 120] ; 025F
    fstp qword ptr [rcx + 128] ; 0262
    fld qword ptr [rcx + 48] ; 0265
    fmul qword ptr [rcx + 120] ; 026B
    fstp qword ptr [rcx + 24] ; 026E
    jmp ikenaga_02d5 ; 0274
ikenaga_0276:
    fld qword ptr [rcx + 48] ; 0276
    fmul qword ptr [rcx + 120] ; 027C
    fld qword ptr [rcx + 168] ; 027F
    fmul qword ptr [rcx + 176] ; 0282
    fdivp st(1), st(0) ; 0285
    fld1 ; 0287
    fsubrp st(1), st(0) ; 0289
    fld qword ptr [rcx + 32] ; 028B
    fmul qword ptr [rcx + 104] ; 0291
    fld qword ptr [rcx + 40] ; 0294
    fmul qword ptr [rcx + 112] ; 029A
    fsubp st(1), st(0) ; 029D
    fmulp st(1), st(0) ; 029F
    fstp qword ptr [rcx + 128] ; 02A1
    fld qword ptr [rcx + 48] ; 02A4
    fmul qword ptr [rcx + 120] ; 02AA
    fld qword ptr [rcx + 168] ; 02AD
    fmul qword ptr [rcx + 176] ; 02B0
    fdivp st(1), st(0) ; 02B3
    fld1 ; 02B5
    fsubrp st(1), st(0) ; 02B7
    fld qword ptr [rcx + 104] ; 02B9
    fmul qword ptr [rcx + 40] ; 02BC
    fld qword ptr [rcx + 32] ; 02C2
    fmul qword ptr [rcx + 112] ; 02C8
    faddp st(1), st(0) ; 02CB
    fmulp st(1), st(0) ; 02CD
    fstp qword ptr [rcx + 24] ; 02CF
ikenaga_02d5:
    fld qword ptr [rcx + 168] ; 02D5
    fmul qword ptr [rcx + 120] ; 02D8
    fld qword ptr [rcx + 176] ; 02DB
    fmul qword ptr [rcx + 48] ; 02DE
    faddp st(1), st(0) ; 02E4
    fstp qword ptr [rcx + 136] ; 02E6
    fld qword ptr [rcx + 144] ; 02E9
    fsub qword ptr [rcx + 128] ; 02EC
    fsub qword ptr [rcx + 56] ; 02EF
    fstp qword ptr [rcx + 128] ; 02F5
    fld qword ptr [rcx + 152] ; 02F8
    fsub qword ptr [rcx + 24] ; 02FB
    fsub qword ptr [rcx + 64] ; 0301
    fstp qword ptr [rcx + 24] ; 0304
    fld qword ptr [rcx + 160] ; 030A
    fsub qword ptr [rcx + 136] ; 030D
    fsub qword ptr [rcx + 72] ; 0310
    fstp qword ptr [rcx + 136] ; 0313
    fld qword ptr [rcx + 32] ; 0316
    fmul qword ptr [rcx + 32] ; 031C
    fld qword ptr [rcx + 40] ; 0322
    fmul qword ptr [rcx + 40] ; 0328
    faddp st(1), st(0) ; 032E
    fstp qword ptr [rcx + 168] ; 0330
    test dword ptr [rcx + 172], 07fffffffh ; 0333
    jne ikenaga_0360 ; 033A
    cmp dword ptr [rcx + 168], 0 ; 033C
    jne ikenaga_0360 ; 0340
    mov dword ptr [rcx + 144], 0 ; 0342
    mov dword ptr [rcx + 148], 0 ; 0349
    mov dword ptr [rcx + 152], 0 ; 0350
    mov dword ptr [rcx + 156], 0 ; 0357
    jmp ikenaga_03d8 ; 035E
ikenaga_0360:
    fld qword ptr [rcx + 48] ; 0360
    fmul qword ptr [rdx + 48] ; 0366
    nop ; 0368
    nop ; 0369
    nop ; 036A
    nop ; 036B
    fmul qword ptr [rcx + 48] ; 036C
    fdiv qword ptr [rcx + 168] ; 0372
    fld1 ; 0375
    fsubrp st(1), st(0) ; 0377
    fstp qword ptr [rcx + 176] ; 0379
    fld qword ptr [rcx + 40] ; 037C
    fmul qword ptr [rdx + 48] ; 0382
    nop ; 0384
    nop ; 0385
    nop ; 0386
    nop ; 0387
    fmul qword ptr [rcx + 40] ; 0388
    fld qword ptr [rcx + 32] ; 038E
    fmul qword ptr [rcx + 32] ; 0394
    fsubrp st(1), st(0) ; 039A
    fld qword ptr [rcx + 176] ; 039C
    fmul qword ptr [rcx + 32] ; 039F
    fmulp st(1), st(0) ; 03A5
    fstp qword ptr [rcx + 144] ; 03A7
    fld qword ptr [rcx + 32] ; 03AA
    fmul qword ptr [rdx + 48] ; 03B0
    nop ; 03B2
    nop ; 03B3
    nop ; 03B4
    nop ; 03B5
    fmul qword ptr [rcx + 32] ; 03B6
    fld qword ptr [rcx + 40] ; 03BC
    fmul qword ptr [rcx + 40] ; 03C2
    fsubp st(1), st(0) ; 03C8
    fld qword ptr [rcx + 176] ; 03CA
    fmul qword ptr [rcx + 40] ; 03CD
    fmulp st(1), st(0) ; 03D3
    fstp qword ptr [rcx + 152] ; 03D5
ikenaga_03d8:
    fld qword ptr [rcx + 48] ; 03D8
    fmul qword ptr [rcx + 48] ; 03DE
    fld qword ptr [rcx + 168] ; 03E4
    fmul qword ptr [rdx + 48] ; 03E7
    nop ; 03E9
    nop ; 03EA
    nop ; 03EB
    nop ; 03EC
    fsubrp st(1), st(0) ; 03ED
    fmul qword ptr [rcx + 48] ; 03EF
    fstp qword ptr [rcx + 136] ; 03F5
    fld qword ptr [rcx + 128] ; 03F8
    fadd qword ptr [rcx + 144] ; 03FB
    fstp qword ptr [rcx + 32] ; 03FE
    fld qword ptr [rcx + 24] ; 0404
    fadd qword ptr [rcx + 152] ; 040A
    fstp qword ptr [rcx + 40] ; 040D
    fld qword ptr [rcx + 136] ; 0413
    fadd qword ptr [rcx + 160] ; 0416
    fstp qword ptr [rcx + 48] ; 0419
    fnop ; 041F
    fld qword ptr [rcx + 32] ; 0421
    fstp qword ptr [rcx] ; 0427
    fld qword ptr [rcx + 40] ; 0429
    fstp qword ptr [rcx + 8] ; 042F
    fld qword ptr [rcx + 48] ; 0431
    fstp qword ptr [rcx + 16] ; 0437
    fnop ; 0439
IkenagabulbOriginal_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
IkenagabulbOriginal_x87 ENDP


; Original CODE 004B..009B. rcx local XYZ/DE; rdx params; r8d apply; r9d logarithmic mode.
WavespIFSDistance_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    fld qword ptr [rcx] ; 004B
    fmul st(0), st(0) ; 004E
    fld qword ptr [rcx + 8] ; 0050
    fmul st(0), st(0) ; 0054
    faddp st(1), st(0) ; 0056
    fld qword ptr [rdx + 56] ; 0058
    fxch st(1) ; 005B
    fabs ; 005D
    fyl2x ; 005F
    test r9d, r9d
    jne wavespifs_0077 ; 0065
    fld1 ; 0067
    fld st(1) ; 0069
    fprem ; 006B
    f2xm1 ; 006D
    faddp st(1), st(0) ; 006F
    fscale ; 0071
    fxch st(1) ; 0073
    fstp st(0) ; 0075
wavespifs_0077:
    fld qword ptr [rcx + 8] ; 0077
    fld qword ptr [rcx] ; 007B
    fpatan ; 007E
    fmul qword ptr [rdx] ; 0080
    faddp st(1), st(0) ; 0083
    fsin ; 0085
    fabs ; 0087
    fmul qword ptr [rdx + 40] ; 0089
    fadd qword ptr [rcx + 16] ; 008C
    fchs ; 0090
    test r8d, r8d
    jne wavespifs_009b ; 0096
    fdiv qword ptr [rdx + 8] ; 0098
wavespifs_009b:
    fstp qword ptr [rcx + 24] ; 009B
WavespIFSDistance_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
WavespIFSDistance_x87 ENDP


; Original CODE 000E..00BF. rcx XYZ/DE; rdx expanded parameters and internal C.
ABoxMod2Original_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    fld qword ptr [rdx + 24] ; 000E
    fld qword ptr [rcx] ; 0011
    fld st(0) ; 0013
    fsub st(0), st(2) ; 0015
    fabs ; 0017
    fadd st(0), st(1) ; 0019
    fxch st(1) ; 001B
    fadd st(0), st(2) ; 001D
    fabs ; 001F
    fsubrp st(1), st(0) ; 0021
    fld qword ptr [rcx + 8] ; 0023
    fld st(0) ; 0025
    fsub st(0), st(3) ; 0027
    fabs ; 0029
    fadd st(0), st(1) ; 002B
    fxch st(1) ; 002D
    fadd st(0), st(3) ; 002F
    fabs ; 0031
    fsubrp st(1), st(0) ; 0033
    fld qword ptr [rdx + 32] ; 0035
    fxch st(3) ; 0038
    fstp st(0) ; 003A
    fld qword ptr [rcx + 16] ; 003C
    fld st(0) ; 003E
    fsub st(0), st(4) ; 0040
    fabs ; 0042
    fadd st(0), st(1) ; 0044
    fxch st(1) ; 0046
    fadd st(0), st(4) ; 0048
    fabs ; 004A
    fsubrp st(1), st(0) ; 004C
    fld st(0) ; 004E
    fabs ; 0050
    fcom qword ptr [rdx + 40] ; 0052
    fnstsw ax ; 0055
    shr ah, 1 ; 0057
    jbe aboxmod2_0070 ; 0059
    fsub qword ptr [rdx + 40] ; 005B
    fmul st(0), st(0) ; 005E
    fld st(2) ; 0060
    fmul st(0), st(3) ; 0062
    faddp st(1), st(0) ; 0064
    fld st(3) ; 0066
    fmul st(0), st(4) ; 0068
    faddp st(1), st(0) ; 006A
    jmp aboxmod2_007c ; 006C
    nop ; 006E
    nop ; 006F
aboxmod2_0070:
    fstp st(0) ; 0070
    fld st(1) ; 0072
    fmul st(0), st(2) ; 0074
    fld st(3) ; 0076
    fmul st(0), st(4) ; 0078
    faddp st(1), st(0) ; 007A
aboxmod2_007c:
    fcom qword ptr [rdx + 16] ; 007C
    fnstsw ax ; 007F
    shr ah, 1 ; 0081
    jae aboxmod2_008c ; 0083
    fstp st(0) ; 0085
    fld qword ptr [rdx + 8] ; 0087
    jmp aboxmod2_00a2 ; 008A
aboxmod2_008c:
    fld1 ; 008C
    fcom st(1) ; 008E
    fnstsw ax ; 0090
    shr ah, 1 ; 0092
    jb aboxmod2_009d ; 0094
    fstp st(0) ; 0096
    fdivr qword ptr [rdx + 0] ; 0098
    jmp aboxmod2_00a2 ; 009B
aboxmod2_009d:
    fcompp ; 009D
    fld qword ptr [rdx + 0] ; 009F
aboxmod2_00a2:
    fld qword ptr [rcx + 24] ; 00A2
    fmul st(0), st(1) ; 00A5
    fstp qword ptr [rcx + 24] ; 00A7
    fmul st(3), st(0) ; 00AA
    fmul st(2), st(0) ; 00AC
    fmulp st(1), st(0) ; 00AE
    fadd qword ptr [rdx + 64] ; 00B0
    fstp qword ptr [rcx + 16] ; 00B3
    fadd qword ptr [rdx + 56] ; 00B5
    fstp qword ptr [rcx + 8] ; 00B8
    fadd qword ptr [rdx + 48] ; 00BA
    fstp qword ptr [rcx] ; 00BD
    fstp st(0) ; 00BF
ABoxMod2Original_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
ABoxMod2Original_x87 ENDP


; Original IQ_NormBulb CODE, XYZ + local spills; parameters include C at index 6.
IQ_NormBulbOriginal_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    fnop ; 0014
    fld qword ptr [rcx] ; 0016
    fmul st(0), st(0) ; 0018
    fld qword ptr [rcx + 8] ; 001A
    fmul st(0), st(0) ; 001C
    fld qword ptr [rcx + 16] ; 001E
    fmul st(0), st(0) ; 0020
    faddp st(1), st(0) ; 0022
    faddp st(1), st(0) ; 0024
    fsqrt ; 0026
    fst qword ptr [rcx + 72] ; 0028
    fld qword ptr [rdx + 0] ; 002B
    fxch st(1) ; 002E
    fabs ; 0030
    fyl2x ; 0032
    fld1 ; 0034
    fld st(1) ; 0036
    fprem ; 0038
    f2xm1 ; 003A
    faddp st(1), st(0) ; 003C
    fscale ; 003E
    fxch st(1) ; 0040
    fstp st(0) ; 0042
    fstp qword ptr [rcx + 64] ; 0044
    fld qword ptr [rcx + 8] ; 0047
    fmul st(0), st(0) ; 0049
    fld qword ptr [rcx + 16] ; 004B
    fmul st(0), st(0) ; 004D
    faddp st(1), st(0) ; 004F
    fsqrt ; 0051
    fst qword ptr [rcx + 56] ; 0053
    fld qword ptr [rcx] ; 0056
    fpatan ; 0058
    fmul qword ptr [rdx + 0] ; 005A
    fsincos ; 005D
    fstp qword ptr [rcx + 40] ; 005F
    fstp qword ptr [rcx + 48] ; 0062
    nop ; 0065
    fld qword ptr [rcx + 8] ; 0066
    fld qword ptr [rcx + 16] ; 0068
    fpatan ; 006A
    fmul qword ptr [rdx + 8] ; 006C
    fsincos ; 006F
    fstp qword ptr [rcx + 24] ; 0071
    fstp qword ptr [rcx + 32] ; 0074
    fnop ; 0077
    fld qword ptr [rcx + 48] ; 0079
    fmul qword ptr [rcx + 64] ; 007C
    fadd qword ptr [rdx + 48] ; 007F
    fstp qword ptr [rcx] ; 0082
    fld qword ptr [rcx + 40] ; 0084
    fmul qword ptr [rcx + 32] ; 0087
    fmul qword ptr [rcx + 64] ; 008A
    fadd qword ptr [rdx + 56] ; 008D
    fstp qword ptr [rcx + 8] ; 0090
    fld qword ptr [rcx + 40] ; 0092
    fmul qword ptr [rcx + 24] ; 0095
    fmul qword ptr [rdx + 16] ; 0098
    fmul qword ptr [rcx + 64] ; 009B
    fadd qword ptr [rdx + 64] ; 009E
    fstp qword ptr [rcx + 16] ; 00A1
    fnop ; 00A3
IQ_NormBulbOriginal_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
IQ_NormBulbOriginal_x87 ENDP


; Original IQ_NormBulb_NoYZ CODE, XYZ + local spills; parameters include C at index 6.
IQ_NormBulb_NoYZOriginal_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    fnop ; 0014
    fld qword ptr [rcx] ; 0016
    fmul st(0), st(0) ; 0018
    fld qword ptr [rcx + 8] ; 001A
    fmul st(0), st(0) ; 001C
    fld qword ptr [rcx + 16] ; 001E
    fmul st(0), st(0) ; 0020
    faddp st(1), st(0) ; 0022
    faddp st(1), st(0) ; 0024
    fsqrt ; 0026
    fst qword ptr [rcx + 72] ; 0028
    fld qword ptr [rdx + 0] ; 002B
    fxch st(1) ; 002E
    fabs ; 0030
    fyl2x ; 0032
    fld1 ; 0034
    fld st(1) ; 0036
    fprem ; 0038
    f2xm1 ; 003A
    faddp st(1), st(0) ; 003C
    fscale ; 003E
    fxch st(1) ; 0040
    fstp st(0) ; 0042
    fstp qword ptr [rcx + 64] ; 0044
    fld qword ptr [rcx + 8] ; 0047
    fmul st(0), st(0) ; 0049
    fld qword ptr [rcx + 16] ; 004B
    fmul st(0), st(0) ; 004D
    faddp st(1), st(0) ; 004F
    fsqrt ; 0051
    fst qword ptr [rcx + 56] ; 0053
    fld qword ptr [rcx] ; 0056
    fpatan ; 0058
    fmul qword ptr [rdx + 0] ; 005A
    fsincos ; 005D
    fstp qword ptr [rcx + 40] ; 005F
    fstp qword ptr [rcx + 48] ; 0062
    nop ; 0065
    fld qword ptr [rcx + 8] ; 0066
    fld qword ptr [rcx + 16] ; 0068
    fpatan ; 006A
    fmul qword ptr [rdx + 8] ; 006C
    fsincos ; 006F
    fstp qword ptr [rcx + 24] ; 0071
    fstp qword ptr [rcx + 32] ; 0074
    fnop ; 0077
    fld qword ptr [rcx + 48] ; 0079
    fmul qword ptr [rcx + 64] ; 007C
    fadd qword ptr [rdx + 48] ; 007F
    fstp qword ptr [rcx] ; 0082
    fld qword ptr [rcx + 40] ; 0084
    fmul qword ptr [rcx + 32] ; 0087
    fmul qword ptr [rcx + 64] ; 008A
    fstp qword ptr [rcx + 8] ; 008D
    fld qword ptr [rcx + 40] ; 008F
    fmul qword ptr [rcx + 24] ; 0092
    fmul qword ptr [rdx + 16] ; 0095
    fmul qword ptr [rcx + 64] ; 0098
    fstp qword ptr [rcx + 16] ; 009B
    fnop ; 009D
IQ_NormBulb_NoYZOriginal_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
IQ_NormBulb_NoYZOriginal_x87 ENDP


; Original 0011..0051; XYZ/DE/sample and four Extended spills.
ABoxMapSFoldPrepare_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    fld qword ptr [rdx + 24] ; 0011
    fld qword ptr [rcx] ; 0014
    fld st(0) ; 0016
    fsub st(0), st(2) ; 0018
    fabs ; 001A
    fadd st(0), st(1) ; 001C
    fxch st(1) ; 001E
    fadd st(0), st(2) ; 0020
    fabs ; 0022
    fsubrp st(1), st(0) ; 0024
    fst qword ptr [rcx + 32] ; 0026
    fld qword ptr [rcx + 8] ; 0029
    fld st(0) ; 002B
    fsub st(0), st(3) ; 002D
    fabs ; 002F
    fadd st(0), st(1) ; 0031
    fxch st(1) ; 0033
    fadd st(0), st(3) ; 0035
    fabs ; 0037
    fsubrp st(1), st(0) ; 0039
    fst qword ptr [rcx + 40] ; 003B
    fld qword ptr [rcx + 16] ; 003F
    fld st(0) ; 0041
    fsub st(0), st(4) ; 0043
    fabs ; 0045
    fadd st(0), st(1) ; 0047
    fxch st(1) ; 0049
    fadd st(0), st(4) ; 004B
    fabs ; 004D
    fsubrp st(1), st(0) ; 004F
    fst qword ptr [rcx + 48] ; 0051
    fstp tbyte ptr [rcx + 64]
    fstp tbyte ptr [rcx + 80]
    fstp tbyte ptr [rcx + 96]
    fstp tbyte ptr [rcx + 112]
ABoxMapSFoldPrepare_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
ABoxMapSFoldPrepare_x87 ENDP


; Original 0062..00D6; Single options expanded exactly to Double.
ABoxMapSFoldFinish_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    fld tbyte ptr [rcx + 112]
    fld tbyte ptr [rcx + 96]
    fld tbyte ptr [rcx + 80]
    fld tbyte ptr [rcx + 64]
    fld qword ptr [rdx + 40] ; 0062
    fld qword ptr [rdx + 48] ; 0065
    fld qword ptr [rcx + 32] ; 0068
    fadd st(0), st(2) ; 006B
    fmul st(0), st(1) ; 006D
    fadd st(0), st(5) ; 006F
    fmul st(0), st(0) ; 0071
    fld qword ptr [rcx + 40] ; 0073
    fadd st(0), st(3) ; 0077
    fmul st(0), st(2) ; 0079
    fadd st(0), st(5) ; 007B
    fmul st(0), st(0) ; 007D
    faddp st(1), st(0) ; 007F
    fld qword ptr [rcx + 48] ; 0081
    fadd st(0), st(3) ; 0085
    fmul st(0), st(2) ; 0087
    fadd st(0), st(4) ; 0089
    fmul st(0), st(0) ; 008B
    faddp st(1), st(0) ; 008D
    fcom qword ptr [rdx + 16] ; 008F
    fnstsw ax ; 0092
    shr ah, 1 ; 0094
    jae aboxmapsfold_009f ; 0096
    fstp st(0) ; 0098
    fld qword ptr [rdx + 8] ; 009A
    jmp aboxmapsfold_00b5 ; 009D
aboxmapsfold_009f:
    fld1 ; 009F
    fcom st(1) ; 00A1
    fnstsw ax ; 00A3
    shr ah, 1 ; 00A5
    jb aboxmapsfold_00b0 ; 00A7
    fstp st(0) ; 00A9
    fdivr qword ptr [rdx] ; 00AB
    jmp aboxmapsfold_00b5 ; 00AE
aboxmapsfold_00b0:
    fcompp ; 00B0
    fld qword ptr [rdx] ; 00B2
aboxmapsfold_00b5:
    fld qword ptr [rcx + 24] ; 00B5
    fmul st(0), st(1) ; 00B8
    fstp qword ptr [rcx + 24] ; 00BA
    fmul st(5), st(0) ; 00BD
    fmul st(4), st(0) ; 00BF
    fmulp st(3), st(0) ; 00C1
    fcompp ; 00C3
    fadd qword ptr [rdx + 72] ; 00C5
    fstp qword ptr [rcx + 16] ; 00C8
    fadd qword ptr [rdx + 64] ; 00CB
    fstp qword ptr [rcx + 8] ; 00CE
    fadd qword ptr [rdx + 56] ; 00D1
    fstp qword ptr [rcx] ; 00D4
    fstp st(0) ; 00D6
ABoxMapSFoldFinish_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
ABoxMapSFoldFinish_x87 ENDP


; Original fold/radius/C-swap sequence; rcx XYZ/DE/VaryScale, rdx params, r8d swap, r9 first.
ABoxVS_icen1Original_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    cmp dword ptr [r9], 0 ; 0014
    jg aboxvs_icen1_0023 ; 0018
    inc dword ptr [r9] ; 001A
    fld qword ptr [rdx + 0] ; 001D
    fstp qword ptr [rcx + 32] ; 0020
aboxvs_icen1_0023:
    fld qword ptr [rcx + 32] ; 0023
    fld1 ; 0026
    fld st(1) ; 0028
    fabs ; 002A
    fsubrp st(1), st(0) ; 002C
    fmul qword ptr [rdx + 24] ; 002E
    faddp st(1), st(0) ; 0031
    fstp qword ptr [rcx + 32] ; 0033
    fld qword ptr [rdx + 16] ; 0036
    fld st(0) ; 0039
    fld st(0) ; 003B
    fadd st(2), st(0) ; 003D
    fchs ; 003F
    fld qword ptr [rcx] ; 0041
    fcom st(2) ; 0043
    fnstsw ax ; 0045
    shr ah, 1 ; 0047
    jb aboxvs_icen1_0051 ; 0049
    fchs ; 004B
    fadd st(0), st(3) ; 004D
    jmp aboxvs_icen1_005d ; 004F
aboxvs_icen1_0051:
    fcom st(1) ; 0051
    fnstsw ax ; 0053
    shr ah, 1 ; 0055
    jae aboxvs_icen1_005d ; 0057
    fchs ; 0059
    fsub st(0), st(3) ; 005B
aboxvs_icen1_005d:
    fld qword ptr [rcx + 8] ; 005D
    fcom st(3) ; 005F
    fnstsw ax ; 0061
    shr ah, 1 ; 0063
    jb aboxvs_icen1_006d ; 0065
    fchs ; 0067
    fadd st(0), st(4) ; 0069
    jmp aboxvs_icen1_0079 ; 006B
aboxvs_icen1_006d:
    fcom st(2) ; 006D
    fnstsw ax ; 006F
    shr ah, 1 ; 0071
    jae aboxvs_icen1_0079 ; 0073
    fchs ; 0075
    fsub st(0), st(4) ; 0077
aboxvs_icen1_0079:
    fld qword ptr [rcx + 16] ; 0079
    fcom st(4) ; 007B
    fnstsw ax ; 007D
    shr ah, 1 ; 007F
    jb aboxvs_icen1_0089 ; 0081
    fchs ; 0083
    fadd st(0), st(5) ; 0085
    jmp aboxvs_icen1_0095 ; 0087
aboxvs_icen1_0089:
    fcom st(3) ; 0089
    fnstsw ax ; 008B
    shr ah, 1 ; 008D
    jae aboxvs_icen1_0095 ; 008F
    fchs ; 0091
    fsub st(0), st(5) ; 0093
aboxvs_icen1_0095:
    fxch st(3) ; 0095
    fstp st(0) ; 0097
    fxch st(3) ; 0099
    fstp st(0) ; 009B
    fxch st(3) ; 009D
    fstp st(0) ; 009F
    test r8d, r8d
    jne aboxvs_icen1_00c1 ; 00A5
    fld qword ptr [rdx + 40] ; 00A7
    fadd st(0), st(1) ; 00AA
    fstp st(0) ; 00AC
    nop ; 00AE
    fld qword ptr [rdx + 48] ; 00AF
    fadd st(0), st(2) ; 00B2
    fstp st(0) ; 00B4
    nop ; 00B6
    fld qword ptr [rdx + 56] ; 00B7
    fadd st(0), st(3) ; 00BA
    fstp st(0) ; 00BC
    nop ; 00BE
    jmp aboxvs_icen1_00d9 ; 00BF
aboxvs_icen1_00c1:
    fld qword ptr [rdx + 72] ; 00C1
    fadd st(0), st(1) ; 00C4
    fstp st(0) ; 00C6
    nop ; 00C8
    fld qword ptr [rdx + 80] ; 00C9
    fadd st(0), st(2) ; 00CC
    fstp st(0) ; 00CE
    nop ; 00D0
    fld qword ptr [rdx + 88] ; 00D1
    fadd st(0), st(3) ; 00D4
    fstp st(0) ; 00D6
    nop ; 00D8
aboxvs_icen1_00d9:
    fld st(0) ; 00D9
    fmul st(0), st(1) ; 00DB
    fld st(2) ; 00DD
    fmul st(0), st(3) ; 00DF
    faddp st(1), st(0) ; 00E1
    fld st(3) ; 00E3
    fmul st(0), st(4) ; 00E5
    faddp st(1), st(0) ; 00E7
    fld qword ptr [rdx + 8] ; 00E9
    fmul st(0), st(0) ; 00EC
    fcomp st(1) ; 00EE
    nop ; 00F0
    fnstsw ax ; 00F1
    shr ah, 1 ; 00F3
    jb aboxvs_icen1_010a ; 00F5
    fstp st(0) ; 00F7
    fld qword ptr [rcx + 32] ; 00F9
    fmul qword ptr [rdx + 32] ; 00FC
    fld qword ptr [rdx + 8] ; 00FF
    fmul st(0), st(0) ; 0102
    fdivp st(1), st(0) ; 0104
    nop ; 0106
    nop ; 0107
    jmp aboxvs_icen1_0124 ; 0108
aboxvs_icen1_010a:
    fld qword ptr [rdx + 32] ; 010A
    fcom st(1) ; 010D
    fnstsw ax ; 010F
    shr ah, 1 ; 0111
    jb aboxvs_icen1_011f ; 0113
    fstp st(0) ; 0115
    fdiv qword ptr [rdx + 32] ; 0117
    fdivr qword ptr [rcx + 32] ; 011A
    jmp aboxvs_icen1_0124 ; 011D
aboxvs_icen1_011f:
    fcompp ; 011F
    fld qword ptr [rcx + 32] ; 0121
aboxvs_icen1_0124:
    fld qword ptr [rcx + 24] ; 0124
    fmul st(0), st(1) ; 0127
    fstp qword ptr [rcx + 24] ; 0129
    fmul st(3), st(0) ; 012C
    fmul st(2), st(0) ; 012E
    fmulp st(1), st(0) ; 0130
    test r8d, r8d
    jne aboxvs_icen1_0152 ; 0136
    fadd qword ptr [rdx + 88] ; 0138
    fsub qword ptr [rdx + 56] ; 013B
    fstp qword ptr [rcx + 16] ; 013E
    fadd qword ptr [rdx + 80] ; 0140
    fsub qword ptr [rdx + 48] ; 0143
    fstp qword ptr [rcx + 8] ; 0146
    fadd qword ptr [rdx + 72] ; 0148
    fsub qword ptr [rdx + 40] ; 014B
    fstp qword ptr [rcx] ; 014E
    jmp aboxvs_icen1_016a ; 0150
aboxvs_icen1_0152:
    fsub qword ptr [rdx + 88] ; 0152
    fadd qword ptr [rdx + 56] ; 0155
    fstp qword ptr [rcx + 16] ; 0158
    fsub qword ptr [rdx + 80] ; 015A
    fadd qword ptr [rdx + 48] ; 015D
    fstp qword ptr [rcx + 8] ; 0160
    fsub qword ptr [rdx + 72] ; 0162
    fadd qword ptr [rdx + 40] ; 0165
    fstp qword ptr [rcx] ; 0168
aboxvs_icen1_016a:
ABoxVS_icen1Original_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
ABoxVS_icen1Original_x87 ENDP


; Original fold/radius/C-swap sequence; rcx XYZ/DE/VaryScale, rdx params, r8d swap, r9 first.
ABoxVS_icen2Original_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    cmp dword ptr [r9], 0 ; 0014
    jg aboxvs_icen2_0023 ; 0018
    inc dword ptr [r9] ; 001A
    fld qword ptr [rdx + 0] ; 001D
    fstp qword ptr [rcx + 32] ; 0020
aboxvs_icen2_0023:
    fld qword ptr [rcx + 32] ; 0023
    fld1 ; 0026
    fld st(1) ; 0028
    fabs ; 002A
    fsubrp st(1), st(0) ; 002C
    fmul qword ptr [rdx + 32] ; 002E
    faddp st(1), st(0) ; 0031
    fstp qword ptr [rcx + 32] ; 0033
    fld qword ptr [rdx + 24] ; 0036
    fld st(0) ; 0039
    fld st(0) ; 003B
    fadd st(2), st(0) ; 003D
    fchs ; 003F
    fld qword ptr [rcx] ; 0041
    fcom st(2) ; 0043
    fnstsw ax ; 0045
    shr ah, 1 ; 0047
    jb aboxvs_icen2_0051 ; 0049
    fchs ; 004B
    fadd st(0), st(3) ; 004D
    jmp aboxvs_icen2_005d ; 004F
aboxvs_icen2_0051:
    fcom st(1) ; 0051
    fnstsw ax ; 0053
    shr ah, 1 ; 0055
    jae aboxvs_icen2_005d ; 0057
    fchs ; 0059
    fsub st(0), st(3) ; 005B
aboxvs_icen2_005d:
    fld qword ptr [rcx + 8] ; 005D
    fcom st(3) ; 005F
    fnstsw ax ; 0061
    shr ah, 1 ; 0063
    jb aboxvs_icen2_006d ; 0065
    fchs ; 0067
    fadd st(0), st(4) ; 0069
    jmp aboxvs_icen2_0079 ; 006B
aboxvs_icen2_006d:
    fcom st(2) ; 006D
    fnstsw ax ; 006F
    shr ah, 1 ; 0071
    jae aboxvs_icen2_0079 ; 0073
    fchs ; 0075
    fsub st(0), st(4) ; 0077
aboxvs_icen2_0079:
    fld qword ptr [rcx + 16] ; 0079
    fcom st(4) ; 007B
    fnstsw ax ; 007D
    shr ah, 1 ; 007F
    jb aboxvs_icen2_0089 ; 0081
    fchs ; 0083
    fadd st(0), st(5) ; 0085
    jmp aboxvs_icen2_0095 ; 0087
aboxvs_icen2_0089:
    fcom st(3) ; 0089
    fnstsw ax ; 008B
    shr ah, 1 ; 008D
    jae aboxvs_icen2_0095 ; 008F
    fchs ; 0091
    fsub st(0), st(5) ; 0093
aboxvs_icen2_0095:
    fxch st(3) ; 0095
    fstp st(0) ; 0097
    fxch st(3) ; 0099
    fstp st(0) ; 009B
    fxch st(3) ; 009D
    fstp st(0) ; 009F
    test r8d, r8d
    jne aboxvs_icen2_00be ; 00A5
    fld st(0) ; 00A7
    fadd qword ptr [rdx + 48] ; 00A9
    fmul st(0), st(1) ; 00AC
    fld st(2) ; 00AE
    fadd qword ptr [rdx + 56] ; 00B0
    fmul st(0), st(3) ; 00B3
    faddp st(1), st(0) ; 00B5
    fld st(3) ; 00B7
    fadd qword ptr [rdx + 64] ; 00B9
    jmp aboxvs_icen2_00d3 ; 00BC
aboxvs_icen2_00be:
    fld st(0) ; 00BE
    fadd qword ptr [rdx + 88] ; 00C0
    fmul st(0), st(1) ; 00C3
    fld st(2) ; 00C5
    fadd qword ptr [rdx + 96] ; 00C7
    fmul st(0), st(3) ; 00CA
    faddp st(1), st(0) ; 00CC
    fld st(3) ; 00CE
    fadd qword ptr [rdx + 80] ; 00D0
aboxvs_icen2_00d3:
    fmul st(0), st(4) ; 00D3
    faddp st(1), st(0) ; 00D5
    fld qword ptr [rdx + 40] ; 00D7
    fxch st(1) ; 00DA
    fldln2 ; 00DC
    fxch st(1) ; 00DE
    fyl2x ; 00E0
    fxch st(1) ; 00E2
    fmulp st(1), st(0) ; 00E4
    fldl2e ; 00E6
    fmulp st(1), st(0) ; 00E8
    fld st(0) ; 00EA
    frndint ; 00EC
    fsub st(1), st(0) ; 00EE
    fxch st(1) ; 00F0
    f2xm1 ; 00F2
    fld1 ; 00F4
    faddp st(1), st(0) ; 00F6
    fscale ; 00F8
    fstp st(1) ; 00FA
    fcom qword ptr [rdx + 16] ; 00FC
    fnstsw ax ; 00FF
    shr ah, 1 ; 0101
    jae aboxvs_icen2_010c ; 0103
    fstp st(0) ; 0105
    fld qword ptr [rdx + 8] ; 0107
    jmp aboxvs_icen2_0122 ; 010A
aboxvs_icen2_010c:
    fld1 ; 010C
    fcom st(1) ; 010E
    fnstsw ax ; 0110
    shr ah, 1 ; 0112
    jb aboxvs_icen2_011d ; 0114
    fstp st(0) ; 0116
    fdivr qword ptr [rcx + 32] ; 0118
    jmp aboxvs_icen2_0122 ; 011B
aboxvs_icen2_011d:
    fcompp ; 011D
    fld qword ptr [rcx + 32] ; 011F
aboxvs_icen2_0122:
    fld qword ptr [rcx + 24] ; 0122
    fmul st(0), st(1) ; 0125
    fstp qword ptr [rcx + 24] ; 0127
    fmul st(3), st(0) ; 012A
    fmul st(2), st(0) ; 012C
    fmulp st(1), st(0) ; 012E
    test r8d, r8d
    jne aboxvs_icen2_0147 ; 0134
    fadd qword ptr [rdx + 104] ; 0136
    fstp qword ptr [rcx + 16] ; 0139
    fadd qword ptr [rdx + 96] ; 013B
    fstp qword ptr [rcx + 8] ; 013E
    fadd qword ptr [rdx + 88] ; 0140
    fstp qword ptr [rcx] ; 0143
    jmp aboxvs_icen2_0156 ; 0145
aboxvs_icen2_0147:
    fadd qword ptr [rdx + 64] ; 0147
    fstp qword ptr [rcx + 16] ; 014A
    fadd qword ptr [rdx + 56] ; 014C
    fstp qword ptr [rcx + 8] ; 014F
    fadd qword ptr [rdx + 48] ; 0151
    fstp qword ptr [rcx] ; 0154
aboxvs_icen2_0156:
ABoxVS_icen2Original_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
ABoxVS_icen2Original_x87 ENDP


; Original IQ-bulb; local Double spills and internal C preserved.
IQ_bulbOriginal_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    fnop ; 0014
    fld qword ptr [rcx] ; 0016
    fmul st(0), st(0) ; 0018
    fld qword ptr [rcx + 8] ; 001A
    fmul st(0), st(0) ; 001C
    fld qword ptr [rcx + 16] ; 001E
    fmul st(0), st(0) ; 0020
    faddp st(1), st(0) ; 0022
    faddp st(1), st(0) ; 0024
    fsqrt ; 0026
    fst qword ptr [rcx + 72] ; 0028
    fld qword ptr [rdx + 0] ; 002B
    fxch st(1) ; 002E
    fabs ; 0030
    fyl2x ; 0032
    fld1 ; 0034
    fld st(1) ; 0036
    fprem ; 0038
    f2xm1 ; 003A
    faddp st(1), st(0) ; 003C
    fscale ; 003E
    fxch st(1) ; 0040
    fstp st(0) ; 0042
    fstp qword ptr [rcx + 64] ; 0044
    fld qword ptr [rcx] ; 0047
    fmul st(0), st(0) ; 0049
    fld qword ptr [rcx + 16] ; 004B
    fmul st(0), st(0) ; 004D
    faddp st(1), st(0) ; 004F
    fsqrt ; 0051
    fst qword ptr [rcx + 56] ; 0053
    fld qword ptr [rcx + 8] ; 0056
    fpatan ; 0058
    fmul qword ptr [rdx + 0] ; 005A
    fsincos ; 005D
    fstp qword ptr [rcx + 40] ; 005F
    fstp qword ptr [rcx + 48] ; 0062
    nop ; 0065
    fld qword ptr [rcx] ; 0066
    fld qword ptr [rcx + 16] ; 0068
    fpatan ; 006A
    fmul qword ptr [rdx + 8] ; 006C
    fsincos ; 006F
    fstp qword ptr [rcx + 24] ; 0071
    fstp qword ptr [rcx + 32] ; 0074
    fnop ; 0077
    fld qword ptr [rcx + 48] ; 0079
    fmul qword ptr [rcx + 32] ; 007C
    fmul qword ptr [rcx + 64] ; 007F
    fadd qword ptr [rdx + 48] ; 0082
    fstp qword ptr [rcx] ; 0085
    fld qword ptr [rcx + 40] ; 0087
    fmul qword ptr [rcx + 64] ; 008A
    fadd qword ptr [rdx + 56] ; 008D
    fstp qword ptr [rcx + 8] ; 0090
    fld qword ptr [rcx + 48] ; 0092
    fmul qword ptr [rcx + 24] ; 0095
    fmul qword ptr [rdx + 16] ; 0098
    fmul qword ptr [rcx + 64] ; 009B
    fadd qword ptr [rdx + 64] ; 009E
    fstp qword ptr [rcx + 16] ; 00A1
    fnop ; 00A3
IQ_bulbOriginal_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
IQ_bulbOriginal_x87 ENDP


; Original IQ-bulb old; local Double spills and internal C preserved.
IQ_bulb_oldOriginal_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    fnop ; 0014
    fld qword ptr [rcx] ; 0016
    fmul st(0), st(0) ; 0018
    fld qword ptr [rcx + 8] ; 001A
    fmul st(0), st(0) ; 001C
    fld qword ptr [rcx + 16] ; 001E
    fmul st(0), st(0) ; 0020
    faddp st(1), st(0) ; 0022
    faddp st(1), st(0) ; 0024
    fsqrt ; 0026
    fst qword ptr [rcx + 24] ; 0028
    fld qword ptr [rdx + 0] ; 002B
    fxch st(1) ; 002E
    fabs ; 0030
    fyl2x ; 0032
    fld1 ; 0034
    fld st(1) ; 0036
    fprem ; 0038
    f2xm1 ; 003A
    faddp st(1), st(0) ; 003C
    fscale ; 003E
    fxch st(1) ; 0040
    fstp st(0) ; 0042
    fstp qword ptr [rcx + 80] ; 0044
    fnop ; 0047
    test r8d, r8d
    jne iq_bulb_old_0057 ; 004D
    fld qword ptr [rcx + 80] ; 004F
    fstp qword ptr [rcx + 24] ; 0052
    jmp iq_bulb_old_0076 ; 0055
iq_bulb_old_0057:
    fld qword ptr [rcx + 24] ; 0057
    fld qword ptr [rdx + 8] ; 005A
    fxch st(1) ; 005D
    fabs ; 005F
    fyl2x ; 0061
    fld1 ; 0063
    fld st(1) ; 0065
    fprem ; 0067
    f2xm1 ; 0069
    faddp st(1), st(0) ; 006B
    fscale ; 006D
    fxch st(1) ; 006F
    fstp st(0) ; 0071
    fstp qword ptr [rcx + 24] ; 0073
iq_bulb_old_0076:
    fld qword ptr [rcx] ; 0076
    fmul st(0), st(0) ; 0078
    fld qword ptr [rcx + 16] ; 007A
    fmul st(0), st(0) ; 007C
    faddp st(1), st(0) ; 007E
    nop ; 0080
    nop ; 0081
    fsqrt ; 0082
    fld qword ptr [rcx] ; 0084
    fmul st(0), st(0) ; 0086
    fld qword ptr [rcx + 8] ; 0088
    fmul st(0), st(0) ; 008A
    fld qword ptr [rcx + 16] ; 008C
    fmul st(0), st(0) ; 008E
    faddp st(1), st(0) ; 0090
    faddp st(1), st(0) ; 0092
    fsqrt ; 0094
    fadd qword ptr [rcx + 8] ; 0096
    fpatan ; 0098
    fadd st(0), st(0) ; 009A
    fmul qword ptr [rdx + 8] ; 009C
    fsincos ; 009F
    fstp qword ptr [rcx + 72] ; 00A1
    fstp qword ptr [rcx + 64] ; 00A4
    fld qword ptr [rcx + 16] ; 00A7
    fld qword ptr [rcx] ; 00A9
    nop ; 00AB
    nop ; 00AC
    fpatan ; 00AD
    test r9d, r9d
    jne iq_bulb_old_00df ; 00B3
    fld qword ptr [rcx] ; 00B5
    nop ; 00B7
    fldz ; 00B8
    fcompp ; 00BA
    fnstsw ax ; 00BC
    and ah, 041h ; 00BE
    ja iq_bulb_old_00df ; 00C1
    fld qword ptr [rcx + 16] ; 00C3
    nop ; 00C5
    fldz ; 00C6
    fcompp ; 00C8
    fnstsw ax ; 00CA
    and ah, 041h ; 00CC
    ja iq_bulb_old_00d9 ; 00CF
    fldpi ; 00D1
    fmul qword ptr [rdx + 40] ; 00D3
    faddp st(1), st(0) ; 00D5
    jmp iq_bulb_old_00df ; 00D7
iq_bulb_old_00d9:
    fldpi ; 00D9
    fmul qword ptr [rdx + 40] ; 00DB
    fsubp st(1), st(0) ; 00DD
iq_bulb_old_00df:
    fmul qword ptr [rdx + 8] ; 00DF
    fsincos ; 00E2
    fstp qword ptr [rcx + 56] ; 00E4
    fstp qword ptr [rcx + 48] ; 00E7
    fnop ; 00EA
    fld qword ptr [rcx + 80] ; 00EC
    fmul qword ptr [rcx + 64] ; 00EF
    fmul qword ptr [rcx + 48] ; 00F2
    nop ; 00F5
    nop ; 00F6
    fadd qword ptr [rdx + 48] ; 00F7
    fstp qword ptr [rcx + 40] ; 00FA
    fnop ; 00FD
    fld qword ptr [rcx + 80] ; 00FF
    fmul qword ptr [rcx + 72] ; 0102
    nop ; 0105
    nop ; 0106
    nop ; 0107
    nop ; 0108
    nop ; 0109
    fadd qword ptr [rdx + 56] ; 010A
    fstp qword ptr [rcx + 32] ; 010D
    fnop ; 0110
    fld qword ptr [rcx + 24] ; 0112
    fmul qword ptr [rcx + 56] ; 0115
    fmul qword ptr [rcx + 64] ; 0118
    fmul qword ptr [rdx + 16] ; 011B
    fadd qword ptr [rdx + 64] ; 011E
    fstp qword ptr [rcx + 24] ; 0121
    fnop ; 0124
    fld qword ptr [rcx + 40] ; 0126
    fld qword ptr [rcx + 32] ; 0129
    fld qword ptr [rcx + 24] ; 012C
    fstp qword ptr [rcx + 16] ; 012F
    fstp qword ptr [rcx + 8] ; 0131
    fstp qword ptr [rcx] ; 0133
    fnop ; 0135
IQ_bulb_oldOriginal_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
IQ_bulb_oldOriginal_x87 ENDP


; Original IQ_PineTree; local Double spills and internal C preserved.
IQ_PineTreeOriginal_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    fnop ; 0014
    fld qword ptr [rcx] ; 0016
    fmul st(0), st(0) ; 0018
    fld qword ptr [rcx + 8] ; 001A
    fmul st(0), st(0) ; 001C
    fld qword ptr [rcx + 16] ; 001E
    fmul st(0), st(0) ; 0020
    faddp st(1), st(0) ; 0022
    faddp st(1), st(0) ; 0024
    fsqrt ; 0026
    fst qword ptr [rcx + 72] ; 0028
    fld qword ptr [rdx + 0] ; 002B
    fxch st(1) ; 002E
    fabs ; 0030
    fyl2x ; 0032
    fld1 ; 0034
    fld st(1) ; 0036
    fprem ; 0038
    f2xm1 ; 003A
    faddp st(1), st(0) ; 003C
    fscale ; 003E
    fxch st(1) ; 0040
    fstp st(0) ; 0042
    fstp qword ptr [rcx + 64] ; 0044
    fld qword ptr [rcx + 8] ; 0047
    fmul st(0), st(0) ; 0049
    fld qword ptr [rcx + 16] ; 004B
    fmul st(0), st(0) ; 004D
    faddp st(1), st(0) ; 004F
    fsqrt ; 0051
    fst qword ptr [rcx + 56] ; 0053
    fld qword ptr [rcx] ; 0056
    fpatan ; 0058
    fmul qword ptr [rdx + 0] ; 005A
    fsincos ; 005D
    fstp qword ptr [rcx + 40] ; 005F
    fstp qword ptr [rcx + 48] ; 0062
    nop ; 0065
    fld qword ptr [rcx + 8] ; 0066
    fld qword ptr [rcx + 16] ; 0068
    fpatan ; 006A
    fmul qword ptr [rdx + 8] ; 006C
    fsincos ; 006F
    fstp qword ptr [rcx + 24] ; 0071
    fstp qword ptr [rcx + 32] ; 0074
    fnop ; 0077
    fld qword ptr [rcx + 40] ; 0079
    fmul qword ptr [rcx + 64] ; 007C
    fadd qword ptr [rdx + 48] ; 007F
    fstp qword ptr [rcx] ; 0082
    fld qword ptr [rcx + 48] ; 0084
    fmul qword ptr [rcx + 24] ; 0087
    fmul qword ptr [rcx + 64] ; 008A
    fadd qword ptr [rdx + 56] ; 008D
    fstp qword ptr [rcx + 8] ; 0090
    fld qword ptr [rcx + 48] ; 0092
    fmul qword ptr [rcx + 32] ; 0095
    fmul qword ptr [rdx + 16] ; 0098
    fmul qword ptr [rcx + 64] ; 009B
    fadd qword ptr [rdx + 64] ; 009E
    fstp qword ptr [rcx + 16] ; 00A1
    fnop ; 00A3
IQ_PineTreeOriginal_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
IQ_PineTreeOriginal_x87 ENDP


; Original IQ_PineTree_NoYZ; local Double spills and internal C preserved.
IQ_PineTree_NoYZOriginal_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    fnop ; 0014
    fld qword ptr [rcx] ; 0016
    fmul st(0), st(0) ; 0018
    fld qword ptr [rcx + 8] ; 001A
    fmul st(0), st(0) ; 001C
    fld qword ptr [rcx + 16] ; 001E
    fmul st(0), st(0) ; 0020
    faddp st(1), st(0) ; 0022
    faddp st(1), st(0) ; 0024
    fsqrt ; 0026
    fst qword ptr [rcx + 72] ; 0028
    fld qword ptr [rdx + 0] ; 002B
    fxch st(1) ; 002E
    fabs ; 0030
    fyl2x ; 0032
    fld1 ; 0034
    fld st(1) ; 0036
    fprem ; 0038
    f2xm1 ; 003A
    faddp st(1), st(0) ; 003C
    fscale ; 003E
    fxch st(1) ; 0040
    fstp st(0) ; 0042
    fstp qword ptr [rcx + 64] ; 0044
    fld qword ptr [rcx + 8] ; 0047
    fmul st(0), st(0) ; 0049
    fld qword ptr [rcx + 16] ; 004B
    fmul st(0), st(0) ; 004D
    faddp st(1), st(0) ; 004F
    fsqrt ; 0051
    fst qword ptr [rcx + 56] ; 0053
    fld qword ptr [rcx] ; 0056
    fpatan ; 0058
    fmul qword ptr [rdx + 0] ; 005A
    fsincos ; 005D
    fstp qword ptr [rcx + 40] ; 005F
    fstp qword ptr [rcx + 48] ; 0062
    nop ; 0065
    fld qword ptr [rcx + 8] ; 0066
    fld qword ptr [rcx + 16] ; 0068
    fpatan ; 006A
    fmul qword ptr [rdx + 8] ; 006C
    fsincos ; 006F
    fstp qword ptr [rcx + 24] ; 0071
    fstp qword ptr [rcx + 32] ; 0074
    fnop ; 0077
    fld qword ptr [rcx + 40] ; 0079
    fmul qword ptr [rcx + 64] ; 007C
    fadd qword ptr [rdx + 48] ; 007F
    fstp qword ptr [rcx] ; 0082
    fld qword ptr [rcx + 48] ; 0084
    fmul qword ptr [rcx + 24] ; 0087
    fmul qword ptr [rcx + 64] ; 008A
    fstp qword ptr [rcx + 8] ; 008D
    fld qword ptr [rcx + 48] ; 008F
    fmul qword ptr [rcx + 32] ; 0092
    fmul qword ptr [rdx + 16] ; 0095
    fmul qword ptr [rcx + 64] ; 0098
    fstp qword ptr [rcx + 16] ; 009B
    fnop ; 009D
IQ_PineTree_NoYZOriginal_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
IQ_PineTree_NoYZOriginal_x87 ENDP


; Original hexgridIFS local matrix/shape; rcx XYZ/scratch/DE, rdx params, r8d apply.
HexGridSolidDistance_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    fld qword ptr [rcx + 16] ; 004E
    fld qword ptr [rcx + 8] ; 0052
    fld qword ptr [rcx] ; 0056
    fld st(0) ; 0059
    fmul qword ptr [rdx + 104] ; 005B
    fld st(2) ; 005E
    fmul qword ptr [rdx + 112] ; 0060
    faddp st(1), st(0) ; 0063
    fld st(3) ; 0065
    fmul qword ptr [rdx + 120] ; 0067
    faddp st(1), st(0) ; 006A
    fld st(1) ; 006C
    fmul qword ptr [rdx + 56] ; 006E
    fld st(3) ; 0071
    fmul qword ptr [rdx + 64] ; 0073
    faddp st(1), st(0) ; 0076
    fld st(4) ; 0078
    fmul qword ptr [rdx + 72] ; 007A
    faddp st(1), st(0) ; 007D
    fxch st(2) ; 007F
    fmul qword ptr [rdx + 80] ; 0081
    fxch st(3) ; 0084
    fmul qword ptr [rdx + 88] ; 0086
    faddp st(3), st(0) ; 0089
    fxch st(3) ; 008B
    fmul qword ptr [rdx + 96] ; 008D
    faddp st(2), st(0) ; 0090
    fstp qword ptr [rcx] ; 0092
    fstp qword ptr [rcx + 8] ; 0095
    fstp qword ptr [rcx + 16] ; 0099
    fld qword ptr [rcx] ; 00A0
    fsub qword ptr [rdx + 40] ; 00A3
    fabs ; 00A6
    fstp qword ptr [rcx] ; 00A8
    fld qword ptr [rcx] ; 00AB
    fdiv qword ptr [rdx + 40] ; 00AE
    fmul qword ptr [rdx + 160] ; 00B1
    fsub qword ptr [rdx + 160] ; 00B3
    frndint ; 00B5
    fadd st(0), st(0) ; 00B7
    fld1 ; 00B9
    faddp st(1), st(0) ; 00BB
    fmul qword ptr [rdx + 40] ; 00BD
    fchs ; 00C0
    fadd qword ptr [rcx] ; 00C2
    fabs ; 00C5
    fstp qword ptr [rcx] ; 00C7
    fld qword ptr [rdx + 40] ; 00CA
    fmul qword ptr [rdx + 168] ; 00CD
    fstp qword ptr [rcx + 24] ; 00D0
    fld qword ptr [rcx + 8] ; 00D4
    fsub qword ptr [rcx + 24] ; 00D8
    fabs ; 00DC
    fstp qword ptr [rcx + 8] ; 00DE
    fld qword ptr [rcx + 8] ; 00E2
    fdiv qword ptr [rcx + 24] ; 00E6
    fmul qword ptr [rdx + 160] ; 00EA
    fsub qword ptr [rdx + 160] ; 00EC
    frndint ; 00EE
    fadd st(0), st(0) ; 00F0
    fld1 ; 00F2
    faddp st(1), st(0) ; 00F4
    fmul qword ptr [rcx + 24] ; 00F6
    fchs ; 00FA
    fadd qword ptr [rcx + 8] ; 00FC
    fabs ; 0100
    fstp qword ptr [rcx + 8] ; 0102
    fld qword ptr [rcx] ; 0106
    fmul qword ptr [rdx + 176] ; 0109
    fld qword ptr [rcx + 8] ; 010C
    fmul qword ptr [rdx + 184] ; 0110
    faddp st(1), st(0) ; 0113
    fadd qword ptr [rdx + 192] ; 0115
    fld st(0) ; 0118
    fabs ; 011A
    fchs ; 011C
    faddp st(1), st(0) ; 011E
    fst qword ptr [rcx + 24] ; 0120
    fld st(0) ; 0124
    fmul qword ptr [rdx + 176] ; 0126
    fchs ; 0129
    fadd qword ptr [rcx] ; 012B
    fabs ; 012E
    fstp qword ptr [rcx] ; 0130
    fmul qword ptr [rdx + 184] ; 0133
    fchs ; 0136
    fadd qword ptr [rcx + 8] ; 0138
    fabs ; 013C
    fstp qword ptr [rcx + 8] ; 013E
    fld qword ptr [rcx] ; 0142
    fmul qword ptr [rdx + 176] ; 0145
    fld qword ptr [rcx + 8] ; 0148
    fmul qword ptr [rdx + 184] ; 014C
    faddp st(1), st(0) ; 014F
    fadd qword ptr [rdx + 192] ; 0151
    fld st(0) ; 0154
    fabs ; 0156
    fchs ; 0158
    faddp st(1), st(0) ; 015A
    fst qword ptr [rcx + 24] ; 015C
    fld st(0) ; 0160
    fmul qword ptr [rdx + 176] ; 0162
    fchs ; 0165
    fadd qword ptr [rcx] ; 0167
    nop ; 016A
    nop ; 016B
    fstp qword ptr [rcx] ; 016C
    fmul qword ptr [rdx + 184] ; 016F
    fchs ; 0172
    fadd qword ptr [rcx + 8] ; 0174
    fabs ; 0178
    fstp qword ptr [rcx + 8] ; 017A
    fld qword ptr [rcx] ; 017E
    fmul qword ptr [rdx + 128] ; 0181
    fld qword ptr [rcx + 8] ; 0184
    fmul qword ptr [rdx + 136] ; 0188
    faddp st(1), st(0) ; 018B
    fadd qword ptr [rdx + 144] ; 018D
    fabs ; 0190
    fld qword ptr [rcx + 16] ; 0192
    fabs ; 0196
    fsub qword ptr [rdx + 152] ; 0198
    fcom st(1) ; 019E
    wait ; 01A0
    fnstsw ax ; 01A1
    shr ah, 1 ; 01A3
    jbe hexgridsolid_01ab ; 01A5
    fstp st(1) ; 01A7
    jmp hexgridsolid_01ad ; 01A9
hexgridsolid_01ab:
    fstp st(0) ; 01AB
hexgridsolid_01ad:
    nop ; 01AD
    nop ; 01AE
    nop ; 01AF
    nop ; 01B0
    nop ; 01B1
    nop ; 01B2
    nop ; 01B3
    nop ; 01B4
    nop ; 01B5
    nop ; 01B6
    nop ; 01B7
    nop ; 01B8
    nop ; 01B9
    nop ; 01BA
    nop ; 01BB
    nop ; 01BC
    nop ; 01BD
    nop ; 01BE
    nop ; 01BF
    nop ; 01C0
    nop ; 01C1
    nop ; 01C2
    nop ; 01C3
    nop ; 01C4
    nop ; 01C5
    nop ; 01C6
    nop ; 01C7
    fsub qword ptr [rdx + 0] ; 01C8
    test r8d, r8d
    jne hexgridsolid_01d4 ; 01CF
    fdiv qword ptr [rdx + 8] ; 01D1
hexgridsolid_01d4:
    fstp qword ptr [rcx + 32] ; 01D4
HexGridSolidDistance_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
HexGridSolidDistance_x87 ENDP


; Original radius power subroutine retains its caller x87 stack; XYZ/DE/VaryScale and signed first counter.
ABoxVaryScale2Original_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    cmp dword ptr [r8], 0 ; 0014
    jg aboxvaryscale2_0023 ; 0018
    inc dword ptr [r8] ; 001A
    fld qword ptr [rdx + 0] ; 001D
    fstp qword ptr [rcx + 32] ; 0020
aboxvaryscale2_0023:
    fld qword ptr [rcx + 32] ; 0023
    fld1 ; 0026
    fld st(1) ; 0028
    fabs ; 002A
    fsubrp st(1), st(0) ; 002C
    fmul qword ptr [rdx + 32] ; 002E
    faddp st(1), st(0) ; 0031
    fstp qword ptr [rcx + 32] ; 0033
    fld qword ptr [rdx + 24] ; 0036
    fld st(0) ; 0039
    fld st(0) ; 003B
    fadd st(2), st(0) ; 003D
    fchs ; 003F
    fld qword ptr [rcx] ; 0041
    fcom st(2) ; 0043
    fnstsw ax ; 0045
    shr ah, 1 ; 0047
    jb aboxvaryscale2_0051 ; 0049
    fchs ; 004B
    fadd st(0), st(3) ; 004D
    jmp aboxvaryscale2_005d ; 004F
aboxvaryscale2_0051:
    fcom st(1) ; 0051
    fnstsw ax ; 0053
    shr ah, 1 ; 0055
    jae aboxvaryscale2_005d ; 0057
    fchs ; 0059
    fsub st(0), st(3) ; 005B
aboxvaryscale2_005d:
    fld qword ptr [rcx + 8] ; 005D
    fcom st(3) ; 005F
    fnstsw ax ; 0061
    shr ah, 1 ; 0063
    jb aboxvaryscale2_006d ; 0065
    fchs ; 0067
    fadd st(0), st(4) ; 0069
    jmp aboxvaryscale2_0079 ; 006B
aboxvaryscale2_006d:
    fcom st(2) ; 006D
    fnstsw ax ; 006F
    shr ah, 1 ; 0071
    jae aboxvaryscale2_0079 ; 0073
    fchs ; 0075
    fsub st(0), st(4) ; 0077
aboxvaryscale2_0079:
    fld qword ptr [rcx + 16] ; 0079
    fcom st(4) ; 007B
    fnstsw ax ; 007D
    shr ah, 1 ; 007F
    jb aboxvaryscale2_0089 ; 0081
    fchs ; 0083
    fadd st(0), st(5) ; 0085
    jmp aboxvaryscale2_0095 ; 0087
aboxvaryscale2_0089:
    fcom st(3) ; 0089
    fnstsw ax ; 008B
    shr ah, 1 ; 008D
    jae aboxvaryscale2_0095 ; 008F
    fchs ; 0091
    fsub st(0), st(5) ; 0093
aboxvaryscale2_0095:
    fxch st(3) ; 0095
    fstp st(0) ; 0097
    fxch st(3) ; 0099
    fstp st(0) ; 009B
    fxch st(3) ; 009D
    fstp st(0) ; 009F
    fld st(0) ; 00A1
    fld qword ptr [rdx + 48] ; 00A3
    call aboxvaryscale2_0117 ; 00A6
    fld st(2) ; 00AB
    fld qword ptr [rdx + 48] ; 00AD
    call aboxvaryscale2_0117 ; 00B0
    faddp st(1), st(0) ; 00B5
    fld st(3) ; 00B7
    fld qword ptr [rdx + 48] ; 00B9
    call aboxvaryscale2_0117 ; 00BC
    faddp st(1), st(0) ; 00C1
    fld qword ptr [rdx + 40] ; 00C3
    call aboxvaryscale2_0117 ; 00C6
    fcom qword ptr [rdx + 16] ; 00CB
    fnstsw ax ; 00CE
    shr ah, 1 ; 00D0
    jae aboxvaryscale2_00db ; 00D2
    fstp st(0) ; 00D4
    fld qword ptr [rdx + 8] ; 00D6
    jmp aboxvaryscale2_00f1 ; 00D9
aboxvaryscale2_00db:
    fld1 ; 00DB
    fcom st(1) ; 00DD
    fnstsw ax ; 00DF
    shr ah, 1 ; 00E1
    jb aboxvaryscale2_00ec ; 00E3
    fstp st(0) ; 00E5
    fdivr qword ptr [rcx + 32] ; 00E7
    jmp aboxvaryscale2_00f1 ; 00EA
aboxvaryscale2_00ec:
    fcompp ; 00EC
    fld qword ptr [rcx + 32] ; 00EE
aboxvaryscale2_00f1:
    fld qword ptr [rcx + 24] ; 00F1
    fmul st(0), st(1) ; 00F4
    fstp qword ptr [rcx + 24] ; 00F6
    fmul st(3), st(0) ; 00F9
    fmul st(2), st(0) ; 00FB
    fmulp st(1), st(0) ; 00FD
    fadd qword ptr [rdx + 72] ; 00FF
    fstp qword ptr [rcx + 16] ; 0102
    fadd qword ptr [rdx + 64] ; 0104
    fstp qword ptr [rcx + 8] ; 0107
    fadd qword ptr [rdx + 56] ; 0109
    fstp qword ptr [rcx] ; 010C
    jmp ABoxVaryScale2Original_x87_done
aboxvaryscale2_0117:
    fxch st(1) ; 0117
    fabs ; 0119
    fldln2 ; 011B
    fxch st(1) ; 011D
    fyl2x ; 011F
    fxch st(1) ; 0121
    fmulp st(1), st(0) ; 0123
    fldl2e ; 0125
    fmulp st(1), st(0) ; 0127
    fld st(0) ; 0129
    frndint ; 012B
    fsub st(1), st(0) ; 012D
    fxch st(1) ; 012F
    f2xm1 ; 0131
    fld1 ; 0133
    faddp st(1), st(0) ; 0135
    fscale ; 0137
    fstp st(1) ; 0139
    ret ; 013B
ABoxVaryScale2Original_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
ABoxVaryScale2Original_x87 ENDP


; Original iteration-dependent formula consumes supplied Rout and adds C once.
FuzzyOriginal_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    fld1 ; 000B
    fiadd dword ptr [r8] ; 000D
    fmul qword ptr [rdx + 0] ; 0010
    fld qword ptr [rcx + 24] ; 0013
    fld st(0) ; 0016
    fsqrt ; 0018
    fdivr qword ptr [rcx + 16] ; 001A
    fld1 ; 001C
    fld st(1) ; 001E
    fmul st(0), st(0) ; 0020
    fsubp st(1), st(0) ; 0022
    fsqrt ; 0024
    fpatan ; 0026
    fmul st(0), st(2) ; 0028
    fld qword ptr [rcx + 8] ; 002A
    fld qword ptr [rcx] ; 002C
    fpatan ; 002E
    fmulp st(3), st(0) ; 0030
    fsincos ; 0032
    fxch st(3) ; 0034
    fsincos ; 0036
    fxch st(3) ; 0038
    fld1 ; 003A
    fsubr qword ptr [rdx + 0] ; 003C
    fxch st(1) ; 003F
    fldln2 ; 0041
    fxch st(1) ; 0043
    fyl2x ; 0045
    fxch st(1) ; 0047
    fmulp st(1), st(0) ; 0049
    fldl2e ; 004B
    fmulp st(1), st(0) ; 004D
    fld st(0) ; 004F
    frndint ; 0051
    fsub st(1), st(0) ; 0053
    fxch st(1) ; 0055
    f2xm1 ; 0057
    fld1 ; 0059
    faddp st(1), st(0) ; 005B
    fscale ; 005D
    fstp st(1) ; 005F
    fmul st(2), st(0) ; 0061
    fmulp st(4), st(0) ; 0063
    fmul st(0), st(3) ; 0065
    fmul qword ptr [rdx + 8] ; 0067
    fadd qword ptr [rdx + 24] ; 006A
    fstp qword ptr [rcx + 8] ; 006D
    fsubr qword ptr [rdx + 32] ; 006F
    fstp qword ptr [rcx + 16] ; 0072
    fmulp st(1), st(0) ; 0074
    fadd qword ptr [rdx + 16] ; 0076
    fstp qword ptr [rcx] ; 0079
FuzzyOriginal_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
FuzzyOriginal_x87 ENDP


; Original quaternion sine; AND clears CF, so following JB is never taken.
Sin4DplusOriginal_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    fld qword ptr [rcx] ; 0010
    fadd qword ptr [rdx + 0] ; 0012
    fstp qword ptr [rcx] ; 0015
    fld qword ptr [rcx + 8] ; 0017
    fadd qword ptr [rdx + 8] ; 0019
    fstp qword ptr [rcx + 8] ; 001C
    fld qword ptr [rcx + 16] ; 001E
    fadd qword ptr [rdx + 16] ; 0020
    fstp qword ptr [rcx + 16] ; 0023
    fld qword ptr [rcx + 24] ; 0025
    fadd qword ptr [rdx + 24] ; 0028
    fstp qword ptr [rcx + 24] ; 002B
    nop ; 002E
    fld qword ptr [rcx + 24] ; 002F
    fmul st(0), st(0) ; 0032
    fld qword ptr [rcx + 8] ; 0034
    fmul st(0), st(0) ; 0036
    faddp st(1), st(0) ; 0038
    fld qword ptr [rcx + 16] ; 003A
    fmul st(0), st(0) ; 003C
    faddp st(1), st(0) ; 003E
    fsqrt ; 0040
    fld st(0) ; 0042
    fld qword ptr [rdx + 48] ; 0044
    fcompp ; 0047
    fnstsw ax ; 0049
    and ah, 041h ; 004B
    jb sin4dplus_00cf ; 004E
    nop ; 0050
    nop ; 0051
    nop ; 0052
    nop ; 0053
    nop ; 0054
    nop ; 0055
    nop ; 0056
    nop ; 0057
    nop ; 0058
    nop ; 0059
    nop ; 005A
    fld qword ptr [rcx] ; 005B
    fsincos ; 005D
    fnop ; 005F
    fld st(2) ; 0061
    fldl2e ; 0063
    fmulp st(1), st(0) ; 0065
    fld1 ; 0067
    fld st(1) ; 0069
    fprem ; 006B
    f2xm1 ; 006D
    faddp st(1), st(0) ; 006F
    fscale ; 0071
    fxch st(1) ; 0073
    fstp st(0) ; 0075
    nop ; 0077
    fld st(0) ; 0078
    fld1 ; 007A
    fld st(1) ; 007C
    fdivp st(1), st(0) ; 007E
    fchs ; 0080
    faddp st(1), st(0) ; 0082
    fmul qword ptr [rdx + 40] ; 0084
    nop ; 0086
    nop ; 0087
    nop ; 0088
    fld st(1) ; 0089
    fld1 ; 008B
    fld st(1) ; 008D
    fdivp st(1), st(0) ; 008F
    fnop ; 0091
    faddp st(1), st(0) ; 0093
    fmul qword ptr [rdx + 40] ; 0095
    nop ; 0097
    nop ; 0098
    nop ; 0099
    fstp st(2) ; 009A
    fld st(3) ; 009C
    fmul st(0), st(2) ; 009E
    fstp qword ptr [rcx] ; 00A0
    fld st(2) ; 00A2
    fmul st(0), st(1) ; 00A4
    fdiv st(0), st(5) ; 00A6
    fstp st(1) ; 00A8
    fstp st(1) ; 00AA
    fstp st(1) ; 00AC
    fstp st(1) ; 00AE
    fstp st(1) ; 00B0
    nop ; 00B2
    nop ; 00B3
    fld qword ptr [rcx + 8] ; 00B4
    fmul st(0), st(1) ; 00B6
    fstp qword ptr [rcx + 8] ; 00B8
    fld qword ptr [rcx + 16] ; 00BA
    fmul st(0), st(1) ; 00BC
    fstp qword ptr [rcx + 16] ; 00BE
    fld qword ptr [rcx + 24] ; 00C0
    fmul st(0), st(1) ; 00C3
    fstp qword ptr [rcx + 24] ; 00C5
    fstp st(0) ; 00C8
    fstp st(0) ; 00CA
    nop ; 00CC
    jmp sin4dplus_00d8 ; 00CD
sin4dplus_00cf:
    fld qword ptr [rcx] ; 00CF
    fsin ; 00D1
    fstp qword ptr [rcx] ; 00D3
    fstp st(0) ; 00D5
    nop ; 00D7
sin4dplus_00d8:
    fld qword ptr [rcx] ; 00D8
    fmul qword ptr [rdx + 32] ; 00DA
    fstp qword ptr [rcx] ; 00DD
    fld qword ptr [rcx + 8] ; 00DF
    fmul qword ptr [rdx + 32] ; 00E1
    fstp qword ptr [rcx + 8] ; 00E4
    fld qword ptr [rcx + 16] ; 00E6
    fmul qword ptr [rdx + 32] ; 00E8
    fstp qword ptr [rcx + 16] ; 00EB
    fld qword ptr [rcx + 24] ; 00ED
    fmul qword ptr [rdx + 32] ; 00F0
    fstp qword ptr [rcx + 24] ; 00F3
Sin4DplusOriginal_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
Sin4DplusOriginal_x87 ENDP


; Original two rotations, asymmetric sort predicates and intermediate Double stores.
OctKochOriginal_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    mov eax, 7fffffffh ; 000D
    and dword ptr [rcx + 20], eax ; 0012
    and dword ptr [rcx + 12], eax ; 0015
    and dword ptr [rcx + 4], eax ; 0018
    fld qword ptr [rcx + 16] ; 001B
    fld qword ptr [rcx + 8] ; 001D
    fld qword ptr [rcx] ; 001F
    fld st(0) ; 0021
    fmul qword ptr [rdx + 80] ; 0023
    fld st(2) ; 0026
    fmul qword ptr [rdx + 88] ; 0028
    faddp st(1), st(0) ; 002B
    fld st(3) ; 002D
    fmul qword ptr [rdx + 96] ; 002F
    faddp st(1), st(0) ; 0032
    fld st(1) ; 0034
    fmul qword ptr [rdx + 32] ; 0036
    fld st(3) ; 0039
    fmul qword ptr [rdx + 40] ; 003B
    faddp st(1), st(0) ; 003E
    fld st(4) ; 0040
    fmul qword ptr [rdx + 48] ; 0042
    faddp st(1), st(0) ; 0045
    fxch st(2) ; 0047
    fmul qword ptr [rdx + 56] ; 0049
    fxch st(3) ; 004C
    fmul qword ptr [rdx + 64] ; 004E
    faddp st(3), st(0) ; 0051
    fxch st(3) ; 0053
    fmul qword ptr [rdx + 72] ; 0055
    faddp st(2), st(0) ; 0058
    fcom st(1) ; 005A
    fnstsw ax ; 005C
    and ah, 041h ; 005E
    je octkoch_0065 ; 0061
    fxch st(1) ; 0063
octkoch_0065:
    fcom st(2) ; 0065
    fnstsw ax ; 0067
    and ah, 041h ; 0069
    je octkoch_0070 ; 006C
    fxch st(2) ; 006E
octkoch_0070:
    fxch st(1) ; 0070
    fcom st(2) ; 0072
    fnstsw ax ; 0074
    and ah, 041h ; 0076
    je octkoch_007d ; 0079
    fxch st(2) ; 007B
octkoch_007d:
    fstp qword ptr [rcx + 8] ; 007D
    fstp qword ptr [rcx] ; 007F
    fstp qword ptr [rcx + 16] ; 0081
    fld1 ; 0083
    fsubr qword ptr [rdx + 0] ; 0085
    fmul qword ptr [rdx + 184] ; 0088
    fld qword ptr [rcx] ; 008B
    fsubrp st(1), st(0) ; 008D
    fstp qword ptr [rcx] ; 008F
    fld1 ; 0091
    fsubr qword ptr [rdx + 0] ; 0093
    fmul qword ptr [rdx + 176] ; 0096
    fld qword ptr [rcx] ; 0099
    fadd st(0), st(1) ; 009B
    fabs ; 009D
    fld qword ptr [rcx] ; 009F
    fsub st(0), st(2) ; 00A1
    fabs ; 00A3
    fchs ; 00A5
    fsub qword ptr [rcx] ; 00A7
    faddp st(1), st(0) ; 00A9
    test r8d, r8d
    je octkoch_00b6 ; 00B2
    fabs ; 00B4
octkoch_00b6:
    fstp qword ptr [rcx] ; 00B6
    fstp st(0) ; 00B8
    fld qword ptr [rcx + 16] ; 00BA
    fld qword ptr [rcx] ; 00BC
    fld qword ptr [rcx + 8] ; 00BE
    fcom st(1) ; 00C0
    fnstsw ax ; 00C2
    shr ah, 1 ; 00C4
    jae octkoch_00ca ; 00C6
    fxch st(1) ; 00C8
octkoch_00ca:
    fcom st(2) ; 00CA
    fnstsw ax ; 00CC
    shr ah, 1 ; 00CE
    jae octkoch_00d4 ; 00D0
    fxch st(2) ; 00D2
octkoch_00d4:
    fxch st(1) ; 00D4
    fcom st(2) ; 00D6
    fnstsw ax ; 00D8
    shr ah, 1 ; 00DA
    jae octkoch_00e0 ; 00DC
    fxch st(2) ; 00DE
octkoch_00e0:
    fld st(1) ; 00E0
    fmul qword ptr [rdx + 152] ; 00E2
    fld st(1) ; 00E5
    fmul qword ptr [rdx + 160] ; 00E7
    faddp st(1), st(0) ; 00EA
    fld st(3) ; 00EC
    fmul qword ptr [rdx + 168] ; 00EE
    faddp st(1), st(0) ; 00F1
    fld st(2) ; 00F3
    fmul qword ptr [rdx + 104] ; 00F5
    fld st(2) ; 00F8
    fmul qword ptr [rdx + 112] ; 00FA
    faddp st(1), st(0) ; 00FD
    fld st(4) ; 00FF
    fmul qword ptr [rdx + 120] ; 0101
    faddp st(1), st(0) ; 0104
    fxch st(3) ; 0106
    fmul qword ptr [rdx + 128] ; 0108
    fxch st(2) ; 010B
    fmul qword ptr [rdx + 136] ; 010D
    faddp st(2), st(0) ; 0110
    fxch st(3) ; 0112
    fmul qword ptr [rdx + 144] ; 0114
    faddp st(1), st(0) ; 0117
    fld qword ptr [rdx + 0] ; 0119
    fld1 ; 011C
    fsubp st(1), st(0) ; 011E
    fld st(0) ; 0120
    fld st(0) ; 0122
    fmul qword ptr [rdx + 8] ; 0124
    fsubp st(4), st(0) ; 0127
    fmul qword ptr [rdx + 16] ; 0129
    fsubp st(2), st(0) ; 012C
    fmul qword ptr [rdx + 24] ; 012E
    fsub st(3), st(0) ; 0131
    fstp st(0) ; 0133
    fld qword ptr [rdx + 0] ; 0135
    fld qword ptr [rcx + 24] ; 0138
    fmul st(0), st(1) ; 013B
    fstp qword ptr [rcx + 24] ; 013D
    fmul st(3), st(0) ; 0140
    fmul st(2), st(0) ; 0142
    fmul st(1), st(0) ; 0144
    fstp st(0) ; 0146
    fstp qword ptr [rcx + 8] ; 0148
    fstp qword ptr [rcx] ; 014A
    fstp qword ptr [rcx + 16] ; 014C
OctKochOriginal_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
OctKochOriginal_x87 ENDP


; Original signed-zero z test, unclamped logarithm and max comparisons; local sin/cos Double spills.
Loxodrome2Distance_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    fld qword ptr [rcx + 16] ; 004B
    fabs ; 004F
    fld qword ptr [rdx + 136] ; 0051
    fcomp st(1) ; 0054
    wait ; 0056
    fnstsw ax ; 0057
    shr ah, 1 ; 0059
    jae loxodrome2_0067 ; 005B
    fstp st(0) ; 005D
    fld qword ptr [rdx + 128] ; 005F
    jmp loxodrome2_00eb ; 0062
loxodrome2_0067:
    cmp r8d, 0
    jge loxodrome2_0072 ; 006B
    fmul qword ptr [rdx + 152] ; 006D
    fsin ; 0070
loxodrome2_0072:
    cmp r8d, 1
    je loxodrome2_0084 ; 0076
    cmp r8d, -1
    je loxodrome2_0084 ; 007C
    fchs ; 007E
    fld1 ; 0080
    faddp st(1), st(0) ; 0082
loxodrome2_0084:
    fld qword ptr [rdx + 40] ; 0084
    fxch st(1) ; 0087
    fyl2x ; 0089
    test dword ptr [rcx + 20], 80000000h
    je loxodrome2_0099 ; 0095
    fchs ; 0097
loxodrome2_0099:
    fadd qword ptr [rdx + 64] ; 0099
    fsincos ; 009C
    fstp qword ptr [rcx + 32] ; 009E
    fstp qword ptr [rcx + 24] ; 00A2
    fld qword ptr [rcx] ; 00A6
    fmul qword ptr [rcx + 24] ; 00A9
    fld qword ptr [rcx + 8] ; 00AD
    fmul qword ptr [rcx + 32] ; 00B1
    fchs ; 00B5
    faddp st(1), st(0) ; 00B7
    fld qword ptr [rcx] ; 00B9
    fmul qword ptr [rcx + 32] ; 00BC
    fld qword ptr [rcx + 8] ; 00C0
    fmul qword ptr [rcx + 24] ; 00C4
    faddp st(1), st(0) ; 00C8
    fpatan ; 00CA
    fmul qword ptr [rdx + 104] ; 00CC
    fdiv qword ptr [rdx + 56] ; 00CE
    fld st(0) ; 00D1
    frndint ; 00D3
    fsubp st(1), st(0) ; 00D5
    fmul qword ptr [rdx + 56] ; 00D7
    fabs ; 00DA
    fdiv qword ptr [rdx + 0] ; 00DC
    fld1 ; 00DF
    fsubp st(1), st(0) ; 00E1
    fmul qword ptr [rdx + 0] ; 00E3
    fmul qword ptr [rdx + 112] ; 00E6
    fchs ; 00E9
loxodrome2_00eb:
    fld qword ptr [rcx] ; 00EB
    fmul st(0), st(0) ; 00EE
    fld qword ptr [rcx + 8] ; 00F0
    fmul st(0), st(0) ; 00F4
    faddp st(1), st(0) ; 00F6
    fld qword ptr [rcx + 16] ; 00F8
    fmul st(0), st(0) ; 00FC
    fmul qword ptr [rdx + 80] ; 00FE
    faddp st(1), st(0) ; 0101
    fsub qword ptr [rdx + 88] ; 0103
    fabs ; 0106
    fsqrt ; 0108
    fsub qword ptr [rdx + 72] ; 010A
    fmul qword ptr [rdx + 120] ; 010D
    fcom st(1) ; 0110
    wait ; 0112
    fnstsw ax ; 0113
    shr ah, 1 ; 0115
    jae loxodrome2_011d ; 0117
    fstp st(0) ; 0119
    jmp loxodrome2_011f ; 011B
loxodrome2_011d:
    fstp st(1) ; 011D
loxodrome2_011f:
    fld qword ptr [rcx + 16] ; 011F
    fabs ; 0123
    fsub qword ptr [rdx + 144] ; 0125
    fcom st(1) ; 0128
    wait ; 012A
    fnstsw ax ; 012B
    shr ah, 1 ; 012D
    jae loxodrome2_0135 ; 012F
    fstp st(0) ; 0131
    jmp loxodrome2_0137 ; 0133
loxodrome2_0135:
    fstp st(1) ; 0135
loxodrome2_0137:
    test r9d, r9d
    jne loxodrome2_0140 ; 013B
    fdiv qword ptr [rdx + 8] ; 013D
loxodrome2_0140:
    fstp qword ptr [rcx + 40] ; 0140
Loxodrome2Distance_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
Loxodrome2Distance_x87 ENDP


; Independent FSIN/FCOS and direct qword stores, matching original CustomFormulas cases 3/4.
FormulaAngleDouble_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    fld qword ptr [r8]
    fsin
    fstp qword ptr [rcx]
    fld qword ptr [r8]
    fcos
    fstp qword ptr [rdx]
    fldcw word ptr [rsp]
    add rsp, 8
    ret
FormulaAngleDouble_x87 ENDP


; Independent FSIN/FCOS and direct dword stores, matching original CustomFormulas cases 3/4.
FormulaAngleSingle_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    fld qword ptr [r8]
    fsin
    fstp dword ptr [rcx]
    fld qword ptr [r8]
    fcos
    fstp dword ptr [rdx]
    fldcw word ptr [rsp]
    add rsp, 8
    ret
FormulaAngleSingle_x87 ENDP


; Original angular tooth, shell, unordered max and optional scale divisor.
SliceDistance_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    fld qword ptr [rcx + 8] ; 004B
    fld qword ptr [rcx] ; 004F
    fpatan ; 0052
    fmul qword ptr [rdx + 72] ; 0054
    fdiv qword ptr [rdx + 56] ; 0056
    fld st(0) ; 0059
    frndint ; 005B
    fsubp st(1), st(0) ; 005D
    fmul qword ptr [rdx + 56] ; 005F
    fabs ; 0062
    fdiv qword ptr [rdx + 0] ; 0064
    fld1 ; 0067
    fsubp st(1), st(0) ; 0069
    fmul qword ptr [rdx + 0] ; 006B
    fmul qword ptr [rdx + 80] ; 006E
    fchs ; 0071
    fld qword ptr [rcx] ; 0073
    fmul st(0), st(0) ; 0076
    fld qword ptr [rcx + 8] ; 0078
    fmul st(0), st(0) ; 007C
    faddp st(1), st(0) ; 007E
    fld qword ptr [rcx + 16] ; 0080
    fmul st(0), st(0) ; 0084
    faddp st(1), st(0) ; 0086
    fsqrt ; 0088
    fsub qword ptr [rdx + 40] ; 008A
    fadd qword ptr [rdx + 64] ; 008D
    fabs ; 0090
    fsub qword ptr [rdx + 64] ; 0092
    fcom st(1) ; 0095
    wait ; 0097
    fnstsw ax ; 0098
    shr ah, 1 ; 009A
    jae sliceifs_00a2 ; 009C
    fstp st(0) ; 009E
    jmp sliceifs_00a4 ; 00A0
sliceifs_00a2:
    fstp st(1) ; 00A2
sliceifs_00a4:
    test r8d, r8d
    jne sliceifs_00ad ; 00A8
    fdiv qword ptr [rdx + 8] ; 00AA
sliceifs_00ad:
    fstp qword ptr [rcx + 24] ; 00AD
SliceDistance_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
SliceDistance_x87 ENDP


; Original local axis swap and HalfCut branch skipping both abs and box subtraction.
QuadricsDistance_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    cmp dword ptr [r8 + 8], 0
    je quadricsifs_005f ; 004F
    fld qword ptr [rcx] ; 0051
    fld qword ptr [rcx + 16] ; 0054
    fstp qword ptr [rcx] ; 0058
    fstp qword ptr [rcx + 16] ; 005B
quadricsifs_005f:
    fld qword ptr [rcx + 16] ; 005F
    fmul st(0), st(0) ; 0063
    fld qword ptr [rcx + 8] ; 0065
    fmul st(0), st(0) ; 0069
    fld qword ptr [rcx] ; 006B
    fmul st(0), st(0) ; 006E
    fmul qword ptr [rdx + 56] ; 0070
    faddp st(1), st(0) ; 0073
    faddp st(1), st(0) ; 0075
    fsub qword ptr [rdx + 64] ; 0077
    fabs ; 007A
    fsqrt ; 007C
    fsub qword ptr [rdx + 40] ; 007E
    fld qword ptr [rcx] ; 0081
    cmp dword ptr [r8 + 4], 0
    jne quadricsifs_008f ; 0088
    fabs ; 008A
    fsub qword ptr [rdx + 0] ; 008C
quadricsifs_008f:
    fld qword ptr [rcx + 8] ; 008F
    fabs ; 0093
    fsub qword ptr [rdx + 0] ; 0095
    fcom st(1) ; 0098
    wait ; 009A
    fnstsw ax ; 009B
    shr ah, 1 ; 009D
    jae quadricsifs_00a5 ; 009F
    fstp st(0) ; 00A1
    jmp quadricsifs_00a7 ; 00A3
quadricsifs_00a5:
    fstp st(1) ; 00A5
quadricsifs_00a7:
    fld qword ptr [rcx + 16] ; 00A7
    fabs ; 00AB
    fsub qword ptr [rdx + 0] ; 00AD
    fcom st(1) ; 00B0
    wait ; 00B2
    fnstsw ax ; 00B3
    shr ah, 1 ; 00B5
    jae quadricsifs_00bd ; 00B7
    fstp st(0) ; 00B9
    jmp quadricsifs_00bf ; 00BB
quadricsifs_00bd:
    fstp st(1) ; 00BD
quadricsifs_00bf:
    fcom st(1) ; 00BF
    wait ; 00C1
    fnstsw ax ; 00C2
    shr ah, 1 ; 00C4
    jae quadricsifs_00cc ; 00C6
    fstp st(0) ; 00C8
    jmp quadricsifs_00ce ; 00CA
quadricsifs_00cc:
    fstp st(1) ; 00CC
quadricsifs_00ce:
    cmp dword ptr [r8], 0
    jne quadricsifs_00d7 ; 00D2
    fdiv qword ptr [rdx + 8] ; 00D4
quadricsifs_00d7:
    fstp qword ptr [rcx + 24] ; 00D7
QuadricsDistance_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
QuadricsDistance_x87 ENDP


; Original three Dini passes, radial cuts and local coordinate restoration.
DiniDistance_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    fld qword ptr [rdx + 112] ; 004B
    fstp qword ptr [rcx + 40] ; 004E
    cmp dword ptr [r8 + 4], 1
    jne diniifs_0066 ; 0056
    fld qword ptr [rcx] ; 0058
    fld qword ptr [rcx + 16] ; 005B
    fstp qword ptr [rcx] ; 005F
    fstp qword ptr [rcx + 16] ; 0062
diniifs_0066:
    cmp dword ptr [r8 + 4], -1
    jne diniifs_007c ; 006A
    fld qword ptr [rcx + 8] ; 006C
    fld qword ptr [rcx + 16] ; 0070
    fstp qword ptr [rcx + 8] ; 0074
    fstp qword ptr [rcx + 16] ; 0078
diniifs_007c:
    cmp dword ptr [r8 + 8], 0
    je diniifs_008c ; 0080
    fld qword ptr [rcx + 16] ; 0082
    fchs ; 0086
    fstp qword ptr [rcx + 16] ; 0088
diniifs_008c:
    mov r10d, 3
diniifs_0091:
    fld qword ptr [rcx + 8] ; 0091
    fchs ; 0095
    fstp qword ptr [rcx + 32] ; 0097
    fld qword ptr [rcx] ; 009B
    fchs ; 009E
    fstp qword ptr [rcx + 48] ; 00A0
    fld qword ptr [rcx + 8] ; 00A4
    fsub qword ptr [rcx] ; 00A8
    fld qword ptr [rcx + 8] ; 00AB
    fadd qword ptr [rcx] ; 00AF
    fpatan ; 00B2
    fmul qword ptr [rdx + 88] ; 00B4
    fstp qword ptr [rcx + 24] ; 00B7
    fld qword ptr [rcx] ; 00BB
    fmul st(0), st(0) ; 00BE
    fld qword ptr [rcx + 8] ; 00C0
    fmul st(0), st(0) ; 00C4
    faddp st(1), st(0) ; 00C6
    fst qword ptr [rcx + 8] ; 00C8
    fsqrt ; 00CC
    fstp qword ptr [rcx] ; 00CE
    fld qword ptr [rdx + 0] ; 00D1
    fmul st(0), st(0) ; 00D4
    fsub qword ptr [rcx + 8] ; 00D6
    fld st(0) ; 00DA
    fabs ; 00DC
    faddp st(1), st(0) ; 00DE
    fsqrt ; 00E0
    fld st(0) ; 00E2
    fadd qword ptr [rdx + 0] ; 00E4
    fdiv qword ptr [rcx] ; 00E7
    fldln2 ; 00EA
    fxch st(1) ; 00EC
    fyl2x ; 00EE
    fmul qword ptr [rdx + 0] ; 00F0
    fxch st(1) ; 00F3
    fchs ; 00F5
    faddp st(1), st(0) ; 00F7
    fsub qword ptr [rcx + 24] ; 00F9
    fsub qword ptr [rcx + 16] ; 00FD
    fabs ; 0101
    fsub qword ptr [rdx + 56] ; 0103
    fld qword ptr [rcx] ; 0106
    fmul qword ptr [rdx + 96] ; 0109
    fadd qword ptr [rdx + 104] ; 010B
    fabs ; 010E
    fmulp st(1), st(0) ; 0110
    fld qword ptr [rcx] ; 0112
    fsub qword ptr [rdx + 40] ; 0115
    fcom st(1) ; 0118
    wait ; 011A
    fnstsw ax ; 011B
    shr ah, 1 ; 011D
    jae diniifs_0125 ; 011F
    fstp st(0) ; 0121
    jmp diniifs_0127 ; 0123
diniifs_0125:
    fstp st(1) ; 0125
diniifs_0127:
    fld qword ptr [rcx] ; 0127
    fsubr qword ptr [rdx + 64] ; 012A
    fcom st(1) ; 012D
    wait ; 012F
    fnstsw ax ; 0130
    shr ah, 1 ; 0132
    jae diniifs_013a ; 0134
    fstp st(0) ; 0136
    jmp diniifs_013c ; 0138
diniifs_013a:
    fstp st(1) ; 013A
diniifs_013c:
    fld qword ptr [rcx + 32] ; 013C
    fcom st(1) ; 0140
    wait ; 0142
    fnstsw ax ; 0143
    shr ah, 1 ; 0145
    jae diniifs_014d ; 0147
    fstp st(0) ; 0149
    jmp diniifs_014f ; 014B
diniifs_014d:
    fstp st(1) ; 014D
diniifs_014f:
    fcom qword ptr [rcx + 40] ; 014F
    wait ; 0153
    fnstsw ax ; 0154
    shr ah, 1 ; 0156
    jae diniifs_0160 ; 0158
    fstp qword ptr [rcx + 40] ; 015A
    jmp diniifs_0162 ; 015E
diniifs_0160:
    fstp st(0) ; 0160
diniifs_0162:
    fld qword ptr [rcx + 48] ; 0162
    fstp qword ptr [rcx] ; 0166
    fld qword ptr [rcx + 32] ; 0169
    fstp qword ptr [rcx + 8] ; 016D
    fld qword ptr [rcx + 16] ; 0171
    fld qword ptr [rdx + 88] ; 0175
    fldpi ; 0178
    fmulp st(1), st(0) ; 017A
    faddp st(1), st(0) ; 017C
    fstp qword ptr [rcx + 16] ; 017E
    dec r10d
    jg diniifs_0091 ; 0183
    fld qword ptr [rcx + 40] ; 0189
    cmp dword ptr [r8], 0
    jne diniifs_0196 ; 0191
    fdiv qword ptr [rdx + 8] ; 0193
diniifs_0196:
    fstp qword ptr [rcx + 56] ; 0196
DiniDistance_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
DiniDistance_x87 ENDP


; Original six Double trig spills, torus power and C-before-Z-multiplier.
AexTorusPowOriginal_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    fnop ; 0014
    fld qword ptr [rcx + 8] ; 0016
    fld qword ptr [rcx] ; 0018
    fpatan ; 001A
    fst qword ptr [rcx + 40] ; 001C
    fmul qword ptr [rdx + 0] ; 001F
    fsincos ; 0022
    fstp qword ptr [rcx + 24] ; 0024
    fstp qword ptr [rcx + 32] ; 0027
    fld qword ptr [rcx + 40] ; 002A
    fsincos ; 002D
    fstp qword ptr [rcx + 40] ; 002F
    fstp qword ptr [rcx + 48] ; 0032
    fld qword ptr [rcx] ; 0035
    fmul st(0), st(0) ; 0037
    fld qword ptr [rcx + 8] ; 0039
    fmul st(0), st(0) ; 003B
    faddp st(1), st(0) ; 003D
    fsqrt ; 003F
    fld qword ptr [rcx + 16] ; 0041
    fpatan ; 0043
    fmul qword ptr [rdx + 0] ; 0045
    fsincos ; 0048
    fadd qword ptr [rdx + 24] ; 004A
    test r8d, r8d
    je aextoruspow_0055 ; 0051
    fxch st(1) ; 0053
aextoruspow_0055:
    fstp qword ptr [rcx + 64] ; 0055
    fstp qword ptr [rcx + 56] ; 0058
    fld qword ptr [rcx] ; 005B
    fld qword ptr [rcx + 40] ; 005D
    fmul qword ptr [rdx + 8] ; 0060
    fchs ; 0063
    faddp st(1), st(0) ; 0065
    fmul st(0), st(0) ; 0067
    fld qword ptr [rcx + 8] ; 0069
    fld qword ptr [rcx + 48] ; 006B
    fmul qword ptr [rdx + 8] ; 006E
    fchs ; 0071
    faddp st(1), st(0) ; 0073
    fmul st(0), st(0) ; 0075
    faddp st(1), st(0) ; 0077
    fld qword ptr [rcx + 16] ; 0079
    fmul st(0), st(0) ; 007B
    faddp st(1), st(0) ; 007D
    fst qword ptr [rcx + 8] ; 007F
    fld qword ptr [rdx + 0] ; 0081
    fmul qword ptr [rdx + 40] ; 0084
    fxch st(1) ; 0086
    fabs ; 0088
    fyl2x ; 008A
    fld1 ; 008C
    fld st(1) ; 008E
    fprem ; 0090
    f2xm1 ; 0092
    faddp st(1), st(0) ; 0094
    fscale ; 0096
    fxch st(1) ; 0098
    fstp st(0) ; 009A
    fst qword ptr [rcx] ; 009C
    fst qword ptr [rcx + 16] ; 009E
    fmul qword ptr [rcx + 64] ; 00A0
    fmul qword ptr [rcx + 24] ; 00A3
    fadd qword ptr [rdx + 56] ; 00A6
    nop ; 00A9
    fld qword ptr [rcx + 16] ; 00AA
    fmul qword ptr [rcx + 64] ; 00AC
    fmul qword ptr [rcx + 32] ; 00AF
    fadd qword ptr [rdx + 64] ; 00B2
    nop ; 00B5
    fld qword ptr [rcx] ; 00B6
    fmul qword ptr [rcx + 56] ; 00B8
    fadd qword ptr [rdx + 72] ; 00BB
    fmul qword ptr [rdx + 16] ; 00BE
    nop ; 00C1
    fstp qword ptr [rcx + 16] ; 00C2
    fstp qword ptr [rcx + 8] ; 00C4
    fstp qword ptr [rcx] ; 00C6
    fnop ; 00C8
AexTorusPowOriginal_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
AexTorusPowOriginal_x87 ENDP


; Original six weighted coordinate transforms, dispatch predicates, spills and unchanged W.
CoordVsPowOriginal_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    cmp dword ptr [r8 + 0], 0 ; 0017
    jne coordvspow_0030 ; 001B
    fldz ; 001D
    fst qword ptr [rcx + 24] ; 001F
    fst qword ptr [rcx + 32] ; 0022
    fstp qword ptr [rcx + 40] ; 0025
    jmp coordvspow_0124 ; 0028
    nop ; 002D
    nop ; 002E
    nop ; 002F
coordvspow_0030:
    fld qword ptr [rdx + 8] ; 0030
    fld1 ; 0033
    fcompp ; 0035
    fnstsw ax ; 0037
    and ah, 041h ; 0039
    xor ah, 040h ; 003C
    je coordvspow_0103 ; 003F
    fld qword ptr [rdx + 8] ; 0045
    fldz ; 0048
    fcompp ; 004A
    fnstsw ax ; 004C
    and ah, 041h ; 004E
    xor ah, 040h ; 0051
    je coordvspow_0103 ; 0054
    fnop ; 005A
    fld qword ptr [rcx] ; 005C
    fmul st(0), st(0) ; 005E
    fld qword ptr [rcx + 8] ; 0060
    fmul st(0), st(0) ; 0062
    fld qword ptr [rcx + 16] ; 0064
    fmul st(0), st(0) ; 0066
    faddp st(1), st(0) ; 0068
    faddp st(1), st(0) ; 006A
    fsqrt ; 006C
    fst qword ptr [rcx + 48] ; 006E
    fld qword ptr [rdx + 8] ; 0071
    fxch st(1) ; 0074
    fabs ; 0076
    fyl2x ; 0078
    fld1 ; 007A
    fld st(1) ; 007C
    fprem ; 007E
    f2xm1 ; 0080
    faddp st(1), st(0) ; 0082
    fscale ; 0084
    fxch st(1) ; 0086
    fstp st(0) ; 0088
    fstp qword ptr [rcx + 56] ; 008A
    fld qword ptr [rcx + 8] ; 008D
    fld qword ptr [rcx] ; 008F
    fpatan ; 0091
    fmul qword ptr [rdx + 8] ; 0093
    nop ; 0096
    nop ; 0097
    nop ; 0098
    fld qword ptr [rcx + 16] ; 0099
    fdiv qword ptr [rcx + 48] ; 009B
    fld st(0) ; 009E
    fmul st(0), st(0) ; 00A0
    fchs ; 00A2
    fld1 ; 00A4
    faddp st(1), st(0) ; 00A6
    fsqrt ; 00A8
    fld1 ; 00AA
    faddp st(1), st(0) ; 00AC
    fxch st(1) ; 00AE
    nop ; 00B0
    nop ; 00B1
    fpatan ; 00B2
    fadd st(0), st(0) ; 00B4
    fmul qword ptr [rdx + 8] ; 00B6
    nop ; 00B9
    fsincos ; 00BA
    fstp qword ptr [rcx + 88] ; 00BC
    fstp qword ptr [rcx + 80] ; 00BF
    fsincos ; 00C2
    fstp qword ptr [rcx + 72] ; 00C4
    fstp qword ptr [rcx + 64] ; 00C7
    fld qword ptr [rcx + 56] ; 00CA
    fmul qword ptr [rcx + 72] ; 00CD
    fmul qword ptr [rcx + 88] ; 00D0
    fimul dword ptr [r8 + 0] ; 00D3
    fmul qword ptr [rdx + 56] ; 00D6
    fstp qword ptr [rcx + 24] ; 00D8
    fld qword ptr [rcx + 56] ; 00DB
    fmul qword ptr [rcx + 64] ; 00DE
    fmul qword ptr [rcx + 88] ; 00E1
    fimul dword ptr [r8 + 0] ; 00E4
    fmul qword ptr [rdx + 56] ; 00E7
    fstp qword ptr [rcx + 32] ; 00E9
    fld qword ptr [rcx + 56] ; 00EC
    fmul qword ptr [rcx + 80] ; 00EF
    nop ; 00F2
    nop ; 00F3
    nop ; 00F4
    fimul dword ptr [r8 + 0] ; 00F5
    fmul qword ptr [rdx + 56] ; 00F8
    fstp qword ptr [rcx + 40] ; 00FA
    jmp coordvspow_0121 ; 00FD
    nop ; 00FF
    nop ; 0100
    nop ; 0101
    nop ; 0102
coordvspow_0103:
    fld qword ptr [rcx] ; 0103
    fimul dword ptr [r8 + 0] ; 0105
    fmul qword ptr [rdx + 56] ; 0108
    fstp qword ptr [rcx + 24] ; 010A
    fld qword ptr [rcx + 8] ; 010D
    fimul dword ptr [r8 + 0] ; 010F
    fmul qword ptr [rdx + 56] ; 0112
    fstp qword ptr [rcx + 32] ; 0114
    fld qword ptr [rcx + 16] ; 0117
    fimul dword ptr [r8 + 0] ; 0119
    fmul qword ptr [rdx + 56] ; 011C
    fstp qword ptr [rcx + 40] ; 011E
coordvspow_0121:
    nop ; 0121
    nop ; 0122
    nop ; 0123
coordvspow_0124:
    cmp dword ptr [r8 + 4], 0 ; 0124
    je coordvspow_0156 ; 0128
    nop ; 012A
    nop ; 012B
    nop ; 012C
    nop ; 012D
    fld qword ptr [rcx] ; 012E
    fmul st(0), st(0) ; 0130
    fld qword ptr [rcx + 8] ; 0132
    fmul st(0), st(0) ; 0134
    faddp st(1), st(0) ; 0136
    fsqrt ; 0138
    fld qword ptr [rcx + 8] ; 013A
    fld qword ptr [rcx] ; 013C
    fpatan ; 013E
    fimul dword ptr [r8 + 4] ; 0140
    fmul qword ptr [rdx + 56] ; 0143
    fadd qword ptr [rcx + 32] ; 0145
    fstp qword ptr [rcx + 32] ; 0148
    fimul dword ptr [r8 + 4] ; 014B
    fmul qword ptr [rdx + 56] ; 014E
    fadd qword ptr [rcx + 24] ; 0150
    fstp qword ptr [rcx + 24] ; 0153
coordvspow_0156:
    cmp dword ptr [r8 + 8], 0 ; 0156
    je coordvspow_01a1 ; 015A
    nop ; 015C
    nop ; 015D
    nop ; 015E
    nop ; 015F
    fld qword ptr [rcx] ; 0160
    fmul st(0), st(0) ; 0162
    fld qword ptr [rcx + 8] ; 0164
    fmul st(0), st(0) ; 0166
    faddp st(1), st(0) ; 0168
    fld qword ptr [rcx + 16] ; 016A
    fmul st(0), st(0) ; 016C
    fadd st(0), st(1) ; 016E
    fsqrt ; 0170
    fxch st(1) ; 0172
    fsqrt ; 0174
    fld qword ptr [rcx + 16] ; 0176
    fpatan ; 0178
    fld qword ptr [rcx + 8] ; 017A
    fld qword ptr [rcx] ; 017C
    fpatan ; 017E
    fimul dword ptr [r8 + 8] ; 0180
    fmul qword ptr [rdx + 56] ; 0183
    fadd qword ptr [rcx + 40] ; 0185
    fstp qword ptr [rcx + 40] ; 0188
    fimul dword ptr [r8 + 8] ; 018B
    fmul qword ptr [rdx + 56] ; 018E
    fadd qword ptr [rcx + 32] ; 0190
    fstp qword ptr [rcx + 32] ; 0193
    fimul dword ptr [r8 + 8] ; 0196
    fmul qword ptr [rdx + 56] ; 0199
    fadd qword ptr [rcx + 24] ; 019B
    fstp qword ptr [rcx + 24] ; 019E
coordvspow_01a1:
    nop ; 01A1
    cmp dword ptr [r8 + 12], 0 ; 01A2
    je coordvspow_01d2 ; 01A6
    nop ; 01A8
    nop ; 01A9
    nop ; 01AA
    nop ; 01AB
    fld qword ptr [rcx] ; 01AC
    fld qword ptr [rcx + 8] ; 01AE
    fcos ; 01B0
    fmulp st(1), st(0) ; 01B2
    fld qword ptr [rcx] ; 01B4
    fld qword ptr [rcx + 8] ; 01B6
    fsin ; 01B8
    fmulp st(1), st(0) ; 01BA
    fimul dword ptr [r8 + 12] ; 01BC
    fmul qword ptr [rdx + 56] ; 01BF
    fadd qword ptr [rcx + 32] ; 01C1
    fstp qword ptr [rcx + 32] ; 01C4
    fimul dword ptr [r8 + 12] ; 01C7
    fmul qword ptr [rdx + 56] ; 01CA
    fadd qword ptr [rcx + 24] ; 01CC
    fstp qword ptr [rcx + 24] ; 01CF
coordvspow_01d2:
    nop ; 01D2
    cmp dword ptr [r8 + 16], 0 ; 01D3
    je coordvspow_021a ; 01D7
    nop ; 01D9
    nop ; 01DA
    nop ; 01DB
    nop ; 01DC
    fld qword ptr [rcx + 8] ; 01DD
    fsincos ; 01DF
    fld qword ptr [rcx + 16] ; 01E1
    fsincos ; 01E3
    fxch st(2) ; 01E5
    fmul qword ptr [rcx] ; 01E7
    fimul dword ptr [r8 + 16] ; 01E9
    fmul qword ptr [rdx + 56] ; 01EC
    fadd qword ptr [rcx + 40] ; 01EE
    fstp qword ptr [rcx + 40] ; 01F1
    fxch st(2) ; 01F4
    fmul st(1), st(0) ; 01F6
    fxch st(1) ; 01F8
    fmul qword ptr [rcx] ; 01FA
    fxch st(2) ; 01FC
    fxch st(1) ; 01FE
    fmul qword ptr [rcx] ; 0200
    fmulp st(1), st(0) ; 0202
    fimul dword ptr [r8 + 16] ; 0204
    fmul qword ptr [rdx + 56] ; 0207
    fadd qword ptr [rcx + 32] ; 0209
    fstp qword ptr [rcx + 32] ; 020C
    fimul dword ptr [r8 + 16] ; 020F
    fmul qword ptr [rdx + 56] ; 0212
    fadd qword ptr [rcx + 24] ; 0214
    fstp qword ptr [rcx + 24] ; 0217
coordvspow_021a:
    nop ; 021A
    cmp dword ptr [r8 + 20], 0 ; 021B
    je coordvspow_02fb ; 021F
    fld qword ptr [rcx] ; 0225
    fmul st(0), st(0) ; 0227
    fld qword ptr [rcx + 8] ; 0229
    fmul st(0), st(0) ; 022B
    faddp st(1), st(0) ; 022D
    fsqrt ; 022F
    fld qword ptr [rcx + 16] ; 0231
    fdivrp st(1), st(0) ; 0233
    fst qword ptr [rcx + 48] ; 0235
    nop ; 0238
    fmul qword ptr [rcx + 8] ; 0239
    fchs ; 023B
    fadd qword ptr [rcx] ; 023D
    fldl2e ; 023F
    fmulp st(1), st(0) ; 0241
    fld1 ; 0243
    fld st(1) ; 0245
    fprem ; 0247
    f2xm1 ; 0249
    faddp st(1), st(0) ; 024B
    fscale ; 024D
    fxch st(1) ; 024F
    fstp st(0) ; 0251
    fmul qword ptr [rdx + 64] ; 0253
    fstp qword ptr [rcx + 56] ; 0256
    nop ; 0259
    fld qword ptr [rcx + 48] ; 025A
    fmul qword ptr [rcx] ; 025D
    fadd qword ptr [rcx + 8] ; 025F
    fsincos ; 0261
    fstp qword ptr [rcx + 72] ; 0263
    fstp qword ptr [rcx + 64] ; 0266
    fld qword ptr [rcx + 48] ; 0269
    fmul qword ptr [rcx] ; 026C
    fsub qword ptr [rcx + 8] ; 026E
    fsincos ; 0270
    fstp qword ptr [rcx + 88] ; 0272
    fstp qword ptr [rcx + 80] ; 0275
    nop ; 0278
    fld qword ptr [rcx + 48] ; 0279
    fmul qword ptr [rcx + 8] ; 027C
    fadd st(0), st(0) ; 027E
    fldl2e ; 0280
    fmulp st(1), st(0) ; 0282
    fld1 ; 0284
    fld st(1) ; 0286
    fprem ; 0288
    f2xm1 ; 028A
    faddp st(1), st(0) ; 028C
    fscale ; 028E
    fxch st(1) ; 0290
    fstp st(0) ; 0292
    fstp qword ptr [rcx + 48] ; 0294
    nop ; 0297
    fld qword ptr [rcx + 48] ; 0298
    fmul qword ptr [rcx + 88] ; 029B
    fadd qword ptr [rcx + 72] ; 029E
    fmul qword ptr [rcx + 56] ; 02A1
    fimul dword ptr [r8 + 20] ; 02A4
    fmul qword ptr [rdx + 56] ; 02A7
    fadd qword ptr [rcx + 24] ; 02A9
    fstp qword ptr [rcx + 24] ; 02AC
    nop ; 02AF
    fld qword ptr [rcx + 48] ; 02B0
    fmul qword ptr [rcx + 80] ; 02B3
    fchs ; 02B6
    fadd qword ptr [rcx + 64] ; 02B8
    fmul qword ptr [rcx + 56] ; 02BB
    fimul dword ptr [r8 + 20] ; 02BE
    fmul qword ptr [rdx + 56] ; 02C1
    fadd qword ptr [rcx + 32] ; 02C3
    fstp qword ptr [rcx + 32] ; 02C6
    nop ; 02C9
    fld qword ptr [rcx] ; 02CA
    fmul st(0), st(0) ; 02CC
    fld qword ptr [rcx + 8] ; 02CE
    fmul st(0), st(0) ; 02D0
    faddp st(1), st(0) ; 02D2
    fsqrt ; 02D4
    fldl2e ; 02D6
    fmulp st(1), st(0) ; 02D8
    fld1 ; 02DA
    fld st(1) ; 02DC
    fprem ; 02DE
    f2xm1 ; 02E0
    faddp st(1), st(0) ; 02E2
    fscale ; 02E4
    fxch st(1) ; 02E6
    fstp st(0) ; 02E8
    fld qword ptr [rcx + 16] ; 02EA
    fsin ; 02EC
    fmulp st(1), st(0) ; 02EE
    fimul dword ptr [r8 + 20] ; 02F0
    fmul qword ptr [rdx + 56] ; 02F3
    fadd qword ptr [rcx + 40] ; 02F5
    fstp qword ptr [rcx + 40] ; 02F8
coordvspow_02fb:
    nop ; 02FB
    fld qword ptr [rcx + 40] ; 02FC
    fstp qword ptr [rcx + 16] ; 02FF
    fld qword ptr [rcx + 32] ; 0301
    fstp qword ptr [rcx + 8] ; 0304
    fld qword ptr [rcx + 24] ; 0306
    fstp qword ptr [rcx] ; 0309
CoordVsPowOriginal_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
CoordVsPowOriginal_x87 ENDP


; Original _Folding2 arithmetic, branches and Double stores. rcx: XYZ/Rout; rdx: expanded params/C.
Folding2Original_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    fld qword ptr [rdx] ; 000A
    fld qword ptr [rcx + 16] ; 000D
    fld st(0) ; 000F
    fsub st(0), st(2) ; 0011
    fabs ; 0013
    fxch st(1) ; 0015
    fsub qword ptr [rdx + 8] ; 0017
    fabs ; 001A
    fsubrp st(1), st(0) ; 001C
    fadd qword ptr [rcx + 16] ; 001E
    fsub st(0), st(1) ; 0020
    fstp qword ptr [rcx + 16] ; 0022
    fld qword ptr [rcx + 8] ; 0024
    fld st(0) ; 0026
    fsub st(0), st(2) ; 0028
    fabs ; 002A
    fxch st(1) ; 002C
    fsub qword ptr [rdx + 8] ; 002E
    fabs ; 0031
    fsubrp st(1), st(0) ; 0033
    fadd qword ptr [rcx + 8] ; 0035
    fsub st(0), st(1) ; 0037
    fstp qword ptr [rcx + 8] ; 0039
    fld qword ptr [rcx] ; 003B
    fld st(0) ; 003D
    fsub st(0), st(2) ; 003F
    fabs ; 0041
    fxch st(1) ; 0043
    fsub qword ptr [rdx + 8] ; 0045
    fabs ; 0048
    fsubrp st(1), st(0) ; 004A
    fadd qword ptr [rcx] ; 004C
    fsubrp st(1), st(0) ; 004E
    fstp qword ptr [rcx] ; 0050
Folding2Original_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
Folding2Original_x87 ENDP


; Original _SphereFolding2 arithmetic, branches and Double stores. rcx: XYZ/Rout; rdx: expanded params/C.
SphereFolding2Original_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    fld qword ptr [rcx + 24] ; 000B
    fcom qword ptr [rdx] ; 000E
    fnstsw ax ; 0011
    shr ah, 1 ; 0013
    jae spherefolding2original_x87_0034 ; 0015
    fsqrt ; 0017
    fld st(0) ; 0019
    fsubr qword ptr [rdx + 8] ; 001B
    fdivrp st(1), st(0) ; 001E
    fld qword ptr [rcx + 16] ; 0020
    fld qword ptr [rcx + 8] ; 0022
    fld qword ptr [rcx] ; 0024
    fmul st(0), st(3) ; 0027
    fstp qword ptr [rcx] ; 0029
    fmul st(0), st(2) ; 002C
    fstp qword ptr [rcx + 8] ; 002E
    fmulp st(1), st(0) ; 0030
    fst qword ptr [rcx + 16] ; 0032
spherefolding2original_x87_0034:
    fstp st(0) ; 0034
SphereFolding2Original_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
SphereFolding2Original_x87 ENDP


; Original KaliDucks arithmetic, branches and Double stores. rcx: XYZ/Rout; rdx: expanded params/C.
KaliDucksOriginal_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    fld qword ptr [rcx + 16] ; 000B
    fabs ; 000D
    fld qword ptr [rcx + 8] ; 000F
    fabs ; 0011
    fld qword ptr [rcx] ; 0013
    fabs ; 0015
    fld qword ptr [rcx + 24] ; 0017
    fld qword ptr [rdx + 8] ; 001A
    fcom st(1) ; 001D
    fnstsw ax ; 001F
    and ah, 041h ; 0021
    je kaliducksoriginal_x87_002d ; 0024
    fstp st(0) ; 0026
    fdivr qword ptr [rdx] ; 0028
    jmp kaliducksoriginal_x87_0037 ; 002B
kaliducksoriginal_x87_002d:
    fmul st(0), st(0) ; 002D
    fld1 ; 002F
    fdivrp st(1), st(0) ; 0031
    fxch st(1) ; 0033
    fstp st(0) ; 0035
kaliducksoriginal_x87_0037:
    fmul st(2), st(0) ; 0037
    fmul st(3), st(0) ; 0039
    fmulp st(1), st(0) ; 003B
    fadd qword ptr [rdx + 16] ; 0040
    fstp qword ptr [rcx] ; 0043
    fadd qword ptr [rdx + 24] ; 0045
    fstp qword ptr [rcx + 8] ; 0048
    fadd qword ptr [rdx + 32] ; 004A
    fstp qword ptr [rcx + 16] ; 004D
KaliDucksOriginal_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
KaliDucksOriginal_x87 ENDP


; Original Kalisets1 arithmetic, branches and Double stores. rcx: XYZ/Rout; rdx: expanded params/C.
Kalisets1Original_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    fld qword ptr [rcx + 16] ; 000B
    fabs ; 000D
    fld qword ptr [rcx + 8] ; 000F
    fabs ; 0011
    fld qword ptr [rcx] ; 0013
    fabs ; 0015
    fld st(0) ; 0017
    fmul st(0), st(0) ; 0019
    fld st(2) ; 001B
    fmul st(0), st(0) ; 001D
    faddp st(1), st(0) ; 001F
    fld st(3) ; 0021
    fmul st(0), st(0) ; 0023
    faddp st(1), st(0) ; 0025
    fadd qword ptr [rdx + 8] ; 0027
    fdivr qword ptr [rdx] ; 002A
    fmul st(2), st(0) ; 002D
    fmul st(3), st(0) ; 002F
    fmulp st(1), st(0) ; 0031
    fadd qword ptr [rdx + 16] ; 0033
    fstp qword ptr [rcx] ; 0036
    fadd qword ptr [rdx + 24] ; 0038
    fstp qword ptr [rcx + 8] ; 003B
    fadd qword ptr [rdx + 32] ; 003D
    fstp qword ptr [rcx + 16] ; 0040
Kalisets1Original_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
Kalisets1Original_x87 ENDP


; Original tube folds, rotations, Extended distances and color tie/NaN branches.
dodecatubeIFSOriginal_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    mov r10d, 2
dodecatubeifs_0050:
    fld qword ptr [rcx] ; 0050
    fabs ; 0053
    fst qword ptr [rcx] ; 0055
    fmul qword ptr [rdx + 128] ; 0058
    fld qword ptr [rcx + 8] ; 005A
    fabs ; 005E
    fst qword ptr [rcx + 8] ; 0060
    fmul qword ptr [rdx + 136] ; 0064
    fld qword ptr [rcx + 16] ; 0067
    fabs ; 006B
    fst qword ptr [rcx + 16] ; 006D
    fmul qword ptr [rdx + 144] ; 0071
    faddp st(1), st(0) ; 0074
    faddp st(1), st(0) ; 0076
    fld st(0) ; 0078
    fabs ; 007A
    faddp st(1), st(0) ; 007C
    fchs ; 007E
    fld st(0) ; 0080
    fmul qword ptr [rdx + 128] ; 0082
    fadd qword ptr [rcx] ; 0084
    fstp qword ptr [rcx] ; 0087
    fld st(0) ; 008A
    fmul qword ptr [rdx + 136] ; 008C
    fadd qword ptr [rcx + 8] ; 008F
    fstp qword ptr [rcx + 8] ; 0093
    fld st(0) ; 0097
    fmul qword ptr [rdx + 144] ; 0099
    fadd qword ptr [rcx + 16] ; 009C
    fstp qword ptr [rcx + 16] ; 00A0
    fstp st(0) ; 00A4
    fld qword ptr [rcx] ; 00A6
    fmul qword ptr [rdx + 136] ; 00A9
    fld qword ptr [rcx + 8] ; 00AC
    fmul qword ptr [rdx + 144] ; 00B0
    fchs ; 00B3
    fld qword ptr [rcx + 16] ; 00B5
    fmul qword ptr [rdx + 128] ; 00B9
    fchs ; 00BB
    faddp st(1), st(0) ; 00BD
    faddp st(1), st(0) ; 00BF
    fld st(0) ; 00C1
    fabs ; 00C3
    faddp st(1), st(0) ; 00C5
    fld st(0) ; 00C7
    fmul qword ptr [rdx + 136] ; 00C9
    fchs ; 00CC
    fadd qword ptr [rcx] ; 00CE
    fstp qword ptr [rcx] ; 00D1
    fld st(0) ; 00D4
    fmul qword ptr [rdx + 144] ; 00D6
    fadd qword ptr [rcx + 8] ; 00D9
    fstp qword ptr [rcx + 8] ; 00DD
    fld st(0) ; 00E1
    fmul qword ptr [rdx + 128] ; 00E3
    fadd qword ptr [rcx + 16] ; 00E5
    fstp qword ptr [rcx + 16] ; 00E9
    fstp st(0) ; 00ED
    dec r10d
    jg dodecatubeifs_0050 ; 00F0
    fld qword ptr [rcx] ; 00F6
    fabs ; 00F9
    fld qword ptr [rcx + 8] ; 00FB
    fabs ; 00FF
    fld qword ptr [rcx + 16] ; 0101
    fabs ; 0105
    fstp qword ptr [rcx + 16] ; 0107
    fstp qword ptr [rcx + 8] ; 010B
    fstp qword ptr [rcx] ; 010F
    fld qword ptr [rcx] ; 0112
    fmul qword ptr [rdx + 56] ; 0115
    fld qword ptr [rcx + 16] ; 0118
    fmul qword ptr [rdx + 64] ; 011C
    faddp st(1), st(0) ; 011F
    fld qword ptr [rcx] ; 0121
    fmul qword ptr [rdx + 64] ; 0124
    fld qword ptr [rcx + 16] ; 0127
    fmul qword ptr [rdx + 56] ; 012B
    fchs ; 012E
    faddp st(1), st(0) ; 0130
    fstp qword ptr [rcx] ; 0132
    fstp qword ptr [rcx + 16] ; 0135
    fld qword ptr [rcx] ; 0139
    fmul qword ptr [rdx + 72] ; 013C
    fld qword ptr [rcx + 8] ; 013F
    fmul qword ptr [rdx + 80] ; 0143
    faddp st(1), st(0) ; 0146
    fld qword ptr [rcx] ; 0148
    fmul qword ptr [rdx + 80] ; 014B
    fld qword ptr [rcx + 8] ; 014E
    fmul qword ptr [rdx + 72] ; 0152
    fchs ; 0155
    faddp st(1), st(0) ; 0157
    fstp qword ptr [rcx] ; 0159
    fstp qword ptr [rcx + 8] ; 015C
    fld qword ptr [rcx + 8] ; 0160
    fsub qword ptr [rdx + 8] ; 0164
    fst qword ptr [rcx + 8] ; 0167
    fmul st(0), st(0) ; 016B
    fld qword ptr [rcx + 16] ; 016D
    fsub qword ptr [rdx + 16] ; 0171
    fst qword ptr [rcx + 16] ; 0174
    fmul st(0), st(0) ; 0178
    faddp st(1), st(0) ; 017A
    fsqrt ; 017C
    fsub qword ptr [rdx + 0] ; 017E
    fld qword ptr [rcx] ; 0181
    fabs ; 0184
    fsub qword ptr [rdx + 96] ; 0186
    fmul st(0), st(0) ; 0189
    fld qword ptr [rcx + 8] ; 018B
    fmul st(0), st(0) ; 018F
    faddp st(1), st(0) ; 0191
    fld qword ptr [rcx + 16] ; 0193
    fmul st(0), st(0) ; 0197
    faddp st(1), st(0) ; 0199
    fsqrt ; 019B
    fsub qword ptr [rdx + 104] ; 019D
    fcom st(1) ; 01A0
    wait ; 01A2
    fnstsw ax ; 01A3
    and ah, 041h ; 01A5
    je dodecatubeifs_01b4 ; 01A8
    fstp st(1) ; 01AA
    fld qword ptr [rdx + 120] ; 01AC
    jmp dodecatubeifs_01bc ; 01B2
dodecatubeifs_01b4:
    fstp st(0) ; 01B4
    fld qword ptr [rdx + 112] ; 01B6
dodecatubeifs_01bc:
    fstp qword ptr [rcx + 24] ; 01BC
    cmp dword ptr [r9], 0
    jne dodecatubeifs_01cb ; 01C6
    fdiv qword ptr [rdx + 24] ; 01C8
dodecatubeifs_01cb:
    fstp qword ptr [rcx + 32] ; 01CB
dodecatubeIFSOriginal_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
dodecatubeIFSOriginal_x87 ENDP


; Original tube folds, rotations, Extended distances and color tie/NaN branches.
tetratubeIFSOriginal_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    fld qword ptr [rcx] ; 004B
    fld qword ptr [rcx + 8] ; 004E
    fld qword ptr [rcx + 16] ; 0052
    fchs ; 0056
    fcom st(1) ; 0058
    fnstsw ax ; 005A
    shr ah, 1 ; 005C
    jb tetratubeifs_0062 ; 005E
    fxch st(1) ; 0060
tetratubeifs_0062:
    fcom st(2) ; 0062
    fnstsw ax ; 0064
    shr ah, 1 ; 0066
    jb tetratubeifs_006c ; 0068
    fxch st(2) ; 006A
tetratubeifs_006c:
    fchs ; 006C
    fxch st(1) ; 006E
    fchs ; 0070
    fcom st(2) ; 0072
    fnstsw ax ; 0074
    shr ah, 1 ; 0076
    jb tetratubeifs_007c ; 0078
    fxch st(2) ; 007A
tetratubeifs_007c:
    fchs ; 007C
    fxch st(1) ; 007E
    fcom st(1) ; 0080
    fnstsw ax ; 0082
    shr ah, 1 ; 0084
    jae tetratubeifs_008a ; 0086
    fxch st(1) ; 0088
tetratubeifs_008a:
    fcom st(2) ; 008A
    fnstsw ax ; 008C
    shr ah, 1 ; 008E
    jae tetratubeifs_0094 ; 0090
    fxch st(2) ; 0092
tetratubeifs_0094:
    fxch st(1) ; 0094
    fcom st(2) ; 0096
    fnstsw ax ; 0098
    shr ah, 1 ; 009A
    jae tetratubeifs_00a0 ; 009C
    fxch st(2) ; 009E
tetratubeifs_00a0:
    nop ; 00A0
    fstp qword ptr [rcx + 16] ; 00A1
    fstp qword ptr [rcx + 8] ; 00A5
    fstp qword ptr [rcx] ; 00A9
    fld qword ptr [rcx] ; 00AC
    fmul qword ptr [rdx + 56] ; 00AF
    fld qword ptr [rcx + 16] ; 00B2
    fmul qword ptr [rdx + 64] ; 00B6
    faddp st(1), st(0) ; 00B9
    fld qword ptr [rcx] ; 00BB
    fmul qword ptr [rdx + 64] ; 00BE
    fld qword ptr [rcx + 16] ; 00C1
    fmul qword ptr [rdx + 56] ; 00C5
    fchs ; 00C8
    faddp st(1), st(0) ; 00CA
    fstp qword ptr [rcx] ; 00CC
    fstp qword ptr [rcx + 16] ; 00CF
    fld qword ptr [rcx] ; 00D3
    fmul qword ptr [rdx + 72] ; 00D6
    fld qword ptr [rcx + 8] ; 00D9
    fmul qword ptr [rdx + 80] ; 00DD
    faddp st(1), st(0) ; 00E0
    fld qword ptr [rcx] ; 00E2
    fmul qword ptr [rdx + 80] ; 00E5
    fld qword ptr [rcx + 8] ; 00E8
    fmul qword ptr [rdx + 72] ; 00EC
    fchs ; 00EF
    faddp st(1), st(0) ; 00F1
    fstp qword ptr [rcx] ; 00F3
    fstp qword ptr [rcx + 8] ; 00F6
    fld qword ptr [rcx + 8] ; 00FA
    fsub qword ptr [rdx + 8] ; 00FE
    fst qword ptr [rcx + 8] ; 0101
    fmul st(0), st(0) ; 0105
    fld qword ptr [rcx + 16] ; 0107
    fsub qword ptr [rdx + 16] ; 010B
    fst qword ptr [rcx + 16] ; 010E
    fmul st(0), st(0) ; 0112
    faddp st(1), st(0) ; 0114
    fsqrt ; 0116
    fsub qword ptr [rdx + 0] ; 0118
    fld qword ptr [rcx] ; 011B
    fabs ; 011E
    fsub qword ptr [rdx + 96] ; 0120
    fmul st(0), st(0) ; 0123
    fld qword ptr [rcx + 8] ; 0125
    fmul st(0), st(0) ; 0129
    faddp st(1), st(0) ; 012B
    fld qword ptr [rcx + 16] ; 012D
    fmul st(0), st(0) ; 0131
    faddp st(1), st(0) ; 0133
    fsqrt ; 0135
    fsub qword ptr [rdx + 104] ; 0137
    fcom st(1) ; 013A
    wait ; 013C
    fnstsw ax ; 013D
    and ah, 041h ; 013F
    je tetratubeifs_014e ; 0142
    fstp st(1) ; 0144
    fld qword ptr [rdx + 120] ; 0146
    jmp tetratubeifs_0156 ; 014C
tetratubeifs_014e:
    fstp st(0) ; 014E
    fld qword ptr [rdx + 112] ; 0150
tetratubeifs_0156:
    fstp qword ptr [rcx + 24] ; 0156
    cmp dword ptr [r9], 0
    jne tetratubeifs_0165 ; 0160
    fdiv qword ptr [rdx + 24] ; 0162
tetratubeifs_0165:
    fstp qword ptr [rcx + 32] ; 0165
tetratubeIFSOriginal_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
tetratubeIFSOriginal_x87 ENDP


; Original logarithmic spiral, signed radius/Mobius and all style branches.
logspIFSOriginal_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    fld qword ptr [rcx + 16] ; 004E
    fld qword ptr [rcx + 8] ; 0052
    fld qword ptr [rcx] ; 0056
    fld st(0) ; 0059
    fmul dword ptr [r8 + 24] ; 005B
    fld st(2) ; 005E
    fmul dword ptr [r8 + 28] ; 0060
    faddp st(1), st(0) ; 0063
    fld st(3) ; 0065
    fmul dword ptr [r8 + 32] ; 0067
    faddp st(1), st(0) ; 006A
    fld st(1) ; 006C
    fmul dword ptr [r8 + 0] ; 006E
    fld st(3) ; 0071
    fmul dword ptr [r8 + 4] ; 0073
    faddp st(1), st(0) ; 0076
    fld st(4) ; 0078
    fmul dword ptr [r8 + 8] ; 007A
    faddp st(1), st(0) ; 007D
    fxch st(2) ; 007F
    fmul dword ptr [r8 + 12] ; 0081
    fxch st(3) ; 0084
    fmul dword ptr [r8 + 16] ; 0086
    faddp st(3), st(0) ; 0089
    fxch st(3) ; 008B
    fmul dword ptr [r8 + 20] ; 008D
    faddp st(2), st(0) ; 0090
    fstp qword ptr [rcx] ; 0092
    fstp qword ptr [rcx + 8] ; 0095
    fstp qword ptr [rcx + 16] ; 0099
    mov r10d, dword ptr [rdx + 44]
    and r10d, 80000000h
    je logspifs_0121 ; 00A9
    fld qword ptr [rcx] ; 00AF
    fld1 ; 00B2
    faddp st(1), st(0) ; 00B4
    fmul st(0), st(0) ; 00B6
    fld qword ptr [rcx + 8] ; 00B8
    fmul st(0), st(0) ; 00BC
    faddp st(1), st(0) ; 00BE
    fld1 ; 00C0
    fdivrp st(1), st(0) ; 00C2
    fld st(0) ; 00C4
    fld qword ptr [rcx] ; 00C6
    fmul st(0), st(0) ; 00C9
    fld qword ptr [rcx + 8] ; 00CB
    fmul st(0), st(0) ; 00CF
    faddp st(1), st(0) ; 00D1
    fld1 ; 00D3
    fsubp st(1), st(0) ; 00D5
    fmulp st(1), st(0) ; 00D7
    fld qword ptr [rcx + 8] ; 00D9
    fchs ; 00DD
    fadd st(0), st(0) ; 00DF
    fmulp st(2), st(0) ; 00E1
    fadd qword ptr [rdx + 32] ; 00E3
    fstp qword ptr [rcx + 8] ; 00E6
    fstp qword ptr [rcx] ; 00EA
    fld qword ptr [rcx] ; 00ED
    fmul dword ptr [r8 + 36] ; 00F0
    fld qword ptr [rcx + 8] ; 00F6
    fmul dword ptr [r8 + 40] ; 00FA
    fchs ; 0100
    faddp st(1), st(0) ; 0102
    fld qword ptr [rcx] ; 0104
    fmul dword ptr [r8 + 40] ; 0107
    fld qword ptr [rcx + 8] ; 010D
    fmul dword ptr [r8 + 36] ; 0111
    faddp st(1), st(0) ; 0117
    fstp qword ptr [rcx] ; 0119
    fstp qword ptr [rcx + 8] ; 011C
    nop ; 0120
logspifs_0121:
    fld qword ptr [rcx] ; 0121
    fmul st(0), st(0) ; 0124
    fld qword ptr [rcx + 8] ; 0126
    fmul st(0), st(0) ; 012A
    faddp st(1), st(0) ; 012C
    fsqrt ; 012E
    fld qword ptr [rdx + 0] ; 0130
    cmp dword ptr [r9 + 8], 1
    jne logspifs_013d ; 0137
    fdivrp st(1), st(0) ; 0139
    jmp logspifs_017d ; 013B
logspifs_013d:
    cmp dword ptr [r9 + 8], 2
    jne logspifs_014d ; 0141
    fxch st(1) ; 0143
    fyl2x ; 0145
    fmul st(0), st(0) ; 0147
    fchs ; 0149
    jmp logspifs_017d ; 014B
logspifs_014d:
    cmp dword ptr [r9 + 8], 3
    jne logspifs_0177 ; 0151
    fxch st(1) ; 0153
    fyl2x ; 0155
    fst qword ptr [rcx + 24] ; 0157
    fabs ; 015B
    fadd qword ptr [rdx + 176] ; 015D
    mov r10d, dword ptr [rcx + 28]
    and r10d, 80000000h
    je logspifs_016e ; 016A
    fchs ; 016C
logspifs_016e:
    nop ; 016E
    fld1 ; 016F
    fdivrp st(1), st(0) ; 0171
    jmp logspifs_017d ; 0173
    nop ; 0175
    nop ; 0176
logspifs_0177:
    fxch st(1) ; 0177
    fyl2x ; 0179
    fnop ; 017B
logspifs_017d:
    fld qword ptr [rcx] ; 017D
    fld qword ptr [rcx + 8] ; 0180
    fpatan ; 0184
    fmul qword ptr [rdx + 168] ; 0186
    fimul dword ptr [r9 + 4] ; 0188
    faddp st(1), st(0) ; 018B
    fld st(0) ; 018D
    frndint ; 018F
    fsubrp st(1), st(0) ; 0191
    fabs ; 0193
    fmul qword ptr [rdx + 64] ; 0195
    fld qword ptr [rcx + 16] ; 0198
    fmul st(0), st(0) ; 019C
    faddp st(1), st(0) ; 019E
    fsqrt ; 01A0
    fld qword ptr [rdx + 40] ; 01A2
    fabs ; 01A5
    fsubp st(1), st(0) ; 01A7
    cmp dword ptr [r9], 0
    jne logspifs_01b2 ; 01AD
    fdiv qword ptr [rdx + 8] ; 01AF
logspifs_01b2:
    fstp qword ptr [rcx + 32] ; 01B2
logspIFSOriginal_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
logspIFSOriginal_x87 ENDP


; Original ducksIFS with even Flip XZ only; caller-dependent odd branch excluded.
DucksEvenOriginal_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    fild dword ptr [r9 + 4] ; 0054
    fistp dword ptr [rcx + 24] ; 0057
    nop ; 005B
    fld qword ptr [rcx + 16] ; 005C
    fabs ; 0060
    fld qword ptr [rdx + 40] ; 0062
    fabs ; 0065
    fadd qword ptr [rdx + 128] ; 0067
    fmul qword ptr [rdx + 120] ; 006A
    nop ; 006D
    fadd qword ptr [rcx + 32] ; 006E
    fcompp ; 0074
    fnstsw ax ; 0076
    shr ah, 1 ; 0078
    jae duckseven_0087 ; 007A
    fld qword ptr [rcx + 16] ; 007C
    fchs ; 0080
    jmp duckseven_012b ; 0082
duckseven_0087:
    fldz ; 0087
    fld qword ptr [rcx] ; 0089
    fld qword ptr [rcx + 8] ; 008C
duckseven_0090:
    fld st(0) ; 0090
    fmul st(0), st(0) ; 0092
    fld st(2) ; 0094
    fmul st(0), st(0) ; 0096
    faddp st(1), st(0) ; 0098
    fldln2 ; 009A
    fxch st(1) ; 009C
    fyl2x ; 009E
    fmul qword ptr [rdx + 112] ; 00A0
    fadd qword ptr [rdx + 72] ; 00A2
    fld st(1) ; 00A5
    fabs ; 00A7
    fld st(3) ; 00A9
    fpatan ; 00AB
    fadd qword ptr [rdx + 80] ; 00AD
    fstp st(2) ; 00B0
    fstp st(2) ; 00B2
    fld st(1) ; 00B4
    fmul dword ptr [r8 + 4] ; 00B6
    fld st(1) ; 00B9
    fmul dword ptr [r8] ; 00BB
    fchs ; 00BE
    faddp st(1), st(0) ; 00C0
    fld st(2) ; 00C2
    fmul dword ptr [r8] ; 00C4
    fld st(2) ; 00C7
    fmul dword ptr [r8 + 4] ; 00C9
    faddp st(1), st(0) ; 00CC
    fstp st(2) ; 00CE
    fstp st(2) ; 00D0
    fld st(0) ; 00D2
    fmul st(0), st(0) ; 00D4
    fld st(2) ; 00D6
    fmul st(0), st(0) ; 00D8
    faddp st(1), st(0) ; 00DA
    fsqrt ; 00DC
    fxch st(3) ; 00DE
    fmul qword ptr [rdx + 0] ; 00E0
    fxch st(3) ; 00E3
    fadd st(3), st(0) ; 00E5
    fstp st(0) ; 00E7
    dec dword ptr [rcx + 24] ; 00E9
    jg duckseven_0090 ; 00ED
    fstp st(0) ; 00EF
    fstp st(0) ; 00F1
    fld1 ; 00F3
    faddp st(1), st(0) ; 00F5
    fldln2 ; 00F7
    fxch st(1) ; 00F9
    fyl2x ; 00FB
    fld st(0) ; 00FD
    fld qword ptr [rdx + 104] ; 00FF
    fldz ; 0102
    fcompp ; 0104
    fnstsw ax ; 0106
    shr ah, 1 ; 0108
    jb duckseven_0114 ; 010A
    fmul st(0), st(0) ; 010C
    nop ; 010E
    nop ; 010F
    nop ; 0110
    nop ; 0111
    nop ; 0112
    nop ; 0113
duckseven_0114:
    fmul qword ptr [rdx + 104] ; 0114
    fabs ; 0117
    fstp qword ptr [rcx + 40] ; 0119
    fidiv dword ptr [r9 + 4] ; 011F
    fmul qword ptr [rdx + 40] ; 0124
    fsub qword ptr [rcx + 16] ; 0127
duckseven_012b:
    cmp dword ptr [r9], 0
    jne duckseven_0134 ; 012F
    fdiv qword ptr [rdx + 8] ; 0131
duckseven_0134:
    fstp qword ptr [rcx + 48] ; 0134
DucksEvenOriginal_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
DucksEvenOriginal_x87 ENDP

.const
kIterationHalfMilli BYTE 59,223,79,141,151,110,18,131,244,63
.code
; FPC -CfSSE2 RMdoBinSearchIt: Double multiply, Extended literal addition.
FinalizeIterationBoundaryStep_x87 PROC
    sub rsp, 16
    fnstcw word ptr [rsp]
    mov word ptr [rsp+2], 137fh
    fldcw word ptr [rsp+2]
    movsd xmm0, qword ptr [rdx]
    mulsd xmm0, qword ptr [rdx+8]
    movsd qword ptr [rsp+8], xmm0
    fld qword ptr [rsp+8]
    fld tbyte ptr kIterationHalfMilli
    faddp st(1), st(0)
    fstp qword ptr [rcx]
    fldcw word ptr [rsp]
    add rsp, 16
    ret
FinalizeIterationBoundaryStep_x87 ENDP

.const
kIterationMilli BYTE 59,223,79,141,151,110,18,131,245,63
.code
AccumulateIterationDepth_sse2 PROC
    movsd xmm0, qword ptr [rcx]
    addsd xmm0, xmm1
    movsd qword ptr [rcx], xmm0
    ret
AccumulateIterationDepth_sse2 ENDP

OffsetIterationDepth_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp+2], 137fh
    fldcw word ptr [rsp+2]
    fld qword ptr [rcx]
    fld tbyte ptr kIterationMilli
    fsubp st(1), st(0)
    fstp qword ptr [rcx]
    fldcw word ptr [rsp]
    add rsp, 8
    ret
OffsetIterationDepth_x87 ENDP


; Original varying-scale first state, Extended radius power, folds and writeback.
ABoxVaryScaleOriginal_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    cmp dword ptr [r9], 0 ; 0014
    jg aboxvaryscale_0023 ; 0018
    inc dword ptr [r9] ; 001A
    fld qword ptr [rdx + 40] ; 001D
    fstp qword ptr [rcx + 32] ; 0020
aboxvaryscale_0023:
    fld qword ptr [rcx + 32] ; 0023
    fld1 ; 0026
    fld st(1) ; 0028
    fabs ; 002A
    fsubrp st(1), st(0) ; 002C
    fmul qword ptr [rdx + 24] ; 002E
    faddp st(1), st(0) ; 0031
    fstp qword ptr [rcx + 32] ; 0033
    fld qword ptr [rdx + 16] ; 0036
    fld st(0) ; 0039
    fld st(0) ; 003B
    fadd st(2), st(0) ; 003D
    fchs ; 003F
    fld qword ptr [rcx] ; 0041
    fcom st(2) ; 0043
    fnstsw ax ; 0045
    shr ah, 1 ; 0047
    jb aboxvaryscale_0051 ; 0049
    fchs ; 004B
    fadd st(0), st(3) ; 004D
    jmp aboxvaryscale_005d ; 004F
aboxvaryscale_0051:
    fcom st(1) ; 0051
    fnstsw ax ; 0053
    shr ah, 1 ; 0055
    jae aboxvaryscale_005d ; 0057
    fchs ; 0059
    fsub st(0), st(3) ; 005B
aboxvaryscale_005d:
    fld qword ptr [rcx + 8] ; 005D
    fcom st(3) ; 005F
    fnstsw ax ; 0061
    shr ah, 1 ; 0063
    jb aboxvaryscale_006d ; 0065
    fchs ; 0067
    fadd st(0), st(4) ; 0069
    jmp aboxvaryscale_0079 ; 006B
aboxvaryscale_006d:
    fcom st(2) ; 006D
    fnstsw ax ; 006F
    shr ah, 1 ; 0071
    jae aboxvaryscale_0079 ; 0073
    fchs ; 0075
    fsub st(0), st(4) ; 0077
aboxvaryscale_0079:
    fld qword ptr [rcx + 16] ; 0079
    fcom st(4) ; 007B
    fnstsw ax ; 007D
    shr ah, 1 ; 007F
    jb aboxvaryscale_0089 ; 0081
    fchs ; 0083
    fadd st(0), st(5) ; 0085
    jmp aboxvaryscale_0095 ; 0087
aboxvaryscale_0089:
    fcom st(3) ; 0089
    fnstsw ax ; 008B
    shr ah, 1 ; 008D
    jae aboxvaryscale_0095 ; 008F
    fchs ; 0091
    fsub st(0), st(5) ; 0093
aboxvaryscale_0095:
    fxch st(3) ; 0095
    fstp st(0) ; 0097
    fxch st(3) ; 0099
    fstp st(0) ; 009B
    fxch st(3) ; 009D
    fstp st(0) ; 009F
    fld st(0) ; 00A1
    fmul st(0), st(1) ; 00A3
    fld st(2) ; 00A5
    fmul st(0), st(3) ; 00A7
    faddp st(1), st(0) ; 00A9
    fld st(3) ; 00AB
    fmul st(0), st(4) ; 00AD
    faddp st(1), st(0) ; 00AF
    fld qword ptr [rdx + 32] ; 00B1
    fxch st(1) ; 00B4
    fldln2 ; 00B6
    fxch st(1) ; 00B8
    fyl2x ; 00BA
    fxch st(1) ; 00BC
    fmulp st(1), st(0) ; 00BE
    fldl2e ; 00C0
    fmulp st(1), st(0) ; 00C2
    fld st(0) ; 00C4
    frndint ; 00C6
    fsub st(1), st(0) ; 00C8
    fxch st(1) ; 00CA
    f2xm1 ; 00CC
    fld1 ; 00CE
    faddp st(1), st(0) ; 00D0
    fscale ; 00D2
    fstp st(1) ; 00D4
    fcom qword ptr [rdx + 8] ; 00D6
    fnstsw ax ; 00D9
    shr ah, 1 ; 00DB
    jae aboxvaryscale_00e6 ; 00DD
    fstp st(0) ; 00DF
    fld qword ptr [rdx + 0] ; 00E1
    jmp aboxvaryscale_00fc ; 00E4
aboxvaryscale_00e6:
    fld1 ; 00E6
    fcom st(1) ; 00E8
    fnstsw ax ; 00EA
    shr ah, 1 ; 00EC
    jb aboxvaryscale_00f7 ; 00EE
    fstp st(0) ; 00F0
    fdivr qword ptr [rcx + 32] ; 00F2
    jmp aboxvaryscale_00fc ; 00F5
aboxvaryscale_00f7:
    fcompp ; 00F7
    fld qword ptr [rcx + 32] ; 00F9
aboxvaryscale_00fc:
    fld qword ptr [rcx + 24] ; 00FC
    fmul st(0), st(1) ; 00FF
    fstp qword ptr [rcx + 24] ; 0101
    fmul st(3), st(0) ; 0104
    fmul st(2), st(0) ; 0106
    fmulp st(1), st(0) ; 0108
    fadd qword ptr [rdx + 64] ; 010A
    fstp qword ptr [rcx + 16] ; 010D
    fadd qword ptr [rdx + 56] ; 010F
    fstp qword ptr [rcx + 8] ; 0112
    fadd qword ptr [rdx + 48] ; 0114
    fstp qword ptr [rcx] ; 0117
ABoxVaryScaleOriginal_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
ABoxVaryScaleOriginal_x87 ENDP


; Original varying-scale first state, Extended radius power, folds and writeback.
ABoxVaryScale4dOriginal_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    cmp dword ptr [r9], 0 ; 0014
    jg aboxvaryscale4d_0023 ; 0018
    inc dword ptr [r9] ; 001A
    fld qword ptr [rdx + 0] ; 001D
    fstp qword ptr [rcx + 32] ; 0020
aboxvaryscale4d_0023:
    fld qword ptr [rcx + 32] ; 0023
    fld1 ; 0026
    fld st(1) ; 0028
    fabs ; 002A
    fsubrp st(1), st(0) ; 002C
    fmul qword ptr [rdx + 32] ; 002E
    faddp st(1), st(0) ; 0031
    fstp qword ptr [rcx + 32] ; 0033
    fld qword ptr [rdx + 24] ; 0036
    fld qword ptr [rcx] ; 0039
    fld st(0) ; 003B
    fsub st(0), st(2) ; 003D
    fabs ; 003F
    fadd st(0), st(1) ; 0041
    fxch st(1) ; 0043
    fadd st(0), st(2) ; 0045
    fabs ; 0047
    fsubrp st(1), st(0) ; 0049
    fld qword ptr [rcx + 8] ; 004B
    fld st(0) ; 004D
    fsub st(0), st(3) ; 004F
    fabs ; 0051
    fadd st(0), st(1) ; 0053
    fxch st(1) ; 0055
    fadd st(0), st(3) ; 0057
    fabs ; 0059
    fsubrp st(1), st(0) ; 005B
    fld qword ptr [rcx + 16] ; 005D
    fld st(0) ; 005F
    fsub st(0), st(4) ; 0061
    fabs ; 0063
    fadd st(0), st(1) ; 0065
    fxch st(1) ; 0067
    fadd st(0), st(4) ; 0069
    fabs ; 006B
    fsubrp st(1), st(0) ; 006D
    fld qword ptr [rcx + 24] ; 006F
    fld st(0) ; 0072
    fsub st(0), st(5) ; 0074
    fabs ; 0076
    fadd st(0), st(1) ; 0078
    fxch st(1) ; 007A
    fadd st(0), st(5) ; 007C
    fabs ; 007E
    fsubrp st(1), st(0) ; 0080
    fxch st(4) ; 0082
    fstp st(0) ; 0084
    fld st(0) ; 0086
    fmul st(0), st(1) ; 0088
    fld st(2) ; 008A
    fmul st(0), st(3) ; 008C
    faddp st(1), st(0) ; 008E
    fld st(3) ; 0090
    fmul st(0), st(4) ; 0092
    faddp st(1), st(0) ; 0094
    fld st(4) ; 0096
    fmul st(0), st(5) ; 0098
    faddp st(1), st(0) ; 009A
    cmp dword ptr [r8], 3f800000h ; 009C
    je aboxvaryscale4d_00ca ; 00A3
    fld dword ptr [r8] ; 00A5
    fxch st(1) ; 00A8
    fldln2 ; 00AA
    fxch st(1) ; 00AC
    fyl2x ; 00AE
    fxch st(1) ; 00B0
    fmulp st(1), st(0) ; 00B2
    fldl2e ; 00B4
    fmulp st(1), st(0) ; 00B6
    fld st(0) ; 00B8
    frndint ; 00BA
    fsub st(1), st(0) ; 00BC
    fxch st(1) ; 00BE
    f2xm1 ; 00C0
    fld1 ; 00C2
    faddp st(1), st(0) ; 00C4
    fscale ; 00C6
    fstp st(1) ; 00C8
aboxvaryscale4d_00ca:
    fcom qword ptr [rdx + 16] ; 00CA
    fnstsw ax ; 00CD
    shr ah, 1 ; 00CF
    jae aboxvaryscale4d_00dd ; 00D1
    fstp st(0) ; 00D3
    fld qword ptr [rcx + 32] ; 00D5
    fdiv qword ptr [rdx + 16] ; 00D8
    jmp aboxvaryscale4d_00f3 ; 00DB
aboxvaryscale4d_00dd:
    fld1 ; 00DD
    fcom st(1) ; 00DF
    fnstsw ax ; 00E1
    shr ah, 1 ; 00E3
    jb aboxvaryscale4d_00ee ; 00E5
    fstp st(0) ; 00E7
    fdivr qword ptr [rcx + 32] ; 00E9
    jmp aboxvaryscale4d_00f3 ; 00EC
aboxvaryscale4d_00ee:
    fcompp ; 00EE
    fld qword ptr [rcx + 32] ; 00F0
aboxvaryscale4d_00f3:
    fld qword ptr [rcx + 40] ; 00F3
    fmul st(0), st(1) ; 00F6
    fstp qword ptr [rcx + 40] ; 00F8
    fmul st(4), st(0) ; 00FB
    fmul st(3), st(0) ; 00FD
    fmul st(2), st(0) ; 00FF
    fmulp st(1), st(0) ; 0101
    fadd qword ptr [rdx + 72] ; 0103
    fstp qword ptr [rcx + 16] ; 0106
    fadd qword ptr [rdx + 64] ; 0108
    fstp qword ptr [rcx + 8] ; 010B
    fadd qword ptr [rdx + 56] ; 010D
    fstp qword ptr [rcx] ; 0110
    fadd qword ptr [rdx + 48] ; 0112
    fadd qword ptr [rdx + 80] ; 0115
    fstp qword ptr [rcx + 24] ; 011B
ABoxVaryScale4dOriginal_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
ABoxVaryScale4dOriginal_x87 ENDP


; Original _AmazingBoxSSE2 000A..008A, same SSE2 operand and sum order.
; rcx: XYZW/DE. rdx: 16-byte aligned lower/upper pairs, radius, quotient, scale, one.
AmazingBoxSSE2Original PROC
    movupd xmm2, xmmword ptr [rcx] ; 000A
    movsd xmm4, qword ptr [rcx + 16] ; 000E
    movapd xmm0, xmm2 ; 0012
    maxpd xmm0, xmmword ptr [rdx] ; 0016
    maxsd xmm4, qword ptr [rdx] ; 001B
    minpd xmm0, xmmword ptr [rdx + 16] ; 0020
    minsd xmm4, qword ptr [rdx + 16] ; 0025
    addpd xmm0, xmm0 ; 002A
    addsd xmm4, xmm4 ; 002E
    subpd xmm0, xmm2 ; 0032
    subsd xmm4, qword ptr [rcx + 16] ; 0036
    movapd xmm1, xmm0 ; 003A
    movsd xmm5, xmm4 ; 003E
    mulpd xmm1, xmm1 ; 0042
    mulsd xmm5, xmm5 ; 0046
    pshufd xmm2, xmm1, 4Eh ; 004A
    addsd xmm1, xmm5 ; 004F
    addsd xmm1, xmm2 ; 0053
    ucomisd xmm1, qword ptr [rdx + 32] ; 0057
    jae amazingboxsse2_0065 ; 005C
    movsd xmm3, qword ptr [rdx + 40] ; 005E
    jmp amazingboxsse2_0074 ; 0063
amazingboxsse2_0065:
    ucomisd xmm1, qword ptr [rdx + 56] ; 0065
    movsd xmm3, qword ptr [rdx + 48] ; 0069
    jae amazingboxsse2_0074 ; 006E
    divsd xmm3, xmm1 ; 0070
amazingboxsse2_0074:
    movhpd xmm4, qword ptr [rcx + 24] ; 0074
    shufpd xmm3, xmm3, 0 ; 0079
    mulpd xmm0, xmm3 ; 007E
    mulpd xmm4, xmm3 ; 0082
    movupd xmmword ptr [rcx], xmm0 ; 0086
    movupd xmmword ptr [rcx + 16], xmm4 ; 008A
    ret
AmazingBoxSSE2Original ENDP


; Original _BStellahedron_F_Sphere arithmetic and comparison order; rcx includes six scratch Doubles.
BStellahedron_F_SphereOriginal_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    fnop ; 0014
    fld qword ptr [rdx + 32] ; 0016
    fdiv qword ptr [rdx + 0] ; 0018
    fst qword ptr [rcx + 24] ; 001B
    fadd st(0), st(0) ; 001E
    fstp qword ptr [rcx + 32] ; 0020
    fld qword ptr [rcx + 24] ; 0023
    fmul qword ptr [rdx + 40] ; 0026
    fld st(0) ; 0029
    fsub qword ptr [rdx + 16] ; 002B
    fcos ; 002E
    fstp qword ptr [rcx + 56] ; 0030
    fcos ; 0033
    fdiv qword ptr [rcx + 56] ; 0035
    fstp qword ptr [rcx + 56] ; 0038
    fld qword ptr [rdx + 32] ; 003B
    fdiv qword ptr [rdx + 8] ; 003D
    fst qword ptr [rcx + 40] ; 0040
    fadd st(0), st(0) ; 0043
    fstp qword ptr [rcx + 48] ; 0045
    fld qword ptr [rcx + 40] ; 0048
    fmul qword ptr [rdx + 40] ; 004B
    fld st(0) ; 004E
    fsub qword ptr [rdx + 24] ; 0050
    fcos ; 0053
    fstp qword ptr [rcx + 64] ; 0055
    fcos ; 0058
    fdiv qword ptr [rcx + 64] ; 005A
    fstp qword ptr [rcx + 64] ; 005D
    fld qword ptr [rcx + 16] ; 0060
    fld qword ptr [rcx + 8] ; 0062
    fpatan ; 0064
    fabs ; 0066
bstellahedron_f_sphere_0068:
    fcom qword ptr [rcx + 24] ; 0068
    fnstsw ax ; 006B
    and ah, 41h ; 006D
    jg bstellahedron_f_sphere_0077 ; 0070
    fsub qword ptr [rcx + 32] ; 0072
    jmp bstellahedron_f_sphere_0068 ; 0075
bstellahedron_f_sphere_0077:
    fabs ; 0077
    fsub qword ptr [rdx + 16] ; 0079
    fcos ; 007C
    fmul qword ptr [rcx + 56] ; 007E
    fst qword ptr [rcx + 32] ; 0081
    fmul qword ptr [rcx + 8] ; 0084
    fst qword ptr [rcx + 24] ; 0086
    fmul st(0), st(0) ; 0089
    fld qword ptr [rcx + 32] ; 008B
    fmul qword ptr [rcx + 16] ; 008E
    fst qword ptr [rcx + 32] ; 0090
    fmul st(0), st(0) ; 0093
    faddp st(1), st(0) ; 0095
    fsqrt ; 0097
    fnop ; 0099
    fld qword ptr [rcx] ; 009B
    fpatan ; 009D
    fabs ; 009F
bstellahedron_f_sphere_00a1:
    fcom qword ptr [rcx + 40] ; 00A1
    fnstsw ax ; 00A4
    and ah, 41h ; 00A6
    jg bstellahedron_f_sphere_00b0 ; 00A9
    fsub qword ptr [rcx + 48] ; 00AB
    jmp bstellahedron_f_sphere_00a1 ; 00AE
bstellahedron_f_sphere_00b0:
    fabs ; 00B0
    fsub qword ptr [rdx + 24] ; 00B2
    fcos ; 00B5
    fmul qword ptr [rcx + 64] ; 00B7
    fld st(0) ; 00BA
    fld st(0) ; 00BC
    fmul qword ptr [rcx + 24] ; 00BE
    fstp qword ptr [rcx + 8] ; 00C1
    fnop ; 00C3
    fmul qword ptr [rcx + 32] ; 00C5
    fstp qword ptr [rcx + 16] ; 00C8
    fnop ; 00CA
    fmul qword ptr [rcx] ; 00CC
    fstp qword ptr [rcx] ; 00CE
    fnop ; 00D0
BStellahedron_F_SphereOriginal_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
BStellahedron_F_SphereOriginal_x87 ENDP


; Original _BPolygonFromCircle arithmetic and comparison order; rcx includes six scratch Doubles.
BPolygonFromCircleOriginal_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    fnop ; 0014
    fld qword ptr [rdx + 8] ; 0016
    fdiv qword ptr [rdx + 0] ; 0018
    fst qword ptr [rcx + 24] ; 001B
    fadd st(0), st(0) ; 001E
    fstp qword ptr [rcx + 32] ; 0020
    fld qword ptr [rcx + 16] ; 0023
    fld qword ptr [rcx + 8] ; 0025
    fpatan ; 0027
    fabs ; 0029
bpolygonfromcircle_002b:
    fcom qword ptr [rcx + 24] ; 002B
    fnstsw ax ; 002E
    and ah, 41h ; 0030
    jg bpolygonfromcircle_003a ; 0033
    fsub qword ptr [rcx + 32] ; 0035
    jmp bpolygonfromcircle_002b ; 0038
bpolygonfromcircle_003a:
    fcos ; 003A
    fld st(0) ; 003C
    fmul qword ptr [rcx + 8] ; 003E
    fstp qword ptr [rcx + 8] ; 0040
    fnop ; 0042
    fmul qword ptr [rcx + 16] ; 0044
    fstp qword ptr [rcx + 16] ; 0046
    fnop ; 0048
BPolygonFromCircleOriginal_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
BPolygonFromCircleOriginal_x87 ENDP


; Original _BPolygonToCircle arithmetic and comparison order; rcx includes six scratch Doubles.
BPolygonToCircleOriginal_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    fnop ; 0014
    fld qword ptr [rdx + 8] ; 0016
    fdiv qword ptr [rdx + 0] ; 0018
    fst qword ptr [rcx + 24] ; 001B
    fadd st(0), st(0) ; 001E
    fstp qword ptr [rcx + 32] ; 0020
    fld qword ptr [rcx + 16] ; 0023
    fld qword ptr [rcx + 8] ; 0025
    fpatan ; 0027
    fabs ; 0029
bpolygontocircle_002b:
    fcom qword ptr [rcx + 24] ; 002B
    fnstsw ax ; 002E
    and ah, 41h ; 0030
    jg bpolygontocircle_003a ; 0033
    fsub qword ptr [rcx + 32] ; 0035
    jmp bpolygontocircle_002b ; 0038
bpolygontocircle_003a:
    fcos ; 003A
    fld1 ; 003C
    fdivrp st(1), st(0) ; 003E
    fld st(0) ; 0040
    fmul qword ptr [rcx + 8] ; 0042
    fstp qword ptr [rcx + 8] ; 0044
    fnop ; 0046
    fmul qword ptr [rcx + 16] ; 0048
    fstp qword ptr [rcx + 16] ; 004A
    fnop ; 004C
BPolygonToCircleOriginal_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
BPolygonToCircleOriginal_x87 ENDP


; Original _BPolyhedronToSphere arithmetic and comparison order; rcx includes six scratch Doubles.
BPolyhedronToSphereOriginal_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    fnop ; 0014
    fld qword ptr [rdx + 16] ; 0016
    fdiv qword ptr [rdx + 0] ; 0018
    fst qword ptr [rcx + 24] ; 001B
    fadd st(0), st(0) ; 001E
    fstp qword ptr [rcx + 32] ; 0020
    fld qword ptr [rdx + 16] ; 0023
    fdiv qword ptr [rdx + 8] ; 0025
    fst qword ptr [rcx + 40] ; 0028
    fadd st(0), st(0) ; 002B
    fstp qword ptr [rcx + 48] ; 002D
    fld qword ptr [rcx + 16] ; 0030
    fmul st(0), st(0) ; 0032
    fld qword ptr [rcx + 8] ; 0034
    fmul st(0), st(0) ; 0036
    faddp st(1), st(0) ; 0038
    fsqrt ; 003A
    fld qword ptr [rcx] ; 003C
    fpatan ; 003E
    fabs ; 0040
bpolyhedrontosphere_0042:
    fcom qword ptr [rcx + 40] ; 0042
    fnstsw ax ; 0045
    and ah, 41h ; 0047
    jg bpolyhedrontosphere_0051 ; 004A
    fsub qword ptr [rcx + 48] ; 004C
    jmp bpolyhedrontosphere_0042 ; 004F
bpolyhedrontosphere_0051:
    fcos ; 0051
    fld1 ; 0053
    fdivrp st(1), st(0) ; 0055
    fld st(0) ; 0057
    fld st(0) ; 0059
    fmul qword ptr [rcx] ; 005B
    fstp qword ptr [rcx] ; 005D
    fmul qword ptr [rcx + 8] ; 005F
    fstp qword ptr [rcx + 40] ; 0061
    fmul qword ptr [rcx + 16] ; 0064
    fst qword ptr [rcx + 48] ; 0066
    fld qword ptr [rcx + 40] ; 0069
    fpatan ; 006C
    fabs ; 006E
bpolyhedrontosphere_0070:
    fcom qword ptr [rcx + 24] ; 0070
    fnstsw ax ; 0073
    and ah, 41h ; 0075
    jg bpolyhedrontosphere_007f ; 0078
    fsub qword ptr [rcx + 32] ; 007A
    jmp bpolyhedrontosphere_0070 ; 007D
bpolyhedrontosphere_007f:
    fcos ; 007F
    fld1 ; 0081
    fdivrp st(1), st(0) ; 0083
    fld st(0) ; 0085
    fmul qword ptr [rcx + 40] ; 0087
    fstp qword ptr [rcx + 8] ; 008A
    fmul qword ptr [rcx + 48] ; 008C
    fstp qword ptr [rcx + 16] ; 008F
    fnop ; 0091
BPolyhedronToSphereOriginal_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
BPolyhedronToSphereOriginal_x87 ENDP


; Original _Fold45double arithmetic and comparison order; rcx includes six scratch Doubles.
Fold45doubleOriginal_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    cmp dword ptr [r8 + 0], 0 ; 0010
    je fold45double_001b ; 0014
    call fold45double_00dd ; 0016
fold45double_001b:
    fld qword ptr [rcx] ; 001B
    cmp dword ptr [r8 + 4], 0 ; 001D
    je fold45double_0025 ; 0021
    fchs ; 0023
fold45double_0025:
    fld qword ptr [rcx + 8] ; 0025
    cmp dword ptr [r8 + 8], 0 ; 0027
    je fold45double_002f ; 002B
    fchs ; 002D
fold45double_002f:
    faddp st(1), st(0) ; 002F
    fld qword ptr [rcx + 16] ; 0031
    cmp dword ptr [r8 + 12], 0 ; 0033
    je fold45double_003b ; 0037
    fchs ; 0039
fold45double_003b:
    faddp st(1), st(0) ; 003B
    fld qword ptr [rdx + 0] ; 003D
    fabs ; 0040
    fsubp st(1), st(0) ; 0042
    fld st(0) ; 0044
    ftst ; 0046
    fnstsw ax ; 0048
    sahf ; 004A
    fstp st(0) ; 004B
    jb fold45double_0086 ; 004D
    fmul qword ptr [rdx + 48] ; 004F
    fld qword ptr [rcx] ; 0051
    cmp dword ptr [r8 + 4], 0 ; 0053
    je fold45double_005d ; 0057
    fadd st(0), st(1) ; 0059
    jmp fold45double_005f ; 005B
fold45double_005d:
    fsub st(0), st(1) ; 005D
fold45double_005f:
    fstp qword ptr [rcx] ; 005F
    fld qword ptr [rcx + 8] ; 0061
    cmp dword ptr [r8 + 8], 0 ; 0063
    je fold45double_006d ; 0067
    fadd st(0), st(1) ; 0069
    jmp fold45double_006f ; 006B
fold45double_006d:
    fsub st(0), st(1) ; 006D
fold45double_006f:
    fstp qword ptr [rcx + 8] ; 006F
    fld qword ptr [rcx + 16] ; 0071
    cmp dword ptr [r8 + 12], 0 ; 0073
    je fold45double_007d ; 0077
    fadd st(0), st(1) ; 0079
    jmp fold45double_007f ; 007B
fold45double_007d:
    fsub st(0), st(1) ; 007D
fold45double_007f:
    fstp qword ptr [rcx + 16] ; 007F
    jmp fold45double_00d0 ; 0081
    nop ; 0083
    nop ; 0084
    nop ; 0085
fold45double_0086:
    fld qword ptr [rdx + 0] ; 0086
    fabs ; 0089
    fadd st(1), st(0) ; 008B
    fadd qword ptr [rdx + 40] ; 008D
    faddp st(1), st(0) ; 0090
    fld st(0) ; 0092
    ftst ; 0094
    fnstsw ax ; 0096
    sahf ; 0098
    fstp st(0) ; 0099
    ja fold45double_00d0 ; 009B
    fmul qword ptr [rdx + 48] ; 009D
    fld qword ptr [rcx] ; 009F
    cmp dword ptr [r8 + 4], 0 ; 00A1
    je fold45double_00ab ; 00A5
    fadd st(0), st(1) ; 00A7
    jmp fold45double_00ad ; 00A9
fold45double_00ab:
    fsub st(0), st(1) ; 00AB
fold45double_00ad:
    fstp qword ptr [rcx] ; 00AD
    fld qword ptr [rcx + 8] ; 00AF
    cmp dword ptr [r8 + 8], 0 ; 00B1
    je fold45double_00bb ; 00B5
    fadd st(0), st(1) ; 00B7
    jmp fold45double_00bd ; 00B9
fold45double_00bb:
    fsub st(0), st(1) ; 00BB
fold45double_00bd:
    fstp qword ptr [rcx + 8] ; 00BD
    fld qword ptr [rcx + 16] ; 00BF
    cmp dword ptr [r8 + 12], 0 ; 00C1
    je fold45double_00cb ; 00C5
    fadd st(0), st(1) ; 00C7
    jmp fold45double_00cd ; 00C9
fold45double_00cb:
    fsub st(0), st(1) ; 00CB
fold45double_00cd:
    fstp qword ptr [rcx + 16] ; 00CD
    nop ; 00CF
fold45double_00d0:
    fstp st(0) ; 00D0
    fnop ; 00D2
    jmp Fold45doubleOriginal_x87_done
fold45double_00dd:
    fld qword ptr [rcx] ; 00DD
    cmp dword ptr [r8 + 4], 0 ; 00DF
    jne fold45double_00e7 ; 00E3
    fchs ; 00E5
fold45double_00e7:
    fld qword ptr [rcx + 8] ; 00E7
    cmp dword ptr [r8 + 8], 0 ; 00E9
    jne fold45double_00f1 ; 00ED
    fchs ; 00EF
fold45double_00f1:
    faddp st(1), st(0) ; 00F1
    fld qword ptr [rcx + 16] ; 00F3
    cmp dword ptr [r8 + 12], 0 ; 00F5
    jne fold45double_00fd ; 00F9
    fchs ; 00FB
fold45double_00fd:
    faddp st(1), st(0) ; 00FD
    fmul qword ptr [rdx + 48] ; 00FF
    fld qword ptr [rcx] ; 0101
    cmp dword ptr [r8 + 4], 0 ; 0103
    jne fold45double_010d ; 0107
    fadd st(0), st(1) ; 0109
    jmp fold45double_010f ; 010B
fold45double_010d:
    fsub st(0), st(1) ; 010D
fold45double_010f:
    fchs ; 010F
    fstp qword ptr [rcx] ; 0111
    fld qword ptr [rcx + 8] ; 0113
    cmp dword ptr [r8 + 8], 0 ; 0115
    jne fold45double_011f ; 0119
    fadd st(0), st(1) ; 011B
    jmp fold45double_0121 ; 011D
fold45double_011f:
    fsub st(0), st(1) ; 011F
fold45double_0121:
    fchs ; 0121
    fstp qword ptr [rcx + 8] ; 0123
    fld qword ptr [rcx + 16] ; 0125
    cmp dword ptr [r8 + 12], 0 ; 0127
    jne fold45double_0131 ; 012B
    fadd st(0), st(1) ; 012D
    jmp fold45double_0133 ; 012F
fold45double_0131:
    fsub st(0), st(1) ; 0131
fold45double_0133:
    fchs ; 0133
    fstp qword ptr [rcx + 16] ; 0135
    fstp st(0) ; 0137
    ret ; 0139
Fold45doubleOriginal_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
Fold45doubleOriginal_x87 ENDP


; Original reciprocal X/Y/Z3, shared scalar arithmetic and sign-bit XOR.
Reciprocal3Original_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    mov r8b, byte ptr [rcx + 7]
    and r8b, 80h
    fld qword ptr [rcx]
    fabs
    fadd qword ptr [rdx]
    fld1
    fdivrp st(1), st(0)
    fchs
    fadd qword ptr [rdx + 8]
    fstp qword ptr [rcx]
    xor byte ptr [rcx + 7], r8b
Reciprocal3Original_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
Reciprocal3Original_x87 ENDP


; Original CitrusIFS CODE: conditional SSE2 commit, local Single rotation, Extended surface and clipping.
CitrusIFSOriginal_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    movupd xmm0, xmmword ptr [rcx] ; 0003
    movsd xmm1, qword ptr [rcx + 16] ; 0008
    movsd xmm2, qword ptr [rdx + 8]
    unpcklpd xmm2, xmm2
    mulpd xmm0, xmm2
    mulsd xmm1, qword ptr [rdx + 8] ; 0012
    movsd xmm3, qword ptr [rdx + 32]
    movsd xmm4, qword ptr [rdx + 24]
    unpcklpd xmm3, xmm4
    addpd xmm0, xmm3
    addsd xmm1, qword ptr [rdx + 16] ; 001C
    test r9d, r9d
    je citrusifs_0040 ; 0025
    movsd xmm5, qword ptr [rcx + 24] ; 0027
    mulsd xmm5, qword ptr [rdx + 8] ; 002C
    movupd xmmword ptr [rcx], xmm0 ; 0031
    movsd qword ptr [rcx + 16], xmm1 ; 0036
    movsd qword ptr [rcx + 24], xmm5 ; 003B
citrusifs_0040:
    movupd xmmword ptr [rcx + 32], xmm0 ; 0040
    movsd qword ptr [rcx + 48], xmm1 ; 0045
    fldz ; 004B
    fst qword ptr [rcx + 56] ; 004D
    fstp qword ptr [rcx + 64] ; 0051
    fld qword ptr [rcx + 48] ; 0058
    fld qword ptr [rcx + 40] ; 005C
    fld qword ptr [rcx + 32] ; 0060
    fld st(0) ; 0063
    fmul dword ptr [r8 + 24] ; 0065
    fld st(2) ; 0068
    fmul dword ptr [r8 + 28] ; 006A
    faddp st(1), st(0) ; 006D
    fld st(3) ; 006F
    fmul dword ptr [r8 + 32] ; 0071
    faddp st(1), st(0) ; 0074
    fld st(1) ; 0076
    fmul dword ptr [r8 + 0] ; 0078
    fld st(3) ; 007B
    fmul dword ptr [r8 + 4] ; 007D
    faddp st(1), st(0) ; 0080
    fld st(4) ; 0082
    fmul dword ptr [r8 + 8] ; 0084
    faddp st(1), st(0) ; 0087
    fxch st(2) ; 0089
    fmul dword ptr [r8 + 12] ; 008B
    fxch st(3) ; 008E
    fmul dword ptr [r8 + 16] ; 0090
    faddp st(3), st(0) ; 0093
    fxch st(3) ; 0095
    fmul dword ptr [r8 + 20] ; 0097
    faddp st(2), st(0) ; 009A
    fstp qword ptr [rcx + 32] ; 009C
    fstp qword ptr [rcx + 40] ; 009F
    fstp qword ptr [rcx + 48] ; 00A3
    fld qword ptr [rcx + 48] ; 00AA
    fmul st(0), st(0) ; 00AE
    fld qword ptr [rcx + 40] ; 00B0
    fmul st(0), st(0) ; 00B4
    faddp st(1), st(0) ; 00B6
    fmul qword ptr [rdx + 40] ; 00B8
    fstp qword ptr [rcx + 56] ; 00BB
    fld qword ptr [rcx + 32] ; 00BF
    fsub qword ptr [rdx + 0] ; 00C2
    fld st(0) ; 00C5
    fmul st(0), st(0) ; 00C7
    fmulp st(1), st(0) ; 00C9
    fmul qword ptr [rcx + 32] ; 00CB
    fmul qword ptr [rcx + 32] ; 00CE
    fmul qword ptr [rcx + 32] ; 00D1
    nop ; 00D4
    ftst ; 00D5
    wait ; 00D7
    fnstsw ax ; 00D8
    shr ah, 1 ; 00DA
    jae citrusifs_00e8 ; 00DC
    fsubr qword ptr [rcx + 64] ; 00DE
    fstp qword ptr [rcx + 64] ; 00E2
    jmp citrusifs_00f0 ; 00E6
citrusifs_00e8:
    fadd qword ptr [rcx + 56] ; 00E8
    fstp qword ptr [rcx + 56] ; 00EC
citrusifs_00f0:
    fld qword ptr [rcx + 56] ; 00F0
    fsqrt ; 00F4
    fsqrt ; 00F6
    fld qword ptr [rcx + 64] ; 00F8
    fsqrt ; 00FC
    fsqrt ; 00FE
    fchs ; 0100
    faddp st(1), st(0) ; 0102
    fld qword ptr [rcx + 32] ; 0104
    fmul st(0), st(0) ; 0107
    fld qword ptr [rcx + 40] ; 0109
    fmul st(0), st(0) ; 010D
    faddp st(1), st(0) ; 010F
    fsqrt ; 0111
    fsub qword ptr [rdx + 40] ; 0113
    fsub qword ptr [rdx + 40] ; 0116
    fsub qword ptr [rdx + 40] ; 0119
    fcom st(1) ; 011C
    wait ; 011E
    fnstsw ax ; 011F
    shr ah, 1 ; 0121
    jae citrusifs_0129 ; 0123
    fstp st(0) ; 0125
    jmp citrusifs_012b ; 0127
citrusifs_0129:
    fstp st(1) ; 0129
citrusifs_012b:
    fld qword ptr [rcx + 48] ; 012B
    fabs ; 012F
    fsub qword ptr [rdx + 40] ; 0131
    fsub qword ptr [rdx + 40] ; 0134
    fsub qword ptr [rdx + 40] ; 0137
    fcom st(1) ; 013A
    wait ; 013C
    fnstsw ax ; 013D
    shr ah, 1 ; 013F
    jae citrusifs_0147 ; 0141
    fstp st(0) ; 0143
    jmp citrusifs_0149 ; 0145
citrusifs_0147:
    fstp st(1) ; 0147
citrusifs_0149:
    test r9d, r9d
    jne citrusifs_0152 ; 014D
    fdiv qword ptr [rdx + 8] ; 014F
citrusifs_0152:
    fstp qword ptr [rcx + 72] ; 0152
CitrusIFSOriginal_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
CitrusIFSOriginal_x87 ENDP


; Original DevilIFS CODE: conditional SSE2 commit, local Single rotation, Extended surface and clipping.
DevilIFSOriginal_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    movupd xmm0, xmmword ptr [rcx] ; 0003
    movsd xmm1, qword ptr [rcx + 16] ; 0008
    movsd xmm2, qword ptr [rdx + 8]
    unpcklpd xmm2, xmm2
    mulpd xmm0, xmm2
    mulsd xmm1, qword ptr [rdx + 8] ; 0012
    movsd xmm3, qword ptr [rdx + 32]
    movsd xmm4, qword ptr [rdx + 24]
    unpcklpd xmm3, xmm4
    addpd xmm0, xmm3
    addsd xmm1, qword ptr [rdx + 16] ; 001C
    test r9d, r9d
    je devilifs_0040 ; 0025
    movsd xmm5, qword ptr [rcx + 24] ; 0027
    mulsd xmm5, qword ptr [rdx + 8] ; 002C
    movupd xmmword ptr [rcx], xmm0 ; 0031
    movsd qword ptr [rcx + 16], xmm1 ; 0036
    movsd qword ptr [rcx + 24], xmm5 ; 003B
devilifs_0040:
    movupd xmmword ptr [rcx + 32], xmm0 ; 0040
    movsd qword ptr [rcx + 48], xmm1 ; 0045
    fldz ; 004B
    fst qword ptr [rcx + 56] ; 004D
    fstp qword ptr [rcx + 64] ; 0051
    fld qword ptr [rcx + 48] ; 0058
    fld qword ptr [rcx + 40] ; 005C
    fld qword ptr [rcx + 32] ; 0060
    fld st(0) ; 0063
    fmul dword ptr [r8 + 24] ; 0065
    fld st(2) ; 0068
    fmul dword ptr [r8 + 28] ; 006A
    faddp st(1), st(0) ; 006D
    fld st(3) ; 006F
    fmul dword ptr [r8 + 32] ; 0071
    faddp st(1), st(0) ; 0074
    fld st(1) ; 0076
    fmul dword ptr [r8 + 0] ; 0078
    fld st(3) ; 007B
    fmul dword ptr [r8 + 4] ; 007D
    faddp st(1), st(0) ; 0080
    fld st(4) ; 0082
    fmul dword ptr [r8 + 8] ; 0084
    faddp st(1), st(0) ; 0087
    fxch st(2) ; 0089
    fmul dword ptr [r8 + 12] ; 008B
    fxch st(3) ; 008E
    fmul dword ptr [r8 + 16] ; 0090
    faddp st(3), st(0) ; 0093
    fxch st(3) ; 0095
    fmul dword ptr [r8 + 20] ; 0097
    faddp st(2), st(0) ; 009A
    fstp qword ptr [rcx + 32] ; 009C
    fstp qword ptr [rcx + 40] ; 009F
    fstp qword ptr [rcx + 48] ; 00A3
    fld qword ptr [rcx + 32] ; 00AA
    nop ; 00AD
    fmul st(0), st(0) ; 00AE
    fstp qword ptr [rcx + 32] ; 00B0
    nop ; 00B3
    fld qword ptr [rcx + 40] ; 00B4
    fmul st(0), st(0) ; 00B8
    fstp qword ptr [rcx + 40] ; 00BA
    fld qword ptr [rcx + 48] ; 00BE
    fmul st(0), st(0) ; 00C2
    fstp qword ptr [rcx + 48] ; 00C4
    fld qword ptr [rcx + 32] ; 00C8
    fmul st(0), st(0) ; 00CB
    fld qword ptr [rcx + 32] ; 00CD
    fmul qword ptr [rcx + 48] ; 00D0
    fadd st(1), st(0) ; 00D4
    faddp st(1), st(0) ; 00D6
    fld qword ptr [rcx + 40] ; 00D8
    fmul qword ptr [rdx + 136] ; 00DC
    faddp st(1), st(0) ; 00DF
    fld qword ptr [rcx + 48] ; 00E1
    fmul st(0), st(0) ; 00E5
    faddp st(1), st(0) ; 00E7
    fstp qword ptr [rcx + 56] ; 00E9
    fld qword ptr [rcx + 32] ; 00ED
    nop ; 00F0
    fmul qword ptr [rdx + 128] ; 00F1
    nop ; 00F3
    fld qword ptr [rcx + 40] ; 00F4
    fmul st(0), st(0) ; 00F8
    faddp st(1), st(0) ; 00FA
    fstp qword ptr [rcx + 64] ; 00FC
    fld qword ptr [rcx + 56] ; 0100
    fsqrt ; 0104
    fsqrt ; 0106
    fld qword ptr [rcx + 64] ; 0108
    fsqrt ; 010C
    fsqrt ; 010E
    fchs ; 0110
    faddp st(1), st(0) ; 0112
    fmul qword ptr [rdx + 144] ; 0114
    fld qword ptr [rcx + 32] ; 0117
    fld qword ptr [rcx + 40] ; 011A
    fld qword ptr [rcx + 48] ; 011E
    faddp st(1), st(0) ; 0122
    faddp st(1), st(0) ; 0124
    fsqrt ; 0126
    fsub qword ptr [rdx + 0] ; 0128
    fcom st(1) ; 012B
    wait ; 012D
    fnstsw ax ; 012E
    shr ah, 1 ; 0130
    jae devilifs_0138 ; 0132
    fstp st(0) ; 0134
    jmp devilifs_013a ; 0136
devilifs_0138:
    fstp st(1) ; 0138
devilifs_013a:
    test r9d, r9d
    jne devilifs_0143 ; 013E
    fdiv qword ptr [rdx + 8] ; 0140
devilifs_0143:
    fstp qword ptr [rcx + 72] ; 0143
DevilIFSOriginal_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
DevilIFSOriginal_x87 ENDP


; Original Mandel4DBiC: two complex squares with internal C and W_add.
Mandel4DBiCOriginal_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    fld qword ptr [rcx] ; 0010
    fmul st(0), st(0) ; 0012
    fld qword ptr [rcx + 8] ; 0014
    fmul st(0), st(0) ; 0016
    fsubp st(1), st(0) ; 0018
    fadd qword ptr [rdx + 8] ; 001A
    fnop ; 001D
    fld qword ptr [rcx] ; 001F
    fmul qword ptr [rcx + 8] ; 0021
    fadd st(0), st(0) ; 0023
    fadd qword ptr [rdx + 16] ; 0025
    fnop ; 0028
    fld qword ptr [rcx + 16] ; 002A
    fmul st(0), st(0) ; 002C
    fld qword ptr [rcx + 24] ; 002E
    fmul st(0), st(0) ; 0031
    fsubp st(1), st(0) ; 0033
    fadd qword ptr [rdx + 24] ; 0035
    fnop ; 0038
    fld qword ptr [rcx + 16] ; 003A
    fmul qword ptr [rcx + 24] ; 003C
    fadd st(0), st(0) ; 003F
    fadd qword ptr [rdx + 32] ; 0041
    fadd qword ptr [rdx] ; 0044
    fnop ; 0047
    fstp qword ptr [rcx + 24] ; 0049
    fstp qword ptr [rcx + 16] ; 004C
    fstp qword ptr [rcx + 8] ; 004E
    fstp qword ptr [rcx] ; 0050
Mandel4DBiCOriginal_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
Mandel4DBiCOriginal_x87 ENDP


; Original Magnet4D: Double spills, Extended triplex arithmetic, first-iteration counter write.
Magnet4DOriginal_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    fnop ; 0014
    test r9d, r9d
    je magnet4d_0044 ; 001D
    cmp dword ptr [r8], 0 ; 001F
    jne magnet4d_0044 ; 0026
    fldz ; 0028
    fst qword ptr [rcx] ; 002A
    fst qword ptr [rcx + 8] ; 002C
    fst qword ptr [rcx + 56] ; 002E
    fstp qword ptr [rcx + 64] ; 0031
    fld qword ptr [rcx + 16] ; 0034
    fstp qword ptr [rcx + 72] ; 0036
    inc dword ptr [r8] ; 0039
    jmp magnet4d_0096 ; 003F
    nop ; 0041
    nop ; 0042
    nop ; 0043
magnet4d_0044:
    fld qword ptr [rcx] ; 0044
    fmul st(0), st(0) ; 0046
    fld qword ptr [rcx + 8] ; 0048
    fmul st(0), st(0) ; 004A
    faddp st(1), st(0) ; 004C
    nop ; 004E
    nop ; 004F
    fstp qword ptr [rcx + 40] ; 0050
    nop ; 0053
    fld qword ptr [rcx + 16] ; 0054
    fmul st(0), st(0) ; 0056
    fdiv qword ptr [rcx + 40] ; 0058
    fchs ; 005B
    fld1 ; 005D
    faddp st(1), st(0) ; 005F
    fstp qword ptr [rcx + 48] ; 0061
    nop ; 0064
    fld qword ptr [rcx] ; 0065
    fmul st(0), st(0) ; 0067
    fld qword ptr [rcx + 8] ; 0069
    fmul st(0), st(0) ; 006B
    fsubp st(1), st(0) ; 006D
    fmul qword ptr [rcx + 48] ; 006F
    fstp qword ptr [rcx + 56] ; 0072
    fnop ; 0075
    fld qword ptr [rcx] ; 0077
    fmul qword ptr [rcx + 8] ; 0079
    fmul qword ptr [rdx + 40] ; 007B
    fmul qword ptr [rcx + 48] ; 007D
    fstp qword ptr [rcx + 64] ; 0080
    fnop ; 0083
    fld qword ptr [rcx + 40] ; 0085
    fsqrt ; 0088
    fmul qword ptr [rcx + 16] ; 008A
    fmul qword ptr [rdx + 40] ; 008C
    fmul qword ptr [rdx + 0] ; 008E
    fstp qword ptr [rcx + 72] ; 0091
    fnop ; 0094
magnet4d_0096:
    fld qword ptr [rcx + 56] ; 0096
    fld qword ptr [rdx + 48] ; 0099
    fmul qword ptr [rdx + 8] ; 009C
    faddp st(1), st(0) ; 009F
    fld qword ptr [rdx + 24] ; 00A1
    nop ; 00A4
    faddp st(1), st(0) ; 00A5
    fstp qword ptr [rcx + 56] ; 00A7
    fnop ; 00AA
    fld qword ptr [rcx + 64] ; 00AC
    fld qword ptr [rdx + 56] ; 00AF
    fmul qword ptr [rdx + 8] ; 00B2
    faddp st(1), st(0) ; 00B5
    fstp qword ptr [rcx + 64] ; 00B7
    fnop ; 00BA
    fld qword ptr [rcx + 72] ; 00BC
    fld qword ptr [rdx + 64] ; 00BF
    fmul qword ptr [rdx + 8] ; 00C2
    faddp st(1), st(0) ; 00C5
    fstp qword ptr [rcx + 72] ; 00C7
    fnop ; 00CA
    fld qword ptr [rcx] ; 00CC
    fmul qword ptr [rdx + 40] ; 00CE
    fld qword ptr [rdx + 24] ; 00D0
    fld1 ; 00D3
    fchs ; 00D5
    faddp st(1), st(0) ; 00D7
    faddp st(1), st(0) ; 00D9
    fld qword ptr [rdx + 48] ; 00DB
    fmul qword ptr [rdx + 16] ; 00DE
    faddp st(1), st(0) ; 00E1
    fstp qword ptr [rcx + 80] ; 00E3
    fnop ; 00E6
    fld qword ptr [rcx + 8] ; 00E8
    fmul qword ptr [rdx + 40] ; 00EA
    fld qword ptr [rdx + 56] ; 00EC
    fmul qword ptr [rdx + 16] ; 00EF
    faddp st(1), st(0) ; 00F2
    fstp qword ptr [rcx + 88] ; 00F4
    fnop ; 00F7
    fld qword ptr [rcx + 16] ; 00F9
    fmul qword ptr [rdx + 40] ; 00FB
    fmul qword ptr [rdx + 0] ; 00FD
    fld qword ptr [rdx + 64] ; 0100
    fmul qword ptr [rdx + 16] ; 0103
    faddp st(1), st(0) ; 0106
    nop ; 0108
    nop ; 0109
    nop ; 010A
    nop ; 010B
    nop ; 010C
    fstp qword ptr [rcx + 96] ; 010D
    fnop ; 0110
    fld qword ptr [rcx + 80] ; 0112
    fmul st(0), st(0) ; 0115
    fld qword ptr [rcx + 88] ; 0117
    fmul st(0), st(0) ; 011A
    fld qword ptr [rcx + 96] ; 011C
    fmul st(0), st(0) ; 011F
    faddp st(1), st(0) ; 0121
    faddp st(1), st(0) ; 0123
    fstp qword ptr [rcx + 32] ; 0125
    nop ; 0128
    fld qword ptr [rcx + 56] ; 0129
    fmul st(0), st(0) ; 012C
    fld qword ptr [rcx + 64] ; 012E
    fmul st(0), st(0) ; 0131
    faddp st(1), st(0) ; 0133
    fsqrt ; 0135
    fstp qword ptr [rcx + 40] ; 0137
    nop ; 013A
    fld qword ptr [rcx + 80] ; 013B
    fmul st(0), st(0) ; 013E
    fld qword ptr [rcx + 88] ; 0140
    fmul st(0), st(0) ; 0143
    faddp st(1), st(0) ; 0145
    fsqrt ; 0147
    fstp qword ptr [rcx + 48] ; 0149
    nop ; 014C
    fld qword ptr [rcx + 48] ; 014D
    fmul qword ptr [rcx + 72] ; 0150
    fld qword ptr [rcx + 40] ; 0153
    fmul qword ptr [rcx + 96] ; 0156
    fsubp st(1), st(0) ; 0159
    fdiv qword ptr [rcx + 32] ; 015B
    fmul qword ptr [rdx + 0] ; 015E
    fstp qword ptr [rcx + 16] ; 0161
    fnop ; 0163
    fld qword ptr [rcx + 72] ; 0165
    fmul qword ptr [rcx + 96] ; 0168
    fdiv qword ptr [rcx + 40] ; 016B
    fdiv qword ptr [rcx + 48] ; 016E
    fld1 ; 0171
    faddp st(1), st(0) ; 0173
    fdiv qword ptr [rcx + 32] ; 0175
    fstp qword ptr [rcx + 32] ; 0178
    nop ; 017B
    fld qword ptr [rcx + 56] ; 017C
    fmul qword ptr [rcx + 80] ; 017F
    fld qword ptr [rcx + 64] ; 0182
    fmul qword ptr [rcx + 88] ; 0185
    faddp st(1), st(0) ; 0188
    fmul qword ptr [rcx + 32] ; 018A
    fstp qword ptr [rcx] ; 018D
    fnop ; 018F
    fld qword ptr [rcx + 56] ; 0191
    fmul qword ptr [rcx + 88] ; 0194
    fld qword ptr [rcx + 64] ; 0197
    fmul qword ptr [rcx + 80] ; 019A
    fsubrp st(1), st(0) ; 019D
    fmul qword ptr [rcx + 32] ; 019F
    fstp qword ptr [rcx + 8] ; 01A2
    fnop ; 01A4
    fld qword ptr [rcx] ; 01A6
    fmul st(0), st(0) ; 01A8
    fld qword ptr [rcx + 8] ; 01AA
    fmul st(0), st(0) ; 01AC
    faddp st(1), st(0) ; 01AE
    fstp qword ptr [rcx + 40] ; 01B0
    fnop ; 01B3
    fld qword ptr [rcx + 16] ; 01B5
    fmul st(0), st(0) ; 01B7
    fdiv qword ptr [rcx + 40] ; 01B9
    fchs ; 01BC
    fld1 ; 01BE
    faddp st(1), st(0) ; 01C0
    fstp qword ptr [rcx + 48] ; 01C2
    nop ; 01C5
    fld qword ptr [rcx] ; 01C6
    fmul st(0), st(0) ; 01C8
    fld qword ptr [rcx + 8] ; 01CA
    fmul st(0), st(0) ; 01CC
    fsubp st(1), st(0) ; 01CE
    fmul qword ptr [rcx + 48] ; 01D0
    fnop ; 01D3
    fld qword ptr [rcx] ; 01D5
    fmul qword ptr [rcx + 8] ; 01D7
    fmul qword ptr [rdx + 40] ; 01D9
    fmul qword ptr [rcx + 48] ; 01DB
    fnop ; 01DE
    fld qword ptr [rcx + 40] ; 01E0
    fsqrt ; 01E3
    fmul qword ptr [rcx + 16] ; 01E5
    fmul qword ptr [rdx + 40] ; 01E7
    fmul qword ptr [rdx + 0] ; 01E9
    fnop ; 01EC
    fstp qword ptr [rcx + 16] ; 01EE
    fstp qword ptr [rcx + 8] ; 01F0
    fstp qword ptr [rcx] ; 01F2
    fnop ; 01F4
    fld qword ptr [rcx] ; 01F6
    fld1 ; 01F8
    fchs ; 01FA
    faddp st(1), st(0) ; 01FC
    fmul st(0), st(0) ; 01FE
    fld qword ptr [rcx + 8] ; 0200
    fmul st(0), st(0) ; 0202
    fld qword ptr [rcx + 16] ; 0204
    fmul st(0), st(0) ; 0206
    faddp st(1), st(0) ; 0208
    faddp st(1), st(0) ; 020A
    fld1 ; 020C
    fdivrp st(1), st(0) ; 020E
    fstp qword ptr [rcx + 24] ; 0210
Magnet4DOriginal_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
Magnet4DOriginal_x87 ENDP


; Original MandelView CODE: native x87 angles/power and internal C before Double stores.
MandelViewOriginal_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    fnop ; 0014
    fld qword ptr [rcx + 8] ; 0016
    fld qword ptr [rcx] ; 0018
    fpatan ; 001A
    fmul qword ptr [rdx] ; 001C
    fsincos ; 001F
    fstp qword ptr [rcx + 32] ; 0021
    fstp qword ptr [rcx + 24] ; 0024
    fld qword ptr [rcx + 16] ; 0027
    fld qword ptr [rcx + 8] ; 0029
    fpatan ; 002B
    fmul qword ptr [rdx] ; 002D
    fsincos ; 0030
    fstp qword ptr [rcx + 48] ; 0032
    fstp qword ptr [rcx + 40] ; 0035
    fld qword ptr [rcx + 16] ; 0038
    fld qword ptr [rcx] ; 003A
    fpatan ; 003C
    fmul qword ptr [rdx] ; 003E
    fsincos ; 0041
    fstp qword ptr [rcx + 64] ; 0043
    fstp qword ptr [rcx + 56] ; 0046
    nop ; 0049
    fld qword ptr [rcx] ; 004A
    fmul st(0), st(0) ; 004C
    fld qword ptr [rcx + 8] ; 004E
    fmul st(0), st(0) ; 0050
    fld qword ptr [rcx + 16] ; 0052
    fmul st(0), st(0) ; 0054
    faddp st(1), st(0) ; 0056
    faddp st(1), st(0) ; 0058
    fsqrt ; 005A
    fld qword ptr [rdx] ; 005C
    fxch st(1) ; 005F
    fabs ; 0061
    fyl2x ; 0063
    fld1 ; 0065
    fld st(1) ; 0067
    fprem ; 0069
    f2xm1 ; 006B
    faddp st(1), st(0) ; 006D
    fscale ; 006F
    fxch st(1) ; 0071
    fstp st(0) ; 0073
    fstp qword ptr [rcx + 72] ; 0075
    nop ; 0078
    nop ; 0079
    fld qword ptr [rcx + 32] ; 007A
    fmul qword ptr [rcx + 64] ; 007D
    fld qword ptr [rcx + 40] ; 0080
    fmul qword ptr [rcx + 56] ; 0083
    fmul qword ptr [rcx + 24] ; 0086
    fchs ; 0089
    faddp st(1), st(0) ; 008B
    fmul qword ptr [rcx + 72] ; 008D
    fadd qword ptr [rdx + 16] ; 0090
    fstp qword ptr [rcx] ; 0093
    fld qword ptr [rcx + 24] ; 0095
    fmul qword ptr [rcx + 48] ; 0098
    fmul qword ptr [rcx + 72] ; 009B
    fadd qword ptr [rdx + 24] ; 009E
    fstp qword ptr [rcx + 8] ; 00A1
    fld qword ptr [rcx + 40] ; 00A3
    fmul qword ptr [rcx + 64] ; 00A6
    fmul qword ptr [rcx + 24] ; 00A9
    fld qword ptr [rcx + 32] ; 00AC
    fmul qword ptr [rcx + 56] ; 00AF
    faddp st(1), st(0) ; 00B2
    fmul qword ptr [rcx + 72] ; 00B4
    fmul qword ptr [rdx + 8] ; 00B7
    fadd qword ptr [rdx + 32] ; 00BA
    fstp qword ptr [rcx + 16] ; 00BD
    fnop ; 00BF
MandelViewOriginal_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
MandelViewOriginal_x87 ENDP


; Original Riemann CODE: native x87 angles/power and internal C before Double stores.
RiemannOriginal_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    fld qword ptr [rcx] ; 000B
    fld qword ptr [rcx + 8] ; 000D
    fld qword ptr [rcx + 16] ; 000F
    fld qword ptr [rcx + 24] ; 0011
    fsqrt ; 0014
    fld1 ; 0016
    fdivrp st(1), st(0) ; 0018
    fmul st(3), st(0) ; 001A
    fmul st(2), st(0) ; 001C
    fmulp st(1), st(0) ; 001E
    fld1 ; 0020
    fsub st(2), st(0) ; 0022
    fdivr st(2), st(0) ; 0024
    fxch st(2) ; 0026
    fmul st(3), st(0) ; 0028
    fmulp st(1), st(0) ; 002A
    fld st(2) ; 002C
    fmul st(0), st(3) ; 002E
    fsub st(0), st(2) ; 0030
    fxch st(1) ; 0032
    fxch st(3) ; 0034
    fadd st(0), st(0) ; 0036
    fxch st(1) ; 0038
    fpatan ; 003A
    fld st(2) ; 003C
    fmul st(0), st(3) ; 003E
    fsub st(0), st(2) ; 0040
    fxch st(1) ; 0042
    fxch st(3) ; 0044
    fadd st(0), st(0) ; 0046
    fxch st(1) ; 0048
    fpatan ; 004A
    fld qword ptr [rdx] ; 004C
    fmul st(3), st(0) ; 004F
    fmulp st(1), st(0) ; 0051
    fsincos ; 0053
    fadd st(0), st(2) ; 0055
    fdivp st(1), st(0) ; 0057
    fxch st(2) ; 0059
    fsincos ; 005B
    fadd st(0), st(2) ; 005D
    fdivp st(1), st(0) ; 005F
    fld st(0) ; 0061
    fmul st(0), st(1) ; 0063
    fld st(3) ; 0065
    fmul st(0), st(4) ; 0067
    faddp st(1), st(0) ; 0069
    fld st(0) ; 006B
    fadd st(0), st(3) ; 006D
    fdivr st(0), st(3) ; 006F
    fxch st(1) ; 0071
    fsubrp st(3), st(0) ; 0073
    fmul st(3), st(0) ; 0075
    fmul st(2), st(0) ; 0077
    fmulp st(1), st(0) ; 0079
    fld qword ptr [rdx] ; 007B
    fmul qword ptr [rdx + 8] ; 007E
    fld qword ptr [rcx + 24] ; 0081
    fldln2 ; 0084
    fxch st(1) ; 0086
    fyl2x ; 0088
    fxch st(1) ; 008A
    fmulp st(1), st(0) ; 008C
    fldl2e ; 008E
    fmulp st(1), st(0) ; 0090
    fld st(0) ; 0092
    frndint ; 0094
    fsub st(1), st(0) ; 0096
    fxch st(1) ; 0098
    f2xm1 ; 009A
    fld1 ; 009C
    faddp st(1), st(0) ; 009E
    fscale ; 00A0
    fstp st(1) ; 00A2
    fmul st(3), st(0) ; 00A4
    fmul st(2), st(0) ; 00A6
    fmulp st(1), st(0) ; 00A8
    fadd st(0), st(0) ; 00AA
    fadd qword ptr [rdx + 16] ; 00AC
    fstp qword ptr [rcx] ; 00AF
    fadd qword ptr [rdx + 24] ; 00B1
    fstp qword ptr [rcx + 8] ; 00B4
    fadd st(0), st(0) ; 00B6
    fadd qword ptr [rdx + 32] ; 00B8
    fstp qword ptr [rcx + 16] ; 00BB
RiemannOriginal_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
RiemannOriginal_x87 ENDP


; Original HeightMap2IFS CODE before callback. state: XYZ,DE,Rold,trap,RGB/scratch,extra,center.
HeightMap2IFSPrepare_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    fld qword ptr [rcx + 16] ; 0004
    fld qword ptr [rcx + 8] ; 0007
    fld qword ptr [rcx] ; 000A
    fld st(0) ; 000D
    fmul st(0), st(0) ; 000F
    fld st(2) ; 0011
    fmul st(0), st(0) ; 0013
    faddp st(1), st(0) ; 0015
    fld st(3) ; 0017
    fmul st(0), st(0) ; 0019
    faddp st(1), st(0) ; 001B
    fld1 ; 001D
    fdivrp st(1), st(0) ; 001F
    fcom qword ptr [rdx + 168] ; 0021
    nop ; 0023
    fnstsw ax ; 0024
    and ah, 41h ; 0026
    jbe heightmap2ifs_prep_003a ; 0029
    fstp st(0) ; 002B
    and r8d, 1
    shl r8d, 3
    fld qword ptr [rdx + r8 + 168] ; 0036
    nop ; 0039
heightmap2ifs_prep_003a:
    fmul st(1), st(0) ; 003A
    fmul st(2), st(0) ; 003C
    fmul st(3), st(0) ; 003E
    fstp qword ptr [rcx + 24] ; 0040
    fld st(0) ; 0043
    fmul qword ptr [rdx + 104] ; 0045
    fld st(2) ; 0048
    fmul qword ptr [rdx + 112] ; 004A
    faddp st(1), st(0) ; 004D
    fld st(3) ; 004F
    fmul qword ptr [rdx + 120] ; 0051
    faddp st(1), st(0) ; 0054
    fld st(1) ; 0056
    fmul qword ptr [rdx + 56] ; 0058
    fld st(3) ; 005B
    fmul qword ptr [rdx + 64] ; 005D
    faddp st(1), st(0) ; 0060
    fld st(4) ; 0062
    fmul qword ptr [rdx + 72] ; 0064
    faddp st(1), st(0) ; 0067
    fxch st(2) ; 0069
    fmul qword ptr [rdx + 80] ; 006B
    fxch st(3) ; 006E
    fmul qword ptr [rdx + 88] ; 0070
    faddp st(3), st(0) ; 0073
    fxch st(3) ; 0075
    fmul qword ptr [rdx + 96] ; 0077
    faddp st(2), st(0) ; 007A
    fld st(2) ; 007C
    fadd qword ptr [rdx + 24] ; 007E
    fld qword ptr [rdx + 48] ; 0081
    fmul qword ptr [rcx + 80] ; 0084
    fsubp st(1), st(0) ; 0087
    fabs ; 0089
    fld qword ptr [rdx + 48] ; 008B
    fabs ; 008E
    fadd qword ptr [rcx + 32] ; 0090
    fcompp ; 0096
    fnstsw ax ; 0098
    shr ah, 1 ; 009A
    jae heightmap2ifs_prep_00b6 ; 009C
    fcompp ; 009E
    fld qword ptr [rcx + 80] ; 00A0
    fmul qword ptr [rdx + 48] ; 00A3
    fsub qword ptr [rdx + 24] ; 00A6
    fsubrp st(1), st(0) ; 00A9
    fdiv qword ptr [rcx + 24] ; 00AB
    fstp qword ptr [rcx + 24] ; 00AE
    xor eax, eax
    jmp HeightMap2IFSPrepare_x87_done
heightmap2ifs_prep_00b6:
    fmul qword ptr [rdx + 32] ; 00B6
    fsub qword ptr [rdx + 8] ; 00B9
    fstp qword ptr [rcx + 48] ; 00BC
    fmul qword ptr [rdx + 40] ; 00BF
    fsub qword ptr [rdx + 16] ; 00C2
    fstp qword ptr [rcx + 56] ; 00C5
    fstp qword ptr [rcx + 72] ; 00C9
    mov eax, 1
HeightMap2IFSPrepare_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
HeightMap2IFSPrepare_x87 ENDP


; Original HeightMap2IFS CODE after callback; masked channels passed in r8d/r9d.
HeightMap2IFSFinish_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    fld qword ptr [rcx + r8*8 + 48] ; 00E0
    fmul qword ptr [rdx + 48] ; 00E3
    fsub qword ptr [rdx + 24] ; 00E6
    fsub qword ptr [rcx + 72] ; 00E9
    fdiv qword ptr [rcx + 24] ; 00ED
    fstp qword ptr [rcx + 24] ; 00F0
    mov r8d, r9d
    fld qword ptr [rcx + r8*8 + 48] ; 00F9
    fadd qword ptr [rdx + 144] ; 00FC
    fmul qword ptr [rdx + 152] ; 00FF
    fstp qword ptr [rcx + 40] ; 0102
HeightMap2IFSFinish_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
HeightMap2IFSFinish_x87 ENDP


; Original SphereHeightMap CODE before callback. state: XYZ,DE,Rold,trap,RGB/scratch,extra,center.
SphereHeightMapPrepare_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    fld qword ptr [rcx + 16] ; 0004
    fsub qword ptr [rdx + 32] ; 0007
    fld qword ptr [rcx + 8] ; 000A
    fsub qword ptr [rdx + 24] ; 000D
    fld qword ptr [rcx] ; 0010
    fsub qword ptr [rdx + 16] ; 0013
    fld st(0) ; 0016
    fmul qword ptr [rdx + 112] ; 0018
    fld st(2) ; 001B
    fmul qword ptr [rdx + 120] ; 001D
    faddp st(1), st(0) ; 0020
    fld st(3) ; 0022
    fmul qword ptr [rdx + 128] ; 0024
    faddp st(1), st(0) ; 0027
    fld st(1) ; 0029
    fmul qword ptr [rdx + 64] ; 002B
    fld st(3) ; 002E
    fmul qword ptr [rdx + 72] ; 0030
    faddp st(1), st(0) ; 0033
    fld st(4) ; 0035
    fmul qword ptr [rdx + 80] ; 0037
    faddp st(1), st(0) ; 003A
    fmul qword ptr [rdx + 40] ; 003C
    fxch st(2) ; 003F
    fmul qword ptr [rdx + 88] ; 0041
    fxch st(3) ; 0044
    fmul qword ptr [rdx + 96] ; 0046
    faddp st(3), st(0) ; 0049
    fxch st(3) ; 004B
    fmul qword ptr [rdx + 104] ; 004D
    faddp st(2), st(0) ; 0050
    fxch st(1) ; 0052
    fmul qword ptr [rdx + 48] ; 0054
    fxch st(1) ; 0057
    fld st(2) ; 0059
    fmul st(0), st(0) ; 005B
    fld st(2) ; 005D
    fmul st(0), st(0) ; 005F
    faddp st(1), st(0) ; 0061
    fld st(1) ; 0063
    fmul st(0), st(0) ; 0065
    faddp st(1), st(0) ; 0067
    fsqrt ; 0069
    fld1 ; 006B
    fsubp st(1), st(0) ; 006D
    fld st(0) ; 006F
    fabs ; 0071
    fld qword ptr [rdx + 56] ; 0073
    fabs ; 0076
    fadd qword ptr [rcx + 32] ; 0078
    fcompp ; 007E
    fnstsw ax ; 0080
    shr ah, 1 ; 0082
    jae sphereheightmap_prep_0092 ; 0084
    fstp qword ptr [rcx + 24] ; 0086
    fcompp ; 0089
    fstp st(0) ; 008B
    xor eax, eax
    jmp SphereHeightMapPrepare_x87_done
sphereheightmap_prep_0092:
    fstp qword ptr [rcx + 72] ; 0092
    fstp qword ptr [rcx + 48] ; 0096
    fstp qword ptr [rcx + 56] ; 0099
    fstp qword ptr [rcx + 64] ; 009D
    mov eax, 1
SphereHeightMapPrepare_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
SphereHeightMapPrepare_x87 ENDP


; Original SphereHeightMap CODE after callback; masked channels passed in r8d/r9d.
SphereHeightMapFinish_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    fld qword ptr [rcx + r8*8 + 48] ; 00B4
    fsub qword ptr [rcx + 80] ; 00B7
    fmul qword ptr [rdx + 56] ; 00BA
    fsubr qword ptr [rcx + 72] ; 00BD
    fstp qword ptr [rcx + 24] ; 00C1
    mov r8d, r9d
    fld qword ptr [rcx + r8*8 + 48] ; 00CA
    fadd qword ptr [rdx + 144] ; 00CD
    fmul qword ptr [rdx + 152] ; 00D0
    fstp qword ptr [rcx + 40] ; 00D3
SphereHeightMapFinish_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
SphereHeightMapFinish_x87 ENDP


; Original SphereHMVoid CODE before callback. state: XYZ,DE,Rold,trap,RGB/scratch,extra,center.
SphereHMVoidPrepare_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    fld qword ptr [rcx + 16] ; 0004
    fsub qword ptr [rdx + 32] ; 0007
    fld qword ptr [rcx + 8] ; 000A
    fsub qword ptr [rdx + 24] ; 000D
    fld qword ptr [rcx] ; 0010
    fsub qword ptr [rdx + 16] ; 0013
    fld st(0) ; 0016
    fmul qword ptr [rdx + 112] ; 0018
    fld st(2) ; 001B
    fmul qword ptr [rdx + 120] ; 001D
    faddp st(1), st(0) ; 0020
    fld st(3) ; 0022
    fmul qword ptr [rdx + 128] ; 0024
    faddp st(1), st(0) ; 0027
    fld st(1) ; 0029
    fmul qword ptr [rdx + 64] ; 002B
    fld st(3) ; 002E
    fmul qword ptr [rdx + 72] ; 0030
    faddp st(1), st(0) ; 0033
    fld st(4) ; 0035
    fmul qword ptr [rdx + 80] ; 0037
    faddp st(1), st(0) ; 003A
    fmul qword ptr [rdx + 40] ; 003C
    fxch st(2) ; 003F
    fmul qword ptr [rdx + 88] ; 0041
    fxch st(3) ; 0044
    fmul qword ptr [rdx + 96] ; 0046
    faddp st(3), st(0) ; 0049
    fxch st(3) ; 004B
    fmul qword ptr [rdx + 104] ; 004D
    faddp st(2), st(0) ; 0050
    fxch st(1) ; 0052
    fmul qword ptr [rdx + 48] ; 0054
    fxch st(1) ; 0057
    fld st(2) ; 0059
    fmul st(0), st(0) ; 005B
    fld st(2) ; 005D
    fmul st(0), st(0) ; 005F
    faddp st(1), st(0) ; 0061
    fld st(1) ; 0063
    fmul st(0), st(0) ; 0065
    faddp st(1), st(0) ; 0067
    fsqrt ; 0069
    fld1 ; 006B
    fsubp st(1), st(0) ; 006D
    fld st(0) ; 006F
    fabs ; 0071
    fld qword ptr [rdx + 56] ; 0073
    fabs ; 0076
    fadd qword ptr [rcx + 32] ; 0078
    fadd qword ptr [rdx + 176] ; 007E
    fcompp ; 0081
    fnstsw ax ; 0083
    shr ah, 1 ; 0085
    jae spherehmvoid_prep_009b ; 0087
    fabs ; 0089
    fstp qword ptr [rcx + 24] ; 008B
    fstp st(0) ; 008E
    fstp st(0) ; 0090
    fstp st(0) ; 0092
    nop ; 0094
    nop ; 0095
    xor eax, eax
    jmp SphereHMVoidPrepare_x87_done
spherehmvoid_prep_009b:
    fstp qword ptr [rcx + 72] ; 009B
    fstp qword ptr [rcx + 48] ; 009F
    fstp qword ptr [rcx + 56] ; 00A2
    fstp qword ptr [rcx + 64] ; 00A6
    fld qword ptr [rcx + 48] ; 00AA
    fabs ; 00AD
    fld qword ptr [rcx + 56] ; 00AF
    fabs ; 00B3
    fcom st(1) ; 00B5
    fnstsw ax ; 00B7
    shr ah, 1 ; 00B9
    jb spherehmvoid_prep_00c1 ; 00BB
    fstp st(1) ; 00BD
    jmp spherehmvoid_prep_00c3 ; 00BF
spherehmvoid_prep_00c1:
    fstp st(0) ; 00C1
spherehmvoid_prep_00c3:
    fld qword ptr [rcx + 64] ; 00C3
    fabs ; 00C7
    fcom st(1) ; 00C9
    fnstsw ax ; 00CB
    shr ah, 1 ; 00CD
    jb spherehmvoid_prep_00d5 ; 00CF
    fstp st(1) ; 00D1
    jmp spherehmvoid_prep_00d7 ; 00D3
spherehmvoid_prep_00d5:
    fstp st(0) ; 00D5
spherehmvoid_prep_00d7:
    fld1 ; 00D7
    fsubp st(1), st(0) ; 00D9
    fmul qword ptr [rdx + 160] ; 00DB
    fld1 ; 00DE
    fsub qword ptr [rdx + 160] ; 00E0
    fmul qword ptr [rcx + 72] ; 00E3
    nop ; 00E7
    nop ; 00E8
    nop ; 00E9
    nop ; 00EA
    nop ; 00EB
    faddp st(1), st(0) ; 00EC
    fstp qword ptr [rcx + 72] ; 00EE
    nop ; 00F2
    nop ; 00F3
    nop ; 00F4
    nop ; 00F5
    nop ; 00F6
    nop ; 00F7
    nop ; 00F8
    mov eax, 1
SphereHMVoidPrepare_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
SphereHMVoidPrepare_x87 ENDP


; Original SphereHMVoid CODE after callback; masked channels passed in r8d/r9d.
SphereHMVoidFinish_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    fld qword ptr [rcx + r8*8 + 48] ; 010C
    fsub qword ptr [rcx + 80] ; 010F
    fmul qword ptr [rdx + 56] ; 0112
    fsubr qword ptr [rcx + 72] ; 0115
    fabs ; 0119
    fsub qword ptr [rdx + 168] ; 011B
    nop ; 011D
    fstp qword ptr [rcx + 24] ; 011E
    mov r8d, r9d
    fld qword ptr [rcx + r8*8 + 48] ; 0127
    fadd qword ptr [rdx + 144] ; 012A
    fmul qword ptr [rdx + 152] ; 012D
    fstp qword ptr [rcx + 40] ; 0130
SphereHMVoidFinish_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
SphereHMVoidFinish_x87 ENDP


; Original Benesi1Pow2 CODE; XYZ(W) and Double spills, branch ordering and internal Julia C.
Benesi1Pow2Original_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    fld qword ptr [rcx + 16] ; 0004
    fld qword ptr [rcx + 8] ; 0006
    fld qword ptr [rcx] ; 0008
    fld st(1) ; 000D
    fmul st(0), st(2) ; 000F
    fld st(3) ; 0011
    fmul st(0), st(4) ; 0013
    fld st(0) ; 0015
    fadd st(0), st(2) ; 0017
    fld st(0) ; 0019
    fsqrt ; 001B
    fmul st(0), st(4) ; 001D
    fadd st(0), st(0) ; 001F
    fdiv st(0), st(1) ; 0021
    fmul st(5), st(0) ; 0023
    fxch st(3) ; 0025
    fsubrp st(2), st(0) ; 0027
    fxch st(3) ; 0029
    fmul st(0), st(0) ; 002B
    fsubrp st(3), st(0) ; 002D
    fmulp st(1), st(0) ; 002F
    fabs ; 0031
    fld qword ptr [rdx + 8] ; 0033
    fabs ; 0036
    fsubrp st(1), st(0) ; 0038
    fstp qword ptr [rcx + 8] ; 003A
    fadd qword ptr [rdx + 0] ; 003C
    fstp qword ptr [rcx] ; 003F
    fmulp st(1), st(0) ; 0041
    fadd st(0), st(0) ; 0043
    fabs ; 0045
    fld qword ptr [rdx + 16] ; 0047
    fabs ; 004A
    fsubrp st(1), st(0) ; 004C
    fstp qword ptr [rcx + 16] ; 004E
Benesi1Pow2Original_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
Benesi1Pow2Original_x87 ENDP


; Original MsltoeFoldQuat CODE; XYZ(W) and Double spills, branch ordering and internal Julia C.
MsltoeFoldQuatOriginal_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    fld qword ptr [rcx + 16] ; 000A
    fld qword ptr [rcx + 8] ; 000C
    fld qword ptr [rcx] ; 000E
    fld st(2) ; 0010
    fmul st(0), st(3) ; 0012
    fld st(2) ; 0014
    fmul st(0), st(3) ; 0016
    fld st(2) ; 0018
    fmul st(0), st(3) ; 001A
    fcom st(2) ; 001C
    fnstsw ax ; 001E
    and ah, 041h ; 0020
    je msltoefoldquat_002d ; 0023
    fxch st(2) ; 0025
    fxch st(3) ; 0027
    fxch st(5) ; 0029
    fxch st(3) ; 002B
msltoefoldquat_002d:
    fcom st(1) ; 002D
    fnstsw ax ; 002F
    and ah, 041h ; 0031
    je msltoefoldquat_003e ; 0034
    fxch st(1) ; 0036
    fxch st(3) ; 0038
    fxch st(4) ; 003A
    fxch st(3) ; 003C
msltoefoldquat_003e:
    fxch st(1) ; 003E
    fcom st(2) ; 0040
    fnstsw ax ; 0042
    and ah, 041h ; 0044
    je msltoefoldquat_0051 ; 0047
    fxch st(2) ; 0049
    fxch st(4) ; 004B
    fxch st(5) ; 004D
    fxch st(4) ; 004F
msltoefoldquat_0051:
    fxch st(1) ; 0051
    fsubrp st(1), st(0) ; 0053
    fsubrp st(1), st(0) ; 0055
    fadd qword ptr [rdx + 0] ; 0057
    fstp qword ptr [rcx] ; 005A
    fmul st(1), st(0) ; 005C
    fmulp st(2), st(0) ; 005E
    fadd st(0), st(0) ; 0060
    fadd qword ptr [rdx + 8] ; 0062
    fstp qword ptr [rcx + 8] ; 0065
    fadd st(0), st(0) ; 0067
    fadd qword ptr [rdx + 16] ; 0069
    fstp qword ptr [rcx + 16] ; 006C
MsltoeFoldQuatOriginal_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
MsltoeFoldQuatOriginal_x87 ENDP


; Original Barnsley4Dc CODE; XYZ(W) and Double spills, branch ordering and internal Julia C.
Barnsley4DcOriginal_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    cmp dword ptr [r8 + 8], 0 ; 0014
    je barnsley4dc_00a1 ; 0018
    fld qword ptr [rcx] ; 001E
    fld qword ptr [rdx + 0] ; 0020
    fmul st(0), st(0) ; 0023
    fld qword ptr [rdx + 8] ; 0025
    fmul st(0), st(0) ; 0028
    faddp st(1), st(0) ; 002A
    fld qword ptr [rdx + 16] ; 002C
    fmul st(0), st(0) ; 002F
    faddp st(1), st(0) ; 0031
    fld qword ptr [rdx + 24] ; 0033
    fmul st(0), st(0) ; 0036
    faddp st(1), st(0) ; 0038
    fsqrt ; 003A
    fmul qword ptr [rdx + 72] ; 003C
    fcompp ; 003E
    fnstsw ax ; 0040
    and ah, 041h ; 0042
    ja barnsley4dc_006d ; 0045
    fld qword ptr [rcx] ; 0047
    fmul qword ptr [rdx + 80] ; 0049
    fstp qword ptr [rcx] ; 004C
    fld qword ptr [rcx + 8] ; 004E
    fmul qword ptr [rdx + 80] ; 0050
    fstp qword ptr [rcx + 8] ; 0053
    fld qword ptr [rcx + 16] ; 0055
    fmul qword ptr [rdx + 80] ; 0057
    fstp qword ptr [rcx + 16] ; 005A
    fld qword ptr [rcx + 24] ; 005C
    fmul qword ptr [rdx + 80] ; 005F
    fadd qword ptr [rdx + 32] ; 0062
    fstp qword ptr [rcx + 24] ; 0065
    jmp barnsley4dc_0120 ; 0068
barnsley4dc_006d:
    fld qword ptr [rdx + 0] ; 006D
    fsub qword ptr [rcx] ; 0070
    fmul qword ptr [rdx + 80] ; 0072
    fstp qword ptr [rcx] ; 0075
    fld qword ptr [rdx + 8] ; 0077
    fsub qword ptr [rcx + 8] ; 007A
    fmul qword ptr [rdx + 80] ; 007C
    fstp qword ptr [rcx + 8] ; 007F
    fld qword ptr [rdx + 16] ; 0081
    fsub qword ptr [rcx + 16] ; 0084
    fmul qword ptr [rdx + 80] ; 0086
    fstp qword ptr [rcx + 16] ; 0089
    fld qword ptr [rdx + 24] ; 008B
    fsub qword ptr [rcx + 24] ; 008E
    fadd qword ptr [rdx + 32] ; 0091
    fmul qword ptr [rdx + 80] ; 0094
    fstp qword ptr [rcx + 24] ; 0097
    nop ; 009A
    jmp barnsley4dc_00f3 ; 009B
    nop ; 009D
    nop ; 009E
    nop ; 009F
    nop ; 00A0
barnsley4dc_00a1:
    fld qword ptr [rcx] ; 00A1
    fld qword ptr [rdx + 40] ; 00A3
    fcompp ; 00A6
    fnstsw ax ; 00A8
    and ah, 041h ; 00AA
    ja barnsley4dc_00d2 ; 00AD
    fld qword ptr [rcx] ; 00AF
    fadd qword ptr [rdx + 0] ; 00B1
    fstp qword ptr [rcx] ; 00B4
    fld qword ptr [rcx + 8] ; 00B6
    fadd qword ptr [rdx + 8] ; 00B8
    fstp qword ptr [rcx + 8] ; 00BB
    fld qword ptr [rcx + 16] ; 00BD
    fadd qword ptr [rdx + 16] ; 00BF
    fstp qword ptr [rcx + 16] ; 00C2
    fld qword ptr [rcx + 24] ; 00C4
    fadd qword ptr [rdx + 24] ; 00C7
    fadd qword ptr [rdx + 32] ; 00CA
    fstp qword ptr [rcx + 24] ; 00CD
    jmp barnsley4dc_0120 ; 00D0
barnsley4dc_00d2:
    fld qword ptr [rcx] ; 00D2
    fsub qword ptr [rdx + 0] ; 00D4
    fstp qword ptr [rcx] ; 00D7
    fld qword ptr [rcx + 8] ; 00D9
    fsub qword ptr [rdx + 8] ; 00DB
    fstp qword ptr [rcx + 8] ; 00DE
    fld qword ptr [rcx + 16] ; 00E0
    fsub qword ptr [rdx + 16] ; 00E2
    fstp qword ptr [rcx + 16] ; 00E5
    fld qword ptr [rcx + 24] ; 00E7
    fsub qword ptr [rdx + 24] ; 00EA
    fadd qword ptr [rdx + 32] ; 00ED
    fstp qword ptr [rcx + 24] ; 00F0
barnsley4dc_00f3:
    fld qword ptr [rdx + 96] ; 00F3
    fstp qword ptr [rcx + 32] ; 00F6
    cmp dword ptr [r8], 0 ; 00F9
    je barnsley4dc_0108 ; 00FD
    fld qword ptr [rdx + 96] ; 00FF
    fchs ; 0102
    fstp qword ptr [rcx + 32] ; 0104
    nop ; 0107
barnsley4dc_0108:
    cmp dword ptr [r8 + 4], 0 ; 0108
    je barnsley4dc_0127 ; 010C
    fld qword ptr [rcx + 32] ; 010E
    fstp qword ptr [rcx + 40] ; 0111
    fld qword ptr [rdx + 88] ; 0114
    fstp qword ptr [rcx + 32] ; 0117
    jmp barnsley4dc_012d ; 011A
    nop ; 011C
    nop ; 011D
    nop ; 011E
    nop ; 011F
barnsley4dc_0120:
    fld qword ptr [rdx + 96] ; 0120
    fstp qword ptr [rcx + 32] ; 0123
    nop ; 0126
barnsley4dc_0127:
    fld qword ptr [rdx + 88] ; 0127
    fstp qword ptr [rcx + 40] ; 012A
barnsley4dc_012d:
    fld qword ptr [rcx] ; 012D
    nop ; 012F
    fmul qword ptr [rcx + 40] ; 0130
    nop ; 0133
    nop ; 0134
    fld qword ptr [rcx + 8] ; 0135
    nop ; 0137
    fmul qword ptr [rcx + 32] ; 0138
    fchs ; 013B
    faddp st(1), st(0) ; 013D
    fld qword ptr [rcx + 16] ; 013F
    nop ; 0141
    fmul qword ptr [rdx + 104] ; 0142
    fchs ; 0145
    faddp st(1), st(0) ; 0147
    fld qword ptr [rcx + 24] ; 0149
    fmul qword ptr [rdx + 112] ; 014C
    nop ; 014F
    nop ; 0150
    faddp st(1), st(0) ; 0151
    fnop ; 0153
    fld qword ptr [rcx + 8] ; 0155
    nop ; 0157
    fmul qword ptr [rcx + 40] ; 0158
    nop ; 015B
    nop ; 015C
    fld qword ptr [rcx] ; 015D
    nop ; 015F
    fmul qword ptr [rcx + 32] ; 0160
    nop ; 0163
    nop ; 0164
    faddp st(1), st(0) ; 0165
    fld qword ptr [rcx + 24] ; 0167
    fmul qword ptr [rdx + 104] ; 016A
    fchs ; 016D
    faddp st(1), st(0) ; 016F
    fld qword ptr [rcx + 16] ; 0171
    nop ; 0173
    fmul qword ptr [rdx + 112] ; 0174
    fchs ; 0177
    faddp st(1), st(0) ; 0179
    fnop ; 017B
    fld qword ptr [rcx + 16] ; 017D
    nop ; 017F
    fmul qword ptr [rcx + 40] ; 0180
    nop ; 0183
    nop ; 0184
    fld qword ptr [rcx + 24] ; 0185
    fmul qword ptr [rcx + 32] ; 0188
    fchs ; 018B
    faddp st(1), st(0) ; 018D
    fld qword ptr [rcx] ; 018F
    nop ; 0191
    fmul qword ptr [rdx + 104] ; 0192
    nop ; 0195
    nop ; 0196
    faddp st(1), st(0) ; 0197
    fld qword ptr [rcx + 8] ; 0199
    nop ; 019B
    fmul qword ptr [rdx + 112] ; 019C
    fchs ; 019F
    faddp st(1), st(0) ; 01A1
    fnop ; 01A3
    fld qword ptr [rcx + 24] ; 01A5
    fmul qword ptr [rcx + 40] ; 01A8
    nop ; 01AB
    nop ; 01AC
    fld qword ptr [rcx + 16] ; 01AD
    nop ; 01AF
    fmul qword ptr [rcx + 32] ; 01B0
    nop ; 01B3
    nop ; 01B4
    faddp st(1), st(0) ; 01B5
    fld qword ptr [rcx + 8] ; 01B7
    nop ; 01B9
    fmul qword ptr [rdx + 104] ; 01BA
    nop ; 01BD
    nop ; 01BE
    faddp st(1), st(0) ; 01BF
    fld qword ptr [rcx] ; 01C1
    nop ; 01C3
    fmul qword ptr [rdx + 112] ; 01C4
    nop ; 01C7
    nop ; 01C8
    faddp st(1), st(0) ; 01C9
    fnop ; 01CB
    nop ; 01CD
    fstp qword ptr [rcx + 24] ; 01CE
    fstp qword ptr [rcx + 16] ; 01D1
    fstp qword ptr [rcx + 8] ; 01D3
    fstp qword ptr [rcx] ; 01D5
    fnop ; 01D7
Barnsley4DcOriginal_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
Barnsley4DcOriginal_x87 ENDP


; Original Barnsley4Dnc CODE; XYZ(W) and Double spills, branch ordering and internal Julia C.
Barnsley4DncOriginal_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    cmp dword ptr [r8 + 8], 0 ; 0014
    je barnsley4dnc_00a1 ; 0018
    fld qword ptr [rcx] ; 001E
    fld qword ptr [rdx + 0] ; 0020
    fmul st(0), st(0) ; 0023
    fld qword ptr [rdx + 8] ; 0025
    fmul st(0), st(0) ; 0028
    faddp st(1), st(0) ; 002A
    fld qword ptr [rdx + 16] ; 002C
    fmul st(0), st(0) ; 002F
    faddp st(1), st(0) ; 0031
    fld qword ptr [rdx + 24] ; 0033
    fmul st(0), st(0) ; 0036
    faddp st(1), st(0) ; 0038
    fsqrt ; 003A
    fmul qword ptr [rdx + 72] ; 003C
    fcompp ; 003E
    fnstsw ax ; 0040
    and ah, 041h ; 0042
    ja barnsley4dnc_006d ; 0045
    fld qword ptr [rcx] ; 0047
    fmul qword ptr [rdx + 80] ; 0049
    fstp qword ptr [rcx] ; 004C
    fld qword ptr [rcx + 8] ; 004E
    fmul qword ptr [rdx + 80] ; 0050
    fstp qword ptr [rcx + 8] ; 0053
    fld qword ptr [rcx + 16] ; 0055
    fmul qword ptr [rdx + 80] ; 0057
    fstp qword ptr [rcx + 16] ; 005A
    fld qword ptr [rcx + 24] ; 005C
    fmul qword ptr [rdx + 80] ; 005F
    fadd qword ptr [rdx + 32] ; 0062
    fstp qword ptr [rcx + 24] ; 0065
    jmp barnsley4dnc_0120 ; 0068
barnsley4dnc_006d:
    fld qword ptr [rdx + 0] ; 006D
    fsub qword ptr [rcx] ; 0070
    fmul qword ptr [rdx + 80] ; 0072
    fstp qword ptr [rcx] ; 0075
    fld qword ptr [rdx + 8] ; 0077
    fsub qword ptr [rcx + 8] ; 007A
    fmul qword ptr [rdx + 80] ; 007C
    fstp qword ptr [rcx + 8] ; 007F
    fld qword ptr [rdx + 16] ; 0081
    fsub qword ptr [rcx + 16] ; 0084
    fmul qword ptr [rdx + 80] ; 0086
    fstp qword ptr [rcx + 16] ; 0089
    fld qword ptr [rdx + 24] ; 008B
    fsub qword ptr [rcx + 24] ; 008E
    fadd qword ptr [rdx + 32] ; 0091
    fmul qword ptr [rdx + 80] ; 0094
    fstp qword ptr [rcx + 24] ; 0097
    nop ; 009A
    jmp barnsley4dnc_00f3 ; 009B
    nop ; 009D
    nop ; 009E
    nop ; 009F
    nop ; 00A0
barnsley4dnc_00a1:
    fld qword ptr [rcx] ; 00A1
    fld qword ptr [rdx + 40] ; 00A3
    fcompp ; 00A6
    fnstsw ax ; 00A8
    and ah, 041h ; 00AA
    ja barnsley4dnc_00d2 ; 00AD
    fld qword ptr [rcx] ; 00AF
    fadd qword ptr [rdx + 0] ; 00B1
    fstp qword ptr [rcx] ; 00B4
    fld qword ptr [rcx + 8] ; 00B6
    fadd qword ptr [rdx + 8] ; 00B8
    fstp qword ptr [rcx + 8] ; 00BB
    fld qword ptr [rcx + 16] ; 00BD
    fadd qword ptr [rdx + 16] ; 00BF
    fstp qword ptr [rcx + 16] ; 00C2
    fld qword ptr [rcx + 24] ; 00C4
    fadd qword ptr [rdx + 24] ; 00C7
    fadd qword ptr [rdx + 32] ; 00CA
    fstp qword ptr [rcx + 24] ; 00CD
    jmp barnsley4dnc_0120 ; 00D0
barnsley4dnc_00d2:
    fld qword ptr [rcx] ; 00D2
    fsub qword ptr [rdx + 0] ; 00D4
    fstp qword ptr [rcx] ; 00D7
    fld qword ptr [rcx + 8] ; 00D9
    fsub qword ptr [rdx + 8] ; 00DB
    fstp qword ptr [rcx + 8] ; 00DE
    fld qword ptr [rcx + 16] ; 00E0
    fsub qword ptr [rdx + 16] ; 00E2
    fstp qword ptr [rcx + 16] ; 00E5
    fld qword ptr [rcx + 24] ; 00E7
    fsub qword ptr [rdx + 24] ; 00EA
    fadd qword ptr [rdx + 32] ; 00ED
    fstp qword ptr [rcx + 24] ; 00F0
barnsley4dnc_00f3:
    fld qword ptr [rdx + 96] ; 00F3
    fstp qword ptr [rcx + 32] ; 00F6
    cmp dword ptr [r8], 0 ; 00F9
    je barnsley4dnc_0108 ; 00FD
    fld qword ptr [rdx + 96] ; 00FF
    fchs ; 0102
    fstp qword ptr [rcx + 32] ; 0104
    nop ; 0107
barnsley4dnc_0108:
    cmp dword ptr [r8 + 4], 0 ; 0108
    je barnsley4dnc_0127 ; 010C
    fld qword ptr [rcx + 32] ; 010E
    fstp qword ptr [rcx + 40] ; 0111
    fld qword ptr [rdx + 88] ; 0114
    fstp qword ptr [rcx + 32] ; 0117
    jmp barnsley4dnc_012d ; 011A
    nop ; 011C
    nop ; 011D
    nop ; 011E
    nop ; 011F
barnsley4dnc_0120:
    fld qword ptr [rdx + 96] ; 0120
    fstp qword ptr [rcx + 32] ; 0123
    nop ; 0126
barnsley4dnc_0127:
    fld qword ptr [rdx + 88] ; 0127
    fstp qword ptr [rcx + 40] ; 012A
barnsley4dnc_012d:
    fld qword ptr [rcx] ; 012D
    nop ; 012F
    fmul qword ptr [rcx + 40] ; 0130
    nop ; 0133
    nop ; 0134
    fld qword ptr [rcx + 8] ; 0135
    nop ; 0137
    fmul qword ptr [rcx + 32] ; 0138
    fchs ; 013B
    faddp st(1), st(0) ; 013D
    fld qword ptr [rcx + 16] ; 013F
    nop ; 0141
    fmul qword ptr [rdx + 104] ; 0142
    fchs ; 0145
    faddp st(1), st(0) ; 0147
    fld qword ptr [rcx + 24] ; 0149
    fmul qword ptr [rdx + 112] ; 014C
    fchs ; 014F
    faddp st(1), st(0) ; 0151
    fnop ; 0153
    fld qword ptr [rcx] ; 0155
    nop ; 0157
    fmul qword ptr [rcx + 32] ; 0158
    nop ; 015B
    nop ; 015C
    fld qword ptr [rcx + 8] ; 015D
    nop ; 015F
    fmul qword ptr [rcx + 40] ; 0160
    nop ; 0163
    nop ; 0164
    faddp st(1), st(0) ; 0165
    fld qword ptr [rcx + 16] ; 0167
    nop ; 0169
    fmul qword ptr [rdx + 112] ; 016A
    nop ; 016D
    nop ; 016E
    faddp st(1), st(0) ; 016F
    fld qword ptr [rcx + 24] ; 0171
    fmul qword ptr [rdx + 104] ; 0174
    fchs ; 0177
    faddp st(1), st(0) ; 0179
    fnop ; 017B
    fld qword ptr [rcx] ; 017D
    nop ; 017F
    fmul qword ptr [rdx + 104] ; 0180
    nop ; 0183
    nop ; 0184
    fld qword ptr [rcx + 8] ; 0185
    nop ; 0187
    fmul qword ptr [rdx + 112] ; 0188
    fchs ; 018B
    faddp st(1), st(0) ; 018D
    fld qword ptr [rcx + 16] ; 018F
    nop ; 0191
    fmul qword ptr [rcx + 40] ; 0192
    nop ; 0195
    nop ; 0196
    faddp st(1), st(0) ; 0197
    fld qword ptr [rcx + 24] ; 0199
    fmul qword ptr [rcx + 32] ; 019C
    nop ; 019F
    nop ; 01A0
    faddp st(1), st(0) ; 01A1
    fnop ; 01A3
    fld qword ptr [rcx] ; 01A5
    nop ; 01A7
    fmul qword ptr [rdx + 112] ; 01A8
    nop ; 01AB
    nop ; 01AC
    fld qword ptr [rcx + 8] ; 01AD
    nop ; 01AF
    fmul qword ptr [rdx + 104] ; 01B0
    nop ; 01B3
    nop ; 01B4
    faddp st(1), st(0) ; 01B5
    fld qword ptr [rcx + 16] ; 01B7
    nop ; 01B9
    fmul qword ptr [rcx + 32] ; 01BA
    fchs ; 01BD
    faddp st(1), st(0) ; 01BF
    fld qword ptr [rcx + 24] ; 01C1
    fmul qword ptr [rcx + 40] ; 01C4
    nop ; 01C7
    nop ; 01C8
    faddp st(1), st(0) ; 01C9
    fnop ; 01CB
    nop ; 01CD
    fstp qword ptr [rcx + 24] ; 01CE
    fstp qword ptr [rcx + 16] ; 01D1
    fstp qword ptr [rcx + 8] ; 01D3
    fstp qword ptr [rcx] ; 01D5
    fnop ; 01D7
Barnsley4DncOriginal_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
Barnsley4DncOriginal_x87 ENDP


; Original greekIFS CODE: rcx=XYZ,VaryScale,Rout; rdx=original PVar.
greekIFSOriginal_x87 PROC
    sub rsp, 104
    fnstcw word ptr [rsp + 80]
    mov word ptr [rsp + 82], 137Fh
    fldcw word ptr [rsp + 82]
    mov r8, rdx
    movupd xmm0, xmmword ptr [rcx] ; 0003
    movsd xmm1, qword ptr [rcx + 16] ; 0008
    mulpd xmm0, xmmword ptr [r8 - 020h] ; 000D
    mulsd xmm1, qword ptr [r8 - 020h] ; 0012
    addpd xmm0, xmmword ptr [r8 - 040h] ; 0017
    addsd xmm1, qword ptr [r8 - 030h] ; 001C
    cmp dword ptr [r8 - 04ch], 0 ; 0021
    je greekifs_apply_0040 ; 0025
    movsd xmm5, qword ptr [rcx + 24] ; 0027
    mulsd xmm5, qword ptr [r8 - 020h] ; 002C
    movupd xmmword ptr [rcx], xmm0 ; 0031
    movsd qword ptr [rcx + 16], xmm1 ; 0036
    movsd qword ptr [rcx + 24], xmm5 ; 003B
greekifs_apply_0040:
    movupd xmmword ptr [rsp], xmm0 ; 0040
    movsd qword ptr [rsp + 010h], xmm1 ; 0045
    add r8, -02ch
    fld qword ptr [rsp + 010h] ; 004E
    fld qword ptr [rsp + 8] ; 0052
    fld qword ptr [rsp] ; 0056
    fld st(0) ; 0059
    fmul dword ptr [r8 - 03ch] ; 005B
    fld st(2) ; 005E
    fmul dword ptr [r8 - 040h] ; 0060
    faddp st(1), st(0) ; 0063
    fld st(3) ; 0065
    fmul dword ptr [r8 - 044h] ; 0067
    faddp st(1), st(0) ; 006A
    fld st(1) ; 006C
    fmul dword ptr [r8 - 024h] ; 006E
    fld st(3) ; 0071
    fmul dword ptr [r8 - 028h] ; 0073
    faddp st(1), st(0) ; 0076
    fld st(4) ; 0078
    fmul dword ptr [r8 - 02ch] ; 007A
    faddp st(1), st(0) ; 007D
    fxch st(2) ; 007F
    fmul dword ptr [r8 - 030h] ; 0081
    fxch st(3) ; 0084
    fmul dword ptr [r8 - 034h] ; 0086
    faddp st(3), st(0) ; 0089
    fxch st(3) ; 008B
    fmul dword ptr [r8 - 038h] ; 008D
    faddp st(2), st(0) ; 0090
    fstp qword ptr [rsp] ; 0092
    fstp qword ptr [rsp + 8] ; 0095
    fstp qword ptr [rsp + 010h] ; 0099
    add r8, 02ch
    mov dl, byte ptr [rsp + 7] ; 00A0
    fld qword ptr [rsp] ; 00A4
    nop ; 00A7
    fabs ; 00A8
    fld qword ptr [rsp + 8] ; 00AA
    cmp dword ptr [r8 - 048h], 0 ; 00AE
    jne greekifs_apply_00b7 ; 00B2
    xor dl, 080h ; 00B4
greekifs_apply_00b7:
    and dl, 080h ; 00B7
    jne greekifs_apply_00c0 ; 00BA
    fadd qword ptr [r8] ; 00BC
    jmp greekifs_apply_00c2 ; 00BE
greekifs_apply_00c0:
    fsub qword ptr [r8] ; 00C0
greekifs_apply_00c2:
    fabs ; 00C2
    fcom st(1) ; 00C4
    wait ; 00C6
    fnstsw ax ; 00C7
    shr ah, 1 ; 00C9
    jb greekifs_apply_00d1 ; 00CB
    fstp st(1) ; 00CD
    jmp greekifs_apply_00d3 ; 00CF
greekifs_apply_00d1:
    fstp st(0) ; 00D1
greekifs_apply_00d3:
    fsub qword ptr [r8] ; 00D3
    fld st(0) ; 00D5
    frndint ; 00D7
    fsubp st(1), st(0) ; 00D9
    cmp dword ptr [r8 - 044h], 0 ; 00DB
    je greekifs_apply_00fb ; 00DF
    fabs ; 00E1
    fld qword ptr [rsp + 010h] ; 00E3
    fabs ; 00E7
    fcom st(1) ; 00E9
    wait ; 00EB
    fnstsw ax ; 00EC
    shr ah, 1 ; 00EE
    jb greekifs_apply_00f6 ; 00F0
    fstp st(1) ; 00F2
    jmp greekifs_apply_00f8 ; 00F4
greekifs_apply_00f6:
    fstp st(0) ; 00F6
greekifs_apply_00f8:
    jmp greekifs_apply_0107 ; 00F8
    nop ; 00FA
greekifs_apply_00fb:
    fmul st(0), st(0) ; 00FB
    fld qword ptr [rsp + 010h] ; 00FD
    fmul st(0), st(0) ; 0101
    faddp st(1), st(0) ; 0103
    fsqrt ; 0105
greekifs_apply_0107:
    fsub qword ptr [r8 - 010h] ; 0107
    cmp dword ptr [r8 - 04ch], 0 ; 010A
    jne greekifs_apply_0113 ; 010E
    fdiv qword ptr [r8 - 020h] ; 0110
greekifs_apply_0113:
    fstp qword ptr [rcx + 32] ; 0113
    fldcw word ptr [rsp + 80]
    add rsp, 104
    ret
greekIFSOriginal_x87 ENDP


; Original KleinBagelIFS CODE: rcx=XYZ,VaryScale,Rout; rdx=original PVar.
KleinBagelIFSOriginal_x87 PROC
    sub rsp, 104
    fnstcw word ptr [rsp + 80]
    mov word ptr [rsp + 82], 137Fh
    fldcw word ptr [rsp + 82]
    mov r8, rdx
    movupd xmm0, xmmword ptr [rcx] ; 0003
    movsd xmm1, qword ptr [rcx + 16] ; 0008
    mulpd xmm0, xmmword ptr [r8 - 020h] ; 000D
    mulsd xmm1, qword ptr [r8 - 020h] ; 0012
    addpd xmm0, xmmword ptr [r8 - 040h] ; 0017
    addsd xmm1, qword ptr [r8 - 030h] ; 001C
    cmp dword ptr [r8 - 04ch], 0 ; 0021
    je kleinbagelifs_apply_0040 ; 0025
    movsd xmm5, qword ptr [rcx + 24] ; 0027
    mulsd xmm5, qword ptr [r8 - 020h] ; 002C
    movupd xmmword ptr [rcx], xmm0 ; 0031
    movsd qword ptr [rcx + 16], xmm1 ; 0036
    movsd qword ptr [rcx + 24], xmm5 ; 003B
kleinbagelifs_apply_0040:
    movupd xmmword ptr [rsp], xmm0 ; 0040
    movsd qword ptr [rsp + 010h], xmm1 ; 0045
    fld qword ptr [rsp] ; 004B
    fld qword ptr [rsp + 8] ; 004E
    fpatan ; 0052
    fimul dword ptr [r8 - 054h] ; 0054
    fmul qword ptr [r8] ; 0057
    fsincos ; 0059
    fstp qword ptr [rsp + 020h] ; 005B
    fstp qword ptr [rsp + 018h] ; 005F
    fld qword ptr [rsp] ; 0063
    fmul qword ptr [r8 - 05ch] ; 0066
    fstp qword ptr [rsp] ; 0069
    cmp dword ptr [r8 - 050h], 0 ; 006C
    jne kleinbagelifs_apply_0083 ; 0070
    fld qword ptr [rsp] ; 0072
    fmul st(0), st(0) ; 0075
    fld qword ptr [rsp + 8] ; 0077
    fmul st(0), st(0) ; 007B
    faddp st(1), st(0) ; 007D
    fsqrt ; 007F
    jmp kleinbagelifs_apply_009d ; 0081
kleinbagelifs_apply_0083:
    fld qword ptr [rsp] ; 0083
    fabs ; 0086
    fld qword ptr [rsp + 8] ; 0088
    fabs ; 008C
    fcom st(1) ; 008E
    wait ; 0090
    fnstsw ax ; 0091
    shr ah, 1 ; 0093
    jae kleinbagelifs_apply_009b ; 0095
    fstp st(0) ; 0097
    jmp kleinbagelifs_apply_009d ; 0099
kleinbagelifs_apply_009b:
    fstp st(1) ; 009B
kleinbagelifs_apply_009d:
    fsub qword ptr [r8 - 010h] ; 009D
    fstp qword ptr [rsp] ; 00A0
    fld qword ptr [rsp] ; 00A3
    fmul qword ptr [rsp + 018h] ; 00A6
    fld qword ptr [rsp + 010h] ; 00AA
    fmul qword ptr [rsp + 020h] ; 00AE
    fchs ; 00B2
    faddp st(1), st(0) ; 00B4
    fld qword ptr [rsp] ; 00B6
    fmul qword ptr [rsp + 020h] ; 00B9
    fld qword ptr [rsp + 010h] ; 00BD
    fmul qword ptr [rsp + 018h] ; 00C1
    faddp st(1), st(0) ; 00C5
    fxch st(1) ; 00C7
    fstp qword ptr [rsp + 010h] ; 00C9
    fmul st(0), st(0) ; 00CD
    fld st(0) ; 00CF
    fmul st(0), st(0) ; 00D1
    fxch st(1) ; 00D3
    fchs ; 00D5
    faddp st(1), st(0) ; 00D7
    fld qword ptr [rsp + 010h] ; 00D9
    fmul st(0), st(0) ; 00DD
    faddp st(1), st(0) ; 00DF
    fabs ; 00E1
    fsqrt ; 00E3
    fmul qword ptr [r8] ; 00E5
    fsub qword ptr [r8 - 048h] ; 00E7
    cmp dword ptr [r8 - 04ch], 0 ; 00EA
    jne kleinbagelifs_apply_00f3 ; 00EE
    fdiv qword ptr [r8 - 020h] ; 00F0
kleinbagelifs_apply_00f3:
    fstp qword ptr [rcx + 32] ; 00F3
    fldcw word ptr [rsp + 80]
    add rsp, 104
    ret
KleinBagelIFSOriginal_x87 ENDP


; Original schwartzIFS CODE: rcx=XYZ,VaryScale,Rout; rdx=original PVar.
schwartzIFSOriginal_x87 PROC
    sub rsp, 104
    fnstcw word ptr [rsp + 80]
    mov word ptr [rsp + 82], 137Fh
    fldcw word ptr [rsp + 82]
    mov r8, rdx
    movupd xmm0, xmmword ptr [rcx] ; 0003
    movsd xmm1, qword ptr [rcx + 16] ; 0008
    mulpd xmm0, xmmword ptr [r8 - 020h] ; 000D
    mulsd xmm1, qword ptr [r8 - 020h] ; 0012
    addpd xmm0, xmmword ptr [r8 - 040h] ; 0017
    addsd xmm1, qword ptr [r8 - 030h] ; 001C
    cmp dword ptr [r8 - 04ch], 0 ; 0021
    je schwartzifs_apply_0040 ; 0025
    movsd xmm5, qword ptr [rcx + 24] ; 0027
    mulsd xmm5, qword ptr [r8 - 020h] ; 002C
    movupd xmmword ptr [rcx], xmm0 ; 0031
    movsd qword ptr [rcx + 16], xmm1 ; 0036
    movsd qword ptr [rcx + 24], xmm5 ; 003B
schwartzifs_apply_0040:
    movupd xmmword ptr [rsp], xmm0 ; 0040
    movsd qword ptr [rsp + 010h], xmm1 ; 0045
    nop ; 004B
    add r8, -02ch
    fld qword ptr [rsp + 010h] ; 004F
    fld qword ptr [rsp + 8] ; 0053
    fld qword ptr [rsp] ; 0057
    fld st(0) ; 005A
    fmul dword ptr [r8 - 03ch] ; 005C
    fld st(2) ; 005F
    fmul dword ptr [r8 - 040h] ; 0061
    faddp st(1), st(0) ; 0064
    fld st(3) ; 0066
    fmul dword ptr [r8 - 044h] ; 0068
    faddp st(1), st(0) ; 006B
    fld st(1) ; 006D
    fmul dword ptr [r8 - 024h] ; 006F
    fld st(3) ; 0072
    fmul dword ptr [r8 - 028h] ; 0074
    faddp st(1), st(0) ; 0077
    fld st(4) ; 0079
    fmul dword ptr [r8 - 02ch] ; 007B
    faddp st(1), st(0) ; 007E
    fxch st(2) ; 0080
    fmul dword ptr [r8 - 030h] ; 0082
    fxch st(3) ; 0085
    fmul dword ptr [r8 - 034h] ; 0087
    faddp st(3), st(0) ; 008A
    fxch st(3) ; 008C
    fmul dword ptr [r8 - 038h] ; 008E
    faddp st(2), st(0) ; 0091
    fstp qword ptr [rsp] ; 0093
    fstp qword ptr [rsp + 8] ; 0096
    fstp qword ptr [rsp + 010h] ; 009A
    add r8, 02ch
    nop ; 00A1
    fld qword ptr [rsp] ; 00A2
    fmul st(0), st(0) ; 00A5
    fld qword ptr [rsp + 8] ; 00A7
    fmul st(0), st(0) ; 00AB
    faddp st(1), st(0) ; 00AD
    fld qword ptr [rsp + 010h] ; 00AF
    fmul st(0), st(0) ; 00B3
    faddp st(1), st(0) ; 00B5
    fld qword ptr [r8 - 010h] ; 00B7
    fabs ; 00BA
    faddp st(1), st(0) ; 00BC
    fstp qword ptr [rsp + 018h] ; 00BE
    fldz ; 00C2
    fstp qword ptr [rsp + 020h] ; 00C4
    nop ; 00C8
    fld qword ptr [rsp] ; 00C9
    fmul qword ptr [r8 - 048h] ; 00CC
    fcos ; 00CF
    ftst ; 00D1
    wait ; 00D3
    fnstsw ax ; 00D4
    shr ah, 1 ; 00D6
    jae schwartzifs_apply_00e4 ; 00D8
    fsubr qword ptr [rsp + 018h] ; 00DA
    fstp qword ptr [rsp + 018h] ; 00DE
    jmp schwartzifs_apply_00ec ; 00E2
schwartzifs_apply_00e4:
    fadd qword ptr [rsp + 020h] ; 00E4
    fstp qword ptr [rsp + 020h] ; 00E8
schwartzifs_apply_00ec:
    fld qword ptr [rsp + 8] ; 00EC
    fmul qword ptr [r8 - 048h] ; 00F0
    fcos ; 00F3
    ftst ; 00F5
    wait ; 00F7
    fnstsw ax ; 00F8
    shr ah, 1 ; 00FA
    jae schwartzifs_apply_0108 ; 00FC
    fsubr qword ptr [rsp + 018h] ; 00FE
    fstp qword ptr [rsp + 018h] ; 0102
    jmp schwartzifs_apply_0110 ; 0106
schwartzifs_apply_0108:
    fadd qword ptr [rsp + 020h] ; 0108
    fstp qword ptr [rsp + 020h] ; 010C
schwartzifs_apply_0110:
    fld qword ptr [rsp + 010h] ; 0110
    fmul qword ptr [r8 - 048h] ; 0114
    fcos ; 0117
    ftst ; 0119
    wait ; 011B
    fnstsw ax ; 011C
    shr ah, 1 ; 011E
    jae schwartzifs_apply_012c ; 0120
    fsubr qword ptr [rsp + 018h] ; 0122
    fstp qword ptr [rsp + 018h] ; 0126
    jmp schwartzifs_apply_0134 ; 012A
schwartzifs_apply_012c:
    fadd qword ptr [rsp + 020h] ; 012C
    fstp qword ptr [rsp + 020h] ; 0130
schwartzifs_apply_0134:
    nop ; 0134
    fld qword ptr [rsp + 020h] ; 0135
    fsqrt ; 0139
    fld qword ptr [rsp + 018h] ; 013B
    fsqrt ; 013F
    fsubrp st(1), st(0) ; 0141
    fld qword ptr [r8 - 048h] ; 0143
    fabs ; 0146
    fld1 ; 0148
    fsubp st(1), st(0) ; 014A
    fld st(0) ; 014C
    fabs ; 014E
    faddp st(1), st(0) ; 0150
    fld1 ; 0152
    faddp st(1), st(0) ; 0154
    fmul qword ptr [r8] ; 0156
    nop ; 0158
    nop ; 0159
    fsqrt ; 015A
    fdivp st(1), st(0) ; 015C
    nop ; 015E
    cmp dword ptr [r8 - 04ch], 0 ; 015F
    jne schwartzifs_apply_0168 ; 0163
    fdiv qword ptr [r8 - 020h] ; 0165
schwartzifs_apply_0168:
    fstp qword ptr [rcx + 32] ; 0168
    fldcw word ptr [rsp + 80]
    add rsp, 104
    ret
schwartzIFSOriginal_x87 ENDP


; Original tritgridIFS CODE: rcx=XYZ,VaryScale,Rout; rdx=original PVar.
tritgridIFSOriginal_x87 PROC
    sub rsp, 104
    fnstcw word ptr [rsp + 80]
    mov word ptr [rsp + 82], 137Fh
    fldcw word ptr [rsp + 82]
    mov r8, rdx
    movupd xmm0, xmmword ptr [rcx] ; 0003
    movsd xmm1, qword ptr [rcx + 16] ; 0008
    mulpd xmm0, xmmword ptr [r8 - 020h] ; 000D
    mulsd xmm1, qword ptr [r8 - 020h] ; 0012
    addpd xmm0, xmmword ptr [r8 - 040h] ; 0017
    addsd xmm1, qword ptr [r8 - 030h] ; 001C
    cmp dword ptr [r8 - 04ch], 0 ; 0021
    je tritgridifs_apply_0040 ; 0025
    movsd xmm5, qword ptr [rcx + 24] ; 0027
    mulsd xmm5, qword ptr [r8 - 020h] ; 002C
    movupd xmmword ptr [rcx], xmm0 ; 0031
    movsd qword ptr [rcx + 16], xmm1 ; 0036
    movsd qword ptr [rcx + 24], xmm5 ; 003B
tritgridifs_apply_0040:
    movupd xmmword ptr [rsp], xmm0 ; 0040
    movsd qword ptr [rsp + 010h], xmm1 ; 0045
    nop ; 004B
    nop ; 004C
    nop ; 004D
    nop ; 004E
    nop ; 004F
    nop ; 0050
    add r8, -02ch
    fld qword ptr [rsp + 010h] ; 0054
    fld qword ptr [rsp + 8] ; 0058
    fld qword ptr [rsp] ; 005C
    fld st(0) ; 005F
    fmul dword ptr [r8 - 03ch] ; 0061
    fld st(2) ; 0064
    fmul dword ptr [r8 - 040h] ; 0066
    faddp st(1), st(0) ; 0069
    fld st(3) ; 006B
    fmul dword ptr [r8 - 044h] ; 006D
    faddp st(1), st(0) ; 0070
    fld st(1) ; 0072
    fmul dword ptr [r8 - 024h] ; 0074
    fld st(3) ; 0077
    fmul dword ptr [r8 - 028h] ; 0079
    faddp st(1), st(0) ; 007C
    fld st(4) ; 007E
    fmul dword ptr [r8 - 02ch] ; 0080
    faddp st(1), st(0) ; 0083
    fxch st(2) ; 0085
    fmul dword ptr [r8 - 030h] ; 0087
    fxch st(3) ; 008A
    fmul dword ptr [r8 - 034h] ; 008C
    faddp st(3), st(0) ; 008F
    fxch st(3) ; 0091
    fmul dword ptr [r8 - 038h] ; 0093
    faddp st(2), st(0) ; 0096
    fstp qword ptr [rsp] ; 0098
    fstp qword ptr [rsp + 8] ; 009B
    fstp qword ptr [rsp + 010h] ; 009F
    add r8, 02ch
    fld qword ptr [rsp] ; 00A6
    fsubr qword ptr [r8 - 048h] ; 00A9
    fld st(0) ; 00AC
    fistp dword ptr [rsp + 020h] ; 00AE
    fild dword ptr [rsp + 020h] ; 00B2
    fsubp st(1), st(0) ; 00B6
    fabs ; 00B8
    nop ; 00BA
    fld qword ptr [rsp] ; 00BB
    fmul qword ptr [r8] ; 00BE
    fld qword ptr [rsp + 8] ; 00C0
    fmul qword ptr [r8 + 8] ; 00C4
    fld st(1) ; 00C7
    fadd st(0), st(1) ; 00C9
    fld st(0) ; 00CB
    fistp dword ptr [rsp + 024h] ; 00CD
    fild dword ptr [rsp + 024h] ; 00D1
    fsubp st(1), st(0) ; 00D5
    fabs ; 00D7
    fxch st(2) ; 00D9
    fsubp st(1), st(0) ; 00DB
    fist dword ptr [rsp + 028h] ; 00DD
    fild dword ptr [rsp + 028h] ; 00E1
    fsubp st(1), st(0) ; 00E5
    fabs ; 00E7
    nop ; 00E9
    fcom st(1) ; 00EA
    wait ; 00EC
    fnstsw ax ; 00ED
    shr ah, 1 ; 00EF
    jb tritgridifs_apply_00f7 ; 00F1
    fstp st(0) ; 00F3
    jmp tritgridifs_apply_00f9 ; 00F5
tritgridifs_apply_00f7:
    fstp st(1) ; 00F7
tritgridifs_apply_00f9:
    fcom st(1) ; 00F9
    wait ; 00FB
    fnstsw ax ; 00FC
    shr ah, 1 ; 00FE
    jb tritgridifs_apply_0106 ; 0100
    fstp st(0) ; 0102
    jmp tritgridifs_apply_0108 ; 0104
tritgridifs_apply_0106:
    fstp st(1) ; 0106
tritgridifs_apply_0108:
    fmul st(0), st(0) ; 0108
    fld qword ptr [rsp + 010h] ; 010A
    fmul st(0), st(0) ; 010E
    faddp st(1), st(0) ; 0110
    fsqrt ; 0112
    fsub qword ptr [r8 - 010h] ; 0114
    cmp dword ptr [r8 - 04ch], 0 ; 0117
    jne tritgridifs_apply_0120 ; 011B
    fdiv qword ptr [r8 - 020h] ; 011D
tritgridifs_apply_0120:
    fstp qword ptr [rcx + 32] ; 0120
    fldcw word ptr [rsp + 80]
    add rsp, 104
    ret
tritgridIFSOriginal_x87 ENDP


; Original MengerKoch CODE. rcx=XYZ,DE/VaryScale,Rout; rdx=original numeric PVar.
MengerKochOriginal_x87 PROC
    sub rsp, 72
    fnstcw word ptr [rsp + 56]
    mov word ptr [rsp + 58], 137Fh
    fldcw word ptr [rsp + 58]
    mov r8, rdx
    mov eax, 07fffffffh ; 000D
    and dword ptr [rcx + 20], eax ; 0012
    and dword ptr [rcx + 12], eax ; 0015
    and dword ptr [rcx + 4], eax ; 0018
    fld qword ptr [rcx + 16] ; 001B
    fld qword ptr [rcx + 8] ; 001D
    fld qword ptr [rcx] ; 001F
    fld st(0) ; 0021
    fmul dword ptr [r8 - 044h] ; 0023
    fld st(2) ; 0026
    fmul dword ptr [r8 - 048h] ; 0028
    faddp st(1), st(0) ; 002B
    fld st(3) ; 002D
    fmul dword ptr [r8 - 04ch] ; 002F
    faddp st(1), st(0) ; 0032
    fld st(1) ; 0034
    fmul dword ptr [r8 - 02ch] ; 0036
    fld st(3) ; 0039
    fmul dword ptr [r8 - 030h] ; 003B
    faddp st(1), st(0) ; 003E
    fld st(4) ; 0040
    fmul dword ptr [r8 - 034h] ; 0042
    faddp st(1), st(0) ; 0045
    fxch st(2) ; 0047
    fmul dword ptr [r8 - 038h] ; 0049
    fxch st(3) ; 004C
    fmul dword ptr [r8 - 03ch] ; 004E
    faddp st(3), st(0) ; 0051
    fxch st(3) ; 0053
    fmul dword ptr [r8 - 040h] ; 0055
    faddp st(2), st(0) ; 0058
    fcom st(1) ; 005A
    fnstsw ax ; 005C
    shr ah, 1 ; 005E
    jae mengerkoch_fold_order_0064 ; 0060
    fxch st(1) ; 0062
mengerkoch_fold_order_0064:
    fcom st(2) ; 0064
    fnstsw ax ; 0066
    shr ah, 1 ; 0068
    jae mengerkoch_fold_order_006e ; 006A
    fxch st(2) ; 006C
mengerkoch_fold_order_006e:
    fxch st(1) ; 006E
    fcom st(2) ; 0070
    fnstsw ax ; 0072
    shr ah, 1 ; 0074
    jae mengerkoch_fold_order_007a ; 0076
    fxch st(2) ; 0078
mengerkoch_fold_order_007a:
    fld qword ptr [r8 - 078h] ; 007A
    fsub st(0), st(2) ; 007D
    fabs ; 007F
    fxch st(2) ; 0081
    fstp st(0) ; 0083
    fsub qword ptr [r8 - 080h] ; 0085
    fabs ; 0088
    fnop ; 008A
    fld st(1) ; 008C
    fmul dword ptr [r8 - 068h] ; 008E
    fld st(1) ; 0091
    fmul dword ptr [r8 - 06ch] ; 0093
    faddp st(1), st(0) ; 0096
    fld st(3) ; 0098
    fmul dword ptr [r8 - 070h] ; 009A
    faddp st(1), st(0) ; 009D
    fld st(2) ; 009F
    fmul dword ptr [r8 - 050h] ; 00A1
    fld st(2) ; 00A4
    fmul dword ptr [r8 - 054h] ; 00A6
    faddp st(1), st(0) ; 00A9
    fld st(4) ; 00AB
    fmul dword ptr [r8 - 058h] ; 00AD
    faddp st(1), st(0) ; 00B0
    fxch st(3) ; 00B2
    fmul dword ptr [r8 - 05ch] ; 00B4
    fxch st(2) ; 00B7
    fmul dword ptr [r8 - 060h] ; 00B9
    faddp st(2), st(0) ; 00BC
    fxch st(3) ; 00BE
    fmul dword ptr [r8 - 064h] ; 00C0
    faddp st(1), st(0) ; 00C3
    fld qword ptr [r8 - 010h] ; 00C5
    fld qword ptr [rcx + 24] ; 00C8
    fmul st(0), st(1) ; 00CB
    fstp qword ptr [rcx + 24] ; 00CD
    fmul st(3), st(0) ; 00D0
    fmul st(2), st(0) ; 00D2
    fmul st(1), st(0) ; 00D4
    fld1 ; 00D6
    fsubp st(1), st(0) ; 00D8
    fld st(0) ; 00DA
    fld st(0) ; 00DC
    fmul qword ptr [r8 - 018h] ; 00DE
    fsubp st(4), st(0) ; 00E1
    fmul qword ptr [r8 - 020h] ; 00E3
    fsubp st(2), st(0) ; 00E6
    fmul qword ptr [r8 - 028h] ; 00E8
    fld st(0) ; 00EB
    fmul qword ptr [r8 - 8] ; 00ED
    fcomp st(4) ; 00F0
    fnstsw ax ; 00F2
    and ah, 041h ; 00F4
    je mengerkoch_fold_order_00fb ; 00F7
    fsub st(3), st(0) ; 00F9
mengerkoch_fold_order_00fb:
    fstp st(0) ; 00FB
    fstp qword ptr [rcx + 8] ; 00FD
    fstp qword ptr [rcx] ; 00FF
    fstp qword ptr [rcx + 16] ; 0101
    fldcw word ptr [rsp + 56]
    add rsp, 72
    ret
MengerKochOriginal_x87 ENDP


; Original foldingoctIFS CODE. rcx=XYZ,DE/VaryScale,Rout; rdx=original numeric PVar.
foldingoctIFSOriginal_x87 PROC
    sub rsp, 72
    fnstcw word ptr [rsp + 56]
    mov word ptr [rsp + 58], 137Fh
    fldcw word ptr [rsp + 58]
    mov r8, rdx
    fld qword ptr [rcx + 16] ; 0000
    fadd qword ptr [r8 - 020h] ; 0003
    fld qword ptr [rcx + 8] ; 0006
    fadd qword ptr [r8 - 018h] ; 0009
    fld qword ptr [rcx] ; 000C
    fadd qword ptr [r8 - 010h] ; 000F
    fld st(0) ; 0012
    fmul dword ptr [r8 - 03ch] ; 0014
    fld st(2) ; 0017
    fmul dword ptr [r8 - 040h] ; 0019
    faddp st(1), st(0) ; 001C
    fld st(3) ; 001E
    fmul dword ptr [r8 - 044h] ; 0020
    faddp st(1), st(0) ; 0023
    fld st(1) ; 0025
    fmul dword ptr [r8 - 024h] ; 0027
    fld st(3) ; 002A
    fmul dword ptr [r8 - 028h] ; 002C
    faddp st(1), st(0) ; 002F
    fld st(4) ; 0031
    fmul dword ptr [r8 - 02ch] ; 0033
    faddp st(1), st(0) ; 0036
    fxch st(2) ; 0038
    fmul dword ptr [r8 - 030h] ; 003A
    fxch st(3) ; 003D
    fmul dword ptr [r8 - 034h] ; 003F
    faddp st(3), st(0) ; 0042
    fxch st(3) ; 0044
    fmul dword ptr [r8 - 038h] ; 0046
    faddp st(2), st(0) ; 0049
    mov edx, dword ptr [r8 - 048h] ; 004B
    and edx, 1 ; 004E
    je foldingoctifs_fold_order_0055 ; 0051
    fabs ; 0053
foldingoctifs_fold_order_0055:
    fstp qword ptr [rcx] ; 0055
    mov edx, dword ptr [r8 - 04ch] ; 0058
    and edx, 1 ; 005B
    je foldingoctifs_fold_order_0062 ; 005E
    fabs ; 0060
foldingoctifs_fold_order_0062:
    fstp qword ptr [rcx + 8] ; 0062
    mov edx, dword ptr [r8 - 050h] ; 0065
    and edx, 1 ; 0068
    je foldingoctifs_fold_order_006f ; 006B
    fabs ; 006D
foldingoctifs_fold_order_006f:
    fld qword ptr [rcx + 8] ; 006F
    fld qword ptr [rcx] ; 0072
    mov edx, dword ptr [r8 - 048h] ; 0075
    and edx, 2 ; 0078
    je foldingoctifs_fold_order_0088 ; 007B
    fcom st(1) ; 007D
    fnstsw ax ; 007F
    and ah, 041h ; 0081
    je foldingoctifs_fold_order_0088 ; 0084
    fxch st(1) ; 0086
foldingoctifs_fold_order_0088:
    mov edx, dword ptr [r8 - 04ch] ; 0088
    and edx, 2 ; 008B
    je foldingoctifs_fold_order_009b ; 008E
    fcom st(2) ; 0090
    fnstsw ax ; 0092
    and ah, 041h ; 0094
    je foldingoctifs_fold_order_009b ; 0097
    fxch st(2) ; 0099
foldingoctifs_fold_order_009b:
    fxch st(1) ; 009B
    mov edx, dword ptr [r8 - 050h] ; 009D
    and edx, 2 ; 00A0
    je foldingoctifs_fold_order_00b0 ; 00A3
    fcom st(2) ; 00A5
    fnstsw ax ; 00A7
    and ah, 041h ; 00A9
    je foldingoctifs_fold_order_00b0 ; 00AC
    fxch st(2) ; 00AE
foldingoctifs_fold_order_00b0:
    fstp qword ptr [rcx] ; 00B0
    fstp qword ptr [rcx + 8] ; 00B3
    fstp qword ptr [rcx + 16] ; 00B6
    fldcw word ptr [rsp + 56]
    add rsp, 72
    ret
foldingoctIFSOriginal_x87 ENDP


; Original pyramIFS CODE. rcx=XYZ,DE/VaryScale,Rout; rdx=original numeric PVar.
pyramIFSOriginal_x87 PROC
    sub rsp, 72
    fnstcw word ptr [rsp + 56]
    mov word ptr [rsp + 58], 137Fh
    fldcw word ptr [rsp + 58]
    mov r8, rdx
    movupd xmm0, xmmword ptr [rcx] ; 0003
    movsd xmm1, qword ptr [rcx + 16] ; 0008
    mulpd xmm0, xmmword ptr [r8 - 020h] ; 000D
    mulsd xmm1, qword ptr [r8 - 020h] ; 0012
    addpd xmm0, xmmword ptr [r8 - 040h] ; 0017
    addsd xmm1, qword ptr [r8 - 030h] ; 001C
    cmp dword ptr [r8 - 04ch], 0 ; 0021
    je pyramifs_fold_order_0040 ; 0025
    movsd xmm5, qword ptr [rcx + 24] ; 0027
    mulsd xmm5, qword ptr [r8 - 020h] ; 002C
    movupd xmmword ptr [rcx], xmm0 ; 0031
    movsd qword ptr [rcx + 16], xmm1 ; 0036
    movsd qword ptr [rcx + 24], xmm5 ; 003B
pyramifs_fold_order_0040:
    movupd xmmword ptr [rsp], xmm0 ; 0040
    movsd qword ptr [rsp + 010h], xmm1 ; 0045
    fld qword ptr [rsp + 010h] ; 004B
    fmul qword ptr [r8 - 054h] ; 004F
    fsincos ; 0052
    fstp qword ptr [rsp + 020h] ; 0054
    fstp qword ptr [rsp + 018h] ; 0058
    fld qword ptr [rsp] ; 005C
    fmul qword ptr [rsp + 018h] ; 005F
    fld qword ptr [rsp + 8] ; 0063
    fmul qword ptr [rsp + 020h] ; 0067
    fchs ; 006B
    faddp st(1), st(0) ; 006D
    fld qword ptr [rsp] ; 006F
    fmul qword ptr [rsp + 020h] ; 0072
    fld qword ptr [rsp + 8] ; 0076
    fmul qword ptr [rsp + 018h] ; 007A
    faddp st(1), st(0) ; 007E
    fstp qword ptr [rsp + 8] ; 0080
    fstp qword ptr [rsp] ; 0084
    fld qword ptr [rsp] ; 0087
    fadd qword ptr [rsp + 8] ; 008A
    fadd qword ptr [rsp + 010h] ; 008E
    fld qword ptr [rsp] ; 0092
    fsub qword ptr [rsp + 8] ; 0095
    fsub qword ptr [rsp + 010h] ; 0099
    fcom st(1) ; 009D
    wait ; 009F
    fnstsw ax ; 00A0
    shr ah, 1 ; 00A2
    jae pyramifs_fold_order_00aa ; 00A4
    fstp st(0) ; 00A6
    jmp pyramifs_fold_order_00ac ; 00A8
pyramifs_fold_order_00aa:
    fstp st(1) ; 00AA
pyramifs_fold_order_00ac:
    fld qword ptr [rsp + 8] ; 00AC
    fsub qword ptr [rsp] ; 00B0
    fsub qword ptr [rsp + 010h] ; 00B3
    fcom st(1) ; 00B7
    wait ; 00B9
    fnstsw ax ; 00BA
    shr ah, 1 ; 00BC
    jae pyramifs_fold_order_00c4 ; 00BE
    fstp st(0) ; 00C0
    jmp pyramifs_fold_order_00c6 ; 00C2
pyramifs_fold_order_00c4:
    fstp st(1) ; 00C4
pyramifs_fold_order_00c6:
    fld qword ptr [rsp + 010h] ; 00C6
    fsub qword ptr [rsp] ; 00CA
    fsub qword ptr [rsp + 8] ; 00CD
    fcom st(1) ; 00D1
    wait ; 00D3
    fnstsw ax ; 00D4
    shr ah, 1 ; 00D6
    jae pyramifs_fold_order_00de ; 00D8
    fstp st(0) ; 00DA
    jmp pyramifs_fold_order_00e0 ; 00DC
pyramifs_fold_order_00de:
    fstp st(1) ; 00DE
pyramifs_fold_order_00e0:
    fst qword ptr [rsp + 018h] ; 00E0
    fsub qword ptr [r8 - 010h] ; 00E4
    fld qword ptr [rsp] ; 00E7
    fmul st(0), st(0) ; 00EA
    fld qword ptr [rsp + 8] ; 00EC
    fmul st(0), st(0) ; 00F0
    faddp st(1), st(0) ; 00F2
    fld qword ptr [rsp + 010h] ; 00F4
    fmul st(0), st(0) ; 00F8
    faddp st(1), st(0) ; 00FA
    fsqrt ; 00FC
    fmul qword ptr [r8 - 048h] ; 00FE
    faddp st(1), st(0) ; 0101
    cmp dword ptr [r8 - 04ch], 0 ; 0103
    jne pyramifs_fold_order_010c ; 0107
    fdiv qword ptr [r8 - 020h] ; 0109
pyramifs_fold_order_010c:
    fstp qword ptr [rcx + 32] ; 010C
    fldcw word ptr [rsp + 56]
    add rsp, 72
    ret
pyramIFSOriginal_x87 ENDP


; Original Knots2IFS CODE: rcx=XYZ,VaryScale,Rout; rdx=numeric PVar.
; r8: FormulaWorkCounter. Return eax=0 complete, eax=1 work exhausted.
Knots2IFSOriginal_x87 PROC
    mov r10, r8
    sub rsp, 72
    fnstcw word ptr [rsp + 56]
    mov word ptr [rsp + 58], 137Fh
    fldcw word ptr [rsp + 58]
    mov r8, rdx
    movupd xmm0, xmmword ptr [rcx] ; 0003
    movsd xmm1, qword ptr [rcx + 16] ; 0008
    mulpd xmm0, xmmword ptr [r8 - 020h] ; 000D
    mulsd xmm1, qword ptr [r8 - 020h] ; 0012
    addpd xmm0, xmmword ptr [r8 - 040h] ; 0017
    addsd xmm1, qword ptr [r8 - 030h] ; 001C
    cmp dword ptr [r8 - 04ch], 0 ; 0021
    je knots2ifs_original_0040 ; 0025
    movsd xmm5, qword ptr [rcx + 24] ; 0027
    mulsd xmm5, qword ptr [r8 - 020h] ; 002C
    movupd xmmword ptr [rcx], xmm0 ; 0031
    movsd qword ptr [rcx + 16], xmm1 ; 0036
    movsd qword ptr [rcx + 24], xmm5 ; 003B
knots2ifs_original_0040:
    movupd xmmword ptr [rsp], xmm0 ; 0040
    movsd qword ptr [rsp + 010h], xmm1 ; 0045
    fld qword ptr [rsp + 8] ; 004B
    fld qword ptr [rsp] ; 004F
    fpatan ; 0052
    fld qword ptr [rsp + 8] ; 0054
    fmul st(0), st(0) ; 0058
    fld qword ptr [rsp] ; 005A
    fmul st(0), st(0) ; 005D
    faddp st(1), st(0) ; 005F
    fsqrt ; 0061
    fstp qword ptr [rsp] ; 0063
    fstp qword ptr [rsp + 8] ; 0066
    fld qword ptr [r8 + 010h] ; 006A
    fstp qword ptr [rcx + 32] ; 006D
    mov edx, dword ptr [r8 - 064h] ; 0070
knots2ifs_original_0073:
    mov eax, dword ptr [r10]
    cmp eax, dword ptr [r10 + 4]
    jae knots2ifs_work_exhausted
    inc dword ptr [r10]
    fld qword ptr [rsp + 8] ; 0073
    fadd qword ptr [r8 + 8] ; 0077
    fst qword ptr [rsp + 8] ; 007A
    fidiv dword ptr [r8 - 050h] ; 007E
    fst qword ptr [rsp + 018h] ; 0081
    fimul dword ptr [r8 - 054h] ; 0085
    fcos ; 0088
    fmul qword ptr [r8 - 060h] ; 008A
    fadd qword ptr [rsp] ; 008D
    fsub qword ptr [r8 - 010h] ; 0090
    fmul st(0), st(0) ; 0093
    fld qword ptr [rsp + 018h] ; 0095
    fimul dword ptr [r8 - 058h] ; 0099
    fsin ; 009C
    fmul qword ptr [r8 - 060h] ; 009E
    fadd qword ptr [rsp + 010h] ; 00A1
    fsub qword ptr [r8 - 010h] ; 00A5
    fmul st(0), st(0) ; 00A8
    faddp st(1), st(0) ; 00AA
    fsqrt ; 00AC
    fcom qword ptr [rcx + 32] ; 00AE
    wait ; 00B1
    fnstsw ax ; 00B2
    shr ah, 1 ; 00B4
    jae knots2ifs_original_00bd ; 00B6
    fstp qword ptr [rcx + 32] ; 00B8
    jmp knots2ifs_original_00bf ; 00BB
knots2ifs_original_00bd:
    fstp st(0) ; 00BD
knots2ifs_original_00bf:
    dec edx ; 00BF
    jg knots2ifs_original_0073 ; 00C0
    fld qword ptr [rcx + 32] ; 00C2
    fsub qword ptr [r8 - 048h] ; 00C5
    cmp dword ptr [r8 - 04ch], 0 ; 00C8
    jne knots2ifs_original_00d1 ; 00CC
    fdiv qword ptr [r8 - 020h] ; 00CE
knots2ifs_original_00d1:
    fstp qword ptr [rcx + 32] ; 00D1
    xor eax, eax
    jmp knots2ifs_work_done
knots2ifs_work_exhausted:
    mov eax, 1
knots2ifs_work_done:
    fldcw word ptr [rsp + 56]
    add rsp, 72
    ret
Knots2IFSOriginal_x87 ENDP


; Original WeirdCubeIFS CODE: rcx=XYZ,VaryScale,Rout; rdx=numeric PVar.
WeirdCubeIFSOriginal_x87 PROC
    sub rsp, 120
    fnstcw word ptr [rsp + 104]
    mov word ptr [rsp + 106], 137Fh
    fldcw word ptr [rsp + 106]
    mov r8, rdx
    movupd xmm0, xmmword ptr [rcx] ; 0003
    movsd xmm1, qword ptr [rcx + 16] ; 0008
    mulpd xmm0, xmmword ptr [r8 - 020h] ; 000D
    mulsd xmm1, qword ptr [r8 - 020h] ; 0012
    addpd xmm0, xmmword ptr [r8 - 040h] ; 0017
    addsd xmm1, qword ptr [r8 - 030h] ; 001C
    cmp dword ptr [r8 - 04ch], 0 ; 0021
    je weirdcubeifs_original_0040 ; 0025
    movsd xmm5, qword ptr [rcx + 24] ; 0027
    mulsd xmm5, qword ptr [r8 - 020h] ; 002C
    movupd xmmword ptr [rcx], xmm0 ; 0031
    movsd qword ptr [rcx + 16], xmm1 ; 0036
    movsd qword ptr [rcx + 24], xmm5 ; 003B
weirdcubeifs_original_0040:
    movupd xmmword ptr [rsp], xmm0 ; 0040
    movsd qword ptr [rsp + 010h], xmm1 ; 0045
    fldz ; 004B
    fst qword ptr [rsp + 018h] ; 004D
    fstp qword ptr [rsp + 020h] ; 0051
    sub r8, 44 ; Original local matrix base
    fld qword ptr [rsp + 010h] ; 0058
    fld qword ptr [rsp + 8] ; 005C
    fld qword ptr [rsp] ; 0060
    fld st(0) ; 0063
    fmul dword ptr [r8 - 03ch] ; 0065
    fld st(2) ; 0068
    fmul dword ptr [r8 - 040h] ; 006A
    faddp st(1), st(0) ; 006D
    fld st(3) ; 006F
    fmul dword ptr [r8 - 044h] ; 0071
    faddp st(1), st(0) ; 0074
    fld st(1) ; 0076
    fmul dword ptr [r8 - 024h] ; 0078
    fld st(3) ; 007B
    fmul dword ptr [r8 - 028h] ; 007D
    faddp st(1), st(0) ; 0080
    fld st(4) ; 0082
    fmul dword ptr [r8 - 02ch] ; 0084
    faddp st(1), st(0) ; 0087
    fxch st(2) ; 0089
    fmul dword ptr [r8 - 030h] ; 008B
    fxch st(3) ; 008E
    fmul dword ptr [r8 - 034h] ; 0090
    faddp st(3), st(0) ; 0093
    fxch st(3) ; 0095
    fmul dword ptr [r8 - 038h] ; 0097
    faddp st(2), st(0) ; 009A
    fstp qword ptr [rsp] ; 009C
    fstp qword ptr [rsp + 8] ; 009F
    fstp qword ptr [rsp + 010h] ; 00A3
    add r8, 44
    fld qword ptr [rsp + 010h] ; 00AA
    fmul st(0), st(0) ; 00AE
    fld qword ptr [rsp + 8] ; 00B0
    fmul st(0), st(0) ; 00B4
    fld qword ptr [rsp] ; 00B6
    nop ; 00B9
    fmul st(0), st(0) ; 00BA
    fstp qword ptr [rsp + 028h] ; 00BC
    fstp qword ptr [rsp + 030h] ; 00C0
    fstp qword ptr [rsp + 038h] ; 00C4
    fld qword ptr [rsp] ; 00C8
    fmul qword ptr [rsp + 8] ; 00CB
    fld qword ptr [rsp + 8] ; 00CF
    fmul qword ptr [rsp + 010h] ; 00D3
    fld qword ptr [rsp] ; 00D7
    fmul qword ptr [rsp + 010h] ; 00DA
    fstp qword ptr [rsp + 050h] ; 00DE
    fstp qword ptr [rsp + 048h] ; 00E2
    fstp qword ptr [rsp + 040h] ; 00E6
    fnop ; 00EA
    fld qword ptr [rsp + 040h] ; 00EC
    fmul st(0), st(0) ; 00F0
    fld qword ptr [rsp + 048h] ; 00F2
    fmul st(0), st(0) ; 00F6
    fld qword ptr [rsp + 050h] ; 00F8
    fmul st(0), st(0) ; 00FC
    faddp st(1), st(0) ; 00FE
    faddp st(1), st(0) ; 0100
    fmul qword ptr [r8 + 8] ; 0102
    fstp qword ptr [rsp + 018h] ; 0105
    nop ; 0109
    fld qword ptr [rsp] ; 010A
    fmul qword ptr [rsp + 028h] ; 010D
    fld qword ptr [rsp + 8] ; 0111
    fadd qword ptr [rsp + 010h] ; 0115
    fmulp st(1), st(0) ; 0119
    fmul qword ptr [r8] ; 011B
    nop ; 011D
    ftst ; 011E
    wait ; 0120
    fnstsw ax ; 0121
    shr ah, 1 ; 0123
    jae weirdcubeifs_original_0131 ; 0125
    fsubr qword ptr [rsp + 020h] ; 0127
    fstp qword ptr [rsp + 020h] ; 012B
    jmp weirdcubeifs_original_0139 ; 012F
weirdcubeifs_original_0131:
    fadd qword ptr [rsp + 018h] ; 0131
    fstp qword ptr [rsp + 018h] ; 0135
weirdcubeifs_original_0139:
    fld qword ptr [rsp + 8] ; 0139
    fmul qword ptr [rsp + 030h] ; 013D
    fld qword ptr [rsp] ; 0141
    nop ; 0144
    fadd qword ptr [rsp + 010h] ; 0145
    fmulp st(1), st(0) ; 0149
    fmul qword ptr [r8] ; 014B
    nop ; 014D
    ftst ; 014E
    wait ; 0150
    fnstsw ax ; 0151
    shr ah, 1 ; 0153
    jae weirdcubeifs_original_0161 ; 0155
    fsubr qword ptr [rsp + 020h] ; 0157
    fstp qword ptr [rsp + 020h] ; 015B
    jmp weirdcubeifs_original_0169 ; 015F
weirdcubeifs_original_0161:
    fadd qword ptr [rsp + 018h] ; 0161
    fstp qword ptr [rsp + 018h] ; 0165
weirdcubeifs_original_0169:
    fld qword ptr [rsp + 010h] ; 0169
    fmul qword ptr [rsp + 038h] ; 016D
    fld qword ptr [rsp] ; 0171
    nop ; 0174
    fadd qword ptr [rsp + 8] ; 0175
    fmulp st(1), st(0) ; 0179
    fmul qword ptr [r8] ; 017B
    nop ; 017D
    ftst ; 017E
    wait ; 0180
    fnstsw ax ; 0181
    shr ah, 1 ; 0183
    jae weirdcubeifs_original_0191 ; 0185
    fsubr qword ptr [rsp + 020h] ; 0187
    fstp qword ptr [rsp + 020h] ; 018B
    jmp weirdcubeifs_original_0199 ; 018F
weirdcubeifs_original_0191:
    fadd qword ptr [rsp + 018h] ; 0191
    fstp qword ptr [rsp + 018h] ; 0195
weirdcubeifs_original_0199:
    fld qword ptr [rsp] ; 0199
    fmul qword ptr [rsp + 048h] ; 019C
    fmul qword ptr [r8 - 048h] ; 01A0
    ftst ; 01A3
    wait ; 01A5
    fnstsw ax ; 01A6
    shr ah, 1 ; 01A8
    jae weirdcubeifs_original_01b6 ; 01AA
    fsubr qword ptr [rsp + 020h] ; 01AC
    fstp qword ptr [rsp + 020h] ; 01B0
    jmp weirdcubeifs_original_01be ; 01B4
weirdcubeifs_original_01b6:
    fadd qword ptr [rsp + 018h] ; 01B6
    fstp qword ptr [rsp + 018h] ; 01BA
weirdcubeifs_original_01be:
    fld qword ptr [rsp + 028h] ; 01BE
    fmul qword ptr [r8 + 018h] ; 01C2
    fmul qword ptr [r8 - 048h] ; 01C5
    nop ; 01C8
    fld1 ; 01C9
    faddp st(1), st(0) ; 01CB
    fmul qword ptr [rsp + 048h] ; 01CD
    fmul qword ptr [r8 + 020h] ; 01D1
    ftst ; 01D4
    wait ; 01D6
    fnstsw ax ; 01D7
    shr ah, 1 ; 01D9
    jae weirdcubeifs_original_01e7 ; 01DB
    fsubr qword ptr [rsp + 020h] ; 01DD
    fstp qword ptr [rsp + 020h] ; 01E1
    jmp weirdcubeifs_original_01ef ; 01E5
weirdcubeifs_original_01e7:
    fadd qword ptr [rsp + 018h] ; 01E7
    fstp qword ptr [rsp + 018h] ; 01EB
weirdcubeifs_original_01ef:
    fld qword ptr [rsp + 030h] ; 01EF
    fmul qword ptr [r8 + 018h] ; 01F3
    fmul qword ptr [r8 - 048h] ; 01F6
    fld1 ; 01F9
    faddp st(1), st(0) ; 01FB
    fmul qword ptr [rsp + 050h] ; 01FD
    fmul qword ptr [r8 + 020h] ; 0201
    ftst ; 0204
    wait ; 0206
    fnstsw ax ; 0207
    shr ah, 1 ; 0209
    jae weirdcubeifs_original_0217 ; 020B
    fsubr qword ptr [rsp + 020h] ; 020D
    fstp qword ptr [rsp + 020h] ; 0211
    jmp weirdcubeifs_original_021f ; 0215
weirdcubeifs_original_0217:
    fadd qword ptr [rsp + 018h] ; 0217
    fstp qword ptr [rsp + 018h] ; 021B
weirdcubeifs_original_021f:
    fld qword ptr [rsp + 038h] ; 021F
    fmul qword ptr [r8 + 018h] ; 0223
    fmul qword ptr [r8 - 048h] ; 0226
    fld1 ; 0229
    faddp st(1), st(0) ; 022B
    fmul qword ptr [rsp + 040h] ; 022D
    fmul qword ptr [r8 + 020h] ; 0231
    ftst ; 0234
    wait ; 0236
    fnstsw ax ; 0237
    shr ah, 1 ; 0239
    jae weirdcubeifs_original_0247 ; 023B
    fsubr qword ptr [rsp + 020h] ; 023D
    fstp qword ptr [rsp + 020h] ; 0241
    jmp weirdcubeifs_original_024f ; 0245
weirdcubeifs_original_0247:
    fadd qword ptr [rsp + 018h] ; 0247
    fstp qword ptr [rsp + 018h] ; 024B
weirdcubeifs_original_024f:
    fnop ; 024F
    fld qword ptr [rsp + 018h] ; 0251
    fsqrt ; 0255
    fsqrt ; 0257
    fld qword ptr [rsp + 020h] ; 0259
    fsqrt ; 025D
    fsqrt ; 025F
    fchs ; 0261
    faddp st(1), st(0) ; 0263
    fmul qword ptr [r8 + 028h] ; 0265
    nop ; 0268
    fld qword ptr [rsp] ; 0269
    fmul st(0), st(0) ; 026C
    fld qword ptr [rsp + 8] ; 026E
    fmul st(0), st(0) ; 0272
    fld qword ptr [rsp + 010h] ; 0274
    fmul st(0), st(0) ; 0278
    faddp st(1), st(0) ; 027A
    faddp st(1), st(0) ; 027C
    fsqrt ; 027E
    fsub qword ptr [r8 - 010h] ; 0280
    fcom st(1) ; 0283
    wait ; 0285
    fnstsw ax ; 0286
    shr ah, 1 ; 0288
    jae weirdcubeifs_original_0290 ; 028A
    fstp st(0) ; 028C
    jmp weirdcubeifs_original_0292 ; 028E
weirdcubeifs_original_0290:
    fstp st(1) ; 0290
weirdcubeifs_original_0292:
    cmp dword ptr [r8 - 04ch], 0 ; 0292
    jne weirdcubeifs_original_029b ; 0296
    fdiv qword ptr [r8 - 020h] ; 0298
weirdcubeifs_original_029b:
    fstp qword ptr [rcx + 32] ; 029B
    fldcw word ptr [rsp + 104]
    add rsp, 120
    ret
WeirdCubeIFSOriginal_x87 ENDP


; Original _FoldingVar CODE; rcx=XYZ, rdx=expanded Double parameters.
FoldingVarOriginal_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    fld qword ptr [rcx + 16] ; 000B
    fld qword ptr [rcx + 8] ; 000D
    fld qword ptr [rcx] ; 000F
    fld st(0) ; 0011
    fmul qword ptr [rdx + 8] ; 0013
    fld st(2) ; 0016
    fmul qword ptr [rdx + 16] ; 0018
    faddp st(1), st(0) ; 001B
    fld st(3) ; 001D
    fmul qword ptr [rdx + 24] ; 001F
    faddp st(1), st(0) ; 0022
    fadd qword ptr [rdx + 0] ; 0024
    fld st(1) ; 0027
    fsub st(0), st(1) ; 0029
    fabs ; 002B
    fxch st(1) ; 002D
    fadd st(0), st(2) ; 002F
    fabs ; 0031
    fsubrp st(1), st(0) ; 0033
    fsub st(0), st(1) ; 0035
    fld st(1) ; 0037
    fmul qword ptr [rdx + 40] ; 0039
    fld st(3) ; 003C
    fmul qword ptr [rdx + 48] ; 003E
    faddp st(1), st(0) ; 0041
    fld st(4) ; 0043
    fmul qword ptr [rdx + 56] ; 0045
    faddp st(1), st(0) ; 0048
    fadd qword ptr [rdx + 32] ; 004A
    fld st(3) ; 004D
    fsub st(0), st(1) ; 004F
    fabs ; 0051
    fxch st(1) ; 0053
    fadd st(0), st(4) ; 0055
    fabs ; 0057
    fsubrp st(1), st(0) ; 0059
    fsub st(0), st(3) ; 005B
    fxch st(2) ; 005D
    fmul qword ptr [rdx + 72] ; 005F
    fxch st(3) ; 0062
    fmul qword ptr [rdx + 80] ; 0064
    faddp st(3), st(0) ; 0067
    fld st(3) ; 0069
    fmul qword ptr [rdx + 88] ; 006B
    fadd qword ptr [rdx + 64] ; 006E
    faddp st(3), st(0) ; 0071
    fld st(2) ; 0073
    fsub st(0), st(4) ; 0075
    fabs ; 0077
    fxch st(3) ; 0079
    fadd st(0), st(4) ; 007B
    fabs ; 007D
    fsubrp st(3), st(0) ; 007F
    fxch st(3) ; 0081
    fsubp st(2), st(0) ; 0083
    fstp qword ptr [rcx + 8] ; 0085
    fstp qword ptr [rcx + 16] ; 0087
    fstp qword ptr [rcx] ; 0089
FoldingVarOriginal_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
FoldingVarOriginal_x87 ENDP


; Original _poincare CODE; rcx=XYZ, rdx=expanded Double parameters.
PoincareOriginal_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    fld qword ptr [rcx + 16] ; 000A
    fld qword ptr [rcx + 8] ; 000C
    fld qword ptr [rcx] ; 000E
    fadd qword ptr [rdx + 0] ; 0010
    fstp qword ptr [rcx] ; 0013
    fadd qword ptr [rdx + 8] ; 0015
    fstp qword ptr [rcx + 8] ; 0018
    fadd qword ptr [rdx + 16] ; 001A
    fstp qword ptr [rcx + 16] ; 001D
    fld qword ptr [rcx] ; 001F
    fmul st(0), st(0) ; 0021
    fld qword ptr [rcx + 8] ; 0023
    fmul st(0), st(0) ; 0025
    faddp st(1), st(0) ; 0027
    fld qword ptr [rcx + 16] ; 0029
    fmul st(0), st(0) ; 002B
    faddp st(1), st(0) ; 002D
    fchs ; 002F
    fld qword ptr [rdx + 32] ; 0031
    faddp st(1), st(0) ; 0034
    fabs ; 0036
    fld qword ptr [rcx] ; 0038
    fabs ; 003A
    fdiv st(0), st(1) ; 003C
    fld qword ptr [rdx + 24] ; 003E
    fmulp st(1), st(0) ; 0041
    fsub qword ptr [rdx + 0] ; 0043
    fstp qword ptr [rcx] ; 0046
    fld qword ptr [rcx + 8] ; 0048
    fabs ; 004A
    fdiv st(0), st(1) ; 004C
    fld qword ptr [rdx + 24] ; 004E
    fmulp st(1), st(0) ; 0051
    fsub qword ptr [rdx + 8] ; 0053
    fstp qword ptr [rcx + 8] ; 0056
    fld qword ptr [rcx + 16] ; 0058
    fabs ; 005A
    fxch st(1) ; 005C
    fdivp st(1), st(0) ; 005E
    fld qword ptr [rdx + 24] ; 0060
    fmulp st(1), st(0) ; 0063
    fsub qword ptr [rdx + 16] ; 0065
    fstp qword ptr [rcx + 16] ; 0068
PoincareOriginal_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
PoincareOriginal_x87 ENDP


; Original lidinoidIFS: XYZ,VaryScale,Rout,OTrap; aligned numeric PVar.
lidinoidIFSOriginal_x87 PROC
    sub rsp, 88
    fnstcw word ptr [rsp + 80]
    mov word ptr [rsp + 82], 137Fh
    fldcw word ptr [rsp + 82]
    mov r8, rdx
    movupd xmm0, xmmword ptr [rcx] ; 0003
    movsd xmm1, qword ptr [rcx + 16] ; 0008
    mulpd xmm0, xmmword ptr [r8 - 020h] ; 000D
    mulsd xmm1, qword ptr [r8 - 020h] ; 0012
    addpd xmm0, xmmword ptr [r8 - 040h] ; 0017
    addsd xmm1, qword ptr [r8 - 030h] ; 001C
    cmp dword ptr [r8 - 04ch], 0 ; 0021
    je lidinoidIFSOriginal_x87_0040 ; 0025
    movsd xmm5, qword ptr [rcx + 24] ; 0027
    mulsd xmm5, qword ptr [r8 - 020h] ; 002C
    movupd xmmword ptr [rcx], xmm0 ; 0031
    movsd qword ptr [rcx + 16], xmm1 ; 0036
    movsd qword ptr [rcx + 24], xmm5 ; 003B
lidinoidIFSOriginal_x87_0040:
    movupd xmmword ptr [rsp], xmm0 ; 0040
    movsd qword ptr [rsp + 010h], xmm1 ; 0045
    nop ; 004B
    sub r8, 44
    fld qword ptr [rsp + 010h] ; 004F
    fld qword ptr [rsp + 8] ; 0053
    fld qword ptr [rsp] ; 0057
    fld st(0) ; 005A
    fmul dword ptr [r8 - 03ch] ; 005C
    fld st(2) ; 005F
    fmul dword ptr [r8 - 040h] ; 0061
    faddp st(1), st(0) ; 0064
    fld st(3) ; 0066
    fmul dword ptr [r8 - 044h] ; 0068
    faddp st(1), st(0) ; 006B
    fld st(1) ; 006D
    fmul dword ptr [r8 - 024h] ; 006F
    fld st(3) ; 0072
    fmul dword ptr [r8 - 028h] ; 0074
    faddp st(1), st(0) ; 0077
    fld st(4) ; 0079
    fmul dword ptr [r8 - 02ch] ; 007B
    faddp st(1), st(0) ; 007E
    fxch st(2) ; 0080
    fmul dword ptr [r8 - 030h] ; 0082
    fxch st(3) ; 0085
    fmul dword ptr [r8 - 034h] ; 0087
    faddp st(3), st(0) ; 008A
    fxch st(3) ; 008C
    fmul dword ptr [r8 - 038h] ; 008E
    faddp st(2), st(0) ; 0091
    fstp qword ptr [rsp] ; 0093
    fstp qword ptr [rsp + 8] ; 0096
    fstp qword ptr [rsp + 010h] ; 009A
    add r8, 44
    fld qword ptr [rsp + 010h] ; 00A1
    fmul qword ptr [r8 - 048h] ; 00A5
    fsincos ; 00A8
    fld qword ptr [rsp + 8] ; 00AA
    fmul qword ptr [r8 - 048h] ; 00AE
    fsincos ; 00B1
    fld qword ptr [rsp] ; 00B3
    fmul qword ptr [r8 - 048h] ; 00B6
    fsincos ; 00B9
    fstp qword ptr [rsp + 018h] ; 00BB
    fstp qword ptr [rsp + 020h] ; 00BF
    fstp qword ptr [rsp + 028h] ; 00C3
    fstp qword ptr [rsp + 030h] ; 00C7
    fstp qword ptr [rsp + 038h] ; 00CB
    fstp qword ptr [rsp + 040h] ; 00CF
    fld qword ptr [rsp + 010h] ; 00D3
    fsincos ; 00D7
    fld qword ptr [rsp + 8] ; 00D9
    fsincos ; 00DD
    fld qword ptr [rsp] ; 00DF
    fsincos ; 00E2
    fmul qword ptr [rsp + 020h] ; 00E4
    fmulp st(3), st(0) ; 00E8
    fmul qword ptr [rsp + 040h] ; 00EA
    fmulp st(3), st(0) ; 00EE
    fmul qword ptr [rsp + 030h] ; 00F0
    fmulp st(3), st(0) ; 00F4
    fld st(0) ; 00F6
    fsub st(0), st(2) ; 00F8
    fsub st(0), st(3) ; 00FA
    fmul qword ptr [r8 - 078h] ; 00FC
    fabs ; 00FF
    fstp qword ptr [rcx + 40] ; 0101
    faddp st(1), st(0) ; 0107
    faddp st(1), st(0) ; 0109
    fld qword ptr [rsp + 018h] ; 010B
    fmul qword ptr [rsp + 028h] ; 010F
    fld qword ptr [rsp + 028h] ; 0113
    fmul qword ptr [rsp + 038h] ; 0117
    fld qword ptr [rsp + 038h] ; 011B
    fmul qword ptr [rsp + 018h] ; 011F
    faddp st(1), st(0) ; 0123
    faddp st(1), st(0) ; 0125
    fchs ; 0127
    faddp st(1), st(0) ; 0129
    fmul qword ptr [r8] ; 012B
    fadd qword ptr [r8 + 8] ; 012D
    nop ; 0130
    fld qword ptr [rsp + 010h] ; 0131
    fabs ; 0135
    fld qword ptr [rsp + 8] ; 0137
    fabs ; 013B
    fcom st(1) ; 013D
    wait ; 013F
    fnstsw ax ; 0140
    shr ah, 1 ; 0142
    jae lidinoidIFSOriginal_x87_014a ; 0144
    fstp st(0) ; 0146
    jmp lidinoidIFSOriginal_x87_014c ; 0148
lidinoidIFSOriginal_x87_014a:
    fstp st(1) ; 014A
lidinoidIFSOriginal_x87_014c:
    fld qword ptr [rsp] ; 014C
    nop ; 014F
    fabs ; 0150
    fcom st(1) ; 0152
    wait ; 0154
    fnstsw ax ; 0155
    shr ah, 1 ; 0157
    jae lidinoidIFSOriginal_x87_015f ; 0159
    fstp st(0) ; 015B
    jmp lidinoidIFSOriginal_x87_0161 ; 015D
lidinoidIFSOriginal_x87_015f:
    fstp st(1) ; 015F
lidinoidIFSOriginal_x87_0161:
    fsub qword ptr [r8 - 010h] ; 0161
    fcom st(1) ; 0164
    wait ; 0166
    fnstsw ax ; 0167
    shr ah, 1 ; 0169
    jae lidinoidIFSOriginal_x87_0171 ; 016B
    fstp st(0) ; 016D
    jmp lidinoidIFSOriginal_x87_0173 ; 016F
lidinoidIFSOriginal_x87_0171:
    fstp st(1) ; 0171
lidinoidIFSOriginal_x87_0173:
    fdiv qword ptr [r8 - 048h] ; 0173
    cmp dword ptr [r8 - 04ch], 0 ; 0176
    jne lidinoidIFSOriginal_x87_017f ; 017A
    fdiv qword ptr [r8 - 020h] ; 017C
lidinoidIFSOriginal_x87_017f:
    fstp qword ptr [rcx + 32] ; 017F
    fldcw word ptr [rsp + 80]
    add rsp, 88
    ret
lidinoidIFSOriginal_x87 ENDP


; Original QuatPow CODE; rcx=XYZW, rdx=expanded parameters including internal C.
QuatPowOriginal_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    fld qword ptr [rcx + 24] ; 0010
    fmul st(0), st(0) ; 0013
    fld qword ptr [rcx + 8] ; 0015
    fmul st(0), st(0) ; 0017
    faddp st(1), st(0) ; 0019
    fld qword ptr [rcx + 16] ; 001B
    fmul st(0), st(0) ; 001D
    faddp st(1), st(0) ; 001F
    fsqrt ; 0021
    fld st(0) ; 0023
    ftst ; 0025
    fnstsw ax ; 0027
    fnop ; 0029
    sahf ; 002B
    ffree st(0) ; 002C
    fincstp ; 002E
    je QuatPowOriginal_x87_0083 ; 0030
    fld st(0) ; 0032
    fld qword ptr [rcx] ; 0034
    fpatan ; 0036
    fld st(1) ; 0038
    fdivp st(1), st(0) ; 003A
    fld qword ptr [rcx] ; 003C
    fmul st(0), st(0) ; 003E
    fld st(2) ; 0040
    fmul st(0), st(0) ; 0042
    faddp st(1), st(0) ; 0044
    fld st(0) ; 0046
    fld qword ptr [rdx + 72] ; 0048
    fcompp ; 004A
    fnstsw ax ; 004C
    and ah, 041h ; 004E
    je QuatPowOriginal_x87_0060 ; 0051
    fld st(0) ; 0053
    fldln2 ; 0055
    fxch st(1) ; 0057
    fyl2x ; 0059
    fmul qword ptr [rdx + 80] ; 005B
    jmp QuatPowOriginal_x87_0063 ; 005E
QuatPowOriginal_x87_0060:
    fld qword ptr [rdx + 88] ; 0060
QuatPowOriginal_x87_0063:
    fstp qword ptr [rcx] ; 0063
    fld qword ptr [rcx + 8] ; 0065
    fmul st(0), st(2) ; 0067
    fstp qword ptr [rcx + 8] ; 0069
    fld qword ptr [rcx + 16] ; 006B
    fmul st(0), st(2) ; 006D
    fstp qword ptr [rcx + 16] ; 006F
    fld qword ptr [rcx + 24] ; 0071
    fmul st(0), st(2) ; 0074
    fstp qword ptr [rcx + 24] ; 0076
    fstp st(0) ; 0079
    fstp st(0) ; 007B
    fstp st(0) ; 007D
    jmp QuatPowOriginal_x87_00a6 ; 007F
    nop ; 0081
    nop ; 0082
QuatPowOriginal_x87_0083:
    nop ; 0083
    fstp st(0) ; 0084
    fld qword ptr [rcx] ; 0086
    fld qword ptr [rdx + 72] ; 0088
    fcompp ; 008A
    fnstsw ax ; 008C
    and ah, 041h ; 008E
    je QuatPowOriginal_x87_00a1 ; 0091
    fld qword ptr [rcx] ; 0093
    fldln2 ; 0095
    fxch st(1) ; 0097
    fyl2x ; 0099
    fstp qword ptr [rcx] ; 009B
    jmp QuatPowOriginal_x87_00a6 ; 009D
    nop ; 009F
    nop ; 00A0
QuatPowOriginal_x87_00a1:
    fld qword ptr [rdx + 88] ; 00A1
    fstp qword ptr [rcx] ; 00A4
QuatPowOriginal_x87_00a6:
    fnop ; 00A6
    fnop ; 00A8
    fld qword ptr [rcx + 24] ; 00AA
    fld qword ptr [rdx + 56] ; 00AD
    fmulp st(1), st(0) ; 00B0
    fstp qword ptr [rcx + 24] ; 00B2
    fld qword ptr [rcx + 16] ; 00B5
    fld qword ptr [rdx + 48] ; 00B7
    fmulp st(1), st(0) ; 00BA
    fstp qword ptr [rcx + 16] ; 00BC
    fld qword ptr [rcx + 8] ; 00BE
    fld qword ptr [rdx + 40] ; 00C0
    fmulp st(1), st(0) ; 00C3
    fstp qword ptr [rcx + 8] ; 00C5
    fld qword ptr [rcx] ; 00C7
    fld qword ptr [rdx + 32] ; 00C9
    fmulp st(1), st(0) ; 00CC
    fstp qword ptr [rcx] ; 00CE
    fnop ; 00D0
    fnop ; 00D2
    fld qword ptr [rcx] ; 00D4
    nop ; 00D6
    fmul qword ptr [rdx + 0] ; 00D7
    nop ; 00DA
    nop ; 00DB
    fld qword ptr [rcx + 8] ; 00DC
    nop ; 00DE
    fmul qword ptr [rdx + 8] ; 00DF
    fchs ; 00E2
    faddp st(1), st(0) ; 00E4
    fld qword ptr [rcx + 16] ; 00E6
    nop ; 00E8
    fmul qword ptr [rdx + 16] ; 00E9
    fchs ; 00EC
    faddp st(1), st(0) ; 00EE
    fld qword ptr [rcx + 24] ; 00F0
    fmul qword ptr [rdx + 24] ; 00F3
    fchs ; 00F6
    faddp st(1), st(0) ; 00F8
    fnop ; 00FA
    fld qword ptr [rcx] ; 00FC
    nop ; 00FE
    fmul qword ptr [rdx + 8] ; 00FF
    nop ; 0102
    nop ; 0103
    fld qword ptr [rcx + 8] ; 0104
    nop ; 0106
    fmul qword ptr [rdx + 0] ; 0107
    nop ; 010A
    nop ; 010B
    faddp st(1), st(0) ; 010C
    fld qword ptr [rcx + 16] ; 010E
    nop ; 0110
    fmul qword ptr [rdx + 24] ; 0111
    nop ; 0114
    nop ; 0115
    faddp st(1), st(0) ; 0116
    fld qword ptr [rcx + 24] ; 0118
    fmul qword ptr [rdx + 16] ; 011B
    fchs ; 011E
    faddp st(1), st(0) ; 0120
    fnop ; 0122
    fld qword ptr [rcx] ; 0124
    nop ; 0126
    fmul qword ptr [rdx + 16] ; 0127
    nop ; 012A
    nop ; 012B
    fld qword ptr [rcx + 8] ; 012C
    nop ; 012E
    fmul qword ptr [rdx + 24] ; 012F
    fchs ; 0132
    faddp st(1), st(0) ; 0134
    fld qword ptr [rcx + 16] ; 0136
    nop ; 0138
    fmul qword ptr [rdx + 0] ; 0139
    nop ; 013C
    nop ; 013D
    faddp st(1), st(0) ; 013E
    fld qword ptr [rcx + 24] ; 0140
    fmul qword ptr [rdx + 8] ; 0143
    nop ; 0146
    nop ; 0147
    faddp st(1), st(0) ; 0148
    fnop ; 014A
    fld qword ptr [rcx] ; 014C
    nop ; 014E
    fmul qword ptr [rdx + 24] ; 014F
    nop ; 0152
    nop ; 0153
    fld qword ptr [rcx + 8] ; 0154
    nop ; 0156
    fmul qword ptr [rdx + 16] ; 0157
    nop ; 015A
    nop ; 015B
    faddp st(1), st(0) ; 015C
    fld qword ptr [rcx + 16] ; 015E
    nop ; 0160
    fmul qword ptr [rdx + 8] ; 0161
    fchs ; 0164
    faddp st(1), st(0) ; 0166
    fld qword ptr [rcx + 24] ; 0168
    fmul qword ptr [rdx + 0] ; 016B
    nop ; 016E
    nop ; 016F
    faddp st(1), st(0) ; 0170
    fnop ; 0172
    nop ; 0174
    fstp qword ptr [rcx + 24] ; 0175
    fstp qword ptr [rcx + 16] ; 0178
    fstp qword ptr [rcx + 8] ; 017A
    fstp qword ptr [rcx] ; 017C
    fld qword ptr [rcx + 24] ; 017E
    fmul st(0), st(0) ; 0181
    fld qword ptr [rcx + 8] ; 0183
    fmul st(0), st(0) ; 0185
    faddp st(1), st(0) ; 0187
    fld qword ptr [rcx + 16] ; 0189
    fmul st(0), st(0) ; 018B
    faddp st(1), st(0) ; 018D
    fsqrt ; 018F
    fld st(0) ; 0191
    fsincos ; 0193
    fnop ; 0195
    fld qword ptr [rcx] ; 0197
    fldl2e ; 0199
    fmulp st(1), st(0) ; 019B
    fld1 ; 019D
    fld st(1) ; 019F
    fprem ; 01A1
    f2xm1 ; 01A3
    faddp st(1), st(0) ; 01A5
    fscale ; 01A7
    fxch st(1) ; 01A9
    fstp st(0) ; 01AB
    fld st(0) ; 01AD
    fmul st(0), st(2) ; 01AF
    fstp qword ptr [rcx] ; 01B1
    fmul st(0), st(2) ; 01B3
    fld st(3) ; 01B5
    fld qword ptr [rdx + 72] ; 01B7
    fcompp ; 01B9
    fnstsw ax ; 01BB
    and ah, 041h ; 01BD
    je QuatPowOriginal_x87_01c4 ; 01C0
    fdiv st(0), st(3) ; 01C2
QuatPowOriginal_x87_01c4:
    fld qword ptr [rcx + 8] ; 01C4
    fmul st(0), st(1) ; 01C6
    fstp qword ptr [rcx + 8] ; 01C8
    fld qword ptr [rcx + 16] ; 01CA
    fmul st(0), st(1) ; 01CC
    fstp qword ptr [rcx + 16] ; 01CE
    fld qword ptr [rcx + 24] ; 01D0
    fmul st(0), st(1) ; 01D3
    fstp qword ptr [rcx + 24] ; 01D5
    fstp st(0) ; 01D8
    fstp st(0) ; 01DA
    fstp st(0) ; 01DC
    fstp st(0) ; 01DE
    fnop ; 01E0
    fnop ; 01E2
    fld qword ptr [rcx + 16] ; 01E4
    fld qword ptr [rcx + 8] ; 01E6
    fld qword ptr [rcx] ; 01E8
    fld qword ptr [rdx + 96] ; 01EA
    fmul qword ptr [rdx + 64] ; 01ED
    faddp st(1), st(0) ; 01F0
    fstp qword ptr [rcx] ; 01F2
    fld qword ptr [rdx + 104] ; 01F4
    fmul qword ptr [rdx + 64] ; 01F7
    faddp st(1), st(0) ; 01FA
    fstp qword ptr [rcx + 8] ; 01FC
    fld qword ptr [rdx + 112] ; 01FE
    fmul qword ptr [rdx + 64] ; 0201
    faddp st(1), st(0) ; 0204
    fstp qword ptr [rcx + 16] ; 0206
    nop ; 0208
QuatPowOriginal_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
QuatPowOriginal_x87 ENDP


; Original RuckerBrot1 CODE; rcx=XYZW, rdx=expanded parameters including internal C.
RuckerBrot1Original_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    fld qword ptr [rcx] ; 0010
    fmul st(0), st(0) ; 0012
    fld qword ptr [rcx + 8] ; 0014
    fmul st(0), st(0) ; 0016
    fld qword ptr [rcx + 16] ; 0018
    fmul st(0), st(0) ; 001A
    faddp st(1), st(0) ; 001C
    faddp st(1), st(0) ; 001E
    fld qword ptr [rcx] ; 0020
    fmul st(0), st(0) ; 0022
    fld qword ptr [rcx + 16] ; 0024
    fmul st(0), st(0) ; 0026
    faddp st(1), st(0) ; 0028
    fsqrt ; 002A
    fadd qword ptr [rdx + 32] ; 002C
    fdivp st(1), st(0) ; 002E
    fnop ; 0030
    fld qword ptr [rcx] ; 0032
    fmul st(0), st(0) ; 0034
    fld qword ptr [rcx + 8] ; 0036
    fmul st(0), st(0) ; 0038
    fchs ; 003A
    faddp st(1), st(0) ; 003C
    fld qword ptr [rcx] ; 003E
    fmul st(0), st(0) ; 0040
    fld qword ptr [rcx + 8] ; 0042
    fmul st(0), st(0) ; 0044
    faddp st(1), st(0) ; 0046
    fadd qword ptr [rdx + 32] ; 0048
    fdivp st(1), st(0) ; 004A
    fmul st(0), st(1) ; 004C
    fld qword ptr [rcx] ; 004E
    fabs ; 0050
    fmulp st(1), st(0) ; 0052
    fmul qword ptr [rdx + 0] ; 0054
    fadd qword ptr [rdx + 40] ; 0057
    fnop ; 005A
    fld qword ptr [rcx] ; 005C
    fmul qword ptr [rcx + 8] ; 005E
    fld qword ptr [rcx] ; 0060
    fabs ; 0062
    fmulp st(1), st(0) ; 0064
    fld qword ptr [rcx] ; 0066
    fmul st(0), st(0) ; 0068
    fld qword ptr [rcx + 8] ; 006A
    fmul st(0), st(0) ; 006C
    faddp st(1), st(0) ; 006E
    fadd qword ptr [rdx + 32] ; 0070
    fdivp st(1), st(0) ; 0072
    fmul st(0), st(2) ; 0074
    fmul qword ptr [rdx + 8] ; 0076
    fadd qword ptr [rdx + 48] ; 0079
    fnop ; 007C
    fld qword ptr [rcx + 16] ; 007E
    nop ; 0080
    nop ; 0081
    fld qword ptr [rcx] ; 0082
    fmul st(0), st(0) ; 0084
    fld qword ptr [rcx + 8] ; 0086
    fmul st(0), st(0) ; 0088
    fld qword ptr [rcx + 16] ; 008A
    fmul st(0), st(0) ; 008C
    faddp st(1), st(0) ; 008E
    faddp st(1), st(0) ; 0090
    fsqrt ; 0092
    fmulp st(1), st(0) ; 0094
    nop ; 0096
    nop ; 0097
    fmul qword ptr [rdx + 16] ; 0098
    fld qword ptr [rdx + 56] ; 009B
    fabs ; 009E
    nop ; 00A0
    nop ; 00A1
    nop ; 00A2
    nop ; 00A3
    fmul qword ptr [rdx + 24] ; 00A4
    faddp st(1), st(0) ; 00A7
    fnop ; 00A9
    fstp qword ptr [rcx + 16] ; 00AB
    fstp qword ptr [rcx + 8] ; 00AD
    fstp qword ptr [rcx] ; 00AF
    fstp st(0) ; 00B1
    nop ; 00B3
RuckerBrot1Original_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
RuckerBrot1Original_x87 ENDP


; Original _invbisp CODE, including cancellation and Double spills.
InvBispOriginal_x87 PROC
    sub rsp, 40
    fnstcw word ptr [rsp + 32]
    mov word ptr [rsp + 34], 137Fh
    fldcw word ptr [rsp + 34]
    fld qword ptr [rcx] ; 0015
    fmul st(0), st(0) ; 0017
    fld qword ptr [rcx + 8] ; 0019
    fmul st(0), st(0) ; 001B
    fld qword ptr [rcx + 16] ; 001D
    fmul st(0), st(0) ; 001F
    faddp st(1), st(0) ; 0021
    faddp st(1), st(0) ; 0023
    fst qword ptr [rsp] ; 0025
    nop ; 0028
    fld qword ptr [rdx] ; 0029
    fmul st(0), st(0) ; 002C
    faddp st(1), st(0) ; 002E
    fmul st(0), st(0) ; 0030
    fld qword ptr [rdx] ; 0032
    fmul qword ptr [rcx + 16] ; 0035
    fadd st(0), st(0) ; 0037
    fmul st(0), st(0) ; 0039
    fchs ; 003B
    faddp st(1), st(0) ; 003D
    fabs ; 003F
    fsqrt ; 0041
    fstp qword ptr [rsp + 8] ; 0043
    nop ; 0046
    fld qword ptr [rsp] ; 0047
    fld qword ptr [rdx] ; 004A
    fmul st(0), st(0) ; 004D
    fchs ; 004F
    faddp st(1), st(0) ; 0051
    fld qword ptr [rdx] ; 0053
    fmul qword ptr [rcx + 16] ; 0056
    fadd st(0), st(0) ; 0058
    fstp qword ptr [rsp + 16] ; 005A
    fstp qword ptr [rsp] ; 005D
    nop ; 0060
    fld qword ptr [rcx + 8] ; 0061
    fld qword ptr [rcx] ; 0063
    fpatan ; 0065
    fstp qword ptr [rcx + 16] ; 0067
    nop ; 0069
    fld qword ptr [rsp + 8] ; 006A
    fmul st(0), st(0) ; 006D
    fld qword ptr [rsp] ; 006F
    fmul st(0), st(0) ; 0072
    fsubp st(1), st(0) ; 0074
    fsqrt ; 0076
    fld qword ptr [rsp] ; 0078
    fpatan ; 007B
    fsubr qword ptr [rdx + 8] ; 007D
    fstp qword ptr [rcx] ; 007F
    nop ; 0081
    fld qword ptr [rsp + 16] ; 0082
    fdiv qword ptr [rsp + 8] ; 0085
    fld st(0) ; 0088
    fmul st(0), st(0) ; 008A
    fld1 ; 008C
    faddp st(1), st(0) ; 008E
    fsqrt ; 0090
    faddp st(1), st(0) ; 0092
    fldl2e ; 0094
    fxch st(1) ; 0096
    fyl2x ; 0098
    fstp qword ptr [rcx + 8] ; 009A
    fldcw word ptr [rsp + 32]
    add rsp, 40
    ret
InvBispOriginal_x87 ENDP


; Original borgIFS CODE; aligned PVar and original local stores.
borgIFSOriginal_x87 PROC
    sub rsp, 88
    fnstcw word ptr [rsp + 80]
    mov word ptr [rsp + 82], 137Fh
    fldcw word ptr [rsp + 82]
    mov r8, rdx
    movupd xmm0, xmmword ptr [rcx] ; 0003
    movsd xmm1, qword ptr [rcx + 16] ; 0008
    mulpd xmm0, xmmword ptr [r8 - 020h] ; 000D
    mulsd xmm1, qword ptr [r8 - 020h] ; 0012
    addpd xmm0, xmmword ptr [r8 - 040h] ; 0017
    addsd xmm1, qword ptr [r8 - 030h] ; 001C
    cmp dword ptr [r8 - 04ch], 0 ; 0021
    je borgIFSOriginal_x87_0040 ; 0025
    movsd xmm5, qword ptr [rcx + 24] ; 0027
    mulsd xmm5, qword ptr [r8 - 020h] ; 002C
    movupd xmmword ptr [rcx], xmm0 ; 0031
    movsd qword ptr [rcx + 16], xmm1 ; 0036
    movsd qword ptr [rcx + 24], xmm5 ; 003B
borgIFSOriginal_x87_0040:
    movupd xmmword ptr [rsp], xmm0 ; 0040
    movsd qword ptr [rsp + 010h], xmm1 ; 0045
    nop ; 004B
    add r8, -02ch
    fld qword ptr [rsp + 010h] ; 004F
    fld qword ptr [rsp + 8] ; 0053
    fld qword ptr [rsp] ; 0057
    fld st(0) ; 005A
    fmul dword ptr [r8 - 03ch] ; 005C
    fld st(2) ; 005F
    fmul dword ptr [r8 - 040h] ; 0061
    faddp st(1), st(0) ; 0064
    fld st(3) ; 0066
    fmul dword ptr [r8 - 044h] ; 0068
    faddp st(1), st(0) ; 006B
    fld st(1) ; 006D
    fmul dword ptr [r8 - 024h] ; 006F
    fld st(3) ; 0072
    fmul dword ptr [r8 - 028h] ; 0074
    faddp st(1), st(0) ; 0077
    fld st(4) ; 0079
    fmul dword ptr [r8 - 02ch] ; 007B
    faddp st(1), st(0) ; 007E
    fxch st(2) ; 0080
    fmul dword ptr [r8 - 030h] ; 0082
    fxch st(3) ; 0085
    fmul dword ptr [r8 - 034h] ; 0087
    faddp st(3), st(0) ; 008A
    fxch st(3) ; 008C
    fmul dword ptr [r8 - 038h] ; 008E
    faddp st(2), st(0) ; 0091
    fstp qword ptr [rsp] ; 0093
    fstp qword ptr [rsp + 8] ; 0096
    fstp qword ptr [rsp + 010h] ; 009A
    add r8, 02ch
    fld qword ptr [rsp] ; 00A1
    fmul qword ptr [rsp + 8] ; 00A4
    fmul qword ptr [r8 - 048h] ; 00A8
    fsin ; 00AB
    fld qword ptr [rsp + 010h] ; 00AD
    fmul qword ptr [rsp + 8] ; 00B1
    fmul qword ptr [r8 - 048h] ; 00B5
    fsin ; 00B8
    fld qword ptr [rsp + 010h] ; 00BA
    fmul qword ptr [rsp] ; 00BE
    fmul qword ptr [r8 - 048h] ; 00C1
    fsin ; 00C4
    fld st(0) ; 00C6
    fsub st(0), st(2) ; 00C8
    fsub st(0), st(3) ; 00CA
    fmul qword ptr [r8 - 078h] ; 00CC
    fabs ; 00CF
    fstp qword ptr [rcx + 40] ; 00D1
    faddp st(1), st(0) ; 00D7
    faddp st(1), st(0) ; 00D9
    fld qword ptr [rsp + 010h] ; 00DB
    fabs ; 00DF
    fld qword ptr [rsp + 8] ; 00E1
    fabs ; 00E5
    fcom st(1) ; 00E7
    wait ; 00E9
    fnstsw ax ; 00EA
    shr ah, 1 ; 00EC
    jae borgIFSOriginal_x87_00f4 ; 00EE
    fstp st(0) ; 00F0
    jmp borgIFSOriginal_x87_00f6 ; 00F2
borgIFSOriginal_x87_00f4:
    fstp st(1) ; 00F4
borgIFSOriginal_x87_00f6:
    fld qword ptr [rsp] ; 00F6
    nop ; 00F9
    fabs ; 00FA
    fcom st(1) ; 00FC
    wait ; 00FE
    fnstsw ax ; 00FF
    shr ah, 1 ; 0101
    jae borgIFSOriginal_x87_0109 ; 0103
    fstp st(0) ; 0105
    jmp borgIFSOriginal_x87_010b ; 0107
borgIFSOriginal_x87_0109:
    fstp st(1) ; 0109
borgIFSOriginal_x87_010b:
    fsub qword ptr [r8 - 010h] ; 010B
    fcom st(1) ; 010E
    wait ; 0110
    fnstsw ax ; 0111
    shr ah, 1 ; 0113
    jae borgIFSOriginal_x87_011b ; 0115
    fstp st(0) ; 0117
    jmp borgIFSOriginal_x87_011d ; 0119
borgIFSOriginal_x87_011b:
    fstp st(1) ; 011B
borgIFSOriginal_x87_011d:
    fdiv qword ptr [r8 - 048h] ; 011D
    cmp dword ptr [r8 - 04ch], 0 ; 0120
    jne borgIFSOriginal_x87_0129 ; 0124
    fdiv qword ptr [r8 - 020h] ; 0126
borgIFSOriginal_x87_0129:
    fstp qword ptr [rcx + 32] ; 0129
    fldcw word ptr [rsp + 80]
    add rsp, 88
    ret
borgIFSOriginal_x87 ENDP


; Original DecoCubeIFS CODE; aligned PVar and original local stores.
DecoCubeIFSOriginal_x87 PROC
    sub rsp, 88
    fnstcw word ptr [rsp + 80]
    mov word ptr [rsp + 82], 137Fh
    fldcw word ptr [rsp + 82]
    mov r8, rdx
    movupd xmm0, xmmword ptr [rcx] ; 0003
    movsd xmm1, qword ptr [rcx + 16] ; 0008
    mulpd xmm0, xmmword ptr [r8 - 020h] ; 000D
    mulsd xmm1, qword ptr [r8 - 020h] ; 0012
    addpd xmm0, xmmword ptr [r8 - 040h] ; 0017
    addsd xmm1, qword ptr [r8 - 030h] ; 001C
    cmp dword ptr [r8 - 04ch], 0 ; 0021
    je DecoCubeIFSOriginal_x87_0040 ; 0025
    movsd xmm5, qword ptr [rcx + 24] ; 0027
    mulsd xmm5, qword ptr [r8 - 020h] ; 002C
    movupd xmmword ptr [rcx], xmm0 ; 0031
    movsd qword ptr [rcx + 16], xmm1 ; 0036
    movsd qword ptr [rcx + 24], xmm5 ; 003B
DecoCubeIFSOriginal_x87_0040:
    movupd xmmword ptr [rsp], xmm0 ; 0040
    movsd qword ptr [rsp + 010h], xmm1 ; 0045
    fld qword ptr [rsp] ; 004B
    fmul st(0), st(0) ; 004E
    fstp qword ptr [rsp] ; 0050
    fld qword ptr [rsp + 8] ; 0053
    fmul st(0), st(0) ; 0057
    fstp qword ptr [rsp + 8] ; 0059
    fld qword ptr [rsp + 010h] ; 005D
    fmul st(0), st(0) ; 0061
    fstp qword ptr [rsp + 010h] ; 0063
    fld qword ptr [rsp] ; 0067
    fadd qword ptr [rsp + 8] ; 006A
    fsub qword ptr [r8 - 010h] ; 006E
    fmul st(0), st(0) ; 0071
    fld qword ptr [rsp + 010h] ; 0073
    fsub qword ptr [r8 - 048h] ; 0077
    fmul st(0), st(0) ; 007A
    faddp st(1), st(0) ; 007C
    fld qword ptr [rsp] ; 007E
    fadd qword ptr [rsp + 010h] ; 0081
    fsub qword ptr [r8 - 010h] ; 0085
    fmul st(0), st(0) ; 0088
    fld qword ptr [rsp + 8] ; 008A
    fsub qword ptr [r8 - 048h] ; 008E
    fmul st(0), st(0) ; 0091
    faddp st(1), st(0) ; 0093
    fld qword ptr [rsp + 010h] ; 0095
    fadd qword ptr [rsp + 8] ; 0099
    fsub qword ptr [r8 - 010h] ; 009D
    fmul st(0), st(0) ; 00A0
    fld qword ptr [rsp] ; 00A2
    fsub qword ptr [r8 - 048h] ; 00A5
    fmul st(0), st(0) ; 00A8
    faddp st(1), st(0) ; 00AA
    fmulp st(1), st(0) ; 00AC
    fmulp st(1), st(0) ; 00AE
    fsqrt ; 00B0
    fsqrt ; 00B2
    fsub qword ptr [r8 - 054h] ; 00B4
    nop ; 00B7
    fld qword ptr [r8] ; 00B8
    fcomp st(1) ; 00BA
    wait ; 00BC
    fnstsw ax ; 00BD
    shr ah, 1 ; 00BF
    jae DecoCubeIFSOriginal_x87_00d2 ; 00C1
    fstp st(0) ; 00C3
    fld qword ptr [rsp] ; 00C5
    fadd qword ptr [rsp + 8] ; 00C8
    fadd qword ptr [rsp + 010h] ; 00CC
    fsqrt ; 00D0
DecoCubeIFSOriginal_x87_00d2:
    nop ; 00D2
    cmp dword ptr [r8 - 04ch], 0 ; 00D3
    jne DecoCubeIFSOriginal_x87_00dc ; 00D7
    fdiv qword ptr [r8 - 020h] ; 00D9
DecoCubeIFSOriginal_x87_00dc:
    fstp qword ptr [rcx + 32] ; 00DC
    fldcw word ptr [rsp + 80]
    add rsp, 88
    ret
DecoCubeIFSOriginal_x87 ENDP


; Original gumdropIFS CODE; aligned PVar and original local stores.
gumdropIFSOriginal_x87 PROC
    sub rsp, 88
    fnstcw word ptr [rsp + 80]
    mov word ptr [rsp + 82], 137Fh
    fldcw word ptr [rsp + 82]
    mov r8, rdx
    movupd xmm0, xmmword ptr [rcx] ; 0003
    movsd xmm1, qword ptr [rcx + 16] ; 0008
    mulpd xmm0, xmmword ptr [r8 - 020h] ; 000D
    mulsd xmm1, qword ptr [r8 - 020h] ; 0012
    addpd xmm0, xmmword ptr [r8 - 040h] ; 0017
    addsd xmm1, qword ptr [r8 - 030h] ; 001C
    cmp dword ptr [r8 - 04ch], 0 ; 0021
    je gumdropIFSOriginal_x87_0040 ; 0025
    movsd xmm5, qword ptr [rcx + 24] ; 0027
    mulsd xmm5, qword ptr [r8 - 020h] ; 002C
    movupd xmmword ptr [rcx], xmm0 ; 0031
    movsd qword ptr [rcx + 16], xmm1 ; 0036
    movsd qword ptr [rcx + 24], xmm5 ; 003B
gumdropIFSOriginal_x87_0040:
    movupd xmmword ptr [rsp], xmm0 ; 0040
    movsd qword ptr [rsp + 010h], xmm1 ; 0045
    nop ; 004B
    add r8, -02ch
    fld qword ptr [rsp + 010h] ; 004F
    fld qword ptr [rsp + 8] ; 0053
    fld qword ptr [rsp] ; 0057
    fld st(0) ; 005A
    fmul dword ptr [r8 - 03ch] ; 005C
    fld st(2) ; 005F
    fmul dword ptr [r8 - 040h] ; 0061
    faddp st(1), st(0) ; 0064
    fld st(3) ; 0066
    fmul dword ptr [r8 - 044h] ; 0068
    faddp st(1), st(0) ; 006B
    fld st(1) ; 006D
    fmul dword ptr [r8 - 024h] ; 006F
    fld st(3) ; 0072
    fmul dword ptr [r8 - 028h] ; 0074
    faddp st(1), st(0) ; 0077
    fld st(4) ; 0079
    fmul dword ptr [r8 - 02ch] ; 007B
    faddp st(1), st(0) ; 007E
    fxch st(2) ; 0080
    fmul dword ptr [r8 - 030h] ; 0082
    fxch st(3) ; 0085
    fmul dword ptr [r8 - 034h] ; 0087
    faddp st(3), st(0) ; 008A
    fxch st(3) ; 008C
    fmul dword ptr [r8 - 038h] ; 008E
    faddp st(2), st(0) ; 0091
    fmul st(0), st(0) ; 0093
    fstp qword ptr [rsp] ; 0095
    fmul st(0), st(0) ; 0098
    fstp qword ptr [rsp + 8] ; 009A
    fmul st(0), st(0) ; 009E
    fstp qword ptr [rsp + 010h] ; 00A0
    add r8, 02ch
    nop ; 00A7
    fld1 ; 00A8
    fld qword ptr [r8 - 010h] ; 00AA
    fabs ; 00AD
    fst qword ptr [rsp + 018h] ; 00AF
    fld qword ptr [r8 - 048h] ; 00B3
    fabs ; 00B6
    fst qword ptr [rsp + 020h] ; 00B8
    faddp st(1), st(0) ; 00BC
    faddp st(1), st(0) ; 00BE
    fld1 ; 00C0
    fdivrp st(1), st(0) ; 00C2
    fstp qword ptr [rsp + 028h] ; 00C4
    fld qword ptr [rsp + 010h] ; 00C8
    fadd qword ptr [rsp + 8] ; 00CC
    fmul st(0), st(0) ; 00D0
    fld qword ptr [rsp] ; 00D2
    fmul st(0), st(0) ; 00D5
    faddp st(1), st(0) ; 00D7
    fld qword ptr [rsp + 010h] ; 00D9
    fadd qword ptr [rsp + 8] ; 00DD
    fmul qword ptr [rsp] ; 00E1
    fld qword ptr [rsp + 018h] ; 00E4
    nop ; 00E8
    fmul st(1), st(0) ; 00E9
    faddp st(1), st(0) ; 00EB
    faddp st(1), st(0) ; 00ED
    fmul qword ptr [rsp + 028h] ; 00EF
    fsqrt ; 00F3
    fsqrt ; 00F5
    fld qword ptr [rsp + 010h] ; 00F7
    fadd qword ptr [rsp + 8] ; 00FB
    fadd qword ptr [rsp] ; 00FF
    fmul qword ptr [rsp + 020h] ; 0102
    fmul qword ptr [rsp + 028h] ; 0106
    fsqrt ; 010A
    fsqrt ; 010C
    fchs ; 010E
    faddp st(1), st(0) ; 0110
    fmul qword ptr [r8] ; 0112
    nop ; 0114
    cmp dword ptr [r8 - 04ch], 0 ; 0115
    jne gumdropIFSOriginal_x87_011e ; 0119
    fdiv qword ptr [r8 - 020h] ; 011B
gumdropIFSOriginal_x87_011e:
    fstp qword ptr [rcx + 32] ; 011E
    fldcw word ptr [rsp + 80]
    add rsp, 88
    ret
gumdropIFSOriginal_x87 ENDP


; Original LimpetIFS CODE; aligned PVar and original local stores.
LimpetIFSOriginal_x87 PROC
    sub rsp, 88
    fnstcw word ptr [rsp + 80]
    mov word ptr [rsp + 82], 137Fh
    fldcw word ptr [rsp + 82]
    mov r8, rdx
    movupd xmm0, xmmword ptr [rcx] ; 0003
    movsd xmm1, qword ptr [rcx + 16] ; 0008
    mulpd xmm0, xmmword ptr [r8 - 020h] ; 000D
    mulsd xmm1, qword ptr [r8 - 020h] ; 0012
    addpd xmm0, xmmword ptr [r8 - 040h] ; 0017
    addsd xmm1, qword ptr [r8 - 030h] ; 001C
    cmp dword ptr [r8 - 04ch], 0 ; 0021
    je LimpetIFSOriginal_x87_0040 ; 0025
    movsd xmm5, qword ptr [rcx + 24] ; 0027
    mulsd xmm5, qword ptr [r8 - 020h] ; 002C
    movupd xmmword ptr [rcx], xmm0 ; 0031
    movsd qword ptr [rcx + 16], xmm1 ; 0036
    movsd qword ptr [rcx + 24], xmm5 ; 003B
LimpetIFSOriginal_x87_0040:
    movupd xmmword ptr [rsp], xmm0 ; 0040
    movsd qword ptr [rsp + 010h], xmm1 ; 0045
    add r8, -034h
    fld qword ptr [rsp + 010h] ; 004E
    fld qword ptr [rsp + 8] ; 0052
    fld qword ptr [rsp] ; 0056
    fld st(0) ; 0059
    fmul dword ptr [r8 - 03ch] ; 005B
    fld st(2) ; 005E
    fmul dword ptr [r8 - 040h] ; 0060
    faddp st(1), st(0) ; 0063
    fld st(3) ; 0065
    fmul dword ptr [r8 - 044h] ; 0067
    faddp st(1), st(0) ; 006A
    fld st(1) ; 006C
    fmul dword ptr [r8 - 024h] ; 006E
    fld st(3) ; 0071
    fmul dword ptr [r8 - 028h] ; 0073
    faddp st(1), st(0) ; 0076
    fld st(4) ; 0078
    fmul dword ptr [r8 - 02ch] ; 007A
    faddp st(1), st(0) ; 007D
    fxch st(2) ; 007F
    fmul dword ptr [r8 - 030h] ; 0081
    fxch st(3) ; 0084
    fmul dword ptr [r8 - 034h] ; 0086
    faddp st(3), st(0) ; 0089
    fxch st(3) ; 008B
    fmul dword ptr [r8 - 038h] ; 008D
    faddp st(2), st(0) ; 0090
    fstp qword ptr [rsp] ; 0092
    fstp qword ptr [rsp + 8] ; 0095
    fstp qword ptr [rsp + 010h] ; 0099
    add r8, 034h
    nop ; 00A0
    fld qword ptr [rsp] ; 00A1
    fmul st(0), st(0) ; 00A4
    fld qword ptr [rsp + 8] ; 00A6
    fmul st(0), st(0) ; 00AA
    faddp st(1), st(0) ; 00AC
    fsqrt ; 00AE
    fstp qword ptr [rsp] ; 00B0
    nop ; 00B3
    fld qword ptr [r8 - 010h] ; 00B4
    fld qword ptr [rsp] ; 00B7
    fmul qword ptr [rsp + 010h] ; 00BA
    faddp st(1), st(0) ; 00BE
    nop ; 00C0
    nop ; 00C1
    fld qword ptr [rsp] ; 00C2
    fmul qword ptr [r8 - 054h] ; 00C5
    faddp st(1), st(0) ; 00C8
    fmul st(0), st(0) ; 00CA
    fld qword ptr [rsp + 010h] ; 00CC
    fmul st(0), st(0) ; 00D0
    faddp st(1), st(0) ; 00D2
    fsqrt ; 00D4
    fsqrt ; 00D6
    fsub qword ptr [r8 - 048h] ; 00D8
    fld qword ptr [rsp] ; 00DB
    fsub qword ptr [r8 - 048h] ; 00DE
    fsub qword ptr [r8 - 048h] ; 00E1
    fsub qword ptr [r8 - 048h] ; 00E4
    fcom st(1) ; 00E7
    wait ; 00E9
    fnstsw ax ; 00EA
    shr ah, 1 ; 00EC
    jae LimpetIFSOriginal_x87_00f4 ; 00EE
    fstp st(0) ; 00F0
    jmp LimpetIFSOriginal_x87_00f6 ; 00F2
LimpetIFSOriginal_x87_00f4:
    fstp st(1) ; 00F4
LimpetIFSOriginal_x87_00f6:
    fld qword ptr [rsp + 010h] ; 00F6
    fabs ; 00FA
    fsub qword ptr [r8 - 048h] ; 00FC
    fsub qword ptr [r8 - 048h] ; 00FF
    fsub qword ptr [r8 - 048h] ; 0102
    fcom st(1) ; 0105
    wait ; 0107
    fnstsw ax ; 0108
    shr ah, 1 ; 010A
    jae LimpetIFSOriginal_x87_0112 ; 010C
    fstp st(0) ; 010E
    jmp LimpetIFSOriginal_x87_0114 ; 0110
LimpetIFSOriginal_x87_0112:
    fstp st(1) ; 0112
LimpetIFSOriginal_x87_0114:
    cmp dword ptr [r8 - 04ch], 0 ; 0114
    jne LimpetIFSOriginal_x87_011d ; 0118
    fdiv qword ptr [r8 - 020h] ; 011A
LimpetIFSOriginal_x87_011d:
    fstp qword ptr [rcx + 32] ; 011D
    fldcw word ptr [rsp + 80]
    add rsp, 88
    ret
LimpetIFSOriginal_x87 ENDP


; Original QCylIFS CODE; aligned PVar and original local stores.
QCylIFSOriginal_x87 PROC
    sub rsp, 88
    fnstcw word ptr [rsp + 80]
    mov word ptr [rsp + 82], 137Fh
    fldcw word ptr [rsp + 82]
    mov r8, rdx
    movupd xmm0, xmmword ptr [rcx] ; 0003
    movsd xmm1, qword ptr [rcx + 16] ; 0008
    mulpd xmm0, xmmword ptr [r8 - 020h] ; 000D
    mulsd xmm1, qword ptr [r8 - 020h] ; 0012
    addpd xmm0, xmmword ptr [r8 - 040h] ; 0017
    addsd xmm1, qword ptr [r8 - 030h] ; 001C
    cmp dword ptr [r8 - 04ch], 0 ; 0021
    je QCylIFSOriginal_x87_0040 ; 0025
    movsd xmm5, qword ptr [rcx + 24] ; 0027
    mulsd xmm5, qword ptr [r8 - 020h] ; 002C
    movupd xmmword ptr [rcx], xmm0 ; 0031
    movsd qword ptr [rcx + 16], xmm1 ; 0036
    movsd qword ptr [rcx + 24], xmm5 ; 003B
QCylIFSOriginal_x87_0040:
    movupd xmmword ptr [rsp], xmm0 ; 0040
    movsd qword ptr [rsp + 010h], xmm1 ; 0045
    fld qword ptr [rsp + 010h] ; 004B
    fabs ; 004F
    fstp qword ptr [rsp + 010h] ; 0051
    fld qword ptr [rsp] ; 0055
    fmul st(0), st(0) ; 0058
    fld qword ptr [rsp + 8] ; 005A
    fmul st(0), st(0) ; 005E
    faddp st(1), st(0) ; 0060
    fsqrt ; 0062
    fst qword ptr [rsp] ; 0064
    fld st(0) ; 0067
    fmul qword ptr [rsp + 010h] ; 0069
    fxch st(1) ; 006D
    fld1 ; 006F
    fchs ; 0071
    faddp st(1), st(0) ; 0073
    fmul qword ptr [r8 - 010h] ; 0075
    faddp st(1), st(0) ; 0078
    fsub qword ptr [r8 - 048h] ; 007A
    fld qword ptr [rsp + 010h] ; 007D
    fsub qword ptr [r8 - 054h] ; 0081
    fld st(0) ; 0084
    fabs ; 0086
    faddp st(1), st(0) ; 0088
    faddp st(1), st(0) ; 008A
    cmp dword ptr [r8 - 04ch], 0 ; 008C
    jne QCylIFSOriginal_x87_0095 ; 0090
    fdiv qword ptr [r8 - 020h] ; 0092
QCylIFSOriginal_x87_0095:
    fstp qword ptr [rcx + 32] ; 0095
    fldcw word ptr [rsp + 80]
    add rsp, 88
    ret
QCylIFSOriginal_x87 ENDP


; Original RiemSqrtIFS CODE; aligned PVar and original local stores.
RiemSqrtIFSOriginal_x87 PROC
    sub rsp, 88
    fnstcw word ptr [rsp + 80]
    mov word ptr [rsp + 82], 137Fh
    fldcw word ptr [rsp + 82]
    mov r8, rdx
    movupd xmm0, xmmword ptr [rcx] ; 0003
    movsd xmm1, qword ptr [rcx + 16] ; 0008
    mulpd xmm0, xmmword ptr [r8 - 020h] ; 000D
    mulsd xmm1, qword ptr [r8 - 020h] ; 0012
    addpd xmm0, xmmword ptr [r8 - 040h] ; 0017
    addsd xmm1, qword ptr [r8 - 030h] ; 001C
    cmp dword ptr [r8 - 04ch], 0 ; 0021
    je RiemSqrtIFSOriginal_x87_0040 ; 0025
    movsd xmm5, qword ptr [rcx + 24] ; 0027
    mulsd xmm5, qword ptr [r8 - 020h] ; 002C
    movupd xmmword ptr [rcx], xmm0 ; 0031
    movsd qword ptr [rcx + 16], xmm1 ; 0036
    movsd qword ptr [rcx + 24], xmm5 ; 003B
RiemSqrtIFSOriginal_x87_0040:
    movupd xmmword ptr [rsp], xmm0 ; 0040
    movsd qword ptr [rsp + 010h], xmm1 ; 0045
    fld qword ptr [rsp] ; 004B
    fmul st(0), st(0) ; 004E
    fld qword ptr [rsp + 8] ; 0050
    fmul st(0), st(0) ; 0054
    faddp st(1), st(0) ; 0056
    fsqrt ; 0058
    fsqrt ; 005A
    fld qword ptr [rsp + 8] ; 005C
    fld qword ptr [rsp] ; 0060
    fpatan ; 0063
    fimul dword ptr [r8 - 050h] ; 0065
    fld qword ptr [r8 - 048h] ; 0068
    fmul qword ptr [r8 + 8] ; 006B
    faddp st(1), st(0) ; 006E
    fmul qword ptr [r8] ; 0070
    fld st(1) ; 0072
    fmul qword ptr [r8 - 060h] ; 0074
    faddp st(1), st(0) ; 0077
    fcos ; 0079
    fabs ; 007B
    fmulp st(1), st(0) ; 007D
    fmul qword ptr [r8 - 058h] ; 007F
    fld st(0) ; 0082
    fadd qword ptr [rsp + 010h] ; 0084
    fxch st(1) ; 0088
    fsub qword ptr [rsp + 010h] ; 008A
    fcom st(1) ; 008E
    wait ; 0090
    fnstsw ax ; 0091
    shr ah, 1 ; 0093
    jae RiemSqrtIFSOriginal_x87_009b ; 0095
    fstp st(1) ; 0097
    jmp RiemSqrtIFSOriginal_x87_009d ; 0099
RiemSqrtIFSOriginal_x87_009b:
    fstp st(0) ; 009B
RiemSqrtIFSOriginal_x87_009d:
    fchs ; 009D
    fld qword ptr [rsp] ; 009F
    fabs ; 00A2
    fsub qword ptr [r8 - 010h] ; 00A4
    fcom st(1) ; 00A7
    wait ; 00A9
    fnstsw ax ; 00AA
    shr ah, 1 ; 00AC
    jae RiemSqrtIFSOriginal_x87_00b4 ; 00AE
    fstp st(0) ; 00B0
    jmp RiemSqrtIFSOriginal_x87_00b6 ; 00B2
RiemSqrtIFSOriginal_x87_00b4:
    fstp st(1) ; 00B4
RiemSqrtIFSOriginal_x87_00b6:
    fld qword ptr [rsp + 8] ; 00B6
    fabs ; 00BA
    fsub qword ptr [r8 - 010h] ; 00BC
    fcom st(1) ; 00BF
    wait ; 00C1
    fnstsw ax ; 00C2
    shr ah, 1 ; 00C4
    jae RiemSqrtIFSOriginal_x87_00cc ; 00C6
    fstp st(0) ; 00C8
    jmp RiemSqrtIFSOriginal_x87_00ce ; 00CA
RiemSqrtIFSOriginal_x87_00cc:
    fstp st(1) ; 00CC
RiemSqrtIFSOriginal_x87_00ce:
    cmp dword ptr [r8 - 04ch], 0 ; 00CE
    jne RiemSqrtIFSOriginal_x87_00d7 ; 00D2
    fdiv qword ptr [r8 - 020h] ; 00D4
RiemSqrtIFSOriginal_x87_00d7:
    fstp qword ptr [rcx + 32] ; 00D7
    fldcw word ptr [rsp + 80]
    add rsp, 88
    ret
RiemSqrtIFSOriginal_x87 ENDP


; Original ScarabIFS CODE; aligned PVar and original local stores.
ScarabIFSOriginal_x87 PROC
    sub rsp, 88
    fnstcw word ptr [rsp + 80]
    mov word ptr [rsp + 82], 137Fh
    fldcw word ptr [rsp + 82]
    mov r8, rdx
    movupd xmm0, xmmword ptr [rcx] ; 0003
    movsd xmm1, qword ptr [rcx + 16] ; 0008
    mulpd xmm0, xmmword ptr [r8 - 020h] ; 000D
    mulsd xmm1, qword ptr [r8 - 020h] ; 0012
    addpd xmm0, xmmword ptr [r8 - 040h] ; 0017
    addsd xmm1, qword ptr [r8 - 030h] ; 001C
    cmp dword ptr [r8 - 04ch], 0 ; 0021
    je ScarabIFSOriginal_x87_0040 ; 0025
    movsd xmm5, qword ptr [rcx + 24] ; 0027
    mulsd xmm5, qword ptr [r8 - 020h] ; 002C
    movupd xmmword ptr [rcx], xmm0 ; 0031
    movsd qword ptr [rcx + 16], xmm1 ; 0036
    movsd qword ptr [rcx + 24], xmm5 ; 003B
ScarabIFSOriginal_x87_0040:
    movupd xmmword ptr [rsp], xmm0 ; 0040
    movsd qword ptr [rsp + 010h], xmm1 ; 0045
    fld qword ptr [rsp] ; 004B
    fst qword ptr [rsp + 018h] ; 004E
    fmul st(0), st(0) ; 0052
    fst qword ptr [rsp] ; 0054
    fld qword ptr [rsp + 8] ; 0057
    fmul st(0), st(0) ; 005B
    fld qword ptr [rsp + 010h] ; 005D
    fmul st(0), st(0) ; 0061
    faddp st(1), st(0) ; 0063
    fst qword ptr [rsp + 8] ; 0065
    faddp st(1), st(0) ; 0069
    fld st(0) ; 006B
    fxch st(1) ; 006D
    fld qword ptr [r8 - 010h] ; 006F
    fmul qword ptr [rsp + 018h] ; 0072
    faddp st(1), st(0) ; 0076
    fmul st(0), st(0) ; 0078
    fmulp st(1), st(0) ; 007A
    fld qword ptr [rsp] ; 007C
    fmul qword ptr [rsp + 8] ; 007F
    fld qword ptr [r8 - 048h] ; 0083
    fabs ; 0086
    fst qword ptr [rsp + 018h] ; 0088
    fmul st(0), st(0) ; 008C
    fmulp st(1), st(0) ; 008E
    faddp st(1), st(0) ; 0090
    fsqrt ; 0092
    fsqrt ; 0094
    fld qword ptr [rsp] ; 0096
    fmul st(0), st(0) ; 0099
    fld qword ptr [rsp + 8] ; 009B
    fmul st(0), st(0) ; 009F
    faddp st(1), st(0) ; 00A1
    fsqrt ; 00A3
    fmul qword ptr [rsp + 018h] ; 00A5
    fsqrt ; 00A9
    fchs ; 00AB
    faddp st(1), st(0) ; 00AD
    cmp dword ptr [r8 - 04ch], 0 ; 00AF
    jne ScarabIFSOriginal_x87_00b8 ; 00B3
    fdiv qword ptr [r8 - 020h] ; 00B5
ScarabIFSOriginal_x87_00b8:
    fstp qword ptr [rcx + 32] ; 00B8
    fldcw word ptr [rsp + 80]
    add rsp, 88
    ret
ScarabIFSOriginal_x87 ENDP


; Original toupieIFS CODE; aligned PVar and original local stores.
toupieIFSOriginal_x87 PROC
    sub rsp, 88
    fnstcw word ptr [rsp + 80]
    mov word ptr [rsp + 82], 137Fh
    fldcw word ptr [rsp + 82]
    mov r8, rdx
    movupd xmm0, xmmword ptr [rcx] ; 0003
    movsd xmm1, qword ptr [rcx + 16] ; 0008
    mulpd xmm0, xmmword ptr [r8 - 020h] ; 000D
    mulsd xmm1, qword ptr [r8 - 020h] ; 0012
    addpd xmm0, xmmword ptr [r8 - 040h] ; 0017
    addsd xmm1, qword ptr [r8 - 030h] ; 001C
    cmp dword ptr [r8 - 04ch], 0 ; 0021
    je toupieIFSOriginal_x87_0040 ; 0025
    movsd xmm5, qword ptr [rcx + 24] ; 0027
    mulsd xmm5, qword ptr [r8 - 020h] ; 002C
    movupd xmmword ptr [rcx], xmm0 ; 0031
    movsd qword ptr [rcx + 16], xmm1 ; 0036
    movsd qword ptr [rcx + 24], xmm5 ; 003B
toupieIFSOriginal_x87_0040:
    movupd xmmword ptr [rsp], xmm0 ; 0040
    movsd qword ptr [rsp + 010h], xmm1 ; 0045
    nop ; 004B
    add r8, -02ch
    fld qword ptr [rsp + 010h] ; 004F
    fld qword ptr [rsp + 8] ; 0053
    fld qword ptr [rsp] ; 0057
    fld st(0) ; 005A
    fmul dword ptr [r8 - 03ch] ; 005C
    fld st(2) ; 005F
    fmul dword ptr [r8 - 040h] ; 0061
    faddp st(1), st(0) ; 0064
    fld st(3) ; 0066
    fmul dword ptr [r8 - 044h] ; 0068
    faddp st(1), st(0) ; 006B
    fld st(1) ; 006D
    fmul dword ptr [r8 - 024h] ; 006F
    fld st(3) ; 0072
    fmul dword ptr [r8 - 028h] ; 0074
    faddp st(1), st(0) ; 0077
    fld st(4) ; 0079
    fmul dword ptr [r8 - 02ch] ; 007B
    faddp st(1), st(0) ; 007E
    fxch st(2) ; 0080
    fmul dword ptr [r8 - 030h] ; 0082
    fxch st(3) ; 0085
    fmul dword ptr [r8 - 034h] ; 0087
    faddp st(3), st(0) ; 008A
    fxch st(3) ; 008C
    fmul dword ptr [r8 - 038h] ; 008E
    faddp st(2), st(0) ; 0091
    fstp qword ptr [rsp] ; 0093
    fstp qword ptr [rsp + 8] ; 0096
    fstp qword ptr [rsp + 010h] ; 009A
    add r8, 02ch
    nop ; 00A1
    fld qword ptr [rsp] ; 00A2
    fmul st(0), st(0) ; 00A5
    fld qword ptr [rsp + 8] ; 00A7
    fmul st(0), st(0) ; 00AB
    faddp st(1), st(0) ; 00AD
    fst qword ptr [rsp] ; 00AF
    fsqrt ; 00B2
    fst qword ptr [rsp + 018h] ; 00B4
    fsqrt ; 00B8
    fmul qword ptr [r8 - 048h] ; 00BA
    fld qword ptr [rsp + 010h] ; 00BD
    fabs ; 00C1
    fsubr qword ptr [r8 - 010h] ; 00C3
    fsubp st(1), st(0) ; 00C6
    fld qword ptr [rsp + 018h] ; 00C8
    fsub qword ptr [r8 - 048h] ; 00CC
    fsub qword ptr [r8 - 048h] ; 00CF
    fsub qword ptr [r8 - 048h] ; 00D2
    fcom st(1) ; 00D5
    wait ; 00D7
    fnstsw ax ; 00D8
    shr ah, 1 ; 00DA
    jae toupieIFSOriginal_x87_00e2 ; 00DC
    fstp st(0) ; 00DE
    jmp toupieIFSOriginal_x87_00e4 ; 00E0
toupieIFSOriginal_x87_00e2:
    fstp st(1) ; 00E2
toupieIFSOriginal_x87_00e4:
    fld qword ptr [rsp + 010h] ; 00E4
    fabs ; 00E8
    fsub qword ptr [r8 - 048h] ; 00EA
    fsub qword ptr [r8 - 048h] ; 00ED
    fsub qword ptr [r8 - 048h] ; 00F0
    fcom st(1) ; 00F3
    wait ; 00F5
    fnstsw ax ; 00F6
    shr ah, 1 ; 00F8
    jae toupieIFSOriginal_x87_0100 ; 00FA
    fstp st(0) ; 00FC
    jmp toupieIFSOriginal_x87_0102 ; 00FE
toupieIFSOriginal_x87_0100:
    fstp st(1) ; 0100
toupieIFSOriginal_x87_0102:
    cmp dword ptr [r8 - 04ch], 0 ; 0102
    jne toupieIFSOriginal_x87_010b ; 0106
    fdiv qword ptr [r8 - 020h] ; 0108
toupieIFSOriginal_x87_010b:
    fstp qword ptr [rcx + 32] ; 010B
    fldcw word ptr [rsp + 80]
    add rsp, 88
    ret
toupieIFSOriginal_x87 ENDP


; Original UmbrellaIFS CODE; aligned PVar and original local stores.
UmbrellaIFSOriginal_x87 PROC
    sub rsp, 88
    fnstcw word ptr [rsp + 80]
    mov word ptr [rsp + 82], 137Fh
    fldcw word ptr [rsp + 82]
    mov r8, rdx
    movupd xmm0, xmmword ptr [rcx] ; 0003
    movsd xmm1, qword ptr [rcx + 16] ; 0008
    mulpd xmm0, xmmword ptr [r8 - 020h] ; 000D
    mulsd xmm1, qword ptr [r8 - 020h] ; 0012
    addpd xmm0, xmmword ptr [r8 - 040h] ; 0017
    addsd xmm1, qword ptr [r8 - 030h] ; 001C
    cmp dword ptr [r8 - 04ch], 0 ; 0021
    je UmbrellaIFSOriginal_x87_0040 ; 0025
    movsd xmm5, qword ptr [rcx + 24] ; 0027
    mulsd xmm5, qword ptr [r8 - 020h] ; 002C
    movupd xmmword ptr [rcx], xmm0 ; 0031
    movsd qword ptr [rcx + 16], xmm1 ; 0036
    movsd qword ptr [rcx + 24], xmm5 ; 003B
UmbrellaIFSOriginal_x87_0040:
    movupd xmmword ptr [rsp], xmm0 ; 0040
    movsd qword ptr [rsp + 010h], xmm1 ; 0045
    cmp dword ptr [r8 - 050h], 0 ; 004B
    je UmbrellaIFSOriginal_x87_005f ; 004F
    fld qword ptr [rsp] ; 0051
    fld qword ptr [rsp + 010h] ; 0054
    fstp qword ptr [rsp] ; 0058
    fstp qword ptr [rsp + 010h] ; 005B
UmbrellaIFSOriginal_x87_005f:
    fld qword ptr [rsp + 010h] ; 005F
    fmul qword ptr [rsp + 8] ; 0063
    fmul qword ptr [rsp + 8] ; 0067
    fld qword ptr [rsp] ; 006B
    fmul st(0), st(0) ; 006E
    fsubrp st(1), st(0) ; 0070
    fadd qword ptr [r8 - 048h] ; 0072
    fabs ; 0075
    fsub qword ptr [r8 - 048h] ; 0077
    fmul qword ptr [r8] ; 007A
    fld1 ; 007C
    fcomp st(1) ; 007E
    wait ; 0080
    fnstsw ax ; 0081
    shr ah, 1 ; 0083
    jae UmbrellaIFSOriginal_x87_008b ; 0085
    fsqrt ; 0087
    fsqrt ; 0089
UmbrellaIFSOriginal_x87_008b:
    fld qword ptr [rsp] ; 008B
    fabs ; 008E
    fsub qword ptr [r8 - 010h] ; 0090
    fld qword ptr [rsp + 8] ; 0093
    fabs ; 0097
    fsub qword ptr [r8 - 010h] ; 0099
    fcom st(1) ; 009C
    wait ; 009E
    fnstsw ax ; 009F
    shr ah, 1 ; 00A1
    jae UmbrellaIFSOriginal_x87_00a9 ; 00A3
    fstp st(0) ; 00A5
    jmp UmbrellaIFSOriginal_x87_00ab ; 00A7
UmbrellaIFSOriginal_x87_00a9:
    fstp st(1) ; 00A9
UmbrellaIFSOriginal_x87_00ab:
    fld qword ptr [rsp + 010h] ; 00AB
    fabs ; 00AF
    fsub qword ptr [r8 - 010h] ; 00B1
    fcom st(1) ; 00B4
    wait ; 00B6
    fnstsw ax ; 00B7
    shr ah, 1 ; 00B9
    jae UmbrellaIFSOriginal_x87_00c1 ; 00BB
    fstp st(0) ; 00BD
    jmp UmbrellaIFSOriginal_x87_00c3 ; 00BF
UmbrellaIFSOriginal_x87_00c1:
    fstp st(1) ; 00C1
UmbrellaIFSOriginal_x87_00c3:
    fcom st(1) ; 00C3
    wait ; 00C5
    fnstsw ax ; 00C6
    shr ah, 1 ; 00C8
    jae UmbrellaIFSOriginal_x87_00d0 ; 00CA
    fstp st(0) ; 00CC
    jmp UmbrellaIFSOriginal_x87_00d2 ; 00CE
UmbrellaIFSOriginal_x87_00d0:
    fstp st(1) ; 00D0
UmbrellaIFSOriginal_x87_00d2:
    cmp dword ptr [r8 - 04ch], 0 ; 00D2
    jne UmbrellaIFSOriginal_x87_00db ; 00D6
    fdiv qword ptr [r8 - 020h] ; 00D8
UmbrellaIFSOriginal_x87_00db:
    fstp qword ptr [rcx + 32] ; 00DB
    fld qword ptr [rsp + 8] ; 00DE
    fmul qword ptr [rsp] ; 00E2
    fsub qword ptr [rsp + 010h] ; 00E5
    fmul qword ptr [r8 - 058h] ; 00E9
    fild dword ptr [rcx + 48] ; 00EC
    fmul qword ptr [r8 - 060h] ; 00EF
    faddp st(1), st(0) ; 00F2
    fadd qword ptr [r8 - 068h] ; 00F4
    fabs ; 00F7
    fstp qword ptr [rcx + 40] ; 00F9
    fldcw word ptr [rsp + 80]
    add rsp, 88
    ret
UmbrellaIFSOriginal_x87 ENDP


; Original TgladTetra: rcx=XYZ,C; rdx=PVar. Overflow bypasses C.
TgladTetraOriginal_x87 PROC
    sub rsp, 104
    fnstcw word ptr [rsp + 96]
    mov word ptr [rsp + 98], 137Fh
    fldcw word ptr [rsp + 98]
    xor eax, eax
    fnop ; 0014
    mov ah, byte ptr [rdx - 0ch] ; 0016
    and ah, 7 ; 0019
TgladTetraOriginal_x87_001c:
    fld qword ptr [rcx] ; 001C
    fmul st(0), st(0) ; 001E
    fld qword ptr [rcx + 8] ; 0020
    fmul st(0), st(0) ; 0022
    faddp st(1), st(0) ; 0024
    fld st(0) ; 0026
    fld qword ptr [rcx + 16] ; 0028
    fmul st(0), st(0) ; 002A
    faddp st(1), st(0) ; 002C
    shl eax, 010h ; 002E
    fcom qword ptr [rdx + 030h] ; 0031
    wait ; 0034
    fnstsw ax ; 0035
    shr ah, 1 ; 0037
    jae TgladTetraOriginal_x87_01ea ; 0039
    shr eax, 010h ; 003F
    fsqrt ; 0042
    fst qword ptr [rsp + 24] ; 0044
    fld qword ptr [rcx + 16] ; 0047
    fdivrp st(1), st(0) ; 0049
    fstp qword ptr [rsp + 88] ; 004B
    fsqrt ; 004E
    fsub qword ptr [rcx + 8] ; 0050
    fdiv qword ptr [rcx] ; 0052
    fst qword ptr [rsp + 80] ; 0054
    fmul st(0), st(0) ; 0057
    fst qword ptr [rsp + 56] ; 0059
    fld1 ; 005C
    faddp st(1), st(0) ; 005E
    fld1 ; 0060
    fdivrp st(1), st(0) ; 0062
    nop ; 0064
    fld1 ; 0065
    fadd qword ptr [rsp + 88] ; 0067
    fld1 ; 006A
    fsub qword ptr [rsp + 88] ; 006C
    fdivrp st(1), st(0) ; 006F
    fsqrt ; 0071
    nop ; 0073
    fmulp st(1), st(0) ; 0074
    fld st(0) ; 0076
    fadd st(0), st(0) ; 0078
    fmul qword ptr [rsp + 80] ; 007A
    fstp qword ptr [rcx] ; 007D
    nop ; 007F
    fld1 ; 0080
    fsub qword ptr [rsp + 56] ; 0082
    fmulp st(1), st(0) ; 0085
    fstp qword ptr [rcx + 8] ; 0087
    fnop ; 0089
    mov al, byte ptr [rdx - 010h] ; 008B
    and al, 7 ; 008E
    nop ; 0090
TgladTetraOriginal_x87_0091:
    fld qword ptr [rcx] ; 0091
    fld st(0) ; 0093
    fmul st(0), st(0) ; 0095
    fmulp st(1), st(0) ; 0097
    fld qword ptr [rcx + 8] ; 0099
    fmul st(0), st(0) ; 009B
    fmul qword ptr [rdx + 8] ; 009D
    fmul qword ptr [rcx] ; 00A0
    faddp st(1), st(0) ; 00A2
    shl eax, 010h ; 00A4
    fcom qword ptr [rdx + 030h] ; 00A7
    wait ; 00AA
    fnstsw ax ; 00AB
    shr ah, 1 ; 00AD
    jae TgladTetraOriginal_x87_01ec ; 00AF
    shr eax, 010h ; 00B5
    fstp qword ptr [rsp + 64] ; 00B8
    fld qword ptr [rcx + 8] ; 00BB
    fld st(0) ; 00BD
    fmul st(0), st(0) ; 00BF
    fmulp st(1), st(0) ; 00C1
    fld qword ptr [rcx] ; 00C3
    fmul st(0), st(0) ; 00C5
    fmul qword ptr [rdx + 8] ; 00C7
    fmul qword ptr [rcx + 8] ; 00CA
    faddp st(1), st(0) ; 00CC
    fst qword ptr [rsp + 56] ; 00CE
    fmul qword ptr [rdx + 020h] ; 00D1
    fstp qword ptr [rsp + 48] ; 00D4
    fld qword ptr [rsp + 56] ; 00D7
    fmul st(0), st(0) ; 00DA
    fstp qword ptr [rsp + 56] ; 00DC
    fld qword ptr [rsp + 64] ; 00DF
    fadd qword ptr [rdx + 018h] ; 00E2
    fld qword ptr [rdx + 010h] ; 00E5
    fsub qword ptr [rsp + 64] ; 00E8
    fmulp st(1), st(0) ; 00EB
    fsub qword ptr [rsp + 56] ; 00ED
    fstp qword ptr [rsp + 40] ; 00F0
    fld qword ptr [rsp + 64] ; 00F3
    fadd qword ptr [rdx + 018h] ; 00F6
    fmul st(0), st(0) ; 00F9
    fadd qword ptr [rsp + 56] ; 00FB
    fld qword ptr [rdx + 018h] ; 00FE
    fchs ; 0101
    fdivrp st(1), st(0) ; 0103
    fstp qword ptr [rsp + 32] ; 0105
    nop ; 0108
    fld qword ptr [rsp + 40] ; 0109
    fmul qword ptr [rcx] ; 010C
    fld qword ptr [rsp + 48] ; 010E
    fmul qword ptr [rcx + 8] ; 0111
    fchs ; 0113
    faddp st(1), st(0) ; 0115
    fmul qword ptr [rsp + 32] ; 0117
    nop ; 011A
    fld qword ptr [rsp + 40] ; 011B
    fmul qword ptr [rcx + 8] ; 011E
    fld qword ptr [rsp + 48] ; 0120
    fmul qword ptr [rcx] ; 0123
    faddp st(1), st(0) ; 0125
    fmul qword ptr [rsp + 32] ; 0127
    nop ; 012A
    fstp qword ptr [rcx + 8] ; 012B
    fstp qword ptr [rcx] ; 012D
    fld qword ptr [rsp + 24] ; 012F
    fmul st(0), st(0) ; 0132
    fstp qword ptr [rsp + 24] ; 0134
    dec al ; 0137
    jg TgladTetraOriginal_x87_0091 ; 0139
    fnop ; 013F
    fld qword ptr [rcx] ; 0141
    fmul st(0), st(0) ; 0143
    fld qword ptr [rcx + 8] ; 0145
    fmul st(0), st(0) ; 0147
    faddp st(1), st(0) ; 0149
    fst qword ptr [rsp + 32] ; 014B
    fsqrt ; 014E
    fst qword ptr [rsp + 72] ; 0150
    fsub qword ptr [rcx + 8] ; 0153
    fdiv qword ptr [rcx] ; 0155
    fst qword ptr [rsp + 80] ; 0157
    fmul st(0), st(0) ; 015A
    fst qword ptr [rsp + 56] ; 015C
    fld1 ; 015F
    faddp st(1), st(0) ; 0161
    fld1 ; 0163
    fdivrp st(1), st(0) ; 0165
    fst qword ptr [rsp + 64] ; 0167
    fld1 ; 016A
    fsub qword ptr [rsp + 56] ; 016C
    fmulp st(1), st(0) ; 016F
    fstp qword ptr [rsp + 40] ; 0171
    nop ; 0174
    fld qword ptr [rsp + 80] ; 0175
    fadd st(0), st(0) ; 0178
    fmul qword ptr [rsp + 64] ; 017A
    fstp qword ptr [rsp + 48] ; 017D
    nop ; 0180
    fld1 ; 0181
    fadd qword ptr [rsp + 32] ; 0183
    fld qword ptr [rdx + 028h] ; 0186
    fdivrp st(1), st(0) ; 0189
    fst qword ptr [rsp + 56] ; 018B
    fmul qword ptr [rsp + 72] ; 018E
    fstp qword ptr [rsp + 64] ; 0191
    nop ; 0194
    nop ; 0195
    fld qword ptr [rsp + 24] ; 0196
    fmul qword ptr [rsp + 48] ; 0199
    fmul qword ptr [rsp + 64] ; 019C
    nop ; 019F
    fld qword ptr [rsp + 24] ; 01A0
    fmul qword ptr [rsp + 40] ; 01A3
    fmul qword ptr [rsp + 64] ; 01A6
    nop ; 01A9
    fld qword ptr [rsp + 56] ; 01AA
    fld1 ; 01AD
    fsubp st(1), st(0) ; 01AF
    fmul qword ptr [rsp + 24] ; 01B1
    nop ; 01B4
    nop ; 01B5
    fstp qword ptr [rcx + 16] ; 01B6
    fstp qword ptr [rcx + 8] ; 01B8
    fstp qword ptr [rcx] ; 01BA
    fnop ; 01BC
    dec ah ; 01BE
    cmp ah, 0 ; 01C0
    jg TgladTetraOriginal_x87_001c ; 01C3
    fld qword ptr [rcx] ; 01C9
    fadd qword ptr [rcx + 24] ; 01CB
    fld qword ptr [rcx + 8] ; 01CE
    fadd qword ptr [rcx + 32] ; 01D0
    fld qword ptr [rcx + 16] ; 01D3
    fadd qword ptr [rcx + 40] ; 01D5
TgladTetraOriginal_x87_01d8:
    fstp qword ptr [rcx + 16] ; 01D8
    fstp qword ptr [rcx + 8] ; 01DA
    fstp qword ptr [rcx] ; 01DC
    nop ; 01DE
    jmp TgladTetraOriginal_x87_done
TgladTetraOriginal_x87_01ea:
    fstp st(0) ; 01EA
TgladTetraOriginal_x87_01ec:
    fstp st(0) ; 01EC
    fld qword ptr [rdx + 030h] ; 01EE
    fld st(0) ; 01F1
    fld st(0) ; 01F3
    jmp TgladTetraOriginal_x87_01d8 ; 01F5
TgladTetraOriginal_x87_done:
    fldcw word ptr [rsp + 96]
    add rsp, 104
    ret
TgladTetraOriginal_x87 ENDP


; Original TgladTetra2: rcx=XYZ,C; rdx=PVar. Overflow bypasses C.
TgladTetra2Original_x87 PROC
    sub rsp, 104
    fnstcw word ptr [rsp + 96]
    mov word ptr [rsp + 98], 137Fh
    fldcw word ptr [rsp + 98]
    xor eax, eax
    fnop ; 0014
    mov ah, byte ptr [rdx - 0ch] ; 0016
    and ah, 7 ; 0019
TgladTetra2Original_x87_001c:
    fld qword ptr [rcx] ; 001C
    fmul st(0), st(0) ; 001E
    fld qword ptr [rcx + 8] ; 0020
    fmul st(0), st(0) ; 0022
    faddp st(1), st(0) ; 0024
    fld st(0) ; 0026
    fld qword ptr [rcx + 16] ; 0028
    fmul st(0), st(0) ; 002A
    faddp st(1), st(0) ; 002C
    shl eax, 010h ; 002E
    fcom qword ptr [rdx + 030h] ; 0031
    wait ; 0034
    fnstsw ax ; 0035
    shr ah, 1 ; 0037
    jae TgladTetra2Original_x87_01f2 ; 0039
    shr eax, 010h ; 003F
    fsqrt ; 0042
    fst qword ptr [rsp + 24] ; 0044
    fld qword ptr [rcx + 16] ; 0047
    fdivrp st(1), st(0) ; 0049
    fstp qword ptr [rsp + 88] ; 004B
    fsqrt ; 004E
    fsub qword ptr [rcx + 8] ; 0050
    fdiv qword ptr [rcx] ; 0052
    fst qword ptr [rsp + 80] ; 0054
    fmul st(0), st(0) ; 0057
    fst qword ptr [rsp + 56] ; 0059
    fld1 ; 005C
    faddp st(1), st(0) ; 005E
    fld1 ; 0060
    fdivrp st(1), st(0) ; 0062
    nop ; 0064
    fld1 ; 0065
    fadd qword ptr [rsp + 88] ; 0067
    fld1 ; 006A
    fsub qword ptr [rsp + 88] ; 006C
    fdivrp st(1), st(0) ; 006F
    fsqrt ; 0071
    nop ; 0073
    fmulp st(1), st(0) ; 0074
    fld st(0) ; 0076
    fadd st(0), st(0) ; 0078
    fmul qword ptr [rsp + 80] ; 007A
    fstp qword ptr [rcx + 8] ; 007D
    nop ; 007F
    fld1 ; 0080
    fsubr qword ptr [rsp + 56] ; 0082
    fmulp st(1), st(0) ; 0085
    fstp qword ptr [rcx] ; 0087
    fnop ; 0089
    mov al, byte ptr [rdx - 010h] ; 008B
    and al, 7 ; 008E
    nop ; 0090
TgladTetra2Original_x87_0091:
    fld qword ptr [rcx] ; 0091
    fld st(0) ; 0093
    fmul st(0), st(0) ; 0095
    fmulp st(1), st(0) ; 0097
    fld qword ptr [rcx + 8] ; 0099
    fmul st(0), st(0) ; 009B
    fmul qword ptr [rdx + 8] ; 009D
    fmul qword ptr [rcx] ; 00A0
    faddp st(1), st(0) ; 00A2
    shl eax, 010h ; 00A4
    fcom qword ptr [rdx + 030h] ; 00A7
    wait ; 00AA
    fnstsw ax ; 00AB
    shr ah, 1 ; 00AD
    jae TgladTetra2Original_x87_01f4 ; 00AF
    shr eax, 010h ; 00B5
    fstp qword ptr [rsp + 64] ; 00B8
    fld qword ptr [rcx + 8] ; 00BB
    fld st(0) ; 00BD
    fmul st(0), st(0) ; 00BF
    fmulp st(1), st(0) ; 00C1
    fld qword ptr [rcx] ; 00C3
    fmul st(0), st(0) ; 00C5
    fmul qword ptr [rdx + 8] ; 00C7
    fmul qword ptr [rcx + 8] ; 00CA
    faddp st(1), st(0) ; 00CC
    fst qword ptr [rsp + 56] ; 00CE
    fmul qword ptr [rdx + 020h] ; 00D1
    fstp qword ptr [rsp + 48] ; 00D4
    fld qword ptr [rsp + 56] ; 00D7
    fmul st(0), st(0) ; 00DA
    fstp qword ptr [rsp + 56] ; 00DC
    fld qword ptr [rsp + 64] ; 00DF
    fadd qword ptr [rdx + 018h] ; 00E2
    fld qword ptr [rdx + 010h] ; 00E5
    fsub qword ptr [rsp + 64] ; 00E8
    fmulp st(1), st(0) ; 00EB
    fsub qword ptr [rsp + 56] ; 00ED
    fstp qword ptr [rsp + 40] ; 00F0
    fld qword ptr [rsp + 64] ; 00F3
    fadd qword ptr [rdx + 018h] ; 00F6
    fmul st(0), st(0) ; 00F9
    fadd qword ptr [rsp + 56] ; 00FB
    fld qword ptr [rdx + 018h] ; 00FE
    fchs ; 0101
    fdivrp st(1), st(0) ; 0103
    fstp qword ptr [rsp + 32] ; 0105
    nop ; 0108
    fld qword ptr [rsp + 40] ; 0109
    fmul qword ptr [rcx] ; 010C
    fld qword ptr [rsp + 48] ; 010E
    fmul qword ptr [rcx + 8] ; 0111
    fchs ; 0113
    faddp st(1), st(0) ; 0115
    fmul qword ptr [rsp + 32] ; 0117
    nop ; 011A
    fld qword ptr [rsp + 40] ; 011B
    fmul qword ptr [rcx + 8] ; 011E
    fld qword ptr [rsp + 48] ; 0120
    fmul qword ptr [rcx] ; 0123
    faddp st(1), st(0) ; 0125
    fmul qword ptr [rsp + 32] ; 0127
    nop ; 012A
    fstp qword ptr [rcx + 8] ; 012B
    fstp qword ptr [rcx] ; 012D
    fld qword ptr [rsp + 24] ; 012F
    fmul st(0), st(0) ; 0132
    fstp qword ptr [rsp + 24] ; 0134
    dec al ; 0137
    jg TgladTetra2Original_x87_0091 ; 0139
    fld qword ptr [rcx] ; 013F
    fld qword ptr [rcx + 8] ; 0141
    fstp qword ptr [rcx] ; 0143
    fstp qword ptr [rcx + 8] ; 0145
    fnop ; 0147
    fld qword ptr [rcx] ; 0149
    fmul st(0), st(0) ; 014B
    fld qword ptr [rcx + 8] ; 014D
    fmul st(0), st(0) ; 014F
    faddp st(1), st(0) ; 0151
    fst qword ptr [rsp + 32] ; 0153
    fsqrt ; 0156
    fst qword ptr [rsp + 72] ; 0158
    fsub qword ptr [rcx + 8] ; 015B
    fdiv qword ptr [rcx] ; 015D
    fst qword ptr [rsp + 80] ; 015F
    fmul st(0), st(0) ; 0162
    fst qword ptr [rsp + 56] ; 0164
    fld1 ; 0167
    faddp st(1), st(0) ; 0169
    fld1 ; 016B
    fdivrp st(1), st(0) ; 016D
    fst qword ptr [rsp + 64] ; 016F
    fld1 ; 0172
    fsub qword ptr [rsp + 56] ; 0174
    fmulp st(1), st(0) ; 0177
    fstp qword ptr [rsp + 40] ; 0179
    nop ; 017C
    fld qword ptr [rsp + 80] ; 017D
    fadd st(0), st(0) ; 0180
    fmul qword ptr [rsp + 64] ; 0182
    fstp qword ptr [rsp + 48] ; 0185
    nop ; 0188
    fld1 ; 0189
    fadd qword ptr [rsp + 32] ; 018B
    fld qword ptr [rdx + 028h] ; 018E
    fdivrp st(1), st(0) ; 0191
    fst qword ptr [rsp + 56] ; 0193
    fmul qword ptr [rsp + 72] ; 0196
    fstp qword ptr [rsp + 64] ; 0199
    nop ; 019C
    nop ; 019D
    fld qword ptr [rsp + 24] ; 019E
    fmul qword ptr [rsp + 48] ; 01A1
    fmul qword ptr [rsp + 64] ; 01A4
    nop ; 01A7
    fld qword ptr [rsp + 24] ; 01A8
    fmul qword ptr [rsp + 40] ; 01AB
    fmul qword ptr [rsp + 64] ; 01AE
    nop ; 01B1
    fld qword ptr [rsp + 56] ; 01B2
    fld1 ; 01B5
    fsubrp st(1), st(0) ; 01B7
    fmul qword ptr [rsp + 24] ; 01B9
    nop ; 01BC
    nop ; 01BD
    fstp qword ptr [rcx + 16] ; 01BE
    fstp qword ptr [rcx + 8] ; 01C0
    fstp qword ptr [rcx] ; 01C2
    fnop ; 01C4
    dec ah ; 01C6
    cmp ah, 0 ; 01C8
    jg TgladTetra2Original_x87_001c ; 01CB
    fld qword ptr [rcx] ; 01D1
    fadd qword ptr [rcx + 24] ; 01D3
    fld qword ptr [rcx + 8] ; 01D6
    fadd qword ptr [rcx + 32] ; 01D8
    fld qword ptr [rcx + 16] ; 01DB
    fadd qword ptr [rcx + 40] ; 01DD
TgladTetra2Original_x87_01e0:
    fstp qword ptr [rcx + 16] ; 01E0
    fstp qword ptr [rcx + 8] ; 01E2
    fstp qword ptr [rcx] ; 01E4
    nop ; 01E6
    jmp TgladTetra2Original_x87_done
TgladTetra2Original_x87_01f2:
    fstp st(0) ; 01F2
TgladTetra2Original_x87_01f4:
    fstp st(0) ; 01F4
    fld qword ptr [rdx + 030h] ; 01F6
    fld st(0) ; 01F9
    fld st(0) ; 01FB
    jmp TgladTetra2Original_x87_01e0 ; 01FD
TgladTetra2Original_x87_done:
    fldcw word ptr [rsp + 96]
    add rsp, 104
    ret
TgladTetra2Original_x87 ENDP


; Original TgladDihed: rcx=XYZ,C; rdx=PVar. Overflow bypasses C.
TgladDihedOriginal_x87 PROC
    sub rsp, 104
    fnstcw word ptr [rsp + 96]
    mov word ptr [rsp + 98], 137Fh
    fldcw word ptr [rsp + 98]
    xor eax, eax
    fnop ; 0014
    mov ah, byte ptr [rdx - 0ch] ; 0016
    and ah, 7 ; 0019
TgladDihedOriginal_x87_001c:
    fld qword ptr [rcx] ; 001C
    fmul st(0), st(0) ; 001E
    fld qword ptr [rcx + 8] ; 0020
    fmul st(0), st(0) ; 0022
    faddp st(1), st(0) ; 0024
    fld st(0) ; 0026
    fld qword ptr [rcx + 16] ; 0028
    fmul st(0), st(0) ; 002A
    faddp st(1), st(0) ; 002C
    shl eax, 010h ; 002E
    fcom qword ptr [rdx + 030h] ; 0031
    wait ; 0034
    fnstsw ax ; 0035
    shr ah, 1 ; 0037
    jae TgladDihedOriginal_x87_0203 ; 0039
    shr eax, 010h ; 003F
    fsqrt ; 0042
    fst qword ptr [rsp + 24] ; 0044
    fld qword ptr [rcx + 16] ; 0047
    fdivrp st(1), st(0) ; 0049
    fstp qword ptr [rsp + 88] ; 004B
    fsqrt ; 004E
    fsub qword ptr [rcx + 8] ; 0050
    fdiv qword ptr [rcx] ; 0052
    fst qword ptr [rsp + 80] ; 0054
    fmul st(0), st(0) ; 0057
    fst qword ptr [rsp + 56] ; 0059
    fld1 ; 005C
    faddp st(1), st(0) ; 005E
    fld1 ; 0060
    fdivrp st(1), st(0) ; 0062
    nop ; 0064
    fld1 ; 0065
    fadd qword ptr [rsp + 88] ; 0067
    fld1 ; 006A
    fsub qword ptr [rsp + 88] ; 006C
    fdivrp st(1), st(0) ; 006F
    fsqrt ; 0071
    nop ; 0073
    fmulp st(1), st(0) ; 0074
    fld st(0) ; 0076
    fadd st(0), st(0) ; 0078
    fmul qword ptr [rsp + 80] ; 007A
    fstp qword ptr [rcx] ; 007D
    nop ; 007F
    fld1 ; 0080
    fsub qword ptr [rsp + 56] ; 0082
    fmulp st(1), st(0) ; 0085
    fstp qword ptr [rcx + 8] ; 0087
    fnop ; 0089
    mov al, byte ptr [rdx - 010h] ; 008B
    and al, 7 ; 008E
    nop ; 0090
TgladDihedOriginal_x87_0091:
    fld qword ptr [rcx] ; 0091
    fmul st(0), st(0) ; 0093
    fld qword ptr [rcx + 8] ; 0095
    fmul st(0), st(0) ; 0097
    faddp st(1), st(0) ; 0099
    shl eax, 010h ; 009B
    fcom qword ptr [rdx + 030h] ; 009E
    wait ; 00A1
    fnstsw ax ; 00A2
    shr ah, 1 ; 00A4
    jae TgladDihedOriginal_x87_0205 ; 00A6
    shr eax, 010h ; 00AC
    fstp qword ptr [rsp + 72] ; 00AF
    nop ; 00B2
    fld qword ptr [rcx + 8] ; 00B3
    fchs ; 00B5
    fld qword ptr [rsp + 72] ; 00B7
    fld1 ; 00BA
    faddp st(1), st(0) ; 00BC
    fmulp st(1), st(0) ; 00BE
    fld qword ptr [rsp + 72] ; 00C0
    fld qword ptr [rcx] ; 00C3
    fmul qword ptr [rdx + 028h] ; 00C5
    fsubp st(1), st(0) ; 00C8
    fld1 ; 00CA
    fsubp st(1), st(0) ; 00CC
    fmulp st(1), st(0) ; 00CE
    fld qword ptr [rsp + 72] ; 00D0
    fld qword ptr [rcx] ; 00D3
    fmul qword ptr [rdx + 028h] ; 00D5
    faddp st(1), st(0) ; 00D8
    fld1 ; 00DA
    fsubp st(1), st(0) ; 00DC
    fmulp st(1), st(0) ; 00DE
    fstp qword ptr [rsp + 88] ; 00E0
    fld qword ptr [rcx] ; 00E3
    fld qword ptr [rsp + 72] ; 00E5
    fld1 ; 00E8
    fsubp st(1), st(0) ; 00EA
    fmulp st(1), st(0) ; 00EC
    fld qword ptr [rsp + 72] ; 00EE
    fmul st(0), st(0) ; 00F1
    fld qword ptr [rsp + 72] ; 00F3
    fmul qword ptr [rdx + 028h] ; 00F6
    faddp st(1), st(0) ; 00F9
    fld qword ptr [rcx + 8] ; 00FB
    fmul qword ptr [rdx + 028h] ; 00FD
    fmul st(0), st(0) ; 0100
    faddp st(1), st(0) ; 0102
    fld1 ; 0104
    faddp st(1), st(0) ; 0106
    fmulp st(1), st(0) ; 0108
    fstp qword ptr [rsp + 80] ; 010A
    fld qword ptr [rsp + 72] ; 010D
    fld qword ptr [rcx + 8] ; 0110
    fmul qword ptr [rdx + 028h] ; 0112
    fsubp st(1), st(0) ; 0115
    fld1 ; 0117
    faddp st(1), st(0) ; 0119
    fld qword ptr [rsp + 72] ; 011B
    fld qword ptr [rcx + 8] ; 011E
    fmul qword ptr [rdx + 028h] ; 0120
    faddp st(1), st(0) ; 0123
    fld1 ; 0125
    faddp st(1), st(0) ; 0127
    fmulp st(1), st(0) ; 0129
    fmul st(0), st(0) ; 012B
    fld qword ptr [rdx + 028h] ; 012D
    fdivrp st(1), st(0) ; 0130
    fld st(0) ; 0132
    fmul qword ptr [rsp + 80] ; 0134
    fstp qword ptr [rcx] ; 0137
    fmul qword ptr [rsp + 88] ; 0139
    fstp qword ptr [rcx + 8] ; 013C
    fld qword ptr [rsp + 24] ; 013E
    fmul st(0), st(0) ; 0141
    fstp qword ptr [rsp + 24] ; 0143
    nop ; 0146
    nop ; 0147
    dec al ; 0148
    jg TgladDihedOriginal_x87_0091 ; 014A
    nop ; 0150
    nop ; 0151
    nop ; 0152
    nop ; 0153
    nop ; 0154
    nop ; 0155
    nop ; 0156
    nop ; 0157
    fnop ; 0158
    fld qword ptr [rcx] ; 015A
    fmul st(0), st(0) ; 015C
    fld qword ptr [rcx + 8] ; 015E
    fmul st(0), st(0) ; 0160
    faddp st(1), st(0) ; 0162
    fst qword ptr [rsp + 32] ; 0164
    fsqrt ; 0167
    fst qword ptr [rsp + 72] ; 0169
    fsub qword ptr [rcx + 8] ; 016C
    fdiv qword ptr [rcx] ; 016E
    fst qword ptr [rsp + 80] ; 0170
    fmul st(0), st(0) ; 0173
    fst qword ptr [rsp + 56] ; 0175
    fld1 ; 0178
    faddp st(1), st(0) ; 017A
    fld1 ; 017C
    fdivrp st(1), st(0) ; 017E
    fst qword ptr [rsp + 64] ; 0180
    fld1 ; 0183
    fsub qword ptr [rsp + 56] ; 0185
    fmulp st(1), st(0) ; 0188
    fstp qword ptr [rsp + 40] ; 018A
    nop ; 018D
    fld qword ptr [rsp + 80] ; 018E
    fadd st(0), st(0) ; 0191
    fmul qword ptr [rsp + 64] ; 0193
    fstp qword ptr [rsp + 48] ; 0196
    nop ; 0199
    fld1 ; 019A
    fadd qword ptr [rsp + 32] ; 019C
    fld qword ptr [rdx + 028h] ; 019F
    fdivrp st(1), st(0) ; 01A2
    fst qword ptr [rsp + 56] ; 01A4
    fmul qword ptr [rsp + 72] ; 01A7
    fstp qword ptr [rsp + 64] ; 01AA
    nop ; 01AD
    nop ; 01AE
    fld qword ptr [rsp + 24] ; 01AF
    fmul qword ptr [rsp + 48] ; 01B2
    fmul qword ptr [rsp + 64] ; 01B5
    nop ; 01B8
    fld qword ptr [rsp + 24] ; 01B9
    fmul qword ptr [rsp + 40] ; 01BC
    fmul qword ptr [rsp + 64] ; 01BF
    nop ; 01C2
    fld qword ptr [rsp + 56] ; 01C3
    fld1 ; 01C6
    fsubp st(1), st(0) ; 01C8
    fmul qword ptr [rsp + 24] ; 01CA
    nop ; 01CD
    nop ; 01CE
    fstp qword ptr [rcx + 16] ; 01CF
    fstp qword ptr [rcx + 8] ; 01D1
    fstp qword ptr [rcx] ; 01D3
    fnop ; 01D5
    dec ah ; 01D7
    cmp ah, 0 ; 01D9
    jg TgladDihedOriginal_x87_001c ; 01DC
    fld qword ptr [rcx] ; 01E2
    fadd qword ptr [rcx + 24] ; 01E4
    fld qword ptr [rcx + 8] ; 01E7
    fadd qword ptr [rcx + 32] ; 01E9
    fld qword ptr [rcx + 16] ; 01EC
    fadd qword ptr [rcx + 40] ; 01EE
TgladDihedOriginal_x87_01f1:
    fstp qword ptr [rcx + 16] ; 01F1
    fstp qword ptr [rcx + 8] ; 01F3
    fstp qword ptr [rcx] ; 01F5
    nop ; 01F7
    jmp TgladDihedOriginal_x87_done
TgladDihedOriginal_x87_0203:
    fstp st(0) ; 0203
TgladDihedOriginal_x87_0205:
    fstp st(0) ; 0205
    fld qword ptr [rdx + 030h] ; 0207
    fld st(0) ; 020A
    fld st(0) ; 020C
    jmp TgladDihedOriginal_x87_01f1 ; 020E
TgladDihedOriginal_x87_done:
    fldcw word ptr [rsp + 96]
    add rsp, 104
    ret
TgladDihedOriginal_x87 ENDP


; Original TgladDihed2: rcx=XYZ,C; rdx=PVar. Overflow bypasses C.
TgladDihed2Original_x87 PROC
    sub rsp, 104
    fnstcw word ptr [rsp + 96]
    mov word ptr [rsp + 98], 137Fh
    fldcw word ptr [rsp + 98]
    xor eax, eax
    fnop ; 0014
    mov ah, byte ptr [rdx - 0ch] ; 0016
    and ah, 7 ; 0019
TgladDihed2Original_x87_001c:
    fld qword ptr [rcx] ; 001C
    fmul st(0), st(0) ; 001E
    fld qword ptr [rcx + 8] ; 0020
    fmul st(0), st(0) ; 0022
    faddp st(1), st(0) ; 0024
    fld st(0) ; 0026
    fld qword ptr [rcx + 16] ; 0028
    fmul st(0), st(0) ; 002A
    faddp st(1), st(0) ; 002C
    shl eax, 010h ; 002E
    fcom qword ptr [rdx + 030h] ; 0031
    wait ; 0034
    fnstsw ax ; 0035
    shr ah, 1 ; 0037
    jae TgladDihed2Original_x87_0203 ; 0039
    shr eax, 010h ; 003F
    fsqrt ; 0042
    fst qword ptr [rsp + 24] ; 0044
    fld qword ptr [rcx + 16] ; 0047
    fdivrp st(1), st(0) ; 0049
    fstp qword ptr [rsp + 88] ; 004B
    fsqrt ; 004E
    fsub qword ptr [rcx + 8] ; 0050
    fdiv qword ptr [rcx] ; 0052
    fst qword ptr [rsp + 80] ; 0054
    fmul st(0), st(0) ; 0057
    fst qword ptr [rsp + 56] ; 0059
    fld1 ; 005C
    faddp st(1), st(0) ; 005E
    fld1 ; 0060
    fdivrp st(1), st(0) ; 0062
    nop ; 0064
    fld1 ; 0065
    fadd qword ptr [rsp + 88] ; 0067
    fld1 ; 006A
    fsub qword ptr [rsp + 88] ; 006C
    fdivrp st(1), st(0) ; 006F
    fsqrt ; 0071
    nop ; 0073
    fmulp st(1), st(0) ; 0074
    fld st(0) ; 0076
    fadd st(0), st(0) ; 0078
    fmul qword ptr [rsp + 80] ; 007A
    fstp qword ptr [rcx + 8] ; 007D
    nop ; 007F
    fld1 ; 0080
    fsubr qword ptr [rsp + 56] ; 0082
    fmulp st(1), st(0) ; 0085
    fstp qword ptr [rcx] ; 0087
    fnop ; 0089
    mov al, byte ptr [rdx - 010h] ; 008B
    and al, 7 ; 008E
    nop ; 0090
TgladDihed2Original_x87_0091:
    fld qword ptr [rcx] ; 0091
    fmul st(0), st(0) ; 0093
    fld qword ptr [rcx + 8] ; 0095
    fmul st(0), st(0) ; 0097
    faddp st(1), st(0) ; 0099
    shl eax, 010h ; 009B
    fcom qword ptr [rdx + 030h] ; 009E
    wait ; 00A1
    fnstsw ax ; 00A2
    shr ah, 1 ; 00A4
    jae TgladDihed2Original_x87_0205 ; 00A6
    shr eax, 010h ; 00AC
    fstp qword ptr [rsp + 72] ; 00AF
    nop ; 00B2
    fld qword ptr [rcx + 8] ; 00B3
    fchs ; 00B5
    fld qword ptr [rsp + 72] ; 00B7
    fld1 ; 00BA
    faddp st(1), st(0) ; 00BC
    fmulp st(1), st(0) ; 00BE
    fld qword ptr [rsp + 72] ; 00C0
    fld qword ptr [rcx] ; 00C3
    fmul qword ptr [rdx + 028h] ; 00C5
    fsubp st(1), st(0) ; 00C8
    fld1 ; 00CA
    fsubp st(1), st(0) ; 00CC
    fmulp st(1), st(0) ; 00CE
    fld qword ptr [rsp + 72] ; 00D0
    fld qword ptr [rcx] ; 00D3
    fmul qword ptr [rdx + 028h] ; 00D5
    faddp st(1), st(0) ; 00D8
    fld1 ; 00DA
    fsubp st(1), st(0) ; 00DC
    fmulp st(1), st(0) ; 00DE
    fstp qword ptr [rsp + 88] ; 00E0
    fld qword ptr [rcx] ; 00E3
    fld qword ptr [rsp + 72] ; 00E5
    fld1 ; 00E8
    fsubp st(1), st(0) ; 00EA
    fmulp st(1), st(0) ; 00EC
    fld qword ptr [rsp + 72] ; 00EE
    fmul st(0), st(0) ; 00F1
    fld qword ptr [rsp + 72] ; 00F3
    fmul qword ptr [rdx + 028h] ; 00F6
    faddp st(1), st(0) ; 00F9
    fld qword ptr [rcx + 8] ; 00FB
    fmul qword ptr [rdx + 028h] ; 00FD
    fmul st(0), st(0) ; 0100
    faddp st(1), st(0) ; 0102
    fld1 ; 0104
    faddp st(1), st(0) ; 0106
    fmulp st(1), st(0) ; 0108
    fstp qword ptr [rsp + 80] ; 010A
    fld qword ptr [rsp + 72] ; 010D
    fld qword ptr [rcx + 8] ; 0110
    fmul qword ptr [rdx + 028h] ; 0112
    fsubp st(1), st(0) ; 0115
    fld1 ; 0117
    faddp st(1), st(0) ; 0119
    fld qword ptr [rsp + 72] ; 011B
    fld qword ptr [rcx + 8] ; 011E
    fmul qword ptr [rdx + 028h] ; 0120
    faddp st(1), st(0) ; 0123
    fld1 ; 0125
    faddp st(1), st(0) ; 0127
    fmulp st(1), st(0) ; 0129
    fmul st(0), st(0) ; 012B
    fld qword ptr [rdx + 028h] ; 012D
    fdivrp st(1), st(0) ; 0130
    fld st(0) ; 0132
    fmul qword ptr [rsp + 80] ; 0134
    fstp qword ptr [rcx] ; 0137
    fmul qword ptr [rsp + 88] ; 0139
    fstp qword ptr [rcx + 8] ; 013C
    fld qword ptr [rsp + 24] ; 013E
    fmul st(0), st(0) ; 0141
    fstp qword ptr [rsp + 24] ; 0143
    nop ; 0146
    nop ; 0147
    dec al ; 0148
    jg TgladDihed2Original_x87_0091 ; 014A
    fld qword ptr [rcx] ; 0150
    fld qword ptr [rcx + 8] ; 0152
    fstp qword ptr [rcx] ; 0154
    fstp qword ptr [rcx + 8] ; 0156
    fnop ; 0158
    fld qword ptr [rcx] ; 015A
    fmul st(0), st(0) ; 015C
    fld qword ptr [rcx + 8] ; 015E
    fmul st(0), st(0) ; 0160
    faddp st(1), st(0) ; 0162
    fst qword ptr [rsp + 32] ; 0164
    fsqrt ; 0167
    fst qword ptr [rsp + 72] ; 0169
    fsub qword ptr [rcx + 8] ; 016C
    fdiv qword ptr [rcx] ; 016E
    fst qword ptr [rsp + 80] ; 0170
    fmul st(0), st(0) ; 0173
    fst qword ptr [rsp + 56] ; 0175
    fld1 ; 0178
    faddp st(1), st(0) ; 017A
    fld1 ; 017C
    fdivrp st(1), st(0) ; 017E
    fst qword ptr [rsp + 64] ; 0180
    fld1 ; 0183
    fsub qword ptr [rsp + 56] ; 0185
    fmulp st(1), st(0) ; 0188
    fstp qword ptr [rsp + 40] ; 018A
    nop ; 018D
    fld qword ptr [rsp + 80] ; 018E
    fadd st(0), st(0) ; 0191
    fmul qword ptr [rsp + 64] ; 0193
    fstp qword ptr [rsp + 48] ; 0196
    nop ; 0199
    fld1 ; 019A
    fadd qword ptr [rsp + 32] ; 019C
    fld qword ptr [rdx + 028h] ; 019F
    fdivrp st(1), st(0) ; 01A2
    fst qword ptr [rsp + 56] ; 01A4
    fmul qword ptr [rsp + 72] ; 01A7
    fstp qword ptr [rsp + 64] ; 01AA
    nop ; 01AD
    nop ; 01AE
    fld qword ptr [rsp + 24] ; 01AF
    fmul qword ptr [rsp + 48] ; 01B2
    fmul qword ptr [rsp + 64] ; 01B5
    nop ; 01B8
    fld qword ptr [rsp + 24] ; 01B9
    fmul qword ptr [rsp + 40] ; 01BC
    fmul qword ptr [rsp + 64] ; 01BF
    nop ; 01C2
    fld qword ptr [rsp + 56] ; 01C3
    fld1 ; 01C6
    fsubrp st(1), st(0) ; 01C8
    fmul qword ptr [rsp + 24] ; 01CA
    nop ; 01CD
    nop ; 01CE
    fstp qword ptr [rcx + 16] ; 01CF
    fstp qword ptr [rcx + 8] ; 01D1
    fstp qword ptr [rcx] ; 01D3
    fnop ; 01D5
    dec ah ; 01D7
    cmp ah, 0 ; 01D9
    jg TgladDihed2Original_x87_001c ; 01DC
    fld qword ptr [rcx] ; 01E2
    fadd qword ptr [rcx + 24] ; 01E4
    fld qword ptr [rcx + 8] ; 01E7
    fadd qword ptr [rcx + 32] ; 01E9
    fld qword ptr [rcx + 16] ; 01EC
    fadd qword ptr [rcx + 40] ; 01EE
TgladDihed2Original_x87_01f1:
    fstp qword ptr [rcx + 16] ; 01F1
    fstp qword ptr [rcx + 8] ; 01F3
    fstp qword ptr [rcx] ; 01F5
    nop ; 01F7
    jmp TgladDihed2Original_x87_done
TgladDihed2Original_x87_0203:
    fstp st(0) ; 0203
TgladDihed2Original_x87_0205:
    fstp st(0) ; 0205
    fld qword ptr [rdx + 030h] ; 0207
    fld st(0) ; 020A
    fld st(0) ; 020C
    jmp TgladDihed2Original_x87_01f1 ; 020E
TgladDihed2Original_x87_done:
    fldcw word ptr [rsp + 96]
    add rsp, 104
    ret
TgladDihed2Original_x87 ENDP


; Original CylinderHeightMap before callback. State: XYZ,DE,trap,48-byte original scratch. RDX: packed PVar.
CylinderHeightMapPrepare_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    fld qword ptr [rcx + 16] ; 0004
    fsub dword ptr [rdx - 28] ; 0007
    fld qword ptr [rcx + 8] ; 000A
    fsub dword ptr [rdx - 24] ; 000D
    fld qword ptr [rcx] ; 0010
    fsub dword ptr [rdx - 20] ; 0013
    fld st(0) ; 0016
    fmul dword ptr [rdx - 68] ; 0018
    fld st(2) ; 001B
    fmul dword ptr [rdx - 72] ; 001D
    faddp st(1), st(0) ; 0020
    fld st(3) ; 0022
    fmul dword ptr [rdx - 76] ; 0024
    faddp st(1), st(0) ; 0027
    fld st(1) ; 0029
    fmul dword ptr [rdx - 44] ; 002B
    fld st(3) ; 002E
    fmul dword ptr [rdx - 48] ; 0030
    faddp st(1), st(0) ; 0033
    fld st(4) ; 0035
    fmul dword ptr [rdx - 52] ; 0037
    faddp st(1), st(0) ; 003A
    fxch st(2) ; 003C
    fmul dword ptr [rdx - 56] ; 003E
    fxch st(3) ; 0041
    fmul dword ptr [rdx - 60] ; 0043
    faddp st(3), st(0) ; 0046
    fxch st(3) ; 0048
    fmul dword ptr [rdx - 64] ; 004A
    faddp st(2), st(0) ; 004D
    fld st(1) ; 004F
    fmul st(0), st(0) ; 0051
    fld st(1) ; 0053
    fmul st(0), st(0) ; 0055
    faddp st(1), st(0) ; 0057
    fsqrt ; 0059
    fst dword ptr [rcx + 68] ; 005B
    fsub dword ptr [rdx - 36] ; 005F
    fld st(3) ; 0062
    fabs ; 0064
    fsub dword ptr [rdx - 32] ; 0066
    fcom st(1) ; 0069
    fnstsw ax ; 006B
    shr ah, 1 ; 006D
    jb cylinderheightmap_prep_0073 ; 006F
    fxch st(1) ; 0071
cylinderheightmap_prep_0073:
    fstp st(0) ; 0073
    fld st(0) ; 0075
    fabs ; 0077
    fld dword ptr [rdx - 40] ; 0079
    fabs ; 007C
    fcompp ; 007E
    fnstsw ax ; 0080
    shr ah, 1 ; 0082
    jae cylinderheightmap_prep_0092 ; 0084
    fstp qword ptr [rcx + 24] ; 0086
    fcompp ; 0089
    fstp st(0) ; 008B
    xor eax, eax
    jmp CylinderHeightMapPrepare_x87_done
cylinderheightmap_prep_0092:
    fstp dword ptr [rcx + 64] ; 0092
    fld st(1) ; 0096
    fpatan ; 0098
    fmul qword ptr [rdx] ; 009A
    fstp qword ptr [rcx + 40] ; 009C
    fstp st(0) ; 009F
    fmul qword ptr [rdx] ; 00A1
    fdiv dword ptr [rdx - 36] ; 00A3
    fstp qword ptr [rcx + 48] ; 00A6
    mov eax, 1
CylinderHeightMapPrepare_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
CylinderHeightMapPrepare_x87 ENDP


; Original CylinderHeightMap after callback; channel 3 retains original scratch alias.
CylinderHeightMapFinish_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    mov r8d, dword ptr [rdx - 16] ; 00B7
    and r8d, 3 ; 00BA
    fld qword ptr [rcx + r8*8 + 40] ; 00BD
    fsub qword ptr [rdx - 8] ; 00C0
    fmul dword ptr [rdx - 40] ; 00C3
    fmul dword ptr [rcx + 68] ; 00C6
    fsubr dword ptr [rcx + 64] ; 00CA
    fstp qword ptr [rcx + 24] ; 00CE
    mov r8d, dword ptr [rdx - 80] ; 00D1
    and r8d, 3 ; 00D4
    fld qword ptr [rcx + r8*8 + 40] ; 00D7
    fadd dword ptr [rdx - 84] ; 00DA
    fmul dword ptr [rdx - 88] ; 00DD
    fstp qword ptr [rcx + 32] ; 00E0
CylinderHeightMapFinish_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
CylinderHeightMapFinish_x87 ENDP


; Original CylinderHMVoid before callback. State: XYZ,DE,trap,48-byte original scratch. RDX: packed PVar.
CylinderHMVoidPrepare_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    fld qword ptr [rcx + 16] ; 0004
    fsub dword ptr [rdx - 28] ; 0007
    fld qword ptr [rcx + 8] ; 000A
    fsub dword ptr [rdx - 24] ; 000D
    fld qword ptr [rcx] ; 0010
    fsub dword ptr [rdx - 20] ; 0013
    fld st(0) ; 0016
    fmul dword ptr [rdx - 68] ; 0018
    fld st(2) ; 001B
    fmul dword ptr [rdx - 72] ; 001D
    faddp st(1), st(0) ; 0020
    fld st(3) ; 0022
    fmul dword ptr [rdx - 76] ; 0024
    faddp st(1), st(0) ; 0027
    fld st(1) ; 0029
    fmul dword ptr [rdx - 44] ; 002B
    fld st(3) ; 002E
    fmul dword ptr [rdx - 48] ; 0030
    faddp st(1), st(0) ; 0033
    fld st(4) ; 0035
    fmul dword ptr [rdx - 52] ; 0037
    faddp st(1), st(0) ; 003A
    fxch st(2) ; 003C
    fmul dword ptr [rdx - 56] ; 003E
    fxch st(3) ; 0041
    fmul dword ptr [rdx - 60] ; 0043
    faddp st(3), st(0) ; 0046
    fxch st(3) ; 0048
    fmul dword ptr [rdx - 64] ; 004A
    faddp st(2), st(0) ; 004D
    fld st(1) ; 004F
    fmul st(0), st(0) ; 0051
    fld st(1) ; 0053
    fmul st(0), st(0) ; 0055
    faddp st(1), st(0) ; 0057
    fsqrt ; 0059
    fld st(3) ; 005B
    fmul dword ptr [rdx - 92] ; 005D
    faddp st(1), st(0) ; 0060
    fst dword ptr [rcx + 68] ; 0062
    fsub dword ptr [rdx - 36] ; 0066
    fld st(3) ; 0069
    fabs ; 006B
    fsub dword ptr [rdx - 32] ; 006D
    fcom st(1) ; 0070
    fnstsw ax ; 0072
    shr ah, 1 ; 0074
    jb cylinderhmvoid_prep_007a ; 0076
    fxch st(1) ; 0078
cylinderhmvoid_prep_007a:
    fstp st(0) ; 007A
    fld st(0) ; 007C
    fabs ; 007E
    fld dword ptr [rdx - 40] ; 0080
    fabs ; 0083
    fcompp ; 0085
    fnstsw ax ; 0087
    shr ah, 1 ; 0089
    jae cylinderhmvoid_prep_009a ; 008B
    fabs ; 008D
    fstp qword ptr [rcx + 24] ; 008F
    fstp st(0) ; 0092
    fstp st(0) ; 0094
    fstp st(0) ; 0096
    xor eax, eax
    jmp CylinderHMVoidPrepare_x87_done
cylinderhmvoid_prep_009a:
    fstp dword ptr [rcx + 64] ; 009A
    fld st(1) ; 009E
    fpatan ; 00A0
    fmul qword ptr [rdx] ; 00A2
    fstp qword ptr [rcx + 40] ; 00A4
    fstp st(0) ; 00A7
    fmul qword ptr [rdx] ; 00A9
    fdiv dword ptr [rdx - 36] ; 00AB
    fstp qword ptr [rcx + 48] ; 00AE
    mov eax, 1
CylinderHMVoidPrepare_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
CylinderHMVoidPrepare_x87 ENDP


; Original CylinderHMVoid after callback; channel 3 retains original scratch alias.
CylinderHMVoidFinish_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    mov r8d, dword ptr [rdx - 16] ; 00BF
    and r8d, 3 ; 00C2
    fld qword ptr [rcx + r8*8 + 40] ; 00C5
    fsub qword ptr [rdx - 8] ; 00C8
    fmul dword ptr [rdx - 40] ; 00CB
    fmul dword ptr [rcx + 68] ; 00CE
    fsubr dword ptr [rcx + 64] ; 00D2
    fabs ; 00D6
    fsub qword ptr [rdx + 8] ; 00D8
    fstp qword ptr [rcx + 24] ; 00DB
    mov r8d, dword ptr [rdx - 80] ; 00DE
    and r8d, 3 ; 00E1
    fld qword ptr [rcx + r8*8 + 40] ; 00E4
    fadd dword ptr [rdx - 84] ; 00E7
    fmul dword ptr [rdx - 88] ; 00EA
    fstp qword ptr [rcx + 32] ; 00ED
CylinderHMVoidFinish_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
CylinderHMVoidFinish_x87 ENDP


; Original TorusHMVoid before callback. State: XYZ,DE,trap,48-byte original scratch. RDX: packed PVar.
TorusHMVoidPrepare_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    fld qword ptr [rcx + 16] ; 0004
    fsub dword ptr [rdx - 28] ; 0007
    fld qword ptr [rcx + 8] ; 000A
    fsub dword ptr [rdx - 24] ; 000D
    fld qword ptr [rcx] ; 0010
    fsub dword ptr [rdx - 20] ; 0013
    fld st(0) ; 0016
    fmul dword ptr [rdx - 68] ; 0018
    fld st(2) ; 001B
    fmul dword ptr [rdx - 72] ; 001D
    faddp st(1), st(0) ; 0020
    fld st(3) ; 0022
    fmul dword ptr [rdx - 76] ; 0024
    faddp st(1), st(0) ; 0027
    fld st(1) ; 0029
    fmul dword ptr [rdx - 44] ; 002B
    fld st(3) ; 002E
    fmul dword ptr [rdx - 48] ; 0030
    faddp st(1), st(0) ; 0033
    fld st(4) ; 0035
    fmul dword ptr [rdx - 52] ; 0037
    faddp st(1), st(0) ; 003A
    fxch st(2) ; 003C
    fmul dword ptr [rdx - 56] ; 003E
    fxch st(3) ; 0041
    fmul dword ptr [rdx - 60] ; 0043
    faddp st(3), st(0) ; 0046
    fxch st(3) ; 0048
    fmul dword ptr [rdx - 64] ; 004A
    faddp st(2), st(0) ; 004D
    fstp qword ptr [rcx + 48] ; 004F
    fstp qword ptr [rcx + 56] ; 0053
    fstp qword ptr [rcx + 64] ; 0057
    fld qword ptr [rcx + 48] ; 005B
    fmul st(0), st(0) ; 005F
    fld qword ptr [rcx + 56] ; 0061
    fmul st(0), st(0) ; 0065
    faddp st(1), st(0) ; 0067
    fsqrt ; 0069
    fsub dword ptr [rdx - 32] ; 006B
    fld st(0) ; 006E
    fmul st(0), st(0) ; 0070
    fld qword ptr [rcx + 64] ; 0072
    fmul st(0), st(0) ; 0076
    faddp st(1), st(0) ; 0078
    fsqrt ; 007A
    fsub dword ptr [rdx - 36] ; 007C
    nop ; 007F
    fld st(0) ; 0080
    fabs ; 0082
    fld dword ptr [rdx - 40] ; 0084
    fabs ; 0087
    fadd qword ptr [rdx + 8] ; 0089
    fadd qword ptr [rdx + 8] ; 008C
    fcompp ; 008F
    fnstsw ax ; 0091
    shr ah, 1 ; 0093
    jae torushmvoid_prep_00a4 ; 0095
    fabs ; 0097
    fstp qword ptr [rcx + 24] ; 0099
    fstp st(0) ; 009C
    fstp st(0) ; 009E
    fstp st(0) ; 00A0
    xor eax, eax
    jmp TorusHMVoidPrepare_x87_done
torushmvoid_prep_00a4:
    fld st(1) ; 00A4
    fld qword ptr [rcx + 64] ; 00A6
    fpatan ; 00AA
    fmul qword ptr [rdx] ; 00AC
    fld qword ptr [rcx + 48] ; 00AE
    fld qword ptr [rcx + 56] ; 00B2
    fpatan ; 00B6
    fmul qword ptr [rdx] ; 00B8
    nop ; 00BA
    fstp qword ptr [rcx + 40] ; 00BB
    fstp qword ptr [rcx + 48] ; 00BE
    fstp dword ptr [rcx + 64] ; 00C2
    fstp st(0) ; 00C6
    mov eax, 1
TorusHMVoidPrepare_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
TorusHMVoidPrepare_x87 ENDP


; Original TorusHMVoid after callback; channel 3 retains original scratch alias.
TorusHMVoidFinish_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    mov r8d, dword ptr [rdx - 16] ; 00D5
    and r8d, 3 ; 00D8
    fld qword ptr [rcx + r8*8 + 40] ; 00DB
    fsub qword ptr [rdx - 8] ; 00DE
    fmul dword ptr [rdx - 40] ; 00E1
    fsubr dword ptr [rcx + 64] ; 00E4
    fabs ; 00E8
    fsub qword ptr [rdx + 8] ; 00EA
    fstp qword ptr [rcx + 24] ; 00ED
    mov r8d, dword ptr [rdx - 80] ; 00F0
    and r8d, 3 ; 00F3
    fld qword ptr [rcx + r8*8 + 40] ; 00F6
    fadd dword ptr [rdx - 84] ; 00F9
    fmul dword ptr [rdx - 88] ; 00FC
    fstp qword ptr [rcx + 32] ; 00FF
TorusHMVoidFinish_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
TorusHMVoidFinish_x87 ENDP


; Original nonzero integer powers, logical shifts and complex products. RCX: XYZ,Rout,Cxyz. RDX: packed PVar.
TrifoxComplexAnglesOriginal_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    fld1 ; 000B
    mov eax, dword ptr [rdx - 12] ; 000D
    fld qword ptr [rcx + 24] ; 0010
    fsqrt ; 0013
    jmp trifox_0019 ; 0015
trifox_0017:
    fmul st(0), st(0) ; 0017
trifox_0019:
    shr eax, 1 ; 0019
    jae trifox_0017 ; 001B
    fmul st(1), st(0) ; 001D
    jne trifox_0017 ; 001F
    fstp st(0) ; 0021
    fldz ; 0023
    fld1 ; 0025
    fld qword ptr [rcx] ; 0027
    fld qword ptr [rcx + 8] ; 002A
    fld qword ptr [rcx + 16] ; 002C
    fld st(2) ; 002E
    fmul st(0), st(0) ; 0030
    fld st(2) ; 0032
    fmul st(0), st(0) ; 0034
    faddp st(1), st(0) ; 0036
    fsqrt ; 0038
    fpatan ; 003A
    fxch st(2) ; 003C
    fpatan ; 003E
    fld dword ptr [rdx - 16] ; 0040
    fmul st(2), st(0) ; 0043
    fmulp st(1), st(0) ; 0045
    cmp dword ptr [rdx - 24], 0 ; 0047
    je trifox_004f ; 004B
    fxch st(1) ; 004D
trifox_004f:
    cmp dword ptr [rdx - 32], 0 ; 004F
    je trifox_006a ; 0053
    fld st(1) ; 0055
    fmul st(0), st(0) ; 0057
    fld st(1) ; 0059
    fmul st(0), st(0) ; 005B
    faddp st(1), st(0) ; 005D
    fadd dword ptr [rdx - 36] ; 005F
    fld1 ; 0062
    fdivrp st(1), st(0) ; 0064
    fmul st(2), st(0) ; 0066
    fmulp st(1), st(0) ; 0068
trifox_006a:
    mov eax, dword ptr [rdx - 28] ; 006A
    jmp trifox_007f ; 006D
trifox_006f:
    fld st(1) ; 006F
    fmul st(0), st(1) ; 0071
    fadd st(0), st(0) ; 0073
    fxch st(2) ; 0075
    fmul st(0), st(0) ; 0077
    fxch st(1) ; 0079
    fmul st(0), st(0) ; 007B
    fsubrp st(1), st(0) ; 007D
trifox_007f:
    shr eax, 1 ; 007F
    jae trifox_006f ; 0081
    fld st(2) ; 0083
    fmul st(0), st(1) ; 0085
    fld st(4) ; 0087
    fmul st(0), st(3) ; 0089
    fsubp st(1), st(0) ; 008B
    fxch st(3) ; 008D
    fmul st(0), st(2) ; 008F
    fld st(1) ; 0091
    fmulp st(5), st(0) ; 0093
    faddp st(4), st(0) ; 0095
    jne trifox_006f ; 0097
    fcompp ; 0099
    fld dword ptr [rdx - 20] ; 009B
    fmul st(2), st(0) ; 009E
    fmulp st(1), st(0) ; 00A0
    fsincos ; 00A2
    fxch st(2) ; 00A4
    fsincos ; 00A6
    fmul st(0), st(3) ; 00A8
    fmul st(0), st(4) ; 00AA
    fadd qword ptr [rcx + 32] ; 00AC
    fstp qword ptr [rcx] ; 00AF
    fmulp st(2), st(0) ; 00B2
    fmul st(0), st(2) ; 00B4
    fmul dword ptr [rdx - 40] ; 00B6
    fadd qword ptr [rcx + 48] ; 00B9
    fstp qword ptr [rcx + 16] ; 00BC
    fmulp st(1), st(0) ; 00BE
    fadd qword ptr [rcx + 40] ; 00C0
    fstp qword ptr [rcx + 8] ; 00C3
TrifoxComplexAnglesOriginal_x87_done:
    fldcw word ptr [rsp]
    add rsp, 8
    ret
TrifoxComplexAnglesOriginal_x87 ENDP

; PG-bulb.m3f 002B..017A, original x87 products and Double spills.
; RCX: XYZ,W,JuliaXYZ,JuliaW. RDX: expanded params. R8D: checked power1..16.
; R9D: Old angles. Fixed coordinates stay unchanged until the final stores.
PGBulbOriginal_x87 PROC
    sub rsp, 64
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    fld qword ptr [rcx]
    fstp qword ptr [rsp + 8]
    fld qword ptr [rcx + 8]
    fstp qword ptr [rsp + 16]
    fld qword ptr [rcx + 16]
    fstp qword ptr [rsp + 24]
    fld qword ptr [rcx + 24]
    fadd qword ptr [rdx]
    fstp qword ptr [rsp + 32]
    mov eax, r8d
    test r9d, r9d
    jne pg_bulb_0075
    fld      qword ptr [rcx] ; 004D
    fmul st(0), st(0) ; 004F
    fld      qword ptr [rcx + 8] ; 0051
    fmul st(0), st(0) ; 0053
    faddp    st(1), st(0) ; 0055
    fld      qword ptr [rcx] ; 0057
    fmul st(0), st(0) ; 0059
    fld      qword ptr [rcx + 8] ; 005B
    fmul st(0), st(0) ; 005D
    fsubp    st(1), st(0) ; 005F
    fdiv st(0), st(1) ; 0061
    fstp     qword ptr [rsp + 40] ; 0063
    fld      qword ptr [rcx] ; 0066
    fmul     qword ptr [rcx + 8] ; 0068
    fdiv st(0), st(1) ; 006A
    fstp     qword ptr [rsp + 48] ; 006C
    fstp     st(0) ; 006F
    jmp      pg_bulb_009f ; 0071
pg_bulb_0075:
    fld      qword ptr [rcx + 32] ; 0075
    fmul st(0), st(0) ; 0078
    fld      qword ptr [rcx + 40] ; 007A
    fmul st(0), st(0) ; 007D
    faddp    st(1), st(0) ; 007F
    fld      qword ptr [rcx + 32] ; 0081
    fmul st(0), st(0) ; 0084
    fld      qword ptr [rcx + 40] ; 0086
    fmul st(0), st(0) ; 0089
    fsubp    st(1), st(0) ; 008B
    fdiv st(0), st(1) ; 008D
    fstp     qword ptr [rsp + 40] ; 008F
    fld      qword ptr [rcx + 32] ; 0092
    fmul     qword ptr [rcx + 40] ; 0095
    fdiv st(0), st(1) ; 0098
    fstp     qword ptr [rsp + 48] ; 009A
    fstp     st(0) ; 009D
pg_bulb_009f:
    dec      eax ; 009F
pg_bulb_00a1:
    dec      eax ; 00A1
    fld      qword ptr [rsp + 8] ; 00A2
    fmul     qword ptr [rcx] ; 00A5
    fld      qword ptr [rsp + 16] ; 00A7
    fmul     qword ptr [rcx + 8] ; 00AA
    fsubp    st(1), st(0) ; 00AC
    fld      qword ptr [rsp + 40] ; 00AE
    fmul     qword ptr [rsp + 24] ; 00B1
    fmul     qword ptr [rcx + 16] ; 00B4
    fsubp    st(1), st(0) ; 00B6
    fld      qword ptr [rsp + 24] ; 00B8
    fmul     qword ptr [rcx + 24] ; 00BB
    fld      qword ptr [rsp + 32] ; 00BE
    fmul     qword ptr [rcx + 16] ; 00C1
    faddp    st(1), st(0) ; 00C3
    fmul     qword ptr [rsp + 48] ; 00C5
    faddp    st(1), st(0) ; 00C8
    fld      qword ptr [rsp + 40] ; 00CA
    fmul     qword ptr [rsp + 32] ; 00CD
    fmul     qword ptr [rcx + 24] ; 00D0
    faddp    st(1), st(0) ; 00D3
    fld      qword ptr [rsp + 8] ; 00D7
    fmul     qword ptr [rcx + 8] ; 00DA
    fld      qword ptr [rsp + 16] ; 00DD
    fmul     qword ptr [rcx] ; 00E0
    faddp    st(1), st(0) ; 00E2
    fld      qword ptr [rsp + 48] ; 00E4
    fmul     qword ptr [rsp + 24] ; 00E7
    fmul     qword ptr [rcx + 16] ; 00EA
    fsubp    st(1), st(0) ; 00EC
    fld      qword ptr [rsp + 24] ; 00EE
    fmul     qword ptr [rcx + 24] ; 00F1
    fld      qword ptr [rsp + 32] ; 00F4
    fmul     qword ptr [rcx + 16] ; 00F7
    faddp    st(1), st(0) ; 00F9
    fmul     qword ptr [rsp + 40] ; 00FB
    fsubp    st(1), st(0) ; 00FE
    fld      qword ptr [rsp + 48] ; 0100
    fmul     qword ptr [rsp + 32] ; 0103
    fmul     qword ptr [rcx + 24] ; 0106
    faddp    st(1), st(0) ; 0109
    fld      qword ptr [rsp + 8] ; 010D
    fmul     qword ptr [rcx + 16] ; 0110
    fld      qword ptr [rsp + 24] ; 0113
    fmul     qword ptr [rcx] ; 0116
    faddp    st(1), st(0) ; 0118
    fld      qword ptr [rsp + 16] ; 011A
    fmul     qword ptr [rcx + 24] ; 011D
    fsubp    st(1), st(0) ; 0120
    fld      qword ptr [rsp + 32] ; 0122
    fmul     qword ptr [rcx + 8] ; 0125
    fsubp    st(1), st(0) ; 0128
    fld      qword ptr [rsp + 8] ; 012C
    fmul     qword ptr [rcx + 24] ; 012F
    fld      qword ptr [rsp + 32] ; 0132
    fmul     qword ptr [rcx] ; 0135
    faddp    st(1), st(0) ; 0137
    fld      qword ptr [rsp + 16] ; 0139
    fmul     qword ptr [rcx + 16] ; 013C
    faddp    st(1), st(0) ; 013F
    fld      qword ptr [rsp + 24] ; 0141
    fmul     qword ptr [rcx + 8] ; 0144
    faddp    st(1), st(0) ; 0147
    cmp      eax, 0 ; 014B
    jle      pg_bulb_0165 ; 014E
    fstp     qword ptr [rsp + 32] ; 0154
    fstp     qword ptr [rsp + 24] ; 0157
    fstp     qword ptr [rsp + 16] ; 015A
    fstp     qword ptr [rsp + 8] ; 015D
    jmp      pg_bulb_00a1 ; 0160
pg_bulb_0165:
    fadd     qword ptr [rcx + 56] ; 0167
    fstp     qword ptr [rcx + 24] ; 016A
    fadd     qword ptr [rcx + 48] ; 016D
    fstp     qword ptr [rcx + 16] ; 0170
    fadd     qword ptr [rcx + 40] ; 0172
    fstp     qword ptr [rcx + 8] ; 0175
    fadd     qword ptr [rcx + 32] ; 0177
    fstp     qword ptr [rcx] ; 017A
    fldcw word ptr [rsp]
    add rsp, 64
    ret
PGBulbOriginal_x87 ENDP

; GeneralQuat.m3f 000E..00B0. RCX: XYZ,W,JuliaXYZ,JuliaW.
; RDX: eight int32 selectors. Keep all cross-products in x87 until the
; original qword accumulator stores; use the original 53-bit x87 profile.
GeneralQuatOriginal_x87 PROC
    sub rsp, 40
    fnstcw word ptr [rsp + 32]
    mov word ptr [rsp + 34], 137Fh
    fldcw word ptr [rsp + 34]
    fld qword ptr [rcx + 24]
    fld qword ptr [rcx + 16]
    fld qword ptr [rcx]
    fld qword ptr [rcx + 8]
    fld st(1)
    fmul st(0), st(0)
    fld st(1)
    fmul st(0), st(0)
    fsubp st(1), st(0)
    fstp qword ptr [rsp]
    fld st(1)
    fmul st(0), st(1)
    fadd st(0), st(0)
    fstp qword ptr [rsp + 8]
    fld st(1)
    fmul st(0), st(3)
    fadd st(0), st(0)
    fstp qword ptr [rsp + 16]
    fld st(1)
    fmul st(0), st(4)
    fadd st(0), st(0)
    fstp qword ptr [rsp + 24]
    fld st(3)
    fmul st(4), st(0)
    fmul st(3), st(0)
    fld st(3)
    fxch st(3)
    fxch st(1)
    fmul st(0), st(2)
    fld st(0)
    fxch st(3)
    fxch st(1)
    fxch st(2)
    fstp st(0)
    fld qword ptr [rcx + 16]
    fld st(0)
    fmul st(0), st(0)
    fxch st(2)
    fmulp st(1), st(0)
    fld st(0)
    xor r8d, r8d
general_quat_term:
    mov eax, dword ptr [rdx + r8*4]
    dec eax
    jns general_quat_positive
    neg eax
    sub eax, 2
    fchs
general_quat_positive:
    and eax, 3
    fadd qword ptr [rsp + rax*8]
    fstp qword ptr [rsp + rax*8]
    inc r8d
    cmp r8d, 8
    jb general_quat_term
    fld qword ptr [rsp + 24]
    fld qword ptr [rsp + 16]
    fld qword ptr [rsp + 8]
    fld qword ptr [rsp]
    fadd qword ptr [rcx + 32]
    fstp qword ptr [rcx]
    fadd qword ptr [rcx + 40]
    fstp qword ptr [rcx + 8]
    fadd qword ptr [rcx + 48]
    fstp qword ptr [rcx + 16]
    fadd qword ptr [rcx + 56]
    fstp qword ptr [rcx + 24]
    fldcw word ptr [rsp + 32]
    add rsp, 40
    ret
GeneralQuatOriginal_x87 ENDP

; Original _recFold CODE 000E..0086. RCX XYZ, RDX expanded options:
; FoldX, Asymm, P, Q, integer-count-as-Double. Preserve the original
; x87 extended sums until each explicit qword store; double-only sums
; lose cancellation bits near powers of two. Y is never written.
RecFoldOriginal_x87 PROC
    sub rsp, 16
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    mov r8, 3FE0000000000000h
    mov qword ptr [rsp + 8], r8
    cvttsd2si r8d, qword ptr [rdx + 32]
    fld qword ptr [rcx]
    fabs
    fld qword ptr [rcx + 16]
    fabs
    fld st(0)
    fadd st(0), st(2)
    fld st(1)
    fsub st(0), st(3)
    fsub qword ptr [rdx + 8]
    fstp qword ptr [rcx]
    fsub qword ptr [rdx + 16]
    fstp qword ptr [rcx + 16]
    fstp st(0)
    fstp st(0)
    fld qword ptr [rcx]
    fabs
    fchs
    fld qword ptr [rcx + 16]
    fld st(0)
    fsubr st(0), st(2)
    fld st(1)
    fadd st(0), st(3)
    fchs
    fsub qword ptr [rdx + 24]
    fmul qword ptr [rsp + 8]
    fstp qword ptr [rcx]
    fsub qword ptr [rdx + 24]
    fmul qword ptr [rsp + 8]
    fstp qword ptr [rcx + 16]
    fstp st(0)
    fstp st(0)
    fld qword ptr [rdx]
    fld qword ptr [rcx]
    ftst
    fstsw ax
    shr ah, 1
    jbe recfold_done
    mov eax, r8d
    dec eax
    jle recfold_done
    cmp eax, 999
    jg recfold_done
recfold_loop:
    fsubr st(0), st(1)
    fabs
    fsubr st(0), st(1)
    fabs
    dec eax
    jg recfold_loop
recfold_done:
    fstp st(1)
    fstp qword ptr [rcx]
    fldcw word ptr [rsp]
    add rsp, 16
    ret
RecFoldOriginal_x87 ENDP

; Original ABoxMod4d CODE 0x0b..0x87. RCX: x,y,z,w,Deriv1;
; RDX: Scale,Fold,Wadd,epsilon,Jx,Jy,Jz,Jw. Preserve all extended
; temporaries until the original stores; no caller-visible CW change.
ABoxMod4dOriginal_x87 PROC
    sub rsp, 8
    fnstcw word ptr [rsp]
    mov word ptr [rsp + 2], 137Fh
    fldcw word ptr [rsp + 2]
    fld qword ptr [rdx + 8]
    fld qword ptr [rcx + 24]
    fadd st(0), st(1)
    fabs
    fsub st(0), st(1)
    fld qword ptr [rcx + 16]
    fadd st(0), st(2)
    fabs
    fsub st(0), st(2)
    fld qword ptr [rcx + 8]
    fadd st(0), st(3)
    fabs
    fsub st(0), st(3)
    fld qword ptr [rcx]
    fadd st(0), st(4)
    fabs
    fsub st(0), st(4)
    fld qword ptr [rdx]
    fld st(4)
    fmul st(0), st(0)
    fld st(4)
    fmul st(0), st(0)
    faddp st(1), st(0)
    fld st(3)
    fmul st(0), st(0)
    faddp st(1), st(0)
    fld st(2)
    fmul st(0), st(0)
    faddp st(1), st(0)
    fadd qword ptr [rdx + 24]
    fld1
    fcomp st(1)
    fnstsw ax
    shr ah, 1
    jb aboxmod4d_multiplier_ready
    fdiv st(1), st(0)
aboxmod4d_multiplier_ready:
    fstp st(0)
    fmul st(4), st(0)
    fmul st(3), st(0)
    fmul st(2), st(0)
    fmul st(1), st(0)
    fmul qword ptr [rcx + 32]
    fstp qword ptr [rcx + 32]
    fadd qword ptr [rdx + 32]
    fstp qword ptr [rcx]
    fadd qword ptr [rdx + 40]
    fstp qword ptr [rcx + 8]
    fadd qword ptr [rdx + 48]
    fstp qword ptr [rcx + 16]
    fadd qword ptr [rdx + 56]
    fadd qword ptr [rdx + 16]
    fstp qword ptr [rcx + 24]
    fstp st(0)
    fldcw word ptr [rsp]
    add rsp, 8
    ret
ABoxMod4dOriginal_x87 ENDP

END
