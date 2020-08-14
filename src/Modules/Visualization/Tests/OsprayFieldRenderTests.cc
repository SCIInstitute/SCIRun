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
#include <Modules/Visualization/ShowField.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Algorithms/Legacy/Fields/ConvertMeshType/ConvertMeshToTriSurfMeshAlgo.h>
#include <Core/Algorithms/Legacy/Fields/MeshDerivatives/GetFieldBoundaryAlgo.h>
#include <Core/Utils/Exception.h>
#include <Core/Logging/Log.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Mesh/VirtualMeshFacade.h>

#include <ospray/ospray.h>
//#include <ospray/version.h>

//TODO: rewrite for ospray2
#if 0

using namespace SCIRun;
using namespace SCIRun::Testing;
using namespace SCIRun::TestUtils;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;
using namespace Fields;
using namespace SCIRun::Core::Algorithms::Visualization;
using namespace SCIRun::Modules::Visualization;
using namespace SCIRun::Core;
using namespace SCIRun::Core::Logging;
using ::testing::Values;
using ::testing::Combine;
using ::testing::Range;

class OsprayFieldRenderTest : public ParameterizedModuleTest<int>
{
public:
  static void SetUpTestCase()
  {
    // initialize OSPRay; OSPRay parses (and removes) its commandline parameters, e.g. "--osp:debug"
    const char* argv[] = { "" };
    int argc = 0;
    int init_error = ospInit(&argc, argv);
    if (init_error != OSP_NO_ERROR)
      throw init_error;
  }
protected:
  virtual void SetUp() override
  {
    LogSettings::Instance().setVerbose(false);
    auto size = GetParam();
    auto max = static_cast<double>(size);
    latVol = CreateEmptyLatVol(size, size, size, DOUBLE_E, { 0, 0, 0 }, { max, max, max });
    GeneralLog::Instance().get()->info("Setting up ShowField with size {}^3 latvol", size);
  }

  UseRealModuleStateFactory f;
  ModuleHandle showField;
  FieldHandle latVol;
};


namespace osprayImpl
{

  // helper function to write the rendered image as PPM file
  void writePPM(const char *fileName,
    const osp::vec2i &size,
    const uint32_t *pixel)
  {
    FILE *file = fopen(fileName, "wb");
    fprintf(file, "P6\n%i %i\n255\n", size.x, size.y);
    unsigned char *out = (unsigned char *)alloca(3 * size.x);
    for (int y = 0; y < size.y; y++) {
      const unsigned char *in = (const unsigned char *)&pixel[(size.y - 1 - y)*size.x];
      for (int x = 0; x < size.x; x++) {
        out[3 * x + 0] = in[4 * x + 0];
        out[3 * x + 1] = in[4 * x + 1];
        out[3 * x + 2] = in[4 * x + 2];
      }
      fwrite(out, 3 * size.x, sizeof(char), file);
    }
    fprintf(file, "\n");
    fclose(file);
    std::cout << "wrote file " << fileName << std::endl;
  }

