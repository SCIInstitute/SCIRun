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


/// @todo Documentation Core/Datatypes/Legacy/Field/FieldInformation.cc

#include <Core/Utils/Legacy/TypeDescription.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/Legacy/Field/Mesh.h>
#include <Core/Utils/Legacy/StringUtil.h>

using namespace SCIRun;
using namespace SCIRun::Core::Geometry;


/// @todo DAN: use std::replace for strings below.

void
FieldTypeInformation::insert_field_type_information(Field* field)
{
  std::string temp;
  // Get the name of the GenericField class
  // This should give GenericField

  if (!field)
    return;
  field_type = field->get_type_description(Field::FIELD_NAME_ONLY_E)->get_name();

  // Analyze the mesh type
  // This will result in
  // mesh_type, mesh_basis_type, and point_type
  const TypeDescription* mesh_td = field->get_type_description(Field::MESH_TD_E);
  TypeDescription::td_vec* mesh_sub_td = mesh_td->get_sub_type();
  const TypeDescription* mesh_basis_td = (*mesh_sub_td)[0];
  TypeDescription::td_vec* mesh_basis_sub_td = mesh_basis_td->get_sub_type();
  const TypeDescription* point_td = (*mesh_basis_sub_td)[0];

  temp = mesh_td->get_name();
  mesh_type = temp.substr(0,temp.find("<"));
  temp = mesh_basis_td->get_name();
  mesh_basis_type = temp.substr(0,temp.find("<"));
  point_type = point_td->get_name();

  // Analyze the basis type

  const TypeDescription* basis_td = field->get_type_description(Field::BASIS_TD_E);
  TypeDescription::td_vec* basis_sub_td = basis_td->get_sub_type();
  const TypeDescription* data_td = (*basis_sub_td)[0];

  temp = basis_td->get_name();
  basis_type = temp.substr(0,temp.find("<"));
  data_type = data_td->get_name();
  for (size_t j=0; j<data_type.size(); j++) if(data_type[j] == ' ') data_type[j] = '_';

  const TypeDescription* container_td = field->get_type_description(Field::FDATA_TD_E);
  temp = container_td->get_name();
  container_type = temp.substr(0,temp.find("<"));
}


FieldInformation::FieldInformation(const std::string& meshtype,const std::string& meshbasis,const std::string& databasis, const std::string& datatype)
{
  set_field_type("GenericField");
  set_point_type("Point");
  set_container_type("vector");
  set_data_type(datatype);
  set_mesh_type(meshtype);
  set_mesh_basis_type(meshbasis);
  set_data_basis_type(databasis);
}

FieldInformation::FieldInformation(const std::string& meshtype,int meshbasis,int databasis, const std::string& datatype)
{
  set_field_type("GenericField");
  set_point_type("Point");
  set_container_type("vector");
  set_data_type(datatype);
  set_mesh_type(meshtype);
  set_mesh_basis_type(meshbasis);
  set_data_basis_type(databasis);
}

FieldInformation::FieldInformation(const std::string& meshtype,const std::string& basis, const std::string& datatype)
{
  set_field_type("GenericField");
  set_point_type("Point");
  set_container_type("vector");
  set_data_type(datatype);
  set_mesh_type(meshtype);
  set_mesh_basis_type(basis);
  set_data_basis_type(basis);
}

FieldInformation::FieldInformation(const std::string& meshtype,int basis, const std::string& datatype)
{
  set_field_type("GenericField");
  set_point_type("Point");
  set_container_type("vector");
  set_data_type(datatype);
  set_mesh_type(meshtype);
  set_mesh_basis_type(basis);
  set_data_basis_type(basis);
}

FieldInformation::FieldInformation(mesh_info_type meshtype,
                                   meshbasis_info_type meshbasis,
                                   databasis_info_type databasis,
                                   data_info_type datatype)
{
  set_field_type("GenericField");
  set_point_type("Point");
  set_container_type("vector");
  set_data_type(datatype);
  set_mesh_type(meshtype);
  set_mesh_basis_type(meshbasis);
  set_data_basis_type(databasis);
}

FieldInformation::FieldInformation(mesh_info_type meshtype,
                                   databasis_info_type databasis,
                                   data_info_type datatype)
{
  set_field_type("GenericField");
  set_point_type("Point");
  set_container_type("vector");
  set_data_type(datatype);
  set_mesh_type(meshtype);
  int meshbasis = databasis;
  set_mesh_basis_type(meshbasis);
  set_data_basis_type(databasis);
}


FieldInformation::FieldInformation(FieldHandle handle)
{
  insert_field_type_information(handle.get());
}


FieldInformation::FieldInformation(Field* field)
{
  insert_field_type_information(field);
}

std::string
FieldInformation::get_field_type() const
{
  return(field_type);
}

void
FieldInformation::set_field_type(const std::string& type)
{
  field_type = type;
}

std::string
FieldInformation::get_mesh_type() const
{
  return(mesh_type);
}

void
FieldInformation::set_mesh_type(const std::string& type)
{
  std::string typeLower = string_tolower(type);
  if (typeLower == "scanlinemesh") set_mesh_type(SCANLINEMESH_E);
  else if (typeLower == "imagemesh") set_mesh_type(IMAGEMESH_E);
  else if (typeLower == "latvolmesh") set_mesh_type(LATVOLMESH_E);
  else if (typeLower == "structcurvemesh") set_mesh_type(STRUCTCURVEMESH_E);
  else if (typeLower == "structquadsurfmesh") set_mesh_type(STRUCTQUADSURFMESH_E);
  else if (typeLower == "structhexvolmesh") set_mesh_type(STRUCTHEXVOLMESH_E);
  else if (typeLower == "pointcloudmesh") set_mesh_type(POINTCLOUDMESH_E);
  else if (typeLower == "curvemesh") set_mesh_type(CURVEMESH_E);
  else if (typeLower == "trisurfmesh") set_mesh_type(TRISURFMESH_E);
  else if (typeLower == "quadsurfmesh") set_mesh_type(QUADSURFMESH_E);
  else if (typeLower == "tetvolmesh") set_mesh_type(TETVOLMESH_E);
  else if (typeLower == "prismvolmesh") set_mesh_type(PRISMVOLMESH_E);
  else if (typeLower == "hexvolmesh") set_mesh_type(HEXVOLMESH_E);
  else { BOOST_THROW_EXCEPTION(UnknownMeshType() << Core::ErrorMessage("Unknown mesh type")); }
}


