#include <string>
#include <map>
#include <iostream>
#include <fstream>

#include <TFile.h>
#include <TTree.h>

#include "DSelector/DTreeInterface.h"

//function declarations
void Print_Usage(void);
void Print_HeaderFile(string locSelectorBaseName, DTreeInterface* locTreeInterface, map<int, map<int, pair<Particle_t, string> > >& locComboInfoMap);
void Print_SourceFile(string locSelectorBaseName, DTreeInterface* locTreeInterface, map<int, map<int, pair<Particle_t, string> > >& locComboInfoMap);

int main(int argc, char* argv[])
{
	if(argc != 4)
	{
		Print_Usage();
		return 0;
	}

	string locInputFileName = argv[1];
	string locTreeName = argv[2];
	string locSelectorBaseName = argv[3];

	TFile* locInputFile = new TFile(locInputFileName.c_str(), "READ");
	TTree* locTree = (TTree*)locInputFile->Get(locTreeName.c_str());

	DTreeInterface* locTreeInterface = new DTreeInterface(locTree);

	//get combo info
	map<int, map<int, pair<Particle_t, string> > > locComboInfoMap;
	locTreeInterface->Get_ComboInfo(locComboInfoMap);

	Print_HeaderFile(locSelectorBaseName, locTreeInterface, locComboInfoMap);
	Print_SourceFile(locSelectorBaseName, locTreeInterface, locComboInfoMap);

	cout << "Selector files " << locSelectorName << ".* generated." << endl;

	return 0;
}

void Print_Usage(void)
{
	cout << endl;
	cout << "Makes a custom DSelector for the input TTree created by the DANA ANALYSIS library." << endl;
	cout << "1st argument: The input ROOT TTree file name." << endl;
	cout << "2nd argument: The name of the TTree in the input ROOT file that you want to make a selector for." << endl;
	cout << "3rd argument: The base-name of the selector class & files you want to generate." << endl;
	cout << "The generated files will be named \"DSelector_<base-name>.C\" and \"DSelector_<base-name>.h\"." << endl;
	cout << endl;
}

