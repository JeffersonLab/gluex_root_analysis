#ifndef DNeutralParticleHypothesis_h
#define DNeutralParticleHypothesis_h

#include <string>

#include "TBranch.h"
#include "TClonesArray.h"

#include <particleType.h>
#include <GlueX.h>

#include "DTreeInterface.h"
#include "DKinematicData.h"

using namespace std;

class DSelector;
class DParticleComboStep;

//https://halldweb1.jlab.org/wiki/index.php/Analysis_TTreeFormat
class DNeutralParticleHypothesis : public DKinematicData
{
	friend class DSelector; //for calling ReInitialize
	friend class DParticleComboStep; //for calling ReInitialize

	public:

		DNeutralParticleHypothesis(DTreeInterface* locTreeInterface, string locBranchNamePrefix, Particle_t locPID = UnknownParticle);
		~DNeutralParticleHypothesis(void){}

		//IDENTIFIERS / MATCHING
		Int_t Get_NeutralID(void) const; //each physical particle has its own # (to keep track of different pid hypotheses for the same particle)
		Int_t Get_ThrownIndex(void) const; //the array index of the thrown particle it is matched with (-1 for no match) //only present if simulated data
		Int_t Get_ID(void) const{return Get_NeutralID();}

		//TIMING INFO
		Float_t Get_HitTime(void) const; //the system that is hit is in order of preference: BCAL/TOF/FCAL/ST 
			//to determine which, look whether energy was deposited in these systems
		Float_t Get_PhotonRFDeltaTVar(void) const; //Variance of X4_Measured.T() - RFTime, regardless of which RF bunch is chosen. 
			//Can be used to compute timing ChiSq //RF bunch is combo-dependent
		UInt_t Get_NDF_Timing(void) const;
		Float_t Get_Beta_Timing(void) const; //is kinfit if kinfit, else is measured
		Float_t Get_ChiSq_Timing(void) const; //is kinfit if kinfit, else is measured
		Float_t Get_ConfidenceLevel_Timing(void) const; //is kinfit if kinfit, else is measured
		Float_t Get_Beta_Timing_Measured(void) const;
		Float_t Get_ChiSq_Timing_Measured(void) const;
		Float_t Get_ConfidenceLevel_Timing_Measured(void) const; //is kinfit if kinfit, else is measured
		DetectorSystem_t Get_Detector_System_Timing(void) const;

		//SHOWER INFO
		TLorentzVector Get_X4_Shower(void) const;
		Float_t Get_Shower_Quality(void) const;
		Int_t Get_Shower_SC_BCAL_match(void) const;
		Int_t Get_Shower_SC_FCAL_match(void) const;
		Int_t Get_Shower_TOF_FCAL_match(void) const;
		Float_t Get_Shower_SC_BCAL_phi_min(void) const;
		Float_t Get_Shower_SC_FCAL_phi_min(void) const;
		Float_t Get_Shower_TOF_FCAL_x_min(void) const;
		Float_t Get_Shower_TOF_FCAL_y_min(void) const;

		Float_t Get_Energy_BCAL(void) const;
		Float_t Get_Energy_BCALPreshower(void) const;
		Float_t Get_Energy_BCALLayer2(void) const;
		Float_t Get_Energy_BCALLayer3(void) const;
		Float_t Get_Energy_BCALLayer4(void) const;
		Float_t Get_SigLong_BCAL(void) const;
        Float_t Get_SigTheta_BCAL(void) const;
        Float_t Get_SigTrans_BCAL(void) const;
        Float_t Get_RMSTime_BCAL(void) const;
        
		Float_t Get_Energy_FCAL(void) const;
		Float_t Get_E1E9_FCAL(void) const;
		Float_t Get_E9E25_FCAL(void) const;
		Float_t Get_SumU_FCAL(void) const;
		Float_t Get_SumV_FCAL(void) const;
		Float_t Get_NumBlocks_FCAL(void) const;
		Float_t Get_TrackFCAL_DeltaT(void) const;
	
	Float_t Get_Energy_ECAL(void) const;
	Float_t Get_NumBlocks_ECAL(void) const;

		//TRACK MATCHING
		Float_t Get_TrackBCAL_DeltaPhi(void) const; //999.0 if not matched //units are radians
		Float_t Get_TrackBCAL_DeltaZ(void) const; //999.0 if not matched
		Float_t Get_TrackFCAL_DOCA(void) const; //999.0 if not matched

