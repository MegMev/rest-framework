#ifndef RestCore_TRestProcessRunner
#define RestCore_TRestProcessRunner

#include "TRestEvent.h"
#include "TRestMetadata.h"
#include "TRestEventProcess.h"
#include "TRestExternalFileProcess.h"
#include "TRestRun.h"
#include "TRestAnalysisTree.h"
#include <mutex>
#include <thread>

class TRestThread;
class TRestManager;

class TRestProcessRunner :public TRestMetadata {
public:
	/// REST run class
	ClassDef(TRestProcessRunner, 1);

	TRestProcessRunner();
	~TRestProcessRunner();
	void Initialize();
	void BeginOfInit();
	Int_t ReadConfig(string keydeclare, TiXmlElement* e);
	void EndOfInit();

	//core functionality
	void ReadProcInfo();
	void RunProcess();
	Int_t GetNextevtFunc(TRestEvent* targetevt, TRestAnalysisTree* targettree);
	void FillThreadEventFunc(TRestThread* t);
	void WriteThreadFileFunc(TRestThread* t);
	void ConfigOutputFile();

	//tools
	void ResetRunTimes();
	TRestEventProcess* InstantiateProcess(TString type, TiXmlElement* ele);
	void PrintProcessedEvents(Int_t rateE);


	//getters and setters
	void SetHostmgr(TRestManager*m) { fHostmgr = m; }
	TRestEvent* GetInputEvent();
	TFile* GetTempOutputDataFile() { return fTempOutputDataFile; }
	string GetProcInfo(string infoname) { return ProcessInfo[infoname] == "" ? infoname : ProcessInfo[infoname]; }
	TRestAnalysisTree* GetAnalysisTree();
private:
	//global variable
	TRestManager *fHostmgr;//!
	TRestRun *fRunInfo;//!


	//variables get from TRestFiles
	TRestEvent *fInputEvent;//!


	//self variables for processing
	vector<TRestThread*> fThreads;//!
	TFile* fTempOutputDataFile;//!
	TRestAnalysisTree* fAnalysisTree;//!
	Int_t nBranches;
	Int_t fThreadNumber;
	Int_t fProcessNumber;
	Int_t firstEntry;
	Int_t eventsToProcess;
	Int_t fProcessedEvents;
	map<string, string> ProcessInfo;
	vector<string> fOutputItem;





};

#endif