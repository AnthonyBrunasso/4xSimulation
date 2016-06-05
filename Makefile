CC=clang++

# Detect operating system for proper dynamic lib extension
ifeq ($(OS),Windows_NT)
	DLLEXT := .dll
  EXEEXT := .exe
  # Compile with gcc on windows
  CC=g++
else
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S),Linux)
		DLLEXT := .so
	endif
	ifeq ($(UNAME_S),Darwin)
		DLLEXT := .dylib
	endif
endif

APP = 4xsim$(EXEEXT)
LIB = lib4xsim$(DLLEXT)

INCDIR = include
SRCDIR = src
OBJDIR = obj
LIBDIR = lib

SRCS    := $(wildcard src/*.cpp)
SRCDIRS := $(filter %/, $(wildcard src/*/))
OBJS    := $(patsubst %.cpp,$(OBJDIR)/%.o,$(SRCS))

CFLAGS  = -std=c++11 -c -Wall -Wextra -I$(INCDIR)

all: $(APP)

debug: CFLAGS += -DDEBUG -g
debug: $(APP)

# Remove main when creating a dynamic lib
lib: SRCS := $(filter-out $(SRCDIR)/main.cpp, $(SRCS))
lib: OBJS := $(filter-out $(OBJDIR)/$(SRCDIR)/main.o, $(OBJS))
lib: $(LIB)

# Make dynamic library
$(LIB) : buildsim $(OBJS)
	$(CC) $(OBJS) -dynamiclib -o $@

# Make simulation executable
$(APP) : buildsim $(OBJS)
	$(CC) $(OBJS) -o $@

$(OBJDIR)/%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJS)

buildsim:
	@$(call make-sim)

define make-sim
   for dir in $(SRCDIRS); \
   do \
	mkdir -p $(OBJDIR)/$$dir; \
   done
endef
