#include "DSelector.h"

/******************************************************************** INITIALIZE **********************************************************************/

void DSelector::Init(TTree *locTree)
{
	// The Init() function is called when the selector needs to initialize a new tree or chain.
	// Typically here the branch addresses and branch pointers of the tree will be set.
	// Init() will be called many times when running on PROOF (once per file to be processed).

	// GET OPTION
	cout << "INITIALIZE NEW TREE" << endl;
	dOption = GetOption(); //optional argument given to TTree::Process()
	if(fInput != NULL) 
		dOption = ((TNamed*)fInput->FindObject("OPTIONS"))->GetTitle();
	
	// Parse any runtime options here...
	TString locOptions = dOption;
	locOptions.ToUpper(); // Want options to be case-insensitive
	
	// To reduce disk footprint, can turn off default flat branches at runtime with options flag "DefaultFlatOff"
	// e.g. execute DSelector with: TreeName->Process("DSelector_name.C+","DefaultFlatOff") (case insensitive)
	// or, you can change the value of dSaveDefaultFlatBranches in your DSelector
	if(locOptions.Contains("DEFAULTFLATOFF")) dSaveDefaultFlatBranches=false;
	if(dSaveDefaultFlatBranches==false) {
		cout << "DefaultFlatOff specified" << endl; 
		cout << "DEFAULT FLAT TREE BRANCHES WILL NOT BE SAVED!" << endl;
		cout << "(this will reduce disk footprint of flat trees)" << endl;

	}

	// SETUP OUTPUT
	// This must be done BEFORE initializing the DTreeInterface, etc. Why? I have no idea. Probably something to do with gDirectory changing.
	if(!dInitializedFlag)
		Setup_Output();

	// SETUP TREE
	locTree->GetDirectory()->cd();
	locTree->SetMakeClass(1);
	if(dTreeInterface == NULL)
		dTreeInterface = new DTreeInterface(locTree, true); //true: is input
	else
		dTreeInterface->Set_NewTree(locTree);

	// SETUP BRANCHES
	Setup_Branches();

	// IF PREVIOUSLY INITIALIZED, RE-INIT WRAPPERS
	if(dInitializedFlag)
		ReInitialize_Wrappers();

	// RETURN IF PREVIOUSLY INITIALIZED
	if(dInitializedFlag)
		return;

	// CREATE WRAPPERS
	Create_Wrappers();

	// SETUP TARGET
	Setup_Target();

	// CLONE TTree for writing out if desired
	map<string, TFile*>::const_iterator locFileIterator = dOutputTreeFileMap.begin();
	for(; locFileIterator != dOutputTreeFileMap.end(); ++locFileIterator)
	{
		locFileIterator->second->cd();
		dTreeInterface->Clone_Tree(locFileIterator->first);
	}

	// Create flat output TTree if desired
	if(dOutputFlatTreeFile != NULL)
	{
		dOutputFlatTreeFile->cd();
		Create_FlatTree();
	}

	//Change back to file dir, if it exists //So that user-created histograms are in the right location
	if(dFile != NULL)
		dFile->cd();

	dInitializedFlag = true;
}

/***************************************************************** SETUP DATA ACCESS ******************************************************************/

void DSelector::Setup_Branches(void)
{
	bool locIsMCFlag = (dTreeInterface->Get_Branch("MCWeight") != NULL);
	bool locIsMCGenOnlyFlag = (dTreeInterface->Get_Branch("NumCombos") == NULL);

	// EVENT DATA
	dRunNumber = (UInt_t*)dTreeInterface->Get_Branch("RunNumber")->GetAddress();
	dEventNumber = (ULong64_t*)dTreeInterface->Get_Branch("EventNumber")->GetAddress();
	TBranch* locL1TriggerBitsBranch = dTreeInterface->Get_Branch("L1TriggerBits");
	dL1TriggerBits = (locL1TriggerBitsBranch != NULL) ? (UInt_t*)locL1TriggerBitsBranch->GetAddress() : NULL;
	TBranch* locL1BCALEnergyBranch = dTreeInterface->Get_Branch("L1BCALEnergy");
	dL1BCALEnergy = (locL1BCALEnergyBranch != NULL) ? (Double_t*)locL1BCALEnergyBranch->GetAddress() : NULL;
	TBranch* locL1FCALEnergyBranch = dTreeInterface->Get_Branch("L1FCALEnergy");
	dL1FCALEnergy = (locL1FCALEnergyBranch != NULL) ? (Double_t*)locL1FCALEnergyBranch->GetAddress() : NULL;

	// MC
	if(locIsMCFlag)
	{
		dMCWeight = (Float_t*)dTreeInterface->Get_Branch("MCWeight")->GetAddress();
		TBranch* locGenEnergyBranch = dTreeInterface->Get_Branch("GeneratedEnergy");
		dGeneratedEnergy = (locGenEnergyBranch != NULL) ? (Float_t*)locGenEnergyBranch->GetAddress() : NULL;
		dNumThrown = (UInt_t*)dTreeInterface->Get_Branch("NumThrown")->GetAddress();
		dNumPIDThrown_FinalState = (ULong64_t*)dTreeInterface->Get_Branch("NumPIDThrown_FinalState")->GetAddress();
		dPIDThrown_Decaying = (ULong64_t*)dTreeInterface->Get_Branch("PIDThrown_Decaying")->GetAddress();
		if(!locIsMCGenOnlyFlag)
			dIsThrownTopology = (Bool_t*)dTreeInterface->Get_Branch("IsThrownTopology")->GetAddress();
	}

	// DATA
	if(!locIsMCGenOnlyFlag)
	{
		dNumBeam = (UInt_t*)dTreeInterface->Get_Branch("NumBeam")->GetAddress();
		dNumChargedHypos = (UInt_t*)dTreeInterface->Get_Branch("NumChargedHypos")->GetAddress();
		dNumNeutralHypos = (UInt_t*)dTreeInterface->Get_Branch("NumNeutralHypos")->GetAddress();
		dNumCombos = (UInt_t*)dTreeInterface->Get_Branch("NumCombos")->GetAddress();
		dX4_Production = dTreeInterface->Get_PointerToPointerTo_TObject<TLorentzVector>("X4_Production");
	}
}

void DSelector::Create_Wrappers(void)
{
	if(dNumThrown != NULL) //thrown
	{
		dThrownBeam = new DBeamParticle(dTreeInterface, "ThrownBeam");
		dThrownWrapper = new DMCThrown(dTreeInterface);
	}

	if(dNumCombos != NULL) //detected
	{
		dChargedHypoWrapper = new DChargedTrackHypothesis(dTreeInterface, "ChargedHypo");
		dNeutralHypoWrapper = new DNeutralParticleHypothesis(dTreeInterface, "NeutralHypo");
		dBeamWrapper = new DBeamParticle(dTreeInterface, "Beam");
		dComboWrapper = new DParticleCombo(dTreeInterface);
	}
}

