#ifndef _DParticleComboStep_
#define _DParticleComboStep_

#include <deque>
#include <map>
#include <string>
#include <iostream>

#include "TLorentzVector.h"
#include "TMap.h"

#include "particleType.h"
#include "DKinematicData.h"

using namespace std;

class DParticleCombo;
class DSelector;

class DParticleComboStep
{
	friend class DParticleCombo; //for setting up
	friend class DSelector; //for calling ReInitialize

	public:

		// CONSTRUCTOR:
		DParticleComboStep(DTreeInterface* locTreeInterface, const map<int, pair<Particle_t, DKinematicData*> >& locParticleMap);

		// GET PIDS:
		Particle_t Get_InitialPID(void) const{return dInitialPID;}
		Particle_t Get_TargetPID(void) const{return ((dTargetParticle != NULL) ? dTargetParticle->Get_PID() : Unknown);}
		Particle_t Get_FinalPID(size_t locIndex) const{return dFinalStatePIDs[locIndex];}
		deque<Particle_t> Get_FinalPIDs(void) const{return dFinalStatePIDs;}

		// GET PARTICLES:
		DKinematicData* Get_InitialParticle(void) const{return dInitialParticle;}
		DKinematicData* Get_TargetParticle(void) const{return dTargetParticle;}
		size_t Get_NumFinalParticles(void) const{return dFinalParticles.size();}
		DKinematicData* Get_FinalParticle(size_t locIndex) const{return dFinalParticles[locIndex];}
		DKinematicData* Get_MissingParticle(void) const; //NULL if none or not reconstructed

		//GET PARTICLES BY TRAIT
		deque<DKinematicData*> Get_FinalParticles(bool locOnlyDetectedFlag = false, Particle_t locDesiredPID = Unknown) const;
		deque<DKinematicData*> Get_FinalParticles_ByCharge(bool locChargedFlag, bool locOnlyDetectedFlag = false) const; //locChargedFlag = false for neutrals

		// GET INDICES
		int Get_DecayStepIndex(int locParticleIndex) const{return dDecayStepIndices[locParticleIndex];}
		int Get_MissingParticleIndex(void) const{return dMissingParticleIndex;}
		pair<int, int> Get_InitDecayFromIndices(void) const{return dInitDecayFromIndices;}

		// GET TRAITS - FINAL PARTICLES:
		inline bool Is_FinalParticleDetected(size_t locFinalParticleIndex) const{return (Get_DecayStepIndex(locFinalParticleIndex) == -2);}
		inline bool Is_FinalParticleDecaying(size_t locFinalParticleIndex) const{return (Get_DecayStepIndex(locFinalParticleIndex) >= 0);}
		inline bool Is_FinalParticleMissing(size_t locFinalParticleIndex) const{return (Get_DecayStepIndex(locFinalParticleIndex) == -1);}

		// GET STEP X4
		TLorentzVector Get_X4(void) const;

		void Set_ComboIndex(UInt_t locComboIndex);
		void Print_Reaction(void) const;

		//GENERIC-CODE HELPER FUNCTIONS
		string Get_InitialParticlesROOTName(void) const;
		string Get_FinalParticlesROOTName(void) const;
		void Get_FinalParticlesROOTName(deque<string>& locParticleNames) const;
		string Get_FinalNonMissingParticlesROOTName(void) const;
		string Get_StepROOTName(void) const;
		string Get_StepName(void) const;

	private:
		DParticleComboStep(void){}; //no default constructor!

		//RE-INITIALIZE (e.g. with the next TTree in a chain)
		void ReInitialize(void);

		DTreeInterface* dTreeInterface;
		UInt_t dComboIndex; //the index in the particle-data arrays to use to grab particle data (e.g. corresponding to this combo)

		// INITIAL PARTICLES:
		DKinematicData* dInitialParticle; //if is null: decaying particle not reconstructed by kinematic fitter!
		DKinematicData* dTargetParticle; //NULL for no target

		// FINAL PARTICLES: //are NULL if not reconstructed or fit
		deque<DKinematicData*> dFinalParticles; //these are DChargedTrackHypothesis or DNeutralParticleHypothesis objects if detected

		// PIDs //in case the particles are NULL
		Particle_t dInitialPID;
		deque<Particle_t> dFinalStatePIDs;

