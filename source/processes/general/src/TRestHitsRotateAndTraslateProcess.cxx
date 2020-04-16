///______________________________________________________________________________
///______________________________________________________________________________
///______________________________________________________________________________
///
///
///             RESTSoft : Software for Rare Event Searches with TPCs
///
///             TRestHitsRotateAndTraslateProcess.cxx
///
///             Template to use to design "event process" classes inherited from
///             TRestHitsRotateAndTraslateProcess
///             How to use: replace TRestHitsRotateAndTraslateProcess by your
///             name, fill the required functions following instructions and add
///             all needed additional members and funcionality
///
///             march 2016:   First concept
///                 Created as part of the conceptualization of existing REST
///                 software.
///                 Javier G. Garza
///_______________________________________________________________________________

#include "TRestHitsRotateAndTraslateProcess.h"
using namespace std;

#include <TRandom3.h>

ClassImp(TRestHitsRotateAndTraslateProcess)
    //______________________________________________________________________________
    TRestHitsRotateAndTraslateProcess::TRestHitsRotateAndTraslateProcess() {
    Initialize();
}

//______________________________________________________________________________
TRestHitsRotateAndTraslateProcess::TRestHitsRotateAndTraslateProcess(char* cfgFileName) {
    Initialize();

    if (LoadConfigFromFile(cfgFileName)) LoadDefaultConfig();

    PrintMetadata();

    // TRestHitsRotateAndTraslateProcess default constructor
}

//______________________________________________________________________________
TRestHitsRotateAndTraslateProcess::~TRestHitsRotateAndTraslateProcess() {
    // TRestHitsRotateAndTraslateProcess destructor
}

void TRestHitsRotateAndTraslateProcess::LoadDefaultConfig() {
    SetTitle("Default config");

    fDeltaX = 1.0;
    fDeltaY = 1.0;
    fDeltaZ = 1.0;
    fAlpha = 0.;
    fBeta = 0.;
    fGamma = 0.;
}

//______________________________________________________________________________
void TRestHitsRotateAndTraslateProcess::Initialize() {
    SetSectionName(this->ClassName());

    fDeltaX = 1.0;
    fDeltaY = 1.0;
    fDeltaZ = 1.0;
    fAlpha = 0.;
    fBeta = 0.;
    fGamma = 0.;

    fInputHitsEvent = NULL;
    fOutputHitsEvent = NULL;
}

void TRestHitsRotateAndTraslateProcess::LoadConfig(string cfgFilename) {
    if (LoadConfigFromFile(cfgFilename)) LoadDefaultConfig();

    PrintMetadata();
}

//______________________________________________________________________________
void TRestHitsRotateAndTraslateProcess::InitProcess() {
    // Function to be executed once at the beginning of process
    // (before starting the process of the events)

    // Start by calling the InitProcess function of the abstract class.
    // Comment this if you don't want it.
    // TRestEventProcess::InitProcess();
}

//______________________________________________________________________________
TRestEvent* TRestHitsRotateAndTraslateProcess::ProcessEvent(TRestEvent* evInput) {
    fInputHitsEvent = (TRestHitsEvent*)evInput;

    fOutputHitsEvent = fInputHitsEvent;
    // fInputHitsEvent->CloneTo(fOutputHitsEvent);

    TVector3 meanPosition = fOutputHitsEvent->GetMeanPosition();
    for (int hit = 0; hit < fOutputHitsEvent->GetNumberOfHits(); hit++) {
        fOutputHitsEvent->GetHits()->RotateIn3D(hit, fAlpha, fBeta, fGamma, meanPosition);
        fOutputHitsEvent->GetHits()->Translate(hit, fDeltaX, fDeltaY, fDeltaZ);
    }

    if (fOutputHitsEvent->GetNumberOfHits() == 0) return NULL;

    debug << "Number of hits rotated: " << fInputHitsEvent->GetNumberOfHits() << endl;
    return fOutputHitsEvent;
}

//______________________________________________________________________________
void TRestHitsRotateAndTraslateProcess::EndProcess() {
    // Function to be executed once at the end of the process
    // (after all events have been processed)

    // Start by calling the EndProcess function of the abstract class.
    // Comment this if you don't want it.
    // TRestEventProcess::EndProcess();
}

//______________________________________________________________________________
void TRestHitsRotateAndTraslateProcess::InitFromConfigFile() {
    fDeltaX = GetDblParameterWithUnits("deltaX");
    fDeltaY = GetDblParameterWithUnits("deltaY");
    fDeltaZ = GetDblParameterWithUnits("deltaZ");

    fAlpha = StringToDouble(GetParameter("alpha"));  // rotation angle around Z
    fBeta = StringToDouble(GetParameter("beta"));    // rotation angle around Y
    fGamma = StringToDouble(GetParameter("gamma"));  // rotation angle around X

    // Conversion to radians
    fAlpha = fAlpha * TMath::Pi() / 180.;
    fBeta = fBeta * TMath::Pi() / 180.;
    fGamma = fGamma * TMath::Pi() / 180.;
}
