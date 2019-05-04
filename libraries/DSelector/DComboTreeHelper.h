#ifndef DComboTreeHelper_h
#define DComboTreeHelper_h

#include "TString.h"

#include "DTreeInterface.h"
#include "DParticleCombo.h"
#include "DParticleComboStep.h"
#include "DBeamParticle.h"
#include "DKinematicData.h"
#include "DChargedTrackHypothesis.h"
#include "DNeutralParticleHypothesis.h"

using namespace std;

typedef unsigned long long ULong2;

// Branch types for DComboTreeHelper::Branch method
enum BrType {TInt_t, TFloat_t, TDouble_t, TLoVect};

class DComboTreeHelper
{
	public:
	
		DComboTreeHelper(DParticleCombo *combwrap, DTreeInterface *treeinterf, TString dCombinations);
		size_t CreateBranches();
		void            Fill(int num);
	
	private:
		// these are the interface classes
		DParticleCombo *dComboWrapper;
		DTreeInterface *dTreeInterface;
		
		vector<TString> SplitString(TString s, TString delim=" ");             // chops string in pieces, separated by delim
		vector<ULong2>  ComboString(TString slist, TString ssubset, TString&); // returns all index combs of particles ssubset in final state slist
    	int             CntBits(unsigned long i);                              // count the number of bits set in an integer
		TString         IdxCode(ULong2 i);                                     // int to string with indices, e.g. 12 -> "23" (since bits 2 and 3 are set in 12=1100)
		double          DecayAngle(TLorentzVector moth, TLorentzVector dau);   // decay angle 
		TString         Branch(BrType type, TString pref, TString name, TString cntvar=""); // shortcut to dTreeInterface->Create_Branch...
					
		vector<DKinematicData*> fVecWrappers; // container for the particle wrappers
		vector<TString>         fPrefix;      // prefix for branches to store for a combination
		TString                 fFullFS;      // full final state as blank separated list of particle names
		vector<TString>         fCombFS;      // sub combinations of final states as blank separated list 
		vector<vector<ULong2>>  fCombInd;     // for each sub combo a vector of int, e.g. 13 = 1101 -> combination of particles 0, 1, and 3 of wrapper list 
		vector<set<float>>      fUniMeas;     // measured masses for unique check
		vector<set<float>>      fUniFit;      // fitted masses for unique check		
};

#endif // DComboTreeHelper
