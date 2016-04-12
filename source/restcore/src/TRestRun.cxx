///______________________________________________________________________________
///______________________________________________________________________________
///______________________________________________________________________________
///             
///
///             RESTSoft : Software for Rare Event Searches with TPCs
///
///             TRestRun.cxx
///
///             Base class for managing run data storage. It contains a TRestEvent and TRestMetadata array. 
///
///             apr 2015:   First concept
///                 Created as part of the conceptualization of existing REST 
///                 software.
///             aug 2015    Javier Galan
///_______________________________________________________________________________

#include <TGeoManager.h>

#include "TRestRun.h"
using namespace std;

#define TIME_MEASUREMENT

#ifdef TIME_MEASUREMENT
#include <chrono>
using namespace chrono;
int deltaTime = 0;
int writeTime = 0;
int readTime = 0;
#endif

const int debug = 0;

ClassImp(TRestRun)
//______________________________________________________________________________
    TRestRun::TRestRun()
{
    Initialize();

    SetVersion();
}

TRestRun::TRestRun( char *cfgFileName) : TRestMetadata (cfgFileName)
{
    Initialize();

    this->LoadConfigFromFile( fConfigFileName );

    SetVersion();
}

void TRestRun::Initialize()
{
    SetName( "run" );
    cout << __PRETTY_FUNCTION__ << endl;

    time_t  timev; time(&timev);
    fStartTime = (Double_t) timev;
    fEndTime = fStartTime-1; // So that run length will be -1 if fEndTime is not set

    fRunUser = "";
    fRunNumber = 0;
    fParentRunNumber = 0;
    fRunType = "Null";
    fExperimentName = "Null";
    fRunTag = "Null";

    fOutputFile = NULL;
    fInputFile = NULL;

    fInputEvent = NULL;
    fOutputEvent = NULL;

    fInputEventTree = NULL;
    fOutputEventTree = NULL;

    fInputAnalysisTree = NULL;
    fOutputAnalysisTree = NULL;

    fInputFilename = "null";
    fOutputFilename = "default";

    fOverwrite = false;

    fCurrentEvent = 0;
    fProcessedEvents = 0;
    fEventIDs.clear();
    fSubEventIDs.clear();
    fSubEventTags.clear();

}

void TRestRun::ResetRunTimes()
{
    time_t  timev; time(&timev);
    fStartTime = (Double_t) timev;
    fEndTime = fStartTime-1; // So that run length will be -1 if fEndTime is not set
}


//______________________________________________________________________________
TRestRun::~TRestRun()
{
    cout << "Deleting TRestRun" << endl;
    if( fOutputFile != NULL ) CloseOutputFile();
}

void TRestRun::Start(  )
{
    cout << "TRestRun::Start( ) is OBSOLETE. You should change your code to use ProcessEvents( ) instead" << endl;

    ProcessEvents(  );

}

