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


#include <gtest/gtest.h>
#include <Core/DatabaseManager/DatabaseManager.h>
#include <Testing/Utils/SCIRunUnitTests.h>

using namespace SCIRun::Core::Database;
using namespace SCIRun::TestUtils;

class ModuleDatabaseQueries : public ::testing::Test
{
protected:
  static void SetUpTestCase()
  {
    if (!CreateDatabaseFromScratch())
      throw std::runtime_error("Failure setting up new copy of module database in memory.");
  }

  static DatabaseManager moduleDb_;

  static boost::filesystem::path moduleDatabasePath()
  {
    return TestResources::rootDir() / "Databases" / "scirun5modules.db";
  }

  static bool loadModuleDatabase()
  {
    std::string error;
    return moduleDb_.load_database(moduleDatabasePath(), error);
  }

  static bool saveModuleDatabase()
  {
    std::string error;
    return moduleDb_.save_database(moduleDatabasePath(), error);
  }

  static bool createEmptyModuleTable()
  {
    std::string stmt = "CREATE TABLE modules (ModuleName TEXT PRIMARY KEY, Enabled INTEGER, RegressionNetwork INTEGER, OtherNetworks INTEGER, GuiTested INTEGER, ModuleUnitTest INTEGER, ModuleFunctionalTest INTEGER, AlgorithmUnitTest INTEGER, ParameterizedAlgorithmUnitTest INTEGER, Documentation INTEGER, FeatureComplete INTEGER)";
    std::string error;
    return moduleDb_.run_sql_statement(stmt, error);
  }

