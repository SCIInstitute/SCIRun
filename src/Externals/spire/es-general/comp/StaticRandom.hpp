#ifndef IAUNS_GENERAL_COMP_STATIC_RANDOM_HPP
#define IAUNS_GENERAL_COMP_STATIC_RANDOM_HPP

#include <random>
#include <ctime>

namespace gen {

struct StaticRandom
{
  // -- Data --
  uint64_t currentSeed; ///< Current seed being used to generate random numbers.

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

  void changeSeed(uint64_t seed)
  {
    currentSeed = seed;
    randEngine.seed(seed);
  }

  static const char* getName() {return "gen:StaticRandom";}

  bool serialize(CPM_ES_CEREAL_NS::ComponentSerialize& /* s */, uint64_t /* entityID */)
  {
    return true;
  }
};

}

#endif
