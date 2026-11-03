# Streamline Geometry: SCIRun vs VTK

<a href="../streamlines.html"><b>Open the interactive explainer →</b></a>

An animated walkthrough of how `GenerateStreamLines` builds its output, how a SCIRun
`CurveMesh` differs from VTK's `vtkPolyData` polylines, what `VtkGeometryBuilder` does
to bridge the two, and what goes wrong when per-element current is interpolated
through thin tissue layers.

Analysis and follow-up tasks: [#2819](https://github.com/SCIInstitute/SCIRun/issues/2819).

The page is a standalone HTML file in `docs/explainers/`. Sphinx copies it to the site
root unchanged through `html_extra_path` in `docs/conf.py`.
