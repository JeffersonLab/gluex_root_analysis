#include <string>
#include <map>
#include <iostream>
#include <fstream>

#include <TFile.h>
#include <TTree.h>

#include "DSelector/DTreeInterface.h"
#include "utilities.h"

//function declarations
void Print_Usage(void);
void Print_Config(string locSelectorBaseName, DTreeInterface* locTreeInterface, map<int, map<int, pair<Particle_t, string> > >& locComboInfoMap);
void Print_HeaderFile(string locSelectorBaseName, DTreeInterface* locTreeInterface, map<int, map<int, pair<Particle_t, string> > >& locComboInfoMap, string locConfiguration);
void Print_SourceFile(string locSelectorBaseName, DTreeInterface* locTreeInterface, map<int, map<int, pair<Particle_t, string> > >& locComboInfoMap, string locConfiguration, bool loc_unconstrained_flag);
void Print_HeaderFile_MCGen(string locSelectorBaseName, DTreeInterface* locTreeInterface, string locConfiguration);
void Print_SourceFile_MCGen(string locSelectorBaseName, DTreeInterface* locTreeInterface, string locConfiguration);

//utilities declarations
void Add_Action(std::ofstream& locOfStream, string locConfig);
void Add_HistogramDefinition(std::ofstream& locOfStream, string locConfig);
void Add_HistogramInitialization(std::ofstream& locOfStream, string locConfig);
void Add_UniquenessTrackingDefinition(std::ofstream& locOfStream, string locConfig);
void Add_UniquenessTrackingDefinition_reset(std::ofstream& locOfStream, string locConfig);
void Add_UniquenessTrackingDefinition_Thrown(std::ofstream& locOfStream, string locConfig);
void HistWeight(std::ofstream& locOfStream, string locConfig);
void FillHist(std::ofstream& locOfStream, string locConfig, string UniquenessName);
void Add_UniqueHistogramFilling(std::ofstream& locOfStream, string locConfig, map<int, map<int, pair<Particle_t, string> > >& locComboInfoMap);
void Add_KinematicCalculation(std::ofstream& locOfStream, string locConfig);
void Add_LorentzBoost(std::ofstream& locOfStream, string locConfig, DTreeInterface* locTreeInterface, map<int, map<int, pair<Particle_t, string> > >& locComboInfoMap);
void Add_Thrown_LorentzBoost(std::ofstream& locOfStream, string locConfig);
void Add_MCThrownParticles(std::ofstream& locOfStream, string locConfig);



int main(int argc, char* argv[])
{
	cout<< argc << endl;
	if( argc!=4 && argc!=5)
	{
		Print_Usage();
		//Print_Config("example");
		return 0;
	}

	string locInputFileName = argv[1];
	string locTreeName = argv[2];
	string locSelectorBaseName = argv[3];
	string locConfiguration;
	if( argc == 4 ) locConfiguration = "0";
	if( argc == 5 ) locConfiguration= argv[4];
	
	TFile* locInputFile = new TFile(locInputFileName.c_str(), "READ");
	TTree* locTree = (TTree*)locInputFile->Get(locTreeName.c_str());

	DTreeInterface* locTreeInterface = new DTreeInterface(locTree, true); //true: is input

	//determine if it is mcthrown tree
	bool locIsMCGenTreeFlag = (locTreeInterface->Get_Branch("NumCombos") == NULL);
	if(locIsMCGenTreeFlag)
	{
		cout<<"Analyzing mcthrown tree right now ..."<<endl;
		Print_HeaderFile_MCGen(locSelectorBaseName, locTreeInterface, locConfiguration);
		Print_SourceFile_MCGen(locSelectorBaseName, locTreeInterface, locConfiguration);
		string locSelectorName = string("DSelector_") + locSelectorBaseName;
		cout << "Selector files " << locSelectorName << ".* generated." << endl;
		return 0;
	}

	//get combo info
	map<int, map<int, pair<Particle_t, string> > > locComboInfoMap;
	locTreeInterface->Get_ComboInfo(locComboInfoMap);

	//tell if the tree is unconstrained of decaying mass in kinFit
	bool loc_unconstrained_flag = (locTreeName.find('M')!=std::string::npos);
	
	Print_HeaderFile(locSelectorBaseName, locTreeInterface, locComboInfoMap, locConfiguration);
	Print_SourceFile(locSelectorBaseName, locTreeInterface, locComboInfoMap, locConfiguration, loc_unconstrained_flag);

	string locSelectorName = string("DSelector_") + locSelectorBaseName;
	cout << "Selector files " << locSelectorName << ".* generated." << endl;

	if(argc == 4)
	{
		Print_Config(locSelectorBaseName, locTreeInterface, locComboInfoMap);
	}

	return 0;
}

void Print_Usage(void)
{
	cout << endl;
	cout << "Makes a custom DSelector for the input TTree created by the DANA ANALYSIS library." << endl;
	cout << "1st argument: The input ROOT TTree file name." << endl;
	cout << "2nd argument: The name of the TTree in the input ROOT file that you want to make a selector for." << endl;
	cout << "3rd argument: The base-name of the selector class & files you want to generate." << endl;
	cout << "4th argument(optional): The name of configuration file." << endl;
	cout << "The generated files will be named \"DSelector_<base-name>.C\" and \"DSelector_<base-name>.h\"." << endl;
	cout << endl;
}