void
FieldInformation::set_mesh_type(mesh_info_type type)
{
  switch (type)
  {
    case SCANLINEMESH_E:      mesh_type = "ScanlineMesh"; break;
    case IMAGEMESH_E:         mesh_type = "ImageMesh"; break;
    case LATVOLMESH_E:        mesh_type = "LatVolMesh"; break;
    case STRUCTCURVEMESH_E:      mesh_type = "StructCurveMesh"; break;
    case STRUCTQUADSURFMESH_E:   mesh_type = "StructQuadSurfMesh"; break;
    case STRUCTHEXVOLMESH_E:     mesh_type = "StructHexVolMesh"; break;
    case POINTCLOUDMESH_E:   mesh_type = "PointCloudMesh"; break;
    case CURVEMESH_E:        mesh_type = "CurveMesh"; break;
    case TRISURFMESH_E:      mesh_type = "TriSurfMesh"; break;
    case QUADSURFMESH_E:     mesh_type = "QuadSurfMesh"; break;
    case TETVOLMESH_E:       mesh_type = "TetVolMesh"; break;
    case PRISMVOLMESH_E:     mesh_type = "PrismVolMesh"; break;
    case HEXVOLMESH_E:       mesh_type = "HexVolMesh"; break;
    default: BOOST_THROW_EXCEPTION(UnknownMeshType() << Core::ErrorMessage("Unknown mesh type"));
  }

  if (type == SCANLINEMESH_E)
  {
    if (mesh_basis_type.find("Crv") == std::string::npos)
    {
      set_mesh_basis_type("CrvLinearLgn");
    }
    if (basis_type.find("Crv") == std::string::npos)
    {
      if (basis_type.find("uadraticLgn") != std::string::npos) set_basis_type("CrvQuadraticLgn");
      else if (basis_type.find("ubicHmt") != std::string::npos) set_basis_type("CrvCubicHmt");
      else if (basis_type.find("NoData") != std::string::npos) set_basis_type("NoDataBasis");
      else if (basis_type.find("Constant") != std::string::npos) set_basis_type("ConstantBasis");
      else set_basis_type("CrvLinearLgn");
    }
    set_container_type("vector");
  }
  if (type == IMAGEMESH_E)
  {
    if (mesh_basis_type.find("Quad") == std::string::npos)
    {
      set_mesh_basis_type("QuadBilinearLgn");
    }
    if (basis_type.find("Quad") == std::string::npos)
    {
      if (basis_type.find("uadraticLgn") != std::string::npos) set_basis_type("QuadBiquadraticLgn");
      else if (basis_type.find("ubicHmt") != std::string::npos) set_basis_type("QuadBicubicHmt");
      else if (basis_type.find("NoData") != std::string::npos) set_basis_type("NoDataBasis");
      else if (basis_type.find("Constant") != std::string::npos) set_basis_type("ConstantBasis");
      else set_basis_type("QuadBilinearLgn");
    }
    set_container_type("FData2d");
  }
  if (type == LATVOLMESH_E)
  {
    if (mesh_basis_type.find("Hex") == std::string::npos)
    {
      set_mesh_basis_type("HexTrilinearLgn");
    }
    if (basis_type.find("Hex") == std::string::npos)
    {
      if (basis_type.find("uadraticLgn") != std::string::npos) set_basis_type("HexTriquadraticLgn");
      else if (basis_type.find("ubicHmt") != std::string::npos) set_basis_type("HexTricubicHmt");
      else if (basis_type.find("NoData") != std::string::npos) set_basis_type("NoDataBasis");
      else if (basis_type.find("Constant") != std::string::npos) set_basis_type("ConstantBasis");
      else set_basis_type("HexTrilinearLgn");
    }
    set_container_type("FData3d");
  }
  if (type == STRUCTCURVEMESH_E)
  {
    if (mesh_basis_type.find("Crv") == std::string::npos)
    {
      if (mesh_basis_type.find("uadraticLgn") != std::string::npos) set_mesh_basis_type("CrvQuadraticLgn");
      else if (mesh_basis_type.find("ubicHmt") != std::string::npos) set_mesh_basis_type("CrvCubicHmt");
      else if (mesh_basis_type.find("NoData") != std::string::npos) set_mesh_basis_type("CrvLinearLgn");
      else if (mesh_basis_type.find("Constant") != std::string::npos) set_mesh_basis_type("CrvLinearLgn");
      else set_mesh_basis_type("CrvLinearLgn");
    }
    if (basis_type.find("Crv") == std::string::npos)
    {
      if (basis_type.find("uadraticLgn") != std::string::npos) set_basis_type("CrvQuadraticLgn");
      else if (basis_type.find("ubicHmt") != std::string::npos) set_basis_type("CrvCubicHmt");
      else if (basis_type.find("NoData") != std::string::npos) set_basis_type("NoDataBasis");
      else if (basis_type.find("Constant") != std::string::npos) set_basis_type("ConstantBasis");
      else set_basis_type("CrvLinearLgn");
    }
    set_container_type("vector");
  }

  if (type == STRUCTQUADSURFMESH_E)
  {
    if (mesh_basis_type.find("Quad") == std::string::npos)
    {
      if (mesh_basis_type.find("uadraticLgn") != std::string::npos) set_mesh_basis_type("QuadBiquadraticLgn");
      else if (mesh_basis_type.find("ubicHmt") != std::string::npos) set_mesh_basis_type("QuadBicubicHmt");
      else if (mesh_basis_type.find("NoData") != std::string::npos) set_mesh_basis_type("QuadBilinearLgn");
      else if (mesh_basis_type.find("Constant") != std::string::npos) set_mesh_basis_type("QuadBilinearLgn");
      else set_mesh_basis_type("QuadBilinearLgn");
    }
    if (basis_type.find("Quad") == std::string::npos)
    {
      if (basis_type.find("uadraticLgn") != std::string::npos) set_basis_type("QuadBiquadraticLgn");
      else if (basis_type.find("ubicHmt") != std::string::npos) set_basis_type("QuadBicubicHmt");
      else if (basis_type.find("NoData") != std::string::npos) set_basis_type("NoDataBasis");
      else if (basis_type.find("Constant") != std::string::npos) set_basis_type("ConstantBasis");
      else set_basis_type("QuadBilinearLgn");
    }
    set_container_type("FData2d");
  }

  if (type == STRUCTHEXVOLMESH_E)
  {
    if (mesh_basis_type.find("Hex") == std::string::npos)
    {
      if (mesh_basis_type.find("uadraticLgn") != std::string::npos) set_mesh_basis_type("HexTriquadraticLgn");
      else if (mesh_basis_type.find("ubicHmt") != std::string::npos) set_mesh_basis_type("HexTricubicHmt");
      else if (mesh_basis_type.find("NoData") != std::string::npos) set_mesh_basis_type("HexTrilinearLgn");
      else if (mesh_basis_type.find("Constant") != std::string::npos) set_mesh_basis_type("HexTrilinearLgn");
      else set_mesh_basis_type("HexTrilinearLgn");
    }
    if (basis_type.find("Hex") == std::string::npos)
    {
      if (basis_type.find("uadraticLgn") != std::string::npos) set_basis_type("HexTriquadraticLgn");
      else if (basis_type.find("ubicHmt") != std::string::npos) set_basis_type("HexTricubicHmt");
      else if (basis_type.find("NoData") != std::string::npos) set_basis_type("NoDataBasis");
      else if (basis_type.find("Constant") != std::string::npos) set_basis_type("ConstantBasis");
      else set_basis_type("HexTrilinearLgn");
    }
    set_container_type("FData3d");
  }

  if (type == CURVEMESH_E)
  {
    if (mesh_basis_type.find("Crv") == std::string::npos)
    {
      if (mesh_basis_type.find("uadraticLgn") != std::string::npos) set_mesh_basis_type("CrvQuadraticLgn");
      else if (mesh_basis_type.find("ubicHmt") != std::string::npos) set_mesh_basis_type("CrvCubicHmt");
      else if (mesh_basis_type.find("NoData") != std::string::npos) set_mesh_basis_type("CrvLinearLgn");
      else if (mesh_basis_type.find("Constant") != std::string::npos) set_mesh_basis_type("CrvLinearLgn");
      else set_mesh_basis_type("CrvLinearLgn");
    }
    if (basis_type.find("Crv") == std::string::npos)
    {
      if (basis_type.find("uadraticLgn") != std::string::npos) set_basis_type("CrvQuadraticLgn");
      else if (basis_type.find("ubicHmt") != std::string::npos) set_basis_type("CrvCubicHmt");
      else if (basis_type.find("NoData") != std::string::npos) set_basis_type("NoDataBasis");
      else if (basis_type.find("Constant") != std::string::npos) set_basis_type("ConstantBasis");
      else set_basis_type("CrvLinearLgn");
    }
    set_container_type("vector");
  }

  if (type == TRISURFMESH_E)
  {
    if (mesh_basis_type.find("uadraticLgn") != std::string::npos) set_mesh_basis_type("TriQuadraticLgn");
    else if (mesh_basis_type.find("ubicHmt") != std::string::npos) set_mesh_basis_type("TriCubicHmt");
    else if (mesh_basis_type.find("NoData") != std::string::npos) set_mesh_basis_type("TriLinearLgn");
    else if (mesh_basis_type.find("Constant") != std::string::npos) set_mesh_basis_type("TriLinearLgn");
    else set_mesh_basis_type("TriLinearLgn");

    if (basis_type.find("uadraticLgn") != std::string::npos) set_basis_type("TriQuadraticLgn");
    else if (basis_type.find("ubicHmt") != std::string::npos) set_basis_type("TriCubicHmt");
    else if (basis_type.find("NoData") != std::string::npos) set_basis_type("NoDataBasis");
    else if (basis_type.find("Constant") != std::string::npos) set_basis_type("ConstantBasis");
    else set_basis_type("TriLinearLgn");

    set_container_type("vector");
  }
  if (type == QUADSURFMESH_E)
  {
    if (mesh_basis_type.find("Quad") == std::string::npos)
    {
      if (mesh_basis_type.find("uadraticLgn") != std::string::npos) set_mesh_basis_type("QuadBiquadraticLgn");
      else if (mesh_basis_type.find("ubicHmt") != std::string::npos) set_mesh_basis_type("QuadBicubicHmt");
      else if (mesh_basis_type.find("NoData") != std::string::npos) set_mesh_basis_type("QuadBilinearLgn");
      else if (mesh_basis_type.find("Constant") != std::string::npos) set_mesh_basis_type("QuadBilinearLgn");
      else set_mesh_basis_type("QuadBilinearLgn");
    }
    if (basis_type.find("Quad") == std::string::npos)
    {
      if (basis_type.find("uadraticLgn") != std::string::npos) set_basis_type("QuadBiquadraticLgn");
      else if (basis_type.find("ubicHmt") != std::string::npos) set_basis_type("QuadBicubicHmt");
      else if (basis_type.find("NoData") != std::string::npos) set_basis_type("NoDataBasis");
      else if (basis_type.find("Constant") != std::string::npos) set_basis_type("ConstantBasis");
      else set_basis_type("QuadBilinearLgn");
    }
    set_container_type("vector");
  }

  if (type == TETVOLMESH_E)
  {
    if (mesh_basis_type.find("Tet") == std::string::npos)
    {
      if (mesh_basis_type.find("uadraticLgn") != std::string::npos) set_mesh_basis_type("TetQuadraticLgn");
      else if (mesh_basis_type.find("ubicHmt") != std::string::npos) set_mesh_basis_type("TetCubicHmt");
      else if (mesh_basis_type.find("NoData") != std::string::npos) set_mesh_basis_type("TetLinearLgn");
      else if (mesh_basis_type.find("Constant") != std::string::npos) set_mesh_basis_type("TetLinearLgn");
      else set_mesh_basis_type("TetLinearLgn");
    }
    if (basis_type.find("Tet") == std::string::npos)
    {
      if (basis_type.find("uadraticLgn") != std::string::npos) set_basis_type("TetQuadraticLgn");
      else if (basis_type.find("ubicHmt") != std::string::npos) set_basis_type("TetCubicHmt");
      else if (basis_type.find("NoData") != std::string::npos) set_basis_type("NoDataBasis");
      else if (basis_type.find("Constant") != std::string::npos) set_basis_type("ConstantBasis");
      else set_basis_type("TetLinearLgn");
    }
    set_container_type("vector");
  }

  if (type == PRISMVOLMESH_E)
  {
    if (mesh_basis_type.find("Prism") == std::string::npos)
    {
      if (mesh_basis_type.find("uadraticLgn") != std::string::npos) set_mesh_basis_type("PrismQuadraticLgn");
      else if (mesh_basis_type.find("ubicHmt") != std::string::npos) set_mesh_basis_type("PrismCubicHmt");
      else if (mesh_basis_type.find("NoData") != std::string::npos) set_mesh_basis_type("PrismLinearLgn");
      else if (mesh_basis_type.find("Constant") != std::string::npos) set_mesh_basis_type("PrismLinearLgn");
      else set_mesh_basis_type("PrismLinearLgn");
    }
    if (basis_type.find("Prism") == std::string::npos)
    {
      if (basis_type.find("uadraticLgn") != std::string::npos) set_basis_type("PrismQuadraticLgn");
      else if (basis_type.find("ubicHmt") != std::string::npos) set_basis_type("PrismCubicHmt");
      else if (basis_type.find("NoData") != std::string::npos) set_basis_type("NoDataBasis");
      else if (basis_type.find("Constant") != std::string::npos) set_basis_type("ConstantBasis");
      else set_basis_type("PrismLinearLgn");
    }
    set_container_type("vector");
  }

  if (type == HEXVOLMESH_E)
  {
    if (mesh_basis_type.find("Hex") == std::string::npos)
    {
      if (mesh_basis_type.find("uadraticLgn") != std::string::npos) set_mesh_basis_type("HexTriquadraticLgn");
      else if (mesh_basis_type.find("ubicHmt") != std::string::npos) set_mesh_basis_type("HexTricubicHmt");
      else if (mesh_basis_type.find("NoData") != std::string::npos) set_mesh_basis_type("HexTrilinearLgn");
      else if (mesh_basis_type.find("Constant") != std::string::npos) set_mesh_basis_type("HexTrilinearLgn");
      else set_mesh_basis_type("HexTrilinearLgn");
    }
    if (basis_type.find("Hex") == std::string::npos)
    {
      if (basis_type.find("uadraticLgn") != std::string::npos) set_basis_type("HexTriquadraticLgn");
      else if (basis_type.find("ubicHmt") != std::string::npos) set_basis_type("HexTricubicHmt");
      else if (basis_type.find("NoData") != std::string::npos) set_basis_type("NoDataBasis");
      else if (basis_type.find("Constant") != std::string::npos) set_basis_type("ConstantBasis");
      else set_basis_type("HexTrilinearLgn");
    }
    set_container_type("vector");
  }

  if (type == POINTCLOUDMESH_E)
  {
    if (mesh_basis_type.find("NoData") != std::string::npos) set_mesh_basis_type("ConstantBasis");
    else set_mesh_basis_type("ConstantBasis");

    if (basis_type.find("NoData") != std::string::npos) set_basis_type("NoDataBasis");
    else set_basis_type("ConstantBasis");
    set_container_type("vector");
  }
}


