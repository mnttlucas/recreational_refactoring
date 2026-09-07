#ifndef EXECUTE_H
#define EXECUTE_H

#include "instruction.h"

void instructionADD(CPU *cpu, instruction instr, int printable);
void instructionADDI(CPU *cpu, instruction instr, int printable);
void instructionAND(CPU *cpu, instruction instr, int printable);
void instructionBEQ(CPU *cpu, instruction instr, int printable);
void instructionBGTZ(CPU *cpu, instruction instr, int printable);
void instructionBLEZ(CPU *cpu, instruction instr, int printable);
void instructionBNE(CPU *cpu, instruction instr, int printable);
void instructionDIV(CPU *cpu, instruction instr, int printable);
void instructionJ(CPU *cpu, instruction instr, int printable);
void instructionJAL(CPU *cpu, instruction instr, int printable);
void instructionJR(CPU *cpu, instruction instr, int printable);
void instructionLUI(CPU *cpu, instruction instr, int printable);
void instructionLW(CPU *cpu, instruction instr, int printable);
void instructionMFHI(CPU *cpu, instruction instr, int printable);
void instructionMFLO(CPU *cpu, instruction instr, int printable);
void instructionMULT(CPU *cpu, instruction instr, int printable);
void instructionNOP(CPU *cpu, instruction instr, int printable);
void instructionOR(CPU *cpu, instruction instr, int printable);
void instructionROTR(CPU *cpu, instruction instr, int printable);
void instructionSLL(CPU *cpu, instruction instr, int printable);
void instructionSLT(CPU *cpu, instruction instr, int printable);
void instructionSRL(CPU *cpu, instruction instr, int printable);
void instructionSUB(CPU *cpu, instruction instr, int printable);
void instructionSW(CPU *cpu, instruction instr, int printable);
void instructionXOR(CPU *cpu, instruction instr, int printable);

#endif /* EXECUTE_H */