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


#ifndef GRAPHICS_DATATYPES_GEOMETRY_H
#define GRAPHICS_DATATYPES_GEOMETRY_H

#include <Core/Datatypes/Geometry.h>
#include <Core/GeometryPrimitives/BBox.h>
#include <Core/Algorithms/Visualization/RenderFieldState.h>

//freetype
#include <ft2build.h>
#include FT_FREETYPE_H

#include <glm/glm.hpp>
#include <var-buffer/VarBuffer.hpp>
#include <es-cereal/ComponentSerialize.hpp>
#include <Graphics/Datatypes/share.h>

namespace SCIRun {
  namespace Graphics {
    namespace Datatypes {

      // Schemes individually describing how the data is to be colored.
      // This enumeration may belong in Core/Algorithms/Visualization.
      enum class ColorScheme
      {
        COLOR_UNIFORM = 0,
        COLOR_MAP,
        COLOR_IN_SITU
      };
      /// Different types of rendering support by the system. Strictly speaking,
      /// all of the rendering types can be subsumed in VBO and IBO rendering.
      /// This really just boils down to instanced rendering. Once tesselation
      /// and geometry shaders are supported, we can speed up instanced rendering
      /// in OpenGL.
      enum class RenderType
      {
        RENDER_VBO_IBO,
        RENDER_RLIST_SPHERE,
        RENDER_RLIST_CYLINDER,
      };

      // Could require rvalue references...
      struct SpireVBO
      {
        struct AttributeData
        {
          AttributeData(const std::string& nameIn, size_t sizeIn, bool normalizeIn = false) :
            name(nameIn),
            sizeInBytes(sizeIn),
            normalize(normalizeIn)
          {}

          std::string name;
          size_t      sizeInBytes;
          bool        normalize;
        };

        SpireVBO() : numElements(0), onGPU(false) {}
        SpireVBO(const std::string& vboName, const std::vector<AttributeData> attribs,
          std::shared_ptr<spire::VarBuffer> vboData,
          size_t numVBOElements, const Core::Geometry::BBox& bbox, bool placeOnGPU) :
          name(vboName),
          attributes(attribs),
          data(vboData),
          numElements(numVBOElements),
          boundingBox(bbox),
          onGPU(placeOnGPU)
        {}

        std::string                           name;
        std::vector<AttributeData>            attributes;
        std::shared_ptr<spire::VarBuffer>     data; // Change to unique_ptr w/ move semantics (possibly).
        size_t                                numElements;
        Core::Geometry::BBox                  boundingBox;
        bool                                  onGPU;
      };

      struct SpireIBO
      {
        enum class PRIMITIVE
        {
          POINTS,
          LINES,
          TRIANGLES,
          QUADS
        };

        SpireIBO() : indexSize(0), prim(PRIMITIVE::POINTS) {}
        SpireIBO(const std::string& iboName, PRIMITIVE primIn, size_t iboIndexSize,
          std::shared_ptr<spire::VarBuffer> iboData) :
          name(iboName),
          indexSize(iboIndexSize),
          prim(primIn),
          data(iboData)
        {}

        std::string                           name;
        size_t                                indexSize;
        PRIMITIVE                             prim;
        std::shared_ptr<spire::VarBuffer>     data; // Change to unique_ptr w/ move semantics (possibly).
      };

      struct SpireText
      {
        SpireText() : name(""), width(0), height(0) {}
        SpireText(const char* c, FT_Face f) :
          name(c)
        {
          width = f->glyph->bitmap.width;
          height = f->glyph->bitmap.rows;
          size_t s = width*height;
          bitmap.resize(s);
          std::copy(f->glyph->bitmap.buffer,
            f->glyph->bitmap.buffer + s, bitmap.begin());
        }
        std::string                           name;
        size_t                                width;
        size_t                                height;
        std::vector<uint8_t>                  bitmap;
      };

      struct SpireTexture2D
      {
        SpireTexture2D() : name(""), width(0), height(0) {}
        SpireTexture2D(std::string name , size_t width, size_t height, const char* data) :
          name(name),
          width(width),
          height(height)
        {
            size_t size = width*height*4;
            bitmap.resize(size);
            std::copy(data, data + size, bitmap.begin());
        }
        std::string                           name;
        size_t                                width;
        size_t                                height;
        std::vector<uint8_t>                  bitmap;
      };


