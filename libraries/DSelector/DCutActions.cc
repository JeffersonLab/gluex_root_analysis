#include "DCutActions.h"

string DCutAction_PIDDeltaT::Get_ActionName(void) const
{
	ostringstream locStream;
	locStream << DAnalysisAction::Get_ActionName() << "_" << dPID << "_" << dSystem << "_" << dDeltaTCut;
	return locStream.str();
}

void DCutAction_PIDDeltaT::Initialize(void)
{
	dTargetCenterZ = dParticleComboWrapper->Get_TargetCenter().Z();	
}

bool DCutAction_PIDDeltaT::Perform_Action(void)
{
	for(size_t loc_i = 0; loc_i < dParticleComboWrapper->Get_NumParticleComboSteps(); ++loc_i)
	{
		DParticleComboStep* locComboWrapperStep = dParticleComboWrapper->Get_ParticleComboStep(loc_i);

		//final particles
		for(size_t loc_j = 0; loc_j < locComboWrapperStep->Get_NumFinalParticles(); ++loc_j)
		{
			DKinematicData* locKinematicData = locComboWrapperStep->Get_FinalParticle(loc_j);
			if(locKinematicData == NULL)
				continue; //e.g. a decaying or missing particle whose params aren't set yet

			//-2 if detected, -1 if missing, > 0 if decaying (step where it is the parent)
			int locDecayStepIndex = locComboWrapperStep->Get_DecayStepIndex(loc_j);
			if(locDecayStepIndex != -2)
				continue; //not measured

			if((dPID != Unknown) && (locKinematicData->Get_PID() != dPID))
				continue;

			// determine detector system
			DetectorSystem_t locSystem = SYS_NULL;
			if(ParticleCharge(locKinematicData->Get_PID()) != 0)
			{
				const DChargedTrackHypothesis* locChargedTrackHypothesis = dynamic_cast<const DChargedTrackHypothesis*>(locKinematicData);
				if(locChargedTrackHypothesis != NULL)
					locSystem = locChargedTrackHypothesis->Get_Detector_System_Timing();
			}
			else
			{
				const DNeutralParticleHypothesis* locNeutralParticleHypothesis = dynamic_cast<const DNeutralParticleHypothesis*>(locKinematicData);
				if(locNeutralParticleHypothesis != NULL)
					locSystem = locNeutralParticleHypothesis->Get_Detector_System_Timing();
			}
			
			if((dSystem != SYS_NULL) && (locSystem != dSystem))
				continue;

			TLorentzVector locX4 = dUseKinFitFlag ? locKinematicData->Get_X4() : locKinematicData->Get_X4_Measured();
			double locRFTime = dParticleComboWrapper->Get_RFTime_Measured();
			double locPropagatedRFTime = locRFTime + (locX4.Z() - dTargetCenterZ)/29.9792458;
			double locDeltaT = locX4.T() - locPropagatedRFTime;
			if(fabs(locDeltaT) > dDeltaTCut) 
				return false;

		} //end of particle loop
	} //end of step loop

	return true;
}

string DCutAction_NoPIDHit::Get_ActionName(void) const
{
	ostringstream locStream;
	locStream << DAnalysisAction::Get_ActionName() << "_" << dPID;
	return locStream.str();
}

bool DCutAction_NoPIDHit::Perform_Action(void)
{
	for(size_t loc_i = 0; loc_i < dParticleComboWrapper->Get_NumParticleComboSteps(); ++loc_i)
	{
		DParticleComboStep* locComboWrapperStep = dParticleComboWrapper->Get_ParticleComboStep(loc_i);

		//final particles
		for(size_t loc_j = 0; loc_j < locComboWrapperStep->Get_NumFinalParticles(); ++loc_j)
		{
			DKinematicData* locKinematicData = locComboWrapperStep->Get_FinalParticle(loc_j);
			if(locKinematicData == NULL)
				continue; //e.g. a decaying or missing particle whose params aren't set yet

			//-2 if detected, -1 if missing, > 0 if decaying (step where it is the parent)
			int locDecayStepIndex = locComboWrapperStep->Get_DecayStepIndex(loc_j);
			if(locDecayStepIndex != -2)
				continue; //not measured

			if((dPID != Unknown) && (locKinematicData->Get_PID() != dPID))
				continue;

			// determine detector system
			DetectorSystem_t locSystem = SYS_NULL;
			if(ParticleCharge(locKinematicData->Get_PID()) != 0)
			{
				const DChargedTrackHypothesis* locChargedTrackHypothesis = dynamic_cast<const DChargedTrackHypothesis*>(locKinematicData);
				if(locChargedTrackHypothesis != NULL)
					locSystem = locChargedTrackHypothesis->Get_Detector_System_Timing();
			}
			else
			{
				const DNeutralParticleHypothesis* locNeutralParticleHypothesis = dynamic_cast<const DNeutralParticleHypothesis*>(locKinematicData);
				if(locNeutralParticleHypothesis != NULL)
					locSystem = locNeutralParticleHypothesis->Get_Detector_System_Timing();
			}
			
			if((dSystem != SYS_NULL) && (locSystem != dSystem))
				return false;

		} //end of particle loop
	} //end of step loop

	return true;
}