  int renderLatVol(FieldHandle latvol, float cameraSteps, int max)
  {
    FieldHandle trisurf;
    {
      GetFieldBoundaryAlgo getfieldbound_algo;
      FieldHandle field_boundary;
      getfieldbound_algo.run(latvol, field_boundary);
      ConvertMeshToTriSurfMeshAlgo converter;

      converter.run(field_boundary, trisurf);
    }

    auto facade(trisurf->mesh()->getFacade());

    std::cout << "hello ospray test" << std::endl;
    // image size
    osp::vec2i imgSize;
    imgSize.x = 1024; // width
    imgSize.y = 768; // height

    // camera
    float cam_pos[] = { -1.f, 0.f, -5.f };
    cam_pos[0] += cameraSteps;
    float cam_up[] = { 0.f, 1.f, 0.f };
    float cam_view[] = { 0.5f, 0.5f, 1.f };

    // triangle mesh data
    // float vertex_example[] = { -1.0f, -1.0f, 3.0f, 0.f,
    //   -1.0f, 1.0f, 3.0f, 0.f,
    //   1.0f, -1.0f, 3.0f, 0.f,
    //   0.1f, 0.1f, 0.3f, 0.f };

    std::vector<float> vertex, color;
    float maxColor = max;
    {
      for (const auto& node : facade->nodes())
      {
        auto point = node.point();
        vertex.push_back(static_cast<float>(point.x()));
        vertex.push_back(static_cast<float>(point.y()));
        vertex.push_back(static_cast<float>(point.z()));
        vertex.push_back(0);
        color.push_back(0.9f * point.x() / maxColor);
        color.push_back(0.9f * point.y() / maxColor);
        color.push_back(0.9f * point.z() / maxColor);
        color.push_back(1.0f);
      }
    }

    // float color_example[] = { 0.9f, 0.5f, 0.5f, 1.0f,
    //   0.8f, 0.8f, 0.8f, 1.0f,
    //   0.8f, 0.8f, 0.8f, 1.0f,
    //   0.5f, 0.9f, 0.5f, 1.0f };
    // int32_t index_example[] = { 0, 1, 2,
    //   1, 2, 3};

    std::vector<int32_t> index;
    {
      for (const auto& face : facade->faces())
      {
        auto nodes = face.nodeIndices();
        index.push_back(static_cast<int32_t>(nodes[0]));
        index.push_back(static_cast<int32_t>(nodes[1]));
        index.push_back(static_cast<int32_t>(nodes[2]));
      }
    }

    // create and setup camera
    OSPCamera camera = ospNewCamera("perspective");
    ospSetf(camera, "aspect", imgSize.x / (float)imgSize.y);
    ospSet3fv(camera, "pos", cam_pos);
    ospSet3fv(camera, "dir", cam_view);
    ospSet3fv(camera, "up", cam_up);
    ospCommit(camera); // commit each object to indicate modifications are done

    // create and setup model and mesh
    OSPGeometry mesh = ospNewGeometry("triangles");
    OSPData data = ospNewData(vertex.size() / 4, OSP_FLOAT3A, &vertex[0]); // OSP_FLOAT3 format is also supported for vertex positions
    //OSPData data = ospNewData(4, OSP_FLOAT3A, vertex_example); // OSP_FLOAT3 format is also supported for vertex positions
    ospCommit(data);
    ospSetData(mesh, "vertex", data);

    data = ospNewData(vertex.size() / 4, OSP_FLOAT4, &color[0]);
    //data = ospNewData(4, OSP_FLOAT4, color_example);
    ospCommit(data);
    ospSetData(mesh, "vertex.color", data);

    data = ospNewData(index.size() / 3, OSP_INT3, &index[0]); // OSP_INT4 format is also supported for triangle indices
    //data = ospNewData(2, OSP_INT3, index_example); // OSP_INT4 format is also supported for triangle indices
    ospCommit(data);
    ospSetData(mesh, "index", data);

    ospCommit(mesh);

    OSPModel world = ospNewModel();
    ospAddGeometry(world, mesh);
    ospCommit(world);

    // create renderer
    OSPRenderer renderer = ospNewRenderer("scivis"); // choose Scientific Visualization renderer

    // create and setup light for Ambient Occlusion
    OSPLight light = ospNewLight(renderer, "ambient");
    ospCommit(light);
    OSPData lights = ospNewData(1, OSP_LIGHT, &light);
    ospCommit(lights);

    // complete setup of renderer
    ospSet1i(renderer, "aoSamples", 1);
    ospSet1f(renderer, "bgColor", 1.0f); // white, transparent
    ospSetObject(renderer, "model", world);
    ospSetObject(renderer, "camera", camera);
    ospSetObject(renderer, "lights", lights);
    ospCommit(renderer);

    // create and setup framebuffer
    OSPFrameBuffer framebuffer = ospNewFrameBuffer(imgSize, OSP_FB_SRGBA, OSP_FB_COLOR | /*OSP_FB_DEPTH |*/ OSP_FB_ACCUM);
    ospFrameBufferClear(framebuffer, OSP_FB_COLOR | OSP_FB_ACCUM);

    // render one frame
    ospRenderFrame(framebuffer, renderer, OSP_FB_COLOR | OSP_FB_ACCUM);

    // access framebuffer and write its content as PPM file
    const uint32_t * fb = (uint32_t*)ospMapFrameBuffer(framebuffer, OSP_FB_COLOR);
    ospUnmapFrameBuffer(fb, framebuffer);

    // render 10 more frames, which are accumulated to result in a better converged image
    for (int frames = 0; frames < 10; frames++)
      ospRenderFrame(framebuffer, renderer, OSP_FB_COLOR | OSP_FB_ACCUM);

    fb = (uint32_t*)ospMapFrameBuffer(framebuffer, OSP_FB_COLOR);
    std::ostringstream fileNameAcc;
    fileNameAcc << "accumulatedFrame_" << vertex.size() << "_" << cameraSteps << ".ppm";
    writePPM(fileNameAcc.str().c_str(), imgSize, fb);
    ospUnmapFrameBuffer(fb, framebuffer);

    return 0;
  }

}

TEST_P(OsprayFieldRenderTest, RenderLatVolWithOspray)
{
  for (int inc : {1, 2, 3, 4, 5, 6, 7, 8, 9, 10})
  {
    osprayImpl::renderLatVol(latVol, inc*0.2, GetParam());
  }

  //FAIL() << "todo";
}

TEST(OsprayVersionTest, CanPrintVersion)
{
  std::cout << "ospray version: " << OSPRAY_VERSION_MAJOR << "." <<  OSPRAY_VERSION_MINOR << "."
    << OSPRAY_VERSION_PATCH << std::endl;
}

INSTANTIATE_TEST_CASE_P(
  RenderLatVolWithOspray,
  OsprayFieldRenderTest,
  Values(4//, 40, 60, 80
  //, 100, 120, 150//, //200 //to speed up make test
  //, 256 // probably runs out of memory
  )
  );
#endif
