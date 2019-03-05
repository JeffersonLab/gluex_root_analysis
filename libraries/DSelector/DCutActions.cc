#include "DCutActions.h"

string DCutAction_ChiSqOrCL::Get_ActionName(void) const
{
	ostringstream locStream;
	locStream << DAnalysisAction::Get_ActionName() << "_" << dSecondaryReactionName;
	return locStream.str();
}

void DCutAction_ChiSqOrCL::Initialize(void)
{
	// CREATE & GOTO MAIN FOLDER
	CreateAndChangeTo_ActionDirectory();
        CreateAndChangeTo_Directory("Before");

	size_t locNumConstraints = dParticleComboWrapper->Get_NumKinFitConstraints();
	size_t locNumUnknowns = dParticleComboWrapper->Get_NumKinFitUnknowns();
	size_t locNDF = locNumConstraints - locNumUnknowns;

	ostringstream locHistTitle;
	locHistTitle << "Initial #chi^{2}/ndf for primary reaction";
	locHistTitle << ";Fit #chi^{2}/NDF (" << locNumConstraints;
	locHistTitle << " Constraints, " << locNumUnknowns << " Unknowns: " << locNDF << "-C Fit);# Combos";
	dHist_ChiSqPerDF_Primary = new TH1I("ChiSqPerDF_Primary", locHistTitle.str().c_str(), dNumChiSqPerDFBins, 0.0, dMaxChiSqPerDF);

	locHistTitle.str("");
	locHistTitle << "Initial #chi^{2}/ndf for " << dSecondaryReactionName;
	locHistTitle << ";Fit #chi^{2}/NDF (" << locNumConstraints;
	locHistTitle << " Constraints, " << locNumUnknowns << " Unknowns: " << locNDF << "-C Fit);# Combos";
	dHist_ChiSqPerDF_Secondary = new TH1I("ChiSqPerDF_Secondary", locHistTitle.str().c_str(), dNumChiSqPerDFBins, 0.0, dMaxChiSqPerDF);

	locHistTitle.str("");
	locHistTitle << "#chi^{2}/NDF comparison; #chi^{2}_secondary; #chi^{2}_primary";
	dHist_ChiSq_Comparison = new TH2I("ChiSq_Comparison", locHistTitle.str().c_str(), dNumChiSqPerDFBins, 0.0, dMaxChiSqPerDF, dNumChiSqPerDFBins, 0.0, dMaxChiSqPerDF);

	locHistTitle.str("");
	locHistTitle << "Initial confidence level for primary reaction";
	locHistTitle << ";Confidence Level (" << locNumConstraints;
	locHistTitle << " Constraints, " << locNumUnknowns << " Unknowns: " << locNDF << "-C Fit);# Combos";
	dHist_ConfidenceLevel_Primary = new TH1I("ConfidenceLevel_Primary", locHistTitle.str().c_str(), dNumConLevBins, 0.0, 1.0);

	int locNumBins = 0;
	double* locConLevLogBinning = dAnalysisUtilities.Generate_LogBinning(dConLevLowest10Power, 0, dNumBinsPerConLevPower, locNumBins);
	if(locConLevLogBinning != NULL)
		dHist_ConfidenceLevel_LogX_Primary = new TH1I("ConfidenceLevel_LogX_Primary", locHistTitle.str().c_str(), locNumBins, locConLevLogBinning);
	else
		dHist_ConfidenceLevel_LogX_Primary = NULL;

	locHistTitle.str("");
	locHistTitle << "Initial confidence level for " << dSecondaryReactionName;
	locHistTitle << ";Confidence Level (" << locNumConstraints;
	locHistTitle << " Constraints, " << locNumUnknowns << " Unknowns: " << locNDF << "-C Fit);# Combos";
	dHist_ConfidenceLevel_Secondary = new TH1I("ConfidenceLevel_Secondary", locHistTitle.str().c_str(), dNumConLevBins, 0.0, 1.0);

	if(locConLevLogBinning != NULL)
		dHist_ConfidenceLevel_LogX_Secondary = new TH1I("ConfidenceLevel_LogX_Secondary", locHistTitle.str().c_str(), locNumBins, locConLevLogBinning);
	else
		dHist_ConfidenceLevel_LogX_Secondary = NULL;

	locHistTitle.str("");
	locHistTitle << "Confidence Level comparision; CL_{secondary}; CL_{primary}";
	dHist_ConfidenceLevel_Comparison = new TH2I("ConfidenceLevel_Comparison", locHistTitle.str().c_str(), dNumConLevBins, 0.0, 1.0, dNumConLevBins, 0.0, 1.0);
	
	if(locConLevLogBinning != NULL)
		dHist_ConfidenceLevel_Log_Comparison = new TH2I("ConfidenceLevel_Log_Comparison", locHistTitle.str().c_str(), locNumBins, locConLevLogBinning, locNumBins, locConLevLogBinning);

	// Return to the base directory
	ChangeTo_BaseDirectory();

        // Make a new directory to show the result of the cut
        CreateAndChangeTo_ActionDirectory();
        CreateAndChangeTo_Directory("After");

	locHistTitle.str("");
	locHistTitle << "Kept #chi^{2}/ndf for primary reaction";
	locHistTitle << ";Fit #chi^{2}/NDF (" << locNumConstraints;
	locHistTitle << " Constraints, " << locNumUnknowns << " Unknowns: " << locNDF << "-C Fit);# Combos";
	dHist_ChiSqPerDF_Primary_post = new TH1I("ChiSqPerDF_Primary_post", locHistTitle.str().c_str(), dNumChiSqPerDFBins, 0.0, dMaxChiSqPerDF);

	locHistTitle.str("");
	locHistTitle << "Removed #chi^{2}/ndf for primary reaction";
	locHistTitle << ";Fit #chi^{2}/NDF (" << locNumConstraints;
	locHistTitle << " Constraints, " << locNumUnknowns << " Unknowns: " << locNDF << "-C Fit);# Combos";
	dHist_ChiSqPerDF_Primary_post_removed = new TH1I("ChiSqPerDF_Primary_post_removed", locHistTitle.str().c_str(), dNumChiSqPerDFBins, 0.0, dMaxChiSqPerDF);

	locHistTitle.str("");
	locHistTitle << "Removed #chi^{2}/ndf for " << dSecondaryReactionName;
	locHistTitle << ";Fit #chi^{2}/NDF (" << locNumConstraints;
	locHistTitle << " Constraints, " << locNumUnknowns << " Unknowns: " << locNDF << "-C Fit);# Combos";
	dHist_ChiSqPerDF_Secondary_post = new TH1I("ChiSqPerDF_Secondary_post", locHistTitle.str().c_str(), dNumChiSqPerDFBins, 0.0, dMaxChiSqPerDF);

	locHistTitle.str("");
	locHistTitle << "Kept confidence level for primary reaction";
	locHistTitle << ";Confidence Level (" << locNumConstraints;
	locHistTitle << " Constraints, " << locNumUnknowns << " Unknowns: " << locNDF << "-C Fit);# Combos";
	dHist_ConfidenceLevel_Primary_post = new TH1I("ConfidenceLevel_Primary_post", locHistTitle.str().c_str(), dNumConLevBins, 0.0, 1.0);

	if(locConLevLogBinning != NULL)
		dHist_ConfidenceLevel_LogX_Primary_post = new TH1I("ConfidenceLevel_LogX_Primary_post", locHistTitle.str().c_str(), locNumBins, locConLevLogBinning);
	else
		dHist_ConfidenceLevel_LogX_Primary_post = NULL;

	locHistTitle.str("");
	locHistTitle << "Removed confidence level for " << dSecondaryReactionName;
	locHistTitle << ";Confidence Level (" << locNumConstraints;
	locHistTitle << " Constraints, " << locNumUnknowns << " Unknowns: " << locNDF << "-C Fit);# Combos";
	dHist_ConfidenceLevel_Secondary_post = new TH1I("ConfidenceLevel_Secondary_post", locHistTitle.str().c_str(), dNumConLevBins, 0.0, 1.0);
	if(locConLevLogBinning != NULL)
		dHist_ConfidenceLevel_LogX_Secondary_post = new TH1I("ConfidenceLevel_LogX_Secondary_post", locHistTitle.str().c_str(), locNumBins, locConLevLogBinning);
	else
		dHist_ConfidenceLevel_LogX_Secondary_post = NULL;

	// Return to the base directory
	ChangeTo_BaseDirectory();
}

