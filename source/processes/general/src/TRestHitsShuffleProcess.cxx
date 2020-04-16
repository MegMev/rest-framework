//////////////////////////////////////////////////////////////////////////
///
///             RESTSoft : Software for Rare Event Searches with TPCs
///
///             TRestHitsShuffleProcess.cxx
///
///             Jan 2016:   First concept (Javier Galan)
///
//////////////////////////////////////////////////////////////////////////

#include "TRestHitsShuffleProcess.h"
using namespace std;

ClassImp(TRestHitsShuffleProcess)
    //______________________________________________________________________________
    TRestHitsShuffleProcess::TRestHitsShuffleProcess() {
    Initialize();
}

//______________________________________________________________________________
TRestHitsShuffleProcess::TRestHitsShuffleProcess(char* cfgFileName) {
    Initialize();

    if (LoadConfigFromFile(cfgFileName) == -1) LoadDefaultConfig();
}

//______________________________________________________________________________
TRestHitsShuffleProcess::~TRestHitsShuffleProcess() {
    // delete fHitsEvent;
}

void TRestHitsShuffleProcess::LoadDefaultConfig() {
    SetName("hitsShuffleProcess");
    SetTitle("Default config");

    fIterations = 100;
}

//______________________________________________________________________________
void TRestHitsShuffleProcess::Initialize() {
    SetSectionName(this->ClassName());

    fHitsEvent = NULL;

    fRandom = new TRandom3(0);
}

void TRestHitsShuffleProcess::LoadConfig(std::string cfgFilename, std::string name) {
    if (LoadConfigFromFile(cfgFilename, name) == -1) LoadDefaultConfig();
}

//______________________________________________________________________________
void TRestHitsShuffleProcess::InitProcess() {}

//______________________________________________________________________________
TRestEvent* TRestHitsShuffleProcess::ProcessEvent(TRestEvent* evInput) {
    fHitsEvent = (TRestHitsEvent*)evInput;

    TRestHits* hits = fHitsEvent->GetHits();

    Int_t nHits = hits->GetNumberOfHits();
    if (nHits >= 2) {
        for (int n = 0; n < fIterations; n++) {
            Int_t hit1 = (Int_t)(nHits * fRandom->Uniform(0, 1));
            Int_t hit2 = (Int_t)(nHits * fRandom->Uniform(0, 1));

            hits->SwapHits(hit1, hit2);
        }
    }
    return fHitsEvent;
}

//______________________________________________________________________________
void TRestHitsShuffleProcess::EndProcess() {}

//______________________________________________________________________________
void TRestHitsShuffleProcess::InitFromConfigFile() {
    fIterations = StringToInteger(GetParameter("iterations"));
}
