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


#include <Core/Algorithms/Legacy/DataIO/TriSurfSTLBinaryConverter.h>
#include <Core/Algorithms/Legacy/DataIO/STLUtils.h>

#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>

#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <locale>

/// TODO: use std::unordered_map when porting to SCIRun 5
//#include <unordered_map>
using namespace SCIRun;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Logging;
using namespace SCIRun::Core::Geometry;

namespace SCIRun {
namespace Core {
  namespace Algorithms {
class ConverterPrivate
{
public:
  explicit ConverterPrivate(LoggerHandle pr)
  : pr_(pr)
  {}

  bool readFile(const std::string& filename, FieldHandle& field);
  bool writeFile(const std::string& filename, VMesh *vmesh);

  // assumes always array length 3
  template <typename V>
  Point arrayToPoint(const V& array)
  {
    return Point( array[0], array[1], array[2] );
  }

private:
  LoggerHandle pr_;
  /// 80 byte header, usually ignored
  static constexpr unsigned short STL_HEADER_LENGTH {80};
  /// STL binary contains unsigned ints, floats
  static constexpr unsigned short STL_FIELD_LENGTH {4};
  static constexpr unsigned short POINT_LEN {3};
  static constexpr unsigned short CELL_SIZE {3};
  static constexpr unsigned short FIELD_LEN {12};
  static constexpr unsigned short ATTRIBUTE_BYTE_COUNT {2};

  PointTable pointsLookupTable;
};
}}}

bool
ConverterPrivate::readFile(const std::string& filename, FieldHandle& field)
{
  std::ifstream inputfile;
  inputfile.exceptions( std::ifstream::failbit | std::ifstream::badbit );

  VMesh *vmesh = field->vmesh();

  try
  {
    inputfile.open(filename.c_str(), std::ios::in | std::ios::binary);

    // check for solid and discard
    std::vector<char> headerBuffer(STL_HEADER_LENGTH);
    inputfile.read(&headerBuffer[0], STL_HEADER_LENGTH);

    std::string header(headerBuffer.begin(), headerBuffer.end());
    const std::string solidString("solid");
	  std::locale loc;
    for (unsigned int i = 0; i < solidString.length() && i < header.length(); ++i)
    {
      header[i] = std::tolower(header[i], loc);
    }
    // verify that we're not reading an ASCII file, which should start with 'solid'
    std::size_t index = header.find(solidString);
    if (index == 0)
    {
      // warn, but attempt to parse anyway
      if (this->pr_)
        this->pr_->warning(filename + " header begins with \"solid\". This may be an ASCII STL file.");
    }

    std::vector<char> numTrianglesBuffer(STL_FIELD_LENGTH);
    inputfile.read(&numTrianglesBuffer[0], STL_FIELD_LENGTH);
    unsigned int numTriangles = *( reinterpret_cast<unsigned int*>( &numTrianglesBuffer[0] ) );
    FacetList facetList;

    vmesh->elem_reserve(numTriangles);

    unsigned int pointIndex = 0;
    for (unsigned int i = 0; (i < numTriangles) && (! inputfile.eof()) ; ++i)
    {
      // discard normals
      inputfile.seekg(FIELD_LEN, std::ios_base::cur);

      std::vector<char> vertex1Buffer(FIELD_LEN);
      inputfile.read(&vertex1Buffer[0], FIELD_LEN);
      std::vector<float> vertex1(POINT_LEN);
      memcpy(&vertex1[0], &vertex1Buffer[0], FIELD_LEN);

      std::vector<char> vertex2Buffer(FIELD_LEN);
      inputfile.read(&vertex2Buffer[0], FIELD_LEN);
      std::vector<float> vertex2(POINT_LEN);
      memcpy(&vertex2[0], &vertex2Buffer[0], FIELD_LEN);

      std::vector<char> vertex3Buffer(FIELD_LEN);
      inputfile.read(&vertex3Buffer[0], FIELD_LEN);
      std::vector<float> vertex3(POINT_LEN);
      memcpy(&vertex3[0], &vertex3Buffer[0], FIELD_LEN);

      // discard attribute byte count
      inputfile.seekg(ATTRIBUTE_BYTE_COUNT, std::ios_base::cur);

      Point p1 = arrayToPoint(vertex1),
            p2 = arrayToPoint(vertex2),
            p3 = arrayToPoint(vertex3);
      facetList.push_back( Facet(p1, p2, p3) );

      // adding points to mesh here ensures points get added in order
      // (very important)
      PointTable::iterator it = pointsLookupTable.find(p1);
      if ( it == pointsLookupTable.end() )
      {
        pointsLookupTable[p1] = pointIndex++;
        vmesh->add_point(p1);
      }

      it = pointsLookupTable.find(p2);
      if ( it == pointsLookupTable.end() )
      {
        pointsLookupTable[p2] = pointIndex++;
        vmesh->add_point(p2);
      }

      it = pointsLookupTable.find(p3);
      if ( it == pointsLookupTable.end() )
      {
        pointsLookupTable[p3] = pointIndex++;
        vmesh->add_point(p3);
      }
    }
    inputfile.close();

    FacetList::iterator listIter;
    for (listIter = facetList.begin(); listIter != facetList.end(); ++listIter)
    {
      // use the facet list and point lookup table to match points and element indices
      VMesh::Node::array_type vdata;
      vdata.resize(CELL_SIZE);

      PointTable::iterator it = pointsLookupTable.find(listIter->point1_);
      vdata[0] = it->second;
      it = pointsLookupTable.find(listIter->point2_);
      vdata[1] = it->second;
      it = pointsLookupTable.find(listIter->point3_);
      vdata[2] = it->second;
      vmesh->add_elem(vdata);
    }
  }
  catch (std::ifstream::failure& e)
  {
    if (this->pr_) this->pr_->error("Could not open and read from file " + filename);
    return false;
  }

  return true;
}

