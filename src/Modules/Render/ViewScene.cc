/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2012 Scientific Computing and Imaging Institute,
   University of Utah.

   License for the specific language governing rights and limitations under
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

#include <iostream>
//#include <Modules/Math/ReportMatrixInfo.h>
//#include <Algorithms/Math/ReportMatrixInfo.h>
//#include <Core/Datatypes/DenseMatrix.h>

//using namespace SCIRun::Modules::Math;
//using namespace SCIRun::Domain::Datatypes;
//using namespace SCIRun::Algorithms::Math;
//using namespace SCIRun::Domain::Networks;
//TODO DAN
//
//ReportMatrixInfoModule::ReportMatrixInfoModule() : Module(ModuleLookupInfo("ReportMatrixInfo", "Math", "SCIRun")) {}
//
//void ReportMatrixInfoModule::execute()
//{
//  DatatypeHandleOption input = get_input_handle(0);
//  if (!input)
//    throw std::logic_error("TODO DAN Input data required, need to move this check to Module base class!");
//
//  DenseMatrixConstHandle matrix = boost::dynamic_pointer_cast<DenseMatrix>(*input); //TODO DAN: clean
//  if (!matrix)
//  {
//    //TODO DAN log error? send null? check standard practice.
//    return;
//  }
//
//  ReportMatrixInfoAlgorithm algo;
//  ReportMatrixInfoAlgorithm::Outputs output = algo.run(matrix);
//  get_state()->setValue("ReportedInfo", output);
//
//  std::cout << "nothing outputted yet in ReportMatrixInfo...check state variable ReportedInfo" << std::endl;
//}