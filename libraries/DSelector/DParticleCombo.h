#ifndef _DParticleCombo_
#define _DParticleCombo_

#include <deque>
#include <string>
#include <map>
#include <iostream>
#include <sstream>

#include "TObjString.h"
#include "TList.h"
#include "TMath.h"
#include "TMap.h"

#include "particleType.h"

#include "DTreeInterface.h"
#include "DParticleComboStep.h"
#include "DBeamParticle.h"
#include "DKinematicData.h"
#include "DChargedTrackHypothesis.h"
#include "DNeutralParticleHypothesis.h"

using namespace std;

class DSelector;

class DParticleCombo
{
	friend class DSelector; //for calling ReInitialize

	public:

		// CONSTRUCTOR:
		DParticleCombo(DTreeInterface* locTreeInterface);

		void Set_ComboIndex(UInt_t locComboIndex);
		UInt_t Get_ComboIndex(void) const{return dComboIndex;}
		UInt_t Get_NumCombos(void) const;
		void Print_Reaction(void) const;

		// STEPS:
		size_t Get_NumParticleComboSteps(void) const{return dParticleComboSteps.size();}
		DParticleComboStep* Get_ParticleComboStep(size_t locStepIndex) const{return dParticleComboSteps[locStepIndex];}

		//MISSING:
		DParticleComboStep* Get_MissingParticleStep(void) const; //NULL if none
		DKinematicData* Get_MissingParticle(void) const; //NULL if none or not reconstructed

		// COMBO INFO:
		Bool_t Get_IsComboCut(void) const;
		Bool_t Get_IsTrueCombo(void) const;
		Bool_t Get_IsBDTSignalCombo(void) const;
		void Set_IsComboCut(Bool_t locIsComboCut) const;

		// RF: //Quoted at the center of the target
		Float_t Get_RFTime_Measured(void) const;
		Float_t Get_RFTime(void) const; //kinfit if spacetime fit, else measured

		// TARGET
		Particle_t Get_TargetPID(void) const;
		TVector3 Get_TargetCenter(void) const;

		// KINFIT:
		Float_t Get_ChiSq_KinFit(string branch_tag) const;
		UInt_t Get_NDF_KinFit(string branch_tag) const;
		Float_t Get_ConfidenceLevel_KinFit(string branch_tag ) const;

		// UNUSED ENERGY:
		Float_t Get_Energy_UnusedShowers(void) const;

		// UNUSED TRACKS:
		UChar_t Get_NumUnusedTracks(void) const;

		// EVENT INFO: //Doesn't really belong in DParticleCombo, but much easier to pass into actions this way
		UInt_t Get_RunNumber(void) const;
		ULong64_t Get_EventNumber(void) const;
		UInt_t Get_L1TriggerBits(void) const;
		Bool_t Get_IsThrownTopology(void) const;
		Float_t Get_MCWeight(void) const;

		//GET CUSTOM DATA
		template <typename DType> DType Get_Fundamental(string locBranchName) const;
		template <typename DType> DType Get_TObject(string locBranchName) const;

		//UTILITY FUNCTIONS
		string Get_InitialParticlesROOTName(void) const;
		string Get_DecayChainFinalParticlesROOTNames(Particle_t locInitialPID, bool locKinFitResultsFlag) const;
		string Get_DecayChainFinalParticlesROOTNames(Particle_t locInitialPID, int locUpToStepIndex, deque<Particle_t> locUpThroughPIDs, bool locKinFitResultsFlag) const;
		string Get_DecayChainFinalParticlesROOTNames(size_t locStepIndex, int locUpToStepIndex, deque<Particle_t> locUpThroughPIDs, bool locKinFitResultsFlag, bool locExpandDecayingParticlesFlag) const;

		string Get_KinFitConstraints(void) const{return dKinFitConstraints;}
		size_t Get_NumKinFitConstraints(void) const{return dNumKinFitConstraints;}
		size_t Get_NumKinFitUnknowns(void) const{return dNumKinFitUnknowns;}

	private:
		DParticleCombo(void){}; //no default constructor!

		//SETUP METHODS
		void Setup_Branches(void);
		int Get_DecayStepIndex(int locStepIndex, int locParticleIndex) const;
		pair<int, int> Get_DecayFromIndices(int locStepIndex) const;
		void Setup_X4Branches(void);
		void Setup_EventBranches(void);
		void Setup_KinFitConstraintInfo(void);

		//RE-INITIALIZE (e.g. with the next TTree in a chain)
		void ReInitialize(void);

		DTreeInterface* dTreeInterface;
		UInt_t* dNumCombos;
		UChar_t* dNumUnusedTracks;
		UInt_t dComboIndex; //the index in the particle-data arrays to use to grab particle data (e.g. corresponding to this combo)

		deque<DParticleComboStep*> dParticleComboSteps;

		//For fundamental arrays
			//cannot store array pointer: when array size is increased, a new array is created
			//must store branch pointer instead

