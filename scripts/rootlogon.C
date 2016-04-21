{
	TStyle* dStyle = new TStyle("JLab_Style", "JLab_Style");
	dStyle->Reset("Modern");

	//Size & Margins
	dStyle->SetCanvasDefH(800);
	dStyle->SetCanvasDefW(1200);

	//Basic object fill colors
	dStyle->SetCanvasBorderMode(0);
	dStyle->SetLegendFillColor(0);
	dStyle->SetCanvasColor(0);

	//Stat box
	dStyle->SetOptFit(112);
	dStyle->SetOptStat(10); //entries only //mean/rms should be replaced with a fit

	//Titles and Labels
	dStyle->SetLabelSize(0.05, "xyz");
	dStyle->SetTitleSize(0.055, "xyz");
	dStyle->SetTitleOffset(1.1, "x");
	dStyle->SetTitleOffset(0.8, "y");

	//Margins (correlated with title/label size)
	dStyle->SetPadBottomMargin(0.15);

	//Default Histogram Style Settings
	dStyle->SetHistFillColor(kTeal);
	dStyle->SetHistFillStyle(1001); //solid fill (0 for hollow) //see http://root.cern.ch/root/html/TAttFill.html
	dStyle->SetHistLineColor(kBlack);
	dStyle->SetHistMinimumZero(kTRUE);

	//Default Function Style Settings
	dStyle->SetFuncColor(kBlack);
	dStyle->SetFuncStyle(1); //see http://root.cern.ch/root/html/TAttLine.html
	dStyle->SetFuncWidth(3);

	//Default Line Style Settings
	dStyle->SetLineColor(kBlack);
	dStyle->SetLineStyle(1); //see http://root.cern.ch/root/html/TAttLine.html
	dStyle->SetLineWidth(1); //is border of TLegend

	//Default Graph Style Settings
	dStyle->SetMarkerColor(kBlack);
	dStyle->SetMarkerSize(1); //see http://root.cern.ch/root/html/TAttMarker.html
	dStyle->SetMarkerStyle(22); //see http://root.cern.ch/root/html/TAttMarker.html

	//Grid
	dStyle->SetPadGridX(kTRUE);
	dStyle->SetPadGridY(kTRUE);

	//Histogram/Graph Title Size
	dStyle->SetTitleSize(0.075, "t"); //"t": could be anything that is not "x" "y" or "z"

	//Palette
	dStyle->SetNumberContours(50);
//#if ROOT_VERSION_CODE >= ROOT_VERSION(6,0,0)
//	dStyle->SetPalette(kBird); //57
//#else
	dStyle->SetPalette(55); //rainbow
//#endif

	dStyle->cd();
}

