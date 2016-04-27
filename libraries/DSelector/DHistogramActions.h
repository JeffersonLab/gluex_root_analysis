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

using namespace std;

class DHistogramAction_ParticleComboKinematics
{
	public:
		DHistogramAction_ParticleComboKinematics(const DParticleCombo* locParticleComboWrapper, double locTargetCenterZ, bool locUseMeasuredFlag, string locActionUniqueString = "") :
		dParticleComboWrapper(locParticleComboWrapper), dTargetCenterZ(locTargetCenterZ), dUseMeasuredFlag(locUseMeasuredFlag), dActionUniqueString(locActionUniqueString),
		dNumPBins(500), dNumThetaBins(560), dNumPhiBins(360), dNumVertexZBins(600), dNumTBins(200), dNumVertexXYBins(200), dNumBetaBins(400), dNumDeltaBetaBins(400),
		dNum2DPBins(250), dNum2DThetaBins(140), dNum2DPhiBins(180), dNumDeltaTRFBins(500), dNumPathLengthBins(750), dNumLifetimeBins(500),
		dMinT(-5.0), dMaxT(5.0), dMinP(0.0), dMaxP(10.0), dMinTheta(0.0), dMaxTheta(140.0), dMinPhi(-180.0), dMaxPhi(180.0), dMinVertexZ(0.0), dMaxVertexZ(200.0),
		dMinVertexXY(-5.0), dMaxVertexXY(5.0), dMinBeta(-0.2), dMaxBeta(1.2), dMinDeltaBeta(-1.0), dMaxDeltaBeta(1.0), dMinDeltaTRF(-10.0), dMaxDeltaTRF(10.0),
		dMaxPathLength(15), dMaxLifetime(5.0), dMaxBeamE(12.0) {}

	private:
		const DParticleCombo* dParticleComboWrapper;
		double dTargetCenterZ;
		bool dUseMeasuredFlag;
		string dActionUniqueString;

	public:
		unsigned int dNumPBins, dNumThetaBins, dNumPhiBins, dNumVertexZBins, dNumTBins, dNumVertexXYBins, dNumBetaBins, dNumDeltaBetaBins;
		unsigned int dNum2DPBins, dNum2DThetaBins, dNum2DPhiBins, dNumDeltaTRFBins, dNumPathLengthBins, dNumLifetimeBins;
		double dMinT, dMaxT, dMinP, dMaxP, dMinTheta, dMaxTheta, dMinPhi, dMaxPhi, dMinVertexZ, dMaxVertexZ, dMinVertexXY, dMaxVertexXY;
		double dMinBeta, dMaxBeta, dMinDeltaBeta, dMaxDeltaBeta, dMinDeltaTRF, dMaxDeltaTRF, dMaxPathLength, dMaxLifetime, dMaxBeamE;

		void Reset_NewEvent(void){dPreviouslyHistogrammed.clear();}; //reset uniqueness tracking
		void Initialize(void);
		bool Perform_Action(void); //if true, will reset uniqueness tracking

	private:

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

class DHistogramAction_ParticleID
{
	public:
		DHistogramAction_ParticleID(const DParticleCombo* locParticleComboWrapper, bool locUseMeasuredFlag, string locActionUniqueString = "") :
		dParticleComboWrapper(locParticleComboWrapper), dUseMeasuredFlag(locUseMeasuredFlag), dActionUniqueString(locActionUniqueString),
		dNumPBins(500), dNumThetaBins(560), dNumPhiBins(360), dNumTBins(200), dNumVertexXYBins(200), dNumBetaBins(400), dNumDeltaBetaBins(400),
		dNum2DPBins(250), dNum2DThetaBins(140), dNum2DPhiBins(180), dNumPathLengthBins(750), dNumLifetimeBins(500),
		dNumDeltaTBins(500), dNum2DdEdxBins(250), dNumEoverPBins(200),
		dMinT(-5.0), dMaxT(5.0), dMinP(0.0), dMaxP(10.0), dMinTheta(0.0), dMaxTheta(140.0), dMinPhi(-180.0), dMaxPhi(180.0), dMinVertexZ(0.0), dMaxVertexZ(200.0),
		dMinVertexXY(-5.0), dMaxVertexXY(5.0), dMinBeta(-0.2), dMaxBeta(1.2), dMinDeltaBeta(-1.0), dMaxDeltaBeta(1.0),
		dMaxPathLength(15), dMaxLifetime(5.0), dMaxBeamE(12.0),
		dMinDeltaT(-10.0), dMaxDeltaT(10.0), dMindEdx(0.0), dMaxdEdx(25.0), dMinEoverP(0.0), dMaxEoverP(4.0) {}