void TRestRun::ProcessEvents( Int_t firstEvent, Int_t eventsToProcess ) 
{

	fCurrentEvent = firstEvent;

	if( fEventProcess.size() == 0 ) { cout << "WARNNING Run does not contain processes" << endl; return; }

	this->SetRunType( fEventProcess[fEventProcess.size()-1]->GetProcessName() );

	this->OpenOutputFile();

	this->SetInputEvent( fEventProcess.front()->GetInputEvent() );

	this->SetOutputEvent( fEventProcess.back()->GetOutputEvent() );

	//////////////////
	
	for( unsigned int i = 0; i < fEventProcess.size(); i++ ) fEventProcess[i]->SetAnalysisTree( fOutputAnalysisTree );

	for( unsigned int i = 0; i < fEventProcess.size(); i++ ) fEventProcess[i]->InitProcess();

    fOutputAnalysisTree->CreateObservableBranches( );

    fProcessedEvents = 0;
    fEventIDs.clear();
    fSubEventIDs.clear();
    fSubEventTags.clear();

    if( eventsToProcess == 0 )
    {
       if( fInputEventTree != NULL ) eventsToProcess = fInputEventTree->GetEntries();
       else eventsToProcess = 2E9;
    }

	TRestEvent *processedEvent;
	while( this->GetNextEvent() && eventsToProcess > fProcessedEvents )
	{
		processedEvent = fInputEvent;

#ifdef TIME_MEASUREMENT
        high_resolution_clock::time_point t1 = high_resolution_clock::now();
#endif

		for( unsigned int j = 0; j < fEventProcess.size(); j++ )
		{
			fEventProcess[j]->BeginOfEventProcess();
			processedEvent = fEventProcess[j]->ProcessEvent( processedEvent );
			if( processedEvent == NULL ) break;
			fEventProcess[j]->EndOfEventProcess();
		}

#ifdef TIME_MEASUREMENT
    high_resolution_clock::time_point t2 = high_resolution_clock::now();
    deltaTime += (int) duration_cast<microseconds>( t2 - t1 ).count();
#endif
		

		fOutputEvent = processedEvent;
		if( processedEvent == NULL ) continue;

		if (fInputEventTree != NULL)
		{
		    fOutputEvent->SetID( fInputEvent->GetID() );
		    fOutputEvent->SetTime( fInputEvent->GetTime() );
		}

#ifdef TIME_MEASUREMENT
        high_resolution_clock::time_point t3 = high_resolution_clock::now();
#endif

        this->Fill();

#ifdef TIME_MEASUREMENT
        high_resolution_clock::time_point t4 = high_resolution_clock::now();
        writeTime += (int) duration_cast<microseconds>( t4 - t3 ).count();
#endif

		PrintProcessedEvents(100);
	}

	cout << fOutputEventTree->GetEntries() << " processed events" << endl;

#ifdef TIME_MEASUREMENT
    cout << "Average event process time : " << ((Double_t) deltaTime)/fProcessedEvents/1000. << " ms" << endl;
    cout << "Total write time to disk (per event) : " << ((Double_t) writeTime)/fProcessedEvents/1000. << " ms" << endl;
    cout << "Total read time from disk (per event) : " << ((Double_t) readTime)/fProcessedEvents/1000. << " ms" << endl;
#endif

	for( unsigned int i = 0; i < fEventProcess.size(); i++ )
		fEventProcess[i]->EndProcess();

}

void TRestRun::AddProcess( TRestEventProcess *process, string cfgFilename ) 
{

    // We give a pointer to the metadata stored in TRestRun to the processes. This metadata will be destroyed afterwards
    // it is not intended for storage, just for the processes so that they are aware of all metadata information.
    // Each proccess is responsible to implement GetProcessMetadata so that TRestRun stores this metadata.

    vector <TRestMetadata*> metadata;
    for( size_t i = 0; i < fMetadata.size(); i++ )
        metadata.push_back( fMetadata[i] );
    for( size_t i = 0; i < fHistoricMetadata.size(); i++ )
        metadata.push_back( fHistoricMetadata[i] );
    for( size_t i = 0; i < fEventProcess.size(); i++ )
        metadata.push_back( fEventProcess[i] );
    for( size_t i = 0; i < fHistoricEventProcess.size(); i++ )
        metadata.push_back( fHistoricEventProcess[i] );

    process->SetMetadata( metadata );

    cout << "Metadata given to process : " << process->GetName() << endl;
    cout << "------------------------------------------------------" << endl;
    for( size_t i = 0; i < metadata.size(); i++ )
        cout << metadata[i]->ClassName() << endl;
    cout << "---------------------------" << endl;

    process->LoadConfig( cfgFilename );

    //process->LoadConfigFromFile( cfgFilename );
    // Each proccess is responsible to implement GetMetadata so that TRestRun stores this metadata.

    TRestMetadata *meta = process->GetProcessMetadata();
    if( meta != NULL )
    {
        meta->PrintMetadata();
        this->AddMetadata( meta );
    }

    process->PrintMetadata( );

    fEventProcess.push_back( process ); 

}

