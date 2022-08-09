###########################################################

TARGET=Onion
VERSION=3.13.0
RA_SUBVERSION=0.1

###########################################################

RELEASE_NAME := $(TARGET)-v$(VERSION)

# Directories
ROOT_DIR            := $(shell pwd -P)
SRC_DIR             := $(ROOT_DIR)/src
THIRD_PARTY_DIR     := $(ROOT_DIR)/third-party
BUILD_DIR           := $(ROOT_DIR)/build
BIN_DIR             := $(ROOT_DIR)/build/.tmp_update/bin
DIST_FULL           := $(ROOT_DIR)/dist/full
DIST_CORE           := $(ROOT_DIR)/dist/core
INSTALL_BIN_DIR     := $(DIST_FULL)/miyoo/app/.tmp_update/bin
RELEASE_DIR         := $(ROOT_DIR)/release
STATIC_BUILD        := $(ROOT_DIR)/static/build
STATIC_DIST         := $(ROOT_DIR)/static/dist
STATIC_CONFIGS      := $(ROOT_DIR)/static/configs
CACHE               := $(ROOT_DIR)/cache
STATIC_PACKAGES     := $(ROOT_DIR)/static/packages
PACKAGES_EMU_DEST   := $(BUILD_DIR)/App/The_Onion_Installer/data/Layer1
PACKAGES_APP_DEST   := $(BUILD_DIR)/App/The_Onion_Installer/data/Layer2
PACKAGES_RAPP_DEST  := $(BUILD_DIR)/App/The_Onion_Installer/data/Layer3

TOOLCHAIN := ghcr.io/onionui/miyoomini-toolchain:latest

include ./src/common/commands.mk

###########################################################

.PHONY: all version core apps external release clean git-clean with-toolchain patch lib

all: dist

version: # used by workflow
	@echo $(VERSION)
print-version:
	@echo Onion v$(VERSION)
	@echo RetroArch sub-v$(RA_SUBVERSION)

$(CACHE)/.setup:
	@$(ECHO) $(PRINT_RECIPE)
	@mkdir -p $(BUILD_DIR) $(DIST_FULL) $(DIST_CORE) $(RELEASE_DIR)
	@rsync -a --exclude='.gitkeep' $(STATIC_BUILD)/ $(BUILD_DIR)
	@rsync -a --exclude='.gitkeep' $(STATIC_DIST)/ $(DIST_FULL)
	@cp -R $(ROOT_DIR)/lib/. $(DIST_FULL)/miyoo/app/.tmp_update/lib
	@mkdir -p $(BUILD_DIR)/.tmp_update/onionVersion
	@echo -n "$(VERSION)" > $(BUILD_DIR)/.tmp_update/onionVersion/version.txt
	@chmod a+x $(ROOT_DIR)/.github/get_themes.sh && $(ROOT_DIR)/.github/get_themes.sh
	@touch $(CACHE)/.
# Copy static packages
	@mkdir -p $(PACKAGES_APP_DEST) $(PACKAGES_EMU_DEST) $(PACKAGES_RAPP_DEST)
	@rsync -a --exclude='.gitkeep' $(STATIC_PACKAGES)/App/ $(PACKAGES_APP_DEST)
	@rsync -a --exclude='.gitkeep' $(STATIC_PACKAGES)/Emu/ $(PACKAGES_EMU_DEST)
	@rsync -a --exclude='.gitkeep' $(STATIC_PACKAGES)/RApp/ $(PACKAGES_RAPP_DEST)

build: core apps external

core: $(CACHE)/.setup
	@$(ECHO) $(PRINT_RECIPE)
# Build Onion binaries
	@cd $(SRC_DIR)/bootScreen && BUILD_DIR=$(BIN_DIR) make
	@cd $(SRC_DIR)/chargingState && BUILD_DIR=$(BIN_DIR) make
	@cd $(SRC_DIR)/gameSwitcher && BUILD_DIR=$(BIN_DIR) make
	@cd $(SRC_DIR)/lastGame && BUILD_DIR=$(BIN_DIR) make
	@cd $(SRC_DIR)/mainUiBatPerc && BUILD_DIR=$(BIN_DIR) make
	@cd $(SRC_DIR)/keymon && BUILD_DIR=$(BIN_DIR) make
	@cd $(SRC_DIR)/playActivity && BUILD_DIR=$(BIN_DIR) make
