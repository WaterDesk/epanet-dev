#pragma once
#include <string>
#include <vector>
#include <unordered_map>

#include "project.h"

using namespace Epanet;
class Junction;

class DeltaDemandResult
{
public:
    std::string pressureTapName;
    double      baseHead;
    double      actualHead;
    double      deltaHead;
};

class PressureEvaluation
{
public:
    PressureEvaluation(const char* inpFile, const char* pressureNodeFile, const char* testNodesFile, const char* resultDir, const char* strDemandDelta, const char* strPressureDelta, bool bLogDetails);
    ~PressureEvaluation();

    int error() const;
    void doBaseEvaluation();
    void doAllEvaluation();

private:
    void doEvaluation(int index, double deltaDemand);
    void updateDemand(Junction* pJuction, double deltaDemand);
    void restoreDemand(Junction* pJuction, double originalDemand);
    void initPressureTapIndexes(const char* pressureNodeFile);
    void initTestNodeIndexes(const char* testNodesFile);
    void outputHeadDelta(const std::string& nodeName);
    void doPressureTapAnalysis();

private:
    Project     project;
    Network*    pNetwork;

    int         resultCount;
    std::vector<double> baseHeadResults;

    std::string strDeltaDemandFile;
    std::vector<std::string>    pressureTapNames;
    std::vector<int>            pressureTapIndexes;

    std::vector<std::string>    testNodeNames;
    std::vector<int>            testNodeIndexes;

    int         err;
    double      lcf;
    double      qcf;
    std::ofstream  m_resultFile;

    std::unordered_map<std::string, std::vector<DeltaDemandResult>>      nodeDeltaDemandResultTable;
    double      pressureDelta;
    double      demandDelta;
    std::unordered_map<std::string, std::vector<std::string>> impactedNodeNames;

    bool        m_bLogDetails;
};