		TBranch* dBranch_IsComboCut; //if true, combo has been previously cut (all kFALSE originally, user can apply cuts in TSelector, change this flag, and output new TTree)
		TBranch* dBranch_IsTrueCombo; //"IsThrownTopology" = kTRUE, each particle has the right PID, and the combo particle chain matches the thrown decay chain
		TBranch* dBranch_IsBDTSignalCombo; //Similar to "IsTrueCombo", except other thrown topologies that decay to the DReaction topology are marked as signal

		TBranch* dBranch_RFTime_Measured;
		TBranch* dBranch_RFTime_KinFit; //only if spacetime kinematic fit performed

		TBranch* dBranch_Energy_UnusedShowers;

		//Target not necessarily in the particle combo, so add the info here (for convenience)
		Particle_t dTargetPID;
		TVector3 dTargetCenter;

		//Kinfit constraint info
		string dKinFitConstraints;
		size_t dNumKinFitConstraints;
		size_t dNumKinFitUnknowns;

		// EVENT DATA
		UInt_t* dRunNumber;
		ULong64_t* dEventNumber;
		UInt_t* dL1TriggerBits;
		Float_t* dMCWeight; //only present if simulated data
		Bool_t* dIsThrownTopology; //only present if simulated data
};

inline void DParticleCombo::Print_Reaction(void) const
{
	cout << "Tree reaction:" << endl;
	for(size_t loc_i = 0; loc_i < dParticleComboSteps.size(); ++loc_i)
		dParticleComboSteps[loc_i]->Print_Reaction();
}

/***************************************************************** SETUP DATA ACCESS ******************************************************************/

inline void DParticleCombo::Setup_Branches(void)
{
	// COMBO INFO:
	dNumCombos = (UInt_t*)dTreeInterface->Get_Branch("NumCombos")->GetAddress();
	dBranch_IsComboCut = dTreeInterface->Get_Branch("IsComboCut");
	dBranch_IsTrueCombo = dTreeInterface->Get_Branch("IsTrueCombo");
	dBranch_IsBDTSignalCombo = dTreeInterface->Get_Branch("IsBDTSignalCombo");

	// RF:
	dBranch_RFTime_Measured = dTreeInterface->Get_Branch("RFTime_Measured");
	dBranch_RFTime_KinFit = dTreeInterface->Get_Branch("RFTime_KinFit");

	// UNUSED ENERGY:
	dBranch_Energy_UnusedShowers = dTreeInterface->Get_Branch("Energy_UnusedShowers");

	// UNUSED TRACKS:
	dNumUnusedTracks = (UChar_t*)dTreeInterface->Get_Branch("NumUnusedTracks")->GetAddress();
}

inline UInt_t DParticleCombo::Get_NumCombos(void) const
{
	return *dNumCombos;
}

inline void DParticleCombo::Set_ComboIndex(UInt_t locComboIndex)
{
	dComboIndex = locComboIndex;
	for(size_t loc_i = 0; loc_i < dParticleComboSteps.size(); ++loc_i)
		dParticleComboSteps[loc_i]->Set_ComboIndex(locComboIndex);
}

inline void DParticleCombo::ReInitialize(void)
{
	Setup_Branches();
	for(size_t loc_i = 0; loc_i < dParticleComboSteps.size(); ++loc_i)
		dParticleComboSteps[loc_i]->ReInitialize();
	Setup_X4Branches();
}

/****************************************************************** GET OBJECT DATA *******************************************************************/

// COMBO INFO:
inline Bool_t DParticleCombo::Get_IsComboCut(void) const
{
	return ((Bool_t*)dBranch_IsComboCut->GetAddress())[dComboIndex];
}

inline Bool_t DParticleCombo::Get_IsTrueCombo(void) const
{
	return ((Bool_t*)dBranch_IsTrueCombo->GetAddress())[dComboIndex];
}

inline Bool_t DParticleCombo::Get_IsBDTSignalCombo(void) const
{
	return ((Bool_t*)dBranch_IsBDTSignalCombo->GetAddress())[dComboIndex];
}

inline void DParticleCombo::Set_IsComboCut(Bool_t locIsComboCut) const
{
	dBranch_IsComboCut->GetAddress()[dComboIndex] = locIsComboCut;
	return;
}

// RF:
inline Float_t DParticleCombo::Get_RFTime_Measured(void) const
{
	return ((Float_t*)dBranch_RFTime_Measured->GetAddress())[dComboIndex];
}

inline Float_t DParticleCombo::Get_RFTime(void) const
{
	if(dBranch_RFTime_KinFit != NULL)
		return ((Float_t*)dBranch_RFTime_KinFit->GetAddress())[dComboIndex];
	return Get_RFTime_Measured();
}

//TARGET:
inline Particle_t DParticleCombo::Get_TargetPID(void) const
{
	return dTargetPID;
}

inline TVector3 DParticleCombo::Get_TargetCenter(void) const
{
	return dTargetCenter;
}

