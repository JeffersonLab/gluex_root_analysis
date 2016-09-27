#ifndef _DHistogramActions_
#define _DHistogramActions_

#include <set>
#include <string>
#include <map>
#include <iostream>
#include <sstream>

#include "TROOT.h"
#include "TDirectoryFile.h"
#include "TH1I.h"
#include "TH2I.h"
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
#include "DAnalysisUtilities.h"
#include "DAnalysisAction.h"

using namespace std;

class DHistogramAction_ParticleComboKinematics : public DAnalysisAction
{
	public:
		DHistogramAction_ParticleComboKinematics(const DParticleCombo* locParticleComboWrapper, bool locUseKinFitFlag, string locActionUniqueString = "") :
			DAnalysisAction(locParticleComboWrapper, "Hist_ParticleComboKinematics", locUseKinFitFlag, locActionUniqueString),
			dNumPBins(500), dNumThetaBins(560), dNumPhiBins(360), dNumVertexZBins(600), dNumTBins(200), dNumVertexXYBins(200), dNumBetaBins(400), dNumDeltaBetaBins(400),
			dNum2DPBins(250), dNum2DThetaBins(140), dNum2DPhiBins(180), dNumDeltaTRFBins(500), dNumPathLengthBins(750), dNumLifetimeBins(500),
			dMinT(-5.0), dMaxT(5.0), dMinP(0.0), dMaxP(10.0), dMinTheta(0.0), dMaxTheta(140.0), dMinPhi(-180.0), dMaxPhi(180.0), dMinVertexZ(0.0), dMaxVertexZ(200.0),
			dMinVertexXY(-5.0), dMaxVertexXY(5.0), dMinBeta(-0.2), dMaxBeta(1.2), dMinDeltaBeta(-1.0), dMaxDeltaBeta(1.0), dMinDeltaTRF(-10.0), dMaxDeltaTRF(10.0),
			dMaxPathLength(15), dMaxLifetime(5.0), dMaxBeamE(12.0) {}

		void Reset_NewEvent(void){dPreviouslyHistogrammed.clear();}; //reset uniqueness tracking
		void Initialize(void);
		bool Perform_Action(void);

		unsigned int dNumPBins, dNumThetaBins, dNumPhiBins, dNumVertexZBins, dNumTBins, dNumVertexXYBins, dNumBetaBins, dNumDeltaBetaBins;
		unsigned int dNum2DPBins, dNum2DThetaBins, dNum2DPhiBins, dNumDeltaTRFBins, dNumPathLengthBins, dNumLifetimeBins;
		double dMinT, dMaxT, dMinP, dMaxP, dMinTheta, dMaxTheta, dMinPhi, dMaxPhi, dMinVertexZ, dMaxVertexZ, dMinVertexXY, dMaxVertexXY;
		double dMinBeta, dMaxBeta, dMinDeltaBeta, dMaxDeltaBeta, dMinDeltaTRF, dMaxDeltaTRF, dMaxPathLength, dMaxLifetime, dMaxBeamE;

	private:

		double dTargetCenterZ;

		void Create_Hists(int locStepIndex, string locStepROOTName, Particle_t locPID, bool locIsBeamFlag = false);
		void Fill_Hists(const DKinematicData* locKinematicData, size_t locStepIndex);
		void Fill_BeamHists(const DKinematicData* locKinematicData, double locRFTime);

		TH1I* dBeamParticleHist_DeltaTRF;
		TH2I* dBeamParticleHist_DeltaTRFVsBeamE;

		//keys are step index, PID //beam has PID Unknown
		map<size_t, map<Particle_t, TH2I*> > dHistMap_PVsTheta;
		map<size_t, map<Particle_t, TH2I*> > dHistMap_BetaVsP;
		map<size_t, map<Particle_t, TH2I*> > dHistMap_DeltaBetaVsP;
		map<size_t, map<Particle_t, TH2I*> > dHistMap_PhiVsTheta;
		map<size_t, map<Particle_t, TH1I*> > dHistMap_P;
		map<size_t, map<Particle_t, TH1I*> > dHistMap_Theta;
		map<size_t, map<Particle_t, TH1I*> > dHistMap_Phi;
		map<size_t, map<Particle_t, TH1I*> > dHistMap_VertexZ;
		map<size_t, map<Particle_t, TH1I*> > dHistMap_VertexT;
		map<size_t, map<Particle_t, TH2I*> > dHistMap_VertexYVsX;

