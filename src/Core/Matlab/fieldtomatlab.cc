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


/*
 * FILE: matlabconverter.cc
 * AUTH: Jeroen G Stinstra
 * DATE: 18 MAR 2004
 */

#include <Core/Matlab/fieldtomatlab.h>
#include <Core/Matlab/matlabarray.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Logging/LoggerInterface.h>
#include <Core/GeometryPrimitives/Point.h>

using namespace SCIRun;
using namespace SCIRun::MatlabIO;
using namespace SCIRun::Core::Logging;
using namespace SCIRun::Core::Geometry;


FieldToMatlabAlgo::FieldToMatlabAlgo() :
option_forceoldnames_(false),
  option_nofieldconnectivity_(false),
  option_indexbase_(1)
{
}

void FieldToMatlabAlgo::setreporter(LoggerHandle pr)
{
  pr_ = pr;
}


void FieldToMatlabAlgo::option_forceoldnames(bool value)
{
  option_forceoldnames_ = value;
}

void FieldToMatlabAlgo::option_nofieldconnectivity(bool value)
{
  option_nofieldconnectivity_ = value;
}

void FieldToMatlabAlgo::error(const std::string& error)
{
  if(pr_) pr_->error(error);
}

void FieldToMatlabAlgo::warning(const std::string& warning)
{
  if(pr_) pr_->warning(warning);
}

