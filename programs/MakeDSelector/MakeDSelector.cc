#include <string>
#include <map>
#include <iostream>
#include <fstream>

#include <TFile.h>
#include <TTree.h>

#include "DSelector/DTreeInterface.h"

//function declarations
void Print_Usage(void);
void Print_Config(string locSelectorBaseName, DTreeInterface* locTreeInterface, map<int, map<int, pair<Particle_t, string> > >& locComboInfoMap);
void Print_HeaderFile(string locSelectorBaseName, DTreeInterface* locTreeInterface, map<int, map<int, pair<Particle_t, string> > >& locComboInfoMap, string locConfiguration);
void Print_SourceFile(string locSelectorBaseName, DTreeInterface* locTreeInterface, map<int, map<int, pair<Particle_t, string> > >& locComboInfoMap, string locConfiguration);
void Print_HeaderFile_MCGen(string locSelectorBaseName, DTreeInterface* locTreeInterface);
void Print_SourceFile_MCGen(string locSelectorBaseName, DTreeInterface* locTreeInterface);

//utilities declarations
void Add_Action(std::ofstream& locOfStream, string locConfig);
void Add_HistogramDefinition(std::ofstream& locOfStream, string locConfig);
void Add_HistogramInitialization(std::ofstream& locOfStream, string locConfig);
void Add_UniquenessTrackingDefinition(std::ofstream& locOfStream, string locConfig);
void FillHist(std::ofstream& locOfStream, string locConfig, string UniquenessName);
void Add_UniqueHistogramFilling(std::ofstream& locOfStream, string locConfig);
void Add_KinematicCalculation(std::ofstream& locOfStream, string locConfig);
void Add_LorentzBoost(std::ofstream& locOfStream, string locConfig, DTreeInterface* locTreeInterface, map<int, map<int, pair<Particle_t, string> > >& locComboInfoMap);







