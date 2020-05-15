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

   Author:              Moritz Dannhauer
   Last Modification:   September 1 2014
*/


#include <gtest/gtest.h>
#include <Core/Algorithms/Field/Tests/LoadFieldsForAlgoCoreTests.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Algorithms/Legacy/Fields/MeshDerivatives/SplitByConnectedRegion.h>

using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Algorithms::Fields;

TEST(SplitByConnectedRegionTest, SplitFieldByConnectedRegionAlgoTetTests)
{
  SplitFieldByConnectedRegionAlgo algo;

  algo.set(SplitFieldByConnectedRegionAlgo::SortDomainBySize(), false);
  algo.set(SplitFieldByConnectedRegionAlgo::SortAscending(), false);

  FieldHandle tetmesh  = SplitFieldByConnectedRegionModuleTetTests();

  std::vector<FieldHandle> result = algo.run(tetmesh);

  EXPECT_EQ(result.size(), 126);

  FieldHandle result1=result[0];
  FieldHandle result2=result[1];
  FieldHandle result3=result[2];
  FieldHandle result4=result[3];
  FieldHandle result5=result[4];
  FieldHandle result6=result[5];
  FieldHandle result7=result[6];
  FieldHandle result8=result[7];

  EXPECT_EQ(result1->vmesh()->num_elems(), 7660);
  EXPECT_EQ(result1->vmesh()->num_nodes(), 1862);

  EXPECT_EQ(result2->vmesh()->num_elems(), 7141);
  EXPECT_EQ(result2->vmesh()->num_nodes(), 1760);

  EXPECT_EQ(result3->vmesh()->num_elems(), 7539);
  EXPECT_EQ(result3->vmesh()->num_nodes(), 1808);

  EXPECT_EQ(result4->vmesh()->num_elems(), 7560);
  EXPECT_EQ(result4->vmesh()->num_nodes(), 1810);

  EXPECT_EQ(result5->vmesh()->num_elems(), 7834);
  EXPECT_EQ(result5->vmesh()->num_nodes(), 1877);

  EXPECT_EQ(result6->vmesh()->num_elems(), 7506);
  EXPECT_EQ(result6->vmesh()->num_nodes(), 1805);

  EXPECT_EQ(result7->vmesh()->num_elems(), 7824);
  EXPECT_EQ(result7->vmesh()->num_nodes(), 1846);

  EXPECT_EQ(result8->vmesh()->num_elems(), 7707);
  EXPECT_EQ(result8->vmesh()->num_nodes(), 1805);

}

TEST(SplitByConnectedRegionTest, DISABLED_SplitFieldByConnectedRegionAlgoTetTests2)
{
  SplitFieldByConnectedRegionAlgo algo;

  FieldHandle tetmesh  = SplitFieldByConnectedRegionModuleTetTests();

  algo.set(SplitFieldByConnectedRegionAlgo::SortDomainBySize(), true);
  algo.set(SplitFieldByConnectedRegionAlgo::SortAscending(), false);

  std::vector<FieldHandle> result = algo.run(tetmesh);

  EXPECT_EQ(result.size(), 126);

  FieldHandle result1=result[0];
  FieldHandle result2=result[1];
  FieldHandle result3=result[2];
  FieldHandle result4=result[3];
  FieldHandle result5=result[4];
  FieldHandle result6=result[5];
  FieldHandle result7=result[6];
  FieldHandle result8=result[7];

  EXPECT_EQ(result1->vmesh()->num_elems(), 8054);
  EXPECT_EQ(result1->vmesh()->num_nodes(), 1866);

  EXPECT_EQ(result2->vmesh()->num_elems(), 7888);
  EXPECT_EQ(result2->vmesh()->num_nodes(), 1851);

  EXPECT_EQ(result3->vmesh()->num_elems(), 7714);
  EXPECT_EQ(result3->vmesh()->num_nodes(), 1862);

  EXPECT_EQ(result4->vmesh()->num_elems(), 7880);
  EXPECT_EQ(result4->vmesh()->num_nodes(), 1878);

  EXPECT_EQ(result5->vmesh()->num_elems(), 7824);
  EXPECT_EQ(result5->vmesh()->num_nodes(), 1846);

  EXPECT_EQ(result6->vmesh()->num_elems(), 7613);
  EXPECT_EQ(result6->vmesh()->num_nodes(), 1737);

  EXPECT_EQ(result7->vmesh()->num_elems(), 7695);
  EXPECT_EQ(result7->vmesh()->num_nodes(), 1845);

  EXPECT_EQ(result8->vmesh()->num_elems(), 7800);
  EXPECT_EQ(result8->vmesh()->num_nodes(), 1828);
}

