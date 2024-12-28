TARGET_NAME = $(shell basename "$(TARGET)")

ECHO = echo

COLOR_BLUE = ""
COLOR_NORMAL = ""

PRINT_BUILD = "\n-- [$(TARGET_NAME)] Building $<"
PRINT_RECIPE = "\n:: $(TARGET_NAME) -> $@"
PRINT_DONE = "\n-> $@ [READY!]\n"

COMPILE_CC = $(CC) $(CFLAGS) -std=gnu18 -c "$<" -o "$@"
COMPILE_CXX = $(CXX) $(CXXFLAGS) -std=c++17 -c "$<" -o "$@"

COMPILE_CC_OUT = "> $(COMPILE_CC)$$($(COMPILE_CC) 2>&1)"
COMPILE_CXX_OUT = "> $(COMPILE_CXX)$$($(COMPILE_CXX) 2>&1)"

PLATFORM ?= $(UNION_PLATFORM)
ifeq (,$(PLATFORM))
PLATFORM=linux
endif

ifeq ($(PLATFORM),linux)
ECHO = echo -e
endif

COLOR_REP = sed \
	-e 's/\(\/usr\/sbin\/ld: skipping incompatible.*\)/\\\e[0;33m\1\\\e[0m/g' \
	-e 's/\(warning\)/\\\e[0;33m\1\\\e[0m/g' \
	-e 's/\(make\[[0-9]\+\]: Entering directory.*\)/\\\e[1;30m\1\\\e[0m/g' \
	-e 's/\(make\[[0-9]\+\]: Leaving directory.*\)/\\\e[1;30m\1\\\e[0m/g'
MAKE_DEV = DEBUG=1 COLORS=1 make --no-print-directory
MAKE_ASAN = DEBUG=1 SANITIZE=1 COLORS=1 make --no-print-directory

ifeq (1,$(COLORS))
COLOR_BLUE = "\e[1;34m"
COLOR_NORMAL = "\e[0m"
PRINT_BUILD = "\e[1;34m-- [$(TARGET_NAME)] Building $<\e[0m"
PRINT_RECIPE = "\e[1;34m:: $(TARGET_NAME) -> $@\e[0m"
PRINT_DONE = "\n\e[1;32m-> $@ [READY!]\e[0m\n"
COMPILE_CC_OUT = "\e[1;30m> $(COMPILE_CC)\e[0m$$($(COMPILE_CC) -fdiagnostics-color=always 2>&1)"
COMPILE_CXX_OUT = "\e[1;30m> $(COMPILE_CXX)\e[0m$$($(COMPILE_CXX) -fdiagnostics-color=always 2>&1)"
endif