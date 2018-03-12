///______________________________________________________________________________
///______________________________________________________________________________
///             
///
///             RESTSoft : Software for Rare Event Searches with TPCs
///
///             TRestHitsAnalysisProcess.cxx
///
///
///             First implementation of hits analysis process into REST_v2
///             Date : may/2016
///             Author : J. Galan
///
///_______________________________________________________________________________

#include "TRestHitsAnalysisProcess.h"
using namespace std;

ClassImp(TRestHitsAnalysisProcess)
    //______________________________________________________________________________
TRestHitsAnalysisProcess::TRestHitsAnalysisProcess()
{
    Initialize();
}

//______________________________________________________________________________
TRestHitsAnalysisProcess::TRestHitsAnalysisProcess( char *cfgFileName )
{
    Initialize();

    if( LoadConfigFromFile( cfgFileName ) == -1 ) LoadDefaultConfig( );
}

//______________________________________________________________________________
TRestHitsAnalysisProcess::~TRestHitsAnalysisProcess()
{
    delete fInputHitsEvent;
    delete fOutputHitsEvent;
}

void TRestHitsAnalysisProcess::LoadDefaultConfig()
{
    SetTitle( "Default config" );
}

//______________________________________________________________________________
void TRestHitsAnalysisProcess::Initialize()
{
    SetSectionName( this->ClassName() );

    fInputHitsEvent = new TRestHitsEvent();
    fOutputHitsEvent = new TRestHitsEvent();

    fOutputEvent = fOutputHitsEvent;
    fInputEvent = fInputHitsEvent;

    fPrismFiducial = false;
    fCylinderFiducial = false;
}

void TRestHitsAnalysisProcess::LoadConfig( std::string cfgFilename, std::string name )
{
    if( LoadConfigFromFile( cfgFilename, name ) == -1 ) LoadDefaultConfig( );
}

//______________________________________________________________________________
void TRestHitsAnalysisProcess::InitProcess()
{
    TRestEventProcess::ReadObservables();
}

//______________________________________________________________________________
void TRestHitsAnalysisProcess::BeginOfEventProcess() 
{
    fOutputHitsEvent->Initialize();
}

