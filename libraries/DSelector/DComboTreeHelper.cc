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


#include "DComboTreeHelper.h"

#include "TObjString.h"
#include "TObjArray.h"
#include "TLorentzVector.h"

#include <algorithm>

DComboTreeHelper::DComboTreeHelper(DTreeInterface *treeinterf, DParticleCombo *combwrap, DTreeInterface *flattreeinterf, TString comb, DMCThrown *mc, TString opt):
	dTreeInterface(treeinterf), dComboWrapper(combwrap), dFlatTreeInterface(flattreeinterf), dThrownWrapper(mc), fDebugMode(false), fCurrEvent(-1)
{
	// choose branches to be stored
	fStoreP4      = opt.Contains("p4");        // LorentzVectors
	fStoreMarker  = opt.Contains("marker");    // Bit markers for pid/id/uniqueness
	fStoreAngle   = opt.Contains("angle");     // angles (opening, decay) for two body combinations
	fStoreDalitz  = opt.Contains("dalitz");    // dalitz plot variables for three body combinations
	fStoreAcc     = opt.Contains("acc");       // accidental specific variables (weights, delta_t)
	fStorePid     = opt.Contains("pid");       // delta_t PID info for FS tracks
	fStoreKin     = opt.Contains("kin");       // basic kinematic quantities of FS and composites (p, tht, phi)

	// target info
	fTargetP4     = TLorentzVector(0, 0, 0, 0.938272);
	fTargetCenter = dTreeInterface->Get_TargetCenter();	
	
	// fetch the decay topology
	int nCombSteps = dComboWrapper->Get_NumParticleComboSteps();
	cout <<"Num Combo Steps = "<<nCombSteps<<endl<<endl;
	
	// this later holds the final state particles as string with blank separated names	
	TString fullFS;
	
	// get all final state particle wrappers from DParticleCombo
	for (int i=0;i<nCombSteps;++i)
	{
		DParticleComboStep* dStepWrapper = dComboWrapper->Get_ParticleComboStep(i);
		int nParts = dStepWrapper->Get_NumFinalParticles();
		
		// step 0 initial particle is the beam
		if (i==0) fBeamWrapper = static_cast<DBeamParticle*>(dStepWrapper->Get_InitialParticle());
		printf("Step %d : %s%s -> ", i, Get_ShortName(dStepWrapper->Get_InitialPID()), dStepWrapper->Get_InitialParticle() ? "" : "!"); 
		
		// loop over final states
		for (int j=0;j<nParts;++j)
		{
			cout << Get_ShortName(dStepWrapper->Get_FinalPID(j)) << (dStepWrapper->Get_FinalParticle(j) ? "" : "!") << "  ";
			fVecPid.push_back(dStepWrapper->Get_FinalPID(j));
			fVecWrappers.push_back(dStepWrapper->Get_FinalParticle(j));
			fullFS += TString(Get_ShortName(dStepWrapper->Get_FinalPID(j)))+" ";
			
			// get final state indices
			set<int> fs;
			GetFinalStates(i, j, fs);
			fFSIndices[fVecWrappers.size()-1] = fs;
		}
		cout <<endl;
	}
	cout <<endl<<"Final state : \""<<fullFS<<"\""<<endl;	
	
	// print what was found
	cout <<endl<<"The particle list is:"<<endl;
	for (size_t i=0;i<fVecWrappers.size();++i)
	{
		cout<<setw(2)<<i<<") "<<setw(4)<<Get_ShortName(fVecPid[i])<<"  [wrapper prt = "<<fVecWrappers[i]<<"]  FS: ";
		
		for (auto elem : fFSIndices[i])
		{
			std::cout << elem << " ";
		}
		cout <<endl;
	}
	
	// now we prepare the combinations requested by the user
	// the names have to match the short names (+/- and upper case allowed -> will be replaced)
	cout <<endl<<"Requested combinatorics info for:"<<endl;
	vector<TString> vec = SplitString(comb,";");
	for (size_t i=0;i<vec.size();++i)
	{
		// some replacements and preparations for the FS string (e.g. K+ -> kp)
		vec[i].ToLower();
		vec[i].ReplaceAll("+","p");
		vec[i].ReplaceAll("-","m");
		
		cout <<i<<". "<<vec[i]<<" : ";
		
		// check for doublers
		if (std::find(fCombFS.begin(), fCombFS.end(), vec[i]) != fCombFS.end()) {cout <<" *** Skipping doubled"<<endl;continue;}

		// store the FS string
		fCombFS.push_back(vec[i]);
		
		// and turn it to prefix (remove whitespace)
		TString tmp = vec[i];
		tmp.ReplaceAll(" ","");
		fPrefix.push_back(tmp);
		cout <<" (branch prefix: "<<tmp<<") ";
		
		// create a vector holding the combinations as bit pattern (e.g. 01101 = 2nd, 3rd and 5th particle from fVecWrappers to be combined)
		TString vcombs;
		fCombInd.push_back(ComboString(fullFS, vec[i], vcombs));
		if (vcombs!="") 
			cout <<vcombs<<endl; 
		else 
			cout <<" no combinations found."<<endl;
	}	

	CreateBranches();
}
// ---------------------------------------------------------------
// return set of final state particle indices for current particle index
// ---------------------------------------------------------------
void DComboTreeHelper::GetFinalStates(int stp, int idx, set<int> &fs)
{
	vector<int> offsets = {0};
	for (size_t i=0;i<dComboWrapper->Get_NumParticleComboSteps();++i) offsets.push_back(offsets[i]+dComboWrapper->Get_ParticleComboStep(i)->Get_NumFinalParticles());
	
	DParticleComboStep* dStepWrapper = dComboWrapper->Get_ParticleComboStep(stp);
	
	// final state particle: just add index to set
	if (dStepWrapper->Is_FinalParticleDetected(idx)) fs.insert(idx+offsets[stp]);
	
	// composite particle: add the indices for all daughters
	else if (dStepWrapper->Is_FinalParticleDecaying(idx)) 
	{
		int daustep = dStepWrapper->Get_DecayStepIndex(idx);
		DParticleComboStep* dDauWrapper = dComboWrapper->Get_ParticleComboStep(daustep);
		for (size_t i=0;i<dDauWrapper->Get_NumFinalParticles();++i) GetFinalStates(daustep, i, fs);
	}
	
	return;
}

