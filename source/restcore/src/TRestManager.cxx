//////////////////////////////////////////////////////////////////////////
///
/// This class provides a runtime for other REST application class. Its
/// rml element is usually the root element in the file, inside which 
/// other REST application class elements are defined. TRestManager 
/// instantiates class objects according to the rml file and performs
/// sequential startup for them. Then it runs specific tasks, which is 
/// also defined following the applications, to do the jobs. The tasks
/// are handled by class TRestTask.
///
/// \class TRestManager
///
///--------------------------------------------------------------------------
/// 
/// RESTsoft - Software for Rare Event Searches with TPCs
///
/// History of developments:
///
/// 2014-june: First concept. As part of conceptualization of previous REST
///            code (REST v2)
///            Igor G. Irastorza
/// 
/// 2017-Aug:  Major change to xml reading and class startup procedure
///            Kaixiang Ni
///
/// <hr>
//////////////////////////////////////////////////////////////////////////

#include "TRestManager.h"
#include "TRestTask.h"
#include "TInterpreter.h"
#include "TSystem.h"

ClassImp(TRestManager);


TRestManager::TRestManager()
{
	Initialize();
}




TRestManager::~TRestManager()
{
}

///////////////////////////////////////////////
/// \brief Set the class name as section name during initialization.
///
void TRestManager::Initialize()
{
	SetSectionName(this->ClassName());

	fMetaObjects.clear();

	fMetaObjects.push_back(this);

}

