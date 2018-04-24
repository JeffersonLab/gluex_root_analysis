#include "DHistogramActions.h"

void DHistogramAction_ParticleComboKinematics::Initialize(void)
{
	string locDirName, locHistName, locHistTitle, locStepName, locStepROOTName, locParticleName, locParticleROOTName;

	dTargetCenterZ = dParticleComboWrapper->Get_TargetCenter().Z();

	// CREATE & GOTO MAIN FOLDER
	CreateAndChangeTo_ActionDirectory();

	//Steps
	for(size_t loc_i = 0; loc_i < dParticleComboWrapper->Get_NumParticleComboSteps(); ++loc_i)
	{
		DParticleComboStep* locStep = dParticleComboWrapper->Get_ParticleComboStep(loc_i);

		ostringstream locStepName;
		locStepName << "Step" << loc_i << "__" << locStep->Get_StepName();
		string locStepROOTName = locStep->Get_StepROOTName();
		Particle_t locInitialPID = locStep->Get_InitialPID();
		bool locStepDirectoryCreatedFlag = false;

		//initial //reconstructed &: is gamma, or don't need measured
		bool locIsBeamFlag = (locInitialPID == Gamma);
		if((locStep->Get_InitialParticle() != NULL) && (dUseKinFitFlag || locIsBeamFlag))
		{
			CreateAndChangeTo_Directory(locStepName.str());
			locStepDirectoryCreatedFlag = true;

			locParticleName = ParticleType(locInitialPID);
			locDirName = locIsBeamFlag ? "Beam" : locParticleName;
			CreateAndChangeTo_Directory(locDirName);
			Create_Hists(loc_i, locStepROOTName, locInitialPID, locIsBeamFlag);
			gDirectory->cd("..");
		}

		//final state
		for(size_t loc_j = 0; loc_j < locStep->Get_NumFinalParticles(); ++loc_j)
		{
			if(locStep->Get_FinalParticle(loc_j) == NULL)
				continue; //not reconstructed at all

			//-2 if detected, -1 if missing, > 0 if decaying (step where it is the parent)
			int locDecayStepIndex = locStep->Get_DecayStepIndex(loc_j);
			if(!dUseKinFitFlag && (locDecayStepIndex != -2))
				continue; //not measured

			Particle_t locPID = locStep->Get_FinalPID(loc_j);
			if(dHistMap_P[loc_i].find(locPID) != dHistMap_P[loc_i].end())
				continue; //pid already done

			if(!locStepDirectoryCreatedFlag)
			{
				CreateAndChangeTo_Directory(locStepName.str());
				locStepDirectoryCreatedFlag = true;
			}

			locParticleName = ParticleType(locPID);
			CreateAndChangeTo_Directory(locParticleName);

			Create_Hists(loc_i, locStepROOTName, locPID);
			gDirectory->cd("..");
		} //end of particle loop

		// Vertex
		string locInitParticleROOTName = ParticleName_ROOT(locInitialPID);
		string locInitParticleName = ParticleType(locInitialPID);
		if((loc_i == 0) || IsDetachedVertex(locInitialPID))
		{
			if(!locStepDirectoryCreatedFlag)
			{
				CreateAndChangeTo_Directory(locStepName.str());
				locStepDirectoryCreatedFlag = true;
			}

			locHistName = "StepVertexZ";
			locHistTitle = (loc_i == 0) ? ";Production Vertex-Z (cm)" : string(";") + locInitParticleROOTName + string(" Decay Vertex-Z (cm)");
			dHistMap_StepVertexZ[loc_i] = new TH1I(locHistName.c_str(), locHistTitle.c_str(), dNumVertexZBins, dMinVertexZ, dMaxVertexZ);

			locHistName = "StepVertexYVsX";
			locHistTitle = (loc_i == 0) ? "Production Vertex" : locInitParticleROOTName + string(" Decay Vertex");
			locHistTitle += string(";Vertex-X (cm);Vertex-Y (cm)");
			dHistMap_StepVertexYVsX[loc_i] = new TH2I(locHistName.c_str(), locHistTitle.c_str(), dNumVertexXYBins, dMinVertexXY, dMaxVertexXY, dNumVertexXYBins, dMinVertexXY, dMaxVertexXY);
		}

		if((loc_i != 0) && IsDetachedVertex(locInitialPID))
		{
			if(!locStepDirectoryCreatedFlag)
			{
				CreateAndChangeTo_Directory(locStepName.str());
				locStepDirectoryCreatedFlag = true;
			}

			locHistName = locInitParticleName + string("PathLength");
			locHistTitle = string(";") + locInitParticleROOTName + string(" Path Length (cm)");
			dHistMap_DetachedPathLength[loc_i] = new TH1I(locHistName.c_str(), locHistTitle.c_str(), dNumPathLengthBins, 0.0, dMaxPathLength);

			locHistName = locInitParticleName + string("PathLengthSignificance");
			locHistTitle = string(";") + locInitParticleROOTName + string(" Path Length / Path Length #sigma");
			dHistMap_DetachedPathLengthSignificance[loc_i] = new TH1I(locHistName.c_str(), locHistTitle.c_str(), dNumPathLengthSignificanceBins, 0.0, dMaxPathLengthSignificance);

			locHistName = locInitParticleName + string("Lifetime");
			locHistTitle = string(";") + locInitParticleROOTName + string(" Lifetime (ns)");
			dHistMap_DetachedLifetime[loc_i] = new TH1I(locHistName.c_str(), locHistTitle.c_str(), dNumLifetimeBins, 0.0, dMaxLifetime);

			locHistName = locInitParticleName + string("Lifetime_RestFrame");
			locHistTitle = string(";") + locInitParticleROOTName + string(" Rest Frame Lifetime (ns)");
			dHistMap_DetachedLifetime_RestFrame[loc_i] = new TH1I(locHistName.c_str(), locHistTitle.c_str(), dNumLifetimeBins, 0.0, dMaxLifetime);
		}

		if(locStepDirectoryCreatedFlag)
			gDirectory->cd("..");
	} //end of step loop

	//Return to the base directory
	ChangeTo_BaseDirectory();
}

void DHistogramAction_ParticleComboKinematics::Create_Hists(int locStepIndex, string locStepROOTName, Particle_t locPID, bool locIsBeamFlag)
{
	string locParticleROOTName = ParticleName_ROOT(locPID);
	string locHistName, locHistTitle;
	Particle_t locFillPID = locIsBeamFlag ? Unknown : locPID;

	// Momentum
	double locMaxP = locIsBeamFlag ? dMaxBeamE : dMaxP;
	locHistName = "Momentum";
	locHistTitle = locParticleROOTName + string(", ") + locStepROOTName + string(";p (GeV/c)");
	dHistMap_P[locStepIndex][locFillPID] = new TH1I(locHistName.c_str(), locHistTitle.c_str(), dNumPBins, dMinP, locMaxP);

	if(!locIsBeamFlag)
	{
		// Theta
		locHistName = "Theta";
		locHistTitle = locParticleROOTName + string(", ") + locStepROOTName + string(";#theta#circ");
		dHistMap_Theta[locStepIndex][locFillPID] = new TH1I(locHistName.c_str(), locHistTitle.c_str(), dNumThetaBins, dMinTheta, dMaxTheta);

		// Phi
		locHistName = "Phi";
		locHistTitle = locParticleROOTName + string(", ") + locStepROOTName + string(";#phi#circ");
		dHistMap_Phi[locStepIndex][locFillPID] = new TH1I(locHistName.c_str(), locHistTitle.c_str(), dNumPhiBins, dMinPhi, dMaxPhi);

		// P Vs Theta
		locHistName = "PVsTheta";
		locHistTitle = locParticleROOTName + string(", ") + locStepROOTName + string(";#theta#circ;p (GeV/c)");
		dHistMap_PVsTheta[locStepIndex][locFillPID] = new TH2I(locHistName.c_str(), locHistTitle.c_str(), dNum2DThetaBins, dMinTheta, dMaxTheta, dNum2DPBins, dMinP, dMaxP);

		// Phi Vs Theta
		locHistName = "PhiVsTheta";
		locHistTitle = locParticleROOTName + string(", ") + locStepROOTName + string(";#theta#circ;#phi#circ");
		dHistMap_PhiVsTheta[locStepIndex][locFillPID] = new TH2I(locHistName.c_str(), locHistTitle.c_str(), dNum2DThetaBins, dMinTheta, dMaxTheta, dNum2DPhiBins, dMinPhi, dMaxPhi);
	}

	// Vertex-Z
	locHistName = "VertexZ";
	locHistTitle = locParticleROOTName + string(", ") + locStepROOTName + string(";Vertex-Z (cm)");
	dHistMap_VertexZ[locStepIndex][locFillPID] = new TH1I(locHistName.c_str(), locHistTitle.c_str(), dNumVertexZBins, dMinVertexZ, dMaxVertexZ);

	// Vertex-Y Vs Vertex-X
	locHistName = "VertexYVsX";
	locHistTitle = locParticleROOTName + string(", ") + locStepROOTName + string(";Vertex-X (cm);Vertex-Y (cm)");
	dHistMap_VertexYVsX[locStepIndex][locFillPID] = new TH2I(locHistName.c_str(), locHistTitle.c_str(), dNumVertexXYBins, dMinVertexXY, dMaxVertexXY, dNumVertexXYBins, dMinVertexXY, dMaxVertexXY);

	// Vertex-T
	locHistName = "VertexT";
	locHistTitle = locParticleROOTName + string(", ") + locStepROOTName + string(";Vertex-T (ns)");
	dHistMap_VertexT[locStepIndex][locFillPID] = new TH1I(locHistName.c_str(), locHistTitle.c_str(), dNumTBins, dMinT, dMaxT);

	if(!locIsBeamFlag)
	{
		//beta vs p
		locHistName = "BetaVsP";
		locHistTitle = locParticleROOTName + string(";p (GeV/c);#beta");
		dHistMap_BetaVsP[locStepIndex][locFillPID] = new TH2I(locHistName.c_str(), locHistTitle.c_str(), dNum2DPBins, dMinP, dMaxP, dNumBetaBins, dMinBeta, dMaxBeta);

		//delta-beta vs p
		locHistName = "DeltaBetaVsP";
		locHistTitle = locParticleROOTName + string(";p (GeV/c);#Delta#beta");
		dHistMap_DeltaBetaVsP[locStepIndex][locFillPID] = new TH2I(locHistName.c_str(), locHistTitle.c_str(), dNum2DPBins, dMinP, dMaxP, dNumDeltaBetaBins, dMinDeltaBeta, dMaxDeltaBeta);
	}

	if(locIsBeamFlag)
	{
		// Delta-T (Beam, RF)
		locHistName = "DeltaTRF";
		locHistTitle = string("Beam ") + locParticleROOTName + string(";#Deltat_{Beam - RF} (ns)");
		dBeamParticleHist_DeltaTRF = new TH1I(locHistName.c_str(), locHistTitle.c_str(), dNumDeltaTRFBins, dMinDeltaTRF, dMaxDeltaTRF);

		// Delta-T (Beam, RF) Vs Beam E
		locHistName = "DeltaTRFVsBeamE";
		locHistTitle = string("Beam ") + locParticleROOTName + string(";E (GeV);#Deltat_{Beam - RF} (ns)");
		dBeamParticleHist_DeltaTRFVsBeamE = new TH2I(locHistName.c_str(), locHistTitle.c_str(), dNum2DPBins, dMinP, dMaxP, dNumDeltaTRFBins, dMinDeltaTRF, dMaxDeltaTRF);
	}
}

