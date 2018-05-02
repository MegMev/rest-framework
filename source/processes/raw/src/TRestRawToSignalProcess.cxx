///______________________________________________________________________________
///______________________________________________________________________________
///______________________________________________________________________________
///             
///
///             RESTSoft : Software for Rare Event Searches with TPCs
///
///             TRestRawToSignalProcess.cxx
///
///             Template to use to design "event process" classes inherited from 
///             TRestRawToSignalProcess
///             How to use: replace TRestRawToSignalProcess by your name, 
///             fill the required functions following instructions and add all
///             needed additional members and funcionality
///
///             jun 2014:   First concept
///                 Created as part of the conceptualization of existing REST 
///                 software.
///                 Igor G. Irastorza
///_______________________________________________________________________________


#include "TRestRawToSignalProcess.h"
using namespace std;
#include "TTimeStamp.h"

ClassImp(TRestRawToSignalProcess)
//______________________________________________________________________________
TRestRawToSignalProcess::TRestRawToSignalProcess()
{
  Initialize();
}

TRestRawToSignalProcess::TRestRawToSignalProcess(char *cfgFileName)
{
 Initialize();
 
 if( LoadConfigFromFile( cfgFileName ) ) LoadDefaultConfig( );
  
}


//______________________________________________________________________________
TRestRawToSignalProcess::~TRestRawToSignalProcess()
{
   // TRestRawToSignalProcess destructor
} 

void TRestRawToSignalProcess::LoadConfig( string cfgFilename, string name )
{
    if( LoadConfigFromFile( cfgFilename, name ) == -1 ) { cout << "Loading default" << endl; LoadDefaultConfig( ); }
}

//______________________________________________________________________________
void TRestRawToSignalProcess::Initialize()
{
    SetSectionName( this->ClassName() );
    fSignalEvent = new TRestRawSignalEvent( );

    fInputEvent = NULL;
    fOutputEvent = fSignalEvent;
    fInputBinFile = NULL;

    fMinPoints = 512;

    fIsExternal = true;

    fFilenameFormat = "";

    tStart = 0;
}

void TRestRawToSignalProcess::BeginOfEventProcess() 
{
   // cout << "Begin of event process" << endl;
    fSignalEvent->Initialize();
}

void TRestRawToSignalProcess::InitFromConfigFile(){

    fElectronicsType = GetParameter("electronics");
    fShowSamples = StringToInteger( GetParameter("showSamples", "10") );
    fMinPoints = StringToInteger( GetParameter("minPoints", "512" ) );
    fFilenameFormat = GetParameter("fileFormat");

    if( fElectronicsType == "SingleFeminos" || fElectronicsType == "TCMFeminos" ) return;

    if( GetVerboseLevel() >= REST_Warning )
    {
        cout << "REST WARNING: TRestRawToSignalProcess::InitFromConfigFile" << endl;
        cout << "Electronic type " << fElectronicsType << " not found " << endl;
        cout << "Loading default config" << endl;
    }

    LoadDefaultConfig();

}


void TRestRawToSignalProcess::LoadDefaultConfig()
{
    if( GetVerboseLevel() <= REST_Warning )
    {
        cout<<"REST WARNING: TRestRawToSignalProcess "<<endl;
        cout<<"Error Loading config file "<<endl;
    }

    if( GetVerboseLevel() >= REST_Debug )
        GetChar();

    fElectronicsType = "SingleFeminos";
    fMinPoints = 512;

}


//______________________________________________________________________________

void TRestRawToSignalProcess::EndOfEventProcess() 
{

}


//______________________________________________________________________________
void TRestRawToSignalProcess::EndProcess()
{

//close binary file??? Already done

 cout << __PRETTY_FUNCTION__ << endl;
 
}
//______________________________________________________________________________
Bool_t TRestRawToSignalProcess::OpenInputBinFile ( TString fName )
{
	TRestDetectorSetup *det = (TRestDetectorSetup *) this->GetDetectorSetup();

	if( det != NULL )
	{
		fRunOrigin = det->GetRunNumber();
		fSubRunOrigin = det->GetSubRunNumber();
	}
	else
	{
		cout << "REST WARNING : Detector setup has not been defined. Run and subRunNumber will not be defined!" << endl;

	}

	if(fInputBinFile!= NULL)fclose(fInputBinFile);

	if( (fInputBinFile = fopen(fName.Data(),"rb") )==NULL ) {
		cout << "WARNING. Input file does not exist" << endl;
		return kFALSE;
	}

	return kTRUE;
}

//For debugging
void  TRestRawToSignalProcess::printBits(unsigned short num)
{
   for(unsigned short bit=0;bit<(sizeof(unsigned short) * 8); bit++)
   {
      printf("%i ", num & 0x01);
      num = num >> 1;
   }
   
   printf("\n");
}

//For debugging
void  TRestRawToSignalProcess::printBits(unsigned int num)
{
   for(unsigned int bit=0;bit<(sizeof(unsigned int) * 8); bit++)
   {
      printf("%i ", num & 0x01);
      num = num >> 1;
   }
   
   printf("\n");
}

void TRestRawToSignalProcess::PrintMetadata(){

     cout << endl;
    cout << "====================================" << endl;
    cout << "DAQ : " << GetTitle() << endl;
    cout << "Electronics type : " << fElectronicsType.Data() << endl;
    cout << "Minimum number of points : " << fMinPoints << endl;
    cout << "====================================" << endl;
    
    cout << endl;

}

