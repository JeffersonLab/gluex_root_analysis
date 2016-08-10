#include "DAnalysisUtilities.h"

TLorentzVector DAnalysisUtilities::Calc_MissingP4(const DParticleCombo* locParticleComboWrapper, bool locUseKinFitDataFlag) const
{
	map<Particle_t, set<Int_t> > locSourceObjects;
	return Calc_MissingP4(locParticleComboWrapper, 0, -1, set<size_t>(), locSourceObjects, locUseKinFitDataFlag);
}

TLorentzVector DAnalysisUtilities::Calc_MissingP4(const DParticleCombo* locParticleComboWrapper, map<Particle_t, set<Int_t> >& locSourceObjects, bool locUseKinFitDataFlag) const
{
	return Calc_MissingP4(locParticleComboWrapper, 0, -1, set<size_t>(), locSourceObjects, locUseKinFitDataFlag);
}

TLorentzVector DAnalysisUtilities::Calc_MissingP4(const DParticleCombo* locParticleComboWrapper, size_t locStepIndex, int locUpToStepIndex, set<size_t> locUpThroughIndices, bool locUseKinFitDataFlag) const
{
	map<Particle_t, set<Int_t> > locSourceObjects;
	return Calc_MissingP4(locParticleComboWrapper, locStepIndex, locUpToStepIndex, locUpThroughIndices, locSourceObjects, locUseKinFitDataFlag);
}

TLorentzVector DAnalysisUtilities::Calc_MissingP4(const DParticleCombo* locParticleComboWrapper, size_t locStepIndex, int locUpToStepIndex, set<size_t> locUpThroughIndices, map<Particle_t, set<Int_t> >& locSourceObjects, bool locUseKinFitDataFlag) const
{
	//NOTE: this routine assumes that the p4 of a charged decaying particle with a detached vertex is the same at both vertices!
	//assumes missing particle is not the beam particle
	if(locUseKinFitDataFlag && (locParticleComboWrapper->Get_NDF_KinFit() == 0))
		return Calc_MissingP4(locParticleComboWrapper, locStepIndex, locUpToStepIndex, locUpThroughIndices, locSourceObjects, false); //kinematic fit failed

	TLorentzVector locMissingP4;
	const DParticleComboStep* locParticleComboStepWrapper = locParticleComboWrapper->Get_ParticleComboStep(locStepIndex);

	if(locStepIndex == 0)
	{
		//initial particle
		DKinematicData* locKinematicData = locParticleComboStepWrapper->Get_InitialParticle();
		locSourceObjects[Unknown].insert(locKinematicData->Get_ID()); //Unknown for beam: don't mix with final-state photons //not ideal, but easy
		if(!locUseKinFitDataFlag) //measured
			locMissingP4 += locKinematicData->Get_P4_Measured();
		else
			locMissingP4 += locKinematicData->Get_P4();

		//target particle
		locKinematicData = locParticleComboStepWrapper->Get_TargetParticle();
		if(locKinematicData != NULL)
			locMissingP4 += locKinematicData->Get_P4();
	}

	deque<DKinematicData*> locParticles = locParticleComboStepWrapper->Get_FinalParticles();
	for(size_t loc_j = 0; loc_j < locParticles.size(); ++loc_j)
	{
		//DecayStepIndex: one for each final particle: -2 if detected, -1 if missing, >= 0 if decaying, where the # is the step representing the particle decay
		int locDecayStepIndex = locParticleComboStepWrapper->Get_DecayStepIndex(loc_j);
		if((locDecayStepIndex == -1) || (locDecayStepIndex == -3))
			continue; //missing particle or no blueprint

		if((int(locStepIndex) == locUpToStepIndex) && (locUpThroughIndices.find(loc_j) == locUpThroughIndices.end()))
			continue; //skip it: don't want to include it

		Particle_t locPID = locParticleComboStepWrapper->Get_FinalPID(loc_j);
		if(locDecayStepIndex == -2) //detected
		{
			if(!locUseKinFitDataFlag) //measured
				locMissingP4 -= locParticles[loc_j]->Get_P4_Measured();
			else
				locMissingP4 -= locParticles[loc_j]->Get_P4();
			locSourceObjects[locPID].insert(locParticles[loc_j]->Get_ID());
		}
		else //decaying-particle
			locMissingP4 += Calc_MissingP4(locParticleComboWrapper, locDecayStepIndex, locUpToStepIndex, locUpThroughIndices, locSourceObjects, locUseKinFitDataFlag); //p4 returned is already < 0
	}

	return locMissingP4;
}