		map<size_t, TH1I*> dHistMap_MaxTrackDeltaZ;
		map<size_t, TH1I*> dHistMap_MaxTrackDeltaT;
		map<size_t, TH1I*> dHistMap_MaxTrackDOCA;

		map<size_t, map<Particle_t, set<Int_t> > > dPreviouslyHistogrammed; //step index, PID, particle indices

		//other than first, skipped if not detached vertex
		map<size_t, TH1I*> dHistMap_StepVertexZ;
		map<size_t, TH2I*> dHistMap_StepVertexYVsX;
		map<size_t, TH1I*> dHistMap_StepVertexT;

		//size_t is step index where the detached-vertex particle decays
		map<size_t, TH1I*> dHistMap_DetachedPathLength; //distance between this vertex and the previous one (if detached)
		map<size_t, TH1I*> dHistMap_DetachedLifetime; //delta-t between this vertex and the previous one (if detached)
		map<size_t, TH1I*> dHistMap_DetachedLifetime_RestFrame; //in rest frame
};

class DHistogramAction_ParticleID : public DAnalysisAction
{
	public:
		DHistogramAction_ParticleID(const DParticleCombo* locParticleComboWrapper, bool locUseKinFitFlag, string locActionUniqueString = "") :
			DAnalysisAction(locParticleComboWrapper, "Hist_ParticleID", locUseKinFitFlag, locActionUniqueString),
			dNumPBins(500), dNumThetaBins(560), dNumPhiBins(360), dNumTBins(200), dNumVertexXYBins(200), dNumBetaBins(400), dNumDeltaBetaBins(400),
			dNum2DPBins(250), dNum2DThetaBins(140), dNum2DPhiBins(180), dNumPathLengthBins(750), dNumLifetimeBins(500),
			dNumDeltaTBins(500), dNum2DdEdxBins(250), dNumEoverPBins(200),
			dMinT(-5.0), dMaxT(5.0), dMinP(0.0), dMaxP(10.0), dMinTheta(0.0), dMaxTheta(140.0), dMinPhi(-180.0), dMaxPhi(180.0), dMinVertexZ(0.0), dMaxVertexZ(200.0),
			dMinVertexXY(-5.0), dMaxVertexXY(5.0), dMinBeta(-0.2), dMaxBeta(1.2), dMinDeltaBeta(-1.0), dMaxDeltaBeta(1.0),
			dMaxPathLength(15), dMaxLifetime(5.0), dMaxBeamE(12.0),
			dMinDeltaT(-10.0), dMaxDeltaT(10.0), dMindEdx(0.0), dMaxdEdx(25.0), dMinEoverP(0.0), dMaxEoverP(4.0) {}

		void Reset_NewEvent(void){dPreviouslyHistogrammed.clear();}; //reset uniqueness tracking
		void Initialize(void);
		bool Perform_Action(void);

		unsigned int dNumPBins, dNumThetaBins, dNumPhiBins, dNumTBins, dNumVertexXYBins, dNumBetaBins, dNumDeltaBetaBins;
		unsigned int dNum2DPBins, dNum2DThetaBins, dNum2DPhiBins, dNumDeltaTRFBins, dNumPathLengthBins, dNumLifetimeBins;
		unsigned int dNumDeltaTBins, dNum2DdEdxBins, dNumEoverPBins;
		double dMinT, dMaxT, dMinP, dMaxP, dMinTheta, dMaxTheta, dMinPhi, dMaxPhi, dMinVertexZ, dMaxVertexZ, dMinVertexXY, dMaxVertexXY;
		double dMinBeta, dMaxBeta, dMinDeltaBeta, dMaxDeltaBeta, dMinDeltaTRF, dMaxDeltaTRF, dMaxPathLength, dMaxLifetime, dMaxBeamE;
		double dMinDeltaT, dMaxDeltaT, dMindEdx, dMaxdEdx, dMinEoverP, dMaxEoverP;

	private:

		double dTargetCenterZ;

		void Create_Hists(int locStepIndex, string locStepROOTName, Particle_t locPID);
		void Fill_Hists(const DKinematicData* locKinematicData, size_t locStepIndex);

