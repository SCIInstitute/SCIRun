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
 * DATE: 17 OCT 2004
 */

#include <Core/Matlab/matlabtofield.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Logging/LoggerInterface.h>
#include <Core/Matlab/matlabconverter.h>

using namespace SCIRun;
using namespace SCIRun::MatlabIO;
using namespace SCIRun::Core::Logging;
using namespace SCIRun::Core::Geometry;


bool MatlabToFieldAlgo::execute(FieldHandle& field)
{
  FieldInformation fi(meshtype,meshbasis,fieldbasis,fieldtype);

  field = CreateField(fi);

  if (!field)
  {
    error("MatlobToField: Could not allocate output field");
    return (false);
  }

  VField* vfield = field->vfield();
  VMesh*  vmesh  = field->vmesh();

  if (!vfield || !vmesh)
  {
    error("MatlobToField: The destination field type does not have a virtual interface");
    return (false);
  }

  if (vmesh->is_pointcloudmesh())
  {
    if (!(addnodes(vmesh))) return(false);
    if (!(addderivatives(vmesh))) return (false);
    if (!(addscalefactors(vmesh))) return (false);
  }

  if (vmesh->is_curvemesh())
  {
    if(!(addnodes(vmesh))) return (false);
    if(!(addedges(vmesh))) return (false);
    if(!(addderivatives(vmesh))) return (false);
    if(!(addscalefactors(vmesh))) return (false);
  }

  if (vmesh->is_trisurfmesh())
  {
    if(!(addnodes(vmesh))) return (false);
    if(!(addfaces(vmesh))) return (false);
    if(!(addderivatives(vmesh))) return (false);
    if(!(addscalefactors(vmesh))) return (false);
  }

  if (vmesh->is_quadsurfmesh())
  {
    if(!(addnodes(vmesh))) return (false);
    if(!(addfaces(vmesh))) return (false);
    if(!(addderivatives(vmesh))) return (false);
    if(!(addscalefactors(vmesh))) return (false);
  }

  if (vmesh->is_tetvolmesh())
  {
    if(!(addnodes(vmesh))) return (false);
    if(!(addcells(vmesh))) return (false);
    if(!(addderivatives(vmesh))) return (false);
    if(!(addscalefactors(vmesh))) return (false);
  }

  if (vmesh->is_prismvolmesh())
  {
    if(!(addnodes(vmesh))) return (false);
    if(!(addcells(vmesh))) return (false);
    if(!(addderivatives(vmesh))) return (false);
    if(!(addscalefactors(vmesh))) return (false);
  }

  if (vmesh->is_hexvolmesh())
  {
    if(!(addnodes(vmesh))) return (false);
    if(!(addcells(vmesh))) return (false);
    if(!(addderivatives(vmesh))) return (false);
    if(!(addscalefactors(vmesh))) return (false);
  }

  if (vmesh->is_scanlinemesh())
  {
    if (!(mldims.isdense())) return (false);
    if (mldims.getnumelements() != 1) return (false);

    std::vector<int> dims;
    mldims.getnumericarray(dims);

    Point PointO(0.0,0.0,0.0);
    Point PointP(static_cast<double>(dims[0]),0.0,0.0);

    field.reset();
    MeshHandle handle = CreateMesh(fi,static_cast<unsigned int>(dims[0]),PointO,PointP);
    if (!handle)
    {
      error("MatlabToField: Could not allocate output field");
      return (false);
    }
    field = CreateField(fi,handle);
    if (!field)
    {
      error("MatlabToField: Could not allocate output field");
      return (false);
    }

    vmesh = field->vmesh();
    vfield = field->vfield();
    addtransform(vmesh);
  }

  if (vmesh->is_imagemesh())
  {
    if (!(mldims.isdense())) return (false);
    if (mldims.getnumelements() != 2) return (false);

    std::vector<int> dims;
    mldims.getnumericarray(dims);

    Point PointO(0.0,0.0,0.0);
    Point PointP(static_cast<double>(dims[0]),static_cast<double>(dims[1]),0.0);

    field.reset();
    MeshHandle handle = CreateMesh(fi,static_cast<unsigned int>(dims[0]),static_cast<unsigned int>(dims[1]),PointO,PointP);
    if (!handle)
    {
      error("MatlabToField: Could not allocate output field");
      return (false);
    }
    field = CreateField(fi,handle);
    if (!field)
    {
      error("MatlabToField: Could not allocate output field");
      return (false);
    }

    vmesh = field->vmesh();
    vfield = field->vfield();
    addtransform(vmesh);
  }

  if (vmesh->is_latvolmesh())
  {
    if (!(mldims.isdense())) return (false);
    if (mldims.getnumelements() != 3) return (false);

    std::vector<int> dims;
    mldims.getnumericarray(dims);

    Point PointO(0.0,0.0,0.0);
    Point PointP(static_cast<double>(dims[0]),static_cast<double>(dims[1]),static_cast<double>(dims[2]));

    field.reset();
    MeshHandle handle = CreateMesh(fi,static_cast<unsigned int>(dims[0]),static_cast<unsigned int>(dims[1]),static_cast<unsigned int>(dims[2]),PointO,PointP);
    if (!handle)
    {
      error("MatlabToField: Could not allocate output field");
      return (false);
    }
    field = CreateField(fi,handle);
    if (!field)
    {
      error("MatlabToField: Could not allocate output field");
      return (false);
    }

    vmesh = field->vmesh();
    vfield = field->vfield();
    addtransform(vmesh);
  }

  if (vmesh->is_structcurvemesh())
  {
    std::vector<int> dims;
    std::vector<unsigned int> mdims;
    const int numdim = mlx.getnumdims();
    dims = mlx.getdims();

    mdims.resize(numdim);
    for (int p=0; p < numdim; p++)  mdims[p] = static_cast<unsigned int>(dims[p]);

    if ((numdim == 2)&&(mlx.getn() == 1))
    {
      mdims.resize(1);
      mdims[0] = mlx.getm();
    }

    field.reset();
    MeshHandle handle = CreateMesh(fi,static_cast<unsigned int>(dims[0]));
    if (!handle)
    {
      error("MatlabToField: Could not allocate output field");
      return (false);
    }
    field = CreateField(fi,handle);
    if (!field)
    {
      error("MatlabToField: Could not allocate output field");
      return (false);
    }

    vmesh = field->vmesh();
    vfield = field->vfield();

    int numnodes = mlx.getnumelements();

    std::vector<double> X;
    std::vector<double> Y;
    std::vector<double> Z;
    mlx.getnumericarray(X);
    mly.getnumericarray(Y);
    mlz.getnumericarray(Z);

    for (VMesh::size_type p = 0; p < numnodes; p++)
    {
      vmesh->set_point(Point(X[p],Y[p],Z[p]),VMesh::Node::index_type(p));
    }
  }
  if (vmesh->is_structquadsurfmesh())
  {
    std::vector<int> dims;
    std::vector<unsigned int> mdims;
    int numdim = mlx.getnumdims();
    dims = mlx.getdims();

    mdims.resize(numdim);
    for (int p=0; p < numdim; p++)  mdims[p] = static_cast<unsigned int>(dims[p]);

    field.reset();
    MeshHandle handle = CreateMesh(fi,static_cast<unsigned int>(dims[0]),static_cast<unsigned int>(dims[1]));
    if (!handle)
    {
      error("MatlabToField: Could not allocate output field");
      return (false);
    }
    field = CreateField(fi,handle);
    if (!field)
    {
      error("MatlabToField: Could not allocate output field");
      return (false);
    }

    vmesh = field->vmesh();
    vfield = field->vfield();

    std::vector<double> X;
    std::vector<double> Y;
    std::vector<double> Z;
    mlx.getnumericarray(X);
    mly.getnumericarray(Y);
    mlz.getnumericarray(Z);

    unsigned m = mdims[0]*mdims[1];
    for (unsigned int p = 0; p < m; p++)
    {
      vmesh->set_point(Point(X[p],Y[p],Z[p]),VMesh::Node::index_type(p));
    }
  }

  if (vmesh->is_structhexvolmesh())
  {
    std::vector<int> dims;
    std::vector<unsigned int> mdims;
    int numdim = mlx.getnumdims();
    dims = mlx.getdims();

    mdims.resize(numdim);
    for (int p=0; p < numdim; p++)  mdims[p] = static_cast<unsigned int>(dims[p]);

    std::vector<double> X;
    std::vector<double> Y;
    std::vector<double> Z;
    mlx.getnumericarray(X);
    mly.getnumericarray(Y);
    mlz.getnumericarray(Z);

    field.reset();
    MeshHandle handle = CreateMesh(fi,static_cast<unsigned int>(dims[0]),static_cast<unsigned int>(dims[1]),static_cast<unsigned int>(dims[2]));
    if (!handle)
    {
      error("MatlabToField: Could not allocate output field");
      return (false);
    }
    field = CreateField(fi,handle);
    if (!field)
    {
      error("MatlabToField: Could not allocate output field");
      return (false);
    }

    vmesh = field->vmesh();
    vfield = field->vfield();

    unsigned int m = mdims[0]*mdims[1]*mdims[2];
    for (unsigned int p = 0; p < m; p++)
    {
      vmesh->set_point(Point(X[p],Y[p],Z[p]),VMesh::Node::index_type(p));
    }
  }

  vfield->resize_values();

  if (vfield->basis_order() == 0)
  {
    if (!(addfield(vfield)))
    {
      error("The conversion of the field data failed");
      return (false);
    }
  }

  if (vfield->basis_order() == 1)
  {
    vfield->resize_fdata();
    if (!(addfield(vfield)))
    {
      error("The conversion of the field data failed");
      return (false);
    }
  }

  if (vfield->basis_order() == 2)
  {
    error("There is no converter available for quadratic field data");
    return (false);
  }

  if (vfield->basis_order() == 3)
  {
    error("There is no converter available for cubic field data");
    return (false);
  }

  return (true);
}


int MatlabToFieldAlgo::analyze_iscompatible(const matlabarray& mlarray, std::string& infotext, bool postremark)
{
  infotext = "";

  int ret;
  try
  {
    ret = mlanalyze(mlarray, postremark);
    if (0 == ret)
      return ret;
  }
  catch (matlabconverter::error_type& e)
  {
    std::cerr << "analyze_fieldtype error: " << e.what() << std::endl;
    return 0;
  }

  std::ostringstream oss;
  std::string name = mlarray.getname();
  oss << name << " ";
  if (name.length() < 20) oss << std::string(20-(name.length()),' '); // add some form of spacing

  oss << "[" << meshtype << "<" << meshbasis << "> - ";
  if (fieldbasistype != "nodata")
  {
    std::string fieldtypestr = "Scalar";
    if (fieldtype == "Vector") fieldtypestr = "Vector";
    if (fieldtype == "Tensor") fieldtypestr = "Tensor";
    oss << fieldtypestr << "<" << fieldbasis << "> - ";
  }
  else
  {
    oss << "NoData - ";
  }

  if (numnodesvec.size() > 0)
  {
    for (size_t p = 0; p < numnodesvec.size()-1; p++) oss << numnodesvec[p] << "x";
    oss << numnodesvec[numnodesvec.size()-1];
    oss << " NODES";
  }
  else
  {
    oss << numnodes << " NODES " << numelements << " ELEMENTS";
  }
  oss << "]";
  infotext = oss.str();

  return ret;
}

int MatlabToFieldAlgo::analyze_fieldtype(const matlabarray& mlarray, std::string& fielddesc)
{
  fielddesc = "";

  int ret;
  try
  {
    ret = mlanalyze(mlarray, false);
    if (0 == ret)
      return ret;
  }
  catch (matlabconverter::error_type& e)
  {
    std::cerr << "analyze_fieldtype error: " << e.what() << std::endl;
    return 0;
  }

  if (fieldtype == "") fieldtype = "double";
  if (fieldtype == "nodata") fieldtype = "double";

  fielddesc = "GenericField<"+meshtype+"<"+meshbasis+"<Point> >,"+
              fieldbasis+"<"+fieldtype+">,"+fdatatype+"<"+fieldtype;

  // DEAL WITH SOME MORE SCIRUN INCONSISTENCIES
  if (fdatatype == "FData2d") fielddesc += "," + meshtype + "<" + meshbasis + "<Point> > ";
  if (fdatatype == "FData3d") fielddesc += "," + meshtype + "<" + meshbasis + "<Point> > ";

  fielddesc += "> > ";
  return 1;
}