	private:
		DNeutralParticleHypothesis(void); //Cannot call default constructor!

		void Setup_Branches(void);

		//RE-INITIALIZE (e.g. with the next TTree in a chain)
		void ReInitialize(void);

		//For fundamental arrays
			//cannot store array pointer: when array size is increased, a new array is created
			//must store branch pointer instead

		//IDENTIFIERS / MATCHING
		TBranch* dBranch_NeutralID;
		TBranch* dBranch_ThrownIndex;

		//TIMING INFO
		TBranch* dBranch_HitTime;
		TBranch* dBranch_PhotonRFDeltaTVar;

		TBranch* dBranch_NDF_Timing;
		TBranch* dBranch_Beta_Timing_KinFit;
		TBranch* dBranch_ChiSq_Timing_KinFit;
		TBranch* dBranch_Beta_Timing_Measured;
		TBranch* dBranch_ChiSq_Timing_Measured;
 
		//SHOWER INFO
		TClonesArray** dX4_Shower;
		TBranch* dBranch_Shower_Quality;
		TBranch* dBranch_Shower_SC_BCAL_match;
		TBranch* dBranch_Shower_SC_FCAL_match;
		TBranch* dBranch_Shower_TOF_FCAL_match;
		TBranch* dBranch_Shower_SC_BCAL_phi_min;
		TBranch* dBranch_Shower_SC_FCAL_phi_min;
		TBranch* dBranch_Shower_TOF_FCAL_x_min;
		TBranch* dBranch_Shower_TOF_FCAL_y_min;

		TBranch* dBranch_Energy_BCAL;
		TBranch* dBranch_Energy_BCALPreshower;
		TBranch* dBranch_Energy_BCALLayer2;
		TBranch* dBranch_Energy_BCALLayer3;
		TBranch* dBranch_Energy_BCALLayer4;
		TBranch* dBranch_SigLong_BCAL;
        TBranch* dBranch_SigTheta_BCAL;
        TBranch* dBranch_SigTrans_BCAL;
        TBranch* dBranch_RMSTime_BCAL;
        
		TBranch* dBranch_Energy_FCAL;
		TBranch* dBranch_E1E9_FCAL;
		TBranch* dBranch_E9E25_FCAL;
		TBranch* dBranch_SumU_FCAL;
		TBranch* dBranch_SumV_FCAL;
		TBranch* dBranch_NumBlocks_FCAL;
		TBranch* dBranch_TrackFCAL_DeltaT;

	TBranch* dBranch_Energy_ECAL;	
       	TBranch* dBranch_NumBlocks_ECAL;

		//TRACK MATCHING
		TBranch* dBranch_TrackBCAL_DeltaPhi;
		TBranch* dBranch_TrackBCAL_DeltaZ;
		TBranch* dBranch_TrackFCAL_DOCA;
};

/******************************************************************** CONSTRUCTOR *********************************************************************/

inline DNeutralParticleHypothesis::DNeutralParticleHypothesis(DTreeInterface* locTreeInterface, string locBranchNamePrefix, Particle_t locPID) : 
DKinematicData(locTreeInterface, locBranchNamePrefix, locPID)
{
	Setup_Branches();
}

/***************************************************************** SETUP DATA ACCESS ******************************************************************/

