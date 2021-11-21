/*
* Multi thread gravity simulation
*
* Compile with mpic++ P1_GravitySim.cpp
* 
* Run with:  ./a.out
*
*       or:  mpirun -np 1 a.out or others as described below.  I note that all of the mpirun executions tend to run the memory, including swap space. out of memory.         
*                               Possible memory leak involved when mpirun is used.  I have not investigated this.
 *
 * Run:      mpirun a.out                               uses all available cores
 *           mpirun --use-hwthread-cpus a.out           uses all available hardware threads
 *           mpirun -np 4 a.out                         uses 4 cores
 *           mpirun --use-hwthread-cpus -np 8 a.out     uses 8 hardware threads
 *           mpirun --host m1:32,c1:16,c2:16 a.out      uses 64 threade: 32 threads on m1, 16 threads on c1 and 16 threads on c2
*
* Production representative: Includes a complete PIC calculation.  Particle position is saved at the end of initialization, and each time iteration in pos_x[i], pos_y[i], pos_z[i]
* Output is generated in P1_GravitySim.txt.  Output is controlled by the save variable: 1 = save, 0 = don't save
* This program uses classic Newtonian mathematics for particle motion, and uses the PIC method of encoding/decoding particle acceleration with a 3D grid.  The acceleration
* grid is just earths gravity at the surface of the earth, so a constant -9.81 m/s/s in the y axis.  The PIC encoding in this case is a trivial assignment loop, and 
* demonstrates the concept of using a grid, etc.  The assignment function will be replaced with Electromagnetic force calculations for an electromagetic physics simulation. -KJP
*
* Current tasks:
*    - Eliminate tmp_matrix and just use pos_x[i], pos_y[i], pos_z[i] for particle position at the end of each time iteration - done
*    - Move all file output tasking to main and consolidate related code into groups that can be easily turned off - done
*    - Eliminate the duplicate processes in each of; initialize_particle(), move_particle() and output_data() - done
*    - Remove dead particles from the particle list, so they are not used in the calculation of acc_field, and are not processed in the main loop or inner loop
*          make the adjustment to num_particles, not iterations
* 
*/

#include <thread>
#include <string>
#include <fstream>
#include <cmath>
#include <stdio.h>
#include <stdlib.h>
#include "timer.h"
using namespace std;

int thread_count        = 8;

const int Dim           = 30;
int iterations          = 420;
int j                   = 0;
float delta_t           = 0.01;
const int num_particles = 16;
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

string output_file      = "P1_GravitySim.txt";
bool save               = 1;                          //save = 0 means do not save output to file Neo_GravitySim_mod.txt.
                                                      //The Clementine (H1) computer processes 1,600,000 particles in a 10 second run at 100 steps per second
                                                      //in 16 seconds run time using 8 threads, 11 seconds using 16 threads and 8 seconds using 32 threads.
                                                      //M1 processed 4.8Mil particles in 21 seconds, no problems.

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

float initial_pos_x(int i)
    {
    return 0.0;
    }

float initial_pos_y(int i)
    {
    return 0.0;
    }

float initial_pos_z(int i)
    {
    return (i)/10.0;
    }

float calculate_vel(float vel, float acc)
    {
    return vel + acc * delta_t;
    }

float calculate_pos(float pos, float vel, float vel_n)
    {
    return pos + (delta_t / 2.0) * (vel + vel_n);
    }


/************************************************************************************************************************/
/***************************************end of the setup variables and functions*****************************************/

/************************************************************************************************************************/



                                                      //Trilinear interpolation from https://gist.github.com/begla/1019993
                                                      //The first function is a 1D linear interpolation of x that lies between x1 (value of q00)
                                                      //and x2 (value of q01).

float linear_interp(float x, int x1, int x2, float q00, float q01)
    {
    float x_1=static_cast<float>(x1);
    float x_2=static_cast<float>(x2);
    return ((x_2 - x) / (x_2 - x_1)) * q00 + ((x - x_1) / (x_2 - x_1)) * q01;
    }

                                                      //The second function is the trilinear interpolation of a single value.  This function
                                                      //is called separately for each of x y and z. The intent is:
                                                      //The calling statement provides the point, and the values at the 3D cell corner points  
                                                      //which are then used to calculate the returned value.
                                                      //Note that for referencing xyz cartesian coordinates, the points "qxyz" are mapped to the
                                                      //cartesian points (x,y,z) 