void DSelector::ReInitialize_Wrappers(void)
{
	if(dNumThrown != NULL) //thrown
	{
		dThrownBeam->ReInitialize();
		dThrownWrapper->ReInitialize();
	}

	if(dNumCombos != NULL) //detected
	{
		dChargedHypoWrapper->ReInitialize();
		dNeutralHypoWrapper->ReInitialize();
		dBeamWrapper->ReInitialize();
		dComboWrapper->ReInitialize();
	}
}

void DSelector::Setup_Output(void)
{
	//Read input variables for setting up output
	if(fInput != NULL)
	{
		TNamed* locOutputFileObject = (TNamed*)fInput->FindObject("OUTPUT_FILENAME");
		if(locOutputFileObject != NULL) 
			dOutputFileName = locOutputFileObject->GetTitle();

		TNamed* locOutputTreeFileObject = (TNamed*)fInput->FindObject("OUTPUT_TREE_FILENAME");
		if(locOutputTreeFileObject != NULL) 
			dOutputTreeFileName = locOutputTreeFileObject->GetTitle();

		TNamed* locFlatTreeFileObject = (TNamed*)fInput->FindObject("FLAT_TREE_FILENAME");
		if(locFlatTreeFileObject != NULL)
			dFlatTreeFileName = locFlatTreeFileObject->GetTitle();
	}
	if(dOutputTreeFileName != "")
		dOutputTreeFileNameMap[""] = dOutputTreeFileName;

	//Create output hist file (if desired)
	if(dOutputFileName != "")
	{
		if(gProofServ == NULL)
			dFile = new TFile(dOutputFileName.c_str(), "RECREATE");
		else
		{
			dProofFile = new TProofOutputFile(dOutputFileName.c_str(), TProofOutputFile::kMerge);
			dFile = dProofFile->OpenFile("RECREATE");
		}
	}

	//Create output clone tree file (if desired)
	map<string, string>::const_iterator locIterator = dOutputTreeFileNameMap.begin();
	for(; locIterator != dOutputTreeFileNameMap.end(); ++locIterator)
	{
		if(gProofServ == NULL)
			dOutputTreeFileMap[locIterator->first] = new TFile(locIterator->second.c_str(), "RECREATE");
		else
		{
			dOutputTreeProofFileMap[locIterator->first] = new TProofOutputFile(locIterator->second.c_str(), TProofOutputFile::kMerge);
			dOutputTreeFileMap[locIterator->first] = dOutputTreeProofFileMap[locIterator->first]->OpenFile("RECREATE");
		}
	}

	//Create output flat tree file (if desired)
	if(dFlatTreeFileName != "")
	{
		if(gProofServ == NULL)
			dOutputFlatTreeFile = new TFile(dFlatTreeFileName.c_str(), "RECREATE");
		else
		{
			dOutputFlatTreeProofFile = new TProofOutputFile(dFlatTreeFileName.c_str(), TProofOutputFile::kMerge);
			dOutputFlatTreeFile = dOutputFlatTreeProofFile->OpenFile("RECREATE");
		}
	}
}

void DSelector::Setup_Target(void)
{
	dTargetPID = dTreeInterface->Get_TargetPID();
	dTargetP4 = TLorentzVector(TVector3(), ParticleMass(dTargetPID));
	dTargetCenter = dTreeInterface->Get_TargetCenter();
}

/***************************************************************** PRIMARY FUNCTIONS ******************************************************************/

Bool_t DSelector::Process(Long64_t locEntry)
{
	// The Process() function is called for each entry in the tree. The entry argument
	// specifies which entry in the currently loaded tree is to be processed.
	//
	// This function should contain the "body" of the analysis. It can contain
	// simple or elaborate selection criteria, run algorithms on the data
	// of the event and typically fill histograms.
	//
	// The processing can be stopped by calling Abort().
	// Use fStatus to set the return value of TTree::Process().
	// The return value is currently not used.

	//If operating directly on a chain, must call init for each new tree
	if(dTreeInterface->Get_TreeNumber() != dTreeNumber)
	{
		Init(dTreeInterface->dInputTree); //Init for next tree in chain (not done by TTreePlayer::Process() by default!!!)
		dTreeNumber = dTreeInterface->Get_TreeNumber();
	}

	dTreeInterface->Get_Entry(locEntry);

	// zero out loop indicies if there are no trigger bits set, so that we skip such events
	// leave this as an togglable option for trigger studies
	if( (dL1TriggerBits==0) && dSkipNoTriggerEvents) {
		*dNumBeam = 0;
		*dNumChargedHypos = 0;
		*dNumNeutralHypos = 0;
		*dNumCombos = 0;
		if(dNumThrown != NULL)  *dNumThrown = 0;
	}

	return kTRUE;
}

void DSelector::SlaveTerminate()
{
	// The SlaveTerminate() function is called after all entries or objects
	// have been processed. When running with PROOF SlaveTerminate() is called
	// on each slave server.
	if(gProofServ != NULL)
		Finalize();
}

void DSelector::Terminate()
{
	// The Terminate() function is the last function to be called during
	// a query. It always runs on the client, it can be used to present
	// the results graphically or save the results to file.
	if(gProofServ == NULL)
		Finalize();
}

void DSelector::Finalize()
{
	//Write output flat tree and close file
	if(dOutputFlatTreeFile != NULL)
	{
		dOutputFlatTreeFile->Write(0, TObject::kOverwrite);
		dOutputFlatTreeFile->Close();
	}
	if(dOutputFlatTreeProofFile != NULL)
		fOutput->Add(dOutputFlatTreeProofFile);

	//Write output clone trees and close files
	map<string, TFile*>::const_iterator locFileIterator = dOutputTreeFileMap.begin();
	for(; locFileIterator != dOutputTreeFileMap.end(); ++locFileIterator)
	{
		locFileIterator->second->Write(0, TObject::kOverwrite);
		locFileIterator->second->Close();
	}
	map<string, TProofOutputFile*>::const_iterator locPROOFFileIterator = dOutputTreeProofFileMap.begin();
	for(; locPROOFFileIterator != dOutputTreeProofFileMap.end(); ++locPROOFFileIterator)
		fOutput->Add(locPROOFFileIterator->second);

	//Write output histograms and close file
	if(dFile != NULL)
	{
		dFile->Write();
		dFile->Close();
	}
	if(dProofFile != NULL)
		fOutput->Add(dProofFile);
}

/**************************************************************** EXTRACT THROWN INFO *****************************************************************/