void DCutAction_dEdxProton::Initialize(void)
{
	f = new TF1("f","exp([0]*x + [1]) + [2]",0.0,10.0);
	f->SetParameters(-4.0,2.25,1.0);
}

bool DCutAction_dEdxProton::Perform_Action(void)
{
	for(size_t loc_i = 0; loc_i < dParticleComboWrapper->Get_NumParticleComboSteps(); ++loc_i)
	{
		DParticleComboStep* locComboWrapperStep = dParticleComboWrapper->Get_ParticleComboStep(loc_i);

		//final particles
		for(size_t loc_j = 0; loc_j < locComboWrapperStep->Get_NumFinalParticles(); ++loc_j)
		{
			DKinematicData* locKinematicData = locComboWrapperStep->Get_FinalParticle(loc_j);
			if(locKinematicData == NULL)
				continue; //e.g. a decaying or missing particle whose params aren't set yet

			//-2 if detected, -1 if missing, > 0 if decaying (step where it is the parent)
			int locDecayStepIndex = locComboWrapperStep->Get_DecayStepIndex(loc_j);
			if(locDecayStepIndex != -2)
				continue; //not measured

			if((dPID != Unknown) && (locKinematicData->Get_PID() != dPID))
				continue;

			const DChargedTrackHypothesis* locChargedTrackHypothesis = dynamic_cast<const DChargedTrackHypothesis*>(locKinematicData);
			Float_t locdEdx;
			if(dSystem != SYS_CDC)
				locdEdx = locChargedTrackHypothesis->Get_dEdx_CDC()*1.0E6;
			else
				continue;

			double locp = dUseKinFitFlag ? locChargedTrackHypothesis->Get_P4().Vect().Mag() : locChargedTrackHypothesis->Get_P4_Measured().Vect().Mag();
			if(locdEdx < f->Eval(locp)) 
				return false;

		} //end of particle loop
	} //end of step loop

	return true;
}

string DCutAction_MissingMass::Get_ActionName(void) const
{
	ostringstream locStream;
	locStream << DAnalysisAction::Get_ActionName() << "_" << dMinimumMissingMass << "_" << dMaximumMissingMass;
	return locStream.str();
}

bool DCutAction_MissingMass::Perform_Action(void)
{
	//build all possible combinations of the included pids
	const DParticleComboStep* locComboWrapperStep = dParticleComboWrapper->Get_ParticleComboStep(dMissingMassOffOfStepIndex);
	set<set<size_t> > locIndexCombos = dAnalysisUtilities.Build_IndexCombos(locComboWrapperStep, dMissingMassOffOfPIDs);

	//loop over them: Must fail ALL to fail. if any succeed, return true
	set<set<size_t> >::iterator locComboIterator = locIndexCombos.begin();
	for(; locComboIterator != locIndexCombos.end(); ++locComboIterator)
	{
		TLorentzVector locMissingP4 = dAnalysisUtilities.Calc_MissingP4(dParticleComboWrapper, 0, dMissingMassOffOfStepIndex, *locComboIterator, dUseKinFitFlag);
		double locMissingMass = locMissingP4.M();
		if((locMissingMass >= dMinimumMissingMass) && (locMissingMass <= dMaximumMissingMass))
			return true;
	}

	return false; //all failed
}

string DCutAction_MissingMassSquared::Get_ActionName(void) const
{
	ostringstream locStream;
	locStream << DAnalysisAction::Get_ActionName() << "_" << dMinimumMissingMassSq << "_" << dMaximumMissingMassSq;
	return locStream.str();
}

