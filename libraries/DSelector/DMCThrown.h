#ifndef DMCThrown_h
#define DMCThrown_h

#include <string>

#include "TBranch.h"

#include <particleType.h>

#include "DTreeInterface.h"
#include "DKinematicData.h"

using namespace std;

class DSelector;
class DParticleComboStep;

//https://halldweb1.jlab.org/wiki/index.php/Analysis_TTreeFormat
class DMCThrown : public DKinematicData
{
	friend class DSelector; //for calling ReInitialize
	friend class DParticleComboStep; //for calling ReInitialize

	public:

		DMCThrown(DTreeInterface* locTreeInterface);
		~DMCThrown(void){}

		//GET ID (Overriding base class)
		Int_t Get_ID(void) const{return dMeasuredArrayIndex;}

		Int_t Get_ParentIndex(void) const; //the thrown particle array index of the particle this particle decayed from (-1 if none (e.g. photoproduced))

		Int_t Get_MatchID(void) const; //the "NeutralID"/"TrackID" of the reconstructed shower/track that it is matched with (-1 for no match)
		Float_t Get_MatchFOM(void) const; //Neutrals: confidence level //Tracks: #-matched-hits * hit_fraction //(-1 for no match)

	private:
		DMCThrown(void); //Cannot call default constructor!

		void Setup_Branches(void);

		//RE-INITIALIZE (e.g. with the next TTree in a chain)
		void ReInitialize(void);

		//For fundamental arrays
			//cannot store array pointer: when array size is increased, a new array is created
			//must store branch pointer instead

		TBranch* dBranch_ParentIndex;
		TBranch* dBranch_MatchID;
		TBranch* dBranch_MatchFOM;
};

/******************************************************************** CONSTRUCTOR *********************************************************************/

inline DMCThrown::DMCThrown(DTreeInterface* locTreeInterface) : 
DKinematicData(locTreeInterface, "Thrown", Unknown)
{
	Setup_Branches();
}

/***************************************************************** SETUP DATA ACCESS ******************************************************************/

inline void DMCThrown::Setup_Branches(void)
{
	string locBranchName;

	locBranchName = "Thrown__ParentIndex";
	dBranch_ParentIndex = dTreeInterface->Get_Branch(locBranchName);

	locBranchName = "Thrown__MatchID";
	dBranch_MatchID = dTreeInterface->Get_Branch(locBranchName);

	locBranchName = "Thrown__MatchFOM";
	dBranch_MatchFOM = dTreeInterface->Get_Branch(locBranchName);
}

inline void DMCThrown::ReInitialize(void)
{
	DKinematicData::ReInitialize();
	Setup_Branches();
}

/****************************************************************** GET OBJECT DATA *******************************************************************/

inline Int_t DMCThrown::Get_ParentIndex(void) const
{
	return ((Int_t*)dBranch_ParentIndex->GetAddress())[dMeasuredArrayIndex];
}

inline Int_t DMCThrown::Get_MatchID(void) const
{
	return ((Int_t*)dBranch_MatchID->GetAddress())[dMeasuredArrayIndex];
}

inline Float_t DMCThrown::Get_MatchFOM(void) const
{
	return ((Float_t*)dBranch_MatchFOM->GetAddress())[dMeasuredArrayIndex];
}


#endif //DMCThrown_h

