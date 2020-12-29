///______________________________________________________________________________
///______________________________________________________________________________
///______________________________________________________________________________
///
///
///             RESTSoft : Software for Rare Event Searches with TPCs
///
///             TRestHitsToSignalProcess.cxx
///
///             oct 2015:  Javier Galan
///_______________________________________________________________________________

#include "TRestHitsToSignalProcess.h"

using namespace std;

ClassImp(TRestHitsToSignalProcess)
    //______________________________________________________________________________
    TRestHitsToSignalProcess::TRestHitsToSignalProcess() {
    Initialize();
}

// __________________________________________________________
//     TODO : Perhaps this constructor should be removed
//            since we will allway load the config from TRestRun
//            when we use AddProcess. It would be necessary only if we use the
//            process stand alone but even then we could just call LoadConfig
//            __________________________________________________________
TRestHitsToSignalProcess::TRestHitsToSignalProcess(char* cfgFileName) {
    Initialize();

    if (LoadConfigFromFile(cfgFileName) == -1) LoadDefaultConfig();

    PrintMetadata();

    if (fReadout == NULL) fReadout = new TRestReadout(cfgFileName);

    // TRestHitsToSignalProcess default constructor
}

//______________________________________________________________________________
TRestHitsToSignalProcess::~TRestHitsToSignalProcess() {
    if (fReadout != NULL) delete fReadout;

    delete fSignalEvent;
    // TRestHitsToSignalProcess destructor
}

void TRestHitsToSignalProcess::LoadDefaultConfig() {
    SetName("hitsToSignalProcess-Default");
    SetTitle("Default config");

    cout << "Hits to signal metadata not found. Loading default values" << endl;

    fSampling = 1;
    fElectricField = 1000;
    fGasPressure = 10;
}

void TRestHitsToSignalProcess::LoadConfig(string cfgFilename, string name) {
    // if (LoadConfigFromFile(cfgFilename, name)) LoadDefaultConfig();

    //// If the parameters have no value it tries to obtain it from
    //// electronDiffusionProcess
    // if (fElectricField == PARAMETER_NOT_FOUND_DBL) {
    //    fElectricField =
    //        this->GetDoubleParameterFromFriendsWithUnits("TRestElectronDiffusionProcess", "electricField");
    //    if (fElectricField != PARAMETER_NOT_FOUND_DBL) {
    //        cout << "Getting electric field from electronDiffusionProcess : " << fElectricField << " V/cm"
    //             << endl;
    //    }
    //}
}

//______________________________________________________________________________
void TRestHitsToSignalProcess::Initialize() {
    SetSectionName(this->ClassName());

    fReadout = NULL;
    fGas = NULL;

    fHitsEvent = NULL;
    fSignalEvent = new TRestSignalEvent();
}

//______________________________________________________________________________
void TRestHitsToSignalProcess::InitProcess() {
    // Function to be executed once at the beginning of process
    // (before starting the process of the events)

    // Start by calling the InitProcess function of the abstract class.
    // Comment this if you don't want it.
    // TRestEventProcess::InitProcess();

    fGas = GetMetadata<TRestGas>();
    if (fGas != NULL) {
#ifndef USE_Garfield
        ferr << "A TRestGas definition was found but REST was not linked to Garfield libraries." << endl;
        ferr << "Please, remove the TRestGas definition, and add gas parameters inside the process "
                "TRestHitsToSignalProcess"
             << endl;
        exit(-1);
#endif
        if (fGasPressure <= 0) fGasPressure = fGas->GetPressure();
        if (fElectricField <= 0) fElectricField = fGas->GetElectricField();

        fGas->SetPressure(fGasPressure);
        fGas->SetElectricField(fElectricField);

        if (fDriftVelocity <= 0) fDriftVelocity = fGas->GetDriftVelocity();
    } else {
        warning << "No TRestGas found in TRestRun." << endl;
        if (fDriftVelocity == -1) {
            ferr << "TRestHitsToSignalProcess: drift velocity is undefined in the rml file!" << endl;
            exit(-1);
        }
    }

    fReadout = GetMetadata<TRestReadout>();

    if (fReadout == NULL) {
        ferr << "Readout has not been initialized" << endl;
        exit(-1);
    }
}