// ---------------------------------------------------------------
// creates branches 
// ---------------------------------------------------------------
size_t DComboTreeHelper::CreateBranches()
{
	if (dFlatTreeInterface==nullptr) return 0;
	
	// will be filled with branch names created
	vector<TString> brname;
	
	// create common branches
	brname.push_back( Branch(TFloat_t, "", "mm2"    ) );   // missing mass squared
	
	if (fStoreAcc)
	{
		brname.push_back( Branch(TFloat_t, "", "rf_dt") );  // RF delta t
		brname.push_back( Branch(TFloat_t, "", "w2"   ) );  // weight for 2 side bunches
		brname.push_back( Branch(TFloat_t, "", "w4"   ) );  // weight for 4 side bunches
		brname.push_back( Branch(TFloat_t, "", "w8"   ) );  // weight for 8 side bunches
	}	
	
	// create kin branches for beam
	if (fStoreKin)
	{
		brname.push_back(Branch(TFloat_t, "", "beam_e"  ));  // beam energy
		brname.push_back(Branch(TFloat_t, "", "beam_z"  ));  // beam z position (in target)
	}

	// create delta_t PID and kin branches; use prefixes of original flat tree
	if (fStorePid || fStoreKin)
	{
		for (unsigned int i=0;i<dComboWrapper->Get_NumParticleComboSteps();++i)
		{
			DParticleComboStep* dStepWrapper = dComboWrapper->Get_ParticleComboStep(i);
			for (unsigned int j=0;j<dStepWrapper->Get_NumFinalParticles();++j)
			{
				// pid info for now only for charged tracks
				if (fStorePid && dStepWrapper->Is_FinalParticleDetected(j) && ParticleCharge(dStepWrapper->Get_FinalPID(j))!=0 && dStepWrapper->Get_FinalParticle(j))
				{
					TString bname = dStepWrapper->Get_FinalParticle(j)->Get_BranchNamePrefix();
					bname.ReplaceAll( EnumString(dStepWrapper->Get_FinalPID(j)), Get_ShortName(dStepWrapper->Get_FinalPID(j)) ); 
					
					brname.push_back(Branch(TFloat_t, "", TString(bname)+"_dttof"  ));  // delta_t TOF
					brname.push_back(Branch(TFloat_t, "", TString(bname)+"_dtbcal" ));  // delta_t BCAL
					brname.push_back(Branch(TFloat_t, "", TString(bname)+"_dtfcal" ));  // delta_t FCal
				}

				// kin info for all detected final states
				if (fStoreKin && dStepWrapper->Is_FinalParticleDetected(j) && dStepWrapper->Get_FinalParticle(j))
				{
					TString bname = dStepWrapper->Get_FinalParticle(j)->Get_BranchNamePrefix();
					bname.ReplaceAll( EnumString(dStepWrapper->Get_FinalPID(j)), Get_ShortName(dStepWrapper->Get_FinalPID(j)) ); 
					bname.ReplaceAll( "Photon", "g" );
					
					brname.push_back(Branch(TFloat_t, "", TString(bname)+"_p"  ));  // momentum
					brname.push_back(Branch(TFloat_t, "", TString(bname)+"_pt" ));  // traverse momentum
					brname.push_back(Branch(TFloat_t, "", TString(bname)+"_tht" )); // polar angle
					brname.push_back(Branch(TFloat_t, "", TString(bname)+"_phi" )); // phi angle
				}
				
			}
		}
	}
	
	// create combinatoric dependent branches
	for (size_t i=0;i<fCombInd.size();++i)
	{
		if (fCombInd[i].size()==0) continue;
		
		TString pref = fPrefix[i];
			
		// combinatoric specific variables
		brname.push_back( Branch(TUInt_t,  pref, "ncomb"   ) );            // number of combinations

		brname.push_back( Branch(TFloat_t, pref, "m"        , "ncomb") );  // mass
		brname.push_back( Branch(TFloat_t, pref, "mf"       , "ncomb") );  // fitted mass
		brname.push_back( Branch(TFloat_t, pref, "mt"       , "ncomb") );  // thrown mass
	
		brname.push_back( Branch(TFloat_t, pref, "chg"      ) );           // charge

		brname.push_back( Branch(TInt_t,   pref, "uni"      , "ncomb") );  // unique flag (based on unique combination of track IDs and beam ID)
		brname.push_back( Branch(TInt_t,   pref, "unifs"    , "ncomb") );  // unique flag w/o beam id, only for central beam bunch (based on unique combination of track IDs and beam ID)
		
		// basic kinematic quantities
		if (fStoreKin)
		{
			brname.push_back( Branch(TFloat_t, pref, "p"        , "ncomb") );  // momentum
			brname.push_back( Branch(TFloat_t, pref, "pt"       , "ncomb") );  // traverse momentum
			brname.push_back( Branch(TFloat_t, pref, "tht"      , "ncomb") );  // polar angle
			brname.push_back( Branch(TFloat_t, pref, "phi"      , "ncomb") );  // phi angle
		}
		
		// full TLorentzVectors
		if (fStoreP4)
		{
			brname.push_back( Branch(TLoVect,  pref, "p4_kin"   , "ncomb") );  // fitted p4
			brname.push_back( Branch(TLoVect,  pref, "p4_meas"  , "ncomb") );  // measured p4
			brname.push_back( Branch(TLoVect,  pref, "p4_thr"   , "ncomb") );  // thrown p4
		}
		
		// bit markers for uniqueness
		if (fStoreMarker)
		{
			brname.push_back( Branch(TULong_t, pref, "idxs_gam" , "ncomb") );  // long (used as bit pattern) with indices of used: gammas (bits 0-63)
			brname.push_back( Branch(TULong_t, pref, "idxs_emu" , "ncomb") );  // long (used as bit pattern) with indices of used: e+-(bits 0-31) and mu+-(bits 32-63)
			brname.push_back( Branch(TULong_t, pref, "idxs_pik" , "ncomb") );  // long (used as bit pattern) with indices of used: pi+-(bits 0-31) and K+-(bits 32-63)
			brname.push_back( Branch(TULong_t, pref, "idxs_pbm" , "ncomb") );  // long (used as bit pattern) with indices of used: p/pbar(bits 0-31) and beam (bits 32-63)
		}
				
		int ncombpart = CntBits(fCombInd[i][0]);
		
		switch (ncombpart)
		{
		case 2:
			// angles for 2-body combinations
			if (fStoreAngle)
			{
				brname.push_back( Branch(TFloat_t, pref, "dec"  , "ncomb"));  // decay angle
				brname.push_back( Branch(TFloat_t, pref, "cdec" , "ncomb"));  // cos(decay angle)
				brname.push_back( Branch(TFloat_t, pref, "oang" , "ncomb"));  // openning angle
			}
			break;
			
			
		case 3:
			// Dalitz Plot variables for 3-body combinations
			if (fStoreDalitz)
			{
				brname.push_back( Branch(TFloat_t, pref, "m01"  , "ncomb"));  // m(p0 p1)
				brname.push_back( Branch(TFloat_t, pref, "m02"  , "ncomb"));  // m(p0 p2)
				brname.push_back( Branch(TFloat_t, pref, "m12"  , "ncomb"));  // m(p1 p2);

				brname.push_back( Branch(TFloat_t, pref, "dal01", "ncomb"));  // m2(p0 p1) for Dalitz plots
				brname.push_back( Branch(TFloat_t, pref, "dal02", "ncomb"));  // m2(p0 p2)
				brname.push_back( Branch(TFloat_t, pref, "dal12", "ncomb"));  // m2(p1 p2)
			}
			
			break;		
		}
		
	}
	cout <<endl<<"[DComboTreeHelper] Creating "<<brname.size()<<" branches:"<<endl;
	for (auto name:brname) cout <<name<<"  ";
	cout <<endl<<endl;
	
	return brname.size();	
}

