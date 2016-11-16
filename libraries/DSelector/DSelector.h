#ifndef DSelector_h
#define DSelector_h

#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <set>
#include <string>

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>
#include <TSelector.h>
#include <TH1D.h>
#include <TH2D.h>

#include <TProofOutputFile.h>
#include <TProof.h>
#include <TProofServ.h>

#include <particleType.h>
#include <TLorentzVector.h>
#include <TClonesArray.h>

#include "DBeamParticle.h"
#include "DMCThrown.h"
#include "DChargedTrackHypothesis.h"
#include "DNeutralParticleHypothesis.h"
#include "DParticleCombo.h"
#include "DAnalysisUtilities.h"
#include "DAnalysisAction.h"

class DSelector : public TSelector
{
	public:

		// TSELECTOR METHODS
		DSelector(TTree* locTree = NULL);
		virtual ~DSelector(){}
		Int_t Version() const{return 2;}
		virtual void Init(TTree *locTree);
		virtual Bool_t Process(Long64_t locEntry);
		void SlaveTerminate(void);
		void Terminate(void);
		void FillOutputTree(void);

	protected:

		virtual void Finalize(void);

		bool dInitializedFlag;
		TString dOption;
		string dOutputFileName;
		string dOutputTreeFileName;

		//TREE INTERFACE
		DTreeInterface* dTreeInterface;

		//ANALYSIS UTILITIES
		DAnalysisUtilities dAnalysisUtilities;

		// TARGET INFORMATION
		TVector3 dTargetCenter;
		TLorentzVector dTargetP4;
		Particle_t dTargetPID;

		// The "arrays" below are single objects that WRAP OVER arrays

		// OBJECT ARRAYS: THROWN
		DBeamParticle* dThrownBeam;
		DMCThrown* dThrownWrapper;

		// OBJECT ARRAYS: RECONSTRUCTED
		DChargedTrackHypothesis* dChargedHypoWrapper;
		DNeutralParticleHypothesis* dNeutralHypoWrapper;
		DBeamParticle* dBeamWrapper;
		DParticleCombo* dComboWrapper;

		// EVENT DATA
		UInt_t Get_RunNumber(void) const;
		ULong64_t Get_EventNumber(void) const;
		UInt_t Get_L1TriggerBits(void) const;
		Bool_t Get_IsThrownTopology(void) const;
		Float_t Get_MCWeight(void) const;
		TLorentzVector Get_X4_Production(void) const;

		// ARRAY SIZES
		UInt_t Get_NumBeam(void) const;
		UInt_t Get_NumChargedHypos(void) const;
		UInt_t Get_NumNeutralHypos(void) const;
		UInt_t Get_NumCombos(void) const;
		UInt_t Get_NumThrown(void) const;

		// THROWN REACTION INFO
		ULong64_t Get_NumPIDThrown_FinalState(void) const;
		ULong64_t Get_PIDThrown_Decaying(void) const;
		vector<Particle_t> Get_ThrownDecayingPIDs(void) const;
		map<Particle_t, UInt_t> Get_NumFinalStateThrown(void) const;

		// CUSTOM DATA
		template <typename DType> DType Get_Fundamental(string locBranchName) const;
		template <typename DType> DType Get_Fundamental(string locBranchName, UInt_t locArrayIndex) const;
		template <typename DType> DType Get_TObject(string locBranchName) const;
		template <typename DType> DType Get_TObject(string locBranchName, UInt_t locArrayIndex) const;

		// ANALYSIS ACTIONS
		void Initialize_Actions(void);
		void Create_ComboSurvivalHists(void);
		void Reset_Actions_NewEvent(void);
		bool Execute_Actions(void);
		void Fill_NumCombosSurvivedHists(void);

		vector<DAnalysisAction*> dAnalysisActions;

	private:

		void Setup_Branches(void);
		void Setup_Target(void);
		void Setup_Output(void);
		void Create_Wrappers(void);
		void ReInitialize_Wrappers(void);

