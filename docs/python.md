---
title: Python API 0.2
category: info
tags: python
layout: default
---
<link rel="stylesheet" href="css/main.css">

The API contains global functions for network editing, file management, module state modification, and data object extraction and insertion.

Some functions are not available for execution within the `InterfaceWithPython` module, since they are not useful in that context and can cause instability. They are marked with a (*).

## Global functions

### Network editing
* `scirun_add_module("ModuleName")`
  * Adds a new instance of a module to the network. Returns the module ID as a string. (*)
* `scirun_remove_module("ModuleID")`
  * Removes the module specified by the ID string. (*)
* `scirun_execute_all()`
  * Executes the entire current network. (*)
* `scirun_module_ids()`
  * Returns a list of all module ID strings in the current network, sorted by module creation time.
* `scirun_connect_modules("ModuleIDFrom", fromIndex, "ModuleIDTo", toIndex)`
  * Connects ports between two modules by index. From is the output, To is the input. (*)
* `scirun_disconnect_modules("ModuleIDFrom", fromIndex, "ModuleIDTo", toIndex)`
  * Used to disconnect two modules, with the same syntax as connecting. (*)

### Network file management
* `scirun_save_network("Filename.srn5")`
  * Saves the current network file. (*)
* `scirun_load_network("Filename.srn5")`
  * Loads the specified network file. (*)
* `scirun_import_network("Filename.srn")`
  * Imports the specified v4 network file. (*)
* `scirun_quit_after_execute()`
  * Enables quitting SCIRun after the next network execution is complete. (*)
* `scirun_force_quit()`
  * Quits SCIRun immediately. (*)

### Module state editing
* `scirun_get_module_state("ModuleID", "StateVariableName")`
  * Returns the value of the specified module state variable.
* `scirun_set_module_state("ModuleID", "StateVariableName", value)`
  * Sets the specified module state variable's value.
* `scirun_dump_module_state("ModuleID")`
  * Returns a dictionary with the entire state of the specified module.
* `scirun_get_module_transient_state("ModuleID", "StateVariableName")`
  * Returns the value of the specified module transient state variable.
* `scirun_set_module_transient_state("ModuleID", "StateVariableName", value)`
  * Sets the specified module transient state variable's value. Used to pass data values (strings, matrices, fields [coming soon]) back to modules.

### Module/Datatype input
* `scirun_get_module_input_type("ModuleID", portIndex)`
  * Returns the type of the input data object on the specified port.
* `scirun_get_module_input_object("ModuleID", "PortName")`
  * Returns a special `PyDatatype` wrapper object containing a copy of the data on the specified input port, by name.
* `scirun_get_module_input_value("ModuleID", "PortName")`
  * Returns a Python object containing a copy of the data on the specified input port.
* `scirun_get_module_input_object_by_index("ModuleID", portIndex)`
  * Returns a special `PyDatatype` wrapper object containing a copy of the data on the specified input port, by port index.
* `scirun_get_module_input_value_by_index("ModuleID", portIndex)`
  * Returns a Python object containing a copy of the data on the specified input port, by index.

### InterfaceWithPython special syntax
* This module lets you set input/output variable names in the module UI. Once this is done (or by using the defaults), one can use assignment syntax to read input data and send output data.
  * Examples
     * `pythonOutput = "hello"` will send a string to the output port associated with `pythonOutput`
     * `field = inputField1` will extract a field object from the input port associated with `inputField1`
     * `outputString1 = "string concat " + inputString1` Input/Output can be combined on the same line.
  * Note: for output variable assignment, make sure to include spaces around the `=`.

  ###  InterfaceWithPython Top-Level Script

