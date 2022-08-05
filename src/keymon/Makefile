ifeq (,$(BUILD_DIR))
BUILD_DIR=.
endif

PLATFORM ?= $(UNION_PLATFORM)
ifeq (,$(PLATFORM))
PLATFORM=linux
endif

#########################################################################

TARGET = $(BUILD_DIR)/keymon

#########################################################################

CC 	= $(CROSS_COMPILE)gcc

SOURCES = . ../../include/cjson
CFILES = $(foreach dir, $(SOURCES), $(wildcard $(dir)/*.c))
OFILES = $(CFILES:.c=.o)

CFLAGS = -I../../include -I../common -DPLATFORM_$(shell echo $(PLATFORM) | tr a-z A-Z) -Os
CFLAGS += -ffunction-sections -fdata-sections -Wall
LDFLAGS = -L../../lib -lpthread -lpng -Wl,--gc-sections -s

ifeq ($(PLATFORM),miyoomini)
CFLAGS := $(CFLAGS) -marm -mtune=cortex-a7 -mfpu=neon-vfpv4 -mfloat-abi=hard -march=armv7ve -Wl,-rpath=$(LIB)
LDFLAGS := $(LDFLAGS) -lshmvar
endif

$(TARGET): $(OFILES)
	$(CC) $(OFILES) -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(TARGET2) $(OFILES)
