CC = g++
WFLAGS = -Wall -Wextra -Wno-subobject-linkage -Wno-narrowing
CFLAGS = $(WFLAGS) -std=gnu++23 -MMD $(if $(release),-O3,-g)
INCLUDE = -Iinclude
LDLIBS = -lfftw3f -lportaudio -lsfml-graphics -lsfml-system -lsfml-window -lglfw -lavformat -lavcodec -lavutil
OBJDIR = obj
BINDIR = bin
SRCDIR = src

# List of source files
SRCS = $(wildcard $(SRCDIR)/*.cpp)
# List of object files
OBJS = $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SRCS))
# List of dependency files
DEPS = $(OBJS:.o=.d)

# Default target
all: clear makedirs $(BINDIR)/a.out

# Linking
$(BINDIR)/a.out: $(OBJS) | $(BINDIR)
	$(CC) $^ $(LDLIBS) -o $@

# Compilation
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

# Create necessary directories
$(BINDIR) $(OBJDIR):
	mkdir -p $@

# Clean up
clean:
	rm -rf $(BINDIR) $(OBJDIR)

clear:
	clear

# Install to system
install:
	cp bin/a.out /usr/local/bin/audioviz

# Include the dependency files
-include $(DEPS)

.PHONY: all makedirs clean