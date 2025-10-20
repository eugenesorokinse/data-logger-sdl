TARGET      ?= abox-monitor-mu
BUILD_DIR   ?= build

C_FILES     ?= main.c \
	mainwindow.c	\
	serial.c		\
	data.c			\
	channel.c		\
	

CC          ?= gcc

BUILD_TYPE  ?= release

WARN_FLAGS  := -Wall -Wextra -Wshadow -Wpointer-arith -Wcast-qual -Wstrict-prototypes -Wmissing-prototypes
DEP_FLAGS   := -MMD -MP

ifeq ($(BUILD_TYPE),debug)
  OPT_FLAGS := -O0 -g3 -DDEBUG
else
  OPT_FLAGS := -O2 -g0 -DNDEBUG
endif

SDL2_CFLAGS := $(shell pkg-config --cflags sdl2 2>/dev/null || sdl2-config --cflags 2>/dev/null)
SDL2_LIBS   := $(shell pkg-config --libs   sdl2 2>/dev/null || sdl2-config --libs   2>/dev/null)

TTF_CFLAGS  := $(shell pkg-config --cflags SDL2_ttf 2>/dev/null)
TTF_LIBS    := $(shell pkg-config --libs   SDL2_ttf 2>/dev/null)

CPPFLAGS    ?=
CFLAGS      ?= $(OPT_FLAGS) $(WARN_FLAGS) $(DEP_FLAGS) $(SDL2_CFLAGS) $(TTF_CFLAGS)
LDFLAGS     ?=
LDLIBS      ?= $(SDL2_LIBS) $(TTF_LIBS) -lm -lpthread

OBJS        := $(patsubst %.c,$(BUILD_DIR)/%.o,$(C_FILES))
DEPS        := $(OBJS:.o=.d)
BIN         := $(BUILD_DIR)/$(TARGET)

# ==== Default target ====
.PHONY: all

all: $(BIN)

# Link
$(BIN): $(OBJS) | $(BUILD_DIR)
	@echo "  LINK  $@"
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

# Compile C -> obj (into build/)
$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	@echo "  CC    $<"
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

# Ensure build directory exists
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

# Convenience targets
.PHONY: clean distclean run list
clean:
	@echo "  CLEAN objects"
	@rm -f $(OBJS) $(DEPS)

distclean: clean
	@echo "  CLEAN build dir"
	@rm -rf $(BUILD_DIR)

# Run the built binary
run: $(BIN)
	@$(BIN)

# Show which C files are currently configured
list:
	@echo "C_FILES = $(C_FILES)"

# Include auto-generated dependency files
-include $(DEPS)