int main(int argc, char* argv[])
{
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

	bool locIsMCGenTreeFlag = (locTreeInterface->Get_Branch("NumCombos") == NULL);
	if(locIsMCGenTreeFlag)
	{
		Print_HeaderFile_MCGen(locSelectorBaseName, locTreeInterface);
		Print_SourceFile_MCGen(locSelectorBaseName, locTreeInterface);
		string locSelectorName = string("DSelector_") + locSelectorBaseName;
		cout << "Selector files " << locSelectorName << ".* generated." << endl;
		return 0;
	}

	//get combo info
	map<int, map<int, pair<Particle_t, string> > > locComboInfoMap;
	locTreeInterface->Get_ComboInfo(locComboInfoMap);

	Print_HeaderFile(locSelectorBaseName, locTreeInterface, locComboInfoMap, locConfiguration);
	Print_SourceFile(locSelectorBaseName, locTreeInterface, locComboInfoMap, locConfiguration);

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
	locConfigStream << "#CUT: ChiSqOrCL                =      ## SecondaryReactionName, IsChiSq, Function, ActionUniqueString" << endl;
	locConfigStream << "#CUT: PIDDeltaT                =      ## UseKinFitFlag, DeltaTCut, PID, System, ActionUniqueString " << endl;
	locConfigStream << "#CUT: NoPIDHit                 =      ## PID, System, ActionUniqueString" << endl;
	locConfigStream << "#CUT: dEdx                     =      ## UseKinFitFlag, PID, System, ActionUniqueString" << endl;
	locConfigStream << "#CUT: KinFitFOM                =      ## MinimumConfidenceLevel, ActionUniqueString" << endl;
	locConfigStream << "#CUT: MissingMass              =      ## UseKinFitFlag, MinimumMissingMass, MaximumMissingMass, ActionUniqueString" << endl;
	locConfigStream << "#CUT: MissingMassSquared       =      ## UseKinFitFlag, MinimumMissingMassSq, MaximumMissingMassSq, ActionUniqueString" << endl;
	locConfigStream << "#CUT: MissingEnergy            =      ## UseKinFitFlag, MinimumMissingEnergy, MaximumMissingEnergy, ActionUniqueString" << endl;
	locConfigStream << "#CUT: MissingEnergy            =      ## UseKinFitFlag, MissingEnergyOffOfStepIndex, MissingEnergyOffOfPIDs, MinimumMissingEnergy, MaximumMissingEnergy, ActionUniqueString" << endl;
	locConfigStream << "#CUT: MissingEnergy            =      ## UseKinFitFlag, MissingEnergyOffOfStepIndex, MissingEnergyOffOfPID, MinimumMissingEnergy, MaximumMissingEnergy, ActionUniqueString" << endl;
	locConfigStream << "#CUT: InvariantMass            =      ## UseKinFitFlag, InitialPID, MinMass, MaxMass, ActionUniqueString" << endl;
	locConfigStream << "#CUT: InvariantMass            =      ## UseKinFitFlag, StepIndex, ToIncludePIDs, MinMass, MaxMass, ActionUniqueString" << endl;
	locConfigStream << "#CUT: InvariantMassVeto        =      ## UseKinFitFlag, InitialPID, MinMass, MaxMass, ActionUniqueString" << endl;
	locConfigStream << "#CUT: InvariantMassVeto        =      ## UseKinFitFlag, StepIndex, ToIncludePIDs, MinMass, MaxMass, ActionUniqueString" << endl;
	locConfigStream << "#CUT: BeamEnergy               =      ## UseKinFitFlag, MinBeamEnergy, MaxBeamEnergy, ActionUniqueString" << endl;
	locConfigStream << "#CUT: TrackShowerEOverP        =      ## UseKinFitFlag, Detector, PID, ShowerEOverPCut, ActionUniqueString" << endl;
	locConfigStream << "#CUT: Kinematics               =      ## StepIndex, PID, UseKinFitFlag, CutMinP, CutMaxP, CutMinTheta, CutMaxTheta, CutMinPhi, CutMaxPhi" << endl;
	locConfigStream << "#HIST: ParticleComboKinematics =      ## UseKinFitFlag, ActionUniqueString" << endl;
	locConfigStream << "#HIST: ParticleID              =      ## UseKinFitFlag, ActionUniqueString" << endl;
	locConfigStream << "#HIST: InvariantMass           =      ## UseKinFitFlag, InitialPID, NumMassBins, MinMass, MaxMass, ActionUniqueString" << endl;
	locConfigStream << "#HIST: MissingMass             =      ## UseKinFitFlag, NumMassBins, MinMass, MaxMass, ActionUniqueString" << endl;
	locConfigStream << "#HIST: MissingMassSquared      =      ## UseKinFitFlag, NumMassBins, MinMassSq, MaxMassSq, ActionUniqueString" << endl;
	locConfigStream << "#HIST: MissingP4               =      ## UseKinFitFlag, ActionUniqueString" << endl;
	locConfigStream << "#HIST: 2DInvariantMass         =      ## UseKinFitFlag, StepIndex, XPIDs, YPIDs, NumXBins, MinX, MaxX, NumYBins, MinY, MaxY, ActionUniqueString" << endl;
	locConfigStream << "#HIST: Dalitz                  =      ## UseKinFitFlag, StepIndex, XPIDs, YPIDs, NumXBins, MinX, MaxX, NumYBins, MinY, MaxY, ActionUniqueString" << endl;
	locConfigStream << "#HIST: vanHove                 =      ## UseKinFitFlag, XPIDs, YPIDs, ZPIDs, ActionUniqueString" << endl;
	locConfigStream << "#HIST: KinFitResults           =      ## ActionUniqueString" << endl;
	locConfigStream << "#HIST: BeamEnergy              =      ## UseKinFitFlag, ActionUniqueString" << endl;
	locConfigStream << "#HIST: Energy_UnusedShowers    =      ## ActionUniqueString" << endl;
	locConfigStream << endl;
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
	locConfigStream << endl;
	locConfigStream << "######################################################" << endl;
	locConfigStream << "[SECTION: UNIQUENESS TRACKING]" << endl;
	locConfigStream << "######################################################" << endl;
	locConfigStream << "## Prevent double-counting for specified particles or set of particles" << endl;
	locConfigStream << "## String before \"=\" goes to \"UniquenessTracking\" in \"[HISTOGRAMS]\" section" << endl;
	locConfigStream << "## String after \"=\" will be the particles used during calculation of the variable to be filled" << endl;
	//get particles
	string locFinalStateParticles;
	map<int, map<int, pair<Particle_t, string> > >::iterator locStepIterator = locComboInfoMap.begin();
	for(; locStepIterator != locComboInfoMap.end(); ++locStepIterator)
	{
		//int locStepIndex = locStepIterator->first;
		//locSourceStream << "		//Step " << locStepIndex << endl;
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
	locConfigStream << "No-Uniqueness-Tracking ## Example: use this if you do not wish to keep uniqueness tracking (for example plotting kinfitCL)." << endl;
	locConfigStream << "Particle-specific(Beam)      = Beam ## Example: plot for every beam photon." << endl;
	locConfigStream << "Combo-specific(All)          = "<< locFinalStateParticles << " ## Example: plot for every combo." << endl;
	locConfigStream << endl;                           
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
	locConfigStream << "## If you don't need uniqueness tracking i.e add one entry per combo, just specified Uniqueness as \"No-Uniqueness-Tracking\" ." << endl;
	locConfigStream << "HISTOGRAM = TH1D KinFitCL \";Kinematic Fit Confidence Level\" 100 0 1.0 - - - locConfidenceLevel No-Uniqueness-Tracking" << endl;
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
	locHeaderStream << "		/*************** Begin of definition of histograms **************/" << endl;
	if(locConfiguration != "0") Add_HistogramDefinition(locHeaderStream, locConfiguration);
	locHeaderStream << "		/*************** End of definition of histograms **************/" << endl;

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

void Print_SourceFile(string locSelectorBaseName, DTreeInterface* locTreeInterface, map<int, map<int, pair<Particle_t, string> > >& locComboInfoMap, string locConfiguration)
{
	string locSelectorName = string("DSelector_") + locSelectorBaseName;
	string locSourceName = locSelectorName + string(".C");
	ofstream locSourceStream;
	locSourceStream.open(locSourceName.c_str());

	locSourceStream << "#include \"" << locSelectorName << ".h\"" << endl;
	locSourceStream << endl;
	locSourceStream << "void " << locSelectorName << "::Init(TTree *locTree)" << endl;
	locSourceStream << "{" << endl;
	locSourceStream << "	// USERS: IN THIS FUNCTION, ONLY MODIFY SECTIONS WITH A \"USER\" OR \"EXAMPLE\" LABEL. LEAVE THE REST ALONE." << endl;
	locSourceStream << endl;
	locSourceStream << "	// The Init() function is called when the selector needs to initialize a new tree or chain." << endl;
	locSourceStream << "	// Typically here the branch addresses and branch pointers of the tree will be set." << endl;
	locSourceStream << "	// Init() will be called many times when running on PROOF (once per file to be processed)." << endl;
	locSourceStream << endl;
	locSourceStream << "	//USERS: SET OUTPUT FILE NAME //can be overriden by user in PROOF" << endl;
	locSourceStream << "	dOutputFileName = \"" << locSelectorBaseName << ".root\"; //\"\" for none" << endl;
	locSourceStream << "	dOutputTreeFileName = \"\"; //\"\" for none" << endl;
	locSourceStream << "	dFlatTreeFileName = \"\"; //output flat tree (one combo per tree entry), \"\" for none" << endl;
	locSourceStream << "	dFlatTreeName = \"\"; //if blank, default name will be chosen" << endl;
	locSourceStream << endl;
	locSourceStream << "	//Because this function gets called for each TTree in the TChain, we must be careful:" << endl;
	locSourceStream << "		//We need to re-initialize the tree interface & branch wrappers, but don't want to recreate histograms" << endl;
	locSourceStream << "	bool locInitializedPriorFlag = dInitializedFlag; //save whether have been initialized previously" << endl;
	locSourceStream << "	DSelector::Init(locTree); //This must be called to initialize wrappers for each new TTree" << endl;
	locSourceStream << "	//gDirectory now points to the output file with name dOutputFileName (if any)" << endl;
	locSourceStream << "	if(locInitializedPriorFlag)" << endl;
	locSourceStream << "		return; //have already created histograms, etc. below: exit" << endl;
	locSourceStream << endl;
	locSourceStream << "	Get_ComboWrappers();" << endl;
	locSourceStream << "	dPreviousRunNumber = 0;" << endl;
	locSourceStream << endl;
	locSourceStream << "	/*********************************** EXAMPLE USER INITIALIZATION: ANALYSIS ACTIONS **********************************/" << endl;
	locSourceStream << endl;

	if(locConfiguration != "0") Add_Action(locSourceStream, locConfiguration);

	locSourceStream << "	// EXAMPLE: Create deque for histogramming particle masses:" << endl;
	locSourceStream << "	// // For histogramming the phi mass in phi -> K+ K-" << endl;
	locSourceStream << "	// // Be sure to change this and dAnalyzeCutActions to match reaction" << endl;
	locSourceStream << "	std::deque<Particle_t> MyPhi;" << endl;
	locSourceStream << "	MyPhi.push_back(KPlus); MyPhi.push_back(KMinus);" << endl;
	locSourceStream << endl;
	locSourceStream << "	//ANALYSIS ACTIONS: //Executed in order if added to dAnalysisActions" << endl;
	locSourceStream << "	//false/true below: use measured/kinfit data" << endl;
	locSourceStream << endl;
	locSourceStream << "	//PID" << endl;
	locSourceStream << "	//dAnalysisActions.push_back(new DHistogramAction_ParticleID(dComboWrapper, false));" << endl;
	locSourceStream << "	//below: value: +/- N ns, Unknown: All PIDs, SYS_NULL: all timing systems" << endl;
	locSourceStream << "	//dAnalysisActions.push_back(new DCutAction_PIDDeltaT(dComboWrapper, false, 0.5, KPlus, SYS_BCAL));" << endl;
	locSourceStream << endl;
	locSourceStream << "	//MASSES" << endl;
	locSourceStream << "	//dAnalysisActions.push_back(new DHistogramAction_InvariantMass(dComboWrapper, false, Lambda, 1000, 1.0, 1.2, \"Lambda\"));" << endl;
	locSourceStream << "	//dAnalysisActions.push_back(new DHistogramAction_MissingMassSquared(dComboWrapper, false, 1000, -0.1, 0.1));" << endl;
	locSourceStream << endl;
	locSourceStream << "	//KINFIT RESULTS" << endl;
	locSourceStream << "	dAnalysisActions.push_back(new DHistogramAction_KinFitResults(dComboWrapper));" << endl;
	locSourceStream << endl;
	locSourceStream << "	//CUT MISSING MASS" << endl;
	locSourceStream << "	//dAnalysisActions.push_back(new DCutAction_MissingMassSquared(dComboWrapper, false, -0.03, 0.02));" << endl;
	locSourceStream << endl;
	locSourceStream << "	//BEAM ENERGY" << endl;
	locSourceStream << "	dAnalysisActions.push_back(new DHistogramAction_BeamEnergy(dComboWrapper, false));" << endl;
	locSourceStream << "	//dAnalysisActions.push_back(new DCutAction_BeamEnergy(dComboWrapper, false, 8.4, 9.05));" << endl;
	locSourceStream << endl;
	locSourceStream << "	//KINEMATICS" << endl;
	locSourceStream << "	//dAnalysisActions.push_back(new DHistogramAction_ParticleComboKinematics(dComboWrapper, false));" << endl;
	locSourceStream << endl;
	locSourceStream << "	// ANALYZE CUT ACTIONS" << endl;
	locSourceStream << "	// // Change MyPhi to match reaction" << endl;
	locSourceStream << "	dAnalyzeCutActions = new DHistogramAction_AnalyzeCutActions( dAnalysisActions, dComboWrapper, false, 0, MyPhi, 1000, 0.9, 2.4, \"CutActionEffect\" );" << endl;
	locSourceStream << endl;
	locSourceStream << "	//INITIALIZE ACTIONS" << endl;
	locSourceStream << "	//If you create any actions that you want to run manually (i.e. don't add to dAnalysisActions), be sure to initialize them here as well" << endl;
	locSourceStream << "	Initialize_Actions();" << endl;
	locSourceStream << "	dAnalyzeCutActions->Initialize(); // manual action, must call Initialize()" << endl;
	locSourceStream << endl;
	locSourceStream << "	/******************************** EXAMPLE USER INITIALIZATION: STAND-ALONE HISTOGRAMS *******************************/" << endl;
	locSourceStream << endl;
	locSourceStream << "	//EXAMPLE MANUAL HISTOGRAMS:" << endl;
	locSourceStream << "	dHist_MissingMassSquared = new TH1I(\"MissingMassSquared\", \";Missing Mass Squared (GeV/c^{2})^{2}\", 600, -0.06, 0.06);" << endl;
	locSourceStream << "	dHist_BeamEnergy = new TH1I(\"BeamEnergy\", \";Beam Energy (GeV)\", 600, 0.0, 12.0);" << endl;
	locSourceStream << endl;

	//writing out all the self-defined histograms
	if(locConfiguration != "0") locSourceStream << "	/*************** Begin of histogram initialization **************/" << endl;
	if(locConfiguration != "0") Add_HistogramInitialization(locSourceStream, locConfiguration);


	locSourceStream << "	/************************** EXAMPLE USER INITIALIZATION: CUSTOM OUTPUT BRANCHES - MAIN TREE *************************/" << endl;
	locSourceStream << endl;
	locSourceStream << "	//EXAMPLE MAIN TREE CUSTOM BRANCHES (OUTPUT ROOT FILE NAME MUST FIRST BE GIVEN!!!! (ABOVE: TOP)):" << endl;
	locSourceStream << "	//The type for the branch must be included in the brackets" << endl;
	locSourceStream << "	//1st function argument is the name of the branch" << endl;
	locSourceStream << "	//2nd function argument is the name of the branch that contains the size of the array (for fundamentals only)" << endl;
	locSourceStream << "	/*" << endl;
	locSourceStream << "	dTreeInterface->Create_Branch_Fundamental<Int_t>(\"my_int\"); //fundamental = char, int, float, double, etc." << endl;
	locSourceStream << "	dTreeInterface->Create_Branch_FundamentalArray<Int_t>(\"my_int_array\", \"my_int\");" << endl;
	locSourceStream << "	dTreeInterface->Create_Branch_FundamentalArray<Float_t>(\"my_combo_array\", \"NumCombos\");" << endl;
	locSourceStream << "	dTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>(\"my_p4\");" << endl;
	locSourceStream << "	dTreeInterface->Create_Branch_ClonesArray<TLorentzVector>(\"my_p4_array\");" << endl;
	locSourceStream << "	*/" << endl;
	locSourceStream << endl;
	locSourceStream << "	/************************** EXAMPLE USER INITIALIZATION: CUSTOM OUTPUT BRANCHES - FLAT TREE *************************/" << endl;
	locSourceStream << endl;
	locSourceStream << "	//EXAMPLE FLAT TREE CUSTOM BRANCHES (OUTPUT ROOT FILE NAME MUST FIRST BE GIVEN!!!! (ABOVE: TOP)):" << endl;
	locSourceStream << "	//The type for the branch must be included in the brackets" << endl;
	locSourceStream << "	//1st function argument is the name of the branch" << endl;
	locSourceStream << "	//2nd function argument is the name of the branch that contains the size of the array (for fundamentals only)" << endl;
	locSourceStream << "	/*" << endl;
	locSourceStream << "	dFlatTreeInterface->Create_Branch_Fundamental<Int_t>(\"flat_my_int\"); //fundamental = char, int, float, double, etc." << endl;
	locSourceStream << "	dFlatTreeInterface->Create_Branch_FundamentalArray<Int_t>(\"flat_my_int_array\", \"flat_my_int\");" << endl;
	locSourceStream << "	dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>(\"flat_my_p4\");" << endl;
	locSourceStream << "	dFlatTreeInterface->Create_Branch_ClonesArray<TLorentzVector>(\"flat_my_p4_array\");" << endl;
	locSourceStream << "	*/" << endl;
	locSourceStream << endl;
	locSourceStream << "	/************************************* ADVANCED EXAMPLE: CHOOSE BRANCHES TO READ ************************************/" << endl;
	locSourceStream << endl;
	locSourceStream << "	//TO SAVE PROCESSING TIME" << endl;
	locSourceStream << "		//If you know you don't need all of the branches/data, but just a subset of it, you can speed things up" << endl;
	locSourceStream << "		//By default, for each event, the data is retrieved for all branches" << endl;
	locSourceStream << "		//If you know you only need data for some branches, you can skip grabbing data from the branches you don't need" << endl;
	locSourceStream << "		//Do this by doing something similar to the commented code below" << endl;
	locSourceStream << endl;
	locSourceStream << "	//dTreeInterface->Clear_GetEntryBranches(); //now get none" << endl;
	locSourceStream << "	//dTreeInterface->Register_GetEntryBranch(\"Proton__P4\"); //manually set the branches you want" << endl;
	locSourceStream << "}" << endl;
	locSourceStream << endl;
	locSourceStream << "Bool_t " << locSelectorName << "::Process(Long64_t locEntry)" << endl;
	locSourceStream << "{" << endl;
	locSourceStream << "	// The Process() function is called for each entry in the tree. The entry argument" << endl;
	locSourceStream << "	// specifies which entry in the currently loaded tree is to be processed." << endl;
	locSourceStream << "	//" << endl;
	locSourceStream << "	// This function should contain the \"body\" of the analysis. It can contain" << endl;
	locSourceStream << "	// simple or elaborate selection criteria, run algorithms on the data" << endl;
	locSourceStream << "	// of the event and typically fill histograms." << endl;
	locSourceStream << "	//" << endl;
	locSourceStream << "	// The processing can be stopped by calling Abort()." << endl;
	locSourceStream << "	// Use fStatus to set the return value of TTree::Process()." << endl;
	locSourceStream << "	// The return value is currently not used." << endl;
	locSourceStream << endl;
	locSourceStream << "	//CALL THIS FIRST" << endl;
	locSourceStream << "	DSelector::Process(locEntry); //Gets the data from the tree for the entry" << endl;
	locSourceStream << "	//cout << \"RUN \" << Get_RunNumber() << \", EVENT \" << Get_EventNumber() << endl;" << endl;
	locSourceStream << "	//TLorentzVector locProductionX4 = Get_X4_Production();" << endl;
	locSourceStream << endl;
	locSourceStream << "	/******************************************** GET POLARIZATION ORIENTATION ******************************************/" << endl;
	locSourceStream << endl;
	locSourceStream << "	//Only if the run number changes" << endl;
	locSourceStream << "	//RCDB environment must be setup in order for this to work! (Will return false otherwise)" << endl;
	locSourceStream << "	UInt_t locRunNumber = Get_RunNumber();" << endl;
	locSourceStream << "	if(locRunNumber != dPreviousRunNumber)" << endl;
	locSourceStream << "	{" << endl;
	locSourceStream << "		dIsPolarizedFlag = dAnalysisUtilities.Get_IsPolarizedBeam(locRunNumber, dIsPARAFlag);" << endl;
	locSourceStream << "		dPreviousRunNumber = locRunNumber;" << endl;
	locSourceStream << "	}" << endl;
	locSourceStream << endl;
	locSourceStream << "	/********************************************* SETUP UNIQUENESS TRACKING ********************************************/" << endl;
	locSourceStream << endl;
	locSourceStream << "	//ANALYSIS ACTIONS: Reset uniqueness tracking for each action" << endl;
	locSourceStream << "	//For any actions that you are executing manually, be sure to call Reset_NewEvent() on them here" << endl;
	locSourceStream << "	Reset_Actions_NewEvent();" << endl;
	locSourceStream << "	dAnalyzeCutActions->Reset_NewEvent(); // manual action, must call Reset_NewEvent()" << endl;
	locSourceStream << endl;
	locSourceStream << "	//PREVENT-DOUBLE COUNTING WHEN HISTOGRAMMING" << endl;
	locSourceStream << "		//Sometimes, some content is the exact same between one combo and the next" << endl;
	locSourceStream << "			//e.g. maybe two combos have different beam particles, but the same data for the final-state" << endl;
	locSourceStream << "		//When histogramming, you don\'t want to double-count when this happens: artificially inflates your signal (or background)" << endl;
	locSourceStream << "		//So, for each quantity you histogram, keep track of what particles you used (for a given combo)" << endl;
	locSourceStream << "		//Then for each combo, just compare to what you used before, and make sure it\'s unique" << endl;
	locSourceStream << endl;
	locSourceStream << "	//EXAMPLE 1: Particle-specific info:" << endl;
	locSourceStream << "	set<Int_t> locUsedSoFar_BeamEnergy; //Int_t: Unique ID for beam particles. set: easy to use, fast to search" << endl;
	locSourceStream << endl;
	locSourceStream << "	//EXAMPLE 2: Combo-specific info:" << endl;
	locSourceStream << "		//In general: Could have multiple particles with the same PID: Use a set of Int_t\'s" << endl;
	locSourceStream << "		//In general: Multiple PIDs, so multiple sets: Contain within a map" << endl;
	locSourceStream << "		//Multiple combos: Contain maps within a set (easier, faster to search)" << endl;
	locSourceStream << "	set<map<Particle_t, set<Int_t> > > locUsedSoFar_MissingMass;" << endl;
	locSourceStream << endl;
	locSourceStream << "	//INSERT USER ANALYSIS UNIQUENESS TRACKING HERE" << endl;

	if(locConfiguration != "0") Add_UniquenessTrackingDefinition(locSourceStream, locConfiguration);

	locSourceStream << endl;
	locSourceStream << "	/**************************************** EXAMPLE: FILL CUSTOM OUTPUT BRANCHES **************************************/" << endl;
	locSourceStream << endl;
	locSourceStream << "	/*" << endl;
	locSourceStream << "	Int_t locMyInt = 7;" << endl;
	locSourceStream << "	dTreeInterface->Fill_Fundamental<Int_t>(\"my_int\", locMyInt);" << endl;
	locSourceStream << endl;
	locSourceStream << "	TLorentzVector locMyP4(4.0, 3.0, 2.0, 1.0);" << endl;
	locSourceStream << "	dTreeInterface->Fill_TObject<TLorentzVector>(\"my_p4\", locMyP4);" << endl;
	locSourceStream << endl;
	locSourceStream << "	for(int loc_i = 0; loc_i < locMyInt; ++loc_i)" << endl;
	locSourceStream << "		dTreeInterface->Fill_Fundamental<Int_t>(\"my_int_array\", 3*loc_i, loc_i); //2nd argument = value, 3rd = array index" << endl;
	locSourceStream << "	*/" << endl;
	locSourceStream << endl;
	locSourceStream << "	/************************************************* LOOP OVER COMBOS *************************************************/" << endl;
	locSourceStream << endl;
	locSourceStream << "	//Loop over combos" << endl;
	locSourceStream << "	for(UInt_t loc_i = 0; loc_i < Get_NumCombos(); ++loc_i)" << endl;
	locSourceStream << "	{" << endl;
	locSourceStream << "		//Set branch array indices for combo and all combo particles" << endl;
	locSourceStream << "		dComboWrapper->Set_ComboIndex(loc_i);" << endl;
	locSourceStream << endl;
	locSourceStream << "		// Is used to indicate when combos have been cut" << endl;
	locSourceStream << "		if(dComboWrapper->Get_IsComboCut()) // Is false when tree originally created" << endl;
	locSourceStream << "			continue; // Combo has been cut previously" << endl;
	locSourceStream << endl;
	locSourceStream << "		/********************************************** GET PARTICLE INDICES *********************************************/" << endl;
	locSourceStream << endl;
	locSourceStream << "		//Used for tracking uniqueness when filling histograms, and for determining unused particles" << endl;
	locSourceStream << endl;

	//print particle indices
	map<int, map<int, pair<Particle_t, string> > >::iterator locStepIterator = locComboInfoMap.begin();
	for(; locStepIterator != locComboInfoMap.end(); ++locStepIterator)
	{
		int locStepIndex = locStepIterator->first;
		locSourceStream << "		//Step " << locStepIndex << endl;

		map<int, pair<Particle_t, string> >& locStepInfoMap = locStepIterator->second;
		map<int, pair<Particle_t, string> >::iterator locParticleIterator = locStepInfoMap.begin();
		for(; locParticleIterator != locStepInfoMap.end(); ++locParticleIterator)
		{
			Particle_t locPID = locParticleIterator->second.first;
			string locParticleName = locParticleIterator->second.second;

			if(locPID == Unknown)
				continue;
			else if(locParticleName == "ComboBeam")
				locSourceStream << "		Int_t locBeamID = dComboBeamWrapper->Get_BeamID();" << endl;
			else if(locParticleName.substr(0, 6) == "Target")
				continue;
			else if(locParticleName.substr(0, 8) == "Decaying")
				continue;
			else if(locParticleName.substr(0, 7) == "Missing")
				continue;
			else if(ParticleCharge(locPID) != 0)
				locSourceStream << "		Int_t loc" << locParticleName << "TrackID = d" << locParticleName << "Wrapper->Get_TrackID();" << endl;
			else
				locSourceStream << "		Int_t loc" << locParticleName << "NeutralID = d" << locParticleName << "Wrapper->Get_NeutralID();" << endl;
		}
		locSourceStream << endl;
	}
	locSourceStream << "		/*********************************************** GET FOUR-MOMENTUM and FOUR-POSITION **********************************************/" << endl;
	locSourceStream << endl;

	//get p4's
	locSourceStream << "		// Get P4\'s: //is kinfit if kinfit performed, else is measured" << endl;
	locSourceStream << "		//dTargetP4 is target p4" << endl;
	for(locStepIterator = locComboInfoMap.begin(); locStepIterator != locComboInfoMap.end(); ++locStepIterator)
	{
		int locStepIndex = locStepIterator->first;
		locSourceStream << "		//Step " << locStepIndex << endl;

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
				locSourceStream << "		TLorentzVector locBeamP4 = dComboBeamWrapper->Get_P4();" << endl;
			else if(locParticleName.substr(0, 6) == "Target")
				continue;
			else if(locParticleName.substr(0, 8) == "Decaying")
			{
				string locBranchName = locParticleName + string("__P4_KinFit");
				if((locTreeInterface->Get_Branch(locBranchName) != NULL) && (locParticleIndex < 0)) //else not reconstructed
					locSourceStream << "		TLorentzVector loc" << locParticleName << "P4 = d" << locParticleName << "Wrapper->Get_P4();" << endl;
			}
			else if(locParticleName.substr(0, 7) == "Missing")
			{
				string locBranchName = locParticleName + string("__P4_KinFit");
				if(locTreeInterface->Get_Branch(locBranchName) != NULL) //else not reconstructed
					locSourceStream << "		TLorentzVector loc" << locParticleName << "P4 = d" << locParticleName << "Wrapper->Get_P4();" << endl;
			}
			else//detected
				locSourceStream << "		TLorentzVector loc" << locParticleName << "P4 = d" << locParticleName << "Wrapper->Get_P4();" << endl;
				if(locStepIndex != 0) locSourceStream << "		TLorentzVector loc" << locParticleName << "X4 = d" << locParticleName << "Wrapper->Get_X4();" << endl;
		}
	}
	locSourceStream << endl;

	//get measured p4's
	locSourceStream << "		// Get Measured P4\'s:" << endl;
	for(locStepIterator = locComboInfoMap.begin(); locStepIterator != locComboInfoMap.end(); ++locStepIterator)
	{
		int locStepIndex = locStepIterator->first;
		locSourceStream << "		//Step " << locStepIndex << endl;

		map<int, pair<Particle_t, string> >& locStepInfoMap = locStepIterator->second;
		map<int, pair<Particle_t, string> >::iterator locParticleIterator = locStepInfoMap.begin();
		for(; locParticleIterator != locStepInfoMap.end(); ++locParticleIterator)
		{
			Particle_t locPID = locParticleIterator->second.first;
			string locParticleName = locParticleIterator->second.second;

			if(locPID == Unknown)
				continue;
			else if(locParticleName == "ComboBeam")
				locSourceStream << "		TLorentzVector locBeamP4_Measured = dComboBeamWrapper->Get_P4_Measured();" << endl;
			else if(locParticleName.substr(0, 6) == "Target")
				continue;
			else if(locParticleName.substr(0, 8) == "Decaying")
				continue;
			else if(locParticleName.substr(0, 7) == "Missing")
				continue;
			else //detected
				locSourceStream << "		TLorentzVector loc" << locParticleName << "P4_Measured = d" << locParticleName << "Wrapper->Get_P4_Measured();" << endl;
				if(locStepIndex != 0) locSourceStream << "		TLorentzVector loc" << locParticleName << "X4_Measured = d" << locParticleName << "Wrapper->Get_X4_Measured();" << endl;
		}
	}

	locSourceStream << endl;
	if(locConfiguration != "0") Add_LorentzBoost(locSourceStream, locConfiguration, locTreeInterface, locComboInfoMap);

	locSourceStream << "		/********************************************* COMBINE FOUR-MOMENTUM ********************************************/" << endl;
	locSourceStream << endl;
	locSourceStream << "		// DO YOUR STUFF HERE" << endl;
	locSourceStream << endl;
	locSourceStream << "		// Combine 4-vectors" << endl;
	locSourceStream << "		TLorentzVector locMissingP4_Measured = locBeamP4_Measured + dTargetP4;" << endl;
	locSourceStream << "		locMissingP4_Measured -= ";

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
				locSourceStream << " + ";
			locFirstFlag = false;
			locSourceStream << "loc" << locParticleName << "P4_Measured";
		}
	}
	locSourceStream << ";" << endl;
	locSourceStream << endl;
	locSourceStream << "		/******************************************** EXECUTE ANALYSIS ACTIONS *******************************************/" << endl;
	locSourceStream << endl;
	locSourceStream << "		// Loop through the analysis actions, executing them in order for the active particle combo" << endl;
	locSourceStream <<"		dAnalyzeCutActions->Perform_Action(); // Must be executed before Execute_Actions()" << endl;
	locSourceStream << "		if(!Execute_Actions()) //if the active combo fails a cut, IsComboCutFlag automatically set" << endl;
	locSourceStream << "			continue;" << endl;
	locSourceStream << endl;
	locSourceStream << "		//if you manually execute any actions, and it fails a cut, be sure to call:" << endl;
	locSourceStream << "			//dComboWrapper->Set_IsComboCut(true);" << endl;
	locSourceStream << endl;
	if(locConfiguration != "0") locSourceStream << "		/**************************************** KINEMATIC CALCULATION **************************************/" << endl;
	if(locConfiguration != "0") locSourceStream << endl;

	if(locConfiguration != "0") Add_KinematicCalculation(locSourceStream, locConfiguration);


	locSourceStream << "		/**************************************** EXAMPLE: FILL CUSTOM OUTPUT BRANCHES **************************************/" << endl;
	locSourceStream << endl;
	locSourceStream << "		/*" << endl;
	locSourceStream << "		TLorentzVector locMyComboP4(8.0, 7.0, 6.0, 5.0);" << endl;
	locSourceStream << "		//for arrays below: 2nd argument is value, 3rd is array index" << endl;
	locSourceStream << "		//NOTE: By filling here, AFTER the cuts above, some indices won't be updated (and will be whatever they were from the last event)" << endl;
	locSourceStream << "			//So, when you draw the branch, be sure to cut on \"IsComboCut\" to avoid these." << endl;
	locSourceStream << "		dTreeInterface->Fill_Fundamental<Float_t>(\"my_combo_array\", -2*loc_i, loc_i);" << endl;
	locSourceStream << "		dTreeInterface->Fill_TObject<TLorentzVector>(\"my_p4_array\", locMyComboP4, loc_i);" << endl;
	locSourceStream << "		*/" << endl;
	locSourceStream << endl;
	locSourceStream << "		/**************************************** EXAMPLE: HISTOGRAM BEAM ENERGY *****************************************/" << endl;
	locSourceStream << endl;
	locSourceStream << "		//Histogram beam energy (if haven\'t already)" << endl;
	locSourceStream << "		if(locUsedSoFar_BeamEnergy.find(locBeamID) == locUsedSoFar_BeamEnergy.end())" << endl;
	locSourceStream << "		{" << endl;
	locSourceStream << "			dHist_BeamEnergy->Fill(locBeamP4.E());" << endl;
	locSourceStream << "			locUsedSoFar_BeamEnergy.insert(locBeamID);" << endl;
	locSourceStream << "		}" << endl;
	locSourceStream << endl;
	locSourceStream << "		/**************************************** EXAMPLE: MISSINGMASS UNIQUENESS TRACKING  ************************************/" << endl;
	locSourceStream << endl;
	locSourceStream << "		//Missing Mass Squared" << endl;
	locSourceStream << "		double locMissingMassSquared = locMissingP4_Measured.M2();" << endl;
	locSourceStream << endl;
	locSourceStream << "		//Uniqueness tracking: Build the map of particles used for the missing mass" << endl;
	locSourceStream << "			//For beam: Don\'t want to group with final-state photons. Instead use \"Unknown\" PID (not ideal, but it\'s easy)." << endl;
	locSourceStream << "		map<Particle_t, set<Int_t> > locUsedThisCombo_MissingMass;" << endl;

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
				locSourceStream << "		locUsedThisCombo_MissingMass[Unknown].insert(locBeamID); //beam" << endl;
			else if(locParticleName.substr(0, 6) == "Target")
				continue;
			else if(locParticleName.substr(0, 8) == "Decaying")
				continue;
			else if(locParticleName.substr(0, 7) == "Missing")
				continue;
			else if(ParticleCharge(locPID) != 0)
				locSourceStream << "		locUsedThisCombo_MissingMass[" << EnumString(locPID) << "].insert(loc" << locParticleName << "TrackID);" << endl;
			else
				locSourceStream << "		locUsedThisCombo_MissingMass[" << EnumString(locPID) << "].insert(loc" << locParticleName << "NeutralID);" << endl;
		}
	}
	locSourceStream << endl;

	locSourceStream << "		//compare to what\'s been used so far" << endl;
	locSourceStream << "		if(locUsedSoFar_MissingMass.find(locUsedThisCombo_MissingMass) == locUsedSoFar_MissingMass.end())" << endl;
	locSourceStream << "		{" << endl;
	locSourceStream << "			//unique missing mass combo: histogram it, and register this combo of particles" << endl;
	locSourceStream << "			dHist_MissingMassSquared->Fill(locMissingMassSquared);" << endl;
	locSourceStream << "			locUsedSoFar_MissingMass.insert(locUsedThisCombo_MissingMass);" << endl;
	locSourceStream << "		}" << endl;
	locSourceStream << endl;
	locSourceStream << "		//E.g. Cut" << endl;
	locSourceStream << "		//if((locMissingMassSquared < -0.04) || (locMissingMassSquared > 0.04))" << endl;
	locSourceStream << "		//{" << endl;
	locSourceStream << "		//	dComboWrapper->Set_IsComboCut(true);" << endl;
	locSourceStream << "		//	continue;" << endl;
	locSourceStream << "		//}" << endl;
	locSourceStream << endl;


	if(locConfiguration != "0") locSourceStream << "		/****************************************** FILL HISTOGRAM WHILE KEEPING UNIQUENESSTRACKING ******************************************/" << endl;
	if(locConfiguration != "0") locSourceStream << endl;

	if(locConfiguration != "0") locSourceStream << "        Double_t histWeight = 1.0;" << endl;
	if(locConfiguration != "0") Add_UniqueHistogramFilling(locSourceStream, locConfiguration);

	locSourceStream << "		/****************************************** FILL FLAT TREE (IF DESIRED) ******************************************/" << endl;
	locSourceStream << endl;
	locSourceStream << "		/*" << endl;
	locSourceStream << "		//FILL ANY CUSTOM BRANCHES FIRST!!" << endl;
	locSourceStream << "		Int_t locMyInt_Flat = 7;" << endl;
	locSourceStream << "		dFlatTreeInterface->Fill_Fundamental<Int_t>(\"flat_my_int\", locMyInt_Flat);" << endl;
	locSourceStream << endl;
	locSourceStream << "		TLorentzVector locMyP4_Flat(4.0, 3.0, 2.0, 1.0);" << endl;
	locSourceStream << "		dFlatTreeInterface->Fill_TObject<TLorentzVector>(\"flat_my_p4\", locMyP4_Flat);" << endl;
	locSourceStream << endl;
	locSourceStream << "		for(int loc_j = 0; loc_j < locMyInt_Flat; ++loc_j)" << endl;
	locSourceStream << "		{" << endl;
	locSourceStream << "			dFlatTreeInterface->Fill_Fundamental<Int_t>(\"flat_my_int_array\", 3*loc_j, loc_j); //2nd argument = value, 3rd = array index" << endl;
	locSourceStream << "			TLorentzVector locMyComboP4_Flat(8.0, 7.0, 6.0, 5.0);" << endl;
	locSourceStream << "			dFlatTreeInterface->Fill_TObject<TLorentzVector>(\"flat_my_p4_array\", locMyComboP4_Flat, loc_j);" << endl;
	locSourceStream << "		}" << endl;
	locSourceStream << "		*/" << endl;
	locSourceStream << endl;
	locSourceStream << "		//FILL FLAT TREE" << endl;
	locSourceStream << "		//Fill_FlatTree(); //for the active combo" << endl;
	locSourceStream << "	} // end of combo loop" << endl;
	locSourceStream << endl;
	locSourceStream << "	//FILL HISTOGRAMS: Num combos / events surviving actions" << endl;
	locSourceStream << "	Fill_NumCombosSurvivedHists();" << endl;
	locSourceStream << endl;
	locSourceStream << "	/******************************************* LOOP OVER THROWN DATA (OPTIONAL) ***************************************/" << endl;
	locSourceStream << "/*" << endl;
	locSourceStream << "	//Thrown beam: just use directly" << endl;
	locSourceStream << "	if(dThrownBeam != NULL)" << endl;
	locSourceStream << "		double locEnergy = dThrownBeam->Get_P4().E();" << endl;
	locSourceStream << endl;
	locSourceStream << "	//Loop over throwns" << endl;
	locSourceStream << "	for(UInt_t loc_i = 0; loc_i < Get_NumThrown(); ++loc_i)" << endl;
	locSourceStream << "	{" << endl;
	locSourceStream << "		//Set branch array indices corresponding to this particle" << endl;
	locSourceStream << "		dThrownWrapper->Set_ArrayIndex(loc_i);" << endl;
	locSourceStream << endl;
	locSourceStream << "		//Do stuff with the wrapper here ..." << endl;
	locSourceStream << "	}" << endl;
	locSourceStream << "*/" << endl;
	locSourceStream << "	/****************************************** LOOP OVER OTHER ARRAYS (OPTIONAL) ***************************************/" << endl;
	locSourceStream << "/*" << endl;
	locSourceStream << "	//Loop over beam particles (note, only those appearing in combos are present)" << endl;
	locSourceStream << "	for(UInt_t loc_i = 0; loc_i < Get_NumBeam(); ++loc_i)" << endl;
	locSourceStream << "	{" << endl;
	locSourceStream << "		//Set branch array indices corresponding to this particle" << endl;
	locSourceStream << "		dBeamWrapper->Set_ArrayIndex(loc_i);" << endl;
	locSourceStream << endl;
	locSourceStream << "		//Do stuff with the wrapper here ..." << endl;
	locSourceStream << "	}" << endl;
	locSourceStream << endl;
	locSourceStream << "	//Loop over charged track hypotheses" << endl;
	locSourceStream << "	for(UInt_t loc_i = 0; loc_i < Get_NumChargedHypos(); ++loc_i)" << endl;
	locSourceStream << "	{" << endl;
	locSourceStream << "		//Set branch array indices corresponding to this particle" << endl;
	locSourceStream << "		dChargedHypoWrapper->Set_ArrayIndex(loc_i);" << endl;
	locSourceStream << endl;
	locSourceStream << "		//Do stuff with the wrapper here ..." << endl;
	locSourceStream << "	}" << endl;
	locSourceStream << endl;
	locSourceStream << "	//Loop over neutral particle hypotheses" << endl;
	locSourceStream << "	for(UInt_t loc_i = 0; loc_i < Get_NumNeutralHypos(); ++loc_i)" << endl;
	locSourceStream << "	{" << endl;
	locSourceStream << "		//Set branch array indices corresponding to this particle" << endl;
	locSourceStream << "		dNeutralHypoWrapper->Set_ArrayIndex(loc_i);" << endl;
	locSourceStream << endl;
	locSourceStream << "		//Do stuff with the wrapper here ..." << endl;
	locSourceStream << "	}" << endl;
	locSourceStream << "*/" << endl;
	locSourceStream << endl;
	locSourceStream << "	/************************************ EXAMPLE: FILL CLONE OF TTREE HERE WITH CUTS APPLIED ************************************/" << endl;
	locSourceStream << "/*" << endl;
	locSourceStream << "	Bool_t locIsEventCut = true;" << endl;
	locSourceStream << "	for(UInt_t loc_i = 0; loc_i < Get_NumCombos(); ++loc_i) {" << endl;
	locSourceStream << "		//Set branch array indices for combo and all combo particles" << endl;
	locSourceStream << "		dComboWrapper->Set_ComboIndex(loc_i);" << endl;
	locSourceStream << "		// Is used to indicate when combos have been cut" << endl;
	locSourceStream << "		if(dComboWrapper->Get_IsComboCut())" << endl;
	locSourceStream << "			continue;" << endl;
	locSourceStream << "		locIsEventCut = false; // At least one combo succeeded" << endl;
	locSourceStream << "		break;" << endl;
	locSourceStream << "	}" << endl;
	locSourceStream << "	if(!locIsEventCut && dOutputTreeFileName != \"\")" << endl;
	locSourceStream << "		Fill_OutputTree();" << endl;
	locSourceStream << "*/" << endl;
	locSourceStream << endl;
	locSourceStream << "	return kTRUE;" << endl;
	locSourceStream << "}" << endl;
	locSourceStream << endl;
	locSourceStream << "void " << locSelectorName << "::Finalize(void)" << endl;
	locSourceStream << "{" << endl;
	locSourceStream << "	//Save anything to output here that you do not want to be in the default DSelector output ROOT file." << endl;
	locSourceStream << endl;
	locSourceStream << "	//Otherwise, don\'t do anything else (especially if you are using PROOF)." << endl;
	locSourceStream << "		//If you are using PROOF, this function is called on each thread," << endl;
	locSourceStream << "		//so anything you do will not have the combined information from the various threads." << endl;
	locSourceStream << "		//Besides, it is best-practice to do post-processing (e.g. fitting) separately, in case there is a problem." << endl;
	locSourceStream << endl;
	locSourceStream << "	//DO YOUR STUFF HERE" << endl;
	locSourceStream << endl;
	locSourceStream << "	//CALL THIS LAST" << endl;
	locSourceStream << "	DSelector::Finalize(); //Saves results to the output file" << endl;
	locSourceStream << "}" << endl;

	locSourceStream.close();
}





