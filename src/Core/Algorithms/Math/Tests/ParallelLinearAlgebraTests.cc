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
    (*v)[300] = -300;
    (*v)[size/2] = -6;
    (*v)[size/2 + 1] = -7;
    (*v)[600] = -600;
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
  Copy(ParallelLinearAlgebraSharedData& data, ParallelLinearAlgebra::ParallelVector& v1,
    ParallelLinearAlgebra::ParallelVector& v2, int proc, DenseColumnMatrixHandle vec2copy) :
    data_(data), proc_(proc), v1_(v1), v2_(v2), vec2copy_(vec2copy) {}

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
  EXPECT_EQ(1,v1.data_[0]);
  EXPECT_EQ(0,v1.data_[2]);
  EXPECT_EQ(2,v1.data_[size-1]);
}

struct absdiag
{
   absdiag(ParallelLinearAlgebraSharedData& data, ParallelLinearAlgebra::ParallelMatrix& m1,
     ParallelLinearAlgebra::ParallelVector& v2, int proc, DenseColumnMatrixHandle dcmHandle,
     SparseRowMatrixHandle srmHandle) : data_(data), proc_(proc), m1_(m1), v2_(v2),
     dcmHandle_(dcmHandle), srmHandle_(srmHandle) {}

  ParallelLinearAlgebraSharedData& data_;
  int proc_;
  ParallelLinearAlgebra::ParallelMatrix& m1_;
  ParallelLinearAlgebra::ParallelVector& v2_;
  DenseColumnMatrixHandle dcmHandle_;
  SparseRowMatrixHandle srmHandle_;

  void operator()()
  {
    ParallelLinearAlgebra pla(data_, proc_);
    pla.add_matrix(srmHandle_, m1_);

    pla.new_vector(v2_);
    pla.add_vector(dcmHandle_, v2_);

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
	  absdiag diag0(data, m1, v2, 0, vec2, mat1);
	  absdiag diag1(data, m1, v2, 1, vec2, mat1);

	  boost::thread t1 = boost::thread(boost::ref(diag0));
	  boost::thread t2 = boost::thread(boost::ref(diag1));
	  t1.join();
	  t2.join();
  }

  EXPECT_EQ( v2.size_, size );
  for (size_t i = 0; i < size; ++i)
  {
    EXPECT_GE(v2.data_[i] , 0);
  }
}

struct max
{
  max(ParallelLinearAlgebraSharedData& data, ParallelLinearAlgebra::ParallelVector& v1,
     int proc, DenseColumnMatrixHandle dcmHandle) :
      data_(data), proc_(proc), v1_(v1), dcmHandle_(dcmHandle) {}

  ParallelLinearAlgebraSharedData& data_;
  int proc_;
  ParallelLinearAlgebra::ParallelVector& v1_;
  DenseColumnMatrixHandle dcmHandle_;
  double maxResult_{0};

  void operator()()
  {
    ParallelLinearAlgebra pla(data_, proc_);
    pla.new_vector(v1_);
    pla.add_vector(dcmHandle_, v1_);
    maxResult_ = pla.max(v1_);
  }
};

TEST(ParallelArithmeticTests, CanComputeMaxOfVectorMulti)
{
  /// @todo: multi thread
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
  EXPECT_EQ(1, maxDouble_);
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

  EXPECT_EQ(1,dummyResult.data_[0]);
	EXPECT_DOUBLE_EQ(-0.5,dummyResult.data_[1]);
	EXPECT_DOUBLE_EQ(-0.25,dummyResult.data_[2]);
	EXPECT_EQ(1,dummyResult.data_[size-1]);

  //test vector 3
  pla.zeros(dummyResult);
  ParallelLinearAlgebra::ParallelVector v3;
	auto vec3 = vector3();
	pla.add_vector(vec3, v3);
  pla.absthreshold_invert(v3, dummyResult, 1);

  EXPECT_EQ(1,dummyResult.data_[0]);
	EXPECT_EQ(1,dummyResult.data_[1]);
	EXPECT_EQ(1,dummyResult.data_[2]);
	EXPECT_NEAR(-0.1429, dummyResult.data_[size-1], 0.001);
}

struct absthreshold_inv
{
   absthreshold_inv(ParallelLinearAlgebraSharedData& data, ParallelLinearAlgebra::ParallelVector& v1,
     ParallelLinearAlgebra::ParallelVector& v2, int proc, DenseColumnMatrixHandle dcmHandle) :
    data_(data), proc_(proc), v1_(v1), v2_(v2), dcmHandle_(dcmHandle) {}

