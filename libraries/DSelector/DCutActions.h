#ifndef _DCutActions_
#define _DCutActions_

#include <set>
#include <string>
#include <map>
#include <iostream>
#include <sstream>

#include "TLorentzVector.h"
#include "TVector3.h"

#include "particleType.h"

#include "DParticleCombo.h"
#include "DParticleComboStep.h"
#include "DKinematicData.h"
#include "DBeamParticle.h"
#include "DMCThrown.h"
#include "DChargedTrackHypothesis.h"
#include "DNeutralParticleHypothesis.h"

using namespace std;

class DCutAction_PIDDeltaT
{
	public:
                DCutAction_PIDDeltaT(const DParticleCombo* locParticleComboWrapper, bool locUseKinFitFlag, double locDeltaTCut, Particle_t locPID = Unknown, DetectorSystem_t locSystem = SYS_NULL, string locActionUniqueString = "") :
		dParticleComboWrapper(locParticleComboWrapper), dUseKinFitFlag(locUseKinFitFlag), dActionUniqueString(locActionUniqueString),
		dDeltaTCut(locDeltaTCut), dPID(locPID), dSystem(locSystem) {}

	private:

		double dTargetCenterZ;
		
		const DParticleCombo* dParticleComboWrapper;
		bool dUseKinFitFlag;
		string dActionUniqueString;

	public:
		double dDeltaTCut;
		Particle_t dPID;
		DetectorSystem_t dSystem;

		void Initialize(void);
		bool Perform_Action(void); // flag to reject combos

};

#endif // _DCutActions_