// KINFIT:
inline Float_t DParticleCombo::Get_ChiSq_KinFit(string branch_tag) const
{
	TBranch* branch;
	string b_name = "ChiSq_KinFit";
	if ( branch_tag != "" )
		b_name += "_" + branch_tag;
	branch = dTreeInterface->Get_Branch( b_name );
	if ( branch == NULL )
		return -1.0;
	return ((Float_t*)branch->GetAddress())[dComboIndex];
}

inline UInt_t DParticleCombo::Get_NDF_KinFit(string branch_tag) const
{
	TBranch* branch;
	string b_name = "NDF_KinFit";
	if ( branch_tag != "" )
		b_name += "_" + branch_tag;
	branch = dTreeInterface->Get_Branch( b_name );
	if((branch == NULL) || (dKinFitConstraints != "NA"))
		return dNumKinFitConstraints - dNumKinFitUnknowns;
	return ((UInt_t*)branch->GetAddress())[dComboIndex];
}

inline Float_t DParticleCombo::Get_ConfidenceLevel_KinFit( string branch_tag ) const
{
	Float_t locChiSq = Get_ChiSq_KinFit( branch_tag );
	if(locChiSq < 0.0)
		return -1.0;
	UInt_t locNDF = Get_NDF_KinFit( branch_tag );
	return TMath::Prob(locChiSq, locNDF);
}

// UNUSED ENERGY:
inline Float_t DParticleCombo::Get_Energy_UnusedShowers(void) const
{
	if(dBranch_Energy_UnusedShowers == NULL)
		return -1.0;
	return ((Float_t*)dBranch_Energy_UnusedShowers->GetAddress())[dComboIndex];
}

// UNUSED TRACKS:
inline UChar_t DParticleCombo::Get_NumUnusedTracks(void) const
{
	return *dNumUnusedTracks;
}

// MISSING
inline DKinematicData* DParticleCombo::Get_MissingParticle(void) const
{
	DParticleComboStep* locMissingStep = Get_MissingParticleStep();
	if(locMissingStep == NULL)
		return NULL;
	return locMissingStep->Get_MissingParticle();
}

inline DParticleComboStep* DParticleCombo::Get_MissingParticleStep(void) const
{
	for(size_t loc_i = 0; loc_i < dParticleComboSteps.size(); ++loc_i)
	{
		int locMissingParticleIndex = dParticleComboSteps[loc_i]->Get_MissingParticleIndex();
		if(locMissingParticleIndex != 2)
			return dParticleComboSteps[loc_i];
	}
	return NULL;
}

// EVENT DATA
inline UInt_t DParticleCombo::Get_RunNumber(void) const
{
	return *dRunNumber;
}

inline ULong64_t DParticleCombo::Get_EventNumber(void) const
{
	return *dEventNumber;
}

inline UInt_t DParticleCombo::Get_L1TriggerBits(void) const
{
	return *dL1TriggerBits;
}

inline Bool_t DParticleCombo::Get_IsThrownTopology(void) const
{
	return ((dIsThrownTopology != NULL) ? *dIsThrownTopology : false);
}

inline Float_t DParticleCombo::Get_MCWeight(void) const
{
	return ((dMCWeight != NULL) ? *dMCWeight : 0.0);
}

//GET CUSTOM DATA
template <typename DType> inline DType DParticleCombo::Get_Fundamental(string locBranchName) const
{
	return dTreeInterface->Get_Fundamental<DType>(locBranchName, dComboIndex);
}

template <typename DType> inline DType DParticleCombo::Get_TObject(string locBranchName) const
{
	return dTreeInterface->Get_Fundamental<DType>(locBranchName, dComboIndex);
}

/***************************************************************** UTILITY FUNCTIONS ******************************************************************/

inline string DParticleCombo::Get_DecayChainFinalParticlesROOTNames(Particle_t locInitialPID, bool locKinFitResultsFlag) const
{
	//if multiple decay steps have locInitialPID as the parent, only the first listed is used
	return Get_DecayChainFinalParticlesROOTNames(locInitialPID, -1, deque<Particle_t>(), locKinFitResultsFlag);
}

inline string DParticleCombo::Get_DecayChainFinalParticlesROOTNames(Particle_t locInitialPID, int locUpToStepIndex, deque<Particle_t> locUpThroughPIDs, bool locKinFitResultsFlag) const
{
	//if multiple decay steps have locInitialPID as the parent, only the first listed is used
	deque<Particle_t> locPIDs;
	string locName = "";
	for(size_t loc_i = 0; loc_i < dParticleComboSteps.size(); ++loc_i)
	{
		if(dParticleComboSteps[loc_i]->Get_InitialPID() != locInitialPID)
			continue;
		return Get_DecayChainFinalParticlesROOTNames(loc_i, locUpToStepIndex, locUpThroughPIDs, locKinFitResultsFlag, false);
	}
	return string("");
}

inline string DParticleCombo::Get_InitialParticlesROOTName(void) const
{
	if(dParticleComboSteps.empty())
		return (string());
	return dParticleComboSteps[0]->Get_InitialParticlesROOTName();
}

#endif // _DParticleCombo_

