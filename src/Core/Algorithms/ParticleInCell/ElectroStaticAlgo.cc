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

/*
Note: eliminate all global variables declared in the module and algo header files (if any) and set them all up as local variables in the .cc files
Also: before the ch2 code is added, save the ElectroStatic files as a PIC_shell module
*/


#include<Core/Algorithms/ParticleInCell/ElectroStaticAlgo.h>
//#include<Core/Math/MiscMath.h>                                //delete this if it is not needed to get the module working
#include<Core/Datatypes/MatrixTypeConversions.h>
#include <time.h>
#include <chrono>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::ParticleInCell;

const AlgorithmOutputName ElectroStaticAlgo::x_coordinates("x_coordinates");
const AlgorithmOutputName ElectroStaticAlgo::y_coordinates("y_coordinates");
const AlgorithmOutputName ElectroStaticAlgo::z_coordinates("z_coordinates");

ElectroStaticAlgo::ElectroStaticAlgo()
    {
    addParameter(Variables::Method,0);
    }
/*
************************ Function prototype declarations ************************
*/

/*
************************ End function prototype declarations ************************
*/

AlgorithmOutput ElectroStaticAlgo::run(const AlgorithmInput&) const
    {
    AlgorithmOutput output;

/*
************************************************PIC varables and data collection set up, including particle initialization
*/

    using namespace std;
    const int num_particles = 1000000;
    const int iterations    = 400;
    const int sample_size_p = 100000;
    const int sample_size_i = 2;                              //activate this variable when the outer loop is implemented and buffered data are collected
    const int buffer_size   = (iterations/sample_size_i)*(num_particles/sample_size_p);
    float delta_t           = 0.01;
    auto pos_x              = new double[num_particles];
    auto pos_y              = new double[num_particles];
    auto pos_z              = new double[num_particles];
    auto buffer_pos_x       = new double[buffer_size];
    auto buffer_pos_y       = new double[buffer_size];
    auto buffer_pos_z       = new double[buffer_size];

                                                                //Files that are used for data collection
    DenseMatrixHandle output_mat_0(new DenseMatrix(buffer_size, 1));
    DenseMatrixHandle output_mat_1(new DenseMatrix(buffer_size, 1));
    DenseMatrixHandle output_mat_2(new DenseMatrix(buffer_size, 1));

    clock_t start    = clock();                                 //Instrumentation
    auto start_wall  = chrono::high_resolution_clock::now();
    auto outputTimes = get(Variables::Method).toInt();          //outputTimes is used to show or not show the process time

    for(int i=0; i<num_particles; i++)                          //Initialization
        {
        pos_x[i] = 0.0;
        pos_y[i] = 0.0;
        pos_z[i] = 0.0;

        if(!(i%sample_size_p))                                  //Buffer the data to be visualized
            {
            buffer_pos_x[i/sample_size_p] = pos_x[i];
            buffer_pos_y[i/sample_size_p] = pos_y[i];
            buffer_pos_z[i/sample_size_p] = pos_z[i];
            }
        }

/*
************************************************Simulation code goes here, including the collection of buffered data inside the inner loop

            if(!(i%sample_size_p+j%sample_size_i))    //Buffer the data to be visualized
                {
                buffer_pos_x[(i/sample_size_p)+(j*num_particles/(sample_size_i*sample_size_p))] = pos_x[i];
                buffer_pos_y[(i/sample_size_p)+(j*num_particles/(sample_size_i*sample_size_p))] = pos_y[i];
                buffer_pos_z[(i/sample_size_p)+(j*num_particles/(sample_size_i*sample_size_p))] = pos_z[i];
                }
*/


/*
************************************************End of the simulation code
*/

/*
************************************************Wrap up, including instrumentation and passing buffered data to SCIRun
*/

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
    if(outputTimes)
        {
        printf("Program took %.6f seconds Wall Clock time\n", time_taken.count());
        printf("Program took %.6f seconds CPU time\n",(double)((end+0.0-start)/CLOCKS_PER_SEC));
        }

    return output;
    }


