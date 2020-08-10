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


#include <Core/Algorithms/Legacy/Fields/ClipMesh/ClipMeshByIsovalue.h>
#include <Core/Algorithms/Legacy/Fields/MarchingCubes/MarchingCubes.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/SparseRowMatrixFromMap.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <boost/unordered_map.hpp>

#include <algorithm>
#include <set>


using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Fields;

int tet_permute_table[15][4] = {
  { 0, 0, 0, 0 }, // 0x0
  { 3, 0, 2, 1 }, // 0x1
  { 2, 3, 0, 1 }, // 0x2
  { 0, 1, 2, 3 }, // 0x3
  { 1, 2, 0, 3 }, // 0x4
  { 0, 2, 3, 1 }, // 0x5
  { 0, 3, 1, 2 }, // 0x6
  { 0, 1, 2, 3 }, // 0x7
  { 0, 1, 2, 3 }, // 0x8
  { 2, 1, 3, 0 }, // 0x9
  { 3, 1, 0, 2 }, // 0xa
  { 1, 2, 0, 3 }, // 0xb
  { 3, 2, 1, 0 }, // 0xc
  { 2, 3, 0, 1 }, // 0xd
  { 3, 0, 2, 1 }, // 0xe
};


int tri_permute_table[7][3] = {
  { 0, 0, 0 }, // 0x0
  { 2, 0, 1 }, // 0x1
  { 1, 2, 0 }, // 0x2
  { 0, 1, 2 }, // 0x3
  { 0, 1, 2 }, // 0x4
  { 1, 2, 0 }, // 0x5
  { 2, 0, 1 }, // 0x6
};

namespace detail
{
  struct edgepair_t
  {
    VField::index_type first;
    VField::index_type second;
    double dfirst;
  };

  struct edgepairhash
  {
    size_t operator()(const edgepair_t &a) const
    {
      std::hash<size_t> h;
      return h(a.first ^ a.second);
    }
  };

  typedef boost::unordered_map<VField::index_type, VMesh::Node::index_type> node_hash_type;
  typedef boost::unordered_map<edgepair_t, VMesh::Node::index_type, edgepairhash> edge_hash_type;

  bool operator==(const edgepair_t &a, const edgepair_t &b)
  {
    return a.first == b.first && a.second == b.second;
  }

  bool operator!=(const edgepair_t &a, const edgepair_t &b)
  {
    return !(a == b);
  }

  struct facetriple_t
  {
    VField::index_type first, second, third;
    double dsecond, dthird;
  };

  bool operator==(const facetriple_t &a, const facetriple_t &b)
  {
    return a.first == b.first && a.second == b.second && a.third == b.third;
  }

  bool operator!=(const facetriple_t &a, const facetriple_t &b)
  {
    return !(a == b);
  }

  struct facetriplehash
  {
    size_t operator()(const facetriple_t &a) const
    {
      std::hash<size_t> h;
      return h(a.first ^ a.second ^ a.third);
    }
  };

  typedef boost::unordered_map<facetriple_t, VMesh::Node::index_type, facetriplehash> face_hash_type;

}

ClipMeshByIsovalueAlgo::ClipMeshByIsovalueAlgo()
{
 addParameter(LessThanIsoValue, 1);
 addParameter(ScalarIsoValue, 0.0);
}

class ClipMeshByIsovalueAlgoTet {

  public:
    bool run(const AlgorithmBase* algo,FieldHandle input, FieldHandle& output, MatrixHandle& mapping) const;

  private:
    VMesh::Node::index_type
    edge_lookup(VField::index_type u0, VField::index_type u1, double d0,
          const Point &p, detail::edge_hash_type &edgemap,
          VMesh *clipped) const;

    VMesh::Node::index_type
    face_lookup(VField::index_type u0, VField::index_type u1, VField::index_type u2,
          double d1, double d2,
          const Point &p, detail::face_hash_type &facemap,
          VMesh *clipped) const;

 };

VMesh::Node::index_type ClipMeshByIsovalueAlgoTet::edge_lookup(VField::index_type u0, VField::index_type u1, double d0, const Point &p, detail::edge_hash_type &edgemap, VMesh *clipped) const
{
  using namespace detail;
  edgepair_t np;
  if (u0 < u1)  { np.first = u0; np.second = u1; np.dfirst = d0; }
  else { np.first = u1; np.second = u0; np.dfirst = 1.0 - d0; }

  auto loc = edgemap.find(np);

  if (loc == edgemap.end())
  {
    const VMesh::Node::index_type nodeindex = clipped->add_point(p);
    edgemap[np] = nodeindex;
    return nodeindex;
  }
  else
  {
   return loc->second;
  }
}

VMesh::Node::index_type ClipMeshByIsovalueAlgoTet::face_lookup(VField::index_type u0, VField::index_type u1, VField::index_type u2, double d1, double d2, const Point &p, detail::face_hash_type &facemap, VMesh *clipped) const
{
  using namespace detail;
  facetriple_t nt;
  if (u0 < u1)
  {
    if (u2 < u0)
    {
      nt.first = u2; nt.second = u0; nt.third = u1;
      nt.dsecond = 1.0 - d1 - d2; nt.dthird = d1;
    }
    else if (u2 < u1)
    {
      nt.first = u0; nt.second = u2; nt.third = u1;
      nt.dsecond = d2; nt.dthird = d1;
    }
    else
    {
      nt.first = u0; nt.second = u1; nt.third = u2;
      nt.dsecond = d1; nt.dthird = d2;
    }
  }
  else
  {
    if (u2 > u0)
    {
      nt.first = u1; nt.second = u0; nt.third = u2;
      nt.dsecond = 1.0 - d1 - d2; nt.dthird = d2;
    }
    else if (u2 > u1)
    {
      nt.first = u1; nt.second = u2; nt.third = u0;
      nt.dsecond = d2; nt.dthird = 1.0 - d1 - d2;
    }
    else
    {
      nt.first = u2; nt.second = u1; nt.third = u0;
      nt.dsecond = d1; nt.dthird = 1.0 - d1 - d2;
    }
  }
  const face_hash_type::iterator loc = facemap.find(nt);
  if (loc == facemap.end())
  {
    const VMesh::Node::index_type nodeindex = clipped->add_point(p);
    facemap[nt] = nodeindex;
    return nodeindex;
  }
  else
  {
    return (*loc).second;
  }
}