		//keys are step index, PID //beam has PID Unknown
		map<size_t, map<Particle_t, TH2I*> > dHistMap_dEdxVsP_CDC;
		map<size_t, map<Particle_t, TH2I*> > dHistMap_dEdxVsP_FDC;
		map<size_t, map<Particle_t, TH2I*> > dHistMap_dEdxVsP_ST;
		map<size_t, map<Particle_t, TH2I*> > dHistMap_dEdxVsP_TOF;

		map<size_t, map<Particle_t, TH2I*> > dHistMap_BetaVsP_BCAL;
		map<size_t, map<Particle_t, TH2I*> > dHistMap_BetaVsP_TOF;
		map<size_t, map<Particle_t, TH2I*> > dHistMap_BetaVsP_FCAL;
		map<size_t, map<Particle_t, TH2I*> > dHistMap_DeltaTVsP_BCAL;	
		map<size_t, map<Particle_t, TH2I*> > dHistMap_DeltaTVsP_TOF;
		map<size_t, map<Particle_t, TH2I*> > dHistMap_DeltaTVsP_FCAL;

		map<size_t, map<Particle_t, TH2I*> > dHistMap_EoverPVsP_BCAL;
		map<size_t, map<Particle_t, TH2I*> > dHistMap_EoverPVsTheta_BCAL;
		map<size_t, map<Particle_t, TH2I*> > dHistMap_EoverPVsP_FCAL;
		map<size_t, map<Particle_t, TH2I*> > dHistMap_EoverPVsTheta_FCAL;
		
		map<size_t, map<Particle_t, TH2I*> > dHistMap_ShowerZVsParticleZ;
		map<size_t, map<Particle_t, TH2I*> > dHistMap_ShowerTVsParticleT;

		map<size_t, map<Particle_t, set<Int_t> > > dPreviouslyHistogrammed; //step index, PID, particle indices
};

class DHistogramAction_InvariantMass : public DAnalysisAction
{
	public:
		DHistogramAction_InvariantMass(const DParticleCombo* locParticleComboWrapper, bool locUseKinFitFlag, Particle_t locInitialPID, unsigned int locNumMassBins, double locMinMass, double locMaxMass, string locActionUniqueString = "") :
			DAnalysisAction(locParticleComboWrapper, "Hist_InvariantMass", locUseKinFitFlag, locActionUniqueString),
			dInitialPID(locInitialPID), dStepIndex(-1), dToIncludePIDs(deque<Particle_t>()),
			dNumMassBins(locNumMassBins), dNum2DMassBins(locNumMassBins/2), dMinMass(locMinMass), dMaxMass(locMaxMass),
			dNumConLevBins(1000), dNumBinsPerConLevPower(18), dConLevLowest10Power(-50) {}

		//e.g. if g, p -> pi+, pi-, p
			//call with step = 0, PIDs = pi+, pi-, and will histogram rho mass
		DHistogramAction_InvariantMass(const DParticleCombo* locParticleComboWrapper, bool locUseKinFitFlag, size_t locStepIndex, deque<Particle_t> locToIncludePIDs, unsigned int locNumMassBins, double locMinMass, double locMaxMass, string locActionUniqueString = "") :
			DAnalysisAction(locParticleComboWrapper, "Hist_InvariantMass", locUseKinFitFlag, locActionUniqueString),
			dInitialPID(Unknown), dStepIndex(locStepIndex), dToIncludePIDs(locToIncludePIDs),
			dNumMassBins(locNumMassBins), dNum2DMassBins(locNumMassBins/2), dMinMass(locMinMass), dMaxMass(locMaxMass),
			dNumConLevBins(1000), dNumBinsPerConLevPower(18), dConLevLowest10Power(-50) {}

		void Reset_NewEvent(void) //reset uniqueness tracking
		{
			dPreviouslyHistogrammed.clear();
			dPreviouslyHistogrammed_ConLev.clear();
		}
		void Initialize(void);
		bool Perform_Action(void);

	private:
		Particle_t dInitialPID;
		int dStepIndex;
		deque<Particle_t> dToIncludePIDs;

		unsigned int dNumMassBins, dNum2DMassBins;
		double dMinMass, dMaxMass;

	public:
		unsigned int dNumConLevBins, dNumBinsPerConLevPower;
		int dConLevLowest10Power;

	private:
		DAnalysisUtilities dAnalysisUtilities;
		TH1I* dHist_InvaraintMass;
		TH2I* dHist_InvaraintMassVsConfidenceLevel;
		TH2I* dHist_InvaraintMassVsConfidenceLevel_LogX;

