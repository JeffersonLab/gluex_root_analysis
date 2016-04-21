#! gnumake

C++ = g++

INCLUDE = -I. -I$(ROOT_ANALYSIS_HOME)/$(OS_NAME)/include -I$(HALLD_HOME)/$(BMS_OSNAME)/include
ROOTFLAGS = $(shell $(ROOTSYS)/bin/root-config --cflags)
CXXFLAGS = -O -Wall -fPIC
FLAGS = $(CXXFLAGS) $(ROOTFLAGS) $(INCLUDE)

ROOTGLIBS = $(shell $(ROOTSYS)/bin/root-config --glibs)
LIBS = $(ROOTGLIBS) -L/usr/lib64/ -lstdc++ -L$(ROOT_ANALYSIS_HOME)/$(OS_NAME)/lib/ -lDSelector

all: install

install: build_it install_it

build_it: mkdir_build $(OS_NAME)/bin/MakeDSelector

mkdir_build:
	@mkdir -p $(OS_NAME)/bin

install_it:
	@mkdir -p $(ROOT_ANALYSIS_HOME)/$(OS_NAME)/bin
	@cp $(OS_NAME)/bin/MakeDSelector $(ROOT_ANALYSIS_HOME)/$(OS_NAME)/bin/

$(OS_NAME)/bin/MakeDSelector: MakeDSelector.cc $(ROOT_ANALYSIS_HOME)/$(OS_NAME)/lib/libDSelector.so
	@rm -f $(OS_NAME)/bin/MakeDSelector
	$(C++) $(FLAGS) MakeDSelector.cc -o $(OS_NAME)/bin/MakeDSelector $(LIBS) 
	@chmod 755 $(OS_NAME)/bin/MakeDSelector

clean:
	@rm -rf $(OS_NAME)
	@rm -rf $(ROOT_ANALYSIS_HOME)/$(OS_NAME)/bin/MakeDSelector

env:
	@echo C++ $(C++)
	@echo OS_NAME $(OS_NAME)
	@echo INCLUDE $(INCLUDE)
	@echo FLAGS $(FLAGS)
	@echo LIBS $(LIBS)
