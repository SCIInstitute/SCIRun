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


#ifndef SPIRE_GENERAL_COMP_STATIC_RANDOM_HPP
#define SPIRE_GENERAL_COMP_STATIC_RANDOM_HPP

#include <es-log/trace-log.h>
#include <random>
#include <ctime>
#include <spire/scishare.h>

namespace gen {

struct StaticRandom
{
  // -- Data --
  std::random_device::result_type currentSeed; ///< Current seed being used to generate random numbers.

  /// \todo Look into random number generation on emscripten. I'm not sure
  ///       it is being done correctly -- unsure about hardware random.
  std::mt19937 randEngine;

  // -- Functions --
  StaticRandom()
  {
    changeToRandomSeed();
  }

  // For anything more complex than the functions below, lookup the c++11
  // distributions.

  // Distributions are leightweight objects and their creation shouldn't
  // affect performance.
  float randFloat(float a, float b)
  {
    return std::uniform_real_distribution<float>(a, b)(randEngine);
  }

  int32_t randInt(int32_t a, int32_t b)
  {
    return std::uniform_int_distribution<int32_t>(a, b)(randEngine);
  }

  void changeToRandomSeed()
  {
#ifdef EMSCRIPTEN
    currentSeed = std::time(NULL);
    randEngine.seed(currentSeed);
#else
    // std::array<int, std::mt19937::state_size> seed_data;
    // std::random_device r;
    // std::generate_n(seed_data.data(), seed_data.size(), std::ref(r));
    // std::seed_seq seq(std::begin(seed_data), std::end(seed_data));
    std::random_device r;
    currentSeed = r();//std::time(NULL);
    randEngine.seed(currentSeed);
#endif
  }

  static const char* getName() {return "gen:StaticRandom";}

  bool serialize(spire::ComponentSerialize& /* s */, uint64_t /* entityID */)
  {
    return true;
  }
};

}

#endif