		//In general: Could have multiple particles with the same PID: Use a set of Int_t's
		//In general: Multiple PIDs, so multiple sets: Contain within a map
		//Multiple combos: Contain maps within a set (easier, faster to search)
		set<map<Particle_t, set<Int_t> > > dPreviouslyHistogrammed;
		set<pair<Int_t, map<Particle_t, set<Int_t> > > > dPreviouslyHistogrammed_ConLev; //first Int_t: combo index: kinfit (probably) unique for each combo
};

class DHistogramAction_MissingMass : public DAnalysisAction
{
	public:
		DHistogramAction_MissingMass(const DParticleCombo* locParticleComboWrapper, bool locUseKinFitFlag, unsigned int locNumMassBins, double locMinMass, double locMaxMass, string locActionUniqueString = "") :
			DAnalysisAction(locParticleComboWrapper, "Hist_MissingMass", locUseKinFitFlag, locActionUniqueString),
			dNumMassBins(locNumMassBins), dMinMass(locMinMass), dMaxMass(locMaxMass), dMissingMassOffOfStepIndex(0), dMissingMassOffOfPIDs(deque<Particle_t>()),
			dNum2DMassBins(locNumMassBins/2), dNum2DBeamEBins(600), dNum2DMissPBins(450), dMinBeamE(0.0), dMaxBeamE(12.0), dMinMissP(0.0), dMaxMissP(9.0),
			dNumConLevBins(1000), dNumBinsPerConLevPower(18), dConLevLowest10Power(-50) {}

		//E.g. If:
		//g, p -> K+, K+, Xi-
		//                Xi- -> pi-, Lambda
		//                            Lambda -> (p), pi-
		//And:
		//locMissingMassOffOfStepIndex = 0, locMissingMassOffOfPIDs = K+, K+
		//Then: Will histogram missing-mass: g, p -> K+, K+, (X)
		//Also:
		//locMissingMassOffOfStepIndex = 1, locMissingMassOffOfPID = pi-
		//Then: Will histogram missing-mass: g, p -> K+, K+, pi-
		//But:
		//locMissingMassOffOfStepIndex = 0, locMissingMassOffOfPIDs = K+
		//Then: Will histogram only missing-mass: g, p -> K+_1, (X)    and NOT K+_2!!!
		DHistogramAction_MissingMass(const DParticleCombo* locParticleComboWrapper, bool locUseKinFitFlag, int locMissingMassOffOfStepIndex, deque<Particle_t> locMissingMassOffOfPIDs, unsigned int locNumMassBins, double locMinMass, double locMaxMass, string locActionUniqueString = "") :
			DAnalysisAction(locParticleComboWrapper, "Hist_MissingMass", locUseKinFitFlag, locActionUniqueString),
			dNumMassBins(locNumMassBins), dMinMass(locMinMass), dMaxMass(locMaxMass),
			dMissingMassOffOfStepIndex(locMissingMassOffOfStepIndex), dMissingMassOffOfPIDs(locMissingMassOffOfPIDs),
			dNum2DMassBins(locNumMassBins/2), dNum2DBeamEBins(600), dNum2DMissPBins(450), dMinBeamE(0.0), dMaxBeamE(12.0), dMinMissP(0.0), dMaxMissP(9.0),
			dNumConLevBins(1000), dNumBinsPerConLevPower(18), dConLevLowest10Power(-50) {}

		DHistogramAction_MissingMass(const DParticleCombo* locParticleComboWrapper, bool locUseKinFitFlag, int locMissingMassOffOfStepIndex, Particle_t locMissingMassOffOfPID, unsigned int locNumMassBins, double locMinMass, double locMaxMass, string locActionUniqueString = "") :
			DAnalysisAction(locParticleComboWrapper, "Hist_MissingMass", locUseKinFitFlag, locActionUniqueString),
			dNumMassBins(locNumMassBins), dMinMass(locMinMass), dMaxMass(locMaxMass),
			dMissingMassOffOfStepIndex(locMissingMassOffOfStepIndex), dMissingMassOffOfPIDs(deque<Particle_t>(1, locMissingMassOffOfPID)),
			dNum2DMassBins(locNumMassBins/2), dNum2DBeamEBins(600), dNum2DMissPBins(450), dMinBeamE(0.0), dMaxBeamE(12.0), dMinMissP(0.0), dMaxMissP(9.0),
			dNumConLevBins(1000), dNumBinsPerConLevPower(18), dConLevLowest10Power(-50) {}

