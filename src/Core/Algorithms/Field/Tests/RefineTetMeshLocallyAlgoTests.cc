/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
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

#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Algorithms/Field/RefineTetMeshLocallyAlgorithm.h>
#include <Testing/Utils/SCIRunUnitTests.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Testing/Utils/MatrixTestUtilities.h>
#include <Core/IEPlugin/MatlabFiles_Plugin.h>
#include <Core/Algorithms/Legacy/Fields/MeshData/GetMeshNodes.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::TestUtils;

FieldHandle GetCase1InputField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_input/1.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}

FieldHandle GetCase2InputField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_input/2.mat";
  return MatlabField_reader(0, (file.string()).c_str()); 
}

FieldHandle GetCase3InputField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_input/3.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}

FieldHandle GetCase4InputField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_input/4.mat";
  return MatlabField_reader(0, (file.string()).c_str()); 
}
 
FieldHandle GetCase5InputField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_input/5.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}

FieldHandle GetCase6InputField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_input/6.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}

FieldHandle GetCase7InputField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_input/7.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}

FieldHandle GetCase8InputField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_input/8.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}

FieldHandle GetCase9InputField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_input/9.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}

FieldHandle GetCase10InputField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_input/10.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}

FieldHandle GetCase11InputField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_input/11.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}

FieldHandle GetCase12InputField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_input/12.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}
 
FieldHandle GetCase13InputField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_input/13.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}

FieldHandle GetCase14InputField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_input/14.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}

FieldHandle GetCase15InputField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_input/15.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}

FieldHandle GetCase16InputField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_input/16.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}
 
FieldHandle GetCase17InputField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_input/17.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}

FieldHandle GetCase18InputField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_input/18.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}

FieldHandle GetCase19InputField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_input/19.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}

FieldHandle GetCase20InputField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_input/20.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}
 
FieldHandle GetCase21InputField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_input/21.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}

FieldHandle GetCase22InputField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_input/22.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}

FieldHandle GetCase23InputField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_input/23.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}

FieldHandle GetCase24InputField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_input/24.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}

FieldHandle GetCase25InputField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_input/25.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}

FieldHandle GetCase26InputField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_input/26.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}

FieldHandle GetCase27InputField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_input/27.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}

FieldHandle GetCase28InputField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_input/28.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}
 
FieldHandle GetCase29InputField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_input/29.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}

FieldHandle GetCase30InputField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_input/30.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}

FieldHandle GetCase32InputField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_input/32.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}

FieldHandle GetCase33InputField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_input/33.mat";
  return MatlabField_reader(0, (file.string()).c_str());
} 

FieldHandle GetCase34InputField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_input/34.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}

FieldHandle GetCase35InputField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_input/35.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}

FieldHandle GetCase36InputField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_input/36.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}

FieldHandle GetCase37InputField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_input/37.mat";
  return MatlabField_reader(0, (file.string()).c_str());					    }

FieldHandle GetCase38InputField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_input/38.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}

FieldHandle GetCase39InputField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_input/39.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}
 
FieldHandle GetCase40InputField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_input/40.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}

FieldHandle GetCase41InputField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_input/41.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}

FieldHandle GetCase42InputField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_input/42.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}

FieldHandle GetCase43InputField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_input/43.mat";
  return MatlabField_reader(0, (file.string()).c_str());
} 
 
FieldHandle GetCase44InputField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_input/44.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}

FieldHandle GetCase45InputField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_input/45.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}

FieldHandle GetCase46InputField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_input/46.mat";
  return MatlabField_reader(0, (file.string()).c_str());
} 

FieldHandle GetCase48InputField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_input/48.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}

FieldHandle GetCase49InputField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_input/49.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}

FieldHandle GetCase50InputField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_input/50.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}

FieldHandle GetCase51InputField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_input/51.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}

FieldHandle GetCase52InputField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_input/52.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}
 
FieldHandle GetCase53InputField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_input/53.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}

FieldHandle GetCase54InputField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_input/54.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}

FieldHandle GetCase56InputField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_input/56.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}

FieldHandle GetCase57InputField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_input/57.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}  

FieldHandle GetCase58InputField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_input/58.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}
 
FieldHandle GetCase60InputField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_input/60.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}

FieldHandle GetCase63InputField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_input/63.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}

FieldHandle GetCase1ExpectedResultField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_result/1.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}

FieldHandle GetCase2ExpectedResultField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_result/2.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}

FieldHandle GetCase3ExpectedResultField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_result/3.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}

FieldHandle GetCase4ExpectedResultField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_result/4.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}
 
FieldHandle GetCase5ExpectedResultField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_result/5.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}

FieldHandle GetCase6ExpectedResultField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_result/6.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}

FieldHandle GetCase7ExpectedResultField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_result/7.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}

FieldHandle GetCase8ExpectedResultField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_result/8.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}

FieldHandle GetCase9ExpectedResultField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_result/9.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}

FieldHandle GetCase10ExpectedResultField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_result/10.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}

FieldHandle GetCase11ExpectedResultField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_result/11.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}

FieldHandle GetCase12ExpectedResultField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_result/12.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}
 
FieldHandle GetCase13ExpectedResultField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_result/13.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}

FieldHandle GetCase14ExpectedResultField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_result/14.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}

FieldHandle GetCase15ExpectedResultField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_result/15.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}

FieldHandle GetCase16ExpectedResultField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_result/16.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}
 
FieldHandle GetCase17ExpectedResultField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_result/17.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}

FieldHandle GetCase18ExpectedResultField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_result/18.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}

FieldHandle GetCase19ExpectedResultField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_result/19.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}

FieldHandle GetCase20ExpectedResultField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_result/20.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}
 
FieldHandle GetCase21ExpectedResultField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_result/21.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}

FieldHandle GetCase22ExpectedResultField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_result/22.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}

FieldHandle GetCase23ExpectedResultField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_result/23.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}

FieldHandle GetCase24ExpectedResultField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_result/24.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}

FieldHandle GetCase25ExpectedResultField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_result/25.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}

FieldHandle GetCase26ExpectedResultField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_result/26.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}

FieldHandle GetCase27ExpectedResultField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_result/27.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}

FieldHandle GetCase28ExpectedResultField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_result/28.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}
 
FieldHandle GetCase29ExpectedResultField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_result/29.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}

FieldHandle GetCase30ExpectedResultField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_result/30.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}

FieldHandle GetCase31ExpectedResultField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_result/31.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}

FieldHandle GetCase32ExpectedResultField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_result/32.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}

FieldHandle GetCase33ExpectedResultField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_result/33.mat";
  return MatlabField_reader(0, (file.string()).c_str());
} 

FieldHandle GetCase34ExpectedResultField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_result/34.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}

FieldHandle GetCase35ExpectedResultField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_result/35.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}

FieldHandle GetCase36ExpectedResultField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_result/36.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}

FieldHandle GetCase37ExpectedResultField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_result/37.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}

FieldHandle GetCase38ExpectedResultField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_result/38.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}

FieldHandle GetCase39ExpectedResultField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_result/39.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}
 
FieldHandle GetCase40ExpectedResultField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_result/40.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}

FieldHandle GetCase41ExpectedResultField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_result/41.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}

FieldHandle GetCase42ExpectedResultField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_result/42.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}

FieldHandle GetCase43ExpectedResultField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_result/43.mat";
  return MatlabField_reader(0, (file.string()).c_str());
} 
 
FieldHandle GetCase44ExpectedResultField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_result/44.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}

FieldHandle GetCase45ExpectedResultField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_result/45.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}

FieldHandle GetCase46ExpectedResultField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_result/46.mat";
  return MatlabField_reader(0, (file.string()).c_str());
} 

FieldHandle GetCase47ExpectedResultField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_result/47.mat";
  return MatlabField_reader(0, (file.string()).c_str());
} 

FieldHandle GetCase48ExpectedResultField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_result/48.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}

FieldHandle GetCase49ExpectedResultField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_result/49.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}

FieldHandle GetCase50ExpectedResultField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_result/50.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}

FieldHandle GetCase51ExpectedResultField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_result/51.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}

FieldHandle GetCase52ExpectedResultField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_result/52.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}
 
FieldHandle GetCase53ExpectedResultField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_result/53.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}

FieldHandle GetCase54ExpectedResultField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_result/54.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}

FieldHandle GetCase55ExpectedResultField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_result/55.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}

FieldHandle GetCase56ExpectedResultField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_result/56.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}

FieldHandle GetCase57ExpectedResultField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_result/57.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}  

FieldHandle GetCase58ExpectedResultField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_result/58.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}

FieldHandle GetCase59ExpectedResultField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_result/59.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}
 
FieldHandle GetCase60ExpectedResultField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_result/60.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}

FieldHandle GetCase61ExpectedResultField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_result/61.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}

FieldHandle GetCase62ExpectedResultField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_result/62.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}

FieldHandle GetCase63ExpectedResultField()
{
  auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_result/63.mat";
  return MatlabField_reader(0, (file.string()).c_str());
}

