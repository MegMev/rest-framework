///______________________________________________________________________________
///______________________________________________________________________________
///______________________________________________________________________________
///
///
///             RESTSoft : Software for Rare Event Searches with TPCs
///
///             TRestAnalysisPlot.cxx
///
///             june 2016    Gloria Luzón/Javier Galan
///_______________________________________________________________________________

#include "TRestAnalysisPlot.h"
#include "TRestManager.h"
using namespace std;

#include <TLegend.h>
#include <TStyle.h>

#include <ctime>

ClassImp(TRestAnalysisPlot);
//______________________________________________________________________________
TRestAnalysisPlot::TRestAnalysisPlot() { Initialize(); }

TRestAnalysisPlot::TRestAnalysisPlot(const char* cfgFileName, const char* name) : TRestMetadata(cfgFileName) {
    Initialize();

    LoadConfigFromFile(fConfigFileName, name);
}

void TRestAnalysisPlot::Initialize() {
    SetSectionName(this->ClassName());

    fRun = NULL;

    fNFiles = 0;

    fCombinedCanvas = NULL;
}

//______________________________________________________________________________
TRestAnalysisPlot::~TRestAnalysisPlot() {
    if (fRun != NULL) delete fRun;
}

//______________________________________________________________________________
void TRestAnalysisPlot::InitFromConfigFile() {
    size_t position = 0;
    if (fHostmgr->GetRunInfo() != NULL) {
        fRun = fHostmgr->GetRunInfo();
    }

#pragma region ReadLabels
    debug << "TRestAnalysisPlot: Reading canvas settings" << endl;
    position = 0;
    string formatDefinition;
    if ((formatDefinition = GetKEYDefinition("labels", position)) != "") {
        if (GetVerboseLevel() >= REST_Debug) {
            cout << formatDefinition << endl;
            cout << "Reading format definition : " << endl;
            cout << "---------------------------" << endl;
        }

        fTicksScaleX = StringToDouble(GetFieldValue("ticksScaleX", formatDefinition));
        fTicksScaleY = StringToDouble(GetFieldValue("ticksScaleY", formatDefinition));

        fLabelScaleX = StringToDouble(GetFieldValue("labelScaleX", formatDefinition));
        fLabelScaleY = StringToDouble(GetFieldValue("labelScaleY", formatDefinition));

        fLabelOffsetX = StringToDouble(GetFieldValue("labelOffsetX", formatDefinition));
        fLabelOffsetY = StringToDouble(GetFieldValue("labelOffsetY", formatDefinition));

        if (fLabelOffsetX == -1) fLabelOffsetX = 1.1;
        if (fLabelOffsetY == -1) fLabelOffsetY = 1.3;

        if (fTicksScaleX == -1) fTicksScaleX = 1.5;
        if (fTicksScaleY == -1) fTicksScaleY = 1.5;

        if (fLabelScaleX == -1) fLabelScaleX = 1.3;
        if (fLabelScaleY == -1) fLabelScaleY = 1.3;

        if (GetVerboseLevel() >= REST_Debug) {
            cout << "ticks scale X : " << fTicksScaleX << endl;
            cout << "ticks scale Y : " << fTicksScaleY << endl;
            cout << "label scale X : " << fLabelScaleX << endl;
            cout << "label scale Y : " << fLabelScaleY << endl;
            cout << "label offset X : " << fLabelOffsetX << endl;
            cout << "label offset Y : " << fLabelOffsetY << endl;

            if (GetVerboseLevel() >= REST_Extreme) GetChar();
        }
    }
#pragma endregion

#pragma region ReadLegend
    position = 0;
    string legendDefinition;
    if ((legendDefinition = GetKEYDefinition("legendPosition", position)) != "") {
        if (GetVerboseLevel() >= REST_Debug) {
            cout << legendDefinition << endl;
            cout << "Reading legend definition : " << endl;
            cout << "---------------------------" << endl;
        }

        fLegendX1 = StringToDouble(GetFieldValue("x1", legendDefinition));
        fLegendY1 = StringToDouble(GetFieldValue("y1", legendDefinition));

        fLegendX2 = StringToDouble(GetFieldValue("x2", legendDefinition));
        fLegendY2 = StringToDouble(GetFieldValue("y2", legendDefinition));

        if (fLegendX1 == -1) fLegendX1 = 0.7;
        if (fLegendY1 == -1) fLegendY1 = 0.75;

        if (fLegendX2 == -1) fLegendX2 = 0.88;
        if (fLegendY2 == -1) fLegendY2 = 0.88;

        if (GetVerboseLevel() >= REST_Debug) {
            cout << "x1 : " << fLegendX1 << " y1 : " << fLegendY1 << endl;
            cout << "x2 : " << fLegendX2 << " y2 : " << fLegendY2 << endl;

            if (GetVerboseLevel() >= REST_Extreme) GetChar();
        }
    }
#pragma endregion

#pragma region ReadCanvas
    position = 0;
    string canvasDefinition;
    if ((canvasDefinition = GetKEYDefinition("canvas", position)) != "") {
        fCanvasSize = StringTo2DVector(GetFieldValue("size", canvasDefinition));
        fCanvasDivisions = StringTo2DVector(GetFieldValue("divide", canvasDefinition));
        fCanvasSave = GetFieldValue("save", canvasDefinition);
        if (fCanvasSave == "Not defined") {
            fCanvasSave = GetParameter("pdfFilename", "/tmp/restplot.pdf");
        }
    }
#pragma endregion

#pragma region ReadGlobalCuts
    debug << "TRestAnalysisPlot: Reading global cuts" << endl;
    vector<string> globalCuts;
    TiXmlElement* gCutele = fElement->FirstChildElement("globalCut");
    while (gCutele != NULL)  // general cuts
    {
        string cutActive = GetParameter("value", gCutele);

        if (ToUpper(cutActive) == "ON") {
            string obsName = GetParameter("name", gCutele);
            if (obsName == "Not defined")
                obsName = GetParameter("variable", gCutele);
            else {
                cout << "--W-- REST Warning. <globalCut name=\"var\" is now obsolete." << endl;
                cout << "--W-- Please, replace by : <globalCut variable=\"var\" " << endl;
                cout << endl;
            }

            string cutCondition = GetParameter("condition", gCutele);
            string cutString = obsName + cutCondition;

            globalCuts.push_back(cutString);
        }

        gCutele = gCutele->NextSiblingElement("globalCut");
    }
#pragma endregion

#pragma region ReadPlot
    debug << "TRestAnalysisPlot: Reading plot sections" << endl;
    Int_t maxPlots = (Int_t)fCanvasDivisions.X() * (Int_t)fCanvasDivisions.Y();
    TiXmlElement* plotele = fElement->FirstChildElement("plot");
    while (plotele != NULL) {
        string active = GetParameter("value", plotele, "ON");
        if (ToUpper(active) == "ON") {
            int N = fPlots.size();
            if (N >= maxPlots) {
                ferr << "Your canvas divisions (" << fCanvasDivisions.X() << " , " << fCanvasDivisions.Y()
                     << ") are not enough to show " << N + 1 << " plots" << endl;
                exit(1);
            }
            Plot_Info_Set plot;
            plot.name = RemoveWhiteSpaces(GetParameter("name", plotele, "plot_" + ToString(N)));
            plot.title = GetParameter("title", plotele, plot.name);
            plot.logY = StringToBool(GetParameter("logscale", plotele, "false"));
            plot.logX = false;
            plot.normalize = StringToDouble(GetParameter("norm", plotele, ""));
            plot.labelX = GetParameter("xlabel", plotele, "");
            plot.labelY = GetParameter("ylabel", plotele, "");
            plot.legendOn = StringToBool(GetParameter("legend", plotele, "OFF"));
            plot.staticsOn = StringToBool(GetParameter("stats", plotele, "OFF"));
            plot.save = RemoveWhiteSpaces(GetParameter("save", plotele, ""));

            TiXmlElement* histele = plotele->FirstChildElement("histo");
            if (histele == NULL) {
                // in case for single-hist plot, variables are added directly inside the <plot section
                histele = plotele;
            }
            while (histele != NULL) {
                Histo_Info_Set hist = SetupHistogramFromConfigFile(histele, plot);
                // add global cut
                for (unsigned int i = 0; i < globalCuts.size(); i++) {
                    if (i > 0 || hist.cutString != "") hist.cutString += " && ";
                    if (GetVerboseLevel() >= REST_Debug)
                        cout << "Adding global cut : " << globalCuts[i] << endl;
                    hist.cutString += globalCuts[i];
                }
                // add "SAME" option
                if (plot.histos.size() > 0) {
                    hist.drawOption += "SAME";
                }

                if (hist.plotString == "") {
                    warning << "No variables or histograms defined in the plot, skipping!" << endl;
                } else {
                    plot.histos.push_back(hist);
                }

                if (histele == plotele) {
                    break;
                }
                histele = histele->NextSiblingElement("histo");
            }

            fPlots.push_back(plot);
            plotele = plotele->NextSiblingElement("plot");
        }
    }

#pragma endregion
}

