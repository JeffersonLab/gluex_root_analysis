#include "DKinematicData.h"

using namespace std;

void DKinematicData::Setup_Branches(void)
{
	string locBranchName;

	//Initialize to nullptr
	dBranch_MeasuredIndex = nullptr;
	dBranch_PID = nullptr;
	dP4_KinFit = nullptr;
	dP4_Measured = nullptr;
	dX4_KinFit = nullptr;
	dX4_Measured = nullptr;

	//PID
	if(dPID == Unknown)
	{
		locBranchName = dBranchNamePrefix + string("__PID");
		dBranch_PID = dTreeInterface->Get_Branch(locBranchName);
	}

	//If thrown, p4 & x4
	if((dBranchNamePrefix == "Thrown") || (dBranchNamePrefix == "ThrownBeam"))
	{
		//P4_Measured
		locBranchName = dBranchNamePrefix + string("__P4");
		dP4_Measured = dTreeInterface->Get_Pointer_TClonesArray(locBranchName);

		//X4_Measured
		locBranchName = dBranchNamePrefix + string("__X4");
		dX4_Measured = dTreeInterface->Get_Pointer_TClonesArray(locBranchName);

		return;
	}

	//If non-combo particle, p4 & x4
	bool locIsNonComboParticle = !Get_IsComboParticle();
	if(locIsNonComboParticle)
	{
		//P4_Measured
		locBranchName = dBranchNamePrefix + string("__P4_Measured");
		dP4_Measured = dTreeInterface->Get_Pointer_TClonesArray(locBranchName);

		//X4_Measured
		locBranchName = dBranchNamePrefix + string("__X4_Measured");
		dX4_Measured = dTreeInterface->Get_Pointer_TClonesArray(locBranchName);

		return;
	}

	//Now, is combo particle
	//P4_KinFit
	locBranchName = dBranchNamePrefix + string("__P4_KinFit");
	dP4_KinFit = dTreeInterface->Get_Pointer_TClonesArray(locBranchName);

	//X4_KinFit
	locBranchName = dBranchNamePrefix + string("__X4_KinFit");
	dX4_KinFit = dTreeInterface->Get_Pointer_TClonesArray(locBranchName);

	//if decaying/missing, return
	if((dBranchNamePrefix.substr(0, 8) == "Decaying") || (dBranchNamePrefix.substr(0, 7) == "Missing"))
		return; //are done

	//Now, is detected/target combo particle

	//measured index & p4/x4 measured
	if(dBranchNamePrefix.substr(0, 6) == "Target") //target
	{
		//Fixed P4
		dFixedP4 = TLorentzVector(TVector3(), ParticleMass(dPID));

		//Measured index //For X4_Measured
		locBranchName =  "ComboBeam__BeamIndex";
		dBranch_MeasuredIndex = dTreeInterface->Get_Branch(locBranchName);

		//X4_Measured
		locBranchName = "Beam__X4_Measured";
		dX4_Measured = dTreeInterface->Get_Pointer_TClonesArray(locBranchName);

		//X4_KinFit
		locBranchName = "ComboBeam__X4_KinFit";
		dX4_KinFit = dTreeInterface->Get_Pointer_TClonesArray(locBranchName);
	}
	else if(dBranchNamePrefix == "ComboBeam") //beam
	{
		//Measured Branch Name
		dMeasuredBranchNamePrefix = "Beam";

		//Measured index
		locBranchName = dBranchNamePrefix + string("__BeamIndex");
		dBranch_MeasuredIndex = dTreeInterface->Get_Branch(locBranchName);

		//P4_Measured
		locBranchName = dMeasuredBranchNamePrefix + string("__P4_Measured");
		dP4_Measured = dTreeInterface->Get_Pointer_TClonesArray(locBranchName);

		//X4_Measured
		locBranchName = dMeasuredBranchNamePrefix + string("__X4_Measured");
		dX4_Measured = dTreeInterface->Get_Pointer_TClonesArray(locBranchName);
	}
	else if(ParticleCharge(dPID) == 0) //neutral: get from combo object
	{
		//Measured Branch Name
		dMeasuredBranchNamePrefix = "NeutralHypo";

		//Measured index
		locBranchName = dBranchNamePrefix + string("__NeutralIndex");
		dBranch_MeasuredIndex = dTreeInterface->Get_Branch(locBranchName);

		//P4_Measured
		locBranchName = dBranchNamePrefix + string("__P4_Measured");
		dP4_Measured = dTreeInterface->Get_Pointer_TClonesArray(locBranchName);

		//X4_Measured
		locBranchName = dBranchNamePrefix + string("__X4_Measured");
		dX4_Measured = dTreeInterface->Get_Pointer_TClonesArray(locBranchName);
	}
	else //charged: get from measured object
	{
		//Measured Branch Name
		dMeasuredBranchNamePrefix = "ChargedHypo";

		//Measured index
		locBranchName = dBranchNamePrefix + string("__ChargedIndex");
		dBranch_MeasuredIndex = dTreeInterface->Get_Branch(locBranchName);

		//P4_Measured
		locBranchName = dMeasuredBranchNamePrefix + string("__P4_Measured");
		dP4_Measured = dTreeInterface->Get_Pointer_TClonesArray(locBranchName);

		//X4_Measured
		locBranchName = dMeasuredBranchNamePrefix + string("__X4_Measured");
		dX4_Measured = dTreeInterface->Get_Pointer_TClonesArray(locBranchName);
	}
}
