#include <string>
#include <map>
#include <iostream>
#include <fstream>
#include <TFile.h>
#include <TTree.h>
#include "DSelector/DTreeInterface.h"


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
		if(lineReader.find("[END: ANALYSIS ACTION]") <lineReader.length())                  SectionEndFlag = 1;

		if(SectionBeginFlag==1 && SectionEndFlag==0)
		{
			if(lineReader.find("DHistogramAction") <lineReader.length() || lineReader.find("DCutAction") <lineReader.length())
			{
				locOfStream << "	dAnalysisActions.push_back(new " << lineReader << ");" << std::endl;
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
	bool SectionEndFlag = 0;

	while(std::getline(infile, line))
	{
		std::string lineReader = line.substr(0,line.find("#"));
		if(lineReader == "") continue;
		if(lineReader.find("[SECTION: HISTOGRAMS]") <lineReader.length()) SectionBeginFlag = 1;
		if(lineReader.find("[END: HISTOGRAMS]") <lineReader.length())                  SectionEndFlag = 1;

		if(SectionBeginFlag==1 && SectionEndFlag==0)
		{	
			if(lineReader.substr(0,9) == "HISTOGRAM")
			{
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
			if(lineReader.substr(0,12) == "2D HISTOGRAM")
			{
				std::string histOptions = lineReader.substr(lineReader.find("=")+1, lineReader.length()-lineReader.find("=")-1);
				std::istringstream iss(histOptions);
				string histType, xHistName, yHistName, UniquenessTracking;
				iss >> histType >> xHistName >> yHistName >> UniquenessTracking;
				std::string keyName = yHistName + "_vs_" + xHistName;
				locOfStream << "		" << histType << "* " << "dHist_" << keyName << ";"<< endl;
		    }

			
	    }
	}
	infile.close();
}

void Add_HistogramInitialization(std::ofstream& locOfStream, string locConfig)
{
	std::ifstream infile(locConfig);	
	std::string line;
	bool SectionBeginFlag = 0;
	bool SectionEndFlag = 0;

	while(std::getline(infile, line))
	{
		std::string lineReader = line.substr(0,line.find("#"));
		if(lineReader == "") continue;
		if(lineReader.find("[SECTION: HISTOGRAMS]") <lineReader.length()) SectionBeginFlag = 1;
		if(lineReader.find("[END: HISTOGRAMS]") <lineReader.length())                 SectionEndFlag = 1;

		if(SectionBeginFlag==1 && SectionEndFlag==0)
		{	
			if(lineReader.substr(0,9) == "HISTOGRAM")
			{
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


			if(lineReader.substr(0,12) == "2D HISTOGRAM")
			{
				std::string histOptions = lineReader.substr(lineReader.find("=")+1, lineReader.length()-lineReader.find("=")-1);
				std::istringstream iss(histOptions);
				string histType, xHistName, yHistName, UniquenessTracking;
				iss >> histType >> xHistName >> yHistName >> UniquenessTracking;
				std::string keyName = yHistName + "_vs_" + xHistName;
				//Now search for xHist, yHist information in the config file
				std::ifstream infile2(locConfig);	
				std::string line2;
				std::string titles, binxN, xMin, xMax, binyN, yMin, yMax;
				std::string titles_X, titles_Y;
				xHistName.insert(0, " ");
				yHistName.insert(0, " ");
				xHistName+=" ";
				yHistName+=" ";

				bool xflag = false;
				bool yflag = false;

				while(std::getline(infile2, line2))
				{
					std::string lineReader2 = line2.substr(0,line.find("#"));
					if(lineReader2.substr(0,9) != "HISTOGRAM" || !(lineReader2.find("TH1")<lineReader2.length()) ) continue;
					std::size_t firstQuote, secondQuote;

					if(lineReader2.find(xHistName) <lineReader2.length())
					{
						xflag = true;
						firstQuote = lineReader2.find("\"");
						secondQuote = lineReader2.find("\"", firstQuote+1);
						titles_X = lineReader2.substr(firstQuote+1, secondQuote-firstQuote-1);
						lineReader2.erase(firstQuote, secondQuote-firstQuote+1);
						std::string histOptions_X = lineReader2.substr(lineReader2.find("=")+1, lineReader2.length()-lineReader2.find("=")-1);
						std::istringstream iss_X(histOptions_X);
						string histType_X, keyName_X, binyN0, yMin0, yMax0, xVariable0, yVariable0, UniquenessTracking_X;
						iss_X >> histType_X >> keyName_X >> binxN >> xMin >> xMax >> binyN0 >> yMin0 >> yMax0 >> xVariable0 >> yVariable0 >> UniquenessTracking_X;
					}

					if(lineReader2.find(yHistName) <lineReader2.length())
					{
						yflag = true;
						firstQuote = lineReader2.find("\"");
						secondQuote = lineReader2.find("\"", firstQuote+1);
						titles_Y = lineReader2.substr(firstQuote+1, secondQuote-firstQuote-1);
						lineReader2.erase(firstQuote, secondQuote-firstQuote+1);
						std::string histOptions_Y = lineReader2.substr(lineReader2.find("=")+1, lineReader2.length()-lineReader2.find("=")-1);
						std::istringstream iss_Y(histOptions_Y);
						string histType_Y, keyName_Y, binxN0, xMin0, xMax0, xVariable0, yVariable0, UniquenessTracking_Y;
						iss_Y >> histType_Y >> keyName_Y >> binyN >> yMin >> yMax >> binxN0 >> xMin0 >> xMax0 >>  yVariable0 >> xVariable0 >> UniquenessTracking_Y;
					}


				}
				infile2.close();
				if(xflag == false) cout << "search for histogram X failed!" << endl; 
				if(yflag == false) cout << "search for histogram Y failed!" << endl; 
				titles = titles_X + titles_Y;
				while(titles.find("\\")<titles.length()) titles.replace(titles.find("\\"), 1, "#");
		    	locOfStream << "	dHist_" << keyName << " = new " << histType << "(\"" << keyName << "\", \"" << titles << "\", " << binxN << ", " << xMin << ", " << xMax << ", " << binyN << ", " << yMin << ", " << yMax << ");"<< std::endl;		
			}


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
		if(lineReader.find("[SECTION: UNIQUENESS TRACKING]") <lineReader.length()) {
			SectionBeginFlag = 1;
			continue;
		}
		if(lineReader.find("[END: UNIQUENESS TRACKING]") <lineReader.length()) SectionEndFlag = 1;

		ComboSpecificFlag    = 0;
		ParticleSpecificFlag = 0;
		if(SectionBeginFlag==1 && SectionEndFlag ==0)
		{	
			if(lineReader.find("AfterComboLoop")<lineReader.length()){
				locOfStream << "    Int_t locNumberOfComboAfterCuts_" << "AfterComboLoop" << " = 0;" << std::endl;
			}

			if(lineReader.find("perEvent")<lineReader.length()){
				locOfStream << "    Bool_t locUsedSoFar_perEvent = false;" << std::endl;
				locOfStream << "    Int_t locNumberOfComboAfterCuts_" << "perEvent" << " = 0;" << std::endl;
		
			}

			if(lineReader.find("perCombo")<lineReader.length()){
				locOfStream << "    Int_t locNumberOfComboAfterCuts_" << "perCombo" << " = 0;" << std::endl;
			}
			

			if(lineReader.find("Particle-specific")<lineReader.length()) ParticleSpecificFlag = 1;
			if(lineReader.find("Combo-specific")<lineReader.length()) ComboSpecificFlag = 1;

			std::size_t leftParanthesis = lineReader.find("(");
			std::size_t rightParanthesis = lineReader.find(")", leftParanthesis+1);
			std::string UnqName = line.substr(leftParanthesis+1, rightParanthesis-leftParanthesis-1);

			if(ParticleSpecificFlag == 1){ 
				locOfStream << "    set<Int_t> locUsedSoFar_" << UnqName << ";" << std::endl;
				locOfStream << "    Int_t locNumberOfComboAfterCuts_" << UnqName << " = 0;" << std::endl;
			}
			if(ComboSpecificFlag == 1){  
				locOfStream << "    set<map<Particle_t, set<Int_t> > > locUsedSoFar_" << UnqName << ";" << std::endl;
				locOfStream << "    Int_t locNumberOfComboAfterCuts_" << UnqName << " = 0;" << std::endl;
			}
			
		}
		
	}
	infile.close();
}

//reset the containers for uniqueness tracking, for the second time needed to loop over combos
void Add_UniquenessTrackingDefinition_reset(std::ofstream& locOfStream, string locConfig)
{
	std::ifstream infile(locConfig);	
	std::string line;
	bool SectionBeginFlag = 0;
	bool SectionEndFlag = 0;

	while(std::getline(infile, line))
	{
		std::string lineReader = line.substr(0,line.find("#"));
		if(lineReader == "") continue;
		if(lineReader.find("[SECTION: UNIQUENESS TRACKING]") <lineReader.length()) {
			SectionBeginFlag = 1;
			continue;
		}
		if(lineReader.find("[END: UNIQUENESS TRACKING]") <lineReader.length()) SectionEndFlag = 1;
		if(SectionBeginFlag==1 && SectionEndFlag ==0)
		{	
			std::size_t leftParanthesis = lineReader.find("(");
			std::size_t rightParanthesis = lineReader.find(")", leftParanthesis+1);
			std::string UnqName = line.substr(leftParanthesis+1, rightParanthesis-leftParanthesis-1);
			locOfStream << "    locUsedSoFar_" << UnqName << ".clear();" << std::endl;
		}
	}
	infile.close();
}

void Add_UniquenessTrackingDefinition_Thrown(std::ofstream& locOfStream, string locConfig)
{
	std::ifstream infile(locConfig);	
	std::string line;
	bool SectionBeginFlag = 0;
	bool SectionEndFlag = 0;

	while(std::getline(infile, line))
	{
		std::string lineReader = line.substr(0,line.find("#"));
		if(lineReader == "") continue;
		if(lineReader.find("[SECTION: UNIQUENESS TRACKING]") <lineReader.length()) {
			SectionBeginFlag = 1;
			continue;
		}
		if(lineReader.find("[END: UNIQUENESS TRACKING]") <lineReader.length()) SectionEndFlag = 1;
		if(SectionBeginFlag==1 && SectionEndFlag ==0)
		{	
			if(lineReader.find("AfterComboLoop")<lineReader.length()){
				locOfStream << "    Int_t locNumberOfComboAfterCuts_" << "AfterComboLoop" << " = 0;" << std::endl;
			}

			if(lineReader.find("perEvent")<lineReader.length()){
				locOfStream << "    Bool_t locUsedSoFar_perEvent = false;" << std::endl;
				locOfStream << "    Int_t locNumberOfComboAfterCuts_" << "perEvent" << " = 0;" << std::endl;
		
			}

			if(lineReader.find("perCombo")<lineReader.length()){
				locOfStream << "    Int_t locNumberOfComboAfterCuts_" << "perCombo" << " = 0;" << std::endl;
			}

			if (!(lineReader.find("Particle-specific")<lineReader.length())|| !(lineReader.find("Combo-specific")<lineReader.length()))  continue;
			std::size_t leftParanthesis = lineReader.find("(");
			std::size_t rightParanthesis = lineReader.find(")", leftParanthesis+1);
			std::string UnqName = line.substr(leftParanthesis+1, rightParanthesis-leftParanthesis-1);
			locOfStream << "    Int_t locNumberOfComboAfterCuts_" << UnqName << " = 1;" << std::endl;
		}
	}
	infile.close();
}



void HistWeight(std::ofstream& locOfStream, string locConfig)
{
	std::ifstream infile(locConfig);	
	std::string line;
	bool SectionBeginFlag = 0;
	bool SectionEndFlag = 0;

	while(std::getline(infile, line))
	{
		std::string lineReader = line.substr(0,line.find("#"));
		if(lineReader == "") continue;
		if(lineReader.find("[SECTION: HISTOGRAMS]") <lineReader.length()) SectionBeginFlag = 1;
		if(lineReader.find("[END: HISTOGRAMS]") <lineReader.length())                  SectionEndFlag = 1;

		if(SectionBeginFlag==1 && SectionEndFlag==0)
		{	
			if(lineReader.substr(0,10) == "HISTWEIGHT")
			{
				std::size_t firstQuote = lineReader.find("\"");
				std::size_t secondQuote = lineReader.find("\"", firstQuote+1);
				std::string locWeightValueString = lineReader.substr(firstQuote+1, secondQuote-firstQuote-1);
				locOfStream << "        Double_t histWeight = " << locWeightValueString << ";" << std::endl;
				locOfStream << std::endl;
			}
		}	
	}
	infile.close();
}

void FillHist(std::ofstream& locOfStream, string locConfig, string UniquenessName)
{
	std::ifstream infile(locConfig);	
	std::string line;
	bool SectionBeginFlag = 0;
	bool SectionEndFlag = 0;

	while(std::getline(infile, line))
	{
		std::string lineReader = line.substr(0,line.find("#"));
		if(lineReader == "") continue;
		if(lineReader.find("[SECTION: HISTOGRAMS]") <lineReader.length()) SectionBeginFlag = 1;
		if(lineReader.find("[END: HISTOGRAMS]") <lineReader.length())                  SectionEndFlag = 1;

		if(SectionBeginFlag==1 && SectionEndFlag==0)
		{	
			if(lineReader.substr(0,9) == "HISTOGRAM")
			{
				std::size_t firstQuote = lineReader.find("\"");
				std::size_t secondQuote = lineReader.find("\"", firstQuote+1);
				std::string titles = line.substr(firstQuote+1, secondQuote-firstQuote-1);
				lineReader.erase(firstQuote, secondQuote-firstQuote+1);
				std::string histOptions = lineReader.substr(lineReader.find("=")+1, lineReader.length()-lineReader.find("=")-1);
				std::istringstream iss(histOptions);
				string histType, keyName, binxN, xMin, xMax, binyN, yMin, yMax, xVariable, yVariable, UniquenessTracking;
				iss >> histType >> keyName >> binxN >> xMin >> xMax >> binyN >> yMin >> yMax >> xVariable >> yVariable >> UniquenessTracking;
				if(UniquenessName != UniquenessTracking && UniquenessName != "hist_for_mcGen") continue;
				if(histType.at(2) == '1') locOfStream << "	        	dHist_" << keyName << " -> Fill( " << xVariable << ", histWeight);"<< std::endl;
		    	if(histType.at(2) == '2') locOfStream << "	        	dHist_" << keyName << " -> Fill( " << xVariable << ", " << yVariable << ", histWeight);"<< std::endl;

			}

			if(lineReader.substr(0,12) == "2D HISTOGRAM")
			{
				std::string histOptions = lineReader.substr(lineReader.find("=")+1, lineReader.length()-lineReader.find("=")-1);
				std::istringstream iss(histOptions);
				string histType, xHistName, yHistName, UniquenessTracking;
				iss >> histType >> xHistName >> yHistName >> UniquenessTracking;
				std::string keyName = yHistName + "_vs_" + xHistName;
				xHistName.insert(0, " ");
				yHistName.insert(0, " ");
				xHistName+=" ";
				yHistName+=" ";
				//Now search for xHist, yHist information in the config file
				std::ifstream infile2(locConfig);	
				std::string line2;
				std::string xVariable, yVariable;

				bool xflag = false;
				bool yflag = false;

				while(std::getline(infile2, line2))
				{
					std::string lineReader2 = line2.substr(0,line.find("#"));
					if(lineReader2.substr(0,9) != "HISTOGRAM" || !(lineReader2.find("TH1")<lineReader2.length()) ) continue;
					std::size_t firstQuote, secondQuote;

					if(lineReader2.find(xHistName) <lineReader2.length())
					{
						xflag = true;
						firstQuote = lineReader2.find("\"");
						secondQuote = lineReader2.find("\"", firstQuote+1);
						std::string titles_X = line.substr(firstQuote+1, secondQuote-firstQuote-1);
						lineReader2.erase(firstQuote, secondQuote-firstQuote+1);
						std::string histOptions_X = lineReader2.substr(lineReader2.find("=")+1, lineReader2.length()-lineReader2.find("=")-1);
						std::istringstream iss_X(histOptions_X);
						string histType_X, keyName_X, binxN, xMin, xMax, binyN0, yMin0, yMax0, yVariable0, UniquenessTracking_X;
						iss_X >> histType_X >> keyName_X >> binxN >> xMin >> xMax >> binyN0 >> yMin0 >> yMax0 >> xVariable >> yVariable0 >> UniquenessTracking_X;
					}

					if(lineReader2.find(yHistName) <lineReader2.length())
					{
						yflag = true;
						firstQuote = lineReader2.find("\"");
						secondQuote = lineReader2.find("\"", firstQuote+1);
						std::string titles_Y = line.substr(firstQuote+1, secondQuote-firstQuote-1);
						lineReader2.erase(firstQuote, secondQuote-firstQuote+1);
						std::string histOptions_Y = lineReader2.substr(lineReader2.find("=")+1, lineReader2.length()-lineReader2.find("=")-1);
						std::istringstream iss_Y(histOptions_Y);
						string histType_Y, keyName_Y, binxN0, xMin0, xMax0, binyN, yMin, yMax, xVariable0, UniquenessTracking_Y;
						iss_Y >> histType_Y >> keyName_Y >> binyN >> yMin >> yMax >> binxN0 >> xMin0 >> xMax0 >>  yVariable >> xVariable0 >> UniquenessTracking_Y;
					}


				}
				infile2.close();
				if(xflag == false) cout << "search for histogram X failed!" << endl; 
				if(yflag == false) cout << "search for histogram Y failed!" << endl; 
				if(UniquenessName != UniquenessTracking && UniquenessName != "hist_for_mcGen") continue;
		    	locOfStream << "	        	dHist_" << keyName << " -> Fill( " << xVariable << ", " << yVariable << ", histWeight);"<< std::endl;

			}

		}	
	}
	infile.close();
}


void Add_UniqueHistogramFilling(std::ofstream& locOfStream, string locConfig, map<int, map<int, pair<Particle_t, string> > >& locComboInfoMap)
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
		if(lineReader.find("[END: UNIQUENESS TRACKING]") <lineReader.length()) SectionEndFlag = 1;
		if(SectionBeginFlag==1 && SectionEndFlag==0)
		{	
			if(lineReader.find("perEvent")<lineReader.length())
				{
					locOfStream << "        //No Uniqueness tracking: Add one entry to histograms for every combo." << std::endl;
					locOfStream << "        //Carefully choose histograms to be filled in this section. Be aware of double-counting." << std::endl;
					locOfStream << "        if(locUsedSoFar_perEvent == false)" << std::endl;
					locOfStream << "        {" << std::endl;
					std::string Unq = "perEvent";
					FillHist(locOfStream, locConfig, Unq);
					locOfStream << "        	locUsedSoFar_perEvent = true;" << std::endl;
					locOfStream << "        }" << std::endl;
				} 
			if(lineReader.find("perCombo")<lineReader.length())
				{
					locOfStream << "        //No Uniqueness tracking: Add one entry to histograms for every combo." << std::endl;
					locOfStream << "        //Carefully choose histograms to be filled in this section. Be aware of double-counting." << std::endl;
					std::string Unq = "perCombo";
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
					locOfStream << "        		locUsedSoFar_" << UnqName << ".insert(loc" << particleName << "ID);" << std::endl;
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
						if(locParticleStr == "Beam")
						{
							std::string locPID = "Unknown";//For beam: Don't want to group with final-state photons. Instead use "Unknown" PID (not ideal, but it's easy).
							locOfStream << "        locUsedThisCombo_" << UnqName << "[" << locPID << "].insert(loc" << locParticleStr << "ID);" << std::endl;
						} 
						else
						{
							map<int, map<int, pair<Particle_t, string> > >::iterator locStepIterator = locComboInfoMap.begin();
							for(locStepIterator = locComboInfoMap.begin(); locStepIterator != locComboInfoMap.end(); ++locStepIterator)
							{
								map<int, pair<Particle_t, string> >& locStepInfoMap = locStepIterator->second;
								map<int, pair<Particle_t, string> >::iterator locParticleIterator = locStepInfoMap.begin();
								for(; locParticleIterator != locStepInfoMap.end(); ++locParticleIterator)
								{
									Particle_t locPID = locParticleIterator->second.first;
									string locParticleName = locParticleIterator->second.second;
									if(locParticleName != locParticleStr) continue;

									if(ParticleCharge(locPID) != 0)
									{
										locOfStream << "        locUsedThisCombo_" << UnqName << "[" << EnumString(locPID) << "].insert(loc" << locParticleStr << "TrackID);" << endl;
									}
									else 
									{
										locOfStream << "        locUsedThisCombo_" << UnqName << "[" << EnumString(locPID) << "].insert(loc" << locParticleStr << "NeutralID);" << endl;
									}
								}
							}
										
						}
					}
					locOfStream << "        //compare to what's been used so far" << std::endl;
					locOfStream << "        if(locUsedSoFar_" << UnqName << ".find(locUsedThisCombo_" << UnqName << ") == locUsedSoFar_" << UnqName << ".end())" << std::endl;
					locOfStream << "        {" << std::endl;
					std::string Unq = "Combo-specific("; Unq += UnqName; Unq += ")";
					FillHist(locOfStream, locConfig, Unq);
					locOfStream << "	        	locUsedSoFar_" << UnqName << ".insert(locUsedThisCombo_" << UnqName << ");" << std::endl;
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
		std::string lineReader = line;
		if(lineReader == "") continue;

		if(lineReader.find("#ifdef") <lineReader.length() || lineReader.find("#else") <lineReader.length() || lineReader.find("#endif") <lineReader.length())
		{
			std::string lineReader = line;
		}
		else
		{
			std::string lineReader = line.substr(0,line.find("#"));
			if(lineReader == "") continue;
		}

		if(lineReader.find("[SECTION: KINEMATIC CALCULATION]") <lineReader.length()) 
			{
				SectionBeginFlag = 1; 
				continue;
			}
		if(lineReader.find("[END: KINEMATIC CALCULATION]") <lineReader.length())   
			{
				SectionEndFlag = 1; 
			}
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
		if(lineReader.find("[END: KINEMATIC CALCULATION]") <lineReader.length()) 
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

void Add_Thrown_LorentzBoost(std::ofstream& locOfStream, string locConfig)
{
	//read thrown particle list from config file
	std::ifstream infile0(locConfig);	
	std::string line0;
	bool SectionBeginFlag0 = 0;
	bool SectionEndFlag0 = 0;
	std::string loc_i[10], loc_PID[10], loc_Name[10];
	int loc_index = 0;
	while(std::getline(infile0, line0))
	{
		std::string lineReader0 = line0.substr(0,line0.find("#"));
		if(lineReader0 == "") continue;
		if(lineReader0.find("[SECTION: MC_THROWN]") <lineReader0.length()) SectionBeginFlag0 = 1; 
		if(lineReader0.find("[END: MC_THROWN]") <lineReader0.length())     SectionEndFlag0 = 1;
		if(SectionBeginFlag0==1 && SectionEndFlag0==0 )
		{	
			if(!(lineReader0.find("THROWNPARTICLE") <lineReader0.length())) continue;
			std::string locParticleInfo = lineReader0.substr(lineReader0.find("=")+1,lineReader0.length());
			std::istringstream iss0(locParticleInfo);
			iss0 >> loc_i[loc_index] >> loc_PID[loc_index] >> loc_Name[loc_index];
			loc_index++;
		}
	}
	infile0.close();

	
	std::ifstream infile(locConfig);	
	std::string line;
	bool SectionBeginFlag = 0;
	bool SectionEndFlag = 0;
	while(std::getline(infile, line))
	{
		std::string lineReader = line.substr(0,line.find("#"));
		if(lineReader == "") continue;
		if(lineReader.find("[SECTION: KINEMATIC CALCULATION]") <lineReader.length()) SectionBeginFlag = 1;
		if(lineReader.find("[END: KINEMATIC CALCULATION]") <lineReader.length()) 
		{
			SectionEndFlag = 1; 
			continue;
		}
		if(!(lineReader.find("LorentzTransformation") <lineReader.length())) continue;

		if(SectionBeginFlag==1 && SectionEndFlag==0 )
		{	
			string BoostName;  //unique name of this boost
			string originFrameName;  //from which frame the boost is applied
			string boostVector; // the boost vector 
			std::size_t firstQuote = lineReader.find("\"");
			std::size_t secondQuote = lineReader.find("\"", firstQuote+1);
			boostVector = line.substr(firstQuote+1, secondQuote-firstQuote-1);
			std::string boostOpt = lineReader.substr(lineReader.find("=")+1, firstQuote-lineReader.find("=")-1);
			std::istringstream iss(boostOpt);
			iss >> BoostName >> originFrameName;

			if(originFrameName == "LAB") originFrameName = "";
			else originFrameName = "_" + originFrameName;

			locOfStream << "		/**************** BOOST: " << BoostName << " FRAME ***************/" << endl;
			locOfStream << endl;

			//define the boost 4-vector
			locOfStream << "	TLorentzVector locBoostP4_" << BoostName << " = " << boostVector << "; " << endl;
			locOfStream << endl;
			
			//boost the beam and target
			locOfStream << "	TLorentzVector locBeamP4_" << BoostName << " = locBeamP4" << originFrameName << ";" << endl;
			locOfStream << "	locBeamP4_" << BoostName << ".Boost(-locBoostP4_" << BoostName << ".BoostVector());" << endl;
			locOfStream << "	TLorentzVector dTargetP4_" << BoostName << " = dTargetP4" << originFrameName << ";" << endl;
			locOfStream << "	dTargetP4_" << BoostName << ".Boost(-locBoostP4_" << BoostName << ".BoostVector());" << endl;

			//loop over all thrown particles
			for(int i =0; i<loc_index; i++)
			{
				if(loc_Name[i] == "Unknown")
					continue;
				else //detected
				{
					locOfStream << "	TLorentzVector loc" << loc_Name[i] << "P4_" << BoostName << " = loc" << loc_Name[i] << "P4" << originFrameName << ";" << endl;
					locOfStream << "	loc" << loc_Name[i] << "P4_" << BoostName << ".Boost(-locBoostP4_" << BoostName << ".BoostVector());" << endl;
				}	
			}
		}
	}
	infile.close();
}



void Add_MCThrownParticles(std::ofstream& locOfStream, string locConfig)
{
	std::ifstream infile(locConfig);	
	std::string line;
	bool SectionBeginFlag = 0;
	bool SectionEndFlag = 0;
	std::string loc_i[10], loc_PID[10], loc_Name[10];
	int loc_index = 0;
	while(std::getline(infile, line))
	{
		std::string lineReader = line.substr(0,line.find("#"));
		if(lineReader == "") continue;
		if(lineReader.find("[SECTION: MC_THROWN]") <lineReader.length()) SectionBeginFlag = 1; 
		if(lineReader.find("[END: MC_THROWN]") <lineReader.length())     SectionEndFlag = 1;
		if(SectionBeginFlag==1 && SectionEndFlag==0 )
		{	
			if(!(lineReader.find("THROWNPARTICLE") <lineReader.length())) continue;
			std::string locParticleInfo = lineReader.substr(lineReader.find("=")+1,lineReader.length());
			std::istringstream iss(locParticleInfo);
			iss >> loc_i[loc_index] >> loc_PID[loc_index] >> loc_Name[loc_index];
			loc_index++;
		}
	}
	infile.close();

	locOfStream << "    TLorentzVector dTargetP4(0.0, 0.0, 0.0, 0.93827);"  << endl;
	for(int i =0; i<loc_index; i++)
	{
		locOfStream << "    TLorentzVector loc" << loc_Name[i] << "P4, loc" << loc_Name[i] << "X4;" << endl;
	}
	locOfStream << "	//Loop over throwns" << endl;
	locOfStream << "	for (UInt_t loc_i = 0; loc_i < Get_NumThrown(); ++loc_i)" << endl;
	locOfStream << "	{" << endl;
	locOfStream << "		//Set branch array indices corresponding to this particle" << endl;
	locOfStream << "		dThrownWrapper->Set_ArrayIndex(loc_i);" << endl;
	locOfStream << endl;
	locOfStream << "		//Do stuff with the wrapper here ..." << endl;
	locOfStream << "		Particle_t locPID = dThrownWrapper->Get_PID();" << endl;
	locOfStream << "		TLorentzVector locThrownP4 = dThrownWrapper->Get_P4();" << endl;
	locOfStream << "		TLorentzVector locThrownX4 = dThrownWrapper->Get_X4();" << endl;
	for(int i =0; i<loc_index; i++)
	{
		locOfStream << "		if(loc_i == " << i << ")" << endl;
		locOfStream << "			{" << endl;
		locOfStream << "				loc" << loc_Name[i] << "P4  = locThrownP4;" << endl;
		locOfStream << "				loc" << loc_Name[i] << "X4  = locThrownX4;" << endl;
		locOfStream << "			}" << endl;
	}
	locOfStream << "	}" << endl;

}
