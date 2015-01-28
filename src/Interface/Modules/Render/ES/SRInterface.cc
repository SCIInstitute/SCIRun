/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2013 Scientific Computing and Imaging Institute,
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

// Needed for OpenGL include files on Travis:
#include <gl-platform/GLPlatform.hpp>
#include <Interface/Modules/Render/UndefiningX11Cruft.h>
#include <QtOpenGL/QGLWidget>

#include <Interface/Modules/Render/namespaces.h>
#include <Interface/Modules/Render/ES/SRInterface.h>
#include <Interface/Modules/Render/ES/SRCamera.h>

#include <Core/Application/Application.h>

// CPM modules.

#include <gl-state/GLState.hpp>
#include <es-general/comp/StaticScreenDims.hpp>
#include <es-general/comp/StaticCamera.hpp>
#include <es-general/comp/StaticOrthoCamera.hpp>
#include <es-general/comp/StaticObjRefID.hpp>
#include <es-general/comp/StaticGlobalTime.hpp>
#include <es-general/comp/Transform.hpp>
#include <es-render/comp/StaticGeomMan.hpp>
#include <es-render/comp/StaticIBOMan.hpp>
#include <es-render/comp/StaticVBOMan.hpp>
#include <es-render/comp/StaticShaderMan.hpp>
#include <es-render/comp/Texture.hpp>
#include <es-render/util/Uniform.hpp>
#include <es-render/comp/VBO.hpp>
#include <es-render/comp/IBO.hpp>
#include <es-render/comp/Shader.hpp>
#include <es-fs/fscomp/StaticFS.hpp>
#include <es-fs/Filesystem.hpp>
#include <es-fs/FilesystemSync.hpp>

#include "CoreBootstrap.h"
#include "comp/StaticSRInterface.h"
#include "comp/RenderBasicGeom.h"
#include "comp/RenderColorMapGeom.h"
#include "comp/SRRenderState.h"
#include "comp/RenderList.h"
#include "comp/StaticWorldLight.h"
#include "comp/LightingUniforms.h"
#include "systems/RenderBasicSys.h"
#include "systems/RenderColorMapSys.h"
#include "systems/RenderTransBasicSys.h"
#include "systems/RenderTransColorMapSys.h"

using namespace std::placeholders;

namespace fs = CPM_ES_FS_NS;

namespace SCIRun {
	namespace Render {

		//------------------------------------------------------------------------------
		SRInterface::SRInterface(std::shared_ptr<Gui::GLContext> context,
			const std::vector<std::string>& shaderDirs) :
			mMouseMode(MOUSE_OLDSCIRUN),
			mScreenWidth(640),
			mScreenHeight(480),
			mContext(context),
			mCamera(new SRCamera(*this))  // Should come after all vars have been initialized.
		{
			// Create default colormaps.
			generateColormaps();

      showOrientation_ = true;
      autoRotate_ = false;

			// Construct ESCore. We will need to bootstrap the core. We should also
			// probably add utility static classes.
			setupCore();
		}

		//------------------------------------------------------------------------------
		SRInterface::~SRInterface()
		{
			glDeleteTextures(1, &mRainbowCMap);
			glDeleteTextures(1, &mGrayscaleCMap);
		}

		//------------------------------------------------------------------------------
		void SRInterface::setupCore()
		{
			mCore.addUserSystem(getSystemName_CoreBootstrap());

			// Add screen height / width static component.
			{
				gen::StaticScreenDims dims;
				dims.width = static_cast<uint32_t>(mScreenWidth);
				dims.height = static_cast<uint32_t>(mScreenHeight);
				mCore.addStaticComponent(dims);
			}

			// Be exceptionally careful with non-serializable components. They must be
			// created outside of the normal bootstrap. They cannot depend on anything
			// being serialized correctly. In this circumstance, the filesystem component
			// is system dependent and cannot be reliably serialized, so we add it and
			// mark it as non-serializable.
  {
	  // Generate synchronous filesystem, manually add its static component,
	  // then mark it as non-serializable.
    std::string filesystemRoot = SCIRun::Core::Application::Instance().executablePath().string();
    filesystemRoot += boost::filesystem::path::preferred_separator;
	  fs::StaticFS fileSystem(
		  std::shared_ptr<fs::FilesystemSync>(new fs::FilesystemSync(filesystemRoot)));
	  mCore.addStaticComponent(fileSystem);
	  mCore.disableComponentSerialization<fs::StaticFS>();
  }

			// Add StaticSRInterface
  {
	  StaticSRInterface iface(this);
	  mCore.addStaticComponent(iface);
  }
		}

		//------------------------------------------------------------------------------
		void SRInterface::setMouseMode(MouseMode mode)
		{
			mMouseMode = mode;
		}

		//------------------------------------------------------------------------------
		SRInterface::MouseMode SRInterface::getMouseMode()
		{
			return mMouseMode;
		}

		//------------------------------------------------------------------------------
		void SRInterface::eventResize(size_t width, size_t height)
		{
			mScreenWidth = width;
			mScreenHeight = height;

			mContext->makeCurrent();
			GL(glViewport(0, 0, static_cast<GLsizei>(width), static_cast<GLsizei>(height)));

			// Obtain StaticScreenDims component and populate.
			gen::StaticScreenDims* dims = mCore.getStaticComponent<gen::StaticScreenDims>();
			if (dims)
			{
				dims->width = static_cast<size_t>(width);
				dims->height = static_cast<size_t>(height);
			}

			// Setup default camera projection.
			gen::StaticCamera* cam = mCore.getStaticComponent<gen::StaticCamera>();
			gen::StaticOrthoCamera* orthoCam = mCore.getStaticComponent<gen::StaticOrthoCamera>();

			if (cam == nullptr || orthoCam == nullptr) return;

			float aspect = static_cast<float>(width) / static_cast<float>(height);

			float perspFOVY = 0.59f;
			float perspZNear = 1.0f;
			float perspZFar = 2000.0f;
			glm::mat4 proj = glm::perspective(perspFOVY, aspect, perspZNear, perspZFar);
			cam->data.setProjection(proj, perspFOVY, aspect, perspZNear, perspZFar);

			// Setup default ortho camera projection
			float orthoZNear = -1000.0f;
			float orthoZFar = 1000.0f;
			glm::mat4 orthoProj =
				glm::ortho(/*left*/   -1.0f,      /*right*/ 1.0f,
				/*bottom*/ -1.0f,      /*top*/   1.0f,
				/*znear*/  orthoZNear, /*zfar*/  orthoZFar);
			orthoCam->data.setOrthoProjection(orthoProj, aspect, 2.0f, 2.0f, orthoZNear, orthoZFar);
		}

		//------------------------------------------------------------------------------
		void SRInterface::inputMouseDown(const glm::ivec2& pos, MouseButton btn)
		{
			mCamera->mouseDownEvent(pos, btn);
		}

		//------------------------------------------------------------------------------
		void SRInterface::inputMouseMove(const glm::ivec2& pos, MouseButton btn)
		{
			mCamera->mouseMoveEvent(pos, btn);
		}

		//------------------------------------------------------------------------------
		void SRInterface::inputMouseWheel(int32_t delta)
		{
			mCamera->mouseWheelEvent(delta);
		}

