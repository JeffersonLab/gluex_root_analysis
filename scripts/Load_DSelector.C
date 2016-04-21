{
	gSystem->Load("$(ROOT_ANALYSIS_HOME)/$(OS_NAME)/lib/libDSelector.so");
	gSystem->AddIncludePath("-I${ROOT_ANALYSIS_HOME}/${OS_NAME}/include/");
	gSystem->AddIncludePath("-I${HALLD_HOME}/${BMS_OSNAME}/include/");
}
