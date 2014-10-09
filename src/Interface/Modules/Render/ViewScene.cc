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

#include <gl-platform/GLPlatform.hpp>

#include <Interface/Modules/Render/ViewScenePlatformCompatibility.h>
#include <Interface/Modules/Render/ES/SRInterface.h>
#include <Interface/Modules/Render/GLWidget.h>
#include <Core/Application/Preferences/Preferences.h>
#include <Core/Logging/Log.h>
#include <Modules/Render/ViewScene.h>


using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Thread;
using namespace SCIRun::Core::Algorithms::Render;
using namespace SCIRun::Render;

//------------------------------------------------------------------------------
ViewSceneDialog::ViewSceneDialog(const std::string& name, ModuleStateHandle state,
  QWidget* parent /* = 0 */)
  : ModuleDialogGeneric(state, parent), shown_(false), itemManager_(new ViewSceneItemManager)
{
  setupUi(this);
  setWindowTitle(QString::fromStdString(name));

  addToolBar();
  addViewBar();
  
  // Setup Qt OpenGL widget.
  QGLFormat fmt;
  fmt.setAlpha(true);
  fmt.setRgba(true);
  mGLWidget = new GLWidget(new QtGLContext(fmt));

  if (mGLWidget->isValid())
  {
    // Hook up the GLWidget
    glLayout->addWidget(mGLWidget);
    glLayout->update();

    // Set spire transient value (should no longer be used).
    mSpire = std::weak_ptr<Render::SRInterface>(mGLWidget->getSpire());
  }
  else
  {
    /// \todo Display dialog.
    delete mGLWidget;
  }

  {
    std::shared_ptr<Render::SRInterface> spire = mSpire.lock();
    if (spire == nullptr)
      return;
    if (SCIRun::Core::Preferences::Instance().useNewViewSceneMouseControls)
    {
      spire->setMouseMode(Render::SRInterface::MOUSE_NEWSCIRUN);
    }
    else
    {
      spire->setMouseMode(Render::SRInterface::MOUSE_OLDSCIRUN);
    }
  }

  state->connect_state_changed(boost::bind(&ViewSceneDialog::newGeometryValueForwarder, this));
  connect(this, SIGNAL(newGeometryValueForwarder()), this, SLOT(newGeometryValue()));
}

//------------------------------------------------------------------------------
ViewSceneDialog::~ViewSceneDialog()
{
}

//------------------------------------------------------------------------------
void ViewSceneDialog::closeEvent(QCloseEvent *evt)
{
  // NOTE: At one point this was required because the renderer was
  // multi-threaded. It is likely we will run into the same issue in the
  // future. Kept for future reference.
  //glLayout->removeWidget(mGLWidget);
}

//------------------------------------------------------------------------------
void ViewSceneDialog::newGeometryValue()
{
  LOG_DEBUG("ViewSceneDialog::asyncExecute before locking");

  Guard lock(SCIRun::Modules::Render::ViewScene::mutex_.get());
  
  LOG_DEBUG("ViewSceneDialog::asyncExecute after locking");

  itemManager_->removeAll();
  // Grab the geomData transient value.
  auto geomDataTransient = state_->getTransientValue(Parameters::GeomData);
  if (geomDataTransient && !geomDataTransient->empty())
  {
    auto geomData = optional_any_cast_or_default<SCIRun::Modules::Render::ViewScene::GeomListPtr>(geomDataTransient);
    if (!geomData)
    {
      LOG_DEBUG("Logical error: ViewSceneDialog received an empty list.");
      return;
    }
    std::shared_ptr<Render::SRInterface> spire = mSpire.lock();
    if (spire == nullptr)
    {
      LOG_DEBUG("Logical error: Spire lock not acquired.");
      return;
    }

    std::vector<std::string> validObjects;
    for (auto it = geomData->begin(); it != geomData->end(); ++it)
    {
      boost::shared_ptr<Core::Datatypes::GeometryObject> obj = *it;
      spire->handleGeomObject(obj);
      validObjects.push_back(obj->objectName);
      itemManager_->addItem(QString::fromStdString(obj->objectName));
    }
    spire->gcInvalidObjects(validObjects);
  }
  else
  {
    std::shared_ptr<Render::SRInterface> spire = mSpire.lock();
    if (spire == nullptr)
      return;
    spire->removeAllGeomObjects();
  }
  //TODO IMPORTANT: we need some call somewhere to clear the transient geometry list once spire/ES has received the list of objects. They take up lots of memory...
  //state_->setTransientValue(Parameters::GeomData, boost::shared_ptr<std::list<boost::shared_ptr<Core::Datatypes::GeometryObject>>>(), false);
}

