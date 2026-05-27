#include <TFile.h>
#include <TTree.h>
#include <TSystem.h>

#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>

void Fail(const std::string& message)
{
    std::cerr << "Error: " << message << std::endl;
    gSystem->Exit(1);
}

std::string QuoteCSV(const std::string& text)
{
    std::string out = "\"";

    for (char ch : text) {
        if (ch == '"') {
            out += "\"\"";
        } else {
            out += ch;
        }
    }

    out += "\"";
    return out;
}

Long64_t CountAfterCut(TTree* tree, const std::string& cutExpression)
{
    if (cutExpression.empty()) {
        return tree->GetEntries();
    }

    Long64_t nSelected = tree->GetEntries(cutExpression.c_str());

    if (nSelected < 0) {
        Fail("cut could not be evaluated: " + cutExpression);
    }

    return nSelected;
}

void count_events(const char* sample1FileName,
                  const char* sample2FileName,
                  const char* sample1TreeName,
                  const char* sample2TreeName,
                  const char* sample1Label,
                  const char* sample2Label,
                  const char* cutLabel,
                  const char* cutExpression,
                  const char* outputCsv)
{
    TFile* sample1File = TFile::Open(sample1FileName, "READ");
    TFile* sample2File = TFile::Open(sample2FileName, "READ");

    if (!sample1File || sample1File->IsZombie()) {
        Fail(std::string("could not open sample1 file: ") + sample1FileName);
    }

    if (!sample2File || sample2File->IsZombie()) {
        Fail(std::string("could not open sample2 file: ") + sample2FileName);
    }

    TTree* sample1Tree = (TTree*)sample1File->Get(sample1TreeName);
    TTree* sample2Tree = (TTree*)sample2File->Get(sample2TreeName);

    if (!sample1Tree) {
        Fail(std::string("could not find sample1 tree: ") + sample1TreeName);
    }

    if (!sample2Tree) {
        Fail(std::string("could not find sample2 tree: ") + sample2TreeName);
    }

    Long64_t sample1Total = sample1Tree->GetEntries();
    Long64_t sample2Total = sample2Tree->GetEntries();

    Long64_t sample1Survived = CountAfterCut(sample1Tree, cutExpression);
    Long64_t sample2Survived = CountAfterCut(sample2Tree, cutExpression);

    double sample1Efficiency = 0.0;
    double sample2Efficiency = 0.0;

    if (sample1Total > 0) {
        sample1Efficiency = static_cast<double>(sample1Survived) /
                            static_cast<double>(sample1Total);
    }

    if (sample2Total > 0) {
        sample2Efficiency = static_cast<double>(sample2Survived) /
                            static_cast<double>(sample2Total);
    }

    std::ofstream out(outputCsv);

    if (!out.is_open()) {
        Fail(std::string("could not create output CSV: ") + outputCsv);
    }

    out << "cut_label,cut_expression,sample,total_entries,surviving_entries,efficiency,efficiency_percent\n";

    out << QuoteCSV(cutLabel) << ","
        << QuoteCSV(cutExpression) << ","
        << QuoteCSV(sample1Label) << ","
        << sample1Total << ","
        << sample1Survived << ","
        << std::setprecision(10) << sample1Efficiency << ","
        << 100.0 * sample1Efficiency << "\n";

    out << QuoteCSV(cutLabel) << ","
        << QuoteCSV(cutExpression) << ","
        << QuoteCSV(sample2Label) << ","
        << sample2Total << ","
        << sample2Survived << ","
        << std::setprecision(10) << sample2Efficiency << ","
        << 100.0 * sample2Efficiency << "\n";

    out.close();

    std::cout << "\nEvent count summary" << std::endl;
    std::cout << "Cut label: " << cutLabel << std::endl;

    if (std::string(cutExpression).empty()) {
        std::cout << "Cut expression: none" << std::endl;
    } else {
        std::cout << "Cut expression: " << cutExpression << std::endl;
    }

    std::cout << "\n"
              << sample1Label << ": "
              << sample1Survived << " / " << sample1Total
              << " = " << 100.0 * sample1Efficiency << "% survived"
              << std::endl;

    std::cout << sample2Label << ": "
              << sample2Survived << " / " << sample2Total
              << " = " << 100.0 * sample2Efficiency << "% survived"
              << std::endl;

    std::cout << "\nSaved event counts to: " << outputCsv << std::endl;

    sample1File->Close();
    sample2File->Close();
}
