#ifndef INTERFACE_MODULES_VISUALIZATION_SHOW_MESH_BOUNDING_BOX_DIALOG_H
#define INTERFACE_MODULES_VISUALIZATION_SHOW_MESH_BOUNDING_BOX_DIALOG_H

#include "Interface/Modules/Visualization/ui_ShowMeshBoundingBoxDialog.h"
#include <Interface/Modules/Base/ModuleDialogGeneric.h>
#include <Interface/Modules/Visualization/share.h>

namespace SCIRun {
namespace Gui {
  class SCISHARE ShowMeshBoundingBoxDialog : public ModuleDialogGeneric,
    public Ui::ShowMeshBoundingBoxDialog
  {
    Q_OBJECT

  public:
    ShowMeshBoundingBoxDialog(const std::string& name,
      SCIRun::Dataflow::Networks::ModuleStateHandle state, QWidget* parent = 0);
  };
}}

#endif
