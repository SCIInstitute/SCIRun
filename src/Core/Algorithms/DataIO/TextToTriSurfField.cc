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


#include <fstream>
#include <iostream>
#include <ios>
#include <string>
#include <vector>

#include <boost/filesystem.hpp>
#include <boost/shared_ptr.hpp>

#include <Core/Utils/FileUtil.h>
#include <Core/Utils/StringUtil.h>
#include <Core/Utils/Lockable.h>

#include <Core/Algorithms/DataIO/TextToTriSurfField.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>

#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>

#include <Core/GeometryPrimitives/Point.h>


namespace bfs=boost::filesystem;

using namespace SCIRun;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;

namespace SCIRun {
namespace Core {
namespace Algorithms {
namespace DataIO {

class TextToTriSurfFieldPrivate : public Lockable
{
public:
  TextToTriSurfFieldPrivate(const TextToTriSurfFieldAlgorithm& algo) :
    numberPointsFromHeader_(0),
    numberFacesFromHeader_(0),
    hasPointsHeader_(false),
    hasFacesHeader_(false),
    zeroBased_(false),
    algo_(algo)
  {}

  // be careful here - doesn't account for \r (CR, classic OS X line delimiter)
  // should work for Windows (CRFL, \r\n)
  bool validatePointsFile(const std::string& filename);
  bool validateFacesFile(const std::string& filename);

  bool hasPointsHeader() const { return hasPointsHeader_; }
  bool hasFacesHeader() const { return hasFacesHeader_; }

  // unrecognized extension: .fac or .tri for faces, .pts or .pos for points
  bool validPointsFileExtention(const std::string& filename)
  {
    return bfs::extension(filename) == ".pts" || bfs::extension(filename) == ".pos";
  }

  bool validFacesFileExtention(const std::string& filename)
  {
    return bfs::extension(filename) == ".fac" || bfs::extension(filename) == ".tri";
  }

  // these files just contain geometry (note that data can be applied to field from matrix)
  MeshHandle readField();

private:
  class FileStreamWrapper
  {
  public:
    FileStreamWrapper(const std::string& filename)
    {
      stream_.exceptions( std::ifstream::badbit );
      stream_.open(filename.c_str());
    }
    ~FileStreamWrapper()
    {
      stream_.close();
    }

    std::ifstream stream_;
  };

  size_t numberPointsFromHeader_;
  size_t numberFacesFromHeader_;

  bool hasPointsHeader_;
  bool hasFacesHeader_;
  bool zeroBased_;

  bfs::path pointsFilePath_;
  bfs::path facesFilePath_;