  static bool insertRowsAsOfMay2014()
  {
    std::string stmt = "INSERT INTO modules VALUES "
    // ModuleName,                      Enabled,  RegressionNetwork,  OtherNetworks,  GuiTested,  ModuleUnitTest, ModuleFunctionalTest, AlgorithmUnitTest,  ParameterizedAlgorithmUnitTest, Documentation, FeatureComplete
    // BrainStimulator
    "('ElectrodeCoilSetup',             1,        0,                  0,              0,          1,              0,                    1,                  0,                              0,             0),"
    "('GenerateROIStatistics',          1,        0,                  0,              0,          0,              0,                    1,                  0,                              0,             0),"
    "('SetConductivitiesToMesh',     1,        0,                  0,              0,          1,              0,                    0,                  0,                              0,             0),"
    "('SetupTDCS',          1,        0,                  0,              0,          1,              0,                    0,                  0,                              0,             0),"
    // Bundle
    "('GetFieldsFromBundle',            1,        1,                  0,              0,          1,              0,                    0,                  0,                              1,             0),"
    "('InsertFieldsIntoBundle',         1,        1,                  0,              0,          1,              0,                    0,                  0,                              1,             0),"
    // ChangeFieldData
    "('ApplyMappingMatrix',             1,        0,                  0,              0,          0,              0,                    0,                  0,                              1,             0),"
    "('CalculateFieldData',             1,        1,                  0,              0,          1,              0,                    0,                  0,                              1,             0),"
    "('CalculateGradients',             1,        1,                  0,              0,          1,              0,                    1,                  0,                              1,             0),"
    "('CalculateSignedDistanceToField', 1,        0,                  0,              0,          1,              0,                    0,                  0,                              1,             0),"
    "('CalculateVectorMagnitudes',      1,        0,                  0,              0,          0,              0,                    0,                  0,                              1,             0),"
    "('CreateFieldData',                1,        1,                  1,              0,          0,              0,                    0,                  0,                              1,             0),"
    "('GetFieldData',                   1,        0,                  0,              0,          0,              0,                    1,                  0,                              1,             0),"
    "('MapFieldDataFromElemToNode',     1,        0,                  0,              0,          0,              0,                    1,                  0,                              1,             0),"
    "('MapFieldDataFromNodeToElem',     1,        0,                  0,              0,          0,              0,                    1,                  0,                              1,             0),"
    "('SetFieldData',                   1,        0,                  0,              0,          0,              0,                    1,                  0,                              1,             0),"
    // ChangeMesh
    "('AlignMeshBoundingBoxes',         1,        0,                  0,              0,          1,              0,                    1,                  0,                              1,             0),"
    "('ConvertQuadSurfToTriSurf',       1,        0,                  0,              0,          1,              0,                    0,                  0,                              1,             0),"
    "('ConvertHexVolToTetVol',          1,        1,                  0,              0,          1,              0,                    1,                  0,                              1,             1),"
    "('GetFieldNodes',                  1,        0,                  0,              0,          1,              0,                    0,                  0,                              1,             0),"
    "('SetFieldNodes',                  1,        0,                  0,              0,          1,              0,                    0,                  0,                              1,             0),"
    // DataIO
    "('ReadField',                      1,        1,                  1,              0,          0,              0,                    0,                  0,                              1,             0),"
    "('ReadNRRD',                       0,        0,                  0,              0,          0,              0,                    0,                  0,                              1,             0),"
    "('ReadMatrix',                     1,        1,                  1,              0,          1,              1,                    1,                  0,                              1,             0),"
    "('WriteField',                     1,        0,                  0,              0,          0,              0,                    0,                  0,                              1,             0),"
    "('WriteMatrix',                    1,        1,                  1,              0,          1,              1,                    1,                  0,                              1,             0),"

    //std::string error;
    //bool ret = moduleDb_.run_sql_statement(stmt, error);
    //if (!ret)
    //  std::cout << "INSERT ERROR:\n\t" << error << std::endl;

    //stmt = "INSERT INTO modules VALUES "
    // FiniteElements
    "('BuildFEMatrix',                  1,        1,                  1,              0,          0,              0,                    1,                  0,                              1,             0),"
    "('BuildTDCSMatrix',                1,        1,                  0,              0,          0,              0,                    1,                  0,                              0,             0),"
    "('tDCSSimulator',                  1,        0,                  0,              0,          0,              0,                    0,                  0,                              0,             0),"
    // Math
    "('AddKnownsToLinearSystem',        1,        0,                  0,              0,          0,              0,                    1,                  0,                              1,             0),"
    "('AppendMatrix',                   1,        1,                  1,              0,          1,              0,                    1,                  0,                              1,             1),"
    "('ConvertMatrixType',              1,        0,                  0,              0,          0,              0,                    1,                  0,                              1,             0),"
    "('CreateMatrix',                   1,        1,                  1,              1,          0,              0,                    0,                  0,                              1,             0),"
    "('EvaluateLinearAlgebraBinary',    1,        0,                  0,              0,          1,              0,                    1,                  0,                              1,             0),"
    "('EvaluateLinearAlgebraUnary',     1,        0,                  0,              0,          1,              1,                    1,                  0,                              1,             0),"
    "('ReportMatrixInfo',               1,        1,                  1,              0,          1,              0,                    1,                  0,                              1,             0),"
    "('SelectSubMatrix',                1,        0,                  0,              0,          0,              0,                    1,                  0,                              1,             0),"
    "('SolveLinearSystem',              1,        1,                  0,              0,          1,              0,                    1,                  1,                              1,             0),"
    "('SolveMinNormLeastSqSystem',      1,        0,                  0,              0,          1,              0,                    0,                  0,                              1,             0),"
    // MiscField
    "('ReportFieldInfo',                1,        1,                  1,              0,          0,              0,                    1,                  0,                              1,             0),"
    // NewField
    "('CreateLatVol',                   1,        1,                  1,              0,          1,              0,                    0,                  0,                              1,             0),"
    "('GetDomainBoundary',              1,        1,                  0,              0,          0,              0,                    1,                  1,                              1,             0),"
    "('GetFieldBoundary',               1,        0,                  0,              0,          0,              0,                    1,                  0,                              1,             0),"
    "('InterfaceWithCleaver',           1,        0,                  0,              0,          1,              0,                    1,                  0,                              1,             0),"
    "('JoinFields',                     1,        0,                  0,              0,          1,              0,                    1,                  1,                              1,             0),"
    "('SplitFieldByDomain',             1,        0,                  0,              0,          0,              0,                    0,                  0,                              1,             0),"
    "('SplitFieldByConnectedRegion',    1,        0,                  0,              0,          0,              0,                    0,                  0,                              0,             0),"
    "('SimulateForwardMagneticField',   1,        0,                  0,              0,          0,              0,                    0,                  0,                              0,             0),"
    // Render
    "('ViewScene',                      1,        1,                  1,              0,          1,              0,                    0,                  0,                              1,             0),"
    // String
    "('CreateString',                   1,        0,                  0,              0,          0,              0,                    0,                  0,                              1,             0),"
    "('PrintDatatype',                  1,        1,                  1,              0,          0,              0,                    0,                  0,                              0,             0),"
    // Testing
    "('DynamicPortTester',              1,        1,                  1,              0,          0,              0,                    0,                  0,                              0,             0),"
    "('ReceiveScalar',                  1,        0,                  0,              1,          1,              0,                    0,                  0,                              0,             0),"
    "('SendScalar',                     1,        0,                  1,              1,          1,              0,                    0,                  0,                              0,             0),"
    // Visualization
    "('CreateStandardColorMap',         1,        1,                  1,              0,          1,              0,                    0,                  0,                              1,             0),"
    "('ExtractSimpleIsosurface',        1,        0,                  0,              0,          0,              0,                    0,                  0,                              0,             0),"
    "('ClipVolumeByIsovalue',	        1,        0,                  0,              0,          0,              0,                    0,                  0,                              0,             0),"
    "('RemoveUnsedNodes',	        1,        0,                  0,              0,          1,              0,                    1,                  0,                              0,             1),"
    "('ShowField',                      1,        1,                  1,              0,          0,              0,                    0,                  0,                              1,             0)";

    std::string error;
    bool ret = moduleDb_.run_sql_statement(stmt, error);
    if (!ret)
      std::cout << "INSERT ERROR:\n\t" << error << std::endl;
    return ret;
  }

