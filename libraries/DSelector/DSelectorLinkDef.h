#if defined(__ROOTCLING__) || defined(__CINT__)

//DOCUMENTATION: https://root.cern.ch/root/htmldoc/guides/users-guide/ROOTUsersGuideChapters/AddingaClass.pdf

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link off all typedefs;
#pragma link C++ nestedclasses;

#include <string>
#include <map>
#include <utility>

#include "TVector2.h"
#include "TVector3.h"
#include "TLorentzVector.h"

#include "DBeamParticle.h"
#include "DChargedTrackHypothesis.h"
#include "DKinematicData.h"
#include "DMCThrown.h"
#include "DNeutralParticleHypothesis.h"
#include "DParticleCombo.h"
#include "DParticleComboStep.h"
#include "DSelector.h"
#include "DTreeInterface.h"
#include "DPROOFLiteManager.h"

//CLASSES
//+: Inherits from TObject, need TStreamer, etc.
//-!: Does not inherit from TObject, don't create extra stuff
#pragma link C++ class DBeamParticle-!;
#pragma link C++ class DChargedTrackHypothesis-!;
#pragma link C++ class DKinematicData-!;
#pragma link C++ class DMCThrown-!;
#pragma link C++ class DNeutralParticleHypothesis-!;
#pragma link C++ class DParticleCombo-!;
#pragma link C++ class DParticleComboStep-!;
#pragma link C++ class DTreeInterface-!;
#pragma link C++ class DPROOFLiteManager-!;
#pragma link C++ class DSelector+;

//TEMPLATE FUNCTIONS: DKinematicData
#pragma link C++ function DKinematicData::Get_Fundamental<Char_t>;
#pragma link C++ function DKinematicData::Get_Fundamental<UChar_t>;
#pragma link C++ function DKinematicData::Get_Fundamental<Short_t>;
#pragma link C++ function DKinematicData::Get_Fundamental<UShort_t>;
#pragma link C++ function DKinematicData::Get_Fundamental<Int_t>;
#pragma link C++ function DKinematicData::Get_Fundamental<UInt_t>;
#pragma link C++ function DKinematicData::Get_Fundamental<Float_t>;
#pragma link C++ function DKinematicData::Get_Fundamental<Double_t>;
#pragma link C++ function DKinematicData::Get_Fundamental<Long64_t>;
#pragma link C++ function DKinematicData::Get_Fundamental<ULong64_t>;
#pragma link C++ function DKinematicData::Get_Fundamental<Bool_t>;
#pragma link C++ function DKinematicData::Get_TObject<TVector2>;
#pragma link C++ function DKinematicData::Get_TObject<TVector3>;
#pragma link C++ function DKinematicData::Get_TObject<TLorentzVector>;

//TEMPLATE FUNCTIONS: DParticleCombo
#pragma link C++ function DParticleCombo::Get_Fundamental<Char_t>;
#pragma link C++ function DParticleCombo::Get_Fundamental<UChar_t>;
#pragma link C++ function DParticleCombo::Get_Fundamental<Short_t>;
#pragma link C++ function DParticleCombo::Get_Fundamental<UShort_t>;
#pragma link C++ function DParticleCombo::Get_Fundamental<Int_t>;
#pragma link C++ function DParticleCombo::Get_Fundamental<UInt_t>;
#pragma link C++ function DParticleCombo::Get_Fundamental<Float_t>;
#pragma link C++ function DParticleCombo::Get_Fundamental<Double_t>;
#pragma link C++ function DParticleCombo::Get_Fundamental<Long64_t>;
#pragma link C++ function DParticleCombo::Get_Fundamental<ULong64_t>;
#pragma link C++ function DParticleCombo::Get_Fundamental<Bool_t>;
#pragma link C++ function DParticleCombo::Get_TObject<TVector2>;
#pragma link C++ function DParticleCombo::Get_TObject<TVector3>;
#pragma link C++ function DParticleCombo::Get_TObject<TLorentzVector>;