bool DHistogramAction_ParticleComboKinematics::Perform_Action(void)
{
	for(size_t loc_i = 0; loc_i < dParticleComboWrapper->Get_NumParticleComboSteps(); ++loc_i)
	{
		const DParticleComboStep* locParticleComboStepWrapper = dParticleComboWrapper->Get_ParticleComboStep(loc_i);

		//initial particle
		Particle_t locInitialPID = locParticleComboStepWrapper->Get_InitialPID();
		DKinematicData* locKinematicData = locParticleComboStepWrapper->Get_InitialParticle();
		if(locKinematicData != NULL)
		{
			if(locInitialPID == Gamma)
			{
				//check if will be duplicate
				set<Int_t>& locParticleSet = dPreviouslyHistogrammed[loc_i][Unknown];
				if(locParticleSet.find(locKinematicData->Get_ID()) == locParticleSet.end())
				{
					double locRFTime = dUseKinFitFlag ? dParticleComboWrapper->Get_RFTime() : dParticleComboWrapper->Get_RFTime_Measured();
					Fill_BeamHists(locKinematicData, locRFTime);
					locParticleSet.insert(locKinematicData->Get_ID());
				}
			}
			else if(dUseKinFitFlag) //decaying particle, but reconstructed so can hist
				Fill_Hists(locKinematicData, loc_i); //derived from everything in the combo: is technically unique
		}

		//VERTEX INFORMATION
		//other than first, skipped if not detached vertex
		TLorentzVector locStepSpacetimeVertex = locParticleComboStepWrapper->Get_X4();
		if((loc_i == 0) || IsDetachedVertex(locInitialPID))
		{
			dHistMap_StepVertexZ[loc_i]->Fill(locStepSpacetimeVertex.Z());
			dHistMap_StepVertexYVsX[loc_i]->Fill(locStepSpacetimeVertex.X(), locStepSpacetimeVertex.Y());
		}

		//DETACHED VERTEX INFORMATION
		if((loc_i != 0) && IsDetachedVertex(locInitialPID))
		{
			int locFromStepIndex = locParticleComboStepWrapper->Get_InitDecayFromIndices().first;
			TLorentzVector locFromSpacetimeVertex = dParticleComboWrapper->Get_ParticleComboStep(locFromStepIndex)->Get_X4();
			TLorentzVector locDeltaSpacetime = locStepSpacetimeVertex - locFromSpacetimeVertex;

			double locPathLength = locDeltaSpacetime.Vect().Mag();
			dHistMap_DetachedPathLength[loc_i]->Fill(locPathLength);
			dHistMap_DetachedLifetime[loc_i]->Fill(locDeltaSpacetime.T());

			if(locKinematicData != NULL)
			{
				TLorentzVector locInitialP4 = locKinematicData->Get_P4();
				//below, t, x, and tau are really delta-t, delta-x, and delta-tau
				//tau = gamma*(t - beta*x/c)  //plug in: t = x/(beta*c), gamma = 1/sqrt(1 - beta^2)
				//tau = (x/(beta*c) - beta*x/c)/sqrt(1 - beta^2)  //plug in beta = p*c/E
				//tau = (x*E/(p*c^2) - p*x/E)/sqrt(1 - p^2*c^2/E^2)  //multiply num & denom by E*P, factor x/c^2 out of num
				//tau = (x/c^2)*(E^2 - p^2*c^2)/(p*sqrt(E^2 - p^2*c^2))  //plug in m^2*c^4 = E^2 - p^2*c^2
				//tau = (x/c^2)*m^2*c^4/(p*m*c^2)  //cancel c's & m's
				//tau = x*m/p
				//however, in data, p & m are in units with c = 1, so need an extra 1/c
				double locRestFrameLifetime = locPathLength*ParticleMass(locInitialPID)/(29.9792458*locInitialP4.P()); //tau
				dHistMap_DetachedLifetime_RestFrame[loc_i]->Fill(locRestFrameLifetime);
				//note that tau = hbar / Gamma, hbar = 6.582119E-22 MeV*s, Gamma = Resonance FWHM

				dHistMap_DetachedPathLengthSignificance[loc_i]->Fill(locPathLength/locKinematicData->Get_PathLengthSigma());
			}
		}

		//final particles
		for(size_t loc_j = 0; loc_j < locParticleComboStepWrapper->Get_NumFinalParticles(); ++loc_j)
		{
			DKinematicData* locKinematicData = locParticleComboStepWrapper->Get_FinalParticle(loc_j);
			if(locKinematicData == NULL)
				continue; //e.g. a decaying or missing particle whose params aren't set yet

			//check if measured if not using Kin Fit
			if(!dUseKinFitFlag && locParticleComboStepWrapper->Get_DecayStepIndex(loc_j) != -2)
				continue; //not measured

			//check if duplicate
			set<Int_t>& locParticleSet = dPreviouslyHistogrammed[loc_i][locKinematicData->Get_PID()];
			if(locParticleSet.find(locKinematicData->Get_ID()) != locParticleSet.end())
				continue;

			Fill_Hists(locKinematicData, loc_i);
			locParticleSet.insert(locKinematicData->Get_ID());
		} //end of particle loop
	} //end of step loop

	return true;
}

void DHistogramAction_ParticleComboKinematics::Fill_Hists(const DKinematicData* locKinematicData, size_t locStepIndex)
{
	Particle_t locPID = locKinematicData->Get_PID();
	TLorentzVector locP4 = dUseKinFitFlag ? locKinematicData->Get_P4() : locKinematicData->Get_P4_Measured();
	TLorentzVector locX4 = dUseKinFitFlag ? locKinematicData->Get_X4() : locKinematicData->Get_X4_Measured();

	double locPhi = locP4.Phi()*180.0/TMath::Pi();
	double locTheta = locP4.Theta()*180.0/TMath::Pi();
	double locP = locP4.P();

	double locBeta_Timing = 0.0;
	if(ParticleCharge(locPID) != 0)
	{
		const DChargedTrackHypothesis* locChargedTrackHypothesis = dynamic_cast<const DChargedTrackHypothesis*>(locKinematicData);
		if(locChargedTrackHypothesis != NULL)
			locBeta_Timing = dUseKinFitFlag ? locChargedTrackHypothesis->Get_Beta_Timing() : locChargedTrackHypothesis->Get_Beta_Timing_Measured();
	}
	else
	{
		const DNeutralParticleHypothesis* locNeutralParticleHypothesis = dynamic_cast<const DNeutralParticleHypothesis*>(locKinematicData);
		if(locNeutralParticleHypothesis != NULL)
			locBeta_Timing = dUseKinFitFlag ? locNeutralParticleHypothesis->Get_Beta_Timing() : locNeutralParticleHypothesis->Get_Beta_Timing_Measured();
	}
	double locDeltaBeta = locP4.P()/locP4.E() - locBeta_Timing;

	dHistMap_P[locStepIndex][locPID]->Fill(locP);
	dHistMap_Phi[locStepIndex][locPID]->Fill(locPhi);
	dHistMap_Theta[locStepIndex][locPID]->Fill(locTheta);
	dHistMap_PVsTheta[locStepIndex][locPID]->Fill(locTheta, locP);
	dHistMap_PhiVsTheta[locStepIndex][locPID]->Fill(locTheta, locPhi);
	dHistMap_BetaVsP[locStepIndex][locPID]->Fill(locP, locBeta_Timing);
	dHistMap_DeltaBetaVsP[locStepIndex][locPID]->Fill(locP, locDeltaBeta);
	dHistMap_VertexZ[locStepIndex][locPID]->Fill(locX4.Z());
	dHistMap_VertexYVsX[locStepIndex][locPID]->Fill(locX4.X(), locX4.Y());
	dHistMap_VertexT[locStepIndex][locPID]->Fill(locX4.T());
}

void DHistogramAction_ParticleComboKinematics::Fill_BeamHists(const DKinematicData* locKinematicData, double locRFTime)
{
	TLorentzVector locP4 = dUseKinFitFlag ? locKinematicData->Get_P4() : locKinematicData->Get_P4_Measured();
	TLorentzVector locX4 = dUseKinFitFlag ? locKinematicData->Get_X4() : locKinematicData->Get_X4_Measured();

	double locP = locP4.P();
	double locDeltaTRF = locX4.T() - (locRFTime + (locX4.Z() - dTargetCenterZ)/29.9792458);

	dHistMap_P[0][Unknown]->Fill(locP);
	dHistMap_VertexZ[0][Unknown]->Fill(locX4.Z());
	dHistMap_VertexYVsX[0][Unknown]->Fill(locX4.X(), locX4.Y());
	dHistMap_VertexT[0][Unknown]->Fill(locX4.T());
	dBeamParticleHist_DeltaTRF->Fill(locDeltaTRF);
	dBeamParticleHist_DeltaTRFVsBeamE->Fill(locP4.E(), locDeltaTRF);
}

