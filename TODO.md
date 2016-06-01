User Requests
=============

* General

  - key parameters in networks, environment need to be findable and available in UI
  - expose network XML, or at least key network info (modules, file paths, gui vars...)

    + better network parameter editing
    + maybe a network wizard?
    + show what files are being used, network file name

  - more example nets
  - more documentation, more sample networks (very useful for demonstrating module functionality)
  - search modules, networks for keywords

    + see Matlab descriptors at the top of source files as an example
    + when searching for modules, can we have the option of searching within the module text (i.e. search module GUI component names, variable names etc.)

  - port colors: change intensities, not just colors

    + get updated palette from Nathan

  - standardize field names etc.

    + i.e. interpolation: source -> target, object -> target, source -> object

  - support for transparent background in images or movies (image library)
  - image or movie resolution options (can we generate higher resolution images than displayed?)
  - first time use window
  - Nifti reader for SCIRun
  - STL file support
  - Add preset location for saving SCIRun network files
  - Parser modules need better error reporting
  - Actually implement ColorMap2DSemantics module
  - list field properties
  - if statement module
  - SCIRun should be able to read "obj" format meshes
  - Simple importer for 1D colormaps

* UI

  - collapse network preview window
  - side pane to expose some module parameters

    + expose important dataset parameters too (size, dims, data info etc.)

  - easier way to do notes

    + copy notes with modules
    + easy way to edit or clear notes
    + easy way to manipulate note location
    + render text on top of pipes

  - better graph layout algorithms?

    + look into graphviz, others?
    + line up modules
    + expose-like (OS X)

  - Add GUI element for indicating result of "Clear Output" button of module CollectMatrices
  - Add string port to Read / Write Nrrd
  - Add support for newer Matlab file format
  - Add cylinder to vector glyph types
  - ShowMatrix module should detect matrix dimensions
  - IO plugin for Meshalyzer binary mesh format
  - IO plugin for a binary node index matrix

* Networks

  - halt network without killing SCIRun

* Subnets

  - subnets must work (currently broken) - should be an easy select and create operation in GUI
  - better views for subnets: dockable widgets, tabs etc.
  - duplicate entire subnet
  - import network into other network

    + choice to import as subnet or register network
    + choice to save subnet as separate file

Qt Migration
============

* Basic GUI Redesign

  - Determine overall look and feel.

    + Background color of network editor
    + Placement of main window widgets--match current design at first?

  - Redesigning major parts of the main window.

    + Menu bar
    + Context menus
    + Toolbars
    + Status bar
    + Configuration panel
    + Module Search
    + Module progress indicators
    + Module general settings: port caching, info on ports, general info: available in "drawer" subwidget
    + Tooltips
    + Command window

  - CIBC:Documentation:SCIRun:DevTasks:SCIRunGuiFeatureList

* Major Development Milestones

  - All menu options/preferences work
  - Can walk through entire SCIRun tutorial!

* IO

  - is there a way we can display a brief blurb describing file importer and exporter plugin information (i.e. show a short description of the file types supported, file extensions etc.)?

* Graphics and Visualization

  - Be able to select or click on objects in renderer and get information i.e. node index, position etc.

* OpenGL

  - move all OpenGL code into Core (and intermediate layer as needed)

* Externals

  - verify that libpng and zlib are still needed, if so, update
  - better XML parsing library (libxml2 is overly complex and buggy)

* Build

  - merge all generated headers into a single generated header file

* Testing

  - Add regression testing feature to diff numerical output, either through text files or of matrix objects directly.

* Core

  - Isosurface algorithm is very slow and should be rewritten

* Error Handling and Exceptions

  - SCIRun exceptions could use an overhaul.
  - SCIRun assertions could also use an overhaul.
  - The same kind of assertions need to be used throughout the code. Currently, we're using:

    + assert from cassert
    + SCIRun's custom ASSERT macros
    + Boost static assert (perhaps it would be best to replace all other asserts with Boost assert utilities?)

* Modules

  - Possible Problem Modules

    + ShowMatrix: this module should be completely rewritten
    + CreateMatrix: bug #327
    + GeneratePointSamplesFromFieldOrWidget: spinner boxes in both tabs are not editable. This is really annoying if one wants to generate a large number of samples.
    + GeneratePointSamplesFromField: The execution pattern for this module should be improved. Also, SCIRun test networks are using GeneratePointSamplesFromFieldOrWidget, so there are no tests for this module.
    + CreateAndEditColorMap: the GUI is not very usable and can be difficult to navigate.

  - Module Improvements

    + See if it's possible to encapsulate common functionality in Matlab Importer modules in a parent class. Same with Exporter modules.

