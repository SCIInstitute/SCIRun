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


#include <Modules/Visualization/TextBuilder.h>

#include <sstream>
#include <vector>

using namespace SCIRun;
using namespace Modules::Visualization;
using namespace Core::Geometry;
using namespace Graphics::Datatypes;

std::string TextBuilder::mFSRoot;
std::string TextBuilder::mFSSeparator;

TextBuilder::TextBuilder()
  : ftLib_(nullptr),
  ftFace_(nullptr),
  ftSize_(15),
  ftInit_(false),
  ftValid_(false),
  color_(1.0, 1.0, 1.0, 1.0)
{}

TextBuilder::~TextBuilder()
{
  //???
  //FT_Done_FreeType(ftLib_);
  //FT_Done_Face(ftFace_);
}

void TextBuilder::setColor(float r, float g, float b, float a)
{
  color_ = glm::vec4(r,g,b,a);
}


void TextBuilder::initFreeType(const std::string &libName, size_t size)
{
  FT_Error err;
  if (!ftInit_)
  {
    err = FT_Init_FreeType(&ftLib_);
    if (!err)
      ftInit_ = true;
  }

  if (!ftInit_) return;

  libName_ = libName;
  auto fontPath = mFSRoot + mFSSeparator + "Fonts" + mFSSeparator + libName;

  err = FT_New_Face(ftLib_, fontPath.c_str(), 0, &ftFace_);
  if (!err)
    ftValid_ = true;

  if (ftValid_)
  {
    FT_Select_Charmap(ftFace_, FT_ENCODING_UNICODE);
    FT_Set_Pixel_Sizes(ftFace_, 0, size);
  }
}

void TextBuilder::loadNewFace(const std::string &libName, size_t size)
{
  FT_Error err;
  if (!ftInit_)
  {
    err = FT_Init_FreeType(&ftLib_);
    if (!err)
      ftInit_ = true;
  }

  if (!ftInit_) return;

  if (ftValid_)
  {
    FT_Done_Face(ftFace_);
    ftValid_ = false;
  }

  libName_ = libName;
  auto fontPath = mFSRoot + mFSSeparator + "Fonts" + mFSSeparator + libName;

  err = FT_New_Face(ftLib_, fontPath.c_str(), 0, &ftFace_);
  if (!err)
    ftValid_ = true;

  if (ftValid_)
  {
    FT_Select_Charmap(ftFace_, FT_ENCODING_UNICODE);
    FT_Set_Pixel_Sizes(ftFace_, 0, size);
  }
}

void TextBuilder::setFaceSize(size_t size)
{
  if (!ftValid_)
    return;

  ftSize_ = size;
  FT_Set_Pixel_Sizes(ftFace_, 0, size);
}

void TextBuilder::setFSStrings(const std::string &root, const std::string &separator)
{
  mFSRoot = root;
  mFSSeparator = separator;
}

std::string TextBuilder::getUniqueFontString(char p, double x,
  double y, double z, double w, double h) const
{
  std::stringstream ss;
  ss << libName_ << p << x << y << z << w << h;
  return ss.str();
}

