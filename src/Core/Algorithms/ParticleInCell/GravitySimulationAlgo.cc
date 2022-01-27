/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2020 Scientific Computing and Imaging Institute,
   University of Utah.

   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   DEALINGS IN THE SOFTWARE.
*/

#include<Core/Algorithms/ParticleInCell/GravitySimulationAlgo.h>
#include<Core/Datatypes/MatrixTypeConversions.h>
#include <time.h>
#include <chrono>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::ParticleInCell;

const AlgorithmOutputName GravitySimulationAlgo::x_coordinates("x_coordinates");
const AlgorithmOutputName GravitySimulationAlgo::y_coordinates("y_coordinates");
const AlgorithmOutputName GravitySimulationAlgo::z_coordinates("z_coordinates");

GravitySimulationAlgo::GravitySimulationAlgo()
    {
    addParameter(Variables::Method,0);
    }

/*
************************ Function prototype declaratios for the GravitySim program ************************
*/

void initialize_particles(int ID);
void Generate_acc_field();
void move_particles(int ID);


/*
************************ End function prototype declaratios for the GravitySim program ************************
*/

AlgorithmOutput GravitySimulationAlgo::run(const AlgorithmInput&) const
    {
    AlgorithmOutput output;
                                                      //Set up the files that are used to pass data to SCIRun
    DenseMatrixHandle output_mat_0(new DenseMatrix(buffer_size, 1));
    DenseMatrixHandle output_mat_1(new DenseMatrix(buffer_size, 1));
    DenseMatrixHandle output_mat_2(new DenseMatrix(buffer_size, 1));
    

/*
************************ Source code for the GravitySim program ************************
*/

//#include <time.h>
using namespace std;
                                                      //Initialize program specific variables
    int iterate_end;
    thread t[thread_count];
    ofstream out_x, out_y, out_z;
    auto save = get(Variables::Method).toInt();       //pull parameter from UI

    clock_t start = clock();
    auto start_wall = chrono::high_resolution_clock::now();

                                                      //Initialize thread block size and thread index
    int block_size    = num_particles/thread_count;
    int thread_modulo = num_particles%thread_count;
    t_index[0]        = 0;
    for(int ID = 0; ID < thread_count; ID++)
        {
        t_blk_size[ID] = block_size;
        if(ID < thread_modulo) t_blk_size[ID] += 1;
        if(ID > 0) t_index[ID] = t_index[ID-1] + t_blk_size[ID-1];
        }
                                                      //Initialize particle positions
    for(int i=0; i<thread_count; i++) t[i] = thread(initialize_particles, i);
    for(int i=0; i<thread_count; i++) t[i].join();

    for(j=1; j<iterations; j++)                       //main loop
        {
        iterate_end = thread_count;
                                                      //Create (encode) acc_field
        Generate_acc_field();

                                                      //inner loop
        for(int i=0; i<thread_count; i++) t[i] = thread(move_particles, i);
        for(int i=0; i<thread_count; i++) t[i].join();
                                                      //Inner loop end

        for(int i=0; i<thread_count; i++)             //If all particles are dead, end the main loop
            {
            iterate_end -= t_alive[i];
            t_alive[i] = 0;
            }

        if(iterate_end == thread_count) j = iterations;
        }                                             //main loop end

                                                      //Pass the buffered data to SCIRun
    double *data0=output_mat_0->data();
    double *data1=output_mat_1->data();
    double *data2=output_mat_2->data();

    std::copy(buffer_pos_x, buffer_pos_x+buffer_size, data0);
    std::copy(buffer_pos_y, buffer_pos_y+buffer_size, data1);
    std::copy(buffer_pos_z, buffer_pos_z+buffer_size, data2);

    output[x_coordinates] = output_mat_0;
    output[y_coordinates] = output_mat_1;
    output[z_coordinates] = output_mat_2;

    auto end_wall = chrono::high_resolution_clock::now();       //Instrumentation
    chrono::duration<double> time_taken = end_wall - start_wall;
    clock_t end   = clock();
    if(save)
        {
        printf("Program took %.6f seconds Wall Clock time\n", time_taken.count());
        printf("Program took %.6f seconds CPU time\n",(double)((end+0.0-start)/CLOCKS_PER_SEC));
        }

    return output;
    }

