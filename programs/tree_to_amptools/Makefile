#! gnumake

INCLUDE = -I. -I$(ROOT_ANALYSIS_HOME)/$(BMS_OSNAME)/include -I$(HALLD_RECON_HOME)/$(BMS_OSNAME)/include
ROOTFLAGS = $(shell $(ROOTSYS)/bin/root-config --cflags)
CXXFLAGS = -O -Wall -fPIC
FLAGS = $(CXXFLAGS) $(ROOTFLAGS) $(INCLUDE)

ROOTGLIBS = $(shell $(ROOTSYS)/bin/root-config --glibs)
LIBS = $(ROOTGLIBS)

all: install

install: build_it install_it

build_it: mkdir_build $(BMS_OSNAME)/bin/tree_to_amptools

mkdir_build:
	@mkdir -p $(BMS_OSNAME)/bin

install_it:
	@mkdir -p $(ROOT_ANALYSIS_HOME)/$(BMS_OSNAME)/bin
	@cp $(BMS_OSNAME)/bin/tree_to_amptools $(ROOT_ANALYSIS_HOME)/$(BMS_OSNAME)/bin/

$(BMS_OSNAME)/bin/tree_to_amptools: tree_to_amptools.cc
	@rm -f $(BMS_OSNAME)/bin/tree_to_amptools
	g++ $(FLAGS) tree_to_amptools.cc -o $(BMS_OSNAME)/bin/tree_to_amptools $(LIBS) 
	@chmod 755 $(BMS_OSNAME)/bin/tree_to_amptools

clean:
	@rm -rf $(BMS_OSNAME)
	@rm -rf $(ROOT_ANALYSIS_HOME)/$(BMS_OSNAME)/bin/tree_to_amptools

env:
	@echo BMS_OSNAME $(BMS_OSNAME)
	@echo INCLUDE $(INCLUDE)
	@echo FLAGS $(FLAGS)
	@echo LIBS $(LIBS)
