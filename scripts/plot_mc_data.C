#include <TFile.h>
#include <TH1D.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <TStyle.h>
#include <TSystem.h>
#include <TLatex.h>

#include <iostream>
#include <string>
#include <algorithm>

void Fail(const std::string& message)
{
    std::cerr << "Error: " << message << std::endl;
    gSystem->Exit(1);
}

TH1D* GetHistogramFlexible(TFile* file,
                           const char* primaryName,
                           const char* fallbackName,
                           const char* fileName)
{
    TH1D* h = (TH1D*)file->Get(primaryName);

    if (!h) {
        h = (TH1D*)file->Get(fallbackName);
    }

    if (!h) {
        std::string message = "could not find histogram ";
        message += primaryName;
        message += " or ";
        message += fallbackName;
        message += " in ";
        message += fileName;
        Fail(message);
    }

    return h;
}

void plot_mc_data(const char* sample1HistFileName,
                  const char* sample2HistFileName,
                  const char* branchName,
                  const char* sample1Label,
                  const char* sample2Label,
                  const char* cutLabel,
                  const char* cutExpression,
                  bool normalize,
                  const char* outputPng)
{
    // Silence unused-variable warnings if we do not draw the cut text.
    (void)cutLabel;

    TFile* sample1File = TFile::Open(sample1HistFileName, "READ");
    TFile* sample2File = TFile::Open(sample2HistFileName, "READ");

    if (!sample1File || sample1File->IsZombie()) {
        Fail(std::string("could not open sample1 histogram file: ") +
             sample1HistFileName);
    }

    if (!sample2File || sample2File->IsZombie()) {
        Fail(std::string("could not open sample2 histogram file: ") +
             sample2HistFileName);
    }

    /*
       Your current make_hist.C calls are:

           "MC"   -> histogram name hMC
           "DATA" -> histogram name hDATA

       The fallbacks hsample1 and hsample2 make the script safer if you later
       rename the samples in the Snakefile.
    */

    TH1D* hSample1Original =
        GetHistogramFlexible(sample1File, "hMC", "hsample1", sample1HistFileName);

    TH1D* hSample2Original =
        GetHistogramFlexible(sample2File, "hDATA", "hsample2", sample2HistFileName);

    TH1D* hSample1 = (TH1D*)hSample1Original->Clone("hSample1_plot");
    TH1D* hSample2 = (TH1D*)hSample2Original->Clone("hSample2_plot");

    hSample1->SetDirectory(0);
    hSample2->SetDirectory(0);

    sample1File->Close();
    sample2File->Close();

    if (normalize) {
        double integral1 = hSample1->Integral();
        double integral2 = hSample2->Integral();

        if (integral1 > 0.0) {
            hSample1->Scale(1.0 / integral1);
        }

        if (integral2 > 0.0) {
            hSample2->Scale(1.0 / integral2);
        }
    }

    gStyle->SetOptStat(0);

    hSample1->SetLineColor(kRed + 1);
    hSample1->SetLineWidth(2);
    hSample1->SetLineStyle(1);

    hSample2->SetLineColor(kBlue + 1);
    hSample2->SetLineWidth(2);
    hSample2->SetLineStyle(1);

    hSample1->SetTitle(branchName);
    hSample1->GetXaxis()->SetTitle(branchName);

    if (normalize) {
        hSample1->GetYaxis()->SetTitle("Normalized entries");
    } else {
        hSample1->GetYaxis()->SetTitle("Entries");
    }

    double maxY = std::max(hSample1->GetMaximum(), hSample2->GetMaximum());

    if (maxY > 0.0) {
        hSample1->SetMaximum(1.25 * maxY);
    }

    TCanvas* c = new TCanvas("c", branchName, 900, 700);

    hSample1->Draw("hist");
    hSample2->Draw("hist same");

    TLegend* leg = new TLegend(0.62, 0.74, 0.88, 0.88);
    leg->SetBorderSize(0);
    leg->SetFillStyle(0);
    leg->AddEntry(hSample1, sample1Label, "l");
    leg->AddEntry(hSample2, sample2Label, "l");
    leg->Draw();

    if (std::string(cutExpression).size() > 0) {
        TLatex latex;
        latex.SetNDC();
        latex.SetTextSize(0.030);
        latex.DrawLatex(0.15, 0.84, Form("Cut: %s", cutExpression));
    }

    c->SaveAs(outputPng);

    std::cout << "Saved: " << outputPng << std::endl;
}
