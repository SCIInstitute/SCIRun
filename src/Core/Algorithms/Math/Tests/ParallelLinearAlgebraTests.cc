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
 
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <fstream>
#include <Core/Algorithms/Math/ParallelAlgebra/ParallelLinearAlgebra.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/DenseColumnMatrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/MatrixComparison.h>
#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Core/Datatypes/MatrixIO.h>
#include <Testing/Utils/MatrixTestUtilities.h>
#include <boost/thread/thread.hpp>

using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms::Math;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::TestUtils;
using namespace SCIRun;
using namespace ::testing;

namespace
{
  const int size = 1000;
  SparseRowMatrixHandle matrix1() 
  {
    SparseRowMatrixHandle m(boost::make_shared<SparseRowMatrix>(size,size));
    m->insert(0,0) = 1;
    m->insert(1,2) = -1;
    m->insert(size-1,size-1) = 2;
    return m;
  }

  SparseRowMatrix matrix2() 
  {
    SparseRowMatrix m(size,size);
    m.insert(0,1) = -2;
    m.insert(1,1) = 0.5;
    m.insert(size-1,size-1) = 4;
    return m;
  }

  SparseRowMatrix matrix3() 
  {
    SparseRowMatrix m(size,size);
    m.insert(0,0) = -1;
    m.insert(1,2) = 1;
    m.insert(size-1,size-1) = -2;
    return m;
  }

  DenseColumnMatrixHandle vector1()
  {
    DenseColumnMatrixHandle v(boost::make_shared<DenseColumnMatrix>(size));
	  v->setZero();
//    *v << 1, 2, 4; 
	  (*v)[0] = 1;
  	(*v)[1] = 2;
	  (*v)[2] = 4;
    (*v)[size-1] = -1;
    return v;
  }

  DenseColumnMatrixHandle vector2()
  {
    DenseColumnMatrixHandle v(boost::make_shared<DenseColumnMatrix>(size));
    v->setZero();
    //*v << -1, -2, -4;
	  (*v)[0] = -1;
	  (*v)[1] = -2;
	  (*v)[2] = -4; 
    (*v)[size-1] = 1;
    return v;
  }

  DenseColumnMatrixHandle vector3()
  {
    DenseColumnMatrixHandle v(boost::make_shared<DenseColumnMatrix>(size));
    v->setZero();
   // *v << 0, 1, 0;
	  (*v)[0] = 0;
	  (*v)[1] = 1; 
	  (*v)[2] = 0; 
    (*v)[size-1] = -7;
    return v;
  }

  unsigned int numProcs()
  {
    return boost::thread::hardware_concurrency();
  }

  SolverInputs getDummySystem()
  {
    SolverInputs system;
    system.A = matrix1();
    system.b = vector1();
    system.x = vector2();
    system.x0 = vector3();
    return system;
  }

  const int SINGLE_THREADED_TEST_NUMPROCS = 1;
  const int SINGLE_THREADED_TEST_PROC_INDEX = 0;
}

TEST(ParallelLinearAlgebraTests, CanCreateEmptyParallelVector)
{
  ParallelLinearAlgebraSharedData data(getDummySystem(), SINGLE_THREADED_TEST_NUMPROCS);
  ParallelLinearAlgebra pla(data, SINGLE_THREADED_TEST_PROC_INDEX);

  ParallelLinearAlgebra::ParallelVector v;
  EXPECT_TRUE(pla.new_vector(v));

  EXPECT_EQ(size, v.size_);
}

TEST(ParallelLinearAlgebraTests, CanCreateParallelVectorFromVectorAsShallowReference)
{
  ParallelLinearAlgebraSharedData data(getDummySystem(), 1);
  ParallelLinearAlgebra pla(data, 0);

  ParallelLinearAlgebra::ParallelVector v;
  auto v1 = vector1();
  EXPECT_TRUE(pla.add_vector(v1, v));

  EXPECT_EQ(v1->nrows(), v.size_);
  for (size_t i = 0; i < size; ++i)
    EXPECT_EQ((*v1)[i], v.data_[i]);

  EXPECT_EQ(0, (*v1)[100]);
  v.data_[100]++;
  EXPECT_EQ(1, (*v1)[100]);
}

