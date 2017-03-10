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

#ifndef RestCore_TRestEventProcess
#define RestCore_TRestEventProcess

#include "TNamed.h"

#include "TRestEvent.h"
#include "TRestMetadata.h"
#include "TRestAnalysisTree.h"

class TRestEventProcess:public TRestMetadata {
	protected:
		Int_t fStatusOfProcess;	///< integer to hold the status of task: 0 = successful >0 = some error happened
		Int_t fVerbose;              ///<! Verbose level of the process execution [0...3] OBSOLETE!!

		virtual void InitFromConfigFile() = 0;

#ifndef __CINT__
		TRestEvent *fInputEvent;	///< Pointer to input event
		TRestEvent *fOutputEvent;    ///< Pointer to output event

		std::vector <TRestMetadata*> fRunMetadata; ///< Array to other metadata classes needed by the process

		std::vector <string> fObservableNames; ///< Array to observables names to be produced by the process

		TRestAnalysisTree *fAnalysisTree; ///< Pointer to analysis tree where to store the observables. 

 	        Bool_t fIsExternal; ///< It defines if the process reads event data from an external source.

  		TString fInputFileName;

		Bool_t fCreateCanvas;
#endif

        template <typename eventType> 
        void TransferEvent ( eventType *evOutput, eventType *evInput )
        { 
            if( evOutput != NULL ) 
            {
                delete evOutput;
                evOutput = NULL;
            }
            evOutput = (eventType *) evInput->Clone();
        } 

        public:
            virtual TRestEvent *GetInputEvent() { return fInputEvent; } ///< Get pointer to input event
            virtual TRestEvent *GetOutputEvent() { return fOutputEvent; } ///< Get pointer to output event

        virtual Bool_t OpenInputFile(TString fName);
	   TString GetInputFilename( ) { return fInputFileName; }

		virtual void InitProcess() { } ///< To be executed at the beginning of the run
		virtual TRestEvent *ProcessEvent( TRestEvent *evInput ) = 0; ///< Process one event
		virtual void EndProcess() { } ///< To be executed at the end of the run
		virtual void BeginOfEventProcess() { fOutputEvent->Initialize(); } ///< To be executed before processing event
		virtual void EndOfEventProcess() { } ///< To be executed after processing event

        virtual TString GetProcessName();

        virtual void LoadDefaultConfig();

        virtual void LoadConfig( std::string cfgFilename, std::string cfgName = "" );

        Bool_t isExternal( ) { return fIsExternal; } 

		void EnableCanvas() { fCreateCanvas = true; }
		Bool_t CreateCanvas() { return fCreateCanvas; }

		vector <string> ReadObservables( );

		TRestMetadata *GetGasMetadata( );
		TRestMetadata *GetReadoutMetadata( );
		TRestMetadata *GetGeant4Metadata( );
		TRestMetadata *GetDetectorSetup( );

		Double_t GetDoubleParameterFromClass( TString className, TString parName );
		Double_t GetDoubleParameterFromClassWithUnits( TString className, TString parName );

		virtual TRestMetadata *GetProcessMetadata() { return NULL; }
		void SetMetadata( std::vector <TRestMetadata*> meta ) { fRunMetadata = meta; }

		void SetAnalysisTree( TRestAnalysisTree *tree ) { fAnalysisTree = tree; }

		void BeginPrintProcess();
		void EndPrintProcess();

		//Getters
		Int_t GetStatus() { return fStatusOfProcess; }
	//	Int_t GetVerboseLevel() { return fVerbose; } 
		//Setters
		void SetVerboseLevel(Int_t verbose) { fVerbose = verbose; }

		//Constructor
		TRestEventProcess();
		//Destructor
		~TRestEventProcess();

		ClassDef(TRestEventProcess, 1);      // Base class for a REST process
};
#endif
