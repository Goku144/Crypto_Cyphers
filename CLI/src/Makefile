# --- config ---
BIN_DIR := bin
SRC     := main.c
PRG     := cypher
EXE     := $(BIN_DIR)/$(PRG).exe

CC      ?= gcc
CFLAGS  ?= -O2 -Wall -Wextra
LDLIBS  ?= -lcypher

.PHONY: all clean
all: $(EXE)

$(BIN_DIR):
	@mkdir -p "$(BIN_DIR)"

$(EXE): $(SRC) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o "$@" "$<" $(LDLIBS)

clean:
	@rm -f "$(EXE)"