vector<Particle_t> DSelector::Get_ThrownDecayingPIDs(void) const
{
	//the types of the thrown decaying particles in the event (multiplexed in base 2)
	//not the quantity of each, just whether or not they were present (1 or 0)
	//binary power of a PID is given by ParticleMultiplexPower() (defined in libraries/include/particleType.h)
	vector<Particle_t> locPIDs;

	ULong64_t locPIDThrown_Decaying = Get_PIDThrown_Decaying();
	if(locPIDThrown_Decaying == 0)
		return locPIDs;
	for(ULong64_t locBit = 0; locBit < 64; ++locBit) //ULong64_t is 64 bits
	{
		ULong64_t locParticleMultiplexID = (ULong64_t(1) << ULong64_t(locBit)); //bit-shift
		if((locPIDThrown_Decaying & locParticleMultiplexID) == 0) //bit-wise and
			continue;
		Particle_t locPID = DemultiplexPID(int(locBit), 1);
		locPIDs.push_back(locPID);
	}
	return locPIDs;
}

map<Particle_t, UInt_t> DSelector::Get_NumFinalStateThrown(void) const
{
	//the # of thrown final-state particles (+ pi0) of each type (multiplexed in base 10)
		//types (in order from 10^0 -> 10^15): g, e+, e-, nu, mu+, mu-, pi0, pi+, pi-, KLong, K+, K-, n, p, p-bar, n-bar
		//e.g. particles decaying from final-state particles are NOT included (e.g. photons from pi0, muons from pions, etc.)
	//is sum of #-of-PID * 10^ParticleMultiplexPower() (defined in libraries/include/particleType.h)
	//ParticleMultiplexPower() returns a different power of 10 for each final-state PID type. 
	//A value of 9 should be interpreted as >= 9.  
	map<Particle_t, UInt_t> locNumFinalStateThrown;

	ULong64_t locNumPIDThrown_FinalState = Get_NumPIDThrown_FinalState();
	if(locNumPIDThrown_FinalState == 0)
		return locNumFinalStateThrown;
	ULong64_t locParticleMultiplexID = 1;
	for(ULong64_t locDecimalIndex = 0; locDecimalIndex < 19; ++locDecimalIndex) //Max ULong64_t is ~1.8e19
	{
		Particle_t locPID = DemultiplexPID(locDecimalIndex, 0);
		locNumFinalStateThrown[locPID] = (locNumPIDThrown_FinalState / locParticleMultiplexID) % ULong64_t(10);
		locParticleMultiplexID *= ULong64_t(10);
	}
	return locNumFinalStateThrown;
}

TString DSelector::Get_ThrownTopologyString() const
{
	TString locReactionName;
	int locNumPi0s = 0;

	// Get final state particles and multiplicity
	map<Particle_t, UInt_t> locNumFinalStateThrown = Get_NumFinalStateThrown();
	for(auto const& locParticle : locNumFinalStateThrown) {
		Particle_t locPID = locParticle.first; //= DemultiplexPID(ibit, false);
		int locNumParticles = locParticle.second; //= locNumPIDThrown_FinalState/long(pow(10,ParticleMultiplexPower(locDemultiplexPID)))%10;
		if(locNumParticles > 0) {
			// List pi0s with decaying particles, since all final state photons are given
			if(locPID == Pi0) {
				locNumPi0s = locNumParticles;
				continue;
			}

			if(locNumParticles > 1) locReactionName += locNumParticles;
			locReactionName += ParticleName_ROOT(locPID);
		}
	}

	// Get list of decaying particles (if they exist)
	vector<Particle_t> locThrownDecayingPIDs = Get_ThrownDecayingPIDs();
	if(locThrownDecayingPIDs.size() > 0 || locNumPi0s > 0) {
		locReactionName += "[";
		if(locNumPi0s == 1) locReactionName += "#pi^{0}";
		else if(locNumPi0s > 1) {
			locReactionName += locNumPi0s;
			locReactionName += "#pi^{0}";
		}
					  
		// add comma if previous decaying particle exists
		bool locAddComma = false; 
		if(locNumPi0s > 0) locAddComma = true;

		for(uint i = 0; i<locThrownDecayingPIDs.size(); i++) {
			Particle_t locPID = locThrownDecayingPIDs[i]; //DemultiplexPID(ibit, true);
			if(locAddComma) locReactionName += ",";
			locReactionName += ParticleName_ROOT(locPID);
			locAddComma = true;
		}
		locReactionName += "]";
	}

	return locReactionName;
}

void DSelector::Fill_OutputTree(string locKeyName)
{
	// The FillOutputTree() function is called for events in the tree which pass
	// the user-defined analysis cuts.  The output file then contains the TTree 
	// with these events that can be used for higher-level analysis (eg. AmptTools fit,
	// PWA, etc.)

	dTreeInterface->Fill_OutputTree(locKeyName);
}

void DSelector::Create_ComboSurvivalHists(void)
{
	vector<string> locActionNames;
	size_t locNumActions = dAnalysisActions.size();
	for(size_t loc_j = 0; loc_j < locNumActions; ++loc_j)
		locActionNames.push_back(dAnalysisActions[loc_j]->Get_ActionName());

	gDirectory->cd("/");

	dHist_NumEventsSurvivedAction = new TH1D("NumEventsSurvivedAction", ";;# Events Survived Action", locNumActions + 1, -0.5, locNumActions + 1.0 - 0.5); //+1 for input
	dHist_NumEventsSurvivedAction->GetXaxis()->SetBinLabel(1, "Input"); // a new event
	for(size_t loc_j = 0; loc_j < locActionNames.size(); ++loc_j)
		dHist_NumEventsSurvivedAction->GetXaxis()->SetBinLabel(2 + loc_j, locActionNames[loc_j].c_str());

	dHist_NumCombosSurvivedAction = new TH2D("NumCombosSurvivedAction", ";;# Particle Combos Survived Action", locNumActions + 1, -0.5, locNumActions + 1 - 0.5, 101, -0.5, 100.5); //+1 for input
	dHist_NumCombosSurvivedAction->GetXaxis()->SetBinLabel(1, "Input");
	for(size_t loc_j = 0; loc_j < locActionNames.size(); ++loc_j)
		dHist_NumCombosSurvivedAction->GetXaxis()->SetBinLabel(2 + loc_j, locActionNames[loc_j].c_str());

	dHist_NumCombosSurvivedAction1D = new TH1D("NumCombosSurvivedAction1D", ";;# Particle Combos Survived Action", locNumActions + 1, -0.5, locNumActions + 1 - 0.5); //+1 for # tracks
	dHist_NumCombosSurvivedAction1D->GetXaxis()->SetBinLabel(1, "Input");
	for(size_t loc_j = 0; loc_j < locActionNames.size(); ++loc_j)
		dHist_NumCombosSurvivedAction1D->GetXaxis()->SetBinLabel(2 + loc_j, locActionNames[loc_j].c_str());

	dNumCombosSurvivedAction.assign(locNumActions + 1, 0);
}

