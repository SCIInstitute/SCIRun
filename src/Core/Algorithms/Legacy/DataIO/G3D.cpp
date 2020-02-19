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


#include "src/duality/G3D.h"

#include "duality/Error.h"
#include "src/duality/AbstractIO.h"

#include <string>

std::unique_ptr<G3D::GeometrySoA> G3D::createLineGeometry(std::vector<uint32_t> indices, std::vector<float> positions,
                                                          std::vector<float> colors) {
    auto geometry = std::make_unique<G3D::GeometrySoA>();

    const uint32_t numberIndices = static_cast<uint32_t>(indices.size());
    const uint32_t numberPositions = static_cast<uint32_t>(positions.size());
    const uint32_t indicesPerPrimitive = 2;

    geometry->info.primitiveType = G3D::Line;
    geometry->info.indexSize = sizeof(uint32_t);
    geometry->info.numberIndices = numberIndices;

    geometry->info.numberPrimitives = numberIndices / indicesPerPrimitive;
    geometry->info.numberVertices = numberPositions / G3D::floats(G3D::AttributeSemantic::Position);
    geometry->info.vertexSize = sizeof(float) * G3D::floats(G3D::AttributeSemantic::Position) +
                                sizeof(float) * G3D::floats(G3D::AttributeSemantic::Color); // vertex size in bytes for all attributes
    geometry->info.vertexType = G3D::SoA;
    geometry->info.attributeSemantics.push_back(G3D::AttributeSemantic::Position);
    geometry->info.attributeSemantics.push_back(G3D::AttributeSemantic::Color);

    geometry->indices = indices;

    geometry->vertexAttributes.push_back(std::move(positions));
    geometry->vertexAttributes.push_back(std::move(colors));

    return geometry;
}


void G3D::writeHeader(AbstractWriter& writer, const GeometryInfo& info, const uint32_t* const vertexType) {
    writer.write((char*)&info.isOpaque, sizeof(bool));
    writer.write((char*)&info.numberPrimitives, sizeof(uint32_t));
    writer.write((char*)&info.primitiveType, sizeof(uint32_t));

    uint32_t numberSemantics = (uint32_t)info.attributeSemantics.size();
    writer.write((char*)&numberSemantics, sizeof(uint32_t));

    writer.write((char*)&info.numberIndices, sizeof(uint32_t));
    writer.write((char*)&info.indexSize, sizeof(uint32_t));
    writer.write((char*)&info.numberVertices, sizeof(uint32_t));
    writer.write((char*)&info.vertexSize, sizeof(uint32_t));

    writer.write((char*)(vertexType ? vertexType : &info.vertexType), sizeof(uint32_t));

    writer.write((char*)&(info.attributeSemantics.at(0)), sizeof(uint32_t) * numberSemantics);
}

void G3D::writeIndices(AbstractWriter& writer, const std::vector<uint32_t>& indices, const GeometryInfo& info) {
    writer.write((char*)indices.data(), info.numberIndices * info.indexSize);
}

void G3D::writeVertices(AbstractWriter& writer, const std::vector<float>& vertices, const GeometryInfo& info) {
    writer.write((char*)vertices.data(), info.numberVertices * info.vertexSize);
}

void G3D::writeContent(AbstractWriter& writer, const GeometryAoS& geometry) {
    writeIndices(writer, geometry.indices, geometry.info);
    writeVertices(writer, geometry.vertices, geometry.info);
}

void G3D::write(AbstractWriter& writer, const GeometryAoS& geometry, uint32_t vertexType) {
    if (writer.isOpen()) {
        if (vertexType == AoS) {
            writeHeader(writer, geometry.info);
            writeContent(writer, geometry);
        } else if (vertexType == SoA) {
            writeHeader(writer, geometry.info, &vertexType);
            writeIndices(writer, geometry.indices, geometry.info);
            auto vertexAttributes = convertVertices(geometry.vertices, geometry.info);
            writeVertexAttributes(writer, vertexAttributes, geometry.info);
        }
    }
}

void G3D::writeVertexAttributes(AbstractWriter& writer, const std::vector<std::vector<float>>& vertexAttributes, const GeometryInfo& info) {
    uint32_t i = 0;
    for (auto semantic : info.attributeSemantics) {
        writer.write((char*)vertexAttributes.at(i).data(), info.numberVertices * floats(semantic) * sizeof(float));
        ++i;
    }
}

void G3D::writeContent(AbstractWriter& writer, const GeometrySoA& geometry) {
    writeIndices(writer, geometry.indices, geometry.info);
    writeVertexAttributes(writer, geometry.vertexAttributes, geometry.info);
}