// ---------------------------------------------------------------
// fills branches ; happens for each combo in DSelector
// ---------------------------------------------------------------
void DComboTreeHelper::Fill(unsigned int evnum)
{
	// no tree -> return
	if (dFlatTreeInterface==nullptr) return;
	
	// new event? reset arrays
	if (evnum!=fCurrEvent)
	{
		NewEvent();
		fCurrEvent = evnum;
	}

	// compute time difference between tagger and RF (corrected for production vertex position relative to target center)
	double rf_dt = fBeamWrapper->Get_X4_Measured().T() - (dComboWrapper->Get_RFTime_Measured() + (fBeamWrapper->Get_X4_Measured().Z() - fTargetCenter.Z())/29.9792458); 

	// compute and fill accidental specific variables
	if (fStoreAcc)
	{
		// Combine 4-vectors
		TLorentzVector mm2_p4 = fBeamWrapper->Get_P4_Measured() + fTargetP4;
		
		// subtract p4 of all final states (FIXME: should be detected particles, which is assumed to be identical)
		for (size_t k=0; k<fVecWrappers.size(); ++k) 
			if (Is_FinalStateParticle(fVecPid[k])) mm2_p4 -= fVecWrappers[k]->Get_P4_Measured();
		
		double w2 = (abs(rf_dt)<2.004)*1.5 - 0.5;
		double w4 = (abs(rf_dt)<2.004)*1.25 - 0.25;
		double w8 = (abs(rf_dt)<2.004)*1.125 - 0.125;
	
		dFlatTreeInterface->Fill_Fundamental<Float_t>("mm2",     mm2_p4.M2()  );  // missing mass squared
		dFlatTreeInterface->Fill_Fundamental<Float_t>("rf_dt",   rf_dt        );  // RF delta t
		dFlatTreeInterface->Fill_Fundamental<Float_t>("w2",      w2           );  // weight for 2 side bunches
		dFlatTreeInterface->Fill_Fundamental<Float_t>("w4",      w4           );  // weight for 4 side bunches
		dFlatTreeInterface->Fill_Fundamental<Float_t>("w8",      w8           );  // weight for 8 side bunches
	}	

	// create kin branches for beam
	if (fStoreKin && fBeamWrapper)
	{
		dFlatTreeInterface->Fill_Fundamental<Float_t>("beam_e",    fBeamWrapper->Get_P4().P()    );  // momentum 
		dFlatTreeInterface->Fill_Fundamental<Float_t>("beam_z",    fBeamWrapper->Get_X4().Z()    );  // traverse momentum 
	}

	// compute and fill PID specific variables
	if (fStorePid || fStoreKin)
	{
		double rftime = dComboWrapper->Get_RFTime_Measured();

		for (unsigned int i=0;i<dComboWrapper->Get_NumParticleComboSteps();++i)
		{
			DParticleComboStep* dStepWrapper = dComboWrapper->Get_ParticleComboStep(i);
			for (unsigned int j=0;j<dStepWrapper->Get_NumFinalParticles();++j)
			{
				if (fStorePid && dStepWrapper->Is_FinalParticleDetected(j) && ParticleCharge(dStepWrapper->Get_FinalPID(j))!=0 && dStepWrapper->Get_FinalParticle(j))
				{
					DChargedTrackHypothesis *trk = static_cast<DChargedTrackHypothesis*>(dStepWrapper->Get_FinalParticle(j));
					
					TString bname      = trk->Get_BranchNamePrefix();
					bname.ReplaceAll( EnumString(dStepWrapper->Get_FinalPID(j)), Get_ShortName(dStepWrapper->Get_FinalPID(j)) ); 
					
					TLorentzVector pX4 = trk->Get_X4_Measured();
					
					double proptime = rftime + (pX4.Z() - fTargetCenter.Z()) / 29.9792458;

					double dttof   = (trk->Get_Detector_System_Timing() == SYS_TOF)  ? pX4.T()-proptime : -999.;				
					double dtbcal  = (trk->Get_Detector_System_Timing() == SYS_BCAL) ? pX4.T()-proptime : -999.;				
					double dtfcal  = (trk->Get_Detector_System_Timing() == SYS_FCAL) ? pX4.T()-proptime : -999.;			

					dFlatTreeInterface->Fill_Fundamental<Float_t>((bname+"_dttof").Data(),  dttof  );  // dt for TOF 
					dFlatTreeInterface->Fill_Fundamental<Float_t>((bname+"_dtbcal").Data(), dtbcal );  // dt for BCAL 
					dFlatTreeInterface->Fill_Fundamental<Float_t>((bname+"_dtfcal").Data(), dtfcal );  // dt for FCAL  	
				}
				

				// kin info for all detected final states
				if (fStoreKin && dStepWrapper->Is_FinalParticleDetected(j) && dStepWrapper->Get_FinalParticle(j))
				{
					TString bname = dStepWrapper->Get_FinalParticle(j)->Get_BranchNamePrefix();
					bname.ReplaceAll( EnumString(dStepWrapper->Get_FinalPID(j)), Get_ShortName(dStepWrapper->Get_FinalPID(j)) ); 
					bname.ReplaceAll( "Photon", "g" );
					
					// p4 is kinfit if kinfit, else is measured
					TLorentzVector p4 = dStepWrapper->Get_FinalParticle(j)->Get_P4();
					
					dFlatTreeInterface->Fill_Fundamental<Float_t>((bname+"_p").Data(),    p4.P()     );  // momentum 
					dFlatTreeInterface->Fill_Fundamental<Float_t>((bname+"_pt").Data(),   p4.Pt()    );  // traverse momentum 
					dFlatTreeInterface->Fill_Fundamental<Float_t>((bname+"_tht").Data(),  p4.Theta() );  // polar angle 
					dFlatTreeInterface->Fill_Fundamental<Float_t>((bname+"_phi").Data(),  p4.Phi()   );  // phi angle
				}
			}
		}
	}
	

	// maps particle types (particleType.h) to bit markers (7 different)
	// 0: gamma, 1: e+-, 2:mu+-, 3:pi+-, 4:K+-, 5:p/pbar 6:beam 
	map<Particle_t, unsigned int> fpartidx = { {Gamma,0} , {Positron,1} , {Electron,1} , {MuonPlus,2} , {MuonMinus,2} , {PiPlus,3} , {PiMinus,3} , {KPlus,4} , {KMinus,4} , {Proton,5} , {AntiProton,5} , {Unknown,6} };
		
	
	// cache measured/fitted p4 (also those from fitted composites by summing final state measured p4)
	vector<TLorentzVector> p4_meas, p4_kin;
	for (size_t k=0; k<fVecWrappers.size(); ++k)
	{
		TLorentzVector p4, p4k;
		
		// loop over all used final state particles (e.g. K_S -> pi+ pi- uses two tracks) and sum up measured p4
		for (auto wrap_idx : fFSIndices[k]) {p4 += fVecWrappers[wrap_idx]->Get_P4_Measured(); p4k += fVecWrappers[wrap_idx]->Get_P4();}
		p4_meas.push_back(p4);
		p4_kin.push_back(p4k); 
	}
	
	// cache thrown p4 (also those from fitted composites by summing final state measured p4)
	vector<TLorentzVector> p4_thrown;
	for (size_t k=0; k<fVecWrappers.size(); ++k)
	{
		TLorentzVector p4;
		
		bool thrown_complete = true;
		
		// do we have MC data
		if (dThrownWrapper!=0)
		{
			// loop over all used final state particles (e.g. K_S -> pi+ pi- uses two tracks) and sum up thrown p4
			for (auto wrap_idx : fFSIndices[k]) 
			{
				int thr_id  = -1;
				if (ParticleCharge(fVecWrappers[wrap_idx]->Get_PID())==0)
					thr_id = static_cast<DNeutralParticleHypothesis*>(fVecWrappers[wrap_idx])->Get_ThrownIndex();
				else
					thr_id = static_cast<DChargedTrackHypothesis*>(fVecWrappers[wrap_idx])->Get_ThrownIndex();

				// no match 
				if (thr_id==-1) {thrown_complete = false; continue;}
				
				dThrownWrapper->Set_ArrayIndex(thr_id);
				
				// check PID code
				if (dThrownWrapper->Get_PID() == fVecWrappers[wrap_idx]->Get_PID())
					p4 += dThrownWrapper->Get_P4();
				else
					thrown_complete = false;
			}
		}
		
		// if all FS have correct PID code store thrown vector
		if (thrown_complete) 
			p4_thrown.push_back(p4);
		// else store empty P4
		else 
			p4_thrown.push_back(TLorentzVector());
	}
	
	// loop over all sub sets (e.g. K+ K-, pi+ pi0, gam gam, ...)
	for (unsigned int i=0;i<fCombInd.size();++i)
	{
		if (fCombInd[i].size()==0) continue;
		
		TString pref           = fPrefix[i];   // prefix for the branches of this combination
		vector<ULong2> combs   = fCombInd[i];  // vector with combinatoric indices (as bit pattern in ulong)
		size_t ncombs          = combs.size(); // number of combinations (of the particle wrapper inidices, _not_ track indices)

		dFlatTreeInterface->Fill_Fundamental<UInt_t>((pref+"_ncomb").Data(), ncombs);              
		
		int curridx = 0;
		
		// loop over the corresponding combinations within one set e.g. indices 01 and 02 in (pi+ pi- out of pi+ pi+ pi-)
		for (size_t j=0;j<ncombs;++j)
		{	
			// here we add up the p4 for the combinations	
			TLorentzVector lvcomb_kin(0.,0.,0.,0.);  // fitted
			TLorentzVector lvcomb_meas(0.,0.,0.,0.); // measured
			TLorentzVector lvcomb_thr(0.,0.,0.,0.); // thrown
			
			Float_t chg = 0;  // collect charge
			ULong2  idx = 1;  // 1 bit mask to select particles for a composite
			
			// a vector of the fitted p4 for computing decay angels, Dalitz plot variables
			vector<TLorentzVector> vlv;
			
			// this is a bit vector representing the used track/neutral IDs; stored in ttree
			ULong2 IDpatt[7] = {0};
			map<Particle_t, set<int> > thisCombo, thisComboFS;
			thisCombo[Unknown].insert(fBeamWrapper->Get_BeamID());
			
			if (fDebugMode) cout <<"set "<<i<<"("<<pref<<"), comb "<<j<<"  FS: beam/"<<fBeamWrapper->Get_BeamID()<<" ";

			bool thrown_complete=true;
			
			for (size_t k=0; k<fVecWrappers.size(); ++k)
			{
				if (idx & combs[j]) 
				{
					// loop over all used final state particles (e.g. K_S -> pi+ pi- uses two tracks)
					for (auto wrap_idx : fFSIndices[k])
					{
						// store used FS trackID in bit marker
						int      fs_trkID = fVecWrappers[wrap_idx]->Get_ID();					
						Particle_t fs_pid = fVecWrappers[wrap_idx]->Get_PID();
						
						if (fpartidx.find(fs_pid)==fpartidx.end()) fs_pid = Unknown;
						IDpatt[fpartidx[fs_pid]] |= 1<<fs_trkID;
						
						thisCombo[fs_pid].insert(fs_trkID);
						thisComboFS[fs_pid].insert(fs_trkID);
						
						if (fDebugMode) cout <<ShortName(fs_pid)<<"/"<<fs_trkID<<" ";
					}

					// get pid for particle
					Particle_t pid = fVecPid[k];
								
					// add up composite p4 and charge
					// fitted 4-vector either directly from wrapper or (if not existing) from sum of fitted FS
					TLorentzVector p4_fit = fVecWrappers[k] ? fVecWrappers[k]->Get_P4() : p4_kin[k];
						
					lvcomb_kin  += p4_fit;	
					lvcomb_meas += p4_meas[k]; //Is_FinalStateParticle(pid) ? fVecWrappers[k]->Get_P4_Measured() : fVecWrappers[k]->Get_P4();
					lvcomb_thr  += p4_thrown[k];
 					chg         += ParticleCharge(pid);
 					
 					// it one contribution to thrown is 0 -> not matching
 					if (p4_thrown[k].M()<0.0001) thrown_complete=false;
 					
					//store p4 kin fit
 					vlv.push_back(p4_fit);
				}
				idx *= 2; // shift mask to next place
			}
											
			// if MC match failed reset p4
			if (!thrown_complete) lvcomb_thr = TLorentzVector();
			
			// check uniqueness
			int unique = ( fUsedSoFar[i].find(thisCombo) == fUsedSoFar[i].end() );
			if (unique) fUsedSoFar[i].insert(thisCombo);
				
			// FS uniqueness only checked for central beam bunch
			int uniquefs = ( fabs(rf_dt)<2.004 && fUsedSoFarFS[i].find(thisComboFS) == fUsedSoFarFS[i].end() );
			if (uniquefs) fUsedSoFarFS[i].insert(thisComboFS);
				
			if (fDebugMode) { if (unique) cout <<" unique"; cout <<endl; }

			// fill tree branches
 			dFlatTreeInterface->Fill_Fundamental<Float_t>((pref+"_m").Data(),      lvcomb_meas.M(),  curridx); 
 			dFlatTreeInterface->Fill_Fundamental<Float_t>((pref+"_mf").Data(),     lvcomb_kin.M(),   curridx); 
 			dFlatTreeInterface->Fill_Fundamental<Float_t>((pref+"_mt").Data(),     lvcomb_thr.M(),   curridx); 
 			
			dFlatTreeInterface->Fill_Fundamental<Float_t>((pref+"_chg").Data(),    chg,              curridx);  

			dFlatTreeInterface->Fill_Fundamental<Int_t>(  (pref+"_uni").Data(),    unique,           curridx);  
			dFlatTreeInterface->Fill_Fundamental<Int_t>(  (pref+"_unifs").Data(),  uniquefs,         curridx);  

			if (fStoreKin)
			{
				dFlatTreeInterface->Fill_Fundamental<Float_t>((pref+"_p").Data(),    lvcomb_kin.P(),     curridx);
				dFlatTreeInterface->Fill_Fundamental<Float_t>((pref+"_pt").Data(),   lvcomb_kin.Pt(),    curridx);
				dFlatTreeInterface->Fill_Fundamental<Float_t>((pref+"_tht").Data(),  lvcomb_kin.Theta(), curridx);
				dFlatTreeInterface->Fill_Fundamental<Float_t>((pref+"_phi").Data(),  lvcomb_kin.Phi(),   curridx);
			}

			if (fStoreP4)
			{
				dFlatTreeInterface->Fill_TObject<TLorentzVector>((pref+"_p4_kin").Data(),  lvcomb_kin,  curridx);    
				dFlatTreeInterface->Fill_TObject<TLorentzVector>((pref+"_p4_meas").Data(), lvcomb_meas, curridx);   
				dFlatTreeInterface->Fill_TObject<TLorentzVector>((pref+"_p4_thr").Data(),  lvcomb_thr,  curridx);   			
			}
			
			if (fStoreMarker)
			{
				dFlatTreeInterface->Fill_Fundamental<Long_t>( (pref+"_idxs_gam").Data(), IDpatt[0]       , curridx);  
				dFlatTreeInterface->Fill_Fundamental<Long_t>( (pref+"_idxs_emu").Data(), (IDpatt[1]<<32)+IDpatt[2], curridx);  
				dFlatTreeInterface->Fill_Fundamental<Long_t>( (pref+"_idxs_pik").Data(), (IDpatt[3]<<32)+IDpatt[4], curridx);  
				dFlatTreeInterface->Fill_Fundamental<Long_t>( (pref+"_idxs_pbm").Data(), (IDpatt[5]<<32)+IDpatt[6], curridx);  
 			}
 
			switch (vlv.size())
			{
			case 2:
				{
					if (fStoreAngle)
					{
						TLorentzVector lv0    = vlv[0]; 
						TLorentzVector lv1    = vlv[1]; 

						double decang = DecayAngle(lv0, lv1);
						dFlatTreeInterface->Fill_Fundamental<Float_t>((pref+"_dec").Data(),   decang,                       curridx);  
						dFlatTreeInterface->Fill_Fundamental<Float_t>((pref+"_cdec").Data(),  cos(decang),                  curridx);  
						dFlatTreeInterface->Fill_Fundamental<Float_t>((pref+"_oang").Data(),  lv0.Vect().Angle(lv1.Vect()), curridx); 
					}
					
					break;
				}

			case 3:
				{
					if (fStoreDalitz)
					{
						TLorentzVector lv01 = vlv[0]+vlv[1]; 
						TLorentzVector lv02 = vlv[0]+vlv[2]; 
						TLorentzVector lv12 = vlv[1]+vlv[2]; 

						dFlatTreeInterface->Fill_Fundamental<Float_t>((pref+"_m01").Data(), lv01.M(), curridx);  
						dFlatTreeInterface->Fill_Fundamental<Float_t>((pref+"_m02").Data(), lv02.M(), curridx);  
						dFlatTreeInterface->Fill_Fundamental<Float_t>((pref+"_m12").Data(), lv12.M(), curridx);  

						dFlatTreeInterface->Fill_Fundamental<Float_t>((pref+"_dal01").Data(), lv01.M2(), curridx);  
						dFlatTreeInterface->Fill_Fundamental<Float_t>((pref+"_dal02").Data(), lv02.M2(), curridx);  
						dFlatTreeInterface->Fill_Fundamental<Float_t>((pref+"_dal12").Data(), lv12.M2(), curridx);  
					}
					
					break;	
				}	
			}
			
			curridx++;
		}		
		if (fDebugMode) cout<<" ---set---"<<endl;
	}
	if (fDebugMode) cout<<" *** combo ***"<<endl<<endl;
}

