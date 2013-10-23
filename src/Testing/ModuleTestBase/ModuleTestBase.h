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

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <Dataflow/Network/NetworkFwd.h>
#include <Core/Datatypes/DatatypeFwd.h>
#include <Core/Algorithms/Base/AlgorithmBase.h>
#include <Testing/ModuleTestBase/share.h>

namespace SCIRun 
{
  namespace Testing 
  {
    class SCISHARE ModuleTest : public ::testing::Test
    {
    protected:
      ModuleTest();

      Dataflow::Networks::ModuleHandle makeModule(const std::string& name);

      void stubPortNWithThisData(Dataflow::Networks::ModuleHandle module, size_t portNum, Core::Datatypes::DatatypeHandle data);
      void connectDummyOutputConnection(Dataflow::Networks::ModuleHandle module, size_t portNum);
    private:
      Dataflow::Networks::ModuleFactoryHandle factory_;
    };

    SCISHARE FieldHandle CreateEmptyLatVol();

    class SCISHARE MockAlgorithm : public SCIRun::Core::Algorithms::AlgorithmBase
    {
    public:
      MOCK_CONST_METHOD1(run_generic, SCIRun::Core::Algorithms::AlgorithmOutput(const SCIRun::Core::Algorithms::AlgorithmInput&));
      MOCK_METHOD1(keyNotFoundPolicy, void(const SCIRun::Core::Algorithms::AlgorithmParameterName&));
      MOCK_METHOD2(set, void(const SCIRun::Core::Algorithms::AlgorithmParameterName&, const SCIRun::Core::Algorithms::AlgorithmParameter::Value&));
      MOCK_CONST_METHOD1(get, const SCIRun::Core::Algorithms::AlgorithmParameter&(const SCIRun::Core::Algorithms::AlgorithmParameterName&));
      //MOCK_METHOD2(set_option, void(const AlgorithmParameterName&, const std::string& value));
      //MOCK_CONST_METHOD1(get_option, std::string(const AlgorithmParameterName&));
    };

    typedef boost::shared_ptr< ::testing::NiceMock<MockAlgorithm> > MockAlgorithmPtr;

    class SCISHARE UseRealAlgorithmFactory
    {
    public:
      UseRealAlgorithmFactory();
      ~UseRealAlgorithmFactory();
    };
  }
}
