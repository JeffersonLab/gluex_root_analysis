#ifndef DChargedTrackHypothesis_h
#define DChargedTrackHypothesis_h

#include <string>

#include "TBranch.h"

#include <particleType.h>
#include <GlueX.h>

#include "DTreeInterface.h"
#include "DKinematicData.h"

using namespace std;

class DSelector;
class DParticleComboStep;

//https://halldweb1.jlab.org/wiki/index.php/Analysis_TTreeFormat
class DChargedTrackHypothesis : public DKinematicData
{
	friend class DSelector; //for calling ReInitialize
	friend class DParticleComboStep; //for calling ReInitialize

	public:

		DChargedTrackHypothesis(DTreeInterface* locTreeInterface, string locBranchNamePrefix, Particle_t locPID = Unknown);
		~DChargedTrackHypothesis(void){}

		//IDENTIFIERS / MATCHING
		Int_t Get_TrackID(void) const; //each physical particle has its own # (to keep track of different pid hypotheses for the same particle)
		Int_t Get_ThrownIndex(void) const; //the array index of the thrown particle it is matched with (-1 for no match) //only present if simulated data
		Int_t Get_ID(void) const{return Get_TrackID();}

		//TRACKING INFO
		UInt_t Get_NDF_Tracking(void) const;
		Float_t Get_ChiSq_Tracking(void) const;
		UInt_t Get_NDF_DCdEdx(void) const;
		Float_t Get_ChiSq_DCdEdx(void) const;
		Float_t Get_dEdx_CDC(void) const;
		Float_t Get_dEdx_FDC(void) const;

		//TIMING INFO
		Float_t Get_HitTime(void) const; //the system that is hit is in order of preference: BCAL/TOF/FCAL/ST 
			//to determine which, look whether energy was deposited in these systems
		Float_t Get_RFDeltaTVar(void) const; //Variance of X4_Measured.T() - RFTime, regardless of which RF bunch is chosen. 
			//Can be used to compute timing ChiSq //RF bunch is combo-dependent
		UInt_t Get_NDF_Timing(void) const;
		Float_t Get_Beta_Timing(void) const; //is kinfit if kinfit, else is measured
		Float_t Get_ChiSq_Timing(void) const; //is kinfit if kinfit, else is measured
		Float_t Get_ConfidenceLevel_Timing(void) const; //is kinfit if kinfit, else is measured
		Float_t Get_Beta_Timing_Measured(void) const;
		Float_t Get_ChiSq_Timing_Measured(void) const;
		Float_t Get_ConfidenceLevel_Timing_Measured(void) const; //is kinfit if kinfit, else is measured
		DetectorSystem_t Get_Detector_System_Timing(void) const;

		//HIT ENERGY
		Float_t Get_dEdx_TOF(void) const;
		Float_t Get_dEdx_ST(void) const;
		Float_t Get_Energy_BCAL(void) const;
		Float_t Get_Energy_BCALPreshower(void) const;
		Float_t Get_Energy_FCAL(void) const;

		//SHOWER MATCHING
		Float_t Get_TrackBCAL_DeltaPhi(void) const; //999.0 if not matched //units are radians
		Float_t Get_TrackBCAL_DeltaZ(void) const; //999.0 if not matched
		Float_t Get_TrackFCAL_DOCA(void) const; //999.0 if not matched

	private:
		DChargedTrackHypothesis(void); //Cannot call default constructor!

		void Setup_Branches(void);

		//RE-INITIALIZE (e.g. with the next TTree in a chain)
		void ReInitialize(void);

		//For fundamental arrays
			//cannot store array pointer: when array size is increased, a new array is created
			//must store branch pointer instead

		//IDENTIFIERS / MATCHING
		TBranch* dBranch_TrackID;
		TBranch* dBranch_ThrownIndex;

		//TRACKING INFO
		TBranch* dBranch_NDF_Tracking;
		TBranch* dBranch_ChiSq_Tracking;
		TBranch* dBranch_NDF_DCdEdx;
		TBranch* dBranch_ChiSq_DCdEdx;
		TBranch* dBranch_dEdx_CDC;
		TBranch* dBranch_dEdx_FDC;
 
		//TIMING INFO
		TBranch* dBranch_HitTime;
		TBranch* dBranch_RFDeltaTVar;

		TBranch* dBranch_NDF_Timing;
		TBranch* dBranch_Beta_Timing_KinFit;
		TBranch* dBranch_ChiSq_Timing_KinFit;
		TBranch* dBranch_Beta_Timing_Measured;
		TBranch* dBranch_ChiSq_Timing_Measured;
 