void DSelector::Create_FlatTree(void)
{
	//create flat tree & interface
	string locTreeName = (dFlatTreeName != "") ? dFlatTreeName : dTreeInterface->Get_TreeName().substr(0, dTreeInterface->Get_TreeName().size() - 5);
	TTree* locFlatTree = new TTree(locTreeName.c_str(), locTreeName.c_str());
	dFlatTreeInterface = new DTreeInterface(locFlatTree, false, dSaveTLorentzVectorsAsFundamentaFlatTree); //false: is output

	//set user info
	TList* locInputUserInfo = dTreeInterface->Get_UserInfo();
	TList* locOutputUserInfo = locFlatTree->GetUserInfo();
	for(Int_t loc_i = 0; loc_i < locInputUserInfo->GetSize(); ++loc_i)
		locOutputUserInfo->Add(locInputUserInfo->At(loc_i)->Clone());

	bool locIsMCFlag = (dTreeInterface->Get_Branch("MCWeight") != NULL);
	bool locIsMCGenOnlyFlag = (dTreeInterface->Get_Branch("NumCombos") == NULL);

	// Skip default branches below
	// (custom branches defined in your DSelector will still be saved)
	if(!dSaveDefaultFlatBranches) {
		return; //Stop here
	}

	//CREATE BRANCHES: MAIN EVENT INFO //Copy memory addresses from main tree, so won't even need to set these branch's data
	dFlatTreeInterface->Create_Branch_Fundamental<UInt_t>("run", dTreeInterface->Get_BranchMemory_Fundamental<UInt_t>("RunNumber"));
	dFlatTreeInterface->Create_Branch_Fundamental<ULong64_t>("event", dTreeInterface->Get_BranchMemory_Fundamental<ULong64_t>("EventNumber"));
	if(locIsMCFlag)
	{
		dFlatTreeInterface->Create_Branch_Fundamental<Float_t>("weight", dTreeInterface->Get_BranchMemory_Fundamental<Float_t>("MCWeight"));
		dFlatTreeInterface->Create_Branch_Fundamental<ULong64_t>("numtruepid_final", dTreeInterface->Get_BranchMemory_Fundamental<ULong64_t>("NumPIDThrown_FinalState"));
		dFlatTreeInterface->Create_Branch_Fundamental<ULong64_t>("truepids_decay", dTreeInterface->Get_BranchMemory_Fundamental<ULong64_t>("PIDThrown_Decaying"));
		if(!locIsMCGenOnlyFlag)
			dFlatTreeInterface->Create_Branch_Fundamental<Bool_t>("is_truetop", dTreeInterface->Get_BranchMemory_Fundamental<Bool_t>("IsThrownTopology"));
	}
	else
		dFlatTreeInterface->Create_Branch_Fundamental<UInt_t>("trigbits", dTreeInterface->Get_BranchMemory_Fundamental<UInt_t>("L1TriggerBits"));

	if(locIsMCGenOnlyFlag)
	{
		//CODE SOMETHING HERE!!
		return;
	}

	//CREATE BRANCHES: MAIN COMBO INFO
	if(locIsMCFlag)
	{
		dFlatTreeInterface->Create_Branch_Fundamental<Bool_t>("is_truecombo");
		dFlatTreeInterface->Create_Branch_Fundamental<Bool_t>("is_bdtcombo");
	}
	dFlatTreeInterface->Create_Branch_Fundamental<Float_t>("rftime");
	if(dTreeInterface->Get_Branch("ChiSq_KinFit") != NULL)
	{
		dFlatTreeInterface->Create_Branch_Fundamental<Float_t>("kin_chisq");
		dFlatTreeInterface->Create_Branch_Fundamental<UInt_t>("kin_ndf");
	}

	//CREATE BRANCHES: COMBO
	set<string> locBranchesCreatedParticles;
	for(UInt_t loc_i = 0; loc_i < dComboWrapper->Get_NumParticleComboSteps(); ++loc_i)
	{
		DParticleComboStep* locStepWrapper = dComboWrapper->Get_ParticleComboStep(loc_i);

		//initial particle
		DKinematicData* locInitialParticle = locStepWrapper->Get_InitialParticle();
		if(locInitialParticle != NULL)
		{
			string locEventBranchPrefix = locInitialParticle->Get_BranchNamePrefix();
			if(locBranchesCreatedParticles.find(locEventBranchPrefix) == locBranchesCreatedParticles.end())
			{
				Create_FlatBranches(locInitialParticle, locIsMCFlag);
				locBranchesCreatedParticles.insert(locEventBranchPrefix);
			}
		}

		//final particles
		for(UInt_t loc_j = 0; loc_j < locStepWrapper->Get_NumFinalParticles(); ++loc_j)
		{
			DKinematicData* locFinalParticle = locStepWrapper->Get_FinalParticle(loc_j);
			if(locFinalParticle == NULL)
				continue;

			string locEventBranchPrefix = locFinalParticle->Get_BranchNamePrefix();
			if(locBranchesCreatedParticles.find(locEventBranchPrefix) == locBranchesCreatedParticles.end())
			{
				Create_FlatBranches(locFinalParticle, locIsMCFlag);
				locBranchesCreatedParticles.insert(locEventBranchPrefix);
			}
		}
	}
}