void Print_HeaderFile_MCGen(string locSelectorBaseName, DTreeInterface* locTreeInterface)
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
	locHeaderStream << endl;
	locHeaderStream << "	ClassDef(" << locSelectorName << ", 0);" << endl;
	locHeaderStream << "};" << endl;
	locHeaderStream << endl;
	locHeaderStream << "#endif // " << locSelectorName << "_h" << endl;

	locHeaderStream.close();
}

void Print_SourceFile_MCGen(string locSelectorBaseName, DTreeInterface* locTreeInterface)
{
	string locSelectorName = string("DSelector_") + locSelectorBaseName;
	string locSourceName = locSelectorName + string(".C");
	ofstream locSourceStream;
	locSourceStream.open(locSourceName.c_str());

	locSourceStream << "#include \"" << locSelectorName << ".h\"" << endl;
	locSourceStream << endl;
	locSourceStream << "void " << locSelectorName << "::Init(TTree *locTree)" << endl;
	locSourceStream << "{" << endl;
	locSourceStream << "	// USERS: IN THIS FUNCTION, ONLY MODIFY SECTIONS WITH A \"USER\" OR \"EXAMPLE\" LABEL. LEAVE THE REST ALONE." << endl;
	locSourceStream << endl;
	locSourceStream << "	// The Init() function is called when the selector needs to initialize a new tree or chain." << endl;
	locSourceStream << "	// Typically here the branch addresses and branch pointers of the tree will be set." << endl;
	locSourceStream << "	// Init() will be called many times when running on PROOF (once per file to be processed)." << endl;
	locSourceStream << endl;
	locSourceStream << "	//USERS: SET OUTPUT FILE NAME //can be overriden by user in PROOF" << endl;
	locSourceStream << "	dOutputFileName = \"" << locSelectorBaseName << ".root\"; //\"\" for none" << endl;
	locSourceStream << "	//USERS: SET OUTPUT TREE FILES/NAMES //e.g. binning into separate files for AmpTools" << endl;
	locSourceStream << "	//dOutputTreeFileNameMap[\"Bin1\"] = \"mcgen_bin1.root\"; //key is user-defined, value is output file name" << endl;
	locSourceStream << "	//dOutputTreeFileNameMap[\"Bin2\"] = \"mcgen_bin2.root\"; //key is user-defined, value is output file name" << endl;
	locSourceStream << "	//dOutputTreeFileNameMap[\"Bin3\"] = \"mcgen_bin3.root\"; //key is user-defined, value is output file name" << endl;
	locSourceStream << endl;
	locSourceStream << "	//Because this function gets called for each TTree in the TChain, we must be careful:" << endl;
	locSourceStream << "		//We need to re-initialize the tree interface & branch wrappers, but don't want to recreate histograms" << endl;
	locSourceStream << "	bool locInitializedPriorFlag = dInitializedFlag; //save whether have been initialized previously" << endl;
	locSourceStream << "	DSelector::Init(locTree); //This must be called to initialize wrappers for each new TTree" << endl;
	locSourceStream << "	//gDirectory now points to the output file with name dOutputFileName (if any)" << endl;
	locSourceStream << "	if(locInitializedPriorFlag)" << endl;
	locSourceStream << "		return; //have already created histograms, etc. below: exit" << endl;
	locSourceStream << endl;
	locSourceStream << "	dPreviousRunNumber = 0;" << endl;
	locSourceStream << endl;
	locSourceStream << "	/******************************** EXAMPLE USER INITIALIZATION: STAND-ALONE HISTOGRAMS *******************************/" << endl;
	locSourceStream << endl;
	locSourceStream << "	/************************************* ADVANCED EXAMPLE: CHOOSE BRANCHES TO READ ************************************/" << endl;
	locSourceStream << endl;
	locSourceStream << "	//TO SAVE PROCESSING TIME" << endl;
	locSourceStream << "		//If you know you don't need all of the branches/data, but just a subset of it, you can speed things up" << endl;
	locSourceStream << "		//By default, for each event, the data is retrieved for all branches" << endl;
	locSourceStream << "		//If you know you only need data for some branches, you can skip grabbing data from the branches you don't need" << endl;
	locSourceStream << "		//Do this by doing something similar to the commented code below" << endl;
	locSourceStream << endl;
	locSourceStream << "	//dTreeInterface->Clear_GetEntryBranches(); //now get none" << endl;
	locSourceStream << "	//dTreeInterface->Register_GetEntryBranch(\"Proton__P4\"); //manually set the branches you want" << endl;
	locSourceStream << "}" << endl;
	locSourceStream << endl;


	locSourceStream << "Bool_t " << locSelectorName << "::Process(Long64_t locEntry)" << endl;
	locSourceStream << "{" << endl;
	locSourceStream << "	// The Process() function is called for each entry in the tree. The entry argument" << endl;
	locSourceStream << "	// specifies which entry in the currently loaded tree is to be processed." << endl;
	locSourceStream << "	//" << endl;
	locSourceStream << "	// This function should contain the \"body\" of the analysis. It can contain" << endl;
	locSourceStream << "	// simple or elaborate selection criteria, run algorithms on the data" << endl;
	locSourceStream << "	// of the event and typically fill histograms." << endl;
	locSourceStream << "	//" << endl;
	locSourceStream << "	// The processing can be stopped by calling Abort()." << endl;
	locSourceStream << "	// Use fStatus to set the return value of TTree::Process()." << endl;
	locSourceStream << "	// The return value is currently not used." << endl;
	locSourceStream << endl;
	locSourceStream << "	//CALL THIS FIRST" << endl;
	locSourceStream << "	DSelector::Process(locEntry); //Gets the data from the tree for the entry" << endl;
	locSourceStream << "	//cout << \"RUN \" << Get_RunNumber() << \", EVENT \" << Get_EventNumber() << endl;" << endl;
	locSourceStream << endl;
	locSourceStream << "	/******************************************** GET POLARIZATION ORIENTATION ******************************************/" << endl;
	locSourceStream << endl;
	locSourceStream << "	//Only if the run number changes" << endl;
	locSourceStream << "	//RCDB environment must be setup in order for this to work! (Will return false otherwise)" << endl;
	locSourceStream << "	UInt_t locRunNumber = Get_RunNumber();" << endl;
	locSourceStream << "	if(locRunNumber != dPreviousRunNumber)" << endl;
	locSourceStream << "	{" << endl;
	locSourceStream << "		dIsPolarizedFlag = dAnalysisUtilities.Get_IsPolarizedBeam(locRunNumber, dIsPARAFlag);" << endl;
	locSourceStream << "		dPreviousRunNumber = locRunNumber;" << endl;
	locSourceStream << "	}" << endl;
	locSourceStream << endl;
	locSourceStream << "	/********************************************* SETUP UNIQUENESS TRACKING ********************************************/" << endl;
	locSourceStream << endl;
	locSourceStream << "	//INSERT USER ANALYSIS UNIQUENESS TRACKING HERE" << endl;
	locSourceStream << endl;
	locSourceStream << "	/******************************************* LOOP OVER THROWN DATA ***************************************/" << endl;
	locSourceStream << endl;
	locSourceStream << "	//Thrown beam: just use directly" << endl;
	locSourceStream << "	double locBeamEnergyUsedForBinning = 0.0;" << endl;
	locSourceStream << "	if(dThrownBeam != NULL)" << endl;
	locSourceStream << "		locBeamEnergyUsedForBinning = dThrownBeam->Get_P4().E();" << endl;
	locSourceStream << endl;
	locSourceStream << "	//Loop over throwns" << endl;
	locSourceStream << "	for(UInt_t loc_i = 0; loc_i < Get_NumThrown(); ++loc_i)" << endl;
	locSourceStream << "	{" << endl;
	locSourceStream << "		//Set branch array indices corresponding to this particle" << endl;
	locSourceStream << "		dThrownWrapper->Set_ArrayIndex(loc_i);" << endl;
	locSourceStream << endl;
	locSourceStream << "		//Do stuff with the wrapper here ..." << endl;
	locSourceStream << "		Particle_t locPID = dThrownWrapper->Get_PID();" << endl;
	locSourceStream << "		TLorentzVector locThrownP4 = dThrownWrapper->Get_P4();" << endl;
	locSourceStream << "		//cout << \"Thrown \" << loc_i << \": \" << locPID << \", \" << locThrownP4.Px() << \", \" << locThrownP4.Py() << \", \" << locThrownP4.Pz() << \", \" << locThrownP4.E() << endl;" << endl;
	locSourceStream << "	}" << endl;
	locSourceStream << endl;
	locSourceStream << "	//OR Manually:" << endl;
	locSourceStream << "	//BEWARE: Do not expect the particles to be at the same array indices from one event to the next!!!!" << endl;
	locSourceStream << "	//Why? Because while your channel may be the same, the pions/kaons/etc. will decay differently each event." << endl;
	locSourceStream << endl;
	locSourceStream << "	//BRANCHES: https://halldweb.jlab.org/wiki/index.php/Analysis_TTreeFormat#TTree_Format:_Simulated_Data" << endl;
	locSourceStream << "	TClonesArray** locP4Array = dTreeInterface->Get_PointerToPointerTo_TClonesArray(\"Thrown__P4\");" << endl;
	locSourceStream << "	TBranch* locPIDBranch = dTreeInterface->Get_Branch(\"Thrown__PID\");" << endl;
	locSourceStream << "/*" << endl;
	locSourceStream << "	Particle_t locThrown1PID = PDGtoPType(((Int_t*)locPIDBranch->GetAddress())[0]);" << endl;
	locSourceStream << "	TLorentzVector locThrown1P4 = *((TLorentzVector*)(*locP4Array)->At(0));" << endl;
	locSourceStream << "	cout << \"Particle 1: \" << locThrown1PID << \", \" << locThrown1P4.Px() << \", \" << locThrown1P4.Py() << \", \" << locThrown1P4.Pz() << \", \" << locThrown1P4.E() << endl;" << endl;
	locSourceStream << "	Particle_t locThrown2PID = PDGtoPType(((Int_t*)locPIDBranch->GetAddress())[1]);" << endl;
	locSourceStream << "	TLorentzVector locThrown2P4 = *((TLorentzVector*)(*locP4Array)->At(1));" << endl;
	locSourceStream << "	cout << \"Particle 2: \" << locThrown2PID << \", \" << locThrown2P4.Px() << \", \" << locThrown2P4.Py() << \", \" << locThrown2P4.Pz() << \", \" << locThrown2P4.E() << endl;" << endl;
	locSourceStream << "*/" << endl;
	locSourceStream << endl;
	locSourceStream << endl;
	locSourceStream << "	/******************************************* BIN THROWN DATA INTO SEPARATE TREES FOR AMPTOOLS ***************************************/" << endl;
	locSourceStream << endl;
	locSourceStream << "/*" << endl;
	locSourceStream << "	//THESE KEYS MUST BE DEFINED IN THE INIT SECTION (along with the output file names)" << endl;
	locSourceStream << "	if((locBeamEnergyUsedForBinning >= 8.0) && (locBeamEnergyUsedForBinning < 9.0))" << endl;
	locSourceStream << "		Fill_OutputTree(\"Bin1\"); //your user-defined key" << endl;
	locSourceStream << "	else if((locBeamEnergyUsedForBinning >= 9.0) && (locBeamEnergyUsedForBinning < 10.0))" << endl;
	locSourceStream << "		Fill_OutputTree(\"Bin2\"); //your user-defined key" << endl;
	locSourceStream << "	else if((locBeamEnergyUsedForBinning >= 10.0) && (locBeamEnergyUsedForBinning < 11.0))" << endl;
	locSourceStream << "		Fill_OutputTree(\"Bin3\"); //your user-defined key" << endl;
	locSourceStream << "*/" << endl;
	locSourceStream << endl;
	locSourceStream << "	return kTRUE;" << endl;
	locSourceStream << "}" << endl;
	locSourceStream << endl;
	locSourceStream << "void " << locSelectorName << "::Finalize(void)" << endl;
	locSourceStream << "{" << endl;
	locSourceStream << "	//Save anything to output here that you do not want to be in the default DSelector output ROOT file." << endl;
	locSourceStream << endl;
	locSourceStream << "	//Otherwise, don\'t do anything else (especially if you are using PROOF)." << endl;
	locSourceStream << "		//If you are using PROOF, this function is called on each thread," << endl;
	locSourceStream << "		//so anything you do will not have the combined information from the various threads." << endl;
	locSourceStream << "		//Besides, it is best-practice to do post-processing (e.g. fitting) separately, in case there is a problem." << endl;
	locSourceStream << endl;
	locSourceStream << "	//DO YOUR STUFF HERE" << endl;
	locSourceStream << endl;
	locSourceStream << "	//CALL THIS LAST" << endl;
	locSourceStream << "	DSelector::Finalize(); //Saves results to the output file" << endl;
	locSourceStream << "}" << endl;

	locSourceStream.close();
}


