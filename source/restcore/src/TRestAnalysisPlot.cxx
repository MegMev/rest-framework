///______________________________________________________________________________
///______________________________________________________________________________
///______________________________________________________________________________
///             
///
///             RESTSoft : Software for Rare Event Searches with TPCs
///
///             TRestAnalysisPlot.cxx
///
///             june 2016    Gloria Luzón/Javier Galan
///_______________________________________________________________________________


#include "TRestAnalysisPlot.h"
using namespace std;

#include <TStyle.h>


const int debug = 0;

ClassImp(TRestAnalysisPlot)
    //______________________________________________________________________________
TRestAnalysisPlot::TRestAnalysisPlot()
{
    Initialize();

}

TRestAnalysisPlot::TRestAnalysisPlot( const char *cfgFileName, const char *name) : TRestMetadata (cfgFileName)
{
    Initialize();

    LoadConfigFromFile( fConfigFileName, name );

}

void TRestAnalysisPlot::Initialize()
{
    SetSectionName( this->ClassName() );

    fRun = NULL;

    fNFiles = 0;

    fCombinedCanvas = NULL;

    fStartTime = 0;
    fEndTime = 0;
}


//______________________________________________________________________________
TRestAnalysisPlot::~TRestAnalysisPlot()
{
    if( fRun != NULL ) delete fRun;
}

