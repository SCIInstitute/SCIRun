/*
  For more information, please see: http://software.sci.utah.edu

  The MIT License

  Copyright (c) 2014 Scientific Computing and Imaging Institute,
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

#include <Core/Algorithms/Legacy/DataIO/VTKToTriSurfReader.h>

#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/Legacy/Field/Mesh.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Utils/Legacy/StringUtil.h>
#include <Core/Logging/LoggerInterface.h>

#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include <boost/foreach.hpp>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>

using namespace SCIRun;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Logging;
using namespace SCIRun::Core::Geometry;

namespace SCIRun {
  namespace Core {
    namespace Algorithms {

// TODO: add vector and tensor data support

template<class T>
class TriSurfScalarDataReaderPrivate
{
public:
  std::vector<T> data_;

  TriSurfScalarDataReaderPrivate(std::ifstream& fileStream)
    : fileStream_(fileStream) {}

  void
  readScalarData();

private:
  std::ifstream& fileStream_;
};

class VTKToTriSurfReaderPrivate
{
public:
  typedef std::vector<std::string> StringList;
  typedef std::vector<double> PointList;
  typedef std::vector<VMesh::index_type> ElemList;

  enum DATA_TYPE
  {
    UNKNOWN,
    SCALAR,
    COLOR,
    VECTOR,
    TENSOR
  };

  VTKToTriSurfReaderPrivate(LoggerHandle pr)
    : pr_(pr),
      meshType_(TRISURFMESH_E),
      meshBasisType_(LINEARMESH_E),
      fieldDataBasisType_(NODATA_E),
      fieldDataType_(NONE_E),
      vtkDataType_(UNKNOWN),
      POINTS_DIM(3),
      CELL_SIZE(3),
      numPointsFromFile_(0),
      numCellsFromFile_(0),
      numDataValuesFromFile_(0)
  {
    this->fileStream_.exceptions( std::ifstream::badbit );
  }

  bool
  readFile(const std::string& vtk_filename, FieldHandle& fieldHandle);

private:
  LoggerHandle  pr_;

  const mesh_info_type meshType_;
  const meshbasis_info_type meshBasisType_; // assuming linear
  databasis_info_type fieldDataBasisType_;
  data_info_type fieldDataType_;
  DATA_TYPE vtkDataType_;

  const VMesh::size_type POINTS_DIM;
  const VMesh::size_type CELL_SIZE;

  PointList::size_type numPointsFromFile_;
  ElemList::size_type numCellsFromFile_;
  PointList::size_type numDataValuesFromFile_;

  std::ifstream fileStream_;
  PointList points_;
  ElemList elements_;

  std::string
  discardBlankAndCommentLines()
  {
    std::string line;
    std::getline(this->fileStream_, line, '\n');
    while ( (! this->fileStream_.eof()) && ( (line.size() < 1) || (line[0] == '#') || (line[0] == '\r') ) )
      std::getline(this->fileStream_, line, '\n');

    return line;
  }

  void
  readPoints();

  VMesh::size_type
  readElements();

  bool
  parseDatasetDefinition(const std::string& line);

  bool
  parsePointsDefinition(const std::string& line);

  bool
  parseCellDefinition(const std::string& line);

  bool
  parseDataDefinition(std::string& line);

  bool
  matchPoints(const std::string& line)
  {
    return ( line.find("POINTS") != std::string::npos );
  }

  bool
  matchCell(const std::string& line)
  {
    return ( ( line.find("POLYGONS") != std::string::npos ) ||
             ( line.find("VERTICES") != std::string::npos ) );
  }

  bool
  matchPointData(const std::string& line)
  {
    return ( line.find("POINT_DATA") != std::string::npos );
  }

  bool
  matchCellData(const std::string& line)
  {
    return ( line.find("CELL_DATA") != std::string::npos );
  }
};
}}}

template<class T>
void
TriSurfScalarDataReaderPrivate<T>::readScalarData()
{
  std::string line;

  this->data_.clear();

  boost::regex re("^[-.]?[[:digit:]].*$", boost::regex::extended);

  int linecounter = 0;

  while ( std::getline(this->fileStream_, line, '\n') )
  {
    // Scalar data should be one continous block.
    // Stop parsing when a blank line or a line not starting with a number is reached.
    if ( boost::regex_match(line, re) )
    {
      T value;
      std::istringstream iss(line);
      iss.exceptions( std::ifstream::badbit );

      try
      {
        while (iss >> value)
        {
          this->data_.push_back(value);
        }
      }
      catch (...)
      {
        // TODO: log error...
        break;
      }
      ++linecounter;
    }
    else
    {
      // end of block
      break;
    }
  }
}

void
VTKToTriSurfReaderPrivate::readPoints()
{
  std::string line;
  PointList values;
  std::ifstream::streampos pos = this->fileStream_.tellg();

  this->points_.clear();

  boost::regex re("^[-.]?[[:digit:]].*$", boost::regex::extended);

  while ( std::getline(this->fileStream_, line, '\n') )
  {
    // Points should be one continous block.
    // Stop parsing when a blank line or a line not starting with a number is reached.
    if ( boost::regex_match(line, re) )
    {
      values.clear();
      if ( multiple_from_string(line, values) )
      {
        BOOST_FOREACH(double v, values)
        {
          this->points_.push_back(v);
        }
      }
      pos = this->fileStream_.tellg();
    }
    else
    {
      this->fileStream_.seekg(pos);
      break;
    }
  }
}

VMesh::size_type
VTKToTriSurfReaderPrivate::readElements()
{
  std::string line;
  std::ifstream::streampos pos = this->fileStream_.tellg();
  VMesh::size_type lineCounter = 0;

  this->elements_.clear();
  this->elements_.resize(0);

  boost::regex re("^[[:digit:]]+[[:space:]]+[[:digit:]]+.*$", boost::regex::extended);

  while ( std::getline(this->fileStream_, line, '\n') )
  {
    // polygons or vertices should be one continous block,
    // each line starting with the number of points
    if ( boost::regex_match(line, re) )
    {
      ElemList ivalues;
      if ( multiple_from_string(line, ivalues) )
      {
        if ( ivalues[0] == ivalues.size()-1 )
        {
          for (ElemList::size_type i = 1; i < ivalues.size(); ++i)
          {
            this->elements_.push_back( ivalues[i] );
          }
        }
        else
        {
          if (this->pr_)
            this->pr_->warning("Discarding line...");
        }
      }
      pos = this->fileStream_.tellg();
    }
    else
    {
      this->fileStream_.seekg(pos);
      break;
    }
    ++lineCounter;
  }
  return lineCounter;
}

bool
VTKToTriSurfReaderPrivate::parseDatasetDefinition(const std::string& line)
{
  StringList strings;
  const StringList::size_type DEF_LEN = 2;
  boost::split( strings, line, boost::is_any_of(" \t") );

  if (strings.size() < DEF_LEN)
  {
    return false;
  }

  if (! ( strings[0] == "DATASET" ) && ( strings[1] == "POLYDATA" ) )
  {
    return false;
  }
  return true;
}

bool
VTKToTriSurfReaderPrivate::parsePointsDefinition(const std::string& line)
{
  StringList strings;
  const StringList::size_type DEF_LEN = 3;
  boost::split( strings, line, boost::is_any_of(" \t") );

  if (strings.size() < DEF_LEN)
  {
    return false;
  }
  std::istringstream iss(strings[1]);
  iss.exceptions( std::ifstream::badbit );
  try
  {
    iss >> this->numPointsFromFile_;
  }
  catch (...)
  {
    return false;
  }
  return true;
}

bool
VTKToTriSurfReaderPrivate::parseCellDefinition(const std::string& line)
{
  StringList strings;
  const StringList::size_type DEF_LEN = 3;
  boost::split( strings, line, boost::is_any_of(" \t") );

  if (strings.size() < DEF_LEN)
  {
    return false;
  }
  std::istringstream iss(strings[1]);
  iss.exceptions( std::ifstream::badbit );
  try
  {
    iss >> this->numCellsFromFile_;
  }
  catch (...)
  {
    return false;
  }
  return true;
}

bool
VTKToTriSurfReaderPrivate::parseDataDefinition(std::string& line)
{
  StringList strings;
  const StringList::size_type DEF_LEN = 2;
  const StringList::size_type ATTR_LEN = 3; // newer versions have up to 4
  bool supportedDataFound = false;

  boost::split( strings, line, boost::is_any_of(" \t") );
  if (strings.size() < DEF_LEN)
  {
    return false;
  }
  std::istringstream iss(strings[1]);
  iss.exceptions( std::ifstream::badbit );
  try
  {
    iss >> this->numDataValuesFromFile_;
  }
  catch (...)
  {
    return false;
  }

  // look for scalar data
  boost::regex re("^[[:upper:]]+.*$", boost::regex::extended);

  while ( std::getline(this->fileStream_, line, '\n') && (! supportedDataFound) )
  {
    strings.clear();
    if ( boost::regex_match(line, re) )
    {
      boost::split( strings, line, boost::is_any_of(" \t") );
      if (strings[0] == "SCALARS")
      {
        vtkDataType_ = SCALAR;
        if (strings.size() < ATTR_LEN)
        {
          continue;
        }
        supportedDataFound = true;

        if (strings[2] == "char")
        {
          this->fieldDataType_ = CHAR_E;
        }
        else if (strings[2] == "unsigned_char")
        {
          this->fieldDataType_ = UNSIGNED_CHAR_E;
        }
        else if (strings[2] == "short")
        {
          this->fieldDataType_ = SHORT_E;
        }
        else if (strings[2] == "unsigned_short")
        {
          this->fieldDataType_ = UNSIGNED_SHORT_E;
        }
        else if (strings[2] == "int")
        {
          this->fieldDataType_ = INT_E;
        }
        else if (strings[2] == "unsigned_int")
        {
          this->fieldDataType_ = UNSIGNED_INT_E;
        }
        else if (strings[2] == "long")
        {
          this->fieldDataType_ = LONGLONG_E;
        }
        else if (strings[2] == "unsigned_long")
        {
          this->fieldDataType_ = UNSIGNED_LONGLONG_E;
        }
        else if (strings[2] == "float")
        {
          this->fieldDataType_ = FLOAT_E;
        }
        else if (strings[2] == "double")
        {
          this->fieldDataType_ = DOUBLE_E;
        }
        else
        {
          return false;
        }
        // ignore LOOKUP_TABLE attribute for now
      }
      else if (strings[0] == "COLOR_SCALARS")
      {
        vtkDataType_ = COLOR;
      }
      // TODO: vectors, tensors
    }
  }

  return supportedDataFound;
}

bool
VTKToTriSurfReaderPrivate::readFile(const std::string& vtk_filename, FieldHandle& fieldHandle)
{
  std::string filename = vtk_filename;
  std::string::size_type pos = filename.find_last_of(".");
  if (pos == std::string::npos)
  {
    filename = filename + ".vtk";
  }

  bool valid_vtk_file = false,
       data_available = false,
       processed_points = false,
       processed_elems = false;
  std::string line;

  try
  {
    this->fileStream_.open(filename.c_str());

    // part 1: header
    std::getline(this->fileStream_, line, '\n');
    boost::char_separator<char> sep(" \t\r");
    boost::tokenizer< boost::char_separator<char> > tokens(line, sep);
    BOOST_FOREACH(std::string t, tokens)
    {
      if ( boost::iequals(t, "vtk") )
      {
        valid_vtk_file = true;
        break;
      }
    }
    if (! valid_vtk_file )
    {
      if (this->pr_)
        this->pr_->error(filename + " is not a valid VTK file. Line 1 should contain 'vtk'.");

      return false;
    }

    // discard title (part 2)
    line = discardBlankAndCommentLines();

    // part 3: data type (ASCII or BINARY)
    line = discardBlankAndCommentLines();

    if ( line.find("ASCII") == std::string::npos )
    {
      if (this->pr_)
        this->pr_->error(filename + " is not a valid VTK file. Line 3 should be ASCII | BINARY.");

      return false;
    }

    // part 4: geometry/topology
    line = discardBlankAndCommentLines();
    PointList::size_type numCells = 0;

    if ( ! parseDatasetDefinition(line) )
    {
      if (this->pr_)
        this->pr_->error(filename + "is not compatible with this importer. Geometry/topology type must be POLYDATA.");

      return false;
    }

    line = discardBlankAndCommentLines();

    if ( matchPoints(line) && (! processed_points) )
    {
      if (! parsePointsDefinition(line) )
      {
        if (this->pr_)
          this->pr_->error(filename +
                           " is not a valid VTK file. Parsing POINTS attribute number of points failed.");
        return false;
      }

      readPoints();
      processed_points = true;
    }
    else if ( matchCell(line) && (! processed_elems) )
    {
      // formatting of polydata topology options (POLYGONS, VERTICES) are the same
      if (! parseCellDefinition(line) )
      {
        if (this->pr_)
          this->pr_->error(filename +
                           " is not a valid VTK file. Dataset topology attributes are number of cells and size of cell list.");
        return false;
      }

      numCells = readElements();
      processed_elems = true;
    }

    line = discardBlankAndCommentLines();

    if ( matchPoints(line) && (! processed_points) )
    {
      if (! parsePointsDefinition(line) )
      {
        if (this->pr_)
          this->pr_->error(filename +
                           " is not a valid VTK file. Parsing POINTS attribute number of points failed.");
        return false;
      }

      readPoints();
      processed_points = true;
    }
    else if ( matchCell(line) && (! processed_elems) )
    {
      // formatting of polydata topology options (POLYGONS, VERTICES) are the same
      if (! parseCellDefinition(line) )
      {
        if (this->pr_)
          this->pr_->error(filename +
            " is not a valid VTK file. Dataset topology attributes are number of cells and size of cell list.");
        return false;
      }

      numCells = readElements();
      processed_elems = true;
    }
    if (! ( processed_points && processed_elems ) )
    {
      if (this->pr_)
        this->pr_->error(filename +
          " is not compatible with this importer. The file should contain points and triangle cells.");

      return false;
    }

    if (points_.size() != this->numPointsFromFile_ * POINTS_DIM)
    {
      if (this->pr_)
        this->pr_->error("Could not read points from " + filename);

      return false;
    }

    if (numCells != this->numCellsFromFile_)
    {
      if (this->pr_)
        this->pr_->error("Could not read triangle elements from " + filename);

      return false;
    }

    // part 5: dataset attributes (data not required)
    // only first dataset will be processed (VTK supports more than one)
    line = discardBlankAndCommentLines();

    if ( matchPointData(line) )
    {
      this->fieldDataBasisType_ = LINEARDATA_E;
      data_available = true;
    }
    else if ( matchCellData(line) )
    {
      this->fieldDataBasisType_ = CONSTANTDATA_E;
      data_available = true;
    }

    if (data_available)
    {
      // sets field data type
      if (! parseDataDefinition(line) )
      {
        if (this->vtkDataType_ == COLOR)
        {
          data_available = false;
          if (this->pr_)
            this->pr_->warning("Color scalars found. This data type is not supported and will be ignored.");
        }
        else
        {
          if (this->pr_)
            this->pr_->error(filename +
              " is not compatible with this importer. Only scalar data is currently supported.");
          return false;
        }
      }
    }

    // first create field and populate mesh, then process data
    fieldHandle = CreateField(this->meshType_,
                              this->meshBasisType_,
                              this->fieldDataBasisType_,
                              this->fieldDataType_);
    if ( !fieldHandle )
    {
      if (this->pr_)
        this->pr_->error("CreateField failed to create a SCIRun field from the VTK file.");

      return false;
    }
    VMesh *vmesh = fieldHandle->vmesh();

    for (PointList::size_type p = 0; p <= this->points_.size()-POINTS_DIM; p += POINTS_DIM)
    {
      vmesh->add_point(Point(this->points_[p], this->points_[p+1], this->points_[p+2]));
    }

    for (ElemList::size_type e = 0; e <= this->elements_.size()-CELL_SIZE; e += CELL_SIZE)
    {
      VMesh::Node::array_type vdata;
      vdata.resize(CELL_SIZE);
      vdata[0] = this->elements_[e]; vdata[1] = this->elements_[e+1]; vdata[2] = this->elements_[e+2];
      vmesh->add_elem(vdata);
    }

    if (data_available)
    {
      // TODO: try using boost::variant here to fix repetitive code
      VField *vfield = fieldHandle->vfield();
      vfield->resize_values();
      switch(this->fieldDataType_)
      {
        case CHAR_E:
        {
          TriSurfScalarDataReaderPrivate<char> dataReader(this->fileStream_);
          dataReader.readScalarData();
          if (this->numDataValuesFromFile_ != dataReader.data_.size())
          {
            if (this->pr_)
              this->pr_->error("Could not read data from " + filename);

            return false;
          }
          vfield->set_values(dataReader.data_);
          break;
        }
        case UNSIGNED_CHAR_E:
        {
          TriSurfScalarDataReaderPrivate<unsigned char> dataReader(this->fileStream_);
          dataReader.readScalarData();
          if (this->numDataValuesFromFile_ != dataReader.data_.size())
          {
            if (this->pr_)
              this->pr_->error("Could not read data from " + filename);

            return false;
          }
          vfield->set_values(dataReader.data_);
          break;
        }
        case SHORT_E:
        {
          TriSurfScalarDataReaderPrivate<short> dataReader(this->fileStream_);
          dataReader.readScalarData();
          if (this->numDataValuesFromFile_ != dataReader.data_.size())
          {
            if (this->pr_)
              this->pr_->error("Could not read data from " + filename);

            return false;
          }
          vfield->set_values(dataReader.data_);
          break;
        }
        case UNSIGNED_SHORT_E:
        {
          TriSurfScalarDataReaderPrivate<unsigned short> dataReader(this->fileStream_);
          dataReader.readScalarData();
          if (this->numDataValuesFromFile_ != dataReader.data_.size())
          {
            if (this->pr_)
              this->pr_->error("Could not read data from " + filename);

            return false;
          }
          vfield->set_values(dataReader.data_);
          break;
        }
        case INT_E:
        {
          TriSurfScalarDataReaderPrivate<int> dataReader(this->fileStream_);
          dataReader.readScalarData();
          if (this->numDataValuesFromFile_ != dataReader.data_.size())
          {
            if (this->pr_)
              this->pr_->error("Could not read data from " + filename);

            return false;
          }
          vfield->set_values(dataReader.data_);
          break;
        }
        case UNSIGNED_INT_E:
        {
          TriSurfScalarDataReaderPrivate<unsigned int> dataReader(this->fileStream_);
          dataReader.readScalarData();
          if (this->numDataValuesFromFile_ != dataReader.data_.size())
          {
            if (this->pr_)
              this->pr_->error("Could not read data from " + filename);

            return false;
          }
          vfield->set_values(dataReader.data_);
          break;
        }
        case LONGLONG_E:
        {
          TriSurfScalarDataReaderPrivate<long long> dataReader(this->fileStream_);
          dataReader.readScalarData();
          if (this->numDataValuesFromFile_ != dataReader.data_.size())
          {
            if (this->pr_)
              this->pr_->error("Could not read data from " + filename);

            return false;
          }
          vfield->set_values(dataReader.data_);
          break;
        }
        case UNSIGNED_LONGLONG_E:
        {
          TriSurfScalarDataReaderPrivate<unsigned long long> dataReader(this->fileStream_);
          dataReader.readScalarData();
          if (this->numDataValuesFromFile_ != dataReader.data_.size())
          {
            if (this->pr_)
              this->pr_->error("Could not read data from " + filename);

            return false;
          }
          vfield->set_values(dataReader.data_);
          break;
        }
        case FLOAT_E:
        {
          TriSurfScalarDataReaderPrivate<float> dataReader(this->fileStream_);
          dataReader.readScalarData();
          if (this->numDataValuesFromFile_ != dataReader.data_.size())
          {
            if (this->pr_)
              this->pr_->error("Could not read data from " + filename);

            return false;
          }
          vfield->set_values(dataReader.data_);
          break;
        }
        case DOUBLE_E:
        {
          TriSurfScalarDataReaderPrivate<double> dataReader(this->fileStream_);
          dataReader.readScalarData();
          if (this->numDataValuesFromFile_ != dataReader.data_.size())
          {
            if (this->pr_)
              this->pr_->error("Could not read data from " + filename);

            return false;
          }
          vfield->set_values(dataReader.data_);
          break;
        }
        default:
          if (this->pr_)
            this->pr_->error("Unsupported data type detected from parsing data section.");
          return false;
      }
    }

    this->fileStream_.close();
  }
  catch (...)
  {
    if (this->pr_)
      this->pr_->error("Could not open file: " + filename);

    return false;
  }

  return true;
}

VTKToTriSurfReader::VTKToTriSurfReader(LoggerHandle pr)
: private_( new VTKToTriSurfReaderPrivate(pr))
{}

bool
VTKToTriSurfReader::run(const std::string& filename, FieldHandle& fieldHandle)
{
  return private_->readFile(filename, fieldHandle);
}
