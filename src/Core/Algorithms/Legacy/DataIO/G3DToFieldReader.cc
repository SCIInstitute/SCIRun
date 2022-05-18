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


#include <Core/Algorithms/Legacy/DataIO/G3DToFieldReader.h>

#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/Legacy/Field/Mesh.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/GeometryPrimitives/Point.h>

#include<Core/Algorithms/Legacy/DataIO/G3D.h>

#include <Core/Utils/Legacy/StringUtil.h>
#include <Core/Logging/Log.h>

#include <fstream>
#include <iostream>
#include <sstream>

using namespace SCIRun;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Logging;

G3DToFieldReader::G3DToFieldReader(LoggerHandle log) : log_(log) {}

bool G3DToFieldReader::read(const std::string& filename, FieldHandle& field_handle)
{
  std::ifstream inputfile;
  inputfile.open(filename.c_str());
  if (! inputfile)
  {
    if (log_) log_->error("Failed to open input file " + filename);
    return false;
  }

  std::string line, str;
  double x, y, z;
  const char* nodeKey = "v"; // geometric vertices
  const char* faceKey = "f"; // face elements

  FieldInformation fi("TriSurfMesh", 1, "double");
  field_handle = CreateField(fi);
  VMesh *mesh = field_handle->vmesh();
  //MeshHandle mesh = field_handle->mesh();

  while ( getline(inputfile, line, '\n') )
  {
    if (line.size() == 0) continue;

    // block out comments
    if ((line[0] == '#') || (line[0] == '%')) continue;

    // replace comma's and tabs with white spaces
    for (size_t p = 0; p < line.size(); ++p)
    {
      if ((line[p] == '\t') || (line[p] == ',') || (line[p]=='"')) line[p] = ' ';
    }

    // anything else that is not a vertex or face is ignored
    if ( (line[0] != 'v') && (line[0] != 'f') ) continue;

    std::istringstream lss(line);
    lss.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try
    {
      lss >> str >> x >> y >> z;
    }
    catch (...)
    {
      if (log_) log_->error("Parsing line " + line + " failed.");
      return false;
    }

    if(str == nodeKey)
    {
      mesh->add_point(Point(x,y,z));
    }
    else if (str == faceKey)
    {
      VMesh::Node::array_type n(3);
      n[0] = x-1;
      n[1] = y-1;
      n[2] = z-1;
      mesh->add_elem(n);
    }
  }

  inputfile.close();

  return true;

  /*
  std::shared_ptr<Mesh>
  MobileGeoConverter::ConvertToMesh(const std::string& strFilename) {
  VertVec       vertices;
  NormVec       normals;
  TexCoordVec   texcoords;
  ColorVec      colors;

  IndexVec      VertIndices;
  IndexVec      NormalIndices;
  IndexVec      TCIndices;
  IndexVec      COLIndices;

  G3D::GeometrySoA geometry;
  G3D::read(strFilename, &geometry);
  if (geometry.info.indexSize == sizeof(uint16_t))
  {
  for (uint32_t i=0; i<geometry.info.numberIndices; ++i) VertIndices.push_back((uint32_t)((uint16_t*)geometry.indices)[i]);
  }
  else VertIndices = IndexVec(geometry.indices, (uint32_t*)geometry.indices + geometry.info.numberIndices);

  uint32_t i = 0;
  for (std::vector<uint32_t>::iterator it=geometry.info.attributeSemantics.begin(); it<geometry.info.attributeSemantics.end(); ++it)
  {
  if (*it == G3D::Position) vertices = VertVec((FLOATVECTOR3*)geometry.vertexAttributes.at(i), (FLOATVECTOR3*)geometry.vertexAttributes.at(i) + geometry.info.numberVertices);
  else if (*it == G3D::Normal) normals = NormVec((FLOATVECTOR3*)geometry.vertexAttributes.at(i), (FLOATVECTOR3*)geometry.vertexAttributes.at(i) + geometry.info.numberVertices);
  else if (*it == G3D::Color) colors = ColorVec((FLOATVECTOR4*)geometry.vertexAttributes.at(i), (FLOATVECTOR4*)geometry.vertexAttributes.at(i) + geometry.info.numberVertices);
  else if (*it == G3D::Tex) texcoords = TexCoordVec((FLOATVECTOR2*)geometry.vertexAttributes.at(i), (FLOATVECTOR2*)geometry.vertexAttributes.at(i) + geometry.info.numberVertices);
  ++i;
  }
  G3D::clean(&geometry);

  std::string desc = m_vConverterDesc + " data converted from " + SysTools::GetFilename(strFilename);
  return std::shared_ptr<Mesh>(
  new Mesh(vertices,normals,texcoords,colors,
  VertIndices,NormalIndices,TCIndices,COLIndices,
  false, false, desc, Mesh::MT_TRIANGLES)
  );
  */
}