void DHistogramAction_ParticleID::Initialize(void)
{
	string locDirName, locHistName, locHistTitle, locStepName, locStepROOTName, locParticleName, locParticleROOTName;

	dTargetCenterZ = dParticleComboWrapper->Get_TargetCenter().Z();

	// CREATE & GOTO MAIN FOLDER
	CreateAndChangeTo_ActionDirectory();

	//Steps
	for(size_t loc_i = 0; loc_i < dParticleComboWrapper->Get_NumParticleComboSteps(); ++loc_i)
	{
		DParticleComboStep* locStep = dParticleComboWrapper->Get_ParticleComboStep(loc_i);

		ostringstream locStepName;
		locStepName << "Step" << loc_i << "__" << locStep->Get_StepName();
		string locStepROOTName = locStep->Get_StepROOTName();
		bool locStepDirectoryCreatedFlag = false;

		// final state particles
		for(size_t loc_j = 0; loc_j < locStep->Get_NumFinalParticles(); ++loc_j)
		{
			if(locStep->Get_FinalParticle(loc_j) == NULL)
				continue; //not reconstructed at all

			//-2 if detected, -1 if missing, > 0 if decaying (step where it is the parent)
			int locDecayStepIndex = locStep->Get_DecayStepIndex(loc_j);
			if(locDecayStepIndex != -2)
				continue; //not measured

			Particle_t locFinalStatePID = locStep->Get_FinalPID(loc_j);
			if(dHistMap_BetaVsP_BCAL[loc_i].find(locFinalStatePID) != dHistMap_BetaVsP_BCAL[loc_i].end())
				continue; //pid already done

			if(!locStepDirectoryCreatedFlag)
			{
				CreateAndChangeTo_Directory(locStepName.str());
				locStepDirectoryCreatedFlag = true;
			}

			locParticleName = ParticleType(locFinalStatePID);
			CreateAndChangeTo_Directory(locParticleName);

			//loop over background PIDs
			if(dChargedHypoWrapper != NULL)
			{
				set<Particle_t>::iterator locIterator = dBackgroundPIDs.begin();
				for(; locIterator != dBackgroundPIDs.end(); ++locIterator)
				{
					// Make sure only particle of the same charge are in the same directory
					Particle_t locBackgroundPID = *locIterator;
					if(ParticleCharge(locBackgroundPID) != ParticleCharge(locFinalStatePID))
						continue;
					if(locBackgroundPID == locFinalStatePID)
						continue; //only want the background!

					string locBackgroundDirName = string(ParticleType(locBackgroundPID)) + string("_Background");
					CreateAndChangeTo_Directory(locBackgroundDirName);

					Create_BackgroundHists(loc_i, locFinalStatePID, locBackgroundPID, locStepROOTName);
					gDirectory->cd("..");
				}
			} //end background

			Create_Hists(loc_i, locFinalStatePID, locStepROOTName);
			gDirectory->cd("..");
		} //end of particle loop

		if(locStepDirectoryCreatedFlag)
			gDirectory->cd("..");
	} //end of step loop

	//Return to the base directory
	ChangeTo_BaseDirectory();
}

void DHistogramAction_ParticleID::Create_Hists(int locStepIndex, Particle_t locPID, string locStepROOTName)
{
	string locParticleROOTName = ParticleName_ROOT(locPID);
	string locHistName, locHistTitle;
	
	// deltaT and beta vs p
	locHistName = "DeltaTVsP_BCAL";
	locHistTitle = locParticleROOTName + string(";p (GeV/c); BCAL #Delta T (ns)");
	dHistMap_DeltaTVsP_BCAL[locStepIndex][locPID] = new TH2I(locHistName.c_str(), locHistTitle.c_str(), dNum2DPBins, dMinP, dMaxP, dNumDeltaTBins, dMinDeltaT, dMaxDeltaT);
	locHistName = "BetaVsP_BCAL";
	locHistTitle = locParticleROOTName + string(";p (GeV/c); BCAL #beta");
	dHistMap_BetaVsP_BCAL[locStepIndex][locPID] = new TH2I(locHistName.c_str(), locHistTitle.c_str(), dNum2DPBins, dMinP, dMaxP, dNumBetaBins, dMinBeta, dMaxBeta);

	locHistName = "DeltaTVsP_FCAL";
	locHistTitle = locParticleROOTName + string(";p (GeV/c); FCAL #Delta T (ns)");
	dHistMap_DeltaTVsP_FCAL[locStepIndex][locPID] = new TH2I(locHistName.c_str(), locHistTitle.c_str(), dNum2DPBins, dMinP, dMaxP, dNumDeltaTBins, dMinDeltaT, dMaxDeltaT);
	locHistName = "BetaVsP_FCAL";
	locHistTitle = locParticleROOTName + string(";p (GeV/c); FCAL #beta");
	dHistMap_BetaVsP_FCAL[locStepIndex][locPID] = new TH2I(locHistName.c_str(), locHistTitle.c_str(), dNum2DPBins, dMinP, dMaxP, dNumBetaBins, dMinBeta, dMaxBeta);

	if(ParticleCharge(locPID) != 0)
	{
		
		locHistName = "DeltaTVsP_TOF";
		locHistTitle = locParticleROOTName + string(";p (GeV/c); TOF #Delta T (ns)");
		dHistMap_DeltaTVsP_TOF[locStepIndex][locPID] = new TH2I(locHistName.c_str(), locHistTitle.c_str(), dNum2DPBins, dMinP, dMaxP, dNumDeltaTBins, dMinDeltaT, dMaxDeltaT);
		locHistName = "BetaVsP_TOF";
		locHistTitle = locParticleROOTName + string(";p (GeV/c); TOF #beta");
		dHistMap_BetaVsP_TOF[locStepIndex][locPID] = new TH2I(locHistName.c_str(), locHistTitle.c_str(), dNum2DPBins, dMinP, dMaxP, dNumBetaBins, dMinBeta, dMaxBeta);

		// dE/dx vs P
		locHistName = "dEdxVsP_CDC";
		locHistTitle = locParticleROOTName + string(";p (GeV/c); CDC dE/dx (MeV/cm) ");
		dHistMap_dEdxVsP_CDC[locStepIndex][locPID] = new TH2I(locHistName.c_str(), locHistTitle.c_str(), dNum2DPBins, dMinP, dMaxP, dNum2DdEdxBins, dMindEdx, dMaxdEdx);
	
		locHistName = "dEdxVsP_FDC";
		locHistTitle = locParticleROOTName + string(";p (GeV/c); FDC dE/dx (MeV/cm) ");
		dHistMap_dEdxVsP_FDC[locStepIndex][locPID] = new TH2I(locHistName.c_str(), locHistTitle.c_str(), dNum2DPBins, dMinP, dMaxP, dNum2DdEdxBins, dMindEdx, dMaxdEdx);
		
		locHistName = "dEdxVsP_ST";
		locHistTitle = locParticleROOTName + string(";p (GeV/c); ST dE/dx (MeV/cm) ");
		dHistMap_dEdxVsP_ST[locStepIndex][locPID] = new TH2I(locHistName.c_str(), locHistTitle.c_str(), dNum2DPBins, dMinP, dMaxP, dNum2DdEdxBins, dMindEdx, dMaxdEdx);
		
		locHistName = "dEdxVsP_TOF";
		locHistTitle = locParticleROOTName + string(";p (GeV/c); TOF dE/dx (MeV/cm) ");
		dHistMap_dEdxVsP_TOF[locStepIndex][locPID] = new TH2I(locHistName.c_str(), locHistTitle.c_str(), dNum2DPBins, dMinP, dMaxP, dNum2DdEdxBins, dMindEdx, dMaxdEdx);
		
		// E/p vs p, theta
		locHistName = "EoverPVsP_BCAL";
		locHistTitle = locParticleROOTName + string(";p (GeV/c); BCAL E/p");
		dHistMap_EoverPVsP_BCAL[locStepIndex][locPID] = new TH2I(locHistName.c_str(), locHistTitle.c_str(), dNum2DPBins, dMinP, dMaxP, dNumEoverPBins, dMinEoverP, dMaxEoverP);
		locHistName = "EoverPVsTheta_BCAL";
		locHistTitle = locParticleROOTName + string(";#theta (degrees); BCAL E/p");
		dHistMap_EoverPVsTheta_BCAL[locStepIndex][locPID] = new TH2I(locHistName.c_str(), locHistTitle.c_str(), 260, 10., 140., dNumEoverPBins, dMinEoverP, dMaxEoverP);
		
		locHistName = "EoverPVsP_FCAL";
		locHistTitle = locParticleROOTName + string(";p (GeV/c); FCAL E/p");
		dHistMap_EoverPVsP_FCAL[locStepIndex][locPID] = new TH2I(locHistName.c_str(), locHistTitle.c_str(), dNum2DPBins, dMinP, dMaxP, dNumEoverPBins, dMinEoverP, dMaxEoverP);
		locHistName = "EoverPVsTheta_FCAL";
		locHistTitle = locParticleROOTName + string(";#theta (degrees); FCAL E/p");
		dHistMap_EoverPVsTheta_FCAL[locStepIndex][locPID] = new TH2I(locHistName.c_str(), locHistTitle.c_str(), 120, 0., 12., dNumEoverPBins, dMinEoverP, dMaxEoverP);

		// PreshowerFraction BCAL vs p, theta
		locHistName = "PreshowerFractionVsP_BCAL";
		locHistTitle = locParticleROOTName + string(";p (GeV/c); BCAL Preshower Energy / Shower Energy");
		dHistMap_PreshowerFractionVsP_BCAL[locStepIndex][locPID] = new TH2I(locHistName.c_str(), locHistTitle.c_str(), dNum2DPBins, dMinP, dMaxP, dNumPreshowerFractionBins, dMinPreshowerFraction, dMaxPreshowerFraction);
		locHistName = "PreshowerFractionVsTheta_BCAL";
		locHistTitle = locParticleROOTName + string(";#theta (degrees); BCAL Preshower Energy / Shower Energy");
		dHistMap_PreshowerFractionVsTheta_BCAL[locStepIndex][locPID] = new TH2I(locHistName.c_str(), locHistTitle.c_str(), 260, 10., 140., dNumPreshowerFractionBins, dMinPreshowerFraction, dMaxPreshowerFraction);
		locHistName = "EoverPVs_PreshowerFraction_BCAL";
		locHistTitle = locParticleROOTName + string("; BCAL Preshower Energy / Shower Energy;  BCAL E/p");
		dHistMap_EoverPVs_PreshowerFraction[locStepIndex][locPID] = new TH2I(locHistName.c_str(), locHistTitle.c_str(), dNumPreshowerFractionBins, dMinPreshowerFraction, dMaxPreshowerFraction, dNumEoverPBins, dMinEoverP, dMaxEoverP);	
	}
	else
	{
		locHistName = "ShowerZVsParticleZ";
		locHistTitle = locParticleROOTName + string(";Particle Vertex Z (cm); Shower Vertex Z (cm)");
		dHistMap_ShowerZVsParticleZ[locStepIndex][locPID] = new TH2I(locHistName.c_str(), locHistTitle.c_str(), 200, 0., 200., 450, 0., 450);
		locHistName = "ShowerTVsParticleT";
		locHistTitle = locParticleROOTName + string(";Particle Vertex T (ns); Shower Vertex T (ns)");
		dHistMap_ShowerTVsParticleT[locStepIndex][locPID] = new TH2I(locHistName.c_str(), locHistTitle.c_str(), 200, -100., 100., 200, -100., 100);
	}
}

