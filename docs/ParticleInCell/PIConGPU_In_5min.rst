.. _PIConGPUIn5min:

PIConGPU in 5 Minutes on a Workstation
======================================

A guide to easily run, but not understand PIConGPU.
It is aimed at users with access to a personal use multi GPU Workstation.

This guide assumes the operator has **admin privelege** on the Workstation.
Consider getting familiar with the shell (*command line*, usually `bash`) and git.


   resources for the command line (bash)
     `a tutorial <http://www.bu.edu/tech/files/2018/05/2018-Summer-Tutorial-Intro-to-Linux.pdf>`_ |
     `another tutorial <https://cscar.research.umich.edu/wp-content/uploads/sites/5/2016/09/Intro-to-Command-Line.pdf>`_ |
     `scripting by examples <https://learnxinyminutes.com/docs/bash/>`_

   resources for git
     `official tutorial <https://git-scm.com/docs/gittutorial>`_ 
     `w3school tutorial <https://www.w3schools.com/git/default.asp>`_ |
     `brief introduction <https://learnxinyminutes.com/docs/git/>`_ |
     `cheatsheet (by github) <https://training.github.com/downloads/github-git-cheat-sheet.pdf>`_
  
We will use the following files and directories:

- `$HOME/src/picongpu`: source files from github
- `picongpu.dependencies`: loads PIConGPU dependencies
- `picongpu.profile`: sets up directories and system variables
- `$HOME/picInputs`: scenarios to simulate
- `$HOME/scratch/runs`: output of the simulation runs

Setup
-----

Use executable scripts to set up the directory structure and install software dependencies used by PIConGPU.
Download *picongpu.dependencies* and *picongpu.profile* executable scripts by downloading the kimjohn1/SCIRun repo::


  git clone https://github.com/kimjohn1/SCIRun.git
  git checkout PIConGPU

  cp $HOME/SCIRun/scripts/picongpu/picongpu.dependencies .
  cp $HOME/SCIRun/scripts/picongpu/picongpu.profile .

Install PIConGPU
----------------

Install the PIConGPU dependencies with::

  source picongpu.dependencies

Set up a simulation
-------------------

As an example we will use the example LaserWakefield simulation.
Copy the *LaserWakefield_compile_run* script to the $HOME directory::

  cp $HOME/SCIRun/scripts/picongpu/LaserWakefield_compile_run .

Compile and Run
---------------

Compile and run the simulation with::

  source LaserWakefield_compile_run

This may take a while, go grab a coffee.

Examine the Results
-------------------

Results are located at::

  $HOME/scratch/runs/lwfa_001/simOutput

To view pretty pictures you can have a look at the images found at::

  $HOME/scratch/runs/lwfa_001/simOutput/pngElectronsYX

Further Reading
---------------

You now know a simple process for using PIConGPU.
Carry on reading the documentation to understand it.
