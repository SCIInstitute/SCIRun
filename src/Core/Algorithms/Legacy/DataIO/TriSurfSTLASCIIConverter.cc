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


#include <Core/Algorithms/Legacy/DataIO/TriSurfSTLASCIIConverter.h>
#include <Core/Algorithms/Legacy/DataIO/STLUtils.h>

#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>

#include <iomanip>
#include <fstream>
#include <sstream>

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/shared_array.hpp>

#include <Core/Utils/Legacy/StringUtil.h>

using namespace SCIRun;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Logging;
using namespace SCIRun::Core::Geometry;

namespace SCIRun {
namespace Core {
  namespace Algorithms {

class AsciiConverterPrivate
{
public:
//  // point(vertex) lookup table
//  typedef boost::unordered_map< Point, unsigned int, PointHash > PointTable;
//  typedef std::list<Facet> FacetList;

  explicit AsciiConverterPrivate(LoggerHandle pr)
  : pr_(pr),
    CELL_SIZE(3)
  {}

  bool readFile(const std::string& filename, FieldHandle& field);
  bool writeFile(const std::string& filename, VMesh *vmesh);
  void formatLine(std::string& line) const
  {
    // replace comma's and tabs with white spaces
    for (size_t p = 0; p < line.size(); ++p)
    {
      if ( (line[p] == '\t') || (line[p] == ',') ) line[p] = ' ';
    }
  }

  // assumes always length 3
  inline Point vectorToPoint(const std::vector<float>& v) const
  {
    return Point( v[0], v[1], v[2] );
  }

private:
  LoggerHandle pr_;
  const unsigned short CELL_SIZE;

  PointTable pointsLookupTable;
};
}}}

