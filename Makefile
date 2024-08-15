## Boolean flags:
# RELEASE: compile with -O3 instead of -g
# NO_PORTAUDIO: compile without PortAudio

CC = g++
WFLAGS = -Wall -Wextra -Wno-subobject-linkage -Wno-narrowing
DEFINES = $(if $(NO_PORTAUDIO),,-DPORTAUDIO)
CFLAGS = $(WFLAGS) $(DEFINES) -std=gnu++23 -MMD $(if $(RELEASE),-O3,-g)
INCLUDE = -Iinclude -Ideps -Ideps/SFML/include -Ideps/libavpp/include $(if $(NO_PORTAUDIO),,-Ideps/portaudio-pp/include)
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
SUBM = deps/libavpp deps/SFML $(if $(NO_PORTAUDIO),,deps/portaudiopp)

## Phony targets

all: clear bin/audioviz

# Formats the codebase
format:
	find src include -name '*.cpp' -o -name '*.hpp' | xargs clang-format -i

# Creates clangd compilation database (compile_commands.json)
compdb:
	# -n causes a dry-run: shell commands are printed but not run \
	make -n clean all | \
	\
	# only match compile commands \
	grep -wPo '^((g\+\+).+)' | \
	grep -w '\-c' | \
	\
	# convert the commands into 'compile_commands.json' objects \
	jq -nR --tab "[inputs | { directory:\"$PWD\", command:., file: match(\" -c (?<file>[^ ]+)\").string[4:] }]" >compile_commands.json

clean:
	find obj bin -type f -delete

clear:
	clear

install:
	cd deps/SFML/build && make install
	cp bin/audioviz /usr/local/bin/audioviz

.PHONY: all format compdb clean clear install

## File/directory targets

# Linking
bin/audioviz: $(OBJS) | $(DIRS) $(SUBM) deps/SFML/build/lib
	$(CC) $^ $(LDPATHS) $(LDLIBS) -o $@

# Compilation
obj/%.o: src/%.cpp | $(DIRS) $(SUBM)
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

# Source directories
$(DIRS):
	mkdir $@

# Git submodules
$(SUBM):
	git submodule update --init

# SFML
deps/SFML/build/lib: deps/SFML
	cd deps/SFML && \
	git checkout 1a40f0195788185d56eca04687a8cd793c90b2fc && \
	cmake -DBUILD_SHARED_LIBS=1 -S. -Bbuild && \
	cd build && \
	make sfml-graphics sfml-system sfml-window

# Include the dependency files (-MMD)
-include $(DEPS)