  const TextToTriSurfFieldAlgorithm& algo_;
};

bool TextToTriSurfFieldPrivate::validatePointsFile(const std::string& filename)
{
  FileStreamWrapper wrapper(filename);

  std::string line;
  bool first_line = true;

  size_t ncols = 0, nrows = 0, lineColumnCount = 0;

  while (getline(wrapper.stream_, line, '\n'))
  {
    if ( lineStartsWithComment(line) ) continue;

    replaceDelimitersWithWhitespace(line);
    auto values(parseLineOfNumbers<double>(line));
    lineColumnCount = values.size();

    if (first_line)
    {
      if (lineColumnCount == 1)
      {
        hasPointsHeader_ = true;
        numberPointsFromHeader_ = static_cast<int>(values[0]) + 1;
      }
      else if ((lineColumnCount == 3) || (lineColumnCount == 2))
      {
        hasPointsHeader_ = false;
        first_line = false;
        nrows++;
        ncols = lineColumnCount;
      }
      else
      {
        algo_.error("Invalid file format. The first line must either contain 2D or 3D point coordinates, or a header (single number indicating number of points in file)");
        return false;
      }
    }
    else
    {
      if (lineColumnCount > 0)
      {
        nrows++;
        if (ncols > 0)
        {
          if (ncols != lineColumnCount)
          {
            algo_.error("Improper format of text file, not every line contains the same amount of points");
            return false;
          }
        }
        else
        {
          ncols = lineColumnCount;
        }
      }
    }
  }

  if (0 == numberPointsFromHeader_)
  {
    numberPointsFromHeader_ = nrows;
  }
  else if ( hasPointsHeader_ && (nrows != numberPointsFromHeader_) )
  {
    // let it pass anyways
    std::ostringstream oss;
    oss << "Number of points listed in header (" << numberPointsFromHeader_ << ") does not match number of non-header rows in file (" << nrows <<  "). Using number of points from header.";
    algo_.warning(oss.str());
  }

  pointsFilePath_ = filename;
  return true;
}

bool TextToTriSurfFieldPrivate::validateFacesFile(const std::string& filename)
{
  FileStreamWrapper wrapper(filename);

  std::string line;
  bool first_line = true;

  size_t ncols = 0, nrows = 0, lineColumnCount = 0;

  while (getline(wrapper.stream_, line, '\n'))
  {
    if ( lineStartsWithComment(line) ) continue;

    replaceDelimitersWithWhitespace(line);
    auto values(parseLineOfNumbers<double>(line));
    lineColumnCount = values.size();

    for (auto j = 0; j < values.size(); ++j)
      if (values[j] == 0.0) zeroBased_ = true;

    if (first_line)
    {
      if (lineColumnCount == 1)
      {
        hasFacesHeader_ = true;
        numberFacesFromHeader_ = static_cast<int>(values[0]) + 1;
      }
      else if (lineColumnCount == 3)
      {
        hasFacesHeader_ = false;
        first_line = false;
        nrows++;
        ncols = lineColumnCount;
      }
      else
      {
        algo_.error("Invalid file format. The first line must either 3 triangle faces indices, or a header (single number indicating number of points in file)");
        return false;
      }
    }
    else
    {
      if (lineColumnCount > 0)
      {
        nrows++;
        if (ncols > 0)
        {
          if (ncols != lineColumnCount)
          {
            algo_.error("Improper format of text file, not every line contains the same amount of coordinates");
            return false;
          }
        }
        else
        {
          ncols = lineColumnCount;
        }
      }
    }
  }

  if (0 == numberFacesFromHeader_)
  {
    numberFacesFromHeader_ = nrows;
  }
  else if ( hasFacesHeader_ && (nrows != numberFacesFromHeader_) )
  {
    // let it pass anyways
    std::ostringstream oss;
    oss << "Number of faces listed in header (" << numberFacesFromHeader_ << ") does not match number of non-header rows in file (" << nrows <<  "). Using number of faces from header.";
    algo_.warning(oss.str());
  }

  facesFilePath_ = filename;
  return true;
}

MeshHandle TextToTriSurfFieldPrivate::readField()
{
  FieldInformation fi("TriSurfMesh", LINEARDATA_E, "double");
  auto triSurfMesh = CreateMesh(fi);
  auto triSurfVMesh = triSurfMesh->vmesh();

  triSurfVMesh->node_reserve(numberPointsFromHeader_);
  triSurfVMesh->elem_reserve(numberPointsFromHeader_);

  FileStreamWrapper pointsStreamWrapper(pointsFilePath_.string());

  std::string line;
  for(auto i = 0; i < numberPointsFromHeader_ && getline(pointsStreamWrapper.stream_, line, '\n'); ++i)
  {
    // block out comments
    if ( lineStartsWithComment(line) ) continue;
    replaceDelimitersWithWhitespace(line);

    auto values(parseLineOfNumbers<double>(line));

    if (values.size() == 3) triSurfVMesh->add_point(Point(values[0],values[1],values[2]));
    if (values.size() == 2) triSurfVMesh->add_point(Point(values[0],values[1],0.0));
  }

  FileStreamWrapper facesStreamWrapper(facesFilePath_.string());

  VMesh::Node::array_type vdata;
  vdata.resize(3);

  for (auto i = 0; i < numberFacesFromHeader_ && getline(facesStreamWrapper.stream_, line,'\n'); ++i)
  {
    if ( lineStartsWithComment(line) ) continue;
    replaceDelimitersWithWhitespace(line);

    auto ivalues(parseLineOfNumbers<VMesh::index_type>(line));

    for (auto j = 0; j < ivalues.size() && j < 3; j++)
    {
      if (zeroBased_) vdata[j] = ivalues[j];
      else vdata[j] = ivalues[j]-1;
    }

    if (ivalues.size() > 2) triSurfVMesh->add_elem(vdata);
  }

  return triSurfMesh;
}

// Text file format for tetrahedral meshes (files supported with or without headers)
// points: x y z
// elems: n1 n2 n3
MeshHandle TextToTriSurfFieldAlgorithm::run(const std::string& filename)
{
  TextToTriSurfFieldPrivate privateImpl(*this);

  bool validPointsFile = false, validFacesFile = false;

  try
  {
    // starting with usual .pts and .fac combination, beginning with
    // .pts as the supplied file
    if ( privateImpl.validPointsFileExtention(filename) )
    {
      {
        TextToTriSurfFieldPrivate::lock_type lock( privateImpl.get_mutex() );
        ENSURE_FILE_EXISTS(filename);
      }
      std::string stem(bfs::path(filename).stem().string());
      this->remark("Reading triangle surface from " + stem);
      validPointsFile = privateImpl.validatePointsFile(filename);

      if (! validPointsFile)
      {
        std::ostringstream oss;
        oss << filename << " is invalid. Unable to continue reading triangle surface.";
        error(oss.str());
        return MeshHandle();
      }

      bool foundFacesFile = false;
      bfs::path facesFilePath = bfs::path(filename).replace_extension(".fac");
      {
        TextToTriSurfFieldPrivate::lock_type lock( privateImpl.get_mutex() );
        if ( boost::filesystem::exists(facesFilePath) )
        {
          foundFacesFile = true;
        }
      }
      // try again with .tri
      if (! foundFacesFile )
      {
        facesFilePath = bfs::path(filename).replace_extension(".tri");
        TextToTriSurfFieldPrivate::lock_type lock( privateImpl.get_mutex() );
        if ( boost::filesystem::exists(facesFilePath) )
        {
          foundFacesFile = true;
        }
      }

      if (foundFacesFile)
        validFacesFile = privateImpl.validateFacesFile(facesFilePath.string());

      // can't continue without a valid faces file
     if (! (validFacesFile && foundFacesFile) )
      {
        std::ostringstream oss;
        oss << "Cannot locate a valid faces file corresponding to " << filename << ". Unable to continue reading triangle surface.";
        error(oss.str());
        return MeshHandle();
      }
    }
    else if ( privateImpl.validFacesFileExtention(filename) ) // if .fac file was supplied
    {
      {
        TextToTriSurfFieldPrivate::lock_type lock( privateImpl.get_mutex() );
        ENSURE_FILE_EXISTS(filename);
      }
      std::string stem(bfs::path(filename).stem().string());
      this->remark("Reading triangle surface from " + stem);
      validFacesFile = privateImpl.validateFacesFile(filename);

      if (! validFacesFile)
      {
        std::ostringstream oss;
        oss << filename << " is invalid. Unable to continue reading triangle surface.";
        error(oss.str());
        return MeshHandle();
      }

      bool foundPointsFile = false;
      bfs::path pointsFilePath = bfs::path(filename).replace_extension(".pts");
      {
        TextToTriSurfFieldPrivate::lock_type lock( privateImpl.get_mutex() );
        if ( boost::filesystem::exists(pointsFilePath) )
        {
          foundPointsFile = true;
        }
      }
      // try again with .pos
      if (! foundPointsFile )
      {
        pointsFilePath = bfs::path(filename).replace_extension(".pos");
        TextToTriSurfFieldPrivate::lock_type lock( privateImpl.get_mutex() );
        if ( boost::filesystem::exists(pointsFilePath) )
        {
          foundPointsFile = true;
        }
      }

      if (foundPointsFile)
        validPointsFile = privateImpl.validatePointsFile(pointsFilePath.string());

      // can't continue without a valid points file
      if (! (validPointsFile && foundPointsFile) )
      {
        std::ostringstream oss;
        oss << "Cannot locate a valid points file corresponding to " << filename << ". Unable to continue reading triangle surface.";
        error(oss.str());
        return MeshHandle();
      }
    }

    /// @todo: change to FieldHandle when available...
    auto mesh = privateImpl.readField();
    return mesh;
  }
  catch(std::ifstream::failure& e)
  {
    std::ostringstream oss;
    oss << "Parsing triangle surface points file " << filename << " failed.\n" << e.what();
    error(oss.str());
  }
  catch(AlgorithmInputException& e)
  {
    std::ostringstream oss;
    oss << "Locating input file " << filename << " failed " << e.what();
    error(oss.str());
  }
  /// @todo: other exceptions?
  return MeshHandle();
}

AlgorithmOutput TextToTriSurfFieldAlgorithm::run(const AlgorithmInput& input) const
{
  throw 2;
}

}}}}