void Print_Config(string locSelectorBaseName, DTreeInterface* locTreeInterface, map<int, map<int, pair<Particle_t, string> > >& locComboInfoMap)
{
	//get particles
	string locFinalStateParticles;
	map<int, map<int, pair<Particle_t, string> > >::iterator locStepIterator = locComboInfoMap.begin();
	for(; locStepIterator != locComboInfoMap.end(); ++locStepIterator)
	{
		//int locStepIndex = locStepIterator->first;
		//locOfStream << "		//Step " << locStepIndex << endl;
		map<int, pair<Particle_t, string> >& locStepInfoMap = locStepIterator->second;
		map<int, pair<Particle_t, string> >::iterator locParticleIterator = locStepInfoMap.begin();
		for(; locParticleIterator != locStepInfoMap.end(); ++locParticleIterator)
		{
			Particle_t locPID = locParticleIterator->second.first;
			string locParticleName = locParticleIterator->second.second;

			if(locPID == Unknown)
				continue;
			else if(locParticleName == "ComboBeam")
				locFinalStateParticles += "Beam";
			else if(locParticleName.substr(0, 6) == "Target")
				continue;
			else if(locParticleName.substr(0, 8) == "Decaying")
				continue;
			else if(locParticleName.substr(0, 7) == "Missing")
				continue;
			else if(ParticleCharge(locPID) != 0)
				locFinalStateParticles += locParticleName;
			else
				locFinalStateParticles += locParticleName;

			locFinalStateParticles += " ";
		}
		
	}

	//write out the configuration template
	string locConfigName = locSelectorBaseName + string(".config");
	ofstream locConfigStream;
	locConfigStream.open(locConfigName.c_str());

	locConfigStream << "## This is a templete of the DSelector's configuration file" << endl;
	locConfigStream << "## To turn on this feature in \"MakeDSelector\" command, use the fullpath name of this configuration file as the 4th argument" << endl;
	locConfigStream << endl;
	locConfigStream << endl;
	locConfigStream << "######################################################" << endl;
	locConfigStream << "[SECTION: ANALYSIS ACTION]" << endl;
	locConfigStream << "######################################################" << endl;
	locConfigStream << "## uncomment to book actions (or add more with locActionUniqueString specified)" << endl;
	locConfigStream << "## DSelector will preserve the sequence of all actions" << endl;
	locConfigStream << "## specify cut values and options after \"=\", (items separated by space or comma)" << endl;
	locConfigStream << "## a list of available actions, and the required option fields:" << endl;
	locConfigStream << "#DCutAction_ChiSqOrCL              ##(dComboWrapper, SecondaryReactionName, IsChiSq, Function, ActionUniqueString)" << endl;
	locConfigStream << "#DCutAction_PIDDeltaT              ##(dComboWrapper, UseKinFitFlag, DeltaTCut, PID, System, ActionUniqueString) " << endl;
	locConfigStream << "#DCutAction_NoPIDHit               ##(dComboWrapper, PID, System, ActionUniqueString)" << endl;
	locConfigStream << "#DCutAction_dEdx                   ##(dComboWrapper, UseKinFitFlag, PID, System, ActionUniqueString)" << endl;
	locConfigStream << "#DCutAction_KinFitFOM              ##(dComboWrapper, MinimumConfidenceLevel, ActionUniqueString)" << endl;
	locConfigStream << "#DCutAction_MissingMass            ##(dComboWrapper, UseKinFitFlag, MinimumMissingMass, MaximumMissingMass, ActionUniqueString)" << endl;
	locConfigStream << "#DCutAction_MissingMassSquared     ##(dComboWrapper, UseKinFitFlag, MinimumMissingMassSq, MaximumMissingMassSq, ActionUniqueString)" << endl;
	locConfigStream << "#DCutAction_MissingEnergy          ##(dComboWrapper, UseKinFitFlag, MinimumMissingEnergy, MaximumMissingEnergy, ActionUniqueString)" << endl;
	locConfigStream << "#DCutAction_MissingEnergy          ##(dComboWrapper, UseKinFitFlag, MissingEnergyOffOfStepIndex, MissingEnergyOffOfPIDs, MinimumMissingEnergy, MaximumMissingEnergy, ActionUniqueString)" << endl;
	locConfigStream << "#DCutAction_MissingEnergy          ##(dComboWrapper, UseKinFitFlag, MissingEnergyOffOfStepIndex, MissingEnergyOffOfPID, MinimumMissingEnergy, MaximumMissingEnergy, ActionUniqueString)" << endl;
	locConfigStream << "#DCutAction_InvariantMass          ##(dComboWrapper, UseKinFitFlag, InitialPID, MinMass, MaxMass, ActionUniqueString)" << endl;
	locConfigStream << "#DCutAction_InvariantMass          ##(dComboWrapper, UseKinFitFlag, StepIndex, ToIncludePIDs, MinMass, MaxMass, ActionUniqueString)" << endl;
	locConfigStream << "#DCutAction_InvariantMassVeto      ##(dComboWrapper, UseKinFitFlag, InitialPID, MinMass, MaxMass, ActionUniqueString)" << endl;
	locConfigStream << "#DCutAction_InvariantMassVeto      ##(dComboWrapper, UseKinFitFlag, StepIndex, ToIncludePIDs, MinMass, MaxMass, ActionUniqueString)" << endl;
	locConfigStream << "#DCutAction_BeamEnergy             ##(dComboWrapper, UseKinFitFlag, MinBeamEnergy, MaxBeamEnergy, ActionUniqueString)" << endl;
	locConfigStream << "#DCutAction_TrackShowerEOverP      ##(dComboWrapper, UseKinFitFlag, Detector, PID, ShowerEOverPCut, ActionUniqueString)" << endl;
	locConfigStream << "#DCutAction_Kinematics             ##(dComboWrapper, StepIndex, PID, UseKinFitFlag, CutMinP, CutMaxP, CutMinTheta, CutMaxTheta, CutMinPhi, CutMaxPhi)" << endl;
	locConfigStream << "#DHistogramAction_ParticleComboKinematics  ##(dComboWrapper, UseKinFitFlag, ActionUniqueString)" << endl;
	locConfigStream << "#DHistogramAction_ParticleID               ##(dComboWrapper, UseKinFitFlag, ActionUniqueString)" << endl;
	locConfigStream << "#DHistogramAction_InvariantMass            ##(dComboWrapper, UseKinFitFlag, InitialPID, NumMassBins, MinMass, MaxMass, ActionUniqueString)" << endl;
	locConfigStream << "#DHistogramAction_MissingMass              ##(dComboWrapper, UseKinFitFlag, NumMassBins, MinMass, MaxMass, ActionUniqueString)" << endl;
	locConfigStream << "#DHistogramAction_MissingMassSquared       ##(dComboWrapper, UseKinFitFlag, NumMassBins, MinMassSq, MaxMassSq, ActionUniqueString)" << endl;
	locConfigStream << "#DHistogramAction_MissingP4                ##(dComboWrapper, UseKinFitFlag, ActionUniqueString)" << endl;
	locConfigStream << "#DHistogramAction_2DInvariantMass          ##(dComboWrapper, UseKinFitFlag, StepIndex, XPIDs, YPIDs, NumXBins, MinX, MaxX, NumYBins, MinY, MaxY, ActionUniqueString)" << endl;
	locConfigStream << "#DHistogramAction_Dalitz                   ##(dComboWrapper, UseKinFitFlag, StepIndex, XPIDs, YPIDs, NumXBins, MinX, MaxX, NumYBins, MinY, MaxY, ActionUniqueString)" << endl;
	locConfigStream << "#DHistogramAction_vanHove                  ##(dComboWrapper, UseKinFitFlag, XPIDs, YPIDs, ZPIDs, ActionUniqueString)" << endl;
	locConfigStream << "#DHistogramAction_KinFitResults            ##(dComboWrapper, ActionUniqueString)" << endl;
	locConfigStream << "#DHistogramAction_BeamEnergy               ##(dComboWrapper, UseKinFitFlag, ActionUniqueString)" << endl;
	locConfigStream << "#DHistogramAction_Energy_UnusedShowers     ##(dComboWrapper, ActionUniqueString)" << endl;

	locConfigStream << endl;
	locConfigStream << "[END: ANALYSIS ACTION]" << endl;
	locConfigStream << endl;
	locConfigStream << "######################################################" << endl;
	locConfigStream << "[SECTION: KINEMATIC CALCULATION]" << endl;
	locConfigStream << "######################################################" << endl;
	locConfigStream << "## For lorentz transformation on TLorentzVector," << endl;
	locConfigStream << "## fill out as: LorentzTransformation = frameName, fromWhichFrame, \"expression of the boostVector\" " << endl;
	locConfigStream << "## by default, all particle's P4 will be boosted, new LorentzVector will come with suffix \"_frameName\" " << endl;
	locConfigStream << "## note that the default suffix for LAB FRAME is blank" << endl;
	locConfigStream << "LorentzTransformation = COM LAB \"locBeamP4 + dTargetP4\" ## An example of boost from lab frame to center-of-mass frame" << endl;
	locConfigStream << endl;
	locConfigStream << "## For Kinematics: " << endl;
	locConfigStream << "## Format: variableType variableName = Expression(using TLorentzVector and TMath library)" << endl;
	locConfigStream << "## VariableName goes to \"FillVariable\" in \"[HISTOGRAMS]\" section" << endl;
	locConfigStream << "## Comments start with // will be passed to your DSelector as well."<< endl;
	locConfigStream << endl;
	locConfigStream << "//Kinfit Confidence Level" << endl;
	locConfigStream << "Double_t locConfidenceLevel = dComboWrapper->Get_ConfidenceLevel_KinFit(\"\");" << endl;
	locConfigStream << "// BeamRFDeltaT" << endl;
	locConfigStream << "TLorentzVector locBeamX4 = dComboBeamWrapper->Get_X4();" << endl;
	locConfigStream << "Double_t locBeamRFDeltaT = locBeamX4.T() - (dComboWrapper->Get_RFTime() + (locBeamX4.Z() - dComboWrapper->Get_TargetCenter().Z())/29.9792458);" << endl;
	locConfigStream << endl;
	locConfigStream << "[END: KINEMATIC CALCULATION]" << endl;
	locConfigStream << endl;
	locConfigStream << "######################################################" << endl;
	locConfigStream << "[SECTION: UNIQUENESS TRACKING]" << endl;
	locConfigStream << "######################################################" << endl;
	locConfigStream << "## Prevent double-counting for specified particles or set of particles" << endl;
	locConfigStream << "## String before \"=\" goes to \"UniquenessTracking\" in \"[HISTOGRAMS]\" section" << endl;
	locConfigStream << "## String after \"=\" will be the particles used during calculation of the variable to be filled" << endl;
	locConfigStream << "AfterComboLoop  ## fill the histogram only once, at the very end of combo loop (once per event)." << endl;
	locConfigStream << "perEvent ## Example: use this if you want fill hist once per event." << endl;
	locConfigStream << "perCombo ## Example: use this if you do not wish to keep uniqueness tracking (for example plotting kinfitCL)." << endl;
	locConfigStream << "Particle-specific(Beam)      = Beam ## Example: plot for every beam photon." << endl;
	locConfigStream << "Combo-specific(All)          = "<< locFinalStateParticles << " ## Example: plot for every combo." << endl;
	locConfigStream << endl;  
	locConfigStream << "[END: UNIQUENESS TRACKING]" << endl;                         
	locConfigStream << endl;
	locConfigStream << "######################################################" << endl;
	locConfigStream << "[SECTION: HISTOGRAMS]" << endl;
	locConfigStream << "######################################################" << endl;
	locConfigStream << "## Specify after \"HISTOGRAM = \": histType, keyName, histTitles (\"Main Title; X Title; Y Title\"), " << endl;
	locConfigStream << "## binxN, xMin, xMax, binyN, yMin, yMax, xVariable, yVariable, UniquenessTracking" << endl;
	locConfigStream << "##" << endl;
	locConfigStream << "## For 1-dimensional histogram, place \"-\" for binyN, yMin, yMax, yVariable" << endl;
	locConfigStream << "## For histTitle, latex is available. Use \"\\\" as the control. For example, \\frac{}{} for fraction" << endl;
	locConfigStream << "## FillVariable's formula need to be specified in \"[KINEMATIC CALCULATION]\" section" << endl;
	locConfigStream << "## UniquenessTracking need to be specified in \"[UNIQUENESS TRACKING]\" section" << endl;
	locConfigStream << "## If you don't need uniqueness tracking i.e add one entry per combo, just specified Uniqueness as \"perCombo\" ." << endl;
	locConfigStream << "## If you want one entry per event, just specified Uniqueness as \"perEvent\" ." << endl;
	locConfigStream << "HISTOGRAM = TH1D loc_NumberComboAfterCuts_All \";\\Number of combos passed all cuts\" 20 0.0 20.0 - - - loc_NumberComboAfterAllCuts - AfterComboLoop" << endl;
	locConfigStream << "HISTOGRAM = TH1D loc_NumberComboAfterCuts_Beam \";\\Number of combos passed all cuts with uniqueness tracking on beam photon\" 20 0.0 20.0 - - - loc_NumberComboAfterCuts_Beam - AfterComboLoop" << endl;
	locConfigStream << "HISTOGRAM = TH1D KinFitCL \";Kinematic Fit Confidence Level\" 100 0 1.0 - - - locConfidenceLevel - perCombo" << endl;
	locConfigStream << "HISTOGRAM = TH1D locBeamRFDeltaT_per_Combo \";\\Delta t_{Beam - RF} (ns)\" 100 -10.0 10.0 - - - locBeamRFDeltaT - Combo-specific(All)" << endl;
	locConfigStream << "HISTOGRAM = TH1D locBeamRFDeltaT_per_Beam \";\\Delta t_{Beam - RF} (ns)\" 100 -10.0 10.0 - - - locBeamRFDeltaT - Particle-specific(Beam)" << endl;
	locConfigStream << endl;
	locConfigStream << "[END: HISTOGRAMS]" << endl;
	locConfigStream << endl;

	locConfigStream.close();

	cout << "Add a 4th argument(config) to turn on more features." << endl;
	cout << "Config file template " << locSelectorBaseName << ".config generated." << endl;

	cout << endl;


}