void TRestRun::SetOutputEvent( TRestEvent *evt ) 
{ 
    cout << "Setting output event" << endl;
    fOutputEvent = evt;

    if( fOutputEventTree == NULL )
    {
        TString treeName = (TString) evt->GetName() + "Tree";
        fOutputEventTree  = new TTree( GetName(), GetTitle() );
        if( GetVerboseLevel() == REST_Debug ) cout << "Creating tree : " << fOutputEventTree << endl;
        fOutputEventTree->SetName( treeName );
        fOutputEventTree->Branch("eventBranch", evt->GetName(), fOutputEvent);

    }
    if( fOutputAnalysisTree == NULL )
    {
        fOutputAnalysisTree = new TRestAnalysisTree( "TRestAnalysisTree", GetTitle() );
        fOutputAnalysisTree->CreateEventBranches( );

        if( fInputAnalysisTree != NULL )
        {
            Int_t nObs = fInputAnalysisTree->GetNumberOfObservables( );
            for( int n = 0; n < nObs; n++ )
                fOutputAnalysisTree->AddObservable( fInputAnalysisTree->GetObservableName( n ) );
        }
    }
}

void TRestRun::SetInputEvent( TRestEvent *evt ) 
{ 
    fInputEvent = evt;

    if( evt == NULL ) return;

    TString treeName = (TString) evt->GetName() + "Tree";

    if( GetObjectKeyByName( treeName ) == NULL )
    {
        cout << "REST ERROR (SetInputEvent) : " << treeName << " was not found" << endl;
        return;
    }

    fInputEventTree = (TTree * ) fInputFile->Get( treeName );

    TBranch *br = fInputEventTree->GetBranch( "eventBranch" );

    br->SetAddress( &fInputEvent );

    if( GetObjectKeyByName( "TRestAnalysisTree" ) == NULL )
    {
        cout << "REST ERROR (SetInputEvent) : TRestAnalysisTree was not found" << endl;
        return;
    }

    fInputAnalysisTree = ( TRestAnalysisTree * ) fInputFile->Get( "TRestAnalysisTree" ); 

    fInputAnalysisTree->ConnectEventBranches( );
    fInputAnalysisTree->ConnectObservables( );

}

Bool_t TRestRun::isClass( TString className )
{
	if( fInputFile == NULL ) { cout << "No input file" << endl; return kFALSE; }

	TIter nextkey( fInputFile->GetListOfKeys() );
	TKey *key;
	while ( (key = (TKey*) nextkey() ) ) 
	{
		TString cName (key->GetName());

		if ( cName.Contains(className.Data()) )
		{
			cout << "className : " << cName << " target "<< className << endl;
			return kTRUE;
		}
	}

	cout << "Class " << className << " not found" << endl;

	return kFALSE;
}

TKey *TRestRun::GetObjectKeyByClass( TString className )
{
    if( fInputFile == NULL ) { cout << "REST ERROR (GetObjectKey) : No file open" << endl; return NULL; }

    TIter nextkey(fInputFile->GetListOfKeys());
    TKey *key;
    while ( (key = (TKey*)nextkey() ) ) {

        string cName = key->GetClassName();

        if ( cName == className ) return key;
    }
    cout << "REST ERROR (GetObjectKey) : " << className << " was not found" << endl;
    return NULL;

}

TKey *TRestRun::GetObjectKeyByName( TString name )
{
    if( fInputFile == NULL ) { cout << "REST ERROR (GetObjectKey) : No file open" << endl; return NULL; }

    TIter nextkey(fInputFile->GetListOfKeys());
    TKey *key;
    while ( (key = (TKey*)nextkey() ) ) {

        string kName = key->GetName();

        if ( kName == name ) return key;
    }
    cout << "REST ERROR (GetObjectKey) : " << name << " was not found" << endl;
    return NULL;

}

TRestMetadata *TRestRun::GetMetadata( TString name )
{
    for( unsigned int i = 0; i < fMetadata.size(); i++ )
        if( fMetadata[i]->GetName() == name ) return fMetadata[i];

    for( unsigned int i = 0; i < fHistoricMetadata.size(); i++ )
        if( fHistoricMetadata[i]->GetName() == name ) return fHistoricMetadata[i];

    return NULL;

}