// ---------------------------------------------------------------
// creates combinatorics by vector of bit patterns
//
// e.g. slist = "p pi+ pi- pi-", ssubset = "pi+ pi-" 
//   --> vector<int> = {6, 10}  (6 = 0110, 10 = 0101)
//       combs       = "12:13"  (indices of particles to comb.)
// ---------------------------------------------------------------

vector<ULong2> DComboTreeHelper::ComboString(TString slist, TString ssubset, TString &combs)
{
	vector<ULong2> res;
	combs="";

	static const TString seq = "0123456789abcdefg";
		
	if (ssubset=="" || slist=="") return res;
	
	vector<TString> parts  = SplitString(slist, " ");
	vector<TString> subset = SplitString(ssubset, " ");

	int Ntot = parts.size();
	int Nsub = subset.size();
	
	if (Ntot>63) Ntot = 63;
	
	ULong2 Npow = pow(2,Ntot);
		
	for (ULong2 i=0;i<Npow;++i)
	{
		if (CntBits(i)!=Nsub) continue;
		
		TString temp = ssubset;
		TString idxc = IdxCode(i);
		
		for (int j=0;j<Nsub;++j) 
		{
			int k = seq.Index(idxc[j]);
			int pos = temp.Index(parts[k]);
			if (pos!=-1) temp.Replace(pos,parts[k].Length(),"");
		}
		temp.ReplaceAll(" ","");
		
		if (temp=="") {combs+=":"+idxc; res.push_back(i);}
	}	
	combs = combs(1,1000);
	return res;
}


