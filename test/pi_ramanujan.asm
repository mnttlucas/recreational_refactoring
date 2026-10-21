# Ramanujan (1910) : 1/pi = (2*sqrt(2)/9801) * sum_k (4k)! (1103 + 26390 k) / ((k!)^4 396^(4k))
# Each term adds ~8 correct digits. a_k = (4k)!/((k!)^4 396^(4k)) is built from a_(k-1) with a ratio, no factorial overflow.
# Estimate after term k is left in $f(20+k) : $f20 = 1 term ... 
# constants : f1 = 1, f2 = 4, f3 = 396^4, f4 = 1103, f5 = 26390, f6 = 9801, f7 = 2*sqrt(2)
ORI $8, $0, 1
MTC1 $8, $f1
CVT.D.W $f1, $f1
ORI $8, $0, 4
MTC1 $8, $f2
CVT.D.W $f2, $f2
ORI $8, $0, 1103
MTC1 $8, $f4
CVT.D.W $f4, $f4
ORI $8, $0, 26390
MTC1 $8, $f5
CVT.D.W $f5, $f5
ORI $8, $0, 9801
MTC1 $8, $f6
CVT.D.W $f6, $f6
ORI $8, $0, 1
MTC1 $8, $f8
CVT.D.W $f8, $f8
ORI $8, $0, 0
MTC1 $8, $f9
CVT.D.W $f9, $f9
ORI $8, $0, 0
MTC1 $8, $f10
CVT.D.W $f10, $f10
ORI $8, $0, 396
MTC1 $8, $f3
CVT.D.W $f3, $f3
MUL.D $f3, $f3, $f3
MUL.D $f3, $f3, $f3
ORI $8, $0, 2
MTC1 $8, $f7
CVT.D.W $f7, $f7
SQRT.D $f7, $f7
ADD.D $f7, $f7, $f7
# state : f8 = a_k (starts at 1), f9 = running sum, f10 = k
# --- term k = 0 ---
MOV.D $f11, $f4
MADDF.D $f11, $f5, $f10
MADDF.D $f9, $f8, $f11
MUL.D $f12, $f7, $f9
DIV.D $f20, $f6, $f12
MUL.D $f12, $f10, $f2
ADD.D $f12, $f12, $f1
ADD.D $f13, $f12, $f1
ADD.D $f14, $f13, $f1
ADD.D $f15, $f14, $f1
MUL.D $f12, $f12, $f13
MUL.D $f12, $f12, $f14
MUL.D $f12, $f12, $f15
ADD.D $f10, $f10, $f1
MUL.D $f16, $f10, $f10
MUL.D $f16, $f16, $f16
MUL.D $f16, $f16, $f3
MUL.D $f8, $f8, $f12
DIV.D $f8, $f8, $f16
# --- term k = 1 ---
MOV.D $f11, $f4
MADDF.D $f11, $f5, $f10
MADDF.D $f9, $f8, $f11
MUL.D $f12, $f7, $f9
DIV.D $f21, $f6, $f12
MUL.D $f12, $f10, $f2
ADD.D $f12, $f12, $f1
ADD.D $f13, $f12, $f1
ADD.D $f14, $f13, $f1
ADD.D $f15, $f14, $f1
MUL.D $f12, $f12, $f13
MUL.D $f12, $f12, $f14
MUL.D $f12, $f12, $f15
ADD.D $f10, $f10, $f1
MUL.D $f16, $f10, $f10
MUL.D $f16, $f16, $f16
MUL.D $f16, $f16, $f3
MUL.D $f8, $f8, $f12
DIV.D $f8, $f8, $f16
# --- term k = 2 ---
MOV.D $f11, $f4
MADDF.D $f11, $f5, $f10
MADDF.D $f9, $f8, $f11
MUL.D $f12, $f7, $f9
DIV.D $f22, $f6, $f12
MUL.D $f12, $f10, $f2
ADD.D $f12, $f12, $f1
ADD.D $f13, $f12, $f1
ADD.D $f14, $f13, $f1
ADD.D $f15, $f14, $f1
MUL.D $f12, $f12, $f13
MUL.D $f12, $f12, $f14
MUL.D $f12, $f12, $f15
ADD.D $f10, $f10, $f1
MUL.D $f16, $f10, $f10
MUL.D $f16, $f16, $f16
MUL.D $f16, $f16, $f3
MUL.D $f8, $f8, $f12
DIV.D $f8, $f8, $f16
# --- term k = 3 ---
MOV.D $f11, $f4
MADDF.D $f11, $f5, $f10
MADDF.D $f9, $f8, $f11
MUL.D $f12, $f7, $f9
DIV.D $f23, $f6, $f12
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
SUB.D $f27, $f23, $f26
ABS.D $f28, $f27
CMP.LT.D $f30, $f28, $f29
MFC1 $2, $f30
ANDI $2, $2, 1
# $f0 = result, $f27 = signed error, $2 = 1 if |error| < threshold, $3 = CLASS.D mask of the result (128 = positive normal)
MOV.D $f0, $f23
CLASS.D $f31, $f0
MFC1 $3, $f31
# $4 / $5 = low / high word of the result
MFC1 $4, $f0
MFHC1 $5, $f0