std::string
FieldInformation::get_mesh_basis_type() const
{
  return(mesh_basis_type);
}

void
FieldInformation::set_mesh_basis_type(int order)
{
  if (order == 1) set_mesh_basis_type("linear");
  if (order == 2) set_mesh_basis_type("quadratic");
  if (order == 3) set_mesh_basis_type("cubic");
}

void
FieldInformation::set_mesh_basis_type(const std::string& type)
{
  if (type == "Linear" || type == "linear" || type == "linearmesh" || type == "LinearMesh" )
    { make_linearmesh(); return; }
  if (type == "Quadratic" || type == "quadratic" || type == "quadraticmesh" || type == "QuadraticMesh")
    { make_quadraticmesh(); return; }
  if (type == "Cubic" || type == "cubic" || type == "cubicmesh" || type == "CubicMesh")
    { make_cubicmesh(); return; }
  mesh_basis_type = type;
}

std::string
FieldInformation::get_point_type() const
{
  return(point_type);
}

void
FieldInformation::set_point_type(const std::string& type)
{
  point_type = type;
}


std::string
FieldInformation::get_basis_type() const
{
  return(basis_type);
}

void
FieldInformation::set_basis_type(int order)
{
  if (order == -1) set_basis_type("nodata");
  if (order == 0) set_basis_type("constant");
  if (order == 1) set_basis_type("linear");
  if (order == 2) set_basis_type("quadratic");
  if (order == 3) set_basis_type("cubic");
}


