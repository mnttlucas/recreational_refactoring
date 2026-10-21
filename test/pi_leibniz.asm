# Leibniz-Gregory : pi/4 = 1 - 1/3 + 1/5 - ... The baseline : the error is about 1/N, so it takes N = 1000 terms to get ~3 digits.
ORI $8, $0, 1
MTC1 $8, $f1
CVT.D.W $f1, $f1
ORI $8, $0, 2
MTC1 $8, $f9
CVT.D.W $f9, $f9
ORI $8, $0, 4
MTC1 $8, $f4
CVT.D.W $f4, $f4
ORI $8, $0, 1
MTC1 $8, $f8
CVT.D.W $f8, $f8
ORI $8, $0, 1
MTC1 $8, $f10
CVT.D.W $f10, $f10
ORI $8, $0, 0
MTC1 $8, $f11
CVT.D.W $f11, $f11
ORI $9, $0, 0
ORI $10, $0, 1000
DIV.D $f13, $f10, $f8
ADD.D $f11, $f11, $f13
ADD.D $f8, $f8, $f9
NEG.D $f10, $f10
ADDIU $9, $9, 1
BNEC $9, $10, -6
MUL.D $f20, $f11, $f4
# --- verification : error against the double closest to pi, then pass flag ---
LUI $8, 16393
ORI $8, $8, 8699
LUI $9, 21572
ORI $9, $9, 11544
MTC1 $9, $f26
MTHC1 $8, $f26
LUI $8, 16210
ORI $8, $8, 1468
LUI $9, 419
ORI $9, $9, 28207
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
