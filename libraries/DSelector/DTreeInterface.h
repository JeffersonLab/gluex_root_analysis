#ifndef DTreeInterface_h
#define DTreeInterface_h

#include <map>
#include <set>
#include <string>
#include <iostream>
#include <sstream>

#include <TTree.h>
#include <TObject.h>
#include <TClonesArray.h>
#include <TVector2.h>
#include <TVector3.h>
#include <TLorentzVector.h>
#include <TList.h>
#include <TChain.h>
#include <TMap.h>

#include <particleType.h>

using namespace std;

class DSelector;

//https://halldweb1.jlab.org/wiki/index.php/Analysis_TTreeFormat
class DTreeInterface
{
	//ASSUME: One leaf per branch: No splitting
	//ASSUME: Tree only stores: Fundamental type objects (not const char*!!), Fundamental type arrays, TObject's, TClonesArray's
	//ASSUME: TObject's are of type TVector3 & TLorentzVector: Need to expand template type calls if more are used

	friend class DSelector;

	public:

		/**************************************************************** INITIALIZE ****************************************************************/

		//Constructor
		DTreeInterface(TTree* locTree, bool locIsInputTreeFlag);

		//Set New Tree (e.g. reading a TChain)
		void Set_NewTree(TTree* locTree);

		// Is optional. If needed array size is not specified, it will be set to a default value.
		void Set_InitialArraySize(string locArraySizeBranchName, UInt_t locInitialSize);

		/********************************************************* READ GLUEX TTREE METADATA ********************************************************/

		// For setting up DParticleCombo. Returns # steps. Keys are step index, particle index.
			//ParticleIndex = -1 for initial, -2 for target, 0+ for final state
		size_t Get_ComboInfo(map<int, map<int, pair<Particle_t, string> > >& locComboInfoMap) const;
		Particle_t Get_TargetPID(void) const;
		TVector3 Get_TargetCenter(void) const;
		string Get_TreeName(void) const{return dIsInputTreeFlag ? dInputTree->GetName() : dOutputTreeMap.begin()->second->GetName();}

		/********************************************************* SPECIFY GET-ENTRY BRANCHES *******************************************************/

		//Specify get entry branches (default is all)
		void Reset_GetEntryBranches(void); //reset to default (all)
		void Clear_GetEntryBranches(void);
		void Register_GetEntryBranch(string locBranchName);
		void Remove_GetEntryBranch(string locBranchName);

		/************************************************************* GET ENTRY AND FILL ***********************************************************/

		TList* Get_UserInfo(void) const
		{
			TTree* locTree = dIsInputTreeFlag ? dInputTree : dOutputTreeMap.begin()->second;
			TChain* locChain = dynamic_cast<TChain*>(locTree);
			return (locChain != NULL) ? locChain->GetTree()->GetUserInfo() : locTree->GetUserInfo();
		}
		Int_t Get_TreeNumber(void) const
		{
			TTree* locTree = dIsInputTreeFlag ? dInputTree : dOutputTreeMap.begin()->second;
			TChain* locChain = dynamic_cast<TChain*>(locTree);
			return (locChain != NULL) ? locChain->GetTreeNumber() : 0;
		}

		void Get_Entry(Long64_t locEntry);
		void Clone_Tree(string locTreeKeyName);
		void Fill_OutputTree(string locTreeKeyName){dOutputTreeMap[locTreeKeyName]->Fill();}

		/************************************************************ GET BRANCHES AND DATA *********************************************************/

		//GET BRANCH
		TBranch* Get_Branch(string locBranchName) const;

		//GET OBJECT POINTERS
		template <typename DType> DType* Get_Pointer_Fundamental(string locBranchName) const;
		template <typename DType> DType** Get_PointerToPointerTo_TObject(string locBranchName) const;
		TClonesArray** Get_PointerToPointerTo_TClonesArray(string locBranchName); //ROOT sometimes changes the pointer!

		//GET OBJECTS
		template <typename DType> DType Get_Fundamental(string locBranchName) const;
		template <typename DType> DType Get_Fundamental(string locBranchName, UInt_t locArrayIndex) const;
		template <typename DType> DType Get_TObject(string locBranchName) const;
		template <typename DType> DType Get_TObject(string locBranchName, UInt_t locArrayIndex);