//______________________________________________________________________________
TRestEvent* TRestHitsAnalysisProcess::ProcessEvent( TRestEvent *evInput )
{
    fInputHitsEvent = (TRestHitsEvent *) evInput;
   
    TString obsName;

	TRestHits *hits = fInputHitsEvent->GetHits();
    for( int n = 0; n < hits->GetNumberOfHits(); n++ )
    {
        Double_t eDep = hits->GetEnergy(n);

        Double_t x = hits->GetX(n);
        Double_t y = hits->GetY(n);
        Double_t z = hits->GetZ(n);

        fOutputHitsEvent->AddHit( x, y, z, eDep );
    }

    if( fOutputHitsEvent->GetNumberOfHits() == 0 ) return NULL;


    Double_t energy = fOutputHitsEvent->GetEnergy( );
    TVector3 meanPosition = fOutputHitsEvent->GetMeanPosition();

    Int_t nHits = fOutputHitsEvent->GetNumberOfHits( );
    Int_t nHitsX = fOutputHitsEvent->GetNumberOfHitsX( );
    Int_t nHitsY = fOutputHitsEvent->GetNumberOfHitsY( );

    obsName = this->GetName() + (TString) ".nHits";
    fAnalysisTree->SetObservableValue( obsName, nHits );

    obsName = this->GetName() + (TString) ".nHitsX";
    fAnalysisTree->SetObservableValue( obsName, nHitsX );

    obsName = this->GetName() + (TString) ".nHitsY";
    fAnalysisTree->SetObservableValue( obsName, nHitsY );


    // Checking hits inside fiducial cylinder
    if( fCylinderFiducial )
    {
        Int_t isInsideCylinder = 0;
        if( fOutputHitsEvent->isHitsEventInsideCylinder( fFid_x0, fFid_x1, fFid_R ) )
            isInsideCylinder = 1;

        Int_t nCylVol = fOutputHitsEvent->GetNumberOfHitsInsideCylinder( fFid_x0, fFid_x1, fFid_R );

        Double_t enCylVol = fOutputHitsEvent->GetEnergyInCylinder( fFid_x0, fFid_x1, fFid_R );

        obsName = this->GetName() + (TString) ".isInsideCylindricalVolume";
        fAnalysisTree->SetObservableValue( obsName, isInsideCylinder );

        obsName = this->GetName() + (TString) ".nInsideCylindricalVolume";
        fAnalysisTree->SetObservableValue( obsName, nCylVol );

        obsName = this->GetName() + (TString) ".energyInsideCylindricalVolume";
        fAnalysisTree->SetObservableValue( obsName, enCylVol );
    }

    // Checking hits inside fiducial prism 
    if( fPrismFiducial )
    {
        Int_t isInsidePrism = 0;
        if ( fOutputHitsEvent->isHitsEventInsidePrism( fFid_x0,  fFid_x1, fFid_sX, fFid_sY ) )
            isInsidePrism = 1;

        Int_t nPrismVol = fOutputHitsEvent->GetNumberOfHitsInsidePrism( fFid_x0,  fFid_x1, fFid_sX, fFid_sY );

        Double_t enPrismVol = fOutputHitsEvent->GetEnergyInPrism( fFid_x0,  fFid_x1, fFid_sX, fFid_sY );

        obsName = this->GetName() + (TString) ".isInsidePrismVolume";
        fAnalysisTree->SetObservableValue( obsName, isInsidePrism );

        obsName = this->GetName() + (TString) ".nInsidePrismVolume";
        fAnalysisTree->SetObservableValue( obsName, nPrismVol );

        obsName = this->GetName() + (TString) ".energyInsidePrismVolume";
        fAnalysisTree->SetObservableValue( obsName, enPrismVol );
    }

    ///////////////////////////////////////

    if( fCylinderFiducial )
    {
        // Adding distances to cylinder wall
        Double_t dToCylWall = fOutputHitsEvent->GetClosestHitInsideDistanceToCylinderWall( fFid_x0, fFid_x1, fFid_R );
        Double_t dToCylTop  = fOutputHitsEvent->GetClosestHitInsideDistanceToCylinderTop( fFid_x0, fFid_x1, fFid_R );
        Double_t dToCylBottom = fOutputHitsEvent->GetClosestHitInsideDistanceToCylinderBottom( fFid_x0, fFid_x1, fFid_R );

        obsName = this->GetName() + (TString) ".distanceToCylinderWall";
        fAnalysisTree->SetObservableValue( obsName, dToCylWall );
        obsName = this->GetName() + (TString) ".distanceToCylinderTop";
        fAnalysisTree->SetObservableValue( obsName, dToCylTop );
        obsName = this->GetName() + (TString) ".distanceToCylinderBottom";
        fAnalysisTree->SetObservableValue( obsName, dToCylBottom );
    }
    
    if( fPrismFiducial )
    {
        // Adding distances to prism wall
        Double_t dToPrismWall = fOutputHitsEvent->GetClosestHitInsideDistanceToPrismWall( fFid_x0,  fFid_x1, fFid_sX, fFid_sY );
        Double_t dToPrismTop = fOutputHitsEvent->GetClosestHitInsideDistanceToPrismTop( fFid_x0, fFid_x1, fFid_sX, fFid_sY );
        Double_t dToPrismBottom = fOutputHitsEvent->GetClosestHitInsideDistanceToPrismBottom( fFid_x0, fFid_x1, fFid_sX, fFid_sY );

        obsName = this->GetName() + (TString) ".distanceToPrismWall";
        fAnalysisTree->SetObservableValue( obsName, dToPrismWall );

        obsName = this->GetName() + (TString) ".distanceToPrismTop";
        fAnalysisTree->SetObservableValue( obsName, dToPrismTop );

        obsName = this->GetName() + (TString) ".distanceToPrismBottom";
        fAnalysisTree->SetObservableValue( obsName, dToPrismBottom );
    }

    ///////////////////////////////////////

    obsName = this->GetName() + (TString) ".energy";
    fAnalysisTree->SetObservableValue( obsName, energy );

    obsName = this->GetName() + (TString) ".xMean";
    fAnalysisTree->SetObservableValue( obsName, meanPosition.X() );

    obsName = this->GetName() + (TString) ".yMean";
    fAnalysisTree->SetObservableValue( obsName, meanPosition.Y() );

    obsName = this->GetName() + (TString) ".zMean";
    fAnalysisTree->SetObservableValue( obsName, meanPosition.Z() );


    if( GetVerboseLevel() >= REST_Extreme )
    {
        fOutputHitsEvent->PrintEvent( 1000 );
        GetChar();
    }

    return fOutputHitsEvent;
}

//______________________________________________________________________________
void TRestHitsAnalysisProcess::EndOfEventProcess() 
{
}

//______________________________________________________________________________
void TRestHitsAnalysisProcess::EndProcess()
{
    // Function to be executed once at the end of the process 
    // (after all events have been processed)

    //Start by calling the EndProcess function of the abstract class. 
    //Comment this if you don't want it.
    //TRestEventProcess::EndProcess();
}


//______________________________________________________________________________
void TRestHitsAnalysisProcess::InitFromConfigFile( )
{
   
    fFid_x0 = Get3DVectorParameterWithUnits( "fiducial_x0", TVector3(0,0,0) );
    fFid_x1 = Get3DVectorParameterWithUnits( "fiducial_x1", TVector3(0,0,0) );

    fFid_R = GetDblParameterWithUnits( "fiducial_R", 1 );
    fFid_sX = GetDblParameterWithUnits( "fiducial_sX", 1 );
    fFid_sY = GetDblParameterWithUnits( "fiducial_sY", 1 );

    if( GetParameter( "cylinderFiducialization", "false" ) == "true" )
        fCylinderFiducial = true;

    if( GetParameter( "prismFiducialization", "false" ) == "true" )
        fPrismFiducial = true;

    if( fCylinderFiducial ) cout << "Cylinder fiducial active" << endl;
    if( fPrismFiducial ) cout << "Prism fiducial active" << endl;
}