void
FieldInformation::set_basis_type(const std::string& type)
{
  if (type == "NoData" || type == "nodata" || type == "No" || type == "no")
    { make_nodata(); return; }
  if (type == "Constant" || type == "constant"|| type == "ConstantData" || type == "constantdata")
    { make_constantdata(); return; }
  if (type == "Linear" || type == "linear" || type == "LinearData" || type == "lineardata")
    { make_lineardata(); return; }
  if (type == "Quadratic" || type == "quadratic" || type == "QuadraticData" || type == "quadraticdata")
    { make_quadraticdata(); return; }
  if (type == "Cubic" || type == "cubic" || type == "CubicData" || type == "cubicdata")
    { make_cubicdata(); return; }

  basis_type = type;
}


std::string
FieldInformation::get_data_type() const
{
  return(data_type);
}

void
FieldInformation::set_data_type(const std::string& type1)
{
  std::string type = type1;
  if (type == "nodata") type = "double";
  if (type == "vector") type = "Vector";
  if (type == "tensor") type = "Tensor";
  if (type == "scalar") type = "double";
  if (type == "Scalar") type = "double";

  // be compatible with old dynamic compilation rules
  for (size_t j=0;j<type.size(); j++) if (type[j]==' ') type[j] = '_';

  data_type = type;
}

bool
FieldInformation::set_data_type_by_string(const std::string& type)
{
  if (type == "char")
  {
    return make_char();
  }
  else if (type == "unsigned char")
  {
    return make_unsigned_char();
  }
  else if (type == "short")
  {
    return make_short();
  }
  else if (type == "unsigned short")
  {
    return make_unsigned_short();
  }
  else if (type == "int")
  {
    return make_int();
  }
  else if (type == "unsigned int")
  {
    return make_unsigned_int();
  }
  else if (type == "long long")
  {
    return make_long_long();
  }
  else if (type == "unsigned long long")
  {
    return make_unsigned_long_long();
  }
  else if (type == "float")
  {
    return make_float();
  }
  else if (type == "double")
  {
    return make_double();
  }
  else if (type == "Vector")
  {
    return make_vector();
  }
  else if (type == "Tensor")
  {
    return make_tensor();
  }
  else
  {
    BOOST_THROW_EXCEPTION(UnknownMeshType() << Core::ErrorMessage("INTERNAL ERROR - unknown type"));
  }
}

void
FieldInformation::set_data_type(data_info_type type)
{
  switch (type)
  {
    case NONE_E:                data_type = "double"; break;
    case CHAR_E:                data_type = "char"; break;
    case UNSIGNED_CHAR_E:       data_type = "unsigned_char"; break;
    case SHORT_E:               data_type = "short"; break;
    case UNSIGNED_SHORT_E:      data_type = "unsigned_short"; break;
    case INT_E:                 data_type = "int"; break;
    case UNSIGNED_INT_E:        data_type = "unsigned_int"; break;
    case LONGLONG_E:            data_type = "long_long"; break;
    case UNSIGNED_LONGLONG_E:   data_type = "unsigned_long_long"; break;
    case FLOAT_E:               data_type = "float"; break;
    case DOUBLE_E:              data_type = "double"; break;
    case VECTOR_E:              data_type = "Vector"; break;
    case TENSOR_E:              data_type = "Tensor"; break;

    default: BOOST_THROW_EXCEPTION(UnknownMeshType() << Core::ErrorMessage("INTERNAL ERROR - unknown data_info_type"));
  }
}


std::string
FieldInformation::get_container_type() const
{
  return(container_type);
}

void
FieldInformation::set_container_type(const std::string& type)
{
  container_type = type;
}

std::string
FieldInformation::get_field_type_id() const
{
  // Deal with some SCIRun design flaw
  std::string meshptr = "";
  if ((container_type.find("2d") != std::string::npos)||(container_type.find("3d") != std::string::npos))
    meshptr = "," + mesh_type + "<" + mesh_basis_type + "<" + point_type + ">" + ">";

  std::string field_template = field_type + "<" + mesh_type + "<" +
    mesh_basis_type + "<" + point_type + ">" + ">" + "," +
    basis_type + "<" + data_type + ">" + "," + container_type + "<" +
    data_type + meshptr + ">" + ">";

  for (std::string::size_type r=0; r< field_template.size(); r++) if (field_template[r] == ' ') field_template[r] = '_';

  return(field_template);
}


std::string
FieldInformation::get_mesh_type_id() const
{
  std::string mesh_template =  mesh_type + "<" + mesh_basis_type + "<" + point_type + ">" + ">";

  for (std::string::size_type r=0; r< mesh_template.size(); r++) if (mesh_template[r] == ' ') mesh_template[r] = '_';

  return(mesh_template);
}


bool
FieldTypeInformation::is_isomorphic() const
{
  return((mesh_basis_type == basis_type));
}

bool
FieldTypeInformation::is_nonlinear() const
{
  return((is_nonlineardata())||(is_nonlinearmesh()));
}

bool
FieldTypeInformation::is_linear() const
{
  return((is_lineardata())&&(is_linearmesh()));
}

bool
FieldTypeInformation::is_nodata() const
{
  return((basis_type == "NoDataBasis"));
}

bool
FieldTypeInformation::is_constantdata() const
{
  return((basis_type == "ConstantBasis"));
}

bool
FieldTypeInformation::is_lineardata() const
{
  return((basis_type.find("inear") != std::string::npos));
}

bool
FieldTypeInformation::is_nonlineardata() const
{
  return( (basis_type.find("uadratic") != std::string::npos)||
          (basis_type.find("ubicHmt") != std::string::npos));
}

bool
FieldTypeInformation::is_quadraticdata() const
{
  return ((basis_type.find("uadratic") != std::string::npos));
}

bool
FieldTypeInformation::is_cubicdata() const
{
  return ((basis_type.find("ubicHmt") != std::string::npos));
}

