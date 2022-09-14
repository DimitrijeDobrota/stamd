# GNU Makefile for Game of Life simulation
#
# Usage: make [-f path\Makefile] [DEBUG=Y] target

NAME = stamd
CC = gcc

CFLAGS = -Iinclude
LDFLAGS += -lmd4c-html

SRC = src
OBJ = obj
BINDIR = bin

BIN = bin/$(NAME)
SRCS=$(wildcard $(SRC)/*.c)
OBJS=$(patsubst $(SRC)/%.c, $(OBJ)/%.o, $(SRCS))

ifeq ($(DEBUG),Y)
	CFLAGS += -lciid
	CFLAGS += -Wall -ggdb
else
	CFLAGS += -lcii
endif

all: $(BIN)

$(BIN): $(OBJS)
	$(CC) $^ $(CFLAGS) $(LDFLAGS) -o $@

$(OBJ)/%.o: $(SRC)/%.c
	$(CC) -c $< -o $@ $(CFLAGS) $(LDFLAGS)

clean:
	-$(RM) $(BIN) $(OBJS)

help:
	@echo "Stamd - Static Markdown Page Generator"
	@echo
	@echo "Usage: make [-f path\Makefile] [DEBUG=Y] target"
	@echo
	@echo "Target rules:"
	@echo "    all         - Compiles binary file [Default]"
	@echo "    clean       - Clean the project by removing binaries"
	@echo "    help        - Prints a help message with target rules"
	@echo
	@echo "Optional parameters:"
	@echo "    DEBUG       - Compile binary file with debug flags enabled"
	@echo

.PHONY: all clean help docs
