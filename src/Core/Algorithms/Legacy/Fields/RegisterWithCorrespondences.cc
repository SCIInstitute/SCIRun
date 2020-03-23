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


#include <Core/Algorithms/Legacy/Fields/RegisterWithCorrespondences.h>

#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/GeometryPrimitives/Vector.h>
#include <Core/GeometryPrimitives/Point.h>
#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Eigen/SVD>
#include <Core/Datatypes/Legacy/Field/VMesh.h>

#include <sstream>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Utility;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Core::Geometry;


static void printMatrix(const DenseMatrix& m, const std::string& tag = "tag")
{
#if 0
  std::cout << tag << std::endl;
  std::cout << "Size: " << m.nrows() << " x " << m.ncols() << std::endl;
  std::cout << "Min/Max: " << std::setprecision(15) << m.minCoeff() << " , " << m.maxCoeff() << std::endl;
#endif
}

RegisterWithCorrespondencesAlgo::RegisterWithCorrespondencesAlgo()
{
  addParameter(Variables::Operator, static_cast<int>(TransformType::AFFINE));
}

AlgorithmOutput RegisterWithCorrespondencesAlgo::run(const AlgorithmInput& input) const
{
  auto input_field = input.get<Field>(Variables::InputField);
  auto corres1 = input.get<Field>(Correspondences1);
  auto corres2 = input.get<Field>(Correspondences2);

  FieldHandle return_field;
  DenseMatrixHandle transform;

  auto op = TransformType(get(Variables::Operator).toInt());
  switch (op)
  {
  case TransformType::MORPH:
    transform = runMorph(input_field, corres1, corres2, return_field);
    break;
  case TransformType::AFFINE:
    transform = runAffine(input_field, corres1, corres2, return_field);
    break;
  case TransformType::RIGID:
    transform = runRigid_P(input_field, corres1, corres2, return_field);
    break;
  case TransformType::NONE:
    transform = runNone(input_field, corres1, corres2, return_field);
    break;
  }

  printMatrix(*transform,"Transform");

  AlgorithmOutput output;
  output[Variables::OutputField] = return_field;
  output[TransformMatrix] = transform;
  return output;
}

