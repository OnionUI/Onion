include ../common/config.mk

TARGET = sqldaemon
CFLAGS := $(CFLAGS) -D_DEFAULT_SOURCE
LDFLAGS := $(LDFLAGS) -lsqlite3 -lpthread

include ../common/commands.mk
include ../common/recipes.mk