TEST(ParallelLinearAlgebraTests, CanCopyParallelSparseMatrixAsShallowReference)
{
  ParallelLinearAlgebraSharedData data(getDummySystem(), 1);
  ParallelLinearAlgebra pla(data, 0);

  ParallelLinearAlgebra::ParallelMatrix m;
  auto m1 = matrix1();
  EXPECT_TRUE(pla.add_matrix(m1, m));

  EXPECT_EQ(m1->nrows(), m.m_);
  EXPECT_EQ(m1->ncols(), m.n_);
  EXPECT_EQ(m1->nonZeros(), m.nnz_);
  EXPECT_EQ(m1->coeff(1,2), m.data_[1]);

  EXPECT_EQ(1, m1->coeff(0,0));
  m.data_[0]++;
  EXPECT_EQ(2, m1->coeff(0,0));
}

TEST(ParallelLinearAlgebraTests, CanCopyContentsOfVector)
{
  ParallelLinearAlgebraSharedData data(getDummySystem(), 1);
  ParallelLinearAlgebra pla(data, 0);

  ParallelLinearAlgebra::ParallelVector v1;
  pla.new_vector(v1);
  
  ParallelLinearAlgebra::ParallelVector v2;
  auto vec2 = vector2();
  pla.add_vector(vec2, v2);
  
  pla.copy(v2, v1);

  EXPECT_EQ(v1.size_, v2.size_);
  for (size_t i = 0; i < size; ++i)
  {
    EXPECT_EQ(v2.data_[i], v1.data_[i]);
  }
  v1.data_[7]++;
  EXPECT_NE(v1.data_[7], v2.data_[7]);
}

struct Copy
{
  Copy(ParallelLinearAlgebraSharedData& data, ParallelLinearAlgebra::ParallelVector& v1, ParallelLinearAlgebra::ParallelVector& v2, int proc, DenseColumnMatrixHandle vec2copy) : 
    data_(data), v1_(v1), v2_(v2), proc_(proc), vec2copy_(vec2copy) {}

  ParallelLinearAlgebraSharedData& data_;
  int proc_;
  ParallelLinearAlgebra::ParallelVector& v1_;
  ParallelLinearAlgebra::ParallelVector& v2_;
  DenseColumnMatrixHandle vec2copy_;

  void operator()()
  {
    ParallelLinearAlgebra pla(data_, proc_);
    pla.new_vector(v1_);
    pla.add_vector(vec2copy_, v2_);
    pla.copy(v2_, v1_);
  }
};

TEST(ParallelLinearAlgebraTests, CanCopyContentsOfVectorMulti)
{
	const int NUM_THREADS = 2;
  ParallelLinearAlgebraSharedData data(getDummySystem(), NUM_THREADS);
  
  ParallelLinearAlgebra::ParallelVector v1, v2;

  auto vec2copy = vector2();
  {
	  Copy c0(data, v1, v2, 0, vec2copy);
	  Copy c1(data, v1, v2, 1, vec2copy);
  
	  boost::thread t1 = boost::thread(boost::ref(c0));
	  boost::thread t2 = boost::thread(boost::ref(c1));
	  t1.join();
	  t2.join();
  }

  EXPECT_EQ(v1.size_, v2.size_);
  for (size_t i = 0; i < size; ++i)
  {
    EXPECT_EQ(v2.data_[i], v1.data_[i]);
  }
  v1.data_[7]++;
  EXPECT_NE(v1.data_[7], v2.data_[7]);
}


TEST(ParallelArithmeticTests, CanComputeMaxOfVector)
{
  ParallelLinearAlgebraSharedData data(getDummySystem(), SINGLE_THREADED_TEST_NUMPROCS);
  ParallelLinearAlgebra pla(data, SINGLE_THREADED_TEST_PROC_INDEX);

  ParallelLinearAlgebra::ParallelVector v1;
  auto vec1 = vector1();
  pla.add_vector(vec1, v1);
  double max1 = pla.max(v1);
  ParallelLinearAlgebra::ParallelVector v2;
  auto vec2 = vector2();
  pla.add_vector(vec2, v2);
  double max2 = pla.max(v2);
  
  EXPECT_EQ(4, max1);
  EXPECT_EQ(1, max2);
}

