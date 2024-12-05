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

		// Global PID
		Float_t Get_PIDFOM(void) const;

		//TRACKING INFO
		UInt_t Get_NDF_Tracking(void) const;
		Float_t Get_ChiSq_Tracking(void) const;
		UInt_t Get_NDF_DCdEdx(void) const;
		Float_t Get_ChiSq_DCdEdx(void) const;
		Float_t Get_dEdx_CDC(void) const;
		Float_t Get_dEdx_CDC_integral(void) const;
		Float_t Get_dEdx_FDC(void) const;
  		Float_t Get_FDC1_X(void) const;
    		Float_t Get_FDC1_Y(void) const;
  		Float_t Get_FDC2_X(void) const;
    		Float_t Get_FDC2_Y(void) const;
  		Float_t Get_FDC3_X(void) const;
    		Float_t Get_FDC3_Y(void) const;
  		Float_t Get_FDC4_X(void) const;
    		Float_t Get_FDC4_Y(void) const;


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

		//SHOWER MATCHING
		Float_t Get_TrackBCAL_DeltaPhi(void) const; //999.0 if not matched //units are radians
		Float_t Get_TrackBCAL_DeltaZ(void) const; //999.0 if not matched
		Float_t Get_TrackFCAL_DOCA(void) const; //999.0 if not matched

		//DIRC INFORMATION
                Int_t Get_Track_NumPhotons_DIRC(void) const;

                Float_t Get_Track_ExtrapolatedX_DIRC(void) const;
                Float_t Get_Track_ExtrapolatedY_DIRC(void) const;
                Float_t Get_Track_ThetaC_DIRC(void) const;
                Float_t Get_Track_Lele_DIRC(void) const;
                Float_t Get_Track_Lpi_DIRC(void) const;
                Float_t Get_Track_Lk_DIRC(void) const;
                Float_t Get_Track_Lp_DIRC(void) const;

				Int_t Get_DIRC_Bar_Number(void) const;



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

		// GLOBAL PID
		TBranch* dBranch_PIDFOM;

		//TRACKING INFO
		TBranch* dBranch_NDF_Tracking;
		TBranch* dBranch_ChiSq_Tracking;
		TBranch* dBranch_NDF_DCdEdx;
		TBranch* dBranch_ChiSq_DCdEdx;
		TBranch* dBranch_dEdx_CDC;
		TBranch* dBranch_dEdx_CDC_integral;
		TBranch* dBranch_dEdx_FDC;
		TBranch* dBranch_FDC1_X;
  		TBranch* dBranch_FDC1_Y;
		TBranch* dBranch_FDC2_X;
  		TBranch* dBranch_FDC2_Y;
		TBranch* dBranch_FDC3_X;
  		TBranch* dBranch_FDC3_Y;
		TBranch* dBranch_FDC4_X;
  		TBranch* dBranch_FDC4_Y;

 
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
 
		//SHOWER MATCHING
		TBranch* dBranch_TrackBCAL_DeltaPhi;
		TBranch* dBranch_TrackBCAL_DeltaZ;
		TBranch* dBranch_TrackFCAL_DOCA;

		// DIRC INFORMATION
                TBranch* dBranch_Track_NumPhotons_DIRC;
                TBranch* dBranch_Track_ExtrapolatedX_DIRC;
                TBranch* dBranch_Track_ExtrapolatedY_DIRC;
                TBranch* dBranch_Track_ThetaC_DIRC;
                TBranch* dBranch_Track_Lele_DIRC;
                TBranch* dBranch_Track_Lpi_DIRC;
                TBranch* dBranch_Track_Lk_DIRC;
                TBranch* dBranch_Track_Lp_DIRC;

		const Float_t DIRC_BAR_Y[48] = { 
-10.9715 , -14.4865 , -18.0015 , -21.5165 , -25.0315 , -28.5465 , -32.0615 , -35.5765 , -39.0915 , -42.6065 , -46.1215 , -49.6365 , -62.4417 , -65.9567 , -69.4717 , -72.9867 , -76.5017 , -80.0167 , -83.5317 , -87.0467 , -90.5617 , -94.0767 , -97.5917 , -101.107 , 10.528 , 14.043 , 17.558 , 21.073 , 24.588 , 28.103 , 31.618 , 35.133 , 38.648 , 42.163 , 45.678 , 49.193 , 62.0265 , 65.5415 , 69.0565 , 72.5715 , 76.0865 , 79.6015 , 83.1165 , 86.6315 , 90.1465 , 93.6615 , 97.1765 , 100.691};

		const Float_t DIRC_QZBL_DY = 3.515;

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
	
	// GLOBAL PID
	locBranchName = "ChargedHypo__PIDFOM";
	dBranch_PIDFOM = dTreeInterface->Get_Branch(locBranchName);

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

	locBranchName = "ChargedHypo__dEdx_CDC_integral";
	dBranch_dEdx_CDC_integral = dTreeInterface->Get_Branch(locBranchName);

	locBranchName = "ChargedHypo__dEdx_FDC";
	dBranch_dEdx_FDC = dTreeInterface->Get_Branch(locBranchName);

	locBranchName = "ChargedHypo__FDC1_X";
	dBranch_FDC1_X = dTreeInterface->Get_Branch(locBranchName);

	locBranchName = "ChargedHypo__FDC1_Y";
	dBranch_FDC1_Y = dTreeInterface->Get_Branch(locBranchName);

	locBranchName = "ChargedHypo__FDC2_X";
	dBranch_FDC2_X = dTreeInterface->Get_Branch(locBranchName);

	locBranchName = "ChargedHypo__FDC2_Y";
	dBranch_FDC2_Y = dTreeInterface->Get_Branch(locBranchName);
	
	locBranchName = "ChargedHypo__FDC3_X";
	dBranch_FDC3_X = dTreeInterface->Get_Branch(locBranchName);

	locBranchName = "ChargedHypo__FDC3_Y";
	dBranch_FDC3_Y = dTreeInterface->Get_Branch(locBranchName);

	locBranchName = "ChargedHypo__FDC4_X";
	dBranch_FDC4_X = dTreeInterface->Get_Branch(locBranchName);

	locBranchName = "ChargedHypo__FDC4_Y";
	dBranch_FDC4_Y = dTreeInterface->Get_Branch(locBranchName);
	

	
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

	locBranchName = "ChargedHypo__Energy_BCALLayer2";
	dBranch_Energy_BCALLayer2 = dTreeInterface->Get_Branch(locBranchName);

	locBranchName = "ChargedHypo__Energy_BCALLayer3";
	dBranch_Energy_BCALLayer3 = dTreeInterface->Get_Branch(locBranchName);

	locBranchName = "ChargedHypo__Energy_BCALLayer4";
	dBranch_Energy_BCALLayer4 = dTreeInterface->Get_Branch(locBranchName);

	locBranchName = "ChargedHypo__SigLong_BCAL";
	dBranch_SigLong_BCAL = dTreeInterface->Get_Branch(locBranchName);
	
	locBranchName = "ChargedHypo__SigTheta_BCAL";
	dBranch_SigTheta_BCAL = dTreeInterface->Get_Branch(locBranchName);
	
	locBranchName = "ChargedHypo__SigTrans_BCAL";
	dBranch_SigTrans_BCAL = dTreeInterface->Get_Branch(locBranchName);
	
	locBranchName = "ChargedHypo__RMSTime_BCAL";
	dBranch_RMSTime_BCAL = dTreeInterface->Get_Branch(locBranchName);
	
	locBranchName = "ChargedHypo__Energy_FCAL";
	dBranch_Energy_FCAL = dTreeInterface->Get_Branch(locBranchName);

	locBranchName = "ChargedHypo__E1E9_FCAL";
	dBranch_E1E9_FCAL = dTreeInterface->Get_Branch(locBranchName);

	locBranchName = "ChargedHypo__E9E25_FCAL";
	dBranch_E9E25_FCAL = dTreeInterface->Get_Branch(locBranchName);

	locBranchName = "ChargedHypo__SumU_FCAL";
	dBranch_SumU_FCAL = dTreeInterface->Get_Branch(locBranchName);

	locBranchName = "ChargedHypo__SumV_FCAL";
	dBranch_SumV_FCAL = dTreeInterface->Get_Branch(locBranchName);

	//SHOWER MATCHING:
	locBranchName = "ChargedHypo__TrackBCAL_DeltaPhi";
	dBranch_TrackBCAL_DeltaPhi = dTreeInterface->Get_Branch(locBranchName);

	locBranchName = "ChargedHypo__TrackBCAL_DeltaZ";
	dBranch_TrackBCAL_DeltaZ = dTreeInterface->Get_Branch(locBranchName);

	locBranchName = "ChargedHypo__TrackFCAL_DOCA";
	dBranch_TrackFCAL_DOCA = dTreeInterface->Get_Branch(locBranchName);

	//DIRC INFORMATION:
        locBranchName = "ChargedHypo__NumPhotons_DIRC";
        dBranch_Track_NumPhotons_DIRC = dTreeInterface->Get_Branch(locBranchName);

        locBranchName = "ChargedHypo__ExtrapolatedX_DIRC";
        dBranch_Track_ExtrapolatedX_DIRC = dTreeInterface->Get_Branch(locBranchName);

        locBranchName = "ChargedHypo__ExtrapolatedY_DIRC";
        dBranch_Track_ExtrapolatedY_DIRC = dTreeInterface->Get_Branch(locBranchName);

        locBranchName = "ChargedHypo__ThetaC_DIRC";
        dBranch_Track_ThetaC_DIRC = dTreeInterface->Get_Branch(locBranchName);

        locBranchName = "ChargedHypo__Lele_DIRC";
        dBranch_Track_Lele_DIRC = dTreeInterface->Get_Branch(locBranchName);

        locBranchName = "ChargedHypo__Lpi_DIRC";
        dBranch_Track_Lpi_DIRC = dTreeInterface->Get_Branch(locBranchName);

        locBranchName = "ChargedHypo__Lk_DIRC";
        dBranch_Track_Lk_DIRC = dTreeInterface->Get_Branch(locBranchName);

        locBranchName = "ChargedHypo__Lp_DIRC";
        dBranch_Track_Lp_DIRC = dTreeInterface->Get_Branch(locBranchName);
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
	if(dBranch_ThrownIndex == NULL)
		return -1;
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

