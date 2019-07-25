#### PROJECT SETTINGS ####
# The name of the executable to be created
BIN_NAME := linksysat
# Compiler used
CXX = gcc
AR = ar
# Extension of source files used in the project
SRC_EXT = c
# Path to the source directory, relative to the makefile
SRC_PATH = .
# FIles for creating libraries ( without main.c)
LIB_FILES = at_parse.c at_parse.h
# General compiler flags
#COMPILE_FLAGS = -std=c++11 -Wall -Wextra
COMPILE_FLAGS = -Wall -Wextra
# Additional release-specific flags
RCOMPILE_FLAGS = -D NDEBUG
# Additional debug-specific flags
DCOMPILE_FLAGS = -g 
# Add additional include paths
INCLUDES = -I $(SRC_PATH)/
# General linker settings
LINK_FLAGS = 
# Additional release-specific linker settings
RLINK_FLAGS = 
# Additional debug-specific linker settings
DLINK_FLAGS = 
# Destination directory, like a jail or mounted system
DESTDIR = /
# Install path (bin/ is appended automatically)
#INSTALL_PREFIX = usr/local
INSTALL_PREFIX = `pwd`
#### END PROJECT SETTINGS ####

# Generally should not need to edit below this line

# Verbose option, to output compile and link commands
export V = false
export CMD_PREFIX = @
ifeq ($(V),true)
	CMD_PREFIX = 
endif

# Combine compiler and linker flags
release: export CXXFLAGS := $(CXXFLAGS) $(COMPILE_FLAGS) $(RCOMPILE_FLAGS)
release: export LDFLAGS := $(LDFLAGS) $(LINK_FLAGS) $(RLINK_FLAGS)
debug: export CXXFLAGS := $(CXXFLAGS) $(COMPILE_FLAGS) $(DCOMPILE_FLAGS)
debug: export LDFLAGS := $(LDFLAGS) $(LINK_FLAGS) $(DLINK_FLAGS)

# Build and output paths
release: export BUILD_PATH := build/release
release: export BIN_PATH := bin/release
debug: export BUILD_PATH := build/debug
debug: export BIN_PATH := bin/debug
install: export BIN_PATH := bin/release

# Find all source files in the source directory
SOURCES = $(shell find $(SRC_PATH)/ -name '*.$(SRC_EXT)')
# Set the object file names, with the source directory stripped
# from the path, and the build path prepended in its place
OBJECTS = $(SOURCES:$(SRC_PATH)/%.$(SRC_EXT)=$(BUILD_PATH)/%.o)
# Set the dependency files that will be used to add header dependencies
DEPS = $(OBJECTS:.o=.d)
#DEPS = $(OBJECTS:.o)

# Macros for timing compilation
TIME_FILE = $(dir $@).$(notdir $@)_time
START_TIME = date '+%s' > $(TIME_FILE)
END_TIME = read st < $(TIME_FILE) ; \
	$(RM) $(TIME_FILE) ; \
	st=$$((`date '+%s'` - $$st - 86400)) ; \
	echo `date -u -d @$$st '+%H:%M:%S'` 


VERSION_MAJOR := "0"
VERSION_MINOR := "1"
#VERSION_REVISION := "0" 
VERSION_REVISION :=$(shell date +%Y%m%d%H%M)
VERSION_STRING := \
	"$(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_REVISION)"
override CXXFLAGS := $(CXXFLAGS) \
	-D VERSION_MAJOR=$(VERSION_MAJOR) \
	-D VERSION_MINOR=$(VERSION_MINOR) \
	-D VERSION_REVISION=$(VERSION_REVISION)
# 	-D VERSION_PATCH=$(VERSION_PATCH) \
# 	-D VERSION_REVISION=$(VERSION_REVISION) \
# 	-D VERSION_HASH=\"$(VERSION_HASH)\"
VERSION_NUMBER := `echo	"$(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_REVISION)" | tr -d ' '`