                void Reset_NewEvent(void) //reset uniqueness tracking
                {
                        dPreviouslyHistogrammed.clear();
                        dPreviouslyHistogrammed_ConLev.clear();
                }
		void Initialize(void);
		bool Perform_Action(void);

	private:
		unsigned int dNumMassBins;
		double dMinMass, dMaxMass;
		int dMissingMassOffOfStepIndex;
		deque<Particle_t> dMissingMassOffOfPIDs;

	public:
		unsigned int dNum2DMassBins, dNum2DBeamEBins, dNum2DMissPBins;
		double dMinBeamE, dMaxBeamE, dMinMissP, dMaxMissP;

		unsigned int dNumConLevBins, dNumBinsPerConLevPower;
		int dConLevLowest10Power;

	private:
		TH1I* dHist_MissingMass;
		TH2I* dHist_MissingMassVsBeamE;
		TH2I* dHist_MissingMassVsMissingP;
		TH2I* dHist_MissingMassVsConfidenceLevel;
		TH2I* dHist_MissingMassVsConfidenceLevel_LogX;
		DAnalysisUtilities dAnalysisUtilities;

		//In general: Could have multiple particles with the same PID: Use a set of Int_t's
		//In general: Multiple PIDs, so multiple sets: Contain within a map
		//Multiple combos: Contain maps within a set (easier, faster to search)
		set<map<Particle_t, set<Int_t> > > dPreviouslyHistogrammed;
                set<pair<Int_t, map<Particle_t, set<Int_t> > > > dPreviouslyHistogrammed_ConLev; //first Int_t: combo index: kinfit (probably) unique for each combo
};

class DHistogramAction_MissingMassSquared : public DAnalysisAction
{
	public:
		DHistogramAction_MissingMassSquared(const DParticleCombo* locParticleComboWrapper, bool locUseKinFitFlag, unsigned int locNumMassBins, double locMinMassSq, double locMaxMassSq, string locActionUniqueString = "") :
			DAnalysisAction(locParticleComboWrapper, "Hist_MissingMassSquared", locUseKinFitFlag, locActionUniqueString),
			dNumMassBins(locNumMassBins), dMinMass(locMinMassSq), dMaxMass(locMaxMassSq), dMissingMassOffOfStepIndex(0), dMissingMassOffOfPIDs(deque<Particle_t>()),
			dNum2DMassBins(locNumMassBins/2), dNum2DBeamEBins(600), dNum2DMissPBins(450), dMinBeamE(0.0), dMaxBeamE(12.0), dMinMissP(0.0), dMaxMissP(9.0),
			dNumConLevBins(1000), dNumBinsPerConLevPower(18), dConLevLowest10Power(-50) {}

		//E.g. If:
		//g, p -> K+, K+, Xi-
		//                Xi- -> pi-, Lambda
		//                            Lambda -> (p), pi-
		//And:
		//locMissingMassOffOfStepIndex = 0, locMissingMassOffOfPIDs = K+, K+
		//Then: Will histogram missing-mass: g, p -> K+, K+, (X)
		//Also:
		//locMissingMassOffOfStepIndex = 1, locMissingMassOffOfPID = pi-
		//Then: Will histogram missing-mass: g, p -> K+, K+, pi-
		//But:
		//locMissingMassOffOfStepIndex = 0, locMissingMassOffOfPIDs = K+
		//Then: Will histogram only missing-mass: g, p -> K+_1, (X)    and NOT K+_2!!!
		DHistogramAction_MissingMassSquared(const DParticleCombo* locParticleComboWrapper, bool locUseKinFitFlag, int locMissingMassOffOfStepIndex, deque<Particle_t> locMissingMassOffOfPIDs, unsigned int locNumMassBins, double locMinMassSq, double locMaxMassSq, string locActionUniqueString = "") :
			DAnalysisAction(locParticleComboWrapper, "Hist_MissingMassSquared", locUseKinFitFlag, locActionUniqueString),
			dNumMassBins(locNumMassBins), dMinMass(locMinMassSq), dMaxMass(locMaxMassSq),
			dMissingMassOffOfStepIndex(locMissingMassOffOfStepIndex), dMissingMassOffOfPIDs(locMissingMassOffOfPIDs),
			dNum2DMassBins(locNumMassBins/2), dNum2DBeamEBins(600), dNum2DMissPBins(450), dMinBeamE(0.0), dMaxBeamE(12.0), dMinMissP(0.0), dMaxMissP(9.0),
			dNumConLevBins(1000), dNumBinsPerConLevPower(18), dConLevLowest10Power(-50) {}

