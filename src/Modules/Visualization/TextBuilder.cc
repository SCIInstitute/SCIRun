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

#include <algorithm>
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
  atlas_.reset();
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
  atlas_.reset();
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
  atlas_.reset();
  FT_Set_Pixel_Sizes(ftFace_, 0, size);
}

void TextBuilder::setFSStrings(const std::string &root, const std::string &separator)
{
  mFSRoot = root;
  mFSSeparator = separator;
}

const TextBuilder::Atlas& TextBuilder::atlas() const
{
  if (atlas_)
    return *atlas_;

  // Printable ASCII on a 16-column grid. The 1-pixel gutter keeps linear
  // filtering from bleeding between neighbours.
  static const int first = 32, last = 126, columns = 16, pad = 1;
  auto atlas = std::make_shared<Atlas>();

  int cellW = 0, cellH = 0;
  for (int c = first; c <= last; ++c)
  {
    if (FT_Load_Char(ftFace_, c, FT_LOAD_RENDER))
      continue;
    cellW = std::max(cellW, static_cast<int>(ftFace_->glyph->bitmap.width));
    cellH = std::max(cellH, static_cast<int>(ftFace_->glyph->bitmap.rows));
  }
  cellW += 2 * pad;
  cellH += 2 * pad;
  const int rowCount = (last - first) / columns + 1;
  const int width = columns * cellW, height = rowCount * cellH;

  auto& tex = atlas->texture;
  tex.name = "atlas" + libName_ + std::to_string(ftSize_);
  tex.width = width;
  tex.height = height;
  tex.bitmap.assign(static_cast<size_t>(width) * height, 0);

  for (int c = first; c <= last; ++c)
  {
    if (FT_Load_Char(ftFace_, c, FT_LOAD_RENDER))
      continue;
    const auto g = ftFace_->glyph;
    const auto& bm = g->bitmap;
    const int x = ((c - first) % columns) * cellW + pad;
    const int y = ((c - first) / columns) * cellH + pad;
    for (unsigned int row = 0; row < bm.rows; ++row)
      std::copy_n(bm.buffer + row * bm.pitch, bm.width, tex.bitmap.begin() + (y + row) * width + x);

    Glyph glyph;
    glyph.u0 = static_cast<float>(x) / width;
    glyph.v0 = static_cast<float>(y) / height;
    glyph.u1 = static_cast<float>(x + bm.width) / width;
    glyph.v1 = static_cast<float>(y + bm.rows) / height;
    glyph.width = bm.width;
    glyph.rows = bm.rows;
    glyph.left = g->bitmap_left;
    glyph.top = g->bitmap_top;
    glyph.advance = g->advance.x >> 6;
    atlas->glyphs[static_cast<char>(c)] = glyph;
  }

  atlas_ = atlas;
  return *atlas_;
}

std::vector<TextBuilder::Quad> TextBuilder::layout(const std::string& oneline, const Vector& shiftPxlSpc) const
{
  const auto& glyphs = atlas().glyphs;
  std::vector<Quad> quads;
  quads.reserve(oneline.size());
  auto penX = static_cast<float>(shiftPxlSpc.x());
  const auto penY = static_cast<float>(shiftPxlSpc.y());
  for (const auto c : oneline)
  {
    const auto it = glyphs.find(c);
    if (it == glyphs.end())
      continue;
    const auto& g = it->second;
    if (g.width > 0 && g.rows > 0)
    {
      const float x = penX + g.left, top = penY + g.top;
      quads.push_back({ x, top, x + g.width, top - g.rows, g.u0, g.v0, g.u1, g.v1 });
    }
    penX += g.advance;
  }
  return quads;
}

namespace
{
  void writeQuadIndices(spire::VarBuffer& ibo, uint32_t firstVertex)
  {
    for (const uint32_t i : { 0, 1, 2, 2, 1, 3 })
      ibo.write(firstVertex + i);
  }

  void addTextPass(GeometryObjectSpire& geom, const std::string& name, const std::string& shader,
    const std::vector<SpireVBO::AttributeData>& attribs,
    std::shared_ptr<spire::VarBuffer> vboBuffer, std::shared_ptr<spire::VarBuffer> iboBuffer,
    size_t numVertices, const BBox& bbox,
    const std::vector<SpireSubPass::Uniform>& uniforms, const SpireText& texture)
  {
    const auto vboName = name + "VBO", iboName = name + "IBO";
    SpireVBO geomVBO(vboName, attribs, vboBuffer, static_cast<int64_t>(numVertices), bbox, true);
    geom.vbos().push_back(geomVBO);
    SpireIBO geomIBO(iboName, SpireIBO::PRIMITIVE::TRIANGLES, sizeof(uint32_t), iboBuffer);
    geom.ibos().push_back(geomIBO);

    RenderState renState;
    renState.set(RenderState::ActionFlags::USE_COLORMAP, false);
    renState.set(RenderState::ActionFlags::USE_TRANSPARENCY, false);
    renState.set(RenderState::ActionFlags::IS_TEXT, true);

    SpireSubPass pass(name + "Pass", vboName, iboName, shader,
      ColorScheme::COLOR_MAP, renState, RenderType::RENDER_VBO_IBO, geomVBO, geomIBO, texture);
    for (const auto& uniform : uniforms) { pass.addUniform(uniform); }
    geom.passes().push_back(pass);
  }
}

