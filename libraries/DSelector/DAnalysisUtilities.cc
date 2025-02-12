#include "DAnalysisUtilities.h"

TLorentzVector DAnalysisUtilities::Calc_MissingP4(const DParticleCombo* locParticleComboWrapper, bool locUseKinFitDataFlag) const
{
	map<unsigned int, set<Int_t> > locSourceObjects;
	return Calc_MissingP4(locParticleComboWrapper, 0, -1, set<size_t>(), locSourceObjects, locUseKinFitDataFlag);
}

TLorentzVector DAnalysisUtilities::Calc_MissingP4(const DParticleCombo* locParticleComboWrapper, map<unsigned int, set<Int_t> >& locSourceObjects, bool locUseKinFitDataFlag) const
{
	return Calc_MissingP4(locParticleComboWrapper, 0, -1, set<size_t>(), locSourceObjects, locUseKinFitDataFlag);
}

TLorentzVector DAnalysisUtilities::Calc_MissingP4(const DParticleCombo* locParticleComboWrapper, size_t locStepIndex, int locUpToStepIndex, set<size_t> locUpThroughIndices, bool locUseKinFitDataFlag) const
{
	map<unsigned int, set<Int_t> > locSourceObjects;
	return Calc_MissingP4(locParticleComboWrapper, locStepIndex, locUpToStepIndex, locUpThroughIndices, locSourceObjects, locUseKinFitDataFlag);
}

TLorentzVector DAnalysisUtilities::Calc_MissingP4(const DParticleCombo* locParticleComboWrapper, size_t locStepIndex, int locUpToStepIndex, set<size_t> locUpThroughIndices, map<unsigned int, set<Int_t> >& locSourceObjects, bool locUseKinFitDataFlag) const
{
	//NOTE: this routine assumes that the p4 of a charged decaying particle with a detached vertex is the same at both vertices!
	//assumes missing particle is not the beam particle
	if(locUseKinFitDataFlag && (locParticleComboWrapper->Get_NDF_KinFit( "" ) == 0))
		return Calc_MissingP4(locParticleComboWrapper, locStepIndex, locUpToStepIndex, locUpThroughIndices, locSourceObjects, false); //kinematic fit failed

	TLorentzVector locMissingP4;
	const DParticleComboStep* locParticleComboStepWrapper = locParticleComboWrapper->Get_ParticleComboStep(locStepIndex);

	if(locStepIndex == 0)
	{
		//initial particle
		DKinematicData* locKinematicData = locParticleComboStepWrapper->Get_InitialParticle();
		locSourceObjects[UnknownParticle].insert(locKinematicData->Get_ID()); //UnknownParticle for beam: don't mix with final-state photons //not ideal, but easy
		if(!locUseKinFitDataFlag) //measured
			locMissingP4 += locKinematicData->Get_P4_Measured();
		else
			locMissingP4 += locKinematicData->Get_P4();
	}

	//target particle
	Particle_t locTargetPID = locParticleComboStepWrapper->Get_TargetPID();
	if(locTargetPID != UnknownParticle)
	{
		double locMass = ParticleMass(locTargetPID);
		locMissingP4 += TLorentzVector(TVector3(0.0, 0.0, 0.0), locMass);
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

			locSourceObjects[abs(PDGtype(locPID))].insert(locParticles[loc_j]->Get_ID());
		}
		else //decaying-particle
			locMissingP4 += Calc_MissingP4(locParticleComboWrapper, locDecayStepIndex, locUpToStepIndex, locUpThroughIndices, locSourceObjects, locUseKinFitDataFlag); //p4 returned is already < 0
	}

	return locMissingP4;
}

TLorentzVector DAnalysisUtilities::Calc_FinalStateP4(const DParticleCombo* locParticleComboWrapper, size_t locStepIndex, bool locUseKinFitDataFlag) const
{
	map<unsigned int, set<Int_t> > locSourceObjects;
	return Calc_FinalStateP4(locParticleComboWrapper, locStepIndex, set<size_t>(), locSourceObjects, locUseKinFitDataFlag);
}

TLorentzVector DAnalysisUtilities::Calc_FinalStateP4(const DParticleCombo* locParticleComboWrapper, size_t locStepIndex, map<unsigned int, set<Int_t> >& locSourceObjects, bool locUseKinFitDataFlag) const
{
	return Calc_FinalStateP4(locParticleComboWrapper, locStepIndex, set<size_t>(), locSourceObjects, locUseKinFitDataFlag);
}

TLorentzVector DAnalysisUtilities::Calc_FinalStateP4(const DParticleCombo* locParticleComboWrapper, size_t locStepIndex, set<size_t> locToIncludeIndices, bool locUseKinFitDataFlag) const
{
	map<unsigned int, set<Int_t> > locSourceObjects;
	return Calc_FinalStateP4(locParticleComboWrapper, locStepIndex, locToIncludeIndices, locSourceObjects, locUseKinFitDataFlag);
}

