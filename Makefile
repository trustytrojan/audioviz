# Boolean flags (set to any string):
# RELEASE: compile with -O3 instead of -g
# NO_PORTAUDIO: compile without PortAudio

CC = g++
WFLAGS = -Wall -Wextra -Wno-subobject-linkage -Wno-narrowing
DEFINES = $(if $(NO_PORTAUDIO),,-DPORTAUDIO)
CFLAGS = $(WFLAGS) $(DEFINES) -std=gnu++23 -MMD $(if $(RELEASE),-O3,-g)
INCLUDE = -Iinclude -Ideps/SFML/include -Ideps/libavpp/include $(if $(NO_PORTAUDIO),,-Ideps/portaudio-pp/include)
LDPATHS = -Ldeps/SFML/build/lib
LDLIBS = -llua -lfftw3f -lsfml-graphics -lsfml-system -lsfml-window -lavformat -lavcodec -lavutil -lswresample -lswscale \
	$(if $(NO_PORTAUDIO),,-lportaudio)

# List of source files
SRCS = $(shell find src -name *.cpp)
# List of object files
OBJS = $(patsubst src/%.cpp,obj/%.o,$(SRCS))
# List of dependency files
DEPS = $(OBJS:.o=.d)
# Output directories
DIRS = bin obj $(sort $(dir $(OBJS)))
# Git submodules
SUBM = deps/libavpp $(if $(NO_PORTAUDIO),,deps/portaudiopp) deps/SFML

all: clear bin/audioviz

clean:
	find obj bin -type f -delete

clear:
	clear

install:
	cp bin/audioviz /usr/local/bin/audioviz

.PHONY: all clean clear install

# Linking
bin/audioviz: $(OBJS) | $(DIRS) $(SUBM) deps/SFML/build/lib
	$(CC) $^ $(LDPATHS) $(LDLIBS) -o $@

# Compilation
obj/%.o: src/%.cpp | $(DIRS) $(SUBM)
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

$(DIRS):
	mkdir $@

$(SUBM):
	git submodule update --init

deps/SFML/build/lib: deps/SFML
	cd deps/SFML && \
	git checkout 1a40f0195788185d56eca04687a8cd793c90b2fc && \
	cmake -DBUILD_SHARED_LIBS=1 -S. -Bbuild && \
	cd build && \
	make sfml-graphics sfml-system sfml-window

# Include the dependency files
-include $(DEPS)
