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
                DCutAction_PIDDeltaT(const DParticleCombo* locParticleComboWrapper, bool locUseMeasuredFlag, double locDeltaTCut, Particle_t locPID = Unknown, DetectorSystem_t locSystem = SYS_NULL, string locActionUniqueString = "") :
		dParticleComboWrapper(locParticleComboWrapper), dUseMeasuredFlag(locUseMeasuredFlag), dActionUniqueString(locActionUniqueString),
		dDeltaTCut(locDeltaTCut), dPID(locPID), dSystem(locSystem) {}

	private:
		const DParticleCombo* dParticleComboWrapper;
		bool dUseMeasuredFlag;
		string dActionUniqueString;

	public:
		double dDeltaTCut;
		Particle_t dPID;
		DetectorSystem_t dSystem;

		bool Perform_Action(void); // flag to reject combos

};

#endif // _DCutActions_
