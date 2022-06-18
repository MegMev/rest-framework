#include <TApplication.h>
#include <TROOT.h>
#include <TRestMetadata.h>
#include <TRestRun.h>
#include <TRestTools.h>
#include <TRint.h>
#include <TSystem.h>

#include "TRestVersion.h"

using namespace std;

#ifdef WIN32
// in windows the pointer address from string conversion is without "0x", we must add
// the prefix so that ROOT can correctly initialize run/metadata objects
#define PTR_ADDR_PREFIX "0x"  
#else
#define PTR_ADDR_PREFIX ""
#endif  // WIN32

// Note!
// Don't use cout in the main function!
// This will make cout un-usable in the command line!
int main(int argc, char* argv[]) {
    // set the env and debug status
    setenv("REST_VERSION", REST_RELEASE, 1);

    Int_t loadMacros = 0;
    for (int i = 1; i < argc; i++) {
        char* c = &argv[i][0];
        if (*c == '-') {
            c++;
            if (*c == '-') c++;
            switch (*c) {
                case 'v':
                    gVerbose = StringToVerboseLevel(argv[i + 1]);
                    break;
                case 'm':
                    loadMacros = StringToInteger(argv[i + 1]);
                    break;
                case 'h':
                    // We use cout here since we will just exit afterwards
                    printf("\n");
                    printf("-----------------------------------\n");
                    printf("restRoot basic options description.\n");
                    printf("-----------------------------------\n");
                    printf("\n");
                    printf(" In order to define the verbosity of restRoot you may use:\n");
                    printf("\n");
                    printf(" restRoot --v [VERBOSE_LEVEL]\n");
                    printf("\n");
                    printf(" Where VERBOSE_LEVEL=0,1,2,3 is equivalent to silent, warning, info, debug\n");
                    printf("\n");
                    printf("-----\n");
                    printf("\n");
                    printf(" In order to decide if REST macros should be loadedd you may use:\n");
                    printf("\n");
                    printf(" restRoot --m [0,1]\n");
                    printf("\n");
                    printf(" Option 0 will disable macro loading. Option 1 is the default.\n");
                    printf("\n");
                    exit(0);
            }
        }
    }

    bool silent = false;
    if (gVerbose == TRestStringOutput::REST_Verbose_Level::REST_Silent) silent = true;

    bool debug = false;
    if (gVerbose >= TRestStringOutput::REST_Verbose_Level::REST_Debug) debug = true;

    // load rest library and macros
    TRestTools::LoadRESTLibrary(silent);

    gROOT->ProcessLine("#include <TRestStringHelper.h>");
    gROOT->ProcessLine("#include <TRestPhysics.h>");
    if (loadMacros) {
        if (!silent) printf("= Loading macros ...\n");
        auto macfiles = TRestTools::GetFilesMatchingPattern(REST_PATH + "/macros/*REST_*.C");
        for (auto macfile : macfiles) {
            if (macfile.find("swp") != string::npos || macfile.find("svn") != string::npos ||
                macfile.find("swo") != string::npos)
                continue;
            if (debug) printf("Loading macro : %s\n", macfile.c_str());

            gROOT->ProcessLine((".L " + macfile).c_str());
        }
    }

    // load input root file with TRestRun, initialize input event, analysis tree and metadata structures
    int Nfile = 0;
    for (int i = 1; i < argc; i++) {
        string opt = (string)argv[i];
        if (opt.find("http") != string::npos ||
            (TRestTools::fileExists(opt) && TRestTools::isRootFile(opt))) {
            printf("\nAttaching file %s as run%i...\n", opt.c_str(), Nfile);

            TRestRun* runTmp = new TRestRun(opt);
            string runcmd =
                Form("TRestRun* run%i = (TRestRun*)%s;", Nfile, (PTR_ADDR_PREFIX + ToString(runTmp)).c_str());
            if (debug) printf("%s\n", runcmd.c_str());
            gROOT->ProcessLine(runcmd.c_str());
            if (runTmp->GetInputEvent() != nullptr) {
                string eventType = runTmp->GetInputEvent()->ClassName();

                printf("Attaching event %s as ev%i...\n", eventType.c_str(), Nfile);
                string evcmd = Form("%s* ev%i = (%s*)%s;", eventType.c_str(), Nfile, eventType.c_str(),
                                    (PTR_ADDR_PREFIX + ToString(runTmp->GetInputEvent())).c_str());
                if (debug) printf("%s\n", evcmd.c_str());
                gROOT->ProcessLine(evcmd.c_str());
                runTmp->GetEntry(0);
            }

            // command line AnalysisTree object
            if (runTmp->GetAnalysisTree() != nullptr) {
                // if (runTmp->GetAnalysisTree()->GetChain() != nullptr) {
                //    printf("Attaching ana_tree%i...\n", Nfile);
                //    string evcmd = Form("TChain* ana_tree%i = (TChain*)%s;", Nfile,
                //        ToString(runTmp->GetAnalysisTree()->GetChain()).c_str());
                //    if (debug) printf("%s\n", evcmd.c_str());
                //    gROOT->ProcessLine(evcmd.c_str());
                //}
                // else
                //{
                printf("Attaching ana_tree%i...\n", Nfile);
                string evcmd = Form("TRestAnalysisTree* ana_tree%i = (TRestAnalysisTree*)%s;", Nfile,
                                    (PTR_ADDR_PREFIX + ToString(runTmp->GetAnalysisTree())).c_str());
                if (debug) printf("%s\n", evcmd.c_str());
                gROOT->ProcessLine(evcmd.c_str());
                // runTmp->GetEntry(0);
                //}
            }

            // command line EventTree object
            if (runTmp->GetEventTree() != nullptr) {
                printf("Attaching ev_tree%i...\n", Nfile);
                string evcmd = Form("TTree* ev_tree%i = (TTree*)%s;", Nfile,
                                    (PTR_ADDR_PREFIX + ToString(runTmp->GetEventTree())).c_str());
                if (debug) printf("%s\n", evcmd.c_str());
                gROOT->ProcessLine(evcmd.c_str());
            }

            printf("\n%s\n", "Attaching metadata structures...");
            Int_t Nmetadata = runTmp->GetNumberOfMetadataStructures();
            map<string, int> metanames;
            for (int n = 0; n < Nmetadata; n++) {
                string metaName = runTmp->GetMetadataStructureNames()[n];
                if (metaName.find("Historic") != -1) {
                    continue;
                }

                TRestMetadata* md = runTmp->GetMetadata(metaName);
                string metaType = md->ClassName();

                string metaFixed = Replace(metaName, "-", "_");
                metaFixed = Replace(metaFixed, " ", "");
                metaFixed = Replace(metaFixed, ".", "_");
                metaFixed = "md" + ToString(Nfile) + "_" + metaFixed;
                if (metanames.count(metaFixed) != 0) continue;
                metanames[metaFixed] = 0;
                printf("- %s (%s)\n", metaFixed.c_str(), metaType.c_str());

                string mdcmd = Form("%s* %s = (%s*)%s;", metaType.c_str(), metaFixed.c_str(),
                                    metaType.c_str(), (PTR_ADDR_PREFIX + ToString(md)).c_str());

                if (debug) printf("%s\n", mdcmd.c_str());

                gROOT->ProcessLine(mdcmd.c_str());

                // if (metaType == "TRestGas") {
                //    string gascmd = Form("%s->LoadGasFile();", metaFixed.c_str());
                //    gROOT->ProcessLine(gascmd.c_str());
                //}
            }

            argv[i] = (char*)"";
            Nfile++;
        } else if (TRestTools::isRootFile(opt)) {
            printf("\nFile %s not found ... !!\n", opt.c_str());
        }
    }

    // display root's command line
    TRint theApp("App", &argc, argv);
    theApp.Run();

    return 0;
}