Known Bugs
==========

* Duplicated modules in networks
* Adjusting current displayed slice in ShowTextureSlices
* Connecting module after disabling and enabling can crash SCIRun
* Row indices not displayed for newly created rows in CreateMatrix
* Error reading fields created using older SCIRun versions
* BioTensor crashes on Windows
* BioTensor fails on OS X Snow Leopard
* SCIRun crashes when applying colormap to field from OBJ file
* InsertHexVolSheetAlongSurface gets invalid indices, doesn't converge
* changing light direction for volume rendering is broken
* closing the ViewWindow causes a crash
* Crash while not SCIRun is idle
* Create module skeleton creates bad CMake files
* Crash: replacing port on ShowTextureSlices
* Dipole does not change orientation after edit
* X11 crash when loading network
* SCIRun Matlab field exported by SCIRun cannot be read by SCIRun
* Incorrect rendering when network executed on startup
* Installer install path defaults to previously installed path
* UnuAxInfo should show current axis attributes
* Disabled modules execute during regression testing
* File dialog selection gets reset
* Showfield does not update appropriately.
* ShowAndEditCameraWidget hangs SCIRun
* Regression tests timeout before loading network
* Networks that hang on execution in regression testing mode fail to output image.
* scirun hangs while viewing extracted isosurface
* ResampleRegularMesh module missing resampling kernel options
* CalculateFieldDataMetric Integral option broken
* Cannot change field type in CreateParameterBundle module GUI
* CreateAndEditColormap SegFault
* Segfault caused by key/button click on widget
* Matlab file text field gets cleared on execute in ExportDatatypesToMatlab
* Transparency not supported in ExtractIsosurface geometry output
* RemoveZerosFromMatrix module is broken
* RemoveZeroRowsAndColumns module does not remove zero columns
* GenerateLinearSegments output field has geometry size 0
* CreateAndEditColorMap hangs on execution
* ExtractIsosurface module geometry ignores opacity
* SolveLinearSystem Jacobi method does not converge when used with AddKnownsToLinearSystem
* ShowField crashes if attempting to display data value text in a field with no data
* ShowTextureSlices crashes when its output port is used more than once.
* InsertHexVolSheet creates invalid elements
* ExtractIsosurface creates crossing elements
* RefineMesh local refinement bug
* FairMesh should check for and report crossing elements
* ConvertColorMap2ToNrrd always fails on assert
* EvaluateLinAlgUnary writes to invalid SparseMatrix location
* Setting SparseMatrix entries to same constant fails with EvaluateLinAlgUnary
* Review OptimizeConductivities module to make sure it works as expected.
* OptimizeDipole sample networks hang
* ViewScene record movie fails silently if path doesn't exist
* torso-tank-bem.srn network fails if cage is moved
* ExportNrrdsToMatlab doens't preserve header information
* ViewSlices module not ported properly from SCIRun 3

Resolved Bugs
=============

* SCIRun crashes when you write a movie to a directory where you do not have the permission to.
* libxml2 does not resolve local DTD path
* SCIRun module GeneratePlanarElectrode causes SCIRun to crash when loading.
* ShowAndEditCameraWidget UI is broken
* Changing spaces in OS X 10.6.3 logs user out
* Seg3D freezes when using 'Save Volume'
* MatlabInterface broken in SCIRun OS X app
* Tcl/Tk interpreter crashes on startup on Ubuntu 11
* Problem reading Seg3D2 segmentations (.nrrds) into SCIRun
* ExportFieldsToMatlab executes when disabled
* Record movie causes SCIRun to hang
* Memory leaks in BuildFEGridMapping algorithm
* CollectMatrices does not detect new inputs
* Missing parser documentation in CreateFieldData	Medium
* String port input does not update parser module expression
* Deleting CreateAndEditColorMap2D from network crashes SCIRun.
* SolveLinearSystem graph does not show the current iteration
* ExportMatricesToMatlab file path error on Windows
* GetNetworkFileName does not update when network is cleared.
* Tcl/Tk GUI code cannot handle infinite floating point values
* Deleting swatches from CreateAndEditColorMap2D's UI crashes SCIRun.
* Issues with ConvertMatricesToMesh module: GUI
* Issues with ConvertMatricesToMesh module: C++
* Script tab in ReadField does not work
* Closing rendering window crashes X11 in XQuartz 2.7.x
* Crash when displaying text in LatVol (duplicate)
* SparseRowMatrix::put cannot put values in matrix if number of non-zero values 0
* Visualization test networks hang on Ubuntu when executed on startup
* TriSurf mesh get_edges with nodes argument broken