		//HIT ENERGY
		TBranch* dBranch_dEdx_TOF;
		TBranch* dBranch_dEdx_ST;
		TBranch* dBranch_Energy_BCAL;
		TBranch* dBranch_Energy_BCALPreshower;
		TBranch* dBranch_Energy_FCAL;
 
		//SHOWER MATCHING
		TBranch* dBranch_TrackBCAL_DeltaPhi;
		TBranch* dBranch_TrackBCAL_DeltaZ;
		TBranch* dBranch_TrackFCAL_DOCA;
};

/******************************************************************** CONSTRUCTOR *********************************************************************/

inline DChargedTrackHypothesis::DChargedTrackHypothesis(DTreeInterface* locTreeInterface, string locBranchNamePrefix, Particle_t locPID) : 
DKinematicData(locTreeInterface, locBranchNamePrefix, locPID)
{
	Setup_Branches();
}

/***************************************************************** SETUP DATA ACCESS ******************************************************************/

inline void DChargedTrackHypothesis::Setup_Branches(void)
{
	string locBranchName;

	//IDENTIFIERS / MATCHING
	locBranchName = "ChargedHypo__TrackID";
	dBranch_TrackID = dTreeInterface->Get_Branch(locBranchName);

	locBranchName = "ChargedHypo__ThrownIndex";
	dBranch_ThrownIndex = dTreeInterface->Get_Branch(locBranchName);

	//TRACKING INFO
	locBranchName = "ChargedHypo__NDF_Tracking";
	dBranch_NDF_Tracking = dTreeInterface->Get_Branch(locBranchName);

	locBranchName = "ChargedHypo__ChiSq_Tracking";
	dBranch_ChiSq_Tracking = dTreeInterface->Get_Branch(locBranchName);

	locBranchName = "ChargedHypo__NDF_DCdEdx";
	dBranch_NDF_DCdEdx = dTreeInterface->Get_Branch(locBranchName);

	locBranchName = "ChargedHypo__ChiSq_DCdEdx";
	dBranch_ChiSq_DCdEdx = dTreeInterface->Get_Branch(locBranchName);

	locBranchName = "ChargedHypo__dEdx_CDC";
	dBranch_dEdx_CDC = dTreeInterface->Get_Branch(locBranchName);

	locBranchName = "ChargedHypo__dEdx_FDC";
	dBranch_dEdx_FDC = dTreeInterface->Get_Branch(locBranchName);

	//TIMING INFO
	locBranchName = "ChargedHypo__HitTime";
	dBranch_HitTime = dTreeInterface->Get_Branch(locBranchName);

	locBranchName = "ChargedHypo__RFDeltaTVar";
	dBranch_RFDeltaTVar = dTreeInterface->Get_Branch(locBranchName);

	locBranchName = "ChargedHypo__NDF_Timing";
	dBranch_NDF_Timing = dTreeInterface->Get_Branch(locBranchName);

	locBranchName = dBranchNamePrefix + string("__Beta_Timing_KinFit");
	dBranch_Beta_Timing_KinFit = dTreeInterface->Get_Branch(locBranchName);

	locBranchName = dBranchNamePrefix + string("__ChiSq_Timing_KinFit");
	dBranch_ChiSq_Timing_KinFit = dTreeInterface->Get_Branch(locBranchName);

	if(dBranchNamePrefix != "ChargedHypo")
	{
		locBranchName = dBranchNamePrefix + string("__Beta_Timing_Measured");
		dBranch_Beta_Timing_Measured = dTreeInterface->Get_Branch(locBranchName);
		if(dBranch_Beta_Timing_Measured == NULL) //for backwards compatibility
			dBranch_Beta_Timing_Measured = dTreeInterface->Get_Branch("ChargedHypo__Beta_Timing");
	}
	else
		dBranch_Beta_Timing_Measured = dTreeInterface->Get_Branch("ChargedHypo__Beta_Timing");

	if(dBranchNamePrefix != "ChargedHypo")
	{
		locBranchName = dBranchNamePrefix + string("__ChiSq_Timing_Measured");
		dBranch_ChiSq_Timing_Measured = dTreeInterface->Get_Branch(locBranchName);
		if(dBranch_ChiSq_Timing_Measured == NULL) //for backwards compatibility
			dBranch_ChiSq_Timing_Measured = dTreeInterface->Get_Branch("ChargedHypo__ChiSq_Timing");
	}
	else
		dBranch_ChiSq_Timing_Measured = dTreeInterface->Get_Branch("ChargedHypo__ChiSq_Timing");

	//HIT ENERGY
	locBranchName = "ChargedHypo__dEdx_TOF";
	dBranch_dEdx_TOF = dTreeInterface->Get_Branch(locBranchName);

	locBranchName = "ChargedHypo__dEdx_ST";
	dBranch_dEdx_ST = dTreeInterface->Get_Branch(locBranchName);

	locBranchName = "ChargedHypo__Energy_BCAL";
	dBranch_Energy_BCAL = dTreeInterface->Get_Branch(locBranchName);

	locBranchName = "ChargedHypo__Energy_BCALPreshower";
	dBranch_Energy_BCALPreshower = dTreeInterface->Get_Branch(locBranchName);

	locBranchName = "ChargedHypo__Energy_FCAL";
	dBranch_Energy_FCAL = dTreeInterface->Get_Branch(locBranchName);

	//SHOWER MATCHING:
	locBranchName = "ChargedHypo__TrackBCAL_DeltaPhi";
	dBranch_TrackBCAL_DeltaPhi = dTreeInterface->Get_Branch(locBranchName);

	locBranchName = "ChargedHypo__TrackBCAL_DeltaZ";
	dBranch_TrackBCAL_DeltaZ = dTreeInterface->Get_Branch(locBranchName);

	locBranchName = "ChargedHypo__TrackFCAL_DOCA";
	dBranch_TrackFCAL_DOCA = dTreeInterface->Get_Branch(locBranchName);
}