		TFile* dFile;
		TFile* dOutputTreeFile;
		TProofOutputFile* dProofFile;
		TProofOutputFile* dOutputTreeProofFile;
		Int_t dTreeNumber;

		// EVENT DATA
		UInt_t* dRunNumber;
		ULong64_t* dEventNumber;
		UInt_t* dL1TriggerBits;
		Float_t* dMCWeight; //only present if simulated data
		Bool_t* dIsThrownTopology; //only present if simulated data
		TLorentzVector* dX4_Production;

		// ARRAY SIZES
		UInt_t* dNumBeam;
		UInt_t* dNumChargedHypos;
		UInt_t* dNumNeutralHypos;
		UInt_t* dNumCombos;
		UInt_t* dNumThrown; //only present if simulated data

		// THROWN REACTION INFO //only present if simulated data
		ULong64_t* dNumPIDThrown_FinalState;
		//the # of thrown final-state particles (+ pi0) of each type (multiplexed in base 10)
			//types (in order from 10^0 -> 10^15): g, e+, e-, nu, mu+, mu-, pi0, pi+, pi-, KLong, K+, K-, n, p, p-bar, n-bar
			//e.g. particles decaying from final-state particles are NOT included (e.g. photons from pi0, muons from pions, etc.)
		//is sum of #-of-PID * 10^ParticleMultiplexPower() (defined in libraries/include/particleType.h)
		//ParticleMultiplexPower() returns a different power of 10 for each final-state PID type. 
		//A value of 9 should be interpreted as >= 9.  

		ULong64_t* dPIDThrown_Decaying;
		//the types of the thrown decaying particles in the event (multiplexed in base 2)
		//not the quantity of each, just whether or not they were present (1 or 0)
		//binary power of a PID is given by ParticleMultiplexPower() (defined in libraries/include/particleType.h)
		//types: most Particle_t's 

		TH1D* dHist_NumEventsSurvivedAction;
		TH2D* dHist_NumCombosSurvivedAction;
		TH1D* dHist_NumCombosSurvivedAction1D;
		vector<size_t> dNumCombosSurvivedAction;

	ClassDef(DSelector, 0);
};

/******************************************************************** CONSTRUCTOR *********************************************************************/

inline DSelector::DSelector(TTree* locTree) :
		dInitializedFlag(false), dOption(""), dOutputFileName(""), dOutputTreeFileName(""), dTreeInterface(NULL),
		dAnalysisUtilities(DAnalysisUtilities()), dTargetCenter(TVector3()), dTargetP4(TLorentzVector()), dTargetPID(Unknown),
		dThrownBeam(NULL), dThrownWrapper(NULL), dChargedHypoWrapper(NULL), dNeutralHypoWrapper(NULL),
		dBeamWrapper(NULL), dComboWrapper(NULL), dAnalysisActions(vector<DAnalysisAction*>()),
		dFile(NULL), dOutputTreeFile(NULL), dProofFile(NULL), dOutputTreeProofFile(NULL), dTreeNumber(0),
		dRunNumber(NULL), dEventNumber(NULL), dL1TriggerBits(NULL), dMCWeight(NULL), dIsThrownTopology(NULL), dX4_Production(NULL),
		dNumBeam(NULL), dNumChargedHypos(NULL), dNumNeutralHypos(NULL), dNumCombos(NULL), dNumThrown(NULL),
		dNumPIDThrown_FinalState(NULL), dPIDThrown_Decaying(NULL) {}

/****************************************************************** GET OBJECT DATA *******************************************************************/

// EVENT DATA
inline UInt_t DSelector::Get_RunNumber(void) const
{
	return *dRunNumber;
}

inline ULong64_t DSelector::Get_EventNumber(void) const
{
	return *dEventNumber;
}

inline UInt_t DSelector::Get_L1TriggerBits(void) const
{
	return *dL1TriggerBits;
}

inline Bool_t DSelector::Get_IsThrownTopology(void) const
{
	return ((dIsThrownTopology != NULL) ? *dIsThrownTopology : false);
}

inline Float_t DSelector::Get_MCWeight(void) const
{
	return ((dMCWeight != NULL) ? *dMCWeight : 0.0);
}