inline void DNeutralParticleHypothesis::Setup_Branches(void)
{
	string locBranchName;

	//IDENTIFIERS / MATCHING
	locBranchName = "NeutralHypo__NeutralID";
	dBranch_NeutralID = dTreeInterface->Get_Branch(locBranchName);

	locBranchName = "NeutralHypo__ThrownIndex";
	dBranch_ThrownIndex = dTreeInterface->Get_Branch(locBranchName);

	//TIMING INFO
	locBranchName = "NeutralHypo__HitTime";
	dBranch_HitTime = dTreeInterface->Get_Branch(locBranchName);

	locBranchName = "NeutralHypo__PhotonRFDeltaTVar";
	dBranch_PhotonRFDeltaTVar = dTreeInterface->Get_Branch(locBranchName);

	locBranchName = "NeutralHypo__NDF_Timing";
	dBranch_NDF_Timing = dTreeInterface->Get_Branch(locBranchName);

	locBranchName = dBranchNamePrefix + string("__Beta_Timing_KinFit");
	dBranch_Beta_Timing_KinFit = dTreeInterface->Get_Branch(locBranchName);

	locBranchName = dBranchNamePrefix + string("__ChiSq_Timing_KinFit");
	dBranch_ChiSq_Timing_KinFit = dTreeInterface->Get_Branch(locBranchName);

	locBranchName = Get_IsDetectedComboNeutralParticle() ? dBranchNamePrefix + string("__Beta_Timing_Measured") : "NeutralHypo__Beta_Timing";
	dBranch_Beta_Timing_Measured = dTreeInterface->Get_Branch(locBranchName);

	locBranchName = Get_IsDetectedComboNeutralParticle() ? dBranchNamePrefix + string("__ChiSq_Timing_Measured") : "NeutralHypo__ChiSq_Timing";
	dBranch_ChiSq_Timing_Measured = dTreeInterface->Get_Branch(locBranchName);

	//SHOWER INFO
	locBranchName = "NeutralHypo__X4_Shower";
	dX4_Shower = dTreeInterface->Get_PointerToPointerTo_TClonesArray(locBranchName);

	locBranchName = "NeutralHypo__ShowerQuality";
	dBranch_Shower_Quality = dTreeInterface->Get_Branch(locBranchName);

	locBranchName = "NeutralHypo__ShowerSC_BCAL_match";
	dBranch_Shower_SC_BCAL_match = dTreeInterface->Get_Branch(locBranchName);

	locBranchName = "NeutralHypo__ShowerSC_FCAL_match";
	dBranch_Shower_SC_FCAL_match = dTreeInterface->Get_Branch(locBranchName);

	locBranchName = "NeutralHypo__ShowerTOF_FCAL_match";
	dBranch_Shower_TOF_FCAL_match = dTreeInterface->Get_Branch(locBranchName);
	
	locBranchName = "NeutralHypo__ShowerSC_BCAL_phi_min";
	dBranch_Shower_SC_BCAL_phi_min = dTreeInterface->Get_Branch(locBranchName);

	locBranchName = "NeutralHypo__ShowerSC_FCAL_phi_min";
	dBranch_Shower_SC_FCAL_phi_min = dTreeInterface->Get_Branch(locBranchName);

	locBranchName = "NeutralHypo__ShowerTOF_FCAL_x_min";
	dBranch_Shower_TOF_FCAL_x_min = dTreeInterface->Get_Branch(locBranchName);
	
	locBranchName = "NeutralHypo__ShowerTOF_FCAL_y_min";
	dBranch_Shower_TOF_FCAL_y_min = dTreeInterface->Get_Branch(locBranchName);

	locBranchName = "NeutralHypo__Energy_BCAL";
	dBranch_Energy_BCAL = dTreeInterface->Get_Branch(locBranchName);

	locBranchName = "NeutralHypo__Energy_BCALPreshower";
	dBranch_Energy_BCALPreshower = dTreeInterface->Get_Branch(locBranchName);
	
	locBranchName = "NeutralHypo__Energy_BCALLayer2";
	dBranch_Energy_BCALLayer2 = dTreeInterface->Get_Branch(locBranchName);

	locBranchName = "NeutralHypo__Energy_BCALLayer3";
	dBranch_Energy_BCALLayer3 = dTreeInterface->Get_Branch(locBranchName);

	locBranchName = "NeutralHypo__Energy_BCALLayer4";
	dBranch_Energy_BCALLayer4 = dTreeInterface->Get_Branch(locBranchName);

	locBranchName = "NeutralHypo__SigLong_BCAL";
    dBranch_SigLong_BCAL = dTreeInterface->Get_Branch(locBranchName);

    locBranchName = "NeutralHypo__SigTheta_BCAL";
    dBranch_SigTheta_BCAL = dTreeInterface->Get_Branch(locBranchName);

    locBranchName = "NeutralHypo__SigTrans_BCAL";
    dBranch_SigTrans_BCAL = dTreeInterface->Get_Branch(locBranchName);

    locBranchName = "NeutralHypo__RMSTime_BCAL";
    dBranch_RMSTime_BCAL = dTreeInterface->Get_Branch(locBranchName);

	locBranchName = "NeutralHypo__Energy_FCAL";
	dBranch_Energy_FCAL = dTreeInterface->Get_Branch(locBranchName);

	locBranchName = "NeutralHypo__E1E9_FCAL";
	dBranch_E1E9_FCAL = dTreeInterface->Get_Branch(locBranchName);

	locBranchName = "NeutralHypo__E9E25_FCAL";
	dBranch_E9E25_FCAL = dTreeInterface->Get_Branch(locBranchName);

	locBranchName = "NeutralHypo__SumU_FCAL";
	dBranch_SumU_FCAL = dTreeInterface->Get_Branch(locBranchName);

	locBranchName = "NeutralHypo__SumV_FCAL";
	dBranch_SumV_FCAL = dTreeInterface->Get_Branch(locBranchName);

	locBranchName = "NeutralHypo__NumBlocks_FCAL";
	dBranch_NumBlocks_FCAL = dTreeInterface->Get_Branch(locBranchName);

	locBranchName = "NeutralHypo__TrackFCAL_DeltaT";
	dBranch_TrackFCAL_DeltaT = dTreeInterface->Get_Branch(locBranchName);

	locBranchName = "NeutralHypo__Energy_ECAL";
	dBranch_Energy_ECAL = dTreeInterface->Get_Branch(locBranchName);

	locBranchName = "NeutralHypo__NumBlocks_ECAL";
	dBranch_NumBlocks_ECAL = dTreeInterface->Get_Branch(locBranchName);

	//TRACK MATCHING:
	locBranchName = "NeutralHypo__TrackBCAL_DeltaPhi";
	dBranch_TrackBCAL_DeltaPhi = dTreeInterface->Get_Branch(locBranchName);

	locBranchName = "NeutralHypo__TrackBCAL_DeltaZ";
	dBranch_TrackBCAL_DeltaZ = dTreeInterface->Get_Branch(locBranchName);

	locBranchName = "NeutralHypo__TrackFCAL_DOCA";
	dBranch_TrackFCAL_DOCA = dTreeInterface->Get_Branch(locBranchName);
}