Int_t TRestHitsToSignalProcess::FindModule(Int_t readoutPlane, Double_t x, Double_t y) {
    // TODO verify this
    TRestReadoutPlane* plane = &(*fReadout)[readoutPlane];
    for (int md = 0; md < plane->GetNumberOfModules(); md++)
        if ((*plane)[md].isInside(x, y)) return md;

    return -1;
}

//______________________________________________________________________________
TRestEvent* TRestHitsToSignalProcess::ProcessEvent(TRestEvent* evInput) {
    fHitsEvent = (TRestHitsEvent*)evInput;
    fSignalEvent->SetEventInfo(fHitsEvent);
    //     fHitsEvent = dynamic_cast<TRestHitsEvent*>(evInput);

    if (GetVerboseLevel() >= REST_Debug) {
        cout << "Number of hits : " << fHitsEvent->GetNumberOfHits() << endl;
        cout << "--------------------------" << endl;
    }

    for (int hit = 0; hit < fHitsEvent->GetNumberOfHits(); hit++) {
        Double_t x = fHitsEvent->GetX(hit);
        Double_t y = fHitsEvent->GetY(hit);
        Double_t z = fHitsEvent->GetZ(hit);
        Double_t t = fHitsEvent->GetTime(hit);

        if (GetVerboseLevel() >= REST_Extreme && hit < 20)
            cout << "Hit : " << hit << " x : " << x << " y : " << y << " z : " << z << " t : " << t << endl;

        Int_t planeId = -1;
        Int_t moduleId = -1;
        Int_t channelId = -1;

        Int_t daqId = fReadout->GetHitsDaqChannel(TVector3(x, y, z), planeId, moduleId, channelId);

        if (daqId >= 0) {
            TRestReadoutPlane* plane = fReadout->GetReadoutPlaneWithID(planeId);

            Double_t energy = fHitsEvent->GetEnergy(hit);

            Double_t time = plane->GetDistanceTo(x, y, z) / fDriftVelocity + t;

            if (GetVerboseLevel() >= REST_Debug && hit < 20)
                cout << "Module : " << moduleId << " Channel : " << channelId << " daq ID : " << daqId
                     << endl;

            if (GetVerboseLevel() >= REST_Debug && hit < 20)
                cout << "Energy : " << energy << " time : " << time << endl;

            if (GetVerboseLevel() >= REST_Extreme && hit < 20)
                printf(
                    " TRestHitsToSignalProcess: x %lf y %lf z %lf energy %lf t %lf "
                    "fDriftVelocity %lf fSampling %lf time %lf\n",
                    x, y, z, energy, t, fDriftVelocity, fSampling, time);

            if (GetVerboseLevel() >= REST_Extreme)
                cout << "Drift velocity : " << fDriftVelocity << " mm/us" << endl;

            time = ((Int_t)(time / fSampling)) * fSampling;  // now time is in unit "us", but dispersed

            fSignalEvent->AddChargeToSignal(daqId, time, energy);
        } else {
            if (GetVerboseLevel() >= REST_Debug)
                cout << "readout channel not find for position (" << x << ", " << y << ", " << z << ")!"
                     << endl;
        }
    }

    fSignalEvent->SortSignals();

    if (GetVerboseLevel() >= REST_Debug) {
        cout << "TRestHitsToSignalProcess : Number of signals added : " << fSignalEvent->GetNumberOfSignals()
             << endl;
        cout << "TRestHitsToSignalProcess : Total signals integral : " << fSignalEvent->GetIntegral() << endl;
    }

    return fSignalEvent;
}

//______________________________________________________________________________
void TRestHitsToSignalProcess::EndProcess() {
    // Function to be executed once at the end of the process
    // (after all events have been processed)

    // Start by calling the EndProcess function of the abstract class.
    // Comment this if you don't want it.
    // TRestEventProcess::EndProcess();
}

//______________________________________________________________________________
void TRestHitsToSignalProcess::InitFromConfigFile() {
    fSampling = GetDblParameterWithUnits("sampling");
    // returned in REST standard unit: atm
    fGasPressure = GetDblParameterWithUnits("gasPressure", -1.);
    // convert REST standard unit "V/mm" to "V/cm"
    fElectricField = GetDblParameterWithUnits("electricField", -1.);
    // DONE : velocity units are implemented with standard unit "mm/us"
    fDriftVelocity = GetDblParameterWithUnits("driftVelocity", -1.);
}
