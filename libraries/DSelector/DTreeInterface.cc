#include "DTreeInterface.h"

/**************************************************************** SETUP INPUT BRANCHES ****************************************************************/

void DTreeInterface::Set_BranchAddresses(bool locFirstTreeFlag)
{
	//ASSUME: One leaf per branch: No splitting

	//Loop over branches
	//TChain* locChain = dynamic_cast<TChain*>(dInputTree);
	//cout << "TREE, CHAIN POINTERS = " << dInputTree << ", " << locChain << endl;
	TObjArray* locBranchArray = dInputTree->GetListOfBranches();
	for(Int_t loc_i = 0; loc_i < locBranchArray->GetEntriesFast(); ++loc_i)
	{
		//Get branch
		TBranch* locBranch = dynamic_cast<TBranch*>(locBranchArray->At(loc_i));
		string locBranchName = locBranch->GetName();

		//Register branch
		dInputBranches.insert(locBranchName);
		dBranchMap_InputTree[locBranchName] = locBranch;

		//Set branch address based on type
		string locClassName = locBranch->GetClassName(); //is "" for fundamental types, TLorentzVector, TClonesArray (not subtype!)
		if(locClassName == "") //Is fundamental type
			Set_FundamentalBranchAddress(locBranchName, locBranch->GetTitle());
		else if(locClassName == "TClonesArray")
			Set_ClonesArrayBranchAddress(locBranchName);
		else //TObject
			Set_TObjectBranchAddress(locBranchName, locClassName);
	}

	if(locFirstTreeFlag)
		dGetEntryBranches = dInputBranches;
}

void DTreeInterface::Clone_Tree(string locTreeKeyName)
{
	//make sure you've cd'd into the output file before calling this function!
	if(dOutputTreeMap.find(locTreeKeyName) != dOutputTreeMap.end())
		return; //already cloned!!

	//cannot use TTree::CloneTree(): In PROOF, the selector is only given the TTree, and not the TChain
	//and, the branch addresses change from tree-to-tree (file-to-file), which is assumed not to happen in TTree::CloneTree()
	//so, must set it up manually (thanks ROOT!)
	dOutputTreeMap[locTreeKeyName] = new TTree(dInputTree->GetName(), dInputTree->GetName());

	//set user info
	TList* locInputUserInfo = Get_UserInfo();
	TList* locOutputUserInfo = dOutputTreeMap[locTreeKeyName]->GetUserInfo();
	for(Int_t loc_i = 0; loc_i < locInputUserInfo->GetSize(); ++loc_i)
		locOutputUserInfo->Add(locInputUserInfo->At(loc_i)->Clone());

	//Loop over branches in order
	TObjArray* locBranchArray = dInputTree->GetListOfBranches();
	for(Int_t loc_i = 0; loc_i < locBranchArray->GetEntriesFast(); ++loc_i)
	{
		//Get branch
		TBranch* locBranch = dynamic_cast<TBranch*>(locBranchArray->At(loc_i));
		string locBranchName = locBranch->GetName();

		//Clone branch based on type
		if(dMemoryMap_Fundamental.find(locBranchName) != dMemoryMap_Fundamental.end())
		{
			//is fundamental type. check if it is an array or not
			if(dBranchToArraySizeMap.find(locBranchName) == dBranchToArraySizeMap.end())
				Clone_Branch_Fundamental(locBranchName, locTreeKeyName); //not an array
			else //is an array
				Clone_Branch_FundamentalArray(locBranchName, locTreeKeyName);
		}
		else if(dMemoryMap_ClonesArray.find(locBranchName) != dMemoryMap_ClonesArray.end())
			Clone_Branch_ClonesArray(locBranchName, locTreeKeyName); //is a clonesarray
		else //is a tobject
			Clone_Branch_TObject(locBranchName, locTreeKeyName);
	}
}

