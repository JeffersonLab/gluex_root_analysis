#ifndef _DCutActions_
#define _DCutActions_

#include <set>
#include <string>
#include <map>
#include <iostream>
#include <sstream>

#include "TLorentzVector.h"
#include "TVector3.h"
#include "TF1.h"

#include "particleType.h"

#include "DParticleCombo.h"
#include "DParticleComboStep.h"
#include "DKinematicData.h"
#include "DBeamParticle.h"
#include "DMCThrown.h"
#include "DChargedTrackHypothesis.h"
#include "DNeutralParticleHypothesis.h"
#include "DAnalysisUtilities.h"
#include "DAnalysisAction.h"

using namespace std;

class DCutAction_PIDDeltaT : public DAnalysisAction
{
	public:
		DCutAction_PIDDeltaT(const DParticleCombo* locParticleComboWrapper, bool locUseKinFitFlag, double locDeltaTCut, Particle_t locPID = Unknown, DetectorSystem_t locSystem = SYS_NULL, string locActionUniqueString = "") :
			DAnalysisAction(locParticleComboWrapper, "Cut_PIDDeltaT", locUseKinFitFlag, locActionUniqueString),
			dChargedHypoWrapper(NULL), dDeltaTCut(locDeltaTCut), dPID(locPID), dSystem(locSystem), dTargetCenterZ(0.0)
			{
				dBackgroundPIDs.insert(Proton);  dBackgroundPIDs.insert(KPlus);  dBackgroundPIDs.insert(PiPlus);
				dBackgroundPIDs.insert(KMinus);  dBackgroundPIDs.insert(PiMinus);
			}

		//for cutting background
		DCutAction_PIDDeltaT(const DParticleCombo* locParticleComboWrapper, DChargedTrackHypothesis* locChargedHypoWrapper, bool locUseKinFitFlag, double locDeltaTCut, Particle_t locPID = Unknown, DetectorSystem_t locSystem = SYS_NULL, string locActionUniqueString = "", Particle_t locPID_BG = Unknown, bool locUseCurveCutFlag = false) :	
			DAnalysisAction(locParticleComboWrapper, "Cut_PIDDeltaT", locUseKinFitFlag, locActionUniqueString),
			dChargedHypoWrapper(locChargedHypoWrapper), dDeltaTCut(locDeltaTCut), dPID(locPID), dSystem(locSystem), dPID_BG(locPID_BG), dUseCurveCutFlag(locUseCurveCutFlag), dTargetCenterZ(0.0)
			{
				dBackgroundPIDs.insert(Proton);  dBackgroundPIDs.insert(KPlus);  dBackgroundPIDs.insert(PiPlus);
				dBackgroundPIDs.insert(KMinus);  dBackgroundPIDs.insert(PiMinus);
			}

		string Get_ActionName(void) const;
		void Initialize(void);
		void Reset_NewEvent(void){}
		bool Perform_Action(void);

	private:
		DChargedTrackHypothesis * dChargedHypoWrapper;
		double dDeltaTCut;
		Particle_t dPID;
		DetectorSystem_t dSystem;
		Particle_t dPID_BG;
		bool dUseCurveCutFlag;
		double dTargetCenterZ;

	public:
		set<Particle_t> dBackgroundPIDs;
	
};

class DCutAction_NoPIDHit : public DAnalysisAction
{
	public:
		DCutAction_NoPIDHit(const DParticleCombo* locParticleComboWrapper, Particle_t locPID = Unknown, DetectorSystem_t locSystem = SYS_NULL, string locActionUniqueString = "") :
			DAnalysisAction(locParticleComboWrapper, "Cut_NoPIDHit", false, locActionUniqueString),
			dPID(locPID), dSystem(locSystem) {}

		string Get_ActionName(void) const;
		void Initialize(void){}
		void Reset_NewEvent(void){}
		bool Perform_Action(void);

	private:
		Particle_t dPID;
		DetectorSystem_t dSystem;
};

class DCutAction_dEdx : public DAnalysisAction
{
	public:
		DCutAction_dEdx(const DParticleCombo* locParticleComboWrapper, bool locUseKinFitFlag, Particle_t locPID, DetectorSystem_t locSystem = SYS_CDC, string locActionUniqueString = "") :
			DAnalysisAction(locParticleComboWrapper, "Cut_dEdx", locUseKinFitFlag, locActionUniqueString),
			dPID(locPID), dSystem(locSystem), dFunc_dEdxCut_SelectHeavy(NULL), dFunc_dEdxCut_SelectLight(NULL), dMaxRejectionFlag(false) {}

