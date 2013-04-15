/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2009 Scientific Computing and Imaging Institute,
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

///////////////////////////
// PORTED SCIRUN v4 CODE //
///////////////////////////

#include <boost/algorithm/string.hpp>
#include <Core/Datatypes/Mesh/FieldInformation.h>
#include <Core/Datatypes/Mesh/Field.h>
#include <Core/GeometryPrimitives/Point.h>

using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER

void Field5TypeInformation::insert_field_type_information(Field5* field)
{
  std::string temp;
  // Get the name of the GenericField class
  // This should give GenericField
  
  if (!field) 
    return;
  field_type = field->get_type_description(Field5::FIELD_NAME_ONLY_E)->get_name();

  // Analyze the mesh type
  // This will result in
  // mesh_type, mesh_basis_type, and point_type
  const TypeDescription* mesh_td = field->get_type_description(Field5::MESH_TD_E);
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
  
  const TypeDescription* basis_td = field->get_type_description(Field5::BASIS_TD_E);
  TypeDescription::td_vec* basis_sub_td = basis_td->get_sub_type();
  const TypeDescription* data_td = (*basis_sub_td)[0]; 
  
  temp = basis_td->get_name(); 
  basis_type = temp.substr(0,temp.find("<"));
  data_type = data_td->get_name();
  for (size_t j=0; j<data_type.size(); j++) if(data_type[j] == ' ') data_type[j] = '_';

  const TypeDescription* container_td = field->get_type_description(Field5::FDATA_TD_E);
  temp = container_td->get_name(); 
  container_type = temp.substr(0,temp.find("<"));
}


Field5Information::Field5Information(const std::string& meshtype,const std::string& meshbasis,const std::string& databasis, const std::string& datatype)
{
  set_field_type("GenericField");
  set_point_type("Point");
  set_container_type("vector");
  set_data_type(datatype);
  set_mesh_type(meshtype);
  set_mesh_basis_type(meshbasis);
  set_data_basis_type(databasis);
}

Field5Information::Field5Information(const std::string& meshtype,int meshbasis,int databasis, const std::string& datatype)
{
  set_field_type("GenericField");
  set_point_type("Point");
  set_container_type("vector");
  set_data_type(datatype);
  set_mesh_type(meshtype);
  set_mesh_basis_type(meshbasis);
  set_data_basis_type(databasis);
}    

Field5Information::Field5Information(const std::string& meshtype,const std::string& basis, const std::string& datatype)
{
  set_field_type("GenericField");
  set_point_type("Point");
  set_container_type("vector");
  set_data_type(datatype);
  set_mesh_type(meshtype);
  set_mesh_basis_type(basis);
  set_data_basis_type(basis);
}
#endif

Field5Information::Field5Information(const std::string& meshtype,int basis, const std::string& datatype)
{
  set_field_type("GenericField");
  set_point_type("Point");
  set_container_type("vector");
  set_data_type(datatype);
  set_mesh_type(meshtype);
  set_mesh_basis_type(basis);
  set_data_basis_type(basis);
}

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
Field5Information::Field5Information(mesh_info_type meshtype,
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

Field5Information::Field5Information(mesh_info_type meshtype,
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


Field5Information::Field5Information(FieldHandle5 handle)
{
  insert_field_type_information(handle.get_rep());
}


Field5Information::Field5Information(Field5* field)
{
  insert_field_type_information(field);
}
#endif
std::string
Field5Information::get_field_type() const
{
  return(field_type);
}

void
Field5Information::set_field_type(const std::string& type)
{
  field_type = type;
}

std::string
Field5Information::get_mesh_type() const
{
  return(mesh_type);
}

void
Field5Information::set_mesh_type(const std::string& type)
{
  std::string typeLower = boost::algorithm::to_lower_copy(type);
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
  else 
  { 
    BOOST_THROW_EXCEPTION(UnknownMeshType() << Core::ErrorMessage("Unknown mesh type")); 
  }
}


void
Field5Information::set_mesh_type(mesh_info_type type)
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
Field5Information::get_mesh_basis_type() const
{
  return(mesh_basis_type);
}

void
Field5Information::set_mesh_basis_type(int order)
{
  if (order == 1) set_mesh_basis_type("linear");
  if (order == 2) set_mesh_basis_type("quadratic");
  if (order == 3) set_mesh_basis_type("cubic");  
}

void
Field5Information::set_mesh_basis_type(const std::string& type)
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
Field5Information::get_point_type() const
{
  return(point_type);
}

void
Field5Information::set_point_type(const std::string& type)
{
  point_type = type;
}


std::string
Field5Information::get_basis_type() const
{
  return(basis_type);
}

void
Field5Information::set_basis_type(int order)
{
  if (order == -1) set_basis_type("nodata");
  if (order == 0) set_basis_type("constant");
  if (order == 1) set_basis_type("linear");
  if (order == 2) set_basis_type("quadratic");
  if (order == 3) set_basis_type("cubic");  
}


void
Field5Information::set_basis_type(const std::string& type)
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
Field5Information::get_data_type() const
{
  return(data_type);
}

void
Field5Information::set_data_type(const std::string& type1)
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


void
Field5Information::set_data_type(data_info_type type)
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

    default: 
      BOOST_THROW_EXCEPTION(MeshException() << Core::ErrorMessage("INTERNAL ERROR - unknown data_info_type"));
  }
}

