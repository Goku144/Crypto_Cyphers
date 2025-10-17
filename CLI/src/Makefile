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

# ---------------- Linux additions (Windows part above is untouched) ----------------

UNAME_S  := $(shell uname -s)
IS_LINUX := $(findstring Linux,$(UNAME_S))

# Linux build output (no .exe)
LINUX_EXE := $(BIN_DIR)/$(PRG)

# Install prefix (PATH location on Linux)
PREFIX  ?= /usr/local
BINDIR  := $(PREFIX)/bin
DESTDIR ?=

.PHONY: linux install uninstall print-dirs

# Build Linux binary (separate from Windows .exe)
linux: $(LINUX_EXE)

$(LINUX_EXE): $(SRC) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o "$@" "$<" $(LDLIBS)

# Install Linux binary into system PATH
install:
ifeq ($(IS_LINUX),Linux)
	@$(MAKE) --no-print-directory linux
	@install -d "$(DESTDIR)$(BINDIR)"
	@install -m755 "$(LINUX_EXE)" "$(DESTDIR)$(BINDIR)/$(PRG)"
	@echo "Installed: $(DESTDIR)$(BINDIR)/$(PRG)"
else
	@echo "Install is Linux-only. Windows build remains '$(EXE)'."
endif

uninstall:
ifeq ($(IS_LINUX),Linux)
	@rm -f "$(DESTDIR)$(BINDIR)/$(PRG)"
	@echo "Removed: $(DESTDIR)$(BINDIR)/$(PRG)"
else
	@echo "Uninstall is Linux-only."
endif

print-dirs:
	@echo UNAME_S=$(UNAME_S)
	@echo IS_LINUX=$(IS_LINUX)
	@echo PREFIX=$(PREFIX)
	@echo BINDIR=$(BINDIR)
	@echo BIN_DIR=$(BIN_DIR)
	@echo WINDOWS_EXE=$(EXE)
	@echo LINUX_EXE=$(LINUX_EXE)