SCIRun With PIConGPU Simulation Capability
==========================================

This guide is for use with a Linux OS, and was developed and tested using the following software versions:

- Linux Mint 20.3
- Nvidia CUDA Toolkit version xxx
- PIConGPU version xxx
- SCIRun version xxx

Simulation output uses openPMD, which is presented as an open standard for particle-mesh data files.  Reference::

    https://github.com/openPMD/openPMD-api
    https://openpmd-api.readthedocs.io/en/latest/?badge=latest

The PIConGPU Particle In Cell implementation in SCIRun includes output cability for either real time in situ visualization or for storing data for post run analysis.  The target use case is real time in situ visualization for a single user (researcher) using their Personal Workstation computer, for quick analysis of a simulation experiment.  The researcher could invest time and resources to develop a complete simulation package to be run on available supercomputer hardware if the quick analysis indicated potential value.

The set up described in this guide uses Nvidia GPUs and the CUDA Toolkit for math processing.  PIConGPU can be run on other accelerated math processing hardware and can use other acceleration enabling software (ROCm or OpenCL).  See the PIConGPU resource for more information::

    https://github.com/ComputationalRadiationPhysics/picongpu

The PIConGPU simulation software has been developed for running on exascale supercomputer hardware. The target platform hardware for this project is a single user Workstation computer.  The software developed for this project will run on typical laptop and desktop computers, with or without GPU support, with simulation execution time being considerably longer for non-GPU equipped machines.

For optimum performance on a Workstation, the PIConGPU developers recommend using multiple GPUs, GPUs with large onboard memory capacity, and approximately twice as much system memory (RAM) as the total amount of GPU memory for all attached GPUs.

Installation
------------

SCIRun with PIConGPU simulation capability requires installation of three large software programs; The CUDA Toolkit, PIConGPU and SCIRun.  It may be useful to take system restore points prior to running the installation for each of these programs in case something goes wrong with the installation.

Install Nvidia CUDA Toolkit
"""""""""""""""""""""""""""

- Go to https://developer.nvidia.com/cuda-downloads
- Step through the selections for Linux, X86_64, Ubuntu, 20.04, runfile (local)
- Execute the 2 instructions as described, then:

  - Click continue when the warning appears
  - Scroll to the bottom of the user agreement, type ``accept`` then enter
  - When the GUI is presented, un-check (space bar) everything except ``CUDA Toolkit``
  - Scroll down to ``install``
  - Click ``enter``

- Set the CUDA Toolkit path (note that this is for CUDA version 11.7)::

    sudo su
    echo "export PATH=/usr/local/cuda-11.7/bin${PATH:+:${PATH}}" >> /etc/profile
    reboot

Install PIConGPU
""""""""""""""""

Reference:

    https://picongpu.readthedocs.io/en/latest/install/path.html

    https://picongpu.readthedocs.io/en/latest/install/dependencies.html

The second reference provided above is the PIConGPU installation from source procedure, and is the process implemented in the dev2_picongpu.dependencies script.  The script installs the following dependencies:

- The gcc compiler available from the build-essential repository package
- The latest version of CMake available from the Kitware repository
- openMPI
- zlib
- boost
- git
- rsync
- picongpu source code (latest development branch)
- libpng
- pngwriter
- hdf5
- ADIOS2
- openPMD

The script is run from the home directory.  Open a terminal window and execute the following::

    git clone https://github.com/kimjohn1/SCIRun.git
    git checkout PIConGPU

    cp $HOME/SCIRun/scripts/PIConGPU/dev2_picongpu.dependencies .
    cp $HOME/SCIRun/scripts/PIConGPU/picongpu.profile .

    source dev2-picongpu.dependencies

Install SCIRun
""""""""""""""

Reference:

    https://scirun.readthedocs.io/en/latest/index.html

Install dependencies::

    sudo apt update
    sudo apt install qt5-qmake cmake-curses-gui
    sudo apt install libgl1-mesa-dev
    sudo apt install libglfw3-dev libglu1-mesa-dev
    sudo apt install libffi-dev

Install OSPRay::

    cd $HOME
    git clone https://github.com/ospray/ospray.git
    sudo apt install libtbb-dev
    sudo apt install xorg-dev

    mkdir build
    cd build
    cmake ../ospray/scripts/superbuild
    cmake --build .

Install Qt version 5.13.2

If you don't have a Qt account, create one:

    https://login.qt.io/login

Download the installer:

    https://download.qt.io/archive/qt/

-   Click on 5.13
-   Click on 5.13.2
-   Click qt-opensource-linux-x64-5.13.2.run
-   Open a terminal

Navigate to the Downloads directory and do the following::

    chmod +x qt-opensource-linux-x64-5.13.2.run
    ./qt-opensource-linux-x64-5.13.2.run

Enter your Qt user name and password, then step through the installation process.  When complete, set the Qt path with::

    sudo su
    export PATH=/home/kj/Qt5.13.2/5.13.2/gcc_64:$PATH

Installation is complete.

SCIRun With PIConGPU
--------------------

Reference:

    https://scirun.readthedocs.io/en/latest/index.html

    https://picongpu.readthedocs.io/en/latest/index.html

*SCIRun description and references*

PIConGPU is a fully relativistic, manycore particle-in-cell (PIC) code originally presented in *PIConGPU: A Fully Relativistic Particle-in-Cell Code for a GPU Cluster*:

    Burau, H. et al., "PIConGPU: A Fully Relativistic Particle-in-Cell Code for a GPU Cluster," in IEEE Transactions on Plasma Science, vol. 38, no. 10, pp. 2831-2839, Oct. 2010, doi: 10.1109/TPS.2010.2064310.

It is developed and maintained by the Computational Radiation Physics Group at the Institute for Radiation Physics at HZDR in close collaboration with the Center for Information Services and High Performance Computing (ZIH) of the Technical University Dresden (TUD).

An extensive list of citations and references is available at:

    https://picongpu.readthedocs.io/en/latest/usage/reference.html

The PIConGPU application is portable to multiple hardware architectures but the emphasis and forte of the application is optimized performance on GPU hardware.

PIConGPU is implemented in SCIRun as an independent program running in a separate process.  The PIConGPU module in SCIRun implements a User Interface that allows the User to select the particular simulation to be run, select the configuration file to be used, and to set the paths and names of the directories to be used to hold a working copy of the simulation, and output generated by the simulation during execution.

If the User sets ``sst.cfg`` as the configuration file to be used, simulation output data is not saved, and the PIConGPU simulation passes output data to SCIRun using the openPMD implementation of a streaming Writer/Reader communication.

PIConGPU Module User Interface
------------------------------



openPMD Streaming
-----------------



File Structure
--------------



Operation
---------