TRestAnalysisPlot::Histo_Info_Set TRestAnalysisPlot::SetupHistogramFromConfigFile(TiXmlElement* histele,
                                                                                  Plot_Info_Set plot) {
    Histo_Info_Set hist;
    hist.name = RemoveWhiteSpaces(GetParameter("name", histele, plot.name));
    hist.drawOption = GetParameter("option", histele, "colz");

    // 1. construct plot variables for the hist
    // read variables
    vector<string> varNames;
    vector<TVector2> ranges;
    vector<Int_t> bins;
    TiXmlElement* varele = histele->FirstChildElement("variable");
    while (varele != NULL) {
        varNames.push_back(GetParameter("name", varele));

        string rangeStr = GetParameter("range", varele);
        rangeStr = Replace(rangeStr, "unixTime", std::to_string(std::time(nullptr)));
        rangeStr = Replace(rangeStr, "days", "24*3600");
        ranges.push_back(StringTo2DVector(rangeStr));

        bins.push_back(StringToInteger(GetParameter("nbins", varele)));

        varele = varele->NextSiblingElement("variable");
    }
    if (GetVerboseLevel() >= REST_Debug) {
        for (unsigned int n = 0; n < bins.size(); n++) {
            cout << "Variable " << varNames[n] << endl;
            cout << "------------------------------------------" << endl;
            cout << "Plot range : ( " << ranges.back().X() << " , " << ranges.back().Y() << " ) " << endl;
            cout << "bins : " << bins.back() << endl;
            cout << endl;
        }
    }
    string pltString = "";
    for (unsigned int i = 0; i < varNames.size(); i++) {
        pltString += varNames[i];
        if (i < varNames.size() - 1) pltString += ":";
    }
    hist.plotString = pltString;

    // 2. construct plot name for the hist
    string rangestr = "";
    for (int i = ((int)bins.size()) - 1; i >= 0; i--) {
        // The range definitions are in reversed ordered. Compared to ROOT
        // variable definitions
        string binsStr = ToString(bins[i]);
        if (bins[i] == -1) binsStr = " ";

        string rXStr = ToString(ranges[i].X());
        if (ranges[i].X() == -1) rXStr = " ";

        string rYStr = ToString(ranges[i].Y());
        if (ranges[i].Y() == -1) rYStr = " ";

        if (i == (int)bins.size() - 1) rangestr += "(";

        rangestr += binsStr + " , " + rXStr + " , " + rYStr;
        if (i > 0) rangestr += ",";
        if (i == 0) rangestr += ")";
    }
    hist.range = rangestr;

    // 3. read cuts
    string cutString = "";
    Int_t n = 0;
    TiXmlElement* cutele = histele->FirstChildElement("cut");
    while (cutele != NULL) {
        string cutActive = GetParameter("value", cutele);
        if (ToUpper(cutActive) == "ON") {
            string cutVariable = GetParameter("variable", cutele);
            string cutCondition = GetParameter("condition", cutele);
            if (n > 0) cutString += " && ";
            if (GetVerboseLevel() >= REST_Debug)
                cout << "Adding local cut : " << cutVariable << cutCondition << endl;

            cutString += cutVariable + cutCondition;
            n++;
        }
        cutele = cutele->NextSiblingElement("cut");
    }
    hist.cutString = cutString;

    // 4. read classify condition
    hist.classifyMap.clear();
    TiXmlElement* classifyele = histele->FirstChildElement("classify");
    while (classifyele != NULL) {
        string Active = GetParameter("value", classifyele);
        if (ToUpper(Active) == "ON") {
            TiXmlAttribute* attr = classifyele->FirstAttribute();
            while (attr != NULL) {
                if (attr->Value() != NULL && string(attr->Value()) != "") {
                    hist.classifyMap[attr->Name()] = attr->Value();
                }
                attr = attr->Next();
            }
        }
        classifyele = classifyele->NextSiblingElement("classify");
    }

    // 5. read draw style(line color, width, fill style, etc.)
    hist.lineColor = StringToInteger(GetParameter("lineColor", histele));
    hist.lineWidth = StringToInteger(GetParameter("lineWidth", histele));
    hist.lineStyle = StringToInteger(GetParameter("lineStyle", histele));
    hist.fillStyle = StringToInteger(GetParameter("fillStyle", histele));
    hist.fillColor = StringToInteger(GetParameter("fillColor", histele));

    return hist;
}

