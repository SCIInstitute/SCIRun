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
#include <time.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::ParticleInCell;

HelloWorldAlgo::HelloWorldAlgo()
    {
    addParameter(Variables::Method,0);
    }

AlgorithmOutput HelloWorldAlgo::run(const AlgorithmInput&) const
    {
    AlgorithmOutput output;

    using namespace std;

    string output_txt  = "test1";
//    int  num_particles = 10;
    auto save = get(Variables::Method).toInt();       //pull parameter from UI

    clock_t start = clock();
                                             
    printf("Debug 1: save is:  %d HW_num_particles is: %d\n", save, HW_num_particles);         // print save using printf

    std::cout<<output_txt<<std::endl;                 // print a message using std::cout

    if(save)                                          // save a file to storage
        {
        printf("Debug 02: save is : %d\n", save);
        ofstream myfile;
        myfile.open ("../../../Simulation-output/example.txt");
        myfile << "Hello world\n";
        myfile.close();
        }

    clock_t end = clock();
    printf("Program took %.3f seconds\n",(double)((end+0.0-start)/CLOCKS_PER_SEC));

//    delete [] HW_pos_x;

    return output;
    }
