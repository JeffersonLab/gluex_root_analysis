#include "DParticleCombo.h"

using namespace std;

/******************************************************************** CONSTRUCTOR *********************************************************************/

DParticleCombo::DParticleCombo(DTreeInterface* locTreeInterface) : dTreeInterface(locTreeInterface), dComboIndex(0)
{
	//get combo info
	map<int, map<int, pair<Particle_t, string> > > locComboInfoMap;
	size_t locNumSteps = dTreeInterface->Get_ComboInfo(locComboInfoMap);

	//build particle map
	pair<int, int> locMissingIndices(-1, -1); //1st is step index, 2nd is particle index
	map<int, map<int, pair<Particle_t, DKinematicData*> > > locParticleMap; //1st key is step, 2nd is particle
	map<int, map<int, pair<Particle_t, string> > >::iterator locStepIterator = locComboInfoMap.begin();
	map<string, DKinematicData*> locDecayingParticleMap;
	DKinematicData* locMissingParticle = NULL;
	for(; locStepIterator != locComboInfoMap.end(); ++locStepIterator)
	{
		int locStepIndex = locStepIterator->first;
		map<int, pair<Particle_t, string> >& locStepInfoMap = locStepIterator->second;
		map<int, pair<Particle_t, string> >::iterator locParticleIterator = locStepInfoMap.begin();
		for(; locParticleIterator != locStepInfoMap.end(); ++locParticleIterator)
		{
			int locParticleIndex = locParticleIterator->first;
			Particle_t locPID = locParticleIterator->second.first;
			string locParticleName = locParticleIterator->second.second;

			//Create particle
			DKinematicData* locKinematicData = NULL;
			if(locPID == Unknown)
				locKinematicData = NULL;
			else if(locParticleName == "ComboBeam")
				locKinematicData = new DBeamParticle(dTreeInterface, locParticleName, locPID);
			else if(locParticleName.substr(0, 6) == "Target")
				locKinematicData = new DKinematicData(dTreeInterface, locParticleName, locPID);
			else if(locParticleName.substr(0, 8) == "Decaying")
			{
				string locBranchName = locParticleName + string("__P4_KinFit");
				if(dTreeInterface->Get_Branch(locBranchName) != NULL) //else not reconstructed
				{
					map<string, DKinematicData*>::iterator locDecayingIterator = locDecayingParticleMap.find(locParticleName);
					if(locDecayingIterator != locDecayingParticleMap.end())
						locKinematicData = locDecayingIterator->second; //already created!
					else
					{
						locKinematicData = new DKinematicData(dTreeInterface, locParticleName, locPID);
						locDecayingParticleMap[locParticleName] = locKinematicData;
					}
				}
			}
			else if(locParticleName.substr(0, 7) == "Missing")
			{
				string locBranchName = locParticleName + string("__P4_KinFit");
				if(dTreeInterface->Get_Branch(locBranchName) != NULL) //else not reconstructed
				{
					locKinematicData = new DKinematicData(dTreeInterface, locParticleName, locPID);
					locMissingParticle = locKinematicData;
				}
				locMissingIndices = pair<int, int>(locStepIndex, locParticleIndex);
			}
			else if(ParticleCharge(locPID) != 0)
				locKinematicData = new DChargedTrackHypothesis(dTreeInterface, locParticleName, locPID);
			else
				locKinematicData = new DNeutralParticleHypothesis(dTreeInterface, locParticleName, locPID);
			locParticleMap[locStepIndex][locParticleIndex] = pair<Particle_t, DKinematicData*>(locPID, locKinematicData);
		}
	}

	//Set target info directly
	dTargetPID = dTreeInterface->Get_TargetPID();
	dTargetCenter = dTreeInterface->Get_TargetCenter();

	//Create steps
	for(size_t loc_i = 0; loc_i < locNumSteps; ++loc_i)
		dParticleComboSteps.push_back(new DParticleComboStep(dTreeInterface, locParticleMap[loc_i]));

	//Set decay indices
	for(size_t loc_i = 0; loc_i < locNumSteps; ++loc_i)
	{
		dParticleComboSteps[loc_i]->dInitDecayFromIndices = Get_DecayFromIndices(loc_i);
		for(size_t loc_j = 0; loc_j < dParticleComboSteps[loc_i]->Get_NumFinalParticles(); ++loc_j)
		{
			//decay index
			int locDecayStepIndex = Get_DecayStepIndex(loc_i, loc_j);
			dParticleComboSteps[loc_i]->dDecayStepIndices.push_back(locDecayStepIndex);
		}
	}

	//Set missing
	if(locMissingIndices.first != -1)
	{
		if(locMissingIndices.second != -1)
			dParticleComboSteps[locMissingIndices.first]->dDecayStepIndices[locMissingIndices.second] = -1;
		else //initial particle
			dParticleComboSteps[locMissingIndices.first]->dInitDecayFromIndices = pair<int, int>(-1, -2);
		dParticleComboSteps[locMissingIndices.first]->dMissingParticleIndex = locMissingIndices.second;
	}

	Setup_Branches();
	Setup_X4Branches();

	Print_Reaction();
}