bool
ConverterPrivate::writeFile(const std::string& filename, VMesh *vmesh)
{
  std::ofstream outputfile;
  outputfile.exceptions( std::ofstream::failbit | std::ofstream::badbit );

  try
  {
    outputfile.open(filename.c_str(), std::ios::out | std::ios::binary);

    std::string header("STL header: SCIRun TriSurf field to STL Binary export");
    char* headerBuffer = const_cast<char*>(header.c_str());
    outputfile.write(headerBuffer, STL_HEADER_LENGTH);

    unsigned int numTriangles = static_cast<unsigned int>( vmesh->num_faces() );
    outputfile.write(reinterpret_cast<char*>(&numTriangles), STL_FIELD_LENGTH);

    VMesh::Face::iterator meshFaceIter;
    VMesh::Face::iterator meshFaceEnd;

    VMesh::Node::array_type nodesFromFace(CELL_SIZE);
    vmesh->end(meshFaceEnd);

    // 0 is an acceptable value for this field
    unsigned short byteAttributeCount = 0;

    for (vmesh->begin(meshFaceIter); meshFaceIter != meshFaceEnd; ++meshFaceIter)
    {
      // get nodes from mesh element
      VMesh::Face::index_type elemID = *meshFaceIter;

      vmesh->get_nodes(nodesFromFace, elemID);
      Point p1, p2, p3;
      vmesh->get_center(p1, nodesFromFace[0]);
      vmesh->get_center(p2, nodesFromFace[1]);
      vmesh->get_center(p3, nodesFromFace[2]);

      auto normal = computeFaceNormal(p1, p2, p3);
      outputfile.write(reinterpret_cast<char*>(&normal[0]), FIELD_LEN);

      std::vector<float> vertex1 = {static_cast<float>(p1.x()), static_cast<float>(p1.y()), static_cast<float>(p1.z())};
      outputfile.write(reinterpret_cast<char*>(&vertex1[0]), FIELD_LEN);

      std::vector<float> vertex2 = {static_cast<float>(p2.x()), static_cast<float>(p2.y()), static_cast<float>(p2.z())};
      outputfile.write(reinterpret_cast<char*>(&vertex2[0]), FIELD_LEN);

      std::vector<float> vertex3 = {static_cast<float>(p3.x()), static_cast<float>(p3.y()), static_cast<float>(p3.z())};
      outputfile.write(reinterpret_cast<char*>(&vertex3[0]), FIELD_LEN);

      outputfile.write(reinterpret_cast<char*>(&byteAttributeCount), ATTRIBUTE_BYTE_COUNT);
    }
    outputfile.close();
  }
  catch (std::ifstream::failure e)
  {
    if (this->pr_) this->pr_->error("Could not open and write to file " + filename);
    return false;
  }
  catch (...)
  {
    if (this->pr_) this->pr_->error("Error while exporting TriSurf field to " + filename);
    return false;
  }

  return true;
}

TriSurfSTLBinaryConverter::TriSurfSTLBinaryConverter(LoggerHandle pr) :
  pr_(pr), converter_(new ConverterPrivate(pr))
{}

bool
TriSurfSTLBinaryConverter::read(const std::string& filename, FieldHandle& field)
{
  // no data in STL file, make no basis for now
  FieldInformation fieldInfo("TriSurfMesh", -1, "double");
  field = CreateField(fieldInfo);

  bool result = converter_->readFile(filename, field);
  return result;
}

bool
TriSurfSTLBinaryConverter::write(const std::string& filename, const FieldHandle& field)
{
  VMesh *vmesh = field->vmesh();

  // validate
  if (! vmesh->is_trisurfmesh() )
  {
    if (this->pr_) this->pr_->error("STL Binary converter only supports TriSurf mesh fields.");
    return false;
  }

  bool result = converter_->writeFile(filename, vmesh);
  return result;
}