		/******************************************************** CREATE & FILL NEW BRANCHES ********************************************************/

		//On the output TTree!

		//CREATE BRANCHES
		template <typename DType> void Create_Branch_Fundamental(string locBranchName, DType* locMemoryPointer = NULL);
		template <typename DType> void Create_Branch_NoSplitTObject(string locBranchName, DType* locMemoryPointer = NULL);
		//for the fundamental array, if memory is reused from elsewhere, you MUST pass in it's size!!
		template <typename DType> void Create_Branch_FundamentalArray(string locBranchName, string locArraySizeString, unsigned int locInitialSize = 100, DType* locMemoryPointer = NULL);
		template <typename DType> void Create_Branch_ClonesArray(string locBranchName, unsigned int locSize = 100, TClonesArray* locMemoryPointer = NULL);

		//FILL BRANCHES
		template <typename DType> void Fill_Fundamental(string locBranchName, DType locValue);
		template <typename DType> void Fill_Fundamental(string locBranchName, DType locValue, unsigned int locArrayIndex);
		template <typename DType> void Fill_TObject(string locBranchName, const DType& locObject, unsigned int locArrayIndex);
		template <typename DType> void Fill_TObject(string locBranchName, const DType& locObject);

		/************************************************************** GET BRANCH MEMORY ***********************************************************/

		//ONLY CALL THESE IF YOU KNOW WHAT YOU'RE DOING!!
		template <typename DType> DType* Get_BranchMemory_Fundamental(string locBranchName) const;
		template <typename DType> DType* Get_BranchMemory_TObject(string locBranchName) const;
		template <typename DType> DType* Get_BranchMemory_FundamentalArray(string locBranchName, UInt_t& locArrayLength) const;
		TClonesArray* Get_BranchMemory_ClonesArray(string locBranchName) const;

	private:

		/*********************************************************** SETUP INPUT BRANCHES ***********************************************************/

		//Primary function
		void Set_BranchAddresses(bool locFirstTreeFlag);

		//Setup by type
		void Set_TObjectBranchAddress(string locBranchName, string locClassName);
		void Set_FundamentalBranchAddress(string locBranchName, string locBranchTitle);
		template <typename DType> void Set_FundamentalBranchAddress_Type(string locBranchName, string locArraySizeString);
		template <typename DType> void Set_TObjectBranchAddress(string locBranchName);
		void Set_ClonesArrayBranchAddress(string locBranchName);

		//Utility Function
		void Parse_BranchTitle(string locBranchTitle, string& locArraySizeString, string& locTypeString);

		/************************************************************** CLONE BRANCHES **************************************************************/

		void Clone_Branch_Fundamental(string locBranchName, string locTreeKeyName);
		void Clone_Branch_ClonesArray(string locBranchName, string locTreeKeyName);
		void Clone_Branch_FundamentalArray(string locBranchName, string locTreeKeyName);
		void Clone_Branch_TObject(string locBranchName, string locTreeKeyName);

		/************************************************************** MISCELLANEOUS ***************************************************************/

		DTreeInterface(void); //private default constructor: cannot call

		//For ROOT type string for fundamental data variables
			//Defined in https://root.cern.ch/root/htmldoc/TTree.html
		template<typename DType> struct DROOTTypeString { static const char* GetTypeString() {return "";} }; // Main template class

		/************************************************************* MEMBER VARIABLES *************************************************************/

		bool dIsInputTreeFlag;
		bool dFirstInputTreeFlag;
		TTree* dInputTree;
		map<string, TTree*> dOutputTreeMap;

		/************************************************************ GET ENTRY CONTROL *************************************************************/

		void Update_GetEntryBranches(void);
		void Increase_ArraySize(string locBranchName, string locBranchType, int locNewArraySize);
		template <typename DType> void Increase_ArraySize(string locBranchName, int locNewArraySize);

		bool dUpdateGetEntryBranchesFlag; //when user changes what branches to get, this will signify that the below maps need to be updated

		//Get Branch names
		set<string> dInputBranches; //only call GetEntry on these!!!!
		set<string> dGetEntryBranches; //user-specified subset of dInputBranches. if empty, get all from dInputBranches. 