void DHistogramAction_ParticleID::Create_BackgroundHists(int locStepIndex, Particle_t locFinalStatePID, Particle_t locBackgroundPID, string locStepROOTName)
{
	string locParticleROOTName = ParticleName_ROOT(locBackgroundPID);
	string locHistName, locHistTitle;
	
	// deltaT and beta vs p
	locHistName = "DeltaTVsP_BCAL";
	locHistTitle = locParticleROOTName + string(";p (GeV/c); BCAL #Delta T (ns)");
	dBackgroundHistMap_DeltaTVsP_BCAL[locStepIndex][locFinalStatePID][locBackgroundPID] = new TH2I(locHistName.c_str(), locHistTitle.c_str(), dNum2DPBins, dMinP, dMaxP, dNumDeltaTBins, dMinDeltaT, dMaxDeltaT);

	locHistName = "DeltaTVsP_FCAL";
	locHistTitle = locParticleROOTName + string(";p (GeV/c); FCAL #Delta T (ns)");
	dBackgroundHistMap_DeltaTVsP_FCAL[locStepIndex][locFinalStatePID][locBackgroundPID] = new TH2I(locHistName.c_str(), locHistTitle.c_str(), dNum2DPBins, dMinP, dMaxP, dNumDeltaTBins, dMinDeltaT, dMaxDeltaT);

	locHistName = "DeltaTVsP_TOF";
	locHistTitle = locParticleROOTName + string(";p (GeV/c); TOF #Delta T (ns)");
	dBackgroundHistMap_DeltaTVsP_TOF[locStepIndex][locFinalStatePID][locBackgroundPID] = new TH2I(locHistName.c_str(), locHistTitle.c_str(), dNum2DPBins, dMinP, dMaxP, dNumDeltaTBins, dMinDeltaT, dMaxDeltaT);
}

bool DHistogramAction_ParticleID::Perform_Action(void)
{
	for(size_t loc_i = 0; loc_i < dParticleComboWrapper->Get_NumParticleComboSteps(); ++loc_i)
	{
		const DParticleComboStep* locParticleComboStepWrapper = dParticleComboWrapper->Get_ParticleComboStep(loc_i);

		//final particles
		for(size_t loc_j = 0; loc_j < locParticleComboStepWrapper->Get_NumFinalParticles(); ++loc_j)
		{
			DKinematicData* locKinematicData = locParticleComboStepWrapper->Get_FinalParticle(loc_j);
			if(locKinematicData == NULL)
				continue; //e.g. a decaying or missing particle whose params aren't set yet

			//-2 if detected, -1 if missing, > 0 if decaying (step where it is the parent)
			int locDecayStepIndex = locParticleComboStepWrapper->Get_DecayStepIndex(loc_j);
			if(locDecayStepIndex != -2)
				continue; //not measured

			//check if duplicate
			Particle_t locFinalStatePID = locKinematicData->Get_PID();
			set<Int_t>& locParticleSet = dPreviouslyHistogrammed[loc_i][locFinalStatePID];
			if(locParticleSet.find(locKinematicData->Get_ID()) != locParticleSet.end())
				continue;

			//fill hists
			Fill_Hists(locKinematicData, loc_i);
			locParticleSet.insert(locKinematicData->Get_ID());

			//check if should fill background hists
			if(dChargedHypoWrapper == NULL)
				continue;

			//loop over background pids
			set<Particle_t>::iterator locIterator = dBackgroundPIDs.begin();
			for(; locIterator != dBackgroundPIDs.end(); ++locIterator)
			{
				// Make sure has same charge
				Particle_t locBackgroundPID = *locIterator;
				if(ParticleCharge(locBackgroundPID) != ParticleCharge(locFinalStatePID))
					continue;
				if(locBackgroundPID == locFinalStatePID)
					continue; //only want the background!

				//Find and set array index in dChargedHypoWrapper corresponding to this background PID
				Int_t locTrackID = locKinematicData->Get_ID();
				bool locFoundFlag = false;
				for(UInt_t loc_k = 0; loc_k < dChargedHypoWrapper->Get_ArraySize(); ++loc_k)
				{
					dChargedHypoWrapper->Set_ArrayIndex(loc_k);
					if(dChargedHypoWrapper->Get_ID() != locTrackID)
						continue; //wrong track
					if(dChargedHypoWrapper->Get_PID() != locBackgroundPID)
						continue; //wrong background PID

					locFoundFlag = true;
					break;
				}
				if(!locFoundFlag)
					continue; //desired background PID not found

				//check if duplicate
				set<Int_t>& locBackgroundParticleSet = dPreviouslyHistogrammed_Background[loc_i][locFinalStatePID][locBackgroundPID];
				if(locBackgroundParticleSet.find(dChargedHypoWrapper->Get_ID()) != locBackgroundParticleSet.end())
					continue;

				//fill background hists
				Fill_BackgroundHists(loc_i, locFinalStatePID);
				locBackgroundParticleSet.insert(dChargedHypoWrapper->Get_ID());
			} //end of background pid loop
		} //end of particle loop
	} //end of step loop

	return true;
}

void DHistogramAction_ParticleID::Fill_Hists(const DKinematicData* locKinematicData, size_t locStepIndex)
{
	double locRFTime = dParticleComboWrapper->Get_RFTime_Measured();

	Particle_t locPID = locKinematicData->Get_PID();
	TLorentzVector locP4 = dUseKinFitFlag ? locKinematicData->Get_P4() : locKinematicData->Get_P4_Measured();
	TLorentzVector locX4 = dUseKinFitFlag ? locKinematicData->Get_X4() : locKinematicData->Get_X4_Measured();

	double locTheta = locP4.Theta()*180.0/TMath::Pi();
	double locP = locP4.P();

	double locBeta_Timing = 0.0;
	if(ParticleCharge(locPID) != 0)
	{
		const DChargedTrackHypothesis* locChargedTrackHypothesis = dynamic_cast<const DChargedTrackHypothesis*>(locKinematicData);
		if(locChargedTrackHypothesis != NULL) {
			locBeta_Timing = dUseKinFitFlag ? locChargedTrackHypothesis->Get_Beta_Timing() : locChargedTrackHypothesis->Get_Beta_Timing_Measured();
			double locPropagatedRFTime = locRFTime + (locX4.Z() - dTargetCenterZ)/29.9792458;
			double locDeltaT = locX4.T() - locPropagatedRFTime;

			if(locChargedTrackHypothesis->Get_Detector_System_Timing() == SYS_BCAL) {
				dHistMap_BetaVsP_BCAL[locStepIndex][locPID]->Fill(locP, locBeta_Timing);
				dHistMap_DeltaTVsP_BCAL[locStepIndex][locPID]->Fill(locP, locDeltaT);
			}
			else if(locChargedTrackHypothesis->Get_Detector_System_Timing() == SYS_TOF) {
				dHistMap_BetaVsP_TOF[locStepIndex][locPID]->Fill(locP, locBeta_Timing);
				dHistMap_DeltaTVsP_TOF[locStepIndex][locPID]->Fill(locP, locDeltaT);
			}
			else if(locChargedTrackHypothesis->Get_Detector_System_Timing() == SYS_FCAL) {
				dHistMap_BetaVsP_FCAL[locStepIndex][locPID]->Fill(locP, locBeta_Timing);
				dHistMap_DeltaTVsP_FCAL[locStepIndex][locPID]->Fill(locP, locDeltaT);
			}
			
			// dE/dx vs p
			if(locChargedTrackHypothesis->Get_dEdx_CDC() > 0.)
				dHistMap_dEdxVsP_CDC[locStepIndex][locPID]->Fill(locP,locChargedTrackHypothesis->Get_dEdx_CDC()*1e6);
			if(locChargedTrackHypothesis->Get_dEdx_FDC() > 0.) 
				dHistMap_dEdxVsP_FDC[locStepIndex][locPID]->Fill(locP,locChargedTrackHypothesis->Get_dEdx_FDC()*1e6);
			if(locChargedTrackHypothesis->Get_dEdx_ST() > 0.) 
				dHistMap_dEdxVsP_ST[locStepIndex][locPID]->Fill(locP,locChargedTrackHypothesis->Get_dEdx_ST()*1e3);
			if(locChargedTrackHypothesis->Get_dEdx_TOF() > 0.) 
				dHistMap_dEdxVsP_TOF[locStepIndex][locPID]->Fill(locP,locChargedTrackHypothesis->Get_dEdx_TOF()*1e3);

			// E over P
			if(locChargedTrackHypothesis->Get_Energy_BCAL() > 0.) {
				double locShowerE = locChargedTrackHypothesis->Get_Energy_BCAL();
				double locEoverP = locShowerE/locP;
				dHistMap_EoverPVsP_BCAL[locStepIndex][locPID]->Fill(locP, locEoverP);
				dHistMap_EoverPVsTheta_BCAL[locStepIndex][locPID]->Fill(locTheta, locEoverP);

				double locPreshowerE = locChargedTrackHypothesis->Get_Energy_BCALPreshower();
                                double locPreshowerFraction = locPreshowerE/locShowerE*sin(locChargedTrackHypothesis->Get_P4().Theta());
                                dHistMap_PreshowerFractionVsP_BCAL[locStepIndex][locPID]->Fill(locP, locPreshowerFraction);
                                dHistMap_PreshowerFractionVsTheta_BCAL[locStepIndex][locPID]->Fill(locTheta, locPreshowerFraction);
                                dHistMap_EoverPVs_PreshowerFraction[locStepIndex][locPID]->Fill(locPreshowerFraction, locEoverP);
			}
			if(locChargedTrackHypothesis->Get_Energy_FCAL() > 0.) {
				double locEoverP = locChargedTrackHypothesis->Get_Energy_FCAL()/locP;
				dHistMap_EoverPVsP_FCAL[locStepIndex][locPID]->Fill(locP, locEoverP);
				dHistMap_EoverPVsTheta_FCAL[locStepIndex][locPID]->Fill(locTheta, locEoverP);
			}
		}
	}
	else
	{
		const DNeutralParticleHypothesis* locNeutralParticleHypothesis = dynamic_cast<const DNeutralParticleHypothesis*>(locKinematicData);
		if(locNeutralParticleHypothesis != NULL) {
			locBeta_Timing = dUseKinFitFlag ? locNeutralParticleHypothesis->Get_Beta_Timing() : locNeutralParticleHypothesis->Get_Beta_Timing_Measured();
			
			//TLorentzVector locX4_Neutral = dUseKinFitFlag ? locKinematicData->Get_X4() : locNeutralParticleHypothesis->Get_X4_Measured();
			TLorentzVector locX4_Neutral = locNeutralParticleHypothesis->Get_X4_Measured();
			double locPropagatedRFTime = locRFTime + (locX4.Z() - dTargetCenterZ)/29.9792458;
			double locDeltaT = locX4_Neutral.T() - locPropagatedRFTime;

			TLorentzVector locX4_Shower = locNeutralParticleHypothesis->Get_X4_Shower();
			//TLorentzVector locX4_Measured = locNeutralParticleHypothesis->Get_X4_Measured();
			dHistMap_ShowerZVsParticleZ[locStepIndex][locPID]->Fill(locX4_Neutral.Z(), locX4_Shower.Z());
			dHistMap_ShowerTVsParticleT[locStepIndex][locPID]->Fill(locX4_Neutral.T(), locX4_Shower.T());
			
			if(locNeutralParticleHypothesis->Get_Energy_BCAL() > 0.) {
				dHistMap_BetaVsP_BCAL[locStepIndex][locPID]->Fill(locP, locBeta_Timing);
				dHistMap_DeltaTVsP_BCAL[locStepIndex][locPID]->Fill(locP, locDeltaT);
			}
			else if(locNeutralParticleHypothesis->Get_Energy_FCAL() > 0.) {
				dHistMap_BetaVsP_FCAL[locStepIndex][locPID]->Fill(locP, locBeta_Timing);
				dHistMap_DeltaTVsP_FCAL[locStepIndex][locPID]->Fill(locP, locDeltaT);
			}
		}
	}
}

