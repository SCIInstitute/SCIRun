---
title: CreateGeometricTransform
category: moduledocs
module:
  category: Math
  package: SCIRun
tags: module

---

# {{ page.title }}

## Category

**{{ page.module.category }}**

## Description

### Summary

This module builds a 4x4 geometric transformation matrix capable of translation, scaling, rotation, sheering and permutation.

**Detailed Description**

#### Default

The default transformation matrix offers no translation, scaling, rotation, shear, or permutation. The output under default settings is a 4x4 identity matrix. By default the module also only applies a single transformation option. For example, if the translate feature is applied the result will only translate, once the user switches to another option (say rotation) the output transformation matrix will return to the original identity matrix and only apply rotation. In order to apply multiple transformations, the user must first click the "composite transform" button so that the transformation remains. Once this button is pushed, the output transformation now becomes the default matrix.

#### Input Port

The module accepts a 4x4 matrix as an input transformation matrix and populates the output to reflect the input matrix. If a matrix of any other size is used as an input, no error is thrown but the matrix is not recognized - resulting in the same, default 4x4 identity matrix.


#### UI Features:

Each UI feature is selectable via radio button.

###### Translate

Translate allows the user to define the x, y, and z directions for translation via UI sliders.

###### Scale

Scaling is manipulated by logarithmic sliders at the bottom of the page. A Log Calculator (base 10) is provided at the bottom of the window for convenience.

Above the scale sliders, translation options are also given as X, Y, and Z, sliders; however, the translational shift is also determined by the log scale factor of the respective cardinal direction. For example, the x translation is augmented or diminished by the Log ScaleX slider.

###### Rotate

Rotation allows the user to define the angle (theta) of rotation on the bottom most slider bar. This rotation angle is applied to a portion of the rotation axes (labeled "Rotate Axis X/Y/Z"). For example, if the Rotate Axis Z is set to 1.00 and the Rotate Theta angle is set to 90. The transformation matrix will rotate an object around the Z axis by 90 degrees (centered at the origin - more on this below). If the X and Z axes are equal to each other, the transformation gives equal weight to rotation in the X and Z directions, no matter the magnitude of each slider value.

The origin can be defined by the slider bars above under the heading "Rotation Fixed Point." If X, Y, and Z are set to 0 in this section, the origin corresponds to the original origin of whatever mesh the module is applied to.

###### Shear

The shear stress component of the module allows the user to define a shear stress vector in the upper panel of the UI. The vector is defined using X, Y, and Z force components. The lower half of the component has allows the user to define fixed plane components with A, B, and C representing the X, Y, and Z directions. The final component D defines the offset from the origin (in the direction of the stress vector) after the stress is applied.

###### Permute

The permute feature allows the user to flip axes. With the FlipX/FlipY/FlipZ commands, the signs of the respective axis are reversed. With the Cycle+/Cycle- options, the axes themselves are alternated (ex. x = y, y = z, z = x). This does not actually change the coordinate system of the field that is being translated. It only alters the matrix to flip over the axis lines.

#### Widget

In order to use the widget feature, the visualization output port must be attached to a viewer window. Once attached a small cubic widget appears on the screen. The widget can be scaled using the Uniform Scale slider (but it does not allow for singe directional changes i.e. you cannot scale in only the X direction...all cardinal directions scale together). Two other features appear in the UI: 1 - Resize Separable adds rotation spheres to the widget (more on this below) and 2 - Ignore changes allows the user to place, scale, and adjust the widget as they choose without applying assigning the transformations in the output matrix. A user may use the Ignore feature to adjust the size and shape of the widget to fit their geometry before they actually start manipulating the transformation.

Once the widget is in place, the user can manipulate it in the viewer window. By holding the shift key and clicking on a portion of the widget, the user can "grab" the widget and move it. Caps Lock also works to capture the widget. If the Resize Separately box is checked in the UI, spheres appear at the center of each of the 6 faces of the cube. Additionally, the user can turn on these spheres by clicking on the widget while holding down the shift+opt key on a mac (EDIT REQUIRED TO ACCOUNT FOR WINDOWS AND LINUX CONTROLS).By so clicking, the user cycles through all of the widget options (turning them on and off with each click). Shift+clicking each part of the widget allows for the following transformations:

_Gray Widget Border:_ 

--Translation

*Blue Sphere:* 

--Rotation

*Green Cylinders:*

  * Scaling (if the widget has not been rotated while Ignore Changes was applied) 

  * Shearing (if the user has rotated the widget with the Ignore Changes applied).

{% capture url %}{% include url.md %}{% endcapture %}
{{ url }}
