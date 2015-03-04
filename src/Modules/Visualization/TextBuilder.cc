/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2012 Scientific Computing and Imaging Institute,
   University of Utah.

   License for the specific language governing rights and limitations under
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
#include <fstream>
#include <stdio.h>  /* TODO delete after testing!!!!! defines FILENAME_MAX */

using namespace SCIRun::Modules::Visualization;
using namespace SCIRun::Core::Geometry;

TextBuilder::TextBuilder(const std::string& text, const double scale,
            const Vector shift)
: text_(text), scale_(scale), shift_(shift) {
    std::ifstream in("Assets/times_new_roman.font");
    if (in.fail()) {
        //try the MAC App location if the UNIX/Windows location didn't work.
        in.open("SCIRun.app/Contents/MacOS/Assets/times_new_roman.font");
        if (in.fail()) {
            std::cerr << "Cannot find font \"Assets/times_new_roman.font\"" << std::endl;
            return;
        }
    }
    in >> font0_w_ >> font0_h_;
    font0_ = new uint16_t[font0_w_*font0_h_];
    in.read((char*)font0_,sizeof(uint16_t)*font0_w_*font0_h_);
    in.close();
}

TextBuilder::~TextBuilder() {
    delete font0_;
}

void TextBuilder::getStringVerts(std::vector<Vector> &verts,  std::vector<Vector> &colors) {
    if (!font0_) return;
    Vector shift = shift_;
    for (auto a : text_) {
        std::vector<Vector> tmp;
        std::vector<Vector> tmp2;
        getCharVerts(a, tmp,tmp2);
        for (auto v : tmp) {
            verts.push_back(v + shift);
        }
        for (auto v : tmp2) {
            colors.push_back(v);
        }
        shift = shift + Vector(scale_,0.,0.);
    }
}

void TextBuilder::getCharVerts(const char c, std::vector<Vector> &verts, std::vector<Vector> &colors) {
    char idx = c - 32;
    if (idx < 0 || idx >= 96) idx = 95;
    //get the offset into the font array.
    size_t row = idx / 16;
    size_t col = c % 16;
    size_t pixel_row = row * 64;
    size_t pixel_col = col * 64;
    const uint16_t *font = font0_;
    size_t resolution = 128;
    for (size_t i = 0; i < resolution; i++) {
        for (size_t j = 0; j < resolution; j++) {
            //sample resolution x resolution across the square of the current letter.
            size_t sample_x = 2+pixel_col + static_cast<size_t>(60. * static_cast<double>(j) /
                                                    static_cast<double>(resolution)+.5);
            size_t sample_y = 2+pixel_row + static_cast<size_t>(60. * static_cast<double>(i) /
                                                    static_cast<double>(resolution)+.5);
            uint16_t pixel = 0;//font[sample_y*1024 + sample_x];
            //anti-alias
            for(int u = -1; u <= 0; u++) {
                for (int v = -1; v <= 0; v++) {
                    size_t sub_x = std::min(std::max((size_t)0,u+sample_x),font0_w_);
                    size_t sub_y = std::min(std::max((size_t)0,v+sample_y),font0_h_);
                    uint16_t pixel2 = font[sub_y*font0_w_ + sub_x];
                    pixel = ((pixel & 0xff00) + ((((pixel2 >> 8) / 4) << 8) & 0xff00)) |
                                ((pixel & 0x00ff) + ((pixel2 & 0x00ff) / 4));
                }
            }
            unsigned char val = (pixel >> 8) & 0x00ff;
            unsigned char alpha = pixel & 0x00ff;
            if (alpha > 100) {
                verts.push_back(scale_ * Vector(static_cast<double>(j),
                                resolution-static_cast<double>(i),0.) /
                                static_cast<double>(resolution));
                colors.push_back(Vector(static_cast<float>(val) / 255.f,
                                        static_cast<float>(alpha) / 255.f,0.));
            }
        }
    }
}

void TextBuilder::reset(const std::string& text, const double scale, const Vector shift) {
    text_ = text;
    scale_ = scale;
    shift_ = shift;
}
