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


#include <Testing/ModuleTestBase/ModuleTestBase.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Modules/Legacy/Fields/CalculateSignedDistanceToField.h>
#include <Core/Algorithms/Legacy/Fields/DistanceField/CalculateSignedDistanceField.h>

using namespace SCIRun;
using namespace SCIRun::Testing;
using namespace SCIRun::TestUtils;
using namespace SCIRun::Modules::Fields;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Dataflow::Networks;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::DefaultValue;
using ::testing::Return;
using ::testing::Mock;

class CalculateSignedDistanceToFieldModuleTests : public ModuleTest
{

};

TEST_F(CalculateSignedDistanceToFieldModuleTests, ThrowsForNullInput)
{
  auto csdf = makeModule("CalculateSignedDistanceToField");
  FieldHandle nullField;
  stubPortNWithThisData(csdf, 0, nullField);
  stubPortNWithThisData(csdf, 1, nullField);

  EXPECT_THROW(csdf->execute(), NullHandleOnPortException);
}

TEST_F(CalculateSignedDistanceToFieldModuleTests, MakesAlgoDecisionBasedOnValuePortConnection)
{
  {
    auto csdf = makeModule("CalculateSignedDistanceToField");
    stubPortNWithThisData(csdf, 0, CreateEmptyLatVol());
    stubPortNWithThisData(csdf, 1, CreateEmptyLatVol());

    AlgorithmParameter::Value connected = false;
    {
      //std::cout << "0ref count of algo ptr: " << csdf->getAlgorithm().use_count() << std::endl;
      auto mockAlgo = boost::static_pointer_cast<MockAlgorithmPtr::element_type>(csdf->getAlgorithm());
      /// @todo: must remove this line. Getting strange leaking behavior without, haven't tracked it down yet.
      //this test correctly tests the oport_connected-based decision. I'll open another issue to figure out this weird fake leak.
      Mock::AllowLeak(mockAlgo.get());
      //std::cout << "1ref count of algo ptr: " << mockAlgo.use_count() << std::endl;
      {
        EXPECT_CALL(*mockAlgo, set(CalculateSignedDistanceFieldAlgo::OutputValueField, connected));
        //std::cout << "2ref count of algo ptr: " << mockAlgo.use_count() << std::endl;
        csdf->execute();
        //std::cout << "3ref count of algo ptr: " << mockAlgo.use_count() << std::endl;
        EXPECT_TRUE(Mock::VerifyAndClearExpectations(mockAlgo.get()));
        //std::cout << "4ref count of algo ptr: " << mockAlgo.use_count() << std::endl;
      }

      {
        //std::cout << "5ref count of algo ptr: " << mockAlgo.use_count() << std::endl;
        connectDummyOutputConnection(csdf, 1);
        connected = true;
        //std::cout << "6ref count of algo ptr: " << mockAlgo.use_count() << std::endl;
        EXPECT_CALL(*mockAlgo, set(CalculateSignedDistanceFieldAlgo::OutputValueField, connected));
        //std::cout << "7ref count of algo ptr: " << mockAlgo.use_count() << std::endl;
        csdf->execute();
        //std::cout << "8ref count of algo ptr: " << mockAlgo.use_count() << std::endl;
        EXPECT_TRUE(Mock::VerifyAndClearExpectations(mockAlgo.get()));
        //std::cout << "9ref count of algo ptr: " << mockAlgo.use_count() << std::endl;
      }
      //std::cout << "10ref count of algo ptr: " << mockAlgo.use_count() << std::endl;
    }
  }
}
