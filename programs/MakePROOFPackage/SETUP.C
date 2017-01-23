Int_t SETUP()
{
	gSystem->Load("./libDSelector.so");
	gSystem->AddIncludePath("-I.");
	gSystem->AddIncludePath("-I${HALLD_HOME}/${BMS_OSNAME}/include/");
	gSystem->AddIncludePath("-I${ROOT_ANALYSIS_HOME}/${BMS_OSNAME}/include/");
	std::cout << "hello?" << std::endl;
	std::cout << "pwd: " << gSystem->pwd() << std::endl;
	std::cout << "INCLUDE PATH: " << gSystem->GetIncludePath() << std::endl;
	return 0;
}
