//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
/// \file radioactivedecay/rdecay01/src/EventAction.cc
/// \brief Implementation of the EventAction class
//
// $Id: EventAction.cc 68030 2013-03-13 13:51:27Z gcosmo $
// 
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#include "EventAction.hh"

#include "G4Event.hh"
#include <iomanip>

#include "Randomize.hh"

#include <TRestRun.h>

extern TRestRun *restRun;
extern TRestG4Metadata *restG4Metadata;
extern TRestG4Event *restG4Event;
extern TRestG4Event *subRestG4Event;
extern TRestG4Track *restTrack;

#include <fstream>
using namespace std;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

EventAction::EventAction()
:G4UserEventAction()
{
    restG4Metadata->isFullChainActivated();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

EventAction::~EventAction()
{
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void EventAction::BeginOfEventAction(const G4Event* evt)
{
    if( evt->GetEventID() % 10000 == 0 ) cout << "Starting event : " << evt->GetEventID() << endl;

    restTrack->Initialize();

    restG4Event->SetEventID( evt->GetEventID() );
    restG4Event->SetOK( true );
    time_t systime = time(NULL);

    restG4Event->SetEventTime( (Double_t) systime );

    // Defining if the hits in a given volume will be stored
    for( int i = 0; i < restG4Metadata->GetNumberOfActiveVolumes(); i++ )
    {
        Double_t rndNumber = G4UniformRand();

        if ( restG4Metadata->GetStorageChance(i) >= rndNumber )
             restG4Event->ActivateVolumeForStorage( i );
        else
             restG4Event->DisableVolumeForStorage( i );
    }

}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void EventAction::EndOfEventAction(const G4Event* evt)
{

    G4int evtNb = evt->GetEventID(); 
    if ( restG4Metadata->GetVerboseLevel() >= REST_Info ) cout << "End of event " << evtNb << endl;
    if( restG4Metadata->GetVerboseLevel() >= REST_Info ) 
    {
        restG4Event->PrintEvent();
        cout << "End of event : " << evtNb << endl;
    }

    Double_t totEnergy = restG4Event->GetSensitiveVolumeEnergy();

    Double_t minEnergy = restG4Metadata->GetMinimumEnergyStored();
    Double_t maxEnergy = restG4Metadata->GetMaximumEnergyStored();

    if ( maxEnergy == 0 ) maxEnergy = totEnergy + 1;

    if( totEnergy > minEnergy && totEnergy < maxEnergy )
    {
        SetTrackSubeventIDs();

        for( int subId = 0; subId < restG4Event->GetNumberOfSubEventIDTracks(); subId++ )
        {
            FillSubEvent( subId );

            if( subRestG4Event->GetTotalDepositedEnergy() > 0. )
                restRun->Fill();
        }
    }
}

void EventAction::FillSubEvent( Int_t subId )
{
    subRestG4Event->Initialize();
    subRestG4Event->ClearVolumes();

    subRestG4Event->SetEventID( restG4Event->GetEventID( ) );
    subRestG4Event->SetSubEventID( subId );

    subRestG4Event->SetPrimaryEventOrigin( restG4Event->GetPrimaryEventOrigin() );
    for( int n = 0; n < restG4Event->GetNumberOfPrimaries(); n++ )
    {
        subRestG4Event->SetPrimaryEventDirection( restG4Event->GetPrimaryEventDirection( n ) );
        subRestG4Event->SetPrimaryEventEnergy( restG4Event->GetPrimaryEventEnergy( n ) );
    }

    for( int n = 0; n < restG4Event->GetNumberOfActiveVolumes(); n++ )
    {
        subRestG4Event->AddActiveVolume();
        if( restG4Event->isVolumeStored( n )) subRestG4Event->ActivateVolumeForStorage( n );
        else subRestG4Event->DisableVolumeForStorage( n );
    }

    for( int n = 0; n < restG4Event->GetNumberOfTracks( ); n++ )
    {
        TRestG4Track *tck = restG4Event->GetTrack( n );

        if( tck->GetSubEventID() == subId )
            subRestG4Event->AddTrack( *tck );
    }

    if( restG4Metadata->isVolumeStored( restG4Metadata->GetSensitiveVolume() ) )
    {
        Int_t sensVolID = restG4Metadata->GetActiveVolumeID( restG4Metadata->GetSensitiveVolume() );

        subRestG4Event->SetSensitiveVolumeEnergy( subRestG4Event->GetEnergyDepositedInVolume( sensVolID ) );
    }

    Double_t minTimestamp = 0;
    if( subRestG4Event->GetNumberOfTracks() > 0 )
        minTimestamp = subRestG4Event->GetTrack(0)->GetGlobalTime();

    for( int n = 0; n < subRestG4Event->GetNumberOfTracks( ); n++ )
    {
        if( subRestG4Event->GetTrack(n)->GetGlobalTime() < minTimestamp )
            minTimestamp = subRestG4Event->GetTrack(n)->GetGlobalTime();
    }

    subRestG4Event->SetEventTimeStamp( minTimestamp );

    if( subId > 0 )
    {
        for( int n = 0; n < restG4Event->GetNumberOfTracks( ); n++ )
        {
            TRestG4Track *tck = restG4Event->GetTrack( n );

            if( tck->GetSubEventID() == subId-1 )
                if( tck->isRadiactiveDecay() ) subRestG4Event->SetSubEventTag( tck->GetParticleName() );
        }
    }
}

void EventAction::SetTrackSubeventIDs()
{
    Int_t nTracks = restG4Event->GetNumberOfTracks();

    Double_t timeDelay = restG4Metadata->GetSubEventTimeDelay() * REST_Units::s;

    vector <Double_t> fTrackTimestampList;
    fTrackTimestampList.clear();

    for( int n = 0; n < nTracks; n++ )
    {
        Double_t trkTime = restG4Event->GetTrack(n)->GetGlobalTime();

        Int_t Ifound = 0;
        for( unsigned int id = 0; id < fTrackTimestampList.size(); id++ )
            if( absDouble ( fTrackTimestampList[id] - trkTime ) < timeDelay ) Ifound = id;

        if ( Ifound == 0 ) fTrackTimestampList.push_back( trkTime );
    }

    for( unsigned int id = 0; id < fTrackTimestampList.size(); id++ )
    {
        for( int n = 0; n < nTracks; n++ )
        {
            Double_t trkTime = restG4Event->GetTrack(n)->GetGlobalTime();

            if( absDouble ( fTrackTimestampList[id] - trkTime ) < timeDelay ) { restG4Event->SetTrackSubEventID( n, id ); }
        }
    }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......