void Print_HeaderFile(string locSelectorBaseName, DTreeInterface* locTreeInterface, map<int, map<int, pair<Particle_t, string> > >& locComboInfoMap)
{
	string locSelectorName = string("DSelector_") + locSelectorBaseName;
	string locHeaderName = locSelectorName + string(".h");
	ofstream locHeaderStream;
	locHeaderStream.open(locHeaderName.c_str());

	locHeaderStream << "#ifndef " << locSelectorName << "_h" << endl;
	locHeaderStream << "#define " << locSelectorName << "_h" << endl;
	locHeaderStream << endl;
	locHeaderStream << "#include <iostream>" << endl;
	locHeaderStream << endl;
	locHeaderStream << "#include \"DSelector/DSelector.h\"" << endl;
	locHeaderStream << "#include \"DSelector/DHistogramActions.h\"" << endl;
	locHeaderStream << "#include \"DSelector/DCutActions.h\"" << endl;
	locHeaderStream << endl;
	locHeaderStream << "#include \"TH1I.h\"" << endl;
	locHeaderStream << "#include \"TH2I.h\"" << endl;
	locHeaderStream << endl;
	locHeaderStream << "class " << locSelectorName << " : public DSelector" << endl;
	locHeaderStream << "{" << endl;
	locHeaderStream << "	public:" << endl;
	locHeaderStream << endl;
	locHeaderStream << "		" << locSelectorName << "(TTree* locTree = NULL) : DSelector(locTree){}" << endl;
	locHeaderStream << "		virtual ~" << locSelectorName << "(){}" << endl;
	locHeaderStream << endl;
	locHeaderStream << "		void Init(TTree *tree);" << endl;
	locHeaderStream << "		Bool_t Process(Long64_t entry);" << endl;
	locHeaderStream << endl;
	locHeaderStream << "	private:" << endl;
	locHeaderStream << endl;
	locHeaderStream << "		void Get_ComboWrappers(void);" << endl;
	locHeaderStream << "		void Finalize(void);" << endl;
	locHeaderStream << endl;
	locHeaderStream << "		// BEAM POLARIZATION INFORMATION" << endl;
	locHeaderStream << "		UInt_t dPreviousRunNumber;" << endl;
	locHeaderStream << "		bool dIsPolarizedFlag; //else is AMO" << endl;
	locHeaderStream << "		bool dIsPARAFlag; //else is PERP or AMO" << endl;
	locHeaderStream << endl;
	locHeaderStream << "		//CREATE REACTION-SPECIFIC PARTICLE ARRAYS" << endl;
	locHeaderStream << endl;

	//print particle step, particle wrapper declarations
	map<int, map<int, pair<Particle_t, string> > >::iterator locStepIterator = locComboInfoMap.begin();
	for(; locStepIterator != locComboInfoMap.end(); ++locStepIterator)
	{
		int locStepIndex = locStepIterator->first;
		locHeaderStream << "		//Step " << locStepIndex << endl;
		locHeaderStream << "		DParticleComboStep* dStep" << locStepIndex << "Wrapper;" << endl;

		map<int, pair<Particle_t, string> >& locStepInfoMap = locStepIterator->second;
		map<int, pair<Particle_t, string> >::iterator locParticleIterator = locStepInfoMap.begin();
		for(; locParticleIterator != locStepInfoMap.end(); ++locParticleIterator)
		{
			int locParticleIndex = locParticleIterator->first;
			Particle_t locPID = locParticleIterator->second.first;
			string locParticleName = locParticleIterator->second.second;

			if(locPID == Unknown)
				continue;
			else if(locParticleName == "ComboBeam")
				locHeaderStream << "		DBeamParticle* dComboBeamWrapper;" << endl;
			else if(locParticleName.substr(0, 6) == "Target")
				continue;
			else if(locParticleName.substr(0, 8) == "Decaying")
			{
				string locBranchName = locParticleName + string("__P4_KinFit");
				if((locTreeInterface->Get_Branch(locBranchName) != NULL) && (locParticleIndex < 0)) //else not reconstructed or in final state
					locHeaderStream << "		DKinematicData* d" << locParticleName << "Wrapper;" << endl;
			}
			else if(locParticleName.substr(0, 7) == "Missing")
			{
				string locBranchName = locParticleName + string("__P4_KinFit");
				if(locTreeInterface->Get_Branch(locBranchName) != NULL) //else not reconstructed
					locHeaderStream << "		DKinematicData* d" << locParticleName << "Wrapper;" << endl;
			}
			else if(ParticleCharge(locPID) != 0)
				locHeaderStream << "		DChargedTrackHypothesis* d" << locParticleName << "Wrapper;" << endl;
			else
				locHeaderStream << "		DNeutralParticleHypothesis* d" << locParticleName << "Wrapper;" << endl;
		}
		locHeaderStream << endl;
	}

	//resume
	locHeaderStream << "		// DEFINE YOUR HISTOGRAM ACTIONS HERE" << endl;
	locHeaderStream << "		// EXAMPLE HISTOGRAM ACTIONS:" << endl;
	locHeaderStream << "		DHistogramAction_ParticleComboKinematics* dHistComboKinematics;" << endl;
	locHeaderStream << "		DHistogramAction_ParticleID* dHistComboPID;" << endl;
	locHeaderStream << "		// EXAMPLE CUT ACTIONS:" << endl;
	locHeaderStream << "		DCutAction_PIDDeltaT* dCutPIDDeltaT;" << endl;
	locHeaderStream << endl;
	locHeaderStream << "		// DEFINE YOUR HISTOGRAMS HERE" << endl;
	locHeaderStream << "		// EXAMPLES:" << endl;
	locHeaderStream << "		TH1I* dHist_MissingMassSquared;" << endl;
	locHeaderStream << "		TH1I* dHist_BeamEnergy;" << endl;
	locHeaderStream << endl;
	locHeaderStream << "	ClassDef(" << locSelectorName << ", 0);" << endl;
	locHeaderStream << "};" << endl;
	locHeaderStream << endl;
	locHeaderStream << "void " << locSelectorName << "::Get_ComboWrappers(void)" << endl;
	locHeaderStream << "{" << endl;

	//print particle step, particle wrapper assignments
	for(locStepIterator = locComboInfoMap.begin(); locStepIterator != locComboInfoMap.end(); ++locStepIterator)
	{
		int locStepIndex = locStepIterator->first;
		if(locStepIndex != 0)
			locHeaderStream << endl;
		locHeaderStream << "	//Step " << locStepIndex << endl;
		locHeaderStream << "	dStep" << locStepIndex << "Wrapper = dComboWrapper->Get_ParticleComboStep(" << locStepIndex << ");" << endl;

		map<int, pair<Particle_t, string> >& locStepInfoMap = locStepIterator->second;
		map<int, pair<Particle_t, string> >::iterator locParticleIterator = locStepInfoMap.begin();
		for(; locParticleIterator != locStepInfoMap.end(); ++locParticleIterator)
		{
			int locParticleIndex = locParticleIterator->first;
			Particle_t locPID = locParticleIterator->second.first;
			string locParticleName = locParticleIterator->second.second;

			if(locPID == Unknown)
				continue;
			else if(locParticleName == "ComboBeam")
				locHeaderStream << "	dComboBeamWrapper = static_cast<DBeamParticle*>(dStep" << locStepIndex << "Wrapper->Get_InitialParticle());" << endl;
			else if(locParticleName.substr(0, 6) == "Target")
				continue;
			else if(locParticleName.substr(0, 8) == "Decaying")
			{
				string locBranchName = locParticleName + string("__P4_KinFit");
				if((locTreeInterface->Get_Branch(locBranchName) != NULL) && (locParticleIndex < 0)) //else not reconstructed
					locHeaderStream << "	d" << locParticleName << "Wrapper = dStep" << locStepIndex << "Wrapper->Get_InitialParticle();" << endl;
			}
			else if(locParticleName.substr(0, 7) == "Missing")
			{
				string locBranchName = locParticleName + string("__P4_KinFit");
				if(locTreeInterface->Get_Branch(locBranchName) != NULL) //else not reconstructed
					locHeaderStream << "	d" << locParticleName << "Wrapper = dStep" << locStepIndex << "Wrapper->Get_FinalParticle(" << locParticleIndex << ");" << endl;
			}
			else if(ParticleCharge(locPID) != 0)
				locHeaderStream << "	d" << locParticleName << "Wrapper = static_cast<DChargedTrackHypothesis*>(dStep" << locStepIndex << "Wrapper->Get_FinalParticle(" << locParticleIndex << "));" << endl;
			else
				locHeaderStream << "	d" << locParticleName << "Wrapper = static_cast<DNeutralParticleHypothesis*>(dStep" << locStepIndex << "Wrapper->Get_FinalParticle(" << locParticleIndex << "));" << endl;
		}
	}

	//resume
	locHeaderStream << "}" << endl;
	locHeaderStream << endl;
	locHeaderStream << "#endif // " << locSelectorName << "_h" << endl;

	locHeaderStream.close();
}