bool
AsciiConverterPrivate::readFile(const std::string& filename, FieldHandle& field)
{
  std::ifstream inputfile;
  inputfile.exceptions( std::ifstream::failbit | std::ifstream::badbit );

  VMesh *vmesh = field->vmesh();

  const std::string solidString("solid");
  const std::string endSolidString("endsolid");
  const std::string facetString("facet");
  const std::string endFacetString("endfacet");
  const std::string outerLoopString("outer loop");
  const std::string endOuterLoopString("endloop");
  const std::string vertexString("vertex");

  try
  {
    inputfile.open(filename.c_str());
    std::string line;
    unsigned int pointIndex = 0, vertexCounter = 0, lineCounter = 0;

    std::vector< std::vector<float> > vertexArray(CELL_SIZE);
    FacetList facetList;

    // loop until header
    // if invalid STL ASCII file, header starting with 'solid' will not be found
    while ( std::getline(inputfile, line, '\n') )
    {
      ++lineCounter;
      boost::algorithm::to_lower(line);
      std::size_t index = line.find(solidString);
      if (index != std::string::npos)
      {
        break;
      }
    }

    // parse facets
    while ( std::getline(inputfile, line, '\n') )
    {
      ++lineCounter;
      boost::algorithm::to_lower(line);
      // block out empty lines
      while ( line.empty() ) continue;
      // block out comments
      while ( line[0] == '#' ) continue;

      // block out comments
      if ( (line[0] == '#') || (line[0] == '%') ) continue;
      formatLine(line);

      // parse facet, skip normal
      if ( line.find(facetString) != std::string::npos )
      {
        continue;
      }
      // begin triangle
      else if ( line.find(outerLoopString) != std::string::npos )
      {
        vertexCounter = 0;
        continue;
      }
      else if ( line.find(vertexString) != std::string::npos )
      {
        if (vertexCounter == CELL_SIZE)
        {
          if (this->pr_)
          {
            this->pr_->error("Loop contains unsupported number of vertices.");
            this->pr_->error("Converter expects 3 vertices per loop.");
          }
          return false;
        }
        multiple_from_string(line, vertexArray[vertexCounter]);
        ++vertexCounter;
      }
      // end triangle
      else if ( line.find(endOuterLoopString) != std::string::npos )
      {
        Point p1 = vectorToPoint(vertexArray[0]),
              p2 = vectorToPoint(vertexArray[1]),
              p3 = vectorToPoint(vertexArray[2]);
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
      else if ( line.find(endFacetString) != std::string::npos )
      {
        continue;
      }
      // done
      else if ( line.find(endSolidString) != std::string::npos )
      {
        break;
      }
#if DEBUG
      else
      {
        std::ostringstream oss;
        oss << "Line " << lineCounter << " [" << line << "] contains unknown keyword.";
        if (this->pr_) this->pr_->warning(oss.str());
      }
#endif
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
  catch (std::ifstream::failure e)
  {
    if (this->pr_) this->pr_->error("Could not open and read from file " + filename);
    return false;
  }

  return true;
}

bool
AsciiConverterPrivate::writeFile(const std::string& filename, VMesh *vmesh)
{
  std::ofstream outputfile;
  outputfile.exceptions( std::ofstream::failbit | std::ofstream::badbit );

  try
  {
    outputfile.open(filename.c_str());

    // these appear to be reasonable formatting flags for output
    std::ios_base::fmtflags ff;
    ff = outputfile.flags();
    ff |= outputfile.showpoint; // write floating-point values including always the decimal point
    ff |= outputfile.fixed; // write floating point values in fixed-point notation
    outputfile.flags(ff);

    const std::string delim(" ");
    const std::string indent_level1("  ");
    const std::string indent_level2("    ");
    const std::string indent_level3("      ");

    VMesh::Face::iterator meshFaceIter;
    VMesh::Face::iterator meshFaceEnd;

    VMesh::Node::array_type nodesFromFace(CELL_SIZE);
    vmesh->end(meshFaceEnd);

    outputfile << "solid " << boost::filesystem::path(filename).stem().string() << std::endl;
    for (vmesh->begin(meshFaceIter); meshFaceIter != meshFaceEnd; ++meshFaceIter)
    {
      // get nodes from mesh element
      VMesh::Face::index_type elemID = *meshFaceIter;

      vmesh->get_nodes(nodesFromFace, elemID);
      Point p1, p2, p3;
      vmesh->get_center(p1, nodesFromFace[0]);
      vmesh->get_center(p2, nodesFromFace[1]);
      vmesh->get_center(p3, nodesFromFace[2]);

      boost::shared_array<float> normal = computeFaceNormal(p1, p2, p3);
      outputfile << indent_level1 << "facet normal " //<< std::fixed
                 << normal[0] << delim << normal[1] << delim << normal[2] << std::endl;

      outputfile << indent_level2 << "outer loop" << std::endl;
      outputfile << indent_level3 << "vertex " //<< std::fixed
                 << p1.x() << delim << p1.y() << delim << p1.z() << std::endl;
      outputfile << indent_level3 << "vertex " //<< std::fixed
                 << p2.x() << delim << p2.y() << delim << p2.z() << std::endl;
      outputfile << indent_level3 << "vertex " //<< std::fixed
                << p3.x() << delim << p3.y() << delim << p3.z() << std::endl;
      outputfile << indent_level2 << "endloop" << std::endl;
      outputfile << indent_level1 << "endfacet" << std::endl;
    }
    outputfile << "endsolid" << std::endl;

    outputfile.close();
  }
  catch (std::ifstream::failure& e)
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

TriSurfSTLASCIIConverter::TriSurfSTLASCIIConverter(LoggerHandle pr) :
  pr_(pr), converter_(new AsciiConverterPrivate(pr))
{}

bool
TriSurfSTLASCIIConverter::read(const std::string& filename, FieldHandle& field)
{
  // no data in STL file, make no basis for now
  FieldInformation fieldInfo("TriSurfMesh", -1, "double");
  field = CreateField(fieldInfo);

  return converter_->readFile(filename, field);
}

bool
TriSurfSTLASCIIConverter::write(const std::string& filename, const FieldHandle& field)
{
  VMesh *vmesh = field->vmesh();

  // validate
  if (! vmesh->is_trisurfmesh() )
  {
    if (pr_)
      pr_->error("STL ASCII converter only supports TriSurf mesh fields.");
    return false;
  }

  return converter_->writeFile(filename, vmesh);
}