void Print_HeaderFile(string locSelectorBaseName, DTreeInterface* locTreeInterface, map<int, map<int, pair<Particle_t, string> > >& locComboInfoMap, string locConfiguration)
{
	string locSelectorName = string("DSelector_") + locSelectorBaseName;
	string locHeaderName = locSelectorName + string(".h");
	ofstream locHeaderStream;
	locHeaderStream.open(locHeaderName.c_str());

	locHeaderStream << "#ifndef " << locSelectorName << "_h" << endl;
	locHeaderStream << "#define " << locSelectorName << "_h" << endl;
	locHeaderStream << endl;
	locHeaderStream << "#include <iostream>" << endl;
	locHeaderStream << endl;
	locHeaderStream << "#include \"DSelector/DSelector.h\"" << endl;
	locHeaderStream << "#include \"DSelector/DHistogramActions.h\"" << endl;
	locHeaderStream << "#include \"DSelector/DCutActions.h\"" << endl;
	locHeaderStream << endl;
	locHeaderStream << "#include \"TH1I.h\"" << endl;
	locHeaderStream << "#include \"TH2I.h\"" << endl;
	locHeaderStream << "#include \"TH1D.h\"" << endl;
	locHeaderStream << "#include \"TH2D.h\"" << endl;
	locHeaderStream << "#include \"TMath.h\"" << endl;
	locHeaderStream << endl;
	locHeaderStream << "class " << locSelectorName << " : public DSelector" << endl;
	locHeaderStream << "{" << endl;
	locHeaderStream << "	public:" << endl;
	locHeaderStream << endl;
	locHeaderStream << "		" << locSelectorName << "(TTree* locTree = NULL) : DSelector(locTree){}" << endl;
	locHeaderStream << "		virtual ~" << locSelectorName << "(){}" << endl;
	locHeaderStream << endl;
	locHeaderStream << "		void Init(TTree *tree);" << endl;
	locHeaderStream << "		Bool_t Process(Long64_t entry);" << endl;
	locHeaderStream << endl;
	locHeaderStream << "	private:" << endl;
	locHeaderStream << endl;
	locHeaderStream << "		void Get_ComboWrappers(void);" << endl;
	locHeaderStream << "		void Finalize(void);" << endl;
	locHeaderStream << endl;
	locHeaderStream << "		// BEAM POLARIZATION INFORMATION" << endl;
	locHeaderStream << "		UInt_t dPreviousRunNumber;" << endl;
	locHeaderStream << "		bool dIsPolarizedFlag; //else is AMO" << endl;
	locHeaderStream << "		bool dIsPARAFlag; //else is PERP or AMO" << endl;
	locHeaderStream << endl;
	locHeaderStream << "		// ANALYZE CUT ACTIONS" << endl;
	locHeaderStream << "		// // Automatically makes mass histograms where one cut is missing" << endl;
	locHeaderStream << "		DHistogramAction_AnalyzeCutActions* dAnalyzeCutActions;" << endl;
	locHeaderStream << endl;
	locHeaderStream << "		//CREATE REACTION-SPECIFIC PARTICLE ARRAYS" << endl;
	locHeaderStream << endl;

	//print particle step, particle wrapper declarations
	map<int, map<int, pair<Particle_t, string> > >::iterator locStepIterator = locComboInfoMap.begin();
	for(; locStepIterator != locComboInfoMap.end(); ++locStepIterator)
	{
		int locStepIndex = locStepIterator->first;
		locHeaderStream << "		//Step " << locStepIndex << endl;
		locHeaderStream << "		DParticleComboStep* dStep" << locStepIndex << "Wrapper;" << endl;

		map<int, pair<Particle_t, string> >& locStepInfoMap = locStepIterator->second;
		map<int, pair<Particle_t, string> >::iterator locParticleIterator = locStepInfoMap.begin();
		for(; locParticleIterator != locStepInfoMap.end(); ++locParticleIterator)
		{
			int locParticleIndex = locParticleIterator->first;
			Particle_t locPID = locParticleIterator->second.first;
			string locParticleName = locParticleIterator->second.second;

			if(locPID == Unknown)
				continue;
			else if(locParticleName == "ComboBeam")
				locHeaderStream << "		DBeamParticle* dComboBeamWrapper;" << endl;
			else if(locParticleName.substr(0, 6) == "Target")
				continue;
			else if(locParticleName.substr(0, 8) == "Decaying")
			{
				string locBranchName = locParticleName + string("__P4_KinFit");
				if((locTreeInterface->Get_Branch(locBranchName) != NULL) && (locParticleIndex < 0)) //else not reconstructed or in final state
					locHeaderStream << "		DKinematicData* d" << locParticleName << "Wrapper;" << endl;
			}
			else if(locParticleName.substr(0, 7) == "Missing")
			{
				string locBranchName = locParticleName + string("__P4_KinFit");
				if(locTreeInterface->Get_Branch(locBranchName) != NULL) //else not reconstructed
					locHeaderStream << "		DKinematicData* d" << locParticleName << "Wrapper;" << endl;
			}
			else if(ParticleCharge(locPID) != 0)
				locHeaderStream << "		DChargedTrackHypothesis* d" << locParticleName << "Wrapper;" << endl;
			else
				locHeaderStream << "		DNeutralParticleHypothesis* d" << locParticleName << "Wrapper;" << endl;
		}
		locHeaderStream << endl;
	}

	//resume
	locHeaderStream << "		// DEFINE YOUR HISTOGRAMS HERE" << endl;
	locHeaderStream << "		// EXAMPLES:" << endl;
	locHeaderStream << "		TH1I* dHist_MissingMassSquared;" << endl;
	locHeaderStream << "		TH1I* dHist_BeamEnergy;" << endl;
	//writing out all the self-defined histograms
	locHeaderStream << "		/*************** Begin of list of your histograms definition **************/" << endl;
	if(locConfiguration != "0") Add_HistogramDefinition(locHeaderStream, locConfiguration);

	locHeaderStream << "	ClassDef(" << locSelectorName << ", 0);" << endl;
	locHeaderStream << "};" << endl;
	locHeaderStream << endl;
	locHeaderStream << "void " << locSelectorName << "::Get_ComboWrappers(void)" << endl;
	locHeaderStream << "{" << endl;

	//print particle step, particle wrapper assignments
	for(locStepIterator = locComboInfoMap.begin(); locStepIterator != locComboInfoMap.end(); ++locStepIterator)
	{
		int locStepIndex = locStepIterator->first;
		if(locStepIndex != 0)
			locHeaderStream << endl;
		locHeaderStream << "	//Step " << locStepIndex << endl;
		locHeaderStream << "	dStep" << locStepIndex << "Wrapper = dComboWrapper->Get_ParticleComboStep(" << locStepIndex << ");" << endl;

		map<int, pair<Particle_t, string> >& locStepInfoMap = locStepIterator->second;
		map<int, pair<Particle_t, string> >::iterator locParticleIterator = locStepInfoMap.begin();
		for(; locParticleIterator != locStepInfoMap.end(); ++locParticleIterator)
		{
			int locParticleIndex = locParticleIterator->first;
			Particle_t locPID = locParticleIterator->second.first;
			string locParticleName = locParticleIterator->second.second;

			if(locPID == Unknown)
				continue;
			else if(locParticleName == "ComboBeam")
				locHeaderStream << "	dComboBeamWrapper = static_cast<DBeamParticle*>(dStep" << locStepIndex << "Wrapper->Get_InitialParticle());" << endl;
			else if(locParticleName.substr(0, 6) == "Target")
				continue;
			else if(locParticleName.substr(0, 8) == "Decaying")
			{
				string locBranchName = locParticleName + string("__P4_KinFit");
				if((locTreeInterface->Get_Branch(locBranchName) != NULL) && (locParticleIndex < 0)) //else not reconstructed
					locHeaderStream << "	d" << locParticleName << "Wrapper = dStep" << locStepIndex << "Wrapper->Get_InitialParticle();" << endl;
			}
			else if(locParticleName.substr(0, 7) == "Missing")
			{
				string locBranchName = locParticleName + string("__P4_KinFit");
				if(locTreeInterface->Get_Branch(locBranchName) != NULL) //else not reconstructed
					locHeaderStream << "	d" << locParticleName << "Wrapper = dStep" << locStepIndex << "Wrapper->Get_FinalParticle(" << locParticleIndex << ");" << endl;
			}
			else if(ParticleCharge(locPID) != 0)
				locHeaderStream << "	d" << locParticleName << "Wrapper = static_cast<DChargedTrackHypothesis*>(dStep" << locStepIndex << "Wrapper->Get_FinalParticle(" << locParticleIndex << "));" << endl;
			else
				locHeaderStream << "	d" << locParticleName << "Wrapper = static_cast<DNeutralParticleHypothesis*>(dStep" << locStepIndex << "Wrapper->Get_FinalParticle(" << locParticleIndex << "));" << endl;
		}
	}

	//resume
	locHeaderStream << "}" << endl;
	locHeaderStream << endl;
	locHeaderStream << "#endif // " << locSelectorName << "_h" << endl;

	locHeaderStream.close();
}

