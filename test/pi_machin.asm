# Machin (1706) : pi = 16 atan(1/5) - 4 atan(1/239), atan(x) = sum (-1)^k x^(2k+1) / (2k+1)
# Real loop of 13 iterations (BNEC, compact branch without delay slot), both series updated together with fused MADDF.D.
ORI $8, $0, 1
MTC1 $8, $f1
CVT.D.W $f1, $f1
ORI $8, $0, 2
MTC1 $8, $f9
CVT.D.W $f9, $f9
ORI $8, $0, 16
MTC1 $8, $f4
CVT.D.W $f4, $f4
ORI $8, $0, 4
MTC1 $8, $f5
CVT.D.W $f5, $f5
ORI $8, $0, 1
MTC1 $8, $f10
CVT.D.W $f10, $f10
ORI $8, $0, 0
MTC1 $8, $f11
CVT.D.W $f11, $f11
ORI $8, $0, 0
MTC1 $8, $f12
CVT.D.W $f12, $f12
ORI $8, $0, 1
MTC1 $8, $f8
CVT.D.W $f8, $f8
ORI $8, $0, 5
MTC1 $8, $f6
CVT.D.W $f6, $f6
DIV.D $f6, $f1, $f6
ORI $8, $0, 239
MTC1 $8, $f7
CVT.D.W $f7, $f7
DIV.D $f7, $f1, $f7
ORI $8, $0, 25
MTC1 $8, $f2
CVT.D.W $f2, $f2
DIV.D $f2, $f1, $f2
ORI $8, $0, 239
MTC1 $8, $f3
CVT.D.W $f3, $f3
MUL.D $f3, $f3, $f3
DIV.D $f3, $f1, $f3
# f6 = 1/5^(2k+1), f7 = 1/239^(2k+1), f8 = 2k+1, f10 = (-1)^k, f11 / f12 = partial atan sums, $9 = counter, $10 = limit
ORI $9, $0, 0
ORI $10, $0, 13
DIV.D $f13, $f6, $f8
MADDF.D $f11, $f10, $f13
DIV.D $f14, $f7, $f8
MADDF.D $f12, $f10, $f14
MUL.D $f6, $f6, $f2
MUL.D $f7, $f7, $f3
ADD.D $f8, $f8, $f9
NEG.D $f10, $f10
ADDIU $9, $9, 1
BNEC $9, $10, -10
MUL.D $f20, $f4, $f11
MSUBF.D $f20, $f5, $f12
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
SUB.D $f27, $f20, $f26
ABS.D $f28, $f27
CMP.LT.D $f30, $f28, $f29
MFC1 $2, $f30
ANDI $2, $2, 1
# $f0 = result, $f27 = signed error, $2 = 1 if |error| < threshold, $3 = CLASS.D mask of the result (128 = positive normal)
MOV.D $f0, $f20
CLASS.D $f31, $f0
MFC1 $3, $f31
# $4 / $5 = low / high word of the result
MFC1 $4, $f0
MFHC1 $5, $f0