//------------------------------------------------------------------------------
void ViewSceneDialog::menuMouseControlChanged(int index)
{
  std::shared_ptr<Render::SRInterface> spire = mSpire.lock();
  if (spire == nullptr)
    return;

  if (index == 0)
  {
    spire->setMouseMode(SRInterface::MOUSE_OLDSCIRUN);
    SCIRun::Core::Preferences::Instance().useNewViewSceneMouseControls.setValue(false);
  }
  else
  {
    spire->setMouseMode(SRInterface::MOUSE_NEWSCIRUN);
    SCIRun::Core::Preferences::Instance().useNewViewSceneMouseControls.setValue(true);
  }
}

//------------------------------------------------------------------------------
void ViewSceneDialog::autoViewClicked()
{
  std::shared_ptr<Render::SRInterface> spireLock = mSpire.lock();
  spireLock->doAutoView();
}

//------------------------------------------------------------------------------
void ViewSceneDialog::viewBarButtonClicked()
{
	hideViewBar_ = !hideViewBar_;
	mViewBar->setHidden(hideViewBar_);
	mDownViewBox->setCurrentIndex(0);
}

//------------------------------------------------------------------------------
void ViewSceneDialog::viewAxisSelected(int index)
{

	mUpVectorBox->clear();
	mUpVectorBox->addItem("------");
	switch (index)
	{
    case 0: //default selection no value
		break;
    case 1: //Look down on +X Vector
		mUpVectorBox->addItem("+Y");
		mUpVectorBox->addItem("-Y");
		mUpVectorBox->addItem("+Z");
		mUpVectorBox->addItem("-Z");
		break;
    case 2: //Look down on +Y Vector
		mUpVectorBox->addItem("+X");
		mUpVectorBox->addItem("-X");
		mUpVectorBox->addItem("+Z");
		mUpVectorBox->addItem("-Z");
		break;
    case 3: //Look down on +Z Vector
		mUpVectorBox->addItem("+X");
		mUpVectorBox->addItem("-X");
		mUpVectorBox->addItem("+Y");
		mUpVectorBox->addItem("-Y");
		break;
    case 4: //Look down on -X Vector
		mUpVectorBox->addItem("+Y");
		mUpVectorBox->addItem("-Y");
		mUpVectorBox->addItem("+Z");
		mUpVectorBox->addItem("-Z");
		break;
    case 5: //Look down on -Y Vector
		mUpVectorBox->addItem("+X");
		mUpVectorBox->addItem("-X");
		mUpVectorBox->addItem("+Z");
		mUpVectorBox->addItem("-Z");
		break;
    case 6: //Look down on -Z Vector
		mUpVectorBox->addItem("+X");
		mUpVectorBox->addItem("-X");
		mUpVectorBox->addItem("+Y");
		mUpVectorBox->addItem("-Y");
		break;
	}

}

//------------------------------------------------------------------------------
void ViewSceneDialog::viewVectorSelected(int index)
{
    int downIndex = mDownViewBox->currentIndex();
	glm::vec3 up = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 view = glm::vec3(0.0f, 0.0f, 0.0f);
    switch(downIndex)
    {
    case 0:
        break;
    case 1:	//+X axis view
		view.x = 1.0f;
		lookDownAxisX(index, up);
        break;
    case 2:	//+Y axis view
		view.y = 1.0f;
		lookDownAxisY(index, up);
        break;
    case 3:	//+Z axis view
		view.z = 1.0f;
		lookDownAxisZ(index, up);
        break;
    case 4:	//-X axis view
		view.x = -1.0f;
		lookDownAxisX(index, up);
        break;
	case 5:	//-Y axis view
		view.y = 1.0f;
		lookDownAxisY(index, up);
        break;
	case 6:	//-Z axis view
		view.z = -1.0f;
		lookDownAxisZ(index, up);
        break;
    }
	if (index > 0)
	{
		std::shared_ptr<Render::SRInterface> spire = mSpire.lock();
		spire->setView(view, up);
		viewBarButtonClicked();
	}
}

