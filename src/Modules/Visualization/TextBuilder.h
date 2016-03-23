/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
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

#ifndef MODULES_VISUALIZATION_TEXTBUILDER_H
#define MODULES_VISUALIZATION_TEXTBUILDER_H

//freetype
#include <ft2build.h>
#include FT_FREETYPE_H

#include <Core/GeometryPrimitives/GeomFwd.h>
#include <Graphics/Datatypes/GeometryImpl.h>
#include <Modules/Visualization/share.h>
#include <glm/glm.hpp>
#include <string>

namespace SCIRun {
  namespace Modules {
    namespace Visualization {

      class SCISHARE TextBuilder
      {
      public:
        TextBuilder();
        ~TextBuilder();

        void initFreeType(const std::string &libName, size_t size);
        void loadNewFace(const std::string &libName, size_t size);
        void setFaceSize(size_t size);
        size_t getFaceSize() { return ftSize_; }
        void setColor(const glm::vec4 &color) { color_ = color; }

        bool isInit() { return ftInit_; }
        bool isValid() { return ftValid_; }

        static void setFSStrings(std::string &root, std::string &separator);

        std::string getUniqueFontString(const char *p, double x,
          double y, double z, double w, double h);

        //startNrmSpc: start position in normalized space [[0, 2][0, 2]], origin at lower left corner
        //shiftPxlSpc: shift from start position in pixel space
        void printString(const std::string oneline,
          const Core::Geometry::Vector &startNrmSpc,
          const Core::Geometry::Vector &shiftPxlSpc,
          const std::string& id,
          Graphics::Datatypes::GeometryHandle geom);
        //get string length based on current settings
        //return value in pixels
        double getStringLen(const std::string oneline);

      private:
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