// ---------------------------------------------------------------
// indices to index string
//
// turns an int into a string of indices, e.g. 9 (=1001) -> "03"
// ---------------------------------------------------------------
TString DComboTreeHelper::IdxCode(ULong2 i)
{
	static const TString seq = "0123456789abcdefghijklmnopqrstuvwxyz";
	TString res = "";
	
	int idx = 0, maxidx = seq.Length()-1;
	
	while (i>0) 
	{
		if (i%2) res += seq[idx];
		i/=2;
		idx++;
		if (maxidx<idx) idx = maxidx;
	} 
	return res;
}

// ---------------------------------------------------------------
// cnts number of bits set in integer
// ---------------------------------------------------------------
int DComboTreeHelper::CntBits(unsigned long i)
{
	// 64 bit variant of efficient Hamming weight
	
	i = i - ((i >> 1) & 0x5555555555555555UL);
	i = (i & 0x3333333333333333UL) + ((i >> 2) & 0x3333333333333333UL);
	return (int)((((i + (i >> 4)) & 0xF0F0F0F0F0F0F0FUL) * 0x101010101010101UL) >> 56);

	// 32 bit variant of efficient Hamming weight

//     i = i - ((i >> 1) & 0x55555555);
//     i = (i & 0x33333333) + ((i >> 2) & 0x33333333);
//     return (((i + (i >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;
}

