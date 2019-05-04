#include "DComboTreeHelper.h"

#include "TObjString.h"
#include "TObjArray.h"
#include "TLorentzVector.h"

#include <algorithm>
#include <particleType.h>

DComboTreeHelper::DComboTreeHelper(DParticleCombo *combwrap, DTreeInterface *treeinterf, TString dCombinations) : 
	dComboWrapper(combwrap), dTreeInterface(treeinterf)
{
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
		
		DKinematicData *iniPart = dStepWrapper->Get_InitialParticle();
		printf("Step %d : %s -> ", i, Get_ShortName(iniPart->Get_PID()));
		
		// loop over final states
		for (int j=0;j<nParts;++j)
		{
			DKinematicData *part = dStepWrapper->Get_FinalParticle(j);
			cout <<"  "<<Get_ShortName(part->Get_PID())<<" ";
			
			// fVecWrappers is a vector of particle wrappers
			fVecWrappers.push_back(part);
			fullFS = Form("%s%s ",fullFS.Data(),Get_ShortName(part->Get_PID()));
		}
		cout <<endl<<"\""<<fullFS<<"\""<<endl;	
	}
	
	// print what was found
	cout <<endl<<"The particle list is:"<<endl;
	for (size_t i=0;i<fVecWrappers.size();++i)
	{
		cout<<setw(2)<<i<<") "<<setw(4)<<Get_ShortName(fVecWrappers[i]->Get_PID())<<"  [wrapper prt = "<<fVecWrappers[i]<<"]"<<endl;
	}
	
	// now we prepare the combinations requested by the user
	// the names have to match the short names (+/- and upper case allowed -> will be replaced)
	cout <<endl<<"Requested combinatorics info for:"<<endl;
	vector<TString> vec = SplitString(dCombinations,";");
	for (size_t i=0;i<vec.size();++i)
	{
		// some replacements and preparations for the FS string (e.g. K+ -> kp)
		vec[i].ToLower();
		vec[i].ReplaceAll("+","p");
		vec[i].ReplaceAll("-","m");
		cout <<i<<". "<<vec[i]<<" : ";
		
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
// creates branches 
// ---------------------------------------------------------------
size_t DComboTreeHelper::CreateBranches()
{
	if (dTreeInterface==nullptr) return 0;
	
	// will be filled with branch names created
	vector<TString> brname;
	
	for (size_t i=0;i<fCombInd.size();++i)
	{
		if (fCombInd[i].size()==0) continue;
		
		TString pref = fPrefix[i];

		brname.push_back( Branch(TInt_t,   pref, "ncomb"   ) );           // number of combinations
		brname.push_back( Branch(TLoVect,  pref, "p4_kin"  , "ncomb") );  // fitted p4
		brname.push_back( Branch(TLoVect,  pref, "p4_meas" , "ncomb") );  // measured p4
		brname.push_back( Branch(TFloat_t, pref, "m"       , "ncomb") );  // mass
		brname.push_back( Branch(TFloat_t, pref, "mf"      , "ncomb") );  // fitted mass
		brname.push_back( Branch(TFloat_t, pref, "chg"     , "ncomb") );  // charge
		brname.push_back( Branch(TInt_t,   pref, "idxs"    , "ncomb") );  // int (taken as bit pattern) with indices of used tracks
		brname.push_back( Branch(TInt_t,   pref, "uni"     , "ncomb") );  // unique flag measured (only valid before additional cuts applied!)
		brname.push_back( Branch(TInt_t,   pref, "unif"    , "ncomb") );  // unique flag fitted (only valid before additional cuts applied!)
		
		int ncombpart = CntBits(fCombInd[i][0]);
		
		switch (ncombpart)
		{
		case 2:
			brname.push_back( Branch(TFloat_t, pref, "dec"  , "ncomb"));  // decay angle
			brname.push_back( Branch(TFloat_t, pref, "cdec" , "ncomb"));  // cos(decay angle)
			brname.push_back( Branch(TFloat_t, pref, "oang" , "ncomb"));  // openning angle
			
			break;
			
			
		case 3:
			brname.push_back( Branch(TFloat_t, pref, "m01"  , "ncomb"));  // m(p0 p1)
			brname.push_back( Branch(TFloat_t, pref, "m02"  , "ncomb"));  // m(p0 p2)
			brname.push_back( Branch(TFloat_t, pref, "m12"  , "ncomb"));  // m(p1 p2);

			brname.push_back( Branch(TFloat_t, pref, "dal01", "ncomb"));  // m2(p0 p1) for Dalitz plots
			brname.push_back( Branch(TFloat_t, pref, "dal02", "ncomb"));  // m2(p0 p2)
			brname.push_back( Branch(TFloat_t, pref, "dal12", "ncomb"));  // m2(p1 p2)
			
			break;		
		}
		
	}
	cout <<endl<<"[DComboTreeHelper] Create branches :"<<endl;
	for (auto name:brname) cout <<name<<"  ";
	cout <<endl<<endl;
	
	return brname.size();	
}

// ---------------------------------------------------------------
// fills branches 
// ---------------------------------------------------------------
void DComboTreeHelper::Fill(int num)
{
	// no tree -> return
	if (dTreeInterface==nullptr) return;

	// if new event, clean up the unique check memory
	if (num==0)
	{
		fUniMeas.clear();
		fUniFit.clear();
		
		for (auto x: fCombInd)
		{
			// fill with empty sets
			set<float> v1, v2;
			fUniMeas.push_back(v1);
			fUniFit.push_back(v2);
		}
	}

	// loop over all sub sets (e.g. K+ K-, pi+ pi0, gam gam, ...)
	for (size_t i=0;i<fCombInd.size();++i)
	{
		if (fCombInd[i].size()==0) continue;
		
		TString pref           = fPrefix[i];   // prefix for the branches of this combination
		vector<ULong2> combs   = fCombInd[i];  // vector with combinatoric indices (as bit pattern in ulong)
		size_t ncombs          = combs.size(); // number of combinations (of the particle wrapper inidices, _not_ track indices)

		dTreeInterface->Fill_Fundamental<Int_t>((pref+"_ncomb").Data(), ncombs);              
		
		int curridx = 0;
		
		for (size_t j=0;j<ncombs;++j)
		{	
			// here we add up the p4 for the combinations	
			TLorentzVector lvcomb_kin(0.,0.,0.,0.);  // fitted
			TLorentzVector lvcomb_meas(0.,0.,0.,0.); // measured
			
			Float_t chg = 0;  // collect charge
			ULong2  idx = 1;  // 1 bit mask to select particles for a composite
			
			// a vector of the fitted p4 for computing decay angels, Dalitz plot variables
			vector<TLorentzVector> vlv;
			
			// this is a bit vector representing the used track/neutral IDs; stored in ttree
			ULong2 IDpatt = 0;
	
			for (size_t k=0; k<fVecWrappers.size(); ++k)
			{
				if (idx & combs[j]) 
				{
					// store used trackID in bit marker
					Int_t trkID = fVecWrappers[k]->Get_ID();
					IDpatt |= 1<<trkID;
					
					Particle_t pid = fVecWrappers[k]->Get_PID();
					
					// add up composite p4 and charge				
					lvcomb_kin  += fVecWrappers[k]->Get_P4();
					lvcomb_meas += Is_FinalStateParticle(pid) ? fVecWrappers[k]->Get_P4_Measured() : fVecWrappers[k]->Get_P4();
 					chg         += ParticleCharge(pid);
 					
					//store p4 kin fit
 					vlv.push_back(fVecWrappers[k]->Get_P4());
				}
				idx *= 2; // shift mask to next place
			}
			
			// use invariant masses as key for uniqueness (FIXME: the bit markers are not unique till now, since PID is not taken into account...)
			float m_meas = lvcomb_meas.M();
			float m_kin  = lvcomb_kin.M();
						
			Int_t unique    = fUniMeas[i].find(m_meas) == fUniMeas[i].end();
			if (unique) fUniMeas[i].insert(m_meas);
			
			Int_t uniquekin = fUniFit[i].find(m_meas) == fUniFit[i].end();
			if (uniquekin) fUniFit[i].insert(m_kin);
			
		
			dTreeInterface->Fill_TObject<TLorentzVector>((pref+"_p4_kin").Data(),  lvcomb_kin,  curridx);    
			dTreeInterface->Fill_TObject<TLorentzVector>((pref+"_p4_meas").Data(), lvcomb_meas, curridx);   
			
 			dTreeInterface->Fill_Fundamental<Float_t>((pref+"_m").Data(),      m_meas        , curridx); 
 			dTreeInterface->Fill_Fundamental<Float_t>((pref+"_mf").Data(),     m_kin         , curridx); 
			dTreeInterface->Fill_Fundamental<Float_t>((pref+"_chg").Data(),    chg           , curridx);  
			dTreeInterface->Fill_Fundamental<Int_t>(  (pref+"_idxs").Data(),   IDpatt        , curridx);  
			dTreeInterface->Fill_Fundamental<Int_t>(  (pref+"_uni").Data(),    unique        , curridx);  
			dTreeInterface->Fill_Fundamental<Int_t>(  (pref+"_unif").Data(),   uniquekin     , curridx);  
 
 
			switch (vlv.size())
			{
			case 2:
				{
 					TLorentzVector lv0 = vlv[0]; 
					TLorentzVector lv1 = vlv[1]; 

					dTreeInterface->Fill_Fundamental<Float_t>((pref+"_dec").Data(),   DecayAngle(lvcomb_kin,lv0),         curridx);  
 					dTreeInterface->Fill_Fundamental<Float_t>((pref+"_cdec").Data(),  cos(DecayAngle(lvcomb_kin,lv0)),    curridx);  
 					dTreeInterface->Fill_Fundamental<Float_t>((pref+"_oang").Data(),  lv0.Vect().Angle(lv1.Vect()),       curridx); 

					break;
				}

			case 3:
				{
					TLorentzVector lv01 = vlv[0]+vlv[1]; 
					TLorentzVector lv02 = vlv[0]+vlv[2]; 
					TLorentzVector lv12 = vlv[1]+vlv[2]; 

 					dTreeInterface->Fill_Fundamental<Float_t>((pref+"_m01").Data(), lv01.M(), curridx);  
 					dTreeInterface->Fill_Fundamental<Float_t>((pref+"_m02").Data(), lv02.M(), curridx);  
 					dTreeInterface->Fill_Fundamental<Float_t>((pref+"_m12").Data(), lv12.M(), curridx);  

					dTreeInterface->Fill_Fundamental<Float_t>((pref+"_dal01").Data(), lv01.M2(), curridx);  
 					dTreeInterface->Fill_Fundamental<Float_t>((pref+"_dal02").Data(), lv02.M2(), curridx);  
 					dTreeInterface->Fill_Fundamental<Float_t>((pref+"_dal12").Data(), lv12.M2(), curridx);  

					break;	
				}	
			}
			
			curridx++;
		}
		
	}
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
double DComboTreeHelper::DecayAngle(TLorentzVector moth, TLorentzVector dau)
{
	TVector3 boost = moth.BoostVector();
	
	TLorentzVector dauboost = dau;
	dau.Boost(-boost);
	
	return moth.Vect().Angle(dauboost.Vect());
}

// ---------------------------------------------------------------
// Create Branch
// ---------------------------------------------------------------
TString DComboTreeHelper::Branch(BrType type, TString pref, TString name, TString cntvar)
{
	TString tbrname  = pref+"_"+name;
	TString tcntname = pref+"_"+cntvar;
	
	string brname  = tbrname.Data();
	string cntname = tcntname.Data();
	
	switch (type) 
	{
	case TInt_t :
		if (cntvar=="")	dTreeInterface->Create_Branch_Fundamental<Int_t>(brname);
		else            dTreeInterface->Create_Branch_FundamentalArray<Int_t>(brname, cntname);
		break;
		
	case TFloat_t :
		if (cntvar=="")	dTreeInterface->Create_Branch_Fundamental<Float_t>(brname);
		else            dTreeInterface->Create_Branch_FundamentalArray<Float_t>(brname, cntname);
		break;

	case TDouble_t :
		if (cntvar=="")	dTreeInterface->Create_Branch_Fundamental<Double_t>(brname);
		else            dTreeInterface->Create_Branch_FundamentalArray<Double_t>(brname, cntname);
		break;

	case TLoVect :
		if (cntvar=="")	dTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>(brname);
		else            dTreeInterface->Create_Branch_ClonesArray<TLorentzVector>(brname);
		break;
	}
	
	return brname;
}

















