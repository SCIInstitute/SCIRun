/*
 For more information, please see: http://software.sci.utah.edu
 
 The MIT License
 
 Copyright (c) 2015 Scientific Computing and Imaging Institute,
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

#include <gtest/gtest.h>

#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/MatrixComparison.h>
#include <Testing/Utils/MatrixTestUtilities.h>
#include <Core/Algorithms/Math/ComputePCA.h>

using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms::Math;
using namespace SCIRun::TestUtils;


namespace
{
    //Matrix for input
    DenseMatrixHandle inputMatrix()
    {
        int column1 [12] = {5,4,7,8,10,10,4,6,8,7,9,6};
        int column2 [12] = {7,6,9,8,5,7,9,3,4,5,5,9};
        
        DenseMatrixHandle inputM(boost::make_shared<DenseMatrix>(12,2));
        for (int i = 0; i < inputM->rows(); i++){
            (*inputM)(i,0) = column1[i];
            (*inputM)(i,1) = column2[i];
        }
        return inputM;
    }
    
    //A after it is centered
    DenseMatrixHandle centeredInputMatrix()
    {
    
        double centeredColumn1 [12] = {-2.0,-3.0,-0.0,1.0,3.0,3.0,-3.0,-1.0,1.0,-0.0,2.0,-1.0};
        double centeredColumn2 [12] = {0.583333,-0.416667,2.583333,1.583333,-1.416667,0.583333,2.583333,-3.416667,-2.416667,-1.416667,-1.416667,2.583333};
        
        DenseMatrixHandle centeredM(boost::make_shared<DenseMatrix>(12,2));
        for (int i = 0; i < centeredM->rows(); i++){
            (*centeredM)(i,0) = centeredColumn1[i];
            (*centeredM)(i,1) = centeredColumn2[i];
        }
        return centeredM;
                                
    }
                            
    //Outputs: U,S,V
    DenseMatrixHandle outputU()
    {}
    DenseMatrixHandle outputS()
    {}
    DenseMatrixHandle outputV()
    {}
    
}



TEST(ComputePCAtest, CenterData)
{
    ComputePCAAlgo algo;
    
    DenseMatrixHandle m1(inputMatrix());
    
    auto centered = ComputePCAAlgo::centerData(m1);
    
    auto expected = *centeredInputMatrix();
    
    for (int i = 0; i < centered.rows(); ++i) {
        for (int j = 0; j < centered.cols(); ++j)
            EXPECT_NEAR(expected(i,j), centered(i,j), 1e-5);
    }
}

//TEST(ComputePCAtest, output1)
//{
//    ComputePCAAlgorithm algo;
//    
//    DenseMatrixHandle m1(testMatrix());
//    ComputePCAAlgorithm::Outputs result = algo.run(ComputePCAAlgorithm::Inputs(m1),ComputePCAAlgorithm);
//    
//}
//
//TEST(ComputePCAtest, output2)
//{
//    ComputePCAAlgorithm algo;
//    
//    DenseMatrixHandle m1(testMatrix());
//    
//}
//
//TEST(ComputePCAtest, output3)
//{
//    ComputePCAAlgorithm algo;
//    
//    DenseMatrixHandle m1(testMatrix());
//    
//}