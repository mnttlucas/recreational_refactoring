# Gauss-Legendre (AGM) : a0=1, b0=1/sqrt(2), t0=1/4, p0=1 ; a,b <- (a+b)/2, sqrt(ab) ; t <- t - p (a-a_new)^2 ; p <- 2p
# pi ~ (a+b)^2 / (4t). The number of correct digits doubles each iteration. Estimate i is left in $f(20+i).
ORI $8, $0, 1
MTC1 $8, $f1
CVT.D.W $f1, $f1
ORI $8, $0, 1
MTC1 $8, $f4
CVT.D.W $f4, $f4
ORI $8, $0, 4
MTC1 $8, $f6
CVT.D.W $f6, $f6
ORI $8, $0, 2
MTC1 $8, $f7
CVT.D.W $f7, $f7
SQRT.D $f2, $f7
DIV.D $f2, $f1, $f2
DIV.D $f5, $f1, $f7
MUL.D $f3, $f5, $f5
# --- iteration 1 ---
ADD.D $f8, $f1, $f2
MUL.D $f8, $f8, $f5
MUL.D $f9, $f1, $f2
SQRT.D $f9, $f9
SUB.D $f10, $f1, $f8
MUL.D $f10, $f10, $f10
MSUBF.D $f3, $f4, $f10
ADD.D $f4, $f4, $f4
MOV.D $f1, $f8
MOV.D $f2, $f9
ADD.D $f11, $f1, $f2
MUL.D $f11, $f11, $f11
MUL.D $f12, $f3, $f6
DIV.D $f20, $f11, $f12
# --- iteration 2 ---
ADD.D $f8, $f1, $f2
MUL.D $f8, $f8, $f5
MUL.D $f9, $f1, $f2
SQRT.D $f9, $f9
SUB.D $f10, $f1, $f8
MUL.D $f10, $f10, $f10
MSUBF.D $f3, $f4, $f10
ADD.D $f4, $f4, $f4
MOV.D $f1, $f8
MOV.D $f2, $f9
ADD.D $f11, $f1, $f2
MUL.D $f11, $f11, $f11
MUL.D $f12, $f3, $f6
DIV.D $f21, $f11, $f12
# --- iteration 3 ---
ADD.D $f8, $f1, $f2
MUL.D $f8, $f8, $f5
MUL.D $f9, $f1, $f2
SQRT.D $f9, $f9
SUB.D $f10, $f1, $f8
MUL.D $f10, $f10, $f10
MSUBF.D $f3, $f4, $f10
ADD.D $f4, $f4, $f4
MOV.D $f1, $f8
MOV.D $f2, $f9
ADD.D $f11, $f1, $f2
MUL.D $f11, $f11, $f11
MUL.D $f12, $f3, $f6
DIV.D $f22, $f11, $f12
# --- verification : error against the double closest to pi, then pass flag ---
LUI $8, 16393
ORI $8, $8, 8699
LUI $9, 21572
ORI $9, $9, 11544
MTC1 $9, $f26
MTHC1 $8, $f26
LUI $8, 15570
ORI $8, $8, 943
LUI $9, 40679
ORI $9, $9, 22038
MTC1 $9, $f29
MTHC1 $8, $f29
SUB.D $f27, $f22, $f26
ABS.D $f28, $f27
CMP.LT.D $f30, $f28, $f29
MFC1 $2, $f30
ANDI $2, $2, 1
# $f0 = result, $f27 = signed error, $2 = 1 if |error| < threshold, $3 = CLASS.D mask of the result (128 = positive normal)
MOV.D $f0, $f22
CLASS.D $f31, $f0
MFC1 $3, $f31
# $4 / $5 = low / high word of the result
MFC1 $4, $f0
MFHC1 $5, $f0
