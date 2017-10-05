#ifndef DKinematicData_h
#define DKinematicData_h

#include <string>

#include "TLorentzVector.h"
#include "TBranch.h"
#include "TClonesArray.h"

#include <particleType.h>

#include "DTreeInterface.h"

using namespace std;

class DParticleCombo;
class DParticleComboStep;
class DSelector;

//https://halldweb1.jlab.org/wiki/index.php/Analysis_TTreeFormat
class DKinematicData
{
	friend class DParticleCombo; //for setting x4 of decaying, missing, target particles
	friend class DParticleComboStep; //for calling ReInitialize
	friend class DSelector; //for calling ReInitialize

	public:

		//locPID should be set for combo particles, and Unknown otherwise
		DKinematicData(DTreeInterface* locTreeInterface, string locBranchNamePrefix, Particle_t locPID = Unknown);
		virtual ~DKinematicData(void){}

		//GET PID & KINEMATIC DATA
		Particle_t Get_PID(void) const;
		TLorentzVector Get_P4(void) const; //is kinfit if kinfit, else is measured
		TLorentzVector Get_X4(void) const; //is kinfit if kinfit, else is measured
		TLorentzVector Get_P4_Measured(void) const;
		TLorentzVector Get_X4_Measured(void) const;

		//GET CUSTOM DATA
		template <typename DType> DType Get_Fundamental(string locVariableName, bool locComboDataFlag = true) const;
		template <typename DType> DType Get_TObject(string locVariableName, bool locComboDataFlag = true) const;
		//locComboDataFlag: 
		//Is ignored if not a combo-object (combo objs: ComboBeam, PiPlus, etc.). 
		//If it IS a combo object:
			//true if the branch name should be prefixed with the particle name (e.g.: ComboBeam, PiPlus, etc.)
			//false if it should be prefixed with "ChargedHypo__", "NeutralHypo__", etc. (non-combo arrays)

		//GET ID //To be overloaded by deriving classes: Get_TrackID(), Get_NeutralID(), etc.
		virtual Int_t Get_ID(void) const{return 0;}

		//GET BRANCH NAME PREFIX
		string Get_BranchNamePrefix(void) const{return dBranchNamePrefix;}

		//ARRAY
		UInt_t Get_ArraySize(void) const;
		void Set_ArrayIndex(UInt_t locArrayIndex);

		//PATH LENGTH SIGMA (sigh)
		Float_t Get_PathLengthSigma(void) const; //for decaying particles with detached vertices only!!!

	protected:

		//RE-INITIALIZE (e.g. with the next TTree in a chain)
		virtual void ReInitialize(void);

		bool Get_IsComboParticle(void) const;
		bool Get_IsDetectedComboNeutralParticle(void) const;

		//Tree interface
		DTreeInterface* dTreeInterface;

		//Branch Name Prefix
		string dBranchNamePrefix;
		string dMeasuredBranchNamePrefix;

		//Branch Indices/Size
		UInt_t* dArraySize; //size of the main array: measured if measured, #combos if combo-specific
		UInt_t dArrayIndex; //the index in the particle-data arrays to use to grab particle data (e.g. corresponding to this combo)
		UInt_t dMeasuredArrayIndex; //the index in the measured-particle data arrays to use to grab particle data

	private:
		DKinematicData(void); //Cannot call default constructor!

		void Setup_Branches(void);

		//For fundamental arrays
			//cannot store array pointer: when array size is increased, a new array is created
			//must store branch pointer instead

		//Branch Indices
		//for combo-dependent objects, points to combo-independent info
		//for missing, decaying, and target particles, is from the particle that helps it retrieve X4_Measured
		TBranch* dBranch_MeasuredIndex;

		// Particle ID
		// If fixed, get from dPID //else is Unknown
		Particle_t dPID; //if Unknown, get from dBranch_PID
		// If not fixed: ChargedHypo, Thrown: varies from particle to particle: must get from branch
		TBranch* dBranch_PID;

		// P4
		TClonesArray** dP4_KinFit; //NULL if not kinfit
		TClonesArray** dP4_Measured; //NULL if target, missing, decaying, or thrown beam
		TLorentzVector dFixedP4; //use if target
		TLorentzVector** dThrownBeamP4; //NULL if not thrown beam

		// X4
		TClonesArray** dX4_KinFit; //NULL if not kinfit
		TClonesArray** dX4_Measured; //NULL if thrown beam //if missing, decaying, or target: is from a different particle
		TLorentzVector** dThrownBeamX4; //NULL if not thrown beam

		// Path length sigma //for decaying particles with detached vertices only!!!
		TBranch* dBranch_PathLengthSigma;
};

/******************************************************************** CONSTRUCTOR *********************************************************************/

inline DKinematicData::DKinematicData(DTreeInterface* locTreeInterface, string locBranchNamePrefix, Particle_t locPID) : 
dTreeInterface(locTreeInterface), dBranchNamePrefix(locBranchNamePrefix), dMeasuredBranchNamePrefix(locBranchNamePrefix), 
dArraySize(NULL), dArrayIndex(0), dMeasuredArrayIndex(0), dBranch_MeasuredIndex(NULL), dPID(locPID), dBranch_PID(NULL),
dP4_KinFit(NULL), dP4_Measured(NULL), dFixedP4(TLorentzVector()), dThrownBeamP4(NULL), dX4_KinFit(NULL), dX4_Measured(NULL), dThrownBeamX4(NULL), dBranch_PathLengthSigma(NULL)
{
	//locPID should be set for combo particles, and Unknown otherwise
	Setup_Branches();
}

