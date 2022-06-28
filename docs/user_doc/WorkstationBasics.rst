.. _usage-basics:

.. Note::

   You need to have an environment loaded (``source $HOME/picongpu.profile``) to complete this chapter.

.. Note::

   Helper tools like ``pic-create`` and ``pic-build`` described in this section rely on Linux utilities.

Basics
======

.. sectionauthor::Kim Peterson

Preparation
-----------

A working copy of the PIConGPU simulation is held in an input directory: ``$HOME/picInputs``
  Simulation output is stored in ``$HOME/scratch/runs``.  


For convenience, these paths are set as system variables PIC_CLONE and PIC_OUTPUT respectively in the 
*picongpu.profile* that is automatically run at the beginning of a PIConGPU simulation run.  You can see these 
variables in the *picongpu.profile* extract shown below:

.. code-block:: bash


# Set variables and paths
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


A detailed explanation of the Build, Compile, Run process is provided on the PIConGPU ReadTheDocs website 
found at https://picongpu.readthedocs.io/en/latest/usage/basics.html

A summary of that three-step explanation, again using the LaserWakefield PIConGPU Example simulation is provided here.

Step-by-Step
------------

1. Create a Working Copy Input Set
""""""""""""""""""""""""""""""""""

Create a working copy of the entire simulation set of files by using the pic-create helper tool:

.. code-block:: bash

   # Create a working copy of the simulation files
   pic-create $PIC_EXAMPLES/LaserWakefield $PIC_CLONE/myLWFA


PIConGPU is controlled via two kinds of textual input sets: compile-time options and runtime options.

Compile-time :ref:`.param files <usage-params>` reside in ``include/picongpu/param/`` and define the physics case and 
deployed numerics.  When any of these files are changed, the simulation must be re-compiled.

:ref:`Runtime options <usage-cfg>` are set in ``etc/picongpu/*.cfg`` configuration files.
These options include things like simulation size, number of devices, output options, etc., and do notrequire a 
re-compile when changed.

Note the PIC_BACKEND system variable defined in *picongpu.profile*, and seen at the top of the extract shown above.  
PIC_BACKEND specifies the computer hardware that will be used to run the simulation::

   export PIC_BACKEND="omp2b:native"  indicates that the CPU alone is to be used for numeric processing
   export PIC_BACKEND="cuda:86"       indicates that Nvidia GPUs with Compute Capability 86 are to be used

The *picongpu.profile* script is run at the beginning of the compile process.  PIC_BACKEND, and all of the system 
variables defined in *picongpu.profile* are examples of compile-time options. 

2. Compile The Simulation
"""""""""""""""""""""""""

Compiling the simulation is accomplished by switching to the Working directory and using the pic-build helper tool:

.. code-block:: bash

   cd $PIC_CLONE/myLWFA
   pic-build

3. Run The Simulation
"""""""""""""""""""""

Execute the simulation from within the working directory.  The execution instruction below includes using the 
``1.cfg`` configuration file, and creates the output directory: ``$PIC_OUTPUT/lwfa_001`` where simulation output 
is stored:

.. code-block:: bash

   tbg -s bash -c etc/picongpu/1.cfg -t etc/picongpu/bash/mpiexec.tpl $PIC_OUTPUT/lwfa_001

This ``tbg`` instruction also creates a subfolder ``input/`` in the output directory.  This directory contains the 
same structure as the working directory: ``$PIC_CLONE/myLWFA`` to archive the working directory (input) files.  
Subfolder ``simOutput/`` has all the simulation results.  Particularly, the simulation progress log is 
in ``simOutput/output``.

4. The Simulation_Compile_Run Script
""""""""""""""""""""""""""""""""""""

Note that with 4 variables:

PIC_EXAMPLES/LaserWakefield The directory containing the simulation to be run
PIC_CLONE/myLWFA            The directory containing the working copy of the simulation
PIC_CFG/1.cfg               The configuration file to be used
PIC_OUTPUT/lwfa_001         The directory to be used for storing output





