#include "DTreeInterface.h"

/**************************************************************** SETUP INPUT BRANCHES ****************************************************************/

void DTreeInterface::Set_BranchAddresses(void)
{
	//ASSUME: One leaf per branch: No splitting

	//Loop over branches
	TObjArray* locBranchArray = dTree->GetListOfBranches();
	for(Int_t loc_i = 0; loc_i < locBranchArray->GetEntriesFast(); ++loc_i)
	{
		//Get branch
		TBranch* locBranch = dynamic_cast<TBranch*>(locBranchArray->At(loc_i));
		string locBranchName = locBranch->GetName();

		//Set addresses based on type
		string locClassName = locBranch->GetClassName(); //is "" for fundamental types, TLorentzVector, TClonesArray (not subtype!)
		if(locClassName == "") //Is fundamental type
			Set_FundamentalBranchAddress(locBranchName, locBranch->GetTitle());
		else if(locClassName == "TClonesArray")
			Set_ClonesArrayBranchAddress(locBranchName);
		else //TObject
			Set_TObjectBranchAddress(locBranchName, locClassName);

		//Register branches
		dBranchMap[locBranchName] = locBranch;
		dInputBranches.insert(locBranchName);
	}

	dGetEntryBranches = dInputBranches;
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

void DTreeInterface::Get_Entry(Long64_t locEntry)
{
	if(dGetEntryBranchesModifiedFlag)
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

	dGetEntryBranchesModifiedFlag = false;
}

size_t DTreeInterface::Get_ComboInfo(map<int, map<int, pair<Particle_t, string> > >& locComboInfoMap) const
{
	//returns num steps
	//map key: step, particle indices, string: name
	TList* locUserInfo = dTree->GetUserInfo();
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

	return locNumSteps;
}