		//------------------------------------------------------------------------------
		void SRInterface::doAutoView()
		{
			if (mSceneBBox.valid())
			{
				mCamera->doAutoView(mSceneBBox);
			}
		}

		//------------------------------------------------------------------------------
		void SRInterface::setView(const glm::vec3& view, const glm::vec3& up)
		{
			mCamera->setView(view, up);
		}

		//------------------------------------------------------------------------------
		void SRInterface::showOrientation(bool value)
		{
			showOrientation_ = value;
		}

    //------------------------------------------------------------------------------
    void SRInterface::setBackgroundColor(QColor color)
    {
      mCore.setBackgroundColor(color.redF(), color.greenF(), color.blueF(), color.alphaF());
    }

		//------------------------------------------------------------------------------
		void SRInterface::inputMouseUp(const glm::ivec2& /*pos*/, MouseButton /*btn*/)
		{
		}

		//------------------------------------------------------------------------------
		uint64_t SRInterface::getEntityIDForName(const std::string& name, int port)
		{
			return (static_cast<uint64_t>(std::hash<std::string>()(name)) >> 8) + (static_cast<uint64_t>(port) << 56);
		}

		//------------------------------------------------------------------------------
		void SRInterface::handleGeomObject(boost::shared_ptr<Core::Datatypes::GeometryObject> obj, int port)
		{
			// Ensure our rendering context is current on our thread.
			mContext->makeCurrent();

			std::string objectName = obj->objectName;
			Core::Geometry::BBox bbox; // Bounding box containing all vertex buffer objects.

			// Check to see if the object already exists in our list. If so, then
			// remove the object. We will re-add it.
			auto foundObject = std::find_if(
				mSRObjects.begin(), mSRObjects.end(),
				[&objectName, this](const SRObject& obj) -> bool
			{
				if (obj.mName == objectName)
					return true;
				else
					return false;
			});

			ren::VBOMan& vboMan = *mCore.getStaticComponent<ren::StaticVBOMan>()->instance;
			ren::IBOMan& iboMan = *mCore.getStaticComponent<ren::StaticIBOMan>()->instance;
			if (foundObject != mSRObjects.end())
			{
				// Iterate through each of the passes and remove their associated
				// entity ID.
				for (const auto& pass : foundObject->mPasses)
				{
					uint64_t entityID = getEntityIDForName(pass.passName, port);
					mCore.removeEntity(entityID);
				}

				// We need to renormalize the core after removing entities. We don't need
				// to run a new pass however. Renormalization is enough to remove
				// old entities from the system.
				mCore.renormalize(true);

				// Run a garbage collection cycle for the VBOs and IBOs. We will likely
				// be using similar VBO and IBO names.
				vboMan.runGCCycle(mCore);
				iboMan.runGCCycle(mCore);

				// Remove the object from the entity system.
				mSRObjects.erase(foundObject);

			}

			// Add vertex buffer objects.
      //-----------------------------------------------------------------
      char* vbo_buffer = 0;
      size_t stride_vbo = 0;
      //-----------------------------------------------------------------
      
			int nameIndex = 0;
			for (auto it = obj->mVBOs.cbegin(); it != obj->mVBOs.cend(); ++it)
			{
				const Core::Datatypes::GeometryObject::SpireVBO& vbo = *it;

				if (vbo.onGPU)
				{
					// Generate vector of attributes to pass into the entity system.
					std::vector<std::tuple<std::string, size_t, bool>> attributeData;
					for (const auto& attribData : vbo.attributes)
					{
						attributeData.push_back(std::make_tuple(attribData.name, attribData.sizeInBytes, attribData.normalize));
					}

					GLuint vboID = vboMan.addInMemoryVBO(vbo.data->getBuffer(), vbo.data->getBufferSize(),
						attributeData, vbo.name);
				}
        //-----------------------------------------------------------------
        vbo_buffer = reinterpret_cast<char*>(vbo.data->getBuffer());
        for (auto a : vbo.attributes)
          stride_vbo += a.sizeInBytes;
        //-----------------------------------------------------------------
				bbox.extend(vbo.boundingBox);
			}

			// Add index buffer objects.
			nameIndex = 0;
			for (auto it = obj->mIBOs.cbegin(); it != obj->mIBOs.cend(); ++it)
			{
				const Core::Datatypes::GeometryObject::SpireIBO& ibo = *it;
				GLenum primType = GL_UNSIGNED_SHORT;
				switch (ibo.indexSize)
				{
				case 1: // 8-bit
					primType = GL_UNSIGNED_BYTE;
					break;

				case 2: // 16-bit
					primType = GL_UNSIGNED_SHORT;
					break;

				case 4: // 32-bit
					primType = GL_UNSIGNED_INT;
					break;

				default:
					primType = GL_UNSIGNED_INT;
					throw std::invalid_argument("Unable to determine index buffer depth.");
					break;
				}

				GLenum primitive = GL_TRIANGLES;
				switch (ibo.prim)
				{
				case Core::Datatypes::GeometryObject::SpireIBO::POINTS:
					primitive = GL_POINTS;
					break;

				case Core::Datatypes::GeometryObject::SpireIBO::LINES:
					primitive = GL_LINES;
					break;

				case Core::Datatypes::GeometryObject::SpireIBO::TRIANGLES:
				default:
					primitive = GL_TRIANGLES;
					break;
				}

        //----------------------------------------------------------------
        uint32_t* ibo_buffer = reinterpret_cast<uint32_t*>(ibo.data->getBuffer());
        size_t num_triangles = ibo.data->getBufferSize() / (sizeof(uint32_t) * 3);
        Core::Geometry::Vector dir(0.0, 0.0, 0.0);

        std::vector<DepthIndex> rel_depth(num_triangles);
        for (int i = 0; i <= 3; ++i)
        {
          std::string name = ibo.name;
          
          if (i == 0)
          {
            dir = Core::Geometry::Vector(1.0, 0.0, 0.0);
            name += "X";
          }
          else if (i == 1)
          {
            dir = Core::Geometry::Vector(0.0, 1.0, 0.0);
            name += "Y";
          }
          else if (i == 2)
          {
            dir = Core::Geometry::Vector(0.0, 0.0, 1.0);
            name += "Z";
          }
          else if (i == 3)
          {
            int numPrimitives = ibo.data->getBufferSize() / ibo.indexSize;
            iboMan.addInMemoryIBO(ibo.data->getBuffer(), ibo.data->getBufferSize(), primitive, primType, numPrimitives, name);
            break;
          }

          for (size_t j = 0; j < num_triangles; j++)
          {
            float* vertex1 = reinterpret_cast<float*>(vbo_buffer + stride_vbo * (ibo_buffer[j * 3]));
            Core::Geometry::Point node1(vertex1[0], vertex1[1], vertex1[2]);

            float* vertex2 = reinterpret_cast<float*>(vbo_buffer + stride_vbo * (ibo_buffer[j * 3 + 1]));
            Core::Geometry::Point node2(vertex2[0], vertex2[1], vertex2[2]);

            float* vertex3 = reinterpret_cast<float*>(vbo_buffer + stride_vbo * (ibo_buffer[j * 3 + 2]));
            Core::Geometry::Point node3(vertex3[0], vertex3[1], vertex3[2]);

            rel_depth[j].mDepth = Core::Geometry::Dot(dir, node1) + Core::Geometry::Dot(dir, node2) + Core::Geometry::Dot(dir, node3);
            rel_depth[j].mIndex = j;
          }

          std::sort(rel_depth.begin(), rel_depth.end());

          int numPrimitives = ibo.data->getBufferSize() / ibo.indexSize;

          std::vector<char> sorted_buffer(ibo.data->getBufferSize());
          char* ibuffer = reinterpret_cast<char*>(ibo.data->getBuffer());
          char* sbuffer = reinterpret_cast<char*>(&sorted_buffer[0]);
          size_t tri_size = ibo.data->getBufferSize() / num_triangles;

          for (size_t j = 0; j < num_triangles; j++)
          {
            memcpy(sbuffer + j * tri_size, ibuffer + rel_depth[j].mIndex * tri_size, tri_size);
          }

          iboMan.addInMemoryIBO(sbuffer, ibo.data->getBufferSize(), primitive, primType, numPrimitives, name);
        }
        //----------------------------------------------------------------
       
				//int numPrimitives = ibo.data->getBufferSize() / ibo.indexSize;

				//iboMan.addInMemoryIBO(ibo.data->getBuffer(), ibo.data->getBufferSize(), primitive, primType, numPrimitives, ibo.name);
			}

			// Add default identity transform to the object globally (instead of per-pass)
			glm::mat4 xform;
			mSRObjects.push_back(SRObject(objectName, xform, bbox, obj->mColorMap, port));
			SRObject& elem = mSRObjects.back();

			ren::ShaderMan& shaderMan = *mCore.getStaticComponent<ren::StaticShaderMan>()->instance;

			// Add passes
			for (auto it = obj->mPasses.begin(); it != obj->mPasses.end(); ++it)
			{
				Core::Datatypes::GeometryObject::SpireSubPass& pass = *it;

				uint64_t entityID = getEntityIDForName(pass.passName, port);

				if (pass.renderType == Core::Datatypes::GeometryObject::RENDER_VBO_IBO)
				{   
          //reorderIBO(pass);
					addVBOToEntity(entityID, pass.vboName);
          for (int i = 0; i <= 3; ++i)
          {
            std::string name = pass.iboName;
            if (i == 1)
              name += "X";
            if (i == 2)
              name += "Y";
            if (i == 3)
              name += "Z";

            addIBOToEntity(entityID, name);
          }
				}
				else
				{
					// We will be constructing a render list from the VBO and IBO.
					RenderList list;

					for (auto it = obj->mVBOs.cbegin(); it != obj->mVBOs.cend(); ++it)
					{
						const Core::Datatypes::GeometryObject::SpireVBO& vbo = *it;
						if (vbo.name == pass.vboName)
						{
							list.data = vbo.data;
							list.attributes = vbo.attributes;
							list.renderType = pass.renderType;
							list.numElements = vbo.numElements;
							mCore.addComponent(entityID, list);
							break;
						}
					}

          // Lookup the VBOs and IBOs associated with this particular draw list
          // and add them to our entity in question.
          std::string assetName = "Assets/sphere.geom";

          if (pass.renderType == Core::Datatypes::GeometryObject::RENDER_RLIST_SPHERE)
          {
            assetName = "Assets/sphere.geom";
          }

          if (pass.renderType == Core::Datatypes::GeometryObject::RENDER_RLIST_CYLINDER)
          {
            assetName = "Assests/arrow.geom";
          }

          //reorderIBO(pass);
          addVBOToEntity(entityID, assetName);
          addIBOToEntity(entityID, assetName);
        }

        // Load vertex and fragment shader will use an already loaded program.
        //addShaderToEntity(entityID, pass.programName);
        shaderMan.loadVertexAndFragmentShader(mCore, entityID, pass.programName);

        // Add transformation
        gen::Transform trafo;

        if (pass.renderType == Core::Datatypes::GeometryObject::RENDER_RLIST_SPHERE)
        {
          double scale = pass.scalar;
          trafo.transform[0].x = scale;
          trafo.transform[1].y = scale;
          trafo.transform[2].z = scale;
        }
        mCore.addComponent(entityID, trafo);

        // Add lighting uniform checks
        LightingUniforms lightUniforms;
        mCore.addComponent(entityID, lightUniforms);

        // Add SCIRun render state.
        SRRenderState state;
        state.state = pass.renderState;
        mCore.addComponent(entityID, state);

        // Add appropriate renderer based on the color scheme to use.
        if (pass.mColorScheme == Core::Datatypes::GeometryObject::COLOR_UNIFORM
          || pass.mColorScheme == Core::Datatypes::GeometryObject::COLOR_IN_SITU)
        {
          RenderBasicGeom geom;
          mCore.addComponent(entityID, geom);
        }
        else if (pass.mColorScheme == Core::Datatypes::GeometryObject::COLOR_MAP
          && obj->mColorMap)
        {
          RenderColorMapGeom geom;
          mCore.addComponent(entityID, geom);

          // Construct texture component and add it to our entity for rendering.
          ren::Texture component;
          component.textureUnit = 0;
          component.setUniformName("uTX0");
          component.textureType = GL_TEXTURE_1D;

          // Setup appropriate texture to render the color map.
          if (*obj->mColorMap == "Rainbow")
          {
            component.glid = mRainbowCMap;
          }
          else
          {
            component.glid = mGrayscaleCMap;
          }
          mCore.addComponent(entityID, component);

          // Compare entity and system requirements.
          //mCore.displayEntityVersusSystemInfo(entityID, getSystemName_RenderColorMap());
        }
        else
        {
          std::cerr << "Renderer: Unknown color scheme!" << std::endl;
          RenderBasicGeom geom;
          mCore.addComponent(entityID, geom);
        }

        // Ensure common uniforms are covered.
        ren::CommonUniforms commonUniforms;
        mCore.addComponent(entityID, commonUniforms);

        for (const auto& uniform : pass.mUniforms)
        {
          applyUniform(entityID, uniform);
        }

        // Add components associated with entity. We just need a base class which
        // we can pass in an entity ID, then a derived class which bundles
        // all associated components (including types) together. We can use
        // a variadic template for this. This will allow us to place any components
        // we want on the objects in question in show field. This could lead to
        // much simpler customization.

        // Add a pass to our local object.
        elem.mPasses.emplace_back(pass.passName, pass.renderType);
      }

      // Recalculate scene bounding box. Should only be done when an object is added.
      mSceneBBox.reset();
      for (auto it = mSRObjects.begin(); it != mSRObjects.end(); ++it)
      {
        if (it->mBBox.valid())
        {
          mSceneBBox.extend(it->mBBox);
        }
      }
    }

