#include "DCutActions.h"

bool DCutAction_PIDDeltaT::Perform_Action(void)
{
	for(size_t loc_i = 0; loc_i < dParticleComboWrapper->Get_NumParticleComboSteps(); ++loc_i)
	{
		DParticleComboStep* locParticleComboStep = dParticleComboWrapper->Get_ParticleComboStep(loc_i);

		//final particles
		for(size_t loc_j = 0; loc_j < locParticleComboStep->Get_NumFinalParticles(); ++loc_j)
		{
			DKinematicData* locKinematicData = locParticleComboStep->Get_FinalParticle(loc_j);
			if(locKinematicData == NULL)
				continue; //e.g. a decaying or missing particle whose params aren't set yet

			//-2 if detected, -1 if missing, > 0 if decaying (step where it is the parent)
			int locDecayStepIndex = locParticleComboStep->Get_DecayStepIndex(loc_j);
			if(locDecayStepIndex != -2)
				continue; //not measured

			if((dPID != Unknown) && (locKinematicData->Get_PID() != dPID))
				continue;

			// determine detector system
			DetectorSystem_t locSystem = SYS_NULL;
			if(ParticleCharge(locKinematicData->Get_PID()) != 0) {
				const DChargedTrackHypothesis* locChargedTrackHypothesis = dynamic_cast<const DChargedTrackHypothesis*>(locKinematicData);
				if(locChargedTrackHypothesis != NULL) {
					locSystem = locChargedTrackHypothesis->Get_Detector_System_Timing();
				}
			}
			else {
				const DNeutralParticleHypothesis* locNeutralParticleHypothesis = dynamic_cast<const DNeutralParticleHypothesis*>(locKinematicData);
				if(locNeutralParticleHypothesis != NULL) {
					locSystem = locNeutralParticleHypothesis->Get_Detector_System_Timing();
				}
			}
			
			if((dSystem != SYS_NULL) && (locSystem != dSystem))
				continue;

			TLorentzVector locX4 = dUseKinFitFlag ? locKinematicData->Get_X4() : locKinematicData->Get_X4_Measured();
			double locRFTime = dParticleComboWrapper->Get_RFTime_Measured();

			double locDeltaT = locX4.T() - locRFTime;
			if(fabs(locDeltaT) > dDeltaTCut) {
				//cout<<locKinematicData->Get_PID()<<" "<<locDeltaT<<endl;
				return false;
			}

		} //end of particle loop
	} //end of step loop

	return true;
}