void Print_SourceFile(string locSelectorBaseName, DTreeInterface* locTreeInterface, map<int, map<int, pair<Particle_t, string> > >& locComboInfoMap)
{
	string locSelectorName = string("DSelector_") + locSelectorBaseName;
	string locSourceName = locSelectorName + string(".C");
	ofstream locSourceStream;
	locSourceStream.open(locSourceName.c_str());

	locSourceStream << "#include \"" << locSelectorName << ".h\"" << endl;
	locSourceStream << endl;
	locSourceStream << "void " << locSelectorName << "::Init(TTree *locTree)" << endl;
	locSourceStream << "{" << endl;
	locSourceStream << "	// The Init() function is called when the selector needs to initialize a new tree or chain." << endl;
	locSourceStream << "	// Typically here the branch addresses and branch pointers of the tree will be set." << endl;
	locSourceStream << "	// Init() will be called many times when running on PROOF (once per file to be processed)." << endl;
	locSourceStream << endl;
	locSourceStream << "	//SET OUTPUT FILE NAME //can be overriden by user in PROOF" << endl;
	locSourceStream << "	dOutputFileName = \"" << locSelectorBaseName << ".root\"; //\"\" for none" << endl;
	locSourceStream << "	dOutputTreeFileName = \"\"; //\"\" for none" << endl;
	locSourceStream << endl;
	locSourceStream << "	//DO THIS NEXT" << endl;
	locSourceStream << "	//Because this function gets called for each TTree in the TChain, we must be careful:" << endl;
	locSourceStream << "		//We need to re-initialize the tree interface & branch wrappers, but don't want to recreate histograms" << endl;
	locSourceStream << "	bool locInitializedPriorFlag = dInitializedFlag; //save whether have been initialized previously" << endl;
	locSourceStream << "	DSelector::Init(locTree); //This must be called to initialize wrappers for each new TTree" << endl;
	locSourceStream << "	//gDirectory now points to the output file with name dOutputFileName (if any)" << endl;
	locSourceStream << "	if(locInitializedPriorFlag)" << endl;
	locSourceStream << "		return; //have already created histograms, etc. below: exit" << endl;
	locSourceStream << endl;
	locSourceStream << "	//THEN THIS" << endl;
	locSourceStream << "	Get_ComboWrappers();" << endl;
	locSourceStream << "	dPreviousRunNumber = 0;" << endl;
	locSourceStream << endl;
	locSourceStream << "	/******************************************** EXAMPLE USER INITIALIZATION *******************************************/" << endl;
	locSourceStream << endl;
	locSourceStream << "	//DO WHATEVER YOU WANT HERE" << endl;
	locSourceStream << endl;
	locSourceStream << "	//EXAMPLE HISTOGRAM ACTIONS:" << endl;
	locSourceStream << "	dHistComboKinematics = new DHistogramAction_ParticleComboKinematics(dComboWrapper, false); //false: use measured data" << endl;
	locSourceStream << "	dHistComboPID = new DHistogramAction_ParticleID(dComboWrapper, false); //false: use measured data" << endl;
	locSourceStream << "	//change histogram binning here" << endl;
	locSourceStream << endl;
	locSourceStream << "	//INITIALIZE ACTIONS" << endl;
	locSourceStream << "	dHistComboKinematics->Initialize();" << endl;
	locSourceStream << "	dHistComboPID->Initialize();" << endl;
	locSourceStream << endl;
	locSourceStream << "	//EXAMPLE CUT ACTIONS:" << endl;
	locSourceStream << "	//below: false: measured data, value: +/- N ns, Unknown: All PIDs, SYS_NULL: all timing systems" << endl;
	locSourceStream << "	dCutPIDDeltaT = new DCutAction_PIDDeltaT(dComboWrapper, false, 2.0, Unknown, SYS_NULL);" << endl;
	locSourceStream << "	dCutPIDDeltaT->Initialize();" << endl;
	locSourceStream << endl;
	locSourceStream << "	//EXAMPLE MANUAL HISTOGRAMS:" << endl;
	locSourceStream << "	dHist_MissingMassSquared = new TH1I(\"MissingMassSquared\", \";Missing Mass Squared (GeV/c^{2})^{2}\", 600, -0.06, 0.06);" << endl;
	locSourceStream << "	dHist_BeamEnergy = new TH1I(\"BeamEnergy\", \";Beam Energy (GeV)\", 600, 0.0, 12.0);" << endl;
	locSourceStream << endl;
	locSourceStream << "	/***************************************** ADVANCED: CHOOSE BRANCHES TO READ ****************************************/" << endl;
	locSourceStream << endl;
	locSourceStream << "	//TO SAVE PROCESSING TIME" << endl;
	locSourceStream << "		//If you know you don't need all of the branches/data, but just a subset of it, you can speed things up" << endl;
	locSourceStream << "		//By default, for each event, the data is retrieved for all branches" << endl;
	locSourceStream << "		//If you know you only need data for some branches, you can skip grabbing data from the branches you don't need" << endl;
	locSourceStream << "		//Do this by doing something similar to the commented code below" << endl;
	locSourceStream << endl;
	locSourceStream << "	//dTreeInterface->Clear_GetEntryBranches(); //now get none" << endl;
	locSourceStream << "	//dTreeInterface->Register_GetEntryBranch(\"Proton__P4\"); //manually set the branches you want" << endl;
	locSourceStream << "}" << endl;
	locSourceStream << endl;
	locSourceStream << "Bool_t " << locSelectorName << "::Process(Long64_t locEntry)" << endl;
	locSourceStream << "{" << endl;
	locSourceStream << "	// The Process() function is called for each entry in the tree. The entry argument" << endl;
	locSourceStream << "	// specifies which entry in the currently loaded tree is to be processed." << endl;
	locSourceStream << "	//" << endl;
	locSourceStream << "	// This function should contain the \"body\" of the analysis. It can contain" << endl;
	locSourceStream << "	// simple or elaborate selection criteria, run algorithms on the data" << endl;
	locSourceStream << "	// of the event and typically fill histograms." << endl;
	locSourceStream << "	//" << endl;
	locSourceStream << "	// The processing can be stopped by calling Abort()." << endl;
	locSourceStream << "	// Use fStatus to set the return value of TTree::Process()." << endl;
	locSourceStream << "	// The return value is currently not used." << endl;
	locSourceStream << endl;
	locSourceStream << "	//CALL THIS FIRST" << endl;
	locSourceStream << "	DSelector::Process(locEntry); //Gets the data from the tree for the entry" << endl;
	locSourceStream << "	//cout << \"RUN \" << Get_RunNumber() << \", EVENT \" << Get_EventNumber() << endl;" << endl;
	locSourceStream << endl;
	locSourceStream << "	/******************************************** GET POLARIZATION ORIENTATION ******************************************/" << endl;
	locSourceStream << endl;
	locSourceStream << "	//Only if the run number changes" << endl;
	locSourceStream << "	//RCDB environment must be setup in order for this to work! (Will return false otherwise)" << endl;
	locSourceStream << "	UInt_t locRunNumber = Get_RunNumber();" << endl;
	locSourceStream << "	if(locRunNumber != dPreviousRunNumber)" << endl;
	locSourceStream << "	{" << endl;
	locSourceStream << "		dIsPolarizedFlag = dAnalysisUtilities.Get_IsPolarizedBeam(locRunNumber, dIsPARAFlag);" << endl;
	locSourceStream << "		dPreviousRunNumber = locRunNumber;" << endl;
	locSourceStream << "	}" << endl;
	locSourceStream << endl;
	locSourceStream << "	/**************************************** SETUP AUTOMATIC UNIQUENESS TRACKING ***************************************/" << endl;
	locSourceStream << endl;
	locSourceStream << "	//Reset uniqueness tracking for each action" << endl;
	locSourceStream << "	dHistComboKinematics->Reset_NewEvent();" << endl;
	locSourceStream << "	dHistComboPID->Reset_NewEvent();" << endl;
	locSourceStream << endl;
	locSourceStream << "	//INSERT OTHER USER ACTIONS HERE" << endl;
	locSourceStream << endl;
	locSourceStream << "	/***************************************** SETUP MANUAL UNIQUENESS TRACKING *****************************************/" << endl;
	locSourceStream << endl;
	locSourceStream << "	//PREVENT-DOUBLE COUNTING WHEN HISTOGRAMMING" << endl;
	locSourceStream << "		//Sometimes, some content is the exact same between one combo and the next" << endl;
	locSourceStream << "			//e.g. maybe two combos have different beam particles, but the same data for the final-state" << endl;
	locSourceStream << "		//When histogramming, you don\'t want to double-count when this happens: artificially inflates your signal (or background)" << endl;
	locSourceStream << "		//So, for each quantity you histogram, keep track of what particles you used (for a given combo)" << endl;
	locSourceStream << "		//Then for each combo, just compare to what you used before, and make sure it\'s unique" << endl;
	locSourceStream << endl;
	locSourceStream << "	//EXAMPLE 1: Particle-specific info:" << endl;
	locSourceStream << "	set<Int_t> locUsedSoFar_BeamEnergy; //Int_t: Unique ID for beam particles. set: easy to use, fast to search" << endl;
	locSourceStream << endl;
	locSourceStream << "	//EXAMPLE 2: Combo-specific info:" << endl;
	locSourceStream << "		//In general: Could have multiple particles with the same PID: Use a set of Int_t\'s" << endl;
	locSourceStream << "		//In general: Multiple PIDs, so multiple sets: Contain within a map" << endl;
	locSourceStream << "		//Multiple combos: Contain maps within a set (easier, faster to search)" << endl;
	locSourceStream << "	set<map<Particle_t, set<Int_t> > > locUsedSoFar_MissingMass;" << endl;
	locSourceStream << endl;
	locSourceStream << "	//INSERT USER ANALYSIS UNIQUENESS TRACKING HERE" << endl;
	locSourceStream << endl;
	locSourceStream << "	/************************************************* LOOP OVER COMBOS *************************************************/" << endl;
	locSourceStream << endl;
	locSourceStream << "	//Loop over combos" << endl;
	locSourceStream << "	for(UInt_t loc_i = 0; loc_i < Get_NumCombos(); ++loc_i)" << endl;
	locSourceStream << "	{" << endl;
	locSourceStream << "		//Set branch array indices for combo and all combo particles" << endl;
	locSourceStream << "		dComboWrapper->Set_ComboIndex(loc_i);" << endl;
	locSourceStream << endl;
	locSourceStream << "		// Is used to indicate when combos have been cut" << endl;
	locSourceStream << "		if(dComboWrapper->Get_IsComboCut()) // Is false when tree originally created" << endl;
	locSourceStream << "			continue; // Combo has been cut previously" << endl;
	locSourceStream << endl;
	locSourceStream << "		/********************************************** GET PARTICLE INDICES *********************************************/" << endl;
	locSourceStream << endl;
	locSourceStream << "		//Used for tracking uniqueness when filling histograms, and for determining unused particles" << endl;
	locSourceStream << endl;

	//print particle indices
	map<int, map<int, pair<Particle_t, string> > >::iterator locStepIterator = locComboInfoMap.begin();
	for(; locStepIterator != locComboInfoMap.end(); ++locStepIterator)
	{
		int locStepIndex = locStepIterator->first;
		locSourceStream << "		//Step " << locStepIndex << endl;

		map<int, pair<Particle_t, string> >& locStepInfoMap = locStepIterator->second;
		map<int, pair<Particle_t, string> >::iterator locParticleIterator = locStepInfoMap.begin();
		for(; locParticleIterator != locStepInfoMap.end(); ++locParticleIterator)
		{
			Particle_t locPID = locParticleIterator->second.first;
			string locParticleName = locParticleIterator->second.second;

			if(locPID == Unknown)
				continue;
			else if(locParticleName == "ComboBeam")
				locSourceStream << "		Int_t locBeamID = dComboBeamWrapper->Get_BeamID();" << endl;
			else if(locParticleName.substr(0, 6) == "Target")
				continue;
			else if(locParticleName.substr(0, 8) == "Decaying")
				continue;
			else if(locParticleName.substr(0, 7) == "Missing")
				continue;
			else if(ParticleCharge(locPID) != 0)
				locSourceStream << "		Int_t loc" << locParticleName << "TrackID = d" << locParticleName << "Wrapper->Get_TrackID();" << endl;
			else
				locSourceStream << "		Int_t loc" << locParticleName << "NeutralID = d" << locParticleName << "Wrapper->Get_NeutralID();" << endl;
		}
		locSourceStream << endl;
	}
	locSourceStream << "		/*********************************************** GET FOUR-MOMENTUM **********************************************/" << endl;
	locSourceStream << endl;

	//get p4's
	locSourceStream << "		// Get P4\'s: //is kinfit if kinfit performed, else is measured" << endl;
	locSourceStream << "		//dTargetP4 is target p4" << endl;
	for(locStepIterator = locComboInfoMap.begin(); locStepIterator != locComboInfoMap.end(); ++locStepIterator)
	{
		int locStepIndex = locStepIterator->first;
		locSourceStream << "		//Step " << locStepIndex << endl;

		map<int, pair<Particle_t, string> >& locStepInfoMap = locStepIterator->second;
		map<int, pair<Particle_t, string> >::iterator locParticleIterator = locStepInfoMap.begin();
		for(; locParticleIterator != locStepInfoMap.end(); ++locParticleIterator)
		{
			int locParticleIndex = locParticleIterator->first;
			Particle_t locPID = locParticleIterator->second.first;
			string locParticleName = locParticleIterator->second.second;

			if(locPID == Unknown)
				continue;
			else if(locParticleName == "ComboBeam")
				locSourceStream << "		TLorentzVector locBeamP4 = dComboBeamWrapper->Get_P4();" << endl;
			else if(locParticleName.substr(0, 6) == "Target")
				continue;
			else if(locParticleName.substr(0, 8) == "Decaying")
			{
				string locBranchName = locParticleName + string("__P4_KinFit");
				if((locTreeInterface->Get_Branch(locBranchName) != NULL) && (locParticleIndex < 0)) //else not reconstructed
					locSourceStream << "		TLorentzVector loc" << locParticleName << "P4 = d" << locParticleName << "Wrapper->Get_P4();" << endl;
			}
			else if(locParticleName.substr(0, 7) == "Missing")
			{
				string locBranchName = locParticleName + string("__P4_KinFit");
				if(locTreeInterface->Get_Branch(locBranchName) != NULL) //else not reconstructed
					locSourceStream << "		TLorentzVector loc" << locParticleName << "P4 = d" << locParticleName << "Wrapper->Get_P4();" << endl;
			}
			else //detected
				locSourceStream << "		TLorentzVector loc" << locParticleName << "P4 = d" << locParticleName << "Wrapper->Get_P4();" << endl;
		}
	}
	locSourceStream << endl;

	//get measured p4's
	locSourceStream << "		// Get Measured P4\'s:" << endl;
	for(locStepIterator = locComboInfoMap.begin(); locStepIterator != locComboInfoMap.end(); ++locStepIterator)
	{
		int locStepIndex = locStepIterator->first;
		locSourceStream << "		//Step " << locStepIndex << endl;

		map<int, pair<Particle_t, string> >& locStepInfoMap = locStepIterator->second;
		map<int, pair<Particle_t, string> >::iterator locParticleIterator = locStepInfoMap.begin();
		for(; locParticleIterator != locStepInfoMap.end(); ++locParticleIterator)
		{
			Particle_t locPID = locParticleIterator->second.first;
			string locParticleName = locParticleIterator->second.second;

			if(locPID == Unknown)
				continue;
			else if(locParticleName == "ComboBeam")
				locSourceStream << "		TLorentzVector locBeamP4_Measured = dComboBeamWrapper->Get_P4_Measured();" << endl;
			else if(locParticleName.substr(0, 6) == "Target")
				continue;
			else if(locParticleName.substr(0, 8) == "Decaying")
				continue;
			else if(locParticleName.substr(0, 7) == "Missing")
				continue;
			else //detected
				locSourceStream << "		TLorentzVector loc" << locParticleName << "P4_Measured = d" << locParticleName << "Wrapper->Get_P4_Measured();" << endl;
		}
	}

	locSourceStream << endl;
	locSourceStream << "		/********************************************* COMBINE FOUR-MOMENTUM ********************************************/" << endl;
	locSourceStream << endl;
	locSourceStream << "		// DO YOUR STUFF HERE" << endl;
	locSourceStream << endl;
	locSourceStream << "		// Combine 4-vectors" << endl;
	locSourceStream << "		TLorentzVector locMissingP4_Measured = locBeamP4_Measured + dTargetP4;" << endl;
	locSourceStream << "		locMissingP4_Measured -= ";

	//calc missing p4
	bool locFirstFlag = true;
	for(locStepIterator = locComboInfoMap.begin(); locStepIterator != locComboInfoMap.end(); ++locStepIterator)
	{
		map<int, pair<Particle_t, string> >& locStepInfoMap = locStepIterator->second;
		map<int, pair<Particle_t, string> >::iterator locParticleIterator = locStepInfoMap.begin();
		for(; locParticleIterator != locStepInfoMap.end(); ++locParticleIterator)
		{
			Particle_t locPID = locParticleIterator->second.first;
			string locParticleName = locParticleIterator->second.second;

			if(locPID == Unknown)
				continue;
			else if(locParticleName == "ComboBeam")
				continue;
			else if(locParticleName.substr(0, 6) == "Target")
				continue;
			else if(locParticleName.substr(0, 8) == "Decaying")
				continue;
			else if(locParticleName.substr(0, 7) == "Missing")
				continue;

			//detected
			if(!locFirstFlag)
				locSourceStream << " + ";
			locFirstFlag = false;
			locSourceStream << "loc" << locParticleName << "P4_Measured";
		}
	}
	locSourceStream << ";" << endl;
	locSourceStream << endl;
	locSourceStream << "		/**************************************** EXAMPLE: HISTOGRAM KINEMATICS ******************************************/" << endl;
	locSourceStream << endl;
	locSourceStream << "		dHistComboKinematics->Perform_Action();" << endl;
	locSourceStream << "		dHistComboPID->Perform_Action();" << endl;
	locSourceStream << endl;
	locSourceStream << "		/**************************************** EXAMPLE: PID CUT ACTION ************************************************/" << endl;
	locSourceStream << "/*" << endl;
	locSourceStream << "		if(!dCutPIDDeltaT->Perform_Action()) {" << endl;
	locSourceStream << "			dComboWrapper->Set_IsComboCut(true);" << endl;
	locSourceStream << "			continue;" << endl;
	locSourceStream << "		}" << endl;
	locSourceStream << "*/" << endl;
	locSourceStream << "		/**************************************** EXAMPLE: HISTOGRAM BEAM ENERGY *****************************************/" << endl;
	locSourceStream << endl;
	locSourceStream << "		//Histogram beam energy (if haven\'t already)" << endl;
	locSourceStream << "		if(locUsedSoFar_BeamEnergy.find(locBeamID) == locUsedSoFar_BeamEnergy.end())" << endl;
	locSourceStream << "		{" << endl;
	locSourceStream << "			dHist_BeamEnergy->Fill(locBeamP4.E());" << endl;
	locSourceStream << "			locUsedSoFar_BeamEnergy.insert(locBeamID);" << endl;
	locSourceStream << "		}" << endl;
	locSourceStream << endl;
	locSourceStream << "		/************************************ EXAMPLE: HISTOGRAM MISSING MASS SQUARED ************************************/" << endl;
	locSourceStream << endl;
	locSourceStream << "		//Missing Mass Squared" << endl;
	locSourceStream << "		double locMissingMassSquared = locMissingP4_Measured.M2();" << endl;
	locSourceStream << endl;
	locSourceStream << "		//Uniqueness tracking: Build the map of particles used for the missing mass" << endl;
	locSourceStream << "			//For beam: Don\'t want to group with final-state photons. Instead use \"Unknown\" PID (not ideal, but it\'s easy)." << endl;
	locSourceStream << "		map<Particle_t, set<Int_t> > locUsedThisCombo_MissingMass;" << endl;

	//insert uniqueness tracking
	//print particle indices
	for(locStepIterator = locComboInfoMap.begin(); locStepIterator != locComboInfoMap.end(); ++locStepIterator)
	{
		map<int, pair<Particle_t, string> >& locStepInfoMap = locStepIterator->second;
		map<int, pair<Particle_t, string> >::iterator locParticleIterator = locStepInfoMap.begin();
		for(; locParticleIterator != locStepInfoMap.end(); ++locParticleIterator)
		{
			Particle_t locPID = locParticleIterator->second.first;
			string locParticleName = locParticleIterator->second.second;

			if(locPID == Unknown)
				continue;
			else if(locParticleName == "ComboBeam")
				locSourceStream << "		locUsedThisCombo_MissingMass[Unknown].insert(locBeamID); //beam" << endl;
			else if(locParticleName.substr(0, 6) == "Target")
				continue;
			else if(locParticleName.substr(0, 8) == "Decaying")
				continue;
			else if(locParticleName.substr(0, 7) == "Missing")
				continue;
			else if(ParticleCharge(locPID) != 0)
				locSourceStream << "		locUsedThisCombo_MissingMass[" << EnumString(locPID) << "].insert(loc" << locParticleName << "TrackID);" << endl;
			else
				locSourceStream << "		locUsedThisCombo_MissingMass[" << EnumString(locPID) << "].insert(loc" << locParticleName << "NeutralID);" << endl;
		}
	}
	locSourceStream << endl;

	locSourceStream << "		//compare to what\'s been used so far" << endl;
	locSourceStream << "		if(locUsedSoFar_MissingMass.find(locUsedThisCombo_MissingMass) == locUsedSoFar_MissingMass.end())" << endl;
	locSourceStream << "		{" << endl;
	locSourceStream << "			//unique missing mass combo: histogram it, and register this combo of particles" << endl;
	locSourceStream << "			dHist_MissingMassSquared->Fill(locMissingMassSquared);" << endl;
	locSourceStream << "			locUsedSoFar_MissingMass.insert(locUsedThisCombo_MissingMass);" << endl;
	locSourceStream << "		}" << endl;
	locSourceStream << endl;
	locSourceStream << "		//E.g. Cut" << endl;
	locSourceStream << "		//if((locMissingMassSquared < -0.04) || (locMissingMassSquared > 0.04))" << endl;
	locSourceStream << "		//	continue; //could also mark combo as cut, then save cut results to a new TTree" << endl;
	locSourceStream << "	}" << endl;
	locSourceStream << endl;
	locSourceStream << "	/******************************************* LOOP OVER THROWN DATA (OPTIONAL) ***************************************/" << endl;
	locSourceStream << "/*" << endl;
	locSourceStream << "	//Thrown beam: just use directly" << endl;
	locSourceStream << "	if(dThrownBeam != NULL)" << endl;
	locSourceStream << "		double locEnergy = dThrownBeam->Get_P4().E();" << endl;
	locSourceStream << endl;
	locSourceStream << "	//Loop over throwns" << endl;
	locSourceStream << "	for(UInt_t loc_i = 0; loc_i < Get_NumThrown(); ++loc_i)" << endl;
	locSourceStream << "	{" << endl;
	locSourceStream << "		//Set branch array indices corresponding to this particle" << endl;
	locSourceStream << "		dThrownWrapper->Set_ArrayIndex(loc_i);" << endl;
	locSourceStream << endl;
	locSourceStream << "		//Do stuff with the wrapper here ..." << endl;
	locSourceStream << "	}" << endl;
	locSourceStream << "*/" << endl;
	locSourceStream << "	/****************************************** LOOP OVER OTHER ARRAYS (OPTIONAL) ***************************************/" << endl;
	locSourceStream << "/*" << endl;
	locSourceStream << "	//Loop over beam particles (note, only those appearing in combos are present)" << endl;
	locSourceStream << "	for(UInt_t loc_i = 0; loc_i < Get_NumBeam(); ++loc_i)" << endl;
	locSourceStream << "	{" << endl;
	locSourceStream << "		//Set branch array indices corresponding to this particle" << endl;
	locSourceStream << "		dBeamWrapper->Set_ArrayIndex(loc_i);" << endl;
	locSourceStream << endl;
	locSourceStream << "		//Do stuff with the wrapper here ..." << endl;
	locSourceStream << "	}" << endl;
	locSourceStream << endl;
	locSourceStream << "	//Loop over charged track hypotheses (all are present, even those not in any combos)" << endl;
	locSourceStream << "	for(UInt_t loc_i = 0; loc_i < Get_NumChargedHypos(); ++loc_i)" << endl;
	locSourceStream << "	{" << endl;
	locSourceStream << "		//Set branch array indices corresponding to this particle" << endl;
	locSourceStream << "		dChargedHypoWrapper->Set_ArrayIndex(loc_i);" << endl;
	locSourceStream << endl;
	locSourceStream << "		//Do stuff with the wrapper here ..." << endl;
	locSourceStream << "	}" << endl;
	locSourceStream << endl;
	locSourceStream << "	//Loop over neutral particle hypotheses (all are present, even those not in any combos)" << endl;
	locSourceStream << "	for(UInt_t loc_i = 0; loc_i < Get_NumNeutralHypos(); ++loc_i)" << endl;
	locSourceStream << "	{" << endl;
	locSourceStream << "		//Set branch array indices corresponding to this particle" << endl;
	locSourceStream << "		dNeutralHypoWrapper->Set_ArrayIndex(loc_i);" << endl;
	locSourceStream << endl;
	locSourceStream << "		//Do stuff with the wrapper here ..." << endl;
	locSourceStream << "	}" << endl;
	locSourceStream << "*/" << endl;
	locSourceStream << endl;
	locSourceStream << "	/************************************ EXAMPLE: FILL CLONE OF TTREE HERE WITH CUTS APPLIED ************************************/" << endl;
	locSourceStream << "/*" << endl;
	locSourceStream << "	Bool_t locIsEventCut = true;" << endl;
	locSourceStream << "	for(UInt_t loc_i = 0; loc_i < Get_NumCombos(); ++loc_i) {" << endl;
	locSourceStream << "		//Set branch array indices for combo and all combo particles" << endl;
	locSourceStream << "		dComboWrapper->Set_ComboIndex(loc_i);" << endl;
	locSourceStream << "		// Is used to indicate when combos have been cut" << endl;
	locSourceStream << "		if(dComboWrapper->Get_IsComboCut())" << endl;
	locSourceStream << "			continue;" << endl;
	locSourceStream << "		locIsEventCut = false; // At least one combo succeeded" << endl;
	locSourceStream << "		break;" << endl;
	locSourceStream << "	}" << endl;
	locSourceStream << "	if(!locIsEventCut && dOutputTreeFileName != \"\")" << endl;
	locSourceStream << "		FillOutputTree();" << endl;
	locSourceStream << "*/" << endl;
	locSourceStream << endl;
	locSourceStream << "	return kTRUE;" << endl;
	locSourceStream << "}" << endl;
	locSourceStream << endl;
	locSourceStream << "void " << locSelectorName << "::Finalize(void)" << endl;
	locSourceStream << "{" << endl;
	locSourceStream << "	//Save anything to output here that you do not want to be in the default DSelector output ROOT file." << endl;
	locSourceStream << endl;
	locSourceStream << "	//Otherwise, don\'t do anything else (especially if you are using PROOF)." << endl;
	locSourceStream << "		//If you are using PROOF, this function is called on each thread," << endl;
	locSourceStream << "		//so anything you do will not have the combined information from the various threads." << endl;
	locSourceStream << "		//Besides, it is best-practice to do post-processing (e.g. fitting) separately, in case there is a problem." << endl;
	locSourceStream << endl;
	locSourceStream << "	//DO YOUR STUFF HERE" << endl;
	locSourceStream << endl;
	locSourceStream << "	//CALL THIS LAST" << endl;
	locSourceStream << "	DSelector::Finalize(); //Saves results to the output file" << endl;
	locSourceStream << "}" << endl;

	locSourceStream.close();
}