bool ClipMeshByIsovalueAlgoTet::run(const AlgorithmBase* algo, FieldHandle input, FieldHandle& output, MatrixHandle &mapping) const
{
  VField* field = input->vfield();
  VMesh*  mesh  = input->vmesh();
  VMesh*  clipped = output->vmesh();

  using namespace detail;
  node_hash_type nodemap;
  edge_hash_type edgemap;
  face_hash_type facemap;

  VMesh::Node::array_type onodes(4);
  std::vector<double> v(4);
  std::vector<Point> p(4);

  double isoval = algo->get(ClipMeshByIsovalueAlgo::ScalarIsoValue).toDouble();

  bool lte = !algo->get(ClipMeshByIsovalueAlgo::LessThanIsoValue).toBool();

  VMesh::size_type num_elems = mesh->num_elems();

  for (VMesh::Elem::index_type idx=0; idx<num_elems; idx++)
  {
    mesh->get_nodes(onodes, idx);

      // Get the values and compute an inside/outside mask.
    VField::index_type inside = 0;
    mesh->get_centers(p, onodes);
    field->get_values(v,onodes);
    for (size_t i = 0; i < onodes.size(); i++)
    {
      inside = inside << 1;
      if (v[i] > isoval)
      {
        inside |= 1;
      }

    }

      // Invert the mask if we are doing less than.
    if (lte) { inside = ~inside & 0xf; }

    if (inside == 0)
    {
        // Discard outside elements.
    }
    else if (inside == 0xf)
    {
        // Add this element to the new mesh.
      VMesh::Node::array_type nnodes(onodes.size());
      for (size_t i = 0; i<onodes.size(); i++)
      {
        if (nodemap.find((VField::index_type)onodes[i]) == nodemap.end())
        {
          const VMesh::Node::index_type nodeindex = clipped->add_point(p[i]);
          nodemap[(VField::index_type)onodes[i]] = nodeindex;
          nnodes[i] = nodeindex;
        }
        else
        {
          nnodes[i] = nodemap[(VField::index_type)onodes[i]];
        }
      }

      clipped->add_elem(nnodes);
    }
    else if (inside == 0x8 || inside == 0x4 || inside == 0x2 || inside == 0x1)
    {
        // Lop off 3 points and add resulting tet to the new mesh.
      const int *perm = tet_permute_table[inside];
      VMesh::Node::array_type nnodes(4);

      if (nodemap.find((VField::index_type)onodes[perm[0]]) == nodemap.end())
      {
        const VMesh::Node::index_type nodeindex = clipped->add_point(p[perm[0]]);
        nodemap[(VField::index_type)onodes[perm[0]]] = nodeindex;
        nnodes[0] = nodeindex;
      }
      else
      {
        nnodes[0] = nodemap[(VField::index_type)onodes[perm[0]]];
      }

      const double imv = isoval - v[perm[0]];
      const double dl1 = imv / (v[perm[1]] - v[perm[0]]);
      const Point l1 = Interpolate(p[perm[0]], p[perm[1]], dl1);
      const double dl2 = imv / (v[perm[2]] - v[perm[0]]);
      const Point l2 = Interpolate(p[perm[0]], p[perm[2]], dl2);
      const double dl3 = imv / (v[perm[3]] - v[perm[0]]);
      const Point l3 = Interpolate(p[perm[0]], p[perm[3]], dl3);


      nnodes[1] = edge_lookup((VField::index_type)onodes[perm[0]],
                              (VField::index_type)onodes[perm[1]],
                              dl1, l1, edgemap, clipped);

      nnodes[2] = edge_lookup((VField::index_type)onodes[perm[0]],
                              (VField::index_type)onodes[perm[2]],
                              dl2, l2, edgemap, clipped);

      nnodes[3] = edge_lookup((VField::index_type)onodes[perm[0]],
                              (VField::index_type)onodes[perm[3]],
                              dl3, l3, edgemap, clipped);

      clipped->add_elem(nnodes);
    }
    else if (inside == 0x7 || inside == 0xb || inside == 0xd || inside == 0xe)
    {
        // Lop off 1 point, break up the resulting quads and add the
        // resulting tets to the mesh.
      const int *perm = tet_permute_table[inside];
      VMesh::Node::array_type nnodes(4);

      VMesh::Node::index_type inodes[9];
      for (size_t i = 1; i < 4; i++)
      {
        if (nodemap.find((VField::index_type)onodes[perm[i]]) == nodemap.end())
        {
          const VMesh::Node::index_type nodeindex = clipped->add_point(p[perm[i]]);
          nodemap[(VField::index_type)onodes[perm[i]]] = nodeindex;
          inodes[i-1] = nodeindex;
        }
        else
        {
          inodes[i-1] = nodemap[(VField::index_type)onodes[perm[i]]];
        }
      }

      const double imv = isoval - v[perm[0]];
      const double dl1 = imv / (v[perm[1]] - v[perm[0]]);
      const Point l1 = Interpolate(p[perm[0]], p[perm[1]], dl1);
      const double dl2 = imv / (v[perm[2]] - v[perm[0]]);
      const Point l2 = Interpolate(p[perm[0]], p[perm[2]], dl2);
      const double dl3 = imv / (v[perm[3]] - v[perm[0]]);
      const Point l3 = Interpolate(p[perm[0]], p[perm[3]], dl3);


      inodes[3] = edge_lookup((index_type)onodes[perm[0]],
                              (index_type)onodes[perm[1]],
                              dl1, l1, edgemap, clipped);

      inodes[4] = edge_lookup((index_type)onodes[perm[0]],
                              (index_type)onodes[perm[2]],
                              dl2, l2, edgemap, clipped);

      inodes[5] = edge_lookup((index_type)onodes[perm[0]],
                              (index_type)onodes[perm[3]],
                              dl3, l3, edgemap, clipped);

      const Point c1 = Interpolate(l1, l2, 0.5);
      const Point c2 = Interpolate(l2, l3, 0.5);
      const Point c3 = Interpolate(l3, l1, 0.5);

      inodes[6] = face_lookup((index_type)onodes[perm[0]],
                              (index_type)onodes[perm[1]],
                              (index_type)onodes[perm[2]],
                              dl1*0.5, dl2*0.5,
                              c1, facemap, clipped);
      inodes[7] = face_lookup((index_type)onodes[perm[0]],
                              (index_type)onodes[perm[2]],
                              (index_type)onodes[perm[3]],
                              dl2*0.5, dl3*0.5,
                              c2, facemap, clipped);
      inodes[8] = face_lookup((index_type)onodes[perm[0]],
                              (index_type)onodes[perm[3]],
                              (index_type)onodes[perm[1]],
                              dl3*0.5, dl1*0.5,
                              c3, facemap, clipped);

      nnodes[0] = inodes[0];
      nnodes[1] = inodes[3];
      nnodes[2] = inodes[8];
      nnodes[3] = inodes[6];
      clipped->add_elem(nnodes);

      nnodes[0] = inodes[1];
      nnodes[1] = inodes[4];
      nnodes[2] = inodes[6];
      nnodes[3] = inodes[7];
      clipped->add_elem(nnodes);

      nnodes[0] = inodes[2];
      nnodes[1] = inodes[5];
      nnodes[2] = inodes[7];
      nnodes[3] = inodes[8];
      clipped->add_elem(nnodes);

      nnodes[0] = inodes[0];
      nnodes[1] = inodes[6];
      nnodes[2] = inodes[8];
      nnodes[3] = inodes[7];
      clipped->add_elem(nnodes);

      nnodes[0] = inodes[0];
      nnodes[1] = inodes[8];
      nnodes[2] = inodes[2];
      nnodes[3] = inodes[7];
      clipped->add_elem(nnodes);

      nnodes[0] = inodes[0];
      nnodes[1] = inodes[6];
      nnodes[2] = inodes[7];
      nnodes[3] = inodes[1];
      clipped->add_elem(nnodes);

      nnodes[0] = inodes[0];
      nnodes[1] = inodes[1];
      nnodes[2] = inodes[7];
      nnodes[3] = inodes[2];
      clipped->add_elem(nnodes);
    }
    else// if (inside == 0x3 || inside == 0x5 || inside == 0x6 ||
          //     inside == 0x9 || inside == 0xa || inside == 0xc)
    {
        // Lop off two points, break the resulting quads, then add the
        // new tets to the mesh.
      const int *perm = tet_permute_table[inside];
      VMesh::Node::array_type nnodes(4);

      VMesh::Node::index_type inodes[8];
      for (size_t i = 2; i < 4; i++)
      {
        if (nodemap.find((VField::index_type)onodes[perm[i]]) == nodemap.end())
        {
          const VMesh::Node::index_type nodeindex = clipped->add_point(p[perm[i]]);
          nodemap[(VField::index_type)onodes[perm[i]]] = nodeindex;
          inodes[i-2] = nodeindex;
        }
        else
        {
          inodes[i-2] = nodemap[(VField::index_type)onodes[perm[i]]];
        }
      }
      const double imv0 = isoval - v[perm[0]];
      const double dl02 = imv0 / (v[perm[2]] - v[perm[0]]);
      const Point l02 = Interpolate(p[perm[0]], p[perm[2]], dl02);
      const double dl03 = imv0 / (v[perm[3]] - v[perm[0]]);
      const Point l03 = Interpolate(p[perm[0]], p[perm[3]], dl03);

      const double imv1 = isoval - v[perm[1]];
      const double dl12 = imv1 / (v[perm[2]] - v[perm[1]]);
      const Point l12 = Interpolate(p[perm[1]], p[perm[2]], dl12);
      const double dl13 = imv1 / (v[perm[3]] - v[perm[1]]);
      const Point l13 = Interpolate(p[perm[1]], p[perm[3]], dl13);


      inodes[2] = edge_lookup((index_type)onodes[perm[0]],
                              (index_type)onodes[perm[2]],
                              dl02, l02, edgemap, clipped);
      inodes[3] = edge_lookup((index_type)onodes[perm[0]],
                              (index_type)onodes[perm[3]],
                              dl03, l03, edgemap, clipped);
      inodes[4] = edge_lookup((index_type)onodes[perm[1]],
                              (index_type)onodes[perm[2]],
                              dl12, l12, edgemap, clipped);
      inodes[5] = edge_lookup((index_type)onodes[perm[1]],
                              (index_type)onodes[perm[3]],
                              dl13, l13, edgemap, clipped);

      const Point c1 = Interpolate(l02, l03, 0.5);
      const Point c2 = Interpolate(l12, l13, 0.5);

      inodes[6] = face_lookup((index_type)onodes[perm[0]],
                              (index_type)onodes[perm[2]],
                              (index_type)onodes[perm[3]],
                              dl02*0.5,
                              dl03*0.5,
                              c1, facemap, clipped);
      inodes[7] = face_lookup((index_type)onodes[perm[1]],
                              (index_type)onodes[perm[2]],
                              (index_type)onodes[perm[3]],
                              dl12*0.5,
                              dl13*0.5,
                              c2, facemap, clipped);

      nnodes[0] = inodes[7];
      nnodes[1] = inodes[2];
      nnodes[2] = inodes[0];
      nnodes[3] = inodes[4];
      clipped->add_elem(nnodes);

      nnodes[0] = inodes[1];
      nnodes[1] = inodes[5];
      nnodes[2] = inodes[3];
      nnodes[3] = inodes[7];
      clipped->add_elem(nnodes);

      nnodes[0] = inodes[1];
      nnodes[1] = inodes[3];
      nnodes[2] = inodes[6];
      nnodes[3] = inodes[7];
      clipped->add_elem(nnodes);

      nnodes[0] = inodes[0];
      nnodes[1] = inodes[7];
      nnodes[2] = inodes[6];
      nnodes[3] = inodes[2];
      clipped->add_elem(nnodes);

      nnodes[0] = inodes[0];
      nnodes[1] = inodes[1];
      nnodes[2] = inodes[6];
      nnodes[3] = inodes[7];
      clipped->add_elem(nnodes);
    }
  }

  VField* ofield = output->vfield();
  ofield->resize_values();
  CopyProperties(*input, *output);

    // Add the data values from the old field to the new field.
  node_hash_type::iterator nmitr = nodemap.begin();
  while (nmitr != nodemap.end())
  {
    ofield->copy_value(field,((*nmitr).first),((*nmitr).second));
    ++nmitr;
  }

    // Put the isovalue at the edge break points.
  edge_hash_type::iterator emitr = edgemap.begin();
  while (emitr != edgemap.end())
  {
    ofield->set_value(isoval,((*emitr).second));
    ++emitr;
  }

    // Put the isovalue at the face break points.  Assumes linear
    // interpolation across the faces (which seems safe, this is what we
    // used to cut with.)
  face_hash_type::iterator fmitr = facemap.begin();
  while (fmitr != facemap.end())
  {
    ofield->set_value(isoval,((*fmitr).second));
    ++fmitr;
  }

  #ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
  if (algo->get_bool("build_mapping"))
  {
      // Create the interpolant matrix.
    const size_type nrows = clipped->num_nodes();
    const size_type ncols = mesh->num_nodes();

    std::vector<index_type> cctmp(nrows*3);
    std::vector<double> dtmp(nrows*3);
    SparseRowMatrix::Builder sparseBuilder;
    const SparseRowMatrix::Rows& rr = sparseBuilder.allocate_rows(nrows + 1);

    for (index_type i = 0; i < nrows * 3; i++)
    {
      cctmp[i] = -1;
    }

    size_type nnz = 0;

      // Add the data values from the old field to the new field.
    nmitr = nodemap.begin();
    while (nmitr != nodemap.end())
    {
      cctmp[(*nmitr).second * 3] = (*nmitr).first;
      dtmp[(*nmitr).second * 3 + 0] = 1.0;
      nnz++;
      ++nmitr;
    }

      // Insert the double hits into cc.
      // Put the isovalue at the edge break points.
    emitr = edgemap.begin();
    while (emitr != edgemap.end())
    {
      cctmp[(*emitr).second * 3 + 0] = (*emitr).first.first;
      cctmp[(*emitr).second * 3 + 1] = (*emitr).first.second;
      dtmp[(*emitr).second * 3 + 0] = 1.0 - (*emitr).first.dfirst;
      dtmp[(*emitr).second * 3 + 1] = (*emitr).first.dfirst;
      nnz+=2;
      ++emitr;
    }

      // Insert the double hits into cc.
      // Put the isovalue at the edge break points.
    fmitr = facemap.begin();
    while (fmitr != facemap.end())
    {
      cctmp[(*fmitr).second * 3 + 0] = (*fmitr).first.first;
      cctmp[(*fmitr).second * 3 + 1] = (*fmitr).first.second;
      cctmp[(*fmitr).second * 3 + 2] = (*fmitr).first.third;
      dtmp[(*fmitr).second * 3 + 0] =
          1.0 - (*fmitr).first.dsecond - (*fmitr).first.dthird;;
      dtmp[(*fmitr).second * 3 + 1] = (*fmitr).first.dsecond;
      dtmp[(*fmitr).second * 3 + 2] = (*fmitr).first.dthird;
      nnz+=3;

      ++fmitr;
    }

    const SparseRowMatrix::Columns& cc = sparseBuilder.allocate_columns(nnz);
    const SparseRowMatrix::Storage& d = sparseBuilder.allocate_data(nnz);

    index_type j;
    index_type counter = 0;
    rr[0] = 0;
    for (j = 0; j < nrows*3; j++)
    {
      if (j%3 == 0) { rr[j/3 + 1] = rr[j/3]; }
      if (cctmp[j] != -1)
      {
        cc[counter] = cctmp[j];
        d[counter] = dtmp[j];
        rr[j/3 + 1]++;
        counter++;
      }
    }
    mapping = new SparseRowMatrix(nrows, ncols, sparseBuilder.build(), nnz);
  }
  #endif

  return (true);
}