TEST(RefineTetMeshLocallyAlgoTests, Test59basicCutCases)
{
 double epsilon=1e-8;
 
 RefineTetMeshLocallyAlgorithm algo;  
 algo.set(Parameters::RefineTetMeshLocallyIsoValue, 1.0);
 algo.set(Parameters::RefineTetMeshLocallyEdgeLength, 0.0);
 algo.set(Parameters::RefineTetMeshLocallyVolume, 0.0);
 algo.set(Parameters::RefineTetMeshLocallyDihedralAngleSmaller, 180.0);
 algo.set(Parameters::RefineTetMeshLocallyDihedralAngleBigger, 0.0);
 algo.set(Parameters::RefineTetMeshLocallyRadioButtons, 0);
 algo.set(Parameters::RefineTetMeshLocallyDoNoSplitSurfaceTets, false);
 algo.set(Parameters::RefineTetMeshLocallyCounterClockWiseOrdering, false);
 algo.set(Parameters::RefineTetMeshLocallyUseModuleInputField, false);
 algo.set(Parameters::RefineTetMeshLocallyMaxNumberRefinementIterations, 1);
 GetMeshNodesAlgo getfieldnodes_algo;
 DenseMatrixHandle output_nodes,exp_result_nodes;
 VMesh::Node::array_type onodes1(4),onodes2(4); 
 VMesh *output_vmesh, *exp_result_vmesh;
 FieldHandle input,output, exp_result;

 /// CASE 1
 input=GetCase1InputField();
 exp_result=GetCase1ExpectedResultField();
  
 try
 {
  algo.runImpl(input, output);
 } catch (...) 
 {
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 1 does not work. " << std::endl;
 }
 
 exp_result_vmesh=exp_result->vmesh();
 output_vmesh=output->vmesh(); 
 
 try
 {
  getfieldnodes_algo.run(output,output_nodes);
  getfieldnodes_algo.run(exp_result,exp_result_nodes);
 } catch (...)
 {
  FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 1 does not work (could not get field nodes from input files). " << std::endl;
 }
 
 if(output_nodes->ncols()!=exp_result_nodes->ncols() || output_nodes->nrows()!=exp_result_nodes->nrows() )
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 1 does not work (number of nodes is different than expected). " << std::endl;
 
 for (long idx=0;idx<exp_result_nodes->nrows();idx++)
 {
   EXPECT_NEAR( (*exp_result_nodes)(idx,0),(*output_nodes)(idx,0), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,1),(*output_nodes)(idx,1), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,2),(*output_nodes)(idx,2), epsilon);
 }
 
 output_vmesh->synchronize(Mesh::NODES_E);
 exp_result_vmesh->synchronize(Mesh::NODES_E);
 
 if( output_vmesh->num_elems()!=exp_result_vmesh->num_elems() || output_vmesh->num_elems()!=exp_result_vmesh->num_elems() )
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 1 does not work (number of elements is different than expected). " << std::endl;
 
 for(VMesh::Elem::index_type idx=0; idx<output_vmesh->num_elems(); idx++)
 {
  output_vmesh->get_nodes(onodes1, idx);
  exp_result_vmesh->get_nodes(onodes2, idx);
  for (int j=0;j<4;j++)
   if (onodes1[j]!=onodes2[j])
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 1 does not work (definition of resulting and expected tet definition differs). " << std::endl;    
 }
 
 
 /// CASE 2
 input=GetCase2InputField();
 exp_result=GetCase2ExpectedResultField();
  
 try
 {
  algo.runImpl(input, output);
 } catch (...) 
 {
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 2 does not work. " << std::endl;
 }
 
 exp_result_vmesh=exp_result->vmesh();
 output_vmesh=output->vmesh(); 
 
 try
 {
  getfieldnodes_algo.run(output,output_nodes);
  getfieldnodes_algo.run(exp_result,exp_result_nodes);
 } catch (...)
 {
  FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 2 does not work (could not get field nodes from input files). " << std::endl;
 }
 
 if(output_nodes->ncols()!=exp_result_nodes->ncols() || output_nodes->nrows()!=exp_result_nodes->nrows() )
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 2 does not work (number of nodes is different than expected). " << std::endl;
 
 for (long idx=0;idx<exp_result_nodes->nrows();idx++)
 {
   EXPECT_NEAR( (*exp_result_nodes)(idx,0),(*output_nodes)(idx,0), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,1),(*output_nodes)(idx,1), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,2),(*output_nodes)(idx,2), epsilon);
 }
 
 output_vmesh->synchronize(Mesh::NODES_E);
 exp_result_vmesh->synchronize(Mesh::NODES_E);
 
 if( output_vmesh->num_elems()!=exp_result_vmesh->num_elems() || output_vmesh->num_elems()!=exp_result_vmesh->num_elems() )
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 2 does not work (number of elements is different than expected). " << std::endl;
 
 for(VMesh::Elem::index_type idx=0; idx<output_vmesh->num_elems(); idx++)
 {
  output_vmesh->get_nodes(onodes1, idx);
  exp_result_vmesh->get_nodes(onodes2, idx);
  for (int j=0;j<4;j++)
   if (onodes1[j]!=onodes2[j])
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 2 does not work (definition of resulting and expected tet definition differs). " << std::endl;    
 } 
 
 
  /// CASE 3
 input=GetCase3InputField();
 exp_result=GetCase3ExpectedResultField();
    
 try
 {
  algo.runImpl(input, output);
 } catch (...) 
 {
  FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 3 does not work. " << std::endl;
 }
  
 exp_result_vmesh=exp_result->vmesh();
 output_vmesh=output->vmesh(); 
 
 try
 {
  getfieldnodes_algo.run(output,output_nodes);
  getfieldnodes_algo.run(exp_result,exp_result_nodes);
 } catch (...)
 {
  FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 3 does not work (could not get field nodes from input files). " << std::endl;
 }
 
 if(output_nodes->ncols()!=exp_result_nodes->ncols() || output_nodes->nrows()!=exp_result_nodes->nrows() )
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 3 does not work (number of nodes is different than expected). " << std::endl;
 
 for (long idx=0;idx<exp_result_nodes->nrows();idx++)
 {
   EXPECT_NEAR( (*exp_result_nodes)(idx,0),(*output_nodes)(idx,0), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,1),(*output_nodes)(idx,1), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,2),(*output_nodes)(idx,2), epsilon);
 }
 
 output_vmesh->synchronize(Mesh::NODES_E);
 exp_result_vmesh->synchronize(Mesh::NODES_E);
 
 if( output_vmesh->num_elems()!=exp_result_vmesh->num_elems() || output_vmesh->num_elems()!=exp_result_vmesh->num_elems() )
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 3 does not work (number of elements is different than expected). " << std::endl;
 
 for(VMesh::Elem::index_type idx=0; idx<output_vmesh->num_elems(); idx++)
 {
  output_vmesh->get_nodes(onodes1, idx);
  exp_result_vmesh->get_nodes(onodes2, idx);
  for (int j=0;j<4;j++)
   if (onodes1[j]!=onodes2[j])
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 3 does not work (definition of resulting and expected tet definition differs). " << std::endl;    
 } 
 
  /// CASE 4
 input=GetCase4InputField();
 exp_result=GetCase4ExpectedResultField();
  
 try
 {
  algo.runImpl(input, output);
 } catch (...) 
 {
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 4 does not work. " << std::endl;
 }
 
 exp_result_vmesh=exp_result->vmesh();
 output_vmesh=output->vmesh(); 
 
 try
 {
  getfieldnodes_algo.run(output,output_nodes);
  getfieldnodes_algo.run(exp_result,exp_result_nodes);
 } catch (...)
 {
  FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 4 does not work (could not get field nodes from input files). " << std::endl;
 }
 
 if(output_nodes->ncols()!=exp_result_nodes->ncols() || output_nodes->nrows()!=exp_result_nodes->nrows() )
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 4 does not work (number of nodes is different than expected). " << std::endl;
 
 for (long idx=0;idx<exp_result_nodes->nrows();idx++)
 {
   EXPECT_NEAR( (*exp_result_nodes)(idx,0),(*output_nodes)(idx,0), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,1),(*output_nodes)(idx,1), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,2),(*output_nodes)(idx,2), epsilon);
 }
 
 output_vmesh->synchronize(Mesh::NODES_E);
 exp_result_vmesh->synchronize(Mesh::NODES_E);
 
 if( output_vmesh->num_elems()!=exp_result_vmesh->num_elems() || output_vmesh->num_elems()!=exp_result_vmesh->num_elems() )
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 4 does not work (number of elements is different than expected). " << std::endl;
 
 for(VMesh::Elem::index_type idx=0; idx<output_vmesh->num_elems(); idx++)
 {
  output_vmesh->get_nodes(onodes1, idx);
  exp_result_vmesh->get_nodes(onodes2, idx);
  for (int j=0;j<4;j++)
   if (onodes1[j]!=onodes2[j])
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 4 does not work (definition of resulting and expected tet definition differs). " << std::endl;    
 } 
 
 /// CASE 5
 input=GetCase5InputField();
 exp_result=GetCase5ExpectedResultField();
  
 try
 {
  algo.runImpl(input, output);
 } catch (...) 
 {
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 5 does not work. " << std::endl;
 }
    
 exp_result_vmesh=exp_result->vmesh();
 output_vmesh=output->vmesh(); 
 
 try
 {
  getfieldnodes_algo.run(output,output_nodes);
  getfieldnodes_algo.run(exp_result,exp_result_nodes);
 } catch (...)
 {
  FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 5 does not work (could not get field nodes from input files). " << std::endl;
 }
 
 if(output_nodes->ncols()!=exp_result_nodes->ncols() || output_nodes->nrows()!=exp_result_nodes->nrows() )
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 5 does not work (number of nodes is different than expected). " << std::endl;
 
 for (long idx=0;idx<exp_result_nodes->nrows();idx++)
 {
   EXPECT_NEAR( (*exp_result_nodes)(idx,0),(*output_nodes)(idx,0), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,1),(*output_nodes)(idx,1), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,2),(*output_nodes)(idx,2), epsilon);
 }
 
 output_vmesh->synchronize(Mesh::NODES_E);
 exp_result_vmesh->synchronize(Mesh::NODES_E);
 
 if( output_vmesh->num_elems()!=exp_result_vmesh->num_elems() || output_vmesh->num_elems()!=exp_result_vmesh->num_elems() )
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 5 does not work (number of elements is different than expected). " << std::endl;
 
 for(VMesh::Elem::index_type idx=0; idx<output_vmesh->num_elems(); idx++)
 {
  output_vmesh->get_nodes(onodes1, idx);
  exp_result_vmesh->get_nodes(onodes2, idx);
  for (int j=0;j<4;j++)
   if (onodes1[j]!=onodes2[j])
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 5 does not work (definition of resulting and expected tet definition differs). " << std::endl;    
 } 

  /// CASE 6
 input=GetCase6InputField();
 exp_result=GetCase6ExpectedResultField();
  
 try
 {
  algo.runImpl(input, output);
 } catch (...) 
 {
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 6 does not work. " << std::endl;
 }
 
 exp_result_vmesh=exp_result->vmesh();
 output_vmesh=output->vmesh(); 
 
 try
 {
  getfieldnodes_algo.run(output,output_nodes);
  getfieldnodes_algo.run(exp_result,exp_result_nodes);
 } catch (...)
 {
  FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 6 does not work (could not get field nodes from input files). " << std::endl;
 }
 
 if(output_nodes->ncols()!=exp_result_nodes->ncols() || output_nodes->nrows()!=exp_result_nodes->nrows() )
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 6 does not work (number of nodes is different than expected). " << std::endl;
 
 for (long idx=0;idx<exp_result_nodes->nrows();idx++)
 {
   EXPECT_NEAR( (*exp_result_nodes)(idx,0),(*output_nodes)(idx,0), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,1),(*output_nodes)(idx,1), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,2),(*output_nodes)(idx,2), epsilon);
 }
 
 output_vmesh->synchronize(Mesh::NODES_E);
 exp_result_vmesh->synchronize(Mesh::NODES_E);
 
 if( output_vmesh->num_elems()!=exp_result_vmesh->num_elems() || output_vmesh->num_elems()!=exp_result_vmesh->num_elems() )
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 6 does not work (number of elements is different than expected). " << std::endl;
 
 for(VMesh::Elem::index_type idx=0; idx<output_vmesh->num_elems(); idx++)
 {
  output_vmesh->get_nodes(onodes1, idx);
  exp_result_vmesh->get_nodes(onodes2, idx);
  for (int j=0;j<4;j++)
   if (onodes1[j]!=onodes2[j])
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 6 does not work (definition of resulting and expected tet definition differs). " << std::endl;    
 }  
 
  /// CASE 7
 input=GetCase7InputField();
 exp_result=GetCase7ExpectedResultField();
  
 try
 {
  algo.runImpl(input, output);
 } catch (...) 
 {
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 7 does not work. " << std::endl;
 }
 
 exp_result_vmesh=exp_result->vmesh();
 output_vmesh=output->vmesh(); 
 
 try
 {
  getfieldnodes_algo.run(output,output_nodes);
  getfieldnodes_algo.run(exp_result,exp_result_nodes);
 } catch (...)
 {
  FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 7 does not work (could not get field nodes from input files). " << std::endl;
 }
 
 if(output_nodes->ncols()!=exp_result_nodes->ncols() || output_nodes->nrows()!=exp_result_nodes->nrows() )
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 7 does not work (number of nodes is different than expected). " << std::endl;
 
 for (long idx=0;idx<exp_result_nodes->nrows();idx++)
 {
   EXPECT_NEAR( (*exp_result_nodes)(idx,0),(*output_nodes)(idx,0), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,1),(*output_nodes)(idx,1), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,2),(*output_nodes)(idx,2), epsilon);
 }
 
 output_vmesh->synchronize(Mesh::NODES_E);
 exp_result_vmesh->synchronize(Mesh::NODES_E);
 
 if( output_vmesh->num_elems()!=exp_result_vmesh->num_elems() || output_vmesh->num_elems()!=exp_result_vmesh->num_elems() )
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 7 does not work (number of elements is different than expected). " << std::endl;
 
 for(VMesh::Elem::index_type idx=0; idx<output_vmesh->num_elems(); idx++)
 {
  output_vmesh->get_nodes(onodes1, idx);
  exp_result_vmesh->get_nodes(onodes2, idx);
  for (int j=0;j<4;j++)
   if (onodes1[j]!=onodes2[j])
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 7 does not work (definition of resulting and expected tet definition differs). " << std::endl;    
 } 
 
 
  /// CASE 8
 input=GetCase8InputField();
 exp_result=GetCase8ExpectedResultField();
  
 try
 {
  algo.runImpl(input, output);
 } catch (...) 
 {
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 8 does not work. " << std::endl;
 }
 
 exp_result_vmesh=exp_result->vmesh();
 output_vmesh=output->vmesh(); 
 
 try
 {
  getfieldnodes_algo.run(output,output_nodes);
  getfieldnodes_algo.run(exp_result,exp_result_nodes);
 } catch (...)
 {
  FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 8 does not work (could not get field nodes from input files). " << std::endl;
 }
 
 if(output_nodes->ncols()!=exp_result_nodes->ncols() || output_nodes->nrows()!=exp_result_nodes->nrows() )
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 8 does not work (number of nodes is different than expected). " << std::endl;
 
 for (long idx=0;idx<exp_result_nodes->nrows();idx++)
 {
   EXPECT_NEAR( (*exp_result_nodes)(idx,0),(*output_nodes)(idx,0), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,1),(*output_nodes)(idx,1), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,2),(*output_nodes)(idx,2), epsilon);
 }
 
 output_vmesh->synchronize(Mesh::NODES_E);
 exp_result_vmesh->synchronize(Mesh::NODES_E);
 
 if( output_vmesh->num_elems()!=exp_result_vmesh->num_elems() || output_vmesh->num_elems()!=exp_result_vmesh->num_elems() )
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 8 does not work (number of elements is different than expected). " << std::endl;
 
 for(VMesh::Elem::index_type idx=0; idx<output_vmesh->num_elems(); idx++)
 {
  output_vmesh->get_nodes(onodes1, idx);
  exp_result_vmesh->get_nodes(onodes2, idx);
  for (int j=0;j<4;j++)
   if (onodes1[j]!=onodes2[j])
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 8 does not work (definition of resulting and expected tet definition differs). " << std::endl;    
 } 
   
  /// CASE 9
 input=GetCase9InputField();
 exp_result=GetCase9ExpectedResultField();
  
 try
 {
  algo.runImpl(input, output);
 } catch (...) 
 {
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 9 does not work. " << std::endl;
 }
 
 exp_result_vmesh=exp_result->vmesh();
 output_vmesh=output->vmesh(); 
 
 try
 {
  getfieldnodes_algo.run(output,output_nodes);
  getfieldnodes_algo.run(exp_result,exp_result_nodes);
 } catch (...)
 {
  FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 9 does not work (could not get field nodes from input files). " << std::endl;
 }
 
 if(output_nodes->ncols()!=exp_result_nodes->ncols() || output_nodes->nrows()!=exp_result_nodes->nrows() )
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 9 does not work (number of nodes is different than expected). " << std::endl;
 
 for (long idx=0;idx<exp_result_nodes->nrows();idx++)
 {
   EXPECT_NEAR( (*exp_result_nodes)(idx,0),(*output_nodes)(idx,0), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,1),(*output_nodes)(idx,1), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,2),(*output_nodes)(idx,2), epsilon);
 }
 
 output_vmesh->synchronize(Mesh::NODES_E);
 exp_result_vmesh->synchronize(Mesh::NODES_E);
 
 if( output_vmesh->num_elems()!=exp_result_vmesh->num_elems() || output_vmesh->num_elems()!=exp_result_vmesh->num_elems() )
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 9 does not work (number of elements is different than expected). " << std::endl;
 
 for(VMesh::Elem::index_type idx=0; idx<output_vmesh->num_elems(); idx++)
 {
  output_vmesh->get_nodes(onodes1, idx);
  exp_result_vmesh->get_nodes(onodes2, idx);
  for (int j=0;j<4;j++)
   if (onodes1[j]!=onodes2[j])
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 9 does not work (definition of resulting and expected tet definition differs). " << std::endl;    
 } 
 
 /// CASE 10
 input=GetCase10InputField();
 exp_result=GetCase10ExpectedResultField();
  
 try
 {
  algo.runImpl(input, output);
 } catch (...) 
 {
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 10 does not work. " << std::endl;
 }
 
 exp_result_vmesh=exp_result->vmesh();
 output_vmesh=output->vmesh(); 
 
 try
 {
  getfieldnodes_algo.run(output,output_nodes);
  getfieldnodes_algo.run(exp_result,exp_result_nodes);
 } catch (...)
 {
  FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 10 does not work (could not get field nodes from input files). " << std::endl;
 }
 
 if(output_nodes->ncols()!=exp_result_nodes->ncols() || output_nodes->nrows()!=exp_result_nodes->nrows() )
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 10 does not work (number of nodes is different than expected). " << std::endl;
 
 for (long idx=0;idx<exp_result_nodes->nrows();idx++)
 {
   EXPECT_NEAR( (*exp_result_nodes)(idx,0),(*output_nodes)(idx,0), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,1),(*output_nodes)(idx,1), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,2),(*output_nodes)(idx,2), epsilon);
 }
 
 output_vmesh->synchronize(Mesh::NODES_E);
 exp_result_vmesh->synchronize(Mesh::NODES_E);
 
 if( output_vmesh->num_elems()!=exp_result_vmesh->num_elems() || output_vmesh->num_elems()!=exp_result_vmesh->num_elems() )
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 10 does not work (number of elements is different than expected). " << std::endl;
 
 for(VMesh::Elem::index_type idx=0; idx<output_vmesh->num_elems(); idx++)
 {
  output_vmesh->get_nodes(onodes1, idx);
  exp_result_vmesh->get_nodes(onodes2, idx);
  for (int j=0;j<4;j++)
   if (onodes1[j]!=onodes2[j])
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 10 does not work (definition of resulting and expected tet definition differs). " << std::endl;    
 } 
 
 /// CASE 11
 input=GetCase11InputField();
 exp_result=GetCase11ExpectedResultField();
  
 try
 {
  algo.runImpl(input, output);
 } catch (...) 
 {
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 11 does not work. " << std::endl;
 }
 
 exp_result_vmesh=exp_result->vmesh();
 output_vmesh=output->vmesh(); 
 
 try
 {
  getfieldnodes_algo.run(output,output_nodes);
  getfieldnodes_algo.run(exp_result,exp_result_nodes);
 } catch (...)
 {
  FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 11 does not work (could not get field nodes from input files). " << std::endl;
 }
 
 if(output_nodes->ncols()!=exp_result_nodes->ncols() || output_nodes->nrows()!=exp_result_nodes->nrows() )
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 11 does not work (number of nodes is different than expected). " << std::endl;
 
 for (long idx=0;idx<exp_result_nodes->nrows();idx++)
 {
   EXPECT_NEAR( (*exp_result_nodes)(idx,0),(*output_nodes)(idx,0), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,1),(*output_nodes)(idx,1), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,2),(*output_nodes)(idx,2), epsilon);
 }
 
 output_vmesh->synchronize(Mesh::NODES_E);
 exp_result_vmesh->synchronize(Mesh::NODES_E);
 
 if( output_vmesh->num_elems()!=exp_result_vmesh->num_elems() || output_vmesh->num_elems()!=exp_result_vmesh->num_elems() )
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 11 does not work (number of elements is different than expected). " << std::endl;
 
 for(VMesh::Elem::index_type idx=0; idx<output_vmesh->num_elems(); idx++)
 {
  output_vmesh->get_nodes(onodes1, idx);
  exp_result_vmesh->get_nodes(onodes2, idx);
  for (int j=0;j<4;j++)
   if (onodes1[j]!=onodes2[j])
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 11 does not work (definition of resulting and expected tet definition differs). " << std::endl;    
 }
 
 
 /// CASE 12
 input=GetCase12InputField();
 exp_result=GetCase12ExpectedResultField();
  
 try
 {
  algo.runImpl(input, output);
 } catch (...) 
 {
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 12 does not work. " << std::endl;
 }
 
 exp_result_vmesh=exp_result->vmesh();
 output_vmesh=output->vmesh(); 
 
 try
 {
  getfieldnodes_algo.run(output,output_nodes);
  getfieldnodes_algo.run(exp_result,exp_result_nodes);
 } catch (...)
 {
  FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 12 does not work (could not get field nodes from input files). " << std::endl;
 }
 
 if(output_nodes->ncols()!=exp_result_nodes->ncols() || output_nodes->nrows()!=exp_result_nodes->nrows() )
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 12 does not work (number of nodes is different than expected). " << std::endl;
 
 for (long idx=0;idx<exp_result_nodes->nrows();idx++)
 {
   EXPECT_NEAR( (*exp_result_nodes)(idx,0),(*output_nodes)(idx,0), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,1),(*output_nodes)(idx,1), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,2),(*output_nodes)(idx,2), epsilon);
 }
 
 output_vmesh->synchronize(Mesh::NODES_E);
 exp_result_vmesh->synchronize(Mesh::NODES_E);
 
 if( output_vmesh->num_elems()!=exp_result_vmesh->num_elems() || output_vmesh->num_elems()!=exp_result_vmesh->num_elems() )
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 12 does not work (number of elements is different than expected). " << std::endl;
 
 for(VMesh::Elem::index_type idx=0; idx<output_vmesh->num_elems(); idx++)
 {
  output_vmesh->get_nodes(onodes1, idx);
  exp_result_vmesh->get_nodes(onodes2, idx);
  for (int j=0;j<4;j++)
   if (onodes1[j]!=onodes2[j])
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 12 does not work (definition of resulting and expected tet definition differs). " << std::endl;    
 } 
 
 
 /// CASE 13
 input=GetCase13InputField();
 exp_result=GetCase13ExpectedResultField();
    
 try
 {
  algo.runImpl(input, output);
 } catch (...) 
 {
  FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 13 does not work. " << std::endl;
 }
  
 exp_result_vmesh=exp_result->vmesh();
 output_vmesh=output->vmesh(); 
 
 try
 {
  getfieldnodes_algo.run(output,output_nodes);
  getfieldnodes_algo.run(exp_result,exp_result_nodes);
 } catch (...)
 {
  FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 13 does not work (could not get field nodes from input files). " << std::endl;
 }
 
 if(output_nodes->ncols()!=exp_result_nodes->ncols() || output_nodes->nrows()!=exp_result_nodes->nrows() )
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 13 does not work (number of nodes is different than expected). " << std::endl;
 
 for (long idx=0;idx<exp_result_nodes->nrows();idx++)
 {
   EXPECT_NEAR( (*exp_result_nodes)(idx,0),(*output_nodes)(idx,0), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,1),(*output_nodes)(idx,1), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,2),(*output_nodes)(idx,2), epsilon);
 }
 
 output_vmesh->synchronize(Mesh::NODES_E);
 exp_result_vmesh->synchronize(Mesh::NODES_E);
 
 if( output_vmesh->num_elems()!=exp_result_vmesh->num_elems() || output_vmesh->num_elems()!=exp_result_vmesh->num_elems() )
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 13 does not work (number of elements is different than expected). " << std::endl;
 
 for(VMesh::Elem::index_type idx=0; idx<output_vmesh->num_elems(); idx++)
 {
  output_vmesh->get_nodes(onodes1, idx);
  exp_result_vmesh->get_nodes(onodes2, idx);
  for (int j=0;j<4;j++)
   if (onodes1[j]!=onodes2[j])
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 13 does not work (definition of resulting and expected tet definition differs). " << std::endl;    
 } 
 
 /// CASE 14
 input=GetCase14InputField();
 exp_result=GetCase14ExpectedResultField();
  
 try
 {
  algo.runImpl(input, output);
 } catch (...) 
 {
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 14 does not work. " << std::endl;
 }
 
 exp_result_vmesh=exp_result->vmesh();
 output_vmesh=output->vmesh(); 
 
 try
 {
  getfieldnodes_algo.run(output,output_nodes);
  getfieldnodes_algo.run(exp_result,exp_result_nodes);
 } catch (...)
 {
  FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 14 does not work (could not get field nodes from input files). " << std::endl;
 }
 
 if(output_nodes->ncols()!=exp_result_nodes->ncols() || output_nodes->nrows()!=exp_result_nodes->nrows() )
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 14 does not work (number of nodes is different than expected). " << std::endl;
 
 for (long idx=0;idx<exp_result_nodes->nrows();idx++)
 {
   EXPECT_NEAR( (*exp_result_nodes)(idx,0),(*output_nodes)(idx,0), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,1),(*output_nodes)(idx,1), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,2),(*output_nodes)(idx,2), epsilon);
 }
 
 output_vmesh->synchronize(Mesh::NODES_E);
 exp_result_vmesh->synchronize(Mesh::NODES_E);
 
 if( output_vmesh->num_elems()!=exp_result_vmesh->num_elems() || output_vmesh->num_elems()!=exp_result_vmesh->num_elems() )
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 14 does not work (number of elements is different than expected). " << std::endl;
 
 for(VMesh::Elem::index_type idx=0; idx<output_vmesh->num_elems(); idx++)
 {
  output_vmesh->get_nodes(onodes1, idx);
  exp_result_vmesh->get_nodes(onodes2, idx);
  for (int j=0;j<4;j++)
   if (onodes1[j]!=onodes2[j])
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 14 does not work (definition of resulting and expected tet definition differs). " << std::endl;    
 } 
 
 
 /// CASE 15
 input=GetCase15InputField();
 exp_result=GetCase15ExpectedResultField();
  
 try
 {
  algo.runImpl(input, output);
 } catch (...) 
 {
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 15 does not work. " << std::endl;
 }
 
 exp_result_vmesh=exp_result->vmesh();
 output_vmesh=output->vmesh(); 
 
 try
 {
  getfieldnodes_algo.run(output,output_nodes);
  getfieldnodes_algo.run(exp_result,exp_result_nodes);
 } catch (...)
 {
  FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 15 does not work (could not get field nodes from input files). " << std::endl;
 }
 
 if(output_nodes->ncols()!=exp_result_nodes->ncols() || output_nodes->nrows()!=exp_result_nodes->nrows() )
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 1 does not work (number of nodes is different than expected). " << std::endl;
 
 for (long idx=0;idx<exp_result_nodes->nrows();idx++)
 {
   EXPECT_NEAR( (*exp_result_nodes)(idx,0),(*output_nodes)(idx,0), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,1),(*output_nodes)(idx,1), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,2),(*output_nodes)(idx,2), epsilon);
 }
 
 output_vmesh->synchronize(Mesh::NODES_E);
 exp_result_vmesh->synchronize(Mesh::NODES_E);
 
 if( output_vmesh->num_elems()!=exp_result_vmesh->num_elems() || output_vmesh->num_elems()!=exp_result_vmesh->num_elems() )
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 15 does not work (number of elements is different than expected). " << std::endl;
 
 for(VMesh::Elem::index_type idx=0; idx<output_vmesh->num_elems(); idx++)
 {
  output_vmesh->get_nodes(onodes1, idx);
  exp_result_vmesh->get_nodes(onodes2, idx);
  for (int j=0;j<4;j++)
   if (onodes1[j]!=onodes2[j])
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 15 does not work (definition of resulting and expected tet definition differs). " << std::endl;    
 } 
 
 /// CASE 16
 input=GetCase16InputField();
 exp_result=GetCase16ExpectedResultField();
  
 try
 {
  algo.runImpl(input, output);
 } catch (...) 
 {
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 16 does not work. " << std::endl;
 }
    
 exp_result_vmesh=exp_result->vmesh();
 output_vmesh=output->vmesh(); 
 
 try
 {
  getfieldnodes_algo.run(output,output_nodes);
  getfieldnodes_algo.run(exp_result,exp_result_nodes);
 } catch (...)
 {
  FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 16 does not work (could not get field nodes from input files). " << std::endl;
 }
 
 if(output_nodes->ncols()!=exp_result_nodes->ncols() || output_nodes->nrows()!=exp_result_nodes->nrows() )
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 16 does not work (number of nodes is different than expected). " << std::endl;
 
 for (long idx=0;idx<exp_result_nodes->nrows();idx++)
 {
   EXPECT_NEAR( (*exp_result_nodes)(idx,0),(*output_nodes)(idx,0), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,1),(*output_nodes)(idx,1), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,2),(*output_nodes)(idx,2), epsilon);
 }
 
 output_vmesh->synchronize(Mesh::NODES_E);
 exp_result_vmesh->synchronize(Mesh::NODES_E);
 
 if( output_vmesh->num_elems()!=exp_result_vmesh->num_elems() || output_vmesh->num_elems()!=exp_result_vmesh->num_elems() )
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 16 does not work (number of elements is different than expected). " << std::endl;
 
 for(VMesh::Elem::index_type idx=0; idx<output_vmesh->num_elems(); idx++)
 {
  output_vmesh->get_nodes(onodes1, idx);
  exp_result_vmesh->get_nodes(onodes2, idx);
  for (int j=0;j<4;j++)
   if (onodes1[j]!=onodes2[j])
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 16 does not work (definition of resulting and expected tet definition differs). " << std::endl;    
 } 

  /// CASE 17
 input=GetCase17InputField();
 exp_result=GetCase17ExpectedResultField();
  
 try
 {
  algo.runImpl(input, output);
 } catch (...) 
 {
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 17 does not work. " << std::endl;
 }
 
 exp_result_vmesh=exp_result->vmesh();
 output_vmesh=output->vmesh(); 
 
 try
 {
  getfieldnodes_algo.run(output,output_nodes);
  getfieldnodes_algo.run(exp_result,exp_result_nodes);
 } catch (...)
 {
  FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 17 does not work (could not get field nodes from input files). " << std::endl;
 }
 
 if(output_nodes->ncols()!=exp_result_nodes->ncols() || output_nodes->nrows()!=exp_result_nodes->nrows() )
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 17 does not work (number of nodes is different than expected). " << std::endl;
 
 for (long idx=0;idx<exp_result_nodes->nrows();idx++)
 {
   EXPECT_NEAR( (*exp_result_nodes)(idx,0),(*output_nodes)(idx,0), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,1),(*output_nodes)(idx,1), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,2),(*output_nodes)(idx,2), epsilon);
 }
 
 output_vmesh->synchronize(Mesh::NODES_E);
 exp_result_vmesh->synchronize(Mesh::NODES_E);
 
 if( output_vmesh->num_elems()!=exp_result_vmesh->num_elems() || output_vmesh->num_elems()!=exp_result_vmesh->num_elems() )
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 17 does not work (number of elements is different than expected). " << std::endl;
 
 for(VMesh::Elem::index_type idx=0; idx<output_vmesh->num_elems(); idx++)
 {
  output_vmesh->get_nodes(onodes1, idx);
  exp_result_vmesh->get_nodes(onodes2, idx);
  for (int j=0;j<4;j++)
   if (onodes1[j]!=onodes2[j])
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 17 does not work (definition of resulting and expected tet definition differs). " << std::endl;    
 } 
  
  /// CASE 18
 input=GetCase18InputField();
 exp_result=GetCase18ExpectedResultField();
  
 try
 {
  algo.runImpl(input, output);
 } catch (...) 
 {
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 18 does not work. " << std::endl;
 }
 
 exp_result_vmesh=exp_result->vmesh();
 output_vmesh=output->vmesh(); 
 
 try
 {
  getfieldnodes_algo.run(output,output_nodes);
  getfieldnodes_algo.run(exp_result,exp_result_nodes);
 } catch (...)
 {
  FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 18 does not work (could not get field nodes from input files). " << std::endl;
 }
 
 if(output_nodes->ncols()!=exp_result_nodes->ncols() || output_nodes->nrows()!=exp_result_nodes->nrows() )
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 18 does not work (number of nodes is different than expected). " << std::endl;
 
 for (long idx=0;idx<exp_result_nodes->nrows();idx++)
 {
   EXPECT_NEAR( (*exp_result_nodes)(idx,0),(*output_nodes)(idx,0), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,1),(*output_nodes)(idx,1), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,2),(*output_nodes)(idx,2), epsilon);
 }
 
 output_vmesh->synchronize(Mesh::NODES_E);
 exp_result_vmesh->synchronize(Mesh::NODES_E);
 
 if( output_vmesh->num_elems()!=exp_result_vmesh->num_elems() || output_vmesh->num_elems()!=exp_result_vmesh->num_elems() )
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 18 does not work (number of elements is different than expected). " << std::endl;
 
 for(VMesh::Elem::index_type idx=0; idx<output_vmesh->num_elems(); idx++)
 {
  output_vmesh->get_nodes(onodes1, idx);
  exp_result_vmesh->get_nodes(onodes2, idx);
  for (int j=0;j<4;j++)
   if (onodes1[j]!=onodes2[j])
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 18 does not work (definition of resulting and expected tet definition differs). " << std::endl;    
 } 
 
 
  /// CASE 19
 input=GetCase19InputField();
 exp_result=GetCase19ExpectedResultField();
  
 try
 {
  algo.runImpl(input, output);
 } catch (...) 
 {
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 19 does not work. " << std::endl;
 }
 
 exp_result_vmesh=exp_result->vmesh();
 output_vmesh=output->vmesh(); 
 
 try
 {
  getfieldnodes_algo.run(output,output_nodes);
  getfieldnodes_algo.run(exp_result,exp_result_nodes);
 } catch (...)
 {
  FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 19 does not work (could not get field nodes from input files). " << std::endl;
 }
 
 if(output_nodes->ncols()!=exp_result_nodes->ncols() || output_nodes->nrows()!=exp_result_nodes->nrows() )
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 1 does not work (number of nodes is different than expected). " << std::endl;
 
 for (long idx=0;idx<exp_result_nodes->nrows();idx++)
 {
   EXPECT_NEAR( (*exp_result_nodes)(idx,0),(*output_nodes)(idx,0), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,1),(*output_nodes)(idx,1), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,2),(*output_nodes)(idx,2), epsilon);
 }
 
 output_vmesh->synchronize(Mesh::NODES_E);
 exp_result_vmesh->synchronize(Mesh::NODES_E);
 
 if( output_vmesh->num_elems()!=exp_result_vmesh->num_elems() || output_vmesh->num_elems()!=exp_result_vmesh->num_elems() )
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 19 does not work (number of elements is different than expected). " << std::endl;
 
 for(VMesh::Elem::index_type idx=0; idx<output_vmesh->num_elems(); idx++)
 {
  output_vmesh->get_nodes(onodes1, idx);
  exp_result_vmesh->get_nodes(onodes2, idx);
  for (int j=0;j<4;j++)
   if (onodes1[j]!=onodes2[j])
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 19 does not work (definition of resulting and expected tet definition differs). " << std::endl;    
 } 
 
  
  /// CASE 20
 input=GetCase20InputField();
 exp_result=GetCase20ExpectedResultField();
  
 try
 {
  algo.runImpl(input, output);
 } catch (...) 
 {
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 20 does not work. " << std::endl;
 }
 
 exp_result_vmesh=exp_result->vmesh();
 output_vmesh=output->vmesh(); 
 
 try
 {
  getfieldnodes_algo.run(output,output_nodes);
  getfieldnodes_algo.run(exp_result,exp_result_nodes);
 } catch (...)
 {
  FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 20 does not work (could not get field nodes from input files). " << std::endl;
 }
 
 if(output_nodes->ncols()!=exp_result_nodes->ncols() || output_nodes->nrows()!=exp_result_nodes->nrows() )
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 20 does not work (number of nodes is different than expected). " << std::endl;
 
 for (long idx=0;idx<exp_result_nodes->nrows();idx++)
 {
   EXPECT_NEAR( (*exp_result_nodes)(idx,0),(*output_nodes)(idx,0), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,1),(*output_nodes)(idx,1), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,2),(*output_nodes)(idx,2), epsilon);
 }
 
 output_vmesh->synchronize(Mesh::NODES_E);
 exp_result_vmesh->synchronize(Mesh::NODES_E);
 
 if( output_vmesh->num_elems()!=exp_result_vmesh->num_elems() || output_vmesh->num_elems()!=exp_result_vmesh->num_elems() )
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 20 does not work (number of elements is different than expected). " << std::endl;
 
 for(VMesh::Elem::index_type idx=0; idx<output_vmesh->num_elems(); idx++)
 {
  output_vmesh->get_nodes(onodes1, idx);
  exp_result_vmesh->get_nodes(onodes2, idx);
  for (int j=0;j<4;j++)
   if (onodes1[j]!=onodes2[j])
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 20 does not work (definition of resulting and expected tet definition differs). " << std::endl;    
 } 
 
   /// CASE 21
 input=GetCase21InputField();
 exp_result=GetCase21ExpectedResultField();
  
 try
 {
  algo.runImpl(input, output);
 } catch (...) 
 {
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 21 does not work. " << std::endl;
 }
 
 exp_result_vmesh=exp_result->vmesh();
 output_vmesh=output->vmesh(); 
 
 try
 {
  getfieldnodes_algo.run(output,output_nodes);
  getfieldnodes_algo.run(exp_result,exp_result_nodes);
 } catch (...)
 {
  FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 21 does not work (could not get field nodes from input files). " << std::endl;
 }
 
 if(output_nodes->ncols()!=exp_result_nodes->ncols() || output_nodes->nrows()!=exp_result_nodes->nrows() )
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 21 does not work (number of nodes is different than expected). " << std::endl;
 
 for (long idx=0;idx<exp_result_nodes->nrows();idx++)
 {
   EXPECT_NEAR( (*exp_result_nodes)(idx,0),(*output_nodes)(idx,0), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,1),(*output_nodes)(idx,1), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,2),(*output_nodes)(idx,2), epsilon);
 }
 
 output_vmesh->synchronize(Mesh::NODES_E);
 exp_result_vmesh->synchronize(Mesh::NODES_E);
 
 if( output_vmesh->num_elems()!=exp_result_vmesh->num_elems() || output_vmesh->num_elems()!=exp_result_vmesh->num_elems() )
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 21 does not work (number of elements is different than expected). " << std::endl;
 
 for(VMesh::Elem::index_type idx=0; idx<output_vmesh->num_elems(); idx++)
 {
  output_vmesh->get_nodes(onodes1, idx);
  exp_result_vmesh->get_nodes(onodes2, idx);
  for (int j=0;j<4;j++)
   if (onodes1[j]!=onodes2[j])
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 21 does not work (definition of resulting and expected tet definition differs). " << std::endl;    
 }  
 
 
  /// CASE 22
 input=GetCase22InputField();
 exp_result=GetCase22ExpectedResultField();
  
 try
 {
  algo.runImpl(input, output);
 } catch (...) 
 {
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 22 does not work. " << std::endl;
 }
 
 exp_result_vmesh=exp_result->vmesh();
 output_vmesh=output->vmesh(); 
 
 try
 {
  getfieldnodes_algo.run(output,output_nodes);
  getfieldnodes_algo.run(exp_result,exp_result_nodes);
 } catch (...)
 {
  FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 22 does not work (could not get field nodes from input files). " << std::endl;
 }
 
 if(output_nodes->ncols()!=exp_result_nodes->ncols() || output_nodes->nrows()!=exp_result_nodes->nrows() )
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 22 does not work (number of nodes is different than expected). " << std::endl;
 
 for (long idx=0;idx<exp_result_nodes->nrows();idx++)
 {
   EXPECT_NEAR( (*exp_result_nodes)(idx,0),(*output_nodes)(idx,0), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,1),(*output_nodes)(idx,1), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,2),(*output_nodes)(idx,2), epsilon);
 }
 
 output_vmesh->synchronize(Mesh::NODES_E);
 exp_result_vmesh->synchronize(Mesh::NODES_E);
 
 if( output_vmesh->num_elems()!=exp_result_vmesh->num_elems() || output_vmesh->num_elems()!=exp_result_vmesh->num_elems() )
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 22 does not work (number of elements is different than expected). " << std::endl;
 
 for(VMesh::Elem::index_type idx=0; idx<output_vmesh->num_elems(); idx++)
 {
  output_vmesh->get_nodes(onodes1, idx);
  exp_result_vmesh->get_nodes(onodes2, idx);
  for (int j=0;j<4;j++)
   if (onodes1[j]!=onodes2[j])
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 22 does not work (definition of resulting and expected tet definition differs). " << std::endl;    
 } 
 
 
  /// CASE 23
 input=GetCase23InputField();
 exp_result=GetCase23ExpectedResultField();
    
 try
 {
  algo.runImpl(input, output);
 } catch (...) 
 {
  FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 23 does not work. " << std::endl;
 }
  
 exp_result_vmesh=exp_result->vmesh();
 output_vmesh=output->vmesh(); 
 
 try
 {
  getfieldnodes_algo.run(output,output_nodes);
  getfieldnodes_algo.run(exp_result,exp_result_nodes);
 } catch (...)
 {
  FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 23 does not work (could not get field nodes from input files). " << std::endl;
 }
 
 if(output_nodes->ncols()!=exp_result_nodes->ncols() || output_nodes->nrows()!=exp_result_nodes->nrows() )
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 23 does not work (number of nodes is different than expected). " << std::endl;
 
 for (long idx=0;idx<exp_result_nodes->nrows();idx++)
 {
   EXPECT_NEAR( (*exp_result_nodes)(idx,0),(*output_nodes)(idx,0), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,1),(*output_nodes)(idx,1), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,2),(*output_nodes)(idx,2), epsilon);
 }
 
 output_vmesh->synchronize(Mesh::NODES_E);
 exp_result_vmesh->synchronize(Mesh::NODES_E);
 
 if( output_vmesh->num_elems()!=exp_result_vmesh->num_elems() || output_vmesh->num_elems()!=exp_result_vmesh->num_elems() )
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 23 does not work (number of elements is different than expected). " << std::endl;
 
 for(VMesh::Elem::index_type idx=0; idx<output_vmesh->num_elems(); idx++)
 {
  output_vmesh->get_nodes(onodes1, idx);
  exp_result_vmesh->get_nodes(onodes2, idx);
  for (int j=0;j<4;j++)
   if (onodes1[j]!=onodes2[j])
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 23 does not work (definition of resulting and expected tet definition differs). " << std::endl;    
 } 
 
  /// CASE 24
 input=GetCase24InputField();
 exp_result=GetCase24ExpectedResultField();
  
 try
 {
  algo.runImpl(input, output);
 } catch (...) 
 {
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 24 does not work. " << std::endl;
 }
 
 exp_result_vmesh=exp_result->vmesh();
 output_vmesh=output->vmesh(); 
 
 try
 {
  getfieldnodes_algo.run(output,output_nodes);
  getfieldnodes_algo.run(exp_result,exp_result_nodes);
 } catch (...)
 {
  FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 24 does not work (could not get field nodes from input files). " << std::endl;
 }
 
 if(output_nodes->ncols()!=exp_result_nodes->ncols() || output_nodes->nrows()!=exp_result_nodes->nrows() )
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 24 does not work (number of nodes is different than expected). " << std::endl;
 
 for (long idx=0;idx<exp_result_nodes->nrows();idx++)
 {
   EXPECT_NEAR( (*exp_result_nodes)(idx,0),(*output_nodes)(idx,0), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,1),(*output_nodes)(idx,1), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,2),(*output_nodes)(idx,2), epsilon);
 }
 
 output_vmesh->synchronize(Mesh::NODES_E);
 exp_result_vmesh->synchronize(Mesh::NODES_E);
 
 if( output_vmesh->num_elems()!=exp_result_vmesh->num_elems() || output_vmesh->num_elems()!=exp_result_vmesh->num_elems() )
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 24 does not work (number of elements is different than expected). " << std::endl;
 
 for(VMesh::Elem::index_type idx=0; idx<output_vmesh->num_elems(); idx++)
 {
  output_vmesh->get_nodes(onodes1, idx);
  exp_result_vmesh->get_nodes(onodes2, idx);
  for (int j=0;j<4;j++)
   if (onodes1[j]!=onodes2[j])
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 24 does not work (definition of resulting and expected tet definition differs). " << std::endl;    
 } 
 
 /// CASE 25
 input=GetCase25InputField();
 exp_result=GetCase25ExpectedResultField();
  
 try
 {
  algo.runImpl(input, output);
 } catch (...) 
 {
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 25 does not work. " << std::endl;
 }
    
 exp_result_vmesh=exp_result->vmesh();
 output_vmesh=output->vmesh(); 
 
 try
 {
  getfieldnodes_algo.run(output,output_nodes);
  getfieldnodes_algo.run(exp_result,exp_result_nodes);
 } catch (...)
 {
  FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 25 does not work (could not get field nodes from input files). " << std::endl;
 }
 
 if(output_nodes->ncols()!=exp_result_nodes->ncols() || output_nodes->nrows()!=exp_result_nodes->nrows() )
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 25 does not work (number of nodes is different than expected). " << std::endl;
 
 for (long idx=0;idx<exp_result_nodes->nrows();idx++)
 {
   EXPECT_NEAR( (*exp_result_nodes)(idx,0),(*output_nodes)(idx,0), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,1),(*output_nodes)(idx,1), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,2),(*output_nodes)(idx,2), epsilon);
 }
 
 output_vmesh->synchronize(Mesh::NODES_E);
 exp_result_vmesh->synchronize(Mesh::NODES_E);
 
 if( output_vmesh->num_elems()!=exp_result_vmesh->num_elems() || output_vmesh->num_elems()!=exp_result_vmesh->num_elems() )
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 25 does not work (number of elements is different than expected). " << std::endl;
 
 for(VMesh::Elem::index_type idx=0; idx<output_vmesh->num_elems(); idx++)
 {
  output_vmesh->get_nodes(onodes1, idx);
  exp_result_vmesh->get_nodes(onodes2, idx);
  for (int j=0;j<4;j++)
   if (onodes1[j]!=onodes2[j])
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 25 does not work (definition of resulting and expected tet definition differs). " << std::endl;    
 } 

  /// CASE 26
 input=GetCase26InputField();
 exp_result=GetCase26ExpectedResultField();
  
 try
 {
  algo.runImpl(input, output);
 } catch (...) 
 {
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 26 does not work. " << std::endl;
 }
 
 exp_result_vmesh=exp_result->vmesh();
 output_vmesh=output->vmesh(); 
 
 try
 {
  getfieldnodes_algo.run(output,output_nodes);
  getfieldnodes_algo.run(exp_result,exp_result_nodes);
 } catch (...)
 {
  FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 26 does not work (could not get field nodes from input files). " << std::endl;
 }
 
 if(output_nodes->ncols()!=exp_result_nodes->ncols() || output_nodes->nrows()!=exp_result_nodes->nrows() )
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 26 does not work (number of nodes is different than expected). " << std::endl;
 
 for (long idx=0;idx<exp_result_nodes->nrows();idx++)
 {
   EXPECT_NEAR( (*exp_result_nodes)(idx,0),(*output_nodes)(idx,0), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,1),(*output_nodes)(idx,1), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,2),(*output_nodes)(idx,2), epsilon);
 }
 
 output_vmesh->synchronize(Mesh::NODES_E);
 exp_result_vmesh->synchronize(Mesh::NODES_E);
 
 if( output_vmesh->num_elems()!=exp_result_vmesh->num_elems() || output_vmesh->num_elems()!=exp_result_vmesh->num_elems() )
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 26 does not work (number of elements is different than expected). " << std::endl;
 
 for(VMesh::Elem::index_type idx=0; idx<output_vmesh->num_elems(); idx++)
 {
  output_vmesh->get_nodes(onodes1, idx);
  exp_result_vmesh->get_nodes(onodes2, idx);
  for (int j=0;j<4;j++)
   if (onodes1[j]!=onodes2[j])
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 26 does not work (definition of resulting and expected tet definition differs). " << std::endl;    
 } 
  
  /// CASE 27
 input=GetCase27InputField();
 exp_result=GetCase27ExpectedResultField();
  
 try
 {
  algo.runImpl(input, output);
 } catch (...) 
 {
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 27 does not work. " << std::endl;
 }
 
 exp_result_vmesh=exp_result->vmesh();
 output_vmesh=output->vmesh(); 
 
 try
 {
  getfieldnodes_algo.run(output,output_nodes);
  getfieldnodes_algo.run(exp_result,exp_result_nodes);
 } catch (...)
 {
  FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 27 does not work (could not get field nodes from input files). " << std::endl;
 }
 
 if(output_nodes->ncols()!=exp_result_nodes->ncols() || output_nodes->nrows()!=exp_result_nodes->nrows() )
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 27 does not work (number of nodes is different than expected). " << std::endl;
 
 for (long idx=0;idx<exp_result_nodes->nrows();idx++)
 {
   EXPECT_NEAR( (*exp_result_nodes)(idx,0),(*output_nodes)(idx,0), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,1),(*output_nodes)(idx,1), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,2),(*output_nodes)(idx,2), epsilon);
 }
 
 output_vmesh->synchronize(Mesh::NODES_E);
 exp_result_vmesh->synchronize(Mesh::NODES_E);
 
 if( output_vmesh->num_elems()!=exp_result_vmesh->num_elems() || output_vmesh->num_elems()!=exp_result_vmesh->num_elems() )
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 27 does not work (number of elements is different than expected). " << std::endl;
 
 for(VMesh::Elem::index_type idx=0; idx<output_vmesh->num_elems(); idx++)
 {
  output_vmesh->get_nodes(onodes1, idx);
  exp_result_vmesh->get_nodes(onodes2, idx);
  for (int j=0;j<4;j++)
   if (onodes1[j]!=onodes2[j])
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 27 does not work (definition of resulting and expected tet definition differs). " << std::endl;    
 } 
 
  /// CASE 28
 input=GetCase28InputField();
 exp_result=GetCase28ExpectedResultField();
  
 try
 {
  algo.runImpl(input, output);
 } catch (...) 
 {
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 28 does not work. " << std::endl;
 }
 
 exp_result_vmesh=exp_result->vmesh();
 output_vmesh=output->vmesh(); 
 
 try
 {
  getfieldnodes_algo.run(output,output_nodes);
  getfieldnodes_algo.run(exp_result,exp_result_nodes);
 } catch (...)
 {
  FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 28 does not work (could not get field nodes from input files). " << std::endl;
 }
 
 if(output_nodes->ncols()!=exp_result_nodes->ncols() || output_nodes->nrows()!=exp_result_nodes->nrows() )
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 28 does not work (number of nodes is different than expected). " << std::endl;
 
 for (long idx=0;idx<exp_result_nodes->nrows();idx++)
 {
   EXPECT_NEAR( (*exp_result_nodes)(idx,0),(*output_nodes)(idx,0), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,1),(*output_nodes)(idx,1), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,2),(*output_nodes)(idx,2), epsilon);
 }
 
 output_vmesh->synchronize(Mesh::NODES_E);
 exp_result_vmesh->synchronize(Mesh::NODES_E);
 
 if( output_vmesh->num_elems()!=exp_result_vmesh->num_elems() || output_vmesh->num_elems()!=exp_result_vmesh->num_elems() )
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 28 does not work (number of elements is different than expected). " << std::endl;
 
 for(VMesh::Elem::index_type idx=0; idx<output_vmesh->num_elems(); idx++)
 {
  output_vmesh->get_nodes(onodes1, idx);
  exp_result_vmesh->get_nodes(onodes2, idx);
  for (int j=0;j<4;j++)
   if (onodes1[j]!=onodes2[j])
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 28 does not work (definition of resulting and expected tet definition differs). " << std::endl;    
 } 
   
  /// CASE 29
 input=GetCase29InputField();
 exp_result=GetCase29ExpectedResultField();
  
 try
 {
  algo.runImpl(input, output);
 } catch (...) 
 {
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 29 does not work. " << std::endl;
 }
 
 exp_result_vmesh=exp_result->vmesh();
 output_vmesh=output->vmesh(); 
 
 try
 {
  getfieldnodes_algo.run(output,output_nodes);
  getfieldnodes_algo.run(exp_result,exp_result_nodes);
 } catch (...)
 {
  FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 29 does not work (could not get field nodes from input files). " << std::endl;
 }
 
 if(output_nodes->ncols()!=exp_result_nodes->ncols() || output_nodes->nrows()!=exp_result_nodes->nrows() )
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 29 does not work (number of nodes is different than expected). " << std::endl;
 
 for (long idx=0;idx<exp_result_nodes->nrows();idx++)
 {
   EXPECT_NEAR( (*exp_result_nodes)(idx,0),(*output_nodes)(idx,0), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,1),(*output_nodes)(idx,1), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,2),(*output_nodes)(idx,2), epsilon);
 }
 
 output_vmesh->synchronize(Mesh::NODES_E);
 exp_result_vmesh->synchronize(Mesh::NODES_E);
 
 if( output_vmesh->num_elems()!=exp_result_vmesh->num_elems() || output_vmesh->num_elems()!=exp_result_vmesh->num_elems() )
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 29 does not work (number of elements is different than expected). " << std::endl;
 
 for(VMesh::Elem::index_type idx=0; idx<output_vmesh->num_elems(); idx++)
 {
  output_vmesh->get_nodes(onodes1, idx);
  exp_result_vmesh->get_nodes(onodes2, idx);
  for (int j=0;j<4;j++)
   if (onodes1[j]!=onodes2[j])
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 29 does not work (definition of resulting and expected tet definition differs). " << std::endl;    
 } 
 
 /// CASE 30
 input=GetCase30InputField();
 exp_result=GetCase30ExpectedResultField();
  
 try
 {
  algo.runImpl(input, output);
 } catch (...) 
 {
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 30 does not work. " << std::endl;
 }
 
 exp_result_vmesh=exp_result->vmesh();
 output_vmesh=output->vmesh(); 
 
 try
 {
  getfieldnodes_algo.run(output,output_nodes);
  getfieldnodes_algo.run(exp_result,exp_result_nodes);
 } catch (...)
 {
  FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 30 does not work (could not get field nodes from input files). " << std::endl;
 }
 
 if(output_nodes->ncols()!=exp_result_nodes->ncols() || output_nodes->nrows()!=exp_result_nodes->nrows() )
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 30 does not work (number of nodes is different than expected). " << std::endl;
 
 for (long idx=0;idx<exp_result_nodes->nrows();idx++)
 {
   EXPECT_NEAR( (*exp_result_nodes)(idx,0),(*output_nodes)(idx,0), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,1),(*output_nodes)(idx,1), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,2),(*output_nodes)(idx,2), epsilon);
 }
 
 output_vmesh->synchronize(Mesh::NODES_E);
 exp_result_vmesh->synchronize(Mesh::NODES_E);
 
 if( output_vmesh->num_elems()!=exp_result_vmesh->num_elems() || output_vmesh->num_elems()!=exp_result_vmesh->num_elems() )
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 30 does not work (number of elements is different than expected). " << std::endl;
 
 for(VMesh::Elem::index_type idx=0; idx<output_vmesh->num_elems(); idx++)
 {
  output_vmesh->get_nodes(onodes1, idx);
  exp_result_vmesh->get_nodes(onodes2, idx);
  for (int j=0;j<4;j++)
   if (onodes1[j]!=onodes2[j])
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 30 does not work (definition of resulting and expected tet definition differs). " << std::endl;    
 } 
 
  /// CASE 32
 input=GetCase32InputField();
 exp_result=GetCase32ExpectedResultField();
  
 try
 {
  algo.runImpl(input, output);
 } catch (...) 
 {
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 32 does not work. " << std::endl;
 }
 
 exp_result_vmesh=exp_result->vmesh();
 output_vmesh=output->vmesh(); 
 
 try
 {
  getfieldnodes_algo.run(output,output_nodes);
  getfieldnodes_algo.run(exp_result,exp_result_nodes);
 } catch (...)
 {
  FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 32 does not work (could not get field nodes from input files). " << std::endl;
 }
 
 if(output_nodes->ncols()!=exp_result_nodes->ncols() || output_nodes->nrows()!=exp_result_nodes->nrows() )
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 32 does not work (number of nodes is different than expected). " << std::endl;
 
 for (long idx=0;idx<exp_result_nodes->nrows();idx++)
 {
   EXPECT_NEAR( (*exp_result_nodes)(idx,0),(*output_nodes)(idx,0), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,1),(*output_nodes)(idx,1), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,2),(*output_nodes)(idx,2), epsilon);
 }
 
 output_vmesh->synchronize(Mesh::NODES_E);
 exp_result_vmesh->synchronize(Mesh::NODES_E);
 
 if( output_vmesh->num_elems()!=exp_result_vmesh->num_elems() || output_vmesh->num_elems()!=exp_result_vmesh->num_elems() )
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 32 does not work (number of elements is different than expected). " << std::endl;
 
 for(VMesh::Elem::index_type idx=0; idx<output_vmesh->num_elems(); idx++)
 {
  output_vmesh->get_nodes(onodes1, idx);
  exp_result_vmesh->get_nodes(onodes2, idx);
  for (int j=0;j<4;j++)
   if (onodes1[j]!=onodes2[j])
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 32 does not work (definition of resulting and expected tet definition differs). " << std::endl;    
 } 
 
 
  /// CASE 33
 input=GetCase33InputField();
 exp_result=GetCase33ExpectedResultField();
    
 try
 {
  algo.runImpl(input, output);
 } catch (...) 
 {
  FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 33 does not work. " << std::endl;
 }
  
 exp_result_vmesh=exp_result->vmesh();
 output_vmesh=output->vmesh(); 
 
 try
 {
  getfieldnodes_algo.run(output,output_nodes);
  getfieldnodes_algo.run(exp_result,exp_result_nodes);
 } catch (...)
 {
  FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 33 does not work (could not get field nodes from input files). " << std::endl;
 }
 
 if(output_nodes->ncols()!=exp_result_nodes->ncols() || output_nodes->nrows()!=exp_result_nodes->nrows() )
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 33 does not work (number of nodes is different than expected). " << std::endl;
 
 for (long idx=0;idx<exp_result_nodes->nrows();idx++)
 {
   EXPECT_NEAR( (*exp_result_nodes)(idx,0),(*output_nodes)(idx,0), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,1),(*output_nodes)(idx,1), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,2),(*output_nodes)(idx,2), epsilon);
 }
 
 output_vmesh->synchronize(Mesh::NODES_E);
 exp_result_vmesh->synchronize(Mesh::NODES_E);
 
 if( output_vmesh->num_elems()!=exp_result_vmesh->num_elems() || output_vmesh->num_elems()!=exp_result_vmesh->num_elems() )
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 33 does not work (number of elements is different than expected). " << std::endl;
 
 for(VMesh::Elem::index_type idx=0; idx<output_vmesh->num_elems(); idx++)
 {
  output_vmesh->get_nodes(onodes1, idx);
  exp_result_vmesh->get_nodes(onodes2, idx);
  for (int j=0;j<4;j++)
   if (onodes1[j]!=onodes2[j])
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 33 does not work (definition of resulting and expected tet definition differs). " << std::endl;    
 } 
 
  /// CASE 34
 input=GetCase34InputField();
 exp_result=GetCase34ExpectedResultField();
  
 try
 {
  algo.runImpl(input, output);
 } catch (...) 
 {
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 34 does not work. " << std::endl;
 }
 
 exp_result_vmesh=exp_result->vmesh();
 output_vmesh=output->vmesh(); 
 
 try
 {
  getfieldnodes_algo.run(output,output_nodes);
  getfieldnodes_algo.run(exp_result,exp_result_nodes);
 } catch (...)
 {
  FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 34 does not work (could not get field nodes from input files). " << std::endl;
 }
 
 if(output_nodes->ncols()!=exp_result_nodes->ncols() || output_nodes->nrows()!=exp_result_nodes->nrows() )
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 34 does not work (number of nodes is different than expected). " << std::endl;
 
 for (long idx=0;idx<exp_result_nodes->nrows();idx++)
 {
   EXPECT_NEAR( (*exp_result_nodes)(idx,0),(*output_nodes)(idx,0), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,1),(*output_nodes)(idx,1), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,2),(*output_nodes)(idx,2), epsilon);
 }
 
 output_vmesh->synchronize(Mesh::NODES_E);
 exp_result_vmesh->synchronize(Mesh::NODES_E);
 
 if( output_vmesh->num_elems()!=exp_result_vmesh->num_elems() || output_vmesh->num_elems()!=exp_result_vmesh->num_elems() )
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 34 does not work (number of elements is different than expected). " << std::endl;
 
 for(VMesh::Elem::index_type idx=0; idx<output_vmesh->num_elems(); idx++)
 {
  output_vmesh->get_nodes(onodes1, idx);
  exp_result_vmesh->get_nodes(onodes2, idx);
  for (int j=0;j<4;j++)
   if (onodes1[j]!=onodes2[j])
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 34 does not work (definition of resulting and expected tet definition differs). " << std::endl;    
 } 
 
 /// CASE 35
 input=GetCase35InputField();
 exp_result=GetCase35ExpectedResultField();
  
 try
 {
  algo.runImpl(input, output);
 } catch (...) 
 {
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 35 does not work. " << std::endl;
 }
    
 exp_result_vmesh=exp_result->vmesh();
 output_vmesh=output->vmesh(); 
 
 try
 {
  getfieldnodes_algo.run(output,output_nodes);
  getfieldnodes_algo.run(exp_result,exp_result_nodes);
 } catch (...)
 {
  FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 35 does not work (could not get field nodes from input files). " << std::endl;
 }
 
 if(output_nodes->ncols()!=exp_result_nodes->ncols() || output_nodes->nrows()!=exp_result_nodes->nrows() )
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 35 does not work (number of nodes is different than expected). " << std::endl;
 
 for (long idx=0;idx<exp_result_nodes->nrows();idx++)
 {
   EXPECT_NEAR( (*exp_result_nodes)(idx,0),(*output_nodes)(idx,0), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,1),(*output_nodes)(idx,1), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,2),(*output_nodes)(idx,2), epsilon);
 }
 
 output_vmesh->synchronize(Mesh::NODES_E);
 exp_result_vmesh->synchronize(Mesh::NODES_E);
 
 if( output_vmesh->num_elems()!=exp_result_vmesh->num_elems() || output_vmesh->num_elems()!=exp_result_vmesh->num_elems() )
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 35 does not work (number of elements is different than expected). " << std::endl;
 
 for(VMesh::Elem::index_type idx=0; idx<output_vmesh->num_elems(); idx++)
 {
  output_vmesh->get_nodes(onodes1, idx);
  exp_result_vmesh->get_nodes(onodes2, idx);
  for (int j=0;j<4;j++)
   if (onodes1[j]!=onodes2[j])
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 35 does not work (definition of resulting and expected tet definition differs). " << std::endl;    
 } 

  /// CASE 36
 input=GetCase36InputField();
 exp_result=GetCase36ExpectedResultField();
  
 try
 {
  algo.runImpl(input, output);
 } catch (...) 
 {
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 36 does not work. " << std::endl;
 }
 
 exp_result_vmesh=exp_result->vmesh();
 output_vmesh=output->vmesh(); 
 
 try
 {
  getfieldnodes_algo.run(output,output_nodes);
  getfieldnodes_algo.run(exp_result,exp_result_nodes);
 } catch (...)
 {
  FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 36 does not work (could not get field nodes from input files). " << std::endl;
 }
 
 if(output_nodes->ncols()!=exp_result_nodes->ncols() || output_nodes->nrows()!=exp_result_nodes->nrows() )
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 36 does not work (number of nodes is different than expected). " << std::endl;
 
 for (long idx=0;idx<exp_result_nodes->nrows();idx++)
 {
   EXPECT_NEAR( (*exp_result_nodes)(idx,0),(*output_nodes)(idx,0), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,1),(*output_nodes)(idx,1), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,2),(*output_nodes)(idx,2), epsilon);
 }
 
 output_vmesh->synchronize(Mesh::NODES_E);
 exp_result_vmesh->synchronize(Mesh::NODES_E);
 
 if( output_vmesh->num_elems()!=exp_result_vmesh->num_elems() || output_vmesh->num_elems()!=exp_result_vmesh->num_elems() )
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 36 does not work (number of elements is different than expected). " << std::endl;
 
 for(VMesh::Elem::index_type idx=0; idx<output_vmesh->num_elems(); idx++)
 {
  output_vmesh->get_nodes(onodes1, idx);
  exp_result_vmesh->get_nodes(onodes2, idx);
  for (int j=0;j<4;j++)
   if (onodes1[j]!=onodes2[j])
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 36 does not work (definition of resulting and expected tet definition differs). " << std::endl;    
 } 
  
  /// CASE 37
 input=GetCase37InputField();
 exp_result=GetCase37ExpectedResultField();
  
 try
 {
  algo.runImpl(input, output);
 } catch (...) 
 {
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 37 does not work. " << std::endl;
 }
 
 exp_result_vmesh=exp_result->vmesh();
 output_vmesh=output->vmesh(); 
 
 try
 {
  getfieldnodes_algo.run(output,output_nodes);
  getfieldnodes_algo.run(exp_result,exp_result_nodes);
 } catch (...)
 {
  FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 37 does not work (could not get field nodes from input files). " << std::endl;
 }
 
 if(output_nodes->ncols()!=exp_result_nodes->ncols() || output_nodes->nrows()!=exp_result_nodes->nrows() )
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 37 does not work (number of nodes is different than expected). " << std::endl;
 
 for (long idx=0;idx<exp_result_nodes->nrows();idx++)
 {
   EXPECT_NEAR( (*exp_result_nodes)(idx,0),(*output_nodes)(idx,0), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,1),(*output_nodes)(idx,1), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,2),(*output_nodes)(idx,2), epsilon);
 }
 
 output_vmesh->synchronize(Mesh::NODES_E);
 exp_result_vmesh->synchronize(Mesh::NODES_E);
 
 if( output_vmesh->num_elems()!=exp_result_vmesh->num_elems() || output_vmesh->num_elems()!=exp_result_vmesh->num_elems() )
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 37 does not work (number of elements is different than expected). " << std::endl;
 
 for(VMesh::Elem::index_type idx=0; idx<output_vmesh->num_elems(); idx++)
 {
  output_vmesh->get_nodes(onodes1, idx);
  exp_result_vmesh->get_nodes(onodes2, idx);
  for (int j=0;j<4;j++)
   if (onodes1[j]!=onodes2[j])
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 37 does not work (definition of resulting and expected tet definition differs). " << std::endl;    
 } 
 
 
  /// CASE 38
 input=GetCase38InputField();
 exp_result=GetCase38ExpectedResultField();
  
 try
 {
  algo.runImpl(input, output);
 } catch (...) 
 {
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 38 does not work. " << std::endl;
 }
 
 exp_result_vmesh=exp_result->vmesh();
 output_vmesh=output->vmesh(); 
 
 try
 {
  getfieldnodes_algo.run(output,output_nodes);
  getfieldnodes_algo.run(exp_result,exp_result_nodes);
 } catch (...)
 {
  FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 38 does not work (could not get field nodes from input files). " << std::endl;
 }
 
 if(output_nodes->ncols()!=exp_result_nodes->ncols() || output_nodes->nrows()!=exp_result_nodes->nrows() )
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 38 does not work (number of nodes is different than expected). " << std::endl;
 
 for (long idx=0;idx<exp_result_nodes->nrows();idx++)
 {
   EXPECT_NEAR( (*exp_result_nodes)(idx,0),(*output_nodes)(idx,0), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,1),(*output_nodes)(idx,1), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,2),(*output_nodes)(idx,2), epsilon);
 }
 
 output_vmesh->synchronize(Mesh::NODES_E);
 exp_result_vmesh->synchronize(Mesh::NODES_E);
 
 if( output_vmesh->num_elems()!=exp_result_vmesh->num_elems() || output_vmesh->num_elems()!=exp_result_vmesh->num_elems() )
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 38 does not work (number of elements is different than expected). " << std::endl;
 
 for(VMesh::Elem::index_type idx=0; idx<output_vmesh->num_elems(); idx++)
 {
  output_vmesh->get_nodes(onodes1, idx);
  exp_result_vmesh->get_nodes(onodes2, idx);
  for (int j=0;j<4;j++)
   if (onodes1[j]!=onodes2[j])
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 38 does not work (definition of resulting and expected tet definition differs). " << std::endl;    
 } 
   
  /// CASE 39
 input=GetCase39InputField();
 exp_result=GetCase39ExpectedResultField();
  
 try
 {
  algo.runImpl(input, output);
 } catch (...) 
 {
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 39 does not work. " << std::endl;
 }
 
 exp_result_vmesh=exp_result->vmesh();
 output_vmesh=output->vmesh(); 
 
 try
 {
  getfieldnodes_algo.run(output,output_nodes);
  getfieldnodes_algo.run(exp_result,exp_result_nodes);
 } catch (...)
 {
  FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 39 does not work (could not get field nodes from input files). " << std::endl;
 }
 
 if(output_nodes->ncols()!=exp_result_nodes->ncols() || output_nodes->nrows()!=exp_result_nodes->nrows() )
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 39 does not work (number of nodes is different than expected). " << std::endl;
 
 for (long idx=0;idx<exp_result_nodes->nrows();idx++)
 {
   EXPECT_NEAR( (*exp_result_nodes)(idx,0),(*output_nodes)(idx,0), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,1),(*output_nodes)(idx,1), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,2),(*output_nodes)(idx,2), epsilon);
 }
 
 output_vmesh->synchronize(Mesh::NODES_E);
 exp_result_vmesh->synchronize(Mesh::NODES_E);
 
 if( output_vmesh->num_elems()!=exp_result_vmesh->num_elems() || output_vmesh->num_elems()!=exp_result_vmesh->num_elems() )
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 39 does not work (number of elements is different than expected). " << std::endl;
 
 for(VMesh::Elem::index_type idx=0; idx<output_vmesh->num_elems(); idx++)
 {
  output_vmesh->get_nodes(onodes1, idx);
  exp_result_vmesh->get_nodes(onodes2, idx);
  for (int j=0;j<4;j++)
   if (onodes1[j]!=onodes2[j])
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 39 does not work (definition of resulting and expected tet definition differs). " << std::endl;    
 } 
 
 /// CASE 40
 input=GetCase40InputField();
 exp_result=GetCase40ExpectedResultField();
  
 try
 {
  algo.runImpl(input, output);
 } catch (...) 
 {
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 40 does not work. " << std::endl;
 }
 
 exp_result_vmesh=exp_result->vmesh();
 output_vmesh=output->vmesh(); 
 
 try
 {
  getfieldnodes_algo.run(output,output_nodes);
  getfieldnodes_algo.run(exp_result,exp_result_nodes);
 } catch (...)
 {
  FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 40 does not work (could not get field nodes from input files). " << std::endl;
 }
 
 if(output_nodes->ncols()!=exp_result_nodes->ncols() || output_nodes->nrows()!=exp_result_nodes->nrows() )
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 1 does not work (number of nodes is different than expected). " << std::endl;
 
 for (long idx=0;idx<exp_result_nodes->nrows();idx++)
 {
   EXPECT_NEAR( (*exp_result_nodes)(idx,0),(*output_nodes)(idx,0), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,1),(*output_nodes)(idx,1), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,2),(*output_nodes)(idx,2), epsilon);
 }
 
 output_vmesh->synchronize(Mesh::NODES_E);
 exp_result_vmesh->synchronize(Mesh::NODES_E);
 
 if( output_vmesh->num_elems()!=exp_result_vmesh->num_elems() || output_vmesh->num_elems()!=exp_result_vmesh->num_elems() )
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 40 does not work (number of elements is different than expected). " << std::endl;
 
 for(VMesh::Elem::index_type idx=0; idx<output_vmesh->num_elems(); idx++)
 {
  output_vmesh->get_nodes(onodes1, idx);
  exp_result_vmesh->get_nodes(onodes2, idx);
  for (int j=0;j<4;j++)
   if (onodes1[j]!=onodes2[j])
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 40 does not work (definition of resulting and expected tet definition differs). " << std::endl;    
 } 
 
 /// CASE 41
 input=GetCase41InputField();
 exp_result=GetCase41ExpectedResultField();
  
 try
 {
  algo.runImpl(input, output);
 } catch (...) 
 {
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 41 does not work. " << std::endl;
 }
 
 exp_result_vmesh=exp_result->vmesh();
 output_vmesh=output->vmesh(); 
 
 try
 {
  getfieldnodes_algo.run(output,output_nodes);
  getfieldnodes_algo.run(exp_result,exp_result_nodes);
 } catch (...)
 {
  FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 41 does not work (could not get field nodes from input files). " << std::endl;
 }
 
 if(output_nodes->ncols()!=exp_result_nodes->ncols() || output_nodes->nrows()!=exp_result_nodes->nrows() )
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 41 does not work (number of nodes is different than expected). " << std::endl;
 
 for (long idx=0;idx<exp_result_nodes->nrows();idx++)
 {
   EXPECT_NEAR( (*exp_result_nodes)(idx,0),(*output_nodes)(idx,0), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,1),(*output_nodes)(idx,1), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,2),(*output_nodes)(idx,2), epsilon);
 }
 
 output_vmesh->synchronize(Mesh::NODES_E);
 exp_result_vmesh->synchronize(Mesh::NODES_E);
 
 if( output_vmesh->num_elems()!=exp_result_vmesh->num_elems() || output_vmesh->num_elems()!=exp_result_vmesh->num_elems() )
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 41 does not work (number of elements is different than expected). " << std::endl;
 
 for(VMesh::Elem::index_type idx=0; idx<output_vmesh->num_elems(); idx++)
 {
  output_vmesh->get_nodes(onodes1, idx);
  exp_result_vmesh->get_nodes(onodes2, idx);
  for (int j=0;j<4;j++)
   if (onodes1[j]!=onodes2[j])
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 41 does not work (definition of resulting and expected tet definition differs). " << std::endl;    
 }   
 
  /// CASE 42
 input=GetCase42InputField();
 exp_result=GetCase42ExpectedResultField();
  
 try
 {
  algo.runImpl(input, output);
 } catch (...) 
 {
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 42 does not work. " << std::endl;
 }
 
 exp_result_vmesh=exp_result->vmesh();
 output_vmesh=output->vmesh(); 
 
 try
 {
  getfieldnodes_algo.run(output,output_nodes);
  getfieldnodes_algo.run(exp_result,exp_result_nodes);
 } catch (...)
 {
  FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 42 does not work (could not get field nodes from input files). " << std::endl;
 }
 
 if(output_nodes->ncols()!=exp_result_nodes->ncols() || output_nodes->nrows()!=exp_result_nodes->nrows() )
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 42 does not work (number of nodes is different than expected). " << std::endl;
 
 for (long idx=0;idx<exp_result_nodes->nrows();idx++)
 {
   EXPECT_NEAR( (*exp_result_nodes)(idx,0),(*output_nodes)(idx,0), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,1),(*output_nodes)(idx,1), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,2),(*output_nodes)(idx,2), epsilon);
 }
 
 output_vmesh->synchronize(Mesh::NODES_E);
 exp_result_vmesh->synchronize(Mesh::NODES_E);
 
 if( output_vmesh->num_elems()!=exp_result_vmesh->num_elems() || output_vmesh->num_elems()!=exp_result_vmesh->num_elems() )
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 42 does not work (number of elements is different than expected). " << std::endl;
 
 for(VMesh::Elem::index_type idx=0; idx<output_vmesh->num_elems(); idx++)
 {
  output_vmesh->get_nodes(onodes1, idx);
  exp_result_vmesh->get_nodes(onodes2, idx);
  for (int j=0;j<4;j++)
   if (onodes1[j]!=onodes2[j])
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 42 does not work (definition of resulting and expected tet definition differs). " << std::endl;    
 } 
 
 
  /// CASE 43
 input=GetCase43InputField();
 exp_result=GetCase43ExpectedResultField();
    
 try
 {
  algo.runImpl(input, output);
 } catch (...) 
 {
  FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 43 does not work. " << std::endl;
 }
  
 exp_result_vmesh=exp_result->vmesh();
 output_vmesh=output->vmesh(); 
 
 try
 {
  getfieldnodes_algo.run(output,output_nodes);
  getfieldnodes_algo.run(exp_result,exp_result_nodes);
 } catch (...)
 {
  FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 43 does not work (could not get field nodes from input files). " << std::endl;
 }
 
 if(output_nodes->ncols()!=exp_result_nodes->ncols() || output_nodes->nrows()!=exp_result_nodes->nrows() )
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 43 does not work (number of nodes is different than expected). " << std::endl;
 
 for (long idx=0;idx<exp_result_nodes->nrows();idx++)
 {
   EXPECT_NEAR( (*exp_result_nodes)(idx,0),(*output_nodes)(idx,0), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,1),(*output_nodes)(idx,1), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,2),(*output_nodes)(idx,2), epsilon);
 }
 
 output_vmesh->synchronize(Mesh::NODES_E);
 exp_result_vmesh->synchronize(Mesh::NODES_E);
 
 if( output_vmesh->num_elems()!=exp_result_vmesh->num_elems() || output_vmesh->num_elems()!=exp_result_vmesh->num_elems() )
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 43 does not work (number of elements is different than expected). " << std::endl;
 
 for(VMesh::Elem::index_type idx=0; idx<output_vmesh->num_elems(); idx++)
 {
  output_vmesh->get_nodes(onodes1, idx);
  exp_result_vmesh->get_nodes(onodes2, idx);
  for (int j=0;j<4;j++)
   if (onodes1[j]!=onodes2[j])
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 43 does not work (definition of resulting and expected tet definition differs). " << std::endl;    
 } 
 
  /// CASE 44
 input=GetCase44InputField();
 exp_result=GetCase44ExpectedResultField();
  
 try
 {
  algo.runImpl(input, output);
 } catch (...) 
 {
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 44 does not work. " << std::endl;
 }
 
 exp_result_vmesh=exp_result->vmesh();
 output_vmesh=output->vmesh(); 
 
 try
 {
  getfieldnodes_algo.run(output,output_nodes);
  getfieldnodes_algo.run(exp_result,exp_result_nodes);
 } catch (...)
 {
  FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 44 does not work (could not get field nodes from input files). " << std::endl;
 }
 
 if(output_nodes->ncols()!=exp_result_nodes->ncols() || output_nodes->nrows()!=exp_result_nodes->nrows() )
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 44 does not work (number of nodes is different than expected). " << std::endl;
 
 for (long idx=0;idx<exp_result_nodes->nrows();idx++)
 {
   EXPECT_NEAR( (*exp_result_nodes)(idx,0),(*output_nodes)(idx,0), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,1),(*output_nodes)(idx,1), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,2),(*output_nodes)(idx,2), epsilon);
 }
 
 output_vmesh->synchronize(Mesh::NODES_E);
 exp_result_vmesh->synchronize(Mesh::NODES_E);
 
 if( output_vmesh->num_elems()!=exp_result_vmesh->num_elems() || output_vmesh->num_elems()!=exp_result_vmesh->num_elems() )
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 44 does not work (number of elements is different than expected). " << std::endl;
 
 for(VMesh::Elem::index_type idx=0; idx<output_vmesh->num_elems(); idx++)
 {
  output_vmesh->get_nodes(onodes1, idx);
  exp_result_vmesh->get_nodes(onodes2, idx);
  for (int j=0;j<4;j++)
   if (onodes1[j]!=onodes2[j])
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 44 does not work (definition of resulting and expected tet definition differs). " << std::endl;    
 } 
 
 /// CASE 45
 input=GetCase45InputField();
 exp_result=GetCase45ExpectedResultField();
  
 try
 {
  algo.runImpl(input, output);
 } catch (...) 
 {
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 45 does not work. " << std::endl;
 }
    
 exp_result_vmesh=exp_result->vmesh();
 output_vmesh=output->vmesh(); 
 
 try
 {
  getfieldnodes_algo.run(output,output_nodes);
  getfieldnodes_algo.run(exp_result,exp_result_nodes);
 } catch (...)
 {
  FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 45 does not work (could not get field nodes from input files). " << std::endl;
 }
 
 if(output_nodes->ncols()!=exp_result_nodes->ncols() || output_nodes->nrows()!=exp_result_nodes->nrows() )
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 45 does not work (number of nodes is different than expected). " << std::endl;
 
 for (long idx=0;idx<exp_result_nodes->nrows();idx++)
 {
   EXPECT_NEAR( (*exp_result_nodes)(idx,0),(*output_nodes)(idx,0), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,1),(*output_nodes)(idx,1), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,2),(*output_nodes)(idx,2), epsilon);
 }
 
 output_vmesh->synchronize(Mesh::NODES_E);
 exp_result_vmesh->synchronize(Mesh::NODES_E);
 
 if( output_vmesh->num_elems()!=exp_result_vmesh->num_elems() || output_vmesh->num_elems()!=exp_result_vmesh->num_elems() )
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 45 does not work (number of elements is different than expected). " << std::endl;
 
 for(VMesh::Elem::index_type idx=0; idx<output_vmesh->num_elems(); idx++)
 {
  output_vmesh->get_nodes(onodes1, idx);
  exp_result_vmesh->get_nodes(onodes2, idx);
  for (int j=0;j<4;j++)
   if (onodes1[j]!=onodes2[j])
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 45 does not work (definition of resulting and expected tet definition differs). " << std::endl;    
 } 

  /// CASE 46
 input=GetCase46InputField();
 exp_result=GetCase46ExpectedResultField();
  
 try
 {
  algo.runImpl(input, output);
 } catch (...) 
 {
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 46 does not work. " << std::endl;
 }
 
 exp_result_vmesh=exp_result->vmesh();
 output_vmesh=output->vmesh(); 
 
 try
 {
  getfieldnodes_algo.run(output,output_nodes);
  getfieldnodes_algo.run(exp_result,exp_result_nodes);
 } catch (...)
 {
  FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 46 does not work (could not get field nodes from input files). " << std::endl;
 }
 
 if(output_nodes->ncols()!=exp_result_nodes->ncols() || output_nodes->nrows()!=exp_result_nodes->nrows() )
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 46 does not work (number of nodes is different than expected). " << std::endl;
 
 for (long idx=0;idx<exp_result_nodes->nrows();idx++)
 {
   EXPECT_NEAR( (*exp_result_nodes)(idx,0),(*output_nodes)(idx,0), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,1),(*output_nodes)(idx,1), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,2),(*output_nodes)(idx,2), epsilon);
 }
 
 output_vmesh->synchronize(Mesh::NODES_E);
 exp_result_vmesh->synchronize(Mesh::NODES_E);
 
 if( output_vmesh->num_elems()!=exp_result_vmesh->num_elems() || output_vmesh->num_elems()!=exp_result_vmesh->num_elems() )
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 46 does not work (number of elements is different than expected). " << std::endl;
 
 for(VMesh::Elem::index_type idx=0; idx<output_vmesh->num_elems(); idx++)
 {
  output_vmesh->get_nodes(onodes1, idx);
  exp_result_vmesh->get_nodes(onodes2, idx);
  for (int j=0;j<4;j++)
   if (onodes1[j]!=onodes2[j])
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 46 does not work (definition of resulting and expected tet definition differs). " << std::endl;    
 } 
 
  /// CASE 48
 input=GetCase48InputField();
 exp_result=GetCase48ExpectedResultField();
  
 try
 {
  algo.runImpl(input, output);
 } catch (...) 
 {
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 48 does not work. " << std::endl;
 }
 
 exp_result_vmesh=exp_result->vmesh();
 output_vmesh=output->vmesh(); 
 
 try
 {
  getfieldnodes_algo.run(output,output_nodes);
  getfieldnodes_algo.run(exp_result,exp_result_nodes);
 } catch (...)
 {
  FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 48 does not work (could not get field nodes from input files). " << std::endl;
 }
 
 if(output_nodes->ncols()!=exp_result_nodes->ncols() || output_nodes->nrows()!=exp_result_nodes->nrows() )
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 48 does not work (number of nodes is different than expected). " << std::endl;
 
 for (long idx=0;idx<exp_result_nodes->nrows();idx++)
 {
   EXPECT_NEAR( (*exp_result_nodes)(idx,0),(*output_nodes)(idx,0), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,1),(*output_nodes)(idx,1), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,2),(*output_nodes)(idx,2), epsilon);
 }
 
 output_vmesh->synchronize(Mesh::NODES_E);
 exp_result_vmesh->synchronize(Mesh::NODES_E);
 
 if( output_vmesh->num_elems()!=exp_result_vmesh->num_elems() || output_vmesh->num_elems()!=exp_result_vmesh->num_elems() )
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 48 does not work (number of elements is different than expected). " << std::endl;
 
 for(VMesh::Elem::index_type idx=0; idx<output_vmesh->num_elems(); idx++)
 {
  output_vmesh->get_nodes(onodes1, idx);
  exp_result_vmesh->get_nodes(onodes2, idx);
  for (int j=0;j<4;j++)
   if (onodes1[j]!=onodes2[j])
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 48 does not work (definition of resulting and expected tet definition differs). " << std::endl;    
 } 
   
  /// CASE 49
 input=GetCase49InputField();
 exp_result=GetCase49ExpectedResultField();
  
 try
 {
  algo.runImpl(input, output);
 } catch (...) 
 {
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 49 does not work. " << std::endl;
 }
 
 exp_result_vmesh=exp_result->vmesh();
 output_vmesh=output->vmesh(); 
 
 try
 {
  getfieldnodes_algo.run(output,output_nodes);
  getfieldnodes_algo.run(exp_result,exp_result_nodes);
 } catch (...)
 {
  FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 49 does not work (could not get field nodes from input files). " << std::endl;
 }
 
 if(output_nodes->ncols()!=exp_result_nodes->ncols() || output_nodes->nrows()!=exp_result_nodes->nrows() )
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 49 does not work (number of nodes is different than expected). " << std::endl;
 
 for (long idx=0;idx<exp_result_nodes->nrows();idx++)
 {
   EXPECT_NEAR( (*exp_result_nodes)(idx,0),(*output_nodes)(idx,0), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,1),(*output_nodes)(idx,1), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,2),(*output_nodes)(idx,2), epsilon);
 }
 
 output_vmesh->synchronize(Mesh::NODES_E);
 exp_result_vmesh->synchronize(Mesh::NODES_E);
 
 if( output_vmesh->num_elems()!=exp_result_vmesh->num_elems() || output_vmesh->num_elems()!=exp_result_vmesh->num_elems() )
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 49 does not work (number of elements is different than expected). " << std::endl;
 
 for(VMesh::Elem::index_type idx=0; idx<output_vmesh->num_elems(); idx++)
 {
  output_vmesh->get_nodes(onodes1, idx);
  exp_result_vmesh->get_nodes(onodes2, idx);
  for (int j=0;j<4;j++)
   if (onodes1[j]!=onodes2[j])
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 49 does not work (definition of resulting and expected tet definition differs). " << std::endl;    
 } 
 
 /// CASE 50
 input=GetCase50InputField();
 exp_result=GetCase50ExpectedResultField();
  
 try
 {
  algo.runImpl(input, output);
 } catch (...) 
 {
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 50 does not work. " << std::endl;
 }
 
 exp_result_vmesh=exp_result->vmesh();
 output_vmesh=output->vmesh(); 
 
 try
 {
  getfieldnodes_algo.run(output,output_nodes);
  getfieldnodes_algo.run(exp_result,exp_result_nodes);
 } catch (...)
 {
  FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 50 does not work (could not get field nodes from input files). " << std::endl;
 }
 
 if(output_nodes->ncols()!=exp_result_nodes->ncols() || output_nodes->nrows()!=exp_result_nodes->nrows() )
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 50 does not work (number of nodes is different than expected). " << std::endl;
 
 for (long idx=0;idx<exp_result_nodes->nrows();idx++)
 {
   EXPECT_NEAR( (*exp_result_nodes)(idx,0),(*output_nodes)(idx,0), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,1),(*output_nodes)(idx,1), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,2),(*output_nodes)(idx,2), epsilon);
 }
 
 output_vmesh->synchronize(Mesh::NODES_E);
 exp_result_vmesh->synchronize(Mesh::NODES_E);
 
 if( output_vmesh->num_elems()!=exp_result_vmesh->num_elems() || output_vmesh->num_elems()!=exp_result_vmesh->num_elems() )
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 50 does not work (number of elements is different than expected). " << std::endl;
 
 for(VMesh::Elem::index_type idx=0; idx<output_vmesh->num_elems(); idx++)
 {
  output_vmesh->get_nodes(onodes1, idx);
  exp_result_vmesh->get_nodes(onodes2, idx);
  for (int j=0;j<4;j++)
   if (onodes1[j]!=onodes2[j])
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 50 does not work (definition of resulting and expected tet definition differs). " << std::endl;    
 } 
 
  /// CASE 51
 input=GetCase51InputField();
 exp_result=GetCase51ExpectedResultField();
  
 try
 {
  algo.runImpl(input, output);
 } catch (...) 
 {
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 51 does not work. " << std::endl;
 }
 
 exp_result_vmesh=exp_result->vmesh();
 output_vmesh=output->vmesh(); 
 
 try
 {
  getfieldnodes_algo.run(output,output_nodes);
  getfieldnodes_algo.run(exp_result,exp_result_nodes);
 } catch (...)
 {
  FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 51 does not work (could not get field nodes from input files). " << std::endl;
 }
 
 if(output_nodes->ncols()!=exp_result_nodes->ncols() || output_nodes->nrows()!=exp_result_nodes->nrows() )
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 51 does not work (number of nodes is different than expected). " << std::endl;
 
 for (long idx=0;idx<exp_result_nodes->nrows();idx++)
 {
   EXPECT_NEAR( (*exp_result_nodes)(idx,0),(*output_nodes)(idx,0), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,1),(*output_nodes)(idx,1), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,2),(*output_nodes)(idx,2), epsilon);
 }
 
 output_vmesh->synchronize(Mesh::NODES_E);
 exp_result_vmesh->synchronize(Mesh::NODES_E);
 
 if( output_vmesh->num_elems()!=exp_result_vmesh->num_elems() || output_vmesh->num_elems()!=exp_result_vmesh->num_elems() )
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 51 does not work (number of elements is different than expected). " << std::endl;
 
 for(VMesh::Elem::index_type idx=0; idx<output_vmesh->num_elems(); idx++)
 {
  output_vmesh->get_nodes(onodes1, idx);
  exp_result_vmesh->get_nodes(onodes2, idx);
  for (int j=0;j<4;j++)
   if (onodes1[j]!=onodes2[j])
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 51 does not work (definition of resulting and expected tet definition differs). " << std::endl;    
 }
 
 
 /// CASE 52
 input=GetCase52InputField();
 exp_result=GetCase52ExpectedResultField();
  
 try
 {
  algo.runImpl(input, output);
 } catch (...) 
 {
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 52 does not work. " << std::endl;
 }
 
 exp_result_vmesh=exp_result->vmesh();
 output_vmesh=output->vmesh(); 
 
 try
 {
  getfieldnodes_algo.run(output,output_nodes);
  getfieldnodes_algo.run(exp_result,exp_result_nodes);
 } catch (...)
 {
  FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 52 does not work (could not get field nodes from input files). " << std::endl;
 }
 
 if(output_nodes->ncols()!=exp_result_nodes->ncols() || output_nodes->nrows()!=exp_result_nodes->nrows() )
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 52 does not work (number of nodes is different than expected). " << std::endl;
 
 for (long idx=0;idx<exp_result_nodes->nrows();idx++)
 {
   EXPECT_NEAR( (*exp_result_nodes)(idx,0),(*output_nodes)(idx,0), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,1),(*output_nodes)(idx,1), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,2),(*output_nodes)(idx,2), epsilon);
 }
 
 output_vmesh->synchronize(Mesh::NODES_E);
 exp_result_vmesh->synchronize(Mesh::NODES_E);
 
 if( output_vmesh->num_elems()!=exp_result_vmesh->num_elems() || output_vmesh->num_elems()!=exp_result_vmesh->num_elems() )
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 52 does not work (number of elements is different than expected). " << std::endl;
 
 for(VMesh::Elem::index_type idx=0; idx<output_vmesh->num_elems(); idx++)
 {
  output_vmesh->get_nodes(onodes1, idx);
  exp_result_vmesh->get_nodes(onodes2, idx);
  for (int j=0;j<4;j++)
   if (onodes1[j]!=onodes2[j])
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 52 does not work (definition of resulting and expected tet definition differs). " << std::endl;    
 } 
 
 
 /// CASE 53
 input=GetCase53InputField();
 exp_result=GetCase53ExpectedResultField();
    
 try
 {
  algo.runImpl(input, output);
 } catch (...) 
 {
  FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 53 does not work. " << std::endl;
 }
  
 exp_result_vmesh=exp_result->vmesh();
 output_vmesh=output->vmesh(); 
 
 try
 {
  getfieldnodes_algo.run(output,output_nodes);
  getfieldnodes_algo.run(exp_result,exp_result_nodes);
 } catch (...)
 {
  FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 53 does not work (could not get field nodes from input files). " << std::endl;
 }
 
 if(output_nodes->ncols()!=exp_result_nodes->ncols() || output_nodes->nrows()!=exp_result_nodes->nrows() )
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 53 does not work (number of nodes is different than expected). " << std::endl;
 
 for (long idx=0;idx<exp_result_nodes->nrows();idx++)
 {
   EXPECT_NEAR( (*exp_result_nodes)(idx,0),(*output_nodes)(idx,0), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,1),(*output_nodes)(idx,1), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,2),(*output_nodes)(idx,2), epsilon);
 }
 
 output_vmesh->synchronize(Mesh::NODES_E);
 exp_result_vmesh->synchronize(Mesh::NODES_E);
 
 if( output_vmesh->num_elems()!=exp_result_vmesh->num_elems() || output_vmesh->num_elems()!=exp_result_vmesh->num_elems() )
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 53 does not work (number of elements is different than expected). " << std::endl;
 
 for(VMesh::Elem::index_type idx=0; idx<output_vmesh->num_elems(); idx++)
 {
  output_vmesh->get_nodes(onodes1, idx);
  exp_result_vmesh->get_nodes(onodes2, idx);
  for (int j=0;j<4;j++)
   if (onodes1[j]!=onodes2[j])
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 53 does not work (definition of resulting and expected tet definition differs). " << std::endl;    
 } 
 
 /// CASE 54
 input=GetCase54InputField();
 exp_result=GetCase54ExpectedResultField();
  
 try
 {
  algo.runImpl(input, output);
 } catch (...) 
 {
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 54 does not work. " << std::endl;
 }
 
 exp_result_vmesh=exp_result->vmesh();
 output_vmesh=output->vmesh(); 
 
 try
 {
  getfieldnodes_algo.run(output,output_nodes);
  getfieldnodes_algo.run(exp_result,exp_result_nodes);
 } catch (...)
 {
  FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 54 does not work (could not get field nodes from input files). " << std::endl;
 }
 
 if(output_nodes->ncols()!=exp_result_nodes->ncols() || output_nodes->nrows()!=exp_result_nodes->nrows() )
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 54 does not work (number of nodes is different than expected). " << std::endl;
 
 for (long idx=0;idx<exp_result_nodes->nrows();idx++)
 {
   EXPECT_NEAR( (*exp_result_nodes)(idx,0),(*output_nodes)(idx,0), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,1),(*output_nodes)(idx,1), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,2),(*output_nodes)(idx,2), epsilon);
 }
 
 output_vmesh->synchronize(Mesh::NODES_E);
 exp_result_vmesh->synchronize(Mesh::NODES_E);
 
 if( output_vmesh->num_elems()!=exp_result_vmesh->num_elems() || output_vmesh->num_elems()!=exp_result_vmesh->num_elems() )
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 54 does not work (number of elements is different than expected). " << std::endl;
 
 for(VMesh::Elem::index_type idx=0; idx<output_vmesh->num_elems(); idx++)
 {
  output_vmesh->get_nodes(onodes1, idx);
  exp_result_vmesh->get_nodes(onodes2, idx);
  for (int j=0;j<4;j++)
   if (onodes1[j]!=onodes2[j])
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 54 does not work (definition of resulting and expected tet definition differs). " << std::endl;    
 } 
 
 /// CASE 56
 input=GetCase56InputField();
 exp_result=GetCase56ExpectedResultField();
  
 try
 {
  algo.runImpl(input, output);
 } catch (...) 
 {
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 56 does not work. " << std::endl;
 }
    
 exp_result_vmesh=exp_result->vmesh();
 output_vmesh=output->vmesh(); 
 
 try
 {
  getfieldnodes_algo.run(output,output_nodes);
  getfieldnodes_algo.run(exp_result,exp_result_nodes);
 } catch (...)
 {
  FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 56 does not work (could not get field nodes from input files). " << std::endl;
 }
 
 if(output_nodes->ncols()!=exp_result_nodes->ncols() || output_nodes->nrows()!=exp_result_nodes->nrows() )
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 56 does not work (number of nodes is different than expected). " << std::endl;
 
 for (long idx=0;idx<exp_result_nodes->nrows();idx++)
 {
   EXPECT_NEAR( (*exp_result_nodes)(idx,0),(*output_nodes)(idx,0), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,1),(*output_nodes)(idx,1), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,2),(*output_nodes)(idx,2), epsilon);
 }
 
 output_vmesh->synchronize(Mesh::NODES_E);
 exp_result_vmesh->synchronize(Mesh::NODES_E);
 
 if( output_vmesh->num_elems()!=exp_result_vmesh->num_elems() || output_vmesh->num_elems()!=exp_result_vmesh->num_elems() )
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 56 does not work (number of elements is different than expected). " << std::endl;
 
 for(VMesh::Elem::index_type idx=0; idx<output_vmesh->num_elems(); idx++)
 {
  output_vmesh->get_nodes(onodes1, idx);
  exp_result_vmesh->get_nodes(onodes2, idx);
  for (int j=0;j<4;j++)
   if (onodes1[j]!=onodes2[j])
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 56 does not work (definition of resulting and expected tet definition differs). " << std::endl;    
 } 

  /// CASE 57
 input=GetCase57InputField();
 exp_result=GetCase57ExpectedResultField();
  
 try
 {
  algo.runImpl(input, output);
 } catch (...) 
 {
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 57 does not work. " << std::endl;
 }
 
 exp_result_vmesh=exp_result->vmesh();
 output_vmesh=output->vmesh(); 
 
 try
 {
  getfieldnodes_algo.run(output,output_nodes);
  getfieldnodes_algo.run(exp_result,exp_result_nodes);
 } catch (...)
 {
  FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 57 does not work (could not get field nodes from input files). " << std::endl;
 }
 
 if(output_nodes->ncols()!=exp_result_nodes->ncols() || output_nodes->nrows()!=exp_result_nodes->nrows() )
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 57 does not work (number of nodes is different than expected). " << std::endl;
 
 for (long idx=0;idx<exp_result_nodes->nrows();idx++)
 {
   EXPECT_NEAR( (*exp_result_nodes)(idx,0),(*output_nodes)(idx,0), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,1),(*output_nodes)(idx,1), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,2),(*output_nodes)(idx,2), epsilon);
 }
 
 output_vmesh->synchronize(Mesh::NODES_E);
 exp_result_vmesh->synchronize(Mesh::NODES_E);
 
 if( output_vmesh->num_elems()!=exp_result_vmesh->num_elems() || output_vmesh->num_elems()!=exp_result_vmesh->num_elems() )
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 57 does not work (number of elements is different than expected). " << std::endl;
 
 for(VMesh::Elem::index_type idx=0; idx<output_vmesh->num_elems(); idx++)
 {
  output_vmesh->get_nodes(onodes1, idx);
  exp_result_vmesh->get_nodes(onodes2, idx);
  for (int j=0;j<4;j++)
   if (onodes1[j]!=onodes2[j])
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 57 does not work (definition of resulting and expected tet definition differs). " << std::endl;    
 } 
 
 
  /// CASE 58
 input=GetCase58InputField();
 exp_result=GetCase58ExpectedResultField();
  
 try
 {
  algo.runImpl(input, output);
 } catch (...) 
 {
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 58 does not work. " << std::endl;
 }
 
 exp_result_vmesh=exp_result->vmesh();
 output_vmesh=output->vmesh(); 
 
 try
 {
  getfieldnodes_algo.run(output,output_nodes);
  getfieldnodes_algo.run(exp_result,exp_result_nodes);
 } catch (...)
 {
  FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 58 does not work (could not get field nodes from input files). " << std::endl;
 }
 
 if(output_nodes->ncols()!=exp_result_nodes->ncols() || output_nodes->nrows()!=exp_result_nodes->nrows() )
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 58 does not work (number of nodes is different than expected). " << std::endl;
 
 for (long idx=0;idx<exp_result_nodes->nrows();idx++)
 {
   EXPECT_NEAR( (*exp_result_nodes)(idx,0),(*output_nodes)(idx,0), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,1),(*output_nodes)(idx,1), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,2),(*output_nodes)(idx,2), epsilon);
 }
 
 output_vmesh->synchronize(Mesh::NODES_E);
 exp_result_vmesh->synchronize(Mesh::NODES_E);
 
 if( output_vmesh->num_elems()!=exp_result_vmesh->num_elems() || output_vmesh->num_elems()!=exp_result_vmesh->num_elems() )
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 58 does not work (number of elements is different than expected). " << std::endl;
 
 for(VMesh::Elem::index_type idx=0; idx<output_vmesh->num_elems(); idx++)
 {
  output_vmesh->get_nodes(onodes1, idx);
  exp_result_vmesh->get_nodes(onodes2, idx);
  for (int j=0;j<4;j++)
   if (onodes1[j]!=onodes2[j])
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 58 does not work (definition of resulting and expected tet definition differs). " << std::endl;    
 } 
  
  /// CASE 60
 input=GetCase60InputField();
 exp_result=GetCase60ExpectedResultField();
  
 try
 {
  algo.runImpl(input, output);
 } catch (...) 
 {
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 60 does not work. " << std::endl;
 }
 
 exp_result_vmesh=exp_result->vmesh();
 output_vmesh=output->vmesh(); 
 
 try
 {
  getfieldnodes_algo.run(output,output_nodes);
  getfieldnodes_algo.run(exp_result,exp_result_nodes);
 } catch (...)
 {
  FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 60 does not work (could not get field nodes from input files). " << std::endl;
 }
 
 if(output_nodes->ncols()!=exp_result_nodes->ncols() || output_nodes->nrows()!=exp_result_nodes->nrows() )
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 60 does not work (number of nodes is different than expected). " << std::endl;
 
 for (long idx=0;idx<exp_result_nodes->nrows();idx++)
 {
   EXPECT_NEAR( (*exp_result_nodes)(idx,0),(*output_nodes)(idx,0), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,1),(*output_nodes)(idx,1), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,2),(*output_nodes)(idx,2), epsilon);
 }
 
 output_vmesh->synchronize(Mesh::NODES_E);
 exp_result_vmesh->synchronize(Mesh::NODES_E);
 
 if( output_vmesh->num_elems()!=exp_result_vmesh->num_elems() || output_vmesh->num_elems()!=exp_result_vmesh->num_elems() )
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 60 does not work (number of elements is different than expected). " << std::endl;
 
 for(VMesh::Elem::index_type idx=0; idx<output_vmesh->num_elems(); idx++)
 {
  output_vmesh->get_nodes(onodes1, idx);
  exp_result_vmesh->get_nodes(onodes2, idx);
  for (int j=0;j<4;j++)
   if (onodes1[j]!=onodes2[j])
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 60 does not work (definition of resulting and expected tet definition differs). " << std::endl;    
 } 
 
  /// CASE 63
 input=GetCase63InputField();
 exp_result=GetCase63ExpectedResultField();
  
 try
 {
  algo.runImpl(input, output);
 } catch (...) 
 {
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 63 does not work. " << std::endl;
 }
 
 exp_result_vmesh=exp_result->vmesh();
 output_vmesh=output->vmesh(); 
 
 try
 {
  getfieldnodes_algo.run(output,output_nodes);
  getfieldnodes_algo.run(exp_result,exp_result_nodes);
 } catch (...)
 {
  FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 63 does not work (could not get field nodes from input files). " << std::endl;
 }
 
 if(output_nodes->ncols()!=exp_result_nodes->ncols() || output_nodes->nrows()!=exp_result_nodes->nrows() )
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 63 does not work (number of nodes is different than expected). " << std::endl;
 
 for (long idx=0;idx<exp_result_nodes->nrows();idx++)
 {
   EXPECT_NEAR( (*exp_result_nodes)(idx,0),(*output_nodes)(idx,0), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,1),(*output_nodes)(idx,1), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,2),(*output_nodes)(idx,2), epsilon);
 }
 
 output_vmesh->synchronize(Mesh::NODES_E);
 exp_result_vmesh->synchronize(Mesh::NODES_E);
 
 if( output_vmesh->num_elems()!=exp_result_vmesh->num_elems() || output_vmesh->num_elems()!=exp_result_vmesh->num_elems() )
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 63 does not work (number of elements is different than expected). " << std::endl;
 
 for(VMesh::Elem::index_type idx=0; idx<output_vmesh->num_elems(); idx++)
 {
  output_vmesh->get_nodes(onodes1, idx);
  exp_result_vmesh->get_nodes(onodes2, idx);
  for (int j=0;j<4;j++)
   if (onodes1[j]!=onodes2[j])
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 63 does not work (definition of resulting and expected tet definition differs). " << std::endl;    
 } 
}


