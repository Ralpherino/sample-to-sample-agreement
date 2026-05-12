#include <TFile.h>
#include <TTree.h>
#include <TH1D.h>
#include <TNamed.h>

#include <iostream>
#include <fstream>
#include <string>

void make_hist(const char* sampleName,
               const char* inputFileName,
               const char* treeName,
               const char* branchName,
               const char* cutExpression,
               int nBins,
               const char* rangeFileName,
               const char* outputHistFileName)
{
    double xMin;
    double xMax;

    std::ifstream rangeFile(rangeFileName);

    if (!rangeFile.is_open()) {
        std::cerr << "Error: could not open range file: "
                  << rangeFileName << std::endl;
        return;
    }

    rangeFile >> xMin >> xMax;
    rangeFile.close();

    TFile* inputFile = TFile::Open(inputFileName, "READ");

    if (!inputFile || inputFile->IsZombie()) {
        std::cerr << "Error: could not open input file: "
                  << inputFileName << std::endl;
        return;
    }

    TTree* tree = (TTree*)inputFile->Get(treeName);

    if (!tree) {
        std::cerr << "Error: could not find tree: "
                  << treeName << std::endl;
        return;
    }

    if (!tree->GetBranch(branchName) && !tree->GetLeaf(branchName)) {
        std::cerr << "Error: branch not found: "
                  << branchName << std::endl;
        return;
    }

    TFile* outputFile = TFile::Open(outputHistFileName, "RECREATE");

    if (!outputFile || outputFile->IsZombie()) {
        std::cerr << "Error: could not create output file: "
                  << outputHistFileName << std::endl;
        return;
    }

    std::string histName = std::string("h") + sampleName;

    TH1D* h = new TH1D(histName.c_str(),
                       branchName,
                       nBins,
                       xMin,
                       xMax);

    h->Sumw2();

    std::string drawCommand =
        std::string(branchName) + ">>" + histName;

    tree->Draw(drawCommand.c_str(), cutExpression, "goff");

    h->Write();

    TNamed branchMeta("branch", branchName);
    TNamed cutMeta("cut", cutExpression);
    TNamed sampleMeta("sample", sampleName);

    branchMeta.Write();
    cutMeta.Write();
    sampleMeta.Write();

    outputFile->Close();
    inputFile->Close();

    std::cout << "Histogram written to: "
              << outputHistFileName << std::endl;
}
