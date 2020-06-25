---
title: GenerateStreamLines
category: moduledocs
module:
  category: Visualization
  package: SCIRun
tags: module

---

# {{ page.title }}

## Category

**{{ page.module.category }}**

## Description

### Summary

This module visualizes vector fields by generating curves that interpolate the vector of vectors in a Field.

**Detailed Description**

The **Vector Field** input port is the vector field on which the path intergration of the seed points will be performed. This may be a vector field with any volume geometry type. Surfaces and lower order fields do not currently work due to numerical error. 

The second input port, **Seeds**, contains the initial values for which the vector will be computed. This may be a field of any type, as just the node positions are used. Seeds which are not contained within the vector field are just discarded. The module requires use of both field inputs in order to execute.

The GenerateStreamLines module has one output, **Streamlines**, a CurveMesh, representing a collection of stream lines computed for the Vector field given the set of initial seed points.

This module uses one of several similar numeric integration methods. They are taken directly from Numerical Analysis by David Kincaid and Ward Cheney, c1991. Adams-Bashforth is a Multi-Step method that offers the fastest performance by reusing existing samples when possible. It presents some interesting artifacts on discontinuous models such that it is easy to visualize where those discontinuities take place. Heun is a second order Runge-Kutta solver, and is sufficient for most smoothly varying fields, such as linearly interpolated gradient fields over volume elements. Classic 4th Order Runge-Kutta is presented for reference and provides excellent results in general. Adaptive Runge-Kutta-Fehlberg is a 5/6 order solver which provides the best overall results at the cost of more computation time. It is also the only adaptive method, meaning the step size of the algorithm is adjusted dynamically as the stream moves through the vector field.

All of the computation methods are similar in that they take a step, (based upon the Step Size indicated) examine the derivative at that point, and then procede in that new direction. Adaptive Runge-Kutta-Fehlberg also determines if a stepsize correction is necessary. This determination is based upon the Error Tolerance value.

The GenerateStreamLines GUI includes text fields for **Error Tolerance**, **Step Size**, and **Maximum Steps**. The Error Tolerance text field represents the margin of error when doing an adaptive calculation of the stream lines. Maximum Steps represents the maximum number of iterations.

The user can specify whether streamlines are computed in negative and/or, positive directions by changing the Direction radio buttons. The **Color Style** options are based on either the seed number, integration step, distance from the seed, or the total distance, which will affect how values are attached to the output field. By selecting seed number, the output field will range from zero to the number of seed points so that every stream line has exactly one value. The integration step option changes the value of output field such that they start at zero where the seed point is at and go up by one for each integration step away in either direction. The distance from seed is similar to the integration step but based on the cord length distance from the seed. Whereas the total length is based on the total cord length from the seed point.

The **Filter Colinear Points** check box is a postprocess on the streams. It passes over the streams and removes all points that are too close together, as well as points that are colinear. It is useful for rendering the streamlines as it greatly simplifies them while retaining their visual appearance.

{% capture url %}{% include url.md %}{% endcapture %}
{{ url }}
