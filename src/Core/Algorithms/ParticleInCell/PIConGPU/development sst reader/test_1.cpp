/*
 ============================================================================

 Author      : From: https://openpmd-api.readthedocs.io/en/latest/usage/streaming.html#c 
 Name        : test_1.cpp
 Version     : 0
 Timeframe   : Spring 2022
 Description : C++ code to test in situ communication between PIConGPU and SCIRun

 Dependencies: Assumes the PIConGPU dev_dependencies script has been run and SCIRun is installed

 To compile and run this program from a terminal (assume PIConGPU has been installed on this machine):

  export CMAKE_PREFIX_PATH=$HOME/lib/ADIOS2:$CMAKE_PREFIX_PATH &&
  export LD_LIBRARY_PATH=$HOME/lib/ADIOS2/lib:$LD_LIBRARY_PATH &&
  export CMAKE_PREFIX_PATH=$HOME/lib/openPMD-api:$CMAKE_PREFIX_PATH &&
  export LD_LIBRARY_PATH=$HOME/lib/openPMD-api/lib:$LD_LIBRARY_PATH

  cd src
  mkdir test_1_build
  cd test_1_build
  cmake ..
  make

  Execute with ./test_1 or mpirun -np 1 test_1 or something similar

 ============================================================================

*/

#include <openPMD/openPMD.hpp>

#include <algorithm>
#include <array>
#include <iostream>
#include <memory>

#include <filesystem>
#include <unistd.h>

using std::cout;
using namespace openPMD;

int main()
{

    while(!std::filesystem::exists("/home/kj/scratch/runs/lwfa_001/simOutput/openPMD/simData.sst")) sleep(1);

#if openPMD_HAVE_ADIOS2
    using position_t = float;
//    using position_t = double;
    auto backends = openPMD::getFileExtensions();
    if (std::find(backends.begin(), backends.end(), "sst") == backends.end())
        {
        std::cout << "SST engine not available in ADIOS2." << std::endl;
        return 0;
        }

    Series series = Series("/home/kj/scratch/runs/lwfa_001/simOutput/openPMD/simData.sst", Access::READ_ONLY);

    for (IndexedIteration iteration : series.readIterations())
        {
        std::cout << "Current iteration: " << iteration.iterationIndex << std::endl;
        Record electronPositions = iteration.particles["e"]["position"];
        std::array<std::shared_ptr<position_t>, 3> loadedChunks;
        std::array<Extent, 3> extents;
        std::array<std::string, 3> const dimensions{{"x", "y", "z"}};

        for (size_t i = 0; i < 3; ++i)
            {
            std::string dim = dimensions[i];
            RecordComponent rc = electronPositions[dim];
            loadedChunks[i] = rc.loadChunk<position_t>(Offset(rc.getDimensionality(), 0), rc.getExtent());
            extents[i] = rc.getExtent();
            }

        iteration.close();
/*
        for (size_t i = 0; i < 3; ++i)
            {
            std::string dim = dimensions[i];
            Extent const &extent = extents[i];
            std::cout << "\ndim: " << dim << "\n" << std::endl;
            auto chunk = loadedChunks[i];
            for (size_t j = 0; j < extent[0]; ++j)
                {
                std::cout << chunk.get()[j] << ", ";
                }
            std::cout << "\n----------\n" << std::endl;
            }
*/
        }

    return 0;
#else
    std::cout << "The streaming example requires that openPMD has been built "
                 "with ADIOS2."
              << std::endl;
    return 0;
#endif

}