bool DCutAction_ChiSqOrCL::Perform_Action(void)
{
	// Primary
	double locKinFitChiSqPerDF = dParticleComboWrapper->Get_ChiSq_KinFit( "" )/dParticleComboWrapper->Get_NDF_KinFit( "" );
	dHist_ChiSqPerDF_Primary->Fill(locKinFitChiSqPerDF);

	double locConfidenceLevel = dParticleComboWrapper->Get_ConfidenceLevel_KinFit( "" );
	dHist_ConfidenceLevel_Primary->Fill(locConfidenceLevel);
	if(dHist_ConfidenceLevel_LogX_Primary != NULL)
		dHist_ConfidenceLevel_LogX_Primary->Fill(locConfidenceLevel);

	// Secondary
	// If the secondary reaction did not have a valid chisq, keep the event by returning true.
	// Skip filling comparison histograms
	if ( dParticleComboWrapper->Get_ChiSq_KinFit( dSecondaryReactionName ) == -1 )
	{
		dHist_ChiSqPerDF_Primary_post->Fill(locKinFitChiSqPerDF);
        	dHist_ConfidenceLevel_Primary_post->Fill(locConfidenceLevel);
		if(dHist_ConfidenceLevel_LogX_Primary_post != NULL)
                	dHist_ConfidenceLevel_LogX_Primary_post->Fill(locConfidenceLevel);
		return true;
	}

	double locKinFitChiSqPerDF_secondary = dParticleComboWrapper->Get_ChiSq_KinFit( dSecondaryReactionName ) / dParticleComboWrapper->Get_NDF_KinFit( dSecondaryReactionName );
	dHist_ChiSqPerDF_Secondary->Fill(locKinFitChiSqPerDF_secondary);

	double locConfidenceLevel_secondary = dParticleComboWrapper->Get_ConfidenceLevel_KinFit( dSecondaryReactionName );
	dHist_ConfidenceLevel_Secondary->Fill(locConfidenceLevel_secondary);
	if(dHist_ConfidenceLevel_LogX_Secondary != NULL)
		dHist_ConfidenceLevel_LogX_Secondary->Fill(locConfidenceLevel_secondary);

	dHist_ChiSq_Comparison->Fill( locKinFitChiSqPerDF_secondary, locKinFitChiSqPerDF );
	dHist_ConfidenceLevel_Comparison->Fill( locConfidenceLevel_secondary, locConfidenceLevel );
	if(dHist_ConfidenceLevel_Log_Comparison != NULL)
		dHist_ConfidenceLevel_Log_Comparison->Fill( locConfidenceLevel_secondary, locConfidenceLevel );

	bool locIsKept = true;
	if ( dIsChiSq )
		locIsKept = ( locKinFitChiSqPerDF < dFunction->Eval( locKinFitChiSqPerDF_secondary ) );
	else
		locIsKept = ( locConfidenceLevel > dFunction->Eval( locConfidenceLevel_secondary ) );

	if ( locIsKept )
	{
		dHist_ChiSqPerDF_Primary_post->Fill(locKinFitChiSqPerDF);
        	dHist_ConfidenceLevel_Primary_post->Fill(locConfidenceLevel);
		if(dHist_ConfidenceLevel_LogX_Primary_post != NULL)
                	dHist_ConfidenceLevel_LogX_Primary_post->Fill(locConfidenceLevel);
	}
	else
	{
		dHist_ChiSqPerDF_Primary_post_removed->Fill(locKinFitChiSqPerDF);
		dHist_ChiSqPerDF_Secondary_post->Fill(locKinFitChiSqPerDF_secondary);
		dHist_ConfidenceLevel_Secondary_post->Fill(locConfidenceLevel_secondary);
		if(dHist_ConfidenceLevel_LogX_Secondary_post != NULL)
                	dHist_ConfidenceLevel_LogX_Secondary_post->Fill(locConfidenceLevel_secondary);
	}

	return locIsKept;
}