inline void DNeutralParticleHypothesis::ReInitialize(void)
{
	DKinematicData::ReInitialize();
	Setup_Branches();
}

/****************************************************************** GET OBJECT DATA *******************************************************************/

//IDENTIFIERS / MATCHING
inline Int_t DNeutralParticleHypothesis::Get_NeutralID(void) const
{
	return ((Int_t*)dBranch_NeutralID->GetAddress())[dMeasuredArrayIndex];
}

inline Int_t DNeutralParticleHypothesis::Get_ThrownIndex(void) const
{
	if(dBranch_ThrownIndex == NULL)
		return -1;
	return ((Int_t*)dBranch_ThrownIndex->GetAddress())[dMeasuredArrayIndex];
}

//TIMING INFO
inline Float_t DNeutralParticleHypothesis::Get_HitTime(void) const
{
	return ((Float_t*)dBranch_HitTime->GetAddress())[dMeasuredArrayIndex];
}

inline Float_t DNeutralParticleHypothesis::Get_PhotonRFDeltaTVar(void) const
{
	return ((Float_t*)dBranch_PhotonRFDeltaTVar->GetAddress())[dMeasuredArrayIndex];
}

inline UInt_t DNeutralParticleHypothesis::Get_NDF_Timing(void) const
{
	return ((UInt_t*)dBranch_NDF_Timing->GetAddress())[dMeasuredArrayIndex];
}

inline Float_t DNeutralParticleHypothesis::Get_Beta_Timing_Measured(void) const
{
	int locArrayIndex = Get_IsDetectedComboNeutralParticle() ? dArrayIndex : dMeasuredArrayIndex;
	return ((Float_t*)dBranch_Beta_Timing_Measured->GetAddress())[locArrayIndex];
}

inline Float_t DNeutralParticleHypothesis::Get_ChiSq_Timing_Measured(void) const
{
	int locArrayIndex = Get_IsDetectedComboNeutralParticle() ? dArrayIndex : dMeasuredArrayIndex;
	return ((Float_t*)dBranch_ChiSq_Timing_Measured->GetAddress())[locArrayIndex];
}

inline Float_t DNeutralParticleHypothesis::Get_Beta_Timing(void) const
{
	if(dBranch_Beta_Timing_KinFit != NULL)
		return ((Float_t*)dBranch_Beta_Timing_KinFit->GetAddress())[dArrayIndex];
	return Get_Beta_Timing_Measured();
}

inline Float_t DNeutralParticleHypothesis::Get_ChiSq_Timing(void) const
{
	if(dBranch_ChiSq_Timing_KinFit != NULL)
		return ((Float_t*)dBranch_ChiSq_Timing_KinFit->GetAddress())[dArrayIndex];
	return Get_ChiSq_Timing_Measured();
}

