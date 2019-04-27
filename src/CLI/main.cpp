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

    if (argc < 6)
    {
        std::cout << "\nCorrect syntax is: epanet3 inpFile pressureNodesFile resultDir demandDelta pressureDelta (logDetails: 1)\n";
        return 0;
    }

    // arg 1: inp file path
    // arg 2: pressure node file path
    // arg 3: result files directory, e.g. D:\test\
    // arg 4: demand delta
    // arg 5: pressure delta
    // arg 6: log details if 1, optional

    const char* inpFile = argv[1];
    const char* pressureNodesFile = argv[2];
    const char* resultDir = argv[3];
    const char* strDemandDelta = argv[4];
    const char* strPressureDelta = argv[5];

    bool bLogDetails = false;
    if (argc > 6)
        bLogDetails = true;

    // ... run a full EPANET analysis
    //EN_runEpanet(f1, f2, "");
    EN_runEpanetPressureEvaluation(inpFile, pressureNodesFile, resultDir, strDemandDelta, strPressureDelta, bLogDetails);
    //system("PAUSE");
    return 0;
}
