#! gnumake
# DOCUMENTATION: https://root.cern.ch/root/htmldoc/guides/users-guide/ROOTUsersGuideChapters/AddingaClass.pdf

C++ = g++

INCLUDE = -I. -I$(HALLD_HOME)/$(BMS_OSNAME)/include
ROOTFLAGS = $(shell $(ROOTSYS)/bin/root-config --cflags)
CXXFLAGS = -O -Wall -fPIC
FLAGS = $(CXXFLAGS) $(ROOTFLAGS) $(INCLUDE)

ROOTGLIBS = $(shell $(ROOTSYS)/bin/root-config --glibs)
LIBS = $(ROOTGLIBS) -L$(ROOTSYS)/lib/ -lProof -L/usr/lib64/ -lstdc++

HFILES = DBeamParticle.h DChargedTrackHypothesis.h DKinematicData.h DMCThrown.h DNeutralParticleHypothesis.h DParticleCombo.h DParticleComboStep.h DSelector.h DTreeInterface.h DPROOFLiteManager.h DSelectorLinkDef.h
CCFILES = $(wildcard *.cc) $(OS_NAME)/obj/DSelectorDict.cc

IS_DARWIN = $(shell echo $OS_NAME | grep -c Darwin)

ifneq ("$(wildcard $(ROOTSYS)/bin/rootcling)","")
	IS_CLING = 1
else
	IS_CLING = 0
endif

ifeq ($(IS_DARWIN), 1)
	SLIB_OPTION = -dynamiclib
else
	SLIB_OPTION = -shared
endif

all: install

install: build_it install_it

build_it: mkdir_build $(OS_NAME)/lib/libDSelector.so

mkdir_build:
	@mkdir -p $(OS_NAME)/lib
	@mkdir -p $(OS_NAME)/obj

install_it:
	@mkdir -p $(ROOT_ANALYSIS_HOME)/$(OS_NAME)/lib
	@mkdir -p $(ROOT_ANALYSIS_HOME)/$(OS_NAME)/include
	@mkdir -p $(ROOT_ANALYSIS_HOME)/$(OS_NAME)/include/DSelector
	@cp $(OS_NAME)/lib/libDSelector.so $(ROOT_ANALYSIS_HOME)/$(OS_NAME)/lib/
	@cp *.h $(ROOT_ANALYSIS_HOME)/$(OS_NAME)/include/DSelector/
ifeq ($(IS_CLING), 1)
	@cp $(OS_NAME)/obj/DSelectorDict_rdict.pcm $(ROOT_ANALYSIS_HOME)/$(OS_NAME)/lib/
	@cp $(OS_NAME)/obj/DSelectorDict.cc $(ROOT_ANALYSIS_HOME)/$(OS_NAME)/lib/
endif

$(OS_NAME)/obj/DSelectorDict.cc: $(HFILES)
	@echo "Generating dictionary DSelectorDict.cc ..."
	@rm -rf $(OS_NAME)/obj/DSelectorDict.cc
ifeq ($(IS_CLING), 1)
	@rm -rf $(OS_NAME)/obj/DSelectorDict_rdict.pcm 
	rootcling $@ -c $(INCLUDE) $^
else
	rootcint -f $@ -c $(INCLUDE) $^
endif
	@echo "done."

$(OS_NAME)/lib/libDSelector.so: $(CCFILES)
	@echo "Linking libDSelector.so ..."
	@rm -f $(OS_NAME)/lib/libDSelector.so
	$(C++) $(FLAGS) $(SLIB_OPTION) $(CCFILES) -o $(OS_NAME)/lib/libDSelector.so $(LIBS)
	@chmod 644 $(OS_NAME)/lib/libDSelector.so

clean:
	@rm -rf $(OS_NAME)
	@rm -rf $(ROOT_ANALYSIS_HOME)/$(OS_NAME)/lib/libDSelector.so
ifeq ($(IS_CLING), 1)
	@rm -rf $(ROOT_ANALYSIS_HOME)/$(OS_NAME)/obj/DSelectorDict_rdict.pcm 
	@rm -rf $(ROOT_ANALYSIS_HOME)/$(OS_NAME)/obj/DSelectorDict.cc
endif

env:
	@echo C++ $(C++)
	@echo OS_NAME $(OS_NAME)
	@echo INCLUDE $(INCLUDE)
	@echo FLAGS $(FLAGS)
	@echo LIBS $(LIBS)
	@echo IS_DARWIN $(IS_DARWIN)
	@echo SLIB_OPTION $(SLIB_OPTION)
	@echo IS_CLING $(IS_CLING)