void TextBuilder::printString(const std::string& oneline,
  const Vector &startNrmSpc, const Vector &shiftPxlSpc,
  const std::string& id, GeometryObjectSpire& geom) const
{
  if (!ftValid_)
    return;

  const auto quads = layout(oneline, shiftPxlSpc);
  if (quads.empty())
    return;

  const auto z = static_cast<float>(shiftPxlSpc.z());
  auto vbo = std::make_shared<spire::VarBuffer>(quads.size() * 4 * 5 * sizeof(float));
  auto ibo = std::make_shared<spire::VarBuffer>(quads.size() * 6 * sizeof(uint32_t));
  uint32_t vertex = 0;
  for (const auto& q : quads)
  {
    for (const float v : { q.x0, q.y0, z, q.u0, q.v0,   q.x1, q.y0, z, q.u1, q.v0,
                           q.x0, q.y1, z, q.u0, q.v1,   q.x1, q.y1, z, q.u1, q.v1 })
      vbo->write(v);
    writeQuadIndices(*ibo, vertex);
    vertex += 4;
  }

  // Callers reuse one id for several strings, so the name carries the rest.
  std::stringstream name;
  name << id << "colorMapLegendTextFont" << libName_ << ftSize_ << oneline
    << startNrmSpc.x() << startNrmSpc.y() << shiftPxlSpc.x() << shiftPxlSpc.y() << shiftPxlSpc.z();

  addTextPass(geom, name.str(), "Shaders/TextBuilder",
    { { "aPos", 3 * sizeof(float) }, { "aTexCoord", 2 * sizeof(float) } },
    vbo, ibo, vertex, BBox(Point{}, Point{}),
    { { "uTrans", glm::vec4(startNrmSpc.x(), startNrmSpc.y(), 0.0, 0.0) }, { "uColor", color_ } },
    atlas().texture);
}

void TextBuilder::printStringsInWorld(const std::vector<WorldLabel>& labels,
  const Vector &shiftPxlSpc, bool alwaysVisible,
  const std::string& id, GeometryObjectSpire& geom) const
{
  if (!ftValid_ || labels.empty())
    return;

  // Anchor, normal and color are per label, so they ride along as attributes.
  static const size_t floatsPerVertex = 2 + 2 + 3 + 4 + 4;
  size_t numChars = 0;
  for (const auto& label : labels)
    numChars += label.text.size();
  auto vbo = std::make_shared<spire::VarBuffer>(numChars * 4 * floatsPerVertex * sizeof(float));
  auto ibo = std::make_shared<spire::VarBuffer>(numChars * 6 * sizeof(uint32_t));

  BBox bbox;
  uint32_t vertex = 0;
  for (const auto& label : labels)
  {
    const auto& p = label.position;
    bbox.extend(p);
    // aNormal.w flags whether there is a normal to cull against.
    const auto n = label.normal
      ? glm::vec4(label.normal->x(), label.normal->y(), label.normal->z(), 1.0)
      : glm::vec4(0.0);
    for (const auto& q : layout(label.text, shiftPxlSpc))
    {
      const float corners[4][4] = { { q.x0, q.y0, q.u0, q.v0 }, { q.x1, q.y0, q.u1, q.v0 },
                                    { q.x0, q.y1, q.u0, q.v1 }, { q.x1, q.y1, q.u1, q.v1 } };
      for (const auto& corner : corners)
      {
        for (const float v : corner)
          vbo->write(v);
        for (const double v : { p.x(), p.y(), p.z() })
          vbo->write(static_cast<float>(v));
        for (const float v : { n.x, n.y, n.z, n.w, label.color.r, label.color.g, label.color.b, label.color.a })
          vbo->write(v);
      }
      writeQuadIndices(*ibo, vertex);
      vertex += 4;
    }
  }
  if (vertex == 0)
    return;

  addTextPass(geom, id + "worldText" + libName_ + std::to_string(ftSize_), "Shaders/TextBuilderWorld",
    { { "aPos", 2 * sizeof(float) }, { "aTexCoord", 2 * sizeof(float) }, { "aAnchor", 3 * sizeof(float) },
      { "aNormal", 4 * sizeof(float) }, { "aColor", 4 * sizeof(float) } },
    vbo, ibo, vertex, bbox,
    { { "uAlwaysVisible", alwaysVisible ? 1.0f : 0.0f } },
    atlas().texture);
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
