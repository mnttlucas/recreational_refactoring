#CFLAGS = -Wall -Wextra -Wpedantic -std=c11 -g

CC = gcc

CFLAGS = -Wall -Wextra -Wpedantic \
          -Wshadow -Wconversion \
          -std=c11 -g \
          -Iinclude

TARGET = emul-mips

SRC = src/main.c \
      src/decode.c \
      src/execute.c \
      src/instruction.c \
      src/memory.c \
      src/registers.c \
      src/utils.c \
      src/cpu.c

OBJ = $(patsubst src/%.c,obj/%.o,$(SRC))

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET)

obj/%.o: src/%.c | obj
	$(CC) $(CFLAGS) -c $< -o $@

obj:
	mkdir -p obj

clean:
	rm -rf obj $(TARGET)

re: clean all

.PHONY: all clean re