// ---------------------------------------------------------------
// Splits string in parts separated by delim
// ---------------------------------------------------------------
std::vector<TString> DComboTreeHelper::SplitString(TString s, TString delim)
{
	std::vector<TString> toks;
	TObjArray *tok = s.Tokenize(delim);
	int N = tok->GetEntries();	
	for (int i=0;i<N;++i) 
	{
		TString token = (((TObjString*)tok->At(i))->String()).Strip(TString::kBoth);
		toks.push_back(token);
	}
	return toks;
}

// ---------------------------------------------------------------
// decay angle 
// ---------------------------------------------------------------
//
// (like in e-mail Andreas Pitka 12/5/18)
//             p4_m
// p4_l <------ 0 ------>  p4_r
//             /
//            /
//           /
//          /
//         /
//        <  p4_b
//double Helicity_Angle( TLorentzVector p4_b, TLorentzVector p4_l, TLorentzVector p4_r )
//{

//// boost to CMS
//TLorentzVector sum = p4_l + p4_r + p4_b;
//TVector3  beta1 = -sum.BoostVector();
//p4_l.Boost(beta1);
//p4_r.Boost(beta1);
//p4_b.Boost(beta1);

//// boost to (l,r) system
//TLorentzVector P4_M = p4_l + p4_r;