// Algorithm for tri meshes

class ClipMeshByIsovalueAlgoTri
{
  public:
    bool run(const AlgorithmBase* algo,FieldHandle input, FieldHandle& output, MatrixHandle& mapping) const;

private:
    VMesh::Node::index_type
    edge_lookup(VField::index_type u0, VField::index_type u1, double d0,
          const Point &p, detail::edge_hash_type &edgemap,
          VMesh *clipped) const;
};

VMesh::Node::index_type ClipMeshByIsovalueAlgoTri::edge_lookup(VField::index_type u0, VField::index_type u1, double d0, const Point &p, detail::edge_hash_type &edgemap, VMesh *clipped) const
{
  using namespace detail;
  edgepair_t np;
  if (u0 < u1)  { np.first = u0; np.second = u1; np.dfirst = d0; }
  else { np.first = u1; np.second = u0; np.dfirst = 1.0 - d0; }
  const auto loc = edgemap.find(np);
  if (loc == edgemap.end())
  {
    const VMesh::Node::index_type nodeindex = clipped->add_point(p);
    edgemap[np] = nodeindex;
    return nodeindex;
  }
  else
  {
    return loc->second;
  }
}

bool ClipMeshByIsovalueAlgoTri::run(const AlgorithmBase* algo, FieldHandle input, FieldHandle& output, MatrixHandle &mapping) const
{
  VField* field = input->vfield();
  VMesh*  mesh  = input->vmesh();
  VMesh*  clipped = output->vmesh();

  using namespace detail;

  node_hash_type nodemap;
  edge_hash_type edgemap;

  VMesh::Node::array_type onodes(3);
  std::vector<double> v(3);
  std::vector<Point>  p(3);

  double isoval = algo->get(ClipMeshByIsovalueAlgo::ScalarIsoValue).toDouble();

  bool lte = !algo->get(ClipMeshByIsovalueAlgo::LessThanIsoValue).toBool();

  VMesh::size_type num_elems = mesh->num_elems();
  for (VMesh::Elem::index_type idx=0; idx<num_elems; idx++)
  {
    mesh->get_nodes(onodes, idx);

    // Get the values and compute an inside/outside mask.
    VField::index_type inside = 0;
    mesh->get_centers(p, onodes);
    field->get_values(v, onodes);

    for (size_t i = 0; i < onodes.size(); i++)
    {
      inside = inside << 1;
      if (v[i] > isoval)
      {
        inside |= 1;
      }
    }

    // Invert the mask if we are doing less than.
    if (lte) { inside = ~inside & 0x7; }

    if (inside == 0)
    {
      // Discard outside elements.
    }
    else if (inside == 0x7)
    {
      // Add this element to the new mesh.
      VMesh::Node::array_type nnodes(onodes.size());

      for (size_t i = 0; i<onodes.size(); i++)
      {
        if (nodemap.find((VField::index_type)onodes[i]) == nodemap.end())
        {
          const VMesh::Node::index_type nodeindex = clipped->add_point(p[i]);
          nodemap[(VField::index_type)onodes[i]] = nodeindex;
          nnodes[i] = nodeindex;
        }
        else
        {
          nnodes[i] = nodemap[(VField::index_type)onodes[i]];
        }
      }

      clipped->add_elem(nnodes);
    }
    else if (inside == 0x1 || inside == 0x2 || inside == 0x4)
    {
      // Add the corner containing the inside point to the mesh.
      const int *perm = tri_permute_table[inside];
      VMesh::Node::array_type nnodes(onodes.size());
      if (nodemap.find((VField::index_type)onodes[perm[0]]) == nodemap.end())
      {
        const VMesh::Node::index_type nodeindex =
            clipped->add_point(p[perm[0]]);
        nodemap[(VField::index_type)onodes[perm[0]]] = nodeindex;
        nnodes[0] = nodeindex;
      }
      else
      {
        nnodes[0] = nodemap[(VField::index_type)onodes[perm[0]]];
      }

      const double imv = isoval - v[perm[0]];

      const double dl1 = imv / (v[perm[1]] - v[perm[0]]);
      const Point l1 = Interpolate(p[perm[0]], p[perm[1]], dl1);
      const double dl2 = imv / (v[perm[2]] - v[perm[0]]);
      const Point l2 = Interpolate(p[perm[0]], p[perm[2]], dl2);


      nnodes[1] = edge_lookup((VField::index_type)onodes[perm[0]],
			      (index_type)onodes[perm[1]],
			      dl1, l1, edgemap, clipped);

      nnodes[2] = edge_lookup((VField::index_type)onodes[perm[0]],
			      (index_type)onodes[perm[2]],
			      dl2, l2, edgemap, clipped);

      clipped->add_elem(nnodes);
    }
    else
    {
      // Lop off the one point that is outside of the mesh, then add
      // the remaining quad to the mesh by dicing it into two
      // triangles.
      const int *perm = tri_permute_table[inside];
      VMesh::Node::array_type inodes(4);
      if (nodemap.find((index_type)onodes[perm[1]]) == nodemap.end())
      {
        const VMesh::Node::index_type nodeindex = clipped->add_point(p[perm[1]]);
        nodemap[(VField::index_type)onodes[perm[1]]] = nodeindex;
        inodes[0] = nodeindex;
      }
      else
      {
        inodes[0] = nodemap[(VField::index_type)onodes[perm[1]]];
      }

      if (nodemap.find((VField::index_type)onodes[perm[2]]) == nodemap.end())
      {
        const VMesh::Node::index_type nodeindex = clipped->add_point(p[perm[2]]);
        nodemap[(VField::index_type)onodes[perm[2]]] = nodeindex;
        inodes[1] = nodeindex;
      }
      else
      {
        inodes[1] = nodemap[(VField::index_type)onodes[perm[2]]];
      }

      const double imv = isoval - v[perm[0]];
      const double dl1 = imv / (v[perm[1]] - v[perm[0]]);
      const Point l1 = Interpolate(p[perm[0]], p[perm[1]], dl1);
      const double dl2 = imv / (v[perm[2]] - v[perm[0]]);
      const Point l2 = Interpolate(p[perm[0]], p[perm[2]], dl2);

      inodes[2] = edge_lookup((VField::index_type)onodes[perm[0]],
			      (VField::index_type)onodes[perm[1]],
			      dl1, l1, edgemap, clipped);

      inodes[3] = edge_lookup((VField::index_type)onodes[perm[0]],
			      (VField::index_type)onodes[perm[2]],
			      dl2, l2, edgemap, clipped);

      VMesh::Node::array_type nnodes(onodes.size());

      nnodes[0] = inodes[0];
      nnodes[1] = inodes[1];
      nnodes[2] = inodes[3];
      clipped->add_elem(nnodes);

      nnodes[0] = inodes[0];
      nnodes[1] = inodes[3];
      nnodes[2] = inodes[2];
      clipped->add_elem(nnodes);
    }
  }

  VField* ofield = output->vfield();
  ofield->resize_values();
  #ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
   ofield->copy_properties(field);
  #endif

  // Add the data values from the old field to the new field.
  node_hash_type::iterator nmitr = nodemap.begin();
  while (nmitr != nodemap.end())
  {
    ofield->copy_value(field,((*nmitr).first),((*nmitr).second));
    ++nmitr;
  }

  // Put the isovalue at the edge break points.
  edge_hash_type::iterator emitr = edgemap.begin();
  while (emitr != edgemap.end())
  {
    ofield->set_value(isoval, ((*emitr).second));
    ++emitr;
  }

  #ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
  if(algo->get_bool("build_mapping"))
  {
    // Create the interpolant matrix.

    const size_type nrows = clipped->num_nodes();
    const size_type ncols = mesh->num_nodes();
    std::vector<index_type> cctmp(nrows*2);
    std::vector<double> dtmp(nrows*2);
    SparseRowMatrix::Builder sparseBuilder;
    const SparseRowMatrix::Rows& rr = sparseBuilder.allocate_rows(nrows + 1);

    for (index_type i = 0; i < nrows * 2; i++)
    {
      cctmp[i] = -1;
    }

    size_type nnz = 0;

    // Add the data values from the old field to the new field.
    nmitr = nodemap.begin();
    while (nmitr != nodemap.end())
    {
      cctmp[(*nmitr).second * 2] = (*nmitr).first;
      dtmp[(*nmitr).second * 2 + 0] = 1.0;
      nnz++;
      ++nmitr;
    }

    // Insert the double hits into cc.
    // Put the isovalue at the edge break points.
    emitr = edgemap.begin();
    while (emitr != edgemap.end())
    {
      cctmp[(*emitr).second * 2 + 0] = (*emitr).first.first;
      cctmp[(*emitr).second * 2 + 1] = (*emitr).first.second;
      dtmp[(*emitr).second * 2 + 0] = 1.0 - (*emitr).first.dfirst;
      dtmp[(*emitr).second * 2 + 1] = (*emitr).first.dfirst;
      nnz+=2;
      ++emitr;
    }

    const SparseRowMatrix::Columns& cc = sparseBuilder.allocate_columns(nnz);
    const SparseRowMatrix::Storage& d = sparseBuilder.allocate_data(nnz);

    index_type j;
    index_type counter = 0;
    rr[0] = 0;
    for (j = 0; j < nrows*2; j++)
    {
      if (j%2 == 0) { rr[j/2 + 1] = rr[j/2]; }
      if (cctmp[j] != -1)
      {
        cc[counter] = cctmp[j];
        d[counter] = dtmp[j];
        rr[j/2 + 1]++;
        counter++;
      }
    }
    mapping = new SparseRowMatrix(nrows, ncols, sparseBuilder.build(), nnz);
  }
  #endif

  return (true);
}

