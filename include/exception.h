#ifndef	EXCEPTION_H
#define	EXCEPTION_H

#include <stdint.h>

typedef enum
{
	EXC_ADDRESS_ERROR_LOAD,		/* load, or instruction fetch (AdEL) */
	EXC_ADDRESS_ERROR_STORE,	/* store (AdES) */
	EXC_BREAKPOINT,
	EXC_DEBUG_BREAKPOINT,		/* SDBBP : a debug exception rather than a Cause.ExcCode one */
	EXC_INTEGER_OVERFLOW,
	EXC_RESERVED_INSTRUCTION,
	EXC_SYSTEM_CALL,
	EXC_TRAP
} Exception;

/* What the exception left, the way COP0 would : Cause (code, BD), EPC and BadVAddr.
   `raised` stops the execution, the instruction that raised it did not complete. */
typedef struct
{
	uint8_t raised;
	uint8_t in_delay_slot;	/* BD */
	Exception code;
	uint32_t epc;
	uint32_t bad_vaddr;		/* only meaningful for the address errors */
} ExceptionState;

#endif	/* EXCEPTION_H */