DenseMatrixHandle RegisterWithCorrespondencesAlgo::runMorph(FieldHandle input, FieldHandle Cors1, FieldHandle Cors2, FieldHandle& output)  const
{
  double sumx2;
  double sumy2;
  double sumz2;
  double sumx;
  double sumy;
  double sumz;

  if (!input) {
    error("No input field");
    return nullptr;
  }
  if (!Cors1) {
    error("No Correspondence1 input field");
    return nullptr;
  }
  if (!Cors2) {
    error("No Correspondence2 input field");
    return nullptr;
  }

  FieldInformation fi(input);

  output.reset(input->deep_clone());

  FieldHandle input_cp, Cors1_cp, Cors2_cp;
  input_cp.reset(input->deep_clone());
  Cors1_cp.reset(Cors1->deep_clone());
  Cors2_cp.reset(Cors2->deep_clone());

  if (!output)
  {
    error("Could not allocate output field");
    return nullptr;
  }

  VMesh* imesh = input_cp->vmesh();
  VMesh* omesh = output->vmesh();
  VMesh* icors1 = Cors1_cp->vmesh();
  VMesh* icors2 = Cors2_cp->vmesh();

  //get the number of nodes in input field
  //VMesh::size_type num_nodes = imesh->num_nodes();

  VMesh::Node::size_type num_cors1, num_cors2, num_pts;
  icors1->size(num_cors1);
  icors2->size(num_cors2);
  imesh->size(num_pts);

  std::vector<double> coefs;//(3*num_cors1+9);
  std::vector<double> rside;//(3*num_cors1+9);
  if (num_cors1 != num_cors2)
  {
    error("Number of correspondence points does not match");
    return nullptr;
  }

  // Request that it generates the node matrix
  imesh->synchronize(SCIRun::Mesh::NODES_E);

  //get centroids of both point clouds
  sumx = 0.0;
  sumy = 0.0;
  sumz = 0.0;
  SCIRun::Core::Geometry::Point mp;

  VMesh::size_type num_nodes = icors1->num_nodes();

  for (VMesh::Node::index_type idx = 0; idx < num_nodes; idx++)
  {
    icors1->get_center(mp, idx);
    sumx = mp.x() + sumx;
    sumy = mp.y() + sumy;
    sumz = mp.z() + sumz;
  }
  sumx = sumx / (double)num_nodes;
  sumy = sumy / (double)num_nodes;
  sumz = sumz / (double)num_nodes;

  sumx2 = 0.0;
  sumy2 = 0.0;
  sumz2 = 0.0;
  Point np;

  VMesh::size_type num_nodes2 = icors2->num_nodes();

  for (VMesh::Node::index_type idx = 0; idx < num_nodes2; idx++)
  {
    icors2->get_center(np, idx);
    sumx2 = np.x() + sumx2;
    sumy2 = np.y() + sumy2;
    sumz2 = np.z() + sumz2;
  }
  sumx2 = sumx2 / (double)num_nodes2;
  sumy2 = sumy2 / (double)num_nodes2;
  sumz2 = sumz2 / (double)num_nodes2;

  //center fields
  SCIRun::Core::Geometry::Point mypoint;

  for (VMesh::Node::index_type idx = 0; idx < num_nodes; idx++)
  {
    icors1->get_center(mp, idx);
    mypoint.x(mp.x() - sumx);
    mypoint.y(mp.y() - sumy);
    mypoint.z(mp.z() - sumz);
    icors1->set_point(mypoint, idx);
  }

  for (VMesh::Node::index_type idx = 0; idx < num_nodes2; idx++)
  {
    icors2->get_center(mp, idx);
    mypoint.x(mp.x() - sumx2);
    mypoint.y(mp.y() - sumy2);
    mypoint.z(mp.z() - sumz2);
    icors2->set_point(mypoint, idx);
  }
  VMesh::size_type num_nodes_mesh = imesh->num_nodes();
  for (VMesh::Node::index_type idx = 0; idx < num_nodes_mesh; idx++)
  {
    imesh->get_center(mp, idx);
    mypoint.x(mp.x() - sumx2);
    mypoint.y(mp.y() - sumy2);
    mypoint.z(mp.z() - sumz2);
    imesh->set_point(mypoint, idx);
  }

  //create B for big matrix//
  DenseMatrix Bm(num_cors1 + 4, num_cors1 + 4);
  VMesh::Node::iterator it;
  SCIRun::Core::Geometry::Point P;

  for (int L1 = 0; L1 < num_cors1; ++L1)
  {
    it = L1;
    icors2->get_point(P, *(it));
    //horizontal x,y,z
    Bm(0, L1) = P.x();
    Bm(1, L1) = P.y();
    Bm(2, L1) = P.z();
    Bm(3, L1) = 1;

    //vertical x,y,z
    Bm(L1 + 4, num_cors1) = P.x();
    Bm(L1 + 4, num_cors1 + 1) = P.y();
    Bm(L1 + 4, num_cors1 + 2) = P.z();
    Bm(L1 + 4, num_cors1 + 3) = 1;
  }

  for (int L1 = num_cors1; L1 < num_cors1 + 4; ++L1)
  {
    Bm(0, L1) = 0;
    Bm(1, L1) = 0;
    Bm(2, L1) = 0;
    Bm(3, L1) = 0;
  }

  //put in sigmas
  DenseMatrixHandle SMat;
  radial_basis_func(icors2, icors2, SMat);

  for (int i = 0; i < num_cors1; ++i)
  {
    for (int j = 0; j < num_cors1; ++j)
    {
      Bm(4 + i, j) = SMat->get(i, j);
    }
  }

  //Create big matrix //
  DenseMatrix BigMat(3 * num_cors1 + 12, 3 * num_cors1 + 12);

  for (int i = 0; i < (3 * num_cors1 + 12); ++i)
  {
    for (int j = 0; j < (3 * num_cors1 + 12); ++j)
    {
      BigMat(i, j) = 0;
    }
  }
  for (int i = 0; i < (num_cors1 + 4); ++i)
  {
    for (int j = 0; j < (num_cors1 + 4); ++j)
    {
      BigMat(i, j) = Bm(i, j);
    }
  }
  for (int i = 0; i < (num_cors1 + 4); ++i)
  {
    for (int j = 0; j < (num_cors1 + 4); ++j)
    {
      BigMat(num_cors1 + 4 + i, num_cors1 + 4 + j) = Bm(i, j);
    }
  }
  for (int i = 0; i < (num_cors1 + 4); ++i)
  {
    for (int j = 0; j < (num_cors1 + 4); ++j)
    {
      BigMat(2 * num_cors1 + 8 + i, 2 * num_cors1 + 8 + j) = Bm(i, j);
    }
  }

  //create right side of equation//
  for (int i = 0; i < 4; ++i){ rside.push_back(0); }
  for (int i = 0; i < num_cors1; ++i)
  {
    it = i;
    icors1->get_point(P, *(it));
    rside.push_back(P.x());
  }
  for (int i = 0; i < 4; ++i){ rside.push_back(0); }
  for (int i = 0; i < num_cors1; ++i)
  {
    it = i;
    icors1->get_point(P, *(it));
    rside.push_back(P.y());
  }
  for (int i = 0; i < 4; ++i){ rside.push_back(0); }
  for (int i = 0; i < num_cors1; ++i)
  {
    it = i;
    icors1->get_point(P, *(it));
    rside.push_back(P.z());
  }

  //Solve system of equations//
  // Bigm->solve(rside,coefs,0);
  //Use svd linear least squares

  //Create sparse matrix for sigmas of svd

  int m = 3 * num_cors1 + 12;
  int n = 3 * num_cors1 + 12;

  //create the U and V matrix
  Eigen::JacobiSVD<DenseMatrix::EigenBase> svd_mat(BigMat, Eigen::ComputeFullU | Eigen::ComputeFullV);
  DenseMatrix UMat = svd_mat.matrixU();
  DenseMatrix VMat = svd_mat.matrixV();
  DenseMatrix matSingularValues = svd_mat.singularValues();

  //Make more storage for the solving the linear least squares
  DenseMatrix RsideMat(m, 1);
  DenseMatrix CMat(n, 1);
  DenseMatrix YMat(n, 1);
  DenseMatrix CoefMat(n, 1);


  for (int loop = 0; loop < n; ++loop)
  {
    RsideMat(loop, 0) = rside[loop];
  }

  //c=trans(Um)*rside;
  CMat = UMat.transpose() * RsideMat;

  for (int k = 0; k<n; k++)
  {
    YMat(k, 0) = CMat(k, 0) / matSingularValues(k, 0);
  }

  CoefMat = VMat * YMat;

  for (int p = 0; p < n; p++)
  {
    coefs.push_back(CoefMat(p, 0));
  }
  DenseMatrixHandle transform(new DenseMatrix(CoefMat.matrix()));

  //done with solve, make the new field

  make_new_points(imesh, icors2, coefs, *omesh, sumx, sumy, sumz);

  return transform;
}