void G3D::write(AbstractWriter& writer, const GeometrySoA& geometry, uint32_t vertexType) {
    if (writer.isOpen()) {
        if (vertexType == SoA) {
            writeHeader(writer, geometry.info);
            writeContent(writer, geometry);
        } else if (vertexType == AoS) {
            writeHeader(writer, geometry.info, &vertexType);
            writeIndices(writer, geometry.indices, geometry.info);
            auto vertices = convertVertices(geometry.vertexAttributes, geometry.info);
            writeVertices(writer, vertices, geometry.info);
        }
    }
}

G3D::GeometryInfo G3D::readHeader(AbstractReader& reader) {
    GeometryInfo info;
    std::vector<char> buffer(8 * sizeof(uint32_t) + sizeof(bool), 0);
    char* bufferPtr = buffer.data();
    reader.read(buffer.data(), 8 * sizeof(uint32_t) + sizeof(bool));
    info.isOpaque = ((bufferPtr++)[0] == 1);
    info.numberPrimitives = ((uint32_t*)bufferPtr)[0];
    info.primitiveType = ((uint32_t*)bufferPtr)[1];
    uint32_t numberSemantics = ((uint32_t*)bufferPtr)[2];
    info.numberIndices = ((uint32_t*)bufferPtr)[3];
    info.indexSize = ((uint32_t*)bufferPtr)[4];
    info.numberVertices = ((uint32_t*)bufferPtr)[5];
    info.vertexSize = ((uint32_t*)bufferPtr)[6];
    info.vertexType = ((uint32_t*)bufferPtr)[7];

    uint32_t divisor =
        (info.primitiveType == Point) ? 1 : (info.primitiveType == Line) ? 2 : (info.primitiveType == Triangle)
                                                                                   ? 3
                                                                                   : (info.primitiveType == TriangleAdj) ? 6 : 0;
    if (divisor > 0 && (info.numberIndices / divisor) == info.numberPrimitives) {
        std::vector<uint32_t> buffer(numberSemantics * sizeof(uint32_t));
        reader.read((char*)buffer.data(), numberSemantics * sizeof(uint32_t));
        for (uint32_t i = 0; i < numberSemantics; ++i) {
            info.attributeSemantics.push_back(buffer[i]);
        }
    } else
        info.numberVertices = 0;
    return info;
}

std::vector<uint32_t> G3D::readIndices(AbstractReader& reader, const GeometryInfo& info) {
    std::vector<uint32_t> indices(info.numberIndices);
    reader.read((char*)indices.data(), info.numberIndices * info.indexSize);
    return indices;
}

std::vector<float> G3D::readVertices(AbstractReader& reader, const GeometryInfo& info) {
    std::vector<float> vertices;
    reader.read((char*)vertices.data(), info.numberVertices * info.vertexSize);
    return vertices;
}

void G3D::readContent(AbstractReader& reader, GeometryAoS& geometry) {
    geometry.indices = readIndices(reader, geometry.info);
    geometry.vertices = readVertices(reader, geometry.info);
}

std::vector<float> G3D::convertVertices(const std::vector<std::vector<float>>& vertexAttributes, const GeometryInfo& info) {
    std::vector<float> vertices(info.numberVertices * info.vertexSize);
    uint32_t vertexFloats = info.vertexSize / sizeof(float);
    for (uint32_t i = 0; i < info.numberVertices; ++i) {
        uint32_t offset = 0;
        uint32_t attributeIndex = 0;
        for (std::vector<uint32_t>::const_iterator it = info.attributeSemantics.begin(); it != info.attributeSemantics.end(); ++it) {
            uint32_t attributeFloats = floats(*it);
            for (uint32_t j = 0; j < attributeFloats; ++j)
                vertices[j + offset + (i * vertexFloats)] = vertexAttributes[attributeIndex][j + (i * attributeFloats)];
            offset += attributeFloats;
            ++attributeIndex;
        }
    }
    return vertices;
}

void G3D::readAoS(AbstractReader& reader, G3D::GeometryAoS& geometry) {
    if (reader.isOpen()) {
        geometry.info = readHeader(reader);
        if (geometry.info.numberVertices == 0)
            return;
        if (geometry.info.vertexType == AoS)
            readContent(reader, geometry);
        else if (geometry.info.vertexType == SoA) {
            geometry.info.vertexType = AoS;
            geometry.indices = readIndices(reader, geometry.info);
            auto vertexAttributes = readVertexAttributes(reader, geometry.info);
            geometry.vertices = convertVertices(vertexAttributes, geometry.info);
        }
    }
}

