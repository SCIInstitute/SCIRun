/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
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

#include <gtest/gtest.h>
#include <Core/ImportExport/Field/FieldIEPlugin.h>
#include <Core/ImportExport/ColorMap/ColorMapIEPlugin.h>
#include <Core/ImportExport/Matrix/MatrixIEPlugin.h>
#include <Testing/ModuleTestBase/ModuleTestBase.h>
#include <Core/Matlab/matlabfile.h>
#include <Core/Matlab/matlabarray.h>
#include <Core/Matlab/matlabconverter.h>

using namespace SCIRun;
using namespace Testing;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::MatlabIO;
using namespace SCIRun::Core::Logging;

TEST(ImportExportPluginManagerTest, CanCreate)
{
  FieldIEPluginManager manager;
  std::vector<std::string> importers;
  manager.get_importer_list(importers);
  EXPECT_TRUE(importers.empty());

  ColorMapIEPluginManager manager2;
  manager2.get_importer_list(importers);
  EXPECT_TRUE(importers.empty());

  MatrixIEPluginManager manager3;
  manager3.get_importer_list(importers);
  EXPECT_TRUE(importers.empty());
}

namespace
{
  FieldHandle freaderDummy(Core::Logging::LoggerHandle pr, const char *filename)
  {
    return FieldHandle();
  }
  bool fwriterDummy(Core::Logging::LoggerHandle pr, FieldHandle f, const char *filename)
  {
    return false;
  }
  ColorMapHandle creaderDummy(Core::Logging::LoggerHandle pr, const char *filename)
  {
    return ColorMapHandle();
  }
  bool cwriterDummy(Core::Logging::LoggerHandle pr, ColorMapHandle f, const char *filename)
  {
    return false;
  }
  MatrixHandle mreaderDummy(Core::Logging::LoggerHandle pr, const char *filename)
  {
    return MatrixHandle();
  }
  bool mwriterDummy(Core::Logging::LoggerHandle pr, MatrixHandle f, const char *filename)
  {
    return false;
  }
}

TEST(ImportExportPluginManagerTest, PluginsAddSelfToManagerField)
{
  FieldIEPluginLegacyAdapter dummy("dummy", ".fld", "123", freaderDummy, fwriterDummy);

  FieldIEPluginManager manager;
  std::vector<std::string> importers;
  manager.get_importer_list(importers);
  EXPECT_EQ(1, importers.size());
  std::vector<std::string> exporters;
  manager.get_exporter_list(exporters);
  EXPECT_EQ(1, exporters.size());
  auto plugin = manager.get_plugin("dummy");
  EXPECT_EQ(&dummy, plugin);

  ColorMapIEPluginManager cmanager;
  EXPECT_EQ(0, cmanager.numPlugins());
  MatrixIEPluginManager mmanager;
  EXPECT_EQ(0, mmanager.numPlugins());
}

TEST(ImportExportPluginManagerTest, PluginsAddSelfToManagerColorMap)
{
  ColorMapIEPluginLegacyAdapter dummy("dummy", ".color", "123", creaderDummy, cwriterDummy);

  ColorMapIEPluginManager manager;
  std::vector<std::string> importers;
  manager.get_importer_list(importers);
  EXPECT_EQ(1, importers.size());
  std::vector<std::string> exporters;
  manager.get_exporter_list(exporters);
  EXPECT_EQ(1, exporters.size());
  auto plugin = manager.get_plugin("dummy");
  EXPECT_EQ(&dummy, plugin);

  FieldIEPluginManager fmanager;
  EXPECT_EQ(0, fmanager.numPlugins());
  MatrixIEPluginManager mmanager;
  EXPECT_EQ(0, mmanager.numPlugins());
}

TEST(ImportExportPluginManagerTest, PluginsAddSelfToManagerMatrix)
{
  MatrixIEPluginLegacyAdapter dummy("dummy", ".mat", "123", mreaderDummy, mwriterDummy);

  MatrixIEPluginManager manager;
  std::vector<std::string> importers;
  manager.get_importer_list(importers);
  EXPECT_EQ(1, importers.size());
  std::vector<std::string> exporters;
  manager.get_exporter_list(exporters);
  EXPECT_EQ(1, exporters.size());
  auto plugin = manager.get_plugin("dummy");
  EXPECT_EQ(&dummy, plugin);

  ColorMapIEPluginManager cmanager;
  EXPECT_EQ(0, cmanager.numPlugins());
  FieldIEPluginManager fmanager;
  EXPECT_EQ(0, fmanager.numPlugins());
}