DenseMatrixHandle RegisterWithCorrespondencesAlgo::runAffine(FieldHandle input, FieldHandle Cors1, FieldHandle Cors2, FieldHandle& output) const
{
  double sumx2;
  double sumy2;
  double sumz2;
  double sumx;
  double sumy;
  double sumz;
  if (!input)
  {
    error("No input field");
    return nullptr;
  }
  if (!Cors1)
  {
    error("No Correspondence1 input field");
    return nullptr;
  }
  if (!Cors2)
  {
    error("No Correspndence2 input field");
    return nullptr;
  }

  FieldInformation fi(input);

  FieldHandle input_cp, Cors1_cp, Cors2_cp;

  output.reset(input->deep_clone());
  input_cp.reset(input->deep_clone());
  Cors1_cp.reset(Cors1->deep_clone());
  Cors2_cp.reset(Cors2->deep_clone());

  if (!output)
  {
    error("Could not allocate output field");
    return nullptr;
  }

  VMesh* imesh = input_cp->vmesh();
  VMesh* omesh = output->vmesh();
  VMesh* icors1 = Cors1_cp->vmesh();
  VMesh* icors2 = Cors2_cp->vmesh();

  //get the number of nodes in input field
  //VMesh::size_type num_nodes = imesh->num_nodes();

  VMesh::Node::size_type num_cors1, num_cors2, num_pts;
  icors1->size(num_cors1);
  icors2->size(num_cors2);
  imesh->size(num_pts);

  std::vector<double> coefs;//(3*num_cors1+9);
  std::vector<double> rside;//(3*num_cors1+9);

  if (num_cors1 != num_cors2)
  {
    error("Number of correspondence points does not match");
    return nullptr;
  }

  // Request that it generates the node matrix
  imesh->synchronize(SCIRun::Mesh::NODES_E);

  //get centroids of both point clouds
  sumx = 0.0;
  sumy = 0.0;
  sumz = 0.0;
  Point mp;

  VMesh::size_type num_nodes = icors1->num_nodes();

  for (VMesh::Node::index_type idx = 0; idx < num_nodes; idx++)
  {
    icors1->get_center(mp, idx);
    sumx = mp.x() + sumx;
    sumy = mp.y() + sumy;
    sumz = mp.z() + sumz;
  }
  sumx = sumx / (double)num_nodes;
  sumy = sumy / (double)num_nodes;
  sumz = sumz / (double)num_nodes;

  sumx2 = 0.0;
  sumy2 = 0.0;
  sumz2 = 0.0;
  Point np;

  VMesh::size_type num_nodes2 = icors2->num_nodes();

  for (VMesh::Node::index_type idx = 0; idx < num_nodes2; idx++)
  {
    icors2->get_center(np, idx);
    sumx2 = np.x() + sumx2;
    sumy2 = np.y() + sumy2;
    sumz2 = np.z() + sumz2;
  }
  sumx2 = sumx2 / (double)num_nodes;
  sumy2 = sumy2 / (double)num_nodes;
  sumz2 = sumz2 / (double)num_nodes;

  //center fields
  Point mypoint;

  for (VMesh::Node::index_type idx = 0; idx < num_nodes; idx++)
  {
    icors1->get_center(mp, idx);
    mypoint.x(mp.x() - sumx);
    mypoint.y(mp.y() - sumy);
    mypoint.z(mp.z() - sumz);
    icors1->set_point(mypoint, idx);
  }

  for (VMesh::Node::index_type idx = 0; idx < num_nodes2; idx++)
  {
    icors2->get_center(mp, idx);
    mypoint.x(mp.x() - sumx2);
    mypoint.y(mp.y() - sumy2);
    mypoint.z(mp.z() - sumz2);
    icors2->set_point(mypoint, idx);
  }

  VMesh::size_type num_nodes_mesh = imesh->num_nodes();
  for (VMesh::Node::index_type idx = 0; idx < num_nodes_mesh; idx++)
  {
    imesh->get_center(mp, idx);
    mypoint.x(mp.x() - sumx2);
    mypoint.y(mp.y() - sumy2);
    mypoint.z(mp.z() - sumz2);
    imesh->set_point(mypoint, idx);
  }

  //create B for big matrix//
  DenseMatrix BMat(num_cors1, 4);
  VMesh::Node::iterator it;
  Point P;

  for (int L1 = 0; L1 < num_cors1; ++L1)
  {
    it = L1;
    icors2->get_point(P, *(it));

    //horizontal x,y,z
    BMat(L1, 0) = P.x();
    BMat(L1, 1) = P.y();
    BMat(L1, 2) = P.z();
    BMat(L1, 3) = 1;
  }


    printMatrix(BMat, "BMat");


  //create right side of equation//
  for (int i = 0; i < num_cors1; ++i)
  {
    it = i;
    icors1->get_point(P, *(it));
    rside.push_back(P.x());
  }
  for (int i = 0; i < num_cors1; ++i)
  {
    it = i;
    icors1->get_point(P, *(it));
    rside.push_back(P.y());
  }
  for (int i = 0; i < num_cors1; ++i)
  {
    it = i;
    icors1->get_point(P, *(it));
    rside.push_back(P.z());
  }

  //Create sparse matrix for sigmas of svd
  int m = num_cors1;
  int n = 4;

  //create the U and V matrix

  printMatrix(BMat, "BMat");
  Eigen::JacobiSVD<DenseMatrix::EigenBase> svd_mat(BMat, Eigen::ComputeFullU | Eigen::ComputeFullV);
  DenseMatrix UMat = svd_mat.matrixU();
  DenseMatrix VMat = svd_mat.matrixV();
  DenseMatrix matSingularValues(svd_mat.singularValues());

  printMatrix(UMat, "UMat");
  printMatrix(VMat, "VMat");
  printMatrix(matSingularValues, "matS");
  //std::cout << "entire matS: \n" << matSingularValues << std::endl;
  //std::cout << "entire UMat: \n" << UMat << std::endl;
  //std::cout << "entire VMat: \n" << VMat << std::endl;


  //Make more storage for the solving the linear least squares
  DenseMatrix RsideMat(m, 1);
  DenseMatrix CMat(m, 1);
  DenseMatrix YMat(n, 1);
  DenseMatrix CoefMat(n, 1);

  for (int xyz = 0; xyz < 3; xyz++)
  {
    //std::cout << "\n-----XYZ " << xyz << "\n" << std::endl;
    for (int loop = 0; loop < m; ++loop)
    {
      RsideMat(loop, 0) = rside[xyz*m + loop];
    }
    printMatrix(RsideMat, "RsideMat");
    //c=trans(Um)*rside;
    CMat = UMat.transpose() * RsideMat;
    printMatrix(CMat, "CMat");

    YMat = CMat.cwiseQuotient(matSingularValues);
    printMatrix(YMat, "YMat");

    CoefMat = VMat * YMat;



    printMatrix(CoefMat, "CoefMat");
    for (int p = 0; p < n; p++)
    {
      coefs.push_back(CoefMat(p, 0));
    }
  }

  DenseMatrix rotation(Eigen::MatrixXd::Identity(4,4));
  DenseMatrix trans1(Eigen::MatrixXd::Identity(4,4));
  DenseMatrix trans2(Eigen::MatrixXd::Identity(4,4));
  double *r_data = rotation.data();

  std::copy(coefs.begin(), coefs.end(), r_data);

  // recreate previous translation
  trans1(0,3) = -sumx2;
  trans1(1,3) = -sumy2;
  trans1(2,3) = -sumz2;

  trans2(0,3) = sumx;
  trans2(1,3) = sumy;
  trans2(2,3) = sumz;

  auto trans_out = boost::make_shared<DenseMatrix>(trans2*rotation*trans1);


  //done with solve, make the new field
  make_new_pointsA(imesh, icors2, coefs, *omesh, sumx, sumy, sumz);





#if 0
  {
    const BBox bbox = omesh->get_bounding_box();
    if (bbox.valid())
    {
      std::cout << "output size: " << bbox.diagonal() << " center: " << bbox.center() << std::endl;
    }
    else
    {
      std::cout << "output bbox invalid" << std::endl;
    }
  }
#endif

  return trans_out;
}


