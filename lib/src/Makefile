# ===== Cross Makefile: Windows (MSYS2/MinGW) & Linux =====
# Build:      make
# Install:    make install
# Uninstall:  make uninstall
# Debug dirs: make print-dirs

# ---- Source & names (safe defaults) ----
SRC   ?= cypher.c
SNAME := $(basename $(notdir $(SRC)))
HDR   ?= $(SNAME).h

# Sanity checks (helpful error messages)
ifeq ($(strip $(wildcard $(SRC))),)
  $(error $(SRC) not found next to this Makefile)
endif
ifeq ($(strip $(wildcard $(HDR))),)
  $(error $(HDR) not found next to this Makefile)
endif

LIBBASE := lib$(SNAME)

# ---- Host detection ----
UNAME_S := $(shell uname -s)
UNAME_M := $(shell uname -m)
IS_WIN  := $(findstring MINGW,$(UNAME_S))$(findstring MSYS,$(UNAME_S))

# ---- Toolchain, flags, artifacts ----
CC      ?= gcc
AR      ?= ar
RM        := rm -f
INSTALL   := install
MKDIR_P   := install -d

BASE_CFLAGS := -O2 -Wall -Wextra

# ===================== Windows (MSYS2/MinGW*) =====================
ifeq ($(IS_WIN),MINGWMSYS)
  DLL     := $(LIBBASE).dll
  IMPLIB  := $(LIBBASE).dll.a
  OBJ     := $(SNAME).o

  PREFIX ?= $(MINGW_PREFIX)
  ifeq ($(PREFIX),)
    ifeq ($(MSYSTEM),MINGW64)
      PREFIX := /mingw64
    else ifeq ($(MSYSTEM),UCRT64)
      PREFIX := /ucrt64
    else ifeq ($(MSYSTEM),CLANG64)
      PREFIX := /clang64
    else ifeq ($(MSYSTEM),MINGW32)
      PREFIX := /mingw32
    else
      ifneq ($(wildcard C:/msys64/mingw64),)
        PREFIX := C:/msys64/mingw64
      else
        PREFIX := /mingw64
      endif
    endif
  endif

  INCDIR := $(PREFIX)/include
  LIBDIR := $(PREFIX)/lib
  BINDIR := $(PREFIX)/bin

  CFLAGS  ?= $(BASE_CFLAGS) -DWIN32_LEAN_AND_MEAN -DBUILDING_$(SNAME)_DLL
  LDFLAGS ?= -shared -Wl,--out-implib,$(IMPLIB) -Wl,--export-all-symbols
  LDLIBS  ?= -lbcrypt

  .PHONY: all clean install uninstall print-dirs
  all: $(DLL)

  $(SNAME).o: $(SRC) $(HDR)
	$(CC) $(CFLAGS) -c -o $@ $<

  $(DLL): $(OBJ)
	$(CC) $(OBJ) $(LDFLAGS) -o $@ $(LDLIBS)

  clean:
	-$(RM) $(OBJ) $(DLL) $(IMPLIB)

  install: all
	$(MKDIR_P) "$(INCDIR)" "$(LIBDIR)" "$(BINDIR)"
	$(INSTALL) -m644 "$(HDR)"    "$(INCDIR)/$(notdir $(HDR))"
	$(INSTALL) -m644 "$(IMPLIB)" "$(LIBDIR)/$(notdir $(IMPLIB))"
	$(INSTALL) -m755 "$(DLL)"    "$(BINDIR)/$(notdir $(DLL))"
	@echo "Installed:"
	@echo "  Header     -> $(INCDIR)/$(notdir $(HDR))"
	@echo "  Import lib -> $(LIBDIR)/$(notdir $(IMPLIB))"
	@echo "  DLL        -> $(BINDIR)/$(notdir $(DLL))"

  uninstall:
	-$(RM) "$(INCDIR)/$(notdir $(HDR))" \
	       "$(LIBDIR)/$(notdir $(IMPLIB))" \
	       "$(BINDIR)/$(notdir $(DLL))"
	@echo "Uninstalled files for $(SNAME) from $(PREFIX)."

  print-dirs:
	@echo UNAME_S=$(UNAME_S)
	@echo UNAME_M=$(UNAME_M)
	@echo MSYSTEM=$(MSYSTEM)
	@echo MINGW_PREFIX=$(MINGW_PREFIX)
	@echo PREFIX=$(PREFIX)
	@echo INCDIR=$(INCDIR)
	@echo LIBDIR=$(LIBDIR)
	@echo BINDIR=$(BINDIR)

# ===================== Linux / POSIX =====================
else

  # Install into GCC default search paths by default
  PREFIX ?= /usr

  # Choose /usr/lib or /usr/lib64 (keep /usr/lib for most distros)
  LIBSUFFIX :=
  ifeq ($(UNAME_M),x86_64)
    # Set LIBSUFFIX=64 if your distro uses /usr/lib64
    LIBSUFFIX ?=
  endif

  INCDIR := $(PREFIX)/include
  LIBDIR := $(PREFIX)/lib$(LIBSUFFIX)

  SO   := $(LIBBASE).so
  OBJ  := $(SNAME).o

  CFLAGS  ?= $(BASE_CFLAGS) -fPIC
  LDFLAGS ?= -shared -Wl,-soname,$(SO)
  LDLIBS  ?=

  .PHONY: all clean install uninstall ldconfig print-dirs
  all: $(SO)

  $(SNAME).o: $(SRC) $(HDR)
	$(CC) $(CFLAGS) -c -o $@ $<

  $(SO): $(OBJ)
	$(CC) $(OBJ) $(LDFLAGS) -o $@ $(LDLIBS)

  clean:
	-$(RM) $(OBJ) $(SO)

  # ---- INSTALL: /usr/include + /usr/lib, then ldconfig, then clean ----
  install: all
	@if [ ! -w "$(INCDIR)" ] || [ ! -w "$(LIBDIR)" ]; then \
	  echo ">> Installing to $(INCDIR) and $(LIBDIR) requires root. Use: sudo make install"; \
	fi
	$(MKDIR_P) "$(INCDIR)" "$(LIBDIR)"
	$(INSTALL) -m644 "$(HDR)" "$(INCDIR)/$(notdir $(HDR))"
	$(INSTALL) -m755 "$(SO)"  "$(LIBDIR)/$(notdir $(SO))"
	@if command -v ldconfig >/dev/null 2>&1; then ldconfig; fi
	@echo "Installed:"
	@echo "  Header -> $(INCDIR)/$(notdir $(HDR))"
	@echo "  So     -> $(LIBDIR)/$(notdir $(SO))"
	@echo "Post-install: cleaning build artifacts in source tree…"
	@$(MAKE) --no-print-directory clean

  uninstall:
	-$(RM) "$(INCDIR)/$(notdir $(HDR))" \
	       "$(LIBDIR)/$(notdir $(SO))"
	@if command -v ldconfig >/dev/null 2>&1; then ldconfig; fi
	@echo "Uninstalled files for $(SNAME) from $(PREFIX)."

  ldconfig:
	@command -v ldconfig >/dev/null 2>&1 && ldconfig || true

  print-dirs:
	@echo UNAME_S=$(UNAME_S)
	@echo UNAME_M=$(UNAME_M)
	@echo PREFIX=$(PREFIX)
	@echo INCDIR=$(INCDIR)
	@echo LIBDIR=$(LIBDIR)
endif
