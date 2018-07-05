#! gnumake
# DOCUMENTATION: https://root.cern.ch/root/htmldoc/guides/users-guide/ROOTUsersGuideChapters/AddingaClass.pdf

INCLUDE = -I. -I$(HALLD_RECON_HOME)/$(BMS_OSNAME)/include
ROOTFLAGS = $(shell $(ROOTSYS)/bin/root-config --cflags)
CXXFLAGS = -O -Wall -fPIC
FLAGS = $(CXXFLAGS) $(ROOTFLAGS) $(INCLUDE)

ROOTGLIBS = $(shell $(ROOTSYS)/bin/root-config --glibs)
LIBS = $(ROOTGLIBS) -L$(ROOTSYS)/lib/ -lProof

HFILES = DBeamParticle.h DChargedTrackHypothesis.h DKinematicData.h DMCThrown.h DNeutralParticleHypothesis.h DParticleCombo.h DParticleComboStep.h DAnalysisUtilities.h DAnalysisAction.h DHistogramActions.h DCutActions.h DSelector.h DTreeInterface.h DPROOFLiteManager.h DSelectorLinkDef.h
CCFILES = $(wildcard *.cc) $(BMS_OSNAME)/obj/DSelectorDict.cc

IS_DARWIN = $(shell echo $BMS_OSNAME | grep -c Darwin)

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

build_it: mkdir_build $(BMS_OSNAME)/lib/libDSelector.so

mkdir_build:
	@mkdir -p $(BMS_OSNAME)/lib
	@mkdir -p $(BMS_OSNAME)/obj

install_it:
	@mkdir -p $(ROOT_ANALYSIS_HOME)/$(BMS_OSNAME)/lib
	@mkdir -p $(ROOT_ANALYSIS_HOME)/$(BMS_OSNAME)/include
	@mkdir -p $(ROOT_ANALYSIS_HOME)/$(BMS_OSNAME)/include/DSelector
	@cp $(BMS_OSNAME)/lib/libDSelector.so $(ROOT_ANALYSIS_HOME)/$(BMS_OSNAME)/lib/
	@cp *.h $(ROOT_ANALYSIS_HOME)/$(BMS_OSNAME)/include/DSelector/
ifeq ($(IS_CLING), 1)
	@cp $(BMS_OSNAME)/obj/DSelectorDict_rdict.pcm $(ROOT_ANALYSIS_HOME)/$(BMS_OSNAME)/lib/
	@cp $(BMS_OSNAME)/obj/DSelectorDict.cc $(ROOT_ANALYSIS_HOME)/$(BMS_OSNAME)/lib/
endif

$(BMS_OSNAME)/obj/DSelectorDict.cc: $(HFILES)
	@echo "Generating dictionary DSelectorDict.cc ..."
	@rm -rf $(BMS_OSNAME)/obj/DSelectorDict.cc
ifeq ($(IS_CLING), 1)
	@rm -rf $(BMS_OSNAME)/obj/DSelectorDict_rdict.pcm 
	rootcling $@ -c $(INCLUDE) $^
else
	rootcint -f $@ -c $(INCLUDE) $^
endif
	@echo "done."

$(BMS_OSNAME)/lib/libDSelector.so: $(CCFILES)
	@echo "Linking libDSelector.so ..."
	@rm -f $(BMS_OSNAME)/lib/libDSelector.so
	g++ $(FLAGS) $(SLIB_OPTION) $(CCFILES) -o $(BMS_OSNAME)/lib/libDSelector.so $(LIBS)
	@chmod 644 $(BMS_OSNAME)/lib/libDSelector.so

clean:
	@rm -rf $(BMS_OSNAME)
	@rm -rf $(ROOT_ANALYSIS_HOME)/$(BMS_OSNAME)/lib/libDSelector.so
ifeq ($(IS_CLING), 1)
	@rm -rf $(ROOT_ANALYSIS_HOME)/$(BMS_OSNAME)/obj/DSelectorDict_rdict.pcm 
	@rm -rf $(ROOT_ANALYSIS_HOME)/$(BMS_OSNAME)/obj/DSelectorDict.cc
endif

env:
	@echo BMS_OSNAME $(BMS_OSNAME)
	@echo INCLUDE $(INCLUDE)
	@echo FLAGS $(FLAGS)
	@echo LIBS $(LIBS)
	@echo IS_DARWIN $(IS_DARWIN)
	@echo SLIB_OPTION $(SLIB_OPTION)
	@echo IS_CLING $(IS_CLING)
