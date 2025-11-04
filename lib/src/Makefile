# ===== OS detection (fixed) =====
UNAME_S := $(shell uname -s)
IS_WIN  := $(findstring MINGW,$(UNAME_S))$(findstring MSYS,$(UNAME_S))

# ======================= WINDOWS / MSYS2 (UNCHANGED) =======================
ifneq ($(IS_WIN),)

# ---- your original Windows MSYS2/MinGW Makefile (unchanged) ----
SRC   ?= cypher.c
SNAME := $(basename $(notdir $(SRC)))
HDR   ?= $(SNAME).h

ifeq ($(strip $(wildcard $(SRC))),)
  $(error $(SRC) not found next to this Makefile)
endif
ifeq ($(strip $(wildcard $(HDR))),)
  $(error $(HDR) not found next to this Makefile)
endif

LIBBASE := lib$(SNAME)
DLL     := $(LIBBASE).dll
IMPLIB  := $(LIBBASE).dll.a
OBJ     := $(SNAME).o
LDLIBS  ?= -lbcrypt -lgmp

CC      ?= gcc
CFLAGS  ?= -O2 -Wall -Wextra -DWIN32_LEAN_AND_MEAN -DBUILDING_$(SNAME)_DLL
LDFLAGS ?= -shared -Wl,--out-implib,$(IMPLIB) -Wl,--export-all-symbols

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

RM        := rm -f
INSTALL   := install
MKDIR_P   := install -d

.PHONY: all clean install uninstall print-dirs
all: $(DLL)

$(OBJ): $(SRC) $(HDR)
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
	@echo MSYSTEM=$(MSYSTEM)
	@echo MINGW_PREFIX=$(MINGW_PREFIX)
	@echo PREFIX=$(PREFIX)
	@echo INCDIR=$(INCDIR)
	@echo LIBDIR=$(LIBDIR)
	@echo BINDIR=$(BINDIR)

# ======================= LINUX / POSIX (NEW) =======================
else

SRC   ?= cypher.c
SNAME := $(basename $(notdir $(SRC)))
HDR   ?= $(SNAME).h

ifeq ($(strip $(wildcard $(SRC))),)
  $(error $(SRC) not found next to this Makefile)
endif
ifeq ($(strip $(wildcard $(HDR))),)
  $(error $(HDR) not found next to this Makefile)
endif

LIBBASE := lib$(SNAME)
SO      := $(LIBBASE).so
OBJ     := $(SNAME).o

CC      ?= gcc
CFLAGS  ?= -O2 -Wall -Wextra -fPIC
LDFLAGS ?= -shared -Wl,-soname,$(SO)
LDLIBS  ?=

PREFIX  ?= /usr
LIBDIR  ?= $(PREFIX)/lib
INCDIR  ?= $(PREFIX)/include

RM        := rm -f
INSTALL   := install
MKDIR_P   := install -d

.PHONY: all clean install uninstall print-dirs
all: $(SO)

$(OBJ): $(SRC) $(HDR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(SO): $(OBJ)
	$(CC) $(OBJ) $(LDFLAGS) -o $@ $(LDLIBS)

clean:
	-$(RM) $(OBJ) $(SO)

install: all
	$(MKDIR_P) "$(INCDIR)" "$(LIBDIR)"
	$(INSTALL) -m644 "$(HDR)" "$(INCDIR)/$(notdir $(HDR))"
	$(INSTALL) -m755 "$(SO)"  "$(LIBDIR)/$(notdir $(SO))"
	@command -v ldconfig >/dev/null 2>&1 && ldconfig || true
	@echo "Installed:"
	@echo "  Header -> $(INCDIR)/$(notdir $(HDR))"
	@echo "  So     -> $(LIBDIR)/$(notdir $(SO))"
	@echo "Now gcc can use:  #include <$(notdir $(HDR))>  and  -l$(SNAME)"

uninstall:
	-$(RM) "$(INCDIR)/$(notdir $(HDR))" \
	       "$(LIBDIR)/$(notdir $(SO))"
	@command -v ldconfig >/dev/null 2>&1 && ldconfig || true
	@echo "Uninstalled files for $(SNAME) from $(PREFIX)."

print-dirs:
	@echo UNAME_S=$(UNAME_S)
	@echo PREFIX=$(PREFIX)
	@echo INCDIR=$(INCDIR)
	@echo LIBDIR=$(LIBDIR)

endif