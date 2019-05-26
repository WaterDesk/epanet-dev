#include "PressureEvaluation.h"
#include "node.h"
#include "junction.h"
#include "Utilities/utilities.h"

#include <time.h>
#include <iostream>
#include <fstream>
#include <iomanip>


PressureEvaluation::PressureEvaluation(const char* inpFile, const char* pressureNodeFile, const char* testNodesFile, const char* resultDir, const char* strDemandDelta, const char* strPressureDelta, bool bLogDetails)
{
    Utilities::parseNumber(strDemandDelta, demandDelta);
    Utilities::parseNumber(strPressureDelta, pressureDelta);
    m_bLogDetails = bLogDetails;

    std::string dir = resultDir;
    if (dir[dir.length() - 1] != '\\')
        dir += '\\';

    std::string rptFile = dir + "report_" + strDemandDelta + "_" + strPressureDelta + ".rpt";
    std::string strResultFile = dir + "PressureEvaluationResult_" + strDemandDelta + "_" + strPressureDelta + ".txt";
    std::string strLogFile = dir + "PressureEvaluationResult_" + strDemandDelta + "_" + strPressureDelta + "_log.txt";

    // ... declare a Project variable and an error indicator
    err = 0;

    // ... initialize execution time clock

    // ... open the command line files and load network data
    if ((err = project.openReport(rptFile.c_str()))) return ;
    std::cout << "\n    Reading input file ...";
    if ((err = project.load(inpFile))) return ;
    //if ((err = p.openOutput(outFile))) return err;
    project.writeSummary();

    pNetwork = project.getNetwork();
    lcf = pNetwork->ucf(Units::LENGTH);
    qcf = pNetwork->ucf(Units::FLOW);
    resultCount = (int)pNetwork->nodes.size();
    baseHeadResults.reserve(resultCount);

    m_resultFile.open(strResultFile);
    m_logFile.open(strLogFile);

    initPressureTapIndexes(pressureNodeFile);
    if (testNodesFile != nullptr)
    {
        initTestNodeIndexes(testNodesFile);
    }
}

PressureEvaluation::~PressureEvaluation()
{
    m_resultFile.close();
    m_logFile.close();
}

int PressureEvaluation::error() const
{
    return err;
}

void PressureEvaluation::doBaseEvaluation()
{
    // ... initialize the solver
    std::cout << "\n    Initializing solver ...";
    if ((err = project.initSolver(false))) return ;
    std::cout << "\n    ";

    int t = 0;
    err = project.runSolver(&t);

    // output node head
    baseHeadResults.reserve(pNetwork->nodes.size());
    for (Node* node : pNetwork->nodes)
    {
        double head = node->head * lcf;
        baseHeadResults.push_back(head);
    }
}

void PressureEvaluation::doAllEvaluation()
{
    doBaseEvaluation();

    clock_t start_t = clock();

    if (testNodeIndexes.size() == 0)
    {
        int nodeCount = (int)pNetwork->nodes.size();
        for (int i = 0; i < nodeCount; i++)
        {
            doEvaluation(i, demandDelta);
            if (i % 100 == 0)
            {
                clock_t end_t = clock();
                double cpu_t = ((double)(end_t - start_t)) / CLOCKS_PER_SEC;
                std::stringstream ss;
                if (cpu_t < 0.001)
                    ss << "< 0.001 sec.";
                else
                    ss << std::setprecision(3) << cpu_t << " sec.";

                std::cout << "do evaluation: " << i << " takes " << ss.str() << "\n";

                start_t = end_t;
            }
        }
    }
    else
    {
        for (size_t i = 0; i < testNodeIndexes.size(); i++)
        {
            int nodeIndex = testNodeIndexes[i];
            if (nodeIndex != -1)
                doEvaluation(nodeIndex, demandDelta);

            if (i % 100 == 0)
            {
                clock_t end_t = clock();
                double cpu_t = ((double)(end_t - start_t)) / CLOCKS_PER_SEC;
                std::stringstream ss;
                if (cpu_t < 0.001)
                    ss << "< 0.001 sec.";
                else
                    ss << std::setprecision(3) << cpu_t << " sec.";

                std::cout << "do evaluation: " << i << " takes " << ss.str() << "\n";

                start_t = end_t;
            }
        }
    }

    doPressureTapAnalysis();
}

void PressureEvaluation::doEvaluation(int nodeIndex, double deltaDemand)
{
    // close hydraulic engine
    project.closeEngines();
    pNetwork->resetData();

    // update demand

    Node* pTestNode = pNetwork->nodes[nodeIndex];
    if (Node::JUNCTION != pTestNode->type())
        return;
    Junction* pJuction = dynamic_cast<Junction*>(pTestNode);
    if (pJuction == nullptr)
        return;

    if (m_bLogDetails)
        m_logFile << " #### compute again:  " << pTestNode->name << "\n";


    //double originalDemand = pJuction->primaryDemand.baseDemand;
    updateDemand(pJuction, deltaDemand);

    // do simulation
    if ((err = project.initSolver(false)))
    {
        restoreDemand(pJuction, deltaDemand);
        return;
    }
    int t = 0;
    err = project.runSolver(&t);

    // output result
    outputHeadDelta(pTestNode->name);

    // rollback the demand
    restoreDemand(pJuction, deltaDemand);
}

void PressureEvaluation::updateDemand(Junction* pJuction, double deltaDemand)
{
    if (pJuction->demands.size() > 0)
    {
        pJuction->demands.begin()->baseDemand += deltaDemand / qcf;
    }

    //pJuction->primaryDemand.baseDemand += deltaDemand;
    //pJuction->demands.clear();
    //pJuction->demands.push_back(pJuction->primaryDemand);
    //for (Demand& demand : pJuction->demands)
    //{
    //    demand.baseDemand /= qcf;
    //}
}

