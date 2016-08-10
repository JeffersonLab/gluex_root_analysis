#ifndef _DAnalysisUtilities_
#define _DAnalysisUtilities_

#include <deque>
#include <set>

#include "DKinematicData.h"
#include "DParticleCombo.h"

using namespace std;

class DAnalysisUtilities
{
	public:
		TLorentzVector Calc_MissingP4(const DParticleCombo* locParticleComboWrapper, bool locUseKinFitDataFlag) const;
		TLorentzVector Calc_MissingP4(const DParticleCombo* locParticleComboWrapper, map<Particle_t, set<Int_t> >& locSourceObjects, bool locUseKinFitDataFlag) const;
		TLorentzVector Calc_MissingP4(const DParticleCombo* locParticleComboWrapper, size_t locStepIndex, int locUpToStepIndex, set<size_t> locUpThroughIndices, bool locUseKinFitDataFlag) const;
		TLorentzVector Calc_MissingP4(const DParticleCombo* locParticleComboWrapper, size_t locStepIndex, int locUpToStepIndex, set<size_t> locUpThroughIndices, map<Particle_t, set<Int_t> >& locSourceObjects, bool locUseKinFitDataFlag) const;
		TLorentzVector Calc_FinalStateP4(const DParticleCombo* locParticleComboWrapper, size_t locStepIndex, bool locUseKinFitDataFlag) const;
		TLorentzVector Calc_FinalStateP4(const DParticleCombo* locParticleComboWrapper, size_t locStepIndex, map<Particle_t, set<Int_t> >& locSourceObjects, bool locUseKinFitDataFlag) const;
		TLorentzVector Calc_FinalStateP4(const DParticleCombo* locParticleComboWrapper, size_t locStepIndex, set<size_t> locToIncludeIndices, bool locUseKinFitDataFlag) const;
		TLorentzVector Calc_FinalStateP4(const DParticleCombo* locParticleComboWrapper, size_t locStepIndex, set<size_t> locToIncludeIndices, map<Particle_t, set<Int_t> >& locSourceObjects, bool locUseKinFitDataFlag) const;

		set<set<size_t> > Build_IndexCombos(const DParticleComboStep* locParticleComboStepWrapper, deque<Particle_t> locToIncludePIDs) const;

	private:

		bool Handle_Decursion(int& locParticleIndex, deque<size_t>& locComboDeque, deque<int>& locResumeAtIndices, deque<deque<size_t> >& locPossibilities) const;
};

#endif // _DAnalysisUtilities_