TEST(ImportExportPluginManagerTest, CanAddMultiple)
{
  FieldIEPluginLegacyAdapter dummy1("dummy1", ".fld", "123", freaderDummy, fwriterDummy);
  FieldIEPluginLegacyAdapter dummy2("dummy2", ".dot", "123", freaderDummy, fwriterDummy);

  FieldIEPluginManager manager;
  std::vector<std::string> importers;
  manager.get_importer_list(importers);
  EXPECT_EQ(2, importers.size());
  std::vector<std::string> exporters;
  manager.get_exporter_list(exporters);
  EXPECT_EQ(2, exporters.size());
  auto plugin = manager.get_plugin("dummy1");
  EXPECT_EQ(&dummy1, plugin);
  plugin = manager.get_plugin("dummy2");
  EXPECT_EQ(&dummy2, plugin);

  EXPECT_EQ(2, manager.numPlugins());
}

TEST(ImportExportPluginManagerTest, ExactDuplicatesDoNotGetAddedToManager)
{
  FieldIEPluginLegacyAdapter dummy1("dummy1", ".fld", "123", freaderDummy, fwriterDummy);
  FieldIEPluginLegacyAdapter dummy1a("dummy1", ".fld", "123", freaderDummy, fwriterDummy);

  FieldIEPluginManager manager;
  std::vector<std::string> importers;
  manager.get_importer_list(importers);
  EXPECT_EQ(1, importers.size());
  std::vector<std::string> exporters;
  manager.get_exporter_list(exporters);
  EXPECT_EQ(1, exporters.size());
  auto plugin = manager.get_plugin("dummy1");
  EXPECT_EQ(&dummy1, plugin);
  plugin = manager.get_plugin("dummy1(2)");
  EXPECT_EQ(0, plugin);

  EXPECT_EQ(1, manager.numPlugins());
}

TEST(ImportExportPluginManagerTest, DuplicatesByNameGetAddedUnderIncrementedName)
{
  FieldIEPluginLegacyAdapter dummy1("dummy1", ".obj", "321", freaderDummy, fwriterDummy);
  FieldIEPluginLegacyAdapter dummy1a("dummy1", ".fld", "123", freaderDummy, fwriterDummy);

  FieldIEPluginManager manager;
  std::vector<std::string> importers;
  manager.get_importer_list(importers);
  EXPECT_EQ(2, importers.size());
  std::vector<std::string> exporters;
  manager.get_exporter_list(exporters);
  EXPECT_EQ(2, exporters.size());
  auto plugin = manager.get_plugin("dummy1");
  EXPECT_EQ(&dummy1, plugin);
  plugin = manager.get_plugin("dummy1(2)");
  EXPECT_EQ(&dummy1a, plugin);

  EXPECT_EQ(2, manager.numPlugins());
}

TEST(WriteMatrixTests, FileExtensionRegex)
{
  EXPECT_EQ("SCIRun Field File", fileTypeDescriptionFromDialogBoxFilter(defaultImportTypeForFile<Field>(0)));
  EXPECT_EQ("ObjToField", fileTypeDescriptionFromDialogBoxFilter("ObjToField (*.obj)"));
  EXPECT_EQ("SCIRun Field ASCII", fileTypeDescriptionFromDialogBoxFilter("SCIRun Field ASCII (*.fld)"));
  EXPECT_EQ("SCIRun Field Binary", fileTypeDescriptionFromDialogBoxFilter("SCIRun Field Binary (*.fld)"));
}

TEST(MatlabFieldFormatTest, CheckMatlabStructure)
{
  auto field = CreateEmptyLatVol(3, 4, 5);
  ASSERT_TRUE(field != nullptr);

  matlabarray ma;

  {
    matlabconverter mc(nullptr);
    std::string name;

    mc.converttostructmatrix();
    mc.sciFieldTOmlArray(field, ma);
  }

  std::cout << "Base type: " << ma.gettype() << std::endl;

  std::cout << "Fields:" << std::endl;
  for (const auto& fieldName : ma.getfieldnames())
  {
    auto subField = ma.getfield(0, fieldName);
    std::cout << "Name: " << fieldName <<
      " Type: " << subField.gettype() <<
      " Dims: " << subField.getnumdims() << " (" << subField.getm() << "x" << subField.getn() << ")"
      << std::endl;
    switch (subField.gettype())
    {
      case matfilebase::miUINT8:
      {
        auto str = subField.getstring();
        std::cout << "string: " << str << std::endl;
        break;
      }
      case matfilebase::miDOUBLE:
      {
        std::vector<double> v;
        subField.getnumericarray(v);
        std::cout << "double array: (vector) " << v.size() << "\n";
        for (const auto& x : v)
          std::cout << x << " ";
        std::cout << std::endl;

        break;
      }
      default:
        std::cout << "some other array: " << std::endl;
        break;
    }
  }
}
