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


#ifndef SPIRE_RENDER_BMFONT_HPP
#define SPIRE_RENDER_BMFONT_HPP

#include <es-log/trace-log.h>
#include <cstdint>
#include <cstddef>
#include <vector>
#include <string>
#include <map>
#include <spire/scishare.h>

// Forward declare a class that we need for parsing BMFont files.
namespace spire {
class BSerialize;
}

namespace ren {

/// \note The texture names are stored in the page block.

/// Loads a BMFont file from memory and stores relevant BMFont blocks.
class SCISHARE BMFont
{
public:
  BMFont();
  virtual ~BMFont();

  void clear();

  void loadFromBuffer(uint8_t* buffer, size_t bytesRead);

  /// Returns the text width in pixels.
  int32_t getASCIITextWidth(const char* text) const;

  /// Returns the line height in pixels.
  int32_t getLineHeight() const;

  /// Returns the kerning pair adjustment for the two characters given,
  /// in pixels.
  int32_t getKerningAdjustment(uint32_t firstCharID, uint32_t secondCharID) const;

  // Block types that can be loaded from BMFont file.
  enum BLOCK_TYPES
  {
    BLOCK_INFO    = 1,
    BLOCK_COMMON  = 2,
    BLOCK_PAGES   = 3,
    BLOCK_CHARS   = 4,
    BLOCK_KERN    = 5
  };

  /// The following structures represent the data as loaded in from a BMFont
  /// file.
  struct InfoBlock
  {
    int16_t   fontSize;
    uint8_t   bitField;
    uint8_t   charSet;
    uint16_t  stretchH;
    uint8_t   aa;
    uint8_t   paddingUp;
    uint8_t   paddingRight;
    uint8_t   paddingDown;
    uint8_t   paddingLeft;
    uint8_t   spacingHoriz;
    uint8_t   spacingVert;
    uint8_t   outline;
    std::string fontName;
  };

  struct CommonBlock
  {
    uint16_t lineHeight;  ///< Distance, in pixels, between each line of text.
    uint16_t base;        ///< The number of pixels from the absolute top of the line to the base
                          ///< of the characters. http://www.angelcode.com/products/bmfont/doc/render_text.html
    uint16_t scaleW;      ///< Width of the texture. Normally used to scale x pos of the char image.
    uint16_t scaleH;      ///< Height of the texture. Normalled used to scale y pos of the char image.
    uint16_t pages;       ///< Number of texture pages included in the font.
    uint8_t bitField;     ///< Contains the packed bit. If packed is set, then monochrome characters have
                          ///< been packed into each of the texture channels. In this case alphaChannel
                          ///< describes what is stored in each channel.

    /// Set to 0 if the channel holds the glyph data, 1 if it holds the outline,
    /// 2 if it holds the outline and the glyph, 3 if it is set to zero, 4 if it
    /// is set to one.
    /// @{
    uint8_t alphaChannel;
    uint8_t redChannel;
    uint8_t greenChannel;
    uint8_t blueChannel;
    /// @}
  };

  struct PageBlock
  {
    // The id of each page is the zero-based index of the string name.
    // So it is the index into this page block.
    std::vector<std::string> pages;
  };

  // Character info struct. Read from the BLOCK_CHARS.
  struct CharInfo
  {
    uint32_t id;        ///< Character id (unicode id).

    /// For the absolute meaning of all of these values, please consult
    /// http://www.angelcode.com/products/bmfont/doc/render_text.html.
    /// @{
    uint16_t x;         ///< The left position of the character image in the texture.
    uint16_t y;         ///< The top position of the character image in the texture.
    uint16_t width;     ///< The width of the character image in the texture.
    uint16_t height;    ///< The height of the character image in the texture.
    int16_t xoffset;    ///< How much the current cursor position should be offset.
    int16_t yoffset;
    int16_t xadvance;   ///< How much the current position should be advanced after drawing the character.
    /// @}
    uint8_t page;
    uint8_t channel;    ///< Texture channel where the character is found.
  };

  // Kerning information struct. The multipmap is based off of the first
  // element in the kern info structure.
  struct KernInfo
  {
    uint32_t firstID;
    uint32_t secondID;
    int16_t amount;
  };

  /// Returns the character info for the given character ID. Returns nullptr
  /// if the character ID is not present in the system.
  const CharInfo* getCharInfo(uint32_t charID) const;

  // Block accessors.
  const InfoBlock&    getInfoBlock()    const {return mInfoBlock;}
  const CommonBlock&  getCommonBlock()  const {return mCommonBlock;}
  const PageBlock&    getPageBlock()    const {return mPageBlock;}
  const std::map<uint32_t, CharInfo>&       getCharMap()  const {return mCharMap;}
  const std::multimap<uint32_t, KernInfo>&  getKernMap()  const {return mKernMultiMap;}

private:

  /// Various functions to read blocks inside of the BMFont file.
  /// @{
  void readInfoBlock(spire::BSerialize& s);
  void readCommonBlock(spire::BSerialize& s);
  void readPageBlock(spire::BSerialize& s, size_t blockSize);
  void readCharBlock(spire::BSerialize& s, size_t blockSize);
  void readKernBlock(spire::BSerialize& s, size_t blockSize);
  /// @}

  InfoBlock   mInfoBlock;
  CommonBlock mCommonBlock;
  PageBlock   mPageBlock;

  // Since the only time we will be accessing these structures is when we
  // are constructing the text to be displayed, cache locality doesn't
  // matter as much. We save more memory by skipping over the characters that
  // aren't represented in the 0 - 255 range. Additionally, we can represent
  // unicode characters which are much larger in range.
  std::map<uint32_t, CharInfo>      mCharMap;
  std::multimap<uint32_t, KernInfo> mKernMultiMap;
};

} // namespace ren

#endif