DenseMatrixHandle RegisterWithCorrespondencesAlgo::runRigid_P(FieldHandle input, FieldHandle Cors1, FieldHandle Cors2, FieldHandle& output) const
{
    double sumx2;
    double sumy2;
    double sumz2;
    double sumx;
    double sumy;
    double sumz;
    if (!input)
    {
        error("No input field");
        return nullptr;
    }
    if (!Cors1)
    {
        error("No Correspondence1 input field");
        return nullptr;
    }
    if (!Cors2)
    {
        error("No Correspndence2 input field");
        return nullptr;
    }

    FieldInformation fi(input);

    FieldHandle input_cp, Cors1_cp, Cors2_cp;

    output.reset(input->deep_clone());
    input_cp.reset(input->deep_clone());
    Cors1_cp.reset(Cors1->deep_clone());
    Cors2_cp.reset(Cors2->deep_clone());

    if (!output)
    {
        error("Could not allocate output field");
        return nullptr;
    }

    VMesh* imesh = input_cp->vmesh();
    VMesh* omesh = output->vmesh();
    VMesh* icors1 = Cors1_cp->vmesh();
    VMesh* icors2 = Cors2_cp->vmesh();

    //get the number of nodes in input field
    //VMesh::size_type num_nodes = imesh->num_nodes();

    VMesh::Node::size_type num_cors1, num_cors2, num_pts;
    icors1->size(num_cors1);
    icors2->size(num_cors2);
    imesh->size(num_pts);

    if (num_cors1 != num_cors2)
    {
        error("Number of correspondence points does not match");
        return nullptr;
    }

    // Request that it generates the node matrix
    imesh->synchronize(SCIRun::Mesh::NODES_E);

    //get centroids of both point clouds
    sumx = 0.0;
    sumy = 0.0;
    sumz = 0.0;
    Point mp;

    VMesh::size_type num_nodes = icors1->num_nodes();

    for (VMesh::Node::index_type idx = 0; idx < num_nodes; idx++)
    {
        icors1->get_center(mp, idx);
        sumx = mp.x() + sumx;
        sumy = mp.y() + sumy;
        sumz = mp.z() + sumz;
    }
    sumx = sumx / (double)num_nodes;
    sumy = sumy / (double)num_nodes;
    sumz = sumz / (double)num_nodes;

    sumx2 = 0.0;
    sumy2 = 0.0;
    sumz2 = 0.0;
    Point np;

    VMesh::size_type num_nodes2 = icors2->num_nodes();

    for (VMesh::Node::index_type idx = 0; idx < num_nodes2; idx++)
    {
        icors2->get_center(np, idx);
        sumx2 = np.x() + sumx2;
        sumy2 = np.y() + sumy2;
        sumz2 = np.z() + sumz2;
    }
    sumx2 = sumx2 / (double)num_nodes;
    sumy2 = sumy2 / (double)num_nodes;
    sumz2 = sumz2 / (double)num_nodes;

    //center fields
    Point mypoint;

    for (VMesh::Node::index_type idx = 0; idx < num_nodes; idx++)
    {
        icors1->get_center(mp, idx);
        mypoint.x(mp.x() - sumx);
        mypoint.y(mp.y() - sumy);
        mypoint.z(mp.z() - sumz);
        icors1->set_point(mypoint, idx);
    }

    for (VMesh::Node::index_type idx = 0; idx < num_nodes2; idx++)
    {
        icors2->get_center(mp, idx);
        mypoint.x(mp.x() - sumx2);
        mypoint.y(mp.y() - sumy2);
        mypoint.z(mp.z() - sumz2);
        icors2->set_point(mypoint, idx);
    }


    //normalize the point matrices

    double norm1;
    double norm2;
    double ssq1x=0,ssq1y=0,ssq1z=0;
    double ssq2x=0,ssq2y=0,ssq2z=0;
    Point sqp1,sqp2,normp1,normp2;

    for (VMesh::Node::index_type idx = 0; idx < num_nodes; idx++)
    {
        icors1->get_center(sqp1, idx);
        ssq1x = sqp1.x()*sqp1.x() + ssq1x;
        ssq1y = sqp1.y()*sqp1.y() + ssq1y;
        ssq1z = sqp1.z()*sqp1.z() + ssq1z;

        icors2->get_center(sqp2, idx);
        ssq2x = sqp2.x()*sqp2.x() + ssq2x;
        ssq2y = sqp2.y()*sqp2.y() + ssq2y;
        ssq2z = sqp2.z()*sqp2.z() + ssq2z;
    }

    norm1=sqrt(ssq1x+ssq1y+ssq1z);
    norm2=sqrt(ssq2x+ssq2y+ssq2z);

    for (VMesh::Node::index_type idx = 0; idx < num_nodes; idx++)
    {
        icors1->get_center(sqp1, idx);
        normp1.x(sqp1.x()/norm1);
        normp1.y(sqp1.y()/norm1);
        normp1.z(sqp1.z()/norm1);
        icors1->set_point(normp1, idx);

        icors2->get_center(sqp2, idx);
        normp2.x(sqp2.x()/norm2);
        normp2.y(sqp2.y()/norm2);
        normp2.z(sqp2.z()/norm2);
        icors2->set_point(normp2, idx);
    }

    VMesh::size_type num_nodes_mesh = imesh->num_nodes();
    for (VMesh::Node::index_type idx = 0; idx < num_nodes_mesh; idx++)
    {
        imesh->get_center(mp, idx);
        normp2.x((mp.x() - sumx2)/norm2);
        normp2.y((mp.y() - sumy2)/norm2);
        normp2.z((mp.z() - sumz2)/norm2);
        imesh->set_point(normp2, idx);
    }

    DenseMatrix Amat(3,3);
    Point p1,p2;
    VMesh::Node::index_type idx;
    DenseMatrix P1(num_cors1,3),P2(num_cors1,3);

    //This is to fine the matrix A where A=P1'*P2
    // P1 and P2 are the normalized, centered point sets of size nx3.
    for (int L1 = 0; L1 < num_cors1; L1++)
    {
        idx=L1;
        icors1->get_point(p1, idx);
        icors2->get_point(p2, idx);

        P1(L1,0)=p1.x();
        P1(L1,1)=p1.y();
        P1(L1,2)=p1.z();
        P2(L1,0)=p2.x();
        P2(L1,1)=p2.y();
        P2(L1,2)=p2.z();
    }

    Amat=P1.transpose()*P2;

    /*
    std::cout<<"P1 " << P1 <<std::endl;
    std::cout<<"P2 " << P2 <<std::endl;
    std::cout<<"Amat " << Amat <<std::endl;
    */
    Eigen::JacobiSVD<DenseMatrix::EigenBase> svd_mat(Amat, Eigen::ComputeFullU | Eigen::ComputeFullV);

    //create the U and V matrix
    DenseMatrix UMat = svd_mat.matrixU();
    DenseMatrix VMat = svd_mat.matrixV();
    DenseMatrix SVMat(svd_mat.singularValues());

    printMatrix(UMat, "UMat");
    printMatrix(VMat, "VMat");
    printMatrix(SVMat, "matS");

    //rotation matrix
    DenseMatrix Tmat(3,3);
    Tmat=UMat*VMat.transpose();

    // check for negative determinant and fix.
    // this prevents reflection in the registration
    if (Tmat.determinant()<0)
    {
        VMat(0,2)=-1*VMat(0,2);
        VMat(1,2)=-1*VMat(1,2);
        VMat(2,2)=-1*VMat(2,2);
        Tmat=UMat*VMat.transpose();
    }

    double traceA;
    traceA=SVMat(0,0)+SVMat(1,0)+SVMat(2,0);

    std::vector<double> coefs = {
      Tmat(0, 0)*traceA*norm1,
      Tmat(0, 1)*traceA*norm1,
      Tmat(0, 2)*traceA*norm1,
      0.0,
      Tmat(1, 0)*traceA*norm1,
      Tmat(1, 1)*traceA*norm1,
      Tmat(1, 2)*traceA*norm1,
      0.0,
      Tmat(2, 0)*traceA*norm1,
      Tmat(2, 1)*traceA*norm1,
      Tmat(2, 2)*traceA*norm1,
      0.0 };

  DenseMatrix rotation(Eigen::MatrixXd::Identity(4,4));
  DenseMatrix trans1(Eigen::MatrixXd::Identity(4,4));
  DenseMatrix trans2(Eigen::MatrixXd::Identity(4,4));
  DenseMatrix scale1(Eigen::MatrixXd::Identity(4,4));
  DenseMatrix scale2(Eigen::MatrixXd::Identity(4,4));
  double *r_data = rotation.data();

  std::copy(coefs.begin(), coefs.end(), r_data);

  // recreate previous moves
  trans1(0,3) = -sumx2;
  trans1(1,3) = -sumy2;
  trans1(2,3) = -sumz2;

  trans2(0,3) = sumx;
  trans2(1,3) = sumy;
  trans2(2,3) = sumz;

  scale1(0,0) = 1/norm2;
  scale1(1,1) = 1/norm2;
  scale1(2,2) = 1/norm2;

  scale2(0,0) = norm1;
  scale2(1,1) = norm1;
  scale2(2,2) = norm1;


  printMatrix(trans1,"trans1");
  printMatrix(trans2,"trans2");
  printMatrix(scale1,"scale1");
  printMatrix(scale2,"scale2");


  auto trans_out = boost::make_shared<DenseMatrix>(trans2*rotation*scale1*trans1);

      //done with solve, make the new field
    printMatrix(*trans_out,"transform");


    make_new_pointsA(imesh, icors2, coefs, *omesh, sumx, sumy, sumz);

#if 0
    {
        const BBox bbox = omesh->get_bounding_box();
        if (bbox.valid())
        {
            std::cout << "output size: " << bbox.diagonal() << " center: " << bbox.center() << std::endl;
        }
        else
        {
            std::cout << "output bbox invalid" << std::endl;
        }
    }
#endif
    return trans_out;
}