void Add_Action(std::ofstream& locOfStream, string locConfig)
{
	std::ifstream infile(locConfig);	
	std::string line;
	bool SectionBeginFlag = 0;
	bool SectionEndFlag = 0;

	while(std::getline(infile, line))
	{
		std::string lineReader = line.substr(0,line.find("#"));
		if(lineReader == "") continue;
		//std::cout << lineReader << std::endl;
		if(lineReader.find("[SECTION: ANALYSIS ACTION]") <lineReader.length()) SectionBeginFlag = 1;
		if(lineReader.find("[SECTION: KINEMATIC CALCULATION]") <lineReader.length())                  SectionEndFlag = 1;

		if(SectionBeginFlag==1 && SectionEndFlag==0)
		{
			bool CutActionFlag = (lineReader.find("CUT:")<lineReader.length() );
			bool HistActionFlag = (lineReader.find("HIST:")<lineReader.length() );
			std::string Options;
			if(CutActionFlag || HistActionFlag) Options = lineReader.substr(lineReader.find('=')+1, lineReader.length()-lineReader.find('='));
			while( Options.find(',')!= string::npos ) Options.replace(Options.find(','), 1, " ");
			//std::cout << Options << std::endl;
			
			if(CutActionFlag==1)
			{
				//write the Cut Action Initiallization in DSelector
				if( lineReader.find("ChiSqOrCL") < lineReader.length() )
				{
					std::string SecondaryReactionName, IsChiSq, Function, ActionUniqueString;
					std::istringstream iss(Options);
					iss >> SecondaryReactionName >> IsChiSq >> Function >> ActionUniqueString;
					locOfStream << "	dAnalysisActions.push_back(new DCutAction_ChiSqOrCL(dComboWrapper, \"" << SecondaryReactionName << "\", " << IsChiSq << ", " << Function << ", \"" << ActionUniqueString << "\"));" << std::endl;
				}  
				if( lineReader.find("KinFitFOM") < lineReader.length() )
				{
					std::string MinimumConfidenceLevel, ActionUniqueString;
					std::istringstream iss(Options);
					iss >> MinimumConfidenceLevel >> ActionUniqueString;
					locOfStream << "	dAnalysisActions.push_back(new DCutAction_KinFitFOM(dComboWrapper, " << MinimumConfidenceLevel << ", \"" << ActionUniqueString << "\"));" << std::endl;
				} 
				if( lineReader.find("BeamEnergy") < lineReader.length() )
				{
					std::string UseKinFitFlag, MinBeamEnergy, MaxBeamEnergy, ActionUniqueString;
					std::istringstream iss(Options);
					iss >> UseKinFitFlag >> MinBeamEnergy >> MaxBeamEnergy >> ActionUniqueString;
					locOfStream << "	dAnalysisActions.push_back(new DCutAction_BeamEnergy(dComboWrapper, " << UseKinFitFlag << ", " << MinBeamEnergy << ", " << MaxBeamEnergy << ", \"" << ActionUniqueString << "\"));" << std::endl;
				} 
			}
			
			if(HistActionFlag==1)
			{
				//write the Cut Action Initiallization in DSelector
				if( lineReader.find("ParticleComboKinematics") < lineReader.length() )
				{
					std::string UseKinFitFlag, ActionUniqueString;
					std::istringstream iss(Options);
					iss >> UseKinFitFlag >> ActionUniqueString;
					locOfStream << "	dAnalysisActions.push_back(new DHistogramAction_ParticleComboKinematics(dComboWrapper, " << UseKinFitFlag << ", \"" << ActionUniqueString << "\"));" << std::endl;
				}  
				if( lineReader.find("ParticleID") < lineReader.length() )
				{
					std::string UseKinFitFlag, ActionUniqueString;
					std::istringstream iss(Options);
					iss >> UseKinFitFlag >> ActionUniqueString;
					locOfStream << "	dAnalysisActions.push_back(new DHistogramAction_ParticleID(dComboWrapper, " << UseKinFitFlag << ", \"" << ActionUniqueString << "\"));" << std::endl;
				}
				if( lineReader.find("InvariantMass") < lineReader.length() )
				{
					std::string UseKinFitFlag, InitialPID, NumMassBins, MinMass, MaxMass, ActionUniqueString;
					std::istringstream iss(Options);
					iss >> UseKinFitFlag >> InitialPID >> NumMassBins >> MinMass >> MaxMass >> ActionUniqueString;
					locOfStream << "	dAnalysisActions.push_back(new DHistogramAction_InvariantMass(dComboWrapper, " << UseKinFitFlag  << ", " << InitialPID  << ", " << NumMassBins  << ", " << MinMass  << ", " << MaxMass << ", \"" << ActionUniqueString << "\"));" << std::endl;
				} 
				if( lineReader.find("MissingMass") < lineReader.length() && !(lineReader.find("MissingMassSquared") < lineReader.length()))
				{
					std::string UseKinFitFlag, NumMassBins, MinMass, MaxMass, ActionUniqueString;
					std::istringstream iss(Options);
					iss >> UseKinFitFlag >> NumMassBins >> MinMass >> MaxMass >> ActionUniqueString;
					locOfStream << "	dAnalysisActions.push_back(new DHistogramAction_MissingMass(dComboWrapper, " << UseKinFitFlag  << ", " << NumMassBins  << ", " << MinMass  << ", " << MaxMass << ", \"" << ActionUniqueString << "\"));" << std::endl;
				}
				if( lineReader.find("MissingMassSquared") < lineReader.length() )
				{
					std::string UseKinFitFlag, NumMassBins, MinMass, MaxMass, ActionUniqueString;
					std::istringstream iss(Options);
					iss >> UseKinFitFlag >> NumMassBins >> MinMass >> MaxMass >> ActionUniqueString;
					locOfStream << "	dAnalysisActions.push_back(new DHistogramAction_MissingMassSquared(dComboWrapper, " << UseKinFitFlag  << ", " << NumMassBins  << ", " << MinMass  << ", " << MaxMass << ", \"" << ActionUniqueString << "\"));" << std::endl;
				}

				if( lineReader.find("MissingP4") < lineReader.length() )
				{
					std::string UseKinFitFlag, ActionUniqueString;
					std::istringstream iss(Options);
					iss >> UseKinFitFlag >> ActionUniqueString;
					locOfStream << "	dAnalysisActions.push_back(new DHistogramAction_MissingP4(dComboWrapper, " << UseKinFitFlag << ", \"" << ActionUniqueString << "\"));" << std::endl;
				}
			}
		}
	}
	infile.close();
}

