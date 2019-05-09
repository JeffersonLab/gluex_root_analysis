//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//  Helper class to fill flat tree with
//  USAGE:
//    DComboTreeHelper(DTreeInterface *treeinterf, DParticleCombo *combwrap, DTreeInterface *flattreeinterf, TString comb, DMCThrown *mc, TString opt);
//  
//  Arguments:
//    
//  	treeinterf     : Tree interface of input tree
//  	combwrap       : Combo wrapper
//      flattreeinterf : Tree interface of output flat tree
//      comb           : string with semicolon separated combinations to be stored, e.g. "K+ K-; pi+ pi-; K+ pi- p"
//      mc             : DMCThrown interface
//      opt            : option string (colon separated keywords) which branches to create, default = "p4:marker:angle:dalitz:acc"
//                       p4     : stores full TLorentzVectors
//                       marker : 64-bit bit markers (bit position=particle index) about used particles: idxs_gam (gamma), idxs_emu (electron & muon), idxs_pik(pi & K), idxs_pbm (proton & beam) 
//                       angle  : 2-body comb angles (opening angle, decay angle, cos(dec-angle)
//                       dalitz : 3-body comb dalitz plot variables (m01, m12, m02, dal01 = m01^2, ..etc)
//                       acc    : accidental information, i.e. weights for 2 (w2), 4 (w4) or 8 (w8) side bunches, delta_t to RF (rf_dt)                    
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------


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
#include "DMCThrown.h"

#include <particleType.h>


using namespace std;

const int MAXCOMBS = 100;

typedef unsigned long long ULong2;

// Branch types for DComboTreeHelper::Branch method
enum BrType {TULong_t, TUInt_t, TInt_t, TFloat_t, TDouble_t, TLoVect};

class DComboTreeHelper
{
	public:
	
		DComboTreeHelper(DTreeInterface *treeinterf, DParticleCombo *combwrap, DTreeInterface *flattreeinterf, TString dCombinations, DMCThrown *mc=0, TString opt="p4:marker:angle:dalitz:acc:pid");
		size_t CreateBranches();
		void   Fill(unsigned int evnum=0);
		void   NewEvent() {for (size_t i=0;i<MAXCOMBS;++i) {fUsedSoFar[i].clear(); fUsedSoFarFS[i].clear();}}		
	
		void   SetDebug(bool debug=true) {fDebugMode=debug;};
		
	private:

		// these are the interface classes
		DTreeInterface* dTreeInterface;     // interface to tree to be read in
		DParticleCombo* dComboWrapper;
		DTreeInterface* dFlatTreeInterface; // interface to output tree
		DMCThrown*      dThrownWrapper;
		
		bool            fDebugMode;         // enable debug prinout
		
		// flags which branches to create
		bool            fStoreP4;           // LorentzVectors
		bool            fStoreMarker;       // Bit markers for pid/id/uniqueness
		bool            fStoreAngle;        // angles (opening, decay) for two body combinations
		bool            fStoreDalitz;       // dalitz plot variables for three body combinations
		bool            fStoreAcc;          // accidental specific variables (weights, delta_t)
		bool            fStorePid;          // delta_t PID info for FS tracks
		bool            fStoreKin;          // basic kinematic quantities of FS and composites (p, tht, phi)
		
		Long64_t        fCurrEvent;         // current event number (check for new event)
		
		
		vector<TString> SplitString(TString s, TString delim=" ");              // chops string in pieces, separated by delim
		vector<ULong2>  ComboString(TString slist, TString ssubset, TString&);  // returns all index combs of particles ssubset in final state slist
    	int             CntBits(unsigned long i);                               // count the number of bits set in an integer
		TString         IdxCode(ULong2 i);                                      // int to string with indices, e.g. 12 -> "23" (since bits 2 and 3 are set in 12=1100)
		double          DecayAngle(TLorentzVector moth, TLorentzVector dau);    // decay angle 
		TString         Branch(BrType type, TString pref, TString name, TString cntvar=""); // shortcut to dTreeInterface->Create_Branch...
		void            GetFinalStates(int stp, int idx, set<int> &sofar); // return set of final state particle indices for current particle step and index
		
		TLorentzVector           fTargetP4;     // 4-vector of target (for now always proton at rest)
		TVector3                 fTargetCenter; // position of target center

		DBeamParticle*           fBeamWrapper;  // beam particle wrapper
		vector<DKinematicData*>  fVecWrappers;  // container for the particle wrappers
		vector<Particle_t>       fVecPid;       // container for particle types correspondnig to wrappers
		map<int, set<int>>       fFSIndices;    // indices of all final state particles for current (perhaps decaying) one
		vector<TString>          fPrefix;       // prefix for branches to store for a combination
		TString                  fFullFS;       // full final state as blank separated list of particle names
		vector<TString>          fCombFS;       // sub combinations of final states as blank separated list 
		vector<vector<ULong2>>   fCombInd;      // for each sub combo a vector of int, e.g. 012:015 = [00111 = 7, 10011 = 19] -> combination of particles 0, 1, 2 and 0, 1, 5 of wrapper list 
		
		set<map<Particle_t, set<int> > > fUsedSoFar[MAXCOMBS]; // the uniqueness markers (for up to MAXCOMBS combinations) FIXME: should be std::vector 
		set<map<Particle_t, set<int> > > fUsedSoFarFS[MAXCOMBS]; // the uniqueness markers only for FS particles in central beam bunch (for up to MAXCOMBS combinations) FIXME: should be std::vector 

};

#endif // DComboTreeHelper
