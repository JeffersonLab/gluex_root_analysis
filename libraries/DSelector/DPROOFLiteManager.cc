#include "DPROOFLiteManager.h"

string gPROOFLiteSandbox = "";

/************************************************************ PROCESS ************************************************************/

bool DPROOFLiteManager::Process_Tree(string locInputFileName, string locTreeName, string locSelectorName, string locOutputFileName, string locOutputTreeFileName, string locOptions, unsigned int locNumThreads)
{
	TChain* locChain = new TChain(locTreeName.c_str());
	locChain->Add(locInputFileName.c_str());
	return Process_Chain(locChain, locSelectorName, locOutputFileName, locOutputTreeFileName, locOptions, locNumThreads);
}

bool DPROOFLiteManager::Process_Tree(string locInputFileName, string locTreeName, string locSelectorName, string locOutputFileName, string locOutputTreeFileName, unsigned int locNumThreads)
{
	return Process_Tree(locInputFileName, locTreeName, locSelectorName, locOutputFileName, locOutputTreeFileName, "", locNumThreads);
}

bool DPROOFLiteManager::Process_Tree(string locInputFileName, string locTreeName, string locSelectorName, string locOutputFileName, unsigned int locNumThreads)
{
	return Process_Tree(locInputFileName, locTreeName, locSelectorName, locOutputFileName, "", "", locNumThreads);
}

bool DPROOFLiteManager::Process_Chain(TChain* locChain, string locSelectorName, string locOutputFileName, string locOutputTreeFileName, string locOptions, unsigned int locNumThreads)
{
	string locPackageName = Get_PackagePath();
	Setup_PROOFSession(locPackageName, "", locOutputFileName, locOutputTreeFileName, locOptions, locNumThreads);
	return Process_Chain(locChain, locSelectorName);
}

bool DPROOFLiteManager::Process_Chain(TChain* locChain, string locSelectorName, string locOutputFileName, string locOutputTreeFileName, unsigned int locNumThreads)
{
	return Process_Chain(locChain, locSelectorName, locOutputFileName, locOutputTreeFileName, "", locNumThreads);
}

bool DPROOFLiteManager::Process_Chain(TChain* locChain, string locSelectorName, string locOutputFileName, unsigned int locNumThreads)
{
	return Process_Chain(locChain, locSelectorName, locOutputFileName, "", "", locNumThreads);
}

bool DPROOFLiteManager::Process_Other(string locSelectorName, string locInputFileName, string locOutputFileName, string locOutputTreeFileName, string locOptions, unsigned int locNumThreads, unsigned int locNumEntries)
{
	string locPackageName = Get_PackagePath();
	TProof* locPROOF = Setup_PROOFSession(locPackageName, locInputFileName, locOutputFileName, locOutputTreeFileName, locOptions, locNumThreads);
	Long64_t locStatus = locPROOF->Process(locSelectorName.c_str(), locNumEntries, "");
	return (locStatus >= Long64_t(0)); //failed if -1
}

/******************************************************* UTILITY FUNCTIONS *******************************************************/

void DPROOFLiteManager::Set_SandBox(string locPath)
{
	gPROOFLiteSandbox = locPath;
}

string DPROOFLiteManager::Get_PackagePath(void)
{
	string locAnalysisHome = gSystem->Getenv("ROOT_ANALYSIS_HOME");
	string locOSName = gSystem->Getenv("BMS_OSNAME");
	return locAnalysisHome + string("/") + locOSName + string("/packages/DSelector.par");
}

TProof* DPROOFLiteManager::Setup_PROOFSession(string locPackageName, string locInputFileName, string locOutputFileName, string locOutputTreeFileName, string locOptions, unsigned int locNumThreads)
{
	locInputFileName = Setup_Path(locInputFileName);
	locOutputFileName = Setup_Path(locOutputFileName);
	locOutputTreeFileName = Setup_Path(locOutputTreeFileName);

	//add FLAT_TREE_FILENAME
	TObjArray* locSessionVariables = new TObjArray(); //use name/title as key/value
	if(locInputFileName != "")
		locSessionVariables->AddLast(new TNamed("INPUT_FILENAME", locInputFileName.c_str()));
	if(locOutputFileName != "")
		locSessionVariables->AddLast(new TNamed("OUTPUT_FILENAME", locOutputFileName.c_str()));
	if(locOutputTreeFileName != "")
		locSessionVariables->AddLast(new TNamed("OUTPUT_TREE_FILENAME", locOutputTreeFileName.c_str()));
	locSessionVariables->AddLast(new TNamed("OPTIONS", locOptions.c_str()));

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

	locPROOF->ClearInput();
	locPROOF->ClearPackages();
	locPROOF->ClearInputData();

	for(size_t loc_i = 0; loc_i < locPackageNames.size(); ++loc_i)
	{
		locPROOF->UploadPackage(locPackageNames[loc_i].c_str());
		locPROOF->EnablePackage(locPackageNames[loc_i].c_str());
	}
   
	for(int loc_j = 0; loc_j < locInputObjects->GetEntriesFast(); ++loc_j)
		locPROOF->AddInput(locInputObjects->At(loc_j));

	return locPROOF;
}

bool DPROOFLiteManager::Process_Chain(TChain* locChain, string locSelectorName)
{
	//uses gProof!
	locChain->SetProof();
	Long64_t locStatus = locChain->Process(locSelectorName.c_str(), "", locChain->GetEntries()); //process this TSelector
	locChain->SetProof(0); //detach from PROOF session
	return (locStatus >= Long64_t(0)); //failed if -1
}
