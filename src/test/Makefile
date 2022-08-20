TEST=1
OFILES := $(OFILES) ../infoPanel/imagesCache.o
include ../common/config.mk

TARGET = test_infoPanel
LDFLAGS := $(LDFLAGS) -lSDL_image -lSDL -lgtest -lgtest_main -lpthread

include ../common/commands.mk
include ../common/recipes.mk