inline TLorentzVector DSelector::Get_X4_Production(void) const
{
	return ((dX4_Production != NULL) ? *dX4_Production : TLorentzVector());
}

// ARRAY SIZES
inline UInt_t DSelector::Get_NumBeam(void) const
{
	return *dNumBeam;
}

inline UInt_t DSelector::Get_NumChargedHypos(void) const
{
	return *dNumChargedHypos;
}

inline UInt_t DSelector::Get_NumNeutralHypos(void) const
{
	return *dNumNeutralHypos;
}

inline UInt_t DSelector::Get_NumCombos(void) const
{
	return *dNumCombos;
}

inline UInt_t DSelector::Get_NumThrown(void) const
{
	return ((dNumThrown != NULL) ? *dNumThrown : 0);
}

// THROWN REACTION INFO
inline ULong64_t DSelector::Get_NumPIDThrown_FinalState(void) const
{
	return ((dNumPIDThrown_FinalState != NULL) ? *dNumPIDThrown_FinalState : 0);
}

inline ULong64_t DSelector::Get_PIDThrown_Decaying(void) const
{
	return ((dPIDThrown_Decaying != NULL) ? *dPIDThrown_Decaying : 0);
}

// CUSTOM DATA
template <typename DType> inline DType DSelector::Get_Fundamental(string locBranchName) const
{
	return dTreeInterface->Get_Fundamental<DType>(locBranchName);
}

template <typename DType> inline DType DSelector::Get_Fundamental(string locBranchName, UInt_t locArrayIndex) const
{
	return dTreeInterface->Get_Fundamental<DType>(locBranchName, locArrayIndex);
}

template <typename DType> inline DType DSelector::Get_TObject(string locBranchName) const
{
	return dTreeInterface->Get_TObject<DType>(locBranchName);
}

template <typename DType> inline DType DSelector::Get_TObject(string locBranchName, UInt_t locArrayIndex) const
{
	return dTreeInterface->Get_TObject<DType>(locBranchName, locArrayIndex);
}

/***************************************************************** ANALYSIS ACTIONS *******************************************************************/

inline void DSelector::Initialize_Actions(void)
{
	for(size_t loc_i = 0; loc_i < dAnalysisActions.size(); ++loc_i)
		dAnalysisActions[loc_i]->Initialize();

	Create_ComboSurvivalHists();
}

inline void DSelector::Reset_Actions_NewEvent(void)
{
	dNumCombosSurvivedAction[0] = 0;
	for(size_t loc_i = 0; loc_i < dAnalysisActions.size(); ++loc_i)
	{
		dAnalysisActions[loc_i]->Reset_NewEvent();
		dNumCombosSurvivedAction[loc_i + 1] = 0;
	}
}

inline bool DSelector::Execute_Actions(void)
{
	++dNumCombosSurvivedAction[0];
	for(size_t loc_i = 0; loc_i < dAnalysisActions.size(); ++loc_i)
	{
		if(dAnalysisActions[loc_i]->Perform_Action())
			++dNumCombosSurvivedAction[loc_i + 1];
		else
		{
			dComboWrapper->Set_IsComboCut(true);
			return false;
		}

	}

	return true;
}

inline void DSelector::Fill_NumCombosSurvivedHists(void)
{
	if(dNumCombosSurvivedAction[0] == 0)
		return;

	for(size_t loc_i = 0; loc_i < dNumCombosSurvivedAction.size(); ++loc_i)
	{
		if(dNumCombosSurvivedAction[loc_i] > 0)
			dHist_NumEventsSurvivedAction->Fill(loc_i);

		dHist_NumCombosSurvivedAction->Fill(loc_i, dNumCombosSurvivedAction[loc_i]);

		Double_t locNum1DCombos = dHist_NumCombosSurvivedAction1D->GetBinContent(loc_i + 1) + dNumCombosSurvivedAction[loc_i];
		dHist_NumCombosSurvivedAction1D->SetBinContent(loc_i + 1, locNum1DCombos);
	}
}

#endif // #ifdef DSelector_h