string DCutAction_PIDDeltaT::Get_ActionName(void) const
{
	ostringstream locStream;
	locStream << DAnalysisAction::Get_ActionName() << "_" << dPID << "_" << dSystem << "_" << dDeltaTCut;
	return locStream.str();
}

void DCutAction_PIDDeltaT::Initialize(void)
{
	if (dFunc_PIDCut_SelectPositive == nullptr){
		dFunc_PIDCut_SelectPositive = new TF1("dFunc_PIDCut_SelectPositive","pol0",0.0, 12.0);
		dFunc_PIDCut_SelectPositive->SetParameter(0,dDeltaTCut);
	}
	else cout << "Using user function for positive PID delta-T cut " << endl;

	if (dFunc_PIDCut_SelectNegative == nullptr){
		dFunc_PIDCut_SelectNegative = new TF1("dFunc_PIDCut_SelectNegative","pol0",0.0, 12.0);
		dFunc_PIDCut_SelectNegative->SetParameter(0,-dDeltaTCut);
	}
	else cout << "Using user function for negative PID delta-T cut " << endl;	
	
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
			TLorentzVector locP4 = dUseKinFitFlag ? locKinematicData->Get_P4() : locKinematicData->Get_P4_Measured();
			double locRFTime = dParticleComboWrapper->Get_RFTime_Measured();
			double locPropagatedRFTime = locRFTime + (locX4.Z() - dTargetCenterZ)/29.9792458;
			double locDeltaT = locX4.T() - locPropagatedRFTime;
			if(  locDeltaT > dFunc_PIDCut_SelectPositive->Eval(locP4.P()) || locDeltaT < dFunc_PIDCut_SelectNegative->Eval(locP4.P())) 
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

string DCutAction_dEdx::Get_ActionName(void) const
{
	ostringstream locStream;
	locStream << DAnalysisAction::Get_ActionName() << "_" << dPID << "_" << dSystem;
	return locStream.str();
}

void DCutAction_dEdx::Initialize(void)
{
	if(dFunc_dEdxCut_SelectHeavy == NULL)
	{
		string locFuncName = "df_dEdxCut_SelectHeavy"; //e.g. proton
		dFunc_dEdxCut_SelectHeavy = new TF1(locFuncName.c_str(), "exp(-1.0*[0]*x + [1]) + [2]", 0.0, 12.0);
		dFunc_dEdxCut_SelectHeavy->SetParameters(dSelectHeavy_c0, dSelectHeavy_c1, dSelectHeavy_c2);
	}

	if(dFunc_dEdxCut_SelectLight == NULL)
	{
		string locFuncName = "df_dEdxCut_SelectLight"; //e.g. pions, kaons
		dFunc_dEdxCut_SelectLight = new TF1(locFuncName.c_str(), "exp(-1.0*[0]*x + [1]) + [2]", 0.0, 12.0);
		dFunc_dEdxCut_SelectLight->SetParameters(dSelectLight_c0, dSelectLight_c1, dSelectLight_c2);
	}
}

bool DCutAction_dEdx::Perform_Action(void)
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

			Particle_t locPID = locKinematicData->Get_PID();
			if((dPID != Unknown) && (locPID != dPID))
				continue;

			const DChargedTrackHypothesis* locChargedTrackHypothesis = dynamic_cast<const DChargedTrackHypothesis*>(locKinematicData);
			if(locChargedTrackHypothesis == NULL)
				continue; //neutral!

			Float_t locdEdx;
			if(dSystem == SYS_CDC)
				locdEdx = locChargedTrackHypothesis->Get_dEdx_CDC()*1.0E6;
			else if(dSystem == SYS_FDC)
				locdEdx = locChargedTrackHypothesis->Get_dEdx_FDC()*1.0E6;
			else if(dSystem == SYS_START)
				locdEdx = locChargedTrackHypothesis->Get_dEdx_ST()*1.0E3;
			else if(dSystem == SYS_TOF)
				locdEdx = locChargedTrackHypothesis->Get_dEdx_TOF()*1.0E3;
			else
				continue;

			if(!(locdEdx > 0.0))
				continue; // Not enough hits in the detector to report a dE/dx: Don't cut

			//cut
			double locP = dUseKinFitFlag ? locChargedTrackHypothesis->Get_P4().Vect().Mag() : locChargedTrackHypothesis->Get_P4_Measured().Vect().Mag();
			if((ParticleMass(locPID) + 0.0001) >= ParticleMass(Proton))
			{
				//heavy
				if(dMaxRejectionFlag) //focus on rejecting background pions
				{
					if(locdEdx < dFunc_dEdxCut_SelectLight->Eval(locP))
						return false;
				}
				else if(locdEdx < dFunc_dEdxCut_SelectHeavy->Eval(locP))
					return false; //focus on keeping signal protons
			}
			else
			{
				//light
				if(dMaxRejectionFlag) //focus on rejecting background pions
				{
					if(locdEdx > dFunc_dEdxCut_SelectHeavy->Eval(locP))
						return false;
				}
				else if(locdEdx > dFunc_dEdxCut_SelectLight->Eval(locP))
					return false; //focus on keeping signal pions
			}
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

		//loop over them: Must fail only ONE to fail. only if all succeed, go to the next step
		set<set<size_t> >::iterator locComboIterator = locIndexCombos.begin();
		for(; locComboIterator != locIndexCombos.end(); ++locComboIterator)
		{
			TLorentzVector locFinalStateP4 = dAnalysisUtilities.Calc_FinalStateP4(dParticleComboWrapper, loc_i, *locComboIterator, dUseKinFitFlag);
			double locInvariantMass = locFinalStateP4.M();
			if((dMinMass < locInvariantMass) && (locInvariantMass < dMaxMass))
			  return false;
		}
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
	double locConfidenceLevel = dParticleComboWrapper->Get_ConfidenceLevel_KinFit( "" );
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

			TLorentzVector locP4 = dUseKinFitFlag ? locKinematicData->Get_P4() : locKinematicData->Get_P4_Measured();
			double locP = locP4.P();
			locShowerEOverP = locShowerEnergy/locP;

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

string DCutAction_TrackBCALPreshowerFraction::Get_ActionName(void) const
{
	ostringstream locStream;
	locStream << DAnalysisAction::Get_ActionName() << "_" << dPID << "_" << dPreshowerFractionCut;
	return locStream.str();
}

bool DCutAction_TrackBCALPreshowerFraction::Perform_Action(void)
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

			if(locKinematicData->Get_PID() != dPID)
				continue;
			if(ParticleCharge(locKinematicData->Get_PID()) == 0)
				continue;

			const DChargedTrackHypothesis* locChargedTrackHypothesis = dynamic_cast<const DChargedTrackHypothesis*>(locKinematicData);
			double locBCALPreshowerE = locChargedTrackHypothesis->Get_Energy_BCALPreshower();
			double locBCALShowerE = locChargedTrackHypothesis->Get_Energy_BCAL();
			if(!(locBCALShowerE > 0.0))
				continue;

			double locPreshowerFraction = locBCALPreshowerE/locBCALShowerE*sin(locChargedTrackHypothesis->Get_P4().Theta());

			if((dPID == Electron) || (dPID == Positron))
			{
				if(locPreshowerFraction < dPreshowerFractionCut)
					return false;
			}       
			//else if(locPreshowerFraction > dPreshowerFractionCut)
			//	 return false;

		}
	}

	return true;
}