		//dGetEntryBranches, broken up into those that are arrays, and those that aren't
		set<string> dArraySizeBranches_GetEntryNeeded;
		set<string> dNonArraySizeBranches_GetEntryNeeded;

		//array size mapping //key is array-size branch name, value is set of branches that need that array size
		map<string, set<string> > dArraySizeToBranchMap_GetEntryNeeded;

		/****************************************************** BRANCH MEMORY AND TYPE MAPPING ******************************************************/

		//For fundamental objects/arrays: memory stored in the branches themselves
		map<string, TClonesArray*> dMemoryMap_ClonesArray;
		map<string, TObject*> dMemoryMap_TObject;
		map<string, void*> dMemoryMap_Fundamental;

		map<string, string> dFundamentalBranchTypeMap; //key is branch name, value is the root string for the branch type

		//branch maps
		map<string, TBranch*> dBranchMap_InputTree; //all branches: Input tree
		map<string, TBranch*> dBranchMap_OutputTree; //new branches: FIRST output tree (object address memory is shared amongst all output trees, so getting/filling one is sufficient for all)

		/************************************************************ ARRAY SIZE MAPPING ************************************************************/

		//Branch Fundamental Array Maps
		map<string, UInt_t> dFundamentalArraySizeMap; //keys are the names of the branches that contain sizes, value is the current size //make sure has init value!!
		map<string, set<string> > dArraySizeToBranchMap; //key is array-size branch name, value is set of branches that need that array size
		map<string, string> dBranchToArraySizeMap; //key is branch name, value is the name of the branch that contains the size of its array
};

/********************************************************************* INITIALIZE *********************************************************************/

//Constructor
inline DTreeInterface::DTreeInterface(TTree* locTree, bool locIsInputTreeFlag) :
	dIsInputTreeFlag(locIsInputTreeFlag), dFirstInputTreeFlag(true),
	dInputTree(locIsInputTreeFlag ? locTree : NULL)
{
	dUpdateGetEntryBranchesFlag = true;
	if(!locIsInputTreeFlag)
		dOutputTreeMap[""] = locTree;

	//If input tree is a brand new tree, this function call does nothing.
	if(dIsInputTreeFlag)
		Set_BranchAddresses(true);
}

//Set New Tree (e.g. reading a TChain)
inline void DTreeInterface::Set_NewTree(TTree* locTree)
{
	if(!dIsInputTreeFlag)
		return;

	//reset branch maps
	dBranchMap_InputTree.clear();

	dFirstInputTreeFlag = false;
	dInputTree = locTree;
	Set_BranchAddresses(false);
}

inline void DTreeInterface::Set_InitialArraySize(string locArraySizeBranchName, UInt_t locInitialSize)
{
	dFundamentalArraySizeMap[locArraySizeBranchName] = locInitialSize;
}

/************************************************************* SPECIFY GET-ENTRY BRANCHES *************************************************************/

inline void DTreeInterface::Reset_GetEntryBranches(void)
{
	dGetEntryBranches = dInputBranches;
	dNonArraySizeBranches_GetEntryNeeded.clear();
	dArraySizeBranches_GetEntryNeeded.clear();
	dArraySizeToBranchMap_GetEntryNeeded.clear();
	dUpdateGetEntryBranchesFlag = true;
}

inline void DTreeInterface::Clear_GetEntryBranches(void)
{
	dGetEntryBranches.clear();
	dNonArraySizeBranches_GetEntryNeeded.clear();
	dArraySizeBranches_GetEntryNeeded.clear();
	dArraySizeToBranchMap_GetEntryNeeded.clear();
}

inline void DTreeInterface::Register_GetEntryBranch(string locBranchName)
{
	dGetEntryBranches.insert(locBranchName);
	dUpdateGetEntryBranchesFlag = true;
}

inline void DTreeInterface::Remove_GetEntryBranch(string locBranchName)
{
	dGetEntryBranches.erase(locBranchName);
	dNonArraySizeBranches_GetEntryNeeded.clear();
	dArraySizeBranches_GetEntryNeeded.clear();
	dArraySizeToBranchMap_GetEntryNeeded.clear();
	dUpdateGetEntryBranchesFlag = true;
}

/**************************************************************** SETUP INPUT BRANCHES ****************************************************************/

