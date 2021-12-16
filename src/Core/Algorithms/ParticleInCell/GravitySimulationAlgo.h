
#ifndef CORE_ALGORITHMS_ParticleInCell_GravitySimulationAlgo_H
#define CORE_ALGORITHMS_ParticleInCell_GravitySimulationAlgo_H

#include <string>
#include <sstream>
#include <vector>
#include <algorithm>

#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Algorithms/Base/AlgorithmBase.h>
#include <Core/Algorithms/ParticleInCell/share.h>

namespace SCIRun {
namespace Core {
namespace Algorithms {
namespace ParticleInCell {

/*
************************ Global variables used by the GravitySim program ************************
*/

using namespace std;

int thread_count        = 8;
const int Dim           = 100;
int iterations          = 420;
int j                   = 0;
float delta_t           = 0.01;
const int num_particles = 10;
//float alpha             = 0.3;

float initial_vel_x     = 1.0;
float initial_vel_y     = 20.0;
float initial_vel_z     = 0.0;

float x_min             = 0.0;
float y_min             = 0.0;
float z_min             = 0.0;

float x_max             = 100.0;
float y_max             = 100.0;
float z_max             = 100.0;

string output_file      = "P2_GravitySim.txt";
bool save               = 1;                          //save = 0 means do not save output to file P2_GravitySim.txt.
                                                      //The Clementine (H1) computer processes 1,600,000 particles in a 10 second run at 100 steps per second
                                                      //in 16 seconds run time using 8 threads, 11 seconds using 16 threads and 8 seconds using 32 threads.
                                                      //H1 processed 4.8Mil particles in 21 seconds, no problems.

                                                      //The HP laptop takes 63 seconds to process 1.6 Mil particles using 1 thread, 26 seconds using 4 threads,
                                                      //and 18 seconds using 8 threads.  Processing 4.8 Mil particles using 8 threads took 70 seconds.

auto pos_x       = new float[num_particles];
auto pos_y       = new float[num_particles];
auto pos_z       = new float[num_particles];
auto vel_x       = new float[num_particles];
auto vel_y       = new float[num_particles];
auto vel_z       = new float[num_particles];
auto acc_x       = new float[num_particles];
auto acc_y       = new float[num_particles];
auto acc_z       = new float[num_particles];
auto acc_field   = new float[Dim][Dim][Dim][3];
auto p_alive     = new int  [num_particles];
auto t_alive     = new int  [thread_count];
auto t_blk_size  = new int  [thread_count];
auto t_index     = new int  [thread_count];

/*
************************ End of global variables used by the GravitySim program ************************
*/

class SCISHARE GravitySimulationAlgo : public AlgorithmBase
    {
    private:
        void initialize_particles(int ID);
        void output_data(int ID);
        void Generate_acc_field(float pos_x[num_particles], float pos_y[num_particles], float pos_z[num_particles], float vel_x[num_particles], float vel_y[num_particles], float vel_z[num_particles]);
        void move_particles(int ID);
    public:
      GravitySimulationAlgo();
      AlgorithmOutput run() const override;
    };

}}}}
#endif