//TVector3  beta = -P4_M.BoostVector();

//p4_l.Boost(beta);
//p4_r.Boost(beta);

//TLorentzVector P4_Reso = p4_l + p4_r; 

//// p4_l in (l,r) system, p4_b in CMS
//double theta = p4_l.Angle( -p4_b.Vect() );

//return cos( theta );

//} /// End Helicity_Angle
// ---------------------------------------------------------------
double DComboTreeHelper::DecayAngle(TLorentzVector dau1, TLorentzVector dau2)
{
	TLorentzVector cms = fTargetP4 + fBeamWrapper->Get_P4();
	TVector3 cmsboost = -cms.BoostVector();
	
	dau1.Boost(cmsboost);
	dau2.Boost(cmsboost);
	
	TLorentzVector reso = dau1+dau2;
	TVector3 resoboost  = -reso.BoostVector(); 

	dau1.Boost(resoboost);	
	
	return dau1.Angle(reso.Vect());
}

// ---------------------------------------------------------------
// Create Branch
// ---------------------------------------------------------------
TString DComboTreeHelper::Branch(BrType type, TString pref, TString name, TString cntvar)
{
	TString tbrname  = pref!="" ? pref+"_"+name   : name;
	TString tcntname = pref!="" ? pref+"_"+cntvar : cntvar;
	
	string brname  = tbrname.Data();
	string cntname = tcntname.Data();
	
	switch (type) 
	{
	case TULong_t :
		if (cntvar=="")	dFlatTreeInterface->Create_Branch_Fundamental<ULong2>(brname);
		else            dFlatTreeInterface->Create_Branch_FundamentalArray<ULong2>(brname, cntname);
		break;
		
	case TUInt_t :
		if (cntvar=="")	dFlatTreeInterface->Create_Branch_Fundamental<UInt_t>(brname);
		else            dFlatTreeInterface->Create_Branch_FundamentalArray<UInt_t>(brname, cntname);
		break;

	case TInt_t :
		if (cntvar=="")	dFlatTreeInterface->Create_Branch_Fundamental<Int_t>(brname);
		else            dFlatTreeInterface->Create_Branch_FundamentalArray<Int_t>(brname, cntname);
		break;
		
	case TFloat_t :
		if (cntvar=="")	dFlatTreeInterface->Create_Branch_Fundamental<Float_t>(brname);
		else            dFlatTreeInterface->Create_Branch_FundamentalArray<Float_t>(brname, cntname);
		break;

	case TDouble_t :
		if (cntvar=="")	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>(brname);
		else            dFlatTreeInterface->Create_Branch_FundamentalArray<Double_t>(brname, cntname);
		break;

	case TLoVect :
		if (cntvar=="")	dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>(brname);
		else            dFlatTreeInterface->Create_Branch_ClonesArray<TLorentzVector>(brname);
		break;
	}
	
	return brname;
}

