//SET BRANCH ADDRESSES
template <typename DType> inline void DTreeInterface::Set_FundamentalBranchAddress_Type(string locBranchName, string locArraySizeString)
{
	if(!dFirstInputTreeFlag)
	{
		//New tree: Branch memory already exists from the old tree. Use it instead of creating a new object
		DType* locMemoryPointer = static_cast<DType*>(dMemoryMap_Fundamental[locBranchName]);
		dInputTree->SetBranchAddress(locBranchName.c_str(), locMemoryPointer);

		//if cloning an output, set the branch address for it as well (may be invalidated when done with original TTree (read TTree::CloneTree()))
		if(!dOutputTreeMap.empty())
		{
			map<string, TTree*>::const_iterator locIterator = dOutputTreeMap.begin();
			for(; locIterator != dOutputTreeMap.end(); ++locIterator)
				locIterator->second->SetBranchAddress(locBranchName.c_str(), locMemoryPointer);
		}

		return;
	}

	if(locArraySizeString == "")
	{
		dMemoryMap_Fundamental[locBranchName] = static_cast<void*>(new DType());
		dInputTree->SetBranchAddress(locBranchName.c_str(), dMemoryMap_Fundamental[locBranchName]);
		return;
	}

	//creating an array: get the array size
	map<string, UInt_t>::iterator locArraySizeIterator = dFundamentalArraySizeMap.find(locArraySizeString);
	if(locArraySizeIterator == dFundamentalArraySizeMap.end())
		dFundamentalArraySizeMap[locArraySizeString] = 100;
	UInt_t locInitialArraySize = dFundamentalArraySizeMap[locArraySizeString];

	//Create new array, and register the array size information
	dMemoryMap_Fundamental[locBranchName] = static_cast<void*>(new DType[locInitialArraySize]);
	dInputTree->SetBranchAddress(locBranchName.c_str(), dMemoryMap_Fundamental[locBranchName]);

	dArraySizeToBranchMap[locArraySizeString].insert(locBranchName);
	dBranchToArraySizeMap[locBranchName] = locArraySizeString;
	dFundamentalArraySizeMap[locBranchName] = locInitialArraySize;
}

template <typename DType> inline void DTreeInterface::Set_TObjectBranchAddress(string locBranchName)
{
	if(dFirstInputTreeFlag)
		dMemoryMap_TObject[locBranchName] = (TObject*)(new DType());

	DType** locPointerToPointer = (DType**)&(dMemoryMap_TObject[locBranchName]);
	dInputTree->SetBranchAddress(locBranchName.c_str(), locPointerToPointer);

	//if cloning an output, set the branch address for it as well (may be invalidated when done with original TTree (read TTree::CloneTree()))
	if(!dOutputTreeMap.empty() && !dFirstInputTreeFlag)
	{
		map<string, TTree*>::const_iterator locIterator = dOutputTreeMap.begin();
		for(; locIterator != dOutputTreeMap.end(); ++locIterator)
			locIterator->second->SetBranchAddress(locBranchName.c_str(), locPointerToPointer);
	}
}

inline void DTreeInterface::Set_ClonesArrayBranchAddress(string locBranchName)
{
	if(dFirstInputTreeFlag)
		dMemoryMap_ClonesArray[locBranchName] = new TClonesArray();

	TClonesArray** locPointerToPointer = &(dMemoryMap_ClonesArray[locBranchName]);
	dInputTree->SetBranchAddress(locBranchName.c_str(), locPointerToPointer);

	if(dFirstInputTreeFlag) //initialize TClonesArray, getting the type of the array (TClass), so that cloning will work
		Get_Branch(locBranchName)->GetEntry(0);

	//if cloning an output, set the branch address for it as well (may be invalidated when done with original TTree (read TTree::CloneTree()))
	if(!dOutputTreeMap.empty() && !dFirstInputTreeFlag)
	{
		map<string, TTree*>::const_iterator locIterator = dOutputTreeMap.begin();
		for(; locIterator != dOutputTreeMap.end(); ++locIterator)
			locIterator->second->SetBranchAddress(locBranchName.c_str(), locPointerToPointer);
	}
}

/**************************************************************** GET BRANCHES AND DATA ***************************************************************/

