CC = g++
WFLAGS = -Wall -Wextra -Wno-subobject-linkage -Wno-narrowing
CFLAGS = $(WFLAGS) -std=gnu++23 -MMD $(if $(release),-O3,-g)
INCLUDE = -Iinclude -Ideps/libavpp/include -Ideps/portaudio-pp/include
LDLIBS = -lfftw3f -lportaudio -lsfml-graphics -lsfml-system -lsfml-window -lglfw -lavformat -lavcodec -lavutil -lswresample -lswscale

# List of source files
SRCS = $(shell find src -name "*.cpp")
# List of object files
OBJS = $(patsubst src/%.cpp,obj/%.o,$(SRCS))
# List of dependency files
#DEPS = $(OBJS:.o=.d)
# Output directories
DIRS = bin obj $(sort $(dir $(OBJS)))

all: clear bin/audioviz

# Linking
bin/audioviz: $(OBJS) | $(DIRS)
	$(CC) $^ $(LDLIBS) -o $@

# Compilation
obj/%.o: src/%.cpp | $(DIRS)
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

$(DIRS):
	mkdir $@

clean:
	rm -rf bin obj

clear:
	clear

install:
	cp bin/audioviz /usr/local/bin/audioviz



.PHONY: all clean clear install

# Include the dependency files
-include $(DEPS)