		//------------------------------------------------------------------------------
		void SRInterface::addVBOToEntity(uint64_t entityID, const std::string& vboName)
		{
			ren::VBOMan& vboMan = *mCore.getStaticComponent<ren::StaticVBOMan>()->instance;
			ren::VBO vbo;

			vbo.glid = vboMan.hasVBO(vboName);

			mCore.addComponent(entityID, vbo);
		}

		//------------------------------------------------------------------------------
		void SRInterface::addIBOToEntity(uint64_t entityID, const std::string& iboName)
		{
			ren::IBOMan& iboMan = *mCore.getStaticComponent<ren::StaticIBOMan>()->instance;
			ren::IBO ibo;

			auto iboData = iboMan.getIBOData(iboName);

			ibo.glid = iboMan.hasIBO(iboName);
			ibo.primType = iboData.primType;
			ibo.primMode = iboData.primMode;
			ibo.numPrims = iboData.numPrims;
      
			mCore.addComponent(entityID, ibo);
		}

    //------------------------------------------------------------------------------
    void SRInterface::reorderIBO(Core::Datatypes::GeometryObject::SpireSubPass& pass)
    { 
      char* vbo_buffer = reinterpret_cast<char*>(pass.vbo.data->getBuffer());
      uint32_t* ibo_buffer = reinterpret_cast<uint32_t*>(pass.ibo.data->getBuffer());
      size_t num_triangles = pass.ibo.data->getBufferSize() / (sizeof(uint32_t) * 3);
      size_t stride_vbo = 0;
      for (auto a : pass.vbo.attributes)
        stride_vbo += a.sizeInBytes;

      std::vector<DepthIndex> rel_depth(num_triangles);
      Core::Geometry::Vector dir(mCamera->getViewToWorld()[0][2], mCamera->getViewToWorld()[1][2], mCamera->getViewToWorld()[2][2]);
      
      for (size_t j = 0; j < num_triangles; j++)
      {
        float* vertex1 = reinterpret_cast<float*>(vbo_buffer + stride_vbo * (ibo_buffer[j * 3]));
        Core::Geometry::Point node1(vertex1[0], vertex1[1], vertex1[2]);

        float* vertex2 = reinterpret_cast<float*>(vbo_buffer + stride_vbo * (ibo_buffer[j * 3 + 1]));
        Core::Geometry::Point node2(vertex2[0], vertex2[1], vertex2[2]);

        float* vertex3 = reinterpret_cast<float*>(vbo_buffer + stride_vbo * (ibo_buffer[j * 3 + 2]));
        Core::Geometry::Point node3(vertex3[0], vertex3[1], vertex3[2]);

        rel_depth[j].mDepth = Core::Geometry::Dot(dir, node1) + Core::Geometry::Dot(dir, node2) + Core::Geometry::Dot(dir, node3);
        rel_depth[j].mIndex = j;
      }

      std::sort(rel_depth.begin(), rel_depth.end());

      int numPrimitives = pass.ibo.data->getBufferSize() / pass.ibo.indexSize;

      std::vector<char> sorted_buffer(pass.ibo.data->getBufferSize());
      char* ibuffer = reinterpret_cast<char*>(pass.ibo.data->getBuffer());
      char* sbuffer = reinterpret_cast<char*>(&sorted_buffer[0]);
      size_t tri_size = pass.ibo.data->getBufferSize() / num_triangles;

      for (size_t j = 0; j < num_triangles; j++)
      {
        memcpy(sbuffer + j * tri_size, pass.ibo.data->getBuffer() + rel_depth[j].mIndex * tri_size, tri_size);
      }

      //ren::IBOMan& iboMan = *mCore.getStaticComponent<ren::StaticIBOMan>()->instance;

     // auto iboData = iboMan.getIBOData(pass.iboName);

      //iboMan.addInMemoryIBO(sbuffer, pass.ibo.data->getBufferSize(), iboData.primMode, iboData.primType, iboData.numPrims, pass.iboName);
    }