std::string 
Field5Information::get_container_type() const
{
  return(container_type);
}

void
Field5Information::set_container_type(const std::string& type)
{
  container_type = type;
}

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
std::string
Field5Information::get_field_name()
{
  // Deal with some SCIRun design flaw
  std::string meshptr = "";
  if ((container_type.find("2d") != std::string::npos)||(container_type.find("3d") != std::string::npos)) 
    meshptr = "," + mesh_type + "<" + mesh_basis_type + "<" + point_type + "> " + "> ";
    
  std::string field_template = field_type + "<" + mesh_type + "<" + 
    mesh_basis_type + "<" + point_type + "> " + "> " + "," +
    basis_type + "<" + data_type + "> " + "," + container_type + "<" +
    data_type + meshptr + "> " + "> ";
    
  return(field_template);
}

std::string
Field5Information::get_field_type_id()
{
  // Deal with some SCIRun design flaw
  std::string meshptr = "";
  if ((container_type.find("2d") != std::string::npos)||(container_type.find("3d") != std::string::npos)) 
    meshptr = "," + mesh_type + "<" + mesh_basis_type + "<" + point_type + ">" + ">";
    
  std::string field_template = field_type + "<" + mesh_type + "<" + 
    mesh_basis_type + "<" + point_type + ">" + ">" + "," +
    basis_type + "<" + data_type + ">" + "," + container_type + "<" +
    data_type + meshptr + ">" + ">";
  
  std::replace(field_template.begin(), field_template.end(), ' ', '_');
        
  return field_template;
}
#endif

std::string Field5Information::get_mesh_type_id() const
{
  std::string mesh_template = mesh_type + "<" + mesh_basis_type + "<" + point_type + ">" + ">";
  
  std::replace(mesh_template.begin(), mesh_template.end(), ' ', '_');
        
  return mesh_template;
}

bool
Field5TypeInformation::is_isomorphic()
{
  return((mesh_basis_type == basis_type));
}

bool
Field5TypeInformation::is_nonlinear()
{
  return((is_nonlineardata())||(is_nonlinearmesh()));
}

bool
Field5TypeInformation::is_linear()
{
  return((is_lineardata())&&(is_linearmesh()));
}

bool
Field5TypeInformation::is_nodata()
{
  return((basis_type == "NoDataBasis"));
}

bool
Field5TypeInformation::is_constantdata()
{
  return((basis_type == "ConstantBasis"));
}

bool
Field5TypeInformation::is_lineardata()
{
  return((basis_type.find("inear") != std::string::npos));
}

bool
Field5TypeInformation::is_nonlineardata()
{
  return( (basis_type.find("uadratic") != std::string::npos)||
          (basis_type.find("ubicHmt") != std::string::npos));
}

bool
Field5TypeInformation::is_quadraticdata()
{
  return ((basis_type.find("uadratic") != std::string::npos));
}