void Print_SourceFile(string locSelectorBaseName, DTreeInterface* locTreeInterface, map<int, map<int, pair<Particle_t, string> > >& locComboInfoMap, string locConfiguration, bool loc_unconstrained_flag)
{
	string locSelectorName = string("DSelector_") + locSelectorBaseName;
	string locSourceName = locSelectorName + string(".C");
	ofstream locOfStream;
	locOfStream.open(locSourceName.c_str());

	if(loc_unconstrained_flag) locOfStream << "#define LOC_UNCONSTRAINED_FLAG 1" << endl;
	locOfStream << "#include \"" << locSelectorName << ".h\"" << endl;
	locOfStream << endl;
	locOfStream << "void " << locSelectorName << "::Init(TTree *locTree)" << endl;
	locOfStream << "{" << endl;
	locOfStream << "	// USERS: IN THIS FUNCTION, ONLY MODIFY SECTIONS WITH A \"USER\" OR \"EXAMPLE\" LABEL. LEAVE THE REST ALONE." << endl;
	locOfStream << endl;
	locOfStream << "	// The Init() function is called when the selector needs to initialize a new tree or chain." << endl;
	locOfStream << "	// Typically here the branch addresses and branch pointers of the tree will be set." << endl;
	locOfStream << "	// Init() will be called many times when running on PROOF (once per file to be processed)." << endl;
	locOfStream << endl;
	locOfStream << "	//USERS: SET OUTPUT FILE NAME //can be overriden by user in PROOF" << endl;
	locOfStream << "	dOutputFileName = \"" << locSelectorBaseName << ".root\"; //\"\" for none" << endl;
	locOfStream << "	dOutputTreeFileName = \"\"; //\"\" for none" << endl;
	locOfStream << "	dFlatTreeFileName = \"\"; //output flat tree (one combo per tree entry), \"\" for none" << endl;
	locOfStream << "	dFlatTreeName = \"\"; //if blank, default name will be chosen" << endl;
	locOfStream << endl;
	locOfStream << "	//Because this function gets called for each TTree in the TChain, we must be careful:" << endl;
	locOfStream << "		//We need to re-initialize the tree interface & branch wrappers, but don't want to recreate histograms" << endl;
	locOfStream << "	bool locInitializedPriorFlag = dInitializedFlag; //save whether have been initialized previously" << endl;
	locOfStream << "	DSelector::Init(locTree); //This must be called to initialize wrappers for each new TTree" << endl;
	locOfStream << "	//gDirectory now points to the output file with name dOutputFileName (if any)" << endl;
	locOfStream << "	if(locInitializedPriorFlag)" << endl;
	locOfStream << "		return; //have already created histograms, etc. below: exit" << endl;
	locOfStream << endl;
	locOfStream << "	Get_ComboWrappers();" << endl;
	locOfStream << "	dPreviousRunNumber = 0;" << endl;
	locOfStream << endl;
	locOfStream << "	/*********************************** EXAMPLE USER INITIALIZATION: ANALYSIS ACTIONS **********************************/" << endl;
	locOfStream << endl;

	

	locOfStream << "	// EXAMPLE: Create deque for histogramming particle masses:" << endl;
	locOfStream << "	// // For histogramming the phi mass in phi -> K+ K-" << endl;
	locOfStream << "	// // Be sure to change this and dAnalyzeCutActions to match reaction" << endl;
	locOfStream << "	std::deque<Particle_t> MyPhi;" << endl;
	locOfStream << "	MyPhi.push_back(KPlus); MyPhi.push_back(KMinus);" << endl;
	locOfStream << endl;
	locOfStream << "	//ANALYSIS ACTIONS: //Executed in order if added to dAnalysisActions" << endl;
	locOfStream << "	//false/true below: use measured/kinfit data" << endl;
	locOfStream << endl;
	locOfStream << "	//PID" << endl;
	locOfStream << "	//dAnalysisActions.push_back(new DHistogramAction_ParticleID(dComboWrapper, false));" << endl;
	locOfStream << "	//below: value: +/- N ns, Unknown: All PIDs, SYS_NULL: all timing systems" << endl;
	locOfStream << "	//dAnalysisActions.push_back(new DCutAction_PIDDeltaT(dComboWrapper, false, 0.5, KPlus, SYS_BCAL));" << endl;
	locOfStream << endl;
	locOfStream << "	//MASSES" << endl;
	locOfStream << "	//dAnalysisActions.push_back(new DHistogramAction_InvariantMass(dComboWrapper, false, Lambda, 1000, 1.0, 1.2, \"Lambda\"));" << endl;
	locOfStream << "	//dAnalysisActions.push_back(new DHistogramAction_MissingMassSquared(dComboWrapper, false, 1000, -0.1, 0.1));" << endl;
	locOfStream << endl;
	locOfStream << "	//KINFIT RESULTS" << endl;
	locOfStream << "	dAnalysisActions.push_back(new DHistogramAction_KinFitResults(dComboWrapper));" << endl;
	locOfStream << endl;
	locOfStream << "	//CUT MISSING MASS" << endl;
	locOfStream << "	//dAnalysisActions.push_back(new DCutAction_MissingMassSquared(dComboWrapper, false, -0.03, 0.02));" << endl;
	locOfStream << endl;
	locOfStream << "	//BEAM ENERGY" << endl;
	locOfStream << "	dAnalysisActions.push_back(new DHistogramAction_BeamEnergy(dComboWrapper, false));" << endl;
	locOfStream << "	//dAnalysisActions.push_back(new DCutAction_BeamEnergy(dComboWrapper, false, 8.4, 9.05));" << endl;
	locOfStream << endl;
	locOfStream << "	//KINEMATICS" << endl;
	locOfStream << "	//dAnalysisActions.push_back(new DHistogramAction_ParticleComboKinematics(dComboWrapper, false));" << endl;
	locOfStream << endl;
	if(locConfiguration != "0") Add_Action(locOfStream, locConfiguration);
	locOfStream << endl;
	locOfStream << "	// ANALYZE CUT ACTIONS" << endl;
	locOfStream << "	// // Change MyPhi to match reaction" << endl;
	locOfStream << "	dAnalyzeCutActions = new DHistogramAction_AnalyzeCutActions( dAnalysisActions, dComboWrapper, false, 0, MyPhi, 1000, 0.9, 2.4, \"CutActionEffect\" );" << endl;
	locOfStream << endl;
	locOfStream << "	//INITIALIZE ACTIONS" << endl;
	locOfStream << "	//If you create any actions that you want to run manually (i.e. don't add to dAnalysisActions), be sure to initialize them here as well" << endl;
	locOfStream << "	Initialize_Actions();" << endl;
	locOfStream << "	dAnalyzeCutActions->Initialize(); // manual action, must call Initialize()" << endl;
	locOfStream << endl;
	locOfStream << "	/******************************** EXAMPLE USER INITIALIZATION: STAND-ALONE HISTOGRAMS *******************************/" << endl;
	locOfStream << endl;
	locOfStream << "	//EXAMPLE MANUAL HISTOGRAMS:" << endl;
	locOfStream << "	dHist_MissingMassSquared = new TH1I(\"MissingMassSquared\", \";Missing Mass Squared (GeV/c^{2})^{2}\", 600, -0.06, 0.06);" << endl;
	locOfStream << "	dHist_BeamEnergy = new TH1I(\"BeamEnergy\", \";Beam Energy (GeV)\", 600, 0.0, 12.0);" << endl;
	locOfStream << endl;

	//writing out all the self-defined histograms
	if(locConfiguration != "0") locOfStream << "	/*************** Begin of histogram initialization **************/" << endl;
	if(locConfiguration != "0") Add_HistogramInitialization(locOfStream, locConfiguration);


	locOfStream << "	/************************** EXAMPLE USER INITIALIZATION: CUSTOM OUTPUT BRANCHES - MAIN TREE *************************/" << endl;
	locOfStream << endl;
	locOfStream << "	//EXAMPLE MAIN TREE CUSTOM BRANCHES (OUTPUT ROOT FILE NAME MUST FIRST BE GIVEN!!!! (ABOVE: TOP)):" << endl;
	locOfStream << "	//The type for the branch must be included in the brackets" << endl;
	locOfStream << "	//1st function argument is the name of the branch" << endl;
	locOfStream << "	//2nd function argument is the name of the branch that contains the size of the array (for fundamentals only)" << endl;
	locOfStream << "	/*" << endl;
	locOfStream << "	dTreeInterface->Create_Branch_Fundamental<Int_t>(\"my_int\"); //fundamental = char, int, float, double, etc." << endl;
	locOfStream << "	dTreeInterface->Create_Branch_FundamentalArray<Int_t>(\"my_int_array\", \"my_int\");" << endl;
	locOfStream << "	dTreeInterface->Create_Branch_FundamentalArray<Float_t>(\"my_combo_array\", \"NumCombos\");" << endl;
	locOfStream << "	dTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>(\"my_p4\");" << endl;
	locOfStream << "	dTreeInterface->Create_Branch_ClonesArray<TLorentzVector>(\"my_p4_array\");" << endl;
	locOfStream << "	*/" << endl;
	locOfStream << endl;
	locOfStream << "	/************************** EXAMPLE USER INITIALIZATION: CUSTOM OUTPUT BRANCHES - FLAT TREE *************************/" << endl;
	locOfStream << endl;
	locOfStream << "	//EXAMPLE FLAT TREE CUSTOM BRANCHES (OUTPUT ROOT FILE NAME MUST FIRST BE GIVEN!!!! (ABOVE: TOP)):" << endl;
	locOfStream << "	//The type for the branch must be included in the brackets" << endl;
	locOfStream << "	//1st function argument is the name of the branch" << endl;
	locOfStream << "	//2nd function argument is the name of the branch that contains the size of the array (for fundamentals only)" << endl;
	locOfStream << "	/*" << endl;
	locOfStream << "	dFlatTreeInterface->Create_Branch_Fundamental<Int_t>(\"flat_my_int\"); //fundamental = char, int, float, double, etc." << endl;
	locOfStream << "	dFlatTreeInterface->Create_Branch_FundamentalArray<Int_t>(\"flat_my_int_array\", \"flat_my_int\");" << endl;
	locOfStream << "	dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>(\"flat_my_p4\");" << endl;
	locOfStream << "	dFlatTreeInterface->Create_Branch_ClonesArray<TLorentzVector>(\"flat_my_p4_array\");" << endl;
	locOfStream << "	*/" << endl;
	locOfStream << endl;
	locOfStream << "	/************************************* ADVANCED EXAMPLE: CHOOSE BRANCHES TO READ ************************************/" << endl;
	locOfStream << endl;
	locOfStream << "	//TO SAVE PROCESSING TIME" << endl;
	locOfStream << "		//If you know you don't need all of the branches/data, but just a subset of it, you can speed things up" << endl;
	locOfStream << "		//By default, for each event, the data is retrieved for all branches" << endl;
	locOfStream << "		//If you know you only need data for some branches, you can skip grabbing data from the branches you don't need" << endl;
	locOfStream << "		//Do this by doing something similar to the commented code below" << endl;
	locOfStream << endl;
	locOfStream << "	//dTreeInterface->Clear_GetEntryBranches(); //now get none" << endl;
	locOfStream << "	//dTreeInterface->Register_GetEntryBranch(\"Proton__P4\"); //manually set the branches you want" << endl;
	locOfStream << "}" << endl;
	locOfStream << endl;
	locOfStream << "Bool_t " << locSelectorName << "::Process(Long64_t locEntry)" << endl;
	locOfStream << "{" << endl;
	locOfStream << "	// The Process() function is called for each entry in the tree. The entry argument" << endl;
	locOfStream << "	// specifies which entry in the currently loaded tree is to be processed." << endl;
	locOfStream << "	//" << endl;
	locOfStream << "	// This function should contain the \"body\" of the analysis. It can contain" << endl;
	locOfStream << "	// simple or elaborate selection criteria, run algorithms on the data" << endl;
	locOfStream << "	// of the event and typically fill histograms." << endl;
	locOfStream << "	//" << endl;
	locOfStream << "	// The processing can be stopped by calling Abort()." << endl;
	locOfStream << "	// Use fStatus to set the return value of TTree::Process()." << endl;
	locOfStream << "	// The return value is currently not used." << endl;
	locOfStream << endl;
	locOfStream << "	//CALL THIS FIRST" << endl;
	locOfStream << "	DSelector::Process(locEntry); //Gets the data from the tree for the entry" << endl;
	locOfStream << "	//cout << \"RUN \" << Get_RunNumber() << \", EVENT \" << Get_EventNumber() << endl;" << endl;
	locOfStream << "	//TLorentzVector locProductionX4 = Get_X4_Production();" << endl;
	locOfStream << endl;
	locOfStream << "	/******************************************** GET POLARIZATION ORIENTATION ******************************************/" << endl;
	locOfStream << endl;
	locOfStream << "	//Only if the run number changes" << endl;
	locOfStream << "	//RCDB environment must be setup in order for this to work! (Will return false otherwise)" << endl;
	locOfStream << "	UInt_t locRunNumber = Get_RunNumber();" << endl;
	locOfStream << "	if(locRunNumber != dPreviousRunNumber)" << endl;
	locOfStream << "	{" << endl;
	locOfStream << "		dIsPolarizedFlag = dAnalysisUtilities.Get_IsPolarizedBeam(locRunNumber, dIsPARAFlag);" << endl;
	locOfStream << "		dPreviousRunNumber = locRunNumber;" << endl;
	locOfStream << "	}" << endl;
	locOfStream << endl;
	locOfStream << "	/********************************************* SETUP UNIQUENESS TRACKING ********************************************/" << endl;
	locOfStream << endl;
	locOfStream << "	//ANALYSIS ACTIONS: Reset uniqueness tracking for each action" << endl;
	locOfStream << "	//For any actions that you are executing manually, be sure to call Reset_NewEvent() on them here" << endl;
	locOfStream << "	Reset_Actions_NewEvent();" << endl;
	locOfStream << "	dAnalyzeCutActions->Reset_NewEvent(); // manual action, must call Reset_NewEvent()" << endl;
	locOfStream << endl;
	locOfStream << "	//PREVENT-DOUBLE COUNTING WHEN HISTOGRAMMING" << endl;
	locOfStream << "		//Sometimes, some content is the exact same between one combo and the next" << endl;
	locOfStream << "			//e.g. maybe two combos have different beam particles, but the same data for the final-state" << endl;
	locOfStream << "		//When histogramming, you don\'t want to double-count when this happens: artificially inflates your signal (or background)" << endl;
	locOfStream << "		//So, for each quantity you histogram, keep track of what particles you used (for a given combo)" << endl;
	locOfStream << "		//Then for each combo, just compare to what you used before, and make sure it\'s unique" << endl;
	locOfStream << endl;
	locOfStream << "	//EXAMPLE 1: Particle-specific info:" << endl;
	locOfStream << "	set<Int_t> locUsedSoFar_BeamEnergy; //Int_t: Unique ID for beam particles. set: easy to use, fast to search" << endl;
	locOfStream << endl;
	locOfStream << "	//EXAMPLE 2: Combo-specific info:" << endl;
	locOfStream << "		//In general: Could have multiple particles with the same PID: Use a set of Int_t\'s" << endl;
	locOfStream << "		//In general: Multiple PIDs, so multiple sets: Contain within a map" << endl;
	locOfStream << "		//Multiple combos: Contain maps within a set (easier, faster to search)" << endl;
	locOfStream << "	set<map<Particle_t, set<Int_t> > > locUsedSoFar_MissingMass;" << endl;
	locOfStream << endl;
	locOfStream << "	//INSERT USER ANALYSIS UNIQUENESS TRACKING HERE" << endl;

	if(locConfiguration != "0") Add_UniquenessTrackingDefinition(locOfStream, locConfiguration);
	locOfStream << endl;
	locOfStream << "	/**************************************** EXAMPLE: FILL CUSTOM OUTPUT BRANCHES **************************************/" << endl;
	locOfStream << endl;
	locOfStream << "	/*" << endl;
	locOfStream << "	Int_t locMyInt = 7;" << endl;
	locOfStream << "	dTreeInterface->Fill_Fundamental<Int_t>(\"my_int\", locMyInt);" << endl;
	locOfStream << endl;
	locOfStream << "	TLorentzVector locMyP4(4.0, 3.0, 2.0, 1.0);" << endl;
	locOfStream << "	dTreeInterface->Fill_TObject<TLorentzVector>(\"my_p4\", locMyP4);" << endl;
	locOfStream << endl;
	locOfStream << "	for(int loc_i = 0; loc_i < locMyInt; ++loc_i)" << endl;
	locOfStream << "		dTreeInterface->Fill_Fundamental<Int_t>(\"my_int_array\", 3*loc_i, loc_i); //2nd argument = value, 3rd = array index" << endl;
	locOfStream << "	*/" << endl;
	locOfStream << endl;
	locOfStream << "	/************************************************* LOOP OVER COMBOS *************************************************/" << endl;
	locOfStream << endl;
	locOfStream << "	//Loop over combos" << endl;
	locOfStream << "	for(UInt_t loc_i = 0; loc_i < Get_NumCombos(); ++loc_i)" << endl;
	locOfStream << "	{" << endl;
	locOfStream << "		//Set branch array indices for combo and all combo particles" << endl;
	locOfStream << "		dComboWrapper->Set_ComboIndex(loc_i);" << endl;
	locOfStream << endl;
	locOfStream << "		// Is used to indicate when combos have been cut" << endl;
	locOfStream << "		if(dComboWrapper->Get_IsComboCut()) // Is false when tree originally created" << endl;
	locOfStream << "			continue; // Combo has been cut previously" << endl;
	locOfStream << endl;
	locOfStream << "		/********************************************** GET PARTICLE INDICES *********************************************/" << endl;
	locOfStream << endl;
	locOfStream << "		//Used for tracking uniqueness when filling histograms, and for determining unused particles" << endl;
	locOfStream << endl;

	//print particle indices
	map<int, map<int, pair<Particle_t, string> > >::iterator locStepIterator = locComboInfoMap.begin();
	for(; locStepIterator != locComboInfoMap.end(); ++locStepIterator)
	{
		int locStepIndex = locStepIterator->first;
		locOfStream << "		//Step " << locStepIndex << endl;

		map<int, pair<Particle_t, string> >& locStepInfoMap = locStepIterator->second;
		map<int, pair<Particle_t, string> >::iterator locParticleIterator = locStepInfoMap.begin();
		for(; locParticleIterator != locStepInfoMap.end(); ++locParticleIterator)
		{
			Particle_t locPID = locParticleIterator->second.first;
			string locParticleName = locParticleIterator->second.second;

			if(locPID == Unknown)
				continue;
			else if(locParticleName == "ComboBeam")
				locOfStream << "		Int_t locBeamID = dComboBeamWrapper->Get_BeamID();" << endl;
			else if(locParticleName.substr(0, 6) == "Target")
				continue;
			else if(locParticleName.substr(0, 8) == "Decaying")
				continue;
			else if(locParticleName.substr(0, 7) == "Missing")
				continue;
			else if(ParticleCharge(locPID) != 0)
				locOfStream << "		Int_t loc" << locParticleName << "TrackID = d" << locParticleName << "Wrapper->Get_TrackID();" << endl;
			else
				locOfStream << "		Int_t loc" << locParticleName << "NeutralID = d" << locParticleName << "Wrapper->Get_NeutralID();" << endl;
		}
		locOfStream << endl;
	}
	locOfStream << "		/*********************************************** GET FOUR-MOMENTUM and FOUR-POSITION **********************************************/" << endl;
	locOfStream << endl;

	//get p4's
	locOfStream << "		// Get P4\'s: //is kinfit if kinfit performed, else is measured" << endl;
	locOfStream << "		//dTargetP4 is target p4" << endl;
	for(locStepIterator = locComboInfoMap.begin(); locStepIterator != locComboInfoMap.end(); ++locStepIterator)
	{
		int locStepIndex = locStepIterator->first;
		locOfStream << "		//Step " << locStepIndex << endl;

		map<int, pair<Particle_t, string> >& locStepInfoMap = locStepIterator->second;
		map<int, pair<Particle_t, string> >::iterator locParticleIterator = locStepInfoMap.begin();
		for(; locParticleIterator != locStepInfoMap.end(); ++locParticleIterator)
		{
			int locParticleIndex = locParticleIterator->first;
			Particle_t locPID = locParticleIterator->second.first;
			string locParticleName = locParticleIterator->second.second;

			if(locPID == Unknown)
				continue;
			else if(locParticleName == "ComboBeam")
				locOfStream << "		TLorentzVector locBeamP4 = dComboBeamWrapper->Get_P4();" << endl;
			else if(locParticleName.substr(0, 6) == "Target")
				continue;
			else if(locParticleName.substr(0, 8) == "Decaying")
			{
				string locBranchName = locParticleName + string("__P4_KinFit");
				if((locTreeInterface->Get_Branch(locBranchName) != NULL) && (locParticleIndex < 0)) //else not reconstructed
					locOfStream << "		TLorentzVector loc" << locParticleName << "P4 = d" << locParticleName << "Wrapper->Get_P4();" << endl;
			}
			else if(locParticleName.substr(0, 7) == "Missing")
			{
				string locBranchName = locParticleName + string("__P4_KinFit");
				if(locTreeInterface->Get_Branch(locBranchName) != NULL) //else not reconstructed
					locOfStream << "		TLorentzVector loc" << locParticleName << "P4 = d" << locParticleName << "Wrapper->Get_P4();" << endl;
			}
			else//detected
			{
				locOfStream << "		TLorentzVector loc" << locParticleName << "P4 = d" << locParticleName << "Wrapper->Get_P4();" << endl;
				if(locStepIndex != 0) locOfStream << "		TLorentzVector loc" << locParticleName << "X4 = d" << locParticleName << "Wrapper->Get_X4();" << endl;

			}
				
		}
	}
	locOfStream << endl;

	//get measured p4's
	locOfStream << "		// Get Measured P4\'s:" << endl;
	for(locStepIterator = locComboInfoMap.begin(); locStepIterator != locComboInfoMap.end(); ++locStepIterator)
	{
		int locStepIndex = locStepIterator->first;
		locOfStream << "		//Step " << locStepIndex << endl;

		map<int, pair<Particle_t, string> >& locStepInfoMap = locStepIterator->second;
		map<int, pair<Particle_t, string> >::iterator locParticleIterator = locStepInfoMap.begin();
		for(; locParticleIterator != locStepInfoMap.end(); ++locParticleIterator)
		{
			Particle_t locPID = locParticleIterator->second.first;
			string locParticleName = locParticleIterator->second.second;

			if(locPID == Unknown)
				continue;
			else if(locParticleName == "ComboBeam")
				locOfStream << "		TLorentzVector locBeamP4_Measured = dComboBeamWrapper->Get_P4_Measured();" << endl;
			else if(locParticleName.substr(0, 6) == "Target")
				continue;
			else if(locParticleName.substr(0, 8) == "Decaying")
				continue;
			else if(locParticleName.substr(0, 7) == "Missing")
				continue;
			else //detected
				locOfStream << "		TLorentzVector loc" << locParticleName << "P4_Measured = d" << locParticleName << "Wrapper->Get_P4_Measured();" << endl;
				if(locStepIndex != 0) locOfStream << "		TLorentzVector loc" << locParticleName << "X4_Measured = d" << locParticleName << "Wrapper->Get_X4_Measured();" << endl;
		}
	}

	locOfStream << endl;
	if(locConfiguration != "0") Add_LorentzBoost(locOfStream, locConfiguration, locTreeInterface, locComboInfoMap);

	locOfStream << "		/********************************************* COMBINE FOUR-MOMENTUM ********************************************/" << endl;
	locOfStream << endl;
	locOfStream << "		// DO YOUR STUFF HERE" << endl;
	locOfStream << endl;
	locOfStream << "		// Combine 4-vectors" << endl;
	locOfStream << "		TLorentzVector locMissingP4_Measured = locBeamP4_Measured + dTargetP4;" << endl;
	locOfStream << "		locMissingP4_Measured -= ";

	//calc missing p4
	bool locFirstFlag = true;
	for(locStepIterator = locComboInfoMap.begin(); locStepIterator != locComboInfoMap.end(); ++locStepIterator)
	{
		map<int, pair<Particle_t, string> >& locStepInfoMap = locStepIterator->second;
		map<int, pair<Particle_t, string> >::iterator locParticleIterator = locStepInfoMap.begin();
		for(; locParticleIterator != locStepInfoMap.end(); ++locParticleIterator)
		{
			Particle_t locPID = locParticleIterator->second.first;
			string locParticleName = locParticleIterator->second.second;

			if(locPID == Unknown)
				continue;
			else if(locParticleName == "ComboBeam")
				continue;
			else if(locParticleName.substr(0, 6) == "Target")
				continue;
			else if(locParticleName.substr(0, 8) == "Decaying")
				continue;
			else if(locParticleName.substr(0, 7) == "Missing")
				continue;

			//detected
			if(!locFirstFlag)
				locOfStream << " + ";
			locFirstFlag = false;
			locOfStream << "loc" << locParticleName << "P4_Measured";
		}
	}
	locOfStream << ";" << endl;
	locOfStream << endl;
	if(locConfiguration == "0")
{
	locOfStream << "		/******************************************** EXECUTE ANALYSIS ACTIONS *******************************************/" << endl;
	locOfStream << endl;
	locOfStream << "		// Loop through the analysis actions, executing them in order for the active particle combo" << endl;
	locOfStream <<"		dAnalyzeCutActions->Perform_Action(); // Must be executed before Execute_Actions()" << endl;
	locOfStream << "		if(!Execute_Actions()) //if the active combo fails a cut, IsComboCutFlag automatically set" << endl;
	locOfStream << "			continue;" << endl;
	locOfStream << endl;
	locOfStream << "		//if you manually execute any actions, and it fails a cut, be sure to call:" << endl;
	locOfStream << "			//dComboWrapper->Set_IsComboCut(true);" << endl;
	locOfStream << endl;
}
	if(locConfiguration != "0") locOfStream << "		/**************************************** KINEMATIC CALCULATION **************************************/" << endl;
	if(locConfiguration != "0") locOfStream << endl;
	if(locConfiguration != "0") Add_KinematicCalculation(locOfStream, locConfiguration);
	if(locConfiguration != "0")
{
	locOfStream << "		/******************************************** EXECUTE ANALYSIS ACTIONS *******************************************/" << endl;
	locOfStream << endl;
	locOfStream << "		// Loop through the analysis actions, executing them in order for the active particle combo" << endl;
	locOfStream <<"		dAnalyzeCutActions->Perform_Action(); // Must be executed before Execute_Actions()" << endl;
	locOfStream << "		if(!Execute_Actions()) //if the active combo fails a cut, IsComboCutFlag automatically set" << endl;
	locOfStream << "			continue;" << endl;
	locOfStream << endl;
	locOfStream << "		//if you manually execute any actions, and it fails a cut, be sure to call:" << endl;
	locOfStream << "			//dComboWrapper->Set_IsComboCut(true);" << endl;
	locOfStream << endl;
}

	locOfStream << "		/**************************************** EXAMPLE: FILL CUSTOM OUTPUT BRANCHES **************************************/" << endl;
	locOfStream << endl;
	locOfStream << "		/*" << endl;
	locOfStream << "		TLorentzVector locMyComboP4(8.0, 7.0, 6.0, 5.0);" << endl;
	locOfStream << "		//for arrays below: 2nd argument is value, 3rd is array index" << endl;
	locOfStream << "		//NOTE: By filling here, AFTER the cuts above, some indices won't be updated (and will be whatever they were from the last event)" << endl;
	locOfStream << "			//So, when you draw the branch, be sure to cut on \"IsComboCut\" to avoid these." << endl;
	locOfStream << "		dTreeInterface->Fill_Fundamental<Float_t>(\"my_combo_array\", -2*loc_i, loc_i);" << endl;
	locOfStream << "		dTreeInterface->Fill_TObject<TLorentzVector>(\"my_p4_array\", locMyComboP4, loc_i);" << endl;
	locOfStream << "		*/" << endl;
	locOfStream << endl;
	locOfStream << "		/**************************************** EXAMPLE: HISTOGRAM BEAM ENERGY *****************************************/" << endl;
	locOfStream << endl;
	locOfStream << "		//Histogram beam energy (if haven\'t already)" << endl;
	locOfStream << "		if(locUsedSoFar_BeamEnergy.find(locBeamID) == locUsedSoFar_BeamEnergy.end())" << endl;
	locOfStream << "		{" << endl;
	locOfStream << "			dHist_BeamEnergy->Fill(locBeamP4.E());" << endl;
	locOfStream << "			locUsedSoFar_BeamEnergy.insert(locBeamID);" << endl;
	locOfStream << "		}" << endl;
	locOfStream << endl;
	locOfStream << "		/**************************************** EXAMPLE: MISSINGMASS UNIQUENESS TRACKING  ************************************/" << endl;
	locOfStream << endl;
	locOfStream << "		//Missing Mass Squared" << endl;
	locOfStream << "		double locMissingMassSquared = locMissingP4_Measured.M2();" << endl;
	locOfStream << endl;
	locOfStream << "		//Uniqueness tracking: Build the map of particles used for the missing mass" << endl;
	locOfStream << "			//For beam: Don\'t want to group with final-state photons. Instead use \"Unknown\" PID (not ideal, but it\'s easy)." << endl;
	locOfStream << "		map<Particle_t, set<Int_t> > locUsedThisCombo_MissingMass;" << endl;

	//insert uniqueness tracking
	//print particle indices
	for(locStepIterator = locComboInfoMap.begin(); locStepIterator != locComboInfoMap.end(); ++locStepIterator)
	{
		map<int, pair<Particle_t, string> >& locStepInfoMap = locStepIterator->second;
		map<int, pair<Particle_t, string> >::iterator locParticleIterator = locStepInfoMap.begin();
		for(; locParticleIterator != locStepInfoMap.end(); ++locParticleIterator)
		{
			Particle_t locPID = locParticleIterator->second.first;
			string locParticleName = locParticleIterator->second.second;

			if(locPID == Unknown)
				continue;
			else if(locParticleName == "ComboBeam")
				locOfStream << "		locUsedThisCombo_MissingMass[Unknown].insert(locBeamID); //beam" << endl;
			else if(locParticleName.substr(0, 6) == "Target")
				continue;
			else if(locParticleName.substr(0, 8) == "Decaying")
				continue;
			else if(locParticleName.substr(0, 7) == "Missing")
				continue;
			else if(ParticleCharge(locPID) != 0)
				locOfStream << "		locUsedThisCombo_MissingMass[" << EnumString(locPID) << "].insert(loc" << locParticleName << "TrackID);" << endl;
			else
				locOfStream << "		locUsedThisCombo_MissingMass[" << EnumString(locPID) << "].insert(loc" << locParticleName << "NeutralID);" << endl;
		}
	}
	locOfStream << endl;

	locOfStream << "		//compare to what\'s been used so far" << endl;
	locOfStream << "		if(locUsedSoFar_MissingMass.find(locUsedThisCombo_MissingMass) == locUsedSoFar_MissingMass.end())" << endl;
	locOfStream << "		{" << endl;
	locOfStream << "			//unique missing mass combo: histogram it, and register this combo of particles" << endl;
	locOfStream << "			dHist_MissingMassSquared->Fill(locMissingMassSquared);" << endl;
	locOfStream << "			locUsedSoFar_MissingMass.insert(locUsedThisCombo_MissingMass);" << endl;
	locOfStream << "		}" << endl;
	locOfStream << endl;
	locOfStream << "		//E.g. Cut" << endl;
	locOfStream << "		//if((locMissingMassSquared < -0.04) || (locMissingMassSquared > 0.04))" << endl;
	locOfStream << "		//{" << endl;
	locOfStream << "		//	dComboWrapper->Set_IsComboCut(true);" << endl;
	locOfStream << "		//	continue;" << endl;
	locOfStream << "		//}" << endl;
	locOfStream << endl;


	if(locConfiguration != "0") locOfStream << "		/****************************************** FILL HISTOGRAM WHILE KEEPING UNIQUENESSTRACKING ******************************************/" << endl;
	if(locConfiguration != "0") locOfStream << endl;

	if(locConfiguration != "0") HistWeight(locOfStream, locConfiguration);
	if(locConfiguration != "0") Add_UniqueHistogramFilling(locOfStream, locConfiguration, locComboInfoMap);

	locOfStream << "		/****************************************** FILL FLAT TREE (IF DESIRED) ******************************************/" << endl;
	locOfStream << endl;
	locOfStream << "		/*" << endl;
	locOfStream << "		//FILL ANY CUSTOM BRANCHES FIRST!!" << endl;
	locOfStream << "		Int_t locMyInt_Flat = 7;" << endl;
	locOfStream << "		dFlatTreeInterface->Fill_Fundamental<Int_t>(\"flat_my_int\", locMyInt_Flat);" << endl;
	locOfStream << endl;
	locOfStream << "		TLorentzVector locMyP4_Flat(4.0, 3.0, 2.0, 1.0);" << endl;
	locOfStream << "		dFlatTreeInterface->Fill_TObject<TLorentzVector>(\"flat_my_p4\", locMyP4_Flat);" << endl;
	locOfStream << endl;
	locOfStream << "		for(int loc_j = 0; loc_j < locMyInt_Flat; ++loc_j)" << endl;
	locOfStream << "		{" << endl;
	locOfStream << "			dFlatTreeInterface->Fill_Fundamental<Int_t>(\"flat_my_int_array\", 3*loc_j, loc_j); //2nd argument = value, 3rd = array index" << endl;
	locOfStream << "			TLorentzVector locMyComboP4_Flat(8.0, 7.0, 6.0, 5.0);" << endl;
	locOfStream << "			dFlatTreeInterface->Fill_TObject<TLorentzVector>(\"flat_my_p4_array\", locMyComboP4_Flat, loc_j);" << endl;
	locOfStream << "		}" << endl;
	locOfStream << "		*/" << endl;
	locOfStream << endl;
	locOfStream << "		//FILL FLAT TREE" << endl;
	locOfStream << "		//Fill_FlatTree(); //for the active combo" << endl;
	locOfStream << "	} // end of combo loop" << endl;
	locOfStream << endl;
	locOfStream << "	//FILL HISTOGRAMS: Num combos / events surviving actions" << endl;
	locOfStream << "	Fill_NumCombosSurvivedHists();" << endl;
	locOfStream << endl;
	locOfStream << "	/******************************************* LOOP OVER THROWN DATA (OPTIONAL) ***************************************/" << endl;
	locOfStream << "/*" << endl;
	locOfStream << "	//Thrown beam: just use directly" << endl;
	locOfStream << "	if(dThrownBeam != NULL)" << endl;
	locOfStream << "		double locEnergy = dThrownBeam->Get_P4().E();" << endl;
	locOfStream << endl;
	locOfStream << "	//Loop over throwns" << endl;
	locOfStream << "	for(UInt_t loc_i = 0; loc_i < Get_NumThrown(); ++loc_i)" << endl;
	locOfStream << "	{" << endl;
	locOfStream << "		//Set branch array indices corresponding to this particle" << endl;
	locOfStream << "		dThrownWrapper->Set_ArrayIndex(loc_i);" << endl;
	locOfStream << endl;
	locOfStream << "		//Do stuff with the wrapper here ..." << endl;
	locOfStream << "	}" << endl;
	locOfStream << "*/" << endl;
	locOfStream << "	/****************************************** LOOP OVER OTHER ARRAYS (OPTIONAL) ***************************************/" << endl;
	locOfStream << "/*" << endl;
	locOfStream << "	//Loop over beam particles (note, only those appearing in combos are present)" << endl;
	locOfStream << "	for(UInt_t loc_i = 0; loc_i < Get_NumBeam(); ++loc_i)" << endl;
	locOfStream << "	{" << endl;
	locOfStream << "		//Set branch array indices corresponding to this particle" << endl;
	locOfStream << "		dBeamWrapper->Set_ArrayIndex(loc_i);" << endl;
	locOfStream << endl;
	locOfStream << "		//Do stuff with the wrapper here ..." << endl;
	locOfStream << "	}" << endl;
	locOfStream << endl;
	locOfStream << "	//Loop over charged track hypotheses" << endl;
	locOfStream << "	for(UInt_t loc_i = 0; loc_i < Get_NumChargedHypos(); ++loc_i)" << endl;
	locOfStream << "	{" << endl;
	locOfStream << "		//Set branch array indices corresponding to this particle" << endl;
	locOfStream << "		dChargedHypoWrapper->Set_ArrayIndex(loc_i);" << endl;
	locOfStream << endl;
	locOfStream << "		//Do stuff with the wrapper here ..." << endl;
	locOfStream << "	}" << endl;
	locOfStream << endl;
	locOfStream << "	//Loop over neutral particle hypotheses" << endl;
	locOfStream << "	for(UInt_t loc_i = 0; loc_i < Get_NumNeutralHypos(); ++loc_i)" << endl;
	locOfStream << "	{" << endl;
	locOfStream << "		//Set branch array indices corresponding to this particle" << endl;
	locOfStream << "		dNeutralHypoWrapper->Set_ArrayIndex(loc_i);" << endl;
	locOfStream << endl;
	locOfStream << "		//Do stuff with the wrapper here ..." << endl;
	locOfStream << "	}" << endl;
	locOfStream << "*/" << endl;
	locOfStream << endl;
	locOfStream << "	/************************************ EXAMPLE: FILL CLONE OF TTREE HERE WITH CUTS APPLIED ************************************/" << endl;
	locOfStream << "/*" << endl;
	locOfStream << "	Bool_t locIsEventCut = true;" << endl;
	locOfStream << "	for(UInt_t loc_i = 0; loc_i < Get_NumCombos(); ++loc_i) {" << endl;
	locOfStream << "		//Set branch array indices for combo and all combo particles" << endl;
	locOfStream << "		dComboWrapper->Set_ComboIndex(loc_i);" << endl;
	locOfStream << "		// Is used to indicate when combos have been cut" << endl;
	locOfStream << "		if(dComboWrapper->Get_IsComboCut())" << endl;
	locOfStream << "			continue;" << endl;
	locOfStream << "		locIsEventCut = false; // At least one combo succeeded" << endl;
	locOfStream << "		break;" << endl;
	locOfStream << "	}" << endl;
	locOfStream << "	if(!locIsEventCut && dOutputTreeFileName != \"\")" << endl;
	locOfStream << "		Fill_OutputTree();" << endl;
	locOfStream << "*/" << endl;
	locOfStream << endl;
	locOfStream << "	return kTRUE;" << endl;
	locOfStream << "}" << endl;
	locOfStream << endl;
	locOfStream << "void " << locSelectorName << "::Finalize(void)" << endl;
	locOfStream << "{" << endl;
	locOfStream << "	//Save anything to output here that you do not want to be in the default DSelector output ROOT file." << endl;
	locOfStream << endl;
	locOfStream << "	//Otherwise, don\'t do anything else (especially if you are using PROOF)." << endl;
	locOfStream << "		//If you are using PROOF, this function is called on each thread," << endl;
	locOfStream << "		//so anything you do will not have the combined information from the various threads." << endl;
	locOfStream << "		//Besides, it is best-practice to do post-processing (e.g. fitting) separately, in case there is a problem." << endl;
	locOfStream << endl;
	locOfStream << "	//DO YOUR STUFF HERE" << endl;
	locOfStream << endl;
	locOfStream << "	//CALL THIS LAST" << endl;
	locOfStream << "	DSelector::Finalize(); //Saves results to the output file" << endl;
	locOfStream << "}" << endl;

	locOfStream.close();
}





