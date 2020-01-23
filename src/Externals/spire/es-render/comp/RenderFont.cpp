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


#include "RenderFont.hpp"
#include <glm/gtc/type_ptr.hpp>           // glm::value_ptr
#include <var-buffer/VarBuffer.hpp>

namespace shaders = spire;

namespace ren {

bool RenderFont::isSetUp() const
{
  return (vbo != 0) && (ibo != 0);
}

void RenderFont::setupAttributesAndUniforms(GLuint shaderID)
{
  std::vector<spire::ShaderAttribute> shaderAttribs =
      spire::getProgramAttributes(shaderID);
  spire::sortAttributesAlphabetically(shaderAttribs);

  spire::ShaderAttribute attribs[MaxFontAttributes];

  /// \todo Have an easier way of initializing these. Place in tuple and
  ///       have a function that combines them altogether.
  attribs[0].nameInCode = "aPos2D";
  attribs[0].sizeBytes = 2 * sizeof(float);
  attribs[0].normalize = false;

  attribs[1].nameInCode = "aTextToCenter";
  attribs[1].sizeBytes = 2 * sizeof(float);
  attribs[1].normalize = false;

  attribs[2].nameInCode = "aUV0";
  attribs[2].sizeBytes = 2 * sizeof(float);
  attribs[2].normalize = false;

  attribs[3].nameInCode = "aLetterIndex";
  attribs[3].sizeBytes = sizeof(float);
  attribs[3].normalize = false;

  if (MaxFontAttributes < shaderAttribs.size())
  {
    std::cerr << "RenderFont: Unable to satisfy shader." << std::endl;
  }

  // Use the shader and VBO data to construct an applied state.
  std::tuple<size_t, size_t> sizes = shaders::buildPreappliedAttrib(
      &attribs[0], MaxFontAttributes,
      &shaderAttribs[0], shaderAttribs.size(),
      appliedAttribs, MaxFontAttributes
      );

  attribSize = static_cast<int>(std::get<0>(sizes));
  stride = std::get<1>(sizes);

  std::vector<shaders::ShaderUniform> shaderUniforms =
      shaders::getProgramUniforms(shaderID);
  for (const shaders::ShaderUniform& uniform : shaderUniforms)
  {
    if (uniform.nameInCode == "uTextNumChars")
    {
      numTextCharsUniformLoc = uniform.uniformLoc;
    }
  }
}

void RenderFont::applyUniforms() const
{
  if (numTextCharsUniformLoc != -1)
  {
    float numChars = static_cast<float>(std::strlen(text));
    GL(glUniform1f(numTextCharsUniformLoc, numChars));
  }
}

void RenderFont::constructFontGeometry(const StaticFontMan& fontMan, uint64_t fontID)
{
  // We will use UTF8, but we only use strlen for now. We will need more
  // advanced functionality if we want to supporta wider range of characters.

  // Obtain BMFont info using fontID and fontMan
  const BMFont& fontInfo = fontMan.instance_->getBMFontInfo(fontID);

  /// \todo We will have to move a number of functions into BMFont itself.
  ///       functions such as detecting the length of a line, or finding
  ///       the UV coordinates for a particular glyph.

  // Construct text to be rendered based on the information contained in this
  // component.
  size_t stringLen = std::strlen(text);

  // Our mesh attributes:
  // aPos2D
  // aUV0
  // aLetterIndex

  // Assuming RENDER_LINE for now...

  // Variable buffers that will contain our VBO and IBO.
  spire::VarBuffer vboBuff;
  spire::VarBuffer iboBuff;

  // We create everything in pixel coordinates, then transfer coordinate
  // systems.

  // The following rendering code is based on:
  // http://www.angelcode.com/products/bmfont/doc/render_text.html

  // Corresponds to origin from which base is calculated. This is the variable
  // that would be advanced if we were rendering multiline text.
  int32_t topLineOrig = 0;
  int32_t cursorPos   = 0;
  //int32_t base        = fontInfo.getCommonBlock().base;

  float scaleX = 1.0f / static_cast<float>(fontInfo.getCommonBlock().scaleW);
  float scaleY = 1.0f / static_cast<float>(fontInfo.getCommonBlock().scaleH);

  for (size_t i = 0; i < stringLen; ++i)
  {
    uint32_t charID = static_cast<uint32_t>(text[i]);
    const BMFont::CharInfo* info = fontInfo.getCharInfo(charID);

    // Y offset from topLine
    int32_t top     = topLineOrig - static_cast<int32_t>(info->yoffset);
    int32_t left    = cursorPos + static_cast<int32_t>(info->xoffset);
    int32_t bottom  = top - static_cast<int32_t>(info->height);
    int32_t right   = left + static_cast<int32_t>(info->width);

    float uvTop     = static_cast<float>(info->y) * scaleY;
    float uvLeft    = static_cast<float>(info->x) * scaleX;
    float uvBot     = uvTop + static_cast<float>(info->height) * scaleY;
    float uvRight   = uvLeft + static_cast<float>(info->width) * scaleX;

    // Construct VBO
    float sTop = static_cast<float>(top) * scaleY;
    float sLeft = static_cast<float>(left) * scaleX;
    float sBot = static_cast<float>(bottom) * scaleY;
    float sRight = static_cast<float>(right) * scaleX;

    glm::vec2 toCenter;
    glm::vec2 curPoint;

    // Correct center value. But it is more interesting to squish the bottom.
    glm::vec2 center(sLeft + (sRight - sLeft)/2.0f, sBot + (sTop - sBot)/2.0f);

    // Top left
    curPoint = glm::vec2(sLeft, sTop);
    vboBuff.write<float>(curPoint.x);
    vboBuff.write<float>(curPoint.y);
    toCenter = curPoint - center;
    toCenter = glm::normalize(toCenter);
    vboBuff.write<float>(toCenter.x);
    vboBuff.write<float>(toCenter.y);
    vboBuff.write<float>(uvLeft);
    vboBuff.write<float>(uvTop);
    vboBuff.write<float>(static_cast<float>(i));

    // Top right
    curPoint = glm::vec2(sRight, sTop);
    vboBuff.write<float>(curPoint.x);
    vboBuff.write<float>(curPoint.y);
    toCenter = curPoint - center;
    toCenter = glm::normalize(toCenter);
    vboBuff.write<float>(toCenter.x);
    vboBuff.write<float>(toCenter.y);
    vboBuff.write<float>(uvRight);
    vboBuff.write<float>(uvTop);
    vboBuff.write<float>(static_cast<float>(i));

    // Bottom left
    curPoint = glm::vec2(sLeft, sBot);
    vboBuff.write<float>(curPoint.x);
    vboBuff.write<float>(curPoint.y);
    toCenter = curPoint - center;
    toCenter = glm::normalize(toCenter);
    vboBuff.write<float>(toCenter.x);
    vboBuff.write<float>(toCenter.y);
    vboBuff.write<float>(uvLeft);
    vboBuff.write<float>(uvBot);
    vboBuff.write<float>(static_cast<float>(i));

    // Bottom right
    curPoint = glm::vec2(sRight, sBot);
    vboBuff.write<float>(curPoint.x);
    vboBuff.write<float>(curPoint.y);
    toCenter = curPoint - center;
    toCenter = glm::normalize(toCenter);
    vboBuff.write<float>(toCenter.x);
    vboBuff.write<float>(toCenter.y);
    vboBuff.write<float>(uvRight);
    vboBuff.write<float>(uvBot);
    vboBuff.write<float>(static_cast<float>(i));

    // Construct IBO
    uint16_t idx = static_cast<uint16_t>(i) * 4;
    uint16_t TL = idx;
    uint16_t TR = idx + 1;
    uint16_t BL = idx + 2;
    uint16_t BR = idx + 3;

    iboBuff.write<uint16_t>(TL);
    iboBuff.write<uint16_t>(TR);
    iboBuff.write<uint16_t>(BL);

    iboBuff.write<uint16_t>(BL);
    iboBuff.write<uint16_t>(TR);
    iboBuff.write<uint16_t>(BR);

    // Advance cursor position horizontally.
    cursorPos += info->xadvance;

    // Get next character and adjust horizontal distance if the pair is in
    // the kerning pair map.
    if (i < stringLen - 1)
    {
      cursorPos += fontInfo.getKerningAdjustment(charID, static_cast<uint32_t>(text[i+1]));
    }
  }

  // Generate vbo
  GL(glGenBuffers(1, &vbo));
  GL(glBindBuffer(GL_ARRAY_BUFFER, vbo));
  GL(glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vboBuff.getBufferSize()),
                  vboBuff.getBuffer(), GL_STATIC_DRAW));

  // Generate ibo
  GL(glGenBuffers(1, &ibo));
  GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo));
  GL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(iboBuff.getBufferSize()),
                  iboBuff.getBuffer(), GL_STATIC_DRAW));

  // I don't think we need to store this data.
  primMode = GL_TRIANGLES;
  primType = GL_UNSIGNED_SHORT;
  size_t numIndices = static_cast<size_t>(iboBuff.getBufferSize()) / sizeof(uint16_t);
  numPrims = static_cast<GLsizei>(numIndices);

  // These are the attributes we have to work with.
  //vboMan.addVBOAttributes(
  //    vbo.glid,
  //    { std::make_tuple("aPos2D", 2 * sizeof(float), false),
  //      std::make_tuple("aUV0", 2 * sizeof(float), false),
  //      std::make_tuple("aLetterIndex", sizeof(float), false)});
}

} // namespace ren
