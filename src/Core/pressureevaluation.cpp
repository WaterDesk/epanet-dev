#include "PressureEvaluation.h"
#include "node.h"
#include "junction.h"

#include <time.h>
#include <iostream>
#include <fstream>


PressureEvaluation::PressureEvaluation(const char* inpFile, const char* rptFile, const char* cydFile)
{
    std::cout << "\n... EPANET Version 3.0\n";

    // ... declare a Project variable and an error indicator
    err = 0;

    // ... initialize execution time clock

    // ... open the command line files and load network data
    if ((err = project.openReport(rptFile))) return ;
    std::cout << "\n    Reading input file ...";
    if ((err = project.load(inpFile))) return ;
    //if ((err = p.openOutput(outFile))) return err;
    project.writeSummary();

    pNetwork = project.getNetwork();
    lcf = pNetwork->ucf(Units::LENGTH);
    qcf = pNetwork->ucf(Units::FLOW);
    resultCount = (int)pNetwork->nodes.size();
    baseHeadResults.reserve(resultCount);

    resultFile.open("C:\\Users\\xuxi\\Documents\\EPANET Projects\\head_result.txt");

    strPressureTapFile = cydFile;
    initPressureTapIndexes();
}

PressureEvaluation::~PressureEvaluation()
{
    resultFile.close();
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
    for (Node* node : pNetwork->nodes)
    {
        double head = node->head * lcf;
        baseHeadResults.push_back(head);
        //resultFile << node->name << "    " << head << "\n";
    }
}

void PressureEvaluation::doAllEvaluation()
{
    doBaseEvaluation();

    int nodeCount = (int)pNetwork->nodes.size();
    for (int i = 0; i < nodeCount; i++)
    {
        doEvaluation(i, 100);
    }
}

void PressureEvaluation::doEvaluation(int index, double deltaDemand)
{
    // close hydraulic engine
    project.closeEngines();
    pNetwork->resetData();

    // update demand

    Node* pTestNode = pNetwork->nodes[index];
    if (Node::JUNCTION != pTestNode->type())
        return;
    Junction* pJuction = dynamic_cast<Junction*>(pTestNode);
    if (pJuction == nullptr)
        return;

    resultFile << " #### compute again:  " << pTestNode->name << "\n";


    double originalDemand = pJuction->primaryDemand.baseDemand;
    updateDemand(pJuction, deltaDemand);

    // do simulation
    if ((err = project.initSolver(false)))
    {
        restoreDemand(pJuction, originalDemand);
        return;
    }
    int t = 0;
    err = project.runSolver(&t);

    // output result
    outputHeadDelta();
    //for (size_t i = 0; i < pNetwork->nodes.size(); i++)
    //{
    //    if (i % 1000 != 0)
    //        continue;
    //    Node* node = pNetwork->node((int)i);
    //    double head = node->head * lcf;
    //    double delta = baseHeadResults[i] - head;
    //    resultFile << node->name << "    " << baseHeadResults[i] << "    " << head << "    " << delta << "\n";
    //}

    // rollback the demand
    restoreDemand(pJuction, originalDemand);
}

void PressureEvaluation::updateDemand(Junction* pJuction, double deltaDemand)
{
    pJuction->primaryDemand.baseDemand += deltaDemand;
    pJuction->demands.clear();
    pJuction->demands.push_back(pJuction->primaryDemand);
    for (Demand& demand : pJuction->demands)
    {
        demand.baseDemand /= qcf;
    }
}

void PressureEvaluation::restoreDemand(Junction* pJuction, double originalDemand)
{
    pJuction->primaryDemand.baseDemand = originalDemand;
    pJuction->demands.clear();
    pJuction->demands.push_back(pJuction->primaryDemand);
    for (Demand& demand : pJuction->demands)
    {
        demand.baseDemand /= qcf;
    }
}

void PressureEvaluation::initPressureTapIndexes()
{
    std::ifstream fin(strPressureTapFile, std::ios::in);
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

void PressureEvaluation::outputHeadDelta()
{
    for (int index : pressureTapIndexes)
    {
        double delta = 0;
        if (index != -1)
        {
            Node* node = pNetwork->node(index);
            double head = node->head * lcf;
            delta = head - baseHeadResults[index];
            resultFile << node->name << "    " << baseHeadResults[index] << "    " << head << "    " << delta << "\n";
        }
        else
        {
            resultFile << pressureTapNames[index] << "    " << delta << "\n";
        }
    }
}
