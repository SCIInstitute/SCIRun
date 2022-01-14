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

#include<Core/Algorithms/ParticleInCell/HelloWorldAlgo.h>
#include<Core/Datatypes/MatrixTypeConversions.h>
#include <time.h>
#include <chrono>
#include <algorithm>
#include <vector>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::ParticleInCell;

const AlgorithmOutputName HelloWorldAlgo::x_coordinates("x_coordinates");
const AlgorithmOutputName HelloWorldAlgo::y_coordinates("y_coordinates");
const AlgorithmOutputName HelloWorldAlgo::z_coordinates("z_coordinates");

HelloWorldAlgo::HelloWorldAlgo()
    {
    addParameter(Variables::Method,0);
    }

AlgorithmOutput HelloWorldAlgo::run(const AlgorithmInput&) const
    {
    AlgorithmOutput output;

    DenseMatrixHandle output_mat_0(new DenseMatrix(10, 1));     //Dan's code
    DenseMatrixHandle output_mat_1(new DenseMatrix(10, 1));     //
    DenseMatrixHandle output_mat_2(new DenseMatrix(10, 1));     //
                            
    clock_t start   = clock();                                  //Instrumentation
    auto start_wall = chrono::high_resolution_clock::now();
    auto save       = get(Variables::Method).toInt();                     //save is used to show or not show the process time

    for(int i=0; i<10; i++) HW_pos_x[i] = i*1.0;
printf("Debug1\n");
    double *data0=output_mat_0->data();               //these 3 lines compile but crash SCIRun during execution
    double *data1=output_mat_1->data();
    double *data2=output_mat_2->data();
printf("Debug2\n");
    std::copy(HW_pos_x, HW_pos_x+10, data0);
    std::copy(HW_pos_x, HW_pos_x+10, data1);
    std::copy(HW_pos_x, HW_pos_x+10, data2);

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

//    delete [] HW_pos_x;
    return output;
    }