inline Float_t DNeutralParticleHypothesis::Get_ConfidenceLevel_Timing(void) const
{
	UInt_t locNDF = Get_NDF_Timing();
	Float_t locChiSq = Get_ChiSq_Timing();
	return ((locNDF != 0) ? TMath::Prob(locChiSq, locNDF) : -1.0);
}

inline Float_t DNeutralParticleHypothesis::Get_ConfidenceLevel_Timing_Measured(void) const
{
	UInt_t locNDF = Get_NDF_Timing();
	Float_t locChiSq = Get_ChiSq_Timing_Measured();
	return ((locNDF != 0) ? TMath::Prob(locChiSq, locNDF) : -1.0);
}

inline DetectorSystem_t DNeutralParticleHypothesis::Get_Detector_System_Timing(void) const
{
	if(Get_Energy_BCAL() > 0.) 
		return SYS_BCAL;
	else if(Get_Energy_FCAL() > 0.) 
		return SYS_FCAL;

	return SYS_NULL;
}

//SHOWER INFO
inline Float_t DNeutralParticleHypothesis::Get_Shower_Quality(void) const
{
	return ((Float_t*)dBranch_Shower_Quality->GetAddress())[dMeasuredArrayIndex];
}

inline Int_t DNeutralParticleHypothesis::Get_Shower_SC_BCAL_match(void) const
{
	return ((Int_t*)dBranch_Shower_SC_BCAL_match->GetAddress())[dMeasuredArrayIndex];
}

inline Int_t DNeutralParticleHypothesis::Get_Shower_SC_FCAL_match(void) const
{
	return ((Int_t*)dBranch_Shower_SC_FCAL_match->GetAddress())[dMeasuredArrayIndex];
}

inline Int_t DNeutralParticleHypothesis::Get_Shower_TOF_FCAL_match(void) const
{
	return ((Int_t*)dBranch_Shower_TOF_FCAL_match->GetAddress())[dMeasuredArrayIndex];
}

inline Float_t DNeutralParticleHypothesis::Get_Shower_SC_BCAL_phi_min(void) const
{
	return ((Float_t*)dBranch_Shower_SC_BCAL_phi_min->GetAddress())[dMeasuredArrayIndex];
}

inline Float_t DNeutralParticleHypothesis::Get_Shower_SC_FCAL_phi_min(void) const
{
        return ((Float_t*)dBranch_Shower_SC_FCAL_phi_min->GetAddress())[dMeasuredArrayIndex];
}

inline Float_t DNeutralParticleHypothesis::Get_Shower_TOF_FCAL_x_min(void) const
{
        return ((Float_t*)dBranch_Shower_TOF_FCAL_x_min->GetAddress())[dMeasuredArrayIndex];
}

inline Float_t DNeutralParticleHypothesis::Get_Shower_TOF_FCAL_y_min(void) const
{
        return ((Float_t*)dBranch_Shower_TOF_FCAL_y_min->GetAddress())[dMeasuredArrayIndex];
}

inline Float_t DNeutralParticleHypothesis::Get_Energy_BCAL(void) const
{
	return ((Float_t*)dBranch_Energy_BCAL->GetAddress())[dMeasuredArrayIndex];
}

inline Float_t DNeutralParticleHypothesis::Get_Energy_BCALPreshower(void) const
{
	return ((Float_t*)dBranch_Energy_BCALPreshower->GetAddress())[dMeasuredArrayIndex];
}

inline Float_t DNeutralParticleHypothesis::Get_Energy_BCALLayer2(void) const
{
	if(dBranch_Energy_BCALLayer2 == NULL)
		return -1.;
	else
		return ((Float_t*)dBranch_Energy_BCALLayer2->GetAddress())[dMeasuredArrayIndex];
}

inline Float_t DNeutralParticleHypothesis::Get_Energy_BCALLayer3(void) const
{
	if(dBranch_Energy_BCALLayer3 == NULL)
		return -1.;
	else
		return ((Float_t*)dBranch_Energy_BCALLayer3->GetAddress())[dMeasuredArrayIndex];
}

inline Float_t DNeutralParticleHypothesis::Get_Energy_BCALLayer4(void) const
{
	if(dBranch_Energy_BCALLayer4 == NULL)
		return -1.;
	else
		return ((Float_t*)dBranch_Energy_BCALLayer4->GetAddress())[dMeasuredArrayIndex];
}