/// we reorganize the element order
/// before (v2.1.6 style):
/// <root>
///   <globals/>
///   <TRestManager>
///      <addProcess/>
///      <addMetadata/>
///      <addTask/>
///   </TRestManager>
///   <TRestRun/>
/// </root>
/// after (v2.2 style):
/// <TRestManager>
///   <globals/>
///   <TRestRun/>
///      <addMetadata/>
///   </TRestRun>
///   <TRestProcessRunner>
///      <addProcess/>
///   </TRestProcessRunner>
///   <addTask/>
/// </TRestManager>
void TRestManager::BeginOfInit() {

	if (fElement->FirstChildElement("TRestRun") == NULL &&
		fElementGlobal != NULL && fElementGlobal->Parent() != NULL &&
		fElementGlobal->Parent()->FirstChildElement("TRestRun") != NULL)
	{
		cout << "old style config file of TRestManager is detected!" << endl;
		cout << "trying to re-arranage the xml element..." << endl;
		cout << endl;

		debug << "switch position of the elements" << endl;
		fElement = (TiXmlElement*)fElementGlobal->Parent();
		fElement->SetValue("TRestManager");
		TiXmlElement* TRestProcessRunnerElement = fElement->FirstChildElement("TRestManager");
		TRestProcessRunnerElement->SetValue("TRestProcessRunner");
		TiXmlElement* TRestRunElement_old = fElement->FirstChildElement("TRestRun");
		fElement->InsertBeforeChild(TRestProcessRunnerElement, *TRestRunElement_old);
		fElement->RemoveChild(TRestRunElement_old);
		TiXmlElement* TRestRunElement = fElement->FirstChildElement("TRestRun");



		debug << "handle \"addProcess\"" << endl;
		TString processFile = GetParameter("processesFile", TRestProcessRunnerElement, "");
		TRestProcessRunnerElement->RemoveChild(GetElementWithName("parameter", "processesFile", TRestProcessRunnerElement));
		TiXmlElement* addProcessElement = TRestProcessRunnerElement->FirstChildElement("addProcess");
		while (addProcessElement != NULL) {
			addProcessElement->SetAttribute("file", processFile);
			addProcessElement = addProcessElement->NextSiblingElement("addProcess");
		}

		debug << "handle \"addMetadata\"" << endl;
		TiXmlElement* addMetadataElement = TRestProcessRunnerElement->FirstChildElement("addMetadata");
		while (addMetadataElement != NULL) {
			if (addMetadataElement->Attribute("type") != NULL && (string)addMetadataElement->Attribute("type") == "TRestReadout") {
				TString readoutFile = GetParameter("readoutFile", TRestProcessRunnerElement, "");
				if (readoutFile != "")
				{
					addMetadataElement->SetAttribute("file", readoutFile);
					TRestProcessRunnerElement->RemoveChild(GetElementWithName("parameter", "readoutFile", TRestProcessRunnerElement));
				}
			}
			if (addMetadataElement->Attribute("type") != NULL && (string)addMetadataElement->Attribute("type") == "TRestGas") {
				TString gasFile = GetParameter("gasFile", TRestProcessRunnerElement, "");
				if (gasFile != "")
				{
					addMetadataElement->SetAttribute("file", gasFile);
					TRestProcessRunnerElement->RemoveChild(GetElementWithName("parameter", "gasFile", TRestProcessRunnerElement));
				}
			}
			TRestRunElement->InsertEndChild(*addMetadataElement);
			TiXmlElement* tempelementptr = addMetadataElement;
			addMetadataElement = addMetadataElement->NextSiblingElement("addMetadata");
			TRestProcessRunnerElement->RemoveChild(tempelementptr);
		}

		debug << "handle \"addTask\"" << endl;
		TiXmlElement* addTaskElement = TRestProcessRunnerElement->FirstChildElement("addTask");
		while (addTaskElement != NULL) {

			fElement->InsertEndChild(*addTaskElement);
			TiXmlElement* tempelementptr = addTaskElement;
			addTaskElement = addTaskElement->NextSiblingElement("addTask");
			TRestProcessRunnerElement->RemoveChild(tempelementptr);
		}

		debug << "update parameters" << endl;
		//add parameter "inputAnalysis" for the TRestProcessRunnerElement
		TiXmlElement*parele = new TiXmlElement("prarmeter");
		parele->SetAttribute("name", "inputAnalysis");
		parele->SetAttribute("value", "on");
		TRestProcessRunnerElement->InsertBeforeChild(TRestProcessRunnerElement->FirstChildElement(), *parele);

		//some parameters should be translated
		TString analysisString = GetParameter("pureAnalysisOutput", TRestProcessRunnerElement, "OFF");
		if (analysisString == "ON" || analysisString == "On" || analysisString == "on")
		{
			parele->SetAttribute("name", "inputEvent");
			parele->SetAttribute("value", "off");
			TRestProcessRunnerElement->InsertBeforeChild(TRestProcessRunnerElement->FirstChildElement(), *parele);

			parele->SetAttribute("name", "outputEvent");
			parele->SetAttribute("value", "off");
			TRestProcessRunnerElement->InsertBeforeChild(TRestProcessRunnerElement->FirstChildElement(), *parele);
		}
		else
		{
			parele->SetAttribute("name", "inputEvent");
			parele->SetAttribute("value", "on");
			TRestProcessRunnerElement->InsertBeforeChild(TRestProcessRunnerElement->FirstChildElement(), *parele);

			parele->SetAttribute("name", "outputEvent");
			parele->SetAttribute("value", "on");
			TRestProcessRunnerElement->InsertBeforeChild(TRestProcessRunnerElement->FirstChildElement(), *parele);
		}
		TiXmlElement* inputFileElement = GetElementWithName("parameter", "inputFile", TRestProcessRunnerElement);
		if (inputFileElement != NULL) {
			TRestRunElement->InsertBeforeChild(TRestRunElement->FirstChildElement(),*inputFileElement);
			TRestProcessRunnerElement->RemoveChild(inputFileElement);
		}


		if (TRestProcessRunnerElement->FirstChildElement("readoutPlane") != NULL) {

			warning << "REST WARNING. TRestManager. Readout plane re-definition is not supported currently!" << endl;

		}

		if (fVerboseLevel >= REST_Debug) {
			cout << "updated TRestManager section:" << endl;
			fElement->Print(stdout, 0);
			cout << endl;
			GetChar();
		}









		
	}



}

