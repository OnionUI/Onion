$(TARGET): $(OFILES)
	@$(CXX) $(OFILES) -o "$@" $(LDFLAGS)	
	@if test -z "$(DEBUG)" && test -z "$(SANITIZE)"; then \
		$(STRIP) "$@"; \
	fi
	@-mv -f $(TARGET) "$(BUILD_DIR)/$(TARGET)"

build: $(TARGET)

%.o: %.c
	@$(ECHO) $(PRINT_BUILD)
	@$(ECHO) $(COMPILE_CC_OUT)

%.o: %.cpp
	@$(ECHO) $(PRINT_BUILD)
	@$(ECHO) $(COMPILE_CXX_OUT)

clean:
	@$(ECHO) $(PRINT_RECIPE)
	@rm -f $(TARGET) $(OFILES)

install:
	@echo "do nothing for install"

dev: clean
	@$(MAKE_DEV)

asan: clean
	@$(MAKE_ASAN)