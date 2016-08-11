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

double DAnalysisUtilities::Calc_ProdPlanePhi_Pseudoscalar(double locBeamEnergy, Particle_t locTargetPID, TLorentzVector& locMesonP4, bool locIsPARAFlag) const
{
	//Returns angle in degrees!!

	//Polarization plane:
		//Beam is in the lab z-direction
		//The polarization vector is perpendicular to the direction of the photon
		//Linearly polarized photon beam: Polarization is confined to a plane along the direction of the photon
			//Plane defined by z-direction & some angle phi. Thus, polarization vector defined by phi.
			//PARA: Polarization plane parallel to the floor: The XZ plane. Polarization Vector = +/- x-axis
			//PERP: Polarization plane perpendicular to the floor: The YZ plane. Polarization Vector = +/- y-axis
		//(FYI) Circularly polarized photon beam: Polarization rotates through the plane perpendicular to the direction of the photon: The XY Plane

	//Production CM frame: The center-of-mass frame of the production step.
		//In general, the beam energy is measured more accurately than the combination of all of the final-state particles
		//So define the production CM frame using the initial state
	TLorentzVector locBeamP4(0.0, 0.0, locBeamEnergy, locBeamEnergy);
	TLorentzVector locTargetP4(TVector3(), ParticleMass(locTargetPID));
	TLorentzVector locInitialStateP4 = locBeamP4 + locTargetP4;
	TVector3 locBoostVector_ProdCM = -1.0*(locInitialStateP4.BoostVector()); //negative due to coordinate system convention

	//Boost beam & KPlus to production CM frame
	TLorentzVector locBeamP4_ProdCM(locBeamP4);
	locBeamP4_ProdCM.Boost(locBoostVector_ProdCM);
	TLorentzVector locMesonP4_ProdCM(locMesonP4);
	locMesonP4_ProdCM.Boost(locBoostVector_ProdCM);

	//Production plane:
		//The production plane is the plane containing the produced particles.
		//However, when you boost to the production CM frame, the production plane is no longer well defined: the particles are back-to-back
		//So, by convention, define the production plane in the production CM frame by the beam and the meson.

	//Production CM frame axes: "HELICITY SYSTEM"
		//The z-axis is defined as the direction of the meson: z = meson
		//The y-axis is defined by the vector cross product: y = Beam X meson
		//The x-axis is defined by the vector cross product: x = y cross z
		//Thus the production plane in the production frame is the XZ plane, and the normal vector is the Y-axis

	//Define production CM frame helicity axes
	TVector3 locHelicityZAxis_ProdCM = locMesonP4_ProdCM.Vect().Unit();
	TVector3 locHelicityYAxis_ProdCM = locBeamP4_ProdCM.Vect().Cross(locMesonP4_ProdCM.Vect()).Unit();
	TVector3 locHelicityXAxis_ProdCM = locHelicityYAxis_ProdCM.Cross(locHelicityZAxis_ProdCM).Unit();

	//In the production CM frame, locProdPlanePhi is the angle between the production plane and the floor
		// We could have chosen any other fixed plane besides the floor. The point is that it must be a fixed reference
	TVector3 locFloorUnit(0.0, 1.0, 0.0);
	double locCosProdPlanePhi = locBeamP4_ProdCM.Vect().Unit().Dot(locFloorUnit.Cross(locHelicityYAxis_ProdCM));
	double locProdPlanePhi = acos(locCosProdPlanePhi); //reports phi between 0 and pi: sign ambiguity

	//Resolve the sign ambiguity
	double locSinProdPlanePhi = locFloorUnit.Dot(locHelicityYAxis_ProdCM);
	if(locSinProdPlanePhi < 0.0)
		locProdPlanePhi *= -1.0;

	return 180.0*locProdPlanePhi/TMath::Pi();
}

bool DAnalysisUtilities::Get_IsPolarizedBeam(int locRunNumber, bool& locIsPARAFlag) const
{
	//RCDB environment must be setup!!

	//Pipe the current constant into this function
	ostringstream locCommandStream;
	locCommandStream << "rcnd " << locRunNumber << " polarization_direction";
	FILE* locInputFile = gSystem->OpenPipe(locCommandStream.str().c_str(), "r");
	if(locInputFile == NULL)
		return false;

	//get the first line
	char buff[1024]; // I HATE char buffers
	if(fgets(buff, sizeof(buff), locInputFile) == NULL)
		return 0;
	istringstream locStringStream(buff);

	//Close the pipe
	gSystem->ClosePipe(locInputFile);

	//extract it
	string locPolarizationDirection;
	if(!(locStringStream >> locPolarizationDirection))
		return false;

	if(locPolarizationDirection == "PARA")
	{
		locIsPARAFlag = true;
		return true;
	}
	else if(locPolarizationDirection == "PERP")
	{
		locIsPARAFlag = false;
		return true;
	}

	return false;
}