void DTreeInterface::Set_FundamentalBranchAddress(string locBranchName, string locBranchTitle)
{
	string locArraySizeString, locTypeString;
	Parse_BranchTitle(locBranchTitle, locArraySizeString, locTypeString);
	dFundamentalBranchTypeMap[locBranchName] = locTypeString;

	if(locTypeString == "C")
		Set_FundamentalBranchAddress_Type<const char*>(locBranchName, locArraySizeString);
	else if(locTypeString == "B")
		Set_FundamentalBranchAddress_Type<Char_t>(locBranchName, locArraySizeString);
	else if(locTypeString == "b")
		Set_FundamentalBranchAddress_Type<UChar_t>(locBranchName, locArraySizeString);
	else if(locTypeString == "S")
		Set_FundamentalBranchAddress_Type<Short_t>(locBranchName, locArraySizeString);
	else if(locTypeString == "s")
		Set_FundamentalBranchAddress_Type<UShort_t>(locBranchName, locArraySizeString);
	else if(locTypeString == "I")
		Set_FundamentalBranchAddress_Type<Int_t>(locBranchName, locArraySizeString);
	else if(locTypeString == "i")
		Set_FundamentalBranchAddress_Type<UInt_t>(locBranchName, locArraySizeString);
	else if(locTypeString == "F")
		Set_FundamentalBranchAddress_Type<Float_t>(locBranchName, locArraySizeString);
	else if(locTypeString == "D")
		Set_FundamentalBranchAddress_Type<Double_t>(locBranchName, locArraySizeString);
	else if(locTypeString == "L")
		Set_FundamentalBranchAddress_Type<Long64_t>(locBranchName, locArraySizeString);
	else if(locTypeString == "l")
		Set_FundamentalBranchAddress_Type<ULong64_t>(locBranchName, locArraySizeString);
	else if(locTypeString == "O")
		Set_FundamentalBranchAddress_Type<Bool_t>(locBranchName, locArraySizeString);
	else
		cout << "WARNING: TYPE NOT RECOGNIZED FOR BRANCH: " << locBranchName << endl;
}

void DTreeInterface::Set_TObjectBranchAddress(string locBranchName, string locClassName)
{
	if(locClassName == "TVector3")
		Set_TObjectBranchAddress<TVector3>(locBranchName);
	else if(locClassName == "TVector2")
		Set_TObjectBranchAddress<TVector2>(locBranchName);
	else if(locClassName == "TLorentzVector")
		Set_TObjectBranchAddress<TLorentzVector>(locBranchName);
	else
		cout << "WARNING: TYPE NOT RECOGNIZED FOR BRANCH: " << locBranchName << endl;
}

/****************************************************************** BRANCH UTILITIES ******************************************************************/

void DTreeInterface::Parse_BranchTitle(string locBranchTitle, string& locArraySizeString, string& locTypeString)
{
	locArraySizeString = "";
	locTypeString = "";

	size_t locLeftBracketIndex = locBranchTitle.find("[");
	size_t locRightBracketIndex = locBranchTitle.find("]");
	if((locLeftBracketIndex != string::npos) && (locRightBracketIndex != string::npos))
		locArraySizeString = locBranchTitle.substr(locLeftBracketIndex + 1, locRightBracketIndex - locLeftBracketIndex - 1);

	size_t locSlashIndex = locBranchTitle.find("/");
	if(locSlashIndex != string::npos)
		locTypeString = locBranchTitle.substr(locSlashIndex + 1);
}

void DTreeInterface::Increase_ArraySize(string locBranchName, string locBranchType, int locNewArraySize)
{
	if(locBranchType == "C")
		Increase_ArraySize<const char*>(locBranchName, locNewArraySize);
	else if(locBranchType == "B")
		Increase_ArraySize<Char_t>(locBranchName, locNewArraySize);
	else if(locBranchType == "b")
		Increase_ArraySize<UChar_t>(locBranchName, locNewArraySize);
	else if(locBranchType == "S")
		Increase_ArraySize<Short_t>(locBranchName, locNewArraySize);
	else if(locBranchType == "s")
		Increase_ArraySize<UShort_t>(locBranchName, locNewArraySize);
	else if(locBranchType == "I")
		Increase_ArraySize<Int_t>(locBranchName, locNewArraySize);
	else if(locBranchType == "i")
		Increase_ArraySize<UInt_t>(locBranchName, locNewArraySize);
	else if(locBranchType == "F")
		Increase_ArraySize<Float_t>(locBranchName, locNewArraySize);
	else if(locBranchType == "D")
		Increase_ArraySize<Double_t>(locBranchName, locNewArraySize);
	else if(locBranchType == "L")
		Increase_ArraySize<Long64_t>(locBranchName, locNewArraySize);
	else if(locBranchType == "l")
		Increase_ArraySize<ULong64_t>(locBranchName, locNewArraySize);
	else if(locBranchType == "O")
		Increase_ArraySize<Bool_t>(locBranchName, locNewArraySize);
	else
		cout << "WARNING: TYPE NOT RECOGNIZED FOR BRANCH: " << locBranchName << endl;
}