      /// Defines a Spire object 'pass'.
      struct SCISHARE SpireSubPass
      {
        SpireSubPass() : renderType(RenderType::RENDER_VBO_IBO), scalar(0), mColorScheme(ColorScheme::COLOR_UNIFORM) {}
        SpireSubPass(const std::string& name, const std::string& vboName,
          const std::string& iboName, const std::string& program,
          ColorScheme scheme, const RenderState& state,
          RenderType renType, const SpireVBO& vbo, const SpireIBO& ibo,
          const SpireText& text, const SpireTexture2D& texture = SpireTexture2D()) :
          passName(name),
          vboName(vboName),
          iboName(iboName),
          programName(program),
          renderState(state),
          renderType(renType),
          vbo(vbo),
          ibo(ibo),
          text(text),
          texture(texture),
          scalar(1.0),
          mColorScheme(scheme)
        {}

        static const char* getName() { return "SpireSubPass"; }

        bool serialize(spire::ComponentSerialize& /* s */, uint64_t /* entityID */)
        {
          // No need to serialize.
          return true;
        }

        std::string   passName;
        std::string   vboName;
        std::string   iboName;
        std::string   programName;
        RenderState   renderState;
        RenderType    renderType;
        SpireVBO			vbo;
        SpireIBO			ibo;
        SpireText     text;//draw a string (usually single character) on geometry
        SpireTexture2D texture;
        double        scalar;


        struct Uniform
        {
          enum class UniformType
          {
            UNIFORM_SCALAR,
            UNIFORM_VEC4
          };

          Uniform() : type(UniformType::UNIFORM_SCALAR) {}

          Uniform(const std::string& nameIn, float scalar) :
            name(nameIn),
            type(UniformType::UNIFORM_SCALAR),
            data(scalar, 0.0f, 0.0f, 0.0f)
          {}

          Uniform(const std::string& nameIn, const glm::vec4& vector) :
            name(nameIn),
            type(UniformType::UNIFORM_VEC4),
            data(vector)
          {}

          std::string   name;
          UniformType   type;
          glm::vec4     data;
        };

        std::vector<Uniform>  mUniforms;
        ColorScheme           mColorScheme;

        void addUniform(const std::string& name, const glm::vec4& vector);
        void addOrModifyUniform(const Uniform& uniform);
        void addUniform(const Uniform& uniform);
      };

      using VBOList = std::list<SpireVBO>;
      using IBOList = std::list<SpireIBO>;
      using PassList = std::list<SpireSubPass>;

      class SCISHARE GeometryObjectSpire : public Core::Datatypes::GeometryObject
      {
      public:
        GeometryObjectSpire(const Core::GeometryIDGenerator& idGenerator, const std::string& tag, bool isClippable);

        //encapsulation phase 1: dumb get/set
        const VBOList& vbos() const { return mVBOs; }
        VBOList& vbos() { return mVBOs; }
        const IBOList& ibos() const { return mIBOs; }
        IBOList& ibos() { return mIBOs; }
        const PassList& passes() const { return mPasses; }
        PassList& passes() { return mPasses; }

        bool isClippable() const {return isClippable_;}

        void setColorMap(const std::string& name) { }
        boost::optional<std::string> colorMap() const { return mColorMap; }

      private:
        VBOList mVBOs;  ///< Array of vertex buffer objects.
        IBOList mIBOs;  ///< Array of index buffer objects.
        PassList  mPasses; /// List of passes to setup.
        bool isClippable_;
        boost::optional<std::string> mColorMap;
      };

      typedef boost::shared_ptr<GeometryObjectSpire> GeometryHandle;

      class SCISHARE CompositeGeometryObject : public GeometryObjectSpire
      {
      public:
        template <typename GeomIter>
          CompositeGeometryObject(const Core::GeometryIDGenerator& idGenerator, const std::string& tag, GeomIter begin, GeomIter end)
          : GeometryObjectSpire(idGenerator, tag, true), geoms_(begin, end)
        {}
        ~CompositeGeometryObject();
        void addToList(Core::Datatypes::GeometryBaseHandle handle, Core::Datatypes::GeomList& list) override;
      private:
        std::vector<GeometryHandle> geoms_;
      };

      template <typename GeomIter>
        static GeometryHandle createGeomComposite(const Core::GeometryIDGenerator& idGenerator, const std::string& tag, GeomIter begin, GeomIter end)
      {
        return boost::make_shared<CompositeGeometryObject>(idGenerator, tag, begin, end);
      }
    }
  }
}


#endif
