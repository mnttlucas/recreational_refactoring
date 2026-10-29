CC = gcc

CFLAGS = -Wall -Wextra -Wpedantic \
          -Wshadow -Wconversion \
          -Wmissing-prototypes -Wstrict-prototypes \
          -std=c11 -g \
          -MMD -MP \
          -Iinclude

TARGET = emul-mips

SRC = src/main.c \
      src/modes.c \
      src/decode.c \
      src/display.c \
      src/exception.c \
      src/execute.c \
      src/fp.c \
      src/hex.c \
      src/instruction.c \
      src/mips_memory.c \
      src/mips_registers.c \
      src/utils.c \
      src/cpu.c

OBJ = $(patsubst src/%.c,obj/%.o,$(SRC))
DEP = $(OBJ:.o=.d)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) -lm

obj/%.o: src/%.c | obj
	$(CC) $(CFLAGS) -c $< -o $@

obj:
	mkdir -p obj

clean:
	rm -rf obj $(TARGET)

re: clean all

.PHONY: all clean re

-include $(DEP)