inline void DChargedTrackHypothesis::ReInitialize(void)
{
	DKinematicData::ReInitialize();
	Setup_Branches();
}

/****************************************************************** GET OBJECT DATA *******************************************************************/

//IDENTIFIERS / MATCHING
inline Int_t DChargedTrackHypothesis::Get_TrackID(void) const
{
	return ((Int_t*)dBranch_TrackID->GetAddress())[dMeasuredArrayIndex];
}

inline Int_t DChargedTrackHypothesis::Get_ThrownIndex(void) const
{
	return ((Int_t*)dBranch_ThrownIndex->GetAddress())[dMeasuredArrayIndex];
}

//TRACKING INFO
inline UInt_t DChargedTrackHypothesis::Get_NDF_Tracking(void) const
{
	return ((UInt_t*)dBranch_NDF_Tracking->GetAddress())[dMeasuredArrayIndex];
}

inline Float_t DChargedTrackHypothesis::Get_ChiSq_Tracking(void) const
{
	return ((Float_t*)dBranch_ChiSq_Tracking->GetAddress())[dMeasuredArrayIndex];
}

inline UInt_t DChargedTrackHypothesis::Get_NDF_DCdEdx(void) const
{
	return ((UInt_t*)dBranch_NDF_DCdEdx->GetAddress())[dMeasuredArrayIndex];
}

inline Float_t DChargedTrackHypothesis::Get_ChiSq_DCdEdx(void) const
{
	return ((Float_t*)dBranch_ChiSq_DCdEdx->GetAddress())[dMeasuredArrayIndex];
}

inline Float_t DChargedTrackHypothesis::Get_dEdx_CDC(void) const
{
	return ((Float_t*)dBranch_dEdx_CDC->GetAddress())[dMeasuredArrayIndex];
}

inline Float_t DChargedTrackHypothesis::Get_dEdx_FDC(void) const
{
	return ((Float_t*)dBranch_dEdx_FDC->GetAddress())[dMeasuredArrayIndex];
}

//TIMING INFO
inline Float_t DChargedTrackHypothesis::Get_HitTime(void) const
{
	return ((Float_t*)dBranch_HitTime->GetAddress())[dMeasuredArrayIndex];
}

inline Float_t DChargedTrackHypothesis::Get_RFDeltaTVar(void) const
{
	return ((Float_t*)dBranch_RFDeltaTVar->GetAddress())[dMeasuredArrayIndex];
}

inline UInt_t DChargedTrackHypothesis::Get_NDF_Timing(void) const
{
	return ((UInt_t*)dBranch_NDF_Timing->GetAddress())[dMeasuredArrayIndex];
}

