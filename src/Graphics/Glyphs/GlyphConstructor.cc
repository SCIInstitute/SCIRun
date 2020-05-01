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

#include<Graphics/Glyphs/GlyphConstructor.h>

using namespace SCIRun;
using namespace Graphics;
using namespace Core::Geometry;
using namespace Core::Datatypes;
using namespace Graphics::Datatypes;

GlyphConstructor::GlyphConstructor()
{}

void GlyphConstructor::buildObject(GeometryObjectSpire& geom, const std::string& uniqueNodeID,
  const bool isTransparent, const double transparencyValue, const ColorScheme& colorScheme,
  RenderState state, const SpireIBO::PRIMITIVE& primIn, const BBox& bbox, const bool isClippable,
  const Core::Datatypes::ColorMapHandle colorMap)
{
  bool useColor = colorScheme == ColorScheme::COLOR_IN_SITU || colorScheme == ColorScheme::COLOR_MAP;
  bool useNormals = normals_.size() == points_.size();
  int numAttributes = 3;

  RenderType renderType = RenderType::RENDER_VBO_IBO;
  ColorRGB dft = state.defaultColor;

  std::string shader = (useNormals ? "Shaders/Phong" : "Shaders/Flat");
  std::vector<SpireVBO::AttributeData> attribs;
  std::vector<SpireSubPass::Uniform> uniforms;

  attribs.push_back(SpireVBO::AttributeData("aPos", 3 * sizeof(float)));
  uniforms.push_back(SpireSubPass::Uniform("uUseClippingPlanes", isClippable));
  uniforms.push_back(SpireSubPass::Uniform("uUseFog", true));

  if (useNormals)
  {
    numAttributes += 3;
    attribs.push_back(SpireVBO::AttributeData("aNormal", 3 * sizeof(float)));
    uniforms.push_back(SpireSubPass::Uniform("uAmbientColor", glm::vec4(0.1f, 0.1f, 0.1f, 1.0f)));
    uniforms.push_back(SpireSubPass::Uniform("uSpecularColor", glm::vec4(0.1f, 0.1f, 0.1f, 0.1f)));
    uniforms.push_back(SpireSubPass::Uniform("uSpecularPower", 32.0f));
  }

  SpireText text;
  SpireTexture2D texture;
  if (useColor)
  {
    if(colorMap)
    {
      numAttributes += 2;
      shader += "_ColorMap";
      attribs.push_back(SpireVBO::AttributeData("aTexCoords", 2 * sizeof(float)));

      const static int colorMapResolution = 256;
      for(int i = 0; i < colorMapResolution; ++i)
      {
        ColorRGB color = colorMap->valueToColor(static_cast<float>(i)/colorMapResolution * 2.0f - 1.0f);
        texture.bitmap.push_back(color.r()*255.99f);
        texture.bitmap.push_back(color.g()*255.99f);
        texture.bitmap.push_back(color.b()*255.99f);
        texture.bitmap.push_back(color.a()*255.99f);
      }

      texture.name = "ColorMap";
      texture.height = 1;
      texture.width = colorMapResolution;
    }
    else
    {
      numAttributes += 4;
      shader += "_Color";
      attribs.push_back(SpireVBO::AttributeData("aColor", 4 * sizeof(float)));
    }
  }
  else
  {
    uniforms.push_back(SpireSubPass::Uniform("uDiffuseColor",
      glm::vec4(dft.r(), dft.g(), dft.b(), static_cast<float>(transparencyValue))));
  }

  if (isTransparent) uniforms.push_back(SpireSubPass::Uniform("uTransparency", static_cast<float>(transparencyValue)));

  size_t pointsLeft = points_.size();
  size_t startOfPass = 0;
  int passNumber = 0;
  while(pointsLeft > 0)
  {
    std::string passID = uniqueNodeID + "_" + std::to_string(passNumber++);
    std::string vboName = passID + "VBO";
    std::string iboName = passID + "IBO";
    std::string passName = passID + "Pass";

    const static size_t maxPointsPerPass = 3 << 24; //must be a number divisible by 2, 3 and, 4
    uint32_t pointsInThisPass = std::min(pointsLeft, maxPointsPerPass);
    size_t endOfPass = startOfPass + pointsInThisPass;
    pointsLeft -= pointsInThisPass;

    size_t vboSize = static_cast<size_t>(pointsInThisPass) * numAttributes * sizeof(float);
    size_t iboSize = static_cast<size_t>(pointsInThisPass) * sizeof(uint32_t);
    std::shared_ptr<spire::VarBuffer> iboBufferSPtr(new spire::VarBuffer(iboSize));
    std::shared_ptr<spire::VarBuffer> vboBufferSPtr(new spire::VarBuffer(vboSize));
    auto iboBuffer = iboBufferSPtr.get();
    auto vboBuffer = vboBufferSPtr.get();

    for (auto a : indices_) if(a >= startOfPass && a < endOfPass)
      iboBuffer->write(static_cast<uint32_t>(a - startOfPass));

    BBox newBBox;
    for (size_t i = startOfPass; i < endOfPass; ++i)
    {
      Vector point = points_.at(i);
      newBBox.extend(Point(point.x(), point.y(), point.z()));
      vboBuffer->write(static_cast<float>(point.x()));
      vboBuffer->write(static_cast<float>(point.y()));
      vboBuffer->write(static_cast<float>(point.z()));

      if (useNormals)
      {
        Vector normal = normals_.at(i);
        vboBuffer->write(static_cast<float>(normal.x()));
        vboBuffer->write(static_cast<float>(normal.y()));
        vboBuffer->write(static_cast<float>(normal.z()));
      }

      if (useColor)
      {
        ColorRGB color = colors_.at(i);
        if(!colorMap)
        {
          vboBuffer->write(static_cast<float>(color.r()));
          vboBuffer->write(static_cast<float>(color.g()));
          vboBuffer->write(static_cast<float>(color.b()));
          vboBuffer->write(static_cast<float>(color.a()));
        }
        else
        {
          vboBuffer->write(static_cast<float>(color.r()));
          vboBuffer->write(static_cast<float>(color.r()));
        }
      }
    }
    if(!bbox.valid()) newBBox.reset();

    startOfPass = endOfPass;

    SpireVBO geomVBO(vboName, attribs, vboBufferSPtr, numVBOElements_, newBBox, true);
    SpireIBO geomIBO(iboName, primIn, sizeof(uint32_t), iboBufferSPtr);

    state.set(RenderState::IS_ON, true);
    state.set(RenderState::HAS_DATA, true);
    SpireSubPass pass(passName, vboName, iboName, shader, colorScheme, state, renderType, geomVBO, geomIBO, text, texture);

    for (const auto& uniform : uniforms) pass.addUniform(uniform);

    geom.vbos().push_back(geomVBO);
    geom.ibos().push_back(geomIBO);
    geom.passes().push_back(pass);
  }
}

