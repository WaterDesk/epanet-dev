#pragma once
#include <string>
#include <vector>

#include "project.h"

using namespace Epanet;
class Junction;

class PressureEvaluation
{
public:
    PressureEvaluation(const char* inpFile, const char* rptFile, const char* cydFile);
    ~PressureEvaluation();

    int error() const;
    void doBaseEvaluation();
    void doAllEvaluation();

private:
    void doEvaluation(int index, double deltaDemand);
    void updateDemand(Junction* pJuction, double deltaDemand);
    void restoreDemand(Junction* pJuction, double originalDemand);
    void initPressureTapIndexes();
    void outputHeadDelta();

private:
    Project     project;
    Network*    pNetwork;

    int         resultCount;
    std::vector<double> baseHeadResults;

    std::string strPressureTapFile;
    std::string strDeltaDemandFile;
    std::vector<std::string> pressureTapNames;
    std::vector<int>    pressureTapIndexes;
    std::vector<double> deltaDemands;
    int         err;
    double      lcf;
    double      qcf;
    std::ofstream  resultFile;
};
