# GenerateSinglePointProbeFromField

This module generates SinglePointProbeFromField values in a field.

**Detailed Description**

A GUI Point widget is created and can be moved around in the scene. Use the **shift key/left mouse button** combination to move the widget.

Dragging moves the probe freely in 3D. Check **Keep probe on input mesh** to have the probe projected to the closest point on the input field's mesh each time it is moved, so it stays on a slice or surface (or inside a volume) even when the view is not aligned with it.

The GenerateSinglePointProbeFromField Point port contains a **PointCloudField** field with one point in it at the location of the probe. The value at that point is interpolated from the input field. The input field is also used to determine the initial position and relative size of the probe widget. If there is no input field, a default zero valued probe is returned.

The **Element Index matrix** contains the Index of the item with data that is nearest the cell. If an input field is not available, then there will not be any matrix output.
