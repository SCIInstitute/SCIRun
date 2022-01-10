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

AlgorithmOutput HelloWorldAlgo::run(const AlgorithmInput& input) const
    {
    AlgorithmOutput output;
    DenseMatrixHandle output_mat_0, output_mat_1, output_mat_2;
    DenseMatrixHandle &output_mat_0_ref = output_mat_0;
    DenseMatrixHandle &output_mat_1_ref = output_mat_1;
    DenseMatrixHandle &output_mat_2_ref = output_mat_2;
    clock_t start = clock();
    DenseMatrixHandle return_matrix;                                      //
    auto save = get(Variables::Method).toInt();                           //pull parameter from UI (used to show or not show the process time instrumentation output)

    auto input_matrix=input.get<Matrix>(Variables::InputMatrix);          //Need to replace this definition of input_matrix

    auto mat=castMatrix::toDense(input_matrix);                           //Modify or remove this as needed once input_matrix is defined
    auto start_wall = chrono::high_resolution_clock::now();

    for(int i=0; i<10; i++) HW_pos_x[i] = i*1.0;

    return_matrix.reset(new DenseMatrix(*mat));                           //Modify or remove this as needed once input_matrix is defined

    output[x_coordinates]=return_matrix;
//    output[y_coordinates]=output_mat_1;                                   //add this back when all 3 matricese are implemented
//    output[z_coordinates]=output_mat_2;                                   //add this back when all 3 matricese are implemented

    auto end_wall = chrono::high_resolution_clock::now();
    chrono::duration<double> time_taken = end_wall - start_wall;
    if(save)
        {
        printf("Program took %.6f seconds Wall Clock time\n", time_taken.count());
        clock_t end = clock();
        printf("Program took %.6f seconds CPU time\n",(double)((end+0.0-start)/CLOCKS_PER_SEC));
        }
//    delete [] HW_pos_x;

    return output;
    }
