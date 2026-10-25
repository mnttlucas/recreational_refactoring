#ifndef	HEX_H
#define	HEX_H

#include "instruction.h"

/* 1 if the text is a 32-bit hexadecimal word : exactly 8 digits, or 1 to 8 digits after 0x */
int hex_is_word(const char *text);

/* Decodes the line (a hexadecimal word and nothing else) into `instr`, instr->error is set when the line is not
   a word or when the word is not an implemented instruction. */
void hex_decode_line(const char *text, Instruction *instr);

#endif	/* HEX_H */