inline Float_t DChargedTrackHypothesis::Get_dEdx_CDC_integral(void) const
{
	return ((Float_t*)dBranch_dEdx_CDC_integral->GetAddress())[dMeasuredArrayIndex];
}

inline Float_t DChargedTrackHypothesis::Get_dEdx_FDC(void) const
{
	return ((Float_t*)dBranch_dEdx_FDC->GetAddress())[dMeasuredArrayIndex];
}

inline Float_t DChargedTrackHypothesis::Get_FDC1_X(void) const
{
	return ((Float_t*)dBranch_FDC1_X->GetAddress())[dMeasuredArrayIndex];
}

inline Float_t DChargedTrackHypothesis::Get_FDC1_Y(void) const
{
	return ((Float_t*)dBranch_FDC1_Y->GetAddress())[dMeasuredArrayIndex];
}

inline Float_t DChargedTrackHypothesis::Get_FDC2_X(void) const
{
	return ((Float_t*)dBranch_FDC2_X->GetAddress())[dMeasuredArrayIndex];
}

inline Float_t DChargedTrackHypothesis::Get_FDC2_Y(void) const
{
	return ((Float_t*)dBranch_FDC2_Y->GetAddress())[dMeasuredArrayIndex];
}

inline Float_t DChargedTrackHypothesis::Get_FDC3_X(void) const
{
	return ((Float_t*)dBranch_FDC3_X->GetAddress())[dMeasuredArrayIndex];
}