bool DCutAction_MissingMassSquared::Perform_Action(void)
{
	//build all possible combinations of the included pids
	const DParticleComboStep* locComboWrapperStep = dParticleComboWrapper->Get_ParticleComboStep(dMissingMassOffOfStepIndex);
	set<set<size_t> > locIndexCombos = dAnalysisUtilities.Build_IndexCombos(locComboWrapperStep, dMissingMassOffOfPIDs);

	//loop over them: Must fail ALL to fail. if any succeed, return true
	set<set<size_t> >::iterator locComboIterator = locIndexCombos.begin();
	for(; locComboIterator != locIndexCombos.end(); ++locComboIterator)
	{
		TLorentzVector locMissingP4 = dAnalysisUtilities.Calc_MissingP4(dParticleComboWrapper, 0, dMissingMassOffOfStepIndex, *locComboIterator, dUseKinFitFlag);
		double locMissingMassSq = locMissingP4.M2();
		if((locMissingMassSq >= dMinimumMissingMassSq) && (locMissingMassSq <= dMaximumMissingMassSq))
			return true;
	}

	return false; //all failed
}

string DCutAction_MissingEnergy::Get_ActionName(void) const
{
	ostringstream locStream;
	locStream << DAnalysisAction::Get_ActionName() << "_" << dMinimumMissingEnergy << "_" << dMaximumMissingEnergy;
	return locStream.str();
}

bool DCutAction_MissingEnergy::Perform_Action(void)
{
	//build all possible combinations of the included pids
	const DParticleComboStep* locComboWrapperStep = dParticleComboWrapper->Get_ParticleComboStep(dMissingEnergyOffOfStepIndex);
	set<set<size_t> > locIndexCombos = dAnalysisUtilities.Build_IndexCombos(locComboWrapperStep, dMissingEnergyOffOfPIDs);

	//loop over them: Must fail ALL to fail. if any succeed, return true
	set<set<size_t> >::iterator locComboIterator = locIndexCombos.begin();
	for(; locComboIterator != locIndexCombos.end(); ++locComboIterator)
	{
		TLorentzVector locMissingP4 = dAnalysisUtilities.Calc_MissingP4(dParticleComboWrapper, 0, dMissingEnergyOffOfStepIndex, *locComboIterator, dUseKinFitFlag);
		double locMissingEnergy = locMissingP4.E();
		if((locMissingEnergy >= dMinimumMissingEnergy) && (locMissingEnergy <= dMaximumMissingEnergy))
			return true;
	}

	return false; //all failed
}

string DCutAction_InvariantMass::Get_ActionName(void) const
{
	ostringstream locStream;
	locStream << DAnalysisAction::Get_ActionName() << "_" << dMinMass << "_" << dMaxMass;
	return locStream.str();
}

bool DCutAction_InvariantMass::Perform_Action(void)
{
	for(size_t loc_i = 0; loc_i < dParticleComboWrapper->Get_NumParticleComboSteps(); ++loc_i)
	{
		const DParticleComboStep* locComboWrapperStep = dParticleComboWrapper->Get_ParticleComboStep(loc_i);
		if((dInitialPID != Unknown) && (locComboWrapperStep->Get_InitialPID() != dInitialPID))
			continue;
		if((dStepIndex != -1) && (int(loc_i) != dStepIndex))
			continue;

		//build all possible combinations of the included pids
		set<set<size_t> > locIndexCombos = dAnalysisUtilities.Build_IndexCombos(locComboWrapperStep, dToIncludePIDs);

		//loop over them: Must fail ALL to fail. if any succeed, go to the next step
		set<set<size_t> >::iterator locComboIterator = locIndexCombos.begin();
		bool locAnyOKFlag = false;
		for(; locComboIterator != locIndexCombos.end(); ++locComboIterator)
		{
			TLorentzVector locFinalStateP4 = dAnalysisUtilities.Calc_FinalStateP4(dParticleComboWrapper, loc_i, *locComboIterator, dUseKinFitFlag);
			double locInvariantMass = locFinalStateP4.M();
			if((locInvariantMass > dMaxMass) || (locInvariantMass < dMinMass))
				continue;
			locAnyOKFlag = true;
			break;
		}
		if(!locAnyOKFlag)
			return false;
	}

	return true;
}

string DCutAction_InvariantMassVeto::Get_ActionName(void) const
{
	ostringstream locStream;
	locStream << DAnalysisAction::Get_ActionName() << "_" << dMinMass << "_" << dMaxMass;
	return locStream.str();
}