void TRestAnalysisPlot::AddFile(TString fileName) {
    debug << "TRestAnalysisPlot::AddFile. Adding file. " << endl;
    debug << "File name: " << fileName << endl;

    TRestRun* run = new TRestRun((string)fileName);
    if (run->GetAnalysisTree() != NULL) {
        fRunInputFile.push_back(run);
        fNFiles++;
    }

    // TFile* f = new TFile(fileName);
    // TIter nextkey(f->GetListOfKeys());
    // TKey* key;
    // TString rTag = "notFound";
    // while ((key = (TKey*)nextkey())) {
    //    string kName = key->GetClassName();
    //    if (kName == "TRestRun") {
    //        rTag = ((TRestRun*)f->Get(key->GetName()))->GetRunTag();
    //        break;
    //    }
    //}
    // f->Close();

    // if (fClasifyBy == "runTag") {
    //    debug << "TRestAnalysisPlot::AddFile. Calling GetRunTagIndex. Tag = " << rTag << endl;
    //    Int_t index = GetRunTagIndex(rTag);
    //    debug << "Index. = " << index << endl;

    //    if (index < REST_MAX_TAGS) {
    //        fFileNames[index].push_back(fileName);
    //        fNFiles++;
    //    } else {
    //        ferr << "TRestAnalysisPlot::AddFile. Maximum number of tags per plot is : " << REST_MAX_TAGS
    //             << endl;
    //    }
    //} else if (fClasifyBy == "combineAll") {
    //    fFileNames[0].push_back(fileName);
    //    fNFiles++;
    //} else {
    //    warning << "TRestAnalysisPlot : fClassifyBy not recognized" << endl;

    //    fFileNames[0].push_back(fileName);
    //    fNFiles++;
    //}
}

