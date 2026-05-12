#include <TFile.h>
#include <TTree.h>

#include <iostream>
#include <fstream>
#include <string>
#include <limits>
#include <algorithm>
#include <cmath>

bool GetSelectedMinMax(TTree* tree,
                       const std::string& branchName,
                       const std::string& cutExpression,
                       double& minVal,
                       double& maxVal)
{
    Long64_t nEntries = tree->GetEntries();
    tree->SetEstimate(nEntries + 1);

    Long64_t nSelected =
        tree->Draw(branchName.c_str(), cutExpression.c_str(), "goff");

    if (nSelected <= 0) {
        return false;
    }

    double* values = tree->GetV1();

    minVal = std::numeric_limits<double>::max();
    maxVal = -std::numeric_limits<double>::max();

    bool foundFinite = false;

    for (Long64_t i = 0; i < nSelected; i++) {
        if (!std::isfinite(values[i])) continue;

        foundFinite = true;

        if (values[i] < minVal) minVal = values[i];
        if (values[i] > maxVal) maxVal = values[i];
    }

    return foundFinite;
}

void get_range(const char* mcFileName,
               const char* dataFileName,
               const char* mcTreeName,
               const char* dataTreeName,
               const char* branchName,
               const char* cutExpression,
               const char* outputRangeFile)
{
    TFile* mcFile = TFile::Open(mcFileName, "READ");
    TFile* dataFile = TFile::Open(dataFileName, "READ");

    if (!mcFile || mcFile->IsZombie()) {
        std::cerr << "Error: could not open MC file: "
                  << mcFileName << std::endl;
        return;
    }

    if (!dataFile || dataFile->IsZombie()) {
        std::cerr << "Error: could not open DATA file: "
                  << dataFileName << std::endl;
        return;
    }

    TTree* mcTree = (TTree*)mcFile->Get(mcTreeName);
    TTree* dataTree = (TTree*)dataFile->Get(dataTreeName);

    if (!mcTree) {
        std::cerr << "Error: could not find MC tree: "
                  << mcTreeName << std::endl;
        return;
    }

    if (!dataTree) {
        std::cerr << "Error: could not find DATA tree: "
                  << dataTreeName << std::endl;
        return;
    }

    if (!mcTree->GetBranch(branchName) && !mcTree->GetLeaf(branchName)) {
        std::cerr << "Error: branch not found in MC tree: "
                  << branchName << std::endl;
        return;
    }

    if (!dataTree->GetBranch(branchName) && !dataTree->GetLeaf(branchName)) {
        std::cerr << "Error: branch not found in DATA tree: "
                  << branchName << std::endl;
        return;
    }

    double mcMin;
    double mcMax;
    double dataMin;
    double dataMax;

    bool mcOK =
        GetSelectedMinMax(mcTree, branchName, cutExpression, mcMin, mcMax);

    bool dataOK =
        GetSelectedMinMax(dataTree, branchName, cutExpression, dataMin, dataMax);

    if (!mcOK) {
        std::cerr << "Error: no valid MC entries after cut." << std::endl;
        return;
    }

    if (!dataOK) {
        std::cerr << "Error: no valid DATA entries after cut." << std::endl;
        return;
    }

    double xMin = std::min(mcMin, dataMin);
    double xMax = std::max(mcMax, dataMax);

    if (xMin == xMax) {
        xMin -= 1.0;
        xMax += 1.0;
    }

    double margin = 0.05 * (xMax - xMin);
    xMin -= margin;
    xMax += margin;

    std::ofstream out(outputRangeFile);

    if (!out.is_open()) {
        std::cerr << "Error: could not write range file: "
                  << outputRangeFile << std::endl;
        return;
    }

    out << xMin << " " << xMax << std::endl;
    out.close();

    std::cout << "Range written to: " << outputRangeFile << std::endl;
    std::cout << "xMin = " << xMin << std::endl;
    std::cout << "xMax = " << xMax << std::endl;

    mcFile->Close();
    dataFile->Close();
}
