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

namespace
{
  bool createTable(DatabaseManager& db)
  {
    std::string stmt = "CREATE TABLE modules (ModuleName TEXT PRIMARY KEY, Enabled INTEGER)";
    std::string error;
    return db.run_sql_statement(stmt, error);
  }

  bool insertRows(DatabaseManager& db)
  {
    std::string stmt = "INSERT INTO modules VALUES ('ReadField', 1), ('ReadNRRD', 0)";
    std::string error;
    return db.run_sql_statement(stmt, error);
  }

  size_t countRows(DatabaseManager& db)
  {
    std::string stmt = "SELECT COUNT(*) AS NumModules FROM modules";
    ResultSet result;
    std::string error;
    bool success = db.run_sql_statement(stmt, result, error);
    if (!success || result.empty())
      return -1;
    return boost::any_cast<long long>(result[0]["NumModules"]);
  }
}

TEST(BasicDatabaseTests, CanCreateDatabase)
{
  DatabaseManager db;
  std::string error;
  EXPECT_TRUE(db.load_database(TestResources::rootDir() / "Databases" / "empty.db", error));
  EXPECT_TRUE(error.empty());
}

TEST(BasicDatabaseTests, CanCreateTable)
{
  DatabaseManager db;
  EXPECT_TRUE(createTable(db));
}

TEST(BasicDatabaseTests, CanInsertIntoTable)
{
  DatabaseManager db;
  createTable(db);
  EXPECT_TRUE(insertRows(db));
}

TEST(BasicDatabaseTests, CanCountRows)
{
  DatabaseManager db;
  createTable(db);
  insertRows(db);
  EXPECT_EQ(2, countRows(db));
}

TEST(BasicDatabaseTests, CanSelectFromTable)
{
  DatabaseManager db;
  createTable(db);
  insertRows(db);

  ResultSet result;
  std::string error;
  std::string stmt = "SELECT * FROM modules";
  EXPECT_TRUE(db.run_sql_statement(stmt, result, error));
  EXPECT_EQ(2, result.size());
  auto rec0 = result[0];
  EXPECT_EQ(2, rec0.size());
  EXPECT_EQ("ReadField", boost::any_cast<std::string>(rec0["ModuleName"]));
  EXPECT_EQ(1, boost::any_cast<long long>(rec0["Enabled"]));
  auto rec1 = result[1];
  EXPECT_EQ(2, rec1.size());
  EXPECT_EQ("ReadNRRD", boost::any_cast<std::string>(rec1["ModuleName"]));
  EXPECT_EQ(0, boost::any_cast<long long>(rec1["Enabled"]));
}

TEST(BasicDatabaseTests, ErrorsForWrongTable)
{
  DatabaseManager db;
  createTable(db);
  insertRows(db);

  ResultSet result;
  std::string error;
  std::string stmt = "SELECT * FROM nodules";
  EXPECT_FALSE(db.run_sql_statement(stmt, result, error));
  EXPECT_EQ("The SQL statement 'SELECT * FROM nodules' failed to compile with error: no such table: nodules", error);
}

TEST(BasicDatabaseTests, CanSelectFromTableFiltered)
{
  DatabaseManager db;
  createTable(db);
  insertRows(db);

  ResultSet result;
  std::string error;

  std::string stmt = "SELECT * FROM modules WHERE Enabled = 1";
  EXPECT_TRUE(db.run_sql_statement(stmt, result, error));
  EXPECT_EQ(1, result.size());
  auto rec0 = result[0];
  EXPECT_EQ(2, rec0.size());
  EXPECT_EQ("ReadField", boost::any_cast<std::string>(rec0["ModuleName"]));
  EXPECT_EQ(1, boost::any_cast<long long>(rec0["Enabled"]));

  stmt = "SELECT * FROM modules WHERE Enabled = 0";
  EXPECT_TRUE(db.run_sql_statement(stmt, result, error));
  EXPECT_EQ(1, result.size());
  rec0 = result[0];
  EXPECT_EQ(2, rec0.size());
  EXPECT_EQ("ReadNRRD", boost::any_cast<std::string>(rec0["ModuleName"]));
  EXPECT_EQ(0, boost::any_cast<long long>(rec0["Enabled"]));

  stmt = "SELECT * FROM modules WHERE ModuleName LIKE '%Field%'";
  EXPECT_TRUE(db.run_sql_statement(stmt, result, error));
  EXPECT_EQ(1, result.size());
  rec0 = result[0];
  EXPECT_EQ(2, rec0.size());
  EXPECT_EQ("ReadField", boost::any_cast<std::string>(rec0["ModuleName"]));
  EXPECT_EQ(1, boost::any_cast<long long>(rec0["Enabled"]));
}

TEST(BasicDatabaseTests, CanUpdateRows)
{
  DatabaseManager db;

  createTable(db);
  insertRows(db);

  ResultSet result;
  std::string error;

  const std::string stmt = "SELECT * FROM modules WHERE Enabled = 1";
  EXPECT_TRUE(db.run_sql_statement(stmt, result, error));
  EXPECT_EQ(1, result.size());

  std::string update = "UPDATE modules SET Enabled = 1 WHERE ModuleName = 'ReadNRRD'";
  EXPECT_TRUE(db.run_sql_statement(update, error));

  EXPECT_TRUE(db.run_sql_statement(stmt, result, error));
  EXPECT_EQ(2, result.size());
}

TEST(BasicDatabaseTests, CanDeleteRows)
{
  DatabaseManager db;

  createTable(db);
  insertRows(db);

  ResultSet result;
  std::string error;

  const std::string stmt = "SELECT * FROM modules WHERE Enabled = 1";
  EXPECT_TRUE(db.run_sql_statement(stmt, result, error));
  EXPECT_EQ(1, result.size());

  std::string update = "DELETE from modules WHERE Enabled = 1";
  EXPECT_TRUE(db.run_sql_statement(update, error));

  EXPECT_TRUE(db.run_sql_statement(stmt, result, error));
  EXPECT_EQ(0, result.size());
}
