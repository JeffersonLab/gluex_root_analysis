#ifndef DPROOFLiteManager_h
#define DPROOFLiteManager_h

#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <iostream>

#include <TSystem.h>
#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>
#include <TProof.h>
#include <TTree.h>
#include <TEnv.h>
#include <TNamed.h>
#include <TObjArray.h>

using namespace std;

class DPROOFLiteManager
{
	public:
		static void Set_SandBox(string locPath);

		//SIMPLE INTERFACE //only for using DSelector's
		static bool Process_Tree(string locInputFileName, string locTreeName, string locSelectorName, unsigned int locNumThreads, string locOutputFileName = "", string locOutputTreeFileName = "", string locOptions = "");
		static bool Process_Chain(TChain* locChain, string locSelectorName, unsigned int locNumThreads, string locOutputFileName = "", string locOutputTreeFileName = "", string locOptions = "");
		static bool Process_Other(string locSelectorName, string locInputFileName, unsigned int locNumThreads, unsigned int locNumEntries, string locOutputFileName = "", string locOutputTreeFileName = "", string locOptions = "");

		//FULL INTERFACE //For any use
		//Create PROOF
		static TProof* Setup_PROOFSession(unsigned int locNumThreads, vector<string> locPackageNames = vector<string>(), TObjArray* locInputObjects = NULL);
		//Use PROOF
		static bool Process_Chain(TChain* locChain, string locSelectorName); //Uses gProof: Create it first!

	private:

		//UTILITY FUNCTIONS
		static TProof* Setup_PROOFSession(string locPackageName, string locInputFileName, string locOutputFileName, string locOutputTreeFileName, string locOptions, unsigned int locNumThreads);
		static string Setup_Path(string locInputPath);
		static string Get_PackagePath(void);
};

#endif //DPROOFLiteManager_h