void TRestRun::ImportMetadata( TString rootFile, TString name )
{
    TFile *f = new TFile( rootFile );
    // TODO give error in case we try to obtain a class that is not TRestMetadata
    TRestMetadata *meta = (TRestMetadata *) f->Get( name );
    this->AddMetadata( meta );
    f->Close();
}

void TRestRun::OpenInputFile( TString fName )
{
    if( fInputFile != NULL ) fInputFile->Close();

    if( !fileExists( fName.Data() ) ) {
        cout << "TRestRun. WARNING. Input file does not exist" << endl;
        return;
    }

    fInputFile = new TFile( fName );

    Int_t runNumber = GetRunNumber();
    TString fileName = GetOutputFilename();

    TKey *key = GetObjectKeyByClass( "TRestRun" );
    this->Read( key->GetName() );

    fParentRunNumber = fRunNumber;
    fRunNumber = runNumber;

    fOutputFilename = fileName; // We take this value from the configuration (not from TRestRun)

    // Transfering metadata to historic
    for( size_t i = 0; i < fMetadata.size(); i++ )
        fHistoricMetadata.push_back( fMetadata[i] );
    fMetadata.clear();
    for( size_t i = 0; i < fEventProcess.size(); i++ )
        fHistoricEventProcess.push_back( fEventProcess[i] );
    fEventProcess.clear();
}

void TRestRun::OpenInputFile( TString fName, TString cName )
{
    cout << __PRETTY_FUNCTION__ << endl;
    cout << "OBSOLETE.........." << endl;
    /*
    if( fInputFile != NULL ) fInputFile->Close();

    fInputFile = new TFile( fName );
    TIter nextkey(fInputFile->GetListOfKeys());
    TKey *key;
    while ( (key = (TKey*)nextkey() ) ) {
        string className = key->GetClassName();

        if ( className == cName )
        {
            this->Read( key->GetName() );
        }
    }
    */
}


void TRestRun::OpenOutputFile( )
{
    this->ResetRunTimes();

    SetVersion();

    if( fOutputFilename == "default" ) SetRunFilenameAndIndex();
    else fOutputFilename = GetDataPath() + "/" + fOutputFilename;

    if( GetVerboseLevel() == REST_Info ) cout << "Opening file : " << fOutputFilename << endl;

    fOutputFile = new TFile( fOutputFilename, "recreate" );
    fOutputFile->SetCompressionLevel(0);
}

void TRestRun::CloseOutputFile( )
{
    cout << __PRETTY_FUNCTION__ << endl;
    time_t  timev;
    time(&timev);

    fEndTime = (Double_t) timev;

    fOutputFile->cd();

    if( fInputFile != NULL ) fInputFilename = fInputFile->GetName();

    char tmpString[256];
    if( fMetadata.size() > 0 )
    {
        for( unsigned int i = 0; i < fMetadata.size(); i++ )
        {
            cout << "Writting metadata (" << fMetadata[i]->GetName() << ") : " << fMetadata[i]->GetTitle() << endl;
            sprintf( tmpString, "M%d. %s", i,  fMetadata[i]->GetName() );
            fMetadata[i]->Write( tmpString );
        }
    }

    if( fHistoricMetadata.size() > 0 )
    {
        for( unsigned int i = 0; i < fHistoricMetadata.size(); i++ )
        {
            cout << "Writting historic metadata (" << fHistoricMetadata[i]->GetName() << ") : " << fHistoricMetadata[i]->GetTitle() << endl;
            sprintf( tmpString, "HM%d. %s", i,  fHistoricMetadata[i]->GetName() );
            fHistoricMetadata[i]->Write( tmpString );
        }
    }

    if( fEventProcess.size() > 0 )
    {
        for( unsigned int i = 0; i < fEventProcess.size(); i++ )
        {
            cout << "Writting process (" << fEventProcess[i]->GetName() << ") : " << fEventProcess[i]->GetTitle() << endl;
            sprintf( tmpString, "P%d. %s", i,  fEventProcess[i]->GetName() );
            fEventProcess[i]->Write( tmpString );
        }
    }

    if( fHistoricEventProcess.size() > 0 )
    {
        for( unsigned int i = 0; i < fHistoricEventProcess.size(); i++ )
        {
            cout << "Writting historic process (" << fHistoricEventProcess[i]->GetName() << ") : " << fHistoricEventProcess[i]->GetTitle() << endl;
            sprintf( tmpString, "HP%d. %s", i,  fHistoricEventProcess[i]->GetName() );
            fHistoricEventProcess[i]->Write( tmpString );
        }
    }

    if( fOutputEventTree != NULL )
    {
        cout << "Writting output tree" << endl;
        fOutputEventTree->Write();
        fOutputAnalysisTree->Write();
    }

    if( fInputFile != NULL )
    {
        TGeoManager *geo = (TGeoManager *) fInputFile->Get("Default");
        if( geo != NULL ) geo->Write();


    }

    this->Write();

    cout << "Closing output file : " << endl;
    fOutputFile->Close();
    fOutputFile = NULL;
    cout << fOutputFilename << endl;
}