TEST(SplitByConnectedRegionTest, DISABLED_SplitFieldByConnectedRegionAlgoTetTests3)
{
  SplitFieldByConnectedRegionAlgo algo;

  FieldHandle tetmesh  = SplitFieldByConnectedRegionModuleTetTests();

  algo.set(SplitFieldByConnectedRegionAlgo::SortDomainBySize(), true);
  algo.set(SplitFieldByConnectedRegionAlgo::SortAscending(), true);

  std::vector<FieldHandle> result = algo.run(tetmesh);

  EXPECT_EQ(result.size(), 126);

  FieldHandle result1=result[0];
  FieldHandle result2=result[1];
  FieldHandle result3=result[2];
  FieldHandle result4=result[3];
  FieldHandle result5=result[4];
  FieldHandle result6=result[5];
  FieldHandle result7=result[6];
  FieldHandle result8=result[7];

  EXPECT_EQ(result1->vmesh()->num_elems(), 7259);
  EXPECT_EQ(result1->vmesh()->num_nodes(), 1778);

  EXPECT_EQ(result2->vmesh()->num_elems(), 7205);
  EXPECT_EQ(result2->vmesh()->num_nodes(), 1724);

  EXPECT_EQ(result3->vmesh()->num_elems(), 7371);
  EXPECT_EQ(result3->vmesh()->num_nodes(), 1762);

  EXPECT_EQ(result4->vmesh()->num_elems(), 7322);
  EXPECT_EQ(result4->vmesh()->num_nodes(), 1689);

  EXPECT_EQ(result5->vmesh()->num_elems(), 7457);
  EXPECT_EQ(result5->vmesh()->num_nodes(), 1766);

  EXPECT_EQ(result6->vmesh()->num_elems(), 7372);
  EXPECT_EQ(result6->vmesh()->num_nodes(), 1723);

  EXPECT_EQ(result7->vmesh()->num_elems(), 7506);
  EXPECT_EQ(result7->vmesh()->num_nodes(), 1776);

  EXPECT_EQ(result8->vmesh()->num_elems(), 7403);
  EXPECT_EQ(result8->vmesh()->num_nodes(), 1739);

}

TEST(SplitByConnectedRegionTest, SplitFieldByConnectedRegionAlgoTetTests4)
{
  SplitFieldByConnectedRegionAlgo algo;

  FieldHandle tetmesh  = SplitFieldByConnectedRegionModuleTetTests();

  algo.set(SplitFieldByConnectedRegionAlgo::SortDomainBySize(), false);
  algo.set(SplitFieldByConnectedRegionAlgo::SortAscending(), true);

  std::vector<FieldHandle> result = algo.run(tetmesh);

  EXPECT_EQ(result.size(), 126);

  FieldHandle result1=result[0];
  FieldHandle result2=result[1];
  FieldHandle result3=result[2];
  FieldHandle result4=result[3];
  FieldHandle result5=result[4];
  FieldHandle result6=result[5];
  FieldHandle result7=result[6];
  FieldHandle result8=result[7];

  EXPECT_EQ(result1->vmesh()->num_elems(), 7660);
  EXPECT_EQ(result1->vmesh()->num_nodes(), 1862);

  EXPECT_EQ(result2->vmesh()->num_elems(), 7141);
  EXPECT_EQ(result2->vmesh()->num_nodes(), 1760);

  EXPECT_EQ(result3->vmesh()->num_elems(), 7539);
  EXPECT_EQ(result3->vmesh()->num_nodes(), 1808);

  EXPECT_EQ(result4->vmesh()->num_elems(), 7560);
  EXPECT_EQ(result4->vmesh()->num_nodes(), 1810);

  EXPECT_EQ(result5->vmesh()->num_elems(), 7834);
  EXPECT_EQ(result5->vmesh()->num_nodes(), 1877);

  EXPECT_EQ(result6->vmesh()->num_elems(), 7506);
  EXPECT_EQ(result6->vmesh()->num_nodes(), 1805);

  EXPECT_EQ(result7->vmesh()->num_elems(), 7824);
  EXPECT_EQ(result7->vmesh()->num_nodes(), 1846);

  EXPECT_EQ(result8->vmesh()->num_elems(), 7707);
  EXPECT_EQ(result8->vmesh()->num_nodes(), 1805);

}

