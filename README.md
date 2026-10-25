# Recreational refactoring

Just a fun little project that took me on a nostalgic trip

The desire to make this code readable, the thrill of seeing this little project run again, not to mention 'how on earth did I write that this bad'?

-----

Try it with committed secret.asm or any pi_*.asm :
```console
make re; ./emul-mips test/<file.asm> output.hex output.regs [options]
```

Or in interactive mode :
```console
make re; ./emul-mips [options]
```

Better, if you want to see each instruction step by step :
```console
make re; ./emul-mips test/<file.asm> -step [options]
```

-----

The input can also be 32-bit instruction words in hexadecimal (`24090010`, or `0x24090010`) instead of assembly, with `-hex` : a file is one or the other, never both.
```console
make re; ./emul-mips input.hex output.regs -hex
```
The `.hex` file written by a previous run can therefore be executed again (the option works in interactive and in step-by-step modes too).
A word that is not an implemented instruction, or whose reserved fields are not zero, is rejected with a "Reserved Instruction" message (the `code` field of the traps is ignored, as the manual says).

-----

Where  
```console
[options]
```
are basically printf formatting for FPR/GPR :
```console
-fpr=hex|float|double|all (default=hex)
-gpr=dec|hex|both (default=dec)
```