bool
Field5TypeInformation::is_cubicdata()
{
  return ((basis_type.find("ubicHmt") != std::string::npos));
}

// TODO: an enum would be good for keeping track of basis order...
int
Field5TypeInformation::field_basis_order()
{
  if (is_nodata()) return (-1);
  if (is_constantdata()) return (0);
  if (is_lineardata()) return (1);
  if (is_quadraticdata()) return (2);
  if (is_cubicdata()) return (3);
  return (-1);
}


bool
Field5TypeInformation::is_constantmesh()
{
  return((mesh_basis_type == "ConstantBasis"));
}

bool
Field5TypeInformation::is_linearmesh()
{
  return((mesh_basis_type.find("inear") != std::string::npos));
}

bool
Field5TypeInformation::is_nonlinearmesh()
{
  return( (mesh_basis_type.find("uadratic") != std::string::npos)||
          (mesh_basis_type.find("ubicHmt") != std::string::npos));
}

bool
Field5TypeInformation::is_quadraticmesh()
{
  return ((mesh_basis_type.find("uadratic") != std::string::npos));
}

bool
Field5TypeInformation::is_cubicmesh()
{
  return ((mesh_basis_type.find("ubicHmt") != std::string::npos));
}

int
Field5TypeInformation::mesh_basis_order()
{
  if (is_constantmesh()) return (0);
  if (is_linearmesh()) return (1);
  if (is_quadraticmesh()) return (2);
  if (is_cubicmesh()) return (3);
  return (-1);
}

bool
Field5TypeInformation::is_tensor()
{
  return ((data_type == "Tensor"));
}

bool
Field5TypeInformation::is_vector()
{
  return ((data_type == "Vector"));
}

bool
Field5TypeInformation::is_scalar()
{
  return((!is_tensor())&&(!is_vector())&&(data_type!=""));
}

bool
Field5TypeInformation::is_integer()
{
  return(is_char()||is_short()||is_int()||is_long()||is_longlong()
      ||is_unsigned_char()||is_unsigned_short()
      ||is_unsigned_int()||is_unsigned_long()
      ||is_unsigned_longlong());
}

bool
Field5TypeInformation::is_double()
{
  return((data_type == "double"));
}

bool
Field5TypeInformation::is_float()
{
  return((data_type == "float"));
}

bool
Field5TypeInformation::is_longlong()
{
  return((data_type == "long_long")||(data_type == "signed_long_long"));
}

bool
Field5TypeInformation::is_long()
{
  return((data_type == "long")||(data_type == "signed_long"));
}

bool
Field5TypeInformation::is_int()
{
  return((data_type == "int")||(data_type == "signed_int"));
}

bool
Field5TypeInformation::is_short()
{
  return((data_type == "short")||(data_type == "signed_short"));
}

bool
Field5TypeInformation::is_char()
{
  return((data_type == "char")||(data_type == "signed_char"));
}

bool
Field5TypeInformation::is_unsigned_longlong()
{
  return(data_type == "unsigned_long_long");
}

bool
Field5TypeInformation::is_unsigned_long()
{
  return(data_type == "unsigned_long");
}

bool
Field5TypeInformation::is_unsigned_int()
{
  return(data_type == "unsigned_int");
}

bool
Field5TypeInformation::is_unsigned_short()
{
  return(data_type == "unsigned_short");
}

bool
Field5TypeInformation::is_unsigned_char()
{
  return(data_type == "unsigned_char");
}

bool
Field5TypeInformation::is_dvt()
{
  return(is_double()||is_vector()||is_tensor());
}

bool
Field5TypeInformation::is_svt()
{
  return(is_scalar()||is_vector()||is_tensor());
}


bool
Field5TypeInformation::is_structuredmesh()
{
  return((mesh_type.find("Struct")!=std::string::npos) ||
	 (mesh_type=="ScanlineMesh")||
	 (mesh_type=="ImageMesh")||
	 (mesh_type=="LatVolMesh"));
}