//------------------------------------------------------------------------------
void ViewSceneDialog::lookDownAxisX(int upIndex, glm::vec3& up)
{
    switch(upIndex)
    {
    case 0:
        break;
    case 1: //+Y axis
		up.y = 1.0f;
        break;
    case 2: //-Y axis
		up.y = -1.0f;
        break;
    case 3: //+Z axis
		up.z = 1.0f;
        break;
    case 4: //-Z axis
		up.z = -1.0f;
		break;
    }
}

void ViewSceneDialog::lookDownAxisY(int upIndex, glm::vec3& up)
{
    switch(upIndex)
    {
    case 0:
        break;
    case 1: //+X axis
		up.x = 1.0f;
        break;
    case 2: //-X axis
		up.x = -1.0f;
        break;
    case 3: //+Z axis
		up.z = 1.0f;
        break;
    case 4: //-Z axis
		up.z = -1.0f;
        break;
    }
}

void ViewSceneDialog::lookDownAxisZ(int upIndex, glm::vec3& up)
{
    switch(upIndex)
    {
    case 0:
        break;
    case 1: //+X axis
		up.x = 1.0f;
        break;
    case 2: //-X axis
		up.x = -1.0f;
        break;
    case 3: //+Y axis
		up.y = 1.0f;
        break;
    case 4: //-Y axis
		up.y = -1.0f;
        break;
    }
}

//------------------------------------------------------------------------------
void ViewSceneDialog::addToolBar() 
{
  mToolBar = new QToolBar(this);

  addMouseMenu();
  addAutoViewButton();
  addObjectToggleMenu();

  glLayout->addWidget(mToolBar);
}

void ViewSceneDialog::addMouseMenu()
{
  auto menu = new QComboBox(this);
  menu->setToolTip("Change Mouse Controls");
  menu->addItem("Legacy Mouse Control");
  menu->addItem("New Mouse Control");
  connect(menu, SIGNAL(currentIndexChanged(int)),this, SLOT(menuMouseControlChanged(int)));
  menu->setCurrentIndex(SCIRun::Core::Preferences::Instance().useNewViewSceneMouseControls ? 1 : 0);
  mToolBar->addWidget(menu);
  mToolBar->addSeparator();
}

void ViewSceneDialog::addAutoViewButton()
{
  QPushButton* autoViewBtn = new QPushButton(this);
  autoViewBtn->setToolTip("Fit Object to Screen");
  autoViewBtn->setText("Auto View");
  autoViewBtn->setAutoDefault(false);
  autoViewBtn->setDefault(false);
  autoViewBtn->setShortcut(Qt::Key_0);
  connect(autoViewBtn, SIGNAL(clicked(bool)), this, SLOT(autoViewClicked()));
  mToolBar->addWidget(autoViewBtn);
  mToolBar->addSeparator();
}

class FixMacCheckBoxes : public QStyledItemDelegate
{
public:
  void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
  {
    QStyleOptionViewItem& refToNonConstOption = const_cast<QStyleOptionViewItem&>(option);
    refToNonConstOption.showDecorationSelected = false;
    QStyledItemDelegate::paint(painter, refToNonConstOption, index);
  }
};

void ViewSceneDialog::addObjectToggleMenu()
{
  QComboBox* combo = new QComboBox();
  combo->setItemDelegate(new FixMacCheckBoxes);
  combo->setModel(itemManager_->model());
  combo->setToolTip("Select an Object");
  mToolBar->addWidget(combo);
  mToolBar->addSeparator();
}

void ViewSceneDialog::addViewBarButton()
{
	QPushButton* viewBarBtn = new QPushButton(this);
	viewBarBtn->setToolTip("Show View Options");
	viewBarBtn->setText("Views");
	viewBarBtn->setAutoDefault(false);
	viewBarBtn->setDefault(false);
	connect(viewBarBtn, SIGNAL(clicked(bool)), this, SLOT(viewBarButtonClicked()));
	mToolBar->addWidget(viewBarBtn);
	mToolBar->addSeparator();
}