void TextBuilder::printString(const std::string& oneline,
  const Vector &startNrmSpc, const Vector &shiftPxlSpc,
  const std::string& id, GeometryObjectSpire& geom) const
{
  if (!ftValid_)
    return;

  std::vector<Vector> points;
  std::vector<uint32_t> indices;
  std::vector<Vector> txt_coords;
  auto pos = shiftPxlSpc;

  for (const auto& p : oneline)
  {
    points.clear();
    indices.clear();
    txt_coords.clear();

    if (FT_Load_Char(ftFace_, p, FT_LOAD_RENDER))
      continue;
    auto g = ftFace_->glyph;

    double x = pos.x() + g->bitmap_left;
    double y = -pos.y() - g->bitmap_top;
    double z = pos.z();
    double w = g->bitmap.width;
    double h = g->bitmap.rows;

    //triangle 1
    points.push_back(Vector(x, -y, z));
    points.push_back(Vector(x + w, -y, z));
    points.push_back(Vector(x, -y - h, z));
    //triangle 2
    points.push_back(Vector(x, -y - h, z));
    points.push_back(Vector(x + w, -y, z));
    points.push_back(Vector(x + w, -y - h, z));
    //triangle 1
    txt_coords.push_back(Vector(0.0, 0.0, 0.0));
    txt_coords.push_back(Vector(1.0, 0.0, 0.0));
    txt_coords.push_back(Vector(0.0, 1.0, 0.0));
    //triangle 2
    txt_coords.push_back(Vector(0.0, 1.0, 0.0));
    txt_coords.push_back(Vector(1.0, 0.0, 0.0));
    txt_coords.push_back(Vector(1.0, 1.0, 0.0));
    //triangle 1
    indices.push_back(0);
    indices.push_back(1);
    indices.push_back(2);
    //triangle 2
    indices.push_back(3);
    indices.push_back(4);
    indices.push_back(5);

    pos += Vector(g->advance.x >> 6, g->advance.y >> 6, 0.0);

    int32_t numVBOElements = static_cast<uint32_t>(points.size());

    // IBO/VBOs and sizes
    uint32_t iboSize = sizeof(uint32_t) * static_cast<uint32_t>(indices.size());
    uint32_t vboSize = sizeof(float) * 5 * static_cast<uint32_t>(points.size());

    std::shared_ptr<spire::VarBuffer> iboBufferSPtr2(
      new spire::VarBuffer(vboSize));
    std::shared_ptr<spire::VarBuffer> vboBufferSPtr2(
      new spire::VarBuffer(iboSize));

    spire::VarBuffer* iboBuffer2 = iboBufferSPtr2.get();
    spire::VarBuffer* vboBuffer2 = vboBufferSPtr2.get();

    for (auto a : indices) iboBuffer2->write(a);
    for (size_t i = 0; i < points.size(); i++) {
      vboBuffer2->write(static_cast<float>(points[i].x()));
      vboBuffer2->write(static_cast<float>(points[i].y()));
      vboBuffer2->write(static_cast<float>(points[i].z()));
      vboBuffer2->write(static_cast<float>(txt_coords[i].x()));
      vboBuffer2->write(static_cast<float>(txt_coords[i].y()));
    }

    //add the actual points and colors
    auto uniqueFontStr = getUniqueFontString(p, x, y, z, w, h);
    auto uniqueNodeID = id + "colorMapLegendTextFont" + uniqueFontStr;
    auto vboName = uniqueNodeID + "VBO";
    auto iboName = uniqueNodeID + "IBO";
    auto passName = uniqueNodeID + "Pass";

    // Construct VBO.
    std::string shader = "Shaders/TextBuilder";
    std::vector<SpireVBO::AttributeData> attribs;
    attribs.push_back(SpireVBO::AttributeData("aPos", 3 * sizeof(float)));
    attribs.push_back(SpireVBO::AttributeData("aTexCoord", 2 * sizeof(float)));
    std::vector<SpireSubPass::Uniform> uniforms;
    uniforms.push_back(SpireSubPass::Uniform("uTrans", glm::vec4(startNrmSpc.x(), startNrmSpc.y(), 0.0, 0.0)));
    uniforms.push_back(SpireSubPass::Uniform("uColor", color_));
    auto geomVBO = SpireVBO(vboName, attribs, vboBufferSPtr2,
      numVBOElements, BBox(Point{}, Point{}), true);

    geom.vbos().push_back(geomVBO);

    // Construct IBO.

    SpireIBO geomIBO(iboName, SpireIBO::PRIMITIVE::TRIANGLES, sizeof(uint32_t), iboBufferSPtr2);
    geom.ibos().push_back(geomIBO);
    RenderState renState;
    renState.set(RenderState::USE_COLORMAP, false);
    renState.set(RenderState::USE_TRANSPARENCY, false);
    renState.set(RenderState::IS_TEXT, true);
    char c[2] = { p, 0 };
    SpireText text(c, ftFace_);

    SpireSubPass pass2(passName, vboName, iboName, shader,
                       ColorScheme::COLOR_MAP, renState, RenderType::RENDER_VBO_IBO, geomVBO, geomIBO, text);

    // Add all uniforms generated above to the pass.
    for (const auto& uniform : uniforms) { pass2.addUniform(uniform); }

    geom.passes().push_back(pass2);
  }
}

std::tuple<double, double> TextBuilder::getStringDims(const std::string& oneline) const
{
  if (!ftValid_)
    return {};

  auto len = 0.0;
  auto rows = 0;
  for (const auto& p : oneline)
  {
    if (FT_Load_Char(ftFace_, p, FT_LOAD_RENDER))
      continue;
    auto g = ftFace_->glyph;
    len += g->bitmap.width;
    rows = g->bitmap.rows;
  }
  return std::make_tuple(len, rows);
}

bool TextBuilder::initialize(size_t textSize)
{
  return initialize(textSize, "FreeSans.ttf");
}

bool TextBuilder::initialize(size_t textSize, const std::string& fontName)
{
  if (!isInit())
    initFreeType(fontName, textSize);
  else if (!isValid())
    loadNewFace(fontName, textSize);

  return isReady();
}
