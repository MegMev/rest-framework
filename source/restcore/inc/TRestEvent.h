/*************************************************************************
 * This file is part of the REST software framework.                     *
 *                                                                       *
 * Copyright (C) 2016 GIFNA/TREX (University of Zaragoza)                *
 * For more information see http://gifna.unizar.es/trex                  *
 *                                                                       *
 * REST is free software: you can redistribute it and/or modify          *
 * it under the terms of the GNU General Public License as published by  *
 * the Free Software Foundation, either version 3 of the License, or     *
 * (at your option) any later version.                                   *
 *                                                                       *
 * REST is distributed in the hope that it will be useful,               *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the          *
 * GNU General Public License for more details.                          *
 *                                                                       *
 * You should have a copy of the GNU General Public License along with   *
 * REST in $REST_PATH/LICENSE.                                           *
 * If not, see http://www.gnu.org/licenses/.                             *
 * For the list of contributors see $REST_PATH/CREDITS.                  *
 *************************************************************************/

#ifndef RestCore_TRestEvent
#define RestCore_TRestEvent

#include <iostream>

#include "TObject.h"
#include <TString.h>
#include <TTimeStamp.h>
#include <TPad.h>

class TRestEvent:public TObject {

    protected:
        Int_t fRunOrigin;	        ///< Run ID number of the event
        Int_t fSubRunOrigin;	    ///< Sub-run ID number of the event
        Int_t fEventID;             ///< Event identificative number
        Int_t fSubEventID;          ///< Sub-Event identificative number
        TString fSubEventTag;       ///< A short length label to identify the sub-Event
        TTimeStamp fEventTime;      ///< Absolute event time
        Bool_t fOk;                 ///< Flag to be used by processes to define an event status. fOk=true is the default.

        #ifndef __CINT__

        TPad *fPad;
        #endif

        void RestartPad( Int_t nElements );


    public:
        //Setters
        void SetRunOrigin( Int_t id ) { fRunOrigin = id; }
        void SetSubRunOrigin( Int_t id ) { fSubRunOrigin = id; }

        void SetID( Int_t id ) { fEventID = id; }
        void SetSubID( Int_t id ) { fSubEventID = id; }
        void SetSubEventTag( TString tag ) { fSubEventTag = tag; }

        void SetTime( Double_t time );
        void SetTime( Double_t seconds, Double_t nanoseconds );
        void SetTimeStamp( TTimeStamp time ) { fEventTime = time; }

        void SetState( Bool_t state ) { fOk = state; }
        void SetOK( Bool_t state ) { fOk = state; }

        //Getters
        Int_t GetID( ) { return fEventID; }
        Int_t GetSubID( ) { return fSubEventID; }
        TString GetSubEventTag() { return fSubEventTag; }

        Int_t GetRunOrigin(  ) { return fRunOrigin; }
        Int_t GetSubRunOrigin(  ) { return fSubRunOrigin; }

        Double_t GetTime() { return fEventTime.AsDouble(); }
        TTimeStamp GetTimeStamp() { return fEventTime; }

        Bool_t isOk() { return fOk; }

        // Must be set on the derived events to remove content
        virtual void Initialize() = 0; 

        virtual void PrintEvent( );
        virtual void PrintEvent( int n )  { PrintEvent(); };

        virtual TPad *DrawEvent( TString option = "" ) = 0;

        //Construtor
        TRestEvent();
        //Destructor
        virtual ~ TRestEvent();

        ClassDef(TRestEvent, 1);     // REST event superclass
};
#endif