	private:
		const DParticleCombo* dParticleComboWrapper;
		bool dUseMeasuredFlag;
		string dActionUniqueString;

	public:
		unsigned int dNumPBins, dNumThetaBins, dNumPhiBins, dNumTBins, dNumVertexXYBins, dNumBetaBins, dNumDeltaBetaBins;
		unsigned int dNum2DPBins, dNum2DThetaBins, dNum2DPhiBins, dNumDeltaTRFBins, dNumPathLengthBins, dNumLifetimeBins;
		unsigned int dNumDeltaTBins, dNum2DdEdxBins, dNumEoverPBins;
		double dMinT, dMaxT, dMinP, dMaxP, dMinTheta, dMaxTheta, dMinPhi, dMaxPhi, dMinVertexZ, dMaxVertexZ, dMinVertexXY, dMaxVertexXY;
		double dMinBeta, dMaxBeta, dMinDeltaBeta, dMaxDeltaBeta, dMinDeltaTRF, dMaxDeltaTRF, dMaxPathLength, dMaxLifetime, dMaxBeamE;
		double dMinDeltaT, dMaxDeltaT, dMindEdx, dMaxdEdx, dMinEoverP, dMaxEoverP;

		void Reset_NewEvent(void){dPreviouslyHistogrammed.clear();}; //reset uniqueness tracking
		void Initialize(void);
		bool Perform_Action(void); //if true, will reset uniqueness tracking

	private:

		void Create_Hists(int locStepIndex, string locStepROOTName, Particle_t locPID);
		void Fill_Hists(const DKinematicData* locKinematicData, size_t locStepIndex);

		//keys are step index, PID //beam has PID Unknown
		map<size_t, map<Particle_t, TH2I*> > dHistMap_dEdxVsP_CDC;
		map<size_t, map<Particle_t, TH2I*> > dHistMap_dEdxVsP_FDC;
		map<size_t, map<Particle_t, TH2I*> > dHistMap_dEdxVsP_SC;
		map<size_t, map<Particle_t, TH2I*> > dHistMap_dEdxVsP_TOF;

		map<size_t, map<Particle_t, TH2I*> > dHistMap_BetaVsP_BCAL;
		map<size_t, map<Particle_t, TH2I*> > dHistMap_BetaVsP_TOF;
		map<size_t, map<Particle_t, TH2I*> > dHistMap_BetaVsP_FCAL;
		map<size_t, map<Particle_t, TH2I*> > dHistMap_DeltaTVsP_BCAL;	
		map<size_t, map<Particle_t, TH2I*> > dHistMap_DeltaTVsP_TOF;
		map<size_t, map<Particle_t, TH2I*> > dHistMap_DeltaTVsP_FCAL;
		map<size_t, map<Particle_t, TH2I*> > dHistMap_DeltaTVsP_SC;

		map<size_t, map<Particle_t, TH2I*> > dHistMap_EoverPVsP_BCAL;
		map<size_t, map<Particle_t, TH2I*> > dHistMap_EoverPVsTheta_BCAL;
		map<size_t, map<Particle_t, TH2I*> > dHistMap_EoverPVsP_FCAL;
		map<size_t, map<Particle_t, TH2I*> > dHistMap_EoverPVsTheta_FCAL;
		
		map<size_t, map<Particle_t, TH2I*> > dHistMap_ShowerZVsParticleZ;
		map<size_t, map<Particle_t, TH2I*> > dHistMap_ShowerTVsParticleT;

		map<size_t, map<Particle_t, set<Int_t> > > dPreviouslyHistogrammed; //step index, PID, particle indices
};

#endif // _DHistogramActions_
