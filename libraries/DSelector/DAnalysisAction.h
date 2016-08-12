#ifndef _DAnalysisAction_
#define _DAnalysisAction_

#include <string>

#include "TDirectoryFile.h"

#include "DParticleCombo.h"

using namespace std;

class DAnalysisAction
{
	public:
		DAnalysisAction(const DParticleCombo* locParticleComboWrapper, string locActionBaseName, bool locUseKinFitFlag = false, string locActionUniqueString = "");

		virtual string Get_ActionName(void) const{return dActionName;}

		virtual void Initialize(void) = 0;
		virtual void Reset_NewEvent(void) = 0;
		virtual bool Perform_Action(void) = 0;

	protected:

		const DParticleCombo* dParticleComboWrapper;
		bool dUseKinFitFlag;
		string dActionName;

		TDirectoryFile* CreateAndChangeTo_ActionDirectory(void) const; //get the directory this action should write ROOT objects to.
		TDirectoryFile* ChangeTo_BaseDirectory(void) const; //get and change to the base (file/global) directory
		TDirectoryFile* CreateAndChangeTo_Directory(string locDirName) const;
		TDirectoryFile* CreateAndChangeTo_Directory(string locBaseDirectoryPath, string locDirName) const;
		TDirectoryFile* CreateAndChangeTo_Directory(TDirectoryFile* locBaseDirectory, string locDirName) const;
};

inline DAnalysisAction::DAnalysisAction(const DParticleCombo* locParticleComboWrapper, string locActionBaseName, bool locUseKinFitFlag, string locActionUniqueString) :
dParticleComboWrapper(locParticleComboWrapper), dUseKinFitFlag(locUseKinFitFlag)
{
	dActionName = locActionBaseName;
	if(locActionUniqueString != "")
		dActionName += string("_") + locActionUniqueString;
}

inline TDirectoryFile* DAnalysisAction::CreateAndChangeTo_ActionDirectory(void) const
{
	//get the directory this action should write ROOT objects to.
	return CreateAndChangeTo_Directory("/", dActionName);
}

inline TDirectoryFile* DAnalysisAction::ChangeTo_BaseDirectory(void) const
{
	//get and change to the base (file/global) directory
	return (TDirectoryFile*)gDirectory->cd("/");
}

inline TDirectoryFile* DAnalysisAction::CreateAndChangeTo_Directory(string locDirName) const
{
	return CreateAndChangeTo_Directory((TDirectoryFile*)gDirectory, locDirName);
}

inline TDirectoryFile* DAnalysisAction::CreateAndChangeTo_Directory(string locBaseDirectoryPath, string locDirName) const
{
	TDirectoryFile* locBaseDirectory = static_cast<TDirectoryFile*>(gDirectory->GetDirectory(locBaseDirectoryPath.c_str()));
	return CreateAndChangeTo_Directory(locBaseDirectory, locDirName);
}

inline TDirectoryFile* DAnalysisAction::CreateAndChangeTo_Directory(TDirectoryFile* locBaseDirectory, string locDirName) const
{
	locBaseDirectory->cd();
	TDirectoryFile* locSubDirectory = static_cast<TDirectoryFile*>(locBaseDirectory->Get(locDirName.c_str()));
	if(locSubDirectory == NULL) //else folder already created by a different thread
		locSubDirectory = new TDirectoryFile(locDirName.c_str(), locDirName.c_str());
	locSubDirectory->cd();
	return locSubDirectory;
}

#endif // _DAnalysisAction_
