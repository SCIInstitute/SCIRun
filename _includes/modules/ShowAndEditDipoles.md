---
title: ShowAndEditDipoles
category: moduledocs
module:
  category: Visualization
  package: SCIRun
tags: module
---

# {{ page.title }} #

## Category ##
**{{ page.module.category }}**

## Description ##
This module visualizes point clouds with vector data. 
It allows you to move, rescale, and rotate vectors.

### Ports ###
This module has 1 input port and 2 output ports.
The input port must be a point cloud mesh with vectors on all of the points.
The geometry output port visualizes vectors as widgets, which can be edited through the ViewScene module.
The field output sends the edited data downstream.

### Editing ###
Each vector is visualized as 4 separate widgets.
To interact with widgets, hold the SHIFT key and click and drag the widget.
Moving different widgets will cause different transformations on that vector:

| Part of Vector   | Transformation   |
| :--------------- | :--------------- |
| Sphere           | Movement         |
| Cylinder         | Movement         |
| Cone             | Rotation         |
| Disk             | Resize           |

### Scaling ###
The Visualization Scaling Factor multiplies the scaling of all the dipoles, however it does not affect the field output.

The 3 scaling options rescale the field output.

| Scaling Option                | Transformation                                                                            |
| :---------------------------- | :---------------------------------------------------------------------------------------- |
| Original                      | Uses the scaling values given from the input port                                         |
| Normalize Vector Data         | Normalizes all vector                                                                     |
| Normalize by Largest Vector   | The largest vector is normalized and the rest are rescaled in proportion of the largest   |

Note: Changing scaling options reset to the original scales from the input field.
However, you can resize the widget after changing scaling options.

### Other Options ###

| Option Name                    | Description                                                                               |
| :----------------------------  | :---------------------------------------------------------------------------------------- |
| Show Last As Vector            | Shows the last vector in the field as a point                                             |
| Show Lines                     | Displays lines connecting between the bases of every vector in the field                  |
| Move Dipole Positions Together | When one dipole is moved, every other dipole moves the same direction and distance        |


{% capture url %}{% include url.md %}{% endcapture %}
{{ url }}
