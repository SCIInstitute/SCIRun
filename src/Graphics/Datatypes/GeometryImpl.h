/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
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

// CPM modules
#include <glm/glm.hpp>
#include <var-buffer/VarBuffer.hpp>
#include <es-cereal/ComponentSerialize.hpp>
#include <Graphics/Datatypes/share.h>

namespace SCIRun {
  namespace Graphics {
    namespace Datatypes {

      // Schemes individually describing how the data is to be colored.
      // This enumeration may belong in Core/Algorithms/Visualization.
      enum ColorScheme
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
      enum RenderType
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
          std::shared_ptr<CPM_VAR_BUFFER_NS::VarBuffer> vboData,
          int64_t numVBOElements, const Core::Geometry::BBox& bbox, bool placeOnGPU) :
          name(vboName),
          attributes(attribs),
          data(vboData),
          numElements(numVBOElements),
          boundingBox(bbox),
          onGPU(placeOnGPU)
        {}

        std::string                           name;
        std::vector<AttributeData>            attributes;
        std::shared_ptr<CPM_VAR_BUFFER_NS::VarBuffer> data; // Change to unique_ptr w/ move semantics (possibly).
        int64_t                               numElements;
        Core::Geometry::BBox                  boundingBox;
        bool                                  onGPU;
      };

      struct SpireIBO
      {
        enum PRIMITIVE
        {
          POINTS,
          LINES,
          TRIANGLES,
        };

        SpireIBO() : indexSize(0), prim(POINTS) {}
        SpireIBO(const std::string& iboName, PRIMITIVE primIn, size_t iboIndexSize,
          std::shared_ptr<CPM_VAR_BUFFER_NS::VarBuffer> iboData) :
          name(iboName),
          indexSize(iboIndexSize),
          prim(primIn),
          data(iboData)
        {}

        std::string                           name;
        size_t                                indexSize;
        PRIMITIVE                             prim;
        std::shared_ptr<CPM_VAR_BUFFER_NS::VarBuffer> data; // Change to unique_ptr w/ move semantics (possibly).
      };


      /// Defines a Spire object 'pass'.
      struct SpireSubPass
      {
        SpireSubPass() : renderType(RENDER_VBO_IBO), scalar(0), mColorScheme(COLOR_UNIFORM) {}
        SpireSubPass(const std::string& name, const std::string& vboName,
          const std::string& iboName, const std::string& program,
          ColorScheme scheme, const RenderState& state,
          RenderType renType, const SpireVBO& vbo, const SpireIBO& ibo) :
          passName(name),
          vboName(vboName),
          iboName(iboName),
          programName(program),
          renderState(state),
          renderType(renType),
          vbo(vbo),
          ibo(ibo),
          scalar(1.0),
          mColorScheme(scheme)
        {}

        static const char* getName() { return "SpireSubPass"; }

        bool serialize(CPM_ES_CEREAL_NS::ComponentSerialize& /* s */, uint64_t /* entityID */)
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
        double        scalar;

        struct Uniform
        {
          enum UniformType
          {
            UNIFORM_SCALAR,
            UNIFORM_VEC4
          };

          Uniform() : type(UNIFORM_SCALAR) {}
          Uniform(const std::string& nameIn, float d) :
            name(nameIn),
            type(UNIFORM_SCALAR),
            data(d, 0.0f, 0.0f, 0.0f)
          {}

          Uniform(const std::string& nameIn, const glm::vec4& vec) :
            name(nameIn),
            type(UNIFORM_VEC4),
            data(vec)
          {}

          std::string   name;
          UniformType   type;
          glm::vec4     data;
        };

        std::vector<Uniform>  mUniforms;
        ColorScheme           mColorScheme;

        void addUniform(const std::string& name, float scalar)
        {
          bool existed = false;
          for (auto& i : mUniforms)
          {
            if (i.name == name && i.type == Uniform::UNIFORM_SCALAR)
            {
              i.data.x = scalar;
              existed = true;
            }
          }
          if (!existed)
            mUniforms.push_back(Uniform(name, scalar));
        }

        void addUniform(const std::string& name, const glm::vec4& vector)
        {
          bool existed = false;
          for (auto& i : mUniforms)
          {
            if (i.name == name && i.type == Uniform::UNIFORM_VEC4)
            {
              i.data = vector;
              existed = true;
            }
          }
          if (!existed)
            mUniforms.push_back(Uniform(name, vector));
        }

        void addUniform(const Uniform& uniform)
        {
          mUniforms.push_back(uniform);
        }
      };

      class SCISHARE GeometryObjectSpire : public Core::Datatypes::GeometryObject
      {
      public:
        GeometryObjectSpire(const Core::GeometryIDGenerator& idGenerator, const std::string& tag);

        std::list<SpireVBO> mVBOs;  ///< Array of vertex buffer objects.
        std::list<SpireIBO> mIBOs;  ///< Array of index buffer objects.

        /// List of passes to setup.
        std::list<SpireSubPass>  mPasses;

        /// Optional colormap name.
        boost::optional<std::string> mColorMap;

        double mLowestValue;    ///< Lowest value a field takes on.
        double mHighestValue;   ///< Highest value a field takes on.

        bool isVisible;
      };

      typedef boost::shared_ptr<GeometryObjectSpire> GeometryHandle;
    }
  }
}


#endif
