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


#include <stdexcept>
#include <iostream>
#include <bserialize/BSerialize.hpp>
#include "BMFont.hpp"

namespace bs = spire;

namespace ren {

BMFont::BMFont()
{
}

BMFont::~BMFont()
{
}

void BMFont::clear()
{
}

int32_t BMFont::getASCIITextWidth(const char* text) const
{
  int32_t width = 0;
  size_t stringLen = std::strlen(text);
  for (size_t i = 0; i < stringLen; ++i)
  {
    uint32_t charID = static_cast<uint32_t>(text[i]);
    const CharInfo* info = getCharInfo(charID);
    if (info == nullptr)
    {
      std::cerr << "Unable to find character for: " << charID << std::endl;
      continue;
    }

    /// \todo On the first and last characters, take xoffset into account!
    ///       First character: If xoffset is negative, then expand width.
    ///       Last character: If xoffset is positive, then expand width.

    width += info->xadvance;

    // Get next character and adjust horizontal distance if the pair is in
    // the kerning pair map.
    if (i < stringLen - 1)
    {
      width += getKerningAdjustment(charID, static_cast<uint32_t>(text[i+1]));
    }
  }

  return width;
}

int32_t BMFont::getLineHeight() const
{
  return getCommonBlock().lineHeight;
}

int32_t BMFont::getKerningAdjustment(uint32_t firstID, uint32_t secondID) const
{
  auto kernRange = mKernMultiMap.equal_range(firstID);
  if (kernRange.first != mKernMultiMap.end())
  {
    // Test all of the kern pairs.
    for (auto it = kernRange.first; it != kernRange.second; ++it)
    {
      if (it->second.secondID == secondID)
      {
        // We have found our kerning pair, return its adjustment.
        return static_cast<int32_t>(it->second.amount);
      }
    }

    // We did not find our kerning pair.
    return 0;
  }
  else
  {
    return 0;
  }
}

const BMFont::CharInfo* BMFont::getCharInfo(uint32_t charID) const
{
  auto it = mCharMap.find(charID);
  if (it != mCharMap.end())
  {
    return &it->second;
  }
  else
  {
    return nullptr;
  }
}

void BMFont::loadFromBuffer(uint8_t* buffer, size_t bytesRead)
{
  bs::BSerialize s(reinterpret_cast<char*>(buffer), bytesRead);

  // Check the magic
  if (s.read<int8_t>() != 'B') {throw std::runtime_error("Invalid BMFont header."); return;}
  if (s.read<int8_t>() != 'M') {throw std::runtime_error("Invalid BMFont header."); return;}
  if (s.read<int8_t>() != 'F') {throw std::runtime_error("Invalid BMFont header."); return;}

  // Read BMF version. We are on 3.
  if (s.read<int8_t>() != 3)   {throw std::runtime_error("Invalid BMFont version!"); return;}

  // Loop through all of the blocks in the font file.
  while (s.getBytesLeft() >= 5)
  {
    int8_t blockType = s.read<int8_t>();
    int32_t blockSize = s.read<int32_t>();
    size_t headOffset = s.getOffset();

    switch (blockType)
    {
    case BLOCK_INFO:
      readInfoBlock(s);
      break;

    case BLOCK_COMMON:
      readCommonBlock(s);
      break;

    case BLOCK_PAGES:
      readPageBlock(s, static_cast<size_t>(blockSize));
      break;

    case BLOCK_CHARS:
      readCharBlock(s, static_cast<size_t>(blockSize));
      break;

    case BLOCK_KERN:
      readKernBlock(s, static_cast<size_t>(blockSize));
      break;
    }

    size_t advanceOffset = headOffset + static_cast<size_t>( blockSize );
    if (s.getOffset() > advanceOffset)
    {
      std::cerr << "BMFont: Block read exceeded indicated block size!" << std::endl;
    }
    s.setOffset(advanceOffset);
  }
}

void BMFont::readInfoBlock(bs::BSerialize& s)
{
  mInfoBlock.fontSize     = s.read<int16_t>();
  mInfoBlock.bitField     = s.read<uint8_t>();
  mInfoBlock.charSet      = s.read<uint8_t>();
  mInfoBlock.stretchH     = s.read<uint16_t>();
  mInfoBlock.aa           = s.read<uint8_t>();
  mInfoBlock.paddingUp    = s.read<uint8_t>();
  mInfoBlock.paddingRight = s.read<uint8_t>();
  mInfoBlock.paddingDown  = s.read<uint8_t>();
  mInfoBlock.paddingLeft  = s.read<uint8_t>();
  mInfoBlock.spacingHoriz = s.read<uint8_t>();
  mInfoBlock.spacingVert  = s.read<uint8_t>();
  mInfoBlock.outline      = s.read<uint8_t>();
  mInfoBlock.fontName     = s.readNullTermString();
}

void BMFont::readCommonBlock(bs::BSerialize& s)
{
  mCommonBlock.lineHeight   = s.read<uint16_t>();
  mCommonBlock.base         = s.read<uint16_t>();
  mCommonBlock.scaleW       = s.read<uint16_t>();
  mCommonBlock.scaleH       = s.read<uint16_t>();
  mCommonBlock.pages        = s.read<uint16_t>();
  mCommonBlock.bitField     = s.read<uint8_t>();
  mCommonBlock.alphaChannel = s.read<uint8_t>();
  mCommonBlock.redChannel   = s.read<uint8_t>();
  mCommonBlock.greenChannel = s.read<uint8_t>();
  mCommonBlock.blueChannel  = s.read<uint8_t>();
}

void BMFont::readPageBlock(bs::BSerialize& s, size_t blockSize)
{
  size_t initialOffset = s.getOffset();

  // Keep reading strings until we reach the end of the block.
  while (s.getOffset() - initialOffset < blockSize)
  {
    mPageBlock.pages.push_back(s.readNullTermString());
  }
}

void BMFont::readCharBlock(bs::BSerialize& s, size_t blockSize)
{
  // Calculate the number of characters contained in the block.
  size_t charInfoSize = sizeof(uint32_t)    // id
      + sizeof(uint16_t) + sizeof(uint16_t) // x, y
      + sizeof(uint16_t) + sizeof(uint16_t) // width, height
      + sizeof(uint16_t) + sizeof(uint16_t) // xoffset, yoffset
      + sizeof(uint16_t)                    // xadvance
      + sizeof(uint8_t) + sizeof(uint8_t);  // page, channel

  if (blockSize % charInfoSize != 0)
  {
    std::cerr << "BMFont: mismatch between block size and char info size" << std::endl;
  }

  size_t numChars = blockSize / charInfoSize;
  for (size_t i = 0; i < numChars; ++i)
  {
    CharInfo c;
    c.id       = s.read<uint32_t>();
    c.x        = s.read<uint16_t>();
    c.y        = s.read<uint16_t>();
    c.width    = s.read<uint16_t>();
    c.height   = s.read<uint16_t>();
    c.xoffset  = s.read<int16_t>();
    c.yoffset  = s.read<int16_t>();
    c.xadvance = s.read<int16_t>();
    c.page     = s.read<uint8_t>();
    c.channel  = s.read<uint8_t>();

    // Add character to map.
    mCharMap.insert(std::make_pair(c.id, c));
  }
}

void BMFont::readKernBlock(spire::BSerialize& s, size_t blockSize)
{
  // Size of first, second and amount.
  size_t kernInfoSize = sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint16_t);

  if (blockSize % kernInfoSize != 0)
  {
    std::cerr << "BMFont: mismatch between block size and kern info size." << std::endl;
  }

  size_t numKerns = blockSize / kernInfoSize;
  for (size_t i = 0; i < numKerns; ++i)
  {
    KernInfo k;
    k.firstID = s.read<uint32_t>();
    k.secondID = s.read<uint32_t>();
    k.amount = s.read<int16_t>();

    mKernMultiMap.insert(std::make_pair(k.firstID, k));
  }
}

} // namespace ren