// we can add input file from process's output file
void TRestAnalysisPlot::AddFileFromExternalRun() {
    if (fRun != NULL && fNFiles == 0) {
        if (fHostmgr->GetProcessRunner() != NULL && fRun->GetOutputFileName() != "") {
            // if we have a TRestProcessRunner before head, we use its output file
            AddFile(fRun->GetOutputFileName());
            return;
        } else if (fRun->GetInputFileNames().size() != 0) {
            // if we have only TRestRun, we ask for its input file list
            auto names = fRun->GetInputFileNames();
            for (int i = 0; i < names.size(); i++) {
                this->AddFile(names[i]);
            }
            return;
        }
    }
}

// we can add input file from parameter "inputFile"
void TRestAnalysisPlot::AddFileFromEnv() {
    if (fNFiles == 0) {
        string filepattern = GetParameter("inputFile", "");
        auto files = TRestTools::GetFilesMatchingPattern(filepattern);

        for (unsigned int n = 0; n < files.size(); n++) {
            essential << "Adding file : " << files[n] << endl;
            AddFile(files[n]);
        }
    }
}

Int_t TRestAnalysisPlot::GetPlotIndex(TString plotName) {
    for (unsigned int n = 0; n < fPlots.size(); n++)
        if (fPlots[n].name == plotName) return n;

    warning << "TRestAnalysisPlot::GetPlotIndex. Plot name " << plotName << " not found" << endl;
    return -1;
}

