// gluex skim tree files often contains one tree in it with unknown names
// this script serves to get the tree name 
// Author: Hao Li

void Get_treeName(TString skim_path){
  	TFile * f0 = TFile::Open(skim_path);
  	TIter nextkey( f0->GetListOfKeys() );
	TKey *key;
	TTree * Tree_data;
	while ( (key = (TKey*)nextkey()))
	{
		TObject *obj = key->ReadObj();
		if ( obj->IsA()->InheritsFrom( TTree::Class() ) ) {
			Tree_data = (TTree*)obj;
			break;
		}
	}
	TString tree_name = Tree_data->GetName();
	cout<< "treeName:" <<tree_name<<endl;
}

