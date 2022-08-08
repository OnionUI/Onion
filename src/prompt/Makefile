INCLUDE_SHMVAR=1
INCLUDE_CJSON=1
include ../common/config.mk

TARGET = prompt
LDFLAGS := $(LDFLAGS) -lSDL -lSDL_ttf -lSDL_image -lSDL_rotozoom -pthread

include ../common/commands.mk
include ../common/recipes.mk

clean-all:
	@make clean
	@rm -f ./bin/batmon

dev-all: dev
	@mkdir -p $(BUILD_DIR)/bin
	@cd ../batmon && make clean && BUILD_DIR=$(BUILD_DIR)/bin make dev