//GET BRANCH
inline TBranch* DTreeInterface::Get_Branch(string locBranchName) const
{
	map<string, TBranch*>::const_iterator locIterator = dBranchMap_InputTree.find(locBranchName);
	if(locIterator != dBranchMap_InputTree.end())
		return locIterator->second;
	locIterator = dBranchMap_OutputTree.find(locBranchName);
	return ((locIterator != dBranchMap_OutputTree.end()) ? locIterator->second : NULL);
}

//GET POINTERS
template <typename DType> inline DType* DTreeInterface::Get_Pointer_Fundamental(string locBranchName) const
{
	TBranch* locBranch = Get_Branch(locBranchName);
	return ((locBranch != NULL) ? (DType*)locBranch->GetAddress() : NULL);
}

template <typename DType> inline DType** DTreeInterface::Get_PointerToPointerTo_TObject(string locBranchName) const
{
	TBranch* locBranch = Get_Branch(locBranchName);
	return ((locBranch != NULL) ? (DType**)locBranch->GetAddress() : NULL);
}

inline TClonesArray** DTreeInterface::Get_PointerToPointerTo_TClonesArray(string locBranchName)
{
	TBranch* locBranch = Get_Branch(locBranchName);
	if(locBranch == NULL)
		return NULL;

	return (TClonesArray**)locBranch->GetAddress();
}

//GET OBJECTS
template <typename DType> inline DType DTreeInterface::Get_Fundamental(string locBranchName) const
{
	return *(DType*)Get_Branch(locBranchName)->GetAddress();
}

template <typename DType> inline DType DTreeInterface::Get_Fundamental(string locBranchName, UInt_t locArrayIndex) const
{
	return ((DType*)Get_Branch(locBranchName)->GetAddress())[locArrayIndex];
}

template <typename DType> inline DType DTreeInterface::Get_TObject(string locBranchName) const
{
	return **(DType**)Get_Branch(locBranchName)->GetAddress();
}

template <typename DType> inline DType DTreeInterface::Get_TObject(string locBranchName, UInt_t locArrayIndex)
{
	return *((DType*)(*Get_PointerToPointerTo_TClonesArray(locBranchName))->At(locArrayIndex));
}

//INCREASE ARRAY SIZE //For when reading only! Not when writing
template <typename DType> inline void DTreeInterface::Increase_ArraySize(string locBranchName, int locNewArraySize)
{
	//create a new, larger array if the current one is too small
		//DOES NOT copy the old results!  In other words, only call BETWEEN entries, not DURING an entry
	DType* locOldBranchAddress = Get_Pointer_Fundamental<DType>(locBranchName);

	dMemoryMap_Fundamental[locBranchName] = static_cast<void*>(new DType[locNewArraySize]);
	Get_Branch(locBranchName)->SetAddress(dMemoryMap_Fundamental[locBranchName]);

	//if cloning into output trees, must set their branches addresses as well
	map<string, TTree*>::const_iterator locIterator = dOutputTreeMap.begin();
	for(; locIterator != dOutputTreeMap.end(); ++locIterator)
		locIterator->second->GetBranch(locBranchName.c_str())->SetAddress(dMemoryMap_Fundamental[locBranchName]);

	dFundamentalArraySizeMap[locBranchName] = locNewArraySize;
	delete[] locOldBranchAddress;
}

/*************************************************************** CREATE BRANCHES ************************************************************/

template <typename DType> inline void DTreeInterface::Create_Branch_Fundamental(string locBranchName, DType* locMemoryPointer)
{
	if(dOutputTreeMap.empty())
	{
		cout << "WARNING: CANNOT CREATE BRANCH " << locBranchName << ": OUTPUT TTREE DOES NOT EXIST." << endl;
		return;
	}
	if(dMemoryMap_Fundamental.find(locBranchName) != dMemoryMap_Fundamental.end())
	{
		cout << "WARNING: CANNOT CREATE BRANCH " << locBranchName << ": IT ALREADY EXISTS." << endl;
		return;
	}

	string locTypeString = DROOTTypeString<DType>::GetTypeString();
	string locTypeName = locBranchName + string("/") + locTypeString;

	dMemoryMap_Fundamental[locBranchName] = static_cast<void*>((locMemoryPointer == NULL) ? new DType() : locMemoryPointer);
	void* locVoidMemoryPointer = dMemoryMap_Fundamental[locBranchName];
	
	map<string, TTree*>::const_iterator locIterator = dOutputTreeMap.begin();
	for(; locIterator != dOutputTreeMap.end(); ++locIterator)
	{
		TBranch* locBranch = locIterator->second->Branch(locBranchName.c_str(), locVoidMemoryPointer, locTypeName.c_str());
		if(locIterator == dOutputTreeMap.begin())
			dBranchMap_OutputTree[locBranchName] = locBranch;
	}
}