string DCutAction_Kinematics::Get_ActionName(void) const
{
	ostringstream locStream;
	locStream << DAnalysisAction::Get_ActionName() << "_" << dStepIndex << "_" << dPID << "_" << dUseKinFitFlag << "_";
	locStream << dCutMinP << "_" << dCutMaxP << "_" << dCutMinTheta << "_" << dCutMaxTheta << "_" << dCutMinPhi << "_" << dCutMaxPhi;
	return locStream.str();
}

bool DCutAction_Kinematics::Perform_Action(void)
{
	for(size_t loc_i = 0; loc_i < dParticleComboWrapper->Get_NumParticleComboSteps(); ++loc_i)
	{
		if((int(loc_i) != dStepIndex) && (dStepIndex != -1))
			continue;
		DParticleComboStep* locComboWrapperStep = dParticleComboWrapper->Get_ParticleComboStep(loc_i);

		//final particles
		for(size_t loc_j = 0; loc_j < locComboWrapperStep->Get_NumFinalParticles(); ++loc_j)
		{
			DKinematicData* locKinematicData = locComboWrapperStep->Get_FinalParticle(loc_j);
			if(locKinematicData == NULL)
				continue; //e.g. a decaying or missing particle whose params aren't set yet

			Particle_t locPID = locKinematicData->Get_PID();
			if((locPID != dPID) && (dPID != Unknown))
				continue;

			TLorentzVector locP4 = dUseKinFitFlag ? locKinematicData->Get_P4() : locKinematicData->Get_P4_Measured();
			double locP = locP4.P();
			if((dCutMinP < dCutMaxP) && (locP >= dCutMinP) && (locP <= dCutMaxP))
				return false;

			double locTheta = locP4.Theta()*180.0/TMath::Pi();
			if((dCutMinTheta < dCutMaxTheta) && (locTheta >= dCutMinTheta) && (locTheta <= dCutMaxTheta))
				return false;

			double locPhi = locP4.Phi()*180.0/TMath::Pi();
			if((dCutMinPhi < dCutMaxPhi) && (locPhi >= dCutMinPhi) && (locPhi <= dCutMaxPhi))
				return false;
		}
	}

	return true;
}

