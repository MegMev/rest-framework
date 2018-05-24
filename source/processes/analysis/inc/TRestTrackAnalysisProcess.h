///______________________________________________________________________________
///______________________________________________________________________________
///______________________________________________________________________________
///             
///
///             RESTSoft : Software for Rare Event Searches with TPCs
///
///             TRestTrackAnalysisProcess.h
///
///_______________________________________________________________________________


#ifndef RestCore_TRestTrackAnalysisProcess
#define RestCore_TRestTrackAnalysisProcess

#include <TRestTrackEvent.h>

#include "TRestEventProcess.h"

class TRestTrackAnalysisProcess:public TRestEventProcess {
    private:
#ifndef __CINT__
        TRestTrackEvent *fInputTrackEvent;
        TRestTrackEvent *fOutputTrackEvent;

        vector <Double_t> fPreviousEventTime;

        Bool_t fCutsEnabled;

        std::vector <std::string> fTrack_LE_EnergyObservables;
        std::vector <Double_t> fTrack_LE_Threshold;
        std::vector <Int_t> nTracks_LE;

        std::vector <std::string> fTrack_HE_EnergyObservables;
        std::vector <Double_t> fTrack_HE_Threshold;
        std::vector <Int_t> nTracks_HE;

        std::vector <std::string> fTrack_En_EnergyObservables;
        std::vector <Double_t> fTrack_En_Threshold;
        std::vector <Int_t> nTracks_En;

        Bool_t fEnableTwistParameters;

        std::vector <TString> fTwistLowObservables;
        std::vector <Double_t> fTwistLowTailPercentage;
        std::vector <Double_t> fTwistLowValue;

        std::vector <TString> fTwistHighObservables;
        std::vector <Double_t> fTwistHighTailPercentage;
        std::vector <Double_t> fTwistHighValue;

        std::vector <TString> fTwistWeightedLowObservables;
        std::vector <Double_t> fTwistWeightedLowTailPercentage;
        std::vector <Double_t> fTwistWeightedLowValue;

        std::vector <TString> fTwistWeightedHighObservables;
        std::vector <Double_t> fTwistWeightedHighTailPercentage;
        std::vector <Double_t> fTwistWeightedHighValue;

        std::vector <TString> fTwistLowObservables_X;
        std::vector <Double_t> fTwistLowTailPercentage_X;
        std::vector <Double_t> fTwistLowValue_X;

        std::vector <TString> fTwistHighObservables_X;
        std::vector <Double_t> fTwistHighTailPercentage_X;
        std::vector <Double_t> fTwistHighValue_X;

        std::vector <TString> fTwistWeightedLowObservables_X;
        std::vector <Double_t> fTwistWeightedLowTailPercentage_X;
        std::vector <Double_t> fTwistWeightedLowValue_X;

        std::vector <TString> fTwistWeightedHighObservables_X;
        std::vector <Double_t> fTwistWeightedHighTailPercentage_X;
        std::vector <Double_t> fTwistWeightedHighValue_X;

        std::vector <TString> fTwistLowObservables_Y;
        std::vector <Double_t> fTwistLowTailPercentage_Y;
        std::vector <Double_t> fTwistLowValue_Y;

        std::vector <TString> fTwistHighObservables_Y;
        std::vector <Double_t> fTwistHighTailPercentage_Y;
        std::vector <Double_t> fTwistHighValue_Y;

        std::vector <TString> fTwistWeightedLowObservables_Y;
        std::vector <Double_t> fTwistWeightedLowTailPercentage_Y;
        std::vector <Double_t> fTwistWeightedLowValue_Y;

        std::vector <TString> fTwistWeightedHighObservables_Y;
        std::vector <Double_t> fTwistWeightedHighTailPercentage_Y;
        std::vector <Double_t> fTwistWeightedHighValue_Y;
#endif

        void InitFromConfigFile();

        void Initialize();

        void LoadDefaultConfig();

    protected:

        //add here the members of your event process

        TVector2 fNTracksXCut;
        TVector2 fNTracksYCut;
        Double_t  fDeltaEnergy;

    public:
        void InitProcess();
        void BeginOfEventProcess(); 
        TRestEvent *ProcessEvent( TRestEvent *eventInput );
        void EndOfEventProcess(); 
        void EndProcess();

        void LoadConfig( std::string cfgFilename, std::string name = "" );

        void PrintMetadata() { 

            BeginPrintProcess();

            if( fCutsEnabled )
            {
                cout << "Number of tracks in X cut : ( " << fNTracksXCut.X() << " , " << fNTracksXCut.Y() << " ) " << endl;
                cout << "Number of tracks in Y cut : ( " << fNTracksYCut.X() << " , " << fNTracksYCut.Y() << " ) " << endl;
            }
            else
            {
                cout << endl;
                cout << "No cuts have been enabled" << endl;

            }

            EndPrintProcess();
        }

        TString GetProcessName() { return (TString) "trackAnalysis"; }

        //Constructor
        TRestTrackAnalysisProcess();
        TRestTrackAnalysisProcess( char *cfgFileName );
        //Destructor
        ~TRestTrackAnalysisProcess();

        ClassDef(TRestTrackAnalysisProcess, 1);      // Template for a REST "event process" class inherited from TRestEventProcess
};
#endif