template <typename DType> inline void DTreeInterface::Create_Branch_NoSplitTObject(string locBranchName, DType* locMemoryPointer)
{
	if(dOutputTreeMap.empty())
	{
		cout << "WARNING: CANNOT CREATE BRANCH " << locBranchName << ": OUTPUT TTREE DOES NOT EXIST." << endl;
		return;
	}
	if(dMemoryMap_TObject.find(locBranchName) != dMemoryMap_TObject.end())
	{
		cout << "WARNING: CANNOT CREATE BRANCH " << locBranchName << ": IT ALREADY EXISTS." << endl;
		return;
	}

	dMemoryMap_TObject[locBranchName] = static_cast<TObject*>((locMemoryPointer == NULL) ? new DType() : locMemoryPointer);
	TObject* locTObjectMemoryPointer = dMemoryMap_TObject[locBranchName];

	string locClassName = dMemoryMap_TObject[locBranchName]->ClassName();

	map<string, TTree*>::const_iterator locIterator = dOutputTreeMap.begin();
	for(; locIterator != dOutputTreeMap.end(); ++locIterator)
	{
		TBranch* locBranch = locIterator->second->Branch(locBranchName.c_str(), locClassName.c_str(), locTObjectMemoryPointer, 32000, 0); //0: don't split
		if(locIterator == dOutputTreeMap.begin())
			dBranchMap_OutputTree[locBranchName] = locBranch;
	}
}

template <typename DType> inline void DTreeInterface::Create_Branch_FundamentalArray(string locBranchName, string locArraySizeString, unsigned int locInitialSize, DType* locMemoryPointer)
{
	if(dOutputTreeMap.empty())
	{
		cout << "WARNING: CANNOT CREATE BRANCH " << locBranchName << ": OUTPUT TTREE DOES NOT EXIST." << endl;
		return;
	}
	if(dMemoryMap_Fundamental.find(locBranchName) != dMemoryMap_Fundamental.end())
	{
		cout << "WARNING: CANNOT CREATE BRANCH " << locBranchName << ": IT ALREADY EXISTS." << endl;
		return;
	}

	string locTypeString = DROOTTypeString<DType>::GetTypeString();
	string locArrayName = locBranchName + string("[") + locArraySizeString + string("]/") + locTypeString;

	dMemoryMap_Fundamental[locBranchName] = static_cast<void*>((locMemoryPointer == NULL) ? new DType[locInitialSize] : locMemoryPointer);
	void* locVoidMemoryPointer = dMemoryMap_Fundamental[locBranchName];

	dFundamentalArraySizeMap[locBranchName] = locInitialSize;

	map<string, TTree*>::const_iterator locIterator = dOutputTreeMap.begin();
	for(; locIterator != dOutputTreeMap.end(); ++locIterator)
	{
		TBranch* locBranch = locIterator->second->Branch(locBranchName.c_str(), locVoidMemoryPointer, locArrayName.c_str());
		if(locIterator == dOutputTreeMap.begin())
			dBranchMap_OutputTree[locBranchName] = locBranch;
	}
}

template <typename DType> inline void DTreeInterface::Create_Branch_ClonesArray(string locBranchName, unsigned int locSize, TClonesArray* locMemoryPointer)
{
	if(dOutputTreeMap.empty())
	{
		cout << "WARNING: CANNOT CREATE BRANCH " << locBranchName << ": OUTPUT TTREE DOES NOT EXIST." << endl;
		return;
	}
	if(dMemoryMap_ClonesArray.find(locBranchName) != dMemoryMap_ClonesArray.end())
	{
		cout << "WARNING: CANNOT CREATE BRANCH " << locBranchName << ": IT ALREADY EXISTS." << endl;
		return;
	}

	dMemoryMap_ClonesArray[locBranchName] = (locMemoryPointer == NULL) ? new TClonesArray(DType::Class()->GetName(), locSize) : locMemoryPointer;
	TClonesArray** locPointerToPointer = &(dMemoryMap_ClonesArray[locBranchName]);

	map<string, TTree*>::const_iterator locIterator = dOutputTreeMap.begin();
	for(; locIterator != dOutputTreeMap.end(); ++locIterator)
	{
		TBranch* locBranch = locIterator->second->Branch(locBranchName.c_str(), locPointerToPointer, 32000, 0); //0: don't split
		if(locIterator == dOutputTreeMap.begin())
			dBranchMap_OutputTree[locBranchName] = locBranch;
	}
}