void TRestAnalysisPlot::PlotCombinedCanvas() {
    // Add files, first use <addFile section definition
    TiXmlElement* ele = fElement->FirstChildElement("addFile");
    while (ele != NULL) {
        TString inputfile = GetParameter("name", ele);
        this->AddFile(inputfile);
        ele = ele->NextSiblingElement("addFile");
    }
    // try to add files from external TRestRun handler
    if (fNFiles == 0) AddFileFromExternalRun();
    // try to add files from env "inputFile", which is set by --i argument
    if (fNFiles == 0) AddFileFromEnv();

    if (fNFiles == 0) {
        ferr << "TRestAnalysisPlot: No input files are added!" << endl;
        exit(1);
    }

    // initialize output root file if we have TRestRun running
    TFile* fOutputRootFile = NULL;
    if (fRun != NULL) {
        fOutputRootFile = fRun->GetOutputFile();
        if (fOutputRootFile == NULL) {
            fRun->SetHistoricMetadataSaving(false);
            fOutputRootFile = fRun->FormOutputFile();
        }
    }

    // Initializing canvas window
    if (fCombinedCanvas != NULL) {
        delete fCombinedCanvas;
        fCombinedCanvas = NULL;
    }
    fCombinedCanvas = new TCanvas("combined", "combined", 0, 0, fCanvasSize.X(), fCanvasSize.Y());
    fCombinedCanvas->Divide((Int_t)fCanvasDivisions.X(), (Int_t)fCanvasDivisions.Y());

    // Setting up TStyle
    TStyle* st = new TStyle();
    st->SetPalette(1);

    // start drawing
    vector<TH3F*> histCollectionAll;
    for (unsigned int n = 0; n < fPlots.size(); n++) {
        Plot_Info_Set plot = fPlots[n];

        TPad* targetPad = (TPad*)fCombinedCanvas->cd(n+1);
        targetPad->SetLogy(plot.logY);
        targetPad->SetLeftMargin(0.18);
        targetPad->SetRightMargin(0.1);
        targetPad->SetBottomMargin(0.15);
        targetPad->SetTopMargin(0.07);

        // draw to a new histogram
        vector<TH3F*> histCollectionPlot;
        for (unsigned int i = 0; i < plot.histos.size(); i++) {
            Histo_Info_Set hist = plot.histos[i];

            TString plotString = hist.plotString;
            TString nameString = hist.name;
            TString rangeString = hist.range;
            TString cutString = hist.cutString;
            TString optString = hist.drawOption;

            if (GetVerboseLevel() >= REST_Debug) {
                cout << endl;
                cout << "--------------------------------------" << endl;
                cout << "Plot string : " << plotString << endl;
                cout << "Plot name : " << nameString << endl;
                cout << "Plot range : " << rangeString << endl;
                cout << "Cut : " << cutString << endl;
                cout << "Plot option : " << optString << endl;
                cout << "++++++++++++++++++++++++++++++++++++++" << endl;
            }

			// draw single histo from different file
            for (unsigned int j = 0; j < fRunInputFile.size(); j++) {
				//apply "classify" condition 
                bool flag = true;
                auto iter = hist.classifyMap.begin();
                while (iter != hist.classifyMap.end()) {
                    if (fRunInputFile[j]->GetDataMemberValue(iter->first) != iter->second) {
                        flag = false;
                        break;
                    }
                    iter++;
                }
                if (!flag) continue;

                TRestAnalysisTree* tree = fRunInputFile[j]->GetAnalysisTree();
                int outVal;

                if (j == 0) {
                    outVal = tree->Draw(plotString + ">>" + nameString + rangeString, cutString, optString);
                } else {
                    outVal = tree->Draw(plotString + ">>+" + nameString, cutString, optString);
                }

                if (outVal == -1) {
                    ferr << endl;
                    ferr << "TRestAnalysisPlot::PlotCombinedCanvas. Plot string not properly constructed. "
                            "Does the analysis observable exist inside the file?"
                         << endl;
                    ferr << "Use \" restManager PrintTrees FILE.ROOT\" to get a list of "
                            "existing observables."
                         << endl;
                    ferr << endl;
                    exit(1);
                }
            }

			// adjust the histogram
            TH3F* htemp = (TH3F*)gPad->GetPrimitive(nameString);
            htemp->SetTitle(plot.title.c_str());
            htemp->SetStats(plot.staticsOn);

            htemp->GetXaxis()->SetTitle(plot.labelX.c_str());
            htemp->GetYaxis()->SetTitle(plot.labelY.c_str());

            htemp->GetXaxis()->SetLabelSize(fTicksScaleX * htemp->GetXaxis()->GetLabelSize());
            htemp->GetYaxis()->SetLabelSize(fTicksScaleY * htemp->GetYaxis()->GetLabelSize());
            htemp->GetXaxis()->SetTitleSize(fLabelScaleX * htemp->GetXaxis()->GetTitleSize());
            htemp->GetYaxis()->SetTitleSize(fLabelScaleY * htemp->GetYaxis()->GetTitleSize());
            htemp->GetXaxis()->SetTitleOffset(fLabelOffsetX * htemp->GetXaxis()->GetTitleOffset());
            htemp->GetYaxis()->SetTitleOffset(fLabelOffsetY * htemp->GetYaxis()->GetTitleOffset());
            htemp->GetXaxis()->SetNdivisions(-5);

            htemp->SetLineColor(hist.lineColor);
            htemp->SetLineWidth(hist.lineWidth);
            htemp->SetLineStyle(hist.lineStyle);
            htemp->SetFillColor(hist.fillColor);
            htemp->SetFillStyle(hist.fillStyle);

			histCollectionPlot.push_back(htemp);
        }

        // scale the histograms
        if (plot.normalize > 0) {
            for (unsigned int i = 0; i < histCollectionPlot.size(); i++) {
                Double_t scale = 1.;
                if (histCollectionPlot[i]->Integral() > 0) {
                    scale = plot.normalize / histCollectionPlot[i]->Integral();
                    histCollectionPlot[i]->Scale(scale);
                }
            }
        }

		// draw to the pad
        Double_t maxValue = 0;
        int maxID = 0;
        for (unsigned int i = 0; i < histCollectionPlot.size(); i++) {
            // need to draw the max histogram first, in order to prevent peak hidden problem
            Double_t value = histCollectionPlot[i]->GetBinContent(histCollectionPlot[i]->GetMaximumBin());
            if (i == 0) {
                maxValue = value;
            } else if (value > maxValue) {
                maxValue = value;
                maxID = i;
            }
        }
        histCollectionPlot[maxID]->Draw();
        for (unsigned int i = 0; i < histCollectionPlot.size(); i++) {
            // draw the remaining histo
            if (i != maxID)
                histCollectionPlot[i]->Draw("same");
        }



        // save histogram to root file
        for (unsigned int i = 0; i < histCollectionPlot.size(); i++) {
            if (fRun != NULL) {
                fOutputRootFile->cd();
                histCollectionPlot[i]->Write();
            }
        }

        // draw legend
		TLegend* legend = new TLegend(fLegendX1,fLegendY1,fLegendX2,fLegendY2);
        for (unsigned int i = 0; i < histCollectionPlot.size(); i++) {
            legend->AddEntry(histCollectionPlot[i], histCollectionPlot[i]->GetName(), "lf");
        }
        if (plot.legendOn) legend->Draw("same");

        // save pad
        targetPad->Update();
        if (plot.save != "") SavePlotToPDF(plot.save, n + 1);

        fCombinedCanvas->Update();
    }

    // Save canvas to a PDF file
    fCanvasSave = fRunInputFile[0]->FormFormat(fCanvasSave);
    if (fCanvasSave != "") fCombinedCanvas->Print(fCanvasSave);

    if (ToUpper(GetParameter("previewPlot", "TRUE")) == "TRUE") {
        GetChar();
    }

	// Save this class to the root file
    if (fRun != NULL && fOutputRootFile != NULL) {
        fOutputRootFile->cd();
        this->Write();
        fRun->CloseFile();
    }
}