		DHistogramAction_MissingMassSquared(const DParticleCombo* locParticleComboWrapper, bool locUseKinFitFlag, int locMissingMassOffOfStepIndex, Particle_t locMissingMassOffOfPID, unsigned int locNumMassBins, double locMinMassSq, double locMaxMassSq, string locActionUniqueString = "") :
			DAnalysisAction(locParticleComboWrapper, "Hist_MissingMassSquared", locUseKinFitFlag, locActionUniqueString),
			dNumMassBins(locNumMassBins), dMinMass(locMinMassSq), dMaxMass(locMaxMassSq),
			dMissingMassOffOfStepIndex(locMissingMassOffOfStepIndex), dMissingMassOffOfPIDs(deque<Particle_t>(1, locMissingMassOffOfPID)),
			dNum2DMassBins(locNumMassBins/2), dNum2DBeamEBins(600), dNum2DMissPBins(450), dMinBeamE(0.0), dMaxBeamE(12.0), dMinMissP(0.0), dMaxMissP(9.0),
			dNumConLevBins(1000), dNumBinsPerConLevPower(18), dConLevLowest10Power(-50) {}

                void Reset_NewEvent(void) //reset uniqueness tracking
                {
                        dPreviouslyHistogrammed.clear();
                        dPreviouslyHistogrammed_ConLev.clear();
                }
		void Initialize(void);
		bool Perform_Action(void);

	private:
		unsigned int dNumMassBins;
		double dMinMass, dMaxMass;
		int dMissingMassOffOfStepIndex;
		deque<Particle_t> dMissingMassOffOfPIDs;

	public:
		unsigned int dNum2DMassBins, dNum2DBeamEBins, dNum2DMissPBins;
		double dMinBeamE, dMaxBeamE, dMinMissP, dMaxMissP;

		unsigned int dNumConLevBins, dNumBinsPerConLevPower;
		int dConLevLowest10Power;

	private:
		TH1I* dHist_MissingMass;
		TH2I* dHist_MissingMassVsBeamE;
		TH2I* dHist_MissingMassVsMissingP;
		TH2I* dHist_MissingMassVsConfidenceLevel;
		TH2I* dHist_MissingMassVsConfidenceLevel_LogX;
		DAnalysisUtilities dAnalysisUtilities;

		//In general: Could have multiple particles with the same PID: Use a set of Int_t's
		//In general: Multiple PIDs, so multiple sets: Contain within a map
		//Multiple combos: Contain maps within a set (easier, faster to search)
		set<map<Particle_t, set<Int_t> > > dPreviouslyHistogrammed;
                set<pair<Int_t, map<Particle_t, set<Int_t> > > > dPreviouslyHistogrammed_ConLev; //first Int_t: combo index: kinfit (probably) unique for each combo
};

class DHistogramAction_MissingEnergy : public DAnalysisAction
{
	public:
		DHistogramAction_MissingEnergy(const DParticleCombo* locParticleComboWrapper, bool locUseKinFitFlag, unsigned int locNumEnergyBins, double locMinEnergy, double locMaxEnergy, string locActionUniqueString = "") :
			DAnalysisAction(locParticleComboWrapper, "Hist_MissingEnergy", locUseKinFitFlag, locActionUniqueString),
			dNumEnergyBins(locNumEnergyBins), dMinEnergy(locMinEnergy), dMaxEnergy(locMaxEnergy), dMissingEnergyOffOfStepIndex(0), dMissingEnergyOffOfPIDs(deque<Particle_t>()),
			dNum2DEnergyBins(locNumEnergyBins/2), dNum2DBeamEBins(600), dNum2DMissPBins(450), dMinBeamE(0.0), dMaxBeamE(12.0), dMinMissP(0.0), dMaxMissP(9.0),
			dNumConLevBins(1000), dNumBinsPerConLevPower(18), dConLevLowest10Power(-50) {}