float interp_acc(float x, float y, float z, int xyz)
    {
    int   pos_x_int, pos_y_int, pos_z_int;

    pos_x_int = x;
    pos_y_int = y;
    pos_z_int = z;

    int x1 = pos_x_int;
    int x2 = x1 + 1;
    int y1 = pos_y_int;
    int y2 = y1 + 1;
    int z1 = pos_z_int;
    int z2 = z1 + 1;

    float q000 = acc_field[x1] [y1]   [z1]  [xyz];
    float q001 = acc_field[x1] [y1]   [z1+1][xyz];
    float q010 = acc_field[x1] [y1+1] [z1]  [xyz];
    float q011 = acc_field[x1] [y1+1] [z1+1][xyz];
    float q100 = acc_field[x1+1][y1]  [z1]  [xyz];
    float q101 = acc_field[x1+1][y1]  [z1+1][xyz];
    float q110 = acc_field[x1+1][y1+1][z1]  [xyz];
    float q111 = acc_field[x1+1][y1+1][z1+1][xyz];

    float x00 = linear_interp(x, x1, x2, q000, q100);
    float x10 = linear_interp(x, x1, x2, q010, q110);
    float x01 = linear_interp(x, x1, x2, q001, q101);
    float x11 = linear_interp(x, x1, x2, q011, q111);
    float r0  = linear_interp(y, y1, y2, x00,   x01);
    float r1  = linear_interp(y, y1, y2, x10,   x11);

    return linear_interp(z, z1, z2, r0, r1);
    }

void initialize_particles(int ID)
    {
    int block_size = num_particles/thread_count;
    if (ID < num_particles % thread_count) block_size += 1;
    for(int i_count=0; i_count<block_size; i_count++)
        {
        int i    = (block_size*ID)+i_count;

        pos_x[i] = initial_pos_x(i);
        pos_y[i] = initial_pos_y(i);
        pos_z[i] = initial_pos_z(i);

        vel_x[i] = initial_vel_x;
        vel_y[i] = initial_vel_y;
        vel_z[i] = initial_vel_z;
        }
    }

void Generate_acc_field(float pos_x[num_particles], float pos_y[num_particles], float pos_z[num_particles], 
                        float vel_x[num_particles], float vel_y[num_particles], float vel_z[num_particles])
    {
    for (int k=0; k<Dim; k++) for (int i=0; i<Dim; i++) for (int m=0; m<Dim; m++)
        {
        acc_field[k][i][m][0] = 0.0;
        acc_field[k][i][m][1] = -9.81;
        acc_field[k][i][m][2] = 0.0;
        }
    }

void move_particles(int ID)
    {
    int block_size = num_particles/thread_count;      //Modify block_size to accomodate num_particles that is not evenly divided by thread_count
    if (ID < num_particles % thread_count) block_size += 1;

    float vel_x_n;
    float vel_y_n;
    float vel_z_n;

    float pos_x_new;
    float pos_y_new;
    float pos_z_new;

    for(int j_count=0; j_count<block_size; j_count++)
        {
        int i    = (block_size*ID)+j_count;
                                                      //Save the current velocity
        vel_x_n  = vel_x[i];
        vel_y_n  = vel_y[i];
        vel_z_n  = vel_z[i];

                                                      //Decode current acceleration from acc_field
        acc_x[i] = interp_acc(pos_x[i], pos_y[i], pos_z[i], 0);
        acc_y[i] = interp_acc(pos_x[i], pos_y[i], pos_z[i], 1);
        acc_z[i] = interp_acc(pos_x[i], pos_y[i], pos_z[i], 2);

                                                      //Calculate a new velocity
        vel_x[i] = calculate_vel(vel_x_n, acc_x[i]);
        vel_y[i] = calculate_vel(vel_y_n, acc_y[i]);
        vel_z[i] = calculate_vel(vel_z_n, acc_z[i]);

                                                      //Calculate a new position
        pos_x_new = calculate_pos(pos_x[i], vel_x[i], vel_x_n);
        pos_y_new = calculate_pos(pos_y[i], vel_y[i], vel_y_n);
        pos_z_new = calculate_pos(pos_z[i], vel_z[i], vel_z_n);

                                                      //Save the new position
        if ((pos_x_new >= x_min && pos_x_new <= x_max) && (pos_y_new >= y_min && pos_y_new <= y_max))
            {
            pos_x[i] = pos_x_new;
            pos_y[i] = pos_y_new;
            pos_z[i] = pos_z_new;
            }
        }
    }

