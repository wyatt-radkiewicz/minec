# Sources/build dirs
TARGET_NAME := minec
BUILD_DIR := ./build
INCLUDE_DIRS := ./src ./deps_include
SRCS := $(wildcard src/*.c)
OBJS := $(addprefix $(BUILD_DIR)/,$(notdir $(SRCS:%.c=%.o)))

# Flags
CFLAGS := $(CFLAGS) $(addprefix -I,$(INCLUDE_DIRS)) $(shell sdl2-config --cflags)
SDL_LDFLAGS := $(shell sdl2-config --libs)
LDFLAGS := $(LDFLAGS) $(SDL_LDFLAGS) -lSDL2_mixer -lSDL2_net
DEBUG_CFLAGS := -DDEBUG -g -O0
RELEASE_CFLAGS := -O2
ifeq ($(OS),Windows_NT)
	CFLAGS += -DWIN32
else
	UNAME := $(shell uname -s)
	ifeq ($(UNAME),Darwin)
		CFLAGS += -DOSX -DGL_SILENCE_DEPRECATION
		LDFLAGS += -framework OpenGL
	endif
	ifeq ($(UNAME),Linux)
		CFLAGS += -DLINUX
	endif
endif

# Build modes
debug: CFLAGS += $(DEBUG_CFLAGS)
debug: $(BUILD_DIR)/$(TARGET_NAME)
release: CFLAGS += $(RELEASE_CFLAGS)
release: $(BUILD_DIR)/$(TARGET_NAME)

# Build commands
run: debug
	$(BUILD_DIR)/$(TARGET_NAME)
run_release: release
	$(BUILD_DIR)/$(TARGET_NAME)
$(BUILD_DIR)/$(TARGET_NAME): $(OBJS)
	mkdir -p $(dir $@)
	$(CC) $^ $(LDFLAGS) -o $@

$(BUILD_DIR)/%.o: src/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $^ -o $@

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)