/*
************************ End of source code for the GravitySim program ************************
*/

/*
************************ Functions used by the GravitySim program ************************
*/

float initial_pos_x()
    {
    return 0;
    }

float initial_pos_y()
    {
    return 0.0;
    }

float initial_pos_z(int i)
    {
//    return (i)/10.0;
    return i/(1.0*num_particles);
    }

float calculate_vel(float vel, float acc)
    {
    return vel + acc * delta_t;
    }

float calculate_pos(float pos, float vel, float vel_n)
    {
    return pos + (delta_t / 2.0) * (vel + vel_n);
    }


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
                                                      //Note that for referencing xyz cartesian coordinates, the points "xyz" are mapped to the
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
    t_alive[ID]    = 0;                               //Set the thread alive flag to 'not alive'

    for(int i=t_index[ID]; i<t_index[ID]+t_blk_size[ID]; i++)
        {
        p_alive[i] = 0;                               //Set the particle alive flag to 'not alive'

        pos_x[i]   = initial_pos_x();
        pos_y[i]   = initial_pos_y();
        pos_z[i]   = initial_pos_z(i);

        if(!(i%sample_size_p))                        //Buffer the data to be visualized
            {
            buffer_pos_x[i/sample_size_p] = pos_x[i];
            buffer_pos_y[i/sample_size_p] = pos_y[i];
            buffer_pos_z[i/sample_size_p] = pos_z[i];
            }

        vel_x[i]   = initial_vel_x;
        vel_y[i]   = initial_vel_y;
        vel_z[i]   = initial_vel_z;

        if ((pos_x[i] >= x_min && pos_x[i] <= x_max) && (pos_y[i] >= y_min && pos_y[i] <= y_max))
            {
            p_alive[i]  = 1;                          //Set particle and thread alive flags to 'alive'
            t_alive[ID] = 1;
            }
        }
    }

void Generate_acc_field()
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
    t_alive[ID] = 0;                                  //Assume the thread is dead for the new particle position (change later if particle is alive)

    float vel_x_n;
    float vel_y_n;
    float vel_z_n;

    float pos_x_new;
    float pos_y_new;
    float pos_z_new;

    for(int i=t_index[ID]; i<t_index[ID]+t_blk_size[ID]; i++)
        {
        if (p_alive[i])                               //If the particle is alive, press on with calculating a new particle position, otherwise skip the calculation
            {
            p_alive[i] = 0;                           //The new particle position is assumed to be out of the simulation space, until deterined otherwise

            vel_x_n  = vel_x[i];                      //Save the current velocity
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

                                                      //Save the new position if it is within the simulation space
            if ((pos_x_new >= x_min && pos_x_new <= x_max) && (pos_y_new >= y_min && pos_y_new <= y_max))
                {
                pos_x[i]    = pos_x_new;
                pos_y[i]    = pos_y_new;
                pos_z[i]    = pos_z_new;
                p_alive[i]  = 1;                      //Set particle and thread alive flags to alive
                t_alive[ID] = 1;                      //Note that thread alive is set if any thread controlled particle is alive
                }

            if(!(i%sample_size_p+j%sample_size_i))    //Buffer the data to be visualized
                {
                buffer_pos_x[(i/sample_size_p)+(j*num_particles/(sample_size_i*sample_size_p))] = pos_x[i];
                buffer_pos_y[(i/sample_size_p)+(j*num_particles/(sample_size_i*sample_size_p))] = pos_y[i];
                buffer_pos_z[(i/sample_size_p)+(j*num_particles/(sample_size_i*sample_size_p))] = pos_z[i];
                }
            }
        }
    }
/*

void output_data(int ID)
    {
    string filename = "file"+to_string(ID);
    ofstream myFile(filename);
 
    for(int i = t_index[ID]; i < t_index[ID]+t_blk_size[ID]; i++)
        myFile << pos_x[i] << " " << pos_y[i] << " " << pos_z[i] << "\n";

    myFile.close();
    }
*/


