/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Distributed under the MIT License (see the LICENSE file for details).
 *
 */

//! \file main.cpp
//! \brief The main function used to run EPANET from the command line.

#include "epanet3.h"

#include <iostream>

int main(int argc, char* argv[])
{
    //... check number of command line arguments
    //if (argc < 3)
    //{
    //    std::cout << "\nCorrect syntax is: epanet3 inpFile rptFile (outFile)\n";
    //    return 0;
    //}

    ////... retrieve file names from command line
    //const char* f1 = argv[1];
    //const char* f2 = argv[2];
    //const char* f3 = "";
    //if (argc > 3) f3 = argv[3];

    // ... run a full EPANET analysis
    //EN_runEpanet(f1, f2, "");
    //system("PAUSE");

    const char* inpFile = nullptr;
    const char* pressureNodesFile = nullptr;
    const char* testNodesFile = nullptr;
    const char* resultDir = nullptr;
    const char* demandDelta = nullptr;
    const char* pressureDelta = nullptr;

    bool bLogDetails = false;

    for (int i = 1; i < argc; i++)
    {
        std::string arg = argv[i];
        if (arg == "-inpFile")
        {
            i++;
            if (i < argc)
                inpFile = argv[i];
        }
        else if (arg == "-pressureNodesFile")
        {
            i++;
            if (i < argc)
                pressureNodesFile = argv[i];
        }
        else if (arg == "-testNodesFile")
        {
            i++;
            if (i < argc)
                testNodesFile = argv[i];
        }
        else if (arg == "-resultDir")
        {
            i++;
            if (i < argc)
                resultDir = argv[i];
        }
        else if (arg == "-demandDelta")
        {
            i++;
            if (i < argc)
                demandDelta = argv[i];
        }
        else if (arg == "-pressureDelta")
        {
            i++;
            if (i < argc)
                pressureDelta = argv[i];
        }
        else if (arg == "-logDetails")
        {
            bLogDetails = true;
        }
    }

    if (inpFile == nullptr || pressureNodesFile == nullptr || resultDir == nullptr || demandDelta == nullptr || pressureDelta == nullptr)
    {
        std::cout << "Usage: \n";
        std::cout << "    -inpFile <inpFile>\n";
        std::cout << "    -pressureNodesFile <pressureNodesFile>\n";
        std::cout << "    -resultDir <resultDir>\n";
        std::cout << "    -demandDelta <demandDelta>\n";
        std::cout << "    -pressureDelta <pressureDelta>\n";
        std::cout << "    [-testNodesFile <testNodesFile>]\n";
        std::cout << "    [-logDetails]\n";
        return 0;
    }

    EN_runEpanetPressureEvaluation(inpFile, pressureNodesFile, testNodesFile, resultDir, demandDelta, pressureDelta, bLogDetails);
    return 0;
}