bool DCutAction_InvariantMassVeto::Perform_Action(void)
{
	for(size_t loc_i = 0; loc_i < dParticleComboWrapper->Get_NumParticleComboSteps(); ++loc_i)
	{
		const DParticleComboStep* locComboWrapperStep = dParticleComboWrapper->Get_ParticleComboStep(loc_i);
		if((dInitialPID != Unknown) && (locComboWrapperStep->Get_InitialPID() != dInitialPID))
			continue;
		if((dStepIndex != -1) && (int(loc_i) != dStepIndex))
			continue;

		//build all possible combinations of the included pids
		set<set<size_t> > locIndexCombos = dAnalysisUtilities.Build_IndexCombos(locComboWrapperStep, dToIncludePIDs);

		//loop over them: Must fail ALL to fail. if any succeed, go to the next step
		set<set<size_t> >::iterator locComboIterator = locIndexCombos.begin();
		bool locAnyOKFlag = false;
		for(; locComboIterator != locIndexCombos.end(); ++locComboIterator)
		{
			TLorentzVector locFinalStateP4 = dAnalysisUtilities.Calc_FinalStateP4(dParticleComboWrapper, loc_i, *locComboIterator, dUseKinFitFlag);
			double locInvariantMass = locFinalStateP4.M();
			if((locInvariantMass < dMaxMass) && (locInvariantMass > dMinMass))
				continue;
			locAnyOKFlag = true;
			break;
		}
		if(!locAnyOKFlag)
			return false;
	}

	return true;
}

string DCutAction_KinFitFOM::Get_ActionName(void) const
{
	ostringstream locStream;
	locStream << DAnalysisAction::Get_ActionName() << "_" << dMinimumConfidenceLevel;
	return locStream.str();
}

bool DCutAction_KinFitFOM::Perform_Action(void)
{
	double locConfidenceLevel = dParticleComboWrapper->Get_ConfidenceLevel_KinFit();
	return (locConfidenceLevel > dMinimumConfidenceLevel);
}

string DCutAction_BeamEnergy::Get_ActionName(void) const
{
	ostringstream locStream;
	locStream << DAnalysisAction::Get_ActionName() << "_" << dMinBeamEnergy << "_" << dMaxBeamEnergy;
	return locStream.str();
}

bool DCutAction_BeamEnergy::Perform_Action(void)
{
	const DKinematicData* locInitParticle = dParticleComboWrapper->Get_ParticleComboStep(0)->Get_InitialParticle();
	if(locInitParticle == NULL)
		return false;

	double locBeamEnergy = dUseKinFitFlag ? locInitParticle->Get_P4().E() : locInitParticle->Get_P4_Measured().E();
	return ((locBeamEnergy >= dMinBeamEnergy) && (locBeamEnergy <= dMaxBeamEnergy));
}

string DCutAction_TrackShowerEOverP::Get_ActionName(void) const
{
	ostringstream locStream;
	locStream << DAnalysisAction::Get_ActionName() << "_" << dDetector << "_" << dPID << "_" << dShowerEOverPCut;
	return locStream.str();
}

bool DCutAction_TrackShowerEOverP::Perform_Action(void)
{
	// For all charged tracks except e+/e-, cuts those with E/p > input value
	// For e+/e-, cuts those with E/p < input value
	// Does not cut tracks without a matching FCAL shower

	for(size_t loc_i = 0; loc_i < dParticleComboWrapper->Get_NumParticleComboSteps(); ++loc_i)
	{
		DParticleComboStep* locComboWrapperStep = dParticleComboWrapper->Get_ParticleComboStep(loc_i);

		//final particles
		for(size_t loc_j = 0; loc_j < locComboWrapperStep->Get_NumFinalParticles(); ++loc_j)
		{
			DKinematicData* locKinematicData = locComboWrapperStep->Get_FinalParticle(loc_j);
			if(locKinematicData == NULL)
				continue; //e.g. a decaying or missing particle whose params aren't set yet

			//-2 if detected, -1 if missing, > 0 if decaying (step where it is the parent)
			int locDecayStepIndex = locComboWrapperStep->Get_DecayStepIndex(loc_j);
			if(locDecayStepIndex != -2)
				continue; //not measured

			if(locKinematicData->Get_PID() != dPID)
				continue;
			if(ParticleCharge(locKinematicData->Get_PID()) == 0)
				continue;

			const DChargedTrackHypothesis* locChargedTrackHypothesis = dynamic_cast<const DChargedTrackHypothesis*>(locKinematicData);
			double locShowerEOverP = 0.0;
			double locShowerEnergy = (dDetector == SYS_BCAL) ? locChargedTrackHypothesis->Get_Energy_BCAL() : locChargedTrackHypothesis->Get_Energy_FCAL();
			if(!(locShowerEnergy > 0.0))
				continue;

			if((dPID == Electron) || (dPID == Positron))
			{
				if(locShowerEOverP < dShowerEOverPCut)
					return false;
			}
			else if(locShowerEOverP > dShowerEOverPCut)
				return false;
		}
	}

	return true;
}
