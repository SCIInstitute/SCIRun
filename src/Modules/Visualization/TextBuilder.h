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


#ifndef MODULES_VISUALIZATION_TEXTBUILDER_H
#define MODULES_VISUALIZATION_TEXTBUILDER_H

#include <ft2build.h>
#include FT_FREETYPE_H

#include <Graphics/Datatypes/GeometryImpl.h>
#include <Modules/Visualization/share.h>

namespace SCIRun {
  namespace Modules {
    namespace Visualization {

      class SCISHARE TextBuilder
      {
      public:
        TextBuilder();
        ~TextBuilder();
        bool initialize(size_t textSize);
        bool initialize(size_t textSize, const std::string& fontName);

        void setFaceSize(size_t size);
        size_t getFaceSize() const { return ftSize_; }
        std::string getFontName() const { return libName_; }

        void setColor(float r, float g, float b, float a);

        bool isReady() const { return isInit() && isValid(); }

        static void setFSStrings(const std::string &root, const std::string &separator);

        //startNrmSpc: start position in normalized space [[0, 2][0, 2]], origin at lower left corner
        //shiftPxlSpc: shift from start position in pixel space
        void printString(const std::string& oneline,
          const Core::Geometry::Vector &startNrmSpc,
          const Core::Geometry::Vector &shiftPxlSpc,
          const std::string& id,
          Graphics::Datatypes::GeometryObjectSpire& geom) const;

        //get string length based on current settings
        //return value in pixels
        std::tuple<double,double> getStringDims(const std::string& oneline) const;

      private:
        std::string getUniqueFontString(char p, double x, double y, double z, double w, double h) const;

        void initFreeType(const std::string &libName, size_t size);
        void loadNewFace(const std::string &libName, size_t size);
        bool isInit() const { return ftInit_; }
        bool isValid() const { return ftValid_; }

        std::string libName_;
        FT_Library ftLib_;
        FT_Face ftFace_;
        size_t ftSize_;
        bool ftInit_;
        bool ftValid_;
        static std::string mFSRoot;
        static std::string mFSSeparator;
        glm::vec4 color_;
      };



    }
  }
}

#endif
