CC = gcc
#CFLAGS = -Wall -Wextra -Wpedantic -std=c11 -g
CFLAGS = -Wall -Wextra -Wpedantic -Wshadow -Wconversion -std=c11 -g

TARGET = emul-mips

SRC = main.c \
      decode.c \
      execute.c \
      instruction.c \
      memory.c \
      registers.c \
      utils.c \
      cpu.c

OBJ = $(SRC:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

re: clean all

.PHONY: all clean re