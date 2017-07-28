#include <es-cereal/CerealHeap.hpp>
#include <es-cereal/CerealCore.hpp>

#include "Camera.hpp"

namespace ren {

void selectCamera(CPM_ES_CEREAL_NS::CerealCore& core, uint64_t entityID,
                  gen::CameraSelect::Selection sel, bool newComponent)
{
  bool createComponent = true;

  // Don't search for an existing component if newComponent is true.
  if (newComponent == false)
  {
    // Check to see if there already exists a CameraSelect component for this
    // entity in the system. We are guaranteed that the container will exist.
    CPM_ES_CEREAL_NS::CerealHeap<gen::CameraSelect>* container = 
        core.getOrCreateComponentContainer<gen::CameraSelect>();

    std::pair<const gen::CameraSelect*, size_t> component = container->getComponent(entityID);

    // We have found the component. Modify it.
    if (component.first != nullptr)
    {
      gen::CameraSelect newSelect = *component.first;
      newSelect.cam = sel;
      container->modifyIndex(newSelect, component.second, 0);

      // We have modified the component, ensure we don't create a new component.
      createComponent = false;
    }
    else
    {
      // It could be the case that the component has already been added *this*
      // frame. But we can't know that for absolute certainty, especially
      // if we are multi threaded. So we should proceed as if there is no
      // component present. There are a number of ways to handle this situation:
      // 1) We could check what we know of the added list and see if it is
      //    there. This breaks down when multiple threads are adding components
      //    however.
      // 2) We could have some sort of mechanism (function on components) that
      //    specifies whether a component should be unique. At the end of the
      //    frame, additions are vetted to ensure uniqueness. This would be
      //    like resolving merges with components.

      // The route we should take if this actually turns out to be an issue:
      // Have a simple function that returns the recently added object and
      // lets you modify it instead of creating a new component. This will
      // address most use cases except for duplicate addition in the same
      // frame. Unsure on the correct design for duplicate addition. It very
      // may well be to resolve addition duplicates at the end of the frame
      // (sorted list of additions). This sorted list of additions will allow
      // us to more quickly handle this-frame addition duplicates.
      //
      // Next frame addition duplicates can be handled by searching the current
      // components for the specific entityID whenever add is called. If one
      // already exists, then we should probably throw an exception.
      //
      // See the todo list for cpm-entity-system. It is layed out there in
      // detail.
    }
  }

  if (createComponent)
  {
    // Create a brand new component and add it to the entity.
    gen::CameraSelect newSelect;
    newSelect.cam = sel;
    core.addComponent(entityID, newSelect);
  }
}

} // namespace ren

