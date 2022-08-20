$(TARGET): $(OFILES)
	@$(CXX) $(OFILES) -o "$@" $(LDFLAGS)	
	@if test -z "$(DEBUG)"; then \
		$(STRIP) "$@"; \
	fi
	@-mv -f $(TARGET) "$(BUILD_DIR)/$(TARGET)"

%.o: %.c
	@$(ECHO) $(PRINT_BUILD)
	@$(ECHO) $(COMPILE_CC_OUT)

%.o: %.cpp
	@$(ECHO) $(PRINT_BUILD)
	@$(ECHO) $(COMPILE_CXX_OUT)

../infoPanel/imagesCache.o: ../infoPanel/imagesCache.c
	$(info CXX $(CXX))
	$(info CXXFLAGS $(CXXFLAGS))
	$(CXX) $(CXXFLAGS) -c ../infoPanel/imagesCache.c -o ../infoPanel/imagesCache.o

clean:
	@$(ECHO) $(PRINT_RECIPE)
	@rm -f $(TARGET) $(OFILES)

install:
	@echo "do nothing for install"

dev: clean
	@$(MAKE_DEV)