TLorentzVector DAnalysisUtilities::Calc_FinalStateP4(const DParticleCombo* locParticleComboWrapper, size_t locStepIndex, set<size_t> locToIncludeIndices, map<unsigned int, set<Int_t> >& locSourceObjects, bool locUseKinFitDataFlag) const
{
	if(locUseKinFitDataFlag && (locParticleComboWrapper->Get_NDF_KinFit( "" ) == 0))
		return Calc_FinalStateP4(locParticleComboWrapper, locStepIndex, locToIncludeIndices, locSourceObjects, false); //kinematic fit failed

	TLorentzVector locFinalStateP4;
	const DParticleComboStep* locParticleComboStepWrapper = locParticleComboWrapper->Get_ParticleComboStep(locStepIndex);
	if(locParticleComboStepWrapper == NULL)
		return (TLorentzVector());

	//subtract rescattering target if any!!
	if(locStepIndex != 0)
	{
		Particle_t locPID = locParticleComboStepWrapper->Get_TargetPID();
		if(locPID != UnknownParticle)
			locFinalStateP4 -= TLorentzVector(TVector3(0.0, 0.0, 0.0), ParticleMass(locPID));
	}

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
			//below: measured results, or not constrained by kinfit (either non-fixed mass or excluded from kinfit)
			if((!locUseKinFitDataFlag) || (locParticles[loc_i] == nullptr))
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
			locSourceObjects[abs(PDGtype(locParticles[loc_i]->Get_PID()))].insert(locParticles[loc_i]->Get_ID());
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
		Particle_t locPID = locParticleComboStepWrapper->Get_FinalPID(loc_i);

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

double DAnalysisUtilities::Calc_ProdPlanePhi_Pseudoscalar(double locBeamEnergy, Particle_t locTargetPID, const TLorentzVector& locMesonP4) const
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

	//Boost beam & meson to production CM frame
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

double DAnalysisUtilities::Calc_DecayPlanePsi_Vector_2BodyDecay(double locBeamEnergy, Particle_t locTargetPID, const TLorentzVector& locBaryonP4, const TLorentzVector& locMesonP4, const TLorentzVector& locMesonProduct1P4, double& locDecayPlaneTheta) const
{
	//Returns angles in degrees!!

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

	//boost beam & baryon to production CM frame
		//Baryon P4: Is in general known better than meson p4 (many decay products)
	TLorentzVector locBeamP4_ProdCM(locBeamP4);
	locBeamP4_ProdCM.Boost(locBoostVector_ProdCM);
	TLorentzVector locBaryonP4_ProdCM(locBaryonP4);
	locBaryonP4_ProdCM.Boost(locBoostVector_ProdCM);

	//Production plane:
		//The production plane is the plane containing the produced particles.
		//However, when you boost to the production CM frame, the production plane is no longer well defined: the particles are back-to-back
		//So, by convention, define the production plane in the production CM frame by the beam and the vector meson
		//However, instead of the meson, use the negative momentum of the baryon (better resolution)

	//Production CM frame axes: "HELICITY SYSTEM"
		//The z-axis is defined as the direction of the meson: z = meson
		//The y-axis is defined by the vector cross product: y = Beam X meson
		//The x-axis is defined by the vector cross product: x = y cross z
		//Thus the production plane in the production frame is the XZ plane, and the normal vector is the Y-axis
		//Except, instead of meson, use (-1*baryon)

	//Define production CM frame helicity axes
	TVector3 locHelicityZAxis_ProdCM = -1.0*locBaryonP4_ProdCM.Vect().Unit();
	TVector3 locHelicityYAxis_ProdCM = -1.0*locBeamP4_ProdCM.Vect().Cross(locBaryonP4_ProdCM.Vect()).Unit();
	TVector3 locHelicityXAxis_ProdCM = locHelicityYAxis_ProdCM.Cross(locHelicityZAxis_ProdCM).Unit();

	//In the production CM frame, locProdPlanePhi is the angle between the production plane and the floor
		// We could have chosen any other fixed plane besides the floor. The point is that it must be a fixed reference
	TVector3 locFloorUnit(0.0, 1.0, 0.0);
	// Angle between the floor and production normal y, perpendicular to beam direction p
	        // y * floor, i.e. projection of y on floor
		// p * (floor x y) = floor * (y x p) = y * (p x floor) , i.e. projection of y on axis perpendicular to floor and beam
	        // Angle is defined by the arctan of two legs of the right triangle
	double locProdPlanePhi = atan2(locHelicityYAxis_ProdCM.Dot(locFloorUnit), locBeamP4_ProdCM.Vect().Unit().Dot(locFloorUnit.Cross(locHelicityYAxis_ProdCM)));

	//Now, we need the theta, phi angles between the meson decay plane and the production plane
	//The meson decay plane is defined by decay products in the meson CM frame
		// (FYI) GlueX Convention: angles defined by positive decay product

	//boost to meson CM frame
	TVector3 locBoostVector_MesonCM = -1.0*(locMesonP4.BoostVector()); //negative due to coordinate system convention
	TLorentzVector locBeamP4_MesonCM(locBeamP4);
	locBeamP4_MesonCM.Boost(locBoostVector_MesonCM);
	TLorentzVector locBaryonP4_MesonCM(locBaryonP4);
	locBaryonP4_MesonCM.Boost(locBoostVector_MesonCM);
	TLorentzVector locMesonProduct1P4_MesonCM(locMesonProduct1P4);
	locMesonProduct1P4_MesonCM.Boost(locBoostVector_MesonCM);

	//Define meson CM frame helicity axes
		//These are defined the same way as before, but with the boost, the direction of the x & y axes has changed
	TVector3 locHelicityZAxis_MesonCM = -1.0*locBaryonP4_MesonCM.Vect().Unit();
	TVector3 locHelicityYAxis_MesonCM = -1.0*locBeamP4_MesonCM.Vect().Cross(locBaryonP4_MesonCM.Vect()).Unit();
	TVector3 locHelicityXAxis_MesonCM = locHelicityYAxis_MesonCM.Cross(locHelicityZAxis_MesonCM).Unit();

	//Project the fs particle momentum onto these axes and read off the angles
	TVector3 locMesonProduct1P4_Angles(locMesonProduct1P4_MesonCM.Vect()*locHelicityXAxis_MesonCM,
					   locMesonProduct1P4_MesonCM.Vect()*locHelicityYAxis_MesonCM,
					   locMesonProduct1P4_MesonCM.Vect()*locHelicityZAxis_MesonCM);
	double locCosDecayPlaneTheta = locMesonProduct1P4_Angles.CosTheta();
	locDecayPlaneTheta = acos(locCosDecayPlaneTheta)*TMath::RadToDeg();
	double locDecayPlanePhi = locMesonProduct1P4_Angles.Phi();

	//Compute delta-phi
	double locDeltaPhi = locDecayPlanePhi - locProdPlanePhi;
	while(locDeltaPhi < -1.0*TMath::Pi())
		locDeltaPhi += 2.0*TMath::Pi();
	while(locDeltaPhi > TMath::Pi())
		locDeltaPhi -= 2.0*TMath::Pi();

	return locDeltaPhi*TMath::RadToDeg();
}

double DAnalysisUtilities::Calc_DecayPlanePsi_Vector_3BodyDecay(double locBeamEnergy, Particle_t locTargetPID, const TLorentzVector& locBaryonP4, const TLorentzVector& locMesonP4, const TLorentzVector& locMesonProduct1P4, const TLorentzVector& locMesonProduct2P4, double& locDecayPlaneTheta) const
{
	//Returns angles in degrees!!

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

	//boost beam & baryon to production CM frame
		//Baryon P4: Is in general known better than meson p4 (many decay products)
	TLorentzVector locBeamP4_ProdCM(locBeamP4);
	locBeamP4_ProdCM.Boost(locBoostVector_ProdCM);
	TLorentzVector locBaryonP4_ProdCM(locBaryonP4);
	locBaryonP4_ProdCM.Boost(locBoostVector_ProdCM);

	//Production plane:
		//The production plane is the plane containing the produced particles.
		//However, when you boost to the production CM frame, the production plane is no longer well defined: the particles are back-to-back
		//So, by convention, define the production plane in the production CM frame by the beam and the vector meson
		//However, instead of the meson, use the negative momentum of the baryon (better resolution)

	//Production CM frame axes: "HELICITY SYSTEM"
		//The z-axis is defined as the direction of the meson: z = meson
		//The y-axis is defined by the vector cross product: y = Beam X meson
		//The x-axis is defined by the vector cross product: x = y cross z
		//Thus the production plane in the production frame is the XZ plane, and the normal vector is the Y-axis
		//Except, instead of meson, use (-1*baryon)

	//Define production CM frame helicity axes
	TVector3 locHelicityZAxis_ProdCM = -1.0*locBaryonP4_ProdCM.Vect().Unit();
	TVector3 locHelicityYAxis_ProdCM = -1.0*locBeamP4_ProdCM.Vect().Cross(locBaryonP4_ProdCM.Vect()).Unit();
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

	//Now, we need the theta, phi angles between the meson decay plane and the production plane
	//The meson decay plane is defined by decay products in the meson CM frame
		//2 particles (vectors) define a plane.
		//However, to conserve momentum, the third particle cannot be out of that plane (so must also be in it)
		//So, use any 2 of the decay products to define the plane

	//boost to meson CM frame
	TVector3 locBoostVector_MesonCM = -1.0*(locMesonP4.BoostVector()); //negative due to coordinate system convention
	TLorentzVector locBeamP4_MesonCM(locBeamP4);
	locBeamP4_MesonCM.Boost(locBoostVector_MesonCM);
	TLorentzVector locBaryonP4_MesonCM(locBaryonP4);
	locBaryonP4_MesonCM.Boost(locBoostVector_MesonCM);
	TLorentzVector locMesonProduct1P4_MesonCM(locMesonProduct1P4);
	locMesonProduct1P4_MesonCM.Boost(locBoostVector_MesonCM);
	TLorentzVector locMesonProduct2P4_MesonCM(locMesonProduct2P4);
	locMesonProduct2P4_MesonCM.Boost(locBoostVector_MesonCM);

	//Define meson CM frame helicity axes
		//These are defined the same way as before, but with the boost, the direction of the x & y axes has changed
	TVector3 locHelicityZAxis_MesonCM = -1.0*locBaryonP4_MesonCM.Vect().Unit();
	TVector3 locHelicityYAxis_MesonCM = -1.0*locBeamP4_MesonCM.Vect().Cross(locBaryonP4_MesonCM.Vect()).Unit();
	TVector3 locHelicityXAxis_MesonCM = locHelicityYAxis_MesonCM.Cross(locHelicityZAxis_MesonCM).Unit();

	//Compute the normal vector to the 2 mesons to define the decay plane
	TVector3 locDecayPlaneNormal = (locMesonProduct1P4_MesonCM.Vect().Cross(locMesonProduct2P4_MesonCM.Vect()));

	//Compute the theta angle to the Meson decay plane
	double locCosDecayPlaneTheta = locDecayPlaneNormal.Dot(locHelicityZAxis_MesonCM)/locDecayPlaneNormal.Mag();
	locDecayPlaneTheta = acos(locCosDecayPlaneTheta)*180.0/TMath::Pi();

	//Compute the phi angle to the Meson decay plane
	TVector3 locZCrossMesonNormal = locHelicityZAxis_MesonCM.Cross(locDecayPlaneNormal);
	double locZCrossMesonNormalMag = locZCrossMesonNormal.Mag();
	double locCosDecayPlanePhi = locHelicityYAxis_MesonCM.Dot(locZCrossMesonNormal)/locZCrossMesonNormalMag;
	double locDecayPlanePhi = acos(locCosDecayPlanePhi); //reports phi between 0 and pi: sign ambiguity
	//Resolve the sign ambiguity
	double locSinDecayPlanePhi = -1.0*locHelicityXAxis_MesonCM.Dot(locZCrossMesonNormal)/locZCrossMesonNormalMag;
	if(locSinDecayPlanePhi < 0.0)
		locDecayPlanePhi *= -1.0;

	//Compute delta-phi
	double locDeltaPhi = locDecayPlanePhi - locProdPlanePhi;
	while(locDeltaPhi < -1.0*TMath::Pi())
		locDeltaPhi += 2.0*TMath::Pi();
	while(locDeltaPhi > TMath::Pi())
		locDeltaPhi -= 2.0*TMath::Pi();

	return 180.0*locDeltaPhi/TMath::Pi();
}

std::tuple<double,double> DAnalysisUtilities::Calc_vanHoveCoord(TLorentzVector locXP4, TLorentzVector locYP4, TLorentzVector locZP4)
{
	TLorentzVector locInitialStateP4 = locXP4 + locYP4 + locZP4;
	TVector3 locBoostVector_ProdCMS = -1.0*(locInitialStateP4.BoostVector()); //negative due to coordinate system convention
	TLorentzVector locXP4_ProdCMS(locXP4);
	locXP4_ProdCMS.Boost(locBoostVector_ProdCMS);
	TLorentzVector locYP4_ProdCMS(locYP4);
	locYP4_ProdCMS.Boost(locBoostVector_ProdCMS);
	TLorentzVector locZP4_ProdCMS(locZP4);
	locZP4_ProdCMS.Boost(locBoostVector_ProdCMS);
	
	
	double loclong1 = locXP4_ProdCMS.Pz();
	double loclong2 = locYP4_ProdCMS.Pz();
	double loclong3 = locZP4_ProdCMS.Pz();
	
	double locq = TMath::Sqrt(loclong1*loclong1+loclong2*loclong2+loclong3*loclong3);
	double locomega = TMath::ATan2(-1.*TMath::Sqrt(3.)*loclong1,2.*loclong2+loclong1)+TMath::Pi(); // add pi to map [-pi,pi] on [0,2pi]
	
	return std::make_tuple(locq, locomega);
}

std::tuple<double,double,double> DAnalysisUtilities::Calc_vanHoveCoordFour(TLorentzVector locVec1P4, TLorentzVector locVec2P4, TLorentzVector locVec3P4, TLorentzVector locVec4P4)
{
  //create CM vector from final state particles
  TLorentzVector locInitialStateP4 = locVec1P4 + locVec2P4 + locVec3P4 + locVec4P4;
  
  TVector3 locBoostVector_ProdCMS = (-1)*(locInitialStateP4.BoostVector());
  
  TLorentzVector lcoVec1P4_ProdCMS(locVec1P4);
  lcoVec1P4_ProdCMS.Boost(locBoostVector_ProdCMS);
  TLorentzVector lcoVec2P4_ProdCMS(locVec2P4);
  lcoVec2P4_ProdCMS.Boost(locBoostVector_ProdCMS);
  TLorentzVector lcoVec3P4_ProdCMS(locVec3P4);
  lcoVec3P4_ProdCMS.Boost(locBoostVector_ProdCMS);
  TLorentzVector lcoVec4P4_ProdCMS(locVec4P4);
  lcoVec4P4_ProdCMS.Boost(locBoostVector_ProdCMS);
  
  double loclong1 = lcoVec1P4_ProdCMS.Pz();
  double loclong2 = lcoVec2P4_ProdCMS.Pz();
  double loclong3 = lcoVec3P4_ProdCMS.Pz();
  
  //Calculate (x,y,z) and VH spherical polar angles
  double locx = (TMath::Sqrt(3./8.))*(loclong3-loclong2);
  double locy = (TMath::Sqrt(1./8.))*(2.*loclong1 + 3.*loclong2 + 3.*loclong3);
  double locz = loclong1;
  
  //r = sqrt(x^2 + y^2 + z^2)
  double locr = TMath::Sqrt(locx*locx+locy*locy+locz*locz);

  //theta = arctan(sqrt(x^2 + y^2)/z)
  //Better defined by the ATan2 expression
  double loctheta = TMath::ATan2((TMath::Sqrt(locx*locx + locy*locy)),locz);

  //phi = arctan(y/z)
  //as for the 3-particle implementation, this is better defined by the ATan2 function, pi added to return angles in range [0,2pi]
  double locphi = TMath::ATan2(locy,locx);
  if(locphi<0)
    {
      locphi = locphi + 2*(TMath::Pi());
    }
  
  return std::make_tuple(locr, loctheta, locphi);
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

bool DAnalysisUtilities::Get_PolarizationAngle(int locRunNumber, int& locPolarizationAngle) const
{
	//RCDB environment must be setup!!

	//Pipe the current constant into this function
	ostringstream locCommandStream;
	locCommandStream << "rcnd " << locRunNumber << " polarization_angle";
	FILE* locInputFile = gSystem->OpenPipe(locCommandStream.str().c_str(), "r");
	if(locInputFile == NULL)
		return false;

	//get the first line
	char buff[1024];
	if(fgets(buff, sizeof(buff), locInputFile) == NULL)
		return 0;
	istringstream locStringStream(buff);

	//Close the pipe
	gSystem->ClosePipe(locInputFile);

	//extract it
	string locPolarizationAngleString;
	if(!(locStringStream >> locPolarizationAngleString))
		return false;

	// convert string to integer
	locPolarizationAngle = atoi(locPolarizationAngleString.c_str());
	// amorphous runs have the value -1
	if (locPolarizationAngle == -1)
	  return false;
	
	return true;
}
bool DAnalysisUtilities::Get_CoherentPeak(int locRunNumber, double& locCoherentPeak, bool locIsPolarizedFlag) const
{
	//RCDB environment must be setup!!

	// amorphous runs can have any value
	if (!locIsPolarizedFlag)
	{
	  locCoherentPeak = 0.0;
	  return false;
	}

	//Pipe the current constant into this function
	ostringstream locCommandStream;
	locCommandStream << "rcnd " << locRunNumber << " coherent_peak";
	FILE* locInputFile = gSystem->OpenPipe(locCommandStream.str().c_str(), "r");
	if(locInputFile == NULL)
		return false;

	//get the first line
	char buff[1024];
	if(fgets(buff, sizeof(buff), locInputFile) == NULL)
		return 0;
	istringstream locStringStream(buff);

	//Close the pipe
	gSystem->ClosePipe(locInputFile);

	//extract it
	string locCoherentPeakString;
	if(!(locStringStream >> locCoherentPeakString))
		return false;

	// convert string to double
	locCoherentPeak = atof(locCoherentPeakString.c_str());
	
	return true;
}

double DAnalysisUtilities::Get_BeamBunchPeriod(int locRunNumber) 
{
	// CCDB environment must be setup!!
	
	//If we already cached a value for this run, just return that and we're done
	if(dBeamBunchPeriod_Cache.count(locRunNumber) > 0) return dBeamBunchPeriod_Cache[locRunNumber];
	
	// Retrieving from ccdb is SLOW so we should only execute this once upon encountering a new run number
	//Pipe the current constant into this function
	ostringstream locCommandStream;
	locCommandStream << "ccdb dump PHOTON_BEAM/RF/beam_period -r " << locRunNumber;
	FILE* locInputFile = gSystem->OpenPipe(locCommandStream.str().c_str(), "r");
	if(locInputFile == NULL) {
		cerr << "Could not load PHOTON_BEAM/RF/beam_period from CCDB !" << endl;
		exit(1);        // make sure we don't fail silently
		return -1.0;    // sanity check, this shouldn't be executed!
	}

	//get the first line
	char buff[1024]; // I HATE char buffers
	if(fgets(buff, sizeof(buff), locInputFile) == NULL)
	{
		gSystem->ClosePipe(locInputFile);
		cerr << "Could not parse PHOTON_BEAM/RF/beam_period from CCDB !" << endl;
		exit(1);        // make sure we don't fail silently
		return -1.0;    // sanity check, this shouldn't be executed!
	}

	//get the second line (where the # is)
	if(fgets(buff, sizeof(buff), locInputFile) == NULL)
	{
		gSystem->ClosePipe(locInputFile);
		cerr << "Could not parse PHOTON_BEAM/RF/beam_period from CCDB !" << endl;
		exit(1);        // make sure we don't fail silently		
		return -1.0;    // sanity check, this shouldn't be executed!
	}
	istringstream locStringStream(buff);

	//extract it
	double locBeamBunchPeriod = -1.0;
	if(!(locStringStream >> locBeamBunchPeriod))
		locBeamBunchPeriod = -1.0;

	//Close the pipe
	gSystem->ClosePipe(locInputFile);

	//Save to cache for future calls
	dBeamBunchPeriod_Cache[locRunNumber] = locBeamBunchPeriod;

	return locBeamBunchPeriod;
}

double DAnalysisUtilities::Get_DeltaT_RF(int locRunNumber, const TLorentzVector locBeamX4_Measured, const DParticleCombo* locParticleComboWrapper)
{
	return locBeamX4_Measured.T() - (locParticleComboWrapper->Get_RFTime_Measured() + (locBeamX4_Measured.Z()- locParticleComboWrapper->Get_TargetCenter().Z())/29.9792458 );;
}

int DAnalysisUtilities::Get_RelativeBeamBucket(int locRunNumber, const TLorentzVector locBeamX4_Measured, const DParticleCombo* locParticleComboWrapper)
{
	// returns an integer:
	/// 0 if event is belongs to in-time beam bunch bucket
	/// -1 if event is belongs to first beam bunch left of in-time bucket
	/// +1 if event is belongs to first beam bunch right of in-time bucket
	/// and so on
	
	double locBeamBunchPeriod = Get_BeamBunchPeriod(locRunNumber);
	double locDeltaT_RF = Get_DeltaT_RF(locRunNumber,locBeamX4_Measured,locParticleComboWrapper);
	
	return int(floor( (locDeltaT_RF/locBeamBunchPeriod) + 0.5 ));
}

double DAnalysisUtilities::Get_AccidentalScalingFactor(int locRunNumber, double locBeamEnergy, bool locIsMC)
{
	//CCDB environment must be setup!!

	double locHodoscopeHiFactor = -1.0;
	double locHodoscopeHiFactorErr = -1.0;
	double locHodoscopeLoFactor = -1.0;
	double locHodoscopeLoFactorErr = -1.0;
	double locMicroscopeFactor = -1.0;
	double locMicroscopeFactorErr = -1.0;
	double locTAGMEnergyBoundHi = -1.0;
	double locTAGMEnergyBoundLo = -1.0;

	// check to see if we already loaded the data for this run
	// CCDB access is SLOW, so cache when you can
	if(dAccidentalScalingFactor_Cache.count(locRunNumber) > 0) {
		// 	set the values from the cache
		vector<double> &locCachedValues = dAccidentalScalingFactor_Cache[locRunNumber];
		locHodoscopeHiFactor = locCachedValues[0];
		locHodoscopeHiFactorErr = locCachedValues[1];
		locHodoscopeLoFactor = locCachedValues[2];
		locHodoscopeLoFactorErr = locCachedValues[3];
		locMicroscopeFactor = locCachedValues[4];
		locMicroscopeFactorErr = locCachedValues[5];
		locTAGMEnergyBoundHi = locCachedValues[6];
		locTAGMEnergyBoundLo = locCachedValues[7];
	} else {

		// Guess we have to go to the CCDB...
		//Pipe the current constant into this function
		ostringstream locCommandStream;
		if (locIsMC)
		  locCommandStream << "ccdb dump ANALYSIS/accidental_scaling_factor -v mc -r " << locRunNumber;
		else
		  locCommandStream << "ccdb dump ANALYSIS/accidental_scaling_factor -r " << locRunNumber;
		FILE* locInputFile = gSystem->OpenPipe(locCommandStream.str().c_str(), "r");
		if(locInputFile == NULL) {
		        cerr << "Could not load ANALYSIS/accidental_scaling_factor from CCDB !" << endl;
			gSystem->Exit(1);        // make sure we don't fail silently
			return -1.0;    // sanity check, this shouldn't be executed!
		}

		//get the first line
		char buff[1024]; // I HATE char buffers
		if(fgets(buff, sizeof(buff), locInputFile) == NULL)
		{
            //vector<double> locCachedValues = { -1., -1., -1., -1., -1., -1., -1., -1. };
            //dAccidentalScalingFactor_Cache[locRunNumber] = locCachedValues;   // give up for this run
			gSystem->ClosePipe(locInputFile);
			cerr << "Could not parse ANALYSIS/accidental_scaling_factor from CCDB !" << endl;
			gSystem->Exit(1);        // make sure we don't fail silently
			return -1.0;    // sanity check, this shouldn't be executed!
		}

		//get the second line (where the # is)
		if(fgets(buff, sizeof(buff), locInputFile) == NULL)
		{
            //vector<double> locCachedValues = { -1., -1., -1., -1., -1., -1., -1., -1. };
            //dAccidentalScalingFactor_Cache[locRunNumber] = locCachedValues;   // give up for this run
			gSystem->ClosePipe(locInputFile);
			cerr << "Could not parse ANALYSIS/accidental_scaling_factor from CCDB !" << endl;
			gSystem->Exit(1);        // make sure we don't fail silently
			return -1.0;    // sanity check, this shouldn't be executed!
		}
        
        // catch some CCDB error conditions
        if(strncmp(buff, "Cannot", 6) == 0) 
        {
            // no assignment for this run
            //vector<double> locCachedValues = { -1., -1., -1., -1., -1., -1., -1., -1. };
            //dAccidentalScalingFactor_Cache[locRunNumber] = locCachedValues;   // give up for this run
			gSystem->ClosePipe(locInputFile);
			cerr << "No data available for ANALYSIS/accidental_scaling_factor, run " << locRunNumber << " from CCDB !" << endl;
			gSystem->Exit(1);        // make sure we don't fail silently
			return -1.0;    // sanity check, this shouldn't be executed!
        }

		istringstream locStringStream(buff);

		//extract it
		locStringStream >> locHodoscopeHiFactor >> locHodoscopeHiFactorErr >> locHodoscopeLoFactor
						>> locHodoscopeLoFactorErr >> locMicroscopeFactor >> locMicroscopeFactorErr
						>> locTAGMEnergyBoundHi >> locTAGMEnergyBoundLo;

		//Close the pipe
		gSystem->ClosePipe(locInputFile);
		
		//save the values to a local cache
		vector<double> locCachedValues;
		locCachedValues.push_back(locHodoscopeHiFactor);
		locCachedValues.push_back(locHodoscopeHiFactorErr);
		locCachedValues.push_back(locHodoscopeLoFactor);
		locCachedValues.push_back(locHodoscopeLoFactorErr);
		locCachedValues.push_back(locMicroscopeFactor);
		locCachedValues.push_back(locMicroscopeFactorErr);
		locCachedValues.push_back(locTAGMEnergyBoundHi);
		locCachedValues.push_back(locTAGMEnergyBoundLo);
		
		dAccidentalScalingFactor_Cache[locRunNumber] = locCachedValues;
	}

	if(locBeamEnergy > locTAGMEnergyBoundHi)
		return locHodoscopeHiFactor;
	else if(locBeamEnergy > locTAGMEnergyBoundLo)
		return locMicroscopeFactor;
	else
		return locHodoscopeLoFactor;
}

double DAnalysisUtilities::Get_AccidentalScalingFactorError(int locRunNumber, double locBeamEnergy, bool locIsMC) 
{
	//CCDB environment must be setup!!

	double locHodoscopeHiFactor = -1.0;
	double locHodoscopeHiFactorErr = -1.0;
	double locHodoscopeLoFactor = -1.0;
	double locHodoscopeLoFactorErr = -1.0;
	double locMicroscopeFactor = -1.0;
	double locMicroscopeFactorErr = -1.0;
	double locTAGMEnergyBoundHi = -1.0;
	double locTAGMEnergyBoundLo = -1.0;

	// check to see if we already loaded the data for this run
	// CCDB access is SLOW, so cache when you can
	if(dAccidentalScalingFactor_Cache.count(locRunNumber) > 0) {
		// 	set the values from the cache
		vector<double> &locCachedValues = dAccidentalScalingFactor_Cache[locRunNumber];
		locHodoscopeHiFactor = locCachedValues[0];
		locHodoscopeHiFactorErr = locCachedValues[1];
		locHodoscopeLoFactor = locCachedValues[2];
		locHodoscopeLoFactorErr = locCachedValues[3];
		locMicroscopeFactor = locCachedValues[4];
		locMicroscopeFactorErr = locCachedValues[5];
		locTAGMEnergyBoundHi = locCachedValues[6];
		locTAGMEnergyBoundLo = locCachedValues[7];
	} else {

		// Guess we have to go to the CCDB...
		//Pipe the current constant into this function
		ostringstream locCommandStream;
		if (locIsMC)
		  locCommandStream << "ccdb dump ANALYSIS/accidental_scaling_factor -v mc -r " << locRunNumber;
		else
		  locCommandStream << "ccdb dump ANALYSIS/accidental_scaling_factor -r " << locRunNumber;
		FILE* locInputFile = gSystem->OpenPipe(locCommandStream.str().c_str(), "r");
		if(locInputFile == NULL) {
			cerr << "Could not load ANALYSIS/accidental_scaling_factor from CCDB !" << endl;
			gSystem->Exit(1);        // make sure we don't fail silently
			return -1.0;    // sanity check, this shouldn't be executed!
		}

		//get the first line
		char buff[1024]; // I HATE char buffers
		if(fgets(buff, sizeof(buff), locInputFile) == NULL)
		{
            vector<double> locCachedValues = { -1., -1., -1., -1., -1., -1., -1., -1. };
            dAccidentalScalingFactor_Cache[locRunNumber] = locCachedValues;   // give up for this run
			gSystem->ClosePipe(locInputFile);
			cerr << "Could not load ANALYSIS/accidental_scaling_factor from CCDB !" << endl;
			gSystem->Exit(1);        // make sure we don't fail silently
			return -1.0;    // sanity check, this shouldn't be executed!
		}

		//get the second line (where the # is)
		if(fgets(buff, sizeof(buff), locInputFile) == NULL)
		{
            vector<double> locCachedValues = { -1., -1., -1., -1., -1., -1., -1., -1. };
            dAccidentalScalingFactor_Cache[locRunNumber] = locCachedValues;   // give up for this run
			gSystem->ClosePipe(locInputFile);
			cerr << "Could not load ANALYSIS/accidental_scaling_factor from CCDB !" << endl;
			gSystem->Exit(1);        // make sure we don't fail silently
			return -1.0;    // sanity check, this shouldn't be executed!
		}

        // catch some CCDB error conditions
        if(strncmp(buff, "Cannot", 6) == 0) 
        {
            // no assignment for this run
            //vector<double> locCachedValues = { -1., -1., -1., -1., -1., -1., -1., -1. };
            //dAccidentalScalingFactor_Cache[locRunNumber] = locCachedValues;   // give up for this run
			gSystem->ClosePipe(locInputFile);
			cerr << "No data available for ANALYSIS/accidental_scaling_factor, run " << locRunNumber << " from CCDB !" << endl;
			gSystem->Exit(1);        // make sure we don't fail silently
			return -1.0;    // sanity check, this shouldn't be executed!
        }

		istringstream locStringStream(buff);

		//extract it
		locStringStream >> locHodoscopeHiFactor >> locHodoscopeHiFactorErr >> locHodoscopeLoFactor
						>> locHodoscopeLoFactorErr >> locMicroscopeFactor >> locMicroscopeFactorErr
						>> locTAGMEnergyBoundHi >> locTAGMEnergyBoundLo;

		//Close the pipe
		gSystem->ClosePipe(locInputFile);
		
		//save the values to a local cache
		vector<double> locCachedValues;
		locCachedValues.push_back(locHodoscopeHiFactor);
		locCachedValues.push_back(locHodoscopeHiFactorErr);
		locCachedValues.push_back(locHodoscopeLoFactor);
		locCachedValues.push_back(locHodoscopeLoFactorErr);
		locCachedValues.push_back(locMicroscopeFactor);
		locCachedValues.push_back(locMicroscopeFactorErr);
		locCachedValues.push_back(locTAGMEnergyBoundHi);
		locCachedValues.push_back(locTAGMEnergyBoundLo);
		
		dAccidentalScalingFactor_Cache[locRunNumber] = locCachedValues;
	}
	
	if(locBeamEnergy > locTAGMEnergyBoundHi)
		return locHodoscopeHiFactorErr;
	else if(locBeamEnergy > locTAGMEnergyBoundLo)
		return locMicroscopeFactorErr;
	else
		return locHodoscopeLoFactorErr;
}

double DAnalysisUtilities::Get_BeamEndpoint(int locRunNumber) 
{
	// CCDB environment must be setup!!
	
	//If we already cached a value for this run, just return that and we're done
	if(dBeamEndpoint_Cache.count(locRunNumber) > 0) return dBeamEndpoint_Cache[locRunNumber];
	
	// Retrieving from ccdb is SLOW so we should only execute this once upon encountering a new run number
	//Pipe the current constant into this function
	ostringstream locCommandStream;
	locCommandStream << "ccdb dump PHOTON_BEAM/endpoint_energy -r " << locRunNumber;
	FILE* locInputFile = gSystem->OpenPipe(locCommandStream.str().c_str(), "r");
	if(locInputFile == NULL)
		return -1.0;

	//get the first line
	char buff[1024]; // I HATE char buffers
	if(fgets(buff, sizeof(buff), locInputFile) == NULL)
	{
		gSystem->ClosePipe(locInputFile);
		return -1.0;
	}

	//get the second line (where the # is)
	if(fgets(buff, sizeof(buff), locInputFile) == NULL)
	{
		gSystem->ClosePipe(locInputFile);
		return -1.0;
	}
	istringstream locStringStream(buff);

	//extract it
	double locBeamEndpoint = -1.0;
	if(!(locStringStream >> locBeamEndpoint))
		locBeamEndpoint = -1.0;

	//Close the pipe
	gSystem->ClosePipe(locInputFile);

	//Save to cache for future calls
	dBeamEndpoint_Cache[locRunNumber] = locBeamEndpoint;

	return locBeamEndpoint;
}

vector< pair<double,double> > DAnalysisUtilities::Get_EnergyTAGH(int locRunNumber) {

	// CCDB environment must be setup!!

	//If we already cached a value for this run, just return that and we're done
	if(dEnergyTAGH_Cache.count(locRunNumber) > 0) return dEnergyTAGH_Cache[locRunNumber];

	vector< pair<double,double> > locEnergyTAGH;
	
	// Retrieving from ccdb is SLOW so we should only execute this once upon encountering a new run number
	//Pipe the current constant into this function
	ostringstream locCommandStream;
	locCommandStream << "ccdb dump PHOTON_BEAM/hodoscope/scaled_energy_range -r " << locRunNumber;
	FILE* locInputFile = gSystem->OpenPipe(locCommandStream.str().c_str(), "r");
	if(locInputFile == NULL)
		return locEnergyTAGH;

	char buff[1024];
	for(int i=0; i<274; i++) {

		//get the first line (where the # is)
		if(fgets(buff, sizeof(buff), locInputFile) == NULL)
			{
				gSystem->ClosePipe(locInputFile);
				return locEnergyTAGH;
			}
		//skip over all lines starting with #
		while (buff[0] == '#')
		  if(fgets(buff, sizeof(buff), locInputFile) == NULL)
			{
				gSystem->ClosePipe(locInputFile);
				return locEnergyTAGH;
			}
		istringstream locStringStream(buff);

		//extract it
		int locCounter = -1;
		double locCounterLow = -1.0;
		double locCounterHigh = -1.0;
		locStringStream >> locCounter >> locCounterLow  >> locCounterHigh;

		//double locCounterEnergy = (locCounterLow + locCounterHigh) / 2.;
		pair<double, double> locEnergyCounter(locCounterLow, locCounterHigh);
		locEnergyTAGH.push_back(locEnergyCounter);
	}

	//Close the pipe
	gSystem->ClosePipe(locInputFile);

	//Save to cache for future calls
	dEnergyTAGH_Cache[locRunNumber] = locEnergyTAGH;

	return locEnergyTAGH;
}

int DAnalysisUtilities::Get_CounterTAGH(int locRunNumber, double locBeamEnergy) {
	
	vector< pair<double,double> > locEnergyTAGH;
	if(dEnergyTAGH_Cache.count(locRunNumber) > 0) locEnergyTAGH = dEnergyTAGH_Cache[locRunNumber];
	else locEnergyTAGH = Get_EnergyTAGH(locRunNumber);

	double locEndpoint = 0;
	if(dBeamEndpoint_Cache.count(locRunNumber) > 0) locEndpoint = dBeamEndpoint_Cache[locRunNumber];
	else locEndpoint = Get_BeamEndpoint(locRunNumber);

	// Get the TAGH counter from the low and high energy bounds
	for(uint i=0; i<locEnergyTAGH.size(); i++) {
		if(locBeamEnergy > locEndpoint*locEnergyTAGH[i].first && locBeamEnergy < locEndpoint*locEnergyTAGH[i].second)
			return i+1;
	}

	return -1;
}

vector< pair<double,double> > DAnalysisUtilities::Get_EnergyTAGM(int locRunNumber) {

	// CCDB environment must be setup!!

	//If we already cached a value for this run, just return that and we're done
	if(dEnergyTAGM_Cache.count(locRunNumber) > 0) return dEnergyTAGM_Cache[locRunNumber];

	vector< pair<double,double> > locEnergyTAGM;
	
	// Retrieving from ccdb is SLOW so we should only execute this once upon encountering a new run number
	//Pipe the current constant into this function
	ostringstream locCommandStream;
	locCommandStream << "ccdb dump PHOTON_BEAM/microscope/scaled_energy_range -r " << locRunNumber;
	FILE* locInputFile = gSystem->OpenPipe(locCommandStream.str().c_str(), "r");
	if(locInputFile == NULL)
		return locEnergyTAGM;

	char buff[1024];
	for(int i=0; i<102; i++) {

		//get the first line (where the # is)
		if(fgets(buff, sizeof(buff), locInputFile) == NULL)
			{
				gSystem->ClosePipe(locInputFile);
				return locEnergyTAGM;
			}
		//skip over all lines starting with #
		while (buff[0] == '#')
		  if(fgets(buff, sizeof(buff), locInputFile) == NULL)
			{
				gSystem->ClosePipe(locInputFile);
				return locEnergyTAGM;
			}
		istringstream locStringStream(buff);

		//extract it
		int locCounter = -1;
		double locCounterLow = -1.0;
		double locCounterHigh = -1.0;
		locStringStream >> locCounter >> locCounterLow  >> locCounterHigh;

		pair<double, double> locEnergyCounter(locCounterLow, locCounterHigh);
		locEnergyTAGM.push_back(locEnergyCounter);
	}

	//Close the pipe
	gSystem->ClosePipe(locInputFile);

	//Save to cache for future calls
	dEnergyTAGM_Cache[locRunNumber] = locEnergyTAGM;

	return locEnergyTAGM;
}

int DAnalysisUtilities::Get_ColumnTAGM(int locRunNumber, double locBeamEnergy) {

	vector< pair<double,double> > locEnergyTAGM;
	if(dEnergyTAGM_Cache.count(locRunNumber) > 0) locEnergyTAGM = dEnergyTAGM_Cache[locRunNumber];
	else locEnergyTAGM = Get_EnergyTAGM(locRunNumber);

	double locEndpoint = 0;
	if(dBeamEndpoint_Cache.count(locRunNumber) > 0) locEndpoint = dBeamEndpoint_Cache[locRunNumber];
	else locEndpoint = Get_BeamEndpoint(locRunNumber);

	// Get the TAGM counter from the low and high energy bounds
	for(uint i=0; i<locEnergyTAGM.size(); i++) {
		if(locBeamEnergy > locEndpoint*locEnergyTAGM[i].first && locBeamEnergy < locEndpoint*locEnergyTAGM[i].second)
			return i+1;
	}

	return -1;
}

double* DAnalysisUtilities::Generate_LogBinning(int locLowest10Power, int locHighest10Power, unsigned int locNumBinsPerPower, int& locNumBins) const
{
	if(locHighest10Power <= locLowest10Power)
	{
		locNumBins = -1;
		return NULL;
	}

	int locNumPowerRanges = locHighest10Power - locLowest10Power; //Num powers of 10, minus 1
	locNumBins = locNumBinsPerPower*locNumPowerRanges;

	double* locBinArray = new double[locNumBins + 1];
	locBinArray[0] = pow(10.0, locLowest10Power);
	for(int loc_j = 0; loc_j < locNumPowerRanges; ++loc_j)
	{
		double locCurrent10Power = double(locLowest10Power + loc_j);
		for(unsigned int loc_k = 0; loc_k < locNumBinsPerPower; ++loc_k)
		{
		        double locMultiplier = (double(loc_k) + 1.0) / locNumBinsPerPower;
			locBinArray[loc_j*locNumBinsPerPower + loc_k + 1] = pow(10,locMultiplier) * pow(10.0, locCurrent10Power);
		}
	}

	return locBinArray;
}