void TRestRun::SetVersion()
{

    char originDirectory[255];
    sprintf( originDirectory, "%s", get_current_dir_name() );

    char buffer[255];
    sprintf( buffer, "%s", getenv( "REST_PATH" ) );
    chdir( buffer );

    // Reading the version of libcore.so
    FILE *fV = popen("git rev-parse --verify HEAD", "r");

    int nbytes;
    string versionStr;
    while ((nbytes = fread(buffer, 1, 255, fV)) > 0)
    {
        versionStr = buffer;
        versionStr = versionStr.substr(0, 8 );
    }

    pclose( fV );

    chdir( originDirectory );

    fVersion = versionStr;
}


TString TRestRun::GetTime( Double_t runTime )
{
       time_t tt = (time_t) runTime;
       struct tm *tm = localtime( &tt);

       char time[256];
       strftime(time, sizeof(time), "%H:%M:%S", tm);

       return time;
}

TString TRestRun::GetDateForFilename( Double_t runTime )
{
       time_t tt = (time_t) runTime;
       struct tm *tm = localtime( &tt);

       char date[256];
       strftime(date, sizeof(date), "%Y%m%d", tm);

       return date;
}

TString TRestRun::GetDateFormatted( Double_t runTime )
{
       time_t tt = (time_t) runTime;
       struct tm *tm = localtime( &tt);

       char date[256];
       strftime(date, sizeof(date), "%Y-%B-%d", tm);

       return date;
}

void TRestRun::PrintStartDate()
{
       cout.precision(10);

       cout << "------------------------" << endl;
       cout << "---- Run start date ----" << endl;
       cout << "------------------------" << endl;
       cout << "Unix time : " << fStartTime << endl;
       time_t tt = (time_t) fStartTime;
       struct tm *tm = localtime( &tt);

       char date[20];
       strftime(date, sizeof(date), "%Y-%m-%d", tm);
       cout << "Date : " << date << endl;

       char time[20];
       strftime(time, sizeof(time), "%H:%M:%S", tm);
       cout << "Time : " << time << endl;
       cout << "++++++++++++++++++++++++" << endl;
}

void TRestRun::PrintEndDate()
{
       cout << "----------------------" << endl;
       cout << "---- Run end date ----" << endl;
       cout << "----------------------" << endl;
       cout << "Unix time : " << fEndTime << endl;
       time_t tt = (time_t) fEndTime;
       struct tm *tm = localtime( &tt);

       char date[20];
       strftime(date, sizeof(date), "%Y-%m-%d", tm);
       cout << "Date : " << date << endl;

       char time[20];
       strftime(time, sizeof(time), "%H:%M:%S", tm);
       cout << "Time : " << time << endl;
       cout << "++++++++++++++++++++++++" << endl;
}

Double_t TRestRun::GetRunLength()
{
    if( fEndTime-fStartTime == -1 )
        cout << "Run time is not set" << endl;
    return fEndTime-fStartTime;
}