		// To/From, Particle Indices
		pair<int, int> dInitDecayFromIndices; //step, final-state particle indices //-1 for both if beam //-1, -2 if missing
		deque<int> dDecayStepIndices; //-2 if detected, -1 if missing, > 0 if decaying (step where it is the parent)
		int dMissingParticleIndex; //-2 if none, -1 if parent, > 0 if final state

		// SPACETIME VERTEX
		TClonesArray** dX4_Step;
		TBranch* dBranch_X4MeasuredIndex; //branch index for dX4_Step if retrieved from a measured source //if kinfit, is just combo index
};

inline DParticleComboStep::DParticleComboStep(DTreeInterface* locTreeInterface, const map<int, pair<Particle_t, DKinematicData*> >& locParticleMap) : 
dTreeInterface(locTreeInterface), dComboIndex(0), dInitialParticle(NULL), dTargetParticle(NULL)
{
	dInitialPID = Unknown;
	dMissingParticleIndex = -2;
	dX4_Step = NULL;
	dBranch_X4MeasuredIndex = NULL;

	//ParticleIndex = -1 for initial, -2 for target, 0+ for final state
	map<int, pair<Particle_t, DKinematicData*> >::const_iterator locIterator = locParticleMap.begin();
	for(; locIterator != locParticleMap.end(); ++locIterator)
	{
		if(locIterator->first == -1)
		{
			dInitialPID = locIterator->second.first;
			dInitialParticle = locIterator->second.second;
		}
		else if(locIterator->first == -2)
			dTargetParticle = locIterator->second.second;
		else //guaranteed to be in order
		{
			dFinalStatePIDs.push_back(locIterator->second.first);
			dFinalParticles.push_back(locIterator->second.second);
		}
	}
}

inline void DParticleComboStep::Set_ComboIndex(UInt_t locComboIndex)
{
	dComboIndex = locComboIndex;
	if(dInitialParticle != NULL)
		dInitialParticle->Set_ArrayIndex(locComboIndex);
	if(dTargetParticle != NULL)
		dTargetParticle->Set_ArrayIndex(locComboIndex);
	for(size_t loc_i = 0; loc_i < dFinalParticles.size(); ++loc_i)
	{
		if(dFinalParticles[loc_i] != NULL)
			dFinalParticles[loc_i]->Set_ArrayIndex(locComboIndex);
	}
}

inline void DParticleComboStep::ReInitialize(void)
{
	if(dInitialParticle != NULL)
		dInitialParticle->ReInitialize();
	if(dTargetParticle != NULL)
		dTargetParticle->ReInitialize();
	for(size_t loc_i = 0; loc_i < dFinalParticles.size(); ++loc_i)
	{
		if(dFinalParticles[loc_i] != NULL)
			dFinalParticles[loc_i]->ReInitialize();
	}
}

inline void DParticleComboStep::Print_Reaction(void) const
{
	cout << ParticleType(dInitialPID);
	if(dTargetParticle != NULL)
		cout << ", " << ParticleType(dTargetParticle->Get_PID());
	cout << " -> ";
	for(size_t loc_i = 0; loc_i < dFinalStatePIDs.size(); ++loc_i)
	{
		bool locMissingFlag = (dDecayStepIndices[loc_i] == -1);
		if(locMissingFlag)
			cout << "(";
		cout << ParticleType(dFinalStatePIDs[loc_i]);
		if(locMissingFlag)
			cout << ")";
		if(loc_i != (dFinalStatePIDs.size() - 1))
			cout << ", ";
	}
	cout << endl;
}

inline TLorentzVector DParticleComboStep::Get_X4(void) const
{
	UInt_t locArrayIndex = (dBranch_X4MeasuredIndex == NULL) ? dComboIndex : ((UInt_t*)dBranch_X4MeasuredIndex->GetAddress())[dComboIndex];
	return *((TLorentzVector*)(*dX4_Step)->At(locArrayIndex));
}

inline DKinematicData* DParticleComboStep::Get_MissingParticle(void) const
{
	//dMissingParticleIndex: -2 if none, -1 if parent, > 0 if final state
	if(dMissingParticleIndex == -2)
		return NULL;
	if(dMissingParticleIndex == -1)
		return dInitialParticle;
	return dFinalParticles[dMissingParticleIndex];
}

/*********************************************************** GENERIC-CODE HELPER FUNCTIONS ************************************************************/