/// @todo: an enum would be good for keeping track of basis order...
int
FieldTypeInformation::field_basis_order() const
{
  if (is_nodata()) return (-1);
  if (is_constantdata()) return (0);
  if (is_lineardata()) return (1);
  if (is_quadraticdata()) return (2);
  if (is_cubicdata()) return (3);
  return (-1);
}


bool
FieldTypeInformation::is_constantmesh() const
{
  return((mesh_basis_type == "ConstantBasis"));
}

bool
FieldTypeInformation::is_linearmesh() const
{
  return((mesh_basis_type.find("inear") != std::string::npos));
}

bool
FieldTypeInformation::is_nonlinearmesh() const
{
  return( (mesh_basis_type.find("uadratic") != std::string::npos)||
          (mesh_basis_type.find("ubicHmt") != std::string::npos));
}

bool
FieldTypeInformation::is_quadraticmesh() const
{
  return ((mesh_basis_type.find("uadratic") != std::string::npos));
}

bool
FieldTypeInformation::is_cubicmesh() const
{
  return ((mesh_basis_type.find("ubicHmt") != std::string::npos));
}

int
FieldTypeInformation::mesh_basis_order() const
{
  if (is_constantmesh()) return (0);
  if (is_linearmesh()) return (1);
  if (is_quadraticmesh()) return (2);
  if (is_cubicmesh()) return (3);
  return (-1);
}

bool
FieldTypeInformation::is_tensor() const
{
  return ((data_type == "Tensor"));
}

bool
FieldTypeInformation::is_vector() const
{
  return ((data_type == "Vector"));
}

bool
FieldTypeInformation::is_scalar() const
{
  return((!is_tensor())&&(!is_vector())&&(data_type!=""));
}

bool
FieldTypeInformation::is_integer() const
{
  return(is_char()||is_short()||is_int()||is_long()||is_longlong()
      ||is_unsigned_char()||is_unsigned_short()
      ||is_unsigned_int()||is_unsigned_long()
      ||is_unsigned_longlong());
}

bool
FieldTypeInformation::is_double() const
{
  return((data_type == "double"));
}

bool
FieldTypeInformation::is_float() const
{
  return((data_type == "float"));
}

bool
FieldTypeInformation::is_longlong() const
{
  return((data_type == "long_long")||(data_type == "signed_long_long"));
}

bool
FieldTypeInformation::is_long() const
{
  return((data_type == "long")||(data_type == "signed_long"));
}

bool
FieldTypeInformation::is_int() const
{
  return((data_type == "int")||(data_type == "signed_int"));
}

bool
FieldTypeInformation::is_short() const
{
  return((data_type == "short")||(data_type == "signed_short"));
}

bool
FieldTypeInformation::is_char() const
{
  return((data_type == "char")||(data_type == "signed_char"));
}

bool
FieldTypeInformation::is_unsigned_longlong() const
{
  return(data_type == "unsigned_long_long");
}

bool
FieldTypeInformation::is_unsigned_long() const
{
  return(data_type == "unsigned_long");
}

bool
FieldTypeInformation::is_unsigned_int() const
{
  return(data_type == "unsigned_int");
}

bool
FieldTypeInformation::is_unsigned_short() const
{
  return(data_type == "unsigned_short");
}

bool
FieldTypeInformation::is_unsigned_char() const
{
  return(data_type == "unsigned_char");
}

bool
FieldTypeInformation::is_dvt() const
{
  return(is_double()||is_vector()||is_tensor());
}

bool
FieldTypeInformation::is_svt() const
{
  return(is_scalar()||is_vector()||is_tensor());
}


bool
FieldTypeInformation::is_structuredmesh() const
{
  return((mesh_type.find("Struct")!=std::string::npos) ||
	 (mesh_type=="ScanlineMesh")||
	 (mesh_type=="ImageMesh")||
	 (mesh_type=="LatVolMesh"));
}

bool
FieldTypeInformation::is_regularmesh() const
{
  return((mesh_type=="ScanlineMesh")||
	 (mesh_type=="ImageMesh")||
	 (mesh_type=="LatVolMesh"));
}

bool
FieldTypeInformation::is_irregularmesh() const
{
  return(!is_regularmesh());
}

bool
FieldTypeInformation::is_unstructuredmesh() const
{
  return(!is_structuredmesh());
}

bool
FieldTypeInformation::is_pnt_element() const
{
  return((mesh_type=="PointCloud"));
}

bool
FieldTypeInformation::is_crv_element() const
{
  return((mesh_type=="CurveMesh")||
	 (mesh_type=="StructCurveMesh")||
	 (mesh_type=="ScanlineMesh"));
}

bool
FieldTypeInformation::is_tri_element() const
{
  return((mesh_type=="TriSurfMesh"));
}

bool
FieldTypeInformation::is_quad_element() const
{
  return((mesh_type=="QuadSurfMesh")||
	 (mesh_type=="ImageMesh")||
	 (mesh_type=="StructQuadSurfMesh"));
}

bool
FieldTypeInformation::is_tet_element() const
{
  return((mesh_type=="TetVolMesh"));
}

bool
FieldTypeInformation::is_prism_element() const
{
  return((mesh_type=="PrismVolMesh"));
}

bool
FieldTypeInformation::is_hex_element() const
{
  return((mesh_type=="HexVolMesh")||
	 (mesh_type=="StructHexVolMesh")||
	 (mesh_type=="LatVolMesh")||
	 (mesh_type=="MaskedLatVolMesh"));
}

bool
FieldTypeInformation::is_pointcloudmesh() const
{
  if (mesh_type == "PointCloudMesh") return (true);
  return false;
}

bool
FieldTypeInformation::is_scanlinemesh() const
{
  if (mesh_type == "ScanlineMesh") return (true);
  return false;
}

bool
FieldTypeInformation::is_imagemesh() const
{
  if (mesh_type == "ImageMesh") return (true);
  return false;
}

bool
FieldTypeInformation::is_latvolmesh() const
{
  if (mesh_type == "LatVolMesh") return (true);
  return false;
}

bool
FieldTypeInformation::is_curvemesh() const
{
  if (mesh_type == "CurveMesh") return (true);
  return false;
}

bool
FieldTypeInformation::is_trisurfmesh() const
{
  if (mesh_type == "TriSurfMesh") return (true);
  return false;
}

bool
FieldTypeInformation::is_quadsurfmesh() const
{
  if (mesh_type == "QuadSurfMesh") return (true);
  return false;
}

bool
FieldTypeInformation::is_tetvolmesh() const
{
  if (mesh_type == "TetVolMesh") return (true);
  return false;
}

bool
FieldTypeInformation::is_prismvolmesh() const
{
  if (mesh_type == "PrismVolMesh") return (true);
  return false;
}

bool
FieldTypeInformation::is_hexvolmesh() const
{
  if (mesh_type == "HexVolMesh") return (true);
  return false;
}

bool
FieldTypeInformation::is_structcurvemesh() const
{
  if (mesh_type == "StructCurveMesh") return (true);
  return false;
}

bool
FieldTypeInformation::is_structquadsurfmesh() const
{
  if (mesh_type == "StructQuadSurfMesh") return (true);
  return false;
}

bool
FieldTypeInformation::is_structhexvolmesh() const
{
  if (mesh_type == "StructHexVolMesh") return (true);
  return false;
}


bool
FieldTypeInformation::is_point() const
{
  if (mesh_type == "PointCloudMesh") return (true);
  return false;
}