		string Get_ActionName(void) const;
		void Initialize(void);
		void Reset_NewEvent(void){}
		bool Perform_Action(void);

	private:

		Particle_t dPID;
		DetectorSystem_t dSystem;
		TF1 *dFunc_dEdxCut_SelectHeavy;
		TF1 *dFunc_dEdxCut_SelectLight;
		bool dMaxRejectionFlag;
};

class DCutAction_KinFitFOM : public DAnalysisAction
{
	public:
		DCutAction_KinFitFOM(const DParticleCombo* locParticleComboWrapper, double locMinimumConfidenceLevel, string locActionUniqueString = "") :
			DAnalysisAction(locParticleComboWrapper, "Cut_KinFitFOM", true, locActionUniqueString), dMinimumConfidenceLevel(locMinimumConfidenceLevel){}

		string Get_ActionName(void) const;
		void Initialize(void){};
		void Reset_NewEvent(void){}
		bool Perform_Action(void);

	private:
		const string dKinFitName;
		double dMinimumConfidenceLevel;
};

class DCutAction_MissingMass : public DAnalysisAction
{
	public:
		DCutAction_MissingMass(const DParticleCombo* locParticleComboWrapper, bool locUseKinFitFlag, double locMinimumMissingMass, double locMaximumMissingMass, string locActionUniqueString = "") :
			DAnalysisAction(locParticleComboWrapper, "Cut_MissingMass", locUseKinFitFlag, locActionUniqueString),
			dMinimumMissingMass(locMinimumMissingMass), dMaximumMissingMass(locMaximumMissingMass), dMissingMassOffOfStepIndex(0){}

		//E.g. If:
		//g, p -> K+, K+, Xi-
		//                Xi- -> pi-, Lambda
		//                            Lambda -> (p), pi-
		//And:
		//locMissingMassOffOfStepIndex = 0, locMissingMassOffOfPIDs = K+, K+
		//Then: Will cut missing-mass: g, p -> K+, K+, (X)
		//Also:
		//locMissingMassOffOfStepIndex = 1, locMissingMassOffOfPID = pi-
		//Then: Will cut missing-mass: g, p -> K+, K+, pi- (from Xi- decay)
		//But:
		//locMissingMassOffOfStepIndex = 0, locMissingMassOffOfPIDs = K+
		//Then: Will cut only missing-mass: g, p -> K+_1, (X)    and NOT K+_2!!!
		DCutAction_MissingMass(const DParticleCombo* locParticleComboWrapper, bool locUseKinFitFlag, int locMissingMassOffOfStepIndex, deque<Particle_t> locMissingMassOffOfPIDs, double locMinimumMissingMass, double locMaximumMissingMass, string locActionUniqueString = "") :
			DAnalysisAction(locParticleComboWrapper, "Cut_MissingMass", locUseKinFitFlag, locActionUniqueString),
			dMinimumMissingMass(locMinimumMissingMass), dMaximumMissingMass(locMaximumMissingMass), dMissingMassOffOfStepIndex(locMissingMassOffOfStepIndex),
			dMissingMassOffOfPIDs(locMissingMassOffOfPIDs) {}

		DCutAction_MissingMass(const DParticleCombo* locParticleComboWrapper, bool locUseKinFitFlag, int locMissingMassOffOfStepIndex, Particle_t locMissingMassOffOfPID, double locMinimumMissingMass, double locMaximumMissingMass, string locActionUniqueString = "") :
			DAnalysisAction(locParticleComboWrapper, "Cut_MissingMass", locUseKinFitFlag, locActionUniqueString),
			dMinimumMissingMass(locMinimumMissingMass), dMaximumMissingMass(locMaximumMissingMass), dMissingMassOffOfStepIndex(locMissingMassOffOfStepIndex),
			dMissingMassOffOfPIDs(deque<Particle_t>(1, locMissingMassOffOfPID)) {}

		string Get_ActionName(void) const;
		void Initialize(void){};
		void Reset_NewEvent(void){}
		bool Perform_Action(void);

	private:

