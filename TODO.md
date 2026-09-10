Integer Overflow handling
PC is an index, not memory addresses, so offsets are used raw instead of << 2 in branches
decode_old.c to delete once refactoring is done :)