//______________________________________________________________________________
void TRestAnalysisPlot::InitFromConfigFile()
{
    size_t position = 0;
    string addFileString;
    while( ( addFileString = GetKEYDefinition( "addFile", position ) ) != "" )
    {
        TString inputfile = GetFieldValue( "name", addFileString );

        this->AddFile( inputfile );
    }

    fPlotMode = GetParameter( "plotMode", "compare" );
    fHistoOutputFile = GetParameter( "histoFilename", "/tmp/histos.root" );

    position = 0;
    string canvasDefinition;
    if( ( canvasDefinition = GetKEYDefinition( "canvas", position ) ) != "" )
    {
        fCanvasSize = StringTo2DVector ( GetFieldValue( "size", canvasDefinition ) );
        fCanvasDivisions = StringTo2DVector ( GetFieldValue( "divide", canvasDefinition ) );
        fCanvasSave = GetFieldValue( "save", canvasDefinition );
    }

    vector <TString> globalCuts;

    position = 0;
    string globalCutString;
    while( ( globalCutString = GetKEYDefinition( "globalCut", position ) ) != "" ) //general cuts 
    {  
        TString cutActive = GetFieldValue( "value", globalCutString );

        if( cutActive == "on" || cutActive == "ON" || cutActive == "On" || cutActive == "oN" )
        {
            TString obsName = GetFieldValue( "name", globalCutString );

            TString cutCondition = GetFieldValue( "condition", globalCutString );
            TString cutString = obsName + cutCondition;

            globalCuts.push_back( cutString );
        }
    }

    position = 0;
    string addPlotString;
    while( ( addPlotString = GetKEYStructure( "plot", position ) ) != "NotFound" ) //general cuts 
    {
        TString plotActive = GetFieldValue( "value", addPlotString );

        if( plotActive == "on" || plotActive == "ON" || plotActive == "On" || plotActive == "oN" )
        {     
            TString plotName = RemoveWhiteSpaces( GetFieldValue( "name", addPlotString ) );
            fPlotNames.push_back( plotName );

            TString saveName = RemoveWhiteSpaces( GetFieldValue( "save", addPlotString ) );
            fPlotSaveToFile.push_back( saveName );

            TString logScale = GetFieldValue( "logscale", addPlotString );

            if( logScale == "true" )
                fLogScale.push_back( true );
            else
                fLogScale.push_back( false );

            TString xLabel = GetFieldValue( "xlabel", addPlotString );
            fPlotXLabel.push_back( xLabel );

            TString yLabel = GetFieldValue( "ylabel", addPlotString );
            fPlotYLabel.push_back( yLabel );

            TString title = GetFieldValue( "title", addPlotString );
            fPlotTitle.push_back( title );

            TString option = RemoveWhiteSpaces( GetFieldValue( "option", addPlotString ) );
	    if( option == "Notdefined" )
		option = "colz";
            fPlotOption.push_back( option );

            // scale to be implemented

            vector <TString> varNames;
            vector <TVector2> ranges;
            vector <Int_t> bins;

            string variableDefinition;
            size_t pos = 0;
            while( (variableDefinition = GetKEYDefinition( "variable", pos, addPlotString ) ) != "" )
            {            

                varNames.push_back( GetFieldValue( "name", (string) variableDefinition ) );
                ranges.push_back( StringTo2DVector( GetFieldValue( "range", variableDefinition ) ) );
                bins.push_back( StringToInteger( GetFieldValue( "nbins", variableDefinition ) ) );
            }

            TString pltString = "";
            for( unsigned int i = 0; i < varNames.size(); i++ )
            {
                pltString += varNames[i]; 
                if( i < varNames.size()-1 ) pltString += ":";
            }

	    if( GetVerboseLevel() >= REST_Debug )
	    {
		for( unsigned int n = 0; n < bins.size(); n++ )
		{
			cout << "Variable " << varNames[n] << " range/bins " << endl;
			cout << "------------------------------------------" << endl;
			cout << "rX : " << ranges.back().X() << " rY : " << ranges.back().Y() << endl;
			cout << "bins : " << bins.back() << endl;
			cout << endl;
		}
            }

	    pltString += " >>" + plotName;

	    // The range definitions are in reversed ordered. Compared to ROOT variable definitions
	    for( int i = ((int) bins.size()) - 1; i >= 0; i-- )
	    {

		    TString binsStr;
		    binsStr.Form( "%d", bins[i] );
		    if( bins[i] == -1 )
			binsStr = " ";

		    TString rXStr;
		    rXStr.Form( "%f", ranges[i].X() );
		    if( ranges[i].X() == -1 )
			rXStr = " ";

		    TString rYStr;
		    rYStr.Form( "%f", ranges[i].Y() );
		    if( ranges[i].Y() == -1 )
			rYStr = " ";

		    if( i == (int) bins.size()-1 ) pltString += "(";

		    pltString += binsStr + " , " + rXStr + " , " + rYStr;
		    if( i > 0 ) pltString += ",";
		    if( i == 0 ) pltString += ")";
	    }

            fPlotString.push_back ( pltString );

            pos = 0;
            string addCutString;

	    if( GetVerboseLevel() >= REST_Debug )
	    {
		cout << endl;
		cout << "Plot string : " << pltString << endl;
            }

            TString cutString = "";
            Int_t n = 0;
            while( ( addCutString = GetKEYDefinition( "cut", pos, addPlotString ) ) != "" )
            {
                TString cutActive = GetFieldValue( "value", addCutString );

                if( cutActive == "on" || cutActive == "ON" || cutActive == "On" || cutActive == "oN" )
                {     
                    TString cutVariable = GetFieldValue( "variable", addCutString );
                    TString cutCondition = GetFieldValue( "condition", addCutString );

                    if( n > 0 ) cutString += " && ";

		    if( GetVerboseLevel() >= REST_Debug )
			    cout << "Adding local cut : " << cutVariable << cutCondition << endl;

                    cutString += cutVariable + cutCondition;
                    n++;
                }
            }

            for( unsigned int i = 0; i < globalCuts.size(); i++ )
            {
                if( i > 0 || cutString != "" ) cutString += " && ";
	        if( GetVerboseLevel() >= REST_Debug )
			cout << "Adding global cut : " << globalCuts[i] << endl;
                cutString += globalCuts[i];
            }

            fCutString.push_back( cutString );

	    if( GetVerboseLevel() >= REST_Debug )
	    {
		cout << "-------------------------------" << endl;
            }
        }
    }

    Int_t maxPlots = (Int_t) fCanvasDivisions.X() * (Int_t) fCanvasDivisions.Y();

    Int_t nPlots = (Int_t) fPlotString.size();

    if( nPlots > maxPlots ) 
    {
        cout << "REST ERROR: Your canvas divisions (" << fCanvasDivisions.X() << " , " << fCanvasDivisions.Y();
        cout << ") are not enough to show " << nPlots << " plots" << endl;
        exit(1);
    }
}