void DHistogramAction_ParticleID::Fill_BackgroundHists(size_t locStepIndex, Particle_t locFinalStatePID)
{
	Particle_t locPID = dChargedHypoWrapper->Get_PID();
	TLorentzVector locP4 = dChargedHypoWrapper->Get_P4_Measured();
	TLorentzVector locX4 = dChargedHypoWrapper->Get_X4_Measured();
	double locP = locP4.P();

	double locRFTime = dParticleComboWrapper->Get_RFTime_Measured();
	double locPropagatedRFTime = locRFTime + (locX4.Z() - dTargetCenterZ)/29.9792458;
	double locDeltaT = locX4.T() - locPropagatedRFTime;

	if(dChargedHypoWrapper->Get_Detector_System_Timing() == SYS_BCAL)
		dBackgroundHistMap_DeltaTVsP_BCAL[locStepIndex][locFinalStatePID][locPID]->Fill(locP, locDeltaT);
	else if(dChargedHypoWrapper->Get_Detector_System_Timing() == SYS_TOF)
		dBackgroundHistMap_DeltaTVsP_TOF[locStepIndex][locFinalStatePID][locPID]->Fill(locP, locDeltaT);
	else if(dChargedHypoWrapper->Get_Detector_System_Timing() == SYS_FCAL)
		dBackgroundHistMap_DeltaTVsP_FCAL[locStepIndex][locFinalStatePID][locPID]->Fill(locP, locDeltaT);
}

void DHistogramAction_InvariantMass::Initialize(void)
{
	string locHistName, locHistTitle;
	double locMassPerBin = 1000.0*(dMaxMass - dMinMass)/((double)dNumMassBins);

	string locParticleNamesForHist = "";
	if(dInitialPID != Unknown)
		locParticleNamesForHist = dParticleComboWrapper->Get_DecayChainFinalParticlesROOTNames(dInitialPID, dUseKinFitFlag);
	else
	{
		for(size_t loc_i = 0; loc_i < dToIncludePIDs.size(); ++loc_i)
			locParticleNamesForHist += ParticleName_ROOT(dToIncludePIDs[loc_i]);
	}

	// CREATE & GOTO MAIN FOLDER
	CreateAndChangeTo_ActionDirectory();

	locHistName = "InvariantMass";
	ostringstream locStream;
	locStream << locMassPerBin;
	locHistTitle = string(";") + locParticleNamesForHist + string(" Invariant Mass (GeV/c^{2});# Combos / ") + locStream.str() + string(" MeV/c^{2}");
	dHist_InvariantMass = new TH1I(locHistName.c_str(), locHistTitle.c_str(), dNumMassBins, dMinMass, dMaxMass);

	locHistName = "InvariantMassVsBeamE";
	locHistTitle = string(";Beam Energy (GeV);") + locParticleNamesForHist + string(" Invariant Mass (GeV/c^{2})");
	dHist_InvariantMassVsBeamE = new TH2I(locHistName.c_str(), locHistTitle.c_str(), dNum2DBeamEBins, dMinBeamE, dMaxBeamE, dNum2DMassBins, dMinMass, dMaxMass);

	locHistName = "InvariantMassVsConfidenceLevel";
	locHistTitle = string(";Kinematic Fit Confidence Level;") + locParticleNamesForHist + string(" Invariant Mass (GeV/c^{2})");
	dHist_InvariantMassVsConfidenceLevel = new TH2I(locHistName.c_str(), locHistTitle.c_str(), dNumConLevBins, 0.0, 1.0, dNum2DMassBins, dMinMass, dMaxMass);

	locHistName = "InvariantMassVsConfidenceLevel_LogX";
	int locNumBins = 0;
	double* locConLevLogBinning = dAnalysisUtilities.Generate_LogBinning(dConLevLowest10Power, 0, dNumBinsPerConLevPower, locNumBins);
	if(locConLevLogBinning != NULL)
		dHist_InvariantMassVsConfidenceLevel_LogX = new TH2I(locHistName.c_str(), locHistTitle.c_str(), locNumBins, locConLevLogBinning, dNum2DMassBins, dMinMass, dMaxMass);
	else
		dHist_InvariantMassVsConfidenceLevel_LogX = NULL;

	//Return to the base directory
	ChangeTo_BaseDirectory();
}

bool DHistogramAction_InvariantMass::Perform_Action(void)
{
	double locConfidenceLevel = dParticleComboWrapper->Get_ConfidenceLevel_KinFit( "" );

	for(size_t loc_i = 0; loc_i < dParticleComboWrapper->Get_NumParticleComboSteps(); ++loc_i)
	{
		const DParticleComboStep* locParticleComboStepWrapper = dParticleComboWrapper->Get_ParticleComboStep(loc_i);
		if((dInitialPID != Unknown) && (locParticleComboStepWrapper->Get_InitialPID() != dInitialPID))
			continue;
		if((dStepIndex != -1) && (int(loc_i) != dStepIndex))
			continue;

		DKinematicData* locBeamParticle = dParticleComboWrapper->Get_ParticleComboStep(0)->Get_InitialParticle();
		Int_t locBeamID = locBeamParticle->Get_ID();
		double locBeamEnergy = 0.0;
		if(dUseKinFitFlag)
		  locBeamEnergy = locBeamParticle->Get_P4().E();
		else
		  locBeamEnergy = locBeamParticle->Get_P4_Measured().E();
		
		//build all possible combinations of the included pids
		set<set<size_t> > locIndexCombos = dAnalysisUtilities.Build_IndexCombos(locParticleComboStepWrapper, dToIncludePIDs);

		//loop over them
		set<set<size_t> >::iterator locComboIterator = locIndexCombos.begin();
		for(; locComboIterator != locIndexCombos.end(); ++locComboIterator)
		{
			map<unsigned int, set<Int_t> > locSourceObjects;
			TLorentzVector locFinalStateP4 = dAnalysisUtilities.Calc_FinalStateP4(dParticleComboWrapper, loc_i, *locComboIterator, locSourceObjects, dUseKinFitFlag);

			if(dPreviouslyHistogrammed.find(locSourceObjects) == dPreviouslyHistogrammed.end())
			{
				dPreviouslyHistogrammed.insert(locSourceObjects);
				dHist_InvariantMass->Fill(locFinalStateP4.M());
			}

			pair<Int_t, map<unsigned int, set<Int_t> > > locSourceObjects_BeamE(locBeamID, locSourceObjects);
			if(dPreviouslyHistogrammed_BeamE.find(locSourceObjects_BeamE) == dPreviouslyHistogrammed_BeamE.end())
			{
			  dPreviouslyHistogrammed_BeamE.insert(locSourceObjects_BeamE);
			  dHist_InvariantMassVsBeamE->Fill(locBeamEnergy, locFinalStateP4.M());
			}

			pair<Int_t, map<unsigned int, set<Int_t> > > locSourceObjects_ConLev(dParticleComboWrapper->Get_ComboIndex(), locSourceObjects);
			if(dPreviouslyHistogrammed_ConLev.find(locSourceObjects_ConLev) == dPreviouslyHistogrammed_ConLev.end())
			{
				dPreviouslyHistogrammed_ConLev.insert(locSourceObjects_ConLev);
				dHist_InvariantMassVsConfidenceLevel->Fill(locConfidenceLevel, locFinalStateP4.M());
				dHist_InvariantMassVsConfidenceLevel_LogX->Fill(locConfidenceLevel, locFinalStateP4.M());
			}
		}
		//don't break: e.g. if multiple pi0's, histogram invariant mass of each one
	}

	return true;
}

void DHistogramAction_MissingMass::Initialize(void)
{
	double locMassPerBin = 1000.0*(dMaxMass - dMinMass)/((double)dNumMassBins);
	string locInitialParticlesROOTName = dParticleComboWrapper->Get_InitialParticlesROOTName();
	string locFinalParticlesROOTName = dParticleComboWrapper->Get_DecayChainFinalParticlesROOTNames(0, dMissingMassOffOfStepIndex, dMissingMassOffOfPIDs, dUseKinFitFlag, true);

	dTargetCenterZ = dParticleComboWrapper->Get_TargetCenter().Z();
	dRunNumber = dParticleComboWrapper->Get_RunNumber();
	dBeamBunchPeriod = dAnalysisUtilities.Get_BeamBunchPeriod(dRunNumber);

	// CREATE & GOTO MAIN FOLDER
	CreateAndChangeTo_ActionDirectory();

	string locHistName = "MissingMass";
	ostringstream locStream;
	locStream << locMassPerBin;
	string locHistTitle = string(";") + locInitialParticlesROOTName + string("#rightarrow") + locFinalParticlesROOTName + string(" Missing Mass (GeV/c^{2});# Combos / ") + locStream.str() + string(" MeV/c^{2}");
	dHist_MissingMass = new TH1I(locHistName.c_str(), locHistTitle.c_str(), dNumMassBins, dMinMass, dMaxMass);

	locHistName = "MissingMassVsBeamE";
	locMassPerBin *= ((double)dNumMassBins)/((double)dNum2DMassBins);
	locStream.str("");
	locStream << locMassPerBin;
	locHistTitle = string(";Beam Energy (GeV);") + locInitialParticlesROOTName + string("#rightarrow") + locFinalParticlesROOTName + string(" Missing Mass (GeV/c^{2})");
	dHist_MissingMassVsBeamE = new TH2I(locHistName.c_str(), locHistTitle.c_str(), dNum2DBeamEBins, dMinBeamE, dMaxBeamE, dNum2DMassBins, dMinMass, dMaxMass);

	locHistName = "MissingMassVsMissingP";
	locStream.str("");
	locStream << locMassPerBin;
	locHistTitle = string(";Missing P (GeV/c);") + locInitialParticlesROOTName + string("#rightarrow") + locFinalParticlesROOTName + string(" Missing Mass (GeV/c^{2})");
	dHist_MissingMassVsMissingP = new TH2I(locHistName.c_str(), locHistTitle.c_str(), dNum2DMissPBins, dMinMissP, dMaxMissP, dNum2DMassBins, dMinMass, dMaxMass);

	locHistName = "MissingMassVsConfidenceLevel";
	locHistTitle = string(";Kinematic Fit Confidence Level;") + locInitialParticlesROOTName + string("#rightarrow") + locFinalParticlesROOTName + string(" Missing Mass (GeV/c^{2})");
	dHist_MissingMassVsConfidenceLevel = new TH2I(locHistName.c_str(), locHistTitle.c_str(), dNumConLevBins, 0.0, 1.0, dNum2DMassBins, dMinMass, dMaxMass);

	locHistName = "MissingMassVsConfidenceLevel_LogX";
	int locNumBins = 0;
	double* locConLevLogBinning = dAnalysisUtilities.Generate_LogBinning(dConLevLowest10Power, 0, dNumBinsPerConLevPower, locNumBins);
	if(locConLevLogBinning != NULL)
		dHist_MissingMassVsConfidenceLevel_LogX = new TH2I(locHistName.c_str(), locHistTitle.c_str(), locNumBins, locConLevLogBinning, dNum2DMassBins, dMinMass, dMaxMass);
	else
		dHist_MissingMassVsConfidenceLevel_LogX = NULL;

	//Return to the base directory
	ChangeTo_BaseDirectory();
}