  ParallelLinearAlgebraSharedData& data_;
  int proc_;
  ParallelLinearAlgebra::ParallelVector& v1_;
  ParallelLinearAlgebra::ParallelVector& v2_;
  DenseColumnMatrixHandle dcmHandle_;

  void operator()()
  {
    ParallelLinearAlgebra pla(data_, proc_);

    pla.new_vector(v1_);
    pla.add_vector(dcmHandle_, v1_);

    pla.new_vector(v2_);

    pla.absthreshold_invert(v1_, v2_, 1);
  }
};

TEST(ParallelArithmeticTests, CanInvertElementsOfVectorWithAbsoluteValueThresholdMulti)
{
  const int NUM_THREADS = 2;
  ParallelLinearAlgebraSharedData data(getDummySystem(), NUM_THREADS);

  ParallelLinearAlgebra::ParallelVector v1;
  ParallelLinearAlgebra::ParallelVector v2;

  auto vec2 = vector2();
  {
	  absthreshold_inv absthreshold_inv0(data, v1, v2, 0, vec2);
	  absthreshold_inv absthreshold_inv1(data, v1, v2, 1, vec2);

	  boost::thread t1 = boost::thread(boost::ref(absthreshold_inv0));
	  boost::thread t2 = boost::thread(boost::ref(absthreshold_inv1));
	  t1.join();
	  t2.join();
  }

  //test vector 1
	EXPECT_EQ(1,v2.data_[0]);
	EXPECT_NEAR(-0.5,v2.data_[1],0.001);
	EXPECT_NEAR(-0.25,v2.data_[2],0.001);
  EXPECT_NEAR(-0.0033,v2.data_[300],0.001);
  EXPECT_NEAR(-0.1667,v2.data_[size/2],0.001);
  EXPECT_NEAR(-0.1429,v2.data_[size/2 + 1],0.001);
  EXPECT_NEAR(-0.0017,v2.data_[600],0.001);
	EXPECT_EQ(1,v2.data_[size-1]);

}

TEST(ParallelArithmeticTests, CanInvertElementsOfVectorWithAbsoluteValueThresholdMulti8Threads)
{
  const int NUM_THREADS = 8;
  ParallelLinearAlgebraSharedData data(getDummySystem(), NUM_THREADS);

  ParallelLinearAlgebra::ParallelVector v1;
  ParallelLinearAlgebra::ParallelVector v2;

  auto vec2 = vector1();
  std::vector<boost::shared_ptr<absthreshold_inv>> workers;
  boost::thread_group threads;
  {
    for (int i = 0; i < NUM_THREADS; ++i)
      workers.push_back(boost::make_shared<absthreshold_inv>(data, v1, v2, i, vec2));

    for (int i = 0; i < NUM_THREADS; ++i)
      threads.create_thread(boost::ref(*workers[i]));

    threads.join_all();
  }

  //test vector 1
	EXPECT_EQ(1, v2.data_[0]);
	EXPECT_DOUBLE_EQ(0.5, v2.data_[1]);
	EXPECT_DOUBLE_EQ(0.25,v2.data_[2] );
  EXPECT_DOUBLE_EQ(1, v2.data_[300]);
  EXPECT_DOUBLE_EQ(1, v2.data_[size/2]);
  EXPECT_DOUBLE_EQ(1, v2.data_[size/2 + 1]);
  EXPECT_DOUBLE_EQ(1, v2.data_[600]);
	EXPECT_EQ(1, v2.data_[size-1]);

}

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

  EXPECT_EQ(1,v2.data_[0]);
  EXPECT_EQ(-4,v2.data_[1]);
  EXPECT_EQ(-2,v2.data_[size-1]);
}

struct mv_Multiply
{
  mv_Multiply(ParallelLinearAlgebraSharedData& data, ParallelLinearAlgebra::ParallelMatrix& m1,
    ParallelLinearAlgebra::ParallelVector& v2, ParallelLinearAlgebra::ParallelVector& vR,
    int proc, DenseColumnMatrixHandle dcmHandle, SparseRowMatrixHandle srmHandle) :
    data_(data), proc_(proc), m1_(m1), v2_(v2), vR_(vR), dcmHandle_(dcmHandle), srmHandle_(srmHandle) {}