//______________________________________________________________________________
void TRestRun::InitFromConfigFile()
{
    cout << __PRETTY_FUNCTION__ << endl;

   // Initialize the metadata members from a configfile
   fRunEvents = StringToInteger( GetParameter( "Nevents" ) );

   fRunUser = GetParameter( "user" );

   fRunType = GetParameter( "runType" );

   fRunDescription = GetParameter( "runDescription" );

   TString rNumberStr = (TString) GetParameter( "runNumber" );

   fOutputFilename = GetParameter( "outputFile", "default" );

   if( GetParameter( "overwrite" ) == "on" ) { cout << "Overwrite : on" << endl; fOverwrite = true; }

   if( rNumberStr == "auto" )
   {
       char runFilename[256];
       sprintf( runFilename, "%s/inputData/runNumber", getenv("REST_PATH") );
       if( !fileExists( (string) runFilename ) )
       {
           cout << "REST Warning : File " << runFilename << " does not exist" << endl;
           cout << "Setting run number to 1" << endl;
           fRunNumber = 1;
       }
       else
       {
           FILE *frun = fopen( runFilename, "r" );
           fscanf( frun, "%d\n", &fRunNumber );
           fclose( frun );

           if( fOverwrite )
               fRunNumber -= 1;
       }

       FILE *frun = fopen( runFilename, "w" );
       fprintf( frun, "%d\n", fRunNumber+1 );
       fclose( frun );
   }
   else
   {
       fRunNumber = StringToInteger ( GetParameter( "runNumber" ) );
   }

   fExperimentName = GetParameter( "experiment" );

   fRunTag = GetParameter( "runTag" );

}

void TRestRun::SetRunFilenameAndIndex()
{ 

    string expName = RemoveWhiteSpaces( (string) GetExperimentName() );
    string runType = RemoveWhiteSpaces( (string) GetRunType() );
    char runParentStr[256];
    sprintf( runParentStr, "%05d", fParentRunNumber );
    char runNumberStr[256];
    sprintf( runNumberStr, "%05d", fRunNumber );

    fOutputFilename = GetDataPath() + "/Run_" + expName + "_"+ fRunUser + "_"  
        + runType + "_" + fRunTag + "_" + (TString) runNumberStr + "_" + (TString) runParentStr + "_V" + fVersion + ".root";

    while( !fOverwrite && fileExists( (string) fOutputFilename ) )
    {
        fRunNumber++;
        sprintf( runNumberStr, "%05d", fRunNumber );
        fOutputFilename = GetDataPath() + "/Run_" + expName + "_"+ fRunUser + "_"  
            + runType + "_" + fRunTag + "_" + (TString) runNumberStr + "_" + (TString) runParentStr + "_V" + fVersion + ".root";
    }
}


void TRestRun::PrintInfo( )
{

        cout.precision(10);
        cout << endl;
        cout << "+++++++++++++++++++++++++++++++++++++++++++++" << endl;
        cout << "TRestRun content" << endl;
        cout << "+++++++++++++++++++++++++++++++++++++++++++++" << endl;
        cout << "Config file : " << fConfigFileName << endl;
        cout << "Section name : " << fSectionName << endl;        // section name given in the constructor of TRestSpecificMetadata
        cout << "---------------------------------------" << endl;
        cout << "Name : " << GetName() << endl;
        cout << "Title : " << GetTitle() << endl;
        cout << "---------------------------------------" << endl;
        cout << "Parent run number : " << GetParentRunNumber() << endl; 
        cout << "Run number : " << GetRunNumber() << endl; 
        cout << "Run type : " << GetRunType() << endl;
        cout << "Run tag : " << GetRunTag() << endl;
        cout << "Run user : " << GetRunUser() << endl;
        cout << "Run description : " << GetRunDescription() << endl;
        cout << "Run events : " << GetNumberOfEvents() << endl;
        cout << "Start timestamp : " << GetStartTimestamp() << endl;
        cout << "Date/Time : " << GetDateFormatted( GetStartTimestamp() ) << " / " << GetTime( GetStartTimestamp() ) << endl;
        cout << "End timestamp : " << GetEndTimestamp() << endl;
        cout << "Date/Time : " << GetDateFormatted( GetEndTimestamp() ) << " / " << GetTime( GetEndTimestamp() ) << endl;
        cout << "Input filename : " << fInputFilename << endl;
        cout << "Output filename : " << fOutputFilename << endl;
        cout << "Number of processed events : " << fProcessedEvents << endl;
        cout << "+++++++++++++++++++++++++++++++++++++++++++++" << endl;

}