void DSelector::Create_FlatBranches(DKinematicData* locParticle, bool locIsMCFlag)
{
	//handle beam
	string locEventBranchPrefix = locParticle->Get_BranchNamePrefix();
	if(locEventBranchPrefix == "ComboBeam")
	{
		string locBranchPrefix = "beam";

		//identifiers
		dFlatTreeInterface->Create_Branch_Fundamental<UInt_t>(locBranchPrefix + "_beamid");
		if(locIsMCFlag)
			dFlatTreeInterface->Create_Branch_Fundamental<Bool_t>(locBranchPrefix + "_isgen");

		//kinematics
        dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>(locBranchPrefix + "_x4_meas");
        dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>(locBranchPrefix + "_p4_meas");
    	if(dTreeInterface->Get_Branch(locEventBranchPrefix + "__X4_KinFit") != NULL)
			dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>(locBranchPrefix + "_x4_kin");
    	if(dTreeInterface->Get_Branch(locEventBranchPrefix + "__P4_KinFit") != NULL)
			dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>(locBranchPrefix + "_p4_kin");

		if(locIsMCFlag)
		{
	        dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>(locBranchPrefix + "_x4_true");
	        dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>(locBranchPrefix + "_p4_true");
		}

        return;
	}

	//handle decaying & final state particles

	//get the branch name //first get particle # (e.g. extract the "2" from "PiPlus2")
	Particle_t locPID = locParticle->Get_PID();
	string locBranchParticleName_EventTree = Convert_ToBranchName(locPID);
	string locBranchPrefix;
	if(locEventBranchPrefix.substr(0, 7) == "Missing")
	{
		string locParticleNumber = locEventBranchPrefix.substr(locBranchParticleName_EventTree.length() + 7);
		locBranchPrefix = string("miss") + Get_ShortName(locPID) + locParticleNumber;
	}
	else if(locEventBranchPrefix.substr(0, 8) == "Decaying")
	{
		string locParticleNumber = locEventBranchPrefix.substr(locBranchParticleName_EventTree.length() + 8);
		locBranchPrefix = string("decay") + Get_ShortName(locPID) + locParticleNumber;
	}
	else
	{
		string locParticleNumber = locEventBranchPrefix.substr(locBranchParticleName_EventTree.length());
		locBranchPrefix = Get_ShortName(locPID) + locParticleNumber;
	}

	//handle detected charged
	if(dynamic_cast<DChargedTrackHypothesis*>(locParticle) != NULL)
	{
		//identifiers
		dFlatTreeInterface->Create_Branch_Fundamental<UInt_t>(locBranchPrefix + "_trkid");

		//kinematics
                dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>(locBranchPrefix + "_x4_meas");
                dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>(locBranchPrefix + "_p4_meas");
    	        if(dTreeInterface->Get_Branch(locEventBranchPrefix + "__X4_KinFit") != NULL)
    		        dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>(locBranchPrefix + "_x4_kin");
            	if(dTreeInterface->Get_Branch(locEventBranchPrefix + "__P4_KinFit") != NULL)
    	        	dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>(locBranchPrefix + "_p4_kin");

		if(locIsMCFlag)
		{
			dFlatTreeInterface->Create_Branch_Fundamental<Float_t>(locBranchPrefix + "_true_fom");
	        dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>(locBranchPrefix + "_x4_true");
	        dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>(locBranchPrefix + "_p4_true");
		}

		// Global PID
		dFlatTreeInterface->Create_Branch_Fundamental<Float_t>(locBranchPrefix + "_pid_fom");

		//timing
		dFlatTreeInterface->Create_Branch_Fundamental<Float_t>(locBranchPrefix + "_beta_time");
		dFlatTreeInterface->Create_Branch_Fundamental<Float_t>(locBranchPrefix + "_chisq_time");
		dFlatTreeInterface->Create_Branch_Fundamental<UInt_t>(locBranchPrefix + "_ndf_time");

		//tracking
		dFlatTreeInterface->Create_Branch_Fundamental<UInt_t>(locBranchPrefix + "_ndf_trk");
		dFlatTreeInterface->Create_Branch_Fundamental<Float_t>(locBranchPrefix + "_chisq_trk");
		dFlatTreeInterface->Create_Branch_Fundamental<UInt_t>(locBranchPrefix + "_ndf_dedx");
		dFlatTreeInterface->Create_Branch_Fundamental<Float_t>(locBranchPrefix + "_chisq_dedx");
		dFlatTreeInterface->Create_Branch_Fundamental<Float_t>(locBranchPrefix + "_dedx_cdc");
		dFlatTreeInterface->Create_Branch_Fundamental<Float_t>(locBranchPrefix + "_dedx_fdc");

        //hit energy
		dFlatTreeInterface->Create_Branch_Fundamental<Float_t>(locBranchPrefix + "_dedx_tof");
		dFlatTreeInterface->Create_Branch_Fundamental<Float_t>(locBranchPrefix + "_dedx_st");
		dFlatTreeInterface->Create_Branch_Fundamental<Float_t>(locBranchPrefix + "_ebcal");
		dFlatTreeInterface->Create_Branch_Fundamental<Float_t>(locBranchPrefix + "_eprebcal");
		dFlatTreeInterface->Create_Branch_Fundamental<Float_t>(locBranchPrefix + "_efcal");

		//shower matching
		dFlatTreeInterface->Create_Branch_Fundamental<Float_t>(locBranchPrefix + "_bcal_delphi");
		dFlatTreeInterface->Create_Branch_Fundamental<Float_t>(locBranchPrefix + "_bcal_delz");
		dFlatTreeInterface->Create_Branch_Fundamental<Float_t>(locBranchPrefix + "_fcal_doca");

		return;
	}

	//handle detected neutral
	if(dynamic_cast<DNeutralParticleHypothesis*>(locParticle) != NULL)
	{
		//identifiers
		dFlatTreeInterface->Create_Branch_Fundamental<UInt_t>(locBranchPrefix + "_showid");

		//kinematics
        dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>(locBranchPrefix + "_x4_meas");
        dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>(locBranchPrefix + "_p4_meas");
        if(dTreeInterface->Get_Branch(locEventBranchPrefix + "__X4_KinFit") != NULL)
        	dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>(locBranchPrefix + "_x4_kin");
        if(dTreeInterface->Get_Branch(locEventBranchPrefix + "__P4_KinFit") != NULL)
        	dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>(locBranchPrefix + "_p4_kin");

        //mc
		if(locIsMCFlag)
		{
			dFlatTreeInterface->Create_Branch_Fundamental<Float_t>(locBranchPrefix + "_true_fom");
	        dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>(locBranchPrefix + "_x4_true");
	        dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>(locBranchPrefix + "_p4_true");
		}

		//timing
		dFlatTreeInterface->Create_Branch_Fundamental<Float_t>(locBranchPrefix + "_beta_time");
		dFlatTreeInterface->Create_Branch_Fundamental<Float_t>(locBranchPrefix + "_chisq_time");
		dFlatTreeInterface->Create_Branch_Fundamental<UInt_t>(locBranchPrefix + "_ndf_time");

        //hit energy
		dFlatTreeInterface->Create_Branch_Fundamental<Float_t>(locBranchPrefix + "_ebcal");
		dFlatTreeInterface->Create_Branch_Fundamental<Float_t>(locBranchPrefix + "_eprebcal");
		dFlatTreeInterface->Create_Branch_Fundamental<Float_t>(locBranchPrefix + "_efcal");

		//shower matching
		dFlatTreeInterface->Create_Branch_Fundamental<Float_t>(locBranchPrefix + "_bcal_delphi");
		dFlatTreeInterface->Create_Branch_Fundamental<Float_t>(locBranchPrefix + "_bcal_delz");
		dFlatTreeInterface->Create_Branch_Fundamental<Float_t>(locBranchPrefix + "_fcal_doca");

		return;
	}

	//is a kinfit missing or decaying particle
	{
		//NEED TO ADD THROWN X4 & P4!!
		if(dTreeInterface->Get_Branch(locEventBranchPrefix + "__X4") != NULL)
			dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>(locBranchPrefix + "_x4");
		if(dTreeInterface->Get_Branch(locEventBranchPrefix + "__P4_KinFit") != NULL)
			dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>(locBranchPrefix + "_p4_kin");
	}
}