bool
FieldToMatlabAlgo::execute(FieldHandle fieldH, matlabarray &mlarray)
{
  // Check whether the field actually contains a field;
  if (!fieldH)
  {
    error("FieldToMatlab: Field is empty.");
    return(false);
  }

  // input is a general FieldHandle, cast this to the specific one
  VField* field = fieldH->vfield();
  VMesh*  mesh = fieldH->vmesh();
  FieldInformation fi(fieldH);

  if (!field)
  {
    error("FieldToMatlab: This algorithm cannot handle this kind of field.");
    return(false);
  }

  if (!mesh)
  {
    error("FieldToMatlab: This algorithm cannot handle this kind of mesh.");
    return (false);
  }

  // DEAL WITH REGULAR FIELDS

  if (mesh->is_pointcloudmesh())
  {
    if (!(field->is_constantdata() ||field->is_nodata()))
    {
      error("FieldToMatlab: Point clouds should have data solely confined to the nodes.");
      error("FieldToMatlab: No linear or higher order interpolation in point clouds is supported.");
      return (false);
    }
  }

  //////////////////////////////////////
  //vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv//

  if (field->is_nonlineardata()||mesh->is_nonlinearmesh())
  {
    error("FieldToMatlab: Exporting higher fields is currently disabled.");
    error("FieldToMatlab: Higher order support is work in progress, but is currently disabled.");
    return (false);
  }

  if (mesh->is_scanlinemesh())
  {
    return (  mladdfieldheader(fi,mlarray) &&
              mladdfielddata(field,mesh,mlarray) &&
              mladdfieldderivatives(field,mesh,mlarray) &&
              mladdmeshheader(fi,mlarray) &&
              mladdtransform(mesh,mlarray) &&
              mladddimension1d(mesh,mlarray) );
  }

  if (mesh->is_imagemesh())
  {
    return (  mladdfieldheader(fi,mlarray) &&
              mladdfielddata(field,mesh,mlarray) &&
              mladdfieldderivatives(field,mesh,mlarray) &&
              mladdmeshheader(fi,mlarray) &&
              mladdtransform(mesh,mlarray) &&
              mladddimension2d(mesh,mlarray) );
  }

  if (mesh->is_latvolmesh())
  {
    return (  mladdfieldheader(fi,mlarray) &&
              mladdfielddata(field,mesh,mlarray) &&
              mladdfieldderivatives(field,mesh,mlarray) &&
              mladdmeshheader(fi,mlarray) &&
              mladdtransform(mesh,mlarray) &&
              mladddimension3d(mesh,mlarray) );
  }


  if (mesh->is_structcurvemesh())
  {
    return (  mladdfieldheader(fi,mlarray) &&
              mladdfielddata(field,mesh,mlarray) &&
              mladdfieldderivatives(field,mesh,mlarray) &&
              mladdmeshheader(fi,mlarray) &&
              mladdxyzmesh1d(mesh,mlarray) );
  }

  if (mesh->is_structquadsurfmesh())
  {
    return (  mladdfieldheader(fi,mlarray) &&
              mladdfielddata(field,mesh,mlarray) &&
              mladdfieldderivatives(field,mesh,mlarray) &&
              mladdmeshheader(fi,mlarray) &&
              mladdxyzmesh2d(mesh,mlarray) );
  }

  if (mesh->is_structhexvolmesh())
  {
    return (  mladdfieldheader(fi,mlarray) &&
              mladdfielddata(field,mesh,mlarray) &&
              mladdfieldderivatives(field,mesh,mlarray) &&
              mladdmeshheader(fi,mlarray) &&
              mladdxyzmesh3d(mesh,mlarray) );
  }

  if (mesh->is_pointcloudmesh())
  {
    return (  mladdfieldheader(fi,mlarray) &&
              mladdfielddata(field,mesh,mlarray) &&
              mladdmeshheader(fi,mlarray) &&
              mladdnodes(mesh,mlarray) );
  }

  if (mesh->is_curvemesh())
  {
    return (  mladdfieldheader(fi,mlarray) &&
              mladdfielddata(field,mesh,mlarray) &&
              mladdfieldedges(field,mesh,mlarray) &&
              mladdfieldedgederivatives(field,mesh,mlarray) &&
              mladdmeshheader(fi,mlarray) &&
              mladdnodes(mesh,mlarray) &&
              mladdedges(mesh,mlarray) &&
              mladdmeshderivatives(mesh,mlarray) );
  }

  if (mesh->is_trisurfmesh())
  {
    return (  mladdfieldheader(fi,mlarray) &&
              mladdfielddata(field,mesh,mlarray) &&
              mladdfieldfaces(field,mesh,mlarray) &&
              mladdfieldfacederivatives(field,mesh,mlarray) &&
              mladdmeshheader(fi,mlarray) &&
              mladdnodes(mesh,mlarray) &&
              mladdfaces(mesh,mlarray) &&
              mladdmeshderivatives(mesh,mlarray) );
  }

  if (mesh->is_quadsurfmesh())
  {
    return (  mladdfieldheader(fi,mlarray) &&
              mladdfielddata(field,mesh,mlarray) &&
              mladdfieldfaces(field,mesh,mlarray) &&
              mladdfieldfacederivatives(field,mesh,mlarray) &&
              mladdmeshheader(fi,mlarray) &&
              mladdnodes(mesh,mlarray) &&
              mladdfaces(mesh,mlarray) &&
              mladdmeshderivatives(mesh,mlarray) );
  }

  if (mesh->is_tetvolmesh())
  {
    return (  mladdfieldheader(fi,mlarray) &&
              mladdfielddata(field,mesh,mlarray) &&
              mladdfieldcells(field,mesh,mlarray) &&
              mladdfieldcellderivatives(field,mesh,mlarray) &&
              mladdmeshheader(fi,mlarray) &&
              mladdnodes(mesh,mlarray) &&
              mladdcells(mesh,mlarray) &&
              mladdmeshderivatives(mesh,mlarray) );
  }

  if (mesh->is_prismvolmesh())
  {
    return (  mladdfieldheader(fi,mlarray) &&
              mladdfielddata(field,mesh,mlarray) &&
              mladdfieldcells(field,mesh,mlarray) &&
              mladdfieldcellderivatives(field,mesh,mlarray) &&
              mladdmeshheader(fi,mlarray) &&
              mladdnodes(mesh,mlarray) &&
              mladdcells(mesh,mlarray) &&
              mladdmeshderivatives(mesh,mlarray) );
  }

  if (mesh->is_hexvolmesh())
  {
    return (  mladdfieldheader(fi,mlarray) &&
              mladdfielddata(field,mesh,mlarray) &&
              mladdfieldcells(field,mesh,mlarray) &&
              mladdfieldcellderivatives(field,mesh,mlarray) &&
              mladdmeshheader(fi,mlarray) &&
              mladdnodes(mesh,mlarray) &&
              mladdcells(mesh,mlarray) &&
              mladdmeshderivatives(mesh,mlarray) );
  }

  error("FieldToMatlab: No converter available for this type of field.");
  return (false);
}

bool FieldToMatlabAlgo::mladdmeshheader(const FieldInformation& fi, matlabarray& mlarray)
{
  matlabarray mlmeshbasis;
  mlmeshbasis.createstringarray(fi.get_mesh_basis_type());
  mlarray.setfield(0,"meshbasis",mlmeshbasis);

  matlabarray mlmeshbasisorder;
  mlmeshbasisorder.createdoublescalar(fi.mesh_basis_order());
  mlarray.setfield(0,"meshbasisorder",mlmeshbasisorder);

  matlabarray mlmeshtype;
  mlmeshtype.createstringarray(fi.get_mesh_type());
  mlarray.setfield(0,"meshtype",mlmeshtype);

  return(true);
}