TEST(RefineTetMeshLocallyAlgoTests, Test5MoreTheoreticalCutCases)
{
 double epsilon=1e-8;
 RefineTetMeshLocallyAlgorithm algo; 
 SparseRowMatrixFromMap::Values case31_val, case47_val, case55_val, case59_val, case61_val, case62_val;
 SparseRowMatrixHandle case_;
 FieldHandle input=GetCase63InputField(),Casefld, case_exp_result; /// use a random tet input to check theoretical cases that can only be achieved by splitting tet neighbours
 VMesh* input_vmesh=input->vmesh();
 DenseMatrixHandle output_nodes,exp_result_nodes;
 GetMeshNodesAlgo getfieldnodes_algo;
 VMesh *case_vmesh, *case_exp_vmesh;
 VMesh::Node::array_type onodes1(4),onodes2(4);

 /// CASE 31
 case31_val[1][2]=1;
 case31_val[0][2]=1;
 case31_val[0][3]=1;
 case31_val[1][3]=1;
 case31_val[2][3]=1;
 case_ = SparseRowMatrixFromMap::make(input_vmesh->num_nodes(), input_vmesh->num_nodes(), case31_val);
 Casefld=algo.RefineMesh(input, case_); 
 case_exp_result=GetCase31ExpectedResultField();
 case_vmesh=Casefld->vmesh(); 
 case_exp_vmesh=case_exp_result->vmesh();

 try
 {
  getfieldnodes_algo.run(Casefld,output_nodes);
  getfieldnodes_algo.run(case_exp_result,exp_result_nodes);
 } catch (...)
 {
  FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 31 does not work (could not get field nodes from input files). " << std::endl;
 }
 
 if(output_nodes->ncols()!=exp_result_nodes->ncols() || output_nodes->nrows()!=exp_result_nodes->nrows() )
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 63 does not work (number of nodes is different than expected). " << std::endl;
 
 for (long idx=0;idx<exp_result_nodes->nrows();idx++)
 {
   EXPECT_NEAR( (*exp_result_nodes)(idx,0),(*output_nodes)(idx,0), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,1),(*output_nodes)(idx,1), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,2),(*output_nodes)(idx,2), epsilon);
 }
 
 case_vmesh->synchronize(Mesh::NODES_E);
 case_exp_vmesh->synchronize(Mesh::NODES_E);
 
 if( case_vmesh->num_elems()!=case_exp_vmesh->num_elems() || case_vmesh->num_elems()!=case_exp_vmesh->num_elems() )
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 63 does not work (number of elements is different than expected). " << std::endl;
 
 for(VMesh::Elem::index_type idx=0; idx<case_vmesh->num_elems(); idx++)
 {
  case_vmesh->get_nodes(onodes1, idx);
  case_exp_vmesh->get_nodes(onodes2, idx);
  for (int j=0;j<4;j++)
   if (onodes1[j]!=onodes2[j])
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 63 does not work (definition of resulting and expected tet definition differs). " << std::endl;    
 }  
 
 /// CASE 47
 case47_val[0][1]=1;
 case47_val[0][2]=1;
 case47_val[0][3]=1;
 case47_val[1][3]=1;
 case47_val[2][3]=1;
 case_ = SparseRowMatrixFromMap::make(input_vmesh->num_nodes(), input_vmesh->num_nodes(), case47_val);
 Casefld=algo.RefineMesh(input, case_); 
 case_exp_result=GetCase47ExpectedResultField();
 case_vmesh=Casefld->vmesh(); 
 case_exp_vmesh=case_exp_result->vmesh();

 try
 {
  getfieldnodes_algo.run(Casefld,output_nodes);
  getfieldnodes_algo.run(case_exp_result,exp_result_nodes);
 } catch (...)
 {
  FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 31 does not work (could not get field nodes from input files). " << std::endl;
 }
 
 if(output_nodes->ncols()!=exp_result_nodes->ncols() || output_nodes->nrows()!=exp_result_nodes->nrows() )
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 63 does not work (number of nodes is different than expected). " << std::endl;
 
 for (long idx=0;idx<exp_result_nodes->nrows();idx++)
 {
   EXPECT_NEAR( (*exp_result_nodes)(idx,0),(*output_nodes)(idx,0), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,1),(*output_nodes)(idx,1), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,2),(*output_nodes)(idx,2), epsilon);
 }
 
 case_vmesh->synchronize(Mesh::NODES_E);
 case_exp_vmesh->synchronize(Mesh::NODES_E);
 
 if( case_vmesh->num_elems()!=case_exp_vmesh->num_elems() || case_vmesh->num_elems()!=case_exp_vmesh->num_elems() )
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 63 does not work (number of elements is different than expected). " << std::endl;
 
 for(VMesh::Elem::index_type idx=0; idx<case_vmesh->num_elems(); idx++)
 {
  case_vmesh->get_nodes(onodes1, idx);
  case_exp_vmesh->get_nodes(onodes2, idx);
  for (int j=0;j<4;j++)
   if (onodes1[j]!=onodes2[j])
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 63 does not work (definition of resulting and expected tet definition differs). " << std::endl;    
 } 
 
 /// CASE 55
 case55_val[0][1]=1;
 case55_val[1][2]=1;
 case55_val[0][3]=1;
 case55_val[1][3]=1;
 case55_val[2][3]=1;
 case_ = SparseRowMatrixFromMap::make(input_vmesh->num_nodes(), input_vmesh->num_nodes(), case55_val);
 Casefld=algo.RefineMesh(input, case_); 
 case_exp_result=GetCase55ExpectedResultField();
 case_vmesh=Casefld->vmesh(); 
 case_exp_vmesh=case_exp_result->vmesh();

 try
 {
  getfieldnodes_algo.run(Casefld,output_nodes);
  getfieldnodes_algo.run(case_exp_result,exp_result_nodes);
 } catch (...)
 {
  FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 31 does not work (could not get field nodes from input files). " << std::endl;
 }
 
 if(output_nodes->ncols()!=exp_result_nodes->ncols() || output_nodes->nrows()!=exp_result_nodes->nrows() )
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 63 does not work (number of nodes is different than expected). " << std::endl;
 
 for (long idx=0;idx<exp_result_nodes->nrows();idx++)
 {
   EXPECT_NEAR( (*exp_result_nodes)(idx,0),(*output_nodes)(idx,0), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,1),(*output_nodes)(idx,1), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,2),(*output_nodes)(idx,2), epsilon);
 }
 
 case_vmesh->synchronize(Mesh::NODES_E);
 case_exp_vmesh->synchronize(Mesh::NODES_E);
 
 if( case_vmesh->num_elems()!=case_exp_vmesh->num_elems() || case_vmesh->num_elems()!=case_exp_vmesh->num_elems() )
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 63 does not work (number of elements is different than expected). " << std::endl;
 
 for(VMesh::Elem::index_type idx=0; idx<case_vmesh->num_elems(); idx++)
 {
  case_vmesh->get_nodes(onodes1, idx);
  case_exp_vmesh->get_nodes(onodes2, idx);
  for (int j=0;j<4;j++)
   if (onodes1[j]!=onodes2[j])
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 63 does not work (definition of resulting and expected tet definition differs). " << std::endl;    
 } 
 
 /// CASE 59
 case59_val[0][1]=1;
 case59_val[1][2]=1;
 case59_val[0][2]=1;
 case59_val[1][3]=1;
 case59_val[2][3]=1;
 case_ = SparseRowMatrixFromMap::make(input_vmesh->num_nodes(), input_vmesh->num_nodes(), case59_val);
 Casefld=algo.RefineMesh(input, case_); 
 case_exp_result=GetCase59ExpectedResultField();
 case_vmesh=Casefld->vmesh(); 
 case_exp_vmesh=case_exp_result->vmesh();

 try
 {
  getfieldnodes_algo.run(Casefld,output_nodes);
  getfieldnodes_algo.run(case_exp_result,exp_result_nodes);
 } catch (...)
 {
  FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 31 does not work (could not get field nodes from input files). " << std::endl;
 }
 
 if(output_nodes->ncols()!=exp_result_nodes->ncols() || output_nodes->nrows()!=exp_result_nodes->nrows() )
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 63 does not work (number of nodes is different than expected). " << std::endl;
 
 for (long idx=0;idx<exp_result_nodes->nrows();idx++)
 {
   EXPECT_NEAR( (*exp_result_nodes)(idx,0),(*output_nodes)(idx,0), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,1),(*output_nodes)(idx,1), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,2),(*output_nodes)(idx,2), epsilon);
 }
 
 case_vmesh->synchronize(Mesh::NODES_E);
 case_exp_vmesh->synchronize(Mesh::NODES_E);
 
 if( case_vmesh->num_elems()!=case_exp_vmesh->num_elems() || case_vmesh->num_elems()!=case_exp_vmesh->num_elems() )
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 63 does not work (number of elements is different than expected). " << std::endl;
 
 for(VMesh::Elem::index_type idx=0; idx<case_vmesh->num_elems(); idx++)
 {
  case_vmesh->get_nodes(onodes1, idx);
  case_exp_vmesh->get_nodes(onodes2, idx);
  for (int j=0;j<4;j++)
   if (onodes1[j]!=onodes2[j])
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 63 does not work (definition of resulting and expected tet definition differs). " << std::endl;    
 }  
 
 /// CASE 61
 case61_val[0][1]=1;
 case61_val[1][2]=1;
 case61_val[0][2]=1;
 case61_val[0][3]=1;
 case61_val[2][3]=1;
 case_ = SparseRowMatrixFromMap::make(input_vmesh->num_nodes(), input_vmesh->num_nodes(), case61_val);
 Casefld=algo.RefineMesh(input, case_); 
 case_exp_result=GetCase61ExpectedResultField();
 case_vmesh=Casefld->vmesh(); 
 case_exp_vmesh=case_exp_result->vmesh();

 try
 {
  getfieldnodes_algo.run(Casefld,output_nodes);
  getfieldnodes_algo.run(case_exp_result,exp_result_nodes);
 } catch (...)
 {
  FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 31 does not work (could not get field nodes from input files). " << std::endl;
 }
 
 if(output_nodes->ncols()!=exp_result_nodes->ncols() || output_nodes->nrows()!=exp_result_nodes->nrows() )
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 63 does not work (number of nodes is different than expected). " << std::endl;
 
 for (long idx=0;idx<exp_result_nodes->nrows();idx++)
 {
   EXPECT_NEAR( (*exp_result_nodes)(idx,0),(*output_nodes)(idx,0), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,1),(*output_nodes)(idx,1), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,2),(*output_nodes)(idx,2), epsilon);
 }
 
 case_vmesh->synchronize(Mesh::NODES_E);
 case_exp_vmesh->synchronize(Mesh::NODES_E);
 
 if( case_vmesh->num_elems()!=case_exp_vmesh->num_elems() || case_vmesh->num_elems()!=case_exp_vmesh->num_elems() )
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 63 does not work (number of elements is different than expected). " << std::endl;
 
 for(VMesh::Elem::index_type idx=0; idx<case_vmesh->num_elems(); idx++)
 {
  case_vmesh->get_nodes(onodes1, idx);
  case_exp_vmesh->get_nodes(onodes2, idx);
  for (int j=0;j<4;j++)
   if (onodes1[j]!=onodes2[j])
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 63 does not work (definition of resulting and expected tet definition differs). " << std::endl;    
 }  
 
  /// CASE 62
 case62_val[0][1]=1;
 case62_val[1][2]=1;
 case62_val[0][2]=1;
 case62_val[0][3]=1;
 case62_val[1][3]=1;
 case_ = SparseRowMatrixFromMap::make(input_vmesh->num_nodes(), input_vmesh->num_nodes(), case62_val);
 Casefld=algo.RefineMesh(input, case_); 
 case_exp_result=GetCase62ExpectedResultField();
 case_vmesh=Casefld->vmesh(); 
 case_exp_vmesh=case_exp_result->vmesh();

 try
 {
  getfieldnodes_algo.run(Casefld,output_nodes);
  getfieldnodes_algo.run(case_exp_result,exp_result_nodes);
 } catch (...)
 {
  FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 31 does not work (could not get field nodes from input files). " << std::endl;
 }
 
 if(output_nodes->ncols()!=exp_result_nodes->ncols() || output_nodes->nrows()!=exp_result_nodes->nrows() )
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 63 does not work (number of nodes is different than expected). " << std::endl;
 
 for (long idx=0;idx<exp_result_nodes->nrows();idx++)
 {
   EXPECT_NEAR( (*exp_result_nodes)(idx,0),(*output_nodes)(idx,0), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,1),(*output_nodes)(idx,1), epsilon);
   EXPECT_NEAR( (*exp_result_nodes)(idx,2),(*output_nodes)(idx,2), epsilon);
 }
 
 case_vmesh->synchronize(Mesh::NODES_E);
 case_exp_vmesh->synchronize(Mesh::NODES_E);
 
 if( case_vmesh->num_elems()!=case_exp_vmesh->num_elems() || case_vmesh->num_elems()!=case_exp_vmesh->num_elems() )
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 63 does not work (number of elements is different than expected). " << std::endl;
 
 for(VMesh::Elem::index_type idx=0; idx<case_vmesh->num_elems(); idx++)
 {
  case_vmesh->get_nodes(onodes1, idx);
  case_exp_vmesh->get_nodes(onodes2, idx);
  for (int j=0;j<4;j++)
   if (onodes1[j]!=onodes2[j])
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 63 does not work (definition of resulting and expected tet definition differs). " << std::endl;    
 }  
 
}