//TEMPLATE FUNCTIONS: DSelector
#pragma link C++ function DSelector::Get_Fundamental<Char_t>;
#pragma link C++ function DSelector::Get_Fundamental<UChar_t>;
#pragma link C++ function DSelector::Get_Fundamental<Short_t>;
#pragma link C++ function DSelector::Get_Fundamental<UShort_t>;
#pragma link C++ function DSelector::Get_Fundamental<Int_t>;
#pragma link C++ function DSelector::Get_Fundamental<UInt_t>;
#pragma link C++ function DSelector::Get_Fundamental<Float_t>;
#pragma link C++ function DSelector::Get_Fundamental<Double_t>;
#pragma link C++ function DSelector::Get_Fundamental<Long64_t>;
#pragma link C++ function DSelector::Get_Fundamental<ULong64_t>;
#pragma link C++ function DSelector::Get_Fundamental<Bool_t>;
#pragma link C++ function DSelector::Get_TObject<TVector2>;
#pragma link C++ function DSelector::Get_TObject<TVector3>;
#pragma link C++ function DSelector::Get_TObject<TLorentzVector>;

//TEMPLATE FUNCTIONS: DTreeInterface Get_Pointer
#pragma link C++ function DTreeInterface::Get_Pointer_Fundamental<Char_t>;
#pragma link C++ function DTreeInterface::Get_Pointer_Fundamental<UChar_t>;
#pragma link C++ function DTreeInterface::Get_Pointer_Fundamental<Short_t>;
#pragma link C++ function DTreeInterface::Get_Pointer_Fundamental<UShort_t>;
#pragma link C++ function DTreeInterface::Get_Pointer_Fundamental<Int_t>;
#pragma link C++ function DTreeInterface::Get_Pointer_Fundamental<UInt_t>;
#pragma link C++ function DTreeInterface::Get_Pointer_Fundamental<Float_t>;
#pragma link C++ function DTreeInterface::Get_Pointer_Fundamental<Double_t>;
#pragma link C++ function DTreeInterface::Get_Pointer_Fundamental<Long64_t>;
#pragma link C++ function DTreeInterface::Get_Pointer_Fundamental<ULong64_t>;
#pragma link C++ function DTreeInterface::Get_Pointer_Fundamental<Bool_t>;
#pragma link C++ function DTreeInterface::Get_Pointer_TObject<TVector2>;
#pragma link C++ function DTreeInterface::Get_Pointer_TObject<TVector3>;
#pragma link C++ function DTreeInterface::Get_Pointer_TObject<TLorentzVector>;

//TEMPLATE FUNCTIONS: DTreeInterface Get
#pragma link C++ function DTreeInterface::Get_Fundamental<Char_t>;
#pragma link C++ function DTreeInterface::Get_Fundamental<UChar_t>;
#pragma link C++ function DTreeInterface::Get_Fundamental<Short_t>;
#pragma link C++ function DTreeInterface::Get_Fundamental<UShort_t>;
#pragma link C++ function DTreeInterface::Get_Fundamental<Int_t>;
#pragma link C++ function DTreeInterface::Get_Fundamental<UInt_t>;
#pragma link C++ function DTreeInterface::Get_Fundamental<Float_t>;
#pragma link C++ function DTreeInterface::Get_Fundamental<Double_t>;
#pragma link C++ function DTreeInterface::Get_Fundamental<Long64_t>;
#pragma link C++ function DTreeInterface::Get_Fundamental<ULong64_t>;
#pragma link C++ function DTreeInterface::Get_Fundamental<Bool_t>;
#pragma link C++ function DTreeInterface::Get_TObject<TVector2>;
#pragma link C++ function DTreeInterface::Get_TObject<TVector3>;
#pragma link C++ function DTreeInterface::Get_TObject<TLorentzVector>;