Int_t TRestAnalysisPlot::GetPlotIndex( TString plotName )
{

    for( unsigned int n = 0; n < fPlotNames.size(); n++ )
        if( fPlotNames[n] == plotName ) return n;

    cout << "REST WARNING : GetPlotIndex. Plot name " << plotName << " not found" << endl;
    return -1;
}

void TRestAnalysisPlot::PlotCombinedCanvas( )
{
    // We should check that all the variables defined in the plots exist in our files
    // TOBE implemented and return a REST error in case
    if( fPlotMode == "compare" )
    {
        PlotCombinedCanvasCompare();
        return;
    }

    if( fPlotMode == "add" )
    {
        PlotCombinedCanvasAdd();
        return;
    }

    cout << "REST WARNING: TRestAnalysisPlot::PlotCombinedCanvas(). Plot mode (" << fPlotMode << ") not found" << endl;

}

void TRestAnalysisPlot::PlotCombinedCanvasAdd( )
{

    vector <TRestRun *> runs;
    vector <TRestAnalysisTree *> trees;

    TRestRun *r;
    TRestAnalysisTree *anT;
    for( int n = 0; n < fNFiles; n++ )
    {
        r = new TRestRun();
        runs.push_back( r );
        r->OpenInputFile( fFileNames[n] );
        anT = r->GetAnalysisTree();
        trees.push_back( anT );

	r->SkipEventTree();

	r->GetEntry(0);
	if( fStartTime == 0 || anT->GetTimeStamp() < fStartTime ) fStartTime = anT->GetTimeStamp();

	r->GetEntry( r->GetEntries() - 1);
	if( fEndTime == 0 || anT->GetTimeStamp() > fEndTime ) fEndTime = anT->GetTimeStamp();
    }

    fCanvasSave = ReplaceFilenameTags( fCanvasSave, runs[0] );

    fHistoOutputFile = ReplaceFilenameTags( fHistoOutputFile, runs[0] );
    TFile *f = new TFile( fHistoOutputFile, "RECREATE");

    cout << "Saving histograms to ROOT file : " << fHistoOutputFile << endl;

    if( fCombinedCanvas != NULL ) 
    {
        delete fCombinedCanvas;
        fCombinedCanvas = NULL;
    }

    fCombinedCanvas = new TCanvas( "combined", "combined", 0, 0, fCanvasSize.X(), fCanvasSize.Y() );

    fCombinedCanvas->Divide( (Int_t) fCanvasDivisions.X(), (Int_t) fCanvasDivisions.Y() );

    TStyle *st = new TStyle();
    st->SetPalette(1);

    for( unsigned int n = 0; n < fPlotString.size(); n++ )
    {
        fCombinedCanvas->cd(n+1);
        if( fLogScale[n] ) 
            fCombinedCanvas->cd(n+1)->SetLogy();
        
        for( int m = 0; m < fNFiles; m++ )
        {

            TString plotString = fPlotString[n];

            if( m > 0 )
            {
                plotString = Replace( (string) fPlotString[n], ">>", ">>+", 0 );
                plotString = plotString( 0, fPlotString[n].First(">>+") + 3 ) + fPlotNames[n];
            }

	    if( GetVerboseLevel() >= REST_Debug )
	    {
		cout << endl;
		cout << "Plot name : " << fPlotNames[n] << endl;
		cout << "Plot string : " << plotString << endl;
		cout << "Cut string : " << fCutString[n] << endl;
		cout << "--------------------------------------" << endl;

	    }
            trees[m]->Draw( plotString, fCutString[n], fPlotOption[n] );
        }

        TH3F *htemp = (TH3F*)gPad->GetPrimitive( fPlotNames[n] );
        htemp->SetTitle( fPlotTitle[n] );
        htemp->GetXaxis()->SetTitle( fPlotXLabel[n] );
        htemp->GetYaxis()->SetTitle( fPlotYLabel[n] );

/*
	if( fPlotXLabel[n].Contains("Time") ||  fPlotXLabel[n].Contains("time") )
	{
		cout.precision(12);
		cout << "Start : " << fStartTime << endl;
		cout << "End : " << fEndTime << endl;
		htemp->GetXaxis()->SetRangeUser( fStartTime, fEndTime );
		htemp->GetXaxis()->SetLimits( fStartTime, fEndTime );
		htemp->SetAxisRange( fStartTime, fEndTime,"X");
		GetChar();
	}
*/


        f->cd();
        htemp->Write( fPlotNames[n] );

        if( fPlotSaveToFile[n] != "Notdefined" && fPlotSaveToFile[n] != "" )
            SavePlotToPDF( fPlotNames[n], fPlotSaveToFile[n] );
        fCombinedCanvas->Update();
    }

    if( fCanvasSave != "" )
        fCombinedCanvas->Print( fCanvasSave );

    f->Close();
}