void Add_HistogramDefinition(std::ofstream& locOfStream, string locConfig)
{
	std::ifstream infile(locConfig);	
	std::string line;
	bool SectionBeginFlag = 0;	

	while(std::getline(infile, line))
	{
		std::string lineReader = line.substr(0,line.find("#"));
		if(lineReader == "") continue;
		if(lineReader.find("[SECTION: HISTOGRAMS]") <lineReader.length()) SectionBeginFlag = 1;
		
		if(SectionBeginFlag==1)
		{	
			if(lineReader.substr(0,9) != "HISTOGRAM") continue;

			std::size_t firstQuote = lineReader.find("\"");
			std::size_t secondQuote = lineReader.find("\"", firstQuote+1);
			std::string titles = line.substr(firstQuote+1, secondQuote-firstQuote-1);
			lineReader.erase(firstQuote, secondQuote-firstQuote+1);
			std::string histOptions = lineReader.substr(lineReader.find("=")+1, lineReader.length()-lineReader.find("=")-1);
			std::istringstream iss(histOptions);
			string histType, keyName;//, binxN, xMin, xMax, binyN, yMin, yMax, xVariable, yVariable, UniquenessTracking;
			iss >> histType >> keyName;// >> binxN >> xMin >> xMax >> binyN >> yMin >> yMax >> xVariable >> yVariable >> UniquenessTracking;
	    	locOfStream << "		" << histType << "* " << "dHist_" << keyName << ";"<< endl;
	    }
	}
	infile.close();
}

