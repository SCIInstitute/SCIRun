Basics
======

In the 'PIConGPU in 5 Minutes on a Workstation' section PIConGPU was installed and then a simulation run using 
the *LaserWakefield_compile_run* script.  This section summarizes the setup, compile and run steps that the 
*LaserWakefield_compile_run* script does automatically.

A more detailed explanation of the PIConGPU setup, compile and run process is provided on the PIConGPU ReadTheDocs 
website found at:

    https://picongpu.readthedocs.io/en/latest/usage/basics.html.

The summary below is informative, and provided for background and foundation.  The implementation of PIConGPU in 
SCIRun uses the equivalent of a *LaserWakefield_compile_run* script.  The SCIRun With PIConGPU Simulation Tutorial 
provides explanation and instruction for how to run a PIConGPU simulation in SCIRun.  

The Process
-----------

A PIConGPU simulation is set up and compiled by running the *picongpu.profile* script to load a system environment, 
then running two helper utilities; *pic-create* and *pic-build* to compile the simulation.  The simulation can then 
be executed using a template batch generator (tbg) instruction.

Setup
-----

The first step is to run the *picongpu.profile* script::

    source picongpu.profile

The directories created by running the *picongpu.profile* are shown in the extract below::

    mkdir -p $HOME/picInputs
    mkdir -p $HOME/scratch $HOME/scratch/runs

The variables set by running the *picongpu.profile* are shown in the extract below::

    export PIC_BACKEND="omp2b:native"   # running on CPU
    #export PIC_BACKEND="cuda:61"        # running on GPU, compute capability 6.1 (GTX 1070)
    #export PIC_BACKEND="cuda:86"        # running on GPU, compute capability 8.6 (RTX 3060)

    export SCRATCH=$HOME/scratch
    export PICSRC=$HOME/src/picongpu
    export PIC_EXAMPLES=$PICSRC/share/picongpu/examples
    export PIC_CLONE=$HOME/picInputs
    export PIC_CFG=etc/picongpu
    export PIC_OUTPUT=$SCRATCH/runs

    export PATH=$PATH:$PICSRC:$PICSRC/bin:$PICSRC/src/tools/bin
    export PYTHONPATH=$PICSRC/lib/python:$PYTHONPATH

    export CMAKE_PREFIX_PATH=$HOME/lib/ADIOS2:$CMAKE_PREFIX_PATH
    export LD_LIBRARY_PATH=$HOME/lib/ADIOS2/lib:$LD_LIBRARY_PATH
    export CMAKE_PREFIX_PATH=$HOME/lib/pngwriter:$CMAKE_PREFIX_PATH
    export LD_LIBRARY_PATH=$HOME/lib/pngwriter/lib:$LD_LIBRARY_PATH
    export CMAKE_PREFIX_PATH=$HOME/lib/openPMD-api:$CMAKE_PREFIX_PATH
    export LD_LIBRARY_PATH=$HOME/lib/openPMD-api/lib:$LD_LIBRARY_PATH

Note that the directories and variables described above are not specific to a particular simulation.  The 
*picongpu.profile* script is generic and is intended to be run prior to any PIConGPU simulation.  

Step-by-Step
------------

The remainder of the process is divided into 3 steps: Create a Working Copy, Compile the Simulation, Run the 
Simulation.  These steps include elements that are simulation specific.  This section uses the LaserWakefield 
PIConGPU Example simulation, as was done in the 'PIConGPU in 5 Minutes on a Workstation' section.

1. Create a Working Copy
""""""""""""""""""""""""

Create a working copy of the entire simulation set of files by using the pic-create helper tool::

   pic-create $PIC_EXAMPLES/LaserWakefield $PIC_CLONE/myLWFA

The *pic-create* instruction includes source and destination options.  The source contains files required to run a particular PIConGPU simulation, and the destination is the directory where output is stored.  *pic-create* creates the destination directory and copies files from the source to the destination.

PIConGPU is controlled via two kinds of textual input sets: compile-time options and runtime options.  Compile-time 
options define the physics and numeric processes.  When any of these files are changed, the simulation must be 
re-compiled.  Runtime options are set in configuration files, found in::

$PIC_CFG/*.cfg

Runtime options include things like simulation size, number of devices, output options, etc., and do not require a 
re-compile when changed.

Note the PIC_BACKEND system variable defined in *picongpu.profile*, and seen at the top of the extract shown above.  
PIC_BACKEND specifies the computer hardware that will be used to run the simulation.  For example, either of the 
following definitions might be used::

   export PIC_BACKEND="omp2b:native"  indicates that the CPU alone is to be used for numeric processing

   export PIC_BACKEND="cuda:86"       indicates that Nvidia GPUs with Compute Capability 86 are to be used

The *picongpu.profile* script is run at the beginning of the compile process.  PIC_BACKEND, and all of the system 
variables defined in *picongpu.profile* are examples of compile-time options. 

2. Compile The Simulation
"""""""""""""""""""""""""

Compiling the simulation is accomplished by switching to the Working directory and using the *pic-build* helper tool::

   cd $PIC_CLONE/myLWFA
   pic-build

3. Run The Simulation
"""""""""""""""""""""

Execute the simulation from within the working directory::

   tbg -s bash -c $PIC_CFG/1.cfg -t etc/picongpu/bash/mpiexec.tpl $PIC_OUTPUT/lwfa_001

The ``tbg`` instruction creates a directory ``$PIC_OUTPUT/lwfa_001/input/`` in the output directory.  This directory 
contains the same structure as the original working directory, and provides an archive of the working directory files 
used to run the simulation.  

The directory ``$PIC_OUTPUT/lwfa_001/simOutput/`` is also created during simulation execution and holds all the 
simulation results.

The Simulation_Compile_Run Script
---------------------------------

Note that with 4 variables::

- PIC_EXAMPLES/LaserWakefield The directory containing the simulation to be run
- PIC_CLONE/myLWFA            The directory containing the working copy of the simulation
- PIC_CFG/1.cfg               The configuration file to be used
- PIC_OUTPUT/lwfa_001         The directory to be used for storing output

A complete PIConGPU simulation can be set up and run.

The *LaserWakefield_compile_run* script is::

    #!/usr/bin bash

    cd /$HOME && source picongpu.profile && pic-create $PIC_EXAMPLES/LaserWakefield $PIC_CLONE/myLWFA
    cd $PIC_CLONE/myLWFA && pic-build && tbg -s bash -c $PIC_CFG/1.cfg -t etc/picongpu/bash/mpiexec.tpl $PIC_OUTPUT/lwfa_001 &

All of the steps described above for setting up, compiling and running the LaserWakefield simulation are 
present and executed in the order required.  A different simulation can be set up and run by setting the 
4 variables noted above as needed for the new simulation.  That process is described in detail in the 
SCIRun With PIConGPU Simulation Tutorial, next.