/*************************************************************** CLONE BRANCHES *************************************************************/

inline void DTreeInterface::Clone_Branch_Fundamental(string locBranchName, string locTreeKeyName)
{
	void* locMemoryPointer = dMemoryMap_Fundamental[locBranchName];
	string locTypeName = locBranchName + string("/") + dFundamentalBranchTypeMap[locBranchName];
	dBranchMap_OutputTree[locBranchName] = dOutputTreeMap[locTreeKeyName]->Branch(locBranchName.c_str(), locMemoryPointer, locTypeName.c_str());
}

inline void DTreeInterface::Clone_Branch_TObject(string locBranchName, string locTreeKeyName)
{
	TObject* locMemoryPointer = dMemoryMap_TObject[locBranchName];
	string locClassName = dMemoryMap_TObject[locBranchName]->ClassName();
	dBranchMap_OutputTree[locBranchName] = dOutputTreeMap[locTreeKeyName]->Branch(locBranchName.c_str(), locClassName.c_str(), locMemoryPointer, 32000, 0); //0: don't split
}

inline void DTreeInterface::Clone_Branch_FundamentalArray(string locBranchName, string locTreeKeyName)
{
	void* locMemoryPointer = dMemoryMap_Fundamental[locBranchName];
	string locArraySizeString = dBranchToArraySizeMap[locBranchName];
	string locArrayName = locBranchName + string("[") + locArraySizeString + string("]/") + dFundamentalBranchTypeMap[locBranchName];
	dBranchMap_OutputTree[locBranchName] = dOutputTreeMap[locTreeKeyName]->Branch(locBranchName.c_str(), locMemoryPointer, locArrayName.c_str());
}

inline void DTreeInterface::Clone_Branch_ClonesArray(string locBranchName, string locTreeKeyName)
{
	TClonesArray** locPointerToPointer = &(dMemoryMap_ClonesArray[locBranchName]);
	dBranchMap_OutputTree[locBranchName] = dOutputTreeMap[locTreeKeyName]->Branch(locBranchName.c_str(), locPointerToPointer, 32000, 0); //0: don't split
}

/**************************************************************** FILL BRANCHES *************************************************************/

template <typename DType> inline void DTreeInterface::Fill_Fundamental(string locBranchName, DType locValue)
{
	*Get_Pointer_Fundamental<DType>(locBranchName) = locValue;
}

template <typename DType> inline void DTreeInterface::Fill_Fundamental(string locBranchName, DType locValue, unsigned int locArrayIndex)
{
	//create a new, larger array if the current one is too small
		//would rather do this in advance, but don't assume that the user has done so!
	unsigned int locCurrentArraySize = dFundamentalArraySizeMap[locBranchName];
	DType* locArray = Get_Pointer_Fundamental<DType>(locBranchName);
	if(locArrayIndex >= locCurrentArraySize)
	{
		DType* locOldArray = locArray;
		unsigned int locNewArraySize = locArrayIndex + 100;
		dMemoryMap_Fundamental[locBranchName] = static_cast<void*>(new DType[locNewArraySize]);

		map<string, TTree*>::const_iterator locIterator = dOutputTreeMap.begin();
		for(; locIterator != dOutputTreeMap.end(); ++locIterator)
			locIterator->second->SetBranchAddress(locBranchName.c_str(), dMemoryMap_Fundamental[locBranchName]);

		locArray = Get_Pointer_Fundamental<DType>(locBranchName);

		//copy the old contents into the new array
		for(unsigned int loc_i = 0; loc_i < locCurrentArraySize; ++loc_i)
			locArray[loc_i] = locOldArray[loc_i];

		delete[] locOldArray;
		dFundamentalArraySizeMap[locBranchName] = locNewArraySize;
	}

	//set the data
	locArray[locArrayIndex] = locValue;
}