bool
FieldTypeInformation::is_line() const
{
  if ((mesh_type == "CurveMesh")||
      (mesh_type == "ScanlineMesh")||
      (mesh_type == "StructCurveMesh")) return (true);
  return false;
}

bool
FieldTypeInformation::is_surface() const
{
  if ((mesh_type == "TriSurfMesh")||
      (mesh_type == "QuadSurfMesh")||
      (mesh_type == "ImageMesh")||
      (mesh_type == "StructQuadSurfMesh")) return (true);
  return false;
}

bool
FieldTypeInformation::is_volume() const
{
  if ((mesh_type == "TetVolMesh")||(mesh_type == "PrismVolMesh")||
      (mesh_type == "HexVolMesh")||(mesh_type == "LatVolMesh")||
      (mesh_type == "StructHexVolMesh")||(mesh_type == "MaskedLatVolMesh")) return (true);
  return false;
}


bool
FieldInformation::operator==(const FieldInformation& fi) const
{
  if ( (field_type == fi.field_type) && (mesh_type == fi.mesh_type) && (mesh_basis_type == fi.mesh_basis_type) &&
       (point_type == fi.point_type) && (basis_type == fi.basis_type) && (data_type == fi.data_type) &&
       (container_type == fi.container_type) ) return (true);
  return (false);
}


bool
FieldInformation::operator!=(const FieldInformation& fi) const
{
  if ( (field_type != fi.field_type) || (mesh_type != fi.mesh_type) || (mesh_basis_type != fi.mesh_basis_type) ||
       (point_type != fi.point_type) || (basis_type != fi.basis_type) || (data_type != fi.data_type) ||
       (container_type != fi.container_type) ) return (true);
  return (false);
}


bool
FieldInformation::make_nodata()
{
  set_basis_type("NoDataBasis");
  set_data_type("double");
  return (true);
}

bool
FieldInformation::make_constantdata()
{
  set_basis_type("ConstantBasis");
  return (true);
}

bool
FieldInformation::make_constantmesh()
{
  set_mesh_basis_type("ConstantBasis");
  return (true);
}

bool
FieldInformation::make_scanlinemesh()
{
  set_mesh_type("ScanlineMesh");
  return (true);
}

bool
FieldInformation::make_curvemesh()
{
  set_mesh_type("CurveMesh");
  return (true);
}

bool
FieldInformation::make_structcurvemesh()
{
  set_mesh_type("StructCurveMesh");
  return (true);
}

bool
FieldInformation::make_imagemesh()
{
  set_mesh_type("ImageMesh");
  return (true);
}

bool
FieldInformation::make_trisurfmesh()
{
  set_mesh_type("TriSurfMesh");
  return (true);
}

bool
FieldInformation::make_quadsurfmesh()
{
  set_mesh_type("QuadSurfMesh");
  return (true);
}

bool
FieldInformation::make_structquadsurfmesh()
{
  set_mesh_type("StructQuadSurfMesh");
  return (true);
}

bool
FieldInformation::make_latvolmesh()
{
  set_mesh_type("LatVolMesh");
  return (true);
}

bool
FieldInformation::make_tetvolmesh()
{
  set_mesh_type("TetVolMesh");
  return (true);
}

bool
FieldInformation::make_prismvolmesh()
{
  set_mesh_type("PrismVolMesh");
  return (true);
}

bool
FieldInformation::make_hexvolmesh()
{
  set_mesh_type("HexVolMesh");
  return (true);
}

bool
FieldInformation::make_structhexvolmesh()
{
  set_mesh_type("StructHexVolMesh");
  return (true);
}

bool
FieldInformation::make_pointcloudmesh()
{
  set_mesh_type("PointCloudMesh");
  return (true);
}


bool
FieldInformation::make_unstructuredmesh()
{
  if (is_pnt_element()) return(make_pointcloudmesh());
  if (is_crv_element()) return(make_curvemesh());
  if (is_tri_element()) return(make_trisurfmesh());
  if (is_quad_element()) return(make_quadsurfmesh());
  if (is_tet_element()) return(make_tetvolmesh());
  if (is_prism_element()) return(make_prismvolmesh());
  if (is_hex_element()) return(make_hexvolmesh());
  return (false);
}


bool
FieldInformation::make_irregularmesh()
{
  if (is_regularmesh())
  {
    if (is_crv_element()) return(make_structcurvemesh());
    if (is_quad_element()) return(make_structquadsurfmesh());
    if (is_hex_element()) return(make_structhexvolmesh());
  }
  return (false);
}

bool
FieldInformation::make_linearmesh()
{
  if (mesh_type == "ScanlineMesh") set_mesh_basis_type("CrvLinearLgn");
  if (mesh_type == "ImageMesh")  set_mesh_basis_type("QuadBilinearLgn");
  if (mesh_type == "LatVolMesh")  set_mesh_basis_type("HexTrilinearLgn");
  if (mesh_type == "MaskedLatVolMesh")  set_mesh_basis_type("HexTrilinearLgn");
  if (mesh_type == "StructCurveMesh") set_mesh_basis_type("CrvLinearLgn");
  if (mesh_type == "StructQuadSurfMesh") set_mesh_basis_type("QuadBilinearLgn");
  if (mesh_type == "StructHexVolMesh") set_mesh_basis_type("HexTrilinearLgn");
  if (mesh_type == "CurveMesh") set_mesh_basis_type("CrvLinearLgn");
  if (mesh_type == "TriSurfMesh") set_mesh_basis_type("TriLinearLgn");
  if (mesh_type == "QuadSurfMesh") set_mesh_basis_type("QuadBilinearLgn");
  if (mesh_type == "TetVolMesh") set_mesh_basis_type("TetLinearLgn");
  if (mesh_type == "PrismVolMesh") set_mesh_basis_type("PrismLinearLgn");
  if (mesh_type == "HexVolMesh") set_mesh_basis_type("HexTrilinearLgn");
  if (mesh_type == "PointCloudMesh") set_mesh_basis_type("ConstantBasis");
  return (true);
}


bool
FieldInformation::make_lineardata()
{
  if (mesh_type == "ScanlineMesh") set_basis_type("CrvLinearLgn");
  if (mesh_type == "ImageMesh")  set_basis_type("QuadBilinearLgn");
  if (mesh_type == "LatVolMesh")  set_basis_type("HexTrilinearLgn");
  if (mesh_type == "MaskedLatVolMesh")  set_basis_type("HexTrilinearLgn");
  if (mesh_type == "StructCurveMesh") set_basis_type("CrvLinearLgn");
  if (mesh_type == "StructQuadSurfMesh") set_basis_type("QuadBilinearLgn");
  if (mesh_type == "StructHexVolMesh") set_basis_type("HexTrilinearLgn");
  if (mesh_type == "CurveMesh") set_basis_type("CrvLinearLgn");
  if (mesh_type == "TriSurfMesh") set_basis_type("TriLinearLgn");
  if (mesh_type == "QuadSurfMesh") set_basis_type("QuadBilinearLgn");
  if (mesh_type == "TetVolMesh") set_basis_type("TetLinearLgn");
  if (mesh_type == "PrismVolMesh") set_basis_type("PrismLinearLgn");
  if (mesh_type == "HexVolMesh") set_basis_type("HexTrilinearLgn");
  if (mesh_type == "PointCloudMesh") set_basis_type("ConstantBasis");
  return (true);
}