DenseMatrixHandle RegisterWithCorrespondencesAlgo::runNone(FieldHandle input, FieldHandle Cors1, FieldHandle Cors2, FieldHandle& output) const
{
  if (!input)
  {
    error("No input field");
    return nullptr;
  }
  if (!Cors1)
  {
    error("No Correspondence1 input field");
    return nullptr;
  }
  if (!Cors2)
  {
    error("No Correspndence2 input field");
    return nullptr;
  }

  FieldHandle input_cp;

  output.reset(input->deep_clone());

  if (!output)
  {
    error("Could not allocate output field");
    return nullptr;
  }

  VMesh* imesh = input->vmesh();
  VMesh* omesh = output->vmesh();

  omesh = imesh;

  return boost::make_shared<DenseMatrix>(Eigen::MatrixXd::Identity(4,4));

}

bool RegisterWithCorrespondencesAlgo::radial_basis_func(VMesh* Cors, VMesh* points, DenseMatrixHandle& Sigma) const
{
  VMesh::Node::size_type num_cors, num_pts;
  VMesh::Node::iterator iti, itj;
  Point Pc, Pp;

  Cors->size(num_cors);
  points->size(num_pts);

  double xcomp = 0.0, ycomp = 0.0, zcomp = 0.0, mag = 0.0;

  Sigma.reset(new DenseMatrix(num_pts, num_cors));

  for (int i = 0; i < num_pts; ++i)
  {
    for (int j = 0; j < num_cors; ++j)
    {
      iti = i;
      itj = j;

      Cors->get_point(Pc, *(itj));
      points->get_point(Pp, *(iti));

      xcomp = Pc.x() - Pp.x();
      ycomp = Pc.y() - Pp.y();
      zcomp = Pc.z() - Pp.z();

      mag = sqrt(pow(xcomp, 2.0) + pow(ycomp, 2.0) + pow(zcomp, 2.0));
      if (mag == 0)
      {
        (*Sigma)(i, j) = 0;
      }
      else
      {
        double temp = pow(mag, 2.0)*log(mag);
        (*Sigma)(i, j) = temp;
      }
    }
  }
  return true;
}

