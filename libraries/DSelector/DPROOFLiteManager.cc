#include "DPROOFLiteManager.h"

string gPROOFLiteSandbox = "";

/************************************************************ PROCESS ************************************************************/

void DPROOFLiteManager::Process_Tree(string locInputFileName, string locTreeName, string locSelectorName, string locOutputFileName, unsigned int locNumThreads)
{
	TChain* locChain = new TChain(locTreeName.c_str());
	locChain->Add(locInputFileName.c_str());
	Process_Chain(locChain, locSelectorName, locOutputFileName, locNumThreads);
}

void DPROOFLiteManager::Process_Chain(TChain* locChain, string locSelectorName, string locOutputFileName, unsigned int locNumThreads)
{
	string locPackageName = Get_PackagePath();
	Setup_PROOFSession(locPackageName, "", locOutputFileName, locNumThreads);
	Process_Chain(locChain, locSelectorName);
}

void DPROOFLiteManager::Process_Other(string locSelectorName, string locInputFileName, string locOutputFileName, unsigned int locNumThreads, unsigned int locNumEntries)
{
	string locPackageName = Get_PackagePath();
	TProof* locPROOF = Setup_PROOFSession(locPackageName, locInputFileName, locOutputFileName, locNumThreads);
	locPROOF->Process(locSelectorName.c_str(), locNumEntries, "");
}

/******************************************************* UTILITY FUNCTIONS *******************************************************/

void DPROOFLiteManager::Set_SandBox(string locPath)
{
	gPROOFLiteSandbox = locPath;
}

string DPROOFLiteManager::Get_PackagePath(void)
{
	string locAnalysisHome = gSystem->Getenv("ROOT_ANALYSIS_HOME");
	string locOSName = gSystem->Getenv("OS_NAME");
	return locAnalysisHome + string("/") + locOSName + string("/packages/DSelector.par");
}

TProof* DPROOFLiteManager::Setup_PROOFSession(string locPackageName, string locInputFileName, string locOutputFileName, unsigned int locNumThreads)
{
	locInputFileName = Setup_Path(locInputFileName);
	locOutputFileName = Setup_Path(locOutputFileName);

	TObjArray* locSessionVariables = new TObjArray(); //use name/title as key/value
	locSessionVariables->AddLast(new TNamed("INPUT_FILENAME", locInputFileName.c_str()));
	locSessionVariables->AddLast(new TNamed("OUTPUT_FILENAME", locOutputFileName.c_str()));

	return Setup_PROOFSession(locNumThreads, vector<string>(1, locPackageName), locSessionVariables);
}

string DPROOFLiteManager::Setup_Path(string locInputPath)
{
	if(locInputPath.empty())
		return locInputPath;

	if(locInputPath.find_first_of("/") == 0)
		return locInputPath;

	//relative path, pre-pend current directory
	string locDirPath = string(gSystem->pwd()) + string("/");
	locInputPath.insert(0, locDirPath);
	return locInputPath;
}

/******************************************************** FULL INTERFACE *********************************************************/

TProof* DPROOFLiteManager::Setup_PROOFSession(unsigned int locNumThreads, vector<string> locPackageNames, TObjArray* locInputObjects)
{
	ostringstream locNumWorkers;
	locNumWorkers << "workers=" << locNumThreads;

	if(gPROOFLiteSandbox != "")
		gEnv->SetValue("ProofLite.Sandbox", gPROOFLiteSandbox.c_str());
	TProof* locPROOF = TProof::Open(locNumWorkers.str().c_str());

	for(size_t loc_i = 0; loc_i < locPackageNames.size(); ++loc_i)
	{
		locPROOF->UploadPackage(locPackageNames[loc_i].c_str());
		locPROOF->EnablePackage(locPackageNames[loc_i].c_str());
	}

	for(int loc_j = 0; loc_j < locInputObjects->GetEntriesFast(); ++loc_j)
		locPROOF->AddInput(locInputObjects->At(loc_j));

	return locPROOF;
}

void DPROOFLiteManager::Process_Chain(TChain* locChain, string locSelectorName)
{
	//uses gProof!
	locChain->SetProof();
	locChain->Process(locSelectorName.c_str(), "", locChain->GetEntries()); //process this TSelector
	locChain->SetProof(0); //detach from PROOF session
}