bool FieldToMatlabAlgo::mladdnodes(VMesh* mesh, matlabarray& mlarray)
{
  matlabarray node;

  // A lot of pointless casting, but that is the way SCIRun was setup .....
  // Iterators and Index classes to make the code really complicated
  // The next code tries to get away with minimal use of all this overhead

  unsigned int numnodes = static_cast<unsigned int>(mesh->num_nodes());

  // Request that it generates the node matrix
  mesh->synchronize(Mesh::NODES_E);

  // Buffers for exporting the data to Matlab.
  // The MatlabIO does not use SCIRun style iterators hence we need to extract
  // the data first.
  std::vector<double> nodes(3*numnodes);
  std::vector<int> dims(2);

  // Setup the dimensions of the Matlab array
  dims[0] = 3; dims[1] = static_cast<int>(numnodes);

  // This works, it might not be really efficient, at least it does not
  // hack into the object.

  Point P;
  unsigned int q = 0;

  VMesh::Node::iterator it, it_end;
  mesh->begin(it);
  mesh->end(it_end);

  while(it != it_end)
  {
    mesh->get_point(P,*(it));
    nodes[q++] = P.x();
    nodes[q++] = P.y();
    nodes[q++] = P.z();
    ++it;
  }

  node.createdoublematrix(nodes,dims);
  mlarray.setfield(0,"node",node);

  if (mesh->is_quadraticmesh())
  {
    // NEED ACCESS FUNCTION TO HO ELEMENTS
    // TO MAKE THIS FUNCTION WORK

    error("FieldToMatlab: Currently no access function available to get higher order node locations.");
    return(false);
  }

  return (true);
}


bool
FieldToMatlabAlgo::mladdedges(VMesh *mesh, matlabarray& mlarray)
{
  mesh->synchronize(Mesh::EDGES_E);

  size_t num = mesh->num_nodes_per_edge();
  size_t numedges = mesh->num_edges();

  std::vector<int> dims(2);
  dims[0] = static_cast<int>(num);
  dims[1] = static_cast<int>(numedges);

  std::vector<VMesh::index_type> edges(num*numedges);

  VMesh::Node::array_type array;

  // SCIRun iterators are limited in supporting any index management
  // Hence I prefer to do it with integer and convert to the required
  // class at the last moment. Hopefully the compiler is smart and
  // has a fast translation.
  VMesh::Edge::iterator it, it_end;
  mesh->begin(it);
  mesh->end(it_end);
  size_t q = 0;

  while (it != it_end)
  {
    mesh->get_nodes(array,*(it));
    for (size_t r = 0; r < num; r++) edges[q++] = static_cast<VMesh::index_type>(array[r]) + option_indexbase_;
    ++it;
  }

	matlabarray edge;
  edge.createdensearray(dims,matlabarray::miUINT32);
  edge.setnumericarray(edges); // store them as UINT32 but treat them as doubles
  mlarray.setfield(0,"edge",edge);

  return (true);
}


bool
FieldToMatlabAlgo::mladdfaces(VMesh *mesh, matlabarray& mlarray)
{
  mesh->synchronize(Mesh::FACES_E);

  size_t num = mesh->num_nodes_per_face();
  size_t numfaces = mesh->num_faces();

  VMesh::Node::array_type array;

  std::vector<VMesh::index_type> faces(num*numfaces);
  std::vector<int> dims(2);
  dims[0] = static_cast<int>(num);
  dims[1] = static_cast<int>(numfaces);

  size_t q = 0;
  VMesh::Face::iterator it, it_end;
  mesh->begin(it);
  mesh->end(it_end);
  while (it != it_end)
  {
    mesh->get_nodes(array,*(it));
    for (size_t r = 0; r < num; r++) faces[q++] = static_cast<VMesh::index_type>(array[r]) + option_indexbase_;
    ++it;
  }

	matlabarray face;
  face.createdensearray(dims,matlabarray::miUINT32);
  face.setnumericarray(faces); // store them as UINT32 but treat them as doubles
  mlarray.setfield(0,"face",face);

  return (true);
}


bool
FieldToMatlabAlgo::mladdcells(VMesh* mesh, matlabarray& mlarray)
{
  mesh->synchronize(Mesh::CELLS_E);

  size_t numcells = mesh->num_cells();
  size_t num = mesh->num_nodes_per_elem();

  VMesh::Node::array_type array;
  std::vector<VMesh::index_type> cells(num*numcells);
  std::vector<int> dims(2);
  dims[0] = static_cast<int>(num);
  dims[1] = static_cast<int>(numcells);

  VMesh::Cell::iterator it, it_end;
  mesh->begin(it);
  mesh->end(it_end);
  size_t q = 0;

  while(it != it_end)
  {
    mesh->get_nodes(array,*(it));
    for (size_t r = 0; r < num; r++) cells[q++] = static_cast<VMesh::index_type>(array[r]) + option_indexbase_;
    ++it;
  }

  matlabarray cell;
  cell.createdensearray(dims,matlabarray::miUINT32);
  cell.setnumericarray(cells); // store them as UINT32 but treat them as doubles
  mlarray.setfield(0,"cell",cell);

  return (true);
}


