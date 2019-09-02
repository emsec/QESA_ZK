#############################################
##                                         ##
##        Universal C/C++ Makefile         ##
##                                         ##
##   Targets:                              ##
##   help      show usage information      ##
##   all       compile, link and execute   ##
##   compile   compile and link            ##
##   clean     remove output directories   ##
##                                         ##
#############################################

#############################################
##              CONFIGURATION              ##
#############################################

# Include directory with the .h files
INC_DIR = inc

# Source directory with the .c and .cpp files
SRC_DIR = src

# Target directory for the .o files
OBJ_DIR = obj

# Output directory
BIN_DIR = bin

# Library directory with the .a files
LIB_DIR = lib

# Compiler options
COMMON_FLAGS = -Wall -Wextra -Wshadow -O3 -fomit-frame-pointer
C_FLAGS = -std=c99
CXX_FLAGS = -std=c++17
DEBUG_FLAGS = -Og -g3
LFLAGS = -lgmp

# Output file
OUTPUT = out

# program arguments
ARGS =

# Compilers
#CC  = clang
#CXX = clang++

# Verbosity flag
V = 0

# Debug flag
D = 0

# Build dependencies flag
DEPS = 1

#############################################
##          CORE (do not touch)            ##
#############################################

.PHONY: all compile clean help directories

# list all .c and .cpp files
C_LIST := $(shell find $(SRC_DIR) -name "*.c")
CXX_LIST := $(shell find $(SRC_DIR) -name "*.cpp")

# list all .a files
LIB_FILES := $(wildcard $(LIB_DIR)/*.a)

# create object file names in the obj directory
OBJ_FILES := $(patsubst $(SRC_DIR)/%,$(OBJ_DIR)/%, $(C_LIST:.c=.o)) $(patsubst $(SRC_DIR)/%,$(OBJ_DIR)/%, $(CXX_LIST:.cpp=.o))

# if verbosity is set to 0 pipe outputs to null and supress command printing
ifeq ($(V),0)
	AT := @
	PIPE := > /dev/null
endif

# if the debug flag is set, append debug options
ifeq ($(D),1)
	C_FLAGS += -g3
	CXX_FLAGS += -g3
endif

# if the dependency flag is set, append dependency options
ifeq ($(DEPS),1)
	DEP_FLAGS = -MT $@ -MMD -MP -MF $(OBJ_DIR)/$*.d
endif

ifeq ($(CXX_LIST),)
	LINK := $(CC)
else
	LINK := $(CXX)
endif

START_TIME := $(shell date +%s%3N)

MAKEFLAGS += --no-print-directory

SYS := $(shell gcc -dumpmachine)

# compile and execute
all: compile
	@echo  '__________Executing__________'
ifeq ($(V), 0)
ifneq ($(ARGS), )
	@echo -e 'program arguments: "$(ARGS)"'
endif
endif
	@echo
	$(AT)$(BIN_DIR)/$(OUTPUT) $(ARGS)

# create obj directory and compile
compile: directories $(BIN_DIR)/$(OUTPUT)
	@diff=$$(($(shell date +%s%3N) - $(START_TIME))); echo 'Build process completed in '$$(($$diff / 1000))'.'$$(($$diff % 1000))'s'
	@echo

# create the obj directory
directories:
	@echo  '__________Building__________'
	@mkdir -p $(OBJ_DIR)
	@mkdir -p $(BIN_DIR)

# remove the object and output directory
clean:
	@echo  Removing .o and .exe files...
	$(AT)rm -rf $(OBJ_DIR)
	$(AT)rm -rf $(BIN_DIR)
	$(AT)rm -rf *.stackdump

# print help information
help:
	@echo 'make {all, compile, clean} [V=1 for verbose output, D=1 for debug compiling, DEPS=0 to disable dependency files generation]'

# link output
$(BIN_DIR)/$(OUTPUT): $(OBJ_FILES)
	@echo
	@echo  '___________Linking___________'
ifeq ($(V), 0)
	@echo  -e 'LINK\t$(OUTPUT)'
endif
	$(AT)$(LINK) $(OBJ_FILES) -I $(INC_DIR) $(LIB_FILES) -o $(BIN_DIR)/$(OUTPUT) $(LFLAGS) $(PIPE)
	@echo

# compile code files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
ifeq ($(V), 0)
	@echo  -e 'CC\t$(notdir $<)'
endif
	@mkdir -p '$(dir $@)'
	$(AT)$(CC) $(DEP_FLAGS) $(COMMON_FLAGS) $(C_FLAGS) -I $(INC_DIR) -c $< -o $@ $(PIPE)
ifeq ($(DEPS), 1)
	@touch $@
ifneq (,$(findstring cygwin, $(SYS)))
	@mv -f $(OBJ_DIR)/$*.d $(OBJ_DIR)/$*.d.tmp
	@sed -e 's|.*:|$(OBJ_DIR)/$*.o:|' < $(OBJ_DIR)/$*.d.tmp > $(OBJ_DIR)/$*.d
	@sed -e 's/.*://' -e 's/\\$$//' < $(OBJ_DIR)/$*.d.tmp | fmt -1 | sed -e 's/^ *//' -e 's/$$/:/' >> $(OBJ_DIR)/$*.d
	@rm -f $(OBJ_DIR)/$*.d.tmp
endif
endif

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
ifeq ($(V), 0)
	@echo  -e 'CXX\t$(notdir $<)'
endif
	@mkdir -p '$(dir $@)'
	$(AT)$(CXX) $(DEP_FLAGS) $(COMMON_FLAGS) $(CXX_FLAGS) -I $(INC_DIR) -c $< -o $@ $(PIPE)
ifeq ($(DEPS), 1)
	@touch $@
ifneq (,$(findstring cygwin, $(SYS)))
	@mv -f $(OBJ_DIR)/$*.d $(OBJ_DIR)/$*.d.tmp
	@sed -e 's|.*:|$(OBJ_DIR)/$*.o:|' < $(OBJ_DIR)/$*.d.tmp > $(OBJ_DIR)/$*.d
	@sed -e 's/.*://' -e 's/\\$$//' < $(OBJ_DIR)/$*.d.tmp | fmt -1 | sed -e 's/^ *//' -e 's/$$/:/' >> $(OBJ_DIR)/$*.d
	@rm -f $(OBJ_DIR)/$*.d.tmp
endif
endif


#Pull in dependency info for *existing* .o files
ifeq ($(DEPS), 1)
-include $(OBJ_FILES:.o=.d)
endif
