#
# Makefile for ftphid_ezupg_ap.
# Date: 2024/09/27.
#
program := ftphid_ezupg_ap
objects :=global.o \
	ftpprogram.o \
	ftputility.o \
	main.o
libraries := stdc++ rt pthread
executable_path := ./bin
source_path := ./src
include_path := ./include 

CXX ?= g++ # Compiler: GCC C++ Compiler
WARN_FLAGS = -Wall -Wno-format-overflow
STD_FLAGS = -ansi
OPT_FLAGS = -O3
DEBUG_FLAGS = 
CXXFLAGS = $(WARN_FLAGS) $(STD_FLAGS) $(OPT_FLAGS) $(DEBUG_FLAGS)

INC_FLAGS += $(addprefix -I, $(include_path))
LIB_FLAGS += $(addprefix -l, $(libraries))
VPATH = $(include_path)
vpath %.h $(include_path)
vpath %.c $(source_path)
vpath %.cpp $(source_path)
.SUFFIXS: .c .cpp .h

.PHONY: all
all: $(objects)
	$(CXX) $^ $(CXXFLAGS) $(CPPFLAGS) $(INC_FLAGS) $(LIB_FLAGS) -o $(program)
	@chmod 777 $(program)
	@mv $(program) $(executable_path)
	@rm -rf $^
	
%.o: %.cpp
	$(CXX) -c $< $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) $(INC_FLAGS) $(LIB_FLAGS)
	
.PHONY: clean
clean: 
	@rm -rf $(executable_path)/$(program) $(objects)