void Print_HeaderFile_MCGen(string locSelectorBaseName, DTreeInterface* locTreeInterface, string locConfiguration)
{
	string locSelectorName = string("DSelector_") + locSelectorBaseName;
	string locHeaderName = locSelectorName + string(".h");
	ofstream locHeaderStream;
	locHeaderStream.open(locHeaderName.c_str());

	locHeaderStream << "#ifndef " << locSelectorName << "_h" << endl;
	locHeaderStream << "#define " << locSelectorName << "_h" << endl;
	locHeaderStream << endl;
	locHeaderStream << "#include <iostream>" << endl;
	locHeaderStream << endl;
	locHeaderStream << "#include \"DSelector/DSelector.h\"" << endl;
	locHeaderStream << endl;
	locHeaderStream << "#include \"TH1I.h\"" << endl;
	locHeaderStream << "#include \"TH2I.h\"" << endl;
	locHeaderStream << endl;
	locHeaderStream << "class " << locSelectorName << " : public DSelector" << endl;
	locHeaderStream << "{" << endl;
	locHeaderStream << "	public:" << endl;
	locHeaderStream << endl;
	locHeaderStream << "		" << locSelectorName << "(TTree* locTree = NULL) : DSelector(locTree){}" << endl;
	locHeaderStream << "		virtual ~" << locSelectorName << "(){}" << endl;
	locHeaderStream << endl;
	locHeaderStream << "		void Init(TTree *tree);" << endl;
	locHeaderStream << "		Bool_t Process(Long64_t entry);" << endl;
	locHeaderStream << endl;
	locHeaderStream << "	private:" << endl;
	locHeaderStream << endl;
	locHeaderStream << "		void Finalize(void);" << endl;
	locHeaderStream << endl;
	locHeaderStream << "		// BEAM POLARIZATION INFORMATION" << endl;
	locHeaderStream << "		UInt_t dPreviousRunNumber;" << endl;
	locHeaderStream << "		bool dIsPolarizedFlag; //else is AMO" << endl;
	locHeaderStream << "		bool dIsPARAFlag; //else is PERP or AMO" << endl;
	//writing out all the self-defined histograms
	locHeaderStream << "		/*************** Begin of list of your histograms definition **************/" << endl;
	if(locConfiguration != "0") Add_HistogramDefinition(locHeaderStream, locConfiguration);

	locHeaderStream << endl;
	locHeaderStream << "	ClassDef(" << locSelectorName << ", 0);" << endl;
	locHeaderStream << "};" << endl;
	locHeaderStream << endl;
	locHeaderStream << "#endif // " << locSelectorName << "_h" << endl;

	locHeaderStream.close();
}

