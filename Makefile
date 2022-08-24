###########################################################

TARGET=Onion
VERSION=4.0.0
RA_SUBVERSION=0.1.1

###########################################################

RELEASE_NAME := $(TARGET)-v$(VERSION)

# Directories
ROOT_DIR            := $(shell pwd -P)
SRC_DIR             := $(ROOT_DIR)/src
THIRD_PARTY_DIR     := $(ROOT_DIR)/third-party
BUILD_DIR           := $(ROOT_DIR)/build
BUILD_TEST_DIR      := $(ROOT_DIR)/build_test
BIN_DIR             := $(ROOT_DIR)/build/.tmp_update/bin
DIST_FULL           := $(ROOT_DIR)/dist/full
DIST_CORE           := $(ROOT_DIR)/dist/core
INSTALLER_DIR       := $(DIST_FULL)/miyoo/app/.tmp_update
RELEASE_DIR         := $(ROOT_DIR)/release
STATIC_BUILD        := $(ROOT_DIR)/static/build
STATIC_DIST         := $(ROOT_DIR)/static/dist
STATIC_CONFIGS      := $(ROOT_DIR)/static/configs
CACHE               := $(ROOT_DIR)/cache
STATIC_PACKAGES     := $(ROOT_DIR)/static/packages
PACKAGES_EMU_DEST   := $(BUILD_DIR)/Packages/Emu
PACKAGES_APP_DEST   := $(BUILD_DIR)/Packages/App
PACKAGES_RAPP_DEST  := $(BUILD_DIR)/Packages/RApp
ifeq (,$(GTEST_INCLUDE_DIR))
GTEST_INCLUDE_DIR = /usr/include/
endif

TOOLCHAIN := ghcr.io/onionui/miyoomini-toolchain:latest

include ./src/common/commands.mk

###########################################################

.PHONY: all version core apps external release clean git-clean with-toolchain patch lib test

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
# Copy shared libraries
	@cp -R $(ROOT_DIR)/lib/. $(DIST_FULL)/miyoo/app/.tmp_update/lib
# Set version number
	@mkdir -p $(BUILD_DIR)/.tmp_update/onionVersion
	@echo -n "$(VERSION)" > $(BUILD_DIR)/.tmp_update/onionVersion/version.txt
	@sed -i "s/{VERSION}/$(VERSION)/g" $(BUILD_DIR)/autorun.inf
# Copy all resources from src folders
	@find \
		$(SRC_DIR)/gameSwitcher \
		$(SRC_DIR)/chargingState \
		$(SRC_DIR)/bootScreen \
		$(SRC_DIR)/themeSwitcher \
		$(SRC_DIR)/tweaks \
		-depth -type d -name res -exec cp -r {}/. $(BUILD_DIR)/.tmp_update/res/ \;
	@find $(SRC_DIR)/installUI -depth -type d -name res -exec cp -r {}/. $(INSTALLER_DIR)/res/ \;
# Download themes from theme repo
	@chmod a+x $(ROOT_DIR)/.github/get_themes.sh && $(ROOT_DIR)/.github/get_themes.sh
# Copy static packages
	@mkdir -p $(PACKAGES_APP_DEST) $(PACKAGES_EMU_DEST) $(PACKAGES_RAPP_DEST)
	@rsync -a --exclude='.gitkeep' $(STATIC_PACKAGES)/App/ $(PACKAGES_APP_DEST)
	@rsync -a --exclude='.gitkeep' $(STATIC_PACKAGES)/Emu/ $(PACKAGES_EMU_DEST)
	@rsync -a --exclude='.gitkeep' $(STATIC_PACKAGES)/RApp/ $(PACKAGES_RAPP_DEST)
# Set flag: finished setup
	@touch $(CACHE)/.setup

build: core apps external

core: $(CACHE)/.setup
	@$(ECHO) $(PRINT_RECIPE)
# Build Onion binaries
	@cd $(SRC_DIR)/bootScreen && BUILD_DIR=$(BIN_DIR) make
	@cd $(SRC_DIR)/chargingState && BUILD_DIR=$(BIN_DIR) make
	@cd $(SRC_DIR)/gameSwitcher && BUILD_DIR=$(BIN_DIR) make
	@cd $(SRC_DIR)/mainUiBatPerc && BUILD_DIR=$(BIN_DIR) make
	@cd $(SRC_DIR)/keymon && BUILD_DIR=$(BIN_DIR) make
	@cd $(SRC_DIR)/playActivity && BUILD_DIR=$(BIN_DIR) make
	@cd $(SRC_DIR)/themeSwitcher && BUILD_DIR=$(BIN_DIR) make
	@cd $(SRC_DIR)/tweaks && BUILD_DIR=$(BIN_DIR) make
	@cd $(SRC_DIR)/packageManager && BUILD_DIR=$(BIN_DIR) make
	@cd $(SRC_DIR)/sendkeys && BUILD_DIR=$(BIN_DIR) make
	@cd $(SRC_DIR)/setState && BUILD_DIR=$(BIN_DIR) make
	@cd $(SRC_DIR)/infoPanel && BUILD_DIR=$(BIN_DIR) make
	@cd $(SRC_DIR)/prompt && BUILD_DIR=$(BIN_DIR) make
	@cd $(SRC_DIR)/batmon && BUILD_DIR=$(BIN_DIR) make
