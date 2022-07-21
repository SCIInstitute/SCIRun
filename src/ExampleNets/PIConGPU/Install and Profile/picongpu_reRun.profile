# Name and Path of this Script ############################### (DO NOT change!)
export PIC_PROFILE=$(cd $(dirname $BASH_SOURCE) && pwd)"/"$(basename $BASH_SOURCE)

# Note: The code that calls this script deletes the existing output directory

# Set variables and paths
export PIC_BACKEND="omp2b:native"   # running on CPU
#export PIC_BACKEND="cuda:61"        # running on GPU, compute capability 6.1 (GTX 1070)
#export PIC_BACKEND="cuda:75"        # running on GPU, compute capability 6.1 (GTX 2060)
#export PIC_BACKEND="cuda:86"        # running on GPU, compute capability 8.6 (RTX 3060)

export SCRATCH=$HOME/scratch
export PICSRC=$HOME/src/picongpu
export PIC_EXAMPLES=$PICSRC/share/picongpu/examples
export PIC_CLONE=$HOME/picInputs
export PIC_CFG=etc/picongpu
export PIC_OUTPUT=$SCRATCH/runs

export PATH=$PATH:$PICSRC:$PICSRC/bin:$PICSRC/src/tools/bin
export PYTHONPATH=$PICSRC/lib/python:$PYTHONPATH

#Un-comment the next 2 lines if HDF5 was installed from source
#export CMAKE_PREFIX_PATH=$HOME/lib/hdf5:$CMAKE_PREFIX_PATH
#export LD_LIBRARY_PATH=$HOME/lib/hdf5/lib:$LD_LIBRARY_PATH
export CMAKE_PREFIX_PATH=$HOME/lib/ADIOS2:$CMAKE_PREFIX_PATH
export LD_LIBRARY_PATH=$HOME/lib/ADIOS2/lib:$LD_LIBRARY_PATH
export CMAKE_PREFIX_PATH=$HOME/lib/pngwriter:$CMAKE_PREFIX_PATH
export LD_LIBRARY_PATH=$HOME/lib/pngwriter/lib:$LD_LIBRARY_PATH
export CMAKE_PREFIX_PATH=$HOME/lib/openPMD-api:$CMAKE_PREFIX_PATH
export LD_LIBRARY_PATH=$HOME/lib/openPMD-api/lib:$LD_LIBRARY_PATH

# Set "tbg" default options ######################################
#
export TBG_SUBMIT="bash"
export TBG_TPLFILE="etc/picongpu/bash/mpirun.tpl"

# Load autocompletion for PIConGPU commands
#

BASH_COMP_FILE=$PICSRC/bin/picongpu-completion.bash
if [ -f $BASH_COMP_FILE ] ; then
    source $BASH_COMP_FILE
else
    echo "bash completion file '$BASH_COMP_FILE' not found." >&2
fi

echo PIC_BACKEND is: $PIC_BACKEND

# Install and Run notes:
#

#OpenMP version
# Check with: echo |cpp -fopenmp -dM |grep -i open
# A return of: #define _OPENMP 201511 means OpeMP is version 4.5, see https://stackoverflow.com/questions/1304363/how-to-check-the-version-of-openmp-on-linux

# Compile and run PIConGPU
#Modify the 1.cfg file if needed, this becomes unnecessary after PIConGPU issue 4004 is implemented
#pic-create $PIC_EXAMPLES/LaserWakefield $HOME/picInputs/myLWFA
#cd $HOME/picInputs/myLWFA
#pic-build 2>err.txt | tee out.txt
#tbg -s bash -c etc/picongpu/1.cfg -t etc/picongpu/bash/mpiexec.tpl $SCRATCH/runs/lwfa_001 2>erroutput

# Install CUDA Toolkit v11.2
#wget https://developer.download.nvidia.com/compute/cuda/11.2.0/local_installers/cuda_11.2.0_460.27.04_linux.run
#sudo sh cuda_11.2.0_460.27.04_linux.run
#When the little GUI is presented, click on continue, then type accept at the bottom
#When the GUI presents the install options, un-check all except CUDA Toolkit, then highlight Install and press Enter

#Temporarily set the CUDA Toolkit (v11.2) path with:
#export PATH=/usr/local/cuda-11.2/bin${PATH:+:${PATH}

#Make the CUDA Toolkit (v11.2) path permanent with the following
#sudo su
#echo "export PATH=/usr/local/cuda-11.2/bin${PATH:+:${PATH}}" >> /etc/profile
#reboot