bool
Field5TypeInformation::is_regularmesh()
{
  return((mesh_type=="ScanlineMesh")||
	 (mesh_type=="ImageMesh")||
	 (mesh_type=="LatVolMesh"));
}

bool
Field5TypeInformation::is_irregularmesh()
{
  return(!is_regularmesh());
}

bool
Field5TypeInformation::is_unstructuredmesh()
{
  return(!is_structuredmesh());
}

bool
Field5TypeInformation::is_pnt_element()
{
  return((mesh_type=="PointCloud"));
}

bool
Field5TypeInformation::is_crv_element()
{
  return((mesh_type=="CurveMesh")||
	 (mesh_type=="StructCurveMesh")||
	 (mesh_type=="ScanlineMesh"));
}

bool
Field5TypeInformation::is_tri_element()
{
  return((mesh_type=="TriSurfMesh"));
}

bool
Field5TypeInformation::is_quad_element()
{
  return((mesh_type=="QuadSurfMesh")||
	 (mesh_type=="ImageMesh")||
	 (mesh_type=="StructQuadSurfMesh"));
}

bool
Field5TypeInformation::is_tet_element()
{
  return((mesh_type=="TetVolMesh"));
}

bool
Field5TypeInformation::is_prism_element()
{
  return((mesh_type=="PrismVolMesh"));
}

bool
Field5TypeInformation::is_hex_element()
{
  return((mesh_type=="HexVolMesh")||
	 (mesh_type=="StructHexVolMesh")||
	 (mesh_type=="LatVolMesh")||
	 (mesh_type=="MaskedLatVolMesh"));
}

bool
Field5TypeInformation::is_pointcloudmesh()
{
  if (mesh_type == "PointCloudMesh") return (true);
  return false;
}

bool
Field5TypeInformation::is_scanlinemesh()
{
  if (mesh_type == "ScanlineMesh") return (true);
  return false;
}

bool
Field5TypeInformation::is_imagemesh()
{
  if (mesh_type == "ImageMesh") return (true);
  return false;
}

bool
Field5TypeInformation::is_latvolmesh()
{
  if (mesh_type == "LatVolMesh") return (true);
  return false;
}

bool
Field5TypeInformation::is_curvemesh()
{
  if (mesh_type == "CurveMesh") return (true);
  return false;
}

bool
Field5TypeInformation::is_trisurfmesh()
{
  if (mesh_type == "TriSurfMesh") return (true);
  return false;
}

bool
Field5TypeInformation::is_quadsurfmesh()
{
  if (mesh_type == "QuadSurfMesh") return (true);
  return false;
}

bool
Field5TypeInformation::is_tetvolmesh()
{
  if (mesh_type == "TetVolMesh") return (true);
  return false;
}

bool
Field5TypeInformation::is_prismvolmesh()
{
  if (mesh_type == "PrismVolMesh") return (true);
  return false;
}

bool
Field5TypeInformation::is_hexvolmesh()
{
  if (mesh_type == "HexVolMesh") return (true);
  return false;
}

bool
Field5TypeInformation::is_structcurvemesh()
{
  if (mesh_type == "StructCurveMesh") return (true);
  return false;
}

bool
Field5TypeInformation::is_structquadsurfmesh()
{
  if (mesh_type == "StructQuadSurfMesh") return (true);
  return false;
}

bool
Field5TypeInformation::is_structhexvolmesh()
{
  if (mesh_type == "StructHexVolMesh") return (true);
  return false;
}


bool
Field5TypeInformation::is_point()
{
  if (mesh_type == "PointCloudMesh") return (true);
  return false;
}

bool
Field5TypeInformation::is_line()
{
  if ((mesh_type == "CurveMesh")||
      (mesh_type == "ScanlineMesh")||
      (mesh_type == "StructCurveMesh")) return (true);
  return false;
}

bool
Field5TypeInformation::is_surface()
{
  if ((mesh_type == "TriSurfMesh")||
      (mesh_type == "QuadSurfMesh")||
      (mesh_type == "ImageMesh")||
      (mesh_type == "StructQuadSurfMesh")) return (true);
  return false;
}