# Build dependencies for installer
	@mkdir -p $(DIST_FULL)/miyoo/app/.tmp_update/bin
	@cd $(SRC_DIR)/installUI && BUILD_DIR=$(INSTALLER_DIR)/bin/ make
	@cp $(BIN_DIR)/prompt $(INSTALLER_DIR)/bin/
	@cp $(BIN_DIR)/batmon $(INSTALLER_DIR)/bin/

apps: $(CACHE)/.setup
	@$(ECHO) $(PRINT_RECIPE)
	@cd $(SRC_DIR)/playActivityUI && BUILD_DIR="$(PACKAGES_APP_DEST)/Activity Tracker/App/PlayActivity" make
	@find $(SRC_DIR)/playActivityUI -depth -type d -name res -exec cp -r {}/. "$(PACKAGES_APP_DEST)/Activity Tracker/App/PlayActivity/res/" \;
	@find $(SRC_DIR)/packageManager -depth -type d -name res -exec cp -r {}/. $(BUILD_DIR)/App/PackageManager/res/ \;
	@cd $(SRC_DIR)/clock && BUILD_DIR="$(PACKAGES_APP_DEST)/Clock (Set emulated time)/App/Clock" make
# Preinstalled apps
	@cp -a "$(PACKAGES_APP_DEST)/Activity Tracker/." $(BUILD_DIR)/
	@cp -a "$(PACKAGES_APP_DEST)/Quick Guide/." $(BUILD_DIR)/
	@cp -a "$(PACKAGES_APP_DEST)/RetroArch (Emulator settings)/." $(BUILD_DIR)/
	@cp -a "$(PACKAGES_APP_DEST)/Tweaks/." $(BUILD_DIR)/

external: $(CACHE)/.setup
	@$(ECHO) $(PRINT_RECIPE)
	@cd $(THIRD_PARTY_DIR)/RetroArch && make && cp retroarch $(BUILD_DIR)/RetroArch/
	@echo $(RA_SUBVERSION) > $(BUILD_DIR)/RetroArch/onion_ra_version.txt
	@cd $(THIRD_PARTY_DIR)/SearchFilter && make build && cp -a build/. "$(PACKAGES_APP_DEST)/Search/" && cp build/App/SearchFilter/tools $(BIN_DIR)
	@cd $(THIRD_PARTY_DIR)/Terminal && make && cp ./st "$(PACKAGES_APP_DEST)/Terminal (Developer tool)/App/Terminal"

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
	@rm -rf $(BUILD_DIR) $(BUILD_TEST_DIR) $(ROOT_DIR)/dist $(ROOT_DIR)/temp/configs
	@rm -f $(CACHE)/.setup
	@find include src -type f -name *.o -exec rm -f {} \;

dev: clean
	@$(MAKE_DEV)

git-clean:
	@git clean -xfd -e .vscode

git-submodules:
	@git submodule update --init --recursive

$(CACHE)/.docker:
	docker pull $(TOOLCHAIN)
	mkdir -p cache
	touch $(CACHE)/.docker

toolchain: $(CACHE)/.docker
	docker run -it --rm -v "$(ROOT_DIR)":/root/workspace $(TOOLCHAIN) /bin/bash

with-toolchain: $(CACHE)/.docker
	docker run --rm -v "$(ROOT_DIR)":/root/workspace $(TOOLCHAIN) /bin/bash -c "source /root/.bashrc; make $(CMD)"

patch:
	@chmod a+x $(ROOT_DIR)/.github/create_patch.sh && $(ROOT_DIR)/.github/create_patch.sh

lib:
	@cd $(ROOT_DIR)/include/cJSON && make clean && make
	@cd $(ROOT_DIR)/include/SDL && make clean && make

test:
	@mkdir -p $(BUILD_TEST_DIR) && cd $(ROOT_DIR)/test && BUILD_DIR=$(BUILD_TEST_DIR)/ make
	$(BUILD_TEST_DIR)/test