		double dMinimumMissingMass;
		double dMaximumMissingMass;
		int dMissingMassOffOfStepIndex;
		deque<Particle_t> dMissingMassOffOfPIDs;

		DAnalysisUtilities dAnalysisUtilities;
};

class DCutAction_MissingMassSquared : public DAnalysisAction
{
	public:
		DCutAction_MissingMassSquared(const DParticleCombo* locParticleComboWrapper, bool locUseKinFitFlag, double locMinimumMissingMassSq, double locMaximumMissingMassSq, string locActionUniqueString = "") :
			DAnalysisAction(locParticleComboWrapper, "Cut_MissingMassSquared", locUseKinFitFlag, locActionUniqueString),
			dMinimumMissingMassSq(locMinimumMissingMassSq), dMaximumMissingMassSq(locMaximumMissingMassSq), dMissingMassOffOfStepIndex(0){}

		//E.g. If:
		//g, p -> K+, K+, Xi-
		//                Xi- -> pi-, Lambda
		//                            Lambda -> (p), pi-
		//And:
		//locMissingMassOffOfStepIndex = 0, locMissingMassOffOfPIDs = K+, K+
		//Then: Will cut missing-mass: g, p -> K+, K+, (X)
		//Also:
		//locMissingMassOffOfStepIndex = 1, locMissingMassOffOfPID = pi-
		//Then: Will cut missing-mass: g, p -> K+, K+, pi-
		//But:
		//locMissingMassOffOfStepIndex = 0, locMissingMassOffOfPIDs = K+
		//Then: Will cut only if BOTH missing-mass: g, p -> K+_1, (X)   AND   g, p -> K+_2, (X) fail.
		DCutAction_MissingMassSquared(const DParticleCombo* locParticleComboWrapper, bool locUseKinFitFlag, int locMissingMassOffOfStepIndex, deque<Particle_t> locMissingMassOffOfPIDs, double locMinimumMissingMassSq, double locMaximumMissingMassSq, string locActionUniqueString = "") :
			DAnalysisAction(locParticleComboWrapper, "Cut_MissingMassSquared", locUseKinFitFlag, locActionUniqueString),
			dMinimumMissingMassSq(locMinimumMissingMassSq), dMaximumMissingMassSq(locMaximumMissingMassSq), dMissingMassOffOfStepIndex(locMissingMassOffOfStepIndex),
			dMissingMassOffOfPIDs(locMissingMassOffOfPIDs) {}

		DCutAction_MissingMassSquared(const DParticleCombo* locParticleComboWrapper, bool locUseKinFitFlag, int locMissingMassOffOfStepIndex, Particle_t locMissingMassOffOfPID, double locMinimumMissingMassSq, double locMaximumMissingMassSq, string locActionUniqueString = "") :
			DAnalysisAction(locParticleComboWrapper, "Cut_MissingMassSquared", locUseKinFitFlag, locActionUniqueString),
			dMinimumMissingMassSq(locMinimumMissingMassSq), dMaximumMissingMassSq(locMaximumMissingMassSq), dMissingMassOffOfStepIndex(locMissingMassOffOfStepIndex),
			dMissingMassOffOfPIDs(deque<Particle_t>(1, locMissingMassOffOfPID)) {}

		string Get_ActionName(void) const;
		void Initialize(void){};
		void Reset_NewEvent(void){}
		bool Perform_Action(void);

	private:

		double dMinimumMissingMassSq;
		double dMaximumMissingMassSq;
		int dMissingMassOffOfStepIndex;
		deque<Particle_t> dMissingMassOffOfPIDs;

		DAnalysisUtilities dAnalysisUtilities;
};

class DCutAction_MissingEnergy : public DAnalysisAction
{
	public:
		DCutAction_MissingEnergy(const DParticleCombo* locParticleComboWrapper, bool locUseKinFitFlag, double locMinimumMissingEnergy, double locMaximumMissingEnergy, string locActionUniqueString = "") :
			DAnalysisAction(locParticleComboWrapper, "Cut_MissingEnergy", locUseKinFitFlag, locActionUniqueString),
			dMinimumMissingEnergy(locMinimumMissingEnergy), dMaximumMissingEnergy(locMaximumMissingEnergy), dMissingEnergyOffOfStepIndex(0){}