inline Float_t DChargedTrackHypothesis::Get_Beta_Timing_Measured(void) const
{
	string locBranchName = dBranchNamePrefix + string("__Beta_Timing_Measured"); //for backwards compatibility
	int locArrayIndex = (dTreeInterface->Get_Branch(locBranchName) != NULL) ? dArrayIndex : dMeasuredArrayIndex;
	return ((Float_t*)dBranch_Beta_Timing_Measured->GetAddress())[locArrayIndex];
}

inline Float_t DChargedTrackHypothesis::Get_ChiSq_Timing_Measured(void) const
{
	string locBranchName = dBranchNamePrefix + string("__ChiSq_Timing_Measured"); //for backwards compatibility
	int locArrayIndex = (dTreeInterface->Get_Branch(locBranchName) != NULL) ? dArrayIndex : dMeasuredArrayIndex;
	return ((Float_t*)dBranch_ChiSq_Timing_Measured->GetAddress())[locArrayIndex];
}

inline Float_t DChargedTrackHypothesis::Get_Beta_Timing(void) const
{
	if(dBranch_Beta_Timing_KinFit != NULL)
		return ((Float_t*)dBranch_Beta_Timing_KinFit->GetAddress())[dArrayIndex];
	return Get_Beta_Timing_Measured();
}

inline Float_t DChargedTrackHypothesis::Get_ChiSq_Timing(void) const
{
	if(dBranch_ChiSq_Timing_KinFit != NULL)
		return ((Float_t*)dBranch_ChiSq_Timing_KinFit->GetAddress())[dArrayIndex];
	return Get_ChiSq_Timing_Measured();
}

inline Float_t DChargedTrackHypothesis::Get_ConfidenceLevel_Timing(void) const
{
	UInt_t locNDF = Get_NDF_Timing();
	Float_t locChiSq = Get_ChiSq_Timing();
	return ((locNDF != 0) ? TMath::Prob(locChiSq, locNDF) : -1.0);
}

inline Float_t DChargedTrackHypothesis::Get_ConfidenceLevel_Timing_Measured(void) const
{
	UInt_t locNDF = Get_NDF_Timing();
	Float_t locChiSq = Get_ChiSq_Timing_Measured();
	return ((locNDF != 0) ? TMath::Prob(locChiSq, locNDF) : -1.0);
}

inline DetectorSystem_t DChargedTrackHypothesis::Get_Detector_System_Timing(void) const
{
	if(Get_Energy_BCAL() > 0.) 
		return SYS_BCAL;
	else if(Get_dEdx_TOF() > 0.)
		return SYS_TOF;
	else if(Get_Energy_FCAL() > 0.) 
		return SYS_FCAL;
	else if(Get_dEdx_ST() > 0.)
		return SYS_START;

	return SYS_NULL;
}

//HIT ENERGY
inline Float_t DChargedTrackHypothesis::Get_dEdx_TOF(void) const
{
	return ((Float_t*)dBranch_dEdx_TOF->GetAddress())[dMeasuredArrayIndex];
}

inline Float_t DChargedTrackHypothesis::Get_dEdx_ST(void) const
{
	return ((Float_t*)dBranch_dEdx_ST->GetAddress())[dMeasuredArrayIndex];
}

inline Float_t DChargedTrackHypothesis::Get_Energy_BCAL(void) const
{
	return ((Float_t*)dBranch_Energy_BCAL->GetAddress())[dMeasuredArrayIndex];
}

inline Float_t DChargedTrackHypothesis::Get_Energy_BCALPreshower(void) const
{
	return ((Float_t*)dBranch_Energy_BCALPreshower->GetAddress())[dMeasuredArrayIndex];
}

inline Float_t DChargedTrackHypothesis::Get_Energy_FCAL(void) const
{
	return ((Float_t*)dBranch_Energy_FCAL->GetAddress())[dMeasuredArrayIndex];
}

//SHOWER MATCHING:
inline Float_t DChargedTrackHypothesis::Get_TrackBCAL_DeltaPhi(void) const
{
	return ((Float_t*)dBranch_TrackBCAL_DeltaPhi->GetAddress())[dMeasuredArrayIndex];
}

inline Float_t DChargedTrackHypothesis::Get_TrackBCAL_DeltaZ(void) const
{
	return ((Float_t*)dBranch_TrackBCAL_DeltaZ->GetAddress())[dMeasuredArrayIndex];
}
inline Float_t DChargedTrackHypothesis::Get_TrackFCAL_DOCA(void) const
{
	return ((Float_t*)dBranch_TrackFCAL_DOCA->GetAddress())[dMeasuredArrayIndex];
}

#endif //DChargedTrackHypothesis_h