  ParallelLinearAlgebraSharedData& data_;
  int proc_;
  ParallelLinearAlgebra::ParallelMatrix& m1_;
  ParallelLinearAlgebra::ParallelVector& v2_;
  ParallelLinearAlgebra::ParallelVector& vR_;
  DenseColumnMatrixHandle dcmHandle_;
  SparseRowMatrixHandle srmHandle_;

  void operator()()
  {
    ParallelLinearAlgebra pla(data_, proc_);

    pla.add_matrix(srmHandle_, m1_);

    pla.new_vector(v2_);
    pla.add_vector(dcmHandle_,v2_);

    pla.new_vector(vR_);
    pla.add_vector(dcmHandle_,vR_);

    pla.mult(m1_, v2_, vR_);
  }
};

TEST(ParallelArithmeticTests, CanMultiplyMatrixByVectorMulti)
{
  ParallelLinearAlgebraSharedData data(getDummySystem(), 2);

  ParallelLinearAlgebra::ParallelMatrix m1;
  ParallelLinearAlgebra::ParallelVector v2;
  ParallelLinearAlgebra::ParallelVector vR;

  auto vecR = vector2();
  auto mat1 = matrix1();
  {
	  mv_Multiply mult_0(data, m1, v2, vR, 0, vecR, mat1);
	  mv_Multiply mult_1(data, m1, v2, vR, 1, vecR, mat1);

	  boost::thread t1 = boost::thread(boost::ref(mult_0));
	  boost::thread t2 = boost::thread(boost::ref(mult_1));
	  t1.join();
	  t2.join();
  }

  EXPECT_EQ(-1,vR.data_[0]);
  EXPECT_EQ(4,vR.data_[1]);
  EXPECT_EQ(0,vR.data_[2]);
  EXPECT_EQ(2,vR.data_[size-1]);
}

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

struct subtract
{
  subtract(ParallelLinearAlgebraSharedData& data, ParallelLinearAlgebra::ParallelVector& v1,
    ParallelLinearAlgebra::ParallelVector& v2, ParallelLinearAlgebra::ParallelVector& vR,
    int proc, DenseColumnMatrixHandle dcmHandle, DenseColumnMatrixHandle dcmHandle2) :
    data_(data), proc_(proc), v1_(v1), v2_(v2), vR_(vR), dcmHandle_(dcmHandle), dcmHandle2_(dcmHandle2) {}

  ParallelLinearAlgebraSharedData& data_;
  int proc_;
  ParallelLinearAlgebra::ParallelVector& v1_;
  ParallelLinearAlgebra::ParallelVector& v2_;
  ParallelLinearAlgebra::ParallelVector& vR_;
  DenseColumnMatrixHandle dcmHandle_;
  DenseColumnMatrixHandle dcmHandle2_;

  void operator()()
  {
    ParallelLinearAlgebra pla(data_, proc_);

    pla.new_vector(v1_);
    pla.add_vector(dcmHandle_, v1_);

    pla.new_vector(v2_);
    pla.add_vector(dcmHandle2_,v2_);

    pla.new_vector(vR_);

    pla.ones(vR_);

    pla.sub(v1_, v2_, vR_);
  }
};

TEST(ParallelArithmeticTests, CanSubtractVectorsMulti)
{
  ParallelLinearAlgebraSharedData data(getDummySystem(),2);

  ParallelLinearAlgebra::ParallelVector v1;
  ParallelLinearAlgebra::ParallelVector v2;
  ParallelLinearAlgebra::ParallelVector vR;

  auto vec2 = vector2();
  auto vec1 = vector1();
    {
	    subtract sub_0(data, v1, v2, vR, 0, vec1, vec2);
	    subtract sub_1(data, v1, v2, vR, 1, vec1, vec2);

	    boost::thread t1 = boost::thread(boost::ref(sub_0));
	    boost::thread t2 = boost::thread(boost::ref(sub_1));
	    t1.join();
	    t2.join();
    }

  EXPECT_EQ(2,  vR.data_[0]);
  EXPECT_EQ(4,  vR.data_[1]);
  EXPECT_EQ(8,  vR.data_[2]);
  EXPECT_EQ(0,  vR.data_[3]);
  EXPECT_EQ(-2, vR.data_[size-1]);
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
  EXPECT_NEAR(670.900,pla.norm(v2),0.001);
  EXPECT_NEAR(7.0711,pla.norm(v3),0.001);
}


