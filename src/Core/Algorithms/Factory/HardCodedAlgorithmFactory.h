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


#ifndef HARD_CODED_ALGORITHM_FACTORY_H
#define HARD_CODED_ALGORITHM_FACTORY_H

#include <map>
#include <boost/function.hpp>
#include <Core/Algorithms/Base/AlgorithmFactory.h>
#include <Core/Algorithms/Factory/share.h>

namespace SCIRun {
  namespace Core {
    namespace Algorithms {

      class SCISHARE HardCodedAlgorithmFactory : public SCIRun::Core::Algorithms::AlgorithmFactory
      {
      public:
        using AlgoMaker = boost::function<AlgorithmBase*()>;
        using NamedAlgoMaker = std::pair<std::string, AlgoMaker>;
        using AlgoMakerMap = std::map<std::string, NamedAlgoMaker>;

        HardCodedAlgorithmFactory();
        virtual SCIRun::Core::Algorithms::AlgorithmHandle create(const std::string& moduleName, const AlgorithmCollaborator* algoCollaborator) const;
        size_t numAlgorithms() const { return factoryMap_.size(); }
        AlgoMakerMap::const_iterator begin() const { return factoryMap_.cbegin(); }
        AlgoMakerMap::const_iterator end() const { return factoryMap_.cend(); }
      private:

        AlgoMakerMap factoryMap_;
        void addToMakerMap();
        void addToMakerMap2(); // @todo: temporary
        void addToMakerMapGenerated();
      };
    }
  }
}

#define ADD_MODULE_ALGORITHM(module, algorithm) (#module, std::make_pair(#algorithm, boost::factory<algorithm*>()))
#define ADD_MODULE_ALGORITHM_GENERATED(module, algorithm) factoryMap_[#module] = std::make_pair(#algorithm, boost::factory<algorithm*>())

#endif