void ViewSceneDialog::addViewBar()
{
	mViewBar = new QToolBar(this);

	addViewOptions();
	hideViewBar_ = true;

	mViewBar->setHidden(hideViewBar_);

	glLayout->addWidget(mViewBar);

	addViewBarButton();
}

void ViewSceneDialog::addViewOptions()
{
	QLabel* axisLabel = new QLabel();
	axisLabel->setText("Look Down Axis: ");
	mViewBar->addWidget(axisLabel);

	mDownViewBox = new QComboBox();
	mDownViewBox->setToolTip("Vector pointing out of the screen");
	mDownViewBox->addItem("------");
	mDownViewBox->addItem("+X");
	mDownViewBox->addItem("+Y");
	mDownViewBox->addItem("+Z");
	mDownViewBox->addItem("-X");
	mDownViewBox->addItem("-Y");
	mDownViewBox->addItem("-Z");
	connect(mDownViewBox, SIGNAL(currentIndexChanged(int)), this, SLOT(viewAxisSelected(int)));
	mViewBar->addWidget(mDownViewBox);
	mViewBar->addSeparator();

	QLabel* vectorLabel = new QLabel();
	vectorLabel->setText("Up Vector: ");
	mViewBar->addWidget(vectorLabel);

	mUpVectorBox = new QComboBox();
	mUpVectorBox->setToolTip("Vector pointing up");
	mUpVectorBox->addItem("------");
	connect(mUpVectorBox, SIGNAL(currentIndexChanged(int)), this, SLOT(viewVectorSelected(int)));
	mViewBar->addWidget(mUpVectorBox);
	mViewBar->addSeparator();
}

void ViewSceneDialog::showEvent(QShowEvent *evt)
{
  ModuleDialogGeneric::showEvent(evt);
  if (!shown_)
  {
    autoViewClicked();
    shown_ = true;
  }
}

ViewSceneItemManager::ViewSceneItemManager() : model_(new QStandardItemModel(3,1))
{
  model_->setItem(0, 0, new QStandardItem(QString("Object Selection")));
  connect(model_, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)), this, SLOT(slotChanged(const QModelIndex&, const QModelIndex&)));

#if 0
  //fill with dummy items for testing:
  for (int r = 0; r < 3; ++r)
  {
    QStandardItem* item = new QStandardItem(QString("Item %0").arg(r));

    item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
    item->setData(Qt::Unchecked, Qt::CheckStateRole);
    items_.push_back(item);

    model_->setItem(r+1, item);
  }
#endif
}

void ViewSceneItemManager::addItem(const QString& name)
{
  QStandardItem* item = new QStandardItem(name);

  item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
  item->setData(Qt::Unchecked, Qt::CheckStateRole);
  items_.push_back(item);

  model_->appendRow(item);
}

void ViewSceneItemManager::removeItem(const QString& name)
{
  auto items = model_->findItems(name);
  Q_FOREACH(QStandardItem* item, items)
  {
    model_->removeRow(item->row());
  }
  items_.erase(std::remove_if(items_.begin(), items_.end(), [&](QStandardItem* item) { return item->text() == name; } ), items_.end());
}

void ViewSceneItemManager::removeAll()
{
  if (model_->rowCount() > 1)
  {
    LOG_DEBUG("ViewScene items cleared" << std::endl);
    model_->removeRows(1, model_->rowCount() - 1);
    items_.clear();
  }
}

void ViewSceneItemManager::slotChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight)
{
  auto index = topLeft.row() - 1;
  QStandardItem* item = items_[index];
  if (item->checkState() == Qt::Unchecked)
  {
    LOG_DEBUG("Item " << item->text().toStdString() << " Unchecked!" << std::endl);
    Q_EMIT itemUnselected(item->text());
  }
  else if (item->checkState() == Qt::Checked)
  {
    LOG_DEBUG("Item " << item->text().toStdString() << " Checked!" << std::endl);
    Q_EMIT itemSelected(item->text());
  }
}