///////////////////////////////////////////////
/// \brief Respond to the input xml element.
///
/// If the declaration of the input element is:
/// 1. TRestXXX: Startup the TRestXXX class with this xml element(sequential startup).
/// 2. AddTask: Do some special operations for the managed application TRestMetadata class
///
/// Suppported tasks:
/// 1. processEvents, analysisPlot, saveMetadata: directly do the jobs
/// 2. name of a REST macro: instaintate TRestTask to parse it, then run this TRestTask.
/// 3. C++ style command: call gInterpreter to execute it.
/// Other types of declarations will be omitted.
///
Int_t TRestManager::ReadConfig(string keydeclare, TiXmlElement* e)
{
	//if (keydeclare == "TRestRun") {
	//	TRestRun* fRunInfo = new TRestRun();
	//	fRunInfo->SetHostmgr(this);
	//	fRunInfo->LoadConfigFromFile(e, fElementGlobal);
	//	fMetaObjects.push_back(fRunInfo);
	//	gROOT->Add(fRunInfo);
	//	return 0;
	//}

	//else if (keydeclare == "TRestProcessRunner") {
	//	TRestProcessRunner* fProcessRunner = new TRestProcessRunner();
	//	fProcessRunner->SetHostmgr(this);
	//	fProcessRunner->LoadConfigFromFile(e, fElementGlobal);
	//	fMetaObjects.push_back(fProcessRunner);
	//	gROOT->Add(fProcessRunner);
	//	return 0;
	//}

	//cout << "----------- " << gROOT->FindObject("SJTU_Proto") << endl;

	if (Count(keydeclare, "TRest") > 0)
	{
		TClass*c = TClass::GetClass(keydeclare.c_str());
		if (c == NULL) {
			cout << " " << endl;
			cout << "REST ERROR. Class : " << keydeclare << " not found!!" << endl;
			cout << "This class will be skipped." << endl;
			return -1;
		}
		TRestMetadata*meta = (TRestMetadata*)c->New();
		meta->SetHostmgr(this);
		meta->SetConfigFile(fConfigFileName);
		meta->LoadConfigFromFile(e, fElementGlobal);
		fMetaObjects.push_back(meta);

		return 0;
	}

	else if (keydeclare == "addTask") {
		string active = GetParameter("value", e, "");
		if (active != "ON" && active != "On" && active != "on") {
			debug << "Inactived task : \"" << ElementToString(e) << "\"" << endl;
			return 0;
		}
		debug << "Loading Task...";

		const char* type = e->Attribute("type");
		const char* cmd = e->Attribute("command");
		if (type == NULL && cmd == NULL) {
			warning << "command or type should be given!" << endl;
			return -1;
		}
		if (type != NULL) {
			debug << " \"" << type << "\" " << endl;
			if ((string)type == "processEvents")
			{
				auto pr = GetProcessRunner();
				if (pr != NULL)
					pr->RunProcess();
			}
			else if ((string)type == "analysisPlot")
			{
				auto ap = GetAnaPlot();
				if (ap != NULL)
					ap->PlotCombinedCanvas();
			}
			else if ((string)type == "saveMetadata")
			{
				auto ri = GetRunInfo();
				if (ri != NULL) {
					ri->FormOutputFile();
					ri->CloseFile();
				}
			}
			else
			{
				TRestTask*tsk = TRestTask::GetTask(type);
				if (tsk == NULL) {
					warning << "REST ERROR. Task : " << type << " not found!!" << endl;
					warning << "This task will be skipped." << endl;
					return -1;
				}
				tsk->LoadConfigFromFile(e, fElementGlobal);
				tsk->RunTask(this);
				return 0;
			}
		}
		else if (cmd != NULL) {
			debug << " \"" << cmd << "\" " << endl;

			TRestTask*tsk = TRestTask::ParseCommand(cmd);
			if (tsk == NULL) {
				warning << "REST ERROR. Command : " << cmd << " cannot be parsed!!" << endl;
				warning << "This task will be skipped." << endl;
				return -1;
			}
			tsk->RunTask(this);
			return 0;
		}
	}


	return -1;
}

void TRestManager::InitFromTask(string taskName, vector<string> arguments) {

	TRestTask*tsk = TRestTask::GetTask(taskName);
	if (tsk == NULL) {
		cout << "REST ERROR. Task : " << taskName << " not found!!" << endl;
		gSystem->Exit(-1);
	}
	tsk->SetArgumentValue(arguments);
	tsk->RunTask(NULL);
	gSystem->Exit(0);
}


///////////////////////////////////////////////
/// \brief Get the application metadata class, according to the type
///
TRestMetadata* TRestManager::GetApplication(string type)
{
	for (int i = 0; i < fMetaObjects.size(); i++)
	{
		if ((string)fMetaObjects[i]->ClassName() == type)
		{
			return fMetaObjects[i];
		}
	}
	return NULL;
}

///////////////////////////////////////////////
/// \brief Get the application metadata class, according to the name
///
TRestMetadata* TRestManager::GetApplicationWithName(string name)
{
	for (int i = 0; i < fMetaObjects.size(); i++)
	{
		if (fMetaObjects[i]->GetName() == name)
		{
			return fMetaObjects[i];
		}
	}
	return NULL;
}


///////////////////////////////////////////////
/// \brief PrintMetadata of this class
///
/// Not implemented.
///
void TRestManager::PrintMetadata()
{
	return;
}