std::vector<std::vector<float>> G3D::readVertexAttributes(AbstractReader& reader, const GeometryInfo& info) {
    std::vector<std::vector<float>> vertexAttributes;
    for (uint32_t i = 0; i < info.attributeSemantics.size(); ++i) {
        uint32_t attributeFloats = floats(info.attributeSemantics.at(i));
        std::vector<float> attibutes(info.numberVertices * attributeFloats);
        vertexAttributes.push_back(attibutes);
        reader.read((char*)vertexAttributes.at(i).data(), info.numberVertices * attributeFloats * sizeof(float));
    }
    return vertexAttributes;
}

void G3D::readContent(AbstractReader& reader, GeometrySoA& geometry) {
    geometry.indices = readIndices(reader, geometry.info);
    geometry.vertexAttributes = readVertexAttributes(reader, geometry.info);
}

std::vector<std::vector<float>> G3D::convertVertices(const std::vector<float>& vertices, const GeometryInfo& info) {
    std::vector<std::vector<float>> vertexAttributes;
    for (auto semantic : info.attributeSemantics) {
        uint32_t attributeFloats = floats(semantic);
        vertexAttributes.push_back(std::vector<float>(info.numberVertices * attributeFloats));
    }

    uint32_t vertexFloats = info.vertexSize / sizeof(float);
    for (uint32_t i = 0; i < info.numberVertices; ++i) {
        uint32_t offset = 0;
        uint32_t attributeIndex = 0;
        for (std::vector<uint32_t>::const_iterator it = info.attributeSemantics.begin(); it != info.attributeSemantics.end(); ++it) {
            uint32_t attributeFloats = floats(*it);
            for (uint32_t j = 0; j < attributeFloats; ++j)
                vertexAttributes[attributeIndex][j + (i * attributeFloats)] = vertices[j + offset + (i * vertexFloats)];
            offset += attributeFloats;
            ++attributeIndex;
        }
    }
    return vertexAttributes;
}

void G3D::readSoA(AbstractReader& reader, G3D::GeometrySoA& geometry) {
    if (reader.isOpen()) {
        geometry.info = readHeader(reader);
        if (geometry.info.numberVertices == 0)
            return;
        if (geometry.info.vertexType == SoA)
            readContent(reader, geometry);
        else if (geometry.info.vertexType == AoS) {
            geometry.info.vertexType = SoA;
            geometry.indices = readIndices(reader, geometry.info);
            std::vector<float> vertices = readVertices(reader, geometry.info);
            geometry.vertexAttributes = convertVertices(vertices, geometry.info);
        }
    }
}

std::string G3D::printPrimitiveType(const Geometry& geometry) {
    return ((geometry.info.primitiveType == Point) ? "Point" : (geometry.info.primitiveType == Line)
                                                                   ? "Line"
                                                                   : (geometry.info.primitiveType == Triangle)
                                                                         ? "Triangle"
                                                                         : (geometry.info.primitiveType == TriangleAdj)
                                                                               ? "Triangle with adjacency"
                                                                               : "Unknown");
}

std::string G3D::printVertexType(const Geometry& geometry) {
    return ((geometry.info.vertexType == AoS) ? "Array of Structs" : (geometry.info.vertexType == SoA) ? "Struct of Arrays" : "Unknown");
}

std::string G3D::printAttributeSemantics(const Geometry& geometry) {
    std::string as;
    for (size_t i = 0; i < geometry.info.attributeSemantics.size(); ++i) {
        if (i)
            as.append(", ");
        auto const& it = geometry.info.attributeSemantics[i];
        as.append(((it) == Position)
                      ? "Position"
                      : ((it) == Normal) ? "Normal"
                                         : ((it) == Tangent)
                                               ? "Tangent"
                                               : ((it) == Color) ? "Color" : ((it) == Tex) ? "Tex" : ((it) == Float) ? "Float" : "Unknown");
        as.append(" (" + std::to_string(floats(it)) + "f)");
    }
    return as;
}

void G3D::print(const Geometry& geometry, std::ostream& output) {
    output << "Opaque: " << (geometry.info.isOpaque ? "yes" : "no") << std::endl;
    output << "Number primitives: " << geometry.info.numberPrimitives << std::endl;
    output << "Primitive type: " << printPrimitiveType(geometry) << std::endl;
    output << "Number indices: " << geometry.info.numberIndices << std::endl;
    output << "Index size: " << geometry.info.indexSize << std::endl;
    output << "Number vertices: " << geometry.info.numberVertices << std::endl;
    output << "Vertex size: " << geometry.info.vertexSize << std::endl;
    output << "Vertex type: " << printVertexType(geometry) << std::endl;
    output << "Vertex attribute semantics: " << printAttributeSemantics(geometry) << std::endl;
}

/*
 The MIT License

 Copyright (c) 2012 Interactive Visualization and Data Analysis Group
 Saarland University, DFKI, Intel VCI, MMCI

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