bool
Field5TypeInformation::is_volume()
{
  if ((mesh_type == "TetVolMesh")||(mesh_type == "PrismVolMesh")||
      (mesh_type == "HexVolMesh")||(mesh_type == "LatVolMesh")||
      (mesh_type == "StructHexVolMesh")||(mesh_type == "MaskedLatVolMesh")) return (true);
  return false;  
}

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER

bool        
Field5Information::operator==(const Field5Information& fi) const
{
  if ( (field_type == fi.field_type) && (mesh_type == fi.mesh_type) && (mesh_basis_type == fi.mesh_basis_type) &&
       (point_type == fi.point_type) && (basis_type == fi.basis_type) && (data_type == fi.data_type) &&
       (container_type == fi.container_type) ) return (true);
  return (false);
}


bool        
Field5Information::operator!=(const Field5Information& fi) const
{
  if ( (field_type != fi.field_type) || (mesh_type != fi.mesh_type) || (mesh_basis_type != fi.mesh_basis_type) ||
       (point_type != fi.point_type) || (basis_type != fi.basis_type) || (data_type != fi.data_type) ||
       (container_type != fi.container_type) ) return (true);
  return (false);
}

#endif
bool
Field5Information::make_nodata()
{
  set_basis_type("NoDataBasis");
  set_data_type("double");
  return (true);
}

bool
Field5Information::make_constantdata()
{
  set_basis_type("ConstantBasis");
  return (true);
}

bool
Field5Information::make_constantmesh()
{
  set_mesh_basis_type("ConstantBasis");
  return (true);
}

bool
Field5Information::make_scanlinemesh()
{
  set_mesh_type("ScanlineMesh");
  return (true);
}

bool
Field5Information::make_curvemesh()
{
  set_mesh_type("CurveMesh");
  return (true);
}

bool
Field5Information::make_structcurvemesh()
{
  set_mesh_type("StructCurveMesh");
  return (true);
}

bool
Field5Information::make_imagemesh()
{
  set_mesh_type("ImageMesh");
  return (true);
}

bool
Field5Information::make_trisurfmesh()
{
  set_mesh_type("TriSurfMesh");
  return (true);
}

bool
Field5Information::make_quadsurfmesh()
{
  set_mesh_type("QuadSurfMesh");
  return (true);
}

bool
Field5Information::make_structquadsurfmesh()
{
  set_mesh_type("StructQuadSurfMesh");
  return (true);
}

bool
Field5Information::make_latvolmesh()
{
  set_mesh_type("LatVolMesh");
  return (true);
}

bool
Field5Information::make_tetvolmesh()
{
  set_mesh_type("TetVolMesh");
  return (true);
}

bool
Field5Information::make_prismvolmesh()
{
  set_mesh_type("PrismVolMesh");
  return (true);
}

bool
Field5Information::make_hexvolmesh()
{
  set_mesh_type("HexVolMesh");
  return (true);
}

bool
Field5Information::make_structhexvolmesh()
{
  set_mesh_type("StructHexVolMesh");
  return (true);
}

bool
Field5Information::make_pointcloudmesh()
{
  set_mesh_type("PointCloudMesh");
  return (true);
}


bool
Field5Information::make_unstructuredmesh()
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
Field5Information::make_irregularmesh()
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
Field5Information::make_linearmesh()
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
Field5Information::make_lineardata()
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
Field5Information::make_quadraticdata()
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
Field5Information::make_quadraticmesh()
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
Field5Information::make_cubicdata()
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
Field5Information::make_cubicmesh()
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
Field5Information::make_scalar()
{
  set_data_type("double");
  return (true);
}


bool
Field5Information::make_char()
{
  set_data_type("char");
  return (true);
}

bool
Field5Information::make_unsigned_char()
{
  set_data_type("unsigned_char");
  return (true);
}

bool
Field5Information::make_short()
{
  set_data_type("short");
  return (true);
}

bool
Field5Information::make_unsigned_short()
{
  set_data_type("unsigned_short");
  return (true);
}

bool
Field5Information::make_int()
{
  set_data_type("int");
  return (true);
}

bool
Field5Information::make_unsigned_int()
{
  set_data_type("unsigned_int");
  return (true);
}