struct norm
{
  norm(ParallelLinearAlgebraSharedData& data, ParallelLinearAlgebra::ParallelVector& v1,
    ParallelLinearAlgebra::ParallelVector& v2, ParallelLinearAlgebra::ParallelVector& v3,
    int proc, DenseColumnMatrixHandle dcmHandle1, DenseColumnMatrixHandle dcmHandle2
    , DenseColumnMatrixHandle dcmHandle3) :
    data_(data), proc_(proc), v1_(v1), v2_(v2), v3_(v3), dcmHandle1_(dcmHandle1),
    dcmHandle2_(dcmHandle2), dcmHandle3_(dcmHandle3) {}

  ParallelLinearAlgebraSharedData& data_;
  int proc_;
  ParallelLinearAlgebra::ParallelVector& v1_;
  ParallelLinearAlgebra::ParallelVector& v2_;
  ParallelLinearAlgebra::ParallelVector& v3_;
  DenseColumnMatrixHandle dcmHandle1_;
  DenseColumnMatrixHandle dcmHandle2_;
  DenseColumnMatrixHandle dcmHandle3_;
  double v1Norm_;
  double v2Norm_;
  double v3Norm_;

  void operator()()
  {
    ParallelLinearAlgebra pla(data_, proc_);

    pla.new_vector(v1_);
    pla.add_vector(dcmHandle1_,v1_);

    pla.new_vector(v2_);
    pla.add_vector(dcmHandle2_,v2_);

    pla.new_vector(v3_);
    pla.add_vector(dcmHandle3_,v3_);

    v1Norm_ = pla.norm(v1_);
    v2Norm_ = pla.norm(v2_);
    v3Norm_ = pla.norm(v3_);
  }
};

TEST(ParallelArithmeticTests, CanCompute2NormMulti)
{
  ParallelLinearAlgebraSharedData data(getDummySystem(), 2);

  ParallelLinearAlgebra::ParallelVector v1;
  ParallelLinearAlgebra::ParallelVector v2;
  ParallelLinearAlgebra::ParallelVector v3;
  double v1Norm_Result;
  double v2Norm_Result;
  double v3Norm_Result;

  auto vec1 = vector1();
  auto vec2 = vector2();
  auto vec3 = vector3();
  {
	  norm norm_0(data, v1, v2, v3, 0, vec1, vec2, vec3);
	  norm norm_1(data, v1, v2, v3, 1, vec1, vec2, vec3);

	  boost::thread t1 = boost::thread(boost::ref(norm_0));
	  boost::thread t2 = boost::thread(boost::ref(norm_1));
	  t1.join();
	  t2.join();

    v1Norm_Result = norm_0.v1Norm_;
    v2Norm_Result = norm_0.v2Norm_;
    v3Norm_Result = norm_0.v3Norm_;
  }

  EXPECT_NEAR(4.6904,v1Norm_Result,0.001);
  EXPECT_NEAR(670.900,v2Norm_Result,0.001);
  EXPECT_NEAR(7.0711,v3Norm_Result,0.001);
}

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

  EXPECT_EQ(0,v2.data_[0]);
  EXPECT_EQ(2,v2.data_[1]);
  EXPECT_EQ(0,v2.data_[2]);
  EXPECT_EQ(7,v2.data_[size-1]);
}
struct multVectors
{
  multVectors(ParallelLinearAlgebraSharedData& data, ParallelLinearAlgebra::ParallelVector& v1,
    ParallelLinearAlgebra::ParallelVector& v2, ParallelLinearAlgebra::ParallelVector& v3,
    int proc, DenseColumnMatrixHandle dcmHandle1, DenseColumnMatrixHandle dcmHandle2) :
    data_(data), proc_(proc), v1_(v1), v2_(v2), v3_(v3),
      dcmHandle1_(dcmHandle1), dcmHandle2_(dcmHandle2) {}

  ParallelLinearAlgebraSharedData& data_;
  int proc_;
  ParallelLinearAlgebra::ParallelVector& v1_;
  ParallelLinearAlgebra::ParallelVector& v2_;
  ParallelLinearAlgebra::ParallelVector& v3_;
  DenseColumnMatrixHandle dcmHandle1_;
  DenseColumnMatrixHandle dcmHandle2_;