//TODO FIX_UNIT_TESTS--Intern.
//TODO: by intern
TEST(ParallelArithmeticTests, CanTakeAbsoluteValueOfDiagonal)
{
  ParallelLinearAlgebraSharedData data(getDummySystem(),1);
  ParallelLinearAlgebra pla(data,0); 

  ParallelLinearAlgebra::ParallelVector v1; 
  auto vec1 = vector1(); 
  vec1->setZero(); 
  pla.add_vector(vec1, v1);

  ParallelLinearAlgebra::ParallelMatrix m1;  
  auto mat1 = matrix1();
  pla.add_matrix(mat1 , m1); 

  pla.absdiag(m1, v1);
  for (size_t i = 0; i < size; ++i)
  {
	  EXPECT_GE(v1.data_[i],0);
  }
  EXPECT_EQ(v1.data_[0], 1); 
  EXPECT_EQ(v1.data_[2], 0);
  EXPECT_EQ(v1.data_[size-1], 2); 
}

//TODO: by intern
struct absdiag
{
   absdiag(ParallelLinearAlgebraSharedData& data, ParallelLinearAlgebra::ParallelMatrix& m1, ParallelLinearAlgebra::ParallelVector& v2, int proc, DenseColumnMatrixHandle dcmHandle) : 
    data_(data), m1_(m1), v2_(v2), proc_(proc), dcmHandle_(dcmHandle) {}

  ParallelLinearAlgebraSharedData& data_;
  int proc_;
  ParallelLinearAlgebra::ParallelMatrix& m1_;
  ParallelLinearAlgebra::ParallelVector& v2_;
  DenseColumnMatrixHandle dcmHandle_;

  void operator()()
  {
    ParallelLinearAlgebra pla(data_, proc_);
    pla.new_vector(v2_);
    auto mOne = matrix1();
    pla.add_matrix(mOne, m1_); 

    pla.absdiag(m1_, v2_);
  }
};

TEST(ParallelArithmeticTests, CanTakeAbsoluteValueOfDiagonalMulti)
{
  const int NUM_THREADS = 2;
  ParallelLinearAlgebraSharedData data(getDummySystem(), NUM_THREADS);
  
  ParallelLinearAlgebra::ParallelVector v2;
  ParallelLinearAlgebra::ParallelMatrix m1; 

  auto vec2 = vector2();
  auto mat1 = matrix1(); 

  {
	  absdiag diag0(data, m1, v2, 0, vec2);
	  absdiag diag1(data, m1, v2, 1, vec2);
  
	  boost::thread t1 = boost::thread(boost::ref(diag0));
	  boost::thread t2 = boost::thread(boost::ref(diag1));
	  t1.join();
	  t2.join();
  }

  EXPECT_EQ(v2.size_, size);
  for (size_t i = 0; i < size; ++i)
  {
    EXPECT_GE(v2.data_[i], 0);
  }
}

struct max
{
   max(ParallelLinearAlgebraSharedData& data, ParallelLinearAlgebra::ParallelVector& v1, int proc, DenseColumnMatrixHandle dcmHandle) : 
    data_(data), v1_(v1), proc_(proc), dcmHandle_(dcmHandle) {}

  ParallelLinearAlgebraSharedData& data_;
  int proc_;
  ParallelLinearAlgebra::ParallelVector& v1_;
  DenseColumnMatrixHandle dcmHandle_;
  double maxResult_; 

  void operator()()
  {
    ParallelLinearAlgebra pla(data_, proc_);
    auto vOne = vector1();
    pla.add_vector(vOne, v1_); 
    maxResult_ = pla.max(v1_);
  }
} ;

TEST(ParallelArithmeticTests, CanComputeMaxOfVectorMulti)
{
  //TODO: multi thread
  ParallelLinearAlgebraSharedData data(getDummySystem(), 2);

  ParallelLinearAlgebra::ParallelVector v1;
  double maxDouble_; 

  auto vec2 = vector2();
  {
	  max max0(data, v1, 0, vec2);
	  max max1(data, v1, 1, vec2);
  
	  boost::thread t1 = boost::thread(boost::ref(max0));
	  boost::thread t2 = boost::thread(boost::ref(max1));
	  t1.join();
	  t2.join();
    maxDouble_ = max0.maxResult_;
  }  
  EXPECT_EQ(4, maxDouble_);
}