TEST(SplitByConnectedRegionTest, SplitFieldByConnectedRegionAlgoTriTests)
{
  SplitFieldByConnectedRegionAlgo algo;

  algo.set(SplitFieldByConnectedRegionAlgo::SortDomainBySize(), false);
  algo.set(SplitFieldByConnectedRegionAlgo::SortAscending(), false);

  FieldHandle trimesh  = SplitFieldByConnectedRegionModuleTriTests();

  std::vector<FieldHandle> result = algo.run(trimesh);

  EXPECT_EQ(result.size(), 126);

  FieldHandle result1=result[0];
  FieldHandle result2=result[1];
  FieldHandle result3=result[2];
  FieldHandle result4=result[3];
  FieldHandle result5=result[4];
  FieldHandle result6=result[5];
  FieldHandle result7=result[6];
  FieldHandle result8=result[7];

  EXPECT_EQ(result1->vmesh()->num_elems(), 1964);
  EXPECT_EQ(result1->vmesh()->num_nodes(), 984);

  EXPECT_EQ(result2->vmesh()->num_elems(), 1890);
  EXPECT_EQ(result2->vmesh()->num_nodes(), 947);

  EXPECT_EQ(result3->vmesh()->num_elems(), 1860);
  EXPECT_EQ(result3->vmesh()->num_nodes(), 932);

  EXPECT_EQ(result4->vmesh()->num_elems(), 1858);
  EXPECT_EQ(result4->vmesh()->num_nodes(), 931);

  EXPECT_EQ(result5->vmesh()->num_elems(), 1942);
  EXPECT_EQ(result5->vmesh()->num_nodes(), 973);

  EXPECT_EQ(result6->vmesh()->num_elems(), 1868);
  EXPECT_EQ(result6->vmesh()->num_nodes(), 936);

  EXPECT_EQ(result7->vmesh()->num_elems(), 1848);
  EXPECT_EQ(result7->vmesh()->num_nodes(), 926);

  EXPECT_EQ(result8->vmesh()->num_elems(), 1786);
  EXPECT_EQ(result8->vmesh()->num_nodes(), 895);

}

TEST(SplitByConnectedRegionTest, DISABLED_SplitFieldByConnectedRegionAlgoTriTests2)
{
  SplitFieldByConnectedRegionAlgo algo;

  algo.set(SplitFieldByConnectedRegionAlgo::SortDomainBySize(), true);
  algo.set(SplitFieldByConnectedRegionAlgo::SortAscending(), false);

  FieldHandle trimesh  = SplitFieldByConnectedRegionModuleTriTests();

  std::vector<FieldHandle> result = algo.run(trimesh);

  EXPECT_EQ(result.size(), 126);

  FieldHandle result1=result[0];
  FieldHandle result2=result[1];
  FieldHandle result3=result[2];
  FieldHandle result4=result[3];
  FieldHandle result5=result[4];
  FieldHandle result6=result[5];
  FieldHandle result7=result[6];
  FieldHandle result8=result[7];

  EXPECT_EQ(result1->vmesh()->num_elems(), 1822);
  EXPECT_EQ(result1->vmesh()->num_nodes(), 913);

  EXPECT_EQ(result2->vmesh()->num_elems(), 1846);
  EXPECT_EQ(result2->vmesh()->num_nodes(), 925);

  EXPECT_EQ(result3->vmesh()->num_elems(), 1938);
  EXPECT_EQ(result3->vmesh()->num_nodes(), 971);

  EXPECT_EQ(result4->vmesh()->num_elems(), 1918);
  EXPECT_EQ(result4->vmesh()->num_nodes(), 961);

  EXPECT_EQ(result5->vmesh()->num_elems(), 1848);
  EXPECT_EQ(result5->vmesh()->num_nodes(), 926);

  EXPECT_EQ(result6->vmesh()->num_elems(), 1978);
  EXPECT_EQ(result6->vmesh()->num_nodes(), 991);

  EXPECT_EQ(result7->vmesh()->num_elems(), 1656);
  EXPECT_EQ(result7->vmesh()->num_nodes(), 830);

  EXPECT_EQ(result8->vmesh()->num_elems(), 1902);
  EXPECT_EQ(result8->vmesh()->num_nodes(), 953);

}