void DSelector::Fill_FlatTree(void)
{
	
	bool locIsMCFlag = (dTreeInterface->Get_Branch("MCWeight") != NULL);
	bool locIsMCGenOnlyFlag = (dTreeInterface->Get_Branch("NumCombos") == NULL);

	if(locIsMCGenOnlyFlag)
	{
		//CODE SOMETHING HERE!!
		dFlatTreeInterface->Fill_OutputTree("");
		return;
	}

	// Fill tree, but not the default branches below
	// (custom branches defined in your DSelector will still be saved)
	if(!dSaveDefaultFlatBranches) {
		//FILL TREE
		dFlatTreeInterface->Fill_OutputTree("");
		return; //Stop here
	}

	//FILL BRANCHES: MAIN COMBO INFO
	if(locIsMCFlag)
	{
		dFlatTreeInterface->Fill_Fundamental<Bool_t>("is_truecombo", dComboWrapper->Get_IsTrueCombo());
		dFlatTreeInterface->Fill_Fundamental<Bool_t>("is_bdtcombo", dComboWrapper->Get_IsBDTSignalCombo());
	}
	dFlatTreeInterface->Fill_Fundamental<Float_t>("rftime", dComboWrapper->Get_RFTime());
	if(dTreeInterface->Get_Branch("ChiSq_KinFit") != NULL)
	{
		dFlatTreeInterface->Fill_Fundamental<Float_t>("kin_chisq", dComboWrapper->Get_ChiSq_KinFit( "" ));
		dFlatTreeInterface->Fill_Fundamental<UInt_t>("kin_ndf", dComboWrapper->Get_NDF_KinFit( "" ));
	}

	//FILL BRANCHES: COMBO
	set<string> locBranchesFilledParticles;
	for(UInt_t loc_i = 0; loc_i < dComboWrapper->Get_NumParticleComboSteps(); ++loc_i)
	{
		DParticleComboStep* locStepWrapper = dComboWrapper->Get_ParticleComboStep(loc_i);

		//initial particle
		DKinematicData* locInitialParticle = locStepWrapper->Get_InitialParticle();
		if(locInitialParticle != NULL)
		{
			string locEventBranchPrefix = locInitialParticle->Get_BranchNamePrefix();
			if(locBranchesFilledParticles.find(locEventBranchPrefix) == locBranchesFilledParticles.end())
			{
				Fill_FlatBranches(locInitialParticle, locIsMCFlag);
				locBranchesFilledParticles.insert(locEventBranchPrefix);
			}
		}

		//final particles
		for(UInt_t loc_j = 0; loc_j < locStepWrapper->Get_NumFinalParticles(); ++loc_j)
		{
			DKinematicData* locFinalParticle = locStepWrapper->Get_FinalParticle(loc_j);
			if(locFinalParticle == NULL)
				continue;
			string locEventBranchPrefix = locFinalParticle->Get_BranchNamePrefix();
			if(locBranchesFilledParticles.find(locEventBranchPrefix) == locBranchesFilledParticles.end())
			{
				Fill_FlatBranches(locFinalParticle, locIsMCFlag);
				locBranchesFilledParticles.insert(locEventBranchPrefix);
			}
		}
	}

	//FILL TREE
	dFlatTreeInterface->Fill_OutputTree("");
}