		DCutAction_MissingEnergy(const DParticleCombo* locParticleComboWrapper, bool locUseKinFitFlag, int locMissingEnergyOffOfStepIndex, deque<Particle_t> locMissingEnergyOffOfPIDs, double locMinimumMissingEnergy, double locMaximumMissingEnergy, string locActionUniqueString = "") :
			DAnalysisAction(locParticleComboWrapper, "Cut_MissingEnergy", locUseKinFitFlag, locActionUniqueString),
			dMinimumMissingEnergy(locMinimumMissingEnergy), dMaximumMissingEnergy(locMaximumMissingEnergy), dMissingEnergyOffOfStepIndex(locMissingEnergyOffOfStepIndex),
			dMissingEnergyOffOfPIDs(locMissingEnergyOffOfPIDs) {}

		DCutAction_MissingEnergy(const DParticleCombo* locParticleComboWrapper, bool locUseKinFitFlag, int locMissingEnergyOffOfStepIndex, Particle_t locMissingEnergyOffOfPID, double locMinimumMissingEnergy, double locMaximumMissingEnergy, string locActionUniqueString = "") :
			DAnalysisAction(locParticleComboWrapper, "Cut_MissingEnergy", locUseKinFitFlag, locActionUniqueString),
			dMinimumMissingEnergy(locMinimumMissingEnergy), dMaximumMissingEnergy(locMaximumMissingEnergy), dMissingEnergyOffOfStepIndex(locMissingEnergyOffOfStepIndex),
			dMissingEnergyOffOfPIDs(deque<Particle_t>(1, locMissingEnergyOffOfPID)) {}

		string Get_ActionName(void) const;
		void Initialize(void){};
		void Reset_NewEvent(void){}
		bool Perform_Action(void);

	private:

		double dMinimumMissingEnergy;
		double dMaximumMissingEnergy;
		int dMissingEnergyOffOfStepIndex;
		deque<Particle_t> dMissingEnergyOffOfPIDs;

		DAnalysisUtilities dAnalysisUtilities;
};

class DCutAction_InvariantMass : public DAnalysisAction
{
	public:
		DCutAction_InvariantMass(const DParticleCombo* locParticleComboWrapper, bool locUseKinFitFlag, Particle_t locInitialPID, double locMinMass, double locMaxMass, string locActionUniqueString = "") :
			DAnalysisAction(locParticleComboWrapper, "Cut_InvariantMass", locUseKinFitFlag, locActionUniqueString),
			dInitialPID(locInitialPID), dStepIndex(-1), dToIncludePIDs(deque<Particle_t>()), dMinMass(locMinMass), dMaxMass(locMaxMass){}

		//e.g. if g, p -> pi+, pi-, p
			//call with step = 0, PIDs = pi+, pi-, and will histogram rho mass
		DCutAction_InvariantMass(const DParticleCombo* locParticleComboWrapper, bool locUseKinFitFlag, size_t locStepIndex, deque<Particle_t> locToIncludePIDs, double locMinMass, double locMaxMass, string locActionUniqueString = "") :
			DAnalysisAction(locParticleComboWrapper, "Cut_InvariantMass", locUseKinFitFlag, locActionUniqueString),
			dInitialPID(Unknown), dStepIndex(locStepIndex), dToIncludePIDs(locToIncludePIDs), dMinMass(locMinMass), dMaxMass(locMaxMass){}

		string Get_ActionName(void) const;
		void Initialize(void){};
		void Reset_NewEvent(void){}
		bool Perform_Action(void);

	private:
		Particle_t dInitialPID;
		int dStepIndex;
		deque<Particle_t> dToIncludePIDs;

		double dMinMass;
		double dMaxMass;
		DAnalysisUtilities dAnalysisUtilities;
};

class DCutAction_InvariantMassVeto : public DAnalysisAction
{
	public:
		DCutAction_InvariantMassVeto(const DParticleCombo* locParticleComboWrapper, bool locUseKinFitFlag, Particle_t locInitialPID, double locMinMass, double locMaxMass, string locActionUniqueString = "") :
			DAnalysisAction(locParticleComboWrapper, "Cut_InvariantMassVeto", locUseKinFitFlag, locActionUniqueString),
			dInitialPID(locInitialPID), dStepIndex(-1), dToIncludePIDs(deque<Particle_t>()), dMinMass(locMinMass), dMaxMass(locMaxMass){}