bool
Field5Information::make_long()
{
  set_data_type("long");
  return (true);
}

bool
Field5Information::make_unsigned_long()
{
  set_data_type("unsigned_long");
  return (true);
}

bool
Field5Information::make_long_long()
{
  set_data_type("long_long");
  return (true);
}

bool
Field5Information::make_unsigned_long_long()
{
  set_data_type("unsigned_long_long");
  return (true);
}

bool
Field5Information::make_float()
{
  set_data_type("float");
  return (true);
}

bool
Field5Information::make_double()
{
  set_data_type("double");
  return (true);
}

bool
Field5Information::make_vector()
{
  set_data_type("Vector");
  return (true);
}

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
bool
Field5Information::make_tensor()
{
  set_data_type("Tensor");
  return (true);
}

//TODO DAN: REFACTORING NEEDED: LEVEL MEDIUM

FieldHandle5 CreateField(const std::string& meshtype, 
                         const std::string& basistype, const std::string& datatype)
{
  Field5Information fi(meshtype,basistype,datatype);
  return(CreateField(fi));
}

FieldHandle5 CreateField(const std::string& meshtype, const std::string& meshbasistype, 
                         const std::string& databasistype, const std::string& datatype)
{
  Field5Information fi(meshtype,meshbasistype,databasistype,datatype);
  return(CreateField(fi));
}

FieldHandle5 CreateField(mesh_info_type mesh, 
                        meshbasis_info_type meshbasis, 
                        databasis_info_type databasis, 
                        data_info_type data)
{
  Field5Information fi(mesh,meshbasis,databasis,data);
  return(CreateField(fi));
}

FieldHandle5 CreateField(mesh_info_type mesh,  
                        databasis_info_type databasis, 
                        data_info_type data)
{
  Field5Information fi(mesh,databasis,data);
  return(CreateField(fi));
}

FieldHandle5
CreateField(Field5Information &info)
{
  if (info.is_nodata()) info.set_data_type("double");
  std::string type = info.get_field_type_id();
  std::string meshtype = info.get_mesh_type_id();

  MeshHandle5 meshhandle = CreateMesh(meshtype);
  
  if (meshhandle.get_rep() == 0) return (0);

  return (CreateField(type,meshhandle));              
}

FieldHandle5
CreateField(Field5Information &info, MeshHandle5 mesh)
{
  if (info.is_nodata()) info.set_data_type("double");
  std::string type = info.get_field_type_id();
  return (CreateField(type,mesh));              
}

MeshHandle5 
CreateMesh(Field5Information &info)
{
  std::string type = info.get_mesh_type_id();
  return (CreateMesh(type));
}

MeshHandle5 
CreateMesh(Field5Information &info,Mesh5::size_type x)
{
  std::string type = info.get_mesh_type_id();
  return (CreateMesh(type,x));
}

MeshHandle5 
CreateMesh(Field5Information &info,Mesh5::size_type x,const Point& min,const Point& max)
{
  std::string type = info.get_mesh_type_id();
  return (CreateMesh(type,x,min,max));
}

MeshHandle5 
CreateMesh(Field5Information &info,Mesh5::size_type x,Mesh5::size_type y)
{
  std::string type = info.get_mesh_type_id();
  return (CreateMesh(type,x,y));
}

MeshHandle5 MeshFactory::CreateMesh(const Field5Information& info, MeshTraits::size_type x, MeshTraits::size_type y, const Point& min, const Point& max)
{
  std::string type = info.get_mesh_type_id();
  return (CreateMesh(type,x,y,min,max));
}

MeshHandle5 
CreateMesh(Field5Information &info,Mesh5::size_type x,Mesh5::size_type y,Mesh5::size_type z)
{
  std::string type = info.get_mesh_type_id();
  return (CreateMesh(type,x,y,z));
}
#endif



#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
MeshHandle5 
CreateMesh(Field5Information &info, const std::vector<Mesh5::size_type>& dim)
{
  std::string type = info.get_mesh_type_id();
  if (dim.size() == 1) return (CreateMesh(type,dim[0]));
  if (dim.size() == 2) return (CreateMesh(type,dim[0],dim[1]));
  if (dim.size() == 3) return (CreateMesh(type,dim[0],dim[1],dim[2]));
  else return (MeshHandle5(0));
}

