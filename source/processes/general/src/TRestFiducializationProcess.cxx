///______________________________________________________________________________
///______________________________________________________________________________
///
///
///             RESTSoft : Software for Rare Event Searches with TPCs
///
///             TRestFiducializationProcess.cxx
///
///
///             First implementation of electron diffusion process into REST_v2
///             Date : Mar/2017
///             Author : J. Galan
///
///_______________________________________________________________________________

#include "TRestFiducializationProcess.h"
using namespace std;

ClassImp(TRestFiducializationProcess)
    //______________________________________________________________________________
    TRestFiducializationProcess::TRestFiducializationProcess() {
    Initialize();
}

//______________________________________________________________________________
TRestFiducializationProcess::TRestFiducializationProcess(char* cfgFileName) {
    Initialize();

    if (LoadConfigFromFile(cfgFileName)) LoadDefaultConfig();
}

//______________________________________________________________________________
TRestFiducializationProcess::~TRestFiducializationProcess() { delete fOutputHitsEvent; }

void TRestFiducializationProcess::LoadDefaultConfig() { SetTitle("Default config"); }

//______________________________________________________________________________
void TRestFiducializationProcess::Initialize() {
    SetSectionName(this->ClassName());

    fOutputHitsEvent = new TRestHitsEvent();
    fInputHitsEvent = NULL;

    fReadout = NULL;
}

void TRestFiducializationProcess::LoadConfig(string cfgFilename, string name) {
    if (LoadConfigFromFile(cfgFilename, name)) LoadDefaultConfig();
}

//______________________________________________________________________________
void TRestFiducializationProcess::InitProcess() {
    fReadout = GetMetadata<TRestDetectorReadout>();
    if (fReadout == NULL) {
        cout << "REST ERRORRRR : Readout has not been initialized" << endl;
        exit(-1);
    }
}

//______________________________________________________________________________
TRestEvent* TRestFiducializationProcess::ProcessEvent(TRestEvent* evInput) {
    fInputHitsEvent = (TRestHitsEvent*)evInput;

    Int_t nHits = fInputHitsEvent->GetNumberOfHits();
    if (nHits <= 0) return NULL;

    TRestHits* hits = fInputHitsEvent->GetHits();
    for (int n = 0; n < nHits; n++) {
        Double_t eDep = hits->GetEnergy(n);

        const Double_t x = hits->GetX(n);
        const Double_t y = hits->GetY(n);
        const Double_t z = hits->GetZ(n);

        const Double_t t = hits->GetTime(n);
        auto type = hits->GetType(n);

        for (int p = 0; p < fReadout->GetNumberOfReadoutPlanes(); p++) {
            TRestDetectorReadoutPlane* plane = fReadout->GetReadoutPlane(p);

            if (plane->GetModuleIDFromPosition(TVector3(x, y, z)) >= 0)
                fOutputHitsEvent->AddHit(x, y, z, eDep, t, type);
        }
    }

    if (fOutputHitsEvent->GetNumberOfHits() == 0) return NULL;

    if (this->GetVerboseLevel() >= REST_Debug) {
        cout << "TRestFiducializationProcess. Hits added : " << fOutputHitsEvent->GetNumberOfHits() << endl;
        cout << "TRestFiducializationProcess. Hits total energy : " << fOutputHitsEvent->GetEnergy() << endl;
    }

    return fOutputHitsEvent;
}

//______________________________________________________________________________
void TRestFiducializationProcess::EndProcess() {
    // Function to be executed once at the end of the process
    // (after all events have been processed)

    // Start by calling the EndProcess function of the abstract class.
    // Comment this if you don't want it.
    // TRestEventProcess::EndProcess();
}

//______________________________________________________________________________
void TRestFiducializationProcess::InitFromConfigFile() {}