bool
FieldInformation::make_quadraticdata()
{
  if (mesh_type == "ScanlineMesh") set_basis_type("CrvQuadraticLgn");
  if (mesh_type == "ImageMesh")  set_basis_type("QuadBiquadraticLgn");
  if (mesh_type == "LatVolMesh")  set_basis_type("HexTriquadraticLgn");
  if (mesh_type == "MaskedLatVolMesh")  set_basis_type("HexTriquadraticLgn");
  if (mesh_type == "StructCurveMesh") set_basis_type("CrvQuadraticLgn");
  if (mesh_type == "StructQuadSurfMesh") set_basis_type("QuadBiquadraticLgn");
  if (mesh_type == "StructHexVolMesh") set_basis_type("HexTriquadraticLgn");
  if (mesh_type == "CurveMesh") set_basis_type("CrvQuadraticLgn");
  if (mesh_type == "TriSurfMesh") set_basis_type("TriQuadraticLgn");
  if (mesh_type == "QuadSurfMesh") set_basis_type("QuadBilinearLgn");
  if (mesh_type == "TetVolMesh") set_basis_type("TetQuadraticLgn");
  if (mesh_type == "PrismVolMesh") set_basis_type("PrismQuadraticLgn");
  if (mesh_type == "HexVolMesh") set_basis_type("HexTriquadraticLgn");
  if (mesh_type == "PointCloudMesh") set_basis_type("ConstantBasis");
  return (true);
}

bool
FieldInformation::make_quadraticmesh()
{
  if (mesh_type == "ScanlineMesh") set_mesh_basis_type("CrvQuadraticLgn");
  if (mesh_type == "ImageMesh")  set_mesh_basis_type("QuadBiquadraticLgn");
  if (mesh_type == "LatVolMesh")  set_mesh_basis_type("HexTriquadraticLgn");
  if (mesh_type == "MaskedLatVolMesh")  set_mesh_basis_type("HexTriquadraticLgn");
  if (mesh_type == "StructCurveMesh") set_mesh_basis_type("CrvQuadraticLgn");
  if (mesh_type == "StructQuadSurfMesh") set_mesh_basis_type("QuadBiquadraticLgn");
  if (mesh_type == "StructHexVolMesh") set_mesh_basis_type("HexTriquadraticLgn");
  if (mesh_type == "CurveMesh") set_mesh_basis_type("CrvQuadraticLgn");
  if (mesh_type == "TriSurfMesh") set_mesh_basis_type("TriQuadraticLgn");
  if (mesh_type == "QuadSurfMesh") set_mesh_basis_type("QuadBilinearLgn");
  if (mesh_type == "TetVolMesh") set_mesh_basis_type("TetQuadraticLgn");
  if (mesh_type == "PrismVolMesh") set_mesh_basis_type("PrismQuadraticLgn");
  if (mesh_type == "HexVolMesh") set_mesh_basis_type("HexTriquadraticLgn");
  if (mesh_type == "PointCloudMesh") set_mesh_basis_type("ConstantBasis");
  return (true);
}

bool
FieldInformation::make_cubicdata()
{
  if (mesh_type == "ScanlineMesh") set_basis_type("CrvCubicHmt");
  if (mesh_type == "ImageMesh")  set_basis_type("QuadBicubicHmt");
  if (mesh_type == "LatVolMesh")  set_basis_type("HexTricubicHmt");
  if (mesh_type == "MaskedLatVolMesh")  set_basis_type("HexTriquadraticLgn");
  if (mesh_type == "StructCurveMesh") set_basis_type("CrvCubicHmt");
  if (mesh_type == "StructQuadSurfMesh") set_basis_type("QuadBicubicHmt");
  if (mesh_type == "StructHexVolMesh") set_basis_type("HexTricubicHmt");
  if (mesh_type == "CurveMesh") set_basis_type("CrvCubicHmt");
  if (mesh_type == "TriSurfMesh") set_basis_type("TriCubicHmt");
  if (mesh_type == "QuadSurfMesh") set_basis_type("QuadBicubicHmt");
  if (mesh_type == "TetVolMesh") set_basis_type("TetCubicHmt");
  if (mesh_type == "PrismVolMesh") set_basis_type("PrismCubicHmt");
  if (mesh_type == "HexVolMesh") set_basis_type("HexTricubicHmt");
  if (mesh_type == "PointCloudMesh") set_basis_type("ConstantBasis");
  return (true);
}


bool
FieldInformation::make_cubicmesh()
{
  if (mesh_type == "ScanlineMesh") set_mesh_basis_type("CrvCubicHmt");
  if (mesh_type == "ImageMesh")  set_mesh_basis_type("QuadBicubicHmt");
  if (mesh_type == "LatVolMesh")  set_mesh_basis_type("HexTricubicHmt");
  if (mesh_type == "MaskedLatVolMesh")  set_mesh_basis_type("HexTriquadraticLgn");
  if (mesh_type == "StructCurveMesh") set_mesh_basis_type("CrvCubicHmt");
  if (mesh_type == "StructQuadSurfMesh") set_mesh_basis_type("QuadBicubicHmt");
  if (mesh_type == "StructHexVolMesh") set_mesh_basis_type("HexTricubicHmt");
  if (mesh_type == "CurveMesh") set_mesh_basis_type("CrvCubicHmt");
  if (mesh_type == "TriSurfMesh") set_mesh_basis_type("TriCubicHmt");
  if (mesh_type == "QuadSurfMesh") set_mesh_basis_type("QuadBicubicHmt");
  if (mesh_type == "TetVolMesh") set_mesh_basis_type("TetCubicHmt");
  if (mesh_type == "PrismVolMesh") set_mesh_basis_type("PrismCubicHmt");
  if (mesh_type == "HexVolMesh") set_mesh_basis_type("HexTricubicHmt");
  if (mesh_type == "PointCloudMesh") set_mesh_basis_type("ConstantBasis");
  return (true);
}

bool
FieldInformation::make_scalar()
{
  set_data_type("double");
  return (true);
}

bool
FieldInformation::make_char()
{
  set_data_type("char");
  return (true);
}

bool
FieldInformation::make_unsigned_char()
{
  set_data_type("unsigned_char");
  return (true);
}

bool
FieldInformation::make_short()
{
  set_data_type("short");
  return (true);
}

bool
FieldInformation::make_unsigned_short()
{
  set_data_type("unsigned_short");
  return (true);
}

bool
FieldInformation::make_int()
{
  set_data_type("int");
  return (true);
}

bool
FieldInformation::make_unsigned_int()
{
  set_data_type("unsigned_int");
  return (true);
}

bool
FieldInformation::make_long()
{
  set_data_type("long");
  return (true);
}

bool
FieldInformation::make_unsigned_long()
{
  set_data_type("unsigned_long");
  return (true);
}

bool
FieldInformation::make_long_long()
{
  set_data_type("long_long");
  return (true);
}

bool
FieldInformation::make_unsigned_long_long()
{
  set_data_type("unsigned_long_long");
  return (true);
}

bool
FieldInformation::make_float()
{
  set_data_type("float");
  return (true);
}

bool
FieldInformation::make_double()
{
  set_data_type("double");
  return (true);
}

bool
FieldInformation::make_vector()
{
  set_data_type("Vector");
  return (true);
}

bool
FieldInformation::make_tensor()
{
  set_data_type("Tensor");
  return (true);
}