bool
FieldToMatlabAlgo::mladdmeshderivatives(VMesh* mesh, matlabarray& /*mlarray*/)
{
  if (mesh->is_cubicmesh())
  {
    // CANNOT DO THIS EITHER, NO ACCESS FUNCTIONS TO DATA
    // UNLESS I HACK INTO THE BASIS CLASS

    error("FieldToMatlab: Currently no access function available to get higher order node derivatives.");
    return (false);
  }
  else
  {
    return (true);
  }
}


bool
FieldToMatlabAlgo::mladdtransform(VMesh* mesh, matlabarray& mlarray)
{
  if (mesh->is_nonlinearmesh())
  {
    error("FieldToMatlab: No higher order elements are supported for regular meshes");
    return(false);
  }

  Transform T;
  matlabarray transform;
  double data[16];

  T = mesh->get_transform();
  T.get_trans(data);
  transform.createdensearray(4,4,matlabarray::miDOUBLE);
  transform.setnumericarray(data,16);
  mlarray.setfield(0,"transform",transform);
  return(true);
}


bool
FieldToMatlabAlgo::mladddimension1d(VMesh* mesh, matlabarray& mlarray)
{
  matlabarray dim;
  dim.createdensearray(1,1,matlabarray::miDOUBLE);
  std::vector<double> dims(1);

  VMesh::Node::size_type size;
  mesh->size(size);
  dims[0] = static_cast<double>(size);

  dim.setnumericarray(dims);
  mlarray.setfield(0,"dims",dim);

  return(true);
}


bool
FieldToMatlabAlgo::mladddimension2d(VMesh* mesh, matlabarray& mlarray)
{
  matlabarray dim;
  dim.createdensearray(1,2,matlabarray::miDOUBLE);
  std::vector<double> dims(2);

  dims[0] = static_cast<double>(mesh->get_ni());
  dims[1] = static_cast<double>(mesh->get_nj());

  dim.setnumericarray(dims);
  mlarray.setfield(0,"dims",dim);

  return(true);
}


bool
FieldToMatlabAlgo::mladddimension3d(VMesh* mesh, matlabarray& mlarray)
{
  matlabarray dim;
  dim.createdensearray(1,3,matlabarray::miDOUBLE);
  std::vector<double> dims(3);

  dims[0] = static_cast<double>(mesh->get_ni());
  dims[1] = static_cast<double>(mesh->get_nj());
  dims[2] = static_cast<double>(mesh->get_nk());

  dim.setnumericarray(dims);
  mlarray.setfield(0,"dims",dim);

  return(true);
}


bool
FieldToMatlabAlgo::mladdxyzmesh1d(VMesh* mesh, matlabarray& mlarray)
{
  if (!(mesh->is_linearmesh()))
  {
    error("FieldToMatlab: Currently no higher order geometry available for structured meshes.");
    return (false);
  }

  matlabarray x,y,z;
  mesh->synchronize(Mesh::NODES_E);

  VMesh::Node::size_type size;
  mesh->size(size);
  unsigned int numnodes = static_cast<unsigned int>(size);

  x.createdensearray(static_cast<int>(numnodes),1,matlabarray::miDOUBLE);
  y.createdensearray(static_cast<int>(numnodes),1,matlabarray::miDOUBLE);
  z.createdensearray(static_cast<int>(numnodes),1,matlabarray::miDOUBLE);

  std::vector<double> xbuffer(numnodes);
  std::vector<double> ybuffer(numnodes);
  std::vector<double> zbuffer(numnodes);

  Point P;
  for (unsigned int p = 0; p < numnodes ; p++)
  {
    mesh->get_point(P,VMesh::Node::index_type(p));
    xbuffer[p] = P.x();
    ybuffer[p] = P.y();
    zbuffer[p] = P.z();
  }

  x.setnumericarray(xbuffer);
  y.setnumericarray(ybuffer);
  z.setnumericarray(zbuffer);

  mlarray.setfield(0,"x",x);
  mlarray.setfield(0,"y",y);
  mlarray.setfield(0,"z",z);

  return (true);
}



