TEST=1
OFILES := $(OFILES) ../src/infoPanel/imagesCache.o
include ../src/common/config.mk

TARGET = test_infoPanel
LDFLAGS := $(LDFLAGS) -lSDL_image -lSDL -lgtest -lgtest_main -lpthread

include ../src/common/commands.mk
include ../src/common/recipes.mk

../src/infoPanel/imagesCache.o: ../src/infoPanel/imagesCache.c
	$(info CXX $(CXX))
	$(info CXXFLAGS $(CXXFLAGS))
	$(CXX) $(CXXFLAGS) -c ../src/infoPanel/imagesCache.c -o ../src/infoPanel/imagesCache.o