void PressureEvaluation::restoreDemand(Junction* pJuction, double deltaDemand)
{
    if (pJuction->demands.size() > 0)
    {
        pJuction->demands.begin()->baseDemand -= deltaDemand / qcf;
    }

    //pJuction->primaryDemand.baseDemand = originalDemand;
    //pJuction->demands.clear();
    //pJuction->demands.push_back(pJuction->primaryDemand);
    //for (Demand& demand : pJuction->demands)
    //{
    //    demand.baseDemand /= qcf;
    //}
}

void PressureEvaluation::initPressureTapIndexes(const char* pressureNodeFile)
{
    std::ifstream fin(pressureNodeFile, std::ios::in);
    for (;;)
    {
        std::string line;
        getline(fin, line, '\n');
        if (fin.fail())
        {
            if (fin.eof()) break;    // end of file reached - normal termination
        }

        std::string s1;
        int i = 0;
        for (; i < line.length(); i++)
        {
            char chLine = line[i];
            if (chLine == ' ' || chLine == '\t')
            {
                if (s1.empty())
                    continue;
                else
                    break;
            }
            else
            {
                s1 += chLine;
            }
        }

        if (s1.empty())
            continue;

        pressureTapNames.push_back(s1);
    }

    pressureTapIndexes.clear();
    pressureTapIndexes.reserve(pressureTapNames.size());
    for (std::string name : pressureTapNames)
    {
        int index = -1;
        for (size_t i = 0; i < pNetwork->nodes.size(); i++)
        {
            Node* node = pNetwork->nodes[i];
            if (node->name.compare(name) == 0)
            {
                index = (int)i;
                break;
            }
        }
        pressureTapIndexes.push_back(index);
    }
}

void PressureEvaluation::initTestNodeIndexes(const char* testNodesFile)
{
    std::ifstream fin(testNodesFile, std::ios::in);
    for (;;)
    {
        std::string line;
        getline(fin, line, '\n');
        if (fin.fail())
        {
            if (fin.eof()) break;    // end of file reached - normal termination
        }

        std::string s1;
        int i = 0;
        for (; i < line.length(); i++)
        {
            char chLine = line[i];
            if (chLine == ' ' || chLine == '\t')
            {
                if (s1.empty())
                    continue;
                else
                    break;
            }
            else
            {
                s1 += chLine;
            }
        }

        if (s1.empty())
            continue;

        testNodeNames.push_back(s1);
    }

    testNodeIndexes.clear();
    testNodeIndexes.reserve(testNodeNames.size());
    for (std::string name : testNodeNames)
    {
        int index = -1;
        for (size_t i = 0; i < pNetwork->nodes.size(); i++)
        {
            Node* node = pNetwork->nodes[i];
            if (node->name.compare(name) == 0)
            {
                index = (int)i;
                break;
            }
        }
        testNodeIndexes.push_back(index);
    }
}

void PressureEvaluation::outputHeadDelta(const std::string& nodeName)
{
    bool bAdded = false;
    for (int index : pressureTapIndexes)
    {
        double delta = 0;
        if (index != -1)
        {
            Node* pressureNode = pNetwork->node(index);
            double head = pressureNode->head * lcf;
            delta = baseHeadResults[index] - head;
            if (m_bLogDetails)
            {
                std::stringstream ss;
                if (delta < 0.0001 && delta > -0.0001)
                    delta = 0;
                ss << std::setprecision(4) << delta;
                m_logFile << nodeName << "    " << pressureNode->name << "    " << baseHeadResults[index] << "    " << head << "    " << ss.str() << "\n";
            }

            DeltaDemandResult result;
            result.pressureTapName = pressureNode->name;
            result.baseHead = baseHeadResults[index];
            result.actualHead = head;
            result.deltaHead = delta;
            if (bAdded)
            {
                nodeDeltaDemandResultTable[nodeName].push_back(result);
            }
            else
            {
                std::vector<DeltaDemandResult> results;
                results.push_back(result);
                nodeDeltaDemandResultTable[nodeName] = results;
                bAdded = true;
            }
        }
        else
        {
            if (m_bLogDetails)
                m_logFile << pressureTapNames[index] << "    " << delta << "\n";
        }
    }
}

void PressureEvaluation::doPressureTapAnalysis()
{
    for (auto it = nodeDeltaDemandResultTable.begin(); it != nodeDeltaDemandResultTable.end(); ++it)
    {
        for (const DeltaDemandResult& deltaResult : it->second)
        {
            if (deltaResult.deltaHead >= pressureDelta)
            {
                if (impactedNodeNames.find(deltaResult.pressureTapName) == impactedNodeNames.end())
                {
                    std::vector<std::string> nodeNames;
                    nodeNames.push_back(it->first);
                    impactedNodeNames[deltaResult.pressureTapName] = nodeNames;
                }
                else
                {
                    impactedNodeNames[deltaResult.pressureTapName].push_back(it->first);
                }
            }
        }
    }

    m_resultFile << "**** Pressure Tap nodes evaluation ****" << "\n";
    m_resultFile << "**** Demand Delta: " << demandDelta << "    Pressure Delta: " << pressureDelta << "    ****\n";

    for (auto it = impactedNodeNames.begin(); it != impactedNodeNames.end(); it++)
    {
        for (const std::string & nodeName : it->second)
        {
            m_resultFile << it->first << "    ";
            m_resultFile << nodeName << "\n";
        }
    }
}