inline Float_t DNeutralParticleHypothesis::Get_SigLong_BCAL(void) const
{
        return ((Float_t*)dBranch_SigLong_BCAL->GetAddress())[dMeasuredArrayIndex];
}

inline Float_t DNeutralParticleHypothesis::Get_SigTheta_BCAL(void) const
{
        return ((Float_t*)dBranch_SigTheta_BCAL->GetAddress())[dMeasuredArrayIndex];
}

inline Float_t DNeutralParticleHypothesis::Get_SigTrans_BCAL(void) const
{
        return ((Float_t*)dBranch_SigTrans_BCAL->GetAddress())[dMeasuredArrayIndex];
}

inline Float_t DNeutralParticleHypothesis::Get_RMSTime_BCAL(void) const
{
        return ((Float_t*)dBranch_RMSTime_BCAL->GetAddress())[dMeasuredArrayIndex];
}

inline Float_t DNeutralParticleHypothesis::Get_Energy_FCAL(void) const
{
	return ((Float_t*)dBranch_Energy_FCAL->GetAddress())[dMeasuredArrayIndex];
}

inline Float_t DNeutralParticleHypothesis::Get_E1E9_FCAL(void) const
{
	if(dBranch_E1E9_FCAL == NULL)
		return -1.;
	else
		return ((Float_t*)dBranch_E1E9_FCAL->GetAddress())[dMeasuredArrayIndex];
}

inline Float_t DNeutralParticleHypothesis::Get_E9E25_FCAL(void) const
{
	if(dBranch_E9E25_FCAL == NULL)
		return -1.;
	else
		return ((Float_t*)dBranch_E9E25_FCAL->GetAddress())[dMeasuredArrayIndex];
}

inline Float_t DNeutralParticleHypothesis::Get_SumU_FCAL(void) const
{
	if(dBranch_SumU_FCAL == NULL)
		return -1.;
	else
		return ((Float_t*)dBranch_SumU_FCAL->GetAddress())[dMeasuredArrayIndex];
}

inline Float_t DNeutralParticleHypothesis::Get_SumV_FCAL(void) const
{
	if(dBranch_SumV_FCAL == NULL)
		return -1.;
	else
		return ((Float_t*)dBranch_SumV_FCAL->GetAddress())[dMeasuredArrayIndex];
}

inline Float_t DNeutralParticleHypothesis::Get_NumBlocks_FCAL(void) const
{
	if(dBranch_NumBlocks_FCAL == NULL)
		return -1.;
	else
		return ((Float_t*)dBranch_NumBlocks_FCAL->GetAddress())[dMeasuredArrayIndex];
}

inline Float_t DNeutralParticleHypothesis::Get_TrackFCAL_DeltaT(void) const
{
	if(dBranch_TrackFCAL_DeltaT == NULL)
		return -1.;
	else
		return ((Float_t*)dBranch_TrackFCAL_DeltaT->GetAddress())[dMeasuredArrayIndex];
}

inline Float_t DNeutralParticleHypothesis::Get_Energy_ECAL(void) const
{
	return ((Float_t*)dBranch_Energy_ECAL->GetAddress())[dMeasuredArrayIndex];
}

inline Float_t DNeutralParticleHypothesis::Get_NumBlocks_ECAL(void) const
{
	if(dBranch_NumBlocks_ECAL == NULL)
		return -1.;
	else
		return ((Float_t*)dBranch_NumBlocks_ECAL->GetAddress())[dMeasuredArrayIndex];
}

inline TLorentzVector DNeutralParticleHypothesis::Get_X4_Shower(void) const
{
	return *((TLorentzVector*)(*dX4_Shower)->At(dMeasuredArrayIndex));
}

//TRACK MATCHING:
inline Float_t DNeutralParticleHypothesis::Get_TrackBCAL_DeltaPhi(void) const
{
	return ((Float_t*)dBranch_TrackBCAL_DeltaPhi->GetAddress())[dMeasuredArrayIndex];
}

inline Float_t DNeutralParticleHypothesis::Get_TrackBCAL_DeltaZ(void) const
{
	return ((Float_t*)dBranch_TrackBCAL_DeltaZ->GetAddress())[dMeasuredArrayIndex];
}

inline Float_t DNeutralParticleHypothesis::Get_TrackFCAL_DOCA(void) const
{
	return ((Float_t*)dBranch_TrackFCAL_DOCA->GetAddress())[dMeasuredArrayIndex];
}

#endif //DNeutralParticleHypothesis_h