inline deque<DKinematicData*> DParticleComboStep::Get_FinalParticles(bool locOnlyDetectedFlag, Particle_t locDesiredPID) const
{
	deque<DKinematicData*> locParticles;
	for(size_t loc_i = 0; loc_i < dFinalParticles.size(); ++loc_i)
	{
		if(locOnlyDetectedFlag && !Is_FinalParticleDetected(loc_i))
			continue;

		Particle_t locPID = Get_FinalPID(loc_i);
		if((locPID == locDesiredPID) || (locDesiredPID == Unknown))
			locParticles.push_back(dFinalParticles[loc_i]);
	}

	return locParticles;
}

inline deque<DKinematicData*> DParticleComboStep::Get_FinalParticles_ByCharge(bool locChargedFlag, bool locOnlyDetectedFlag) const
{
	deque<DKinematicData*> locParticles;
	for(size_t loc_i = 0; loc_i < dFinalParticles.size(); ++loc_i)
	{
		if(locOnlyDetectedFlag && !Is_FinalParticleDetected(loc_i))
			continue;

		int locCharge = ParticleCharge(Get_FinalPID(loc_i));
		if(((locCharge == 0) && !locChargedFlag) || ((locCharge != 0) && locChargedFlag))
			locParticles.push_back(dFinalParticles[loc_i]);
	}

	return locParticles;
}

inline string DParticleComboStep::Get_InitialParticlesROOTName(void) const
{
	string locStepROOTName = ParticleName_ROOT(dInitialPID);
	if(Get_TargetPID() != Unknown)
		locStepROOTName += ParticleName_ROOT(Get_TargetPID());
	return locStepROOTName;
}

inline string DParticleComboStep::Get_FinalParticlesROOTName(void) const
{
	string locStepROOTName;
	for(size_t loc_i = 0; loc_i < dFinalStatePIDs.size(); ++loc_i)
	{
		if(int(loc_i) == dMissingParticleIndex)
			continue;
		locStepROOTName += ParticleName_ROOT(dFinalStatePIDs[loc_i]);
	}
	if(dMissingParticleIndex >= 0)
		locStepROOTName += string("(") + ParticleName_ROOT(dFinalStatePIDs[dMissingParticleIndex]) + string(")");
	return locStepROOTName;
}

inline void DParticleComboStep::Get_FinalParticlesROOTName(deque<string>& locParticleNames) const
{
	locParticleNames.clear();
	for(size_t loc_i = 0; loc_i < dFinalStatePIDs.size(); ++loc_i)
	{
		if(int(loc_i) == dMissingParticleIndex)
			continue;
		locParticleNames.push_back(ParticleName_ROOT(dFinalStatePIDs[loc_i]));
	}
	if(dMissingParticleIndex >= 0)
		locParticleNames.push_back(string("(") + ParticleName_ROOT(dFinalStatePIDs[dMissingParticleIndex]) + string(")"));
}

inline string DParticleComboStep::Get_FinalNonMissingParticlesROOTName(void) const
{
	string locStepROOTName;
	for(size_t loc_i = 0; loc_i < dFinalStatePIDs.size(); ++loc_i)
	{
		if(int(loc_i) == dMissingParticleIndex)
			continue;
		locStepROOTName += ParticleName_ROOT(dFinalStatePIDs[loc_i]);
	}
	return locStepROOTName;
}

inline string DParticleComboStep::Get_StepROOTName(void) const
{
	string locStepROOTName = Get_InitialParticlesROOTName();
	locStepROOTName += "#rightarrow";
	locStepROOTName += Get_FinalParticlesROOTName();
	return locStepROOTName;
}

inline string DParticleComboStep::Get_StepName(void) const
{
	string locStepName = ParticleType(dInitialPID);
	if(Get_TargetPID() != Unknown)
		locStepName += string("_") + ParticleType(Get_TargetPID());
	locStepName += "_->";
	for(size_t loc_i = 0; loc_i < dFinalStatePIDs.size(); ++loc_i)
	{
		if(int(loc_i) == dMissingParticleIndex)
			continue;
		locStepName += string("_") + ParticleType(dFinalStatePIDs[loc_i]);
	}
	if(dMissingParticleIndex >= 0)
		locStepName += string("_(") + ParticleType(dFinalStatePIDs[dMissingParticleIndex]) + string(")");
	return locStepName;
}

#endif // _DParticleComboStep_