bool
FieldToMatlabAlgo::mladdxyzmesh2d(VMesh* mesh, matlabarray& mlarray)
{
  if (!(mesh->is_linearmesh()))
  {
    error("FieldToMatlab: Currently no higher order geometry available for structured meshes.");
    return (false);
  }

  matlabarray x,y,z;
  mesh->synchronize(Mesh::NODES_E);

  unsigned int dim1 = static_cast<unsigned int>(mesh->get_ni());
  unsigned int dim2 = static_cast<unsigned int>(mesh->get_nj());
  unsigned int numnodes = dim1*dim2;

  // Note: the dimensions are in reverse order as SCIRun uses C++
  // ordering
  x.createdensearray(static_cast<int>(dim2),static_cast<int>(dim1),matlabarray::miDOUBLE);
  y.createdensearray(static_cast<int>(dim2),static_cast<int>(dim1),matlabarray::miDOUBLE);
  z.createdensearray(static_cast<int>(dim2),static_cast<int>(dim1),matlabarray::miDOUBLE);

  // We use temp buffers to store all the values before committing them to the Matlab
  // classes, this takes up more memory, but should decrease the number of actual function
  // calls, which should be boost performance
  std::vector<double> xbuffer(numnodes);
  std::vector<double> ybuffer(numnodes);
  std::vector<double> zbuffer(numnodes);

  Point P;
  unsigned int r = 0;
  for (unsigned int p = 0; p < dim1 ; p++)
    for (unsigned int q = 0; q < dim2 ; q++)
    {
      // It's ulgy, it's SCIRun ......
      mesh->get_point(P,VMesh::Node::index_type(r));
      xbuffer[r] = P.x();
      ybuffer[r] = P.y();
      zbuffer[r] = P.z();
      r++;
    }

  x.setnumericarray(xbuffer);
  y.setnumericarray(ybuffer);
  z.setnumericarray(zbuffer);

  mlarray.setfield(0,"x",x);
  mlarray.setfield(0,"y",y);
  mlarray.setfield(0,"z",z);

  return (true);
}


bool
FieldToMatlabAlgo::mladdxyzmesh3d(VMesh* mesh, matlabarray& mlarray)
{
  if (!(mesh->is_linearmesh()))
  {
    error("FieldToMatlab: Currently no higher order geometry available for structured meshes.");
    return (false);
  }

  matlabarray x,y,z;
  mesh->synchronize(Mesh::NODES_E);

  unsigned int dim1 = static_cast<unsigned int>(mesh->get_ni());
  unsigned int dim2 = static_cast<unsigned int>(mesh->get_nj());
  unsigned int dim3 = static_cast<unsigned int>(mesh->get_nk());
  unsigned int numnodes = dim1*dim2*dim3;

  // Note: the dimensions are in reverse order as SCIRun uses C++
  // ordering
  std::vector<int> dims(3); dims[0] = static_cast<int>(dim3);
  dims[1] = static_cast<int>(dim2); dims[2] = static_cast<int>(dim1);
  x.createdensearray(dims,matlabarray::miDOUBLE);
  y.createdensearray(dims,matlabarray::miDOUBLE);
  z.createdensearray(dims,matlabarray::miDOUBLE);

  // We use temp buffers to store all the values before committing them to the Matlab
  // classes, this takes up more memory, but should decrease the number of actual function
  // calls, which should be boost performance
  std::vector<double> xbuffer(numnodes);
  std::vector<double> ybuffer(numnodes);
  std::vector<double> zbuffer(numnodes);

  Point P;
  unsigned int r = 0;
  for (unsigned int p = 0; p < dim1 ; p++)
    for (unsigned int q = 0; q < dim2 ; q++)
      for (unsigned int s = 0; s < dim3 ; s++)
      {
        mesh->get_point(P,VMesh::Node::index_type(r));
        xbuffer[r] = P.x();
        ybuffer[r] = P.y();
        zbuffer[r] = P.z();
        r++;
      }

  x.setnumericarray(xbuffer);
  y.setnumericarray(ybuffer);
  z.setnumericarray(zbuffer);

  mlarray.setfield(0,"x",x);
  mlarray.setfield(0,"y",y);
  mlarray.setfield(0,"z",z);

  return (true);
}


bool
FieldToMatlabAlgo::mladdfieldheader(const FieldInformation& fi,  matlabarray& mlarray)
{
  matlabarray mlfieldbasis;
  mlfieldbasis.createstringarray(fi.get_basis_type());
  mlarray.setfield(0,"fieldbasis",mlfieldbasis);

  matlabarray mlfieldbasisorder;
  mlfieldbasisorder.createdoublescalar(static_cast<double>(fi.field_basis_order()));
  mlarray.setfield(0,"fieldbasisorder",mlfieldbasisorder);

  matlabarray mlfieldtype;
  mlfieldtype.createstringarray(fi.get_data_type());
  mlarray.setfield(0,"fieldtype",mlfieldtype);

  return(true);
}