FieldHandle SCIRun::CreateField(const std::string& meshtype,
                         const std::string& basistype, const std::string& datatype)
{
  FieldInformation fi(meshtype,basistype,datatype);
  return(CreateField(fi));
}

FieldHandle SCIRun::CreateField(const std::string& meshtype, const std::string& meshbasistype,
                         const std::string& databasistype, const std::string& datatype)
{
  FieldInformation fi(meshtype,meshbasistype,databasistype,datatype);
  return(CreateField(fi));
}

FieldHandle SCIRun::CreateField(mesh_info_type mesh,
                        meshbasis_info_type meshbasis,
                        databasis_info_type databasis,
                        data_info_type data)
{
  FieldInformation fi(mesh,meshbasis,databasis,data);
  return(CreateField(fi));
}

FieldHandle SCIRun::CreateField(mesh_info_type mesh,
                        databasis_info_type databasis,
                        data_info_type data)
{
  FieldInformation fi(mesh,databasis,data);
  return(CreateField(fi));
}

FieldHandle
SCIRun::CreateField(FieldInformation &info)
{
  if (info.is_nodata()) info.set_data_type("double");
  std::string type = info.get_field_type_id();
  std::string meshtype = info.get_mesh_type_id();

  MeshHandle meshhandle = CreateMesh(meshtype);

  if (!meshhandle) return FieldHandle();

  return (CreateField(type,meshhandle));
}

FieldHandle
SCIRun::CreateField(FieldInformation &info, MeshHandle mesh)
{
  if (info.is_nodata()) info.set_data_type("double");
  std::string type = info.get_field_type_id();
  return (CreateField(type,mesh));
}

MeshHandle
SCIRun::CreateMesh(FieldInformation &info)
{
  std::string type = info.get_mesh_type_id();
  return (CreateMesh(type));
}

MeshHandle
SCIRun::CreateMesh(FieldInformation &info,size_type x)
{
  std::string type = info.get_mesh_type_id();
  return (CreateMesh(type,x));
}

MeshHandle
SCIRun::CreateMesh(FieldInformation &info,size_type x,const Point& min,const Point& max)
{
  std::string type = info.get_mesh_type_id();
  return (CreateMesh(type,x,min,max));
}

MeshHandle
SCIRun::CreateMesh(FieldInformation &info,size_type x,size_type y)
{
  std::string type = info.get_mesh_type_id();
  return (CreateMesh(type,x,y));
}

MeshHandle
SCIRun::CreateMesh(FieldInformation &info,size_type x,size_type y,const Point& min,const Point& max)
{
  std::string type = info.get_mesh_type_id();
  return (CreateMesh(type,x,y,min,max));
}

MeshHandle
SCIRun::CreateMesh(FieldInformation &info,size_type x,size_type y,size_type z)
{
  std::string type = info.get_mesh_type_id();
  return (CreateMesh(type,x,y,z));
}

MeshHandle
SCIRun::CreateMesh(FieldInformation &info,size_type x,size_type y,size_type z,const Point& min,const Point& max)
{
  std::string type = info.get_mesh_type_id();
  return (CreateMesh(type,x,y,z,min,max));
}

MeshHandle
SCIRun::CreateMesh(FieldInformation &info, const std::vector<size_type>& dim)
{
  std::string type = info.get_mesh_type_id();
  if (dim.size() == 1) return (CreateMesh(type,dim[0]));
  if (dim.size() == 2) return (CreateMesh(type,dim[0],dim[1]));
  if (dim.size() == 3) return (CreateMesh(type,dim[0],dim[1],dim[2]));
  else return (MeshHandle());
}

MeshHandle
SCIRun::CreateMesh(FieldInformation &info, const std::vector<size_type>& dim,const Point& min,const Point& max)
{
  std::string type = info.get_mesh_type_id();
  if (dim.size() == 1) return (CreateMesh(type,dim[0],min,max));
  if (dim.size() == 2) return (CreateMesh(type,dim[0],dim[1],min,max));
  if (dim.size() == 3) return (CreateMesh(type,dim[0],dim[1],dim[2],min,max));
  else return (MeshHandle());
}


MeshHandle
SCIRun::CreateMesh(mesh_info_type mesh)
{
  FieldInformation info(mesh,LINEARDATA_E,DOUBLE_E);
  std::string type = info.get_mesh_type_id();
  return (CreateMesh(type));
}

MeshHandle
SCIRun::CreateMesh(mesh_info_type mesh,size_type x)
{
  FieldInformation info(mesh,LINEARDATA_E,DOUBLE_E);
  std::string type = info.get_mesh_type_id();
  return (CreateMesh(type,x));
}

MeshHandle
SCIRun::CreateMesh(mesh_info_type mesh,size_type x,const Point& min,const Point& max)
{
  FieldInformation info(mesh,LINEARDATA_E,DOUBLE_E);
  std::string type = info.get_mesh_type_id();
  return (CreateMesh(type,x,min,max));
}

MeshHandle
SCIRun::CreateMesh(mesh_info_type mesh,size_type x,size_type y)
{
  FieldInformation info(mesh,LINEARDATA_E,DOUBLE_E);
  std::string type = info.get_mesh_type_id();
  return (CreateMesh(type,x,y));
}

MeshHandle
SCIRun::CreateMesh(mesh_info_type mesh,size_type x,size_type y,const Point& min,const Point& max)
{
  FieldInformation info(mesh,LINEARDATA_E,DOUBLE_E);
  std::string type = info.get_mesh_type_id();
  return (CreateMesh(type,x,y,min,max));
}

MeshHandle
SCIRun::CreateMesh(mesh_info_type mesh,size_type x,size_type y,size_type z)
{
  FieldInformation info(mesh,LINEARDATA_E,DOUBLE_E);
  std::string type = info.get_mesh_type_id();
  return (CreateMesh(type,x,y,z));
}

MeshHandle
SCIRun::CreateMesh(mesh_info_type mesh,size_type x,size_type y,size_type z,const Point& min,const Point& max)
{
  FieldInformation info(mesh,LINEARDATA_E,DOUBLE_E);
  std::string type = info.get_mesh_type_id();
  return (CreateMesh(type,x,y,z,min,max));
}

MeshHandle
SCIRun::CreateMesh(mesh_info_type mesh,const std::vector<size_type>& dim)
{
  FieldInformation info(mesh,LINEARDATA_E,DOUBLE_E);
  std::string type = info.get_mesh_type_id();
  if (dim.size() == 1) return (CreateMesh(type,dim[0]));
  if (dim.size() == 2) return (CreateMesh(type,dim[0],dim[1]));
  if (dim.size() == 3) return (CreateMesh(type,dim[0],dim[1],dim[2]));
  else return (MeshHandle());
}

MeshHandle
SCIRun::CreateMesh(mesh_info_type mesh,const std::vector<size_type>& dim,const Point& min,const Point& max)
{
  FieldInformation info(mesh,LINEARDATA_E,DOUBLE_E);
  std::string type = info.get_mesh_type_id();
  if (dim.size() == 1) return (CreateMesh(type,dim[0],min,max));
  if (dim.size() == 2) return (CreateMesh(type,dim[0],dim[1],min,max));
  if (dim.size() == 3) return (CreateMesh(type,dim[0],dim[1],dim[2],min,max));
  else return (MeshHandle());
}