TLorentzVector DAnalysisUtilities::Calc_FinalStateP4(const DParticleCombo* locParticleComboWrapper, size_t locStepIndex, bool locUseKinFitDataFlag) const
{
	map<Particle_t, set<Int_t> > locSourceObjects;
	return Calc_FinalStateP4(locParticleComboWrapper, locStepIndex, set<size_t>(), locSourceObjects, locUseKinFitDataFlag);
}

TLorentzVector DAnalysisUtilities::Calc_FinalStateP4(const DParticleCombo* locParticleComboWrapper, size_t locStepIndex, map<Particle_t, set<Int_t> >& locSourceObjects, bool locUseKinFitDataFlag) const
{
	return Calc_FinalStateP4(locParticleComboWrapper, locStepIndex, set<size_t>(), locSourceObjects, locUseKinFitDataFlag);
}

TLorentzVector DAnalysisUtilities::Calc_FinalStateP4(const DParticleCombo* locParticleComboWrapper, size_t locStepIndex, set<size_t> locToIncludeIndices, bool locUseKinFitDataFlag) const
{
	map<Particle_t, set<Int_t> > locSourceObjects;
	return Calc_FinalStateP4(locParticleComboWrapper, locStepIndex, locToIncludeIndices, locSourceObjects, locUseKinFitDataFlag);
}

TLorentzVector DAnalysisUtilities::Calc_FinalStateP4(const DParticleCombo* locParticleComboWrapper, size_t locStepIndex, set<size_t> locToIncludeIndices, map<Particle_t, set<Int_t> >& locSourceObjects, bool locUseKinFitDataFlag) const
{
	if(locUseKinFitDataFlag && (locParticleComboWrapper->Get_NDF_KinFit() == 0))
		return Calc_FinalStateP4(locParticleComboWrapper, locStepIndex, locToIncludeIndices, locSourceObjects, false); //kinematic fit failed

	TLorentzVector locFinalStateP4;
	const DParticleComboStep* locParticleComboStepWrapper = locParticleComboWrapper->Get_ParticleComboStep(locStepIndex);
	if(locParticleComboStepWrapper == NULL)
		return (TLorentzVector());

	bool locDoSubsetFlag = !locToIncludeIndices.empty();
	deque<DKinematicData*> locParticles = locParticleComboStepWrapper->Get_FinalParticles();
	for(size_t loc_i = 0; loc_i < locParticles.size(); ++loc_i)
	{
		if(locDoSubsetFlag && (locToIncludeIndices.find(loc_i) == locToIncludeIndices.end()))
			continue; //skip it: don't want to include it

		if(locParticleComboStepWrapper->Is_FinalParticleMissing(loc_i))
			return (TLorentzVector()); //bad!

		if(locParticleComboStepWrapper->Is_FinalParticleDecaying(loc_i))
		{
			//measured results, or not constrained by kinfit (either non-fixed mass or excluded from kinfit)
			if((!locUseKinFitDataFlag) || (!IsFixedMass(locParticleComboStepWrapper->Get_FinalPID(loc_i))))
				locFinalStateP4 += Calc_FinalStateP4(locParticleComboWrapper, locParticleComboStepWrapper->Get_DecayStepIndex(loc_i), set<size_t>(), locSourceObjects, locUseKinFitDataFlag);
			else //want kinfit results, and decaying particle p4 is constrained by kinfit
			{
				if(!locUseKinFitDataFlag) //measured
					locFinalStateP4 += locParticles[loc_i]->Get_P4_Measured();
				else
					locFinalStateP4 += locParticles[loc_i]->Get_P4();
				//still need source objects of decay products! dive down anyway, but ignore p4 result
				Calc_FinalStateP4(locParticleComboWrapper, locParticleComboStepWrapper->Get_DecayStepIndex(loc_i), set<size_t>(), locSourceObjects, locUseKinFitDataFlag);
			}
		}
		else
		{
			if(!locUseKinFitDataFlag) //measured
				locFinalStateP4 += locParticles[loc_i]->Get_P4_Measured();
			else
				locFinalStateP4 += locParticles[loc_i]->Get_P4();
			locSourceObjects[locParticles[loc_i]->Get_PID()].insert(locParticles[loc_i]->Get_ID());
		}
	}
	return locFinalStateP4;
}