bool RegisterWithCorrespondencesAlgo::make_new_points(VMesh* points, VMesh* Cors, const std::vector<double>& coefs, VMesh& omesh, double sumx, double sumy, double sumz) const
{
  VMesh::Node::size_type num_cors, num_pts;
  VMesh::Node::iterator it, itp;
  Point P, Pp;

  Cors->size(num_cors);
  points->size(num_pts);

  int sz = 0;
  double sumerx = 0, sumery = 0, sumerz = 0;
  double sigma = 0.0;
  DenseMatrixHandle SMat;
  radial_basis_func(Cors, points, SMat);

  for (int i = 0; i < num_pts; ++i)
  {
    sumerx = 0; sumery = 0; sumerz = 0;
    for (int j = 0; j < num_cors; ++j)
    {
      sigma = SMat->get(i, j);
      sumerx += coefs[j] * sigma;
      sumery += coefs[j + 4 + num_cors] * sigma;
      sumerz += coefs[j + 8 + 2 * num_cors] * sigma;
    }

    itp = i;
    points->get_point(Pp, *(itp));
    sz = (int)num_cors;


    P.x(sumx + sumerx + (Pp.x()) * (coefs[sz]) + (Pp.y()) * (coefs[sz + 1]) + (Pp.z()) * (coefs[sz + 2]) + coefs[sz + 3]);
    P.y(sumy + sumery + (Pp.x()) * coefs[2 * sz + 4] + (Pp.y())*coefs[2 * sz + 5] + (Pp.z())*coefs[2 * sz + 6] + coefs[2 * sz + 7]);
    P.z(sumz + sumerz + (Pp.x()) * coefs[3 * sz + 8] + (Pp.y())*coefs[3 * sz + 9] + (Pp.z())*coefs[3 * sz + 10] + coefs[3 * sz + 11]);

    omesh.set_point(P, *(itp));
  }
  return true;
}