void DSelector::Fill_FlatBranches(DKinematicData* locParticle, bool locIsMCFlag)
{
	//handle beam
	string locEventBranchPrefix = locParticle->Get_BranchNamePrefix();
	//cout << "fill particle: " << locEventBranchPrefix << endl;
	if(locEventBranchPrefix == "ComboBeam")
	{
		string locBranchPrefix = "beam";
		DBeamParticle* locBeamParticle = dynamic_cast<DBeamParticle*>(locParticle);

		//identifiers
		dFlatTreeInterface->Fill_Fundamental<UInt_t>(locBranchPrefix + "_beamid", locBeamParticle->Get_ID());
		if(locIsMCFlag)
			dFlatTreeInterface->Fill_Fundamental<Bool_t>(locBranchPrefix + "_isgen", locBeamParticle->Get_IsGenerator());

		//kinematics
        dFlatTreeInterface->Fill_TObject<TLorentzVector>(locBranchPrefix + "_x4_meas", locBeamParticle->Get_X4_Measured());
        dFlatTreeInterface->Fill_TObject<TLorentzVector>(locBranchPrefix + "_p4_meas", locBeamParticle->Get_P4_Measured());
    	if(dTreeInterface->Get_Branch(locEventBranchPrefix + "__X4_KinFit") != NULL)
			dFlatTreeInterface->Fill_TObject<TLorentzVector>(locBranchPrefix + "_x4_kin", locBeamParticle->Get_X4());
    	if(dTreeInterface->Get_Branch(locEventBranchPrefix + "__P4_KinFit") != NULL)
			dFlatTreeInterface->Fill_TObject<TLorentzVector>(locBranchPrefix + "_p4_kin", locBeamParticle->Get_P4());

		if(locIsMCFlag)
		{
			if(locBeamParticle->Get_IsGenerator())
			{
				dFlatTreeInterface->Fill_TObject<TLorentzVector>(locBranchPrefix + "_x4_true", dThrownBeam->Get_X4());
				dFlatTreeInterface->Fill_TObject<TLorentzVector>(locBranchPrefix + "_p4_true", dThrownBeam->Get_P4());
			}
			else
			{
				dFlatTreeInterface->Fill_TObject<TLorentzVector>(locBranchPrefix + "_x4_true", TLorentzVector());
				dFlatTreeInterface->Fill_TObject<TLorentzVector>(locBranchPrefix + "_p4_true", TLorentzVector());
			}
		}

		return;
	}

	//handle decaying & final state particles

	//get the branch name //first get particle # (e.g. extract the "2" from "PiPlus2")
	Particle_t locPID = locParticle->Get_PID();
	string locBranchParticleName_EventTree = Convert_ToBranchName(locPID);
	string locBranchPrefix;
	if(locEventBranchPrefix.substr(0, 7) == "Missing")
	{
		string locParticleNumber = locEventBranchPrefix.substr(locBranchParticleName_EventTree.length() + 7);
		locBranchPrefix = string("miss") + Get_ShortName(locPID) + locParticleNumber;
	}
	else if(locEventBranchPrefix.substr(0, 8) == "Decaying")
	{
		string locParticleNumber = locEventBranchPrefix.substr(locBranchParticleName_EventTree.length() + 8);
		locBranchPrefix = string("decay") + Get_ShortName(locPID) + locParticleNumber;
	}
	else
	{
		string locParticleNumber = locEventBranchPrefix.substr(locBranchParticleName_EventTree.length());
		locBranchPrefix = Get_ShortName(locPID) + locParticleNumber;
	}

	//handle detected charged
	DChargedTrackHypothesis* locChargedTrackHypothesis = dynamic_cast<DChargedTrackHypothesis*>(locParticle);
	if(locChargedTrackHypothesis != NULL)
	{
		//identifiers
		dFlatTreeInterface->Fill_Fundamental<UInt_t>(locBranchPrefix + "_trkid", locChargedTrackHypothesis->Get_ID());

		//kinematics
                dFlatTreeInterface->Fill_TObject<TLorentzVector>(locBranchPrefix + "_x4_meas", locChargedTrackHypothesis->Get_X4_Measured());
                dFlatTreeInterface->Fill_TObject<TLorentzVector>(locBranchPrefix + "_p4_meas", locChargedTrackHypothesis->Get_P4_Measured());
            	if(dTreeInterface->Get_Branch(locEventBranchPrefix + "__X4_KinFit") != NULL)
            		dFlatTreeInterface->Fill_TObject<TLorentzVector>(locBranchPrefix + "_x4_kin", locChargedTrackHypothesis->Get_X4());
            	if(dTreeInterface->Get_Branch(locEventBranchPrefix + "__P4_KinFit") != NULL)
            		dFlatTreeInterface->Fill_TObject<TLorentzVector>(locBranchPrefix + "_p4_kin", locChargedTrackHypothesis->Get_P4());
              
		if(locIsMCFlag)
		{
			Int_t locThrownIndex = locChargedTrackHypothesis->Get_ThrownIndex();
			if(locThrownIndex >= 0)
			{
				dThrownWrapper->Set_ArrayIndex(locThrownIndex);
				dFlatTreeInterface->Fill_Fundamental<Float_t>(locBranchPrefix + "_true_fom", dThrownWrapper->Get_MatchFOM());
				dFlatTreeInterface->Fill_TObject<TLorentzVector>(locBranchPrefix + "_x4_true", dThrownWrapper->Get_X4());
				dFlatTreeInterface->Fill_TObject<TLorentzVector>(locBranchPrefix + "_p4_true", dThrownWrapper->Get_P4());
			}
			else
			{
				dFlatTreeInterface->Fill_Fundamental<Float_t>(locBranchPrefix + "_true_fom", -1.0);
				dFlatTreeInterface->Fill_TObject<TLorentzVector>(locBranchPrefix + "_x4_true", TLorentzVector());
				dFlatTreeInterface->Fill_TObject<TLorentzVector>(locBranchPrefix + "_p4_true", TLorentzVector());
			}
		}
	       
		// Global PID
        if(dTreeInterface->Get_Branch(locEventBranchPrefix + "__PIDFOM") != NULL)
            dFlatTreeInterface->Fill_Fundamental<Float_t>(locBranchPrefix + "_pid_fom", locChargedTrackHypothesis->Get_PIDFOM());

		//timing
		dFlatTreeInterface->Fill_Fundamental<Float_t>(locBranchPrefix + "_beta_time", locChargedTrackHypothesis->Get_Beta_Timing());
		dFlatTreeInterface->Fill_Fundamental<Float_t>(locBranchPrefix + "_chisq_time", locChargedTrackHypothesis->Get_ChiSq_Timing());
		dFlatTreeInterface->Fill_Fundamental<UInt_t>(locBranchPrefix + "_ndf_time", locChargedTrackHypothesis->Get_NDF_Timing());

		//tracking
		dFlatTreeInterface->Fill_Fundamental<UInt_t>(locBranchPrefix + "_ndf_trk", locChargedTrackHypothesis->Get_NDF_Tracking());
		dFlatTreeInterface->Fill_Fundamental<Float_t>(locBranchPrefix + "_chisq_trk", locChargedTrackHypothesis->Get_ChiSq_Tracking());
		dFlatTreeInterface->Fill_Fundamental<UInt_t>(locBranchPrefix + "_ndf_dedx", locChargedTrackHypothesis->Get_NDF_DCdEdx());
		dFlatTreeInterface->Fill_Fundamental<Float_t>(locBranchPrefix + "_chisq_dedx", locChargedTrackHypothesis->Get_ChiSq_DCdEdx());
		dFlatTreeInterface->Fill_Fundamental<Float_t>(locBranchPrefix + "_dedx_cdc", locChargedTrackHypothesis->Get_dEdx_CDC());
		dFlatTreeInterface->Fill_Fundamental<Float_t>(locBranchPrefix + "_dedx_fdc", locChargedTrackHypothesis->Get_dEdx_FDC());

        //hit energy
		dFlatTreeInterface->Fill_Fundamental<Float_t>(locBranchPrefix + "_dedx_tof", locChargedTrackHypothesis->Get_dEdx_TOF());
		dFlatTreeInterface->Fill_Fundamental<Float_t>(locBranchPrefix + "_dedx_st", locChargedTrackHypothesis->Get_dEdx_ST());
		dFlatTreeInterface->Fill_Fundamental<Float_t>(locBranchPrefix + "_ebcal", locChargedTrackHypothesis->Get_Energy_BCAL());
		dFlatTreeInterface->Fill_Fundamental<Float_t>(locBranchPrefix + "_eprebcal", locChargedTrackHypothesis->Get_Energy_BCALPreshower());
		dFlatTreeInterface->Fill_Fundamental<Float_t>(locBranchPrefix + "_efcal", locChargedTrackHypothesis->Get_Energy_FCAL());

		//shower matching
		dFlatTreeInterface->Fill_Fundamental<Float_t>(locBranchPrefix + "_bcal_delphi", locChargedTrackHypothesis->Get_TrackBCAL_DeltaPhi());
		dFlatTreeInterface->Fill_Fundamental<Float_t>(locBranchPrefix + "_bcal_delz", locChargedTrackHypothesis->Get_TrackBCAL_DeltaZ());
		dFlatTreeInterface->Fill_Fundamental<Float_t>(locBranchPrefix + "_fcal_doca", locChargedTrackHypothesis->Get_TrackFCAL_DOCA());

		return;
	}

	//handle detected neutral
	DNeutralParticleHypothesis* locNeutralParticleHypothesis = dynamic_cast<DNeutralParticleHypothesis*>(locParticle);
	if(locNeutralParticleHypothesis != NULL)
	{
		//identifiers
		dFlatTreeInterface->Fill_Fundamental<UInt_t>(locBranchPrefix + "_showid", locNeutralParticleHypothesis->Get_ID());

		//kinematics
        dFlatTreeInterface->Fill_TObject<TLorentzVector>(locBranchPrefix + "_x4_meas", locNeutralParticleHypothesis->Get_X4_Measured());
        dFlatTreeInterface->Fill_TObject<TLorentzVector>(locBranchPrefix + "_p4_meas", locNeutralParticleHypothesis->Get_P4_Measured());
        if(dTreeInterface->Get_Branch(locEventBranchPrefix + "__X4_KinFit") != NULL)
        	dFlatTreeInterface->Fill_TObject<TLorentzVector>(locBranchPrefix + "_x4_kin", locNeutralParticleHypothesis->Get_X4());
        if(dTreeInterface->Get_Branch(locEventBranchPrefix + "__P4_KinFit") != NULL)
        	dFlatTreeInterface->Fill_TObject<TLorentzVector>(locBranchPrefix + "_p4_kin", locNeutralParticleHypothesis->Get_P4());

        //mc
		if(locIsMCFlag)
		{
	        Int_t locThrownIndex = locNeutralParticleHypothesis->Get_ThrownIndex();
			if(locThrownIndex >= 0)
			{
				dThrownWrapper->Set_ArrayIndex(locThrownIndex);
				dFlatTreeInterface->Fill_Fundamental<Float_t>(locBranchPrefix + "_true_fom", dThrownWrapper->Get_MatchFOM());
				dFlatTreeInterface->Fill_TObject<TLorentzVector>(locBranchPrefix + "_x4_true", dThrownWrapper->Get_X4());
				dFlatTreeInterface->Fill_TObject<TLorentzVector>(locBranchPrefix + "_p4_true", dThrownWrapper->Get_P4());
			}
			else
			{
				dFlatTreeInterface->Fill_Fundamental<Float_t>(locBranchPrefix + "_true_fom", -1.0);
				dFlatTreeInterface->Fill_TObject<TLorentzVector>(locBranchPrefix + "_x4_true", TLorentzVector());
				dFlatTreeInterface->Fill_TObject<TLorentzVector>(locBranchPrefix + "_p4_true", TLorentzVector());
			}
		}

		//timing
		dFlatTreeInterface->Fill_Fundamental<Float_t>(locBranchPrefix + "_beta_time", locNeutralParticleHypothesis->Get_Beta_Timing());
		dFlatTreeInterface->Fill_Fundamental<Float_t>(locBranchPrefix + "_chisq_time", locNeutralParticleHypothesis->Get_ChiSq_Timing());
		dFlatTreeInterface->Fill_Fundamental<UInt_t>(locBranchPrefix + "_ndf_time", locNeutralParticleHypothesis->Get_NDF_Timing());

        //hit energy
		dFlatTreeInterface->Fill_Fundamental<Float_t>(locBranchPrefix + "_ebcal", locNeutralParticleHypothesis->Get_Energy_BCAL());
		dFlatTreeInterface->Fill_Fundamental<Float_t>(locBranchPrefix + "_eprebcal", locNeutralParticleHypothesis->Get_Energy_BCALPreshower());
		dFlatTreeInterface->Fill_Fundamental<Float_t>(locBranchPrefix + "_efcal", locNeutralParticleHypothesis->Get_Energy_FCAL());

		//shower matching
		dFlatTreeInterface->Fill_Fundamental<Float_t>(locBranchPrefix + "_bcal_delphi", locNeutralParticleHypothesis->Get_TrackBCAL_DeltaPhi());
		dFlatTreeInterface->Fill_Fundamental<Float_t>(locBranchPrefix + "_bcal_delz", locNeutralParticleHypothesis->Get_TrackBCAL_DeltaZ());
		dFlatTreeInterface->Fill_Fundamental<Float_t>(locBranchPrefix + "_fcal_doca", locNeutralParticleHypothesis->Get_TrackFCAL_DOCA());

		return;
	}

	//is a kinfit missing or decaying particle
	{
		//NEED TO ADD THROWN X4 & P4!!
		if(dTreeInterface->Get_Branch(locEventBranchPrefix + "__X4") != NULL)
			dFlatTreeInterface->Fill_TObject<TLorentzVector>(locBranchPrefix + "_x4", locParticle->Get_X4());
		if(dTreeInterface->Get_Branch(locEventBranchPrefix + "__P4_KinFit") != NULL)
			dFlatTreeInterface->Fill_TObject<TLorentzVector>(locBranchPrefix + "_p4_kin", locParticle->Get_P4());
	}
}

