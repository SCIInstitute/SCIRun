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


#ifndef INTERFACE_APPLICATION_SUBNETWORKS_H
#define INTERFACE_APPLICATION_SUBNETWORKS_H

#include "ui_SubnetEditor.h"
#ifndef Q_MOC_RUN
#include <Dataflow/Network/ModuleDescription.h>
#include <Dataflow/Network/Module.h>
#endif
#include <QDockWidget>

class QGraphicsItem;

namespace SCIRun
{
  namespace Gui
  {
    class NetworkEditor;

    //TODO: post-ibbm
    // class SubnetworkWindow : public QMdiArea, public Ui::SubnetEditor2
    // {
    //   Q_OBJECT
    // public:
    //   explicit SubnetworkWindow(QWidget* parent = nullptr);
    // };

    class SubnetworkEditor : public QFrame, public Ui::SubnetEditor
    {
      Q_OBJECT

    public:
      SubnetworkEditor(NetworkEditor* editor, const SCIRun::Dataflow::Networks::ModuleId& subnetModuleId, const QString& name, QWidget* parent = nullptr);
      NetworkEditor* get() { return editor_; }
      ~SubnetworkEditor();

    private Q_SLOTS:
      void expand();
    private:
      NetworkEditor* editor_;
      QString name_;
      SCIRun::Dataflow::Networks::ModuleId subnetModuleId_;
    };

    class SubnetModule;

    class SubnetModuleConnector : public QObject
    {
      Q_OBJECT
    public:
      explicit SubnetModuleConnector(NetworkEditor* parent);
      void setSubnet(NetworkEditor* subnet);
      void setModule(SubnetModule* module) { module_ = module; }
    public Q_SLOTS:
      void moduleAddedToSubnet(const std::string&, SCIRun::Dataflow::Networks::ModuleHandle module);
      void connectionDeletedFromParent();
    private:
      NetworkEditor* parent_;
      NetworkEditor* subnet_;
      SubnetModule* module_;
      bool signalFromParent(QObject* sender) const;
      bool signalFromSubnet(QObject* sender) const;
    };

    class SubnetModule : public Dataflow::Networks::Module
    {
    public:
      SubnetModule(const std::vector<Dataflow::Networks::ModuleHandle>& underlyingModules, const QList<QGraphicsItem*>& items, NetworkEditor* parent);

      void execute() override;

      static const Core::Algorithms::AlgorithmParameterName ModuleInfo;

      void setStateDefaults() override;

      std::string listComponentIds() const;
      void setSubnet(NetworkEditor* subnet) { connector_.setSubnet(subnet); }
    private:
      friend class SubnetModuleConnector;
      std::vector<Dataflow::Networks::ModuleHandle> underlyingModules_;
      QList<QGraphicsItem*> items_;
      static int subnetCount_;
      SubnetModuleConnector connector_;
    };
  }
}

#endif
