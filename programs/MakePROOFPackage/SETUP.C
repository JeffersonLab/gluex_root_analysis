Int_t SETUP()
{
	gSystem->Load("./libDSelector.so");
	gSystem->AddIncludePath("-I.");
	gSystem->AddIncludePath("-I${HALLD_RECON_HOME}/${BMS_OSNAME}/include/");
	gSystem->AddIncludePath("-I${ROOT_ANALYSIS_HOME}/${BMS_OSNAME}/include/");
	return 0;
}