//Find what error is acceptable for the float comparison
TEST(ParallelArithmeticTests, CanInvertElementsOfVectorWithAbsoluteValueThreshold)
{
	ParallelLinearAlgebraSharedData data(getDummySystem(),1);
	ParallelLinearAlgebra pla(data, 0);
	
  DenseColumnMatrixHandle v(boost::make_shared<DenseColumnMatrix>(size));
	v->setZero();

  ParallelLinearAlgebra::ParallelVector dummyResult;
	auto dcmDummy = v;
	pla.add_vector(dcmDummy, dummyResult);
 
  //test vector 1 
  ParallelLinearAlgebra::ParallelVector v1;
	auto vec1 = vector1();
	pla.add_vector(vec1, v1);
  pla.absthreshold_invert(v1, dummyResult, 1);
	EXPECT_EQ(dummyResult.data_[0],1);
	EXPECT_DOUBLE_EQ(dummyResult.data_[1], 0.5);
	EXPECT_DOUBLE_EQ(dummyResult.data_[2], 0.25);
	EXPECT_EQ(dummyResult.data_[size-1], 1);
	
  //test vector 2 
  pla.zeros(dummyResult); 
	ParallelLinearAlgebra::ParallelVector v2;
	auto vec2 = vector2(); 
	pla.add_vector(vec2, v2);
  pla.absthreshold_invert(v2, dummyResult, 1); 
  EXPECT_EQ(dummyResult.data_[0],1);
	EXPECT_DOUBLE_EQ(dummyResult.data_[1], -0.5);
	EXPECT_DOUBLE_EQ(dummyResult.data_[2], -0.25);
	EXPECT_EQ(dummyResult.data_[size-1], 1);
	
  //test vector 3 
  pla.zeros(dummyResult); 
  ParallelLinearAlgebra::ParallelVector v3;
	auto vec3 = vector3(); 
	pla.add_vector(vec3, v3);
  pla.absthreshold_invert(v3, dummyResult, 1); 
  EXPECT_EQ(dummyResult.data_[0], 1);
	EXPECT_EQ(dummyResult.data_[1], 1);
	EXPECT_EQ(dummyResult.data_[2], 1);
	EXPECT_NEAR(dummyResult.data_[size-1], -0.1429, 0.001);
}

//TODO: by intern
TEST(ParallelArithmeticTests, CanInvertElementsOfVectorWithAbsoluteValueThresholdMulti)
{
  EXPECT_TRUE(false);
}


//TODO: by intern
TEST(ParallelLinearAlgebraTests, CanFillVectorWithOnes)
{
  ParallelLinearAlgebraSharedData data(getDummySystem(),1);
  ParallelLinearAlgebra pla(data,0); 

  ParallelLinearAlgebra::ParallelVector v1; 
  auto vec1 = vector1(); 
  pla.add_vector(vec1, v1);

  pla.ones(v1); 

  for (size_t i = 0; i < size; ++i)
  {
    EXPECT_EQ(1,v1.data_[i]);
  }
}

//TODO: by intern
TEST(ParallelArithmeticTests, CanMultiplyMatrixByVector)
{
  ParallelLinearAlgebraSharedData data(getDummySystem(),1);
  ParallelLinearAlgebra pla(data,0); 

  ParallelLinearAlgebra::ParallelVector v1; 
  auto vec1 = vector1(); 
  pla.add_vector(vec1, v1);
   
  ParallelLinearAlgebra::ParallelVector v2; 
  auto vec2 = vector2(); 
  pla.add_vector(vec2, v2);
  pla.zeros(v2); 

  ParallelLinearAlgebra::ParallelMatrix m1;  
  auto mat1 = matrix1();
  pla.add_matrix(mat1 , m1);

  pla.mult(m1,v1,v2); 

  EXPECT_EQ(v2.data_[0],1);
  EXPECT_EQ(v2.data_[1],-4);
  EXPECT_EQ(v2.data_[size-1],-2); 
}

//TODO: by intern
TEST(ParallelArithmeticTests, CanMultiplyMatrixByVectorMulti)
{
  EXPECT_TRUE(false);
}

