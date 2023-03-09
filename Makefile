BUILD_DIR=build
include $(N64_INST)/include/n64.mk

src = $(wildcard source/*.c)
assets_xm = $(wildcard assets/*.xm)
assets_wav = $(wildcard assets/*.wav)
assets_png = $(wildcard assets/*.png)

assets_conv = $(addprefix filesystem/,$(notdir $(assets_png:%.png=%.sprite)))

AUDIOCONV_FLAGS ?=
MKSPRITE_FLAGS ?=
CFLAGS += -funroll-loops

all: normaldemo.z64

filesystem/%.sprite: assets/%.png
	@mkdir -p $(dir $@)
	@echo "    [SPRITE] $@"
	@$(N64_MKSPRITE) $(MKSPRITE_FLAGS) -o filesystem "$<"

filesystem/normal.sprite: MKSPRITE_FLAGS=--format RGBA32
filesystem/normal2.sprite: MKSPRITE_FLAGS=--format RGBA32
filesystem/base.sprite: MKSPRITE_FLAGS=--format RGBA32
filesystem/normal3.sprite: MKSPRITE_FLAGS=--format RGBA32

$(BUILD_DIR)/normaldemo.dfs: $(assets_conv)
$(BUILD_DIR)/normaldemo.elf: $(src:%.c=$(BUILD_DIR)/%.o)

normaldemo.z64: N64_ROM_TITLE="Normal Demo"
normaldemo.z64: $(BUILD_DIR)/normaldemo.dfs 

clean:
	rm -rf $(BUILD_DIR) normaldemo.z64

-include $(wildcard $(BUILD_DIR)/*.d)

.PHONY: all clean
