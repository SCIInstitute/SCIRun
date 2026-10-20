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

/// Regression test for #2732: mesh synchronize() used to detach its Synchronize
/// workers, which hold a raw mesh pointer, so one could still be inside the mesh's
/// mutex after the mesh was destroyed. Before the fix this aborts within a few
/// hundred iterations -- "mutex lock failed: Invalid argument", a synchronization
/// assertion, or a length_error from a table read during teardown.

#include <gtest/gtest.h>

#include <Core/Basis/TriLinearLgn.h>
#include <Core/Basis/TetLinearLgn.h>
#include <Core/Datatypes/Legacy/Field/TriSurfMesh.h>
#include <Core/Datatypes/Legacy/Field/TetVolMesh.h>

#include <memory>
#include <random>
#include <thread>
#include <vector>

using namespace SCIRun;
using namespace SCIRun::Core::Geometry;

namespace
{
  const int iterations = 200;
  const int threadsPerMesh = 4;

  /// Hammer synchronize() from several threads, then destroy the mesh immediately.
  template <typename MeshT, typename BuildFunc>
  void stressSynchronize(BuildFunc build)
  {
    for (int i = 0; i < iterations; ++i)
    {
      auto mesh = std::make_unique<MeshT>();
      build(*mesh);

      std::vector<std::thread> callers;
      for (int t = 0; t < threadsPerMesh; ++t)
      {
        auto* raw = mesh.get();
        callers.emplace_back([raw, t]
        {
          raw->synchronize(t % 2 ? Mesh::NODE_LOCATE_E
                                 : (Mesh::NODE_LOCATE_E | Mesh::NORMALS_E));
        });
      }
      for (auto& c : callers)
        c.join();

      /// Any worker still holding the mesh now has a dangling pointer.
      mesh.reset();
    }
  }
}

TEST(MeshSynchronizeThreadingTest, TriSurfMeshSurvivesConcurrentSynchronize)
{
  using MeshT = TriSurfMesh<Core::Basis::TriLinearLgn<Point>>;
  std::mt19937 rng(7);
  stressSynchronize<MeshT>([&rng](MeshT& mesh)
  {
    const int nodes = 60;
    for (int n = 0; n < nodes; ++n)
      mesh.add_point(Point((rng() % 1000) / 10.0, (rng() % 1000) / 10.0, (rng() % 1000) / 10.0));
    for (int n = 0; n + 2 < nodes; ++n)
      mesh.add_triangle(MeshT::Node::index_type(n),
                        MeshT::Node::index_type(n + 1),
                        MeshT::Node::index_type(n + 2));
  });
  SUCCEED();
}

TEST(MeshSynchronizeThreadingTest, TetVolMeshSurvivesConcurrentSynchronize)
{
  using MeshT = TetVolMesh<Core::Basis::TetLinearLgn<Point>>;
  std::mt19937 rng(11);
  stressSynchronize<MeshT>([&rng](MeshT& mesh)
  {
    const int nodes = 60;
    for (int n = 0; n < nodes; ++n)
      mesh.add_point(Point((rng() % 1000) / 10.0, (rng() % 1000) / 10.0, (rng() % 1000) / 10.0));
    for (int n = 0; n + 3 < nodes; ++n)
      mesh.add_tet(MeshT::Node::index_type(n),
                   MeshT::Node::index_type(n + 1),
                   MeshT::Node::index_type(n + 2),
                   MeshT::Node::index_type(n + 3));
  });
  SUCCEED();
}