void TRestAnalysisPlot::PlotCombinedCanvasCompare( )
{
    vector <TRestRun *> runs;
    vector <TRestAnalysisTree *> trees;

    TRestRun *r;
    TRestAnalysisTree *anT;
    for( int n = 0; n < fNFiles; n++ )
    {
        r = new TRestRun();
        runs.push_back( r );
        r->OpenInputFile( fFileNames[n] );
        anT = r->GetAnalysisTree();
        trees.push_back( anT );
    }

    fCanvasSave = ReplaceFilenameTags( fCanvasSave, runs[0] );

    if( fCombinedCanvas != NULL ) 
    {
        delete fCombinedCanvas;
        fCombinedCanvas = NULL;
    }

    fCombinedCanvas = new TCanvas( "combined", "combined", 0, 0, fCanvasSize.X(), fCanvasSize.Y() );

    fCombinedCanvas->Divide( (Int_t) fCanvasDivisions.X(), (Int_t) fCanvasDivisions.Y() );

    for( unsigned int n = 0; n < fPlotString.size(); n++ )
    {
        fCombinedCanvas->cd(n+1);
        if( fLogScale[n] ) 
            fCombinedCanvas->cd(n+1)->SetLogy();
        
        for( int m = 0; m < fNFiles; m++ )
        {

            TString plotString = fPlotString[n];

            if( m > 0 )
                plotString = fPlotString[n]( 0, fPlotString[n].First(">>") );

            trees[m]->SetLineColor(1+m);
            if( m == 0 )
                trees[m]->Draw( plotString, fCutString[n], "" );
            else 
                trees[m]->Draw( plotString, fCutString[n], "same" );
        }

        TH3F *htemp = (TH3F*)gPad->GetPrimitive( fPlotNames[n] );
        htemp->SetTitle( fPlotTitle[n] );
        htemp->GetXaxis()->SetTitle( fPlotXLabel[n] );
        htemp->GetYaxis()->SetTitle( fPlotYLabel[n] );

        if( fPlotSaveToFile[n] != "Notdefined" && fPlotSaveToFile[n] != "" )
            SavePlotToPDF( fPlotNames[n], fPlotSaveToFile[n] );
        fCombinedCanvas->Update();
    }

    if( fCanvasSave != "" )
        fCombinedCanvas->Print( fCanvasSave );
}

void TRestAnalysisPlot::SavePlotToPDF( TString plotName, TString fileName )
{
    Int_t index = GetPlotIndex( plotName );
    if ( index >= 0 )
        SavePlotToPDF( index, fileName );
    else
        cout << "Save to plot failed. Plot name " << plotName << " not found" << endl;

}

void TRestAnalysisPlot::SavePlotToPDF( Int_t n, TString fileName )
{
    gErrorIgnoreLevel=10;

    TCanvas *c = new TCanvas( fPlotNames[n], fPlotNames[n], 800, 600 );

    TRestRun *run = new TRestRun();
    run->OpenInputFile( fFileNames[0] );

    TRestAnalysisTree *anTree = run->GetAnalysisTree();
    anTree->Draw( fPlotString[n], fCutString[n], "", anTree->GetEntries(), 0 );

    TH3F *htemp = (TH3F*)gPad->GetPrimitive( fPlotNames[n] );
    htemp->SetTitle( fPlotTitle[n] );
    htemp->GetXaxis()->SetTitle( fPlotXLabel[n] );
    htemp->GetYaxis()->SetTitle( fPlotYLabel[n] );

    c->Print( fileName );

    delete c;
}

TString TRestAnalysisPlot::ReplaceFilenameTags( TString filename, TRestRun *run )
{
    return run->ConstructFilename( filename );
}