set<set<size_t> > DAnalysisUtilities::Build_IndexCombos(const DParticleComboStep* locParticleComboStepWrapper, deque<Particle_t> locToIncludePIDs) const
{
	//if locToIncludePIDs is empty, will return one set with all (except missing)
	set<set<size_t> > locCombos;

	deque<Particle_t> locParticleComboStepWrapperPIDs;
	deque<DKinematicData*> locParticles = locParticleComboStepWrapper->Get_FinalParticles();
	int locMissingParticleIndex = locParticleComboStepWrapper->Get_MissingParticleIndex();

	if(locToIncludePIDs.empty())
	{
		set<size_t> locCombo;
		for(size_t loc_i = 0; loc_i < locParticles.size(); ++loc_i)
		{
			if(int(loc_i) == locMissingParticleIndex)
				continue;
			locCombo.insert(loc_i);
		}
		locCombos.insert(locCombo);
		return locCombos;
	}

	//deque indices corresponds to locToIncludePIDs, and each set is what could pass for it
	deque<deque<size_t> > locPossibilities(locToIncludePIDs.size(), deque<size_t>());
	deque<int> locResumeAtIndices(locToIncludePIDs.size(), 0);

	//build possibilities: loop over reaction PIDs
	for(size_t loc_i = 0; loc_i < locParticles.size(); ++loc_i)
	{
		if(int(loc_i) == locMissingParticleIndex)
			continue;
		Particle_t locPID = locParticles[loc_i]->Get_PID();

		//register where this is a valid option: loop over to-include PIDs
		for(size_t loc_j = 0; loc_j < locToIncludePIDs.size(); ++loc_j)
		{
			if(locToIncludePIDs[loc_j] == locPID)
				locPossibilities[loc_j].push_back(loc_i);
		}
	}

	//build combos
	int locParticleIndex = 0;
	deque<size_t> locComboDeque;
	while(true)
	{
		if(locParticleIndex == int(locPossibilities.size())) //end of combo: save it
		{
			set<size_t> locComboSet; //convert set to deque
			for(size_t loc_i = 0; loc_i < locComboDeque.size(); ++loc_i)
				locComboSet.insert(locComboDeque[loc_i]);
			locCombos.insert(locComboSet); //saved

			if(!Handle_Decursion(locParticleIndex, locComboDeque, locResumeAtIndices, locPossibilities))
				break;
			continue;
		}

		int& locResumeAtIndex = locResumeAtIndices[locParticleIndex];

		//if two identical pids: locResumeAtIndex must always be >= the previous locResumeAtIndex (prevents duplicates) e.g. g, p -> p, pi0, pi0
		//search for same pid previously in this step
		Particle_t locToIncludePID = locToIncludePIDs[locParticleIndex];
		for(int loc_i = locParticleIndex - 1; loc_i >= 0; --loc_i)
		{
			if(locToIncludePIDs[loc_i] == locToIncludePID)
			{
				if(locResumeAtIndex < locResumeAtIndices[loc_i])
					locResumeAtIndex = locResumeAtIndices[loc_i];
				break; //dupe type in step: resume-at advanced to next
			}
		}

		if(locResumeAtIndex >= int(locPossibilities[locParticleIndex].size()))
		{
			if(!Handle_Decursion(locParticleIndex, locComboDeque, locResumeAtIndices, locPossibilities))
				break;
			continue;
		}

		// pid found
		locComboDeque.push_back(locPossibilities[locParticleIndex][locResumeAtIndex]);
		++locResumeAtIndex;
		++locParticleIndex;
	}

	return locCombos;
}

bool DAnalysisUtilities::Handle_Decursion(int& locParticleIndex, deque<size_t>& locComboDeque, deque<int>& locResumeAtIndices, deque<deque<size_t> >& locPossibilities) const
{
	do
	{
		if(locParticleIndex < int(locResumeAtIndices.size())) //else just saved a combo
			locResumeAtIndices[locParticleIndex] = 0; //finding this particle failed: reset

		--locParticleIndex; //go to previous particle
		if(locParticleIndex < 0)
			return false; //end of particles: end of finding all combos

		locComboDeque.pop_back(); //reset this index
	}
	while(locResumeAtIndices[locParticleIndex] == int(locPossibilities[locParticleIndex].size()));

	return true;
}