//TEMPLATE FUNCTIONS: DTreeInterface Create
#pragma link C++ function DTreeInterface::Create_Branch_Fundamental<Char_t>;
#pragma link C++ function DTreeInterface::Create_Branch_Fundamental<UChar_t>;
#pragma link C++ function DTreeInterface::Create_Branch_Fundamental<Short_t>;
#pragma link C++ function DTreeInterface::Create_Branch_Fundamental<UShort_t>;
#pragma link C++ function DTreeInterface::Create_Branch_Fundamental<Int_t>;
#pragma link C++ function DTreeInterface::Create_Branch_Fundamental<UInt_t>;
#pragma link C++ function DTreeInterface::Create_Branch_Fundamental<Float_t>;
#pragma link C++ function DTreeInterface::Create_Branch_Fundamental<Double_t>;
#pragma link C++ function DTreeInterface::Create_Branch_Fundamental<Long64_t>;
#pragma link C++ function DTreeInterface::Create_Branch_Fundamental<ULong64_t>;
#pragma link C++ function DTreeInterface::Create_Branch_Fundamental<Bool_t>;
#pragma link C++ function DTreeInterface::Create_Branch_FundamentalArray<Char_t>;
#pragma link C++ function DTreeInterface::Create_Branch_FundamentalArray<UChar_t>;
#pragma link C++ function DTreeInterface::Create_Branch_FundamentalArray<Short_t>;
#pragma link C++ function DTreeInterface::Create_Branch_FundamentalArray<UShort_t>;
#pragma link C++ function DTreeInterface::Create_Branch_FundamentalArray<Int_t>;
#pragma link C++ function DTreeInterface::Create_Branch_FundamentalArray<UInt_t>;
#pragma link C++ function DTreeInterface::Create_Branch_FundamentalArray<Float_t>;
#pragma link C++ function DTreeInterface::Create_Branch_FundamentalArray<Double_t>;
#pragma link C++ function DTreeInterface::Create_Branch_FundamentalArray<Long64_t>;
#pragma link C++ function DTreeInterface::Create_Branch_FundamentalArray<ULong64_t>;
#pragma link C++ function DTreeInterface::Create_Branch_FundamentalArray<Bool_t>;
#pragma link C++ function DTreeInterface::Create_Branch_NoSplitTObject<TVector2>;
#pragma link C++ function DTreeInterface::Create_Branch_NoSplitTObject<TVector3>;
#pragma link C++ function DTreeInterface::Create_Branch_NoSplitTObject<TLorentzVector>;
#pragma link C++ function DTreeInterface::Create_Branch_ClonesArray<TVector2>;
#pragma link C++ function DTreeInterface::Create_Branch_ClonesArray<TVector3>;
#pragma link C++ function DTreeInterface::Create_Branch_ClonesArray<TLorentzVector>;

//TEMPLATE FUNCTIONS: DTreeInterface Fill
#pragma link C++ function DTreeInterface::Fill_Fundamental<Char_t>;
#pragma link C++ function DTreeInterface::Fill_Fundamental<UChar_t>;
#pragma link C++ function DTreeInterface::Fill_Fundamental<Short_t>;
#pragma link C++ function DTreeInterface::Fill_Fundamental<UShort_t>;
#pragma link C++ function DTreeInterface::Fill_Fundamental<Int_t>;
#pragma link C++ function DTreeInterface::Fill_Fundamental<UInt_t>;
#pragma link C++ function DTreeInterface::Fill_Fundamental<Float_t>;
#pragma link C++ function DTreeInterface::Fill_Fundamental<Double_t>;
#pragma link C++ function DTreeInterface::Fill_Fundamental<Long64_t>;
#pragma link C++ function DTreeInterface::Fill_Fundamental<ULong64_t>;
#pragma link C++ function DTreeInterface::Fill_Fundamental<Bool_t>;
#pragma link C++ function DTreeInterface::Fill_TObject<TVector2>;
#pragma link C++ function DTreeInterface::Fill_TObject<TVector3>;
#pragma link C++ function DTreeInterface::Fill_TObject<TLorentzVector>;

#endif