  void operator()()
  {
    ParallelLinearAlgebra pla(data_, proc_);

    pla.new_vector(v1_);
    pla.add_vector(dcmHandle1_, v1_);

    pla.new_vector(v2_);
    pla.add_vector(dcmHandle2_,v2_);

    pla.new_vector(v3_);

    pla.mult(v1_,v2_,v3_);
  }
};
/// @todo: by intern
TEST(ParallelArithmeticTests, CanMultiplyVectorsComponentWiseMulti)
{
  ParallelLinearAlgebraSharedData data(getDummySystem(), 2);

  ParallelLinearAlgebra::ParallelVector v1;
  ParallelLinearAlgebra::ParallelVector v2;
  ParallelLinearAlgebra::ParallelVector v3;

  auto vec1 = vector1();
  auto vec2 = vector2();
  {
	  multVectors mult_0(data, v1, v2, v3, 0, vec1, vec2);
	  multVectors mult_1(data, v1, v2, v3, 1, vec1, vec2);

	  boost::thread t1 = boost::thread(boost::ref(mult_0));
	  boost::thread t2 = boost::thread(boost::ref(mult_1));
	  t1.join();
	  t2.join();
  }

  EXPECT_EQ(-1  , v3.data_[0]);
  EXPECT_EQ(-4  , v3.data_[1]);
  EXPECT_EQ(-16 , v3.data_[2]);
  EXPECT_EQ(-1  , v3.data_[size-1]);
}

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

  EXPECT_EQ(-22 , pla.dot(v1,v2));
  EXPECT_EQ(-9 , pla.dot(v2,v3));
  EXPECT_EQ(9 , pla.dot(v1,v3));
}

struct dotMult
{
  dotMult(ParallelLinearAlgebraSharedData& data, ParallelLinearAlgebra::ParallelVector& v1,
    ParallelLinearAlgebra::ParallelVector& v2, ParallelLinearAlgebra::ParallelVector& v3,
    int proc, DenseColumnMatrixHandle dcmHandle, DenseColumnMatrixHandle dcmHandle2,
    DenseColumnMatrixHandle dcmHandle3) :
      data_(data), proc_(proc), v1_(v1), v2_(v2), v3_(v3),
      dcmHandle_(dcmHandle), dcmHandle2_(dcmHandle2), dcmHandle3_(dcmHandle3) {}

  ParallelLinearAlgebraSharedData& data_;
  int proc_;
  ParallelLinearAlgebra::ParallelVector& v1_;
  ParallelLinearAlgebra::ParallelVector& v2_;
  ParallelLinearAlgebra::ParallelVector& v3_;
  DenseColumnMatrixHandle dcmHandle_;
  DenseColumnMatrixHandle dcmHandle2_;
  DenseColumnMatrixHandle dcmHandle3_;
  double v12_;
  double v23_;
  double v13_;

  void operator()()
  {
    ParallelLinearAlgebra pla(data_, proc_);

    pla.new_vector(v1_);
    pla.add_vector(dcmHandle_, v1_);

    pla.new_vector(v2_);
    pla.add_vector(dcmHandle2_,v2_);

    pla.new_vector(v3_);
    pla.add_vector(dcmHandle3_,v3_);


    v12_ = pla.dot(v1_,v2_);
    v23_ = pla.dot(v2_,v3_);
    v13_ = pla.dot(v1_,v3_);
  }
};

TEST(ParallelArithmeticTests, CanComputeDotProductMulti)
{
  ParallelLinearAlgebraSharedData data(getDummySystem(), 2);

  ParallelLinearAlgebra::ParallelVector v1;
  ParallelLinearAlgebra::ParallelVector v2;
  ParallelLinearAlgebra::ParallelVector v3;
  double v12;
  double v23;
  double v13;

  auto vec1 = vector1();
  auto vec2 = vector2();
  auto vec3 = vector3();
  {
	  dotMult dotMult_0(data, v1, v2, v3, 0, vec1, vec2, vec3);
	  dotMult dotMult_1(data, v1, v2, v3, 1, vec1, vec2, vec3);

	  boost::thread t1 = boost::thread(boost::ref(dotMult_0));
	  boost::thread t2 = boost::thread(boost::ref(dotMult_1));
	  t1.join();
	  t2.join();
    v12 = dotMult_0.v12_;
    v23 = dotMult_0.v23_;
    v13 = dotMult_0.v13_;
  }

  EXPECT_EQ(-22 , v12);
  EXPECT_EQ(-9 , v23);
  EXPECT_EQ(9 , v13);
}
