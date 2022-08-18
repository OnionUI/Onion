TEST=1
OFILES := $(OFILES) ../infoPanel/imagesCache.o
include ../common/config.mk

TARGET = test_infoPanel
CXXFLAGS := $(CXXFLAGS) -I/usr/local/include/SDL/
LDFLAGS := $(LDFLAGS) -lSDL_image -lSDL -lgtest -lgtest_main

include ../common/commands.mk
include ../common/recipes.mk