		DHistogramAction_MissingEnergy(const DParticleCombo* locParticleComboWrapper, bool locUseKinFitFlag, int locMissingEnergyOffOfStepIndex, deque<Particle_t> locMissingEnergyOffOfPIDs, unsigned int locNumEnergyBins, double locMinEnergy, double locMaxEnergy, string locActionUniqueString = "") :
			DAnalysisAction(locParticleComboWrapper, "Hist_MissingEnergy", locUseKinFitFlag, locActionUniqueString),
			dNumEnergyBins(locNumEnergyBins), dMinEnergy(locMinEnergy), dMaxEnergy(locMaxEnergy),
			dMissingEnergyOffOfStepIndex(locMissingEnergyOffOfStepIndex), dMissingEnergyOffOfPIDs(locMissingEnergyOffOfPIDs),
			dNum2DEnergyBins(locNumEnergyBins/2), dNum2DBeamEBins(600), dNum2DMissPBins(450), dMinBeamE(0.0), dMaxBeamE(12.0), dMinMissP(0.0), dMaxMissP(9.0),
			dNumConLevBins(1000), dNumBinsPerConLevPower(18), dConLevLowest10Power(-50) {}

		DHistogramAction_MissingEnergy(const DParticleCombo* locParticleComboWrapper, bool locUseKinFitFlag, int locMissingEnergyOffOfStepIndex, Particle_t locMissingEnergyOffOfPID, unsigned int locNumEnergyBins, double locMinEnergy, double locMaxEnergy, string locActionUniqueString = "") :
			DAnalysisAction(locParticleComboWrapper, "Hist_MissingEnergy", locUseKinFitFlag, locActionUniqueString),
			dNumEnergyBins(locNumEnergyBins), dMinEnergy(locMinEnergy), dMaxEnergy(locMaxEnergy),
			dMissingEnergyOffOfStepIndex(locMissingEnergyOffOfStepIndex), dMissingEnergyOffOfPIDs(deque<Particle_t>(1, locMissingEnergyOffOfPID)),
			dNum2DEnergyBins(locNumEnergyBins/2), dNum2DBeamEBins(600), dNum2DMissPBins(450), dMinBeamE(0.0), dMaxBeamE(12.0), dMinMissP(0.0), dMaxMissP(9.0),
			dNumConLevBins(1000), dNumBinsPerConLevPower(18), dConLevLowest10Power(-50) {}

		void Reset_NewEvent(void){dPreviouslyHistogrammed.clear();}; //reset uniqueness tracking
		void Initialize(void);
		bool Perform_Action(void);

	private:
		unsigned int dNumEnergyBins;
		double dMinEnergy, dMaxEnergy;
		int dMissingEnergyOffOfStepIndex;
		deque<Particle_t> dMissingEnergyOffOfPIDs;

	public:
		unsigned int dNum2DEnergyBins, dNum2DBeamEBins, dNum2DMissPBins;
		double dMinBeamE, dMaxBeamE, dMinMissP, dMaxMissP;

		unsigned int dNumConLevBins, dNumBinsPerConLevPower;
		int dConLevLowest10Power;

	private:
		TH1I* dHist_MissingEnergy;
		TH2I* dHist_MissingEnergyVsBeamE;
		TH2I* dHist_MissingEnergyVsMissingP;
		TH2I* dHist_MissingEnergyVsConfidenceLevel;
		TH2I* dHist_MissingEnergyVsConfidenceLevel_LogX;
		DAnalysisUtilities dAnalysisUtilities;

		//In general: Could have multiple particles with the same PID: Use a set of Int_t's
		//In general: Multiple PIDs, so multiple sets: Contain within a map
		//Multiple combos: Contain maps within a set (easier, faster to search)
		set<map<Particle_t, set<Int_t> > > dPreviouslyHistogrammed;
};

class DHistogramAction_2DInvariantMass : public DAnalysisAction
{
	public:
		DHistogramAction_2DInvariantMass(const DParticleCombo* locParticleComboWrapper, bool locUseKinFitFlag, size_t locStepIndex, deque<Particle_t> locXPIDs, deque<Particle_t> locYPIDs, unsigned int locNumXBins, double locMinX, double locMaxX, unsigned int locNumYBins, double locMinY, double locMaxY, string locActionUniqueString = "") :
			DAnalysisAction(locParticleComboWrapper, "Hist_2DInvariantMass", locUseKinFitFlag, locActionUniqueString),
			dStepIndex(locStepIndex), dXPIDs(locXPIDs), dYPIDs(locYPIDs), dNumXBins(locNumXBins), dNumYBins(locNumYBins),
			dMinX(locMinX), dMaxX(locMaxX), dMinY(locMinY), dMaxY(locMaxY) {}