////////////////////////////////////////


bool
FieldToMatlabAlgo::mladdfielddata(VField* field, VMesh* /*mesh*/, matlabarray& mlarray)
{
  matlabarray mlfield;

  if (field->is_nodata())
  {
    return (true);
  }

  VMesh::size_type size = field->num_values();

  if (field->is_char())
  {
    char dummy; matlabarray::mitype type = mlfield.getmitype(dummy);
    std::vector<char> fdata;
    field->get_values(fdata);
    mlfield.createdensearray(1,static_cast<int>(size),type);
    mlfield.setnumericarray(fdata);
    mlarray.setfield(0,"field",mlfield);
    return (true);
  }

  if (field->is_unsigned_char())
  {
    unsigned char dummy; matlabarray::mitype type = mlfield.getmitype(dummy);
    std::vector<unsigned char> fdata;
    field->get_values(fdata);
    mlfield.createdensearray(1,static_cast<int>(size),type);
    mlfield.setnumericarray(fdata);
    mlarray.setfield(0,"field",mlfield);
    return (true);
  }

  if (field->is_short())
  {
    short dummy; matlabarray::mitype type = mlfield.getmitype(dummy);
    std::vector<short> fdata;
    field->get_values(fdata);
    mlfield.createdensearray(1,static_cast<int>(size),type);
    mlfield.setnumericarray(fdata);
    mlarray.setfield(0,"field",mlfield);
    return (true);
  }

  if (field->is_unsigned_short())
  {
    unsigned short dummy; matlabarray::mitype type = mlfield.getmitype(dummy);
    std::vector<unsigned short> fdata;
    field->get_values(fdata);
    mlfield.createdensearray(1,static_cast<int>(size),type);
    mlfield.setnumericarray(fdata);
    mlarray.setfield(0,"field",mlfield);
    return (true);
  }

  if (field->is_int())
  {
    int dummy; matlabarray::mitype type = mlfield.getmitype(dummy);
    std::vector<int> fdata;
    field->get_values(fdata);
    mlfield.createdensearray(1,static_cast<int>(size),type);
    mlfield.setnumericarray(fdata);
    mlarray.setfield(0,"field",mlfield);
    return (true);
  }

  if (field->is_unsigned_int())
  {
    unsigned int dummy; matlabarray::mitype type = mlfield.getmitype(dummy);
    std::vector<unsigned int> fdata;
    field->get_values(fdata);
    mlfield.createdensearray(1,static_cast<int>(size),type);
    mlfield.setnumericarray(fdata);
    mlarray.setfield(0,"field",mlfield);
    return (true);
  }

  if (field->is_longlong())
  {
    long long dummy; matlabarray::mitype type = mlfield.getmitype(dummy);
    std::vector<long long> fdata;
    field->get_values(fdata);
    mlfield.createdensearray(1,static_cast<int>(size),type);
    mlfield.setnumericarray(fdata);
    mlarray.setfield(0,"field",mlfield);
    return (true);
  }

  if (field->is_unsigned_longlong())
  {
    unsigned long long dummy; matlabarray::mitype type = mlfield.getmitype(dummy);
    std::vector<unsigned long long> fdata;
    field->get_values(fdata);
    mlfield.createdensearray(1,static_cast<int>(size),type);
    mlfield.setnumericarray(fdata);
    mlarray.setfield(0,"field",mlfield);
    return (true);
  }

  if (field->is_float())
  {
    float dummy; matlabarray::mitype type = mlfield.getmitype(dummy);
    std::vector<float> fdata;
    field->get_values(fdata);
    mlfield.createdensearray(1,static_cast<int>(size),type);
    mlfield.setnumericarray(fdata);
    mlarray.setfield(0,"field",mlfield);
    return (true);
  }

  if (field->is_double())
  {
    double dummy; matlabarray::mitype type = mlfield.getmitype(dummy);
    std::vector<double> fdata;
    field->get_values(fdata);
    mlfield.createdensearray(1,static_cast<int>(size),type);
    mlfield.setnumericarray(fdata);
    mlarray.setfield(0,"field",mlfield);
    return (true);
  }

  if (field->is_vector())
  {
    mlfield.createdensearray(3,static_cast<int>(size),matlabarray::miDOUBLE);

    int p,q;
    std::vector<double> data(size*3);
    for (p = 0, q = 0; p < static_cast<int>(size); p++)
    {
      Vector v;
      field->get_value(v,VMesh::index_type(p));
      data[q++] = v.x();
      data[q++] = v.y();
      data[q++] = v.z();
    }
    mlfield.setnumericarray(data);
    mlarray.setfield(0,"field",mlfield);
    return(true);
  }

  if (field->is_tensor())
  {
    mlfield.createdensearray(9,static_cast<int>(size),matlabarray::miDOUBLE);

    int p,q;
    std::vector<double> data(size*9);
    for (p = 0, q = 0; p < static_cast<int>(size); p++)
    {
      Tensor v;
      field->get_value(v,VMesh::index_type(p));
      data[q++] = v.val(0,0);
      data[q++] = v.val(0,1);
      data[q++] = v.val(0,2);
      data[q++] = v.val(1,0);
      data[q++] = v.val(1,1);
      data[q++] = v.val(1,2);
      data[q++] = v.val(2,0);
      data[q++] = v.val(2,1);
      data[q++] = v.val(2,2);
    }
    mlfield.setnumericarray(data);
    mlarray.setfield(0,"field",mlfield);
    return(true);
  }
  return false;
}