void output_data(int ID)
    {
    int block_size = num_particles/thread_count;      //Modify block_size to accomodate num_particles that is not evenly divided by thread_count
    if (ID < num_particles % thread_count) block_size += 1;

    string filename = "file"+to_string(ID);
    ofstream myFile(filename);
                                                      //i_count starts at 0 and steps through block_size
    for(int i_count=0; i_count<block_size; i_count++)  
        {
        int i = (block_size*ID)+i_count;              //i starts at the beginning of a block of particles and steps through the block
        myFile << pos_x[i] << " " << pos_y[i] << " " << pos_z[i] << "\n";
        }
    myFile.close();
    }

/*-------------------------------------------------------------------*/
int main(void)
    {                                                 //Initialize program specific variables
    double start, start1, end;
    thread t[thread_count];
    string flow="cat";
    const char *command=flow.c_str();


    GET_TIME(start);
                                                      //Initialize particle positions
    for(int i=0; i<thread_count;  i++) t[i] = thread(initialize_particles, i);
    for(int i=0; i<thread_count;  i++) t[i].join();

                                                      //Create and append (concatenate) particle positions to the output file 
    if(save)
        {
        for(int i=0; i<thread_count;  i++) t[i] = thread(output_data, i);
        for(int i=0; i<thread_count;  i++) t[i].join();

        for(int i=0; i<thread_count; i++) flow += " file"+to_string(i);
        flow += " > " + output_file;
        command=flow.c_str();
        system(command);

/*

                                                      //-FOR DEMO- Output the initial frame file
        if(save)
            {
            string filename = "frame0";
            filename += ".txt";
            ofstream frame0File(filename);
            for(int i=0; i<num_particles; i++) frame0File << pos_x[i] << " " << pos_y[i] << " " << pos_z[i] << "\n";
            }
*/
        }

    for(j=1; j<iterations; j++)                       //main loop
        {
        flow = "cat";
                                                      //Create (encode) acc_field
        Generate_acc_field(pos_x, pos_y, pos_z, vel_x, vel_y, vel_z);

                                                      //inner loop
        for(int i=0; i<thread_count;  i++) t[i] = thread(move_particles, i);
        for(int i=0; i<thread_count;  i++) t[i].join();
                                                      //Inner loop end

                                                      //Accommodate particles that have left the simulation space (died)

                                                      //Append (concatenate) saved temporary files to the output file
        if(save)
            {
            for(int i=0; i<thread_count;  i++) t[i] = thread(output_data, i);
            for(int i=0; i<thread_count;  i++) t[i].join();

            for(int i=0; i<thread_count; i++) flow += " file"+to_string(i);
            flow += " >> " + output_file;
            command=flow.c_str();
            system(command);
                                                      //Optionally output a frame file for each delta_t iteration
/*
                                                      //-FOR DEMO- Output each iteration frame of data (particle position only)
            string filename = "frame"+to_string(j)+".txt";
            ofstream frameFile(filename);
            for(int i=0; i<num_particles; i++) frameFile << pos_x[i] << " " << pos_y[i] << " " << pos_z[i] << "\n";
            frameFile.close();
*/
            }
        }                                             //main loop end


    GET_TIME(start1);
                                                      //Delete the temporary output files
    if(save)
        {
        flow="rm file*";
        command=flow.c_str();
        system(command);
        }

    GET_TIME(end);
    printf("Program took %f seconds\n",end-start);

    delete [] pos_x;
    delete [] pos_y;
    delete [] pos_z;
    delete [] vel_x;
    delete [] vel_y;
    delete [] vel_z;
    delete [] acc_x;
    delete [] acc_y;
    delete [] acc_z;
    delete [] acc_field;

    return 0;
    }