In the InterfaceWithPython there is a "Top-Level Script" tab which allows users to run matlab code in a broader scope on execution of the InterfaceWithPython Module.  This is helpful if there are variables or modules that are costly to compute or load, yet are used in more than one InterfaceWithPython Module.  The [Matlab engine](#matlab-engine-in-scirun-5-through-python) is one such example.  To launch the matlab engine once per session, use the following code in the top-level script:
```
import matlab.engine
print('matlab imported')
eng =  matlab.engine.start_matlab() if (not 'eng' in vars()) else eng
```
This will allow the same matlab engine instance to be called with `eng`.  *Note: using the [Matlab code block](#matlab-code-block) does not require this code in the top-level tab.*

## Installing packages
* If there is an installation of the packages with other python builds on the machine, the system path can be used to use those packages.  This is likely the easiest way to use python packages in SCIRun, and will also likely to be easiest to maintain.  However, to use packages from another Python build, the Python versions will need to match.  Once the packages are installed, use the SCIRun triggered events to modify the python path to include these packages.*

### Installing Python to maintain Python packages.

The main advantage of doing this step is that a seperate Python will likely have pip or multiple packages already installed, unlike the SCIRun Python distribution (we are working on it).  Most distributions will likely work; some operating systems come with a python distribution that may work, yet it may be an outdated version. [Anaconda](https://www.anaconda.com/distribution/#download-section) is a good choice because most of the commonly used packages will be installed when it is installed.  However, the cleanest option will likely be installing [Python's own distribution](https://www.python.org/downloads/), then to install the packages that you want to use.  Both of these distributions should come with pip installed, which makes it easy to install most common packages.

**Make sure the python versions of the separate Python installation matches the version that SCIRun is using.** To check the Python version in SCIRun, simply open the Python Console, it will print the version upon initialization.  You can also get the version when running on the command line by using the `-v` flag.



### Installing numpy, scipy, and other major packages with pip

Once you have a seperate Python with pip installed, installing packages is usually farily straightforward.  The basic command to install numpy is:
`pip install numpy`
However, if you have multiple python installations, it may be necessary to specify which pip to use.  The pip version will need to match the python version you intend to use.  Often the version name is appended to the function call:
`pip3.6 install numpy`
Yet it may be necessary to point to the specific installation:
`/Library/Frameworks/Python.framework/Versions/3.6/bin/pip3.6 install numpy`
for instance. If ther is an error installing the package, try upgrading pip first:
`pip3.6 install --upgrade pip`


### Installing Matlab engine for python in SCIRun

Full installation instructions for installing the Matlab engine in Python are found [on the mathworks site](http://www.mathworks.com/help/matlab/matlab_external/install-the-matlab-engine-for-python.html).  Keep in mind that each Matlab version will only support a few Python versions, and the Python version much match what is running in SCIRun (see above).  Also, it would probably help if Python Matlab engine installation was in the same location as the other packages to be used in SCIRun.  It is helpful to have numpy and scipy installed aswell.

To install the Matlab engine in Python run in the terminal:

```
cd "matlab_root"/extern/engines/python
"Python_installation"/bin/python3.6 setup.py build install
```
### Adding packages to the Python Path in SCIRun

In order to use the installed packages, SCIRun's Python has to be able to find them.  If the package files are located in the Python Path, the can be used with the `import` function.  The python path can be modified in many ways, but the most practical for general purpose is to use the triggered events in SCIRun.  Triggered events are explained in [another section](#triggered-events), but we will explain how to use it to modify the Python Path.

Open up the Triggered Event Window in SCIRun and choose the "Application Start" tab.  Make sure that this script is enabled by clicking the "Enabled" checkbox.  Now enter a script that will add the directories to the package installation in the textbox.  It will look something like this:

```
import sys
sys.path.append('/Library/Frameworks/Python.framework/Versions/3.6/lib/python3.6/site-packages/')
```

There can be many directories or many `sys.path.append(...)` calls as needed.  This script is saved and will run everytime SCIRun starts, and therefore it will update the path.

If there are module or package paths that need to be set on a network basis (rather than for every network), the InterfaceWithPython module can acheive this with the ["Top-Level Script" Tab](#interfacewithpython-top-level-script).  This tab will execute the code in this tab on a global scale (until SCIRun is closed), so a script similar to the triggered events example will be saved and executed only for the network.


## Matlab engine in SCIRun 5 (through python)

In SCIRun 5, Matlab code and functions can be run using the matlab engine for python in the python console or python interface.  To do so, make sure that the matlab engine is installed (previous section).
Full documentation can be found [here](http://www.mathworks.com/help/matlab/matlab-engine-for-python.html).

### Matlab code block

In the InterfaceWithPython module, there is a Matlab code block option.  This is a series of Matlab functions that are serparated by `%%`.  The InterfaceWithPython module will attempt to convert the native Matlab code in the block as Python code.  The commands cannot be complex in that they must only contain one function per line.  For example:
```
%%
A = magic(3);
[evec, eval] = eig(A);
%%
```
Users can insert a Matlab code block with the button in the InterfaceWithPython UI, or bby typing `%%`.  To use the Matlab code block, make sure:
- Matlab and the Python Matlab engine are installed
- The Matlab Python module is in the Python Path in SCIRun
- The ["MatlabConversion.py"](https://github.com/SCIInstitute/SCIRun/blob/master/src/Modules/Python/MatlabConversion.py) file is in the Python Path.  This is found in the source code: "SCIRun/src/Modules/Python".
- Matlab functions are in the Matlab path.  This is best accomplished in the Matlab UI.

The Matlab Code Block is experimental code, so it will likely not work with complex Matlab functionality. However, if there is a code block detected, SCIRun will start Matlab and it will remain open until SCIRun is closed.   The python variable names should match the variable names in the assignment.

## Triggered Events

Triggered events in SCIRun execute a python script upon certain SCIRun events.  Possible events include: application start,  network load, and adding a module.  The scripts and settings can be modified in the triggered events window.  The triggered event scripts allow for increased customization such as: modifying the Python path, changing the module default settings, and others.  Instructions on how to use triggered events to modify the Python path are [describe previously](#adding-packages-to-the-python-path-in-scirun).

To change the default settings of a module, use the 'post module add' trigger.  Make sure it is enabled and use `scirun_set_module_state()` function.  For example, to change the default matrix type for the ReadMatrix module to matlab type, use the following function:
```
scirun_set_module_state(scirun_module_ids()[-1], 'FileTypeName', 'Matlab Matrix (*.mat)') if scirun_module_ids()[-1].startswith('ReadMatrix') else None
```

As another example, to always open all the viewscene windows when a network is loaded, use the 'on network load' trigger.  With this trigger enabled, use the following command in the script:
```
[scirun_set_module_state(id, '__UI__', True) for id in scirun_module_ids() if id.startswith('ViewScene')]
```
The `__UI__` variable is a hidden state boolean that indicates the open UI window.  

## Python Macros

Python Macros in SCIRun are similar to [triggered events](#triggered-events) except they execute a python script when macro button is pressed.  This feature can be used to do multi-step tasks at the push of a button.  As an example, to have a GetMatrixSlice module to play from the beginning, use the following script:
```
# number of the getmatrixslice.   eg, GetMatrixSlice:0
mod_num = 0
scirun_set_module_state("GetMatrixSlice:"+str(mod_num),"SliceIndex",0)
scirun_set_module_transient_state("GetMatrixSlice:"+str(mod_num),"PlayModeActive",1)
scirun_execute_all()
```
The Macro toolbar will need to be visible to access this feature.

## Running Python Scripts

Python scripts can be used for many things in SCIRun, from building networks to batch executing data.  To run a script saved to disk within SCIRun, there are a couple options.  There is a run script option (looks like a magic wand), which is part of the advanced toolbar. This option will clear any network when a script is run, so it is great for running scripts that build and execute networks.  Scripts can also be called as a command line input with the `-s` or `-S` flags.  This option is similar to the run script tool, but it allows for passing script arguments after the script filename.  For example, in OS X:
`/Applications/SCIRun.app/Contents/MacOS/SCIRun -s *script_filename.py* *arg1*`.

To run a script within SCIRun without clearing the network, open and execute the script in the SCIRun python console:
`exec(open('*path_to_script/filename.py*').read())`.
This syntax can also be used in SCIRun's interactive mode (`-i` flag from the command line).