		void Reset_NewEvent(void){dPreviouslyHistogrammed.clear();}; //reset uniqueness tracking
		void Initialize(void);
		bool Perform_Action(void);

	private:
		int dStepIndex;
		deque<Particle_t> dXPIDs, dYPIDs;
		unsigned int dNumXBins, dNumYBins;
		double dMinX, dMaxX, dMinY, dMaxY;

		DAnalysisUtilities dAnalysisUtilities;
		TH2I* dHist_2DInvaraintMass;

		set<set<map<Particle_t, set<Int_t> > > > dPreviouslyHistogrammed;
};

class DHistogramAction_Dalitz : public DAnalysisAction
{
	public:
		DHistogramAction_Dalitz(const DParticleCombo* locParticleComboWrapper, bool locUseKinFitFlag, size_t locStepIndex, deque<Particle_t> locXPIDs, deque<Particle_t> locYPIDs, unsigned int locNumXBins, double locMinX, double locMaxX, unsigned int locNumYBins, double locMinY, double locMaxY, string locActionUniqueString = "") :
			DAnalysisAction(locParticleComboWrapper, "Hist_Dalitz", locUseKinFitFlag, locActionUniqueString),
			dStepIndex(locStepIndex), dXPIDs(locXPIDs), dYPIDs(locYPIDs), dNumXBins(locNumXBins), dNumYBins(locNumYBins),
			dMinX(locMinX), dMaxX(locMaxX), dMinY(locMinY), dMaxY(locMaxY) {}

		void Reset_NewEvent(void){dPreviouslyHistogrammed.clear();}; //reset uniqueness tracking
		void Initialize(void);
		bool Perform_Action(void);

	private:
		int dStepIndex;
		deque<Particle_t> dXPIDs, dYPIDs;
		unsigned int dNumXBins, dNumYBins;
		double dMinX, dMaxX, dMinY, dMaxY;

		DAnalysisUtilities dAnalysisUtilities;
		TH2I* dHist_2DInvaraintMass;

		set<set<map<Particle_t, set<Int_t> > > > dPreviouslyHistogrammed;
};

class DHistogramAction_KinFitResults : public DAnalysisAction
{
	public:
		DHistogramAction_KinFitResults(const DParticleCombo* locParticleComboWrapper, string locActionUniqueString = "") :
			DAnalysisAction(locParticleComboWrapper, "Hist_KinFitResults", true, locActionUniqueString),
			dNumChiSqPerDFBins(1000), dNumConLevBins(1000), dNumBinsPerConLevPower(18), dConLevLowest10Power(-50), dMaxChiSqPerDF(500) {}

		void Reset_NewEvent(void){}
		void Initialize(void);
		bool Perform_Action(void);

		unsigned int dNumChiSqPerDFBins, dNumConLevBins, dNumBinsPerConLevPower;
		int dConLevLowest10Power;
		double dMaxChiSqPerDF;

	private:

		DAnalysisUtilities dAnalysisUtilities;

		TH1I* dHist_ChiSqPerDF;
		TH1I* dHist_ConfidenceLevel;
		TH1I* dHist_ConfidenceLevel_LogX;
};

class DHistogramAction_BeamEnergy : public DAnalysisAction
{
	public:
		DHistogramAction_BeamEnergy(const DParticleCombo* locParticleComboWrapper, bool locUseKinFitFlag, string locActionUniqueString = "") :
			DAnalysisAction(locParticleComboWrapper, "Hist_BeamEnergy", locUseKinFitFlag, locActionUniqueString),
			dNumBeamEnergyBins(1200), dMinBeamEnergy(0.0), dMaxBeamEnergy(12.0) {}

		void Reset_NewEvent(void){dPreviouslyHistogrammed.clear();}
		void Initialize(void);
		bool Perform_Action(void);

		unsigned int dNumBeamEnergyBins;
		double dMinBeamEnergy, dMaxBeamEnergy;

	private:

		TH1I* dHist_BeamEnergy;

		set<Int_t> dPreviouslyHistogrammed; //Int_t: Unique ID for beam particles. set: easy to use, fast to search
};

#endif // _DHistogramActions_