template <typename DType> inline void DTreeInterface::Fill_TObject(string locBranchName, const DType& locObject, unsigned int locArrayIndex)
{
	TClonesArray* locClonesArray = *Get_PointerToPointerTo_TClonesArray(locBranchName);
	if(locArrayIndex == 0) //ASSUMES FILLED IN ORDER!!!
		locClonesArray->Clear(); //empties array
	*(DType*)locClonesArray->ConstructedAt(locArrayIndex) = locObject;
}

template <typename DType> inline void DTreeInterface::Fill_TObject(string locBranchName, const DType& locObject)
{
	**Get_PointerToPointerTo_TObject<DType>(locBranchName) = locObject;
}

/************************************************************** GET BRANCH MEMORY ***********************************************************/

//ONLY CALL THESE IF YOU KNOW WHAT YOU'RE DOING!! //And if you're calling them, you probably don't
template <typename DType> inline DType* DTreeInterface::Get_BranchMemory_Fundamental(string locBranchName) const
{
	if(dFundamentalArraySizeMap.find(locBranchName) != dFundamentalArraySizeMap.end())
		return NULL; //is an array! wrong method!

	auto locIterator = dMemoryMap_Fundamental.find(locBranchName);
	if(locIterator == dMemoryMap_Fundamental.end())
		return NULL;
	return static_cast<DType*>(locIterator->second);
}

template <typename DType> inline DType* DTreeInterface::Get_BranchMemory_TObject(string locBranchName) const
{
	auto locIterator = dMemoryMap_TObject.find(locBranchName);
	if(locIterator == dMemoryMap_TObject.end())
		return NULL;
	return static_cast<DType*>(locIterator->second);
}

template <typename DType> inline DType* DTreeInterface::Get_BranchMemory_FundamentalArray(string locBranchName, UInt_t& locArrayLength) const
{
	locArrayLength = 0;
	auto locIterator = dMemoryMap_Fundamental.find(locBranchName);
	if(locIterator == dMemoryMap_Fundamental.end())
		return NULL;

	locArrayLength = dFundamentalArraySizeMap.find(locBranchName)->second;
	return static_cast<DType*>(locIterator->second);
}

inline TClonesArray* DTreeInterface::Get_BranchMemory_ClonesArray(string locBranchName) const
{
	auto locIterator = dMemoryMap_ClonesArray.find(locBranchName);
	if(locIterator == dMemoryMap_ClonesArray.end())
		return NULL;
	return locIterator->second;
}

/************************************************************** TEMPLATE SPECIALIZATIONS **************************************************************/

template<> struct DTreeInterface::DROOTTypeString<const char*> { static const char* GetTypeString() {return "C";} };
template<> struct DTreeInterface::DROOTTypeString<Char_t> { static const char* GetTypeString() {return "B";} };
template<> struct DTreeInterface::DROOTTypeString<UChar_t> { static const char* GetTypeString() {return "b";} };
template<> struct DTreeInterface::DROOTTypeString<Short_t> { static const char* GetTypeString() {return "S";} };
template<> struct DTreeInterface::DROOTTypeString<UShort_t> { static const char* GetTypeString() {return "s";} };
template<> struct DTreeInterface::DROOTTypeString<Int_t> { static const char* GetTypeString() {return "I";} };
template<> struct DTreeInterface::DROOTTypeString<UInt_t> { static const char* GetTypeString() {return "i";} };
template<> struct DTreeInterface::DROOTTypeString<Float_t> { static const char* GetTypeString() {return "F";} };
template<> struct DTreeInterface::DROOTTypeString<Double_t> { static const char* GetTypeString() {return "D";} };
template<> struct DTreeInterface::DROOTTypeString<Long64_t> { static const char* GetTypeString() {return "L";} };
template<> struct DTreeInterface::DROOTTypeString<ULong64_t> { static const char* GetTypeString() {return "l";} };
template<> struct DTreeInterface::DROOTTypeString<Bool_t> { static const char* GetTypeString() {return "O";} };

#endif //DTreeInterface_h