string DCutAction_Energy_UnusedShowers::Get_ActionName(void) const
{
	ostringstream locStream;
	locStream << DAnalysisAction::Get_ActionName() << "_" << dMaxEnergy_UnusedShowersCut;
	return locStream.str();
}

bool DCutAction_Energy_UnusedShowers::Perform_Action(void)
{
	return (dParticleComboWrapper->Get_Energy_UnusedShowers() <= dMaxEnergy_UnusedShowersCut);
}

string DCutAction_NumUnusedTracks::Get_ActionName(void) const
{
	ostringstream locStream;
	locStream << DAnalysisAction::Get_ActionName() << "_" << dMaxUnusedTracks;
	return locStream.str();
}

bool DCutAction_NumUnusedTracks::Perform_Action(void)
{
	return (dParticleComboWrapper->Get_NumUnusedTracks() <= dMaxUnusedTracks);
}

string DCutAction_VanHoveAngle::Get_ActionName(void) const
{
	ostringstream locStream;
	locStream << DAnalysisAction::Get_ActionName() << "_" << dMinAngle << "_" << dMaxAngle;
	return locStream.str();
}

bool DCutAction_VanHoveAngle::Perform_Action(void)
{
	double locVanHoveQ, locVanHovePhi;
	const DParticleComboStep* locParticleComboStepWrapper = dParticleComboWrapper->Get_ParticleComboStep(0);

	//build all possible combinations of the included pids
	set<set<size_t> > locXIndexCombos = dAnalysisUtilities.Build_IndexCombos(locParticleComboStepWrapper, dXPIDs);
	set<set<size_t> > locYIndexCombos = dAnalysisUtilities.Build_IndexCombos(locParticleComboStepWrapper, dYPIDs);
	set<set<size_t> > locZIndexCombos = dAnalysisUtilities.Build_IndexCombos(locParticleComboStepWrapper, dZPIDs);
	
	bool locAnyOKFlag = false;
	//(triple) loop over them
	set<set<size_t> >::iterator locXComboIterator = locXIndexCombos.begin();
	for(; locXComboIterator != locXIndexCombos.end(); ++locXComboIterator)
	{
		map<unsigned int, set<Int_t> > locXSourceObjects;
		TLorentzVector locXP4 = dAnalysisUtilities.Calc_FinalStateP4(dParticleComboWrapper, 0, *locXComboIterator, locXSourceObjects, dUseKinFitFlag);
		
		set<set<size_t> >::iterator locYComboIterator = locYIndexCombos.begin();
		for(; locYComboIterator != locYIndexCombos.end(); ++locYComboIterator)
		{
			map<unsigned int, set<Int_t> > locYSourceObjects;
			TLorentzVector locYP4 = dAnalysisUtilities.Calc_FinalStateP4(dParticleComboWrapper, 0, *locYComboIterator, locYSourceObjects, dUseKinFitFlag);
			
			set<set<size_t> >::iterator locZComboIterator = locZIndexCombos.begin();
			for(; locZComboIterator != locZIndexCombos.end(); ++locZComboIterator)
			{
				map<unsigned int, set<Int_t> > locZSourceObjects;
				TLorentzVector locZP4 = dAnalysisUtilities.Calc_FinalStateP4(dParticleComboWrapper, 0, *locZComboIterator, locZSourceObjects, dUseKinFitFlag);
				
				if(locXSourceObjects == locYSourceObjects || locXSourceObjects == locZSourceObjects || locYSourceObjects == locZSourceObjects)
					continue; //the same!
				
				std::tie (locVanHoveQ,locVanHovePhi) = dAnalysisUtilities.Calc_vanHoveCoord(locXP4, locYP4, locZP4);
				if((locVanHovePhi > dMaxAngle) || (locVanHovePhi < dMinAngle))
					continue;
				locAnyOKFlag = true;
				break;
			}
		}
	}
	
	if(!locAnyOKFlag)
		return false;
	
	
	return true;
}