inline Float_t DChargedTrackHypothesis::Get_FDC3_Y(void) const
{
	return ((Float_t*)dBranch_FDC3_Y->GetAddress())[dMeasuredArrayIndex];
}

inline Float_t DChargedTrackHypothesis::Get_FDC4_X(void) const
{
	return ((Float_t*)dBranch_FDC4_X->GetAddress())[dMeasuredArrayIndex];
}

inline Float_t DChargedTrackHypothesis::Get_FDC4_Y(void) const
{
	return ((Float_t*)dBranch_FDC4_Y->GetAddress())[dMeasuredArrayIndex];
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

// Global PID
inline Float_t DChargedTrackHypothesis::Get_PIDFOM(void) const
{
	if(dBranch_PIDFOM == NULL)
                return -1.;
	else
                return ((Float_t*)dBranch_PIDFOM->GetAddress())[dMeasuredArrayIndex];
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

inline Float_t DChargedTrackHypothesis::Get_Energy_BCALLayer2(void) const
{
	if(dBranch_Energy_BCALLayer2 == NULL)
		return -1.;
	else
		return ((Float_t*)dBranch_Energy_BCALLayer2->GetAddress())[dMeasuredArrayIndex];
}

inline Float_t DChargedTrackHypothesis::Get_Energy_BCALLayer3(void) const
{
	if(dBranch_Energy_BCALLayer3 == NULL)
		return -1.;
	else
		return ((Float_t*)dBranch_Energy_BCALLayer3->GetAddress())[dMeasuredArrayIndex];
}

inline Float_t DChargedTrackHypothesis::Get_Energy_BCALLayer4(void) const
{
	if(dBranch_Energy_BCALLayer4 == NULL)
		return -1.;
	else
		return ((Float_t*)dBranch_Energy_BCALLayer4->GetAddress())[dMeasuredArrayIndex];
}

inline Float_t DChargedTrackHypothesis::Get_SigLong_BCAL(void) const
{
        return ((Float_t*)dBranch_SigLong_BCAL->GetAddress())[dMeasuredArrayIndex];
}

inline Float_t DChargedTrackHypothesis::Get_SigTheta_BCAL(void) const
{
        return ((Float_t*)dBranch_SigTheta_BCAL->GetAddress())[dMeasuredArrayIndex];
}

inline Float_t DChargedTrackHypothesis::Get_SigTrans_BCAL(void) const
{
        return ((Float_t*)dBranch_SigTrans_BCAL->GetAddress())[dMeasuredArrayIndex];
}

inline Float_t DChargedTrackHypothesis::Get_RMSTime_BCAL(void) const
{
        return ((Float_t*)dBranch_RMSTime_BCAL->GetAddress())[dMeasuredArrayIndex];
}

inline Float_t DChargedTrackHypothesis::Get_Energy_FCAL(void) const
{
	return ((Float_t*)dBranch_Energy_FCAL->GetAddress())[dMeasuredArrayIndex];
}

inline Float_t DChargedTrackHypothesis::Get_E1E9_FCAL(void) const
{
		return ((Float_t*)dBranch_E1E9_FCAL->GetAddress())[dMeasuredArrayIndex];
}

inline Float_t DChargedTrackHypothesis::Get_E9E25_FCAL(void) const
{
		return ((Float_t*)dBranch_E9E25_FCAL->GetAddress())[dMeasuredArrayIndex];
}

inline Float_t DChargedTrackHypothesis::Get_SumU_FCAL(void) const
{
		return ((Float_t*)dBranch_SumU_FCAL->GetAddress())[dMeasuredArrayIndex];
}

inline Float_t DChargedTrackHypothesis::Get_SumV_FCAL(void) const
{
		return ((Float_t*)dBranch_SumV_FCAL->GetAddress())[dMeasuredArrayIndex];
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

//DIRC INFORMATION
inline Int_t DChargedTrackHypothesis::Get_Track_NumPhotons_DIRC(void) const
{	
	if(dBranch_Track_NumPhotons_DIRC)
	       return ((Int_t*)dBranch_Track_NumPhotons_DIRC->GetAddress())[dMeasuredArrayIndex];
	else 
		return 0;
}

inline Float_t DChargedTrackHypothesis::Get_Track_ExtrapolatedX_DIRC(void) const
{	
	if(dBranch_Track_ExtrapolatedX_DIRC)
	       return ((Float_t*)dBranch_Track_ExtrapolatedX_DIRC->GetAddress())[dMeasuredArrayIndex];
	else 
		return 0;
}

inline Float_t DChargedTrackHypothesis::Get_Track_ExtrapolatedY_DIRC(void) const
{	
	if(dBranch_Track_ExtrapolatedY_DIRC)
	        return ((Float_t*)dBranch_Track_ExtrapolatedY_DIRC->GetAddress())[dMeasuredArrayIndex];
	else 
		return 0;
}

inline Float_t DChargedTrackHypothesis::Get_Track_ThetaC_DIRC(void) const
{
        return ((Float_t*)dBranch_Track_ThetaC_DIRC->GetAddress())[dMeasuredArrayIndex];
}

inline Float_t DChargedTrackHypothesis::Get_Track_Lele_DIRC(void) const
{
        return ((Float_t*)dBranch_Track_Lele_DIRC->GetAddress())[dMeasuredArrayIndex];
}

inline Float_t DChargedTrackHypothesis::Get_Track_Lpi_DIRC(void) const
{
        return ((Float_t*)dBranch_Track_Lpi_DIRC->GetAddress())[dMeasuredArrayIndex];
}

inline Float_t DChargedTrackHypothesis::Get_Track_Lk_DIRC(void) const
{
        return ((Float_t*)dBranch_Track_Lk_DIRC->GetAddress())[dMeasuredArrayIndex];
}

inline Float_t DChargedTrackHypothesis::Get_Track_Lp_DIRC(void) const
{
        return ((Float_t*)dBranch_Track_Lp_DIRC->GetAddress())[dMeasuredArrayIndex];
}

inline Int_t DChargedTrackHypothesis::Get_DIRC_Bar_Number(void) const
{	
	int locBar = 23;
	if(dBranch_Track_ExtrapolatedY_DIRC){
		for(int i=0; i<48; i++) {
			float y = ((Float_t*)dBranch_Track_ExtrapolatedY_DIRC->GetAddress())[dMeasuredArrayIndex];
			if(y > (DIRC_BAR_Y[i] - DIRC_QZBL_DY/2.0) && y <= (DIRC_BAR_Y[i] + DIRC_QZBL_DY/2.0)) {
				locBar = i;
			} else if (y <= DIRC_BAR_Y[23]) {
				// Track position out of bound in -y
				locBar = 23;
			} else if (y > DIRC_BAR_Y[47]) {
				// Track position out of bound in +y
				locBar = 47;
			} else if (y > DIRC_BAR_Y[0] && y < DIRC_BAR_Y[24]) { 
				// Track position out of bound near the beam pipe
				if (y > DIRC_BAR_Y[0] && y <= 0.0) {
 					locBar = 0;
 				} else if (y < DIRC_BAR_Y[24] && y > 0.0) {
 					locBar = 24;
				}
 			} else if (y < DIRC_BAR_Y[11] && y > DIRC_BAR_Y[12]) { 
				// Track position out of bound in -y between two bar boxes
				if (y < DIRC_BAR_Y[11] && y > (DIRC_BAR_Y[11]+DIRC_BAR_Y[12])/2.0) {
 					locBar = 11;
				} else {
 					locBar = 12;
				}
 			} else if (y > DIRC_BAR_Y[35] && y < DIRC_BAR_Y[36]) {
				// Track position out of bound  in -y between two bar boxes
				if (y > DIRC_BAR_Y[35] && y < (DIRC_BAR_Y[35]+DIRC_BAR_Y[36])/2.0) {
 					locBar = 35;
				} else {
 					locBar = 36;
				}
			} else {
				// For some reason, the track still fallthrough the crack, return return to the most inrelevent bar
				locBar = 23;
			}
		}

	    return locBar; }
	else 
		return 23;
}

#endif //DChargedTrackHypothesis_h