class ClipMeshByIsovalueAlgoHex
{
  public:
    bool run(const AlgorithmBase* algo,FieldHandle input, FieldHandle& output, MatrixHandle& mapping) const;

    static bool ud_pair_less(const std::pair<VField::index_type, double> &a,
                             const std::pair<VField::index_type, double> &b)
    {
        return a.first < b.first;
    };

};

bool ClipMeshByIsovalueAlgoHex::run(const AlgorithmBase* algo, FieldHandle input, FieldHandle& output, MatrixHandle &mapping) const
{
  // Do marching cubes
  FieldHandle tri_field;

  MarchingCubesAlgo mc_algo;
  #ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
   mc_algo.set_progress_reporter(algo->get_progress_reporter());
  #endif

  double isoval = algo->get(ClipMeshByIsovalueAlgo::ScalarIsoValue).toDouble();
  mc_algo.set(MarchingCubesAlgo::build_field, true);

  bool lte = !algo->get(ClipMeshByIsovalueAlgo::LessThanIsoValue).toBool();

  std::vector<double> isovals(1);
  isovals[0] = isoval;

   // Run marching cubes
  mc_algo.run(input,isovals,tri_field);
  VMesh *tri_mesh = tri_field->vmesh();
  VField* field = input->vfield();
  VMesh*  mesh  = input->vmesh();
  VMesh*  clipped = output->vmesh();

  // Get a list of the original boundary elements (code from FieldBoundary).
  std::map<VMesh::DElem::index_type, VMesh::DElem::index_type> original_boundary;
  mesh->synchronize(Mesh::ELEM_NEIGHBORS_E | Mesh::FACES_E);

  // Walk all the cells in the mesh looking for faces on the boundary

  VMesh::size_type num_elems = mesh->num_elems();
  VMesh::DElem::array_type delems;
  VMesh::Elem::index_type nidx;

  for (VMesh::Elem::index_type idx=0; idx< num_elems; idx++)
  {
    // Get all the faces in the cell.
    mesh->get_delems(delems, idx);

    for (size_t j=0; j<delems.size(); j++)
    {
      if( !mesh->get_neighbor(nidx, idx, delems[j] ) )
      {
        // Faces with no neighbors are on the boundary.
        original_boundary[delems[j]] = delems[j];
      }
    }
  }

  // Create a map to help differentiate between new nodes created for
  // the inserted sheet, and the nodes on the stair stepped boundary.
  std::map<VMesh::Node::index_type, VMesh::Node::index_type> clipped_to_original_nodemap;
  typedef boost::unordered_map<VField::index_type, VMesh::Node::index_type> hash_type;

  hash_type nodemap;

  std::vector<VMesh::Elem::index_type> elemmap;

  // Find all of the hexes inside the isosurface and add them to the
  // clipped mesh.
  for (VMesh::Elem::index_type idx=0; idx<num_elems; idx++)
  {
    bool inside = false;

    VMesh::Node::array_type onodes;
    mesh->get_nodes(onodes, idx);
    inside = true;

    for (size_t i = 0; i < onodes.size(); i++)
    {
      double v;
      field->get_value(v, onodes[i]);

      if( lte )
      {
        if( v > isoval )
        {
          inside = false;
          break;
        }
      }
      else
      {
        if( v < isoval )
        {
          inside = false;
          break;
        }
      }
    }

    if (inside)
    {
      VMesh::Node::array_type onodes;
      mesh->get_nodes(onodes, idx);

      // Add this element to the new mesh.
      VMesh::Node::array_type nnodes(onodes.size());

      for (size_t i = 0; i<onodes.size(); i++)
      {
        if (nodemap.find((VField::index_type)onodes[i]) == nodemap.end())
        {
          Point np;
          mesh->get_center(np, onodes[i]);
          const VMesh::Node::index_type nodeindex = clipped->add_point(np);
          clipped_to_original_nodemap[nodeindex] = onodes[i];
          nodemap[(VField::index_type)onodes[i]] = nodeindex;
          nnodes[i] = nodeindex;
        }
        else
        {
          nnodes[i] = nodemap[(VField::index_type)onodes[i]];
        }
      }

      clipped->add_elem(nnodes);
      elemmap.push_back(idx); // Assumes elements always added to end.
    }
  }

  // Get the boundary elements of the clipped mesh (code from FieldBoundary)
  // We'll use this list of boundary elements (minus the elements from
  // the original boundary) so we know which nodes to project to the
  // isosurface to create the new sheet of hexes.
  std::map<VMesh::Node::index_type, VMesh::Node::index_type> vertex_map;

  std::vector<VMesh::Node::index_type> node_list;
  std::vector<VMesh::DElem::index_type> face_list;

  clipped->synchronize( Mesh::ELEM_NEIGHBORS_E | Mesh::FACES_E );

  // Walk all the cells in the clipped mesh to find the boundary faces.

  VMesh::size_type num_celems = clipped->num_elems();
  VMesh::DElem::array_type faces;
  VMesh::DElem::index_type old_face;
  VMesh::Node::array_type face_nodes;
  VMesh::Node::array_type nodes;

  for (VMesh::Elem::index_type idx = 0; idx<num_celems; idx++)
  {
    // Get all the faces in the cell.
    clipped->get_delems( faces, idx );

    // Check each face for neighbors.
    VMesh::DElem::array_type::iterator fiter = faces.begin();

    while( fiter != faces.end() )
    {
      VMesh::Elem::index_type nci;
      VMesh::DElem::index_type fi = *fiter;
      ++fiter;

      if( !clipped->get_neighbor( nci, idx, fi ) )
      {
        // Faces with no neighbors are on the boundary.  Make sure
        // that this face isn't on the original boundary.
        bool is_old_boundary = false;
        index_type i;

        clipped->get_nodes( face_nodes, fi );
        for (size_t j=0;j<4; j++) face_nodes[j] = clipped_to_original_nodemap[face_nodes[j]];
        if( mesh->get_delem( old_face, face_nodes) )
        {
          auto bound_iter = original_boundary.find( old_face );
          if( bound_iter != original_boundary.end() )
          {
            is_old_boundary = true;
          }
        }

        // Don't add the nodes from the faces of the original boundary
        // to the list of nodes that we'll be projecting later to
        // create the new sheet of hex elements.
        if( !is_old_boundary )
        {
          face_list.push_back( fi );

          clipped->get_nodes( nodes, fi );

          VMesh::Node::array_type::iterator niter = nodes.begin();

          VMesh::size_type size = nodes.size();
          for( i = 0; i < size; i++ )
          {
            auto node_iter = vertex_map.find( *niter );
            if( node_iter == vertex_map.end() )
            {
              node_list.push_back( *niter );
              vertex_map[*niter] = *niter;
            }
            ++niter;
          }
        }
      }
    }
  }

  // For each new node on the clipped boundary, project a new node to
  // the isosurface create a map between the clipped boundary nodes
  // and the new nodes to help us create hexes with the correct
  // connectivity later.
  if (!tri_mesh->is_empty())
    tri_mesh->synchronize( Mesh::FIND_CLOSEST_ELEM_E );
  std::map<VMesh::Node::index_type, VMesh::Node::index_type> new_map;

  int cnt = 0;
  for(size_t i = 0; i < node_list.size(); i++ )
  {
    VMesh::Node::index_type this_node = node_list[i];
    Point n_p;
    clipped->get_center( n_p, this_node );

    Point new_result;
    VMesh::Elem::index_type face_id;
    double dist;

    tri_mesh->find_closest_elem(dist, new_result, face_id, n_p );

    // Add the new node to the clipped mesh.
    Point new_point( new_result );
    VMesh::Node::index_type this_index = clipped->add_point( new_point );

    // Create a map for the new node to a node on the boundary of the
    // clipped mesh.
    new_map[this_node] = this_index;
    cnt++;
    if (cnt == 100)
    {
     cnt = 0;
     #ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
      algo->update_progress(i,(size_type)(1.3*num_celems));
     #endif
    }
  }

  // For each quad on the clipped boundary we have a map to the new
  // projected nodes so, create the new sheet of hexes from each quad
  // on the clipped boundary
  std::vector<VMesh::Elem::index_type> new_elems;
  for(size_t i = 0; i < face_list.size(); i++ )
  {
    VMesh::Node::array_type nodes;
    clipped->get_nodes( nodes, face_list[i] );

    VMesh::Node::array_type nnodes(8);
    nnodes[0] = nodes[3];
    nnodes[1] = nodes[2];
    nnodes[2] = nodes[1];
    nnodes[3] = nodes[0];
    nnodes[4] = new_map[nodes[3]];
    nnodes[5] = new_map[nodes[2]];
    nnodes[6] = new_map[nodes[1]];
    nnodes[7] = new_map[nodes[0]];

    new_elems.push_back( clipped->add_elem( nnodes ) );
  }

  // Force all the synch data to be rebuilt on next synch call.
  clipped->clear_synchronization();

  VField* ofield = output->vfield();
  ofield->resize_values();
  CopyProperties(*input, *output);

  #ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
  const size_type nrows = nodemap.size() + node_list.size();
  // Create the interpolation matrix for downstream use.
  hash_type::iterator hitr = nodemap.begin();
  const size_type ncols = field->num_values();
  const size_type nnz = nrows+7*node_list.size();
  SparseRowMatrix::Data sparseData(nrows+1, nnz);
  const SparseRowMatrix::Rows& rr = sparseData.rows();
  const SparseRowMatrix::Columns& cc = sparseData.columns();
  const SparseRowMatrix::Storage& d = sparseData.data();

  // Nodes in the original mesh will have the same field values as
  // before since we didn't move any of them.
  while( hitr != nodemap.end() )
  {
    ofield->copy_value(field,((*hitr).first), ((*hitr).second));
    cc[(*hitr).second] = (*hitr).first;
    ++hitr;
  }

  // Nodes in the original mesh have a one-to-one correspondence in
  // the interp matrix.
  size_t i;
  for( i = 0; i < nodemap.size(); i++ )
  {
    rr[i] = i;
    d[i] = 1.0;
  }

  // Figure out the correspondence for the new nodes with the original mesh.
  index_type counter = i;
  index_type rrvalue = i;

  mesh->synchronize(Mesh::LOCATE_E);
  for( size_t j = 0; j < node_list.size(); j++ )
  {
    rr[counter+j] = rrvalue;

    VMesh::Node::index_type ni = new_map[node_list[j]];

    Point p;
    clipped->get_center(p, ni);

    // Find the cell in the old mesh which the new node is located in and
    // interpolate values between the nodes in this cell for the new values.
    VMesh::Elem::index_type el;
    if( mesh->locate( el, p ) )
    {
      VMesh::ElemInterpolate ei;
      mesh->get_interpolate_weights(p, ei, 1);

      std::vector<std::pair<VField::index_type, double> > snodes(8);
      for (int k = 0; k < 8; k++)
      {
        snodes[k].first =  ei.node_index[k];
        snodes[k].second = ei.weights[k];
      }
      std::sort(snodes.begin(), snodes.end(), ud_pair_less);
      double val;
      double actual_val = 0;
      for( int k = 0; k < 8; k++ )
      {
        field->get_value( val, snodes[k].first );
        actual_val += snodes[k].first * val;
        cc[rrvalue+k] = snodes[k].first;
        d[rrvalue+k] = snodes[k].second;
      }
      ofield->set_value( actual_val, ni );
    }
    else
    {
      // Just find nearest node in the mesh.
      VMesh::Node::index_type oi;
      mesh->locate( oi, p );
      cc[rrvalue] = oi;
      d[rrvalue] = 1.0;
      cc[rrvalue+1] = 0;
      d[rrvalue+1] = 0.0;
      cc[rrvalue+2] = 0;
      d[rrvalue+2] = 0.0;
      cc[rrvalue+3] = 0;
      d[rrvalue+3] = 0.0;
      cc[rrvalue+4] = 0;
      d[rrvalue+4] = 0.0;
      cc[rrvalue+5] = 0;
      d[rrvalue+5] = 0.0;
      cc[rrvalue+6] = 0;
      d[rrvalue+6] = 0.0;
      cc[rrvalue+7] = 0;
      d[rrvalue+7] = 0.0;
    }

    rrvalue += 8;
  }
  rr[nrows] = rrvalue; // An extra entry goes on the end of rr.

  // Create the interp matrix.
  mapping = new SparseRowMatrix( nrows, ncols, sparseData, nrows+7*node_list.size() );
  #endif

  return (true);
}