/****************************************************************** GET BRANCH DATA *******************************************************************/

void DTreeInterface::Get_Entry(Long64_t locEntry)
{
	if(dUpdateGetEntryBranchesFlag)
		Update_GetEntryBranches();

	//before getting arrays, must make sure that our current array size is large enough to hold them
	//loop through the branches that contain the needed array sizes
	set<string>::iterator locArraySizeIterator = dArraySizeBranches_GetEntryNeeded.begin();
	for(; locArraySizeIterator != dArraySizeBranches_GetEntryNeeded.end(); ++locArraySizeIterator)
	{
		//get the current array size
		string locArraySizeBranchName = *locArraySizeIterator;
		UInt_t locCurrentArraySize = dFundamentalArraySizeMap[locArraySizeBranchName];

		//get the new array size
		Get_Branch(locArraySizeBranchName)->GetEntry(locEntry);
		UInt_t locNewArraySize = Get_Fundamental<UInt_t>(locArraySizeBranchName);
//cout << "branch name, current array size, new array size = " << locArraySizeBranchName << ", " << locCurrentArraySize << ", " << locNewArraySize << endl;

		//compare them
		if(locNewArraySize <= locCurrentArraySize)
			continue; //new array size <= current array size: no need to expand

		//increase the array sizes for all branches corresponding to this array size
		const set<string>& locSetBranchNames = dArraySizeToBranchMap_GetEntryNeeded.find(locArraySizeBranchName)->second;
		set<string>::const_iterator locBranchIterator = locSetBranchNames.begin();
		for(; locBranchIterator != locSetBranchNames.end(); ++locBranchIterator)
		{
			string locBranchName = *locBranchIterator;
			string locBranchType = dFundamentalBranchTypeMap[locBranchName];
			Increase_ArraySize(locBranchName, locBranchType, locNewArraySize);
		}

		//save the new array size
		dFundamentalArraySizeMap[locArraySizeBranchName] = locNewArraySize;
	}

	//now, can loop through and get entries for all desired branches
	set<string>::iterator locNonArraySizeIterator = dNonArraySizeBranches_GetEntryNeeded.begin();
	for(; locNonArraySizeIterator != dNonArraySizeBranches_GetEntryNeeded.end(); ++locNonArraySizeIterator)
		Get_Branch(*locNonArraySizeIterator)->GetEntry(locEntry);
}

void DTreeInterface::Update_GetEntryBranches(void)
{
	//loop over them branches, separate by: is-array-size branch, and is-not
	set<string>::iterator locIterator = dGetEntryBranches.begin();
	for(; locIterator != dGetEntryBranches.end(); ++locIterator)
	{
		string locBranchName = *locIterator;
		if(dArraySizeToBranchMap.find(locBranchName) != dArraySizeToBranchMap.end())
		{
			//is an aray-size branch
			dArraySizeBranches_GetEntryNeeded.insert(locBranchName);
			continue;
		}

		//is not an array-size branch
		dNonArraySizeBranches_GetEntryNeeded.insert(locBranchName);

		//make sure that if it needs an array size, that branch is added too
		map<string, string>::iterator locArrayNameIterator = dBranchToArraySizeMap.find(locBranchName);
		if(locArrayNameIterator == dBranchToArraySizeMap.end())
			continue; //is not an array

		//is an array, add the array size branch
		string locArraySizeBranchName = locArrayNameIterator->second;
		dArraySizeBranches_GetEntryNeeded.insert(locArraySizeBranchName);
		dArraySizeToBranchMap_GetEntryNeeded[locArraySizeBranchName].insert(locBranchName);
	}

	dUpdateGetEntryBranchesFlag = false;
}

/************************************************************** READ GLUEX TTREE METADATA *************************************************************/

