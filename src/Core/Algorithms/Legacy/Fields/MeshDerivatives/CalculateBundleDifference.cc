/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2022 Scientific Computing and Imaging Institute,
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



#include <Core/Algorithms/Legacy/Fields/MeshDerivatives/CalculateBundleDifference.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Datatypes/Legacy/Bundle/Bundle.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>

using namespace SCIRun;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Utility;
using namespace SCIRun::Core::Algorithms;


CalculateBundleDifferenceAlgo::CalculateBundleDifferenceAlgo()
{
  addOption(Variables::Method, "distance_between_nodes", "distance_between_nodes|area_between_fibers");
}

bool CalculateBundleDifferenceAlgo::runImpl(BundleHandle handle1, BundleHandle handle2, BundleHandle seedHandle, BundleHandle& outHandle1, BundleHandle& outHandle2) const
{
  ScopedAlgorithmStatusReporter asr(this, "CalculateBundleDifference");

  if (!handle1)
  {
    error("The first bundle is missing");
    return (false);
  }

  if (!handle2)
  {
    error("The second bundle is missing");
    return (false);
  }

  if (!seedHandle)
  {
    error("The seed region is missing");
    return (false);
  }

  outHandle1 = handle1;
  outHandle2 = handle2;

  int numFields1 = handle1->numFields();
  int numFields2 = handle2->numFields();

  //! deal with the empty field, if this is the case just return, do nothing
  //if ( (numFields1==0) || (numFields1==0) ) return (true);

  std::string fieldname1, fieldname2, out_fieldname1, out_fieldname2;
  VMesh::Node::size_type nnodes_seed;
  Point point1, point2, pointc, pointd;
  Point point1_, point2_;

  FieldHandle fhandle1, fhandle2, fhandle_seed, out_fhandle1, out_fhandle2;

  VMesh::Node::index_type i1, i2, ic, id, si;
  VMesh* smesh;
  VMesh* imesh1;
  VMesh* imesh2;

  fhandle_seed = seedHandle->getField(seedHandle->getFieldNames()[0]);
  smesh = fhandle_seed->vmesh();
  nnodes_seed = smesh->num_nodes();

  VMesh::Node::size_type nnodes1, nnodes2;

  std::vector<Point> points(static_cast<int>(nnodes_seed));

  //!loop into the seed region, the element in points[] is double
  for (int p = 0; p < nnodes_seed; p++) {
    si = p;
    smesh->get_center(points[si], si);
    //cout << "the seed point is " << (points[p](0) / 1) << " " << (points[p](1) / 1) << " " << (points[p](2) / 1) << endl;
  }

  //Change the points to coordinate
  //!loop into each fiber in bundle 1
  for (int p = 0; p < numFields1; p++)
  {
    fhandle1 = handle1->getField(handle1->getFieldNames()[p]);
    imesh1 = fhandle1->vmesh();
    nnodes1 = imesh1->num_nodes();
  }
  //!loop into each fiber in bundle 2
  for (int p = 0; p < numFields2; p++)
  {
    fhandle2 = handle2->getField(handle2->getFieldNames()[p]);
    imesh2 = fhandle2->vmesh();
    nnodes2 = imesh2->num_nodes();
  }

  //!search the corresponding fiber for the same seed point
  //!the condition for the same point is not that the difference of the x y z coordinate is 0
  //! but the difference for x,y,z coordinate is 1.0E-4, this maybe accurate enough

  std::vector<std::vector<int>> boundleseed1(static_cast<int>(nnodes_seed), std::vector<int>(2));
  auto boundleseed2 = boundleseed1;
  double sx, sy, sz;

  for (int p = 0; p < nnodes_seed; p++)
  {
    sx = points[p](0);
    sy = points[p](1);
    sz = points[p](2);
    boundleseed1[p][0] = -1;
    boundleseed1[p][1] = -1;
    boundleseed2[p][0] = -1;
    boundleseed2[p][1] = -1;
    for (int p2 = 0; p2 < numFields1; p2++)
    {
      fhandle1 = handle1->getField(handle1->getFieldNames()[p2]);
      imesh1 = fhandle1->vmesh();
      nnodes1 = imesh1->num_nodes();
      for (int p3 = 0; p3 < nnodes1; p3++)
      {
        i1 = p3;
        imesh1->get_center(point1, i1);
        if (((fabs(point1(0) - sx)) < 1.0e-4) && ((fabs(point1(1) - sy)) < 1.0e-4) && ((fabs(point1(2) - sz)) < 1.0e-4))
        {
          boundleseed1[p][0] = p2;
          boundleseed1[p][1] = p3;
        }
      }
    }

    for (int p2 = 0; p2 < numFields2; p2++)
    {
      fhandle2 = handle2->getField(handle2->getFieldNames()[p2]);
      imesh2 = fhandle2->vmesh();
      nnodes2 = imesh2->num_nodes();
      for (int p3 = 0; p3 < nnodes2; p3++)
      {
        i2 = p3;
        imesh2->get_center(point2, i2);
        if (((fabs(point2(0) - sx)) < 1.0e-4) && ((fabs(point2(1) - sy)) < 1.0e-4) && ((fabs(point2(2) - sz)) < 1.0e-4))
        {
          boundleseed2[p][0] = p2;
          boundleseed2[p][1] = p3;
        }
      }
    }
  }

  //!this part will check the order of array, whether the array for the fibers oriented in the same or
  //! opposite direction. If not, inverse the order of the array

  std::vector<int> dir_label(static_cast<int>(nnodes_seed));

  for (int p = 0; p < nnodes_seed; p++)
  {
    if ((boundleseed1[p][0] != -1) && (boundleseed2[p][0] != -1))
    {
      fhandle1 = handle1->getField(handle1->getFieldNames()[boundleseed1[p][0]]);
      imesh1 = fhandle1->vmesh();
      nnodes1 = imesh1->num_nodes();
      i1 = boundleseed1[p][1];
      imesh1->get_center(point1, i1);
      i1 = boundleseed1[p][1] - 1;
      imesh1->get_center(point1_, i1);

      fhandle2 = handle2->getField(handle2->getFieldNames()[boundleseed2[p][0]]);
      imesh2 = fhandle2->vmesh();
      nnodes2 = imesh2->num_nodes();
      i2 = boundleseed2[p][1];
      imesh2->get_center(point2, i2);
      i2 = boundleseed2[p][1] - 1;
      imesh2->get_center(point2_, i2);

      if (((point1_(0) - point1(0)) * (point2_(0) - point2(0)) + (point1_(1) - point1(1)) * (point2_(1) - point2(1)) + (point1_(2) - point1(2)) * (point2_(2) - point2(2))) >= 0)
      {
        dir_label[p] = 1;
      }
      else
      {
        dir_label[p] = -1;
      }
    }
    else {
      dir_label[p] = 0;
    }
  }

  int fi, bi;
  double dist, max_dist = 0.0;

  float dist_ab, dist_cd, dist_ac, dist_db, dist_ad, s1, s2, area1, area2, area;
  float max_area = 0.0;

  VField* outField1;
  VField* outField2;

  if (checkOption(Variables::Method, "distance_between_nodes"))
  {
    for (int p = 0; p < nnodes_seed; p++)
    {
      if ((boundleseed1[p][0] != -1) && (boundleseed2[p][0] != -1))
      {
        out_fieldname1 = outHandle1->getFieldNames()[boundleseed1[p][0]];
        out_fhandle1 = outHandle1->getField(out_fieldname1);
        outField1 = out_fhandle1->vfield();

        out_fieldname2 = outHandle2->getFieldNames()[boundleseed2[p][0]];
        out_fhandle2 = outHandle2->getField(out_fieldname2);
        outField2 = out_fhandle2->vfield();

        fieldname1 = handle1->getFieldNames()[boundleseed1[p][0]];
        fhandle1 = handle1->getField(fieldname1);
        imesh1 = fhandle1->vmesh();
        nnodes1 = imesh1->num_nodes();

        fieldname2 = handle2->getFieldNames()[boundleseed2[p][0]];
        fhandle2 = handle2->getField(fieldname2);
        imesh2 = fhandle2->vmesh();
        nnodes2 = imesh2->num_nodes();

        if (dir_label[p] == 1)
        {
          fi = std::min(boundleseed1[p][1], boundleseed2[p][1]);
          bi = std::min((nnodes1 - boundleseed1[p][1] - 1), (nnodes2 - boundleseed2[p][1] - 1));
        }
        else if (dir_label[p] == -1)
        {
          fi = std::min(boundleseed1[p][1], (static_cast<int>(nnodes2) - boundleseed2[p][1] - 1));
          bi = std::min((static_cast<int>(nnodes1) - boundleseed1[p][1] - 1), boundleseed2[p][1]);
        }
        else
        {
        }
        //this part calculate the common area of the two fiber bundles
        if (dir_label[p] == 1)
        {
          for (int q = 0; q < fi; q++)
          {
            i1 = boundleseed1[p][1] - q - 1;
            imesh1->get_center(point1, i1);
            i2 = boundleseed2[p][1] - q - 1;
            imesh2->get_center(point2, i2);
            dist = sqrt((point2[0] - point1[0]) * (point2[0] - point1[0]) + (point2[1] - point1[1]) * (point2[1] - point1[1]) + (point2[2] - point1[2]) * (point2[2] - point1[2]));
            if (dist > max_dist) max_dist = dist;
          }

          for (int q = 0; q < bi; q++)
          {
            i1 = boundleseed1[p][1] + q + 1;
            imesh1->get_center(point1, i1);
            i2 = boundleseed2[p][1] + q + 1;
            imesh2->get_center(point2, i2);
            dist = sqrt((point2[0] - point1[0]) * (point2[0] - point1[0]) + (point2[1] - point1[1]) * (point2[1] - point1[1]) + (point2[2] - point1[2]) * (point2[2] - point1[2]));
            if (dist > max_dist) max_dist = dist;
          }
        }
        else
        {
          for (int q = 0; q < fi; q++)
          {
            i1 = boundleseed1[p][1] - q - 1;
            imesh1->get_center(point1, i1);
            i2 = boundleseed2[p][1] + q + 1;
            imesh2->get_center(point2, i2);
            dist = sqrt((point2[0] - point1[0]) * (point2[0] - point1[0]) + (point2[1] - point1[1]) * (point2[1] - point1[1]) + (point2[2] - point1[2]) * (point2[2] - point1[2]));
            if (dist > max_dist) max_dist = dist;
          }

          for (int q = 0; q < bi; q++)
          {
            i1 = boundleseed1[p][1] + q + 1;
            imesh1->get_center(point1, i1);
            i2 = boundleseed2[p][1] - q - 1;
            imesh2->get_center(point2, i2);
            dist = sqrt((point2[0] - point1[0]) * (point2[0] - point1[0]) + (point2[1] - point1[1]) * (point2[1] - point1[1]) + (point2[2] - point1[2]) * (point2[2] - point1[2]));
            if (dist > max_dist) max_dist = dist;
          }
        }

        //this part set up the value for all of the node in bundle 1
        for (int q = 0; q < nnodes1; q++)
        {
          i1 = q;
          outField1->set_value(max_dist * (1.2), i1);
        }
        //this part set up the value for all of the node in bundle 2
        for (int q = 0; q < nnodes2; q++)
        {
          i2 = q;
          outField2->set_value(max_dist * (1.2), i2);
        }

        // set up the value for the seed region
        i1 = boundleseed1[p][1];
        i2 = boundleseed2[p][1];
        outField1->set_value(0, i1);
        outField2->set_value(0, i2);
        //this part calculate the common area of the two fiber bundles
        if (dir_label[p] == 1)
        {
          for (int q = 0; q < fi; q++)
          {
            i1 = boundleseed1[p][1] - q - 1;
            imesh1->get_center(point1, i1);
            i2 = boundleseed2[p][1] - q - 1;
            imesh2->get_center(point2, i2);
            dist = sqrt((point2[0] - point1[0]) * (point2[0] - point1[0]) + (point2[1] - point1[1]) * (point2[1] - point1[1]) + (point2[2] - point1[2]) * (point2[2] - point1[2]));
            if (dist > max_dist)
              max_dist = dist;
            outField1->set_value(dist, i1);
            outField2->set_value(dist, i2);
          }
          for (int q = 0; q < bi; q++)
          {
            i1 = boundleseed1[p][1] + q + 1;
            imesh1->get_center(point1, i1);
            i2 = boundleseed2[p][1] + q + 1;
            imesh2->get_center(point2, i2);
            dist = sqrt((point2[0] - point1[0]) * (point2[0] - point1[0]) + (point2[1] - point1[1]) * (point2[1] - point1[1]) + (point2[2] - point1[2]) * (point2[2] - point1[2]));
            if (dist > max_dist)
              max_dist = dist;

            outField1->set_value(dist, i1);
            outField2->set_value(dist, i2);
          }
        }
        else
        {
          for (int q = 0; q < fi; q++)
          {
            i1 = boundleseed1[p][1] - q - 1;
            imesh1->get_center(point1, i1);
            i2 = boundleseed2[p][1] + q + 1;
            imesh2->get_center(point2, i2);
            dist = sqrt((point2[0] - point1[0]) * (point2[0] - point1[0]) + (point2[1] - point1[1]) * (point2[1] - point1[1]) + (point2[2] - point1[2]) * (point2[2] - point1[2]));
            if (dist > max_dist)
              max_dist = dist;

            outField1->set_value(dist, i1);
            outField2->set_value(dist, i2);
          }
          for (int q = 0; q < bi; q++)
          {
            i1 = boundleseed1[p][1] + q + 1;
            imesh1->get_center(point1, i1);
            i2 = boundleseed2[p][1] - q - 1;
            imesh2->get_center(point2, i2);
            dist = sqrt((point2[0] - point1[0]) * (point2[0] - point1[0]) + (point2[1] - point1[1]) * (point2[1] - point1[1]) + (point2[2] - point1[2]) * (point2[2] - point1[2]));
            if (dist > max_dist)
              max_dist = dist;

            outField1->set_value(dist, i1);
            outField2->set_value(dist, i2);
          }
        }
      } //end for the boundleseed1 checking
    }

    for (int p = 0; p < nnodes_seed; p++)
    {
      if ((boundleseed1[p][0] != -1) && (boundleseed2[p][0] == -1))
      {
        out_fieldname1 = outHandle1->getFieldNames()[boundleseed1[p][0]];
        out_fhandle1 = outHandle1->getField(out_fieldname1);
        outField1 = out_fhandle1->vfield();

        fieldname1 = handle1->getFieldNames()[boundleseed1[p][0]];
        fhandle1 = handle1->getField(fieldname1);
        imesh1 = fhandle1->vmesh();
        nnodes1 = imesh1->num_nodes();

        for (int q = 0; q < nnodes1; q++)
        {
          i1 = q;
          outField1->set_value(max_dist * (1.2), i1);
        }
      }
      else if ((boundleseed1[p][0] == -1) && (boundleseed2[p][0] != -1))
      {
        out_fieldname2 = outHandle2->getFieldNames()[boundleseed2[p][0]];
        out_fhandle2 = outHandle2->getField(out_fieldname2);
        outField2 = out_fhandle2->vfield();

        fieldname2 = handle2->getFieldNames()[boundleseed2[p][0]];
        fhandle2 = handle2->getField(fieldname2);
        imesh2 = fhandle2->vmesh();
        nnodes2 = imesh2->num_nodes();

        for (int q = 0; q < nnodes2; q++)
        {
          i2 = q;
          outField2->set_value(max_dist * (1.2), i2);
        }
      }
      else
      {
        warning("Empty field at node index " + std::to_string(p));
      }
    }
  }
  else if (checkOption(Variables::Method, "area_between_fibers"))
  {
    for (int p = 0; p < nnodes_seed; p++)
    {
      if ((boundleseed1[p][0] != -1) && (boundleseed2[p][0] != -1))
      {
        out_fieldname1 = outHandle1->getFieldNames()[boundleseed1[p][0]];
        out_fhandle1 = outHandle1->getField(out_fieldname1);
        outField1 = out_fhandle1->vfield();

        out_fieldname2 = outHandle2->getFieldNames()[boundleseed2[p][0]];
        out_fhandle2 = outHandle2->getField(out_fieldname2);
        outField2 = out_fhandle2->vfield();

        fieldname1 = handle1->getFieldNames()[boundleseed1[p][0]];
        fhandle1 = handle1->getField(fieldname1);
        imesh1 = fhandle1->vmesh();
        nnodes1 = imesh1->num_nodes();

        fieldname2 = handle2->getFieldNames()[boundleseed2[p][0]];
        fhandle2 = handle2->getField(fieldname2);
        imesh2 = fhandle2->vmesh();
        nnodes2 = imesh2->num_nodes();

        if (dir_label[p] == 1)
        {
          fi = std::min(boundleseed1[p][1], boundleseed2[p][1]);
          bi = std::min((nnodes1 - boundleseed1[p][1] - 1), (nnodes2 - boundleseed2[p][1] - 1));
        }
        else {
          fi = std::min(boundleseed1[p][1], (static_cast<int>(nnodes2) - boundleseed2[p][1] - 1));
          bi = std::min((static_cast<int>(nnodes1) - boundleseed1[p][1] - 1), boundleseed2[p][1]);
        }
        //this part calculate the common area of the two fiber bundles

        if (dir_label[p] == 1)
        {
          for (int q = 0; q < (fi - 1); q++)
          {
            i1 = boundleseed1[p][1] - q - 1;
            imesh1->get_center(point1, i1);
            i2 = boundleseed2[p][1] - q - 1;
            imesh2->get_center(point2, i2);

            ic = boundleseed1[p][1] - q;
            imesh1->get_center(pointc, ic);
            id = boundleseed2[p][1] - q;
            imesh2->get_center(pointd, id);

            dist_ab = sqrt((point2[0] - point1[0]) * (point2[0] - point1[0]) + (point2[1] - point1[1]) * (point2[1] - point1[1]) + (point2[2] - point1[2]) * (point2[2] - point1[2]));
            dist_cd = sqrt((pointc[0] - pointd[0]) * (pointc[0] - pointd[0]) + (pointc[1] - pointd[1]) * (pointc[1] - pointd[1]) + (pointc[2] - pointd[2]) * (pointc[2] - pointd[2]));
            dist_ac = sqrt((point2[0] - pointc[0]) * (point2[0] - pointc[0]) + (point2[1] - pointc[1]) * (point2[1] - pointc[1]) + (point2[2] - pointc[2]) * (point2[2] - pointc[2]));
            dist_db = sqrt((pointd[0] - point1[0]) * (pointd[0] - point1[0]) + (pointd[1] - point1[1]) * (pointd[1] - point1[1]) + (pointd[2] - point1[2]) * (pointd[2] - point1[2]));
            dist_ad = sqrt((point2[0] - pointd[0]) * (point2[0] - pointd[0]) + (point2[1] - pointd[1]) * (point2[1] - pointd[1]) + (point2[2] - pointd[2]) * (point2[2] - pointd[2]));

            s1 = (dist_ab + dist_ad + dist_db) / 2.0;
            area1 = sqrt(s1 * (s1 - dist_ab) * (s1 - dist_ad) * (s1 - dist_db));
            s2 = (dist_ac + dist_ad + dist_cd) / 2.0;
            area2 = sqrt(s2 * (s2 - dist_ac) * (s2 - dist_ad) * (s2 - dist_cd));

            area = area1 + area2;
            if (area > max_area) max_area = area;
          }
          for (int q = 0; q < (bi - 1); q++)
          {
            i1 = boundleseed1[p][1] + q + 1;
            imesh1->get_center(point1, i1);
            i2 = boundleseed2[p][1] + q + 1;
            imesh2->get_center(point2, i2);

            ic = boundleseed1[p][1] + q + 2;
            imesh1->get_center(pointc, ic);
            id = boundleseed2[p][1] + q + 2;
            imesh2->get_center(pointd, id);

            dist_ab = sqrt((point2[0] - point1[0]) * (point2[0] - point1[0]) + (point2[1] - point1[1]) * (point2[1] - point1[1]) + (point2[2] - point1[2]) * (point2[2] - point1[2]));
            dist_cd = sqrt((pointc[0] - pointd[0]) * (pointc[0] - pointd[0]) + (pointc[1] - pointd[1]) * (pointc[1] - pointd[1]) + (pointc[2] - pointd[2]) * (pointc[2] - pointd[2]));
            dist_ac = sqrt((point2[0] - pointc[0]) * (point2[0] - pointc[0]) + (point2[1] - pointc[1]) * (point2[1] - pointc[1]) + (point2[2] - pointc[2]) * (point2[2] - pointc[2]));
            dist_db = sqrt((pointd[0] - point1[0]) * (pointd[0] - point1[0]) + (pointd[1] - point1[1]) * (pointd[1] - point1[1]) + (pointd[2] - point1[2]) * (pointd[2] - point1[2]));
            dist_ad = sqrt((point2[0] - pointd[0]) * (point2[0] - pointd[0]) + (point2[1] - pointd[1]) * (point2[1] - pointd[1]) + (point2[2] - pointd[2]) * (point2[2] - pointd[2]));

            s1 = (dist_ab + dist_ad + dist_db) / 2.0;
            area1 = sqrt(s1 * (s1 - dist_ab) * (s1 - dist_ad) * (s1 - dist_db));
            s2 = (dist_ac + dist_ad + dist_cd) / 2.0;
            area2 = sqrt(s2 * (s2 - dist_ac) * (s2 - dist_ad) * (s2 - dist_cd));

            area = area1 + area2;
            if (area > max_area) max_area = area;
          }
        }
        else
        {
          for (int q = 0; q < (fi - 1); q++)
          {
            i1 = boundleseed1[p][1] - q - 1;
            imesh1->get_center(point1, i1);
            i2 = boundleseed2[p][1] + q + 1;
            imesh2->get_center(point2, i2);

            ic = boundleseed1[p][1] - q - 2;
            imesh1->get_center(pointc, ic);
            id = boundleseed2[p][1] + q + 2;
            imesh2->get_center(pointd, id);

            dist_ab = sqrt((point2[0] - point1[0]) * (point2[0] - point1[0]) + (point2[1] - point1[1]) * (point2[1] - point1[1]) + (point2[2] - point1[2]) * (point2[2] - point1[2]));
            dist_cd = sqrt((pointc[0] - pointd[0]) * (pointc[0] - pointd[0]) + (pointc[1] - pointd[1]) * (pointc[1] - pointd[1]) + (pointc[2] - pointd[2]) * (pointc[2] - pointd[2]));
            dist_ac = sqrt((point2[0] - pointc[0]) * (point2[0] - pointc[0]) + (point2[1] - pointc[1]) * (point2[1] - pointc[1]) + (point2[2] - pointc[2]) * (point2[2] - pointc[2]));
            dist_db = sqrt((pointd[0] - point1[0]) * (pointd[0] - point1[0]) + (pointd[1] - point1[1]) * (pointd[1] - point1[1]) + (pointd[2] - point1[2]) * (pointd[2] - point1[2]));
            dist_ad = sqrt((point2[0] - pointd[0]) * (point2[0] - pointd[0]) + (point2[1] - pointd[1]) * (point2[1] - pointd[1]) + (point2[2] - pointd[2]) * (point2[2] - pointd[2]));

            s1 = (dist_ab + dist_ad + dist_db) / 2.0;
            area1 = sqrt(s1 * (s1 - dist_ab) * (s1 - dist_ad) * (s1 - dist_db));
            s2 = (dist_ac + dist_ad + dist_cd) / 2.0;
            area2 = sqrt(s2 * (s2 - dist_ac) * (s2 - dist_ad) * (s2 - dist_cd));

            area = area1 + area2;
            if (area > max_area) max_area = area;
          }
          for (int q = 0; q < (bi - 1); q++)
          {
            i1 = boundleseed1[p][1] + q + 1;
            imesh1->get_center(point1, i1);
            i2 = boundleseed2[p][1] - q - 1;
            imesh2->get_center(point2, i2);

            ic = boundleseed1[p][1] + q + 2;
            imesh1->get_center(pointc, ic);
            id = boundleseed2[p][1] - q - 2;
            imesh2->get_center(pointd, id);

            dist_ab = sqrt((point2[0] - point1[0]) * (point2[0] - point1[0]) + (point2[1] - point1[1]) * (point2[1] - point1[1]) + (point2[2] - point1[2]) * (point2[2] - point1[2]));
            dist_cd = sqrt((pointc[0] - pointd[0]) * (pointc[0] - pointd[0]) + (pointc[1] - pointd[1]) * (pointc[1] - pointd[1]) + (pointc[2] - pointd[2]) * (pointc[2] - pointd[2]));
            dist_ac = sqrt((point2[0] - pointc[0]) * (point2[0] - pointc[0]) + (point2[1] - pointc[1]) * (point2[1] - pointc[1]) + (point2[2] - pointc[2]) * (point2[2] - pointc[2]));
            dist_db = sqrt((pointd[0] - point1[0]) * (pointd[0] - point1[0]) + (pointd[1] - point1[1]) * (pointd[1] - point1[1]) + (pointd[2] - point1[2]) * (pointd[2] - point1[2]));
            dist_ad = sqrt((point2[0] - pointd[0]) * (point2[0] - pointd[0]) + (point2[1] - pointd[1]) * (point2[1] - pointd[1]) + (point2[2] - pointd[2]) * (point2[2] - pointd[2]));

            s1 = (dist_ab + dist_ad + dist_db) / 2.0;
            area1 = sqrt(s1 * (s1 - dist_ab) * (s1 - dist_ad) * (s1 - dist_db));
            s2 = (dist_ac + dist_ad + dist_cd) / 2.0;
            area2 = sqrt(s2 * (s2 - dist_ac) * (s2 - dist_ad) * (s2 - dist_cd));

            area = area1 + area2;
            if (area > max_area) max_area = area;
          }
        }

        //this part set up the value for all of the node in bundle 1
        for (int q = 0; q < nnodes1; q++)
        {
          i1 = q;
          outField1->set_value(max_area * (1.0), i1);
        }
        //this part set up the value for all of the node in bundle 2
        for (int q = 0; q < nnodes2; q++)
        {
          i2 = q;
          outField2->set_value(max_area * (1.0), i2);
        }

        // set up the value for the seed region
        i1 = boundleseed1[p][1];
        i2 = boundleseed2[p][1];
        outField1->set_value(0, i1);
        outField2->set_value(0, i2);

        //this part calculate the common area of the two fiber bundles

        if (dir_label[p] == 1)
        {
          for (int q = 0; q < (fi - 1); q++)
          {
            i1 = boundleseed1[p][1] - q - 1;
            imesh1->get_center(point1, i1);
            i2 = boundleseed2[p][1] - q - 1;
            imesh2->get_center(point2, i2);

            ic = boundleseed1[p][1] - q;
            imesh1->get_center(pointc, ic);
            id = boundleseed2[p][1] - q;
            imesh2->get_center(pointd, id);

            dist_ab = sqrt((point2[0] - point1[0]) * (point2[0] - point1[0]) + (point2[1] - point1[1]) * (point2[1] - point1[1]) + (point2[2] - point1[2]) * (point2[2] - point1[2]));
            dist_cd = sqrt((pointc[0] - pointd[0]) * (pointc[0] - pointd[0]) + (pointc[1] - pointd[1]) * (pointc[1] - pointd[1]) + (pointc[2] - pointd[2]) * (pointc[2] - pointd[2]));
            dist_ac = sqrt((point2[0] - pointc[0]) * (point2[0] - pointc[0]) + (point2[1] - pointc[1]) * (point2[1] - pointc[1]) + (point2[2] - pointc[2]) * (point2[2] - pointc[2]));
            dist_db = sqrt((pointd[0] - point1[0]) * (pointd[0] - point1[0]) + (pointd[1] - point1[1]) * (pointd[1] - point1[1]) + (pointd[2] - point1[2]) * (pointd[2] - point1[2]));
            dist_ad = sqrt((point2[0] - pointd[0]) * (point2[0] - pointd[0]) + (point2[1] - pointd[1]) * (point2[1] - pointd[1]) + (point2[2] - pointd[2]) * (point2[2] - pointd[2]));

            s1 = (dist_ab + dist_ad + dist_db) / 2.0;
            area1 = sqrt(s1 * (s1 - dist_ab) * (s1 - dist_ad) * (s1 - dist_db));
            s2 = (dist_ac + dist_ad + dist_cd) / 2.0;
            area2 = sqrt(s2 * (s2 - dist_ac) * (s2 - dist_ad) * (s2 - dist_cd));

            area = area1 + area2;
            if (area > max_area) max_area = area;

            outField1->set_value(area, i1);
            outField2->set_value(area, i2);
          }

          for (int q = 0; q < (bi - 1); q++)
          {
            i1 = boundleseed1[p][1] + q + 1;
            imesh1->get_center(point1, i1);
            i2 = boundleseed2[p][1] + q + 1;
            imesh2->get_center(point2, i2);

            ic = boundleseed1[p][1] + q + 2;
            imesh1->get_center(pointc, ic);
            id = boundleseed2[p][1] + q + 2;
            imesh2->get_center(pointd, id);

            dist_ab = sqrt((point2[0] - point1[0]) * (point2[0] - point1[0]) + (point2[1] - point1[1]) * (point2[1] - point1[1]) + (point2[2] - point1[2]) * (point2[2] - point1[2]));
            dist_cd = sqrt((pointc[0] - pointd[0]) * (pointc[0] - pointd[0]) + (pointc[1] - pointd[1]) * (pointc[1] - pointd[1]) + (pointc[2] - pointd[2]) * (pointc[2] - pointd[2]));
            dist_ac = sqrt((point2[0] - pointc[0]) * (point2[0] - pointc[0]) + (point2[1] - pointc[1]) * (point2[1] - pointc[1]) + (point2[2] - pointc[2]) * (point2[2] - pointc[2]));
            dist_db = sqrt((pointd[0] - point1[0]) * (pointd[0] - point1[0]) + (pointd[1] - point1[1]) * (pointd[1] - point1[1]) + (pointd[2] - point1[2]) * (pointd[2] - point1[2]));
            dist_ad = sqrt((point2[0] - pointd[0]) * (point2[0] - pointd[0]) + (point2[1] - pointd[1]) * (point2[1] - pointd[1]) + (point2[2] - pointd[2]) * (point2[2] - pointd[2]));

            s1 = (dist_ab + dist_ad + dist_db) / 2.0;
            area1 = sqrt(s1 * (s1 - dist_ab) * (s1 - dist_ad) * (s1 - dist_db));
            s2 = (dist_ac + dist_ad + dist_cd) / 2.0;
            area2 = sqrt(s2 * (s2 - dist_ac) * (s2 - dist_ad) * (s2 - dist_cd));

            area = area1 + area2;
            if (area > max_area) max_area = area;

            outField1->set_value(area, i1);
            outField2->set_value(area, i2);
          }
        }
        else
        {
          for (int q = 0; q < (fi - 1); q++)
          {
            i1 = boundleseed1[p][1] - q - 1;
            imesh1->get_center(point1, i1);
            i2 = boundleseed2[p][1] + q + 1;
            imesh2->get_center(point2, i2);

            ic = boundleseed1[p][1] - q - 2;
            imesh1->get_center(pointc, ic);
            id = boundleseed2[p][1] + q + 2;
            imesh2->get_center(pointd, id);

            dist_ab = sqrt((point2[0] - point1[0]) * (point2[0] - point1[0]) + (point2[1] - point1[1]) * (point2[1] - point1[1]) + (point2[2] - point1[2]) * (point2[2] - point1[2]));
            dist_cd = sqrt((pointc[0] - pointd[0]) * (pointc[0] - pointd[0]) + (pointc[1] - pointd[1]) * (pointc[1] - pointd[1]) + (pointc[2] - pointd[2]) * (pointc[2] - pointd[2]));
            dist_ac = sqrt((point2[0] - pointc[0]) * (point2[0] - pointc[0]) + (point2[1] - pointc[1]) * (point2[1] - pointc[1]) + (point2[2] - pointc[2]) * (point2[2] - pointc[2]));
            dist_db = sqrt((pointd[0] - point1[0]) * (pointd[0] - point1[0]) + (pointd[1] - point1[1]) * (pointd[1] - point1[1]) + (pointd[2] - point1[2]) * (pointd[2] - point1[2]));
            dist_ad = sqrt((point2[0] - pointd[0]) * (point2[0] - pointd[0]) + (point2[1] - pointd[1]) * (point2[1] - pointd[1]) + (point2[2] - pointd[2]) * (point2[2] - pointd[2]));

            s1 = (dist_ab + dist_ad + dist_db) / 2.0;
            area1 = sqrt(s1 * (s1 - dist_ab) * (s1 - dist_ad) * (s1 - dist_db));
            s2 = (dist_ac + dist_ad + dist_cd) / 2.0;
            area2 = sqrt(s2 * (s2 - dist_ac) * (s2 - dist_ad) * (s2 - dist_cd));

            area = area1 + area2;
            if (area > max_area) max_area = area;

            outField1->set_value(area, i1);
            outField2->set_value(area, i2);
          }

          for (int q = 0; q < (bi - 1); q++)
          {
            i1 = boundleseed1[p][1] + q + 1;
            imesh1->get_center(point1, i1);
            i2 = boundleseed2[p][1] - q - 1;
            imesh2->get_center(point2, i2);

            ic = boundleseed1[p][1] + q + 2;
            imesh1->get_center(pointc, ic);
            id = boundleseed2[p][1] - q - 2;
            imesh2->get_center(pointd, id);

            dist_ab = sqrt((point2[0] - point1[0]) * (point2[0] - point1[0]) + (point2[1] - point1[1]) * (point2[1] - point1[1]) + (point2[2] - point1[2]) * (point2[2] - point1[2]));
            dist_cd = sqrt((pointc[0] - pointd[0]) * (pointc[0] - pointd[0]) + (pointc[1] - pointd[1]) * (pointc[1] - pointd[1]) + (pointc[2] - pointd[2]) * (pointc[2] - pointd[2]));
            dist_ac = sqrt((point2[0] - pointc[0]) * (point2[0] - pointc[0]) + (point2[1] - pointc[1]) * (point2[1] - pointc[1]) + (point2[2] - pointc[2]) * (point2[2] - pointc[2]));
            dist_db = sqrt((pointd[0] - point1[0]) * (pointd[0] - point1[0]) + (pointd[1] - point1[1]) * (pointd[1] - point1[1]) + (pointd[2] - point1[2]) * (pointd[2] - point1[2]));
            dist_ad = sqrt((point2[0] - pointd[0]) * (point2[0] - pointd[0]) + (point2[1] - pointd[1]) * (point2[1] - pointd[1]) + (point2[2] - pointd[2]) * (point2[2] - pointd[2]));

            s1 = (dist_ab + dist_ad + dist_db) / 2.0;
            area1 = sqrt(s1 * (s1 - dist_ab) * (s1 - dist_ad) * (s1 - dist_db));
            s2 = (dist_ac + dist_ad + dist_cd) / 2.0;
            area2 = sqrt(s2 * (s2 - dist_ac) * (s2 - dist_ad) * (s2 - dist_cd));

            area = area1 + area2;
            if (area > max_area) max_area = area;

            outField1->set_value(area, i1);
            outField2->set_value(area, i2);
          }
        }
      }
    }

    for (int p = 0; p < nnodes_seed; p++)
    {
      if ((boundleseed1[p][0] != -1) && (boundleseed2[p][0] == -1))
      {
        out_fhandle1 = outHandle1->getField(outHandle1->getFieldNames()[boundleseed1[p][0]]);
        outField1 = out_fhandle1->vfield();

        fhandle1 = handle1->getField(handle1->getFieldNames()[boundleseed1[p][0]]);
        imesh1 = fhandle1->vmesh();
        nnodes1 = imesh1->num_nodes();

        for (int q = 0; q < nnodes1; q++)
        {
          i1 = q;
          outField1->set_value(max_area * (1.2), i1);
        }
      }
      else if ((boundleseed1[p][0] == -1) && (boundleseed2[p][0] != -1))
      {
        out_fhandle2 = outHandle2->getField(outHandle2->getFieldNames()[boundleseed2[p][0]]);
        outField2 = out_fhandle2->vfield();

        fhandle2 = handle2->getField(handle2->getFieldNames()[boundleseed2[p][0]]);
        imesh2 = fhandle2->vmesh();
        nnodes2 = imesh2->num_nodes();

        for (int q = 0; q < nnodes2; q++)
        {
          i2 = q;
          outField2->set_value(max_area * (1.2), i2);
        }
      }
      else
      {
        warning("Empty field at node index " + std::to_string(p));
      }
    }
  }
  return (true);
}

AlgorithmOutput CalculateBundleDifferenceAlgo::run(const AlgorithmInput& input) const
{
  auto b1 = input.get<Bundle>(Name("bundle1"));
  auto b2 = input.get<Bundle>(Name("bundle2"));
  auto seed = input.get<Bundle>(Name("seed"));

  BundleHandle out1, out2;
  if (!runImpl(b1, b2, seed, out1, out2))
    THROW_ALGORITHM_PROCESSING_ERROR("False returned on legacy run call.");

  AlgorithmOutput output;
  output[Name("bundleOut1")] = out1;
  output[Name("bundleOut2")] = out2;
  return output;
}