uint32_t GlyphConstructor::setOffset()
{
  offset_ = numVBOElements_;
  return offset_;
}

bool GlyphConstructor::normalsValid() const
{
  return normals_.size() == points_.size();
}

void GlyphConstructor::addVertex(const Vector& point, const Vector& normal, const ColorRGB& color)
{
  points_.push_back(point);
  normals_.push_back(normal);
  colors_.push_back(color);
  ++numVBOElements_;
}

void GlyphConstructor::addVertex(const Vector& point, const ColorRGB& color)
{
  points_.push_back(point);
  colors_.push_back(color);
  ++numVBOElements_;
}

void GlyphConstructor::addLine(const Vector& point1, const Vector& point2, const ColorRGB& color1,
                               const ColorRGB& color2)
{
  points_.push_back(point1);
  colors_.push_back(color1);
  addLineIndex();

  points_.push_back(point2);
  colors_.push_back(color2);
  addLineIndex();

  ++numVBOElements_;
}

void GlyphConstructor::addPoint(const Vector& point, const ColorRGB& color)
{
  addVertex(point, color);
  addLineIndex();
}

void GlyphConstructor::addLineIndex()
{
  indices_.push_back(lineIndex_);
  ++lineIndex_;
}

void GlyphConstructor::addIndex(size_t i)
{
  indices_.push_back(i);
}

void GlyphConstructor::addIndexToOffset(size_t i)
{
  addIndex(i + offset_);
}

void GlyphConstructor::addIndices(size_t i1, size_t i2, size_t i3)
{
  addIndex(i1);
  addIndex(i2);
  addIndex(i3);
}

void GlyphConstructor::addIndicesToOffset(size_t i1, size_t i2, size_t i3)
{
  addIndexToOffset(i1);
  addIndexToOffset(i2);
  addIndexToOffset(i3);
}

size_t GlyphConstructor::getCurrentIndex() const
{
  return numVBOElements_;
}

void GlyphConstructor::popIndicesNTimes(int n)
{
  for (int i = 0; i < n; ++i)
    indices_.pop_back();
}
