///______________________________________________________________________________
///______________________________________________________________________________
///______________________________________________________________________________
///
///
///             RESTSoft : Software for Rare Event Searches with TPCs
///
///             TRestRawUSTCToSignalProcess.h
///
///             Template to use to design "event process" classes inherited from
///             TRestProcess
///             How to use: replace TRestRawUSTCToSignalProcess by your name,
///             fill the required functions following instructions and add all
///             needed additional members and funcionality
///
///             May 2017:   First concept
///                 Created as part of the conceptualization of existing REST
///                 software. Javier Galan
///_______________________________________________________________________________

#ifndef RestCore_TRestRawUSTCToSignalProcess
#define RestCore_TRestRawUSTCToSignalProcess

#include <map>
#include "TRestRawToSignalProcess.h"

//#define V3_Readout_Format_Long
#define V4_Readout_Format
//#define Incoherent_Event_Generation

#ifdef V3_Readout_Format_Long
#define DATA_SIZE 1048
#define DATA_OFFSET (DATA_SIZE - 512 * 2 - 4)
#define PROTOCOL_SIZE 4
#endif

#ifdef V3_Readout_Format_Short
#define DATA_SIZE 1040
#define DATA_OFFSET (DATA_SIZE - 512 * 2 - 4)
#define PROTOCOL_SIZE 4
#endif

#ifdef V4_Readout_Format
#define DATA_SIZE 1036
#define DATA_OFFSET 6
#define HEADER_SIZE 36
#define ENDING_SIZE 16
#define PROTOCOL_SIZE 4
#endif

struct USTCDataFrame {
    // a signal-level data frame
    // e.g.
    // EEEE | E0A0 | 246C0686 | 0001 | 2233 | (A098)(A09C)... | FFFF
    // header | board number | event time | channel id(0~63) | event id | [chip id
    // + data(0~4095)]*512 | ending
    USTCDataFrame() {
        boardId = 0;
        chipId = 0;
        readoutType = 0;
        eventTime = 0;
        channelId = 0;
        evId = -1;
        signalId = 0;
    }
    UChar_t data[1048];  // the size of a signal frame

    Int_t boardId;       // 0~n
    Int_t chipId;        // 0~3 aget number
    Int_t readoutType;   // 2:full readout  1:partial readout
    Long64_t eventTime;  // event time in na
    Int_t channelId;     // 0~63 channels
    Int_t evId;          // if equals -1, this data frame is used but have not been
                         // re-filled

    Int_t signalId;
    Int_t dataPoint[512];
};

class TRestRawUSTCToSignalProcess : public TRestRawToSignalProcess {
   private:
#ifndef __CINT__
    TRestRawSignal sgnl;  //!

    UChar_t fHeader[64];
    UChar_t fEnding[32];

    vector<vector<USTCDataFrame>> fEventBuffer;  //!
    int nBufferedEvent;                          //!
    int fCurrentFile = 0;                        //!
    int fCurrentEvent = -1;                      //!
    int fCurrentBuffer = 0;                      //!
    Long64_t fTimeOffset = 0;
    int fLastBufferedId = 0;  //!

    vector<int> errorevents;  //!
    int unknownerrors = 0;    //!

#endif

   public:
    void InitProcess();
    void Initialize();

    TRestEvent* ProcessEvent(TRestEvent* evInput);

    void EndProcess();

    bool FillBuffer();

    bool GetNextFrame(USTCDataFrame&);

    bool OpenNextFile(USTCDataFrame&);

    void FixToNextFrame(FILE* f);

    bool ReadFrameData(USTCDataFrame& Frame);

    bool AddBuffer(USTCDataFrame& Frame);

    void ClearBuffer();

    Bool_t EndReading();

    // Constructor
    TRestRawUSTCToSignalProcess();
    TRestRawUSTCToSignalProcess(char* cfgFileName);
    // Destructor
    ~TRestRawUSTCToSignalProcess();

    ClassDef(TRestRawUSTCToSignalProcess, 2);  // Template for a REST "event process" class inherited from
                                               // TRestEventProcess
};
#endif