  static long long countModulesWhere(const std::string& where = "1=1")
  {
    std::string stmt = "SELECT COUNT(*) AS NumModules FROM modules WHERE " + where;
    ResultSet result;
    std::string error;
    bool success = moduleDb_.run_sql_statement(stmt, result, error);
    if (!success || result.empty())
      return -1;
    return boost::any_cast<long long>(result[0]["NumModules"]);
  }

  static long long countEnabledModules()
  {
    return countModulesWhere("Enabled = 1");
  }

  static bool CreateDatabaseFromScratch()
  {
    if (boost::filesystem::exists(moduleDatabasePath()))
    {
      std::cout << "Module database already exists, attempting to delete file for a fresh copy." << std::endl;
      if (!boost::filesystem::remove(moduleDatabasePath()))
      {
        std::cout << "Couldn't remove file" << std::endl;
        return false;
      }
    }

    if (!loadModuleDatabase())
    {
      std::cout << "failed loading database" << std::endl;
      return false;
    }
    if (!createEmptyModuleTable())
    {
      std::cout << "failed create table" << std::endl;
      return false;
    }
    if (!insertRowsAsOfMay2014())
    {
      std::cout << "failed insert" << std::endl;
      return false;
    }
    if (!saveModuleDatabase())
    {
      std::cout << "failed saving database" << std::endl;
      return false;
    }
    return true;
  }
};

DatabaseManager ModuleDatabaseQueries::moduleDb_;

//TODO: figure out better way to generate a report from these tests. Need a process to enforce keeping them up to date (special branch?)
namespace
{
  bool assertTestStatistics = false;
}

TEST_F(ModuleDatabaseQueries, CountEnabledModules)
{
  auto count = countEnabledModules();
  std::cout << "# of enabled modules: " << count << std::endl;
  if (assertTestStatistics)
    EXPECT_EQ(57, count);
}

TEST_F(ModuleDatabaseQueries, AllModulesUnitTested)
{
  auto count = countModulesWhere("ModuleUnitTest > 0");
  std::cout << "# of unit tested modules: " << count << std::endl;
  if (assertTestStatistics)
    EXPECT_EQ(countEnabledModules(), count);
}

TEST_F(ModuleDatabaseQueries, AllModulesRegressionTested)
{
  auto count = countModulesWhere("RegressionNetwork > 0");
  std::cout << "# of regression tested modules: " << count << std::endl;
  if (assertTestStatistics)
    EXPECT_EQ(countEnabledModules(), count);
}

TEST_F(ModuleDatabaseQueries, AllAlgorithmsUnitTested)
{
  auto count = countModulesWhere("AlgorithmUnitTest > 0");
  std::cout << "# of unit tested algorithms: " << count << std::endl;
  if (assertTestStatistics)
    EXPECT_EQ(countEnabledModules(), count);
}

TEST_F(ModuleDatabaseQueries, AllAlgorithmsUnitTestedWithParameters)
{
  auto count = countModulesWhere("ParameterizedAlgorithmUnitTest > 0");
  std::cout << "# of parameterized-tested algorithms: " << count << std::endl;
  if (assertTestStatistics)
    EXPECT_EQ(countEnabledModules(), count);
}

TEST_F(ModuleDatabaseQueries, AllModulesDocumented)
{
  auto count = countModulesWhere("Documentation > 0");
  std::cout << "# of documented modules: " << count << std::endl;
  if (assertTestStatistics)
    EXPECT_EQ(countEnabledModules(), count);
}