		//------------------------------------------------------------------------------
		void SRInterface::addShaderToEntity(uint64_t entityID, const std::string& shaderName)
		{
			ren::ShaderMan& shaderMan = *mCore.getStaticComponent<ren::StaticShaderMan>()->instance;
			ren::Shader shader;

			shader.glid = shaderMan.getIDForAsset(shaderName.c_str());

			mCore.addComponent(entityID, shader);
		}

		//------------------------------------------------------------------------------
		void SRInterface::applyUniform(uint64_t entityID, const Core::Datatypes::GeometryObject::SpireSubPass::Uniform& uniform)
		{
			switch (uniform.type)
			{
			case Core::Datatypes::GeometryObject::SpireSubPass::Uniform::UNIFORM_SCALAR:
				ren::addGLUniform(mCore, entityID, uniform.name.c_str(), static_cast<float>(uniform.data.x));
				break;

			case Core::Datatypes::GeometryObject::SpireSubPass::Uniform::UNIFORM_VEC4:
				ren::addGLUniform(mCore, entityID, uniform.name.c_str(), uniform.data);
				break;
			}
		}

		//------------------------------------------------------------------------------
		void SRInterface::removeAllGeomObjects()
		{
			mContext->makeCurrent();
			for (auto it = mSRObjects.begin(); it != mSRObjects.end(); ++it)
			{
				// Iterate through each of the passes and remove their associated
				// entity ID.
				for (const auto& pass : it->mPasses)
				{
					uint64_t entityID = getEntityIDForName(pass.passName, it->mPort);
					mCore.removeEntity(entityID);
				}
			}

			mCore.renormalize(true);

			mSRObjects.clear();
		}

		//------------------------------------------------------------------------------
		void SRInterface::gcInvalidObjects(const std::vector<std::string>& validObjects)
		{
			for (auto it = mSRObjects.begin(); it != mSRObjects.end();)
			{
				if (std::find(validObjects.begin(), validObjects.end(), it->mName) == validObjects.end())
				{
					for (const auto& pass : it->mPasses)
					{
						uint64_t entityID = getEntityIDForName(pass.passName, it->mPort);
						mCore.removeEntity(entityID);
					}
					it = mSRObjects.erase(it);
				}
				else
				{
					++it;
				}
			}

			mCore.renormalize(true);
		}

		//------------------------------------------------------------------------------
		void SRInterface::doFrame(double currentTime, double constantDeltaTime)
		{
			/// \todo Only render a frame if something has changed (new or deleted
			///       objects, or the view point has changed).

			mContext->makeCurrent();

			updateCamera();
			updateWorldLight();

			mCore.execute(currentTime, constantDeltaTime);

			if (showOrientation_)
			{
				// Do not even attempt to render if the framebuffer is not complete.
				// This can happen when the rendering window is hidden (in SCIRun5 for
				// example);
				if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
				{
					renderCoordinateAxes();
				}
			}


			// Set directional light source (in world space).
			// Need to set a static light source component which will be used when
			// uLightDirWorld is requested from a shader.
			// glm::vec3 viewDir = viewToWorld[2].xyz();
			// viewDir = -viewDir; // Cameras look down -Z.
			//mSpire->addGlobalUniform("uLightDirWorld", viewDir);
		}

		//------------------------------------------------------------------------------
		void SRInterface::updateCamera()
		{
			// Update the static camera with the appropriate world to view transform.
			mCamera->applyTransform();
			glm::mat4 viewToWorld = mCamera->getViewToWorld();

			gen::StaticCamera* camera = mCore.getStaticComponent<gen::StaticCamera>();
			if (camera)
			{
				camera->data.setView(viewToWorld);
			}
		}

		//------------------------------------------------------------------------------
		void SRInterface::updateWorldLight()
		{
			glm::mat4 viewToWorld = mCamera->getViewToWorld();

			// Set directional light source (in world space).
			StaticWorldLight* light = mCore.getStaticComponent<StaticWorldLight>();
			if (light)
			{
				glm::vec3 viewDir = viewToWorld[2].xyz();
				viewDir = -viewDir; // Cameras look down -Z.
				light->lightDir = viewDir;
			}
		}

		//------------------------------------------------------------------------------
		void SRInterface::renderCoordinateAxes()
		{
			// Only execute if static rendering resources are available. All of these
			// resource checks wouldn't be necessary if we were operating in the perview
			// of the entity system.
			if (mCore.getStaticComponent<ren::StaticVBOMan>() == nullptr) return;

			// This rendering algorithm is fairly inefficient. Use the entity component
			// system to optimize the rendering of a large amount of objects.
			ren::VBOMan& vboMan = *mCore.getStaticComponent<ren::StaticVBOMan>()->instance;
			ren::IBOMan& iboMan = *mCore.getStaticComponent<ren::StaticIBOMan>()->instance;
			ren::ShaderMan& shaderMan = *mCore.getStaticComponent<ren::StaticShaderMan>()->instance;

			GLuint arrowVBO = vboMan.hasVBO("Assets/arrow.geom");
			GLuint arrowIBO = iboMan.hasIBO("Assets/arrow.geom");
			GLuint shader = shaderMan.getIDForAsset("Shaders/DirPhong");

			// Bail if assets have not been loaded yet (asynchronous loading may take a
			// few frames).
			if (arrowVBO == 0 || arrowIBO == 0 || shader == 0) { return; }

			const ren::IBOMan::IBOData* iboData;
			try
			{
				iboData = &iboMan.getIBOData("Assets/arrow.geom");
			}
			catch (...)
			{
				// Return if IBO data not available.
				return;
			}

			// Ensure shader attributes are setup appropriately.
			mArrowAttribs.setup(arrowVBO, shader, vboMan);

			glm::mat4 trafo;

			GL(glUseProgram(shader));

			GL(glBindBuffer(GL_ARRAY_BUFFER, arrowVBO));
			GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, arrowIBO));

      bool depthMask = glIsEnabled(GL_DEPTH_WRITEMASK);
      bool cullFace = glIsEnabled(GL_CULL_FACE);
      bool blend = glIsEnabled(GL_BLEND);

      GL(glDepthMask(GL_TRUE));
      GL(glDisable(GL_CULL_FACE));
      GL(glDisable(GL_BLEND));

			// Note that we can pull aspect ratio from the screen dimensions static
			// variable.
			gen::StaticScreenDims* dims = mCore.getStaticComponent<gen::StaticScreenDims>();
			float aspect = static_cast<float>(dims->width) / static_cast<float>(dims->height);
			glm::mat4 projection = glm::perspective(0.59f, aspect, 1.0f, 2000.0f);

			// Build world transform for all axes. Rotates about uninverted camera's
			// view, then translates to a specified corner on the screen.
			glm::mat4 axesRot = mCamera->getWorldToView();
			axesRot[3][0] = 0.0f;
			axesRot[3][1] = 0.0f;
			axesRot[3][2] = 0.0f;
			glm::mat4 invCamTrans = glm::translate(glm::mat4(1.0f), glm::vec3(0.375f * aspect, 0.37f, -1.5f));
			glm::mat4 axesScale = glm::scale(glm::mat4(1.0f), glm::vec3(0.8f));
			glm::mat4 axesTransform = axesScale * axesRot;

			GLint locCamViewVec = glGetUniformLocation(shader, "uCamViewVec");
			GLint locLightDirWorld = glGetUniformLocation(shader, "uLightDirWorld");

			GLint locAmbientColor = glGetUniformLocation(shader, "uAmbientColor");
			GLint locDiffuseColor = glGetUniformLocation(shader, "uDiffuseColor");
			GLint locSpecularColor = glGetUniformLocation(shader, "uSpecularColor");
			GLint locSpecularPower = glGetUniformLocation(shader, "uSpecularPower");

			GLint locProjIVObject = glGetUniformLocation(shader, "uProjIVObject");
			GLint locObject = glGetUniformLocation(shader, "uObject");

			GL(glUniform3f(locCamViewVec, 0.0f, 0.0f, -1.0f));
			GL(glUniform3f(locLightDirWorld, 0.0f, 0.0f, -1.0f));

			// Build projection for the axes to use on the screen. The arrors will not
			// use the camera, but will use the camera's transformation matrix.

			mArrowAttribs.bind();

			// X Axis
			{
				glm::mat4 xform = glm::rotate(glm::mat4(1.0f), glm::pi<float>() / 2.0f, glm::vec3(0.0, 1.0, 0.0));
				glm::mat4 finalTrafo = axesTransform * xform;

				GL(glUniform4f(locAmbientColor, 0.5f, 0.01f, 0.01f, 1.0f));
				GL(glUniform4f(locDiffuseColor, 1.0f, 0.0f, 0.0f, 1.0f));
				GL(glUniform4f(locSpecularColor, 0.5f, 0.5f, 0.5f, 1.0f));
				GL(glUniform1f(locSpecularPower, 16.0f));

				glm::mat4 worldToProj = projection * invCamTrans * finalTrafo;
				const GLfloat* ptr = glm::value_ptr(worldToProj);
				GL(glUniformMatrix4fv(locProjIVObject, 1, false, ptr));

				glm::mat4 objectSpace = finalTrafo;
				ptr = glm::value_ptr(objectSpace);
				GL(glUniformMatrix4fv(locObject, 1, false, ptr));

				GL(glDrawElements(iboData->primMode, iboData->numPrims, iboData->primType, 0));
			}

			// X Axis (dark)
  {
	  glm::mat4 xform = glm::rotate(glm::mat4(1.0f), -glm::pi<float>() / 2.0f, glm::vec3(0.0, 1.0, 0.0));
	  glm::mat4 finalTrafo = axesTransform * xform;

	  GL(glUniform4f(locAmbientColor, 0.1f, 0.01f, 0.01f, 1.0f));
	  GL(glUniform4f(locDiffuseColor, 0.25f, 0.0f, 0.0f, 1.0f));
	  GL(glUniform4f(locSpecularColor, 0.0f, 0.0f, 0.0f, 1.0f));
	  GL(glUniform1f(locSpecularPower, 16.0f));

	  glm::mat4 worldToProj = projection * invCamTrans * finalTrafo;
	  const GLfloat* ptr = glm::value_ptr(worldToProj);
	  GL(glUniformMatrix4fv(locProjIVObject, 1, false, ptr));

	  glm::mat4 objectSpace = finalTrafo;
	  ptr = glm::value_ptr(objectSpace);
	  GL(glUniformMatrix4fv(locObject, 1, false, ptr));

	  GL(glDrawElements(iboData->primMode, iboData->numPrims, iboData->primType, 0));
  }

			// Y Axis
  {
	  glm::mat4 xform = glm::rotate(glm::mat4(1.0f), -glm::pi<float>() / 2.0f, glm::vec3(1.0, 0.0, 0.0));
	  glm::mat4 finalTrafo = axesTransform * xform;

	  GL(glUniform4f(locAmbientColor, 0.01f, 0.5f, 0.01f, 1.0f));
	  GL(glUniform4f(locDiffuseColor, 0.0f, 1.0f, 0.0f, 1.0f));
	  GL(glUniform4f(locSpecularColor, 0.5f, 0.5f, 0.5f, 1.0f));
	  GL(glUniform1f(locSpecularPower, 16.0f));

	  glm::mat4 worldToProj = projection * invCamTrans * finalTrafo;
	  const GLfloat* ptr = glm::value_ptr(worldToProj);
	  GL(glUniformMatrix4fv(locProjIVObject, 1, false, ptr));

	  glm::mat4 objectSpace = finalTrafo;
	  ptr = glm::value_ptr(objectSpace);
	  GL(glUniformMatrix4fv(locObject, 1, false, ptr));

	  GL(glDrawElements(iboData->primMode, iboData->numPrims, iboData->primType, 0));
  }

			// Y Axis (dark)
  {
	  glm::mat4 xform = glm::rotate(glm::mat4(1.0f), glm::pi<float>() / 2.0f, glm::vec3(1.0, 0.0, 0.0));
	  glm::mat4 finalTrafo = axesTransform * xform;

	  GL(glUniform4f(locAmbientColor, 0.01f, 0.1f, 0.01f, 1.0f));
	  GL(glUniform4f(locDiffuseColor, 0.0f, 0.25f, 0.0f, 1.0f));
	  GL(glUniform4f(locSpecularColor, 0.0f, 0.0f, 0.0f, 1.0f));
	  GL(glUniform1f(locSpecularPower, 16.0f));

	  glm::mat4 worldToProj = projection * invCamTrans * finalTrafo;
	  const GLfloat* ptr = glm::value_ptr(worldToProj);
	  GL(glUniformMatrix4fv(locProjIVObject, 1, false, ptr));

	  glm::mat4 objectSpace = finalTrafo;
	  ptr = glm::value_ptr(objectSpace);
	  GL(glUniformMatrix4fv(locObject, 1, false, ptr));

	  GL(glDrawElements(iboData->primMode, iboData->numPrims, iboData->primType, 0));
  }

			// Z Axis
  {
	  // No rotation at all
	  glm::mat4 finalTrafo = axesTransform;

	  GL(glUniform4f(locAmbientColor, 0.01f, 0.01f, 0.5f, 1.0f));
	  GL(glUniform4f(locDiffuseColor, 0.0f, 0.0f, 1.0f, 1.0f));
	  GL(glUniform4f(locSpecularColor, 0.5f, 0.5f, 0.5f, 1.0f));
	  GL(glUniform1f(locSpecularPower, 16.0f));

	  glm::mat4 worldToProj = projection * invCamTrans * finalTrafo;
	  const GLfloat* ptr = glm::value_ptr(worldToProj);
	  GL(glUniformMatrix4fv(locProjIVObject, 1, false, ptr));

	  glm::mat4 objectSpace = finalTrafo;
	  ptr = glm::value_ptr(objectSpace);
	  GL(glUniformMatrix4fv(locObject, 1, false, ptr));

	  GL(glDrawElements(iboData->primMode, iboData->numPrims, iboData->primType, 0));
  }

			// Z Axis (dark)
  {
	  // No rotation at all
	  glm::mat4 xform = glm::rotate(glm::mat4(1.0f), glm::pi<float>(), glm::vec3(1.0, 0.0, 0.0));
	  glm::mat4 finalTrafo = axesTransform * xform;

	  GL(glUniform4f(locAmbientColor, 0.01f, 0.01f, 0.1f, 1.0f));
	  GL(glUniform4f(locDiffuseColor, 0.0f, 0.0f, 0.25f, 1.0f));
	  GL(glUniform4f(locSpecularColor, 0.0f, 0.0f, 0.0f, 1.0f));
	  GL(glUniform1f(locSpecularPower, 16.0f));

	  glm::mat4 worldToProj = projection * invCamTrans * finalTrafo;
	  const GLfloat* ptr = glm::value_ptr(worldToProj);
	  GL(glUniformMatrix4fv(locProjIVObject, 1, false, ptr));

	  glm::mat4 objectSpace = finalTrafo;
	  ptr = glm::value_ptr(objectSpace);
	  GL(glUniformMatrix4fv(locObject, 1, false, ptr));

	  GL(glDrawElements(iboData->primMode, iboData->numPrims, iboData->primType, 0));
  }

			mArrowAttribs.unbind();

      if (!depthMask)
      {
        GL(glDepthMask(GL_FALSE));
      }
      if (cullFace)
      {
        GL(glEnable(GL_CULL_FACE));
      }
      if (blend)
      {
        GL(glEnable(GL_BLEND));
      }
		}

		// Manually update the StaticCamera.

		float rainbowRaw[] =
		{
			0.0000, 0.0000, 1.0000, 1.0,
			0.0000, 0.0216, 1.0000, 1.0,
			0.0000, 0.0432, 1.0000, 1.0,
			0.0000, 0.0648, 1.0000, 1.0,
			0.0000, 0.0864, 1.0000, 1.0,
			0.0000, 0.1080, 1.0000, 1.0,
			0.0000, 0.1296, 1.0000, 1.0,
			0.0000, 0.1511, 1.0000, 1.0,
			0.0000, 0.1727, 1.0000, 1.0,
			0.0000, 0.1943, 1.0000, 1.0,
			0.0002, 0.2104, 1.0000, 1.0,
			0.0006, 0.2220, 1.0000, 1.0,
			0.0011, 0.2336, 1.0000, 1.0,
			0.0015, 0.2453, 1.0000, 1.0,
			0.0019, 0.2569, 1.0000, 1.0,
			0.0023, 0.2685, 1.0000, 1.0,
			0.0027, 0.2801, 1.0000, 1.0,
			0.0031, 0.2918, 1.0000, 1.0,
			0.0036, 0.3034, 1.0000, 1.0,
			0.0040, 0.3149, 1.0000, 1.0,
			0.0048, 0.3253, 1.0000, 1.0,
			0.0057, 0.3356, 1.0000, 1.0,
			0.0065, 0.3460, 1.0000, 1.0,
			0.0073, 0.3564, 1.0000, 1.0,
			0.0082, 0.3668, 1.0000, 1.0,
			0.0090, 0.3772, 1.0000, 1.0,
			0.0098, 0.3875, 1.0000, 1.0,
			0.0107, 0.3979, 1.0000, 1.0,
			0.0115, 0.4083, 1.0000, 1.0,
			0.0123, 0.4192, 1.0000, 1.0,
			0.0131, 0.4304, 1.0000, 1.0,
			0.0140, 0.4417, 1.0000, 1.0,
			0.0148, 0.4529, 1.0000, 1.0,
			0.0156, 0.4641, 1.0000, 1.0,
			0.0165, 0.4753, 1.0000, 1.0,
			0.0173, 0.4865, 1.0000, 1.0,
			0.0181, 0.4977, 1.0000, 1.0,
			0.0190, 0.5089, 1.0000, 1.0,
			0.0200, 0.5200, 0.9989, 1.0,
			0.0216, 0.5303, 0.9939, 1.0,
			0.0233, 0.5407, 0.9889, 1.0,
			0.0250, 0.5511, 0.9839, 1.0,
			0.0266, 0.5615, 0.9790, 1.0,
			0.0283, 0.5719, 0.9740, 1.0,
			0.0299, 0.5822, 0.9690, 1.0,
			0.0316, 0.5926, 0.9640, 1.0,
			0.0333, 0.6030, 0.9590, 1.0,
			0.0349, 0.6134, 0.9540, 1.0,
			0.0359, 0.6221, 0.9433, 1.0,
			0.0368, 0.6304, 0.9308, 1.0,
			0.0376, 0.6388, 0.9183, 1.0,
			0.0384, 0.6471, 0.9059, 1.0,
			0.0393, 0.6554, 0.8934, 1.0,
			0.0401, 0.6637, 0.8810, 1.0,
			0.0409, 0.6720, 0.8685, 1.0,
			0.0418, 0.6803, 0.8561, 1.0,
			0.0426, 0.6886, 0.8436, 1.0,
			0.0437, 0.6963, 0.8310, 1.0,
			0.0454, 0.7030, 0.8181, 1.0,
			0.0470, 0.7096, 0.8053, 1.0,
			0.0487, 0.7163, 0.7924, 1.0,
			0.0503, 0.7229, 0.7795, 1.0,
			0.0520, 0.7296, 0.7666, 1.0,
			0.0537, 0.7362, 0.7538, 1.0,
			0.0553, 0.7428, 0.7409, 1.0,
			0.0570, 0.7495, 0.7280, 1.0,
			0.0586, 0.7561, 0.7152, 1.0,
			0.0610, 0.7631, 0.7027, 1.0,
			0.0635, 0.7702, 0.6902, 1.0,
			0.0660, 0.7773, 0.6777, 1.0,
			0.0685, 0.7843, 0.6653, 1.0,
			0.0710, 0.7914, 0.6528, 1.0,
			0.0735, 0.7984, 0.6404, 1.0,
			0.0760, 0.8055, 0.6279, 1.0,
			0.0785, 0.8125, 0.6155, 1.0,
			0.0810, 0.8196, 0.6030, 1.0,
			0.0840, 0.8263, 0.5913, 1.0,
			0.0878, 0.8325, 0.5805, 1.0,
			0.0915, 0.8388, 0.5697, 1.0,
			0.0952, 0.8450, 0.5589, 1.0,
			0.0990, 0.8512, 0.5481, 1.0,
			0.1027, 0.8574, 0.5373, 1.0,
			0.1064, 0.8637, 0.5265, 1.0,
			0.1102, 0.8699, 0.5157, 1.0,
			0.1139, 0.8761, 0.5049, 1.0,
			0.1176, 0.8824, 0.4941, 1.0,
			0.1226, 0.8873, 0.4842, 1.0,
			0.1276, 0.8923, 0.4742, 1.0,
			0.1326, 0.8973, 0.4642, 1.0,
			0.1376, 0.9023, 0.4543, 1.0,
			0.1426, 0.9073, 0.4443, 1.0,
			0.1475, 0.9122, 0.4343, 1.0,
			0.1525, 0.9172, 0.4244, 1.0,
			0.1575, 0.9222, 0.4144, 1.0,
			0.1625, 0.9272, 0.4044, 1.0,
			0.1689, 0.9319, 0.3954, 1.0,
			0.1763, 0.9365, 0.3871, 1.0,
			0.1838, 0.9411, 0.3788, 1.0,
			0.1913, 0.9457, 0.3705, 1.0,
			0.1988, 0.9502, 0.3622, 1.0,
			0.2062, 0.9548, 0.3539, 1.0,
			0.2137, 0.9594, 0.3456, 1.0,
			0.2212, 0.9639, 0.3373, 1.0,
			0.2287, 0.9685, 0.3290, 1.0,
			0.2365, 0.9729, 0.3206, 1.0,
			0.2478, 0.9758, 0.3123, 1.0,
			0.2590, 0.9787, 0.3040, 1.0,
			0.2702, 0.9816, 0.2957, 1.0,
			0.2814, 0.9845, 0.2874, 1.0,
			0.2926, 0.9874, 0.2791, 1.0,
			0.3038, 0.9903, 0.2708, 1.0,
			0.3150, 0.9932, 0.2625, 1.0,
			0.3262, 0.9961, 0.2542, 1.0,
			0.3374, 0.9990, 0.2459, 1.0,
			0.3492, 1.0000, 0.2395, 1.0,
			0.3612, 1.0000, 0.2341, 1.0,
			0.3733, 1.0000, 0.2287, 1.0,
			0.3853, 1.0000, 0.2233, 1.0,
			0.3974, 1.0000, 0.2179, 1.0,
			0.4094, 1.0000, 0.2125, 1.0,
			0.4215, 1.0000, 0.2072, 1.0,
			0.4335, 1.0000, 0.2018, 1.0,
			0.4455, 1.0000, 0.1964, 1.0,
			0.4579, 0.9997, 0.1910, 1.0,
			0.4711, 0.9985, 0.1861, 1.0,
			0.4844, 0.9972, 0.1811, 1.0,
			0.4977, 0.9960, 0.1761, 1.0,
			0.5110, 0.9947, 0.1711, 1.0,
			0.5243, 0.9935, 0.1661, 1.0,
			0.5376, 0.9922, 0.1612, 1.0,
			0.5509, 0.9910, 0.1562, 1.0,
			0.5642, 0.9898, 0.1512, 1.0,
			0.5774, 0.9885, 0.1462, 1.0,
			0.5901, 0.9853, 0.1419, 1.0,
			0.6025, 0.9816, 0.1377, 1.0,
			0.6150, 0.9779, 0.1336, 1.0,
			0.6275, 0.9741, 0.1294, 1.0,
			0.6399, 0.9704, 0.1253, 1.0,
			0.6524, 0.9666, 0.1211, 1.0,
			0.6648, 0.9629, 0.1170, 1.0,
			0.6773, 0.9592, 0.1128, 1.0,
			0.6897, 0.9554, 0.1087, 1.0,
			0.7012, 0.9516, 0.1048, 1.0,
			0.7108, 0.9474, 0.1015, 1.0,
			0.7203, 0.9433, 0.0981, 1.0,
			0.7299, 0.9391, 0.0948, 1.0,
			0.7394, 0.9349, 0.0915, 1.0,
			0.7490, 0.9308, 0.0882, 1.0,
			0.7585, 0.9266, 0.0848, 1.0,
			0.7681, 0.9225, 0.0815, 1.0,
			0.7776, 0.9183, 0.0782, 1.0,
			0.7872, 0.9142, 0.0749, 1.0,
			0.7952, 0.9089, 0.0727, 1.0,
			0.8031, 0.9035, 0.0706, 1.0,
			0.8110, 0.8981, 0.0685, 1.0,
			0.8189, 0.8927, 0.0664, 1.0,
			0.8268, 0.8873, 0.0644, 1.0,
			0.8347, 0.8819, 0.0623, 1.0,
			0.8426, 0.8765, 0.0602, 1.0,
			0.8505, 0.8711, 0.0581, 1.0,
			0.8584, 0.8657, 0.0561, 1.0,
			0.8657, 0.8602, 0.0542, 1.0,
			0.8723, 0.8543, 0.0525, 1.0,
			0.8790, 0.8485, 0.0508, 1.0,
			0.8856, 0.8427, 0.0492, 1.0,
			0.8923, 0.8369, 0.0475, 1.0,
			0.8989, 0.8311, 0.0459, 1.0,
			0.9056, 0.8253, 0.0442, 1.0,
			0.9122, 0.8195, 0.0425, 1.0,
			0.9188, 0.8137, 0.0409, 1.0,
			0.9255, 0.8078, 0.0392, 1.0,
			0.9301, 0.7991, 0.0384, 1.0,
			0.9346, 0.7904, 0.0376, 1.0,
			0.9392, 0.7817, 0.0367, 1.0,
			0.9438, 0.7730, 0.0359, 1.0,
			0.9483, 0.7642, 0.0351, 1.0,
			0.9529, 0.7555, 0.0342, 1.0,
			0.9575, 0.7468, 0.0334, 1.0,
			0.9620, 0.7381, 0.0326, 1.0,
			0.9666, 0.7294, 0.0317, 1.0,
			0.9700, 0.7223, 0.0307, 1.0,
			0.9725, 0.7164, 0.0294, 1.0,
			0.9750, 0.7106, 0.0282, 1.0,
			0.9775, 0.7048, 0.0269, 1.0,
			0.9800, 0.6990, 0.0257, 1.0,
			0.9825, 0.6932, 0.0245, 1.0,
			0.9850, 0.6874, 0.0232, 1.0,
			0.9875, 0.6816, 0.0220, 1.0,
			0.9899, 0.6758, 0.0207, 1.0,
			0.9922, 0.6697, 0.0195, 1.0,
			0.9931, 0.6614, 0.0187, 1.0,
			0.9939, 0.6531, 0.0179, 1.0,
			0.9947, 0.6448, 0.0170, 1.0,
			0.9956, 0.6364, 0.0162, 1.0,
			0.9964, 0.6281, 0.0154, 1.0,
			0.9972, 0.6198, 0.0145, 1.0,
			0.9981, 0.6115, 0.0137, 1.0,
			0.9989, 0.6032, 0.0129, 1.0,
			0.9997, 0.5949, 0.0120, 1.0,
			1.0000, 0.5863, 0.0115, 1.0,
			1.0000, 0.5776, 0.0111, 1.0,
			1.0000, 0.5689, 0.0107, 1.0,
			1.0000, 0.5602, 0.0102, 1.0,
			1.0000, 0.5515, 0.0098, 1.0,
			1.0000, 0.5427, 0.0094, 1.0,
			1.0000, 0.5340, 0.0090, 1.0,
			1.0000, 0.5253, 0.0086, 1.0,
			1.0000, 0.5166, 0.0082, 1.0,
			1.0000, 0.5081, 0.0078, 1.0,
			1.0000, 0.5007, 0.0073, 1.0,
			1.0000, 0.4932, 0.0069, 1.0,
			1.0000, 0.4857, 0.0065, 1.0,
			1.0000, 0.4782, 0.0061, 1.0,
			1.0000, 0.4708, 0.0057, 1.0,
			1.0000, 0.4633, 0.0053, 1.0,
			1.0000, 0.4558, 0.0048, 1.0,
			1.0000, 0.4484, 0.0044, 1.0,
			1.0000, 0.4409, 0.0040, 1.0,
			1.0000, 0.4334, 0.0036, 1.0,
			1.0000, 0.4259, 0.0032, 1.0,
			1.0000, 0.4185, 0.0028, 1.0,
			1.0000, 0.4110, 0.0024, 1.0,
			1.0000, 0.4035, 0.0019, 1.0,
			1.0000, 0.3960, 0.0015, 1.0,
			1.0000, 0.3886, 0.0011, 1.0,
			1.0000, 0.3811, 0.0007, 1.0,
			1.0000, 0.3736, 0.0003, 1.0,
			1.0000, 0.3661, 0.0000, 1.0,
			1.0000, 0.3587, 0.0000, 1.0,
			1.0000, 0.3512, 0.0000, 1.0,
			1.0000, 0.3437, 0.0000, 1.0,
			1.0000, 0.3362, 0.0000, 1.0,
			1.0000, 0.3288, 0.0000, 1.0,
			1.0000, 0.3213, 0.0000, 1.0,
			1.0000, 0.3138, 0.0000, 1.0,
			1.0000, 0.3063, 0.0000, 1.0,
			1.0000, 0.2989, 0.0000, 1.0,
			1.0000, 0.2903, 0.0000, 1.0,
			1.0000, 0.2816, 0.0000, 1.0,
			1.0000, 0.2728, 0.0000, 1.0,
			1.0000, 0.2641, 0.0000, 1.0,
			1.0000, 0.2554, 0.0000, 1.0,
			1.0000, 0.2467, 0.0000, 1.0,
			1.0000, 0.2380, 0.0000, 1.0,
			1.0000, 0.2293, 0.0000, 1.0,
			1.0000, 0.2205, 0.0000, 1.0,
			1.0000, 0.2055, 0.0000, 1.0,
			1.0000, 0.1827, 0.0000, 1.0,
			1.0000, 0.1599, 0.0000, 1.0,
			1.0000, 0.1370, 0.0000, 1.0,
			1.0000, 0.1142, 0.0000, 1.0,
			1.0000, 0.0913, 0.0000, 1.0,
			1.0000, 0.0685, 0.0000, 1.0,
			1.0000, 0.0457, 0.0000, 1.0,
			1.0000, 0.0228, 0.0000, 1.0,
			1.0000, 0.0000, 0.0000, 1.0
		};

		// Create default colormaps.
		void SRInterface::generateColormaps()
		{
			size_t rainbowArraySize = sizeof(rainbowRaw) / sizeof(*rainbowRaw);

			std::vector<uint8_t> rainbow;
			rainbow.reserve(rainbowArraySize);
			for (int i = 0; i < rainbowArraySize; i++)
			{
				rainbow.push_back(static_cast<uint8_t>(rainbowRaw[i] * 255.0f));
			}

			// Build rainbow texture (eyetracking version -- will need to change).
			GL(glGenTextures(1, &mRainbowCMap));
			GL(glBindTexture(GL_TEXTURE_1D, mRainbowCMap));
			GL(glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
			GL(glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
			GL(glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
			GL(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
			GL(glPixelStorei(GL_PACK_ALIGNMENT, 1));
			GL(glTexImage1D(GL_TEXTURE_1D, 0,
				GL_RGBA8,
				static_cast<GLsizei>(rainbow.size() / 4), 0,
				GL_RGBA,
				GL_UNSIGNED_BYTE, &rainbow[0]));

			// build grayscale texture.
			const int grayscaleSize = 255 * 4;
			std::vector<uint8_t> grayscale;
			grayscale.reserve(grayscaleSize);
			for (int i = 0; i < 255; i++)
			{
				grayscale.push_back(i);
				grayscale.push_back(i);
				grayscale.push_back(i);
				grayscale.push_back(255);
			}

			// Grayscale texture.
			GL(glGenTextures(1, &mGrayscaleCMap));
			GL(glBindTexture(GL_TEXTURE_1D, mGrayscaleCMap));
			GL(glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
			GL(glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
			GL(glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
			GL(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
			GL(glPixelStorei(GL_PACK_ALIGNMENT, 1));
			GL(glTexImage1D(GL_TEXTURE_1D, 0,
				GL_RGBA8,
				static_cast<GLsizei>(grayscale.size() / 4), 0,
				GL_RGBA,
				GL_UNSIGNED_BYTE, &grayscale[0]));
		}


	} // namespace Render
} // namespace SCIRun 