bool DHistogramAction_MissingMass::Perform_Action(void)
{
	//beam bunch check, if desired
	if(dBeamBunchRange.first <= dBeamBunchRange.second)
	{
		//get beam/rf delta-t
		DKinematicData* locKinematicData = dParticleComboWrapper->Get_ParticleComboStep(0)->Get_InitialParticle();
		TLorentzVector locBeamX4 = dUseKinFitFlag ? locKinematicData->Get_X4() : locKinematicData->Get_X4_Measured();
		double locRFTime = dUseKinFitFlag ? dParticleComboWrapper->Get_RFTime() : dParticleComboWrapper->Get_RFTime_Measured();
		double locBeamRFDeltaT = locBeamX4.T() - (locRFTime + (locBeamX4.Z() - dTargetCenterZ)/29.9792458);

		//delta-t cut range
		double locMinDeltaT = dBeamBunchPeriod*(double(dBeamBunchRange.first) - 0.5);
		double locMaxDeltaT = dBeamBunchPeriod*(double(dBeamBunchRange.first) + 0.5);

		//check if outside of range
		if((fabs(locBeamRFDeltaT) < locMinDeltaT) || (fabs(locBeamRFDeltaT) > locMaxDeltaT))
			return true;
		if(std::isnan(locRFTime))
			return true;
	}

	double locConfidenceLevel = dParticleComboWrapper->Get_ConfidenceLevel_KinFit( "" );

	DKinematicData* locBeamParticle = dParticleComboWrapper->Get_ParticleComboStep(0)->Get_InitialParticle();
	double locBeamEnergy = 0.0;
	if(dUseKinFitFlag)
		locBeamEnergy = locBeamParticle->Get_P4().E();
	else
		locBeamEnergy = locBeamParticle->Get_P4_Measured().E();

	//build all possible combinations of the included pids
	const DParticleComboStep* locParticleComboStepWrapper = dParticleComboWrapper->Get_ParticleComboStep(dMissingMassOffOfStepIndex);
	set<set<size_t> > locIndexCombos = dAnalysisUtilities.Build_IndexCombos(locParticleComboStepWrapper, dMissingMassOffOfPIDs);

	//loop over them
	set<set<size_t> >::iterator locComboIterator = locIndexCombos.begin();
	for(; locComboIterator != locIndexCombos.end(); ++locComboIterator)
	{
		map<unsigned int, set<Int_t> > locSourceObjects;
		TLorentzVector locMissingP4 = dAnalysisUtilities.Calc_MissingP4(dParticleComboWrapper, 0, dMissingMassOffOfStepIndex, *locComboIterator, locSourceObjects, dUseKinFitFlag);
		pair<Int_t, map<unsigned int, set<Int_t> > > locSourceObjects_ConLev(dParticleComboWrapper->Get_ComboIndex(), locSourceObjects);

		if(dPreviouslyHistogrammed.find(locSourceObjects) == dPreviouslyHistogrammed.end())
		{
			dPreviouslyHistogrammed.insert(locSourceObjects);
			dHist_MissingMass->Fill(locMissingP4.M());
			dHist_MissingMassVsBeamE->Fill(locBeamEnergy, locMissingP4.M());
			dHist_MissingMassVsMissingP->Fill(locMissingP4.P(), locMissingP4.M());
		}
		if(dPreviouslyHistogrammed_ConLev.find(locSourceObjects_ConLev) == dPreviouslyHistogrammed_ConLev.end())
		{
			dPreviouslyHistogrammed_ConLev.insert(locSourceObjects_ConLev);
			dHist_MissingMassVsConfidenceLevel->Fill(locConfidenceLevel, locMissingP4.M());
			dHist_MissingMassVsConfidenceLevel_LogX->Fill(locConfidenceLevel, locMissingP4.M());
		}
	}

	return true;
}

void DHistogramAction_MissingMassSquared::Initialize(void)
{
	double locMassSqPerBin = 1000.0*1000.0*(dMaxMass - dMinMass)/((double)dNumMassBins);
	string locInitialParticlesROOTName = dParticleComboWrapper->Get_InitialParticlesROOTName();
	string locFinalParticlesROOTName = dParticleComboWrapper->Get_DecayChainFinalParticlesROOTNames(0, dMissingMassOffOfStepIndex, dMissingMassOffOfPIDs, dUseKinFitFlag, true);

	dTargetCenterZ = dParticleComboWrapper->Get_TargetCenter().Z();
	dRunNumber = dParticleComboWrapper->Get_RunNumber();
	dBeamBunchPeriod = dAnalysisUtilities.Get_BeamBunchPeriod(dRunNumber);

	// CREATE & GOTO MAIN FOLDER
	CreateAndChangeTo_ActionDirectory();

	string locHistName = "MissingMassSquared";
	ostringstream locStream;
	locStream << locMassSqPerBin;
	string locHistTitle = string(";") + locInitialParticlesROOTName + string("#rightarrow") + locFinalParticlesROOTName + string(" Missing Mass Squared (GeV/c^{2})^{2};# Combos / ") + locStream.str() + string(" (MeV/c^{2})^{2}");
	dHist_MissingMass = new TH1I(locHistName.c_str(), locHistTitle.c_str(), dNumMassBins, dMinMass, dMaxMass);

	locHistName = "MissingMassSquaredVsBeamE";
	locMassSqPerBin *= ((double)dNumMassBins)/((double)dNum2DMassBins);
	locStream.str();
	locStream << locMassSqPerBin;
	locHistTitle = string(";Beam Energy (GeV);") + locInitialParticlesROOTName + string("#rightarrow") + locFinalParticlesROOTName + string(" Missing Mass Squared (GeV/c^{2})^{2};");
	dHist_MissingMassVsBeamE = new TH2I(locHistName.c_str(), locHistTitle.c_str(), dNum2DBeamEBins, dMinBeamE, dMaxBeamE, dNum2DMassBins, dMinMass, dMaxMass);

	locHistName = "MissingMassSquaredVsMissingP";
	locStream.str("");
	locStream << locMassSqPerBin;
	locHistTitle = string(";Missing P (GeV/c);") + locInitialParticlesROOTName + string("#rightarrow") + locFinalParticlesROOTName + string(" Missing Mass Squared (GeV/c^{2})^{2}");
	dHist_MissingMassVsMissingP = new TH2I(locHistName.c_str(), locHistTitle.c_str(), dNum2DMissPBins, dMinMissP, dMaxMissP, dNum2DMassBins, dMinMass, dMaxMass);

	locHistName = "MissingMassSquaredVsConfidenceLevel";
	locHistTitle = string(";Kinematic Fit Confidence Level;") + locInitialParticlesROOTName + string("#rightarrow") + locFinalParticlesROOTName + string(" Missing Mass Squared (GeV/c^{2})^{2}");
	dHist_MissingMassVsConfidenceLevel = new TH2I(locHistName.c_str(), locHistTitle.c_str(), dNumConLevBins, 0.0, 1.0, dNum2DMassBins, dMinMass, dMaxMass);

	locHistName = "MissingMassSquaredVsConfidenceLevel_LogX";
	int locNumBins = 0;
	double* locConLevLogBinning = dAnalysisUtilities.Generate_LogBinning(dConLevLowest10Power, 0, dNumBinsPerConLevPower, locNumBins);
	if(locConLevLogBinning != NULL)
		dHist_MissingMassVsConfidenceLevel_LogX = new TH2I(locHistName.c_str(), locHistTitle.c_str(), locNumBins, locConLevLogBinning, dNum2DMassBins, dMinMass, dMaxMass);
	else
		dHist_MissingMassVsConfidenceLevel_LogX = NULL;

	//Return to the base directory
	ChangeTo_BaseDirectory();
}

bool DHistogramAction_MissingMassSquared::Perform_Action(void)
{
	//beam bunch check, if desired
	if(dBeamBunchRange.first <= dBeamBunchRange.second)
	{
		//get beam/rf delta-t
		DKinematicData* locKinematicData = dParticleComboWrapper->Get_ParticleComboStep(0)->Get_InitialParticle();
		TLorentzVector locBeamX4 = dUseKinFitFlag ? locKinematicData->Get_X4() : locKinematicData->Get_X4_Measured();
		double locRFTime = dUseKinFitFlag ? dParticleComboWrapper->Get_RFTime() : dParticleComboWrapper->Get_RFTime_Measured();
		double locBeamRFDeltaT = locBeamX4.T() - (locRFTime + (locBeamX4.Z() - dTargetCenterZ)/29.9792458);

		//delta-t cut range
		double locMinDeltaT = dBeamBunchPeriod*(double(dBeamBunchRange.first) - 0.5);
		double locMaxDeltaT = dBeamBunchPeriod*(double(dBeamBunchRange.first) + 0.5);

		//check if outside of range
		if((fabs(locBeamRFDeltaT) < locMinDeltaT) || (fabs(locBeamRFDeltaT) > locMaxDeltaT))
			return true;
		if(std::isnan(locRFTime))
			return true;
	}

	double locConfidenceLevel = dParticleComboWrapper->Get_ConfidenceLevel_KinFit( "" );

	DKinematicData* locBeamParticle = dParticleComboWrapper->Get_ParticleComboStep(0)->Get_InitialParticle();
	double locBeamEnergy = 0.0;
	if(dUseKinFitFlag)
		locBeamEnergy = locBeamParticle->Get_P4().E();
	else
		locBeamEnergy = locBeamParticle->Get_P4_Measured().E();

	//build all possible combinations of the included pids
	const DParticleComboStep* locParticleComboStepWrapper = dParticleComboWrapper->Get_ParticleComboStep(dMissingMassOffOfStepIndex);
	set<set<size_t> > locIndexCombos = dAnalysisUtilities.Build_IndexCombos(locParticleComboStepWrapper, dMissingMassOffOfPIDs);

	//loop over them
	set<set<size_t> >::iterator locComboIterator = locIndexCombos.begin();
	for(; locComboIterator != locIndexCombos.end(); ++locComboIterator)
	{
		map<unsigned int, set<Int_t> > locSourceObjects;
		TLorentzVector locMissingP4 = dAnalysisUtilities.Calc_MissingP4(dParticleComboWrapper, 0, dMissingMassOffOfStepIndex, *locComboIterator, locSourceObjects, dUseKinFitFlag);
		pair<Int_t, map<unsigned int, set<Int_t> > > locSourceObjects_ConLev(dParticleComboWrapper->Get_ComboIndex(), locSourceObjects);

		if(dPreviouslyHistogrammed.find(locSourceObjects) == dPreviouslyHistogrammed.end())
		{
			dPreviouslyHistogrammed.insert(locSourceObjects);
			dHist_MissingMass->Fill(locMissingP4.M2());
			dHist_MissingMassVsBeamE->Fill(locBeamEnergy, locMissingP4.M2());
			dHist_MissingMassVsMissingP->Fill(locMissingP4.P(), locMissingP4.M2());
		}
		if(dPreviouslyHistogrammed_ConLev.find(locSourceObjects_ConLev) == dPreviouslyHistogrammed_ConLev.end())
		{
			dPreviouslyHistogrammed_ConLev.insert(locSourceObjects_ConLev);
			dHist_MissingMassVsConfidenceLevel->Fill(locConfidenceLevel, locMissingP4.M2());
			dHist_MissingMassVsConfidenceLevel_LogX->Fill(locConfidenceLevel, locMissingP4.M2());
		}
	}

	return true;
}

