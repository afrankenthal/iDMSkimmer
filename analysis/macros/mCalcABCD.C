#include <algorithm>
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <string.h>
using std::cout, std::endl, std::map, std::vector;

#include <TApplication.h>
#include <TCanvas.h>
#include <TChain.h>
#include <TCollection.h>
#include <TCut.h>
#include <TDatime.h>
#include <TFile.h>
#include <TH1F.h>
#include <TH2D.h>
#include <THStack.h>
#include <TKey.h>
#include <TImage.h>
#include <TROOT.h>
#include <TString.h>
#include <TSystemDirectory.h>
#include <TSystemFile.h>

#include "../utils/common.h"
using namespace common;
#include "../utils/cxxopts.hpp"
#include "../utils/json.hpp"
using json = nlohmann::json;

namespace macro {

    extern "C" bool process([[maybe_unused]] map<TString, SampleInfo> samples, vector<CutInfo> cuts_info, json cfg) {

        // macro options
        vector<std::string> in_filenames = cfg["infilenames"].get<vector<std::string>>();
        if (in_filenames.size() == 0) {
            cout << "ERROR! No input filename. Exiting..." << endl;
            return 0;
        }

        TString in_filename = TString(in_filenames[0]);
        TFile * in_file = new TFile(in_filename, "READ");

        float lowX = cfg["lowX"].get<float>(), highX = cfg["highX"].get<float>();
        float lowY = cfg["lowY"].get<float>(), highY = cfg["highY"].get<float>();

        gROOT->SetBatch(kTRUE);

        for (auto && keyAsObj : *in_file->GetListOfKeys()) {
            auto key = (TKey*)keyAsObj;
            if (TString(key->GetClassName()) != "TCanvas") continue;
            TString canvas_name = TString(key->GetName());
            cout << "Processing " << canvas_name << ", class " << key->GetClassName() << endl;

            if (canvas_name.Contains("canvas2D")) {
                if (canvas_name.Contains("_sig_") || canvas_name.Contains("-BKG") || canvas_name.Contains("-DATA")) {
                    // Get TH2D
                    TString h_name = canvas_name;
                    h_name.Remove(0, 9); // remove "Canvas2D_" to get TH2D name
                    cout << h_name << endl;
                    TCanvas * c = (TCanvas*)in_file->Get(canvas_name);
                    TH2D * h = (TH2D*)c->FindObject(h_name);
                    if (canvas_name.Contains("zoomzoom"))
                        h->Rebin2D(2,2);
                    cout << "Integral with over+underflow: " << h->Integral(0, h->GetNbinsX()+1, 0, h->GetNbinsY()+1) << endl;
                    int lowXBin = h->GetXaxis()->FindBin(lowX), highXBin = h->GetXaxis()->FindBin(highX);
                    int lowYBin = h->GetYaxis()->FindBin(lowY), highYBin = h->GetYaxis()->FindBin(highY);
                    cout << "Low X bin: " << lowXBin << ", high X bin: " << highXBin << endl;
                    cout << "Low Y bin: " << lowYBin << ", high Y bin: " << highYBin << endl;
                    //if (lowXBin == 0) lowXBin = 1;
                    //if (lowYBin == 0) lowYBin = 1;
                    //if (highXBin == h->GetNbinsX()) highXBin--;
                    //if (highYBin == h->GetNbinsY()) highYBin--;
                    // X = dPhi, Y = vtx
                    // C: low dPhi, high vtx; B: high dPhi, low vtx; A: low dPhi, low vtx; D: high dPhi, high vtx
                    float A = h->Integral(lowXBin, highXBin-1, lowYBin, highYBin-1);
                    float B = h->Integral(highXBin, h->GetNbinsX()+1, lowYBin, highYBin-1);
                    float C = h->Integral(lowXBin, highXBin-1, highYBin, h->GetNbinsY()+1);
                    float D = h->Integral(highXBin, h->GetNbinsX()+1, highYBin, h->GetNbinsY()+1);
                    float C_pred = A*D/B;
                    float C_pred_syst_err = C_pred * sqrt(1/A + 1/B + 1/D);
                    float C_pred_stat_err = sqrt(C_pred);
                    cout << "A: " << A << " +/- " << sqrt(A) << endl;
                    cout << "B: " << B << " +/- " << sqrt(B) << endl;
                    cout << "C: " << C << " +/- " << sqrt(C) << endl;
                    cout << "D: " << D << " +/- " << sqrt(D) << endl;
                    cout << "A*D/B: " << C_pred << " +/- "
                        <<  C_pred_stat_err << " (stat.) +/- " << C_pred_syst_err << " (syst.)" << endl;
                    cout << "C: " << C << " +/- " << sqrt(C) << endl;
                    cout << endl;
                }
            }
        }

        gROOT->SetBatch(kFALSE);

        in_file->Close();
        return 0;
    }
} // namespace macro