// Version without building mapping matrix
bool ClipMeshByIsovalueAlgo::run(FieldHandle input, FieldHandle& output) const
{
   // Store old setting
   #ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
   bool temp = get_bool("build_mapping");
   set_bool("build_mapping",false);
   #endif
   MatrixHandle dummy;
   // Run full algorithm

   bool ret = run(input,output,dummy);
   // Reset old setting
   #ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
   set_bool("build_mapping",temp);
   #endif
   // Return result
   return (ret);

}

// Version with building mapping matrix
bool ClipMeshByIsovalueAlgo::run(FieldHandle input, FieldHandle& output, MatrixHandle& mapping) const
{
  // Mark that we are starting the algorithm, but do not report progress
  #ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
   algo_start("ClipMeshByisoValue");
  #endif
  // Step 0:
  // Safety test:
  // Test whether we received actually a field. A handle can point to no object.
  // Using a null handle will cause the program to crash. Hence it is a good
  // policy to check all incoming handles and to see whether they point to actual
  // objects.

  // Handle: the function get_rep() returns the pointer contained in the handle
  if (!input)
  {
    // If we encounter a null pointer we return an error message and return to
    // the program to deal with this error.
    error("No input field");
    return (false);
  }

  // Step 1: determine the type of the input fields and determine what type the
  // output field should be.

  FieldInformation fi(input);
  // Here we test whether the class is part of any of these newly defined
  // non-linear classes. If so we return an error.
  if (fi.is_nonlinear())
  {
    error("This algorithm has not yet been defined for non-linear elements yet");
    return (false);
  }

  if (!(fi.is_tet_element()||fi.is_hex_element()||fi.is_tri_element()))
  {
    // Notify the user that no action is done
    error("This algorithm only works on a TetVolMesh, a HexVolMesh, or a TriSurfMesh");
    return (false);
  }

  // Needs to be scalar data
  if (!(fi.is_scalar()))
  {
    error("This algorithms requires a scalar value as field data");
    return (false);
  }

  // Needs to be linear data
  if (!(fi.is_lineardata()))
  {
    error("This algorithm is only defined for data located at the nodes of the elements");
    return (false);
  }

  // For unstructuring
  if (fi.is_hex_element()) fi.make_hexvolmesh();
  if (fi.is_quad_element()) fi.make_quadsurfmesh();
  if (fi.is_crv_element()) fi.make_curvemesh();

  output = CreateField(fi);
  if (!output)
  {
    error("Could not allocate output field");
    return (false);
  }

  if (fi.is_tet_element())
  {
    ClipMeshByIsovalueAlgoTet algo;
    if(!( algo.run(this,input,output,mapping)))
    {
      return (false);
    }
  }
  else if (fi.is_tri_element())
  {
    ClipMeshByIsovalueAlgoTri algo;
    if(!( algo.run(this,input,output,mapping)))
    {
      return (false);
    }
  }
  else if (fi.is_hex_element())
  {
    ClipMeshByIsovalueAlgoHex algo;
    if(!( algo.run(this,input,output,mapping)))
    {
      return (false);
    }
  }

   #ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
  /// Copy properties of the property manager
	output->copy_properties(input.get_rep());
   #endif
  // Success:
  return (true);
}

AlgorithmInputName ClipMeshByIsovalueAlgo::InputField("InputField");
AlgorithmOutputName ClipMeshByIsovalueAlgo::OutputField("OutputField");
AlgorithmParameterName ClipMeshByIsovalueAlgo::LessThanIsoValue("LessThanIsoValue");
AlgorithmParameterName ClipMeshByIsovalueAlgo::ScalarIsoValue("ScalarIsoValue");

AlgorithmOutput ClipMeshByIsovalueAlgo::run(const AlgorithmInput& input) const
{
  auto inputField = input.get<Field>(InputField);

  FieldHandle output_fld;

  if (!run(inputField, output_fld))
    error("False returned on legacy run call.");

  AlgorithmOutput output;
  output[OutputField] = output_fld;

  #ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
   output[Mapping] = mapping;
  #endif

  return output;
}