bool G3DToFieldReader::write(const std::string& filename, const FieldHandle& field)
{
  std::ofstream os;
  const VMesh* mesh = field->vmesh();

  if (mesh->num_nodes() == 0) { return false; }

  os.open(filename.c_str(), std::ios::out);
  if (!os) { return false; }

  os << "# written by SCIRun\n";

  {
    VMesh::Node::size_type iter;
    VMesh::Node::size_type end = mesh->num_nodes();
    for (iter = 0; iter != end; ++iter)
    {
      Point p;
      mesh->get_point(p, iter);
      os << "v " << p.x() << " " << p.y() << " " << p.z() << "\n";
    }
  }

  {
    VMesh::Face::iterator iter;
    VMesh::Face::iterator end;
    VMesh::Node::array_type faceNodes(4);
    mesh->end(end);
    for (mesh->begin(iter); iter != end; ++iter)
    {
      mesh->get_nodes(faceNodes, *iter);
      // OBJ face indices are 1-based.  Seriously.
      os << "f " << faceNodes[0]+1 << " " << faceNodes[1]+1 << " "
         << faceNodes[2]+1 << "\n";
    }
  }
  os.close();

  return true;

  /*old method inputs
  const Mesh& m,
  const std::string& strTargetFilename)
  */

  //if (m.GetVertices().empty()) return false;

  /*
  // include this once we have mesh downsampling implemented
  bool bSimplify = SysTools::ToUpperCase(
  SysTools::GetExt(strTargetFilename)
  ) == "G3D";
  */
  /*
  G3D::GeometrySoA geometry;
  geometry.info.isOpaque = false;
  geometry.info.numberPrimitives =
    uint32_t(m.GetVertexIndices().size() / m.GetVerticesPerPoly());
  geometry.info.primitiveType = (m.GetMeshType() == Mesh::MT_TRIANGLES)
    ? G3D::Triangle : G3D::Line;
  geometry.info.numberIndices = uint32_t(m.GetVertexIndices().size());
  geometry.info.numberVertices = uint32_t(m.GetVertices().size());
  uint32_t vertexFloats = 0;

  geometry.info.attributeSemantics.push_back(G3D::Position);
  geometry.vertexAttributes.push_back((float*)&m.GetVertices().at(0));
  vertexFloats += G3D::floats(G3D::Position);

  if (m.GetNormals().size() == m.GetVertices().size())
  {
    geometry.info.attributeSemantics.push_back(G3D::Normal);
    geometry.vertexAttributes.push_back((float*)&m.GetNormals().at(0));
    vertexFloats += G3D::floats(G3D::Normal);
  }
  if (m.GetTexCoords().size() == m.GetVertices().size())
  {
    geometry.info.attributeSemantics.push_back(G3D::Tex);
    geometry.vertexAttributes.push_back((float*)&m.GetTexCoords().at(0));
    vertexFloats += G3D::floats(G3D::Tex);
  }
  geometry.info.attributeSemantics.push_back(G3D::Color);
  vertexFloats += G3D::floats(G3D::Color);
  ColorVec colors;
  if (m.GetColors().size() == m.GetVertices().size())
  {
    geometry.vertexAttributes.push_back((float*)&m.GetColors().at(0));
  }
  else
  {
    colors = ColorVec(m.GetVertices().size(), m.GetDefaultColor());
    geometry.vertexAttributes.push_back((float*)&colors.at(0));
  }

  geometry.info.indexSize = sizeof(uint32_t);
  geometry.info.vertexSize = vertexFloats * sizeof(float);
  geometry.indices = (uint32_t*)&m.GetVertexIndices().at(0);
  G3D::write(strTargetFilename, &geometry);
  */

}