void Add_HistogramInitialization(std::ofstream& locOfStream, string locConfig)
{
	std::ifstream infile(locConfig);	
	std::string line;
	bool SectionBeginFlag = 0;

	while(std::getline(infile, line))
	{
		std::string lineReader = line.substr(0,line.find("#"));
		if(lineReader == "") continue;
		if(lineReader.find("[SECTION: HISTOGRAMS]") <lineReader.length()) SectionBeginFlag = 1;
		if(SectionBeginFlag==1)
		{	
			if(lineReader.substr(0,9) != "HISTOGRAM") continue;

			std::size_t firstQuote = lineReader.find("\"");
			std::size_t secondQuote = lineReader.find("\"", firstQuote+1);
			std::string titles = line.substr(firstQuote+1, secondQuote-firstQuote-1);
			while(titles.find("\\")<titles.length()) titles.replace(titles.find("\\"), 1, "#");
			lineReader.erase(firstQuote, secondQuote-firstQuote+1);
			std::string histOptions = lineReader.substr(lineReader.find("=")+1, lineReader.length()-lineReader.find("=")-1);
			std::istringstream iss(histOptions);
			string histType, keyName, binxN, xMin, xMax, binyN, yMin, yMax, xVariable, yVariable, UniquenessTracking;
			iss >> histType >> keyName >> binxN >> xMin >> xMax >> binyN >> yMin >> yMax >> xVariable >> yVariable >> UniquenessTracking;
			if(histType.at(2) == '1') locOfStream << "	dHist_" << keyName << " = new " << histType << "(\"" << keyName << "\", \"" << titles << "\", " << binxN << ", " << xMin << ", " << xMax << ");"<< std::endl;
	    	if(histType.at(2) == '2') locOfStream << "	dHist_" << keyName << " = new " << histType << "(\"" << keyName << "\", \"" << titles << "\", " << binxN << ", " << xMin << ", " << xMax << ", " << binyN << ", " << yMin << ", " << yMax << ");"<< std::endl;
		}	
	}
	infile.close();
}