string DCutAction_VanHoveAngleFour::Get_ActionName(void) const
{
	ostringstream locStream;
	locStream << DAnalysisAction::Get_ActionName() << "_" << dMinThetaAngle << "_" << dMaxThetaAngle << "_" << dMinPhiAngle << "_" << dMaxPhiAngle;
	return locStream.str();
}

bool DCutAction_VanHoveAngleFour::Perform_Action(void)
{
	double locVanHoveR, locVanHoveTheta, locVanHovePhi;
	const DParticleComboStep* locParticleComboStepWrapper = dParticleComboWrapper->Get_ParticleComboStep(0);

	//build all possible combinations of the included pids
	set<set<size_t> > locVec1IndexCombos = dAnalysisUtilities.Build_IndexCombos(locParticleComboStepWrapper, dVec1PIDs);
	set<set<size_t> > locVec2IndexCombos = dAnalysisUtilities.Build_IndexCombos(locParticleComboStepWrapper, dVec2PIDs);
	set<set<size_t> > locVec3IndexCombos = dAnalysisUtilities.Build_IndexCombos(locParticleComboStepWrapper, dVec3PIDs);
	set<set<size_t> > locVec4IndexCombos = dAnalysisUtilities.Build_IndexCombos(locParticleComboStepWrapper, dVec4PIDs);

	bool locAnyOKFlag = false;
	//(triple) loop over them
	//(quadruple) loop over them
	set<set<size_t> >::iterator locVec1ComboIterator = locVec1IndexCombos.begin();
	for(; locVec1ComboIterator != locVec1IndexCombos.end(); ++locVec1ComboIterator)
	{
		map<unsigned int, set<Int_t> > locVec1SourceObjects;
		TLorentzVector locVec1P4 = dAnalysisUtilities.Calc_FinalStateP4(dParticleComboWrapper, 0, *locVec1ComboIterator, locVec1SourceObjects, dUseKinFitFlag);

		set<set<size_t> >::iterator locVec2ComboIterator = locVec2IndexCombos.begin();
		for(; locVec2ComboIterator != locVec2IndexCombos.end(); ++locVec2ComboIterator)
		{
			map<unsigned int, set<Int_t> > locVec2SourceObjects;
			TLorentzVector locVec2P4 = dAnalysisUtilities.Calc_FinalStateP4(dParticleComboWrapper, 0, *locVec2ComboIterator, locVec2SourceObjects, dUseKinFitFlag);

			set<set<size_t> >::iterator locVec3ComboIterator = locVec3IndexCombos.begin();
			for(; locVec3ComboIterator != locVec3IndexCombos.end(); ++locVec3ComboIterator)
			{
				map<unsigned int, set<Int_t> > locVec3SourceObjects;
				TLorentzVector locVec3P4 = dAnalysisUtilities.Calc_FinalStateP4(dParticleComboWrapper, 0, *locVec3ComboIterator, locVec3SourceObjects, dUseKinFitFlag);

				set<set<size_t> >::iterator locVec4ComboIterator = locVec4IndexCombos.begin();
				for(; locVec4ComboIterator != locVec4IndexCombos.end(); ++locVec4ComboIterator)
				  {
				    map<unsigned int, set<Int_t> > locVec4SourceObjects;
				    TLorentzVector locVec4P4 = dAnalysisUtilities.Calc_FinalStateP4(dParticleComboWrapper, 0, *locVec4ComboIterator, locVec4SourceObjects, dUseKinFitFlag);
				    
				    if(locVec1SourceObjects == locVec2SourceObjects || locVec1SourceObjects == locVec3SourceObjects || locVec1SourceObjects == locVec4SourceObjects || locVec2SourceObjects == locVec3SourceObjects || locVec2SourceObjects == locVec4SourceObjects || locVec3SourceObjects == locVec4SourceObjects)
				      continue; //the same!
				    
				    std::tie (locVanHoveR,locVanHoveTheta,locVanHovePhi) = dAnalysisUtilities.Calc_vanHoveCoordFour(locVec1P4, locVec2P4, locVec3P4, locVec4P4);
				    if(((locVanHovePhi > dMaxPhiAngle) || (locVanHovePhi < dMinPhiAngle)) || ((locVanHoveTheta > dMaxThetaAngle) || (locVanHoveTheta < dMinThetaAngle)))
				      continue;
				    locAnyOKFlag = true;
				    break;
				  }
			}
		}
	}

	if(!locAnyOKFlag)
		return false;


	return true;
}