///////////////////////////////////////////////////////////////////

bool
FieldToMatlabAlgo::mladdfieldedges(VField *field,VMesh *mesh, matlabarray& mlarray)
{
  matlabarray fieldedge;

  if (option_nofieldconnectivity_)
  {
    if ((field->basis_order()==mesh->basis_order())||field->is_nodata()||field->is_constantdata())
    {
      return (true);
    }
  }

  if (field->is_nodata())
  {
    return (true);
  }

  if (field->is_constantdata())
  {
    VMesh::size_type numvalues = field->num_values();
    fieldedge.createdensearray(1,static_cast<int>(numvalues),matlabarray::miUINT32);
    std::vector<unsigned int> mapping(numvalues);
    for (VMesh::index_type p = 0; p < numvalues; p++)
    {
      mapping[p] = static_cast<unsigned int>(p) + option_indexbase_;
    }
    fieldedge.setnumericarray(mapping);
    mlarray.setfield(0,"fieldedge",fieldedge);

    return (true);
  }

  if (field->is_lineardata()||field->is_cubicdata())
  {
    size_t num = mesh->num_nodes_per_edge();
    size_t numedges = mesh->num_edges();

    mesh->synchronize(Mesh::EDGES_E);

    VMesh::Node::array_type array;
    std::vector<VMesh::index_type> edges(num*numedges);
    std::vector<int> dims(2);
    dims[0] = static_cast<int>(num);
    dims[1] = static_cast<int>(numedges);
    fieldedge.createdensearray(dims,matlabarray::miUINT32);

    // SCIRun iterators are limited in supporting any index management
    // Hence I prefer to do it with integer and convert to the required
    // class at the last moment. Hopefully the compiler is smart and
    // has a fast translation.
     VMesh::Edge::iterator it, it_end;
    mesh->begin(it);
    mesh->end(it_end);
    size_t q = 0;

    while (it != it_end)
    {
      mesh->get_nodes(array,*(it));
      for (size_t r = 0; r < num; r++) edges[q++] = static_cast<VMesh::index_type>(array[r]) + option_indexbase_;
      ++it;
    }

    fieldedge.setnumericarray(edges);
    mlarray.setfield(0,"fieldedge",fieldedge);
    return (true);
  }

  if (field->is_quadraticdata())
  {
    error("FieldToMatlab: Cannot access the data in the field properly, hence cannot retrieve the data");
    return (false);
  }

  return (false);
}


bool
FieldToMatlabAlgo::mladdfieldfaces(VField *field, VMesh *mesh, matlabarray& mlarray)
{
  matlabarray fieldface;

  if (option_nofieldconnectivity_)
  {
    if ((field->basis_order()==mesh->basis_order())||field->is_nodata()||field->is_constantdata())
    {
      return (true);
    }
  }

  if (field->is_nodata())
  {
    return (true);
  }

  if (field->is_constantdata())
  {
    size_t size = field->num_values();
    fieldface.createdensearray(1,static_cast<int>(size),matlabarray::miUINT32);
    std::vector<unsigned int> mapping(size);
    for (size_t p = 0; p < size; p++)
    {
      mapping[p] = static_cast<unsigned int>(p) + option_indexbase_;
    }
    fieldface.setnumericarray(mapping);
    mlarray.setfield(0,"fieldface",fieldface);

    return (true);
  }

  if (field->is_lineardata()||field->is_cubicdata())
  {
    size_t num = mesh->num_nodes_per_face();
    size_t numfaces = mesh->num_faces();

    mesh->synchronize(Mesh::FACES_E);

    VMesh::Node::array_type array;
    std::vector<VMesh::index_type> faces(num*numfaces);
    std::vector<int> dims(2);
    dims[0] = static_cast<int>(num);
    dims[1] = static_cast<int>(numfaces);
    fieldface.createdensearray(dims,matlabarray::miUINT32);

    VMesh::Face::iterator it, it_end;
    mesh->begin(it);
    mesh->end(it_end);
    size_t q = 0;

    while (it != it_end)
    {
      mesh->get_nodes(array,*(it));
      for (size_t r = 0; r < num; r++) faces[q++] = static_cast<VMesh::index_type>(array[r]) + option_indexbase_;
      ++it;
    }

    fieldface.setnumericarray(faces);
    mlarray.setfield(0,"fieldface",fieldface);
    return (true);
  }

  if (field->is_quadraticdata())
  {
    error("FieldToMatlab: Cannot access the data in the field properly, hence cannot retrieve the data");
    return (false);
  }

  error("FieldToMatlab: Unknow basis type");
  return (false);
}


