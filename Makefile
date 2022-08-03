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
PACKAGE_DIR         := $(ROOT_DIR)/package/full
PACKAGE_CORE_DIR    := $(ROOT_DIR)/package/core
RELEASE_DIR         := $(ROOT_DIR)/release
STATIC_BUILD        := $(ROOT_DIR)/static/build
STATIC_PACKAGE      := $(ROOT_DIR)/static/package
STATIC_CONFIGS      := $(ROOT_DIR)/static/configs
CACHE               := $(ROOT_DIR)/cache

TOOLCHAIN := ghcr.io/onionui/miyoomini-toolchain:latest

PLATFORM ?= $(UNION_PLATFORM)
ifeq (,$(PLATFORM))
PLATFORM=linux
endif

###########################################################

.PHONY: all version core apps external release clean git-clean with-toolchain patch lib

all: clean package

version: # used by workflow
	@echo $(VERSION)
print-version:
	@echo Onion v$(VERSION)
	@echo RetroArch sub-v$(RA_SUBVERSION)

$(CACHE)/.setup:
	@echo :: $(TARGET) - setup
	@mkdir -p $(BUILD_DIR) $(PACKAGE_DIR) $(PACKAGE_CORE_DIR) $(RELEASE_DIR)
	@cp -R $(STATIC_BUILD)/. $(BUILD_DIR)
	@cp -R $(STATIC_PACKAGE)/. $(PACKAGE_DIR)
	@cp -R $(ROOT_DIR)/lib/. $(PACKAGE_DIR)/miyoo/app/.tmp_update/lib
	@mkdir -p $(BUILD_DIR)/.tmp_update/onionVersion
	@echo -n "$(VERSION)" > $(BUILD_DIR)/.tmp_update/onionVersion/version.txt
	@chmod a+x $(ROOT_DIR)/.github/get_themes.sh && $(ROOT_DIR)/.github/get_themes.sh
	@touch $(CACHE)/.setup

build: core apps external

core: $(CACHE)/.setup
	@echo :: $(TARGET) - build core
# Build Onion binaries
	@cd $(SRC_DIR)/bootScreen && BUILD_DIR=$(BIN_DIR) make
	@cd $(SRC_DIR)/chargingState && BUILD_DIR=$(BIN_DIR) make
	@cd $(SRC_DIR)/checkCharge && BUILD_DIR=$(BIN_DIR) make
	@cd $(SRC_DIR)/gameSwitcher && BUILD_DIR=$(BIN_DIR) make
	@cd $(SRC_DIR)/lastGame && BUILD_DIR=$(BIN_DIR) make
	@cd $(SRC_DIR)/mainUiBatPerc && BUILD_DIR=$(BIN_DIR) make
	@cd $(SRC_DIR)/onionKeymon && BUILD_DIR=$(BIN_DIR) make
# Build installer binaries
	@mkdir -p $(PACKAGE_DIR)/miyoo/app/.tmp_update/bin
	@cd $(SRC_DIR)/installUI && BUILD_DIR=$(PACKAGE_DIR)/miyoo/app/.tmp_update/bin make
	@cd $(SRC_DIR)/prompt && BUILD_DIR=$(PACKAGE_DIR)/miyoo/app/.tmp_update/bin make

apps: $(CACHE)/.setup
	@echo :: $(TARGET) - build apps
	@cd $(SRC_DIR)/playActivity && BUILD_DIR=$(BUILD_DIR)/App/PlayActivity make
	@cd $(SRC_DIR)/playActivityUI && BUILD_DIR=$(BUILD_DIR)/App/PlayActivity make
	@cd $(SRC_DIR)/onionInstaller && BUILD_DIR=$(BUILD_DIR)/App/The_Onion_Installer make
	@cd $(SRC_DIR)/themeSwitcher && BUILD_DIR=$(BUILD_DIR)/App/The_Onion_Installer/data/Layer2/ThemeSwitcher/App/ThemeSwitcher make

external: $(CACHE)/.setup
	@echo :: $(TARGET) - build external
	@cd $(THIRD_PARTY_DIR)/RetroArch && make && cp retroarch $(BUILD_DIR)/RetroArch/
	@echo $(RA_SUBVERSION) > $(BUILD_DIR)/RetroArch/onion_ra_version.txt
	@cd $(THIRD_PARTY_DIR)/SearchFilter && make build && cp -a build/. "$(BUILD_DIR)/App/The_Onion_Installer/data/Layer2/Search and Filter/"

package: build
	@echo :: $(TARGET) - package
# Package RetroArch separately
	@cd $(BUILD_DIR) && zip -rq retroarch.pak RetroArch
	@rm -rf $(BUILD_DIR)/RetroArch
	@mkdir -p $(PACKAGE_DIR)/RetroArch
	@mv $(BUILD_DIR)/retroarch.pak $(PACKAGE_DIR)/RetroArch/
	@echo $(RA_SUBVERSION) > $(PACKAGE_DIR)/RetroArch/ra_package_version.txt
# Package themes separately
	@mkdir -p $(PACKAGE_DIR)/Themes
	@mv $(BUILD_DIR)/Themes/* $(PACKAGE_DIR)/Themes/
# Package core
	@cd $(BUILD_DIR) && zip -rq $(PACKAGE_DIR)/miyoo/app/.tmp_update/onion.pak .
# Package configs
	@mkdir -p $(PACKAGE_DIR)/miyoo/app/.tmp_update/config
	@cp -R $(STATIC_CONFIGS)/Saves/CurrentProfile $(STATIC_CONFIGS)/Saves/GuestProfile
	@cd $(STATIC_CONFIGS) && zip -rq $(PACKAGE_DIR)/miyoo/app/.tmp_update/config/configs.pak .
	@rm -rf $(STATIC_CONFIGS)/Saves/GuestProfile
# Create core-only package
	@cp -R $(PACKAGE_DIR)/.tmp_update $(PACKAGE_CORE_DIR)/.tmp_update
	@cp -R $(PACKAGE_DIR)/miyoo $(PACKAGE_CORE_DIR)/miyoo

release: package
	@echo :: $(TARGET) - release
	@rm -f $(RELEASE_DIR)/$(RELEASE_NAME)-full.zip
	@rm -f $(RELEASE_DIR)/$(RELEASE_NAME)-core.zip
	@cd $(PACKAGE_DIR) && zip -rq $(RELEASE_DIR)/$(RELEASE_NAME)-full.zip .
	@cd $(PACKAGE_CORE_DIR) && zip -rq $(RELEASE_DIR)/$(RELEASE_NAME)-core.zip .

clean:
	@rm -rf $(BUILD_DIR) $(ROOT_DIR)/package
	@rm -f $(CACHE)/.setup
	@find include src -type f -name *.o -exec rm -f {} \;
	@echo :: $(TARGET) - cleaned

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
