#ifndef DBeamParticle_h
#define DBeamParticle_h

#include <string>

#include "TBranch.h"
#include "TClonesArray.h"

#include <particleType.h>

#include "DTreeInterface.h"
#include "DKinematicData.h"

using namespace std;

class DSelector;
class DParticleComboStep;

//https://halldweb1.jlab.org/wiki/index.php/Analysis_TTreeFormat
class DBeamParticle : public DKinematicData
{
	friend class DSelector; //for calling ReInitialize
	friend class DParticleComboStep; //for calling ReInitialize

	public:

		DBeamParticle(DTreeInterface* locTreeInterface, string locBranchNamePrefix, Particle_t locPID = Unknown);
		~DBeamParticle(void){}

		Bool_t Get_IsGenerator(void) const;
		Int_t Get_BeamID(void) const;

		//GET ID (Overriding base class)
		Int_t Get_ID(void) const{return Get_BeamID();}

	private:
		DBeamParticle(void); //Cannot call default constructor!

		void Setup_Branches(void);

		//RE-INITIALIZE (e.g. with the next TTree in a chain)
		void ReInitialize(void);

		//For fundamental arrays
			//cannot store array pointer: when array size is increased, a new array is created
			//must store branch pointer instead

		TBranch* dBranch_IsGenerator;
};

/******************************************************************** CONSTRUCTOR *********************************************************************/

inline DBeamParticle::DBeamParticle(DTreeInterface* locTreeInterface, string locBranchNamePrefix, Particle_t locPID) : 
DKinematicData(locTreeInterface, locBranchNamePrefix, locPID)
{
	Setup_Branches();
}

/***************************************************************** SETUP DATA ACCESS ******************************************************************/

inline void DBeamParticle::Setup_Branches(void)
{
	if(dBranchNamePrefix == "ThrownBeam")
		return;

	dBranch_IsGenerator = dTreeInterface->Get_Branch("Beam__IsGenerator");
}

inline void DBeamParticle::ReInitialize(void)
{
	DKinematicData::ReInitialize();
	Setup_Branches();
}

/****************************************************************** GET OBJECT DATA *******************************************************************/

inline Bool_t DBeamParticle::Get_IsGenerator(void) const
{
	if(dBranchNamePrefix == "ThrownBeam")
		return kTRUE;
	return ((Bool_t*)dBranch_IsGenerator->GetAddress())[dMeasuredArrayIndex];
}

inline Int_t DBeamParticle::Get_BeamID(void) const
{
	return dMeasuredArrayIndex;
}

#endif //DBeamParticle_h