void DHistogramAction_MissingP4::Initialize(void)
{
	string locInitialParticlesROOTName = dParticleComboWrapper->Get_InitialParticlesROOTName();
	string locFinalParticlesROOTName = dParticleComboWrapper->Get_DecayChainFinalParticlesROOTNames(0, -1, deque<Particle_t>(), dUseKinFitFlag, !dUseKinFitFlag);
	string locReactionString = locInitialParticlesROOTName + string("#rightarrow") + locFinalParticlesROOTName;
	string locHistName, locHistTitle;

	dTargetCenterZ = dParticleComboWrapper->Get_TargetCenter().Z();
	dRunNumber = dParticleComboWrapper->Get_RunNumber();
	dBeamBunchPeriod = dAnalysisUtilities.Get_BeamBunchPeriod(dRunNumber);

	// CREATE & GOTO MAIN FOLDER
	CreateAndChangeTo_ActionDirectory();

	//1D: missing px, py, pz, pt, E
	locHistName = "MissingE";
	locHistTitle = string(";") + locReactionString + string(" Missing Energy (GeV);# Combos");
	dHist_MissingE = new TH1I(locHistName.c_str(), locHistTitle.c_str(), dNumMissingEPzBins, dMinMissingEPz, dMaxMissingEPz);

	locHistName = "MissingPx";
	locHistTitle = string(";") + locReactionString + string(" Missing P_{x} (GeV/c);# Combos");
	dHist_MissingPx = new TH1I(locHistName.c_str(), locHistTitle.c_str(), dNumMissingPxyBins, -1.0*dMaxMissingPxy, dMaxMissingPxy);

	locHistName = "MissingPy";
	locHistTitle = string(";") + locReactionString + string(" Missing P_{y} (GeV/c);# Combos");
	dHist_MissingPy = new TH1I(locHistName.c_str(), locHistTitle.c_str(), dNumMissingPxyBins, -1.0*dMaxMissingPxy, dMaxMissingPxy);

	locHistName = "MissingPz";
	locHistTitle = string(";") + locReactionString + string(" Missing P_{z} (GeV/c);# Combos");
	dHist_MissingPz = new TH1I(locHistName.c_str(), locHistTitle.c_str(), dNumMissingEPzBins, dMinMissingEPz, dMaxMissingEPz);

	locHistName = "MissingPt";
	locHistTitle = string(";") + locReactionString + string(" Missing P_{Transverse} (GeV/c);# Combos");
	dHist_MissingPt = new TH1I(locHistName.c_str(), locHistTitle.c_str(), dNumMissingPtBins, 0.0, dMaxMissingPt);

	//2D vs beam E: pz, pt, E
	locHistName = "MissingEVsBeamE";
	locHistTitle = string(";Beam Energy (GeV);") + locReactionString + string(" Missing Energy (GeV)");
	dHist_MissingEVsBeamE = new TH2I(locHistName.c_str(), locHistTitle.c_str(), dNum2DBeamEBins, dMinBeamE, dMaxBeamE, dNum2DMissingEPzBins, dMinMissingEPz, dMaxMissingEPz);

	locHistName = "MissingPzVsBeamE";
	locHistTitle = string(";Beam Energy (GeV);") + locReactionString + string(" Missing P_{z} (GeV/c)");
	dHist_MissingPzVsBeamE = new TH2I(locHistName.c_str(), locHistTitle.c_str(), dNum2DBeamEBins, dMinBeamE, dMaxBeamE, dNum2DMissingEPzBins, dMinMissingEPz, dMaxMissingEPz);

	locHistName = "MissingPtVsBeamE";
	locHistTitle = string(";Beam Energy (GeV);") + locReactionString + string(" Missing P_{Transverse} (GeV/c)");
	dHist_MissingPtVsBeamE = new TH2I(locHistName.c_str(), locHistTitle.c_str(), dNum2DBeamEBins, dMinBeamE, dMaxBeamE, dNum2DMissingPtBins, 0.0, dMaxMissingPt);

	//missing py vs px
	locHistName = "MissingPyVsMissingPx";
	locHistTitle = string(";") + locReactionString + string(" Missing P_{x} (GeV/c);") + locReactionString + string(" Missing P_{y} (GeV/c)");
	dHist_MissingPyVsMissingPx = new TH2I(locHistName.c_str(), locHistTitle.c_str(), dNum2DMissingPxyBins, -1.0*dMaxMissingPxy, dMaxMissingPxy, dNum2DMissingPxyBins, -1.0*dMaxMissingPxy, dMaxMissingPxy);

	//missing pt vs pz
	locHistName = "MissingPtVsMissingPz";
	locHistTitle = string(";") + locReactionString + string(" Missing P_{z} (GeV/c);") + locReactionString + string(" Missing P_{Transverse} (GeV/c)");
	dHist_MissingPtVsMissingPz = new TH2I(locHistName.c_str(), locHistTitle.c_str(), dNum2DMissingEPzBins, dMinMissingEPz, dMaxMissingEPz, dNum2DMissingPtBins, 0.0, dMaxMissingPt);

	//Return to the base directory
	ChangeTo_BaseDirectory();
}

bool DHistogramAction_MissingP4::Perform_Action(void)
{
	//beam bunch check, if desired
	if(dBeamBunchRange.first <= dBeamBunchRange.second)
	{
		//get beam/rf delta-t
		DKinematicData* locKinematicData = dParticleComboWrapper->Get_ParticleComboStep(0)->Get_InitialParticle();
		TLorentzVector locBeamX4 = dUseKinFitFlag ? locKinematicData->Get_X4() : locKinematicData->Get_X4_Measured();
		double locRFTime = dUseKinFitFlag ? dParticleComboWrapper->Get_RFTime() : dParticleComboWrapper->Get_RFTime_Measured();
		double locBeamRFDeltaT = locBeamX4.T() - (locRFTime + (locBeamX4.Z() - dTargetCenterZ)/29.9792458);

		//delta-t cut range
		double locMinDeltaT = dBeamBunchPeriod*(double(dBeamBunchRange.first) - 0.5);
		double locMaxDeltaT = dBeamBunchPeriod*(double(dBeamBunchRange.first) + 0.5);

		//check if outside of range
		if((fabs(locBeamRFDeltaT) < locMinDeltaT) || (fabs(locBeamRFDeltaT) > locMaxDeltaT))
			return true;
		if(std::isnan(locRFTime))
			return true;
	}

	DKinematicData* locBeamParticle = dParticleComboWrapper->Get_ParticleComboStep(0)->Get_InitialParticle();
	double locBeamEnergy = 0.0;
	if(dUseKinFitFlag)
		locBeamEnergy = locBeamParticle->Get_P4().E();
	else
		locBeamEnergy = locBeamParticle->Get_P4_Measured().E();

	//get missing p4
	map<unsigned int, set<Int_t> > locSourceObjects;
	TLorentzVector locMissingP4 = dAnalysisUtilities.Calc_MissingP4(dParticleComboWrapper, locSourceObjects, dUseKinFitFlag);

	if(dPreviouslyHistogrammed.find(locSourceObjects) != dPreviouslyHistogrammed.end())
		return true; //dupe: already histed!
	dPreviouslyHistogrammed.insert(locSourceObjects);

	//fill histograms
	dHist_MissingE->Fill(locMissingP4.E());
	dHist_MissingEVsBeamE->Fill(locBeamEnergy, locMissingP4.E());

	dHist_MissingE->Fill(locMissingP4.E());
	dHist_MissingPx->Fill(locMissingP4.Px());
	dHist_MissingPy->Fill(locMissingP4.Py());
	dHist_MissingPz->Fill(locMissingP4.Pz());
	dHist_MissingPt->Fill(locMissingP4.Perp());

	dHist_MissingEVsBeamE->Fill(locBeamEnergy, locMissingP4.E());
	dHist_MissingPzVsBeamE->Fill(locBeamEnergy, locMissingP4.Pz());
	dHist_MissingPtVsBeamE->Fill(locBeamEnergy, locMissingP4.Perp());
	dHist_MissingPtVsMissingPz->Fill(locMissingP4.Pz(), locMissingP4.Perp());
	dHist_MissingPyVsMissingPx->Fill(locMissingP4.Px(), locMissingP4.Py());

	return true;
}

void DHistogramAction_2DInvariantMass::Initialize(void)
{
	string locHistName, locHistTitle;

	string locParticleNamesForHistX = "";
	for(size_t loc_i = 0; loc_i < dXPIDs.size(); ++loc_i)
		locParticleNamesForHistX += ParticleName_ROOT(dXPIDs[loc_i]);

	string locParticleNamesForHistY = "";
	for(size_t loc_i = 0; loc_i < dYPIDs.size(); ++loc_i)
		locParticleNamesForHistY += ParticleName_ROOT(dYPIDs[loc_i]);

	string locMassString = " Invariant Mass (GeV/c^{2});";

	// CREATE & GOTO MAIN FOLDER
	CreateAndChangeTo_ActionDirectory();

	locHistName = "2DInvariantMass";
	locHistTitle = string(";") + locParticleNamesForHistX + locMassString + locParticleNamesForHistY + locMassString;
	dHist_2DInvariantMass = new TH2I(locHistName.c_str(), locHistTitle.c_str(), dNumXBins, dMinX, dMaxX, dNumYBins, dMinY, dMaxY);

	//Return to the base directory
	ChangeTo_BaseDirectory();
}