size_t DTreeInterface::Get_ComboInfo(map<int, map<int, pair<Particle_t, string> > >& locComboInfoMap) const
{
	//returns num steps
	//map key: step, particle indices, string: name
	TList* locUserInfo = Get_UserInfo();
	TMap* locNameToPIDMap = (TMap*)locUserInfo->FindObject("NameToPIDMap");

	//TMap of "StepIndex_ParticleIndex" (stored in TObjString) -> "UniqueParticleName" (TObjString)
		//ParticleIndex = -1 for initial, -2 for target, 0+ for final state
	TMap* locPositionToNameMap = (TMap*)locUserInfo->FindObject("PositionToNameMap");
	//locPositionToNameMap->Print();

	TMapIter* locMapIterator = new TMapIter(locPositionToNameMap);
	TObjString* locKeyObjString = (TObjString*)locMapIterator->Next();

	size_t locNumSteps = 0;
	while(locKeyObjString != NULL)
	{
		string locLocationString = locKeyObjString->GetName();
		size_t locUnderscoreIndex = locLocationString.find("_");

		//Get step index
		size_t locStepIndex;
		string locStepString = locLocationString.substr(0, locUnderscoreIndex);
		istringstream locStepIndexStream;
		locStepIndexStream.str(locStepString);
		locStepIndexStream >> locStepIndex;
		if(locStepIndex >= locNumSteps)
			locNumSteps = locStepIndex + 1;

		//Get particle index
		int locParticleIndex;
		string locParticleString = locLocationString.substr(locUnderscoreIndex + 1);
		istringstream locParticleIndexStream;
		locParticleIndexStream.str(locParticleString);
		locParticleIndexStream >> locParticleIndex;

		//Get particle name
		TObjString* locValueObjString = (TObjString*)locPositionToNameMap->GetValue(locKeyObjString);
		string locParticleName = locValueObjString->GetName();

		//Get PID
		TObjString* locPIDObjString = (TObjString*)locNameToPIDMap->GetValue(locParticleName.c_str());
		int locPDGPID;
		istringstream locPIDStream;
		locPIDStream.str(locPIDObjString->GetName());
		locPIDStream >> locPDGPID;
		Particle_t locPID = PDGtoPType(locPDGPID);

		//Store the result
		pair<Particle_t, string> locPIDPair(locPID, locParticleName);
		locComboInfoMap[locStepIndex][locParticleIndex] = locPIDPair;

		//Get next pair
		locKeyObjString = (TObjString*)locMapIterator->Next();
	}

	//For older trees, the target info was accidentally left out of the PositionToNameMap. Check for it separately:
	if(locComboInfoMap[0].find(-2) == locComboInfoMap[0].end())
	{
		TList* locParticleNameList = (TList*)locUserInfo->FindObject("ParticleNameList");
		if(locParticleNameList->FindObject("Target") != NULL)
		{
			//this problem was only in a very limited time-frame: only proton target used
			pair<Particle_t, string> locPIDPair(Proton, "Target");
			locComboInfoMap[0][-2] = locPIDPair;
		}
	}

	return locNumSteps;
}

Particle_t DTreeInterface::Get_TargetPID(void) const
{
	TMap* locMiscInfoMap = (TMap*)Get_UserInfo()->FindObject("MiscInfoMap");
	if(locMiscInfoMap->FindObject("Target__PID") == NULL)
		return Unknown;

	TObjString* locPIDObjString = (TObjString*)locMiscInfoMap->GetValue("Target__PID");
	int locPDGPID;
	istringstream locPIDStream;
	locPIDStream.str(locPIDObjString->GetName());
	locPIDStream >> locPDGPID;
	return PDGtoPType(locPDGPID);
}

TVector3 DTreeInterface::Get_TargetCenter(void) const
{
	TMap* locMiscInfoMap = (TMap*)Get_UserInfo()->FindObject("MiscInfoMap");
	TVector3 locTargetCenter;

	//TARGET X
	if(locMiscInfoMap->FindObject("Target__CenterX") != NULL)
	{
		TObjString* locObjString = (TObjString*)locMiscInfoMap->GetValue("Target__CenterX");
		double locTargetPosition;

		istringstream locTargetStream;
		locTargetStream.str(locObjString->GetName());
		locTargetStream >> locTargetPosition;

		locTargetCenter.SetX(locTargetPosition);
	}

	//TARGET Y
	if(locMiscInfoMap->FindObject("Target__CenterY") != NULL)
	{
		TObjString* locObjString = (TObjString*)locMiscInfoMap->GetValue("Target__CenterY");
		double locTargetPosition;

		istringstream locTargetStream;
		locTargetStream.str(locObjString->GetName());
		locTargetStream >> locTargetPosition;

		locTargetCenter.SetY(locTargetPosition);
	}

	//TARGET Z
	if(locMiscInfoMap->FindObject("Target__CenterZ") != NULL)
	{
		TObjString* locObjString = (TObjString*)locMiscInfoMap->GetValue("Target__CenterZ");
		double locTargetPosition;

		istringstream locTargetStream;
		locTargetStream.str(locObjString->GetName());
		locTargetStream >> locTargetPosition;

		locTargetCenter.SetZ(locTargetPosition);
	}

	return locTargetCenter;
}