# Standard, non-optimized release build
.PHONY: release
release: dirs
	@echo "Beginning release build v$(VERSION_NUMBER)"
	@$(START_TIME)
	@$(MAKE) all --no-print-directory
	@echo -n "Total build time: "
	@$(END_TIME)

# Debug build for gdb debugging
.PHONY: debug
debug: dirs
	@echo "Beginning debug build v$(VERSION_NUMBER)"
	@$(START_TIME)
	@$(MAKE) all --no-print-directory
	@echo -n "Total build time: "
	@$(END_TIME)

# Create the directories used in the build
.PHONY: dirs
dirs:
	@echo "Creating directories"
	@mkdir -p $(dir $(OBJECTS))
	@mkdir -p $(BIN_PATH)

# Installs to the set path
.PHONY: install
install:
	@echo "Installing to $(DESTDIR)$(INSTALL_PREFIX)/bin"
	@install -m 0755 $(BIN_PATH)/$(BIN_NAME) $(DESTDIR)$(INSTALL_PREFIX)/bin

# Removes all build files
.PHONY: clean
clean:
	@echo "Deleting $(BIN_NAME) symlink"
	@$(RM) $(BIN_NAME)
	@echo "Deleting libs"
	@$(RM) -r lib
	@$(RM) .liblinksysat.*
	@echo "Deleting directories"
	@$(RM) -r build
	@$(RM) -r bin
	@echo "Deleting build time files"
	@$(RM) -f .*_time
#	@$(RM) ../$(BIN_NAME)-$(VERSION_NUMBER).tgz
	
lib: liblinksysat.a
	@echo "installing liblinksysat.a liblinksysat.so.o in $(DESTDIR)$(INSTALL_PREFIX)/lib"
	@mkdir -p $(DESTDIR)$(INSTALL_PREFIX)/lib
	@install -m 0755 liblinksysat.a liblinksysat.so.o $(DESTDIR)$(INSTALL_PREFIX)/lib
	@mv $(DESTDIR)$(INSTALL_PREFIX)/lib/liblinksysat.so.o $(DESTDIR)$(INSTALL_PREFIX)/lib/liblinksysat.$(VERSION_MAJOR).$(VERSION_MINOR).so.o
	@$(RM) liblinksysat.a liblinksysat.so.o
	
liblinksysat.a: liblinksysat.so.o
	@echo "creating static lib $@"
	$(CMD_PREFIX)$(AR) rcs $@ $^

liblinksysat.so.o: $(LIB_FILES)
	@echo "creating dynamic lib $@"
#	gcc -c -o $@ $<
	$(CMD_PREFIX)$(CXX) -fPIC -c -o $@ $<


# Main rule, checks the executable and symlinks to the output
all: $(BIN_PATH)/$(BIN_NAME) lib
	@echo "Making symlink: $(BIN_NAME) -> $<"
	@$(RM) $(BIN_NAME)
	@ln -s $(BIN_PATH)/$(BIN_NAME) $(BIN_NAME)


tarball: clean
	@echo "Making tarball $(BIN_NAME)-$(VERSION_NUMBER).tgz"
	@cd .. && tar -czf $(BIN_NAME)-$(VERSION_NUMBER).tgz $(BIN_NAME)

# Link the executable
$(BIN_PATH)/$(BIN_NAME): $(OBJECTS)
	@echo "Linking: $@"
	@$(START_TIME)
	$(CMD_PREFIX)$(CXX) $(OBJECTS) $(LDFLAGS) -o $@
	@echo -en "\t Link time: "
	@$(END_TIME)

# Add dependency files, if they exist
-include $(DEPS)

# Source file rules
# After the first compilation they will be joined with the rules from the
# dependency files to provide header dependencies
$(BUILD_PATH)/%.o: $(SRC_PATH)/%.$(SRC_EXT)
	@echo "Compiling: $< -> $@"
#	@$(START_TIME)
	$(CMD_PREFIX)$(CXX) $(CXXFLAGS) $(INCLUDES) -MP -MMD -c $< -o $@
	@echo -en "\t Compile time: "
#	@$(END_TIME)

