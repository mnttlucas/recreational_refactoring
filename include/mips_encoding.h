#ifndef	MIPS_ENCODING_H
#define	MIPS_ENCODING_H

/* MIPS bit fields : position of the least significant bit of each field in the 32-bit encoding */
#define FUNCT_SHIFT      0
#define SHAMT_SHIFT      6
#define RD_SHIFT        11
#define RT_SHIFT        16
#define RS_SHIFT        21
#define OPCODE_SHIFT    26

/* COP1 (floating point unit) : the `fmt` field sits in the rs position */
#define COP1_FMT_D     17
#define COP1_FMT_S     16
#define COP1_MF         0
#define COP1_MFH        3
#define COP1_MT         4
#define COP1_MTH        7

/* Function codes */
#define FUNCT_ADD    32
#define FUNCT_ADDU   33
#define FUNCT_AND    36
#define FUNCT_BSHFL  32
#define FUNCT_CLO    17
#define FUNCT_CLZ    16
#define FUNCT_FABS    5
#define FUNCT_FADD    0
#define FUNCT_FDIV    3
#define FUNCT_FMOV    6
#define FUNCT_FMUL    2
#define FUNCT_FNEG    7
#define FUNCT_FSUB    1
#define FUNCT_JALR    9
#define FUNCT_SOP30  24
#define FUNCT_SOP31  25
#define FUNCT_SOP32  26
#define FUNCT_SOP33  27
#define FUNCT_NOR    39
#define FUNCT_OR     37
#define FUNCT_SELEQZ 53
#define FUNCT_SELNEZ 55
#define FUNCT_SLL     0
#define FUNCT_SLLV    4
#define FUNCT_SLT    42
#define FUNCT_SLTU   43
#define FUNCT_SRA     3
#define FUNCT_SRAV    7
#define FUNCT_SRL     2
#define FUNCT_SRLV    6
#define FUNCT_SUB    34
#define FUNCT_SUBU   35
#define FUNCT_XOR    38

/* Operation codes */
#define OPCODE_ADDIU      9
#define OPCODE_ANDI      12
#define OPCODE_AUI       15
#define OPCODE_BALC      58
#define OPCODE_BC        50
#define OPCODE_BEQ        4
#define OPCODE_COP1      17
#define OPCODE_BGTZ       7
#define OPCODE_BLEZ       6
#define OPCODE_BNE        5
#define OPCODE_J          2
#define OPCODE_JAL        3
#define OPCODE_LDC1      53
#define OPCODE_LB        32
#define OPCODE_LBU       36
#define OPCODE_LH        33
#define OPCODE_LHU       37
#define OPCODE_LWC1      49
#define OPCODE_LW        35
#define OPCODE_ORI       13
#define OPCODE_PCREL     59
#define OPCODE_POP06      6
#define OPCODE_POP07      7
#define OPCODE_POP10      8
#define OPCODE_POP26     22
#define OPCODE_POP27     23
#define OPCODE_POP30     24
#define OPCODE_POP66     54
#define OPCODE_POP76     62
#define OPCODE_REGIMM     1
#define OPCODE_SB        40
#define OPCODE_SDC1      61
#define OPCODE_SH        41
#define OPCODE_SLTI      10
#define OPCODE_SLTIU     11
#define OPCODE_SPECIAL    0
#define OPCODE_SPECIAL3  31
#define OPCODE_SWC1      57
#define OPCODE_SW        43
#define OPCODE_XORI      14

/* Offset and immediate sizes */
#define IMM_16_SIZE      16
#define OFFSET_16_SIZE   16
#define IMM_19_SIZE      19
#define OFFSET_19_SIZE   19
#define OFFSET_21_SIZE   21
#define OFFSET_26_SIZE   26

/* Special register values for some operations
This exists for clarity and to match MIPS32 R6 specification
When values are zero, it will be implicit */
#define ALUIPC_RT     31
#define AUIPC_RT      30
#define BAL_RT        17
#define BC1EQZ_FS      9
#define BC1NEZ_FS     13
#define BGEZ_RT        1
#define CLO_CLZ_SA     1
#define DIV_DIVU_SA    2
#define LWPC_RT        8
#define MOD_MODU_SA    3
#define MUH_MUHU_SA    3
#define MUL_MULU_SA    2
#define NAL_RT        16
#define ROTR_RS        1
#define ROTRV_SA       1
#define SEB_SA        16
#define SEH_SA        24

#endif	/* MIPS_ENCODING_H */
