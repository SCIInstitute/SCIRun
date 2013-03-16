/*
 For more information, please see: http://software.sci.utah.edu
 
 The MIT License
 
 Copyright (c) 2013 Scientific Computing and Imaging Institute,
 University of Utah.
 
 License for the specific language governing rights and limitations under
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
#include <string>
#include <vector>

#include <boost/filesystem.hpp>
#include <boost/shared_ptr.hpp>

#include <Core/Utils/FileUtil.h>
#include <Core/Utils/StringUtil.h>
#include <Core/Utils/Lockable.h>

#include <Core/Algorithms/DataIO/TextToTriSurfField.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>

namespace bfs=boost::filesystem;

using namespace SCIRun::Core::Algorithms;

namespace SCIRun {
namespace Core {
namespace Algorithms {
namespace DataIO {

  
class TextToTriSurfFieldPrivate : public Lockable
{
public:
  TextToTriSurfFieldPrivate(const TextToTriSurfFieldAlgorithm& algo) :
    numberPointsFromHeader_(-1),
    numberFacesFromHeader_(-1),
    hasPointsHeader_(false),
    hasFacesHeader_(false),
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
  void readPoints();
  void readFaces();

private:
  int numberPointsFromHeader_;
  int numberFacesFromHeader_;
  bool hasPointsHeader_;
  bool hasFacesHeader_;
  const TextToTriSurfFieldAlgorithm& algo_;
};

bool TextToTriSurfFieldPrivate::validatePointsFile(const std::string& filename)
{
  std::ifstream file;
  file.exceptions( std::ifstream::badbit );
  file.open(filename.c_str());

  std::string line;
  bool first_line = true;

  int ncols = 0;
  int nrows = 0;
  int lineColumnCount = 0;

  while (getline(file, line, '\n'))
  {
    if (line.size() > 0)
    {
      // block out comments
      if ( lineStartsWithComment(line) ) continue;
    }
    
    replaceDelimitersWithWhitespace(line);
    std::vector<double> values(std::move(parseLineOfNumbers<double>(line)));
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

  if (-1 == numberPointsFromHeader_)
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

  file.close();
  return true;
}

bool TextToTriSurfFieldPrivate::validateFacesFile(const std::string& filename)
{
  std::ifstream file;
  file.exceptions( std::ifstream::badbit );
  file.open(filename.c_str());
  
  std::string line;
  bool first_line = true;
  
  int ncols = 0;
  int nrows = 0;
  int lineColumnCount = 0;
  
  while (getline(file, line, '\n'))
  {
    if (line.size() > 0)
    {
      // block out comments
      if ( lineStartsWithComment(line) ) continue;
    }
    
    replaceDelimitersWithWhitespace(line);
    std::vector<double> values(std::move(parseLineOfNumbers<double>(line)));
    lineColumnCount = values.size();
    
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
  
  if (-1 == numberFacesFromHeader_)
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
  
  file.close();
  return true;
}

void TextToTriSurfFieldPrivate::readPoints()
{
  std::cerr << "Number of points=" << numberPointsFromHeader_ << std::endl;
}

void TextToTriSurfFieldPrivate::readFaces()
{
  std::cerr << "Number of faces=" << numberFacesFromHeader_ << std::endl;
}
  
// Text file format for tetrahedral meshes (files supported with or without headers)0
// points: x y z
// elems: n1 n2 n3
void
TextToTriSurfFieldAlgorithm::run(const std::string& filename)
{
//  FieldHandle result = 0;
//  
//  std::string elems_fn(filename);
//  std::string pts_fn(filename);
  TextToTriSurfFieldPrivate privateImpl(*this);

  bool validPointsFile = false, validFacesFile = false;

  // starting with usual .pts and .fac combination, beginning with
  // .pts as the supplied file
  if ( privateImpl.validPointsFileExtention(filename) )
  {
    try
    {
      {
        TextToTriSurfFieldPrivate::LockType lock( privateImpl.getMutex() );
        ENSURE_FILE_EXISTS(filename);
      }
      std::string stem(bfs::path(filename).stem().c_str());
      this->remark("Reading triangle surface from " + stem);
      validPointsFile = privateImpl.validatePointsFile(filename);

      if (! validPointsFile)
      {
        std::ostringstream oss;
        oss << filename << " is invalid. Unable to continue reading triangle surface.";
        error(oss.str());
        return;
      }

      bfs::path facesFilePath = bfs::path(filename).replace_extension(".fac");
      {
        TextToTriSurfFieldPrivate::LockType lock( privateImpl.getMutex() );
        ENSURE_FILE_EXISTS(facesFilePath.string());
      }
      validFacesFile = privateImpl.validateFacesFile(facesFilePath.string());
      if (! validFacesFile)
      {
        std::ostringstream oss;
        oss << facesFilePath.string() << " is invalid. Unable to continue reading triangle surface.";
        error(oss.str());
        return;
      }
      
      privateImpl.readPoints();
      privateImpl.readFaces();
    }
    catch(std::ifstream::failure e)
    {
      std::ostringstream oss;
      oss << "Parsing triangle surface points file " << filename << " failed with error code " << e.code() << ".\n" << e.what();
      error(oss.str());
    }
    // TODO: other exceptions?
  }
}
  
  

}}}}