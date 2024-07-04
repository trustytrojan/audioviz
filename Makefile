CC = g++
WFLAGS = -Wall -Wextra -Wno-subobject-linkage -Wno-narrowing
CFLAGS = $(WFLAGS) -std=gnu++23 -MMD $(if $(release),-O3,-g)
INCLUDE = -Iinclude -Ideps/SFML/include -Ideps/libavpp/include -Ideps/portaudio-pp/include
LDLIBS = -Ldeps/SFML/build/lib -lfftw3f -lportaudio -lsfml-graphics -lsfml-system -lsfml-window -lglfw -lavformat -lavcodec -lavutil -lswresample -lswscale

# List of source files
SRCS = $(shell find src -name *.cpp)
# List of object files
OBJS = $(patsubst src/%.cpp,obj/%.o,$(SRCS))
# List of dependency files
DEPS = $(OBJS:.o=.d)
# Output directories
DIRS = bin obj $(sort $(dir $(OBJS)))
# Git submodules
SUBM = deps/libavpp deps/portaudiopp deps/SFML

all: clear bin/audioviz

# Linking
bin/audioviz: $(OBJS) | $(DIRS) $(SUBM) deps/SFML/build/lib
	$(CC) $^ $(LDLIBS) -o $@

# Compilation
obj/%.o: src/%.cpp | $(DIRS) $(SUBM) deps/SFML/include
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

$(DIRS):
	mkdir $@

$(SUBM):
	git submodule update --init

deps/SFML/build/lib deps/SFML/include: deps/SFML
	cd deps/SFML && \
	git checkout 1a40f0195788185d56eca04687a8cd793c90b2fc && \
	cmake -DBUILD_SHARED_LIBS=1 -S. -Bbuild && \
	cd build && \
	make

clean:
	rm -rf bin obj

clear:
	clear

install:
	cp bin/audioviz /usr/local/bin/audioviz

.PHONY: all clean clear install

# Include the dependency files
-include $(DEPS)
