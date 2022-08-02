###########################################################

TARGET=Onion
VERSION=3.13.0
RA_SUBVERSION=0.1

###########################################################

RELEASE_NAME := $(TARGET)-v$(VERSION)
ROOT_DIR := $(shell pwd -P)
SRC_DIR := $(ROOT_DIR)/src
THIRD_PARTY_DIR := $(ROOT_DIR)/third-party
BUILD_DIR := $(ROOT_DIR)/build
PACKAGE_DIR := $(ROOT_DIR)/package
RELEASE_DIR := $(ROOT_DIR)/release
STATIC_BUILD := $(ROOT_DIR)/static/build
STATIC_PACKAGE := $(ROOT_DIR)/static/package
STATIC_CONFIGS := $(ROOT_DIR)/static/configs
CACHE := $(ROOT_DIR)/cache
TOOLCHAIN := ghcr.io/onionui/miyoomini-toolchain:latest

###########################################################

.PHONY: all version core apps external release clean git-clean with-toolchain patch lib

all: clean package

version:
	@echo $(VERSION)

$(CACHE)/.setup:
	@echo :: $(TARGET) - setup
	@git submodule update --init --recursive
	@mkdir -p $(BUILD_DIR) $(PACKAGE_DIR) $(RELEASE_DIR)
	@cp -R $(STATIC_BUILD)/. $(BUILD_DIR)
	@cp -R $(STATIC_PACKAGE)/. $(PACKAGE_DIR)
	@cp -R $(ROOT_DIR)/lib/. $(BUILD_DIR)/.tmp_update/lib
	@echo -n " v$(VERSION)" > $(BUILD_DIR)/.tmp_update/onionVersion/version.txt
	@chmod a+x $(ROOT_DIR)/.github/get_themes.sh && $(ROOT_DIR)/.github/get_themes.sh
	@touch $(CACHE)/.setup

build: core apps external

core: $(CACHE)/.setup
	@echo :: $(TARGET) - build core
# Build Onion binaries
	@cd $(SRC_DIR)/bootScreen && BUILD_DIR=$(BUILD_DIR)/.tmp_update make
	@cd $(SRC_DIR)/chargingState && BUILD_DIR=$(BUILD_DIR)/.tmp_update make
	@cd $(SRC_DIR)/checkCharge && BUILD_DIR=$(BUILD_DIR)/.tmp_update make
	@cd $(SRC_DIR)/gameSwitcher && BUILD_DIR=$(BUILD_DIR)/.tmp_update make
	@cd $(SRC_DIR)/lastGame && BUILD_DIR=$(BUILD_DIR)/.tmp_update make
	@cd $(SRC_DIR)/mainUiBatPerc && BUILD_DIR=$(BUILD_DIR)/.tmp_update make
	@cd $(SRC_DIR)/onionKeymon && BUILD_DIR=$(BUILD_DIR)/.tmp_update make
# Build install binaries
	@cd $(SRC_DIR)/installUI && BUILD_DIR=$(PACKAGE_DIR)/miyoo/app/.installer make
	@cd $(SRC_DIR)/installPrompt && BUILD_DIR=$(PACKAGE_DIR)/miyoo/app/.installer make

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
	@cd $(THIRD_PARTY_DIR)/SearchFilter && make && cp -a build/. "$(BUILD_DIR)/App/The_Onion_Installer/data/Layer2/Search and Filter/"

package: build
	@echo :: $(TARGET) - package
# Package RetroArch separately
	@cd $(BUILD_DIR) && zip -rq retroarch_package.zip RetroArch
	@rm -rf $(BUILD_DIR)/RetroArch
	@mkdir -p $(PACKAGE_DIR)/RetroArch
	@mv $(BUILD_DIR)/retroarch_package.zip $(PACKAGE_DIR)/RetroArch/
	@echo $(RA_SUBVERSION) > $(PACKAGE_DIR)/RetroArch/ra_package_version.txt
# Package core
	@cd $(BUILD_DIR) && zip -rq $(PACKAGE_DIR)/miyoo/app/.installer/onion_package.zip .
# Package configs
	@cp -R $(STATIC_CONFIGS)/Saves/CurrentProfile $(STATIC_CONFIGS)/Saves/GuestProfile
	@cd $(STATIC_CONFIGS) && zip -rq $(PACKAGE_DIR)/miyoo/app/.installer/configs.zip .
	@rm -rf $(STATIC_CONFIGS)/Saves/GuestProfile

release: package
	@echo :: $(TARGET) - release
	@rm -f $(RELEASE_DIR)/$(RELEASE_NAME).zip
	@cd $(PACKAGE_DIR) && zip -rq $(RELEASE_DIR)/$(RELEASE_NAME).zip .

clean:
	@rm -rf $(BUILD_DIR) $(PACKAGE_DIR)
	@rm -f $(CACHE)/.setup
	@find include src -type f -name *.o -exec rm -f {} \;
	@echo :: $(TARGET) - cleaned

git-clean:
	@git clean -xfd -e .vscode

with-toolchain:
	docker pull $(TOOLCHAIN)
	docker run --rm -v "$(ROOT_DIR)":/root/workspace $(TOOLCHAIN) /bin/bash -c "source /root/.bashrc; make $(CMD)"

patch:
	@chmod a+x $(ROOT_DIR)/.github/create_patch.sh && $(ROOT_DIR)/.github/create_patch.sh

lib:
	@cd $(ROOT_DIR)/include/cJSON && make clean && make
	@cd $(ROOT_DIR)/include/SDL && make clean && make
