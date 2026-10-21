# Recreational refactoring

Just a fun little project that took me on a nostalgic trip

The desire to make this code readable, the thrill of seeing this little project run again, not to mention 'how on earth did I write that this bad'?

-----

Try it with committed secret.asm :
```console
make re; ./emul-mips secret.asm output.hex output.regs [options]
```

Or in interactive mode :
```console
make re; ./emul-mips [options]
```

Better, if you want to see each of ~700 instructions step by step :
```console
make re; ./emul-mips secret.asm -step [options]
```

-----

Where  
```console
[options]
```
are basically printf formatting for FPR/GPR :
```console
-fpr=hex|single|double|all (default=hex)
-gpr=dec|hex|both (default=dec)
```