		//e.g. if g, p -> pi+, pi-, p
			//call with step = 0, PIDs = pi+, pi-, and will histogram rho mass
		DCutAction_InvariantMassVeto(const DParticleCombo* locParticleComboWrapper, bool locUseKinFitFlag, size_t locStepIndex, deque<Particle_t> locToIncludePIDs, double locMinMass, double locMaxMass, string locActionUniqueString = "") :
			DAnalysisAction(locParticleComboWrapper, "Cut_InvariantMassVeto", locUseKinFitFlag, locActionUniqueString),
			dInitialPID(Unknown), dStepIndex(locStepIndex), dToIncludePIDs(locToIncludePIDs), dMinMass(locMinMass), dMaxMass(locMaxMass){}

		string Get_ActionName(void) const;
		void Initialize(void){};
		void Reset_NewEvent(void){}
		bool Perform_Action(void);

	private:
		Particle_t dInitialPID;
		int dStepIndex;
		deque<Particle_t> dToIncludePIDs;

		double dMinMass;
		double dMaxMass;
		DAnalysisUtilities dAnalysisUtilities;
};

class DCutAction_BeamEnergy : public DAnalysisAction
{
	public:
		DCutAction_BeamEnergy(const DParticleCombo* locParticleComboWrapper, bool locUseKinFitFlag, double locMinBeamEnergy, double locMaxBeamEnergy, string locActionUniqueString = "") :
			DAnalysisAction(locParticleComboWrapper, "Cut_BeamEnergy", locUseKinFitFlag, locActionUniqueString),
			dMinBeamEnergy(locMinBeamEnergy), dMaxBeamEnergy(locMaxBeamEnergy){}

		string Get_ActionName(void) const;
		void Initialize(void){};
		void Reset_NewEvent(void){}
		bool Perform_Action(void);

	private:

		double dMinBeamEnergy;
		double dMaxBeamEnergy;
};

class DCutAction_TrackShowerEOverP : public DAnalysisAction
{
	// For all charged tracks except e+/e-, cuts those with FCAL E/p > input value
	// For e+/e-, cuts those with FCAL E/p < input value
	// Does not cut tracks without a matching shower

	public:

		DCutAction_TrackShowerEOverP(const DParticleCombo* locParticleComboWrapper, bool locUseKinFitFlag, DetectorSystem_t locDetector, Particle_t locPID, double locShowerEOverPCut, string locActionUniqueString = "") :
		DAnalysisAction(locParticleComboWrapper, "Cut_TrackShowerEOverP", locUseKinFitFlag, locActionUniqueString),
		dDetector(locDetector), dPID(locPID), dShowerEOverPCut(locShowerEOverPCut) {}

		string Get_ActionName(void) const;
		void Initialize(void){};
		void Reset_NewEvent(void){}
		bool Perform_Action(void);

	private:

		DetectorSystem_t dDetector;
		Particle_t dPID;
		double dShowerEOverPCut;
};

class DCutAction_Kinematics : public DAnalysisAction
{
	//input range is what is cut: cut is ignored if min > max
	//if step index == -1: all steps
	//if PID == Unknown: all particles
	//particle must be detected
	//angles in degrees
	public:
		DCutAction_Kinematics(const DParticleCombo* locParticleComboWrapper, int locStepIndex, Particle_t locPID, bool locUseKinFitFlag,
			double locCutMinP, double locCutMaxP, double locCutMinTheta = 1.0, double locCutMaxTheta = 0.0, double locCutMinPhi = 1.0, double locCutMaxPhi = 0.0) :
			DAnalysisAction(locParticleComboWrapper, "Cut_Kinematics", locUseKinFitFlag, ""),
			dStepIndex(locStepIndex), dPID(locPID), dCutMinP(locCutMinP), dCutMaxP(locCutMaxP), dCutMinTheta(locCutMinTheta),
			dCutMaxTheta(locCutMaxTheta), dCutMinPhi(locCutMinPhi), dCutMaxPhi(locCutMaxPhi) {}

		string Get_ActionName(void) const;
		void Initialize(void){};
		void Reset_NewEvent(void){}
		bool Perform_Action(void);

	private:

		int dStepIndex;
		Particle_t dPID;
		double dCutMinP;
		double dCutMaxP;
		double dCutMinTheta;
		double dCutMaxTheta;
		double dCutMinPhi;
		double dCutMaxPhi;
};

#endif // _DCutActions_