int DParticleCombo::Get_DecayStepIndex(int locStepIndex, int locParticleIndex) const
{
	//check if the input particle decays later in the reaction
	Particle_t locDecayingPID = dParticleComboSteps[locStepIndex]->Get_FinalPID(locParticleIndex);

	if((locDecayingPID == Gamma) || (locDecayingPID == Electron) || (locDecayingPID == Positron) || (locDecayingPID == Proton) || (locDecayingPID == AntiProton))
		return -2; //these particles don't decay: don't search!

	//check to see how many final state particles with this pid type there are before now
	size_t locPreviousPIDCount = 0;
	for(int loc_i = 0; loc_i <= locStepIndex; ++loc_i)
	{
		const DParticleComboStep* locComboStep = dParticleComboSteps[loc_i];
		for(size_t loc_j = 0; loc_j < locComboStep->Get_NumFinalParticles(); ++loc_j)
		{
			if((loc_i == locStepIndex) && (int(loc_j) == locParticleIndex))
				break; //at the current particle: of the search
			if(locComboStep->Get_FinalPID(loc_j) == locDecayingPID)
				++locPreviousPIDCount;
		}
	}

	//now, find the (locPreviousPIDCount + 1)'th time where this pid is a decay parent
	size_t locStepPIDCount = 0;
	for(size_t loc_i = 0; loc_i < dParticleComboSteps.size(); ++loc_i)
	{
		if(dParticleComboSteps[loc_i]->Get_InitialPID() != locDecayingPID)
			continue;
		++locStepPIDCount;
		if(locStepPIDCount <= locPreviousPIDCount)
			continue;
		//decays later in the reaction, at step index loc_i
		return loc_i;
	}

	// does not decay later in the reaction
	return -2;
}

pair<int, int> DParticleCombo::Get_DecayFromIndices(int locStepIndex) const
{
	//check to see how many initial-state particles with this PID type there are before now
	Particle_t locDecayingPID = dParticleComboSteps[locStepIndex]->Get_InitialPID();
	size_t locPreviousPIDCount = 0;
	for(int loc_i = 0; loc_i < locStepIndex; ++loc_i)
	{
		if(dParticleComboSteps[loc_i]->Get_InitialPID() == locDecayingPID)
			++locPreviousPIDCount;
	}

	//now, search through final-state PIDs until finding the (locPreviousPIDCount + 1)'th instance of this PID
	size_t locSearchPIDCount = 0;
	for(int loc_i = 0; loc_i < locStepIndex; ++loc_i)
	{
		DParticleComboStep* locComboStep = dParticleComboSteps[loc_i];
		for(size_t loc_j = 0; loc_j < locComboStep->Get_NumFinalParticles(); ++loc_j)
		{
			if(locComboStep->Get_FinalPID(loc_j) != locDecayingPID)
				continue;
			++locSearchPIDCount;
			if(locSearchPIDCount <= locPreviousPIDCount)
				continue;
			return pair<int, int>(loc_i, loc_j);
		}
	}

	return pair<int, int>(-1, -1);
}

void DParticleCombo::Setup_X4Branches(void)
{
	//Set X4 for decaying particles & steps
	for(size_t loc_i = 0; loc_i < dParticleComboSteps.size(); ++loc_i)
	{
		TClonesArray* locX4Step = NULL;
		TBranch* locBranch_X4MeasuredIndex = NULL;

		//First check if in tree
		DKinematicData* locInitialParticle = dParticleComboSteps[loc_i]->Get_InitialParticle();
		if(locInitialParticle != NULL)
		{
			if(locInitialParticle->dX4_KinFit != NULL)
				locX4Step = locInitialParticle->dX4_KinFit;
			else if(locInitialParticle->dX4_Measured != NULL)
			{
				locX4Step = locInitialParticle->dX4_Measured;
				locBranch_X4MeasuredIndex = locInitialParticle->dBranch_MeasuredIndex;
			}
		}
		if(locX4Step != NULL)
		{
			dParticleComboSteps[loc_i]->dX4_Step = locX4Step;
			dParticleComboSteps[loc_i]->dBranch_X4MeasuredIndex = locBranch_X4MeasuredIndex;
			continue; //in tree: done
		}

		//not in tree: get from previous step
		int locDecayFromStepIndex = dParticleComboSteps[loc_i]->Get_InitDecayFromIndices().first;
		locX4Step = dParticleComboSteps[locDecayFromStepIndex]->dX4_Step;
		dParticleComboSteps[loc_i]->dX4_Step = locX4Step;
		locBranch_X4MeasuredIndex = dParticleComboSteps[locDecayFromStepIndex]->dBranch_X4MeasuredIndex;
		dParticleComboSteps[loc_i]->dBranch_X4MeasuredIndex = locBranch_X4MeasuredIndex;

		//decaying particle x4
		if(dParticleComboSteps[loc_i]->dInitialParticle == NULL)
			continue; //not reconstructed
		if(dParticleComboSteps[loc_i]->dInitialParticle->dX4_Measured != NULL)
			continue; //already set (i.e. detached vertex)

		//not a detached vertex: use the x4 for the previous step
		dParticleComboSteps[loc_i]->dInitialParticle->dX4_Measured = locX4Step;
		dParticleComboSteps[loc_i]->dInitialParticle->dBranch_MeasuredIndex = locBranch_X4MeasuredIndex;
	}

	//Set X4 for missing particles
	DKinematicData* locMissingParticle = Get_MissingParticle();
	if(locMissingParticle != NULL)
	{
		DParticleComboStep* locMissingStep = Get_MissingParticleStep();
		locMissingParticle->dX4_Measured = locMissingStep->dX4_Step;
		locMissingParticle->dBranch_MeasuredIndex = locMissingStep->dBranch_X4MeasuredIndex;
	}
}