# Build installer binaries
	@mkdir -p $(DIST_FULL)/miyoo/app/.tmp_update/bin
	@cd $(SRC_DIR)/installUI && BUILD_DIR=$(INSTALL_BIN_DIR) make
	@cd $(SRC_DIR)/infoPanel && BUILD_DIR=$(INSTALL_BIN_DIR) make
	@cd $(SRC_DIR)/prompt && BUILD_DIR=$(INSTALL_BIN_DIR) make
	@cd $(SRC_DIR)/batmon && BUILD_DIR=$(INSTALL_BIN_DIR) make

apps: $(CACHE)/.setup
	@$(ECHO) $(PRINT_RECIPE)
	@cd $(SRC_DIR)/playActivityUI && BUILD_DIR=$(BUILD_DIR)/App/PlayActivity make
	@cd $(SRC_DIR)/packageManager && BUILD_DIR=$(BUILD_DIR)/App/The_Onion_Installer make
	@cd $(SRC_DIR)/themeSwitcher && BUILD_DIR="$(PACKAGES_APP_DEST)/Theme Switcher/App/ThemeSwitcher" make

external: $(CACHE)/.setup
	@$(ECHO) $(PRINT_RECIPE)
	@cd $(THIRD_PARTY_DIR)/RetroArch && make && cp retroarch $(BUILD_DIR)/RetroArch/
	@echo $(RA_SUBVERSION) > $(BUILD_DIR)/RetroArch/onion_ra_version.txt
	@cd $(THIRD_PARTY_DIR)/SearchFilter && make build && cp -a build/. "$(PACKAGES_APP_DEST)/Search and Filter/"

dist: build
	@$(ECHO) $(PRINT_RECIPE)
# Package RetroArch separately
	@cd $(BUILD_DIR) && zip -rq retroarch.pak RetroArch
	@mv -f $(BUILD_DIR)/RetroArch $(ROOT_DIR)/cache/RetroArch
	@mkdir -p $(DIST_FULL)/RetroArch
	@mv $(BUILD_DIR)/retroarch.pak $(DIST_FULL)/RetroArch/
	@echo $(RA_SUBVERSION) > $(DIST_FULL)/RetroArch/ra_package_version.txt
# Package core
	@cd $(BUILD_DIR) && zip -rq $(DIST_FULL)/miyoo/app/.tmp_update/onion.pak .
# Package configs
	@mkdir -p $(ROOT_DIR)/temp/configs $(DIST_FULL)/miyoo/app/.tmp_update/config
	@rsync -a --exclude='.gitkeep' $(STATIC_CONFIGS)/ $(ROOT_DIR)/temp/configs
	@cp -R $(ROOT_DIR)/temp/configs/Saves/CurrentProfile/ $(ROOT_DIR)/temp/configs/Saves/GuestProfile
	@cd $(ROOT_DIR)/temp/configs && zip -rq $(DIST_FULL)/miyoo/app/.tmp_update/config/configs.pak .
# Create core-only dist
	@cp -R $(DIST_FULL)/.tmp_update $(DIST_CORE)/.tmp_update
	@cp -R $(DIST_FULL)/miyoo $(DIST_CORE)/miyoo
# Restore RetroArch in build dir
	@mv -f $(ROOT_DIR)/cache/RetroArch $(BUILD_DIR)/RetroArch

release: dist
	@$(ECHO) $(PRINT_RECIPE)
	@rm -f $(RELEASE_DIR)/$(RELEASE_NAME)-full.zip $(RELEASE_DIR)/$(RELEASE_NAME)-core.zip
	@cd $(DIST_FULL) && zip -rq $(RELEASE_DIR)/$(RELEASE_NAME)-full.zip .
	@cd $(DIST_CORE) && zip -rq $(RELEASE_DIR)/$(RELEASE_NAME)-core.zip .

clean:
	@$(ECHO) $(PRINT_RECIPE)
	@rm -rf $(BUILD_DIR) $(ROOT_DIR)/dist $(ROOT_DIR)/temp/configs
	@rm -f $(CACHE)/.setup
	@find include src -type f -name *.o -exec rm -f {} \;

dev: clean
	@$(MAKE_DEV)

git-clean:
	@git clean -xfd -e .vscode

git-submodules:
	@git submodule update --init --recursive

with-toolchain:
	docker pull $(TOOLCHAIN)
	docker run --rm -v "$(ROOT_DIR)":/root/workspace $(TOOLCHAIN) /bin/bash -c "source /root/.bashrc; make $(CMD)"

patch:
	@chmod a+x $(ROOT_DIR)/.github/create_patch.sh && $(ROOT_DIR)/.github/create_patch.sh

lib:
	@cd $(ROOT_DIR)/include/cJSON && make clean && make
	@cd $(ROOT_DIR)/include/SDL && make clean && make
