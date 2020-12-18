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

#ifndef RestCore_TRestRawSignalShapingProcess
#define RestCore_TRestRawSignalShapingProcess

#include <TRestRawSignalEvent.h>

#include "TRestEventProcess.h"

//! A process to convolute the input raw signal event with a given input
//! response.
class TRestRawSignalShapingProcess : public TRestEventProcess {
   private:
    /// A pointer to the specific TRestRawSignalEvent input
    TRestRawSignalEvent* fInputSignalEvent;

    /// A pointer to the specific TRestRawSignalEvent output
    TRestRawSignalEvent* fOutputSignalEvent;

    void InitFromConfigFile();

    void Initialize();

    void LoadDefaultConfig();

   protected:
    // add here the members of your event process
    TString fResponseFilename;

    TString fShapingType;

    Double_t fShapingTime;
    Double_t fShapingGain;

   public:
    any GetInputEvent() { return fInputSignalEvent; }
    any GetOutputEvent() { return fOutputSignalEvent; }

    void InitProcess();
    TRestEvent* ProcessEvent(TRestEvent* eventInput);
    void EndProcess();

    void LoadConfig(std::string cfgFilename, std::string name = "");

    /// It prints out the process parameters stored in the metadata structure
    void PrintMetadata() {
        BeginPrintProcess();

        metadata << "Shaping type : " << fShapingType << endl;

        metadata << "Shaping time : " << fShapingTime << endl;

        metadata << "Amplitude gain : " << fShapingGain << endl;

        if (fShapingType == "responseFile") metadata << "Response file : " << fResponseFilename << endl;

        EndPrintProcess();
    }

    /// Returns a new instance of this class
    TRestEventProcess* Maker() { return new TRestRawSignalShapingProcess; }

    /// Returns the name of this process
    TString GetProcessName() { return (TString) "rawSignalShaping"; }

    TRestRawSignalShapingProcess();
    TRestRawSignalShapingProcess(char* cfgFileName);
    ~TRestRawSignalShapingProcess();

    ClassDef(TRestRawSignalShapingProcess, 2);  // Template for a REST "event process" class inherited from
                                                // TRestEventProcess
};
#endif