void TRestRun::PrintProcessedEvents( Int_t rateE)
{

    if(fCurrentEvent%rateE ==0){
        if(fInputEvent==NULL)
        {
            printf("%d processed events now...\r",fCurrentEvent);
            fflush(stdout);
        }
        else
        {
            printf("%.2lf\r",(float)(fCurrentEvent/fInputEventTree->GetEntries())*100.);
            fflush(stdout);
        }

    }
}

Int_t TRestRun::Fill( )
{
    fProcessedEvents++;
    fEventIDs.push_back( fOutputEvent->GetID() );
    fSubEventIDs.push_back( fOutputEvent->GetSubID() );
    fSubEventTags.push_back( fOutputEvent->GetSubEventTag() );

    Int_t found = 0;
    for( unsigned int i = 0; i < fSubEventTagList.size(); i++ )
        if( fOutputEvent->GetSubEventTag() == fSubEventTagList[i] ) found = 1;

    if( !found ) fSubEventTagList.push_back( fOutputEvent->GetSubEventTag() );

    if( fInputAnalysisTree != NULL )
    {
        fInputAnalysisTree->GetEntry( fOutputAnalysisTree->GetEntries()+1);

        for( int n = 0; n < fInputAnalysisTree->GetNumberOfObservables(); n++ )
            fOutputAnalysisTree->SetObservableValue( n, fInputAnalysisTree->GetObservableValue( n ) );
    }
    fOutputAnalysisTree->FillEvent( fOutputEvent );

    return fOutputEventTree->Fill();
}

Int_t TRestRun::GetEventWithID( Int_t eventID, Int_t subEventID )
{
    Int_t currentEvent = fCurrentEvent;

    Int_t nEntries = fInputEventTree->GetEntries();

    if( nEntries != (Int_t) fEventIDs.size() ) { cout << "REST WARNING. Tree and eventIDs have not the same size!!" << endl; return 0; }

    do
    {
        if( fEventIDs[currentEvent] == eventID && fSubEventIDs[currentEvent] == subEventID )
        {
            fCurrentEvent = currentEvent;
            fInputEventTree->GetEntry( fCurrentEvent );
            return 1;
        }

        if( currentEvent == nEntries-1 ) currentEvent = 0;
        else currentEvent++;
    }
    while( currentEvent != fCurrentEvent );

    return 0;
}

Int_t TRestRun::GetEventWithID( Int_t eventID, TString tag )
{
    Int_t currentEvent = fCurrentEvent;

    Int_t nEntries = fInputEventTree->GetEntries();

    if( nEntries != (Int_t) fEventIDs.size() ) { cout << "REST WARNING. Tree and eventIDs have not the same size!!" << endl; return 0; }

    do
    {
        if( fEventIDs[currentEvent] == eventID && fSubEventTags[currentEvent] == tag )
        {
            fCurrentEvent = currentEvent;
            fInputEventTree->GetEntry( fCurrentEvent );
            return 1;
        }

        if( currentEvent == nEntries-1 ) currentEvent = 0;
        else currentEvent++;
    }
    while( currentEvent != fCurrentEvent );

    return 0;
}

//Return false when the file ends
Bool_t TRestRun::GetNextEvent( )
{
    if(fInputEvent == NULL)
    {
        if( fOutputEvent == NULL ) { return kFALSE; }
        fCurrentEvent++;
    }
    else
    {
#ifdef TIME_MEASUREMENT
        high_resolution_clock::time_point t5 = high_resolution_clock::now();
#endif

        if( fInputEventTree->GetEntries() == fCurrentEvent-1 ) return kFALSE;

        fInputEventTree->GetEntry( fCurrentEvent );
        fCurrentEvent++;
#ifdef TIME_MEASUREMENT
        high_resolution_clock::time_point t6 = high_resolution_clock::now();
        readTime += (int) duration_cast<microseconds>( t6 - t5 ).count();
#endif
    }

    return kTRUE;
}