void DSelector::SetupAmpTools_FlatTree() {

	dFlatTreeInterface->Create_Branch_Fundamental<Float_t>("Weight");
	dFlatTreeInterface->Create_Branch_Fundamental<Float_t>("E_Beam");
	dFlatTreeInterface->Create_Branch_Fundamental<Float_t>("Px_Beam");
	dFlatTreeInterface->Create_Branch_Fundamental<Float_t>("Py_Beam");
	dFlatTreeInterface->Create_Branch_Fundamental<Float_t>("Pz_Beam");
	dFlatTreeInterface->Create_Branch_Fundamental<Int_t>("NumFinalState");
	dFlatTreeInterface->Create_Branch_FundamentalArray<Float_t>("E_FinalState","NumFinalState");
	dFlatTreeInterface->Create_Branch_FundamentalArray<Float_t>("Px_FinalState","NumFinalState");
	dFlatTreeInterface->Create_Branch_FundamentalArray<Float_t>("Py_FinalState","NumFinalState");
	dFlatTreeInterface->Create_Branch_FundamentalArray<Float_t>("Pz_FinalState","NumFinalState");

	return;
}

void DSelector::FillAmpTools_FlatTree(TLorentzVector locBeamP4, vector<TLorentzVector> locFinalStateP4) {

	dFlatTreeInterface->Fill_Fundamental<Float_t>("E_Beam", locBeamP4.E());
	dFlatTreeInterface->Fill_Fundamental<Float_t>("Px_Beam", locBeamP4.Px());
	dFlatTreeInterface->Fill_Fundamental<Float_t>("Py_Beam", locBeamP4.Py());
	dFlatTreeInterface->Fill_Fundamental<Float_t>("Pz_Beam", locBeamP4.Pz());
	dFlatTreeInterface->Fill_Fundamental<Int_t>("NumFinalState", (Int_t)locFinalStateP4.size());

	for(unsigned int j=0; j<locFinalStateP4.size(); j++) {
		dFlatTreeInterface->Fill_Fundamental<Float_t>("E_FinalState", locFinalStateP4[j].E(), j);
		dFlatTreeInterface->Fill_Fundamental<Float_t>("Px_FinalState", locFinalStateP4[j].Px(), j);
		dFlatTreeInterface->Fill_Fundamental<Float_t>("Py_FinalState", locFinalStateP4[j].Py(), j);
		dFlatTreeInterface->Fill_Fundamental<Float_t>("Pz_FinalState", locFinalStateP4[j].Pz(), j);
	}	

}