/***************************************************************** SETUP DATA ACCESS ******************************************************************/

inline UInt_t DKinematicData::Get_ArraySize(void) const
{
	return (dArraySize != NULL) ? *dArraySize : 1;
}

inline void DKinematicData::Set_ArrayIndex(UInt_t locArrayIndex)
{
	dArrayIndex = locArrayIndex;
	dMeasuredArrayIndex = (dBranch_MeasuredIndex != NULL) ? ((UInt_t*)dBranch_MeasuredIndex->GetAddress())[dArrayIndex] : dArrayIndex;
}

inline bool DKinematicData::Get_IsComboParticle(void) const
{
	return !((dBranchNamePrefix == "ThrownBeam") || (dBranchNamePrefix == "Thrown") || (dBranchNamePrefix == "Beam") || (dBranchNamePrefix == "ChargedHypo") || (dBranchNamePrefix == "NeutralHypo"));
}

inline void DKinematicData::ReInitialize(void)
{
	Setup_Branches();
}

/****************************************************************** GET OBJECT DATA *******************************************************************/

//IsDetectedComboNeutralParticle
inline bool DKinematicData::Get_IsDetectedComboNeutralParticle(void) const
{
	//Get measured p4 for combo neutral particles from the combo object, not the measured object
	if((ParticleCharge(Get_PID()) != 0) || !Get_IsComboParticle())
		return false;
	if((dBranchNamePrefix.substr(0, 8) == "Decaying") || (dBranchNamePrefix.substr(0, 7) == "Missing") || (dBranchNamePrefix == "ComboBeam"))
		return false;
	return true;
}

//PID
inline Particle_t DKinematicData::Get_PID(void) const
{
	if(dPID != Unknown)
		return dPID;
	Int_t locPID_PDG = ((Int_t*)dBranch_PID->GetAddress())[dArrayIndex];
	return PDGtoPType(locPID_PDG);
}

//KINEMATICS
inline TLorentzVector DKinematicData::Get_P4_Measured(void) const
{
	if(dBranchNamePrefix.substr(0, 6) == "Target")
		return dFixedP4; //target
	if(dBranchNamePrefix == "ThrownBeam")
		return **dThrownBeamP4; //thrown beam

	int locArrayIndex = Get_IsDetectedComboNeutralParticle() ? dArrayIndex : dMeasuredArrayIndex;
	return *((TLorentzVector*)(*dP4_Measured)->At(locArrayIndex));
}

inline TLorentzVector DKinematicData::Get_P4(void) const
{
	if(dP4_KinFit != NULL)
		return *((TLorentzVector*)(*dP4_KinFit)->At(dArrayIndex));
	return Get_P4_Measured();
}

inline TLorentzVector DKinematicData::Get_X4_Measured(void) const
{
	if(dBranchNamePrefix == "ThrownBeam")
		return **dThrownBeamX4; //thrown beam

	int locArrayIndex = Get_IsDetectedComboNeutralParticle() ? dArrayIndex : dMeasuredArrayIndex;
	return *((TLorentzVector*)(*dX4_Measured)->At(locArrayIndex));
}

inline TLorentzVector DKinematicData::Get_X4(void) const
{
	if(dX4_KinFit != NULL)
		return *((TLorentzVector*)(*dX4_KinFit)->At(dArrayIndex));
	return Get_X4_Measured();
}

//PATH LENGTH SIGMA
inline Float_t DKinematicData::Get_PathLengthSigma(void) const
{
	//for decaying particles with detached vertices only!!!
	return (dBranch_PathLengthSigma == NULL) ? 0.0 : ((Float_t*)dBranch_PathLengthSigma->GetAddress())[dArrayIndex];
}

//GET CUSTOM DATA
template <typename DType> inline DType DKinematicData::Get_Fundamental(string locVariableName, bool locComboDataFlag) const
{
	bool locUseComboData = (Get_IsComboParticle() && locComboDataFlag);

	UInt_t locArrayIndex = locUseComboData ? dArrayIndex : dMeasuredArrayIndex;
	string locBranchNamePrefix = locUseComboData ? dBranchNamePrefix : dMeasuredBranchNamePrefix;
	string locBranchName = locBranchNamePrefix + string("__") + locVariableName;

	return dTreeInterface->Get_Fundamental<DType>(locBranchName, locArrayIndex);
}

template <typename DType> inline DType DKinematicData::Get_TObject(string locVariableName, bool locComboDataFlag) const
{
	bool locUseComboData = (Get_IsComboParticle() && locComboDataFlag);

	UInt_t locArrayIndex = locUseComboData ? dArrayIndex : dMeasuredArrayIndex;
	string locBranchNamePrefix = locUseComboData ? dBranchNamePrefix : dMeasuredBranchNamePrefix;
	string locBranchName = locBranchNamePrefix + string("__") + locVariableName;

	return dTreeInterface->Get_TObject<DType>(locBranchName, locArrayIndex);
}

#endif //DKinematicData_h
