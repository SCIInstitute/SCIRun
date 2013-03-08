/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2012 Scientific Computing and Imaging Institute,
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


#ifndef CORE_DATATYPES_GEOMETRY_H
#define CORE_DATATYPES_GEOMETRY_H 

#include <cstdint>
#include <list>
#include <Core/Datatypes/Datatype.h>
#include <Core/Datatypes/Share.h>

namespace SCIRun {
namespace Core {
namespace Datatypes {

  class SCISHARE GeometryObject : public Datatype
  {
  public:
    explicit GeometryObject(DatatypeConstHandle dh);
    GeometryObject(const GeometryObject& other);
    GeometryObject& operator=(const GeometryObject& other);
    DatatypeConstHandle get_underlying() const;
    virtual GeometryObject* clone() const { return new GeometryObject(*this); }

    std::string objectName;     ///< Name of this object. Should be unique 
                                ///< across all modules in the network.

    // Could require rvalue references...
    struct SpireVBO
    {
      SpireVBO(const std::string& vboName, const std::vector<std::string> attribs,
               std::shared_ptr<std::vector<uint8_t>> vboData) :
          name(vboName),
          attribs(attributeNames),
          data(vboData)
      {}

      std::string                           name;
      std::vector<std::string>              attributeNames;
      std::shared_ptr<std::vector<uint8_t>> data;
    };

    // Could require rvalue references...
    struct SpireIBO
    {
      SpireIBO(const std::string& iboName, size_t iboIndexSize,
               std::shared_ptr<std::vector<uint8_t>> iboData) :
          name(iboName),
          indexSize(iboIndexSize),
          data(iboData)
      {}

      std::string                           name;
      size_t                                indexSize;
      std::shared_ptr<std::vector<uint8_t>> data;
    };

    std::list<SpireVBO> mVBOs;  ///< Array of vertex buffer objects.
    std::list<SpireIBO> mIBOs;  ///< Array of index buffer objects.

    /// Defines a Spire object 'pass'.
    struct SpirePass
    {
      SpirePass(const std::string& name, const std::string& vbo, 
                const std::string& ibo, const std::string& program) :
          passName(name),
          vboName(vbo),
          iboName(ibo),
          programName(program)
      {}

      std::string   passName;
      std::string   vboName;
      std::string   iboName;
      std::string   programName;
    };

    /// Array of passes to setup.
    std::list<SpirePass>  mPasses;

    bool      useZTest;
    bool      useZWrite;

  private:
    DatatypeConstHandle data_;
  };

  typedef boost::shared_ptr<GeometryObject> GeometryHandle;

}}}


#endif
