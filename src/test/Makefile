TEST=1
OFILES := $(OFILES) ../infoPanel/imagesCache.o
include ../common/config.mk

TARGET = test_infoPanel
LDFLAGS := $(LDFLAGS) -lSDL_image -lSDL -lgtest -lgtest_main -lpthread

include ../common/commands.mk
include ../common/recipes.mk

../infoPanel/imagesCache.o: ../infoPanel/imagesCache.c
	$(info CXX $(CXX))
	$(info CXXFLAGS $(CXXFLAGS))
	$(CXX) $(CXXFLAGS) -c ../infoPanel/imagesCache.c -o ../infoPanel/imagesCache.o
