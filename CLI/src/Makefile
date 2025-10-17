# --- config ---
BIN_DIR := bin
SRC     := main.c
PRG     := cypher
EXE     := $(BIN_DIR)/$(PRG).exe   # <-- Windows build (unchanged)

CC      ?= gcc
CFLAGS  ?= -O2 -Wall -Wextra
LDLIBS  ?= -lcypher

# ---------------- DO NOT TOUCH: Windows targets ----------------
.PHONY: all clean
all: $(EXE)

$(BIN_DIR):
	@mkdir -p "$(BIN_DIR)"

$(EXE): $(SRC) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o "$@" "$<" $(LDLIBS)

clean:
	@rm -f "$(EXE)"

# ---------------- Linux-only additions ----------------
UNAME_S  := $(shell uname -s)
IS_LINUX := $(findstring Linux,$(UNAME_S))

# Linux build output (no .exe)
LINUX_EXE := $(BIN_DIR)/$(PRG)

# Optional: if libcypher is not in default link paths, set LIBDIR=/usr/local/lib
LIBDIR   ?=
LDFLAGS  :=
ifneq ($(strip $(LIBDIR)),)
  LDFLAGS += -L$(LIBDIR) -Wl,-rpath,$(LIBDIR)
endif

# Install into PATH on Linux (default /usr/local/bin)
PREFIX  ?= /usr/local
BINDIR  := $(PREFIX)/bin
DESTDIR ?=

.PHONY: linux install uninstall print-dirs

# Build the Linux binary (non-.exe) without touching Windows build
linux: $(LINUX_EXE)

$(LINUX_EXE): $(SRC) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o "$@" "$<" $(LDFLAGS) $(LDLIBS)

# Install to system PATH on Linux, then clean local artifacts
install:
ifeq ($(IS_LINUX),Linux)
	@$(MAKE) --no-print-directory linux
	@install -d "$(DESTDIR)$(BINDIR)"
	@install -m755 "$(LINUX_EXE)" "$(DESTDIR)$(BINDIR)/$(PRG)"
	@echo "Installed: $(DESTDIR)$(BINDIR)/$(PRG)"
	@which ldconfig >/dev/null 2>&1 && sudo ldconfig || true
	@$(MAKE) --no-print-directory clean
else
	@echo "Install target is Linux-only. Windows build is already in '$(EXE)'."
endif

uninstall:
ifeq ($(IS_LINUX),Linux)
	@rm -f "$(DESTDIR)$(BINDIR)/$(PRG)"
	@which ldconfig >/dev/null 2>&1 && sudo ldconfig || true
	@echo "Removed: $(DESTDIR)$(BINDIR)/$(PRG)"
else
	@echo "Uninstall target is Linux-only."
endif

print-dirs:
	@echo UNAME_S=$(UNAME_S)
	@echo PREFIX=$(PREFIX)
	@echo BINDIR=$(BINDIR)
	@echo BIN_DIR=$(BIN_DIR)
	@echo WINDOWS_EXE=$(EXE)
	@echo LINUX_EXE=$(LINUX_EXE)