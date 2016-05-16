CC=clang++

APP = 4xSimulation

INCDIR = include
SRCDIR = src
OBJDIR = obj
LIBDIR = lib

SRCS    := $(shell find $(SRCDIR) -name '*.cpp')
SRCDIRS := $(shell find . -name '*.cpp' -exec dirname {} \; | uniq)
OBJS    := $(patsubst %.cpp,$(OBJDIR)/%.o,$(SRCS))

CFLAGS  = -std=c++11 -c -Wall -Wextra -I$(INCDIR)
LDFLAGS = -rpath $(LIBDIR)

all: $(APP)

$(APP) : buildsim $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o $@

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