bool DHistogramAction_2DInvariantMass::Perform_Action(void)
{
	for(size_t loc_i = 0; loc_i < dParticleComboWrapper->Get_NumParticleComboSteps(); ++loc_i)
	{
		const DParticleComboStep* locParticleComboStepWrapper = dParticleComboWrapper->Get_ParticleComboStep(loc_i);
		if((dStepIndex != -1) && (int(loc_i) != dStepIndex))
			continue;

		//build all possible combinations of the included pids
		set<set<size_t> > locXIndexCombos = dAnalysisUtilities.Build_IndexCombos(locParticleComboStepWrapper, dXPIDs);
		set<set<size_t> > locYIndexCombos = dAnalysisUtilities.Build_IndexCombos(locParticleComboStepWrapper, dYPIDs);

		//(double) loop over them
		set<set<size_t> >::iterator locXComboIterator = locXIndexCombos.begin();
		for(; locXComboIterator != locXIndexCombos.end(); ++locXComboIterator)
		{
			map<unsigned int, set<Int_t> > locXSourceObjects;
			TLorentzVector locXP4 = dAnalysisUtilities.Calc_FinalStateP4(dParticleComboWrapper, loc_i, *locXComboIterator, locXSourceObjects, dUseKinFitFlag);

			set<set<size_t> >::iterator locYComboIterator = locYIndexCombos.begin();
			for(; locYComboIterator != locYIndexCombos.end(); ++locYComboIterator)
			{
				map<unsigned int, set<Int_t> > locYSourceObjects;
				TLorentzVector locYP4 = dAnalysisUtilities.Calc_FinalStateP4(dParticleComboWrapper, loc_i, *locYComboIterator, locYSourceObjects, dUseKinFitFlag);

				if(locXSourceObjects == locYSourceObjects)
					continue; //the same!

				set<map<unsigned int, set<Int_t> > > locAllSourceObjects;
				locAllSourceObjects.insert(locXSourceObjects);
				locAllSourceObjects.insert(locYSourceObjects);
				if(dPreviouslyHistogrammed.find(locAllSourceObjects) != dPreviouslyHistogrammed.end())
					continue; //dupe: already histed! (also, could be that the X/Y swapped combo has already been histed: don't double-count!
				dPreviouslyHistogrammed.insert(locAllSourceObjects);

				dHist_2DInvariantMass->Fill(locXP4.M(), locYP4.M());
			}
		}
	}

	return true;
}

void DHistogramAction_Dalitz::Initialize(void)
{
	string locHistName, locHistTitle;

	string locParticleNamesForHistX = "";
	for(size_t loc_i = 0; loc_i < dXPIDs.size(); ++loc_i)
		locParticleNamesForHistX += ParticleName_ROOT(dXPIDs[loc_i]);

	string locParticleNamesForHistY = "";
	for(size_t loc_i = 0; loc_i < dYPIDs.size(); ++loc_i)
		locParticleNamesForHistY += ParticleName_ROOT(dYPIDs[loc_i]);

	string locMassString = " Invariant Mass Squared (GeV/c^{2})^{2};";

	// CREATE & GOTO MAIN FOLDER
	CreateAndChangeTo_ActionDirectory();

	locHistName = "Dalitz";
	locHistTitle = string(";") + locParticleNamesForHistX + locMassString + locParticleNamesForHistY + locMassString;
	dHist_2DInvariantMass = new TH2I(locHistName.c_str(), locHistTitle.c_str(), dNumXBins, dMinX, dMaxX, dNumYBins, dMinY, dMaxY);

	//Return to the base directory
	ChangeTo_BaseDirectory();
}

bool DHistogramAction_Dalitz::Perform_Action(void)
{
	for(size_t loc_i = 0; loc_i < dParticleComboWrapper->Get_NumParticleComboSteps(); ++loc_i)
	{
		const DParticleComboStep* locParticleComboStepWrapper = dParticleComboWrapper->Get_ParticleComboStep(loc_i);
		if((dStepIndex != -1) && (int(loc_i) != dStepIndex))
			continue;

		//build all possible combinations of the included pids
		set<set<size_t> > locXIndexCombos = dAnalysisUtilities.Build_IndexCombos(locParticleComboStepWrapper, dXPIDs);
		set<set<size_t> > locYIndexCombos = dAnalysisUtilities.Build_IndexCombos(locParticleComboStepWrapper, dYPIDs);

		//(double) loop over them
		set<set<size_t> >::iterator locXComboIterator = locXIndexCombos.begin();
		for(; locXComboIterator != locXIndexCombos.end(); ++locXComboIterator)
		{
			map<unsigned int, set<Int_t> > locXSourceObjects;
			TLorentzVector locXP4 = dAnalysisUtilities.Calc_FinalStateP4(dParticleComboWrapper, loc_i, *locXComboIterator, locXSourceObjects, dUseKinFitFlag);

			set<set<size_t> >::iterator locYComboIterator = locYIndexCombos.begin();
			for(; locYComboIterator != locYIndexCombos.end(); ++locYComboIterator)
			{
				map<unsigned int, set<Int_t> > locYSourceObjects;
				TLorentzVector locYP4 = dAnalysisUtilities.Calc_FinalStateP4(dParticleComboWrapper, loc_i, *locYComboIterator, locYSourceObjects, dUseKinFitFlag);

				if(locXSourceObjects == locYSourceObjects)
					continue; //the same!

				set<map<unsigned int, set<Int_t> > > locAllSourceObjects;
				locAllSourceObjects.insert(locXSourceObjects);
				locAllSourceObjects.insert(locYSourceObjects);
				if(dPreviouslyHistogrammed.find(locAllSourceObjects) != dPreviouslyHistogrammed.end())
					continue; //dupe: already histed! (also, could be that the X/Y swapped combo has already been histed: don't double-count!
				dPreviouslyHistogrammed.insert(locAllSourceObjects);

				dHist_2DInvariantMass->Fill(locXP4.M2(), locYP4.M2());
			}
		}
	}

	return true;
}

void DHistogramAction_KinFitResults::Initialize(void)
{
	// CREATE & GOTO MAIN FOLDER
	CreateAndChangeTo_ActionDirectory();

	string locConstraintString = dParticleComboWrapper->Get_KinFitConstraints();
	size_t locNumConstraints = dParticleComboWrapper->Get_NumKinFitConstraints();
	size_t locNumUnknowns = dParticleComboWrapper->Get_NumKinFitUnknowns();
	size_t locNDF = locNumConstraints - locNumUnknowns;

	ostringstream locHistTitle;
	locHistTitle << "Kinematic Fit Constraints: " << locConstraintString << ";Fit #chi^{2}/NDF (" << locNumConstraints;
	locHistTitle << " Constraints, " << locNumUnknowns << " Unknowns: " << locNDF << "-C Fit);# Combos";
	dHist_ChiSqPerDF = new TH1I("ChiSqPerDF", locHistTitle.str().c_str(), dNumChiSqPerDFBins, 0.0, dMaxChiSqPerDF);

	locHistTitle.str("");
	locHistTitle << "Kinematic Fit Constraints: " << locConstraintString << ";Confidence Level (" << locNumConstraints;
	locHistTitle << " Constraints, " << locNumUnknowns << " Unknowns: " << locNDF << "-C Fit);# Combos";
	dHist_ConfidenceLevel = new TH1I("ConfidenceLevel", locHistTitle.str().c_str(), dNumConLevBins, 0.0, 1.0);

	int locNumBins = 0;
	double* locConLevLogBinning = dAnalysisUtilities.Generate_LogBinning(dConLevLowest10Power, 0, dNumBinsPerConLevPower, locNumBins);
	if(locConLevLogBinning != NULL)
		dHist_ConfidenceLevel_LogX = new TH1I("ConfidenceLevel_LogX", locHistTitle.str().c_str(), locNumBins, locConLevLogBinning);
	else
		dHist_ConfidenceLevel_LogX = NULL;

	//Return to the base directory
	ChangeTo_BaseDirectory();
}

bool DHistogramAction_KinFitResults::Perform_Action(void)
{
	double locKinFitChiSqPerNDF = dParticleComboWrapper->Get_ChiSq_KinFit( "" )/dParticleComboWrapper->Get_NDF_KinFit( "" );
	dHist_ChiSqPerDF->Fill(locKinFitChiSqPerNDF);

	double locConfidenceLevel = dParticleComboWrapper->Get_ConfidenceLevel_KinFit( "" );
	dHist_ConfidenceLevel->Fill(locConfidenceLevel);
	if(dHist_ConfidenceLevel_LogX != NULL)
		dHist_ConfidenceLevel_LogX->Fill(locConfidenceLevel);

	return true;
}

void DHistogramAction_BeamEnergy::Initialize(void)
{
	// CREATE & GOTO MAIN FOLDER
	CreateAndChangeTo_ActionDirectory();

	dHist_BeamEnergy = new TH1I("BeamEnergy", ";Beam Energy (GeV)", dNumBeamEnergyBins, dMinBeamEnergy, dMaxBeamEnergy);

	//Return to the base directory
	ChangeTo_BaseDirectory();
}

bool DHistogramAction_BeamEnergy::Perform_Action(void)
{
	DKinematicData* locBeamParticle = dParticleComboWrapper->Get_ParticleComboStep(0)->Get_InitialParticle();
	if(locBeamParticle == NULL)
		return true;

	Int_t locBeamID = locBeamParticle->Get_ID();
	if(dPreviouslyHistogrammed.find(locBeamID) != dPreviouslyHistogrammed.end())
		return true; //previously histogrammed

	double locBeamEnergy = 0.0;
	if(dUseKinFitFlag)
		locBeamEnergy = locBeamParticle->Get_P4().E();
	else
		locBeamEnergy = locBeamParticle->Get_P4_Measured().E();

	dHist_BeamEnergy->Fill(locBeamEnergy);
	dPreviouslyHistogrammed.insert(locBeamID);

	return true;
}

void DHistogramAction_Energy_UnusedShowers::Initialize(void)
{
	// CREATE & GOTO MAIN FOLDER
	CreateAndChangeTo_ActionDirectory();

	dHist_Energy_UnusedShowers = new TH1I("Energy_UnusedShowers", "; Unused Shower Energy (GeV)", dNumEnergyBins, dMinEnergy_UnusedShowers, dMaxEnergy_UnusedShowers);

	//Return to the base directory
	ChangeTo_BaseDirectory();
}

bool DHistogramAction_Energy_UnusedShowers::Perform_Action(void)
{
	// get source objects for uniqueness tracking
	map<unsigned int, set<Int_t> > locSourceObjects;
	TLorentzVector locFinalStateP4 = dAnalysisUtilities.Calc_FinalStateP4(dParticleComboWrapper, 0, locSourceObjects, dUseKinFitFlag);

	if(dPreviouslyHistogrammed.find(locSourceObjects) == dPreviouslyHistogrammed.end()){
		dPreviouslyHistogrammed.insert(locSourceObjects);
	
		double locEnergy_UnusedShowers = dParticleComboWrapper->Get_Energy_UnusedShowers();	
		dHist_Energy_UnusedShowers->Fill(locEnergy_UnusedShowers);
	}

	return true;
}
