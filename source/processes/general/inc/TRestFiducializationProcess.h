///______________________________________________________________________________
///______________________________________________________________________________
///______________________________________________________________________________
///
///
///             RESTSoft : Software for Rare Event Searches with TPCs
///
///             TRestFiducializationProcess.h
///
///_______________________________________________________________________________

#ifndef RestCore_TRestFiducializationProcess
#define RestCore_TRestFiducializationProcess

#include <TRestDetectorReadout.h>
#include <TRestHitsEvent.h>

#include "TRestEventProcess.h"

class TRestFiducializationProcess : public TRestEventProcess {
   private:
#ifndef __CINT__
    TRestHitsEvent* fInputHitsEvent;   //!
    TRestHitsEvent* fOutputHitsEvent;  //!

    TRestDetectorReadout* fReadout;  //!

#endif

    void InitFromConfigFile();

    void Initialize();

    void LoadDefaultConfig();

   protected:
   public:
    any GetInputEvent() { return fInputHitsEvent; }
    any GetOutputEvent() { return fOutputHitsEvent; }

    void InitProcess();
    TRestEvent* ProcessEvent(TRestEvent* eventInput);
    void EndProcess();

    void LoadConfig(std::string cfgFilename, std::string name = "");

    void PrintMetadata() {
        BeginPrintProcess();

        EndPrintProcess();
    }

    TString GetProcessName() { return (TString) "fiducialization"; }

    // Constructor
    TRestFiducializationProcess();
    TRestFiducializationProcess(char* cfgFileName);
    // Destructor
    ~TRestFiducializationProcess();

    ClassDef(TRestFiducializationProcess, 1);  // Template for a REST "event process" class inherited from
                                               // TRestEventProcess
};
#endif