//TODO: by intern
TEST(ParallelArithmeticTests, CanSubtractVectors)
{
  ParallelLinearAlgebraSharedData data(getDummySystem(),1);
  ParallelLinearAlgebra pla(data,0); 

  ParallelLinearAlgebra::ParallelVector v1; 
  auto vec1 = vector1(); 
  pla.add_vector(vec1, v1);
   
  ParallelLinearAlgebra::ParallelVector v2; 
  auto vec2 = vector2(); 
  pla.add_vector(vec2, v2);
  
  ParallelLinearAlgebra::ParallelVector v3; 
  auto vec3 = vector3(); 
  pla.add_vector(vec3, v3);
  pla.zeros(v3); 

  pla.sub(v1, v2, v3); 
  EXPECT_EQ(v3.data_[0],2);
  EXPECT_EQ(v3.data_[1],4);
  EXPECT_EQ(v3.data_[2],8);
  EXPECT_EQ(v3.data_[3],0);
  EXPECT_EQ(v3.data_[size-1],-2);
}

//
TEST(ParallelArithmeticTests, CanCompute2Norm)
{
  ParallelLinearAlgebraSharedData data(getDummySystem(),1);
  ParallelLinearAlgebra pla(data,0);

  ParallelLinearAlgebra::ParallelVector v1;
  auto vec1 = vector1(); 
  pla.add_vector(vec1,v1);

  ParallelLinearAlgebra::ParallelVector v2;
  auto vec2 = vector2(); 
  pla.add_vector(vec2,v2);

  ParallelLinearAlgebra::ParallelVector v3;
  auto vec3 = vector3(); 
  pla.add_vector(vec3,v3);
  
  EXPECT_NEAR(4.6904,pla.norm(v1),0.001); 
  EXPECT_NEAR(4.6904,pla.norm(v2),0.001); 
  EXPECT_NEAR(7.0711,pla.norm(v3),0.001);
}


//TODO: by intern
TEST(ParallelArithmeticTests, CanCompute2NormMulti)
{
  EXPECT_TRUE(false);
}

//TODO: by intern
TEST(ParallelArithmeticTests, CanMultiplyVectorsComponentWise)
{
  ParallelLinearAlgebraSharedData data(getDummySystem(),1);
  ParallelLinearAlgebra pla(data,0);

  ParallelLinearAlgebra::ParallelVector v1;
  auto vec1 = vector1(); 
  pla.add_vector(vec1,v1);

  ParallelLinearAlgebra::ParallelVector v2;
  auto vec2 = vector2(); 
  pla.add_vector(vec2,v2);

  ParallelLinearAlgebra::ParallelVector v3;
  auto vec3 = vector3(); 
  pla.add_vector(vec3,v3);
  pla.zeros(v3); 

  pla.mult(v1,v2,v3); 
  EXPECT_EQ(v3.data_[0],-1);
  EXPECT_EQ(v3.data_[1],-4);
  EXPECT_EQ(v3.data_[2],-16);
  EXPECT_EQ(v3.data_[size-1],-1);

  pla.zeros(v2);
  auto resetV3 = vector3(); 
  pla.add_vector(resetV3,v3);
  pla.mult(v1,v3,v2); 
  EXPECT_EQ(v2.data_[0],0);
  EXPECT_EQ(v2.data_[1],2);
  EXPECT_EQ(v2.data_[2],0);
  EXPECT_EQ(v2.data_[size-1],7);
}

//TODO: by intern
TEST(ParallelArithmeticTests, CanComputeDotProduct)
{
  ParallelLinearAlgebraSharedData data(getDummySystem(),1);
  ParallelLinearAlgebra pla(data,0);

  ParallelLinearAlgebra::ParallelVector v1;
  auto vec1 = vector1(); 
  pla.add_vector(vec1,v1);

  ParallelLinearAlgebra::ParallelVector v2;
  auto vec2 = vector2(); 
  pla.add_vector(vec2,v2);

  ParallelLinearAlgebra::ParallelVector v3;
  auto vec3 = vector3(); 
  pla.add_vector(vec3,v3);

  EXPECT_EQ(pla.dot(v1,v2),-22); 
  EXPECT_EQ(pla.dot(v2,v3),-9); 
  EXPECT_EQ(pla.dot(v1,v3),9); 
}

#if 0 
//TODO: by intern
TEST(ParallelArithmeticTests, CanSubtractVectorsMulti)
{
  EXPECT_TRUE(false);
}

//TODO: by intern
TEST(ParallelArithmeticTests, CanMultiplyVectorsComponentWiseMulti)
{
  EXPECT_TRUE(false);
}

//TODO: by intern
TEST(ParallelArithmeticTests, CanComputeDotProductMulti)
{
  EXPECT_TRUE(false);
}
#endif