bool RegisterWithCorrespondencesAlgo::make_new_pointsA(VMesh* points, VMesh* Cors, const std::vector<double>& coefs, VMesh& omesh, double sumx, double sumy, double sumz) const
{
  VMesh::Node::size_type num_cors, num_pts;
  VMesh::Node::iterator it, itp;
  Point P, Pp;

  points->size(num_pts);

#if 0
  std::cout << "make new points A inputs" <<
    "\ncoefs: " << coefs.size()
    << "\nsumx: " << sumx
    << "\nsumy: " << sumy
    << "\nsumz: " << sumz << "\n" << std::endl;
  std::copy(coefs.begin(), coefs.end(), std::ostream_iterator<double>(std::cout, "\n"));
#endif

  for (int i = 0; i < num_pts; ++i)
  {
    itp = i;
    points->get_point(Pp, *(itp));

    P.x(sumx + (Pp.x()) * (coefs[0]) + (Pp.y()) * (coefs[1]) + (Pp.z()) * (coefs[2]) + coefs[3]);
    P.y(sumy + (Pp.x()) * coefs[4] + (Pp.y())*coefs[5] + (Pp.z())*coefs[6] + coefs[7]);
    P.z(sumz + (Pp.x()) * coefs[8] + (Pp.y())*coefs[9] + (Pp.z())*coefs[10] + coefs[11]);
    omesh.set_point(P, *(itp));
  }
  return true;
}


const AlgorithmInputName RegisterWithCorrespondencesAlgo::Correspondences1("Correspondences1");
const AlgorithmInputName RegisterWithCorrespondencesAlgo::Correspondences2("Correspondences2");
const AlgorithmOutputName RegisterWithCorrespondencesAlgo::TransformMatrix("TransformMatrix");
