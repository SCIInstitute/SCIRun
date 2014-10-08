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

#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Algorithms/Field/InterfaceWithCleaverAlgorithm.h>
#include <Testing/Utils/SCIRunUnitTests.h>
#include <Core/Datatypes/DenseMatrix.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::TestUtils;

  
  FieldHandle SphereSignedDistanceField(bool negative)
  {  
  
   FieldInformation f("LatVolMesh", 1, "float");
   size_type size = 5;
   Point minb(-2.0, -2.0, -2.0);
   Point maxb(2.0, 2.0, 2.0);
   MeshHandle omesh = CreateMesh(f, size, size, size, minb, maxb);
   FieldHandle ofield = CreateField(f,omesh);
   std::vector<double> values(125);
   values[0]=0.9641;values[1]=0.5;values[2]=0.32843;values[3]=0.5;values[4]=0.9641;values[5]=0.5;
   values[6]=-0.05051;values[7]=-0.26393;values[8]=-0.05051;values[9]=0.5;values[10]=0.32843;
   values[11]=-0.26393;values[12]=-0.5;values[13]=-0.26393;values[14]=0.32843;values[15]=0.5;
   values[16]=-0.05051;values[17]=-0.26393;values[18]=-0.05051;values[19]=0.5;values[20]=0.9641;
   values[21]=0.5;values[22]=0.32843;values[23]=0.5;values[24]=0.9641;values[25]=0.5;
   values[26]=-0.05051;values[27]=-0.26393;values[28]=-0.05051;values[29]=0.5;
   values[30]=-0.05051;values[31]=-0.76795;values[32]=-1.0858;values[33]=-0.76795;
   values[34]=-0.05051;values[35]=-0.26393;values[36]=-1.0858;values[37]=-1.5;
   values[38]=-1.0858;values[39]=-0.26393;values[40]=-0.05051;values[41]=-0.76795;
   values[42]=-1.0858;values[43]=-0.76795;values[44]=-0.05051;values[45]=0.5;values[46]=-0.05051;
   values[47]=-0.26393;values[48]=-0.05051;values[49]=0.5;values[50]=0.32843;values[51]=-0.26393;
   values[52]=-0.5;values[53]=-0.26393;values[54]=0.32843;values[55]=-0.26393;values[56]=-1.0858;
   values[57]=-1.5;values[58]=-1.0858;values[59]=-0.26393;values[60]=-0.5;values[61]=-1.5;
   values[62]=-2.5;values[63]=-1.5;values[64]=-0.5;values[65]=-0.26393;values[66]=-1.0858;
   values[67]=-1.5;values[68]=-1.0858;values[69]=-0.26393;values[70]=0.32843;values[71]=-0.26393;
   values[72]=-0.5;values[73]=-0.26393;values[74]=0.32843;values[75]=0.5;values[76]=-0.05051;
   values[77]=-0.26393;values[78]=-0.05051;values[79]=0.5;values[80]=-0.05051;values[81]=-0.76795;
   values[82]=-1.0858;values[83]=-0.76795;values[84]=-0.05051;values[85]=-0.26393;values[86]=-1.0858;
   values[87]=-1.5;values[88]=-1.0858;values[89]=-0.26393;values[90]=-0.05051;values[91]=-0.76795;
   values[92]=-1.0858;values[93]=-0.76795;values[94]=-0.05051;values[95]=0.5;values[96]=-0.05051;
   values[97]=-0.26393;values[98]=-0.05051;values[99]=0.5;values[100]=0.9641;values[101]=0.5;
   values[102]=0.32843;values[103]=0.5;values[104]=0.9641;values[105]=0.5;values[106]=-0.05051;
   values[107]=-0.26393;values[108]=-0.05051;values[109]=0.5;values[110]=0.32843;values[111]=-0.26393;
   values[112]=-0.5;values[113]=-0.26393;values[114]=0.32843;values[115]=0.5;values[116]=-0.05051;
   values[117]=-0.26393;values[118]=-0.05051;values[119]=0.5;values[120]=0.9641;values[121]=0.5;
   values[122]=0.32843;values[123]=0.5;values[124]=0.9641;  
   
   if (negative)
   {
    for (int i=0;i<125;i++)
     {
       values[i]=-values[i];
     }
   }
   
   ofield->vfield()->resize_values();
   ofield->vfield()->set_values(values);
   return ofield;
  }

TEST(CleaverInterfaceTest, SphereSignedDistanceFieldMatrix1)
{
  InterfaceWithCleaverAlgorithm algo;
  FAIL() << "todo";
 
/* auto info = algo.run(SphereSignedDistanceField(false),SphereSignedDistanceField(true));
 
 std::cout << "Number of mesh elements: " <<  info->vmesh()->num_elems() << std::endl;
 std::cout << "Number of mesh nodes: " <<  info->vmesh()->num_nodes() << std::endl;
 std::cout << "Number of mesh values: " <<  info->vfield()->num_values() << std::endl;

 ASSERT_TRUE(info->vmesh()->num_elems() != 98650);
 ASSERT_TRUE(info->vmesh()->num_nodes() != 18367);
 ASSERT_TRUE(info->vfield()->num_values() != 98650);
 */
}
