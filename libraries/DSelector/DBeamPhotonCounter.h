#ifndef _DBeamPhotonCounter_
#define _DBeamPhotonCounter_

#include <iostream>
#include <unordered_map>

using namespace std;

/*! \class DBeamPhotonCounter
 * This class provides the means to count combos in the DSelector for any given beam photon ID that
 * will pass all analyzer cuts. And then can be used in the second loop over those combos to modify
 * the combo weight according to the number of initially found FS with this beam photon ID.
 * This class also provides the choice of using the weighted chi2 rather than just the number of
 * combos as a weight modifyer. 
 */


class DBeamPhotonCounter {

 public:

  //DBeamPhotonCounter();
  //~DBeamPhotonCounter();

  int AddBunch(int ID, double Chi2); /// add combo chi2 for this beamID as 1./chi2
  int GetBunchNentries(int beamID);  /// get total number of combos for this beamID
  double GetBunchChi2Sum(int beamID);   /// get sum of all chi2 for this beamID
  void Init(); /// initialize (clear) all unordered_maps
  double GetWeightN(int ID); /// get the weight modifyer base on the number of Good combos
  double GetWeightChi2(int ID, double chi2); /// get the weight modifyer based on the normalized Chi2 of good combos.
  void PrintAll(); /// Print out all chi2 of all beamphoton-FS combos that passed all cuts separate for each beam Photon
  int Size(){ return Nentries;} /// return the overall size of the unsored map
  
 private:
  /// for each beam photon ID (int) hold a list of Chi2 from KinFit for 
  /// the combos that survive all analysis cuts.
  unordered_map<int, vector<double> > BeamBunchChi2List; ///< List of Chi2 for beam photon id
  unordered_map<int, double > BeamBunchChi2Sum; ///< Sum of Chi2 in the list for beam photon id
  int Nentries; /// Number of entries in map: 

};

#endif //_DBeamPhotonCounter_

void DBeamPhotonCounter::PrintAll(){
  
  for (auto x : BeamBunchChi2List){
    cout << x.first << "   " << BeamBunchChi2List.at(x.first).size(); 
    for (unsigned int k=0; k<BeamBunchChi2List.at(x.first).size(); k++) {
	  cout << "  "<< BeamBunchChi2List.at(x.first)[k];
    }
    cout<<endl;
  }
}

double DBeamPhotonCounter::GetWeightChi2(int ID, double chi2){

  /// return weight according to chi2 as weight = 1./chi2/ Sum(1./chi2_i)
  /// returns zero if vector of chi2 values is empty. this should never happend!
  /// Note that GetBunchChi2Sum will return Sum(1./chi2_i) the sum of all inverse chi2

  double S  = GetBunchChi2Sum(ID);
  if (S == 0.){
    return 0; 
  } else {
    double w = 1./chi2/S;
    return w;
  }

}

double DBeamPhotonCounter::GetWeightN(int ID){
  /// return weight factor according to the size of the vector of chi2s
  /// if the vector is empty the value zero is returned (this should never happend!)

  int N = GetBunchNentries(ID);
  if (N>0) {
    double w = 1./(double)N;
    return w;
  } else {
    return 0;
  }
}

void DBeamPhotonCounter::Init(){
  /// clear all entries in all unorderted maps in DBeamPhotonCounter class
  if (!BeamBunchChi2List.empty())
    BeamBunchChi2List.clear();
  if (!BeamBunchChi2Sum.empty())
    BeamBunchChi2Sum.clear();

  Nentries = 0;
}


int DBeamPhotonCounter::AddBunch(int ID, double Chi2){

  /// Add Chi2 of this combo to the list for the used beam photon ID
  /// First Test if this ID already has an existing List entry
  /// if not create a new list for this beam photon ID else
  /// add the Chi2 to the existing list.

  /// Note, it the value Chi2 is added to the list as 1./Chi2!!!
  /// therefore if Chi2 is zero and nothing will be done. A -1 is the return value!
  /// in this case.
  if (Chi2 == 0.){
    return -1;
  }

  int key = ID;
  //cout<<"Beam Bunch ID: "<<key<<endl;
  if (BeamBunchChi2List.find(key) == BeamBunchChi2List.end()){
    //cout << key << " not found\n\n";
    // This key is new initialize
    vector <double> V;
    V.push_back(1./Chi2);
    BeamBunchChi2List[key] = V;     
  } else {
    BeamBunchChi2List.at(key).push_back(1./Chi2);        
  }

  Nentries += 1;
  /// the return value is the new/current size of the List of Chi2s
  return (int) BeamBunchChi2List.at(key).size();

}

int DBeamPhotonCounter::GetBunchNentries(int ID){
  /// return the number of Chi2 entries in the list for this beam photon ID
  /// First test if a list exists, if not return zero else return the length 
  /// of the list 

  int key = ID;
  if (BeamBunchChi2List.find(key) == BeamBunchChi2List.end()){
    return 0;
  } else{
    return (int) BeamBunchChi2List.at(key).size();
  }
 
}

double DBeamPhotonCounter::GetBunchChi2Sum(int ID){

  /// Return the sum of all Chi2 in the list for this beam photons
  /// or zero if there is no list.

  /// Note, this method should only be used after having looped over all Combos
  /// of a given event and no more calls to method AddBunch() is necessary. 
  /// Othewise the result may be skewed.

  int key = ID;

  /// If the sum has not yet be determined calculate the sum now and then return 
  /// the value, otherwise just return its value. The Sum is stored in the second
  /// unsorted map with name BeamBunchChi2Sum.

  if (BeamBunchChi2Sum.find(key) == BeamBunchChi2Sum.end()){

    if (BeamBunchChi2List.find(key) == BeamBunchChi2List.end()){
      BeamBunchChi2Sum[key] = 0;
      return 0;
    } else {
      double Sum = 0;
      for (unsigned int k=0; k<BeamBunchChi2List.at(key).size(); k++){
	Sum += BeamBunchChi2List.at(key)[k];
      }
      BeamBunchChi2Sum[key] = Sum;
      return Sum;
    }

  } else {
    return BeamBunchChi2Sum[key];
  }

}

