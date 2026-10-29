#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

#include "cpu.h"
#include "exception.h"

/* The names of the exceptions of the manual, in the order of the Exception enum */
static const char *const exception_names[] =
{
	"Address Error (load or fetch)",
	"Address Error (store)",
	"Breakpoint",
	"Debug Breakpoint",
	"Integer Overflow",
	"Reserved Instruction",
	"System Call",
	"Trap"
};

/* There is no handler yet (no COP0, no exception vector) : the exception stops the execution, with the instruction
   that raised it left uncommitted, and its state is kept in cpu->exception. This is the only place that has to change
   once exceptions jump to a handler.
   A delay slot instruction reports the branch in EPC (BD = 1). For a forbidden slot the manual of the instruction set
   does not say, EPC is the instruction itself. */
void raise_exception(Cpu *cpu, Exception exception, uint32_t bad_vaddr)
{
	ExceptionState *state = &cpu->exception;

	if(state->raised) return;

	state->raised = 1;
	state->code = exception;
	state->in_delay_slot = cpu->slot == SLOT_DELAY;
	state->epc = state->in_delay_slot ? cpu->PC - WORD_SIZE : cpu->PC;
	state->bad_vaddr = bad_vaddr;

	fprintf(stderr, "[!] Exception : %s (EPC = 0x%08" PRIx32 ", BD = %u", exception_names[exception], state->epc, (unsigned) state->in_delay_slot);
	if(exception == EXC_ADDRESS_ERROR_LOAD || exception == EXC_ADDRESS_ERROR_STORE) fprintf(stderr, ", BadVAddr = 0x%08" PRIx32, bad_vaddr);
	fprintf(stderr, ")\n");
}
