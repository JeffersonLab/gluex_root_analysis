#include "DSelector.h"

/******************************************************************** INITIALIZE **********************************************************************/

void DSelector::Init(TTree *locTree)
{
	// The Init() function is called when the selector needs to initialize a new tree or chain.
	// Typically here the branch addresses and branch pointers of the tree will be set.
	// Init() will be called many times when running on PROOF (once per file to be processed).

	// GET OPTION
	dOption = GetOption(); //optional argument given to TTree::Process()
	if(fInput != NULL) 
		dOption = ((TNamed*)fInput->FindObject("OPTIONS"))->GetTitle();

	// SETUP OUTPUT
	// This must be done BEFORE initializing the DTreeInterface, etc. Why? I have no idea. Probably something to do with gDirectory changing.
	if(!dInitializedFlag)
		Setup_Output();

	// SETUP TREE
	locTree->GetDirectory()->cd();
	locTree->SetMakeClass(1);
	if(dTreeInterface != NULL)
		delete dTreeInterface;
	dTreeInterface = new DTreeInterface(locTree);

	//This is necessary to compensate for one of ROOT's ... idiosyncrasies.
	//Some branches apparently don't load correctly if the gDirectory(?) is changed between initializing the tree and reading the first entry.
	dTreeInterface->Get_Entry(0);

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
	if(dOutputTreeFile != NULL) {
		dOutputTreeFile->cd();
		dTreeInterface->CloneTree();
	}

	//Change back to file dir, if it exists //So that user-created histograms are in the right location
	if(dFile != NULL) {
		dFile->cd();
	}

	

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

	// MC-ONLY
	if(locIsMCFlag)
	{
		dMCWeight = (Float_t*)dTreeInterface->Get_Branch("MCWeight")->GetAddress();
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
		dThrownBeam->ReInitialize(dTreeInterface);
		dThrownWrapper->ReInitialize(dTreeInterface);
	}

	if(dNumCombos != NULL) //detected
	{
		dChargedHypoWrapper->ReInitialize(dTreeInterface);
		dNeutralHypoWrapper->ReInitialize(dTreeInterface);
		dBeamWrapper->ReInitialize(dTreeInterface);
		dComboWrapper->ReInitialize(dTreeInterface);
	}
}

void DSelector::Setup_Output(void)
{
	//Create output files (if desired)
	if(fInput != NULL)
	{
		TNamed* locOutputFileObject = (TNamed*)fInput->FindObject("OUTPUT_FILENAME");
		if(locOutputFileObject != NULL) 
			dOutputFileName = locOutputFileObject->GetTitle();

		TNamed* locOutputTreeFileObject = (TNamed*)fInput->FindObject("OUTPUT_TREE_FILENAME");
		if(locOutputTreeFileObject != NULL) 
			dOutputTreeFileName = locOutputTreeFileObject->GetTitle();
	}

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

	if(dOutputTreeFileName != "")
	{
		if(gProofServ == NULL)
			dOutputTreeFile = new TFile(dOutputTreeFileName.c_str(), "RECREATE");
		else
		{
			dOutputTreeProofFile = new TProofOutputFile(dOutputTreeFileName.c_str(), TProofOutputFile::kMerge);
			dOutputTreeFile = dOutputTreeProofFile->OpenFile("RECREATE");
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

	dTreeInterface->Get_Entry(locEntry);

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
	if(dFile != NULL)
	{
		dFile->Write();
		dFile->Close();
	}
	if(dOutputTreeFile != NULL)
	{
		dOutputTreeFile->Write(0, TObject::kOverwrite);
		dOutputTreeFile->Close();
	}
	if(dProofFile != NULL)
		fOutput->Add(dProofFile);
	if(dOutputTreeProofFile != NULL)
		fOutput->Add(dOutputTreeProofFile);
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

void DSelector::FillOutputTree()
{
        // The FillOutputTree() function is called for events in the tree which pass
	// the user-defined analysis cuts.  The output file then contains the TTree 
	// with these events that can be used for higher-level analysis (eg. AmptTools fit,
	// PWA, etc.)

        dTreeInterface->FillOutputTree();
}