TEST(SplitByConnectedRegionTest, DISABLED_SplitFieldByConnectedRegionAlgoTriTests3)
{
  SplitFieldByConnectedRegionAlgo algo;

  algo.set(SplitFieldByConnectedRegionAlgo::SortDomainBySize(), true);
  algo.set(SplitFieldByConnectedRegionAlgo::SortAscending(), true);

  FieldHandle trimesh  = SplitFieldByConnectedRegionModuleTriTests();

  std::vector<FieldHandle> result = algo.run(trimesh);

  EXPECT_EQ(result.size(), 126);

  FieldHandle result1=result[0];
  FieldHandle result2=result[1];
  FieldHandle result3=result[2];
  FieldHandle result4=result[3];
  FieldHandle result5=result[4];
  FieldHandle result6=result[5];
  FieldHandle result7=result[6];
  FieldHandle result8=result[7];

  EXPECT_EQ(result1->vmesh()->num_elems(), 1910);
  EXPECT_EQ(result1->vmesh()->num_nodes(), 957);

  EXPECT_EQ(result2->vmesh()->num_elems(), 1762);
  EXPECT_EQ(result2->vmesh()->num_nodes(), 883);

  EXPECT_EQ(result3->vmesh()->num_elems(), 1632);
  EXPECT_EQ(result3->vmesh()->num_nodes(), 818);

  EXPECT_EQ(result4->vmesh()->num_elems(), 1814);
  EXPECT_EQ(result4->vmesh()->num_nodes(), 909);

  EXPECT_EQ(result5->vmesh()->num_elems(), 1800);
  EXPECT_EQ(result5->vmesh()->num_nodes(), 902);

  EXPECT_EQ(result6->vmesh()->num_elems(), 1802);
  EXPECT_EQ(result6->vmesh()->num_nodes(), 903);

  EXPECT_EQ(result7->vmesh()->num_elems(), 1868);
  EXPECT_EQ(result7->vmesh()->num_nodes(), 936);

  EXPECT_EQ(result8->vmesh()->num_elems(), 1946);
  EXPECT_EQ(result8->vmesh()->num_nodes(), 975);

}

TEST(SplitByConnectedRegionTest, DISABLED_SplitFieldByConnectedRegionAlgoTriTests4)
{
  SplitFieldByConnectedRegionAlgo algo;

  algo.set(SplitFieldByConnectedRegionAlgo::SortDomainBySize(), false);
  algo.set(SplitFieldByConnectedRegionAlgo::SortAscending(), true);

  FieldHandle trimesh  = SplitFieldByConnectedRegionModuleTriTests();

  std::vector<FieldHandle> result = algo.run(trimesh);

  EXPECT_EQ(result.size(), 126);

  FieldHandle result1=result[0];
  FieldHandle result2=result[1];
  FieldHandle result3=result[2];
  FieldHandle result4=result[3];
  FieldHandle result5=result[4];
  FieldHandle result6=result[5];
  FieldHandle result7=result[6];
  FieldHandle result8=result[7];

  EXPECT_EQ(result1->vmesh()->num_elems(), 1964);
  EXPECT_EQ(result1->vmesh()->num_nodes(), 984);

  EXPECT_EQ(result2->vmesh()->num_elems(), 1890);
  EXPECT_EQ(result2->vmesh()->num_nodes(), 947);

  EXPECT_EQ(result3->vmesh()->num_elems(), 1860);
  EXPECT_EQ(result3->vmesh()->num_nodes(), 932);

  EXPECT_EQ(result4->vmesh()->num_elems(), 1858);
  EXPECT_EQ(result4->vmesh()->num_nodes(), 931);

  EXPECT_EQ(result5->vmesh()->num_elems(), 1942);
  EXPECT_EQ(result5->vmesh()->num_nodes(), 973);

  EXPECT_EQ(result6->vmesh()->num_elems(), 1868);
  EXPECT_EQ(result6->vmesh()->num_nodes(), 936);

  EXPECT_EQ(result7->vmesh()->num_elems(), 1848);
  EXPECT_EQ(result7->vmesh()->num_nodes(), 926);

  EXPECT_EQ(result8->vmesh()->num_elems(), 1786);
  EXPECT_EQ(result8->vmesh()->num_nodes(), 895);

}