matlabarray MatlabToFieldAlgo::findfield(const matlabarray& mlarray,const std::string& fieldnamesIn)
{
  matlabarray subarray;
  std::string fieldnames(fieldnamesIn);
  while (true)
  {
    size_t loc = fieldnames.find(';');
    if (loc > fieldnames.size()) break;
    std::string fieldname = fieldnames.substr(0,loc);
    fieldnames = fieldnames.substr(loc+1);

    int index = mlarray.getfieldnameindexCI(fieldname);
    if (index > -1)
    {
      subarray = mlarray.getfield(0,index);
      break;
    }
  }

  return(subarray);
}

void MatlabToFieldAlgo::remarkAndThrow(const std::string& msg, bool postremark) const
{
  if (postremark) remark(msg);
  throw matlabconverter::error_type(msg);
}

int MatlabToFieldAlgo::mlanalyze(matlabarray mlarray, bool postremark)
{
  int ret = 1;

  if (mlarray.isempty())
  {
    remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into SCIRun Field (matrix is empty)", postremark);
  }

  if (mlarray.getnumelements() == 0)
  {
    remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into SCIRun Field (matrix is empty)", postremark);
  }
  // If it is regular matrix translate it to a image or a latvol
  // The following section of code rewrites the Matlab matrix into a
  // structure and then the normal routine picks up the field and translates it
  // properly.

  if (mlarray.isdense())
  {
    int numdims = mlarray.getnumdims();
    if ((numdims >0)&&(numdims < 4))
    {
      matlabarray ml;
      matlabarray dimsarray;
      std::vector<int> d = mlarray.getdims();
      if ((d[0]==1)||(d[1]==1))
      {
        if (d[0]==1) d[0] = d[1];
        int temp = d[0];
        d.resize(1);
        d[0] = temp;
      }
      dimsarray.createintvector(d);
      ml.createstructarray();
      ml.setfield(0,"dims",dimsarray);
      ml.setfield(0,"field",mlarray);
      ml.setname(mlarray.getname());
      mlarray = ml;
    }
    else if (numdims == 4)
    {
      matlabarray ml;
      matlabarray dimsarray;
      matlabarray mltype;
      std::vector<int> d = mlarray.getdims();

      if ((d[0] == 1)||(d[0] == 3)||(d[0] == 6)||(d[0] == 9))
      {
        std::vector<int> dm(3);
        for (size_t p = 0; p < 3; p++) dm[p] = d[p+1];
        dimsarray.createintvector(dm);
        if (d[0] == 1) mltype.createstringarray("double");
        else if (d[0] == 3) mltype.createstringarray("Vector");
        else if ((d[0] == 6) || (d[0] == 9)) mltype.createstringarray("Tensor");
        ml.createstructarray();
        ml.setfield(0,"dims",dimsarray);
        ml.setfield(0,"field",mlarray);
        ml.setfield(0,"fieldtype",mltype);
        ml.setname(mlarray.getname());
        mlarray = ml;
      }
      else if ((d[3] == 1)||(d[3] == 3)||(d[3] == 6)||(d[3] == 9))
      {
        std::vector<int> dm(3);
        for (size_t p = 0; p < 3; p++) dm[p] = d[p];
        dimsarray.createintvector(dm);
        if (d[3] == 1) mltype.createstringarray("double");
        else if (d[3] == 3) mltype.createstringarray("Vector");
        else if ((d[3] == 6) || (d[3] == 9)) mltype.createstringarray("Tensor");
        ml.createstructarray();
        ml.setfield(0,"dims",dimsarray);
        ml.setfield(0,"field",mlarray);
        ml.setfield(0,"fieldtype",mltype);
        ml.setname(mlarray.getname());
        mlarray = ml;
      }
    }

    // If the matrix is dense, we score less as we could translate it into a
    // matrix as well. This help for bundle conversions, by which we can
    // automatically determine how to translate objects.
    ret = 0;
  }

  // Check whether we have a structured matrix
  if (!(mlarray.isstruct()))
  {
    remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into SCIRun Field (matrix is not a structured matrix)", postremark);
  }


  // Get all the matrices that specify the mesh
  mlnode = findfield(mlarray,"node;pts;pos;");
  mlx =    findfield(mlarray,"x;");
  mly =    findfield(mlarray,"y;");
  mlz =    findfield(mlarray,"z;");

  mledge = findfield(mlarray,"meshedge;edge;line;");
  mlface = findfield(mlarray,"meshface;face;quad;fac;tri;");
  mlcell = findfield(mlarray,"meshcell;cell;prism;hex;tet;");
  mldims = findfield(mlarray,"dims;dim;dimension;");
  mltransform = findfield(mlarray,"transform;");

  mlmeshderivatives  = findfield(mlarray,"meshderivatives;derivatives;");
  mlmeshscalefactors = findfield(mlarray,"meshscalefactors;scalefactors;");

  mlmeshtype =       findfield(mlarray,"elemtype;meshclass;meshtype;");
  mlmeshbasis =      findfield(mlarray,"meshbasis;basis;");
  mlmeshbasisorder = findfield(mlarray,"meshbasisorder;meshat;meshlocation;");

  // Get all the matrices that specify the field
  mlfield     = findfield(mlarray,"field;scalarfield;scalardata;potvals;data;");
  mlfieldtype =       findfield(mlarray,"fieldtype;datatype;");

  // Make it compatible with some old versions
  matlabarray mlfieldvector = findfield(mlarray,"vectorfield;vectordata;");
  if (mlfieldvector.isdense())
  {
    mlfield = mlfieldvector; mlfieldtype.createstringarray("vector");
  }
  matlabarray mlfieldtensor = findfield(mlarray,"tensorfield;tensordata;");
  if (mlfieldtensor.isdense())
  {
    mlfield = mlfieldtensor; mlfieldtype.createstringarray("tensor");
  }

  //mlfieldedge = findfield(mlarray,"fieldedge;edge;line;");

  mlfieldderivatives  = findfield(mlarray,"fieldderivatives;derivatives;");
  mlfieldscalefactors = findfield(mlarray,"fieldscalefactors;scalefactors;");

  mlfieldbasis =      findfield(mlarray,"fieldbasis;basis;");
  mlfieldbasisorder = findfield(mlarray,"fieldbasisorder;basisorder;fieldat;fieldlocation;dataat;");

  mlchannels        = findfield(mlarray,"channels;");

  // Figure out the basis type
  // Since we went through several systems over the time
  // the next code is all for compatibility.

  if (!(mlmeshbasisorder.isempty()))
  {   // converter table for the string in the mesh array
    if (mlmeshbasisorder.isstring())
    {
      if ((mlmeshbasisorder.compareCI("node"))||(mlmeshbasisorder.compareCI("pts")))
      {
        mlmeshbasisorder.createdoublescalar(1.0);
      }
      else if (mlmeshbasisorder.compareCI("none")||
               mlmeshbasisorder.compareCI("nodata"))
      {
        mlmeshbasisorder.createdoublescalar(-1.0);
      }
      else if (mlmeshbasisorder.compareCI("egde")||
          mlmeshbasisorder.compareCI("line")||mlmeshbasisorder.compareCI("face")||
          mlmeshbasisorder.compareCI("fac")||mlmeshbasisorder.compareCI("cell")||
          mlmeshbasisorder.compareCI("tet")||mlmeshbasisorder.compareCI("hex")||
          mlmeshbasisorder.compareCI("prism"))
      {
        mlmeshbasisorder.createdoublescalar(0.0);
      }
    }
  }

  if (mlmeshbasis.isstring())
  {
    std::string str = mlmeshbasis.getstring();
    for (size_t p = 0; p < str.size(); p++) str[p] = tolower(str[p]);

    if      (str.find("nodata") != std::string::npos)    meshbasistype = "nodata";
    else if (str.find("constant") != std::string::npos)  meshbasistype = "constant";
    else if (str.find("linear") != std::string::npos)    meshbasistype = "linear";
    else if (str.find("quadratic") != std::string::npos) meshbasistype = "quadratic";
    else if (str.find("cubic") != std::string::npos)     meshbasistype = "cubic";
  }

  if ((meshbasistype == "")&&(mlmeshbasisorder.isdense()))
  {
    std::vector<int> data;
    mlmeshbasisorder.getnumericarray(data);

    if (data.size() > 0)
    {
      if (data[0] == -1) meshbasistype = "nodata";
      else if (data[0] ==  0) meshbasistype = "constant";
      else if (data[0] ==  1) meshbasistype = "linear";
      else if (data[0] ==  2) meshbasistype = "quadratic";
    }
  }

  // figure out the basis of the field

  if (!(mlfieldbasisorder.isempty()))
  {   // converter table for the string in the field array
    if (mlfieldbasisorder.isstring())
    {
      if (mlfieldbasisorder.compareCI("node")||mlfieldbasisorder.compareCI("pts"))
      {
        mlfieldbasisorder.createdoublescalar(1.0);
      }
      else if (mlfieldbasisorder.compareCI("none")||
               mlfieldbasisorder.compareCI("nodata"))
      {
        mlfieldbasisorder.createdoublescalar(-1.0);
      }
      else if (mlfieldbasisorder.compareCI("egde")||
          mlfieldbasisorder.compareCI("line")||mlfieldbasisorder.compareCI("face")||
          mlfieldbasisorder.compareCI("fac")||mlfieldbasisorder.compareCI("cell")||
          mlfieldbasisorder.compareCI("tet")||mlfieldbasisorder.compareCI("hex")||
          mlfieldbasisorder.compareCI("prism"))
      {
        mlfieldbasisorder.createdoublescalar(0.0);
      }
    }
  }

  if (mlfieldbasis.isstring())
  {
    std::string str = mlfieldbasis.getstring();
    for (size_t p = 0; p < str.size(); p++) str[p] = tolower(str[p]);

    if (str.find("nodata") != std::string::npos)         fieldbasistype = "nodata";
    else if (str.find("constant") != std::string::npos)  fieldbasistype = "constant";
    else if (str.find("linear") != std::string::npos)    fieldbasistype = "linear";
    else if (str.find("quadratic") != std::string::npos) fieldbasistype = "quadratic";
    else if (str.find("cubic") != std::string::npos)     fieldbasistype = "cubic";
  }

  if ((fieldbasistype == "")&&(mlfieldbasisorder.isdense()))
  {
    std::vector<int> data;
    mlfieldbasisorder.getnumericarray(data);
    if (data.size() > 0)
    {
      if (data[0] == -1) fieldbasistype = "nodata";
      else if (data[0] ==  0) fieldbasistype = "constant";
      else if (data[0] ==  1) fieldbasistype = "linear";
      else if (data[0] ==  2) fieldbasistype = "quadratic";
    }
  }

  // Figure out the fieldtype

  fieldtype = "";
  if (mlfield.isdense())
  {
    if (mlfieldtype.isstring())
    {
      if (mlfieldtype.compareCI("nodata"))             fieldtype = "nodata";
      else if (mlfieldtype.compareCI("vector"))             fieldtype = "Vector";
      else if (mlfieldtype.compareCI("tensor"))             fieldtype = "Tensor";
      else if (mlfieldtype.compareCI("double"))             fieldtype = "double";
      else if (mlfieldtype.compareCI("float"))              fieldtype = "float";
      else if (mlfieldtype.compareCI("long long"))          fieldtype = "long long";
      else if (mlfieldtype.compareCI("unsigned long long")) fieldtype = "unsigned long long";
      else if (mlfieldtype.compareCI("long"))               fieldtype = "long";
      else if (mlfieldtype.compareCI("unsigned long"))      fieldtype = "unsigned long";
      else if (mlfieldtype.compareCI("int"))                fieldtype = "int";
      else if (mlfieldtype.compareCI("unsigned int"))       fieldtype = "unsigned int";
      else if (mlfieldtype.compareCI("short"))              fieldtype = "short";
      else if (mlfieldtype.compareCI("unsigned short"))     fieldtype = "unsigned short";
      else if (mlfieldtype.compareCI("char"))               fieldtype = "char";
      else if (mlfieldtype.compareCI("unsigned char"))      fieldtype = "unsigned char";
    }
  }

  if ((fieldtype == "nodata")||(mlfield.isempty()))
  {
    fieldbasis = "NoDataBasis";
    fieldbasistype = "nodata";
    fieldtype = "double";
  }

  // if no meshbasistype is supplied we need to figure it out on the fly

  // Now figure out the mesh type and check we have everything for that meshtype

  meshtype = "";
  if (mlmeshtype.isstring())
  {
    std::string str = mlmeshtype.getstring();
    for (size_t p = 0; p < str.size(); p++) str[p] = tolower(str[p]);

    if (str.find("pointcloud") != std::string::npos)          meshtype = "PointCloudMesh";
    else if (str.find("scanline")       != std::string::npos) meshtype = "ScanlineMesh";
    else if (str.find("image")          != std::string::npos) meshtype = "ImageMesh";
    else if (str.find("latvol")         != std::string::npos) meshtype = "LatVolMesh";
    else if (str.find("structcurve")    != std::string::npos) meshtype = "StructCurveMesh";
    else if (str.find("structquadsurf") != std::string::npos) meshtype = "StructQuadSurfMesh";
    else if (str.find("structhexvol")   != std::string::npos) meshtype = "StructHexVolMesh";
    else if (str.find("curve")          != std::string::npos) meshtype = "CurveMesh";
    else if (str.find("trisurf")        != std::string::npos) meshtype = "TriSurfMesh";
    else if (str.find("quadsurf")       != std::string::npos) meshtype = "QuadSurfMesh";
    else if (str.find("tetvol")         != std::string::npos) meshtype = "TetVolMesh";
    else if (str.find("prismvol")       != std::string::npos) meshtype = "PrismVolMesh";
    else if (str.find("hexvol")         != std::string::npos) meshtype = "HexVolMesh";
  }

  fdatatype = "vector";
  numnodes = 0;
  numelements = 0;
  numnodesvec.clear();
  numelementsvec.clear();

  if (mltransform.isdense())
  {
    if (mltransform.getnumdims() != 2)
    {
      remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (transformation matrix is not 2D)", postremark);
    }
    if ((mltransform.getn() != 4)&&(mltransform.getm() != 4))
    {
      remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (transformation matrix is not 4x4)", postremark);
    }
  }

  if (mlx.isdense()||mly.isdense()||mly.isdense())
  {
    if (mlx.isempty()||mly.isempty()||mlz.isempty())
    {
      remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (does not have a complete set of  x, y, or z coordinates", postremark);
    }
  }

  // FIGURE OUT THE REGULAR MESH STUFF
  // CHECK WHETHER IT IS ONE, AND IF SO CHECK ALL THE DATA

  if (((mlnode.isempty()&&(mlx.isempty()))||
       (mltransform.isdense())||(meshtype  == "ScanlineMesh")||
       (meshtype == "ImageMesh")||(meshtype == "LatVolMesh"))&&(mldims.isempty()))
  {
    if (mlfield.isdense())
    {
      std::vector<int> dims = mlfield.getdims();

      if ((fieldtype == "")&&(dims.size() > 3))
      {
        if (dims[0] == 3) fieldtype = "Vector";
        if ((dims[0] == 6)||(dims[0] == 9)) fieldtype = "Tensor";
      }

      if ((fieldtype == "Vector")||(fieldtype == "Tensor"))
      {
        if (fieldbasistype == "quadratic")
        {
          if (dims.size() > 2) mldims.createintvector(static_cast<int>((dims.size()-2)),&(dims[1]));
        }
        else
        {
          if (dims.size() > 1) mldims.createintvector(static_cast<int>((dims.size()-1)),&(dims[1]));
        }
      }
      else
      {
        if (fieldbasistype == "quadratic")
        {
          if (dims.size() > 1) mldims.createintvector(static_cast<int>((dims.size()-1)),&(dims[0]));
        }
        else
        {
          mldims.createintvector(dims);
        }
      }
    }

    if (fieldbasistype == "constant")
    {
      std::vector<int> dims = mlfield.getdims();
      // dimensions need to be one bigger
      for (int p = 0; p<static_cast<int>(dims.size()); p++) dims[p] = dims[p]+1;
      mldims.createintvector(dims);
    }
  }

  // CHECK WHETHER WE HAVE A REGULAR FIELD

  if ((mldims.isempty())&&(mlx.isempty())&&(mlnode.isempty()))
  {
    remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated because no node, x, or dims field was found", postremark);
  }

  // WE HAVE POSSIBLY A FIELD
  // NOW CHECK EVERY COMBINATION

  // HANDLE REGULAR MESHES

  numfield = 0;
  datasize = 1;

  if (mldims.isdense())
  {
    size_t size = static_cast<size_t>(mldims.getnumelements());

    if (!((size > 0)&&(size < 4)))
    {
      remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (number of dimensions (.dims field) needs to 1, 2, or 3)", postremark);
    }

    if (meshtype != "")
    {   // explicitly stated type: (check whether type confirms the guessed type, otherwise someone supplied us with improper data)
      if ((meshtype == "ScanlineMesh")&&(size!=1))
      {
        remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (scanline needs only one dimension)", postremark);
      }
      else if ((meshtype == "ImageMesh") && (size != 2))
      {
        remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (an image needs two dimensions)", postremark);
      }
      else if ((meshtype == "LatVolMesh") && (size != 3))
      {
        remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (a latvolmesh needs three dimensions)", postremark);
      }
    }
    else
    {
      if (size == 1) meshtype = "ScanlineMesh";
      else if (size == 2) meshtype = "ImageMesh";
      else if (size == 3) meshtype = "LatVolMesh";
    }

    // We always make this into a linear one
    if (meshbasistype == "") meshbasistype = "linear";

    if (meshbasistype != "linear")
    {
      remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (regular meshes cannot have higher order basis)", postremark);
    }

    if (meshtype == "ScanlineMesh") { meshbasis = "CrvLinearLgn";    fdatatype = "vector"; }
    else if (meshtype == "ImageMesh")    { meshbasis = "QuadBilinearLgn"; fdatatype = "FData2d"; }
    else if (meshtype == "LatVolMesh")   { meshbasis = "HexTrilinearLgn"; fdatatype = "FData3d"; }

    // compute number of elements and number of nodes
    mldims.getnumericarray(numnodesvec);
    numelementsvec = numnodesvec;
    // Number of elements is one less than the dimension in a certain direction
    for (size_t p = 0; p < numnodesvec.size(); p++)
    {
      numelementsvec[p]--;
    }

    // try to figure out the field basis
    if (fieldbasistype == "")
    {
      // In case no data is there
      if (mlfield.isempty())
      {
        fieldbasistype = "nodata";
        fieldtype = "";
      }
      else
      {
        if (fieldtype == "")
        {
          fieldtype = "double";
          auto type = mlfield.gettype();
          if (type == miINT8) fieldtype = "char";
          else if (type == miUINT8) fieldtype = "unsigned char";
          else if (type == miINT16) fieldtype = "short";
          else if (type == miUINT16) fieldtype = "unsigned short";
          else if (type == miINT32) fieldtype = "int";
          else if (type == miUINT32) fieldtype = "unsigned int";
          else if (type == miSINGLE) fieldtype = "float";
        }

        std::vector<int> fdims = mlfield.getdims();
        if (fieldtype == "Vector")
        {
          if (fdims[0] != 3)
          {
            remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (first dimension of field needs to be 3)", postremark);
          }
          std::vector<int> temp(fdims.size()-1);
          for (size_t p = 0; p < temp.size(); p++) temp[p] = fdims[p+1];
          fdims = temp;
        }
        else if (fieldtype == "Tensor")
        {
          if ((fdims[0] != 6)&&(fdims[0] != 9))
          {
            remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (first dimension of field needs to be 6, or 9)", postremark);
          }
          std::vector<int> temp(fdims.size()-1);
          for (size_t p = 0; p < temp.size(); p++) temp[p] = fdims[p+1];
          fdims = temp;
        }

        if ((size == 1)&&(size == fdims.size())&&(fdims[0] == numnodesvec[0])) fieldbasistype = "linear";
        else if ((size == 2) && (size == fdims.size()) && (fdims[0] == numnodesvec[0]) && (fdims[1] == numnodesvec[1])) fieldbasistype = "linear";
        else if ((size == 3) && (size == fdims.size()) && (fdims[0] == numnodesvec[0]) && (fdims[1] == numnodesvec[1]) && (fdims[2] == numnodesvec[2])) fieldbasistype = "linear";

        if ((size == 1)&&(size+1 == fdims.size())&&(fdims[0] == numnodesvec[0])&&(fdims[1] == 2)) fieldbasistype = "quadratic";
        else if ((size == 2) && (size + 1 == fdims.size()) && (fdims[0] == numnodesvec[0]) && (fdims[1] == numnodesvec[1]) && (fdims[2] == 3)) fieldbasistype = "quadratic";
        else if ((size == 3) && (size + 1 == fdims.size()) && (fdims[0] == numnodesvec[0]) && (fdims[1] == numnodesvec[1]) && (fdims[2] == numnodesvec[2]) && (fdims[3] == 4)) fieldbasistype = "quadratic";

        if ((size == 1)&&(size == fdims.size())&&(fdims[0] == numelementsvec[0])) fieldbasistype = "constant";
        else if ((size == 2) && (size == fdims.size()) && (fdims[0] == numelementsvec[0]) && (fdims[1] == numelementsvec[1])) fieldbasistype = "constant";
        else if ((size == 3) && (size == fdims.size()) && (fdims[0] == numelementsvec[0]) && (fdims[1] == numelementsvec[1]) && (fdims[2] == numelementsvec[2])) fieldbasistype = "constant";

        if ((mlfieldderivatives.isdense())&&(fieldbasis == "linear")) fieldbasistype = "cubic";
      }
    }
    //by now we should know what kind of basis we would like

    if (fieldbasistype == "")
    {
      remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (the dimensions field matrix does not match mesh)", postremark);
    }

    if (fieldbasistype == "nodata") fieldbasis = "NoDataBasis";
    else if (fieldbasistype == "constant") fieldbasis = "ConstantBasis";
    else if (fieldbasistype == "linear")
    {
      std::vector<int> fdims = mlfield.getdims();
      if (fieldtype == "Vector")
      {
        if (fdims[0] != 3)
        {
          remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (first dimension of field needs to be 3", postremark);
        }
        std::vector<int> temp(fdims.size()-1);
        for (size_t p = 0; p < temp.size(); p++) temp[p] = fdims[p+1];
        fdims = temp;
        datasize = fdims[0];
      }
      else if (fieldtype == "Tensor")
      {
        if ((fdims[0] != 6)&&(fdims[0] != 9))
        {
          remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (first dimension of field needs to be 6, or 9)", postremark);
        }
        std::vector<int> temp(fdims.size()-1);
        for (size_t p = 0; p < temp.size(); p++) temp[p] = fdims[p+1];
        fdims = temp;
        datasize = fdims[0];
      }

      if ((!((size == 1) && (size == fdims.size()) && (fdims[0] == numnodesvec[0]))) &&
        (!((size == 2) && (size == fdims.size()) && (fdims[0] == numnodesvec[0]) && (fdims[1] == numnodesvec[1]))) &&
        (!((meshtype == "LatVolMesh") && (size == 3) && (2 == fdims.size()) && (fdims[0] * fdims[1] == numnodesvec[0] * numnodesvec[1] * numnodesvec[2]))) &&
        (!((meshtype == "ImageMesh") && (size == 2) && (2 == fdims.size()) && (fdims[0] * fdims[1] == numnodesvec[0] * numnodesvec[1]))) &&
        (!((size == 3) && (size == fdims.size()) && (fdims[0] == numnodesvec[0]) && (fdims[1] == numnodesvec[1]) && (fdims[2] == numnodesvec[2]))))
      {
        remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (dimensions of field do not match dimensions of mesh", postremark);
      }
      if (meshtype == "ScanlineMesh") { fieldbasis = "CrvLinearLgn";}
      else if (meshtype == "ImageMesh")    { fieldbasis = "QuadBilinearLgn"; }
      else if (meshtype == "LatVolMesh")   { fieldbasis = "HexTrilinearLgn"; }
    }
    else if (fieldbasistype == "quadratic")
    {
      std::vector<int> fdims = mlfield.getdims();
      if (fieldtype == "Vector")
      {
        if (fdims[0] != 3)
        {
          remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (first dimension of field needs to be 3", postremark);
        }
        std::vector<int> temp(fdims.size()-1);
        for (size_t p = 0; p < temp.size(); p++) temp[p] = fdims[p+1];
        fdims = temp;
        datasize = fdims[0];
      }
      else if (fieldtype == "Tensor")
      {
        if ((fdims[0] != 6)&&(fdims[0] != 9))
        {
          remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (first dimension of field needs to be 6, or 9)", postremark);
        }
        std::vector<int> temp(fdims.size()-1);
        for (size_t p = 0; p < temp.size(); p++) temp[p] = fdims[p+1];
        fdims = temp;
        datasize = fdims[0];
      }

      if ((!((size == 1)&&(size+1 == fdims.size())&&(fdims[0] == numnodesvec[0])&&(fdims[1] == 2))) &&
          (!((size == 2)&&(size+1 == fdims.size())&&(fdims[0] == numnodesvec[0])&&(fdims[1] == numnodesvec[1])&&(fdims[2] == 3))) &&
          (!((size == 3)&&(size+1 == fdims.size())&&(fdims[0] == numnodesvec[0])&&(fdims[1] == numnodesvec[1])&&(fdims[2] == numnodesvec[2])&&(fdims[3] == 4))))
      {
        remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (dimensions of field do not match dimensions of mesh", postremark);
      }

      if (meshtype == "ScanlineMesh") { fieldbasis = "CrvQuadraticLgn";}
      else if (meshtype == "ImageMesh")    { fieldbasis = "QuadBiquadraticLgn"; }
      else if (meshtype == "LatVolMesh")   { fieldbasis = "HexTriquadraticLgn"; }
    }
    else if (fieldbasistype == "cubic")
    {
      std::vector<int> fdims = mlfield.getdims();
      if (fieldtype == "Vector")
      {
        if (fdims[0] != 3)
        {
          remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (first dimension of field needs to be 3", postremark);
        }
        std::vector<int> temp(fdims.size()-1);
        for (size_t p = 0; p < temp.size(); p++) temp[p] = fdims[p+1];
        fdims = temp;
        datasize = fdims[0];
      }
      else if (fieldtype == "Tensor")
      {
        if ((fdims[0] != 6)&&(fdims[0] != 9))
        {
          remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (first dimension of field needs to be 6, or 9)", postremark);
        }
        std::vector<int> temp(fdims.size()-1);
        for (size_t p = 0; p < temp.size(); p++) temp[p] = fdims[p+1];
        fdims = temp;
        datasize = fdims[0];
      }

      if (meshtype == "ScanlineMesh") { fieldbasis = "CrvCubicHmt";}
      else if (meshtype == "ImageMesh")    { fieldbasis = "QuadBicubicLgn"; }
      else if (meshtype == "LatVolMesh")   { fieldbasis = "HexTricubicLgn"; }

      if (mlfieldderivatives.isdense())
      {
        std::vector<int> derivativesdims = mlfieldderivatives.getdims();
        std::vector<int> fielddims = mlfieldderivatives.getdims();

        if (meshtype == "ScanlineMesh")
        {
          if (derivativesdims.size() != size+2)
          {
            remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field, the derivative matrix should have two more dimensions then the field matrix", postremark);
          }

          if ((derivativesdims[0] != 1)||(derivativesdims[1] != datasize)||(derivativesdims[2] != fielddims[0]))
          {
            remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field, the derivative matrix and the field matrix do not match", postremark);
          }
        }
        else if (meshtype == "ImageMesh")
        {
          if (derivativesdims.size() != size+2)
          {
            remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field, the derivative matrix should have two more dimensions then the field matrix", postremark);
          }

          if ((derivativesdims[0] != 2)||(derivativesdims[1] != datasize)||(derivativesdims[2] != fielddims[0])||(derivativesdims[3] != fielddims[1]))
          {
            remarkAndThrow("Matrix '"+mlarray.getname()+"' cannot be translated into a SCIRun Field, the derivative matrix and the field matrix do not match", postremark);
          }
        }
        else if (meshtype == "LatVolMesh")
        {
          if (derivativesdims.size() != size+2)
          {
            remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field, the derivative matrix should have two more dimensions then the field matrix", postremark);
          }

          if ((derivativesdims[0] != 7)||(derivativesdims[1] != datasize)||(derivativesdims[2] != fielddims[0])||(derivativesdims[3] != fielddims[1])||(derivativesdims[4] != fielddims[2]))
          {
            remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field, the derivative matrix and the field matrix do not match", postremark);
          }
        }
      }
      else
      {
        remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field, the derivative matrix and the field matrix do not match", postremark);
      }
    }

    return(1+ret);
  }

  ///////////////////////////////////////////////////
  // DEAL WITH STRUCTURED MESHES

  if ((mlx.isdense())&&(mly.isdense())&(mlz.isdense()))
  {

    // TEST: The dimensions of the x, y, and z ,atrix should be equal

    size_t size = static_cast<size_t>(mlx.getnumdims());
    if (mly.getnumdims() != static_cast<int>(size))
    {
      remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (the dimensions of the x and y matrix do not match)", postremark);
    }
    if (mlz.getnumdims() != static_cast<int>(size))
    {
      remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (the dimensions of the x and z matrix do not match)", postremark);
    }

    std::vector<int> dimsx = mlx.getdims();
    std::vector<int> dimsy = mly.getdims();
    std::vector<int> dimsz = mlz.getdims();

    // Check dimension by dimension for any problems
    for (size_t p=0 ; p < size ; p++)
    {
      if(dimsx[p] != dimsy[p])
      {
        remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (the dimensions of the x and y matrix do not match)", postremark);
      }
      if(dimsx[p] != dimsz[p])
      {
        remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (the dimensions of the x and z matrix do not match)", postremark);
      }
    }

    // Minimum number of dimensions is in Matlab is 2 and hence detect any empty dimension
    if (size == 2)
    {
      // This case will filter out the scanline objects
      // Currently SCIRun will fail/crash with an image where one of the
      // dimensions is one, hence prevent some troubles
      if ((dimsx[0] == 1)||(dimsx[1] == 1)) size = 1;
    }

    // Disregard data at odd locations. The translation function for those is not straight forward
    // Hence disregard those data locations.

    if ((fieldtype == "Vector")||(fieldtype == "Tensor")) size--;

    if (meshtype != "")
    {   // explicitly stated type (check whether type confirms the guessed type, otherwise someone supplied us with improper data)
      if ((meshtype == "StructCurveMesh")&&(size!=1))
      {
        remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (invalid number of dimensions for x, y, and z matrix)", postremark);
      }
      if ((meshtype == "StructQuadSurfMesh")&&(size!=2))
      {
        remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (invalid number of dimensions for x, y, and z matrix)", postremark);
      }
      if ((meshtype == "StructHexVolMesh")&&(size!=3))
      {
        remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (invalid number of dimensions for x, y, and z matrix)", postremark);
      }
    }

    if (size == 1) { meshtype = "StructCurveMesh"; }
    if (size == 2) { meshtype = "StructQuadSurfMesh"; }
    if (size == 3) { meshtype = "StructHexVolMesh"; }

    std::vector<int> dims = mlx.getdims();
    if ((fieldtype == "Vector")||(fieldtype == "Tensor"))
    {
      std::vector<int> temp(dims.size()-1);
      for (size_t p=0; p < dims.size()-1; p++) temp[p] = dims[p];
      dims = temp;
    }

    numnodesvec = dims;
    numelementsvec = dims;
    for (size_t p = 0; p < numnodesvec.size(); p++) numelementsvec[p]--;

    if (meshtype == "")
    {
      remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (unknown mesh type)", postremark);
    }

    // We always make this into a linear one
    if (meshbasistype == "") meshbasistype = "linear";

    if (meshbasistype != "linear")
    {
      remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (geometrical higher order basis for structured meshes is not yet supported)", postremark);
    }

    if (meshtype == "StructCurveMesh")    { meshbasis = "CrvLinearLgn";   fdatatype = "vector"; }
    if (meshtype == "StructQuadSurfMesh") { meshbasis = "QuadBilinearLgn"; fdatatype = "FData2d"; }
    if (meshtype == "StructHexVolMesh")   { meshbasis = "HexTrilinearLgn"; fdatatype = "FData3d"; }

    // We should have a meshbasis and a meshtype by now

    // try to figure out the field basis
    if (fieldbasistype == "")
    {
      // In case no data is there
      if (mlfield.isempty())
      {
        fieldbasistype = "nodata";
        fieldtype = "";
      }
      else
      {
        if (fieldtype == "") fieldtype = "double";

        std::vector<int> fdims = mlfield.getdims();
        if (fieldtype == "Vector")
        {
          if (fdims[0] != 3)
          {
            remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (first dimension of field needs to be 3)", postremark);
          }
          std::vector<int> temp(fdims.size()-1);
          for (size_t p = 0; p < temp.size(); p++) temp[p] = fdims[p+1];
          fdims = temp;
        }
        if (fieldtype == "Tensor")
        {
          if ((fdims[0] != 6)&&(fdims[0] != 9))
          {
            remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (first dimension of field needs to be 6, or 9)", postremark);
          }
          std::vector<int> temp(fdims.size()-1);
          for (size_t p = 0; p < temp.size(); p++) temp[p] = fdims[p+1];
          fdims = temp;
        }

        if ((size == 1)&&(size == fdims.size())&&(fdims[0] == numnodesvec[0])) fieldbasistype = "linear";
        if ((size == 2)&&(size == fdims.size())&&(fdims[0] == numnodesvec[0])&&(fdims[1] == numnodesvec[1])) fieldbasistype = "linear";
        if ((size == 3)&&(size == fdims.size())&&(fdims[0] == numnodesvec[0])&&(fdims[1] == numnodesvec[1])&&(fdims[2] == numnodesvec[2])) fieldbasistype = "linear";

        if ((size == 1)&&(size+1 == fdims.size())&&(fdims[0] == numnodesvec[0])&&(fdims[1] == 2)) fieldbasistype = "quadratic";
        if ((size == 2)&&(size+1 == fdims.size())&&(fdims[0] == numnodesvec[0])&&(fdims[1] == numnodesvec[1])&&(fdims[2] == 3)) fieldbasistype = "quadratic";
        if ((size == 3)&&(size+1 == fdims.size())&&(fdims[0] == numnodesvec[0])&&(fdims[1] == numnodesvec[1])&&(fdims[2] == numnodesvec[2])&&(fdims[3] == 4)) fieldbasistype = "quadratic";

        if ((size == 1)&&(size == fdims.size())&&(fdims[0] == numelementsvec[0])) fieldbasistype = "constant";
        if ((size == 2)&&(size == fdims.size())&&(fdims[0] == numelementsvec[0])&&(fdims[1] == numelementsvec[1])) fieldbasistype = "constant";
        if ((size == 3)&&(size == fdims.size())&&(fdims[0] == numelementsvec[0])&&(fdims[1] == numelementsvec[1])&&(fdims[2] == numelementsvec[2])) fieldbasistype = "constant";

        if ((mlfieldderivatives.isdense())&&(fieldbasistype == "linear")) fieldbasistype = "cubic";
      }
    }
    //by now we should know what kind of basis we would like

    if (fieldbasistype == "")
    {
      remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (the dimensions field matrix do not match mesh)", postremark);
    }

    if (fieldbasistype == "nodata") fieldbasis = "NoDataBasis";
    if (fieldbasistype == "constant") fieldbasis = "ConstantBasis";

    if (fieldbasistype == "linear")
    {
      std::vector<int> fdims = mlfield.getdims();
      if (fieldtype == "Vector")
      {
        if (fdims[0] != 3)
        {
          remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (first dimension of field needs to be 3", postremark);
        }
        std::vector<int> temp(fdims.size()-1);
        for (size_t p = 0; p < temp.size(); p++) temp[p] = fdims[p+1];
        fdims = temp;
        datasize = fdims[0];
      }
      if (fieldtype == "Tensor")
      {
        if ((fdims[0] != 6)&&(fdims[0] != 9))
        {
          remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (first dimension of field needs to be 6, or 9)", postremark);
        }
        std::vector<int> temp(fdims.size()-1);
        for (size_t p = 0; p < temp.size(); p++) temp[p] = fdims[p+1];
        fdims = temp;
        datasize = fdims[0];
      }

      if ((!((size == 1)&&(size == fdims.size())&&(fdims[0] == numnodesvec[0]))) &&
          (!((size == 2)&&(size == fdims.size())&&(fdims[0] == numnodesvec[0])&&(fdims[1] == numnodesvec[1]))) &&
          (!((size == 3)&&(size == fdims.size())&&(fdims[0] == numnodesvec[0])&&(fdims[1] == numnodesvec[1])&&(fdims[2] == numnodesvec[2]))))
      {
        remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (dimensions of field do not match dimensions of mesh", postremark);
      }
      if (meshtype == "StructCurveMesh")    { fieldbasis = "CrvLinearLgn";}
      else if (meshtype == "StructQuadSurfMesh") { fieldbasis = "QuadBilinearLgn"; }
      else if (meshtype == "StructHexVolMesh")   { fieldbasis = "HexTrilinearLgn"; }
    }
    else if (fieldbasistype == "quadratic")
    {
      std::vector<int> fdims = mlfield.getdims();
      if (fieldtype == "Vector")
      {
        if (fdims[0] != 3)
        {
          remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (first dimension of field needs to be 3", postremark);
        }
        std::vector<int> temp(fdims.size()-1);
        for (size_t p = 0; p < temp.size(); p++) temp[p] = fdims[p+1];
        fdims = temp;
        datasize = fdims[0];
      }
      if (fieldtype == "Tensor")
      {
        if ((fdims[0] != 6)&&(fdims[0] != 9))
        {
          remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (first dimension of field needs to be 6, or 9)", postremark);
        }
        std::vector<int> temp(fdims.size()-1);
        for (size_t p = 0; p < temp.size(); p++) temp[p] = fdims[p+1];
        fdims = temp;
        datasize = fdims[0];
      }

      if ((!((size == 1)&&(size+1 == fdims.size())&&(fdims[0] == numnodesvec[0])&&(fdims[1] == 2))) &&
          (!((size == 2)&&(size+1 == fdims.size())&&(fdims[0] == numnodesvec[0])&&(fdims[1] == numnodesvec[1])&&(fdims[2] == 3))) &&
          (!((size == 3)&&(size+1 == fdims.size())&&(fdims[0] == numnodesvec[0])&&(fdims[1] == numnodesvec[1])&&(fdims[2] == numnodesvec[2])&&(fdims[3] == 4))))
      {
        remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (dimensions of field do not match dimensions of mesh", postremark);
      }

      if (meshtype == "StructCurveMesh")    { fieldbasis = "CrvQuadraticLgn";}
      if (meshtype == "StructQuadSurfMesh") { fieldbasis = "QuadBiquadraticLgn"; }
      if (meshtype == "StructHexVolMesh")   { fieldbasis = "HexTriquadraticLgn"; }
    }
    else if (fieldbasistype == "cubic")
    {
      std::vector<int> fdims = mlfield.getdims();
      if (fieldtype == "Vector")
      {
        if (fdims[0] != 3)
        {
          remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (first dimension of field needs to be 3", postremark);
        }
        std::vector<int> temp(fdims.size()-1);
        for (size_t p = 0; p < temp.size(); p++) temp[p] = fdims[p+1];
        fdims = temp;
        datasize = fdims[0];
      }
      if (fieldtype == "Tensor")
      {
        if ((fdims[0] != 6)&&(fdims[0] != 9))
        {
          remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (first dimension of field needs to be 6, or 9)", postremark);
        }
        std::vector<int> temp(fdims.size()-1);
        for (size_t p = 0; p < temp.size(); p++) temp[p] = fdims[p+1];
        fdims = temp;
        datasize = fdims[0];
      }

      if (meshtype == "StructCurveMesh")    { fieldbasis = "CrvCubicHmt";}
      if (meshtype == "StructQuadSurfMesh") { fieldbasis = "QuadBicubicLgn"; }
      if (meshtype == "StructHexVolMesh")   { fieldbasis = "HexTricubicLgn"; }

      if (mlfieldderivatives.isdense())
      {
        std::vector<int> derivativesdims = mlfieldderivatives.getdims();
        std::vector<int> fielddims = mlfieldderivatives.getdims();

        if (meshtype == "StructCurveMesh")
        {
          if (derivativesdims.size() != size+2)
          {
            remarkAndThrow("Matrix '"+mlarray.getname()+"' cannot be translated into a SCIRun Field, the derivative matrix should have two more dimensions then the field matrix", postremark);
          }

          if ((derivativesdims[0] != 1)||(derivativesdims[1] != datasize)||(derivativesdims[2] != fielddims[0]))
          {
            remarkAndThrow("Matrix '"+mlarray.getname()+"' cannot be translated into a SCIRun Field, the derivative matrix and the field matrix do not match", postremark);
          }
        }

        if (meshtype == "StructQuadSurfMesh")
        {
          if (derivativesdims.size() != size+2)
          {
            remarkAndThrow("Matrix '"+mlarray.getname()+"' cannot be translated into a SCIRun Field, the derivative matrix should have two more dimensions then the field matrix", postremark);
          }

          if ((derivativesdims[0] != 2)||(derivativesdims[1] != datasize)||(derivativesdims[2] != fielddims[0])||(derivativesdims[3] != fielddims[1]))
          {
            remarkAndThrow("Matrix '"+mlarray.getname()+"' cannot be translated into a SCIRun Field, the derivative matrix and the field matrix do not match", postremark);
          }
        }

        if (meshtype == "StructHexVolMesh")
        {
          if (derivativesdims.size() != size+2)
          {
            remarkAndThrow("Matrix '"+mlarray.getname()+"' cannot be translated into a SCIRun Field, the derivative matrix should have two more dimensions then the field matrix", postremark);
          }

          if ((derivativesdims[0] != 7)||(derivativesdims[1] != datasize)||(derivativesdims[2] != fielddims[0])||(derivativesdims[3] != fielddims[1])||(derivativesdims[4] != fielddims[2]))
          {
            remarkAndThrow("Matrix '"+mlarray.getname()+"' cannot be translated into a SCIRun Field, the derivative matrix and the field matrix do not match", postremark);
          }
        }
      }
      else
      {
        remarkAndThrow("Matrix '"+mlarray.getname()+"' cannot be translated into a SCIRun Field, the derivative matrix and the field matrix do not match", postremark);
      }
    }

    return(1+ret);
  }

 ///////////////////////////////////////////////////////////////////////////////
 // CHECK FOR UNSTRUCTURED MESHES:

  // THIS ONE IS NOW KNOWN
  fdatatype = "vector";

  if (mlnode.isempty())
  {
    remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (no node matrix for unstructured mesh, create a .node field)", postremark);
    // a node matrix is always required
  }

  if (mlnode.getnumdims() > 2)
  {
    remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (invalid number of dimensions for node matrix)", postremark);
    // Currently N dimensional arrays are not supported here
  }

  // Check the dimensions of the NODE array supplied only [3xM] or [Mx3] are supported
  int m,n;
  m = mlnode.getm();
  n = mlnode.getn();

  if ((n==0)||(m==0))
  {
    remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (node matrix is empty)", postremark);
    //empty matrix, no nodes => no mesh => no field......
  }

  if ((n != 3)&&(m != 3))
  {
    remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (one of the dimensions of node matrix needs to be 3)", postremark);
    // SCIRun is ONLY 3D data, no 2D, or 1D
  }

  numnodes = n; if ((m!=3)&&(n==3)) { numnodes = m; mlnode.transpose(); }
  numelements = 0;

  //////////////
  // IT IS GOOD TO HAVE THE NUMBER OF ELEMENTS IN THE FIELD

  numfield = 0;
  datasize = 1;

  if (mlfield.isdense())
  {
    if (fieldtype == "Vector")
    {
      n = mlfield.getn(); m = mlfield.getm();
      if ((m != 3)&&(n != 3))
      {
        remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (field matrix with vectors does not have a dimension of 3)", postremark);
      }

      numfield = n;
      if (m!=3) { numfield = m; mlfield.transpose(); }
      datasize = 3;
    }
    else if (fieldtype == "Tensor")
    {
      n = mlfield.getn(); m = mlfield.getm();
      if (((m != 6)&&(m !=9))&&((n != 6)&&(n != 9)))
      {
        remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (field matrix with tensors does not have a dimension of 6 or 9)", postremark);
      }

      numfield = n;
      datasize = m;
      if ((m!=6)&&(m!=9)) { numfield = m; mlfield.transpose(); datasize = n; }
    }
    else if (fieldtype == "")
    {
      n = mlfield.getn(); m = mlfield.getm();
      if (((m != 1)&&(n != 1))&&((m != 3)&&(n != 3))&&((m != 6)&&(n != 6))&&((m != 9)&&(n != 9)))
      {
        remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (field matrix does not have a dimension of 1, 3, 6, or 9)", postremark);
      }

      numfield = n;
      if (m == 1) fieldtype = "double";
      if (m == 3) fieldtype = "Vector";
      if ((m == 6)||(m == 9)) fieldtype = "Tensor";
      datasize = m;

      if ((m!=1)&&(m!=3)&&(m!=6)&&(m!=9))
      {
        numfield = m;
        if (n == 1) fieldtype = "double";
        if (n == 3) fieldtype = "Vector";
        if ((n == 6)||(n == 9)) fieldtype = "Tensor";
        datasize = n;
        mlfield.transpose();
      }
    }
    else
    {
      n = mlfield.getn(); m = mlfield.getm();
      if (((m != 1)&&(n != 1)))
      {
        remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (field matrix does not have a dimension of 1)", postremark);
      }

      numfield = n;
      if (m!=1) { numfield = m; mlfield.transpose(); }
      datasize = 1;
    }
  }

  // FIRST COUPLE OF CHECKS ON FIELDDATA
  if (fieldbasistype == "nodata")
  {
    if (numfield != 0)
    {
      remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (nodata field basis should not have field data)", postremark);
    }
    fieldbasis = "NoDataBasis";
  }
  else if ((fieldbasistype == "linear")||(fieldbasistype == "cubic"))
  {
    if (meshbasistype != "quadratic")
    {
      if (numfield != numnodes)
      {
        remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (number of field entries does not match number of nodes)", postremark);
      }
    }
  }

  if (fieldbasistype == "")
  {
    if (numfield == 0)
    {
      fieldbasis = "NoDataBasis";
      fieldbasistype = "nodata";
    }
  }

  if (meshbasistype == "nodata")
  {
    remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (mesh needs to have points and cannot be nodata)", postremark);
  }

  //// POINTCLOUD CODE ////////////

  if ((mledge.isempty())&&(mlface.isempty())&&(mlcell.isempty()))
  {
    // This has no connectivity data => it must be a pointcloud ;)
    // Supported mesh/field types here:
    // PointCloudField

    if (meshtype == "") meshtype = "PointCloudMesh";

    if (meshtype != "PointCloudMesh")
    {
      // explicitly stated type (check whether type confirms the guessed type, otherwise someone supplied us with improper data)
      remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (data has to be of the pointcloud class)", postremark);
    }

    if (meshbasistype == "")
    {
      meshbasistype = "constant";
    }

    // Now pointcloud does store data at nodes as constant
    if (meshbasistype != "constant")
    {
      remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (element is a point, hence no linear/higher order interpolation is supported)", postremark);
    }

    meshbasis = "ConstantBasis";

    if (fieldbasistype == "")
    {
      if (numfield == numnodes)
      {
        fieldbasistype = "constant";
      }
      else if (numfield == 0)
      {
        fieldbasistype = "nodata";
      }
    }

    if ((fieldbasistype != "nodata")&&(fieldbasistype != "constant"))
    {
      remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (element is a point, hence no linear/higher order interpolation is supported)", postremark);
    }

    numelements = numnodes;

    if (fieldbasistype == "nodata")    fieldbasis = "NoDataBasis";
    if (fieldbasistype == "constant")
    {
      fieldbasis = "ConstantBasis";
      if (numfield != numelements)
      {
        if (datasize == numelements)
        {
          if (numfield == 1) { fieldtype = "double"; numfield = datasize; datasize = 1; mlfield.transpose(); }
          else if (numfield == 3) { fieldtype = "Vector"; numfield = datasize; datasize = 3; mlfield.transpose(); }
          else if (numfield == 6) { fieldtype = "Tensor"; numfield = datasize; datasize = 6; mlfield.transpose(); }
          else if (numfield == 9) { fieldtype = "Tensor"; numfield = datasize; datasize = 9; mlfield.transpose(); }
        }
        if (numfield != numelements)
        {
          remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (number of elements does not match number of field entries)", postremark);
        }
      }
    }

    if ((mlmeshderivatives.isdense())||(mlfieldderivatives.isdense())/*||
      (mlfieldedge.isdense())*/)
    {
      remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (element is a point, hence no linear/higher order interpolation is supported)", postremark);
    }

    return(1+ret);
  }

  if (meshbasistype == "constant")
  {
    remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (spatial distributed elements cannot have a constant mesh interpolation)", postremark);
  }

  ///// CURVEMESH ////////////////////////////////////

  if (mledge.isdense())
  {
    int n,m;
    // Edge data is provide hence it must be some line element!
    // Supported mesh/field types here:
    //  CurveField

    if (meshtype == "") meshtype = "CurveMesh";

    if (meshtype != "CurveMesh")
    {   // explicitly stated type
      remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (edge connectivity does not macth meshtype)", postremark);
    }

    // established meshtype //

    if ((mlface.isdense())||(mlcell.isdense()))
    {   // a matrix with multiple connectivities is not yet allowed
      remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (multiple connectivity matrices defined)", postremark);
    }

    // Connectivity should be 2D
    if ((mledge.getnumdims() > 2)/*||(mlfieldedge.getnumdims() > 2)*/)
    {
      remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (edge connectivity matrix should be 2D)", postremark);
    }

    // Check whether the connectivity data makes any sense
    // from here on meshtype can only be  linear/cubic/quadratic

    if ((meshbasistype == "linear")||(meshbasistype == "cubic"))
    {
      m = mledge.getm(); n = mledge.getn();
      if ((n!=2)&&(m!=2))
      {
        remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (one of the dimensions of edge needs to be of size 2)", postremark);
      }
      numelements = n;
      if ((m!=2)&&(n==2)) { numelements = m; mledge.transpose(); }
      if (meshbasistype == "linear") meshbasis = "CrvLinearLgn"; else meshbasis = "CrvCubicHmt";
    }
    else if (meshbasistype == "quadratic")
    {
      m = mledge.getm(); n = mledge.getn();
      if ((n!=3)&&(m!=3))
      {
        remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (one of the dimensions of edge needs to be of size 3)", postremark);
      }
      numelements = n;
      if ((m!=3)&&(n==3)) { numelements = m; mledge.transpose(); }
      meshbasistype = "CrvQuadraticLgn";
    }
    else
    {
      m = mledge.getm(); n = mledge.getn();
      if (((n!=2)&&(m!=2))&&((n!=3)&&(m!=3)))
      {
        remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (one of the dimensions of edge needs to be of size 2 or 3)", postremark);
      }
      numelements = n;
      if (((m!=2)&&(m!=3))&&((n==2)||(n==3))) { numelements = m; m = n; mledge.transpose(); }
      if (m == 2) { meshbasistype = "linear"; meshbasis = "CrvLinearLgn"; }
      if (m == 3) { meshbasistype = "quadratic"; meshbasis = "CrvQuadraticLgn"; }
    }

    // established meshbasis

    if ((mlmeshderivatives.isempty())&&(meshbasistype == "cubic"))
    {
      remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (no meshderatives matrix was found)", postremark);
    }

    if (meshbasistype == "cubic")
    {
      std::vector<int> ddims = mlmeshderivatives.getdims();
      if (ddims.size() != 4)
      {
        remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (meshderatives matrix has not proper dimensions)", postremark);
      }

      if ((ddims[0] != 1)&&(ddims[1] != 3)&&(ddims[2] != numelements)&&(ddims[3] != 2))
      {
        remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (meshderatives matrix has not proper dimensions)", postremark);
      }
    }

    // established and checked mesh type ///
    // CHECK THE FIELD PROPERTIES

    if (fieldbasistype == "")
    {
      if ((numfield == numelements)&&(numfield != numnodes))
      {
        fieldbasistype = "constant";
      }
      else if (numfield == numnodes)
      {
        if (meshbasistype == "quadratic")
        {
          fieldbasistype = "quadratic";
        }
        else if ((meshbasistype == "cubic")&&(mlfieldderivatives.isdense()))
        {
          fieldbasistype = "cubic";
        }
        else
        {
          fieldbasistype = "linear";
        }
      }
      else
      {
        if ((meshbasistype == "quadratic")/*&&(mlfieldedge.isdense())*/)
        {
          fieldbasistype = "linear";
        }
        else
        {
          remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (number of elements in field does not match mesh)", postremark);
        }
      }
    }

    if (fieldbasistype == "constant")
    {
      if (numfield != numelements)
      {
        if (datasize == numelements)
        {
          if (numfield == 1) { fieldtype = "double"; numfield = datasize; datasize = 1; mlfield.transpose(); }
          else if (numfield == 3) { fieldtype = "Vector"; numfield = datasize; datasize = 3; mlfield.transpose(); }
          else if (numfield == 6) { fieldtype = "Tensor"; numfield = datasize; datasize = 6; mlfield.transpose(); }
          else if (numfield == 9) { fieldtype = "Tensor"; numfield = datasize; datasize = 9; mlfield.transpose(); }
        }
        if (numfield != numelements)
        {
          remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (number of elements does not match number of field entries)", postremark);
        }
      }
      fieldbasis = "ConstantBasis";
    }

    if ((fieldbasistype == "linear")||(fieldbasistype == "cubic"))
    {
      /*if ((meshbasistype == "quadratic")&&(mlfieldedge.isempty()))
      {
        remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (no fieldedge connectivity matrix)", postremark);
      }
*/
      if (fieldbasistype == "linear")
      {
        fieldbasis = "CrvLinearLgn";
      }
      else
      {
        if (mlfieldderivatives.isempty())
        {
          remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (no fieldderivatives matrix)", postremark);
        }
        fieldbasis = "CrvCubicHmt";
      }
    }

    if (fieldbasis == "quadratic")
    {
      //if (((meshbasistype == "linear")||(meshbasistype == "cubic"))&&(mlfieldedge.isempty()))
      //{
      //  remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (no fieldedge connectivity matrix)", postremark);
      //}
      fieldbasis = "CrvQuadraticLgn";
    }

    // established fieldbasis //

    //if (mlfieldedge.isdense())
    //{
    //  m = mlfieldedge.getm(); n = mlfieldedge.getn();
    //  if (fieldbasistype == "quadratic")
    //  {
    //    if (!(((m==3)&&(n==numelements))||((m==numelements)&&(n==3))))
    //    {
    //      remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (one of the dimensions of fieldedge needs to be of size 2 or 3)", postremark);
    //    }
    //    if (m!=3) mlfieldedge.transpose();
    //  }
    //  else
    //  {
    //    if (!(((m==2)&&(n==numelements))||((m==numelements)&&(n==2))))
    //    {
    //      remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (one of the dimensions of fieldedge needs to be of size 2 or 3)", postremark);
    //    }
    //    if (m!=2) mlfieldedge.transpose();
    //  }
    //}

    if (mlfieldderivatives.isdense())
    {
      std::vector<int> ddims = mlfieldderivatives.getdims();
      if (ddims.size() != 4)
      {
        remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (fieldderatives matrix has not proper dimensions)", postremark);
      }

      if ((ddims[0] != 1)&&(ddims[1] != datasize)&&(ddims[3] != numelements)&&(ddims[2] != 2))
      {
        remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (fieldderatives matrix has not proper dimensions)", postremark);
      }
    }

    return(1+ret);
  }

  if (mlface.isdense())
  {
    int n,m;

    // Edge data is provide hence it must be some line element!
    // Supported mesh/field types here:
    //  CurveField

    if (meshtype == "")
    {
      m = mlface.getm();
      n = mlface.getn();

      if ((m==3)||(m==4)||(m==6)||(m==8)) n = m;
      if ((n==3)||(n==4)||(n==6)||(n==8))
      {
        if ((n==3)||(n==6)) meshtype = "TriSurfMesh";
        else meshtype = "QuadSurfMesh";
      }
    }

    if ((meshtype != "TriSurfMesh")&&(meshtype != "QuadSurfMesh"))
    {   // explicitly stated type
      remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (face connectivity does not match meshtype)", postremark);
    }

    // established meshtype //

    if ((mledge.isdense())||(mlcell.isdense())/*||(mlfieldedge.isdense())*/)
    {   // a matrix with multiple connectivities is not yet allowed
      remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (multiple connectivity matrices defined)", postremark);
    }

    // Connectivity should be 2D
    if ((mlface.getnumdims() > 2))
    {
      remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (face connectivity matrix should be 2D)", postremark);
    }

    // Check whether the connectivity data makes any sense
    // from here on meshtype can only be  linear/cubic/quadratic

    if (meshtype == "TriSurfMesh")
    {
      if ((meshbasistype == "linear")||(meshbasistype == "cubic"))
      {
        m = mlface.getm(); n = mlface.getn();
        if ((n!=3)&&(m!=3))
        {
          remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (one of the dimensions of face needs to be of size 3)", postremark);
        }
        numelements = n;
        if ((m!=3)&&(n==3)) { numelements = m; mlface.transpose(); }
        if (meshbasistype == "linear") meshbasis = "TriLinearLgn"; else meshbasis = "TriCubicHmt";
      }
      else if (meshbasistype == "quadratic")
      {
        m = mlface.getm(); n = mlface.getn();
        if ((n!=6)&&(m!=6))
        {
          remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (one of the dimensions of face needs to be of size 6)", postremark);
        }
        numelements = n;
        if ((m!=6)&&(n==6)) { numelements = m; mlface.transpose(); }
        meshbasistype = "TriQuadraticLgn";
      }
      else
      {
        m = mlface.getm(); n = mlface.getn();
        if (((n!=3)&&(m!=3))&&((n!=6)&&(m!=6)))
        {
          remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (one of the dimensions of face needs to be of size 3 or 6)", postremark);
        }
        numelements = n;
        if (((m!=3)&&(m!=6))&&((n==3)||(n==6))) { numelements = m; m = n; mlface.transpose(); }
        if (m == 3) { meshbasistype = "linear"; meshbasis = "TriLinearLgn"; }
        if (m == 6) { meshbasistype = "quadratic"; meshbasis = "TriQuadraticLgn"; }
      }
    }
    else
    {
      if ((meshbasistype == "linear")||(meshbasistype == "cubic"))
      {
        m = mlface.getm(); n = mlface.getn();
        if ((n!=4)&&(m!=4))
        {
          remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (one of the dimensions of face needs to be of size 4)", postremark);
        }
        numelements = n;
        if ((m!=4)&&(n==4)) { numelements = m; mlface.transpose(); }
        if (meshbasistype == "linear") meshbasis = "QuadBilinearLgn"; else meshbasis = "QuadBicubicHmt";
      }
      else if (meshbasistype == "quadratic")
      {
        m = mlface.getm(); n = mlface.getn();
        if ((n!=8)&&(m!=8))
        {
          remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (one of the dimensions of face needs to be of size 8)", postremark);
        }
        numelements = n;
        if ((m!=8)&&(n==8)) { numelements = m; mlface.transpose(); }
        meshbasistype = "QuadBiquadraticLgn";
      }
      else
      {
        m = mlface.getm(); n = mlface.getn();
        if (((n!=4)&&(m!=4))&&((n!=8)&&(m!=8)))
        {
          remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (one of the dimensions of face needs to be of size 4 or 8)", postremark);
        }
        numelements = n;
        if (((m!=4)&&(m!=8))&&((n==4)||(n==8))) { numelements = m; m = n; mlface.transpose(); }
        if (m == 4) { meshbasistype = "linear"; meshbasis = "QuadBilinearLgn"; }
        if (m == 8) { meshbasistype = "quadratic"; meshbasis = "QuadBiquadraticLgn"; }
      }
    }

    // established meshbasis

    if ((mlmeshderivatives.isempty())&&(meshbasistype == "cubic"))
    {
      remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (no meshderatives matrix was found)", postremark);
    }

    if (meshbasistype == "cubic")
    {
      std::vector<int> ddims = mlmeshderivatives.getdims();
      if (ddims.size() != 4)
      {
        remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (meshderatives matrix has not proper dimensions)", postremark);
      }

      if (meshtype == "TriSurfMesh")
      {
        if ((ddims[0] != 2)&&(ddims[1] != 3)&&(ddims[2] != numelements)&&(ddims[3] != 3))
        {
          remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (meshderatives matrix has not proper dimensions)", postremark);
        }
      }
      else
      {
        if ((ddims[0] != 2)&&(ddims[1] != 3)&&(ddims[2] != numelements)&&(ddims[3] != 4))
        {
          remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (meshderatives matrix has not proper dimensions)", postremark);
        }
      }
    }

    // established and checked mesh type ///
    // CHECK THE FIELD PROPERTIES

    if (fieldbasistype == "")
    {
      if ((numfield == numelements)&&(numfield != numnodes))
      {
        fieldbasistype = "constant";
      }
      else if (numfield == numnodes)
      {
        if (meshbasistype == "quadratic")
        {
          fieldbasistype = "quadratic";
        }
        else if ((meshbasistype == "cubic")&&(mlfieldderivatives.isdense()))
        {
          fieldbasistype = "cubic";
        }
        else
        {
          fieldbasistype = "linear";
        }
      }
      else
      {
        if ((meshbasistype == "quadratic"))
        {
          fieldbasistype = "linear";
        }
        else
        {
          remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (number of elements in field does not match mesh)", postremark);
        }
      }
    }

    if (fieldbasistype == "constant")
    {
      if (numfield != numelements)
      {
        remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (number of elements in field does not match mesh)", postremark);
      }
      fieldbasis = "ConstantBasis";
    }

    if ((fieldbasistype == "linear")||(fieldbasistype == "cubic"))
    {
      //if ((meshbasistype == "quadratic")&&(mlfieldedge.isempty()))
      //{
      //  remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (no fieldedge connectivity matrix)", postremark);
      //}

      if (fieldbasistype == "linear")
      {
        if (meshtype == "TriSurfMesh") fieldbasis = "TriLinearLgn"; else fieldbasis = "QuadBilinearLgn";
      }
      else
      {
        if (mlfieldderivatives.isempty())
        {
          remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (no fieldderivatives matrix)", postremark);
        }
        if (meshtype == "TriSurfMesh") fieldbasis = "TriCubicHmt"; else fieldbasis = "QuadBicubicHmt";
      }
    }

    if (fieldbasis == "quadratic")
    {
      if (((meshbasistype == "linear")||(meshbasistype == "cubic")))
      {
        remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (no fieldedge connectivity matrix)", postremark);
      }
      if (meshtype == "TriSurfMesh") fieldbasis = "TriQuadraticLgn"; else fieldbasis = "QuadBiquadraticLgn";
    }

    if ((mlfieldderivatives.isdense())&&(fieldbasistype == "cubic"))
    {
      std::vector<int> ddims = mlfieldderivatives.getdims();
      if (ddims.size() != 4)
      {
        remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (fieldderatives matrix has not proper dimensions)", postremark);
      }

      if (meshtype == "TriSurfMesh")
      {
        if ((ddims[0] != 2)&&(ddims[1] != datasize)&&(ddims[3] != numelements)&&(ddims[2] != 3))
        {
          remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (fieldderatives matrix has not proper dimensions)", postremark);
        }
      }
      else
      {
        if ((ddims[0] != 2)&&(ddims[1] != datasize)&&(ddims[3] != numelements)&&(ddims[2] != 4))
        {
          remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (fieldderatives matrix has not proper dimensions)", postremark);
        }
      }
    }

    return(1+ret);
  }


  if (mlcell.isdense())
  {
    int n,m;

    // Edge data is provide hence it must be some line element!
    // Supported mesh/field types here:
    //  CurveField

    if (meshtype == "")
    {
      m = mlcell.getm();
      n = mlcell.getn();

      if ((m==4)||(m==6)||(m==8)||(m==10)||(m==15)||(m==20)) n = m;
      if ((n==4)||(n==6)||(n==8)||(n==10)||(n==15)||(n==20))
      {
        if ((n==4)||(n==10)) meshtype = "TetVolMesh";
        else if ((n==6)||(n==15)) meshtype = "PrismVolMesh";
        else meshtype = "HexVolMesh";
      }
    }

    if ((meshtype != "TetVolMesh")&&(meshtype != "PrismVolMesh")&&(meshtype != "HexVolMesh"))
    {   // explicitly stated type
      remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (cell connectivity does not match meshtype)", postremark);
    }

    // established meshtype //

    if ((mledge.isdense())||(mlface.isdense())/*||(mlfieldedge.isdense())*/)
    {   // a matrix with multiple connectivities is not yet allowed
      remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (multiple connectivity matrices defined)", postremark);
    }

    // Connectivity should be 2D
    if ((mlcell.getnumdims() > 2))
    {
      remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (cell connectivity matrix should be 2D)", postremark);
    }

    // Check whether the connectivity data makes any sense
    // from here on meshtype can only be  linear/cubic/quadratic

    if (meshtype == "TetVolMesh")
    {
      if ((meshbasistype == "linear")||(meshbasistype == "cubic"))
      {
        m = mlcell.getm(); n = mlcell.getn();
        if ((n!=4)&&(m!=4))
        {
          remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (one of the dimensions of cell needs to be of size 4)", postremark);
        }
        numelements = n;
        if ((m!=4)&&(n==4)) { numelements = m; mlcell.transpose(); }
        if (meshbasistype == "linear") meshbasis = "TetLinearLgn"; else meshbasis = "TetCubicHmt";
      }
      else if (meshbasistype == "quadratic")
      {
        m = mlcell.getm(); n = mlcell.getn();
        if ((n!=10)&&(m!=10))
        {
          remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (one of the dimensions of cell needs to be of size 10)", postremark);
        }
        numelements = n;
        if ((m!=10)&&(n==10)) { numelements = m; mlcell.transpose(); }
        meshbasistype = "TetQuadraticLgn";
      }
      else
      {
        m = mlcell.getm(); n = mlcell.getn();
        if (((n!=4)&&(m!=4))&&((n!=10)&&(m!=10)))
        {
          remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (one of the dimensions of edge needs to be of size 4 or 10)", postremark);
        }
        numelements = n;
        if (((m!=4)&&(m!=10))&&((n==4)||(n==10))) { numelements = m; m = n; mlcell.transpose(); }
        if (m == 4) { meshbasistype = "linear"; meshbasis = "TetLinearLgn"; }
        if (m == 10) { meshbasistype = "quadratic"; meshbasis = "TetQuadraticLgn"; }
      }
    }
    else if (meshtype == "PrismVolMesh")
    {
      if ((meshbasistype == "linear")||(meshbasistype == "cubic"))
      {
        m = mlcell.getm(); n = mlcell.getn();
        if ((n!=6)&&(m!=6))
        {
          remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (one of the dimensions of cell needs to be of size 6)", postremark);
        }
        numelements = n;
        if ((m!=6)&&(n==6)) { numelements = m; mlcell.transpose(); }
        if (meshbasistype == "linear") meshbasis = "TetLinearLgn"; else meshbasis = "PrismCubicHmt";
      }
      else if (meshbasistype == "quadratic")
      {
        m = mlcell.getm(); n = mlcell.getn();
        if ((n!=15)&&(m!=15))
        {
          remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (one of the dimensions of cell needs to be of size 15)", postremark);
        }
        numelements = n;
        if ((m!=15)&&(n==15)) { numelements = m; mlcell.transpose(); }
        meshbasistype = "PrismQuadraticLgn";
      }
      else
      {
        m = mlcell.getm(); n = mlcell.getn();
        if (((n!=6)&&(m!=6))&&((n!=15)&&(m!=15)))
        {

          remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (one of the dimensions of cell needs to be of size 6 or 15)", postremark);
        }
        numelements = n;
        if (((m!=6)&&(m!=15))&&((n==6)||(n==15))) { numelements = m; m = n; mlcell.transpose(); }
        if (m == 6) { meshbasistype = "linear"; meshbasis = "PrismLinearLgn"; }
        if (m == 15) { meshbasistype = "quadratic"; meshbasis = "PrismQuadraticLgn"; }
      }
    }
    else
    {
      if ((meshbasistype == "linear")||(meshbasistype == "cubic"))
      {
        m = mlcell.getm(); n = mlcell.getn();
        if ((n!=8)&&(m!=8))
        {
          remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (one of the dimensions of cell needs to be of size 8)", postremark);
        }
        numelements = n;
        if ((m!=8)&&(n==8)) { numelements = m; mlcell.transpose(); }
        if (meshbasistype == "linear") meshbasis = "HexTrilinearLgn"; else meshbasis = "HexTricubicHmt";
      }
      else if (meshbasistype == "quadratic")
      {
        m = mlcell.getm(); n = mlcell.getn();
        if ((n!=20)&&(m!=20))
        {
          remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (one of the dimensions of cell needs to be of size 20)", postremark);
        }
        numelements = n;
        if ((m!=20)&&(n==20)) { numelements = m; mlcell.transpose(); }
        meshbasistype = "HexTriquadraticLgn";
      }
      else
      {
        m = mlcell.getm(); n = mlcell.getn();
        if (((n!=8)&&(m!=8))&&((n!=20)&&(m!=20)))
        {
          remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (one of the dimensions of cell needs to be of size 8 or 20)", postremark);
        }
        numelements = n;
        if (((m!=8)&&(m!=20))&&((n==8)||(n==20))) { numelements = m; m = n; mlcell.transpose(); }
        if (m == 8) { meshbasistype = "linear"; meshbasis = "HexTrilinearLgn"; }
        if (m == 20) { meshbasistype = "quadratic"; meshbasis = "HexTriquadraticLgn"; }
      }
    }

    // established meshbasis

    if ((mlmeshderivatives.isempty())&&(meshbasistype == "cubic"))
    {
      remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (no meshderatives matrix was found)", postremark);
    }

    if (meshbasistype == "cubic")
    {
      std::vector<int> ddims = mlmeshderivatives.getdims();
      if (ddims.size() != 4)
      {
        remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (meshderatives matrix has not proper dimensions)", postremark);
      }

      if (meshtype == "TetVolMesh")
      {
        if ((ddims[0] != 3)&&(ddims[1] != 3)&&(ddims[2] != numelements)&&(ddims[3] != 4))
        {
          remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (meshderatives matrix has not proper dimensions)", postremark);
        }
      }
      else if (meshtype == "PrismVolMesh")
      {
        if ((ddims[0] != 3)&&(ddims[1] != 3)&&(ddims[2] != numelements)&&(ddims[3] != 6))
        {
          remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (meshderatives matrix has not proper dimensions)", postremark);
        }
      }
      else
      {
        if ((ddims[0] != 7)&&(ddims[1] != 3)&&(ddims[2] != numelements)&&(ddims[3] != 8))
        {
          remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (meshderatives matrix has not proper dimensions)", postremark);
        }
      }
    }

    // established and checked mesh type ///
    // CHECK THE FIELD PROPERTIES

    if (fieldbasistype == "")
    {
      if ((numfield == numelements)&&(numfield != numnodes))
      {
        fieldbasistype = "constant";
      }
      else if (numfield == numnodes)
      {
        if (meshbasistype == "quadratic")
        {
          fieldbasistype = "quadratic";
        }
        else if ((meshbasistype == "cubic")&&(mlfieldderivatives.isdense()))
        {
          fieldbasistype = "cubic";
        }
        else
        {
          fieldbasistype = "linear";
        }
      }
      else
      {
        if ((meshbasistype == "quadratic"))
        {
          fieldbasistype = "linear";
        }
        else
        {
          remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (number of elements in field does not match mesh)", postremark);
        }
      }
    }

    if (fieldbasistype == "constant")
    {
      if (numfield != numelements)
      {
        remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (number of elements in field does not match mesh)", postremark);
      }
      fieldbasis = "ConstantBasis";
    }

    if ((fieldbasistype == "linear")||(fieldbasistype == "cubic"))
    {
      //if ((meshbasistype == "quadratic")&&(mlfieldedge.isempty()))
      //{
      //  remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (no fieldedge connectivity matrix)", postremark);
      //}

      if (fieldbasistype == "linear")
      {
        if (meshtype == "TetVolMesh") fieldbasis = "TetLinearLgn";
        else if (meshtype == "PrismVolMesh") fieldbasis = "PrismLinearLgn";
        else fieldbasis = "HexTrilinearLgn";
      }
      else
      {
        if (mlfieldderivatives.isempty())
        {
          remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (no fieldderivatives matrix)", postremark);
        }
        if (meshtype == "TetVolMesh") fieldbasis = "TetCubicHmt";
        else if (meshtype == "PrismVolMesh") fieldbasis = "PrismCubicHmt";
        else fieldbasis = "HexTricubicHmt";
      }
    }

    if (fieldbasis == "quadratic")
    {
      if (((meshbasistype == "linear")||(meshbasistype == "cubic")))
      {
        remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (no fieldface connectivity matrix)", postremark);
      }
      if (meshtype == "TetVolMesh") fieldbasis = "TetQuadraticLgn";
      else if (meshtype == "PrismVolMesh") fieldbasis = "PrismQuadraticLgn";
      else fieldbasis = "HexTriquadraticLgn";
    }

    if ((mlfieldderivatives.isdense())&&(fieldbasistype == "cubic"))
    {
      std::vector<int> ddims = mlfieldderivatives.getdims();
      if (ddims.size() != 4)
      {
        remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (fieldderatives matrix has not proper dimensions)", postremark);
      }

      if (meshtype == "TetVolMesh")
      {
        if ((ddims[0] != 3)&&(ddims[1] != datasize)&&(ddims[3] != numelements)&&(ddims[2] != 4))
        {
          remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (fieldderatives matrix has not proper dimensions)", postremark);
        }
      }
      else if (meshtype == "PrismVolMesh")
      {
        if ((ddims[0] != 3)&&(ddims[1] != datasize)&&(ddims[3] != numelements)&&(ddims[2] != 6))
        {
          remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (fieldderatives matrix has not proper dimensions)", postremark);
        }
      }
      else
      {
        if ((ddims[0] != 7)&&(ddims[1] != datasize)&&(ddims[3] != numelements)&&(ddims[2] != 8))
        {
          remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (fieldderatives matrix has not proper dimensions)", postremark);
        }
      }
    }
    return(1+ret);
  }

  remarkAndThrow("Matrix '" + mlarray.getname() + "' cannot be translated into a SCIRun Field (cannot match the Matlab structure with any of the supported mesh classes)", postremark);
  return 0;
}


bool MatlabToFieldAlgo::addtransform(VMesh* vmesh)
{
  if (mltransform.isdense())
  {
    Transform T;
    double trans[16];
    mltransform.getnumericarray(trans,16);
    T.set_trans(trans);
    vmesh->set_transform(T);
  }
  return(true);
}


bool MatlabToFieldAlgo::addnodes(VMesh* vmesh)
{
	// Get the data from the Matlab file, which has been buffered
	// but whose format can be anything. The next piece of code
	// copies and casts the data

  if (meshbasistype == "quadratic")
  {
    error("The converter misses code to add quadratic nodes to mesh");
    return (false);
  }

	std::vector<double> mldata;
	mlnode.getnumericarray(mldata);

	// Again the data is copied but now reorganised into
	// a vector of Point objects

	int numnodes = mlnode.getn();
	vmesh->node_reserve(VMesh::Node::size_type(numnodes));

	int p,q;
	for (p = 0, q = 0; p < numnodes; p++, q+=3)
	{
    vmesh->add_point(Point(mldata[q],mldata[q+1],mldata[q+2]));
  }

  return (true);
}


bool MatlabToFieldAlgo::addedges(VMesh* vmesh)
{
	// Get the data from the Matlab file, which has been buffered
	// but whose format can be anything. The next piece of code
	// copies and casts the data

  if (meshbasistype == "quadratic")
  {
    error("The converter misses code to add quadratic edges to mesh");
    return (false);
  }

	std::vector<unsigned int> mldata;
	mledge.getnumericarray(mldata);

	// check whether it is zero based indexing
	// In short if there is a zero it must be zero
	// based numbering right ??
	// If not we assume one based numbering

	bool zerobased = false;
	int size = static_cast<int>(mldata.size());
	for (int p = 0; p < size; p++) { if (mldata[p] == 0) {zerobased = true; break;} }

	if (!zerobased)
	{   // renumber to go from Matlab indexing to C++ indexing
		for (int p = 0; p < size; p++) { mldata[p]--;}
	}

  int m,n;
   m = mledge.getm();
   n = mledge.getn();

	vmesh->elem_reserve(VMesh::Elem::size_type(n));

  VMesh::Node::array_type edge(m);

  int r;
  r = 0;

  for (int p = 0; p < n; p++)
	{
     for (int q = 0 ; q < m; q++)
     {
       edge[q] = mldata[r]; r++;
     }

		vmesh->add_elem(edge);
	}

  return (true);
}


bool MatlabToFieldAlgo::addfaces(VMesh* vmesh)
{
   // Get the data from the Matlab file, which has been buffered
   // but whose format can be anything. The next piece of code
   // copies and casts the data

  if (meshbasistype == "quadratic")
  {
    error("The converter misses code to add quadratic edges to mesh");
    return (false);
  }

  std::vector<unsigned int> mldata;
  mlface.getnumericarray(mldata);

  // check whether it is zero based indexing
  // In short if there is a zero it must be zero
  // based numbering right ??
  // If not we assume one based numbering

  bool zerobased = false;
  int size = static_cast<int>(mldata.size());
  for (int p = 0; p < size; p++) { if (mldata[p] == 0) {zerobased = true; break;} }

  if (!zerobased)
  {   // renumber to go from Matlab indexing to C++ indexing
    for (int p = 0; p < size; p++) { mldata[p]--;}
  }

  int m,n;
  m = mlface.getm();
  n = mlface.getn();

  vmesh->elem_reserve(VMesh::Elem::size_type(n));

  VMesh::Node::array_type face(m);

  int r;
  r = 0;

  for (int p = 0; p < n; p++)
  {
    for (int q = 0 ; q < m; q++)
    {
      face[q] = mldata[r]; r++;
    }
    vmesh->add_elem(face);
  }

  return (true);
}


bool MatlabToFieldAlgo::addcells(VMesh* vmesh)
{
  // Get the data from the Matlab file, which has been buffered
  // but whose format can be anything. The next piece of code
  // copies and casts the data

  if (meshbasistype == "quadratic")
  {
    error("The converter misses code to add quadratic edges to mesh");
    return (false);
  }

  std::vector<unsigned int> mldata;
  mlcell.getnumericarray(mldata);

  // check whether it is zero based indexing
  // In short if there is a zero it must be zero
  // based numbering right ??
  // If not we assume one based numbering

  bool zerobased = false;
  int size = static_cast<int>(mldata.size());
  for (int p = 0; p < size; p++) { if (mldata[p] == 0) {zerobased = true; break;} }

  if (!zerobased)
  {   // renumber to go from Matlab indexing to C++ indexing
    for (int p = 0; p < size; p++) { mldata[p]--;}
  }

  int m,n;
  m = mlcell.getm();
  n = mlcell.getn();

  vmesh->elem_reserve(VMesh::Elem::size_type(n));

  VMesh::Node::array_type cell(m);

  int r;
  r = 0;

  for (int p = 0; p < n; p++)
  {
    for (int q = 0 ; q < m; q++)
    {
      cell[q] = mldata[r]; r++;
    }
    vmesh->add_elem(cell);
  }

  return (true);
}


bool MatlabToFieldAlgo::addderivatives(VMesh* /*vmesh*/)
{
  if (meshbasistype == "cubic")
  {
    error("The converter misses code to add cubic hermitian derivatives edges to mesh");
    return (false);
  }
  return (true);
}


bool MatlabToFieldAlgo::addscalefactors(VMesh* /*vmesh*/)
{
  if (meshbasistype == "cubic")
  {
    error("The converter misses code to add cubic hermitian scalefactors edges to mesh");
    return (false);
  }
  return (true);
}




bool MatlabToFieldAlgo::addfield(VField* field)
{
  if (field->is_scalar())
  {
    if (field->is_char())
    {
      std::vector<char> fielddata;
      mlfield.getnumericarray(fielddata); // cast and copy the real part of the data
      field->set_values(fielddata);
      return (true);
    }
    if (field->is_unsigned_char())
    {
      std::vector<unsigned char> fielddata;
      mlfield.getnumericarray(fielddata); // cast and copy the real part of the data
      field->set_values(fielddata);
      return (true);
    }
    if (field->is_short())
    {
      std::vector<short> fielddata;
      mlfield.getnumericarray(fielddata); // cast and copy the real part of the data
      field->set_values(fielddata);
      return (true);
    }
    if (field->is_unsigned_short())
    {
      std::vector<unsigned short> fielddata;
      mlfield.getnumericarray(fielddata); // cast and copy the real part of the data
      field->set_values(fielddata);
      return (true);
    }
    if (field->is_int())
    {
      std::vector<int> fielddata;
      mlfield.getnumericarray(fielddata); // cast and copy the real part of the data
      field->set_values(fielddata);
      return (true);
    }
    if (field->is_unsigned_int())
    {
      std::vector<unsigned int> fielddata;
      mlfield.getnumericarray(fielddata); // cast and copy the real part of the data
      field->set_values(fielddata);
      return (true);
    }
    if (field->is_longlong())
    {
      std::vector<long long> fielddata;
      mlfield.getnumericarray(fielddata); // cast and copy the real part of the data
      field->set_values(fielddata);
      return (true);
    }
    if (field->is_unsigned_longlong())
    {
      std::vector<unsigned long long> fielddata;
      mlfield.getnumericarray(fielddata); // cast and copy the real part of the data
      field->set_values(fielddata);
      return (true);
    }
    if (field->is_float())
    {
      std::vector<float> fielddata;
      mlfield.getnumericarray(fielddata); // cast and copy the real part of the data
      field->set_values(fielddata);
      return (true);
    }
    if (field->is_double())
    {
      std::vector<double> fielddata;
      mlfield.getnumericarray(fielddata); // cast and copy the real part of the data
      field->set_values(fielddata);
      return (true);
    }
  }

  if (field->is_vector())
  {
    std::vector<double> fielddata;
    mlfield.getnumericarray(fielddata); // cast and copy the real part of the data

    VMesh::size_type numdata = static_cast<VMesh::size_type>(fielddata.size());
    if (numdata > (3*field->num_values())) numdata = (3*field->num_values()); // make sure we do not copy more data than there are elements

    VField::index_type p,q;
    for (p=0,q=0; p < numdata; q++)
    {
      Vector v(fielddata[p],fielddata[p+1],fielddata[p+2]); p+=3;
      field->set_value(v,VMesh::index_type(q));
    }

    return(true);
  }

  if (field->is_tensor())
  {

    std::vector<double> fielddata;
    mlfield.getnumericarray(fielddata); // cast and copy the real part of the data

    VMesh::size_type numdata = static_cast<VMesh::size_type>(fielddata.size());
    Tensor tensor;

    if (mlfield.getm() == 6)
    { // Compressed tensor data : xx,yy,zz,xy,xz,yz
      if (numdata > (6*field->num_values())) numdata = (6*field->num_values()); // make sure we do not copy more data than there are elements
      VField::index_type p,q;
      for (p = 0, q = 0; p < numdata; p +=6, q++)
      {
        compressedtensor(fielddata,tensor,p);
        field->set_value(tensor,VMesh::index_type(q));
      }
    }
    else
    {  // UnCompressed tensor data : xx,xy,xz,yx,yy,yz,zx,zy,zz
      if (numdata > (9*field->num_values())) numdata = (9*field->num_values()); // make sure we do not copy more data than there are elements
      VField::index_type p,q;
      for (p = 0, q = 0; p < numdata; p +=9, q++)
      {
        uncompressedtensor(fielddata,tensor,p);
        field->set_value(tensor,VMesh::index_type(q));
      }
    }
  }

  return(true);
}

MatlabToFieldAlgo::MatlabToFieldAlgo() : numnodes(0), numelements(0), numfield(0), datasize(0)
{
}

void MatlabToFieldAlgo::setreporter(LoggerHandle pr)
{
  pr_ = pr;
}

void MatlabToFieldAlgo::error(const std::string& error) const
{
  if(pr_) pr_->error(error);
}

void MatlabToFieldAlgo::warning(const std::string& warning) const
{
  if(pr_) pr_->warning(warning);
}

void MatlabToFieldAlgo::remark(const std::string& remark) const
{
  if(pr_) pr_->remark(remark);
}

void MatlabToFieldAlgo::compressedtensor(std::vector<double> &fielddata,Tensor &tens, unsigned int p)
{
  tens.val(0,0) = fielddata[p+0];
  tens.val(0,1) = fielddata[p+1];
  tens.val(0,2) = fielddata[p+2];
  tens.val(1,0) = fielddata[p+1];
  tens.val(1,1) = fielddata[p+3];
  tens.val(1,2) = fielddata[p+4];
  tens.val(2,0) = fielddata[p+2];
  tens.val(2,1) = fielddata[p+4];
  tens.val(2,2) = fielddata[p+5];
}

void MatlabToFieldAlgo::uncompressedtensor(std::vector<double> &fielddata,Tensor &tens, unsigned int p)
{
  tens.val(0, 0) = fielddata[p];
  tens.val(0, 1) = fielddata[p + 1];
  tens.val(0, 2) = fielddata[p + 2];
  tens.val(1, 0) = fielddata[p + 3];
  tens.val(1, 1) = fielddata[p + 4];
  tens.val(1, 2) = fielddata[p + 5];
  tens.val(2, 0) = fielddata[p + 6];
  tens.val(2, 1) = fielddata[p + 7];
  tens.val(2, 2) = fielddata[p + 8];
}