bool
FieldToMatlabAlgo::mladdfieldcells(VField *field,VMesh *mesh, matlabarray& mlarray)
{
  matlabarray fieldcell;

  if (option_nofieldconnectivity_)
  {
    if ((field->basis_order()==mesh->basis_order())||field->is_nodata()||field->is_constantdata())
    {
      return (true);
    }
  }

  if (field->is_nodata())
  {
    return (true);
  }

  if (field->is_constantdata())
  {
    size_t size = field->num_values();
    fieldcell.createdensearray(1,static_cast<int>(size),matlabarray::miUINT32);
    std::vector<unsigned int> mapping(size);
    for (size_t p = 0; p < size; p++)
    {
      mapping[p] = static_cast<unsigned int>(p) + option_indexbase_;
    }
    fieldcell.setnumericarray(mapping);
    mlarray.setfield(0,"fieldcell",fieldcell);

    return (true);
  }

  if (field->is_lineardata()||field->is_cubicdata())
  {
    size_t num = mesh->num_nodes_per_elem();
    size_t numcells = mesh->num_elems();

    mesh->synchronize(Mesh::CELLS_E);

    VMesh::Node::array_type array;
    std::vector<VMesh::index_type> cells(num*numcells);
    std::vector<int> dims(2);
    dims[0] = static_cast<int>(num); dims[1] = static_cast<int>(numcells);
    fieldcell.createdensearray(dims,matlabarray::miUINT32);

    // SCIRun iterators are limited in supporting any index management
    // Hence I prefer to do it with integer and convert to the required
    // class at the last moment. Hopefully the compiler is smart and
    // has a fast translation.
    VMesh::Cell::iterator it, it_end;
    mesh->begin(it);
    mesh->end(it_end);
    size_t q = 0;

    while (it != it_end)
    {
      mesh->get_nodes(array,*(it));
      for (size_t r = 0; r < num; r++) cells[q++] = static_cast<VMesh::index_type>(array[r]) + option_indexbase_;
      ++it;
    }

    fieldcell.setnumericarray(cells);
    mlarray.setfield(0,"fieldcell",fieldcell);
    return (true);
  }

  if (field->is_quadraticdata())
  {
    error("FieldToMatlab: Cannot access the data in the field properly, hence cannot retrieve the data");
    return (false);
  }

  error("FieldToMatlab: Unknown basis type");
  return (false);
}

bool FieldToMatlabAlgo::mladdfieldedgederivatives(VField *field, VMesh* /*mesh*/, matlabarray& /*mlarray*/)
{
  if (field->is_cubicdata())
  {
    error("FieldToMatlab: Cannot access the data in the field properly, hence cannot retrieve the data");
    return (false);
  }
  return (true);
}


bool FieldToMatlabAlgo::mladdfieldfacederivatives(VField *field, VMesh* /*mesh*/, matlabarray& /*mlarray*/)
{
  if (field->is_cubicdata())
  {
    error("FieldToMatlab: Cannot access the data in the field properly, hence cannot retrieve the data");
    return (false);
  }
  return (true);
}


bool FieldToMatlabAlgo::mladdfieldcellderivatives(VField *field, VMesh* /*mesh*/, matlabarray& /*mlarray*/)
{
  if (field->is_cubicdata())
  {
    error("FieldToMatlab: Cannot access the data in the field properly, hence cannot retrieve the data");
    return (false);
  }
  return (true);
}


bool FieldToMatlabAlgo::mladdfieldderivatives(VField *field, VMesh* /*mesh*/, matlabarray& /*mlarray*/)
{
  if (field->is_cubicdata())
  {
    error("FieldToMatlab: Cannot access the data in the field properly, hence cannot retrieve the data");
    return (false);
  }
  return (true);
}