void Add_UniquenessTrackingDefinition(std::ofstream& locOfStream, string locConfig)
{
	std::ifstream infile(locConfig);	
	std::string line;
	bool SectionBeginFlag = 0;
	bool SectionEndFlag = 0;
	bool ComboSpecificFlag, ParticleSpecificFlag;

	while(std::getline(infile, line))
	{
		std::string lineReader = line.substr(0,line.find("#"));
		if(lineReader == "") continue;
		if(lineReader.find("[SECTION: UNIQUENESS TRACKING]") <lineReader.length()) SectionBeginFlag = 1;
		if(lineReader.find("[SECTION: HISTOGRAMS]") <lineReader.length()) SectionEndFlag = 1;

		ComboSpecificFlag    = 0;
		ParticleSpecificFlag = 0;
		if(SectionBeginFlag==1 && SectionEndFlag ==0)
		{	
			if(lineReader.find("Particle-specific")<lineReader.length()) ParticleSpecificFlag = 1;
			if(lineReader.find("Combo-specific")<lineReader.length()) ComboSpecificFlag = 1;
			std::size_t leftParanthesis = lineReader.find("(");
			std::size_t rightParanthesis = lineReader.find(")", leftParanthesis+1);
			std::string UnqName = line.substr(leftParanthesis+1, rightParanthesis-leftParanthesis-1);

			if(ParticleSpecificFlag == 1) locOfStream << "    set<Int_t> locUsedSoFar_" << UnqName << ";" << std::endl;
			if(ComboSpecificFlag == 1) locOfStream << "    set<map<Particle_t, set<Int_t> > > locUsedSoFar_" << UnqName << ";" << std::endl;
		}
	}
	infile.close();
}

void FillHist(std::ofstream& locOfStream, string locConfig, string UniquenessName)
{
	std::ifstream infile(locConfig);	
	std::string line;
	bool SectionBeginFlag = 0;

	while(std::getline(infile, line))
	{
		std::string lineReader = line.substr(0,line.find("#"));
		if(lineReader == "") continue;
		if(lineReader.find("[SECTION: HISTOGRAMS]") <lineReader.length()) SectionBeginFlag = 1;
		if(SectionBeginFlag==1)
		{	
			if(lineReader.substr(0,9) != "HISTOGRAM") continue;

			std::size_t firstQuote = lineReader.find("\"");
			std::size_t secondQuote = lineReader.find("\"", firstQuote+1);
			std::string titles = line.substr(firstQuote+1, secondQuote-firstQuote-1);
			lineReader.erase(firstQuote, secondQuote-firstQuote+1);
			std::string histOptions = lineReader.substr(lineReader.find("=")+1, lineReader.length()-lineReader.find("=")-1);
			std::istringstream iss(histOptions);
			string histType, keyName, binxN, xMin, xMax, binyN, yMin, yMax, xVariable, yVariable, UniquenessTracking;
			iss >> histType >> keyName >> binxN >> xMin >> xMax >> binyN >> yMin >> yMax >> xVariable >> yVariable >> UniquenessTracking;
			if(UniquenessName != UniquenessTracking) continue;
			if(histType.at(2) == '1') locOfStream << "	        dHist_" << keyName << " -> Fill( " << xVariable << ", histWeight);"<< std::endl;
	    	if(histType.at(2) == '2') locOfStream << "	        dHist_" << keyName << " -> Fill( " << xVariable << ", " << yVariable << ", histWeight);"<< std::endl;
		}	
	}
	infile.close();
}