void TRestAnalysisPlot::SaveCanvasToPDF(TString fileName) { fCombinedCanvas->Print(fileName); }

void TRestAnalysisPlot::SavePlotToPDF(TString fileName, Int_t n) {
    // gErrorIgnoreLevel = 10;

    if (n == 0) {
        fCombinedCanvas->Print(fileName);
        return;
    }

    TPad* pad = (TPad*)fCombinedCanvas->GetPad(n);

    TCanvas* c = new TCanvas(fPlots[n].name.c_str(), fPlots[n].name.c_str(), 800, 600);
    pad->DrawClone();

    c->Print(fileName);

    delete c;
    return;
}

void TRestAnalysisPlot::SaveHistoToPDF(TString fileName, Int_t nPlot, Int_t nHisto) {
    string name = fPlots[nPlot].histos[nHisto].name;
    TH3F* hist = (TH3F*)gPad->GetPrimitive(name.c_str());

    TCanvas* c = new TCanvas(name.c_str(), name.c_str(), 800, 600);

    hist->Draw();

    c->Print(fileName);

    delete c;
    return;
}

//
// void TRestAnalysisPlot::SaveHistoToPDF(TH1D* h, Int_t n, TString fileName) {
//    gErrorIgnoreLevel = 10;
//
//    TCanvas* c = new TCanvas(h->GetName(), h->GetTitle(), 800, 600);
//
//    h->Draw("colz");
//
//    h->SetTitle(fHistoTitle[n]);
//    h->GetXaxis()->SetTitle(fHistoXLabel[n]);
//    h->GetYaxis()->SetTitle(fHistoYLabel[n]);
//
//    h->Draw("colz");
//    c->Print(fileName);
//
//    delete c;
//}