void Print_SourceFile_MCGen(string locSelectorBaseName, DTreeInterface* locTreeInterface, string locConfiguration)
{
	string locSelectorName = string("DSelector_") + locSelectorBaseName;
	string locSourceName = locSelectorName + string(".C");
	ofstream locOfStream;
	locOfStream.open(locSourceName.c_str());
	locOfStream << "#define MC_THROWN_FLAG 1" << endl;
	locOfStream << "#include \"" << locSelectorName << ".h\"" << endl;
	locOfStream << endl;
	locOfStream << "void " << locSelectorName << "::Init(TTree *locTree)" << endl;
	locOfStream << "{" << endl;
	locOfStream << "	// USERS: IN THIS FUNCTION, ONLY MODIFY SECTIONS WITH A \"USER\" OR \"EXAMPLE\" LABEL. LEAVE THE REST ALONE." << endl;
	locOfStream << endl;
	locOfStream << "	// The Init() function is called when the selector needs to initialize a new tree or chain." << endl;
	locOfStream << "	// Typically here the branch addresses and branch pointers of the tree will be set." << endl;
	locOfStream << "	// Init() will be called many times when running on PROOF (once per file to be processed)." << endl;
	locOfStream << endl;
	locOfStream << "	//USERS: SET OUTPUT FILE NAME //can be overriden by user in PROOF" << endl;
	locOfStream << "	dOutputFileName = \"" << locSelectorBaseName << ".root\"; //\"\" for none" << endl;
	locOfStream << "	//USERS: SET OUTPUT TREE FILES/NAMES //e.g. binning into separate files for AmpTools" << endl;
	locOfStream << "	//dOutputTreeFileNameMap[\"Bin1\"] = \"mcgen_bin1.root\"; //key is user-defined, value is output file name" << endl;
	locOfStream << "	//dOutputTreeFileNameMap[\"Bin2\"] = \"mcgen_bin2.root\"; //key is user-defined, value is output file name" << endl;
	locOfStream << "	//dOutputTreeFileNameMap[\"Bin3\"] = \"mcgen_bin3.root\"; //key is user-defined, value is output file name" << endl;
	locOfStream << endl;
	locOfStream << "	//Because this function gets called for each TTree in the TChain, we must be careful:" << endl;
	locOfStream << "		//We need to re-initialize the tree interface & branch wrappers, but don't want to recreate histograms" << endl;
	locOfStream << "	bool locInitializedPriorFlag = dInitializedFlag; //save whether have been initialized previously" << endl;
	locOfStream << "	DSelector::Init(locTree); //This must be called to initialize wrappers for each new TTree" << endl;
	locOfStream << "	//gDirectory now points to the output file with name dOutputFileName (if any)" << endl;
	locOfStream << "	if(locInitializedPriorFlag)" << endl;
	locOfStream << "		return; //have already created histograms, etc. below: exit" << endl;
	locOfStream << endl;
	locOfStream << "	dPreviousRunNumber = 0;" << endl;
	locOfStream << endl;
	locOfStream << "	/******************************** EXAMPLE USER INITIALIZATION: STAND-ALONE HISTOGRAMS *******************************/" << endl;
	locOfStream << endl;
	//writing out all the self-defined histograms
	if(locConfiguration != "0") Add_HistogramInitialization(locOfStream, locConfiguration);
	locOfStream << endl;
	locOfStream << "	/************************************* ADVANCED EXAMPLE: CHOOSE BRANCHES TO READ ************************************/" << endl;
	locOfStream << endl;
	locOfStream << "	//TO SAVE PROCESSING TIME" << endl;
	locOfStream << "		//If you know you don't need all of the branches/data, but just a subset of it, you can speed things up" << endl;
	locOfStream << "		//By default, for each event, the data is retrieved for all branches" << endl;
	locOfStream << "		//If you know you only need data for some branches, you can skip grabbing data from the branches you don't need" << endl;
	locOfStream << "		//Do this by doing something similar to the commented code below" << endl;
	locOfStream << endl;
	locOfStream << "	//dTreeInterface->Clear_GetEntryBranches(); //now get none" << endl;
	locOfStream << "	//dTreeInterface->Register_GetEntryBranch(\"Proton__P4\"); //manually set the branches you want" << endl;
	locOfStream << "}" << endl;
	locOfStream << endl;


	locOfStream << "Bool_t " << locSelectorName << "::Process(Long64_t locEntry)" << endl;
	locOfStream << "{" << endl;
	locOfStream << "	// The Process() function is called for each entry in the tree. The entry argument" << endl;
	locOfStream << "	// specifies which entry in the currently loaded tree is to be processed." << endl;
	locOfStream << "	//" << endl;
	locOfStream << "	// This function should contain the \"body\" of the analysis. It can contain" << endl;
	locOfStream << "	// simple or elaborate selection criteria, run algorithms on the data" << endl;
	locOfStream << "	// of the event and typically fill histograms." << endl;
	locOfStream << "	//" << endl;
	locOfStream << "	// The processing can be stopped by calling Abort()." << endl;
	locOfStream << "	// Use fStatus to set the return value of TTree::Process()." << endl;
	locOfStream << "	// The return value is currently not used." << endl;
	locOfStream << endl;
	locOfStream << "	//CALL THIS FIRST" << endl;
	locOfStream << "	DSelector::Process(locEntry); //Gets the data from the tree for the entry" << endl;
	locOfStream << "	//cout << \"RUN \" << Get_RunNumber() << \", EVENT \" << Get_EventNumber() << endl;" << endl;
	locOfStream << endl;
	locOfStream << "	/******************************************** GET POLARIZATION ORIENTATION ******************************************/" << endl;
	locOfStream << endl;
	locOfStream << "	//Only if the run number changes" << endl;
	locOfStream << "	//RCDB environment must be setup in order for this to work! (Will return false otherwise)" << endl;
	locOfStream << "	UInt_t locRunNumber = Get_RunNumber();" << endl;
	locOfStream << "	if(locRunNumber != dPreviousRunNumber)" << endl;
	locOfStream << "	{" << endl;
	locOfStream << "		dIsPolarizedFlag = dAnalysisUtilities.Get_IsPolarizedBeam(locRunNumber, dIsPARAFlag);" << endl;
	locOfStream << "		dPreviousRunNumber = locRunNumber;" << endl;
	locOfStream << "	}" << endl;
	locOfStream << endl;
	locOfStream << "	/********************************************* SETUP UNIQUENESS TRACKING ********************************************/" << endl;
	locOfStream << endl;
	locOfStream << "	//INSERT USER ANALYSIS UNIQUENESS TRACKING HERE" << endl;
	if(locConfiguration != "0") Add_UniquenessTrackingDefinition_Thrown(locOfStream, locConfiguration);
	locOfStream << endl;
	locOfStream << "	/******************************************* LOOP OVER THROWN DATA ***************************************/" << endl;
	locOfStream << endl;
	locOfStream << "	//Define TLorentzvectors in the LAB Frame" << endl;
	locOfStream << "	TLorentzVector locBeamP4, locBeamX4;" << endl;
	locOfStream << endl;
	locOfStream << "	//Thrown beam: just use directly" << endl;
	locOfStream << "	double locBeamEnergyUsedForBinning = 0.0;" << endl;
	locOfStream << "	if(dThrownBeam != NULL)" << endl;
	locOfStream << "	{" << endl;
	locOfStream << "	    locBeamP4 = dThrownBeam->Get_P4();" << endl;
	//locOfStream << "	    locBeamX4 = dThrownBeam->Get_X4();" << endl;
	locOfStream << "		locBeamEnergyUsedForBinning = dThrownBeam->Get_P4().E();" << endl;
	locOfStream << "	}" << endl;
	locOfStream << endl;
	if(locConfiguration == "0") 
	{
		locOfStream << "	//Loop over throwns" << endl;
		locOfStream << "	for(UInt_t loc_i = 0; loc_i < Get_NumThrown(); ++loc_i)" << endl;
		locOfStream << "	{" << endl;
		locOfStream << "		//Set branch array indices corresponding to this particle" << endl;
		locOfStream << "		dThrownWrapper->Set_ArrayIndex(loc_i);" << endl;
		locOfStream << endl;
		locOfStream << "		//Do stuff with the wrapper here ..." << endl;
		locOfStream << "		Particle_t locPID = dThrownWrapper->Get_PID();" << endl;
		locOfStream << "		TLorentzVector locThrownP4 = dThrownWrapper->Get_P4();" << endl;
		locOfStream << "		//cout << \"Thrown \" << loc_i << \": \" << locPID << \", \" << locThrownP4.Px() << \", \" << locThrownP4.Py() << \", \" << locThrownP4.Pz() << \", \" << locThrownP4.E() << endl;" << endl;
		locOfStream << "	}" << endl;
		locOfStream << endl;
		locOfStream << "	//OR Manually:" << endl;
		locOfStream << "	//BEWARE: Do not expect the particles to be at the same array indices from one event to the next!!!!" << endl;
		locOfStream << "	//Why? Because while your channel may be the same, the pions/kaons/etc. will decay differently each event." << endl;
		locOfStream << endl;
		locOfStream << "	//BRANCHES: https://halldweb.jlab.org/wiki/index.php/Analysis_TTreeFormat#TTree_Format:_Simulated_Data" << endl;
		locOfStream << "	TClonesArray** locP4Array = dTreeInterface->Get_PointerToPointerTo_TClonesArray(\"Thrown__P4\");" << endl;
		locOfStream << "	TBranch* locPIDBranch = dTreeInterface->Get_Branch(\"Thrown__PID\");" << endl;
		locOfStream << "/*" << endl;
		locOfStream << "	Particle_t locThrown1PID = PDGtoPType(((Int_t*)locPIDBranch->GetAddress())[0]);" << endl;
		locOfStream << "	TLorentzVector locThrown1P4 = *((TLorentzVector*)(*locP4Array)->At(0));" << endl;
		locOfStream << "	cout << \"Particle 1: \" << locThrown1PID << \", \" << locThrown1P4.Px() << \", \" << locThrown1P4.Py() << \", \" << locThrown1P4.Pz() << \", \" << locThrown1P4.E() << endl;" << endl;
		locOfStream << "	Particle_t locThrown2PID = PDGtoPType(((Int_t*)locPIDBranch->GetAddress())[1]);" << endl;
		locOfStream << "	TLorentzVector locThrown2P4 = *((TLorentzVector*)(*locP4Array)->At(1));" << endl;
		locOfStream << "	cout << \"Particle 2: \" << locThrown2PID << \", \" << locThrown2P4.Px() << \", \" << locThrown2P4.Py() << \", \" << locThrown2P4.Pz() << \", \" << locThrown2P4.E() << endl;" << endl;
		locOfStream << "*/" << endl;
		locOfStream << endl;
		locOfStream << endl;
	}
	else
	{
		Add_MCThrownParticles(locOfStream, locConfiguration);
		locOfStream << "	/**************************************** LORENTZ TRANSFORMATION **************************************/" << endl;
		Add_Thrown_LorentzBoost(locOfStream, locConfiguration);
		locOfStream << "		/**************************************** KINEMATIC CALCULATION **************************************/" << endl;
		locOfStream << endl;
		locOfStream << "//only in mc_thrown to flag the event not used to fill histograms " << endl;
		locOfStream << "	Bool_t Set_IsEventCut               = false; // don't need to change it " << endl;
		Add_KinematicCalculation(locOfStream, locConfiguration);
		locOfStream << endl;
		locOfStream << "		/****************************************** FILL HISTOGRAM WHILE KEEPING UNIQUENESSTRACKING ******************************************/" << endl;
		locOfStream << endl;

		locOfStream << "        Double_t histWeight = 1.0; // default weight is set at 1" << endl;
		locOfStream << "        if(Set_IsEventCut == false)" << endl;
		locOfStream << "        {" << endl;
		FillHist(locOfStream, locConfiguration, "hist_for_mcGen");
		locOfStream << "        }" << endl;
		locOfStream << endl;
	} 
	

	locOfStream << "	/******************************************* BIN THROWN DATA INTO SEPARATE TREES FOR AMPTOOLS ***************************************/" << endl;
	locOfStream << endl;
	locOfStream << "/*" << endl;
	locOfStream << "	//THESE KEYS MUST BE DEFINED IN THE INIT SECTION (along with the output file names)" << endl;
	locOfStream << "	if((locBeamEnergyUsedForBinning >= 8.0) && (locBeamEnergyUsedForBinning < 9.0))" << endl;
	locOfStream << "		Fill_OutputTree(\"Bin1\"); //your user-defined key" << endl;
	locOfStream << "	else if((locBeamEnergyUsedForBinning >= 9.0) && (locBeamEnergyUsedForBinning < 10.0))" << endl;
	locOfStream << "		Fill_OutputTree(\"Bin2\"); //your user-defined key" << endl;
	locOfStream << "	else if((locBeamEnergyUsedForBinning >= 10.0) && (locBeamEnergyUsedForBinning < 11.0))" << endl;
	locOfStream << "		Fill_OutputTree(\"Bin3\"); //your user-defined key" << endl;
	locOfStream << "*/" << endl;
	locOfStream << endl;
	locOfStream << "	return kTRUE;" << endl;
	locOfStream << "}" << endl;
	locOfStream << endl;
	locOfStream << "void " << locSelectorName << "::Finalize(void)" << endl;
	locOfStream << "{" << endl;
	locOfStream << "	//Save anything to output here that you do not want to be in the default DSelector output ROOT file." << endl;
	locOfStream << endl;
	locOfStream << "	//Otherwise, don\'t do anything else (especially if you are using PROOF)." << endl;
	locOfStream << "		//If you are using PROOF, this function is called on each thread," << endl;
	locOfStream << "		//so anything you do will not have the combined information from the various threads." << endl;
	locOfStream << "		//Besides, it is best-practice to do post-processing (e.g. fitting) separately, in case there is a problem." << endl;
	locOfStream << endl;
	locOfStream << "	//DO YOUR STUFF HERE" << endl;
	locOfStream << endl;
	locOfStream << "	//CALL THIS LAST" << endl;
	locOfStream << "	DSelector::Finalize(); //Saves results to the output file" << endl;
	locOfStream << "}" << endl;

	locOfStream.close();
}



