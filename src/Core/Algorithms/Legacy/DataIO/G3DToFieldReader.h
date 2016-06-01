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


#ifndef CORE_ALGORITHMS_DATAIO_G3DTOFIELDREADER_H
#define CORE_ALGORITHMS_DATAIO_G3DTOFIELDREADER_H 1

#include <Core/Algorithms/Base/AlgorithmBase.h>
#include <Core/Algorithms/Legacy/DataIO/share.h>

/*
 * Implementation notes:
 *
 * This reader does not read textures, just geometry, since
 * ImageVis3D writes out obj files that only contain geometry.
 * The intended use of this reader is for reading ImageVis3D obj files.
 */

namespace SCIRun
{
  namespace Core
  {
    namespace Algorithms
    {
      class SCISHARE G3DToFieldReader : public AlgorithmBase
      {
      public:
      public:

        enum AttributeSemantic
        {
          Position,
          Normal,
          Tangent,
          Color,
          Tex,
          Float
        };

        enum PrimitiveType
        {
          Point,
          Line,
          Triangle,
          TriangleAdj
        };

        enum VertexType
        {
          SoA,
          AoS
        };

        struct GeometryInfo
        {
          GeometryInfo() : vertexType(AoS), numberPrimitives(0), primitiveType(Triangle), numberIndices(0), numberVertices(0), vertexSize(0), indexSize(0), isOpaque(true) {}
          uint32_t vertexType;
          uint32_t numberPrimitives;
          uint32_t primitiveType;
          uint32_t numberIndices;
          uint32_t numberVertices;
          uint32_t vertexSize;
          uint32_t indexSize;
          bool isOpaque;

          std::vector<uint32_t> attributeSemantics;
        };

        struct Geometry
        {
          Geometry() : indices(NULL) {}
          GeometryInfo info;

          uint32_t * indices;
        };

        struct GeometryAoS : Geometry
        {
          GeometryAoS() : Geometry(), vertices(NULL)
          {
            info.vertexType = AoS;
          }

          float * vertices;
        };

        struct GeometrySoA : Geometry
        {
          GeometrySoA() : Geometry()
          {
            info.vertexType = SoA;
          }

          std::vector<float*> vertexAttributes;
        };

        static uint32_t floats(uint32_t semantic)
        {
          switch (semantic)
          {
          case Position:
            return 3;
          case Normal:
            return 3;
          case Tangent:
            return 3;
          case Color:
            return 4;
          case Tex:
            return 2;
          case Float:
            return 1;
          default:
            return 0;
          }
        }

        explicit G3DToFieldReader(Logging::LoggerHandle log);
        bool read(const std::string& filename, FieldHandle& field_handle);
        bool write(const std::string& filename, const FieldHandle& field);
        virtual AlgorithmOutput run(const AlgorithmInput&) const override { throw "not implemented"; }

        void write(const std::string & file, const GeometryAoS * const geometry, const uint32_t vertexType = AoS);
        void write(const std::string & file, const GeometrySoA * const geometry, const uint32_t vertexType = SoA);
        void read(const std::string & file, GeometryAoS * const geometry);
        void read(const std::string & file, GeometrySoA * const geometry);
        void print(const Geometry * const geometry, std::ostream & output);
        void clean(GeometryAoS * geometry);
        void clean(GeometrySoA * geometry);
      
      private:
        Logging::LoggerHandle log_;

        void writeHeader(std::fstream & fs, const GeometryInfo & info, const uint32_t * const vertexType = NULL);
        void writeIndices(std::fstream & fs, const uint32_t * const indices, const GeometryInfo & info);
        void writeVertices(std::fstream & fs, const float * const vertices, const GeometryInfo & info);
        void writeVertices(std::fstream & fs, const std::vector<float*> & vertexAttributes, const GeometryInfo & info);
        void writeContent(std::fstream & fs, const GeometryAoS & geometry);
        void writeContent(std::fstream & fs, const GeometrySoA & geometry);
        
        void readHeader(std::fstream & fs, GeometryInfo & info);
        void readIndices(std::fstream & fs, uint32_t *& indices, const GeometryInfo & info);
        void readVertices(std::fstream & fs, float *& vertices, const GeometryInfo & info);
        void readVertices(std::fstream & fs, std::vector<float*> & vertexAttributes, const GeometryInfo & info);
        void readContent(std::fstream & fs, GeometryAoS & geometry);
        void readContent(std::fstream & fs, GeometrySoA & geometry);
        
        void convertVertices(const std::vector<float*> & vertexAttributes, float *& vertices, const GeometryInfo & info);
        void convertVertices(const float * const vertices, std::vector<float*> & vertexAttributes, const GeometryInfo & info);

        void cleanIndices(uint32_t * indices);
        void cleanVertices(float * vertices);
        void cleanVertices(std::vector<float*> & vertexAttributes);
      };
    }
  }
}

#endif // CORE_ALGORITHMS_DATAIO_G3DTOFIELDREADER_H