void Add_UniqueHistogramFilling(std::ofstream& locOfStream, string locConfig)
{
	std::ifstream infile(locConfig);	
	std::string line;
	bool SectionBeginFlag = 0;
	bool SectionEndFlag = 0;

	while(std::getline(infile, line))
	{
		std::string lineReader = line.substr(0,line.find("#"));
		if(lineReader == "") continue;
		if(lineReader.find("[SECTION: UNIQUENESS TRACKING]") <lineReader.length()) SectionBeginFlag = 1;
		if(lineReader.find("[SECTION: HISTOGRAMS]") <lineReader.length()) SectionEndFlag = 1;
		if(SectionBeginFlag==1 && SectionEndFlag==0)
		{	
			if(lineReader.find("No-Uniqueness-Tracking")<lineReader.length())
				{
					locOfStream << "        //No Uniqueness tracking: Add one entry to histograms for every combo." << std::endl;
					locOfStream << "        //Carefully choose histograms to be filled in this section. Be aware of double-counting." << std::endl;
					std::string Unq = "-";
					FillHist(locOfStream, locConfig, Unq);
				} 

			std::size_t leftParanthesis = lineReader.find("(");
			std::size_t rightParanthesis = lineReader.find(")", leftParanthesis+1);
			std::string UnqName = line.substr(leftParanthesis+1, rightParanthesis-leftParanthesis-1);
			std::string particleList = lineReader.substr(lineReader.find("=")+1, lineReader.length()-lineReader.find("=")-1);
			std::istringstream iss(particleList);

			if(lineReader.find("Particle-specific")<lineReader.length())
				{
					std::string particleName;
					iss >> particleName;
					locOfStream << "        //Uniqueness tracking: Build the map of particle used for the " << UnqName << std::endl;
					locOfStream << "        if(locUsedSoFar_" << UnqName << ".find(loc" << particleName << "ID) == locUsedSoFar_" << UnqName << ".end())" << std::endl;
					locOfStream << "        {" << std::endl;
					std::string Unq = "Particle-specific("; Unq += UnqName; Unq += ")";
					FillHist(locOfStream, locConfig, Unq);
					locOfStream << "        	locUsedSoFar_" << UnqName << ".insert(loc" << particleName << "ID);" << std::endl;
					locOfStream << "        }" << std::endl;
				} 

			if(lineReader.find("Combo-specific")<lineReader.length()) 
				{
					locOfStream << "        //Uniqueness tracking: Build the map of particles used for the " << UnqName << std::endl;
					locOfStream << "        map<Particle_t, set<Int_t> > locUsedThisCombo_" << UnqName << ";" << std::endl;
					std::string locParticleStr, lastStr;
					while(iss)
					{
						iss >> locParticleStr;
						if(locParticleStr == lastStr) continue;
						lastStr = locParticleStr;
						std::string locPID = locParticleStr;
						if(locParticleStr == "Beam")
						{
							locPID = "Unknown";//For beam: Don't want to group with final-state photons. Instead use "Unknown" PID (not ideal, but it's easy).
							locOfStream << "        locUsedThisCombo_" << UnqName << "[" << locPID << "].insert(loc" << locParticleStr << "ID);" << std::endl;
						} 
						else
						{
							for(int locIndex = 1; locIndex<10; locIndex++)
							{
								std::size_t locPIDIndex = locParticleStr.find(std::to_string(locIndex));
								if( locPIDIndex < locParticleStr.length() ) locPID.erase(locPIDIndex, locParticleStr.length()-locPIDIndex+1);
							}
							locOfStream << "        locUsedThisCombo_" << UnqName << "[" << locPID << "].insert(loc" << locParticleStr << "TrackID);" << std::endl;
						}
					}
					locOfStream << "        //compare to what's been used so far" << std::endl;
					locOfStream << "        if(locUsedSoFar_" << UnqName << ".find(locUsedThisCombo_" << UnqName << ") == locUsedSoFar_" << UnqName << ".end())" << std::endl;
					locOfStream << "        {" << std::endl;
					std::string Unq = "Combo-specific("; Unq += UnqName; Unq += ")";
					FillHist(locOfStream, locConfig, Unq);
					locOfStream << "	        locUsedSoFar_" << UnqName << ".insert(locUsedThisCombo_" << UnqName << ");" << std::endl;
					locOfStream << "        }" << std::endl;
					locOfStream << endl;
					locOfStream << endl;
				}
		}	
	}
	infile.close();
}
void Add_KinematicCalculation(std::ofstream& locOfStream, string locConfig)
{
	std::ifstream infile(locConfig);	
	std::string line;
	bool SectionBeginFlag = 0;
	bool SectionEndFlag = 0;
	while(std::getline(infile, line))
	{
		std::string lineReader = line.substr(0,line.find("#"));
		if(lineReader == "") continue;
		if(lineReader.find("[SECTION: KINEMATIC CALCULATION]") <lineReader.length()) SectionBeginFlag = 1; 
		if(lineReader.find("[SECTION: UNIQUENESS TRACKING]") <lineReader.length()) 
		{
			SectionEndFlag = 1; 
		}

		if(lineReader.find("[SECTION: KINEMATIC CALCULATION]") <lineReader.length()) continue;
		if(lineReader.find("LorentzTransformation") <lineReader.length()) continue;
		if(SectionBeginFlag==1 && SectionEndFlag==0 )
		{	
			std::string locLine = "        ";
			locLine += lineReader;
			locOfStream << locLine << std::endl;
		}
	}
	infile.close();
}
void Add_LorentzBoost(std::ofstream& locOfStream, string locConfig, DTreeInterface* locTreeInterface, map<int, map<int, pair<Particle_t, string> > >& locComboInfoMap)
{
	std::ifstream infile(locConfig);	
	std::string line;
	bool SectionBeginFlag = 0;
	bool SectionEndFlag = 0;
	while(std::getline(infile, line))
	{
		std::string lineReader = line.substr(0,line.find("#"));
		if(lineReader == "") continue;
		if(lineReader.find("[SECTION: KINEMATIC CALCULATION]") <lineReader.length()) SectionBeginFlag = 1;
		if(lineReader.find("[SECTION: UNIQUENESS TRACKING]") <lineReader.length()) 
		{
			SectionEndFlag = 1; 
			continue;
		}
		if(!(lineReader.find("LorentzTransformation") <lineReader.length())) continue;

		if(SectionBeginFlag==1 && SectionEndFlag==0 )
		{	
			string BoostName;
			string originFrameName;
			string boostVector;
			std::size_t firstQuote = lineReader.find("\"");
			std::size_t secondQuote = lineReader.find("\"", firstQuote+1);
			boostVector = line.substr(firstQuote+1, secondQuote-firstQuote-1);
			std::string boostOpt = lineReader.substr(lineReader.find("=")+1, firstQuote-lineReader.find("=")-1);
			std::istringstream iss(boostOpt);
			iss >> BoostName >> originFrameName;

			if(originFrameName == "LAB") originFrameName = "";
			else originFrameName = "_" + originFrameName;

			locOfStream << "		/*********************************************** BOOST: " << BoostName << " FRAME **********************************************/" << endl;
			locOfStream << endl;

			//kinfit value
				locOfStream << "		// Get P4\'s: //is kinfit if kinfit performed, else is measured" << endl;
				locOfStream << "        TLorentzVector locBoostP4_" << BoostName << " = " << boostVector << "; " << endl;
				map<int, map<int, pair<Particle_t, string> > >::iterator locStepIterator = locComboInfoMap.begin();
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
						{
							locOfStream << "		TLorentzVector locBeamP4_" << BoostName << " = locBeamP4" << originFrameName << ";" << endl;
							locOfStream << "		locBeamP4_" << BoostName << ".Boost(-locBoostP4_" << BoostName << ".BoostVector());" << endl;
						}

						else if(locParticleName.substr(0, 6) == "Target")
						{
							locOfStream << "		TLorentzVector dTargetP4_" << BoostName << " = dTargetP4" << originFrameName << ";" << endl;
							locOfStream << "		dTargetP4_" << BoostName << ".Boost(-locBoostP4_" << BoostName << ".BoostVector());" << endl;
						}
						else if(locParticleName.substr(0, 8) == "Decaying")
						{
							string locBranchName = locParticleName + string("__P4_KinFit");
							if((locTreeInterface->Get_Branch(locBranchName) != NULL) && (locParticleIndex < 0)) //else not reconstructed
							{
								locOfStream << "		TLorentzVector loc" << locParticleName << "P4_" << BoostName << " = loc" << locParticleName << "P4" << originFrameName << ";" << endl;
								locOfStream << "		loc" << locParticleName << "P4_" << BoostName << ".Boost(-locBoostP4_" << BoostName << ".BoostVector());" << endl;
							}
						}
						else if(locParticleName.substr(0, 7) == "Missing")
						{
							string locBranchName = locParticleName + string("__P4_KinFit");
							if(locTreeInterface->Get_Branch(locBranchName) != NULL) //else not reconstructed
							{
								locOfStream << "		TLorentzVector loc" << locParticleName << "P4_" << BoostName << " = loc" << locParticleName << "P4" << originFrameName << ";" << endl;
								locOfStream << "		loc" << locParticleName << "P4_" << BoostName << ".Boost(-locBoostP4_" << BoostName << ".BoostVector());" << endl;
							}
						}
						else //detected
						{
							locOfStream << "		TLorentzVector loc" << locParticleName << "P4_" << BoostName << " = loc" << locParticleName << "P4" << originFrameName << ";" << endl;
							locOfStream << "		loc" << locParticleName << "P4_" << BoostName << ".Boost(-locBoostP4_" << BoostName << ".BoostVector());" << endl;
						}	
					}
				}
				locOfStream << endl;

			//measured value

				locOfStream << "		// Measured values" << endl;

				for(locStepIterator = locComboInfoMap.begin(); locStepIterator != locComboInfoMap.end(); ++locStepIterator)
				{
					int locStepIndex = locStepIterator->first;
					locOfStream << "		//Step " << locStepIndex << endl;

					map<int, pair<Particle_t, string> >& locStepInfoMap = locStepIterator->second;
					map<int, pair<Particle_t, string> >::iterator locParticleIterator = locStepInfoMap.begin();
					for(; locParticleIterator != locStepInfoMap.end(); ++locParticleIterator)
					{
						//int locParticleIndex = locParticleIterator->first;
						Particle_t locPID = locParticleIterator->second.first;
						string locParticleName = locParticleIterator->second.second;

						if(locPID == Unknown)
							continue;
						else if(locParticleName == "ComboBeam")
						{
							locOfStream << "		TLorentzVector locBeamP4_Measured_" << BoostName << " = locBeamP4_Measured" << originFrameName << ";" << endl;
							locOfStream << "		locBeamP4_Measured_" << BoostName << ".Boost(-locBoostP4_" << BoostName << ".BoostVector());" << endl;
						}

						else if(locParticleName.substr(0, 6) == "Target")
						{
							continue;
						}
						else if(locParticleName.substr(0, 8) == "Decaying")
						{
							continue;
						}
						else if(locParticleName.substr(0, 7) == "Missing")
						{
							string locBranchName = locParticleName + string("__P4_KinFit");
							if(locTreeInterface->Get_Branch(locBranchName) != NULL) //else not reconstructed
							{
								locOfStream << "		TLorentzVector loc" << locParticleName << "P4_Measured_" << BoostName << " = loc" << locParticleName << "P4_Measured" << originFrameName << ";" << endl;
								locOfStream << "		loc" << locParticleName << "P4_Measured_" << BoostName << ".Boost(-locBoostP4_" << BoostName << ".BoostVector());" << endl;
							}
						}
						else //detected
						{
							locOfStream << "		TLorentzVector loc" << locParticleName << "P4_Measured_" << BoostName << " = loc" << locParticleName << "P4_Measured" << originFrameName << ";" << endl;
							locOfStream << "		loc" << locParticleName << "P4_Measured_" << BoostName << ".Boost(-locBoostP4_" << BoostName << ".BoostVector());" << endl;
						}	
					}
				}
				locOfStream << endl;

		}
	}
	infile.close();
}