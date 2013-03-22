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

#include <Interface/Modules/Render/ViewScene.h>
#include <Dataflow/Network/ModuleStateInterface.h>
#include <Core/Datatypes/Geometry.h>
#include <QFileDialog>

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Datatypes;

//------------------------------------------------------------------------------
ViewSceneDialog::ViewSceneDialog(const std::string& name, ModuleStateHandle state,
  QWidget* parent /* = 0 */)
  : ModuleDialogGeneric(state, parent)
{
  setupUi(this);
  setWindowTitle(QString::fromStdString(name));

  // Setup Qt OpenGL widget.
  QGLFormat fmt;
  fmt.setAlpha(true);
  fmt.setRgba(true);
  mGLWidget = new GLWidget(fmt);

  if (mGLWidget->isValid())
  {
    // Hook up the GLWidget
    glLayout->addWidget(mGLWidget);
    glLayout->update();

    // Set spire transient value (should no longer be used).
    mSpire = std::weak_ptr<Spire::SCIRun::SRInterface>(mGLWidget->getSpire());
  }
  else
  {
    /// \todo Display dialog.
    delete mGLWidget;
  }
}

//------------------------------------------------------------------------------
ViewSceneDialog::~ViewSceneDialog()
{
}

//------------------------------------------------------------------------------
void ViewSceneDialog::moduleExecuted()
{
  // Grab the geomData transient value.
  boost::any geomDataTransient = state_->getTransientValue("geomData");
  if (!geomDataTransient.empty())
  {
    boost::shared_ptr<std::list<boost::shared_ptr<Core::Datatypes::GeometryObject>>> geomData; // Whoa...
    try
    {
      geomData = boost::any_cast<boost::shared_ptr<std::list<boost::shared_ptr<Core::Datatypes::GeometryObject>>>>(geomDataTransient);
    }
    catch (const boost::bad_any_cast&)
    {
      //error("Unable to cast boost::any transient value to spire pointer.");
      return;
    }
    std::shared_ptr<Spire::SCIRun::SRInterface> spire = mSpire.lock();
    if (spire == nullptr)
      return;

    // Grab objects (to be regenerated with supplied data)
    std::shared_ptr<Spire::StuInterface> stuPipe = spire->getStuPipe();

    // Remove ALL prior objects.
    stuPipe->removeAllObjects();

    for (auto it = geomData->begin(); it != geomData->end(); ++it)
    {
      boost::shared_ptr<Core::Datatypes::GeometryObject> obj = *it;

      // Try/catch is for single-threaded cases. Exceptions are handled on the
      // spire thread when spire is threaded.
      try
      {
        // Will remove all traces of old VBO's / IBO's not in use.
        // (remember, we remove the VBOs/IBOs we added at the end of this loop,
        //  this is to ensure there is only 1 shared_ptr reference to the IBOs
        //  and VBOs in Spire).
        stuPipe->removeObject(obj->objectName);
      }
      catch (std::out_of_range& e)
      {
        // Ignore
      }

      stuPipe->addObject(obj->objectName);

      // Add vertex buffer objects.
      for (auto it = obj->mVBOs.cbegin(); it != obj->mVBOs.cend(); ++it)
      {
        const GeometryObject::SpireVBO& vbo = *it;
        stuPipe->addVBO(vbo.name, vbo.data, vbo.attributeNames);
      }

      // Add index buffer objects.
      for (auto it = obj->mIBOs.cbegin(); it != obj->mIBOs.cend(); ++it)
      {
        const GeometryObject::SpireIBO& ibo = *it;
        Spire::StuInterface::IBO_TYPE type;
        switch (ibo.indexSize)
        {
          case 1: // 8-bit
            type = Spire::StuInterface::IBO_8BIT;
            break;

          case 2: // 16-bit
            type = Spire::StuInterface::IBO_16BIT;
            break;

          case 4: // 32-bit
            type = Spire::StuInterface::IBO_32BIT;
            break;

          default:
            type = Spire::StuInterface::IBO_32BIT;
            throw std::invalid_argument("Unable to determine index buffer depth.");
            break;
        }
        stuPipe->addIBO(ibo.name, ibo.data, type);
      }

      // Add passes
      for (auto it = obj->mPasses.cbegin(); it != obj->mPasses.cend(); ++it)
      {
        const GeometryObject::SpirePass& pass = *it;
        stuPipe->addPassToObject(obj->objectName, pass.passName, pass.programName,
                                 pass.vboName, pass.iboName, pass.type);

        // Add uniforms associated with the pass
        for (auto it = pass.uniforms.begin(); it != pass.uniforms.end(); ++it)
        {
          std::string uniformName = std::get<0>(*it);
          std::shared_ptr<Spire::AbstractUniformStateItem> uniform(std::get<1>(*it));
          stuPipe->addPassUniformConcrete(obj->objectName, pass.passName,
                                          uniformName, uniform);
        }

        // Add gpu state if it has been set.
        if (pass.hasGPUState == true)
          stuPipe->addPassGPUState(obj->objectName, pass.passName, pass.gpuState);
      }

      // Now that we have created all of the appropriate passes, get rid of the
      // VBOs and IBOs.
      for (auto it = obj->mVBOs.cbegin(); it != obj->mVBOs.cend(); ++it)
      {
        const GeometryObject::SpireVBO& vbo = *it;
        stuPipe->removeVBO(vbo.name);
      }

      for (auto it = obj->mIBOs.cbegin(); it != obj->mIBOs.cend(); ++it)
      {
        const GeometryObject::SpireIBO& ibo = *it;
        stuPipe->removeIBO(ibo.name);
      }
    }
  }
}