MeshHandle5 
CreateMesh(Field5Information &info, const std::vector<Mesh5::size_type>& dim,const Point& min,const Point& max)
{
  std::string type = info.get_mesh_type_id();
  if (dim.size() == 1) return (CreateMesh(type,dim[0],min,max));
  if (dim.size() == 2) return (CreateMesh(type,dim[0],dim[1],min,max));
  if (dim.size() == 3) return (CreateMesh(type,dim[0],dim[1],dim[2],min,max));
  else return (MeshHandle5(0));
}


MeshHandle5 
CreateMesh(mesh_info_type mesh)
{
  Field5Information info(mesh,LINEARDATA_E,DOUBLE_E);
  std::string type = info.get_mesh_type_id();
  return (CreateMesh(type));
}

MeshHandle5 
CreateMesh(mesh_info_type mesh,Mesh5::size_type x)
{
  Field5Information info(mesh,LINEARDATA_E,DOUBLE_E);
  std::string type = info.get_mesh_type_id();
  return (CreateMesh(type,x));
}

MeshHandle5 
CreateMesh(mesh_info_type mesh,Mesh5::size_type x,const Point& min,const Point& max)
{
  Field5Information info(mesh,LINEARDATA_E,DOUBLE_E);
  std::string type = info.get_mesh_type_id();
  return (CreateMesh(type,x,min,max));
}

MeshHandle5 
CreateMesh(mesh_info_type mesh,Mesh5::size_type x,Mesh5::size_type y)
{
  Field5Information info(mesh,LINEARDATA_E,DOUBLE_E);
  std::string type = info.get_mesh_type_id();
  return (CreateMesh(type,x,y));
}

MeshHandle5 
CreateMesh(mesh_info_type mesh,Mesh5::size_type x,Mesh5::size_type y,const Point& min,const Point& max)
{
  Field5Information info(mesh,LINEARDATA_E,DOUBLE_E);
  std::string type = info.get_mesh_type_id();
  return (CreateMesh(type,x,y,min,max));
}

MeshHandle5 
CreateMesh(mesh_info_type mesh,Mesh5::size_type x,Mesh5::size_type y,Mesh5::size_type z)
{
  Field5Information info(mesh,LINEARDATA_E,DOUBLE_E);
  std::string type = info.get_mesh_type_id();
  return (CreateMesh(type,x,y,z));
}

MeshHandle5 
CreateMesh(mesh_info_type mesh,Mesh5::size_type x,Mesh5::size_type y,Mesh5::size_type z,const Point& min,const Point& max)
{
  Field5Information info(mesh,LINEARDATA_E,DOUBLE_E);
  std::string type = info.get_mesh_type_id();
  return (CreateMesh(type,x,y,z,min,max));
}

MeshHandle5 
CreateMesh(mesh_info_type mesh,const std::vector<Mesh5::size_type>& dim)
{
  Field5Information info(mesh,LINEARDATA_E,DOUBLE_E);
  std::string type = info.get_mesh_type_id();
  if (dim.size() == 1) return (CreateMesh(type,dim[0]));
  if (dim.size() == 2) return (CreateMesh(type,dim[0],dim[1]));
  if (dim.size() == 3) return (CreateMesh(type,dim[0],dim[1],dim[2]));
  else return (MeshHandle5(0));
}

MeshHandle5 
CreateMesh(mesh_info_type mesh,const std::vector<Mesh5::size_type>& dim,const Point& min,const Point& max)
{
  Field5Information info(mesh,LINEARDATA_E,DOUBLE_E);
  std::string type = info.get_mesh_type_id();
  if (dim.size() == 1) return (CreateMesh(type,dim[0],min,max));
  if (dim.size() == 2) return (CreateMesh(type,dim[0],dim[1],min,max));
  if (dim.size() == 3) return (CreateMesh(type,dim[0],dim[1],dim[2],min,max));
  else return (MeshHandle5